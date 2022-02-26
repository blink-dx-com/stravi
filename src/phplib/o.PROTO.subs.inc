<?php
require_once ("f.assocUpdate.inc");

/**
 * manipulation functions for concrete protocol steps (CONCRETE_PROTO_STEP)
 * - the functions handle exact ONE step
 * - update or insert one CONCRETE_PROTO_STEP
 * - main functions: setCprotoID() oneStepCreate() oneStepSave()
 * @package o.PROTO.subs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oCprotoMani {
    
    private $rowHasData=0;
    const NOTES_MAX_LEN=4000;
    
    // Map: Update-keys => real column names
    const UP_KEY_2_COL = array(
        "quantity"	=>"QUANTITY",
        "subst"		=>"CONCRETE_SUBST_ID",
        "dev"		=>"DEV_ID",
        "notes"		=>"NOTES",
        "not_done"	=>"NOT_DONE"
    );
    const COL_2_UP_KEY = array(
        "QUANTITY"  => "quantity",
        "CONCRETE_SUBST_ID"	=>"subst",
        "DEV_ID"    =>"dev",
        "NOTES"		=>"notes",
        "NOT_DONE"	=>"not_done"
    );
    
    // new structure: PROTO_STEP_short_keys ...
    const SHORT_KEYS2REAL=array(
        'qu'=> 'QUANTITY',
        'cs'=> 'CONCRETE_SUBST_ID',
        'dev'=>'DEV_ID',
        'cn'=> 'NOTES',
        'dea'=> 'NOT_DONE',
    );
    const REAL2SHORT_KEYS=array(
        'QUANTITY'=>'qu',
        'CONCRETE_SUBST_ID'=>'cs' ,
        'DEV_ID'  =>'dev',
        'NOTES'   =>'cn' ,
        'NOT_DONE'=>'dea',
    );
    
    /**
     * 
     * @param object $sqlo
     * @param int $cpid
     * @param int $apid - can be 0 on some methods ...
     * @param number $acheck
     */
    function setCprotoID ( &$sqlo, $cpid, $apid, $acheck=0 ) {
    	$this->cpid = $cpid;
    	$this->apid = $apid;
    
    	
    
    	$this->assoclib = new  fAssocUpdate();
    	$asopt = NULL;
    	if ( !$acheck ) $asopt['noAccCheck']=1;
    	$this->assoclib->setObj( $sqlo, 'CONCRETE_PROTO_STEP', $cpid, $asopt );
    	$this->assoclib->set_log_level(fAssocUpdate::MASS_LOG_FIRST);
    }
    
    // get maparr = array[COL] = Update-keys
    function getColMap() {
        return self::COL_2_UP_KEY;
    }
    
    /**
     * - create a single empty protocol step (if not exist)
     * - with touch management
     * @param  $sql
     * @param  $step_nr
     * @param  $opt // DEPRICATED: "acheck" = [0] | 1  with_access_check ??? 
     * @return int flag, if created 0|1
     */
    function oneStepCreate( &$sql, $step_nr,$opt  = NULL) { 
    	$cpid = $this->cpid;
    	$apid = $this->apid;
        // $rowHasData = 0;
    	
    	$sqls= "select 1 from CONCRETE_PROTO_STEP ".
               " where CONCRETE_PROTO_ID=".$cpid. " AND STEP_NR=" . $step_nr;
        $sql->query($sqls);
        if ( $sql->ReadRow() ) { 
    		return (0); // already exists
    	}
    	
    	$argu = NULL;
    	$argu["ABSTRACT_PROTO_ID"] = $apid;
    	$argu["STEP_NR"]  = $step_nr;
    		
    	$this->assoclib->insert( $sql, $argu );
    	return 1;
    }
    
    /**
     * check the input for USER-errors
     * @param object $sqlo
     * @param array $params
     * @param int $step_nr
     * @param array $opt
     * @return array of errors
     * 's'=> STRING
     * 'n'=> STRING
     * 'q'=> STRING
     */
    function oneStepInputCheck(object$sqlo, array $args_in, int $step_nr ) {
        
        $err_inf=array();
        
        if ($args_in["quantity"]!="") {
            if (!is_numeric($args_in["quantity"])) {
                $err_inf['q']='Is not a valid number!';
            }
        }
        
        // check each substance if exists in DB
        if ($args_in["subst"]!=NULL) {
            do {
                if (!is_numeric($args_in["subst"])) {
                    $err_inf['s']='Is not a valid number!';
                    break;
                }
                $args_in["subst"] = trim($args_in["subst"]);
                if ( !gObject_exists ($sqlo, 'CONCRETE_SUBST', $args_in["subst"]) ) {
                    $err_inf['s']='MAC:'.$args_in["subst"].' does not exist in DB';
                }
            } while (0);
        }
        if ($args_in["notes"]!=NULL) {
            if (strlen($args_in["notes"]> self::NOTES_MAX_LEN)) {
                $err_inf['n']='Too many chars. MAX:'.self::NOTES_MAX_LEN;
            }
        }

        
        
        return $err_inf;
    }
    
    /**
     * - update OR create one proto_step, identified by $step_nr
     * - the input data will NOT be checked for USER-INPUT-format-errors : use oneStepInputCheck()
     * - if  $opt["acheck"] => check access and touch protocol
     * - produces flag $this->rowHasData
     * @param object $sql
     * @param array  $new   array of values:  ["quantity"], ["notes"], ["subst"], ["dev"], ["not_done"]
     * @param int    $step_nr
     * @param array  $opt  options
     * 		"acheck"     : [0] | 1  with_access_check ??? 
     *      "steps.slim" : -1 , [1]  on CREATE: only insert a STEP if it has data ...
    		"defcols"    : 1 -- just update defined columns in $new,
    				            with this option, a step can NOT be removed
     * @return $info info-string
     */
    function oneStepSave( &$sql, $new, $step_nr, $opt  = NULL) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
    	$cpid = $this->cpid;
    	$apid = $this->apid;
    	$info ='StepNr: '.$step_nr.': ';
    	
    	$check_rowHasData= 1;
        $this->rowHasData = 0;
    	$maparr     = self::UP_KEY_2_COL;
    	
    	if (!is_array($new)) {
    		$new = array(); // if $new = NULL
    	}
    	if ($opt["steps.slim"]<0) {
    	    $check_rowHasData= 0;
    	}
    
    
    	// check values exist ?
    	if ( $check_rowHasData ) {
    		if ( is_array($new) ) {
    			foreach( $new as $key=>$val) {
    			    if ($key=='not_done' and !$val) {
    			        continue; // no specific data
    			    }
    				if ($val!==NULL and $val!=='') $this->rowHasData = 1;  // is not NULL and not empty string
    			}
    			$info .= glob_array2String( $new );
    		}
    		//$logx = &$sql->_sql_log;
    		//$logx->write_textline( 'step:'.$step_nr.' '.print_r($new,1). ' rowHasData:'.$this->rowHasData  );
    	}
    
        $sqls = "select 1 from CONCRETE_PROTO_STEP ".
                " where CONCRETE_PROTO_ID=".$cpid. " AND STEP_NR=" . $step_nr;
        $sql->query($sqls);
        
        if ( $sql->ReadRow() ) { //update
            
             // also, if DATE is empty
    		$argu = NULL;
    		$argu ["ABSTRACT_PROTO_ID"] = $apid;
    		$idarr = NULL;
    		$idarr["STEP_NR"]          = $step_nr;
    
    		// only if  key is set ...
    		if ( array_key_exists('not_done', $new) and ($new['not_done']==NULL) )  
    			$new["not_done"] = 0;  // must be 0 or 1
    		
    		if ( $opt["defcols"] == 1) {
    			//  just update defined columns in $new
    			foreach( $maparr as $key=>$argukey) {
    				if (  array_key_exists($key, $new) ) $argu[$argukey] = $new[$key];
    			}
    			 
    		} else { 
    			// update each column, even if the value is not set !
    			foreach( $maparr as $key=>$argukey) {
    				$argu[$argukey]	       = $new[$key];
    			}
    			
    		}
    
    		$retval = $this->assoclib->update( $sql, $argu, $idarr ); 
    		$info .= " updated (retval:$retval) ARGS:".print_r($argu,1);
                	
            
        } else { // insert
        	
            if ( $this->rowHasData ) {
            
                if ($new["not_done"]==NULL)  $new["not_done"] = 0; // must be set !
    		
    			$argu = array();
    			$argu["ABSTRACT_PROTO_ID"] = $apid;
    			$argu["STEP_NR"]  = $step_nr;
    			
    			//Check if stepnr exists for ABSTRACT_PROTO with ABSTRACT_PROTO_STEP
    			$keyArray = array( "STEP_NR"=>$step_nr, "ABSTRACT_PROTO_ID"=>$apid ); 
    			if (!gElement_exist2($sql, "ABSTRACT_PROTO_STEP", $keyArray )) {
    				$error->set( $FUNCNAME, 1, "Invalid Step Nr ".$step_nr." for abstract Protocol(ID: ". $apid.")");
    				return;
    			}		
    			
    			foreach( $maparr as $key=>$argukey) {
    				$argu[$argukey]	= $new[$key];
    			}

    			$retval = $this->assoclib->insert( $sql, $argu);
                $info   .= " inserted ($retval) ARGS:".print_r($argu,1);
                
            }
        } 
    	
        return ($info);
    }   
    
    function get_rowHasData() {
        return $this->rowHasData;
    }
    
    /**
     * get touch info dict
     * @return array ('act'=>$this->touchAct, 'timest'=>$this->touchTimeStamp)
     */
    function getTouchDict() {
    	return $this->assoclib->getTouchDict();
    }
    
    /**
     * RAW input
     * @param object $sqlo
     * @param array $argu
     */
    function oneStepInsertRaw(&$sqlo, array $argu) {
        $this->assoclib->insert( $sqlo, $argu);
    }
    
    function close(&$sqlo) {
    	$this->assoclib->close($sqlo);
    }

}
