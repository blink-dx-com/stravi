<?php
require_once ("f.assocUpdate.inc");

/**
 * class for ABSTRACT protocol : modify steps
 * @package o.ABSTRACT_PROTO.stepMod.inc
 * @swreq UREQ:xxxxxxxxxxxx
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/o.ABSTRACT_PROTO.stepMod.inc 59 2018-11-21 09:04:09Z $
 */
class oAbsProtoStepMod {

    function init(&$sqlo, $protoid) {
        
    	$this->protoid = $protoid;
    	if (!$this->protoid) {
    	    sys_error_my( "Input: protoid missing." );
    	}
    	$this->touched = 0;
    	
    	$this->assoclib = new  fAssocUpdate();
    	$this->assoclib->setObj( $sqlo, 'ABSTRACT_PROTO_STEP', $this->protoid );
    	
    }
    
    function set_log_level($flag) {
        $this->assoclib->set_log_level($flag);
    }
    
    /**
     * insert new steps
     * @param object $sqlo
     * @param array $steparr = array of
     *   STEP_NR => array('NAME', 'ABSTRACT_SUBST_ID', 'SUB_STEP_NR', ...)
     *    !!! MAIN_STEP_NR can not be set, will be overwritten !
     *    SUB_STEP_NR can be set !
     */
    function insert_steps(&$sqlo, $steparr) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	if (!$this->protoid) {
    	    sys_error_my( "Class not initialized." );
    	}
    	
    	$mainstep_nr=1;
    	
    	foreach($steparr as $step_nr => $oneStepArr ) {
    		
    	    if (!$step_nr) {
    	        $error->set( $FUNCNAME, 1, 'Input: Step_nr must be a positive number!');
    	        return;
    	    }
    	    
    		$argu = $oneStepArr;
    		$argu['STEP_NR']      = $step_nr;
    		$argu['MAIN_STEP_NR'] = $mainstep_nr;
    		$substep=0;
    		if ($oneStepArr['SUB_STEP_NR']>0) $substep=$oneStepArr['SUB_STEP_NR'];
    		$argu['SUB_STEP_NR'] = $substep;
    		
    		$this->assoclib->insert( $sqlo, $argu );
    		if ($error->Got(READONLY))  {
    			$error->set( $FUNCNAME, 1, 'Error on Step:'.$step_nr );
    			return;
    		}
    		
    		$mainstep_nr++;
    	}
    	
    }
    
    /**
     *
     * @param object $sqlo
     * @param int $step_nr
     * @param array $argu
     */
    function insert_step(&$sqlo, $step_nr, $argu) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$this->protoid) {
            sys_error_my( "Class not initialized." );
        }
    
        $argu['STEP_NR'] = $step_nr;
        $this->assoclib->insert( $sqlo, $argu );
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on insert of Step:'.$step_nr );
            return;
        }
    
    }
    
    /**
     * 
     * @param object $sqlo
     * @param int $step_nr
     * @param array $argu
     */
    function update_step(&$sqlo, $step_nr, $argu) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$this->protoid) {
            sys_error_my( "Class not initialized." );
        }
       
        if ($argu['SUB_STEP_NR']<=0)  $argu['SUB_STEP_NR'] = 0;  // correct bad values
        
        $idarr = array('STEP_NR'=> $step_nr);
        $this->assoclib->update( $sqlo, $argu, $idarr );
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on Step:'.$step_nr );
            return;
        }
            
    }
    
    function step_nr_exists($sqlo, int $step_nr) {
        $out = 0;
        $sqlsel = "STEP_NR from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$this->protoid .' and STEP_NR='.$step_nr;
        $sqlo->Quesel($sqlsel);
        if ($sqlo->ReadRow() ) {
            $out=1;
        }
        return $out;
    }
    
    /**
     * move MAIN_STEP_NR of step_arr to START (behind)
     * 
     * x   OLD-Step a
     * x+1 new step 1
     * x+2 new step 2
     * x+3 OLD-step b <== first match
     * @param object $sqlo
     * @param array $step_nr_arr -- steps already have an old value for MAIN_STEP_NR ...
     * @param int $start_nr_beh
     *    -1 - at beginning (or 0)
     *    -2 - at the end
     */
    function move_steps( object $sqlo, array $new_step_arr, int $start_nr_beh ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$this->protoid) {
            sys_error_my( "Class not initialized." );
        }
        
        $old_steps = array();
        
        $start_main_step_nr = 0;
        debugOut('(139) start_nr_beh: '.$start_nr_beh, $FUNCNAME, 1);
        
        if ($start_nr_beh==-1) $start_nr_beh=0;
        
        if ($start_nr_beh>=0) {
            
            $do_caching = 0;
            
            if (!$start_nr_beh) $do_caching = 1;
          
            $sqlsel = "STEP_NR, MAIN_STEP_NR ".
                " from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$this->protoid.
                " ORDER by MAIN_STEP_NR";
            $sqlo->Quesel($sqlsel);

            while ( $sqlo->ReadRow() ) {
                
                $step_nr= $sqlo->RowData[0];
                $main_nr= $sqlo->RowData[1];

                if ($do_caching) {
                    if (!$start_main_step_nr) $start_main_step_nr=$main_nr;
                    
                    if (in_array($step_nr, $new_step_arr) ) {
                        // ignore STEP-NR of steps TO-BE-MOVED
                        continue;
                    }
                    
                    $old_steps[]=$step_nr; // reorder these steps ...
                } else {
                    if ($step_nr==$start_nr_beh) {
                        // start behind this step_nr
                        $do_caching=1;
                        continue;
                    }
                }

            }

        } 
        
        if (empty($old_steps)) {
            $sqlsel = "max(MAIN_STEP_NR) ".
                " from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$this->protoid ;
            $sqlo->Quesel($sqlsel);
            $sqlo->ReadRow();
            $start_main_step_nr = $sqlo->RowData[0];
            if (!$start_main_step_nr) $start_main_step_nr=1;
        }

        
        // merge arrays + renumerate
        $main_step_nr    = $start_main_step_nr;
        $up_steps_nr_arr = array_merge($new_step_arr, $old_steps);
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
            
            debugOut('start_main_step_nr: '.$start_main_step_nr.' $up_steps_nr_arr: '.print_r($up_steps_nr_arr,1), $FUNCNAME, 1);
        }
        
        // update object
        $this->assoclib->set_log_level(fAssocUpdate::MASS_LOG_POS_RANGE);
        foreach($up_steps_nr_arr as $step_nr)  {
            $argu=array('MAIN_STEP_NR'=>$main_step_nr);
            $this->update_step($sqlo, $step_nr, $argu);
            $main_step_nr++;
        }
        $this->assoclib->close($sqlo);
        
        
    }
    
    /**
     * delete one step
     * @param  $sql
     * @param int $step_nr
     */
    function f_del(&$sqlo, $step_nr) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	if (!$this->protoid) {
    	    sys_error_my( "Class not initialized." );
    	}
    	
    	$sqlsWhere= "ABSTRACT_PROTO_ID=".$this->protoid. " AND STEP_NR=".$step_nr;
    	
    	// first check, if used in CONCRETE_PROTO to prevent hardcore error
    	// SQL-error: "integrity constraint (CCT.FK_A_PROTO_STEP2C_PROTO_STEP) violated"
    	$sqlsel = 'CONCRETE_PROTO_ID from CONCRETE_PROTO_STEP where '.$sqlsWhere;
    	$sqlo->Quesel($sqlsel);
    	if ( $sqlo->ReadRow() ) {
    		$retid = $sqlo->RowData[0];
    		$error->set( $FUNCNAME, 2, 'step (PRA-ID:'.$this->protoid.', ST:'.$step_nr.') already used in a "concrete protocol".' );
    		return;
    	}
    	
    	$idarr=array('STEP_NR'=>$step_nr);
    	$this->assoclib->delOneRow($sqlo, $idarr);
    	
    	if ($error->Got(READONLY))  {
    	    $error->set( $FUNCNAME, 1, 'Error on delete of Step:'.$step_nr );
    	    return;
    	}
    	
    	
    }
    
    function close($sqlo) {
        $this->assoclib->close($sqlo);
    }

}