<?php

/**
 * workflow actions
 * @package o.CCT_ACCLOG.wfl.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oAccLogWfl {
    
    const STATE_END=-2;
    private $wfl_states;
    
    function __construct($tablename) {
        $this->tablename = $tablename;
    }
    
    /**
     * test, if key flow.def exists for business object in MOD_META
     * @param $sql, $cct_access_id
     * @return  NULL, if not exists, $flowdef, if set (serialized value of flow.def in MOD_META
     */
    static function testifFlow(&$sql, $wflow_id){
        $sqlsel = "VALUE from MOD_META where MXID = " . $wflow_id . "and KEY = 'flow.def'";
        $sql->Quesel($sqlsel);
        $sql->ReadRow();
        if ($sql->RowData[0] == NULL){
            return NULL;
        } else{
            $flowdef = $sql->RowData[0];
            return $flowdef;
        }
    }
    
    /**
     * 
     
     * @param int $oldStatusID
     * @param string $old_WflStName
     * @param int $newStatusID
     * @param string $newStatusName
     * @return array
     *    array( 
     *    'ok'=>
     *        1: OK
     *       -1: Final status reached
     *       -2:Current status is not part of the workflo
     *       -3 New status  is not allowed from old status
     *    'fieldx'=>$fieldsx,
     *    'pos_states' => $possibleStates
     *    );
     *    
     *    
     */
    private function _flow_manage($oldStatusID, $old_WflStName, $newStatusID, $newStatusName) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        if ($this->tablename==NULL) {
            throw new Exception('Class not initialized: tablename missing');
        }
        
        $possibleStates = array();
        
        $wfl_states = $this->wfl_states;
        
        // check, if LAST status == "started" is there define ?
        //	 YES: normal going
        //   NO: if status=='started' ==> do like it is NULL ...
        if ($old_WflStName=='started' and !$wfl_states[$oldStatusID]) {
            $oldStatusID = "0";// simulate the start
        }
        
        if ($wfl_states[$oldStatusID] != NULL){
            
            foreach ($wfl_states[$oldStatusID] as $state => $value){
                if ($value == 1){
                    if ($state == self::STATE_END){
                        $fieldsx = array ("title" => "Final status reached!", "object"=> "info");
                        return array('ok'=>-1, 'fieldx'=>$fieldsx);
                    } else {
                        $possibleStates[] = $state;
                        //$tmpkomma = ",";
                    }
                }
            }
            
        } else {
            $fieldsx = array ("title" => "Current status is not part of the workflow!", "object"=> "info");
            return array('ok'=>-2, 'fieldx'=>$fieldsx);
        }
        
        if ( $newStatusID>0 and !in_array($newStatusID, $possibleStates) ) {
            $fieldsx = array ("title" => 'New status "'.$newStatusName.'" is not allowed from old status "'.
                $old_WflStName.'" by the workflow!', "object"=> "info", "backcolor"=>"#FFE0E0");
            return array('ok'=>-3, 'fieldx'=>$fieldsx);
        }
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
            debugOut('(86) : wfl_states: '.print_r($wfl_states,1), $FUNCNAME, 1);
            debugOut('(86) : possibleStates: '.print_r($possibleStates,1), $FUNCNAME, 1);
        }

        return array('ok'=>1, 'pos_states' => $possibleStates );
    }
    
    /**
     * workflow has $statusID entries for  SOURCE ID ?
     * @param int $statusID
     * @return NULL|number
     */
    private function wfl_states_have_SRC_state($statusID) {
        if (empty($this->wfl_states[$statusID])) {
            return NULL;
        } else {
            return 1;
        }
    }

    
    /**
     * analyse Workflow for OLD status $oldStatusID
     * @param object $sql
     * @param int $oldStatusID  OLD
     * @param int $newStatusID  NEW
     * @param string  $flow_str (serialized) contains the workflow-flow
     * @param array $formOpt
     *   "ignore" 0,1 QM ignored the status dependencies
     * @return array
     *     'ok'=> int
     *     'fieldx'   => array -- variable exists on workflow problems ...
     *     'ok_states'=> array( H_ALOG_ACT_ID => NAME) POSSIBLE STATES
     *     'status.preset' => int
     */
    function workflow_ana($sql, $oldStatusID, $newStatusID, $flow_str, $formOpt) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if ($this->tablename==NULL) {
            throw new Exception('Class not initialized: tablename missing');
        }
        
        $iniarr = array();
        $this->wfl_states = NULL;
        if ($flow_str!=NULL) {
            $this->wfl_states = unserialize($flow_str);
        }
        
        if ($oldStatusID) $old_WflStName  = oH_ALOG_ACT_subs::statusID2Name($sql, $oldStatusID);
        else $old_WflStName='';
        $newStatusName  = oH_ALOG_ACT_subs::statusID2Name($sql, $newStatusID);
        
        $preSetStatusID = $newStatusID;
        
        $table_states = oH_ALOG_ACT_subs::getActionEntries($sql, $this->tablename);
        $this->reopen_ID    = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sql, $this->tablename, "reopen" );
        
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
            debugOut('(125) :  tablename: '. $this->tablename.' table_states: '.print_r($table_states,1), $FUNCNAME, 2);
        }
        
        /**
         * SUBREQ:
         *  - always possible: quarantine, rejected, finished, reopen
         *  - else: check workflow
         */
        $allowedStats = array('quarantine', 'rejected', 'finished', 'reopen');
        
        do {
            if ($newStatusID) {
                if (in_array($newStatusName, $allowedStats)) {
                    // o.k., always allowed; no further check
                    // no flow check.
                    $iniarr[$newStatusID] = $newStatusName;
                    break;
                }
            }
            
            $wfl_has_reopen = $this->wfl_states_have_SRC_state($this->reopen_ID);
            
            $do_flow_manage=0;
            do {
                if (!empty($this->wfl_states)) $do_flow_manage=1;
                if (!$oldStatusID) {
                    $do_flow_manage=0;
                    break;
                }
                if ( $formOpt["ignore"]==1)  {
                    $do_flow_manage=0;
                    break;
                }
                if ($oldStatusID==$this->reopen_ID and !$wfl_has_reopen) {
                    // old status =='reopen', but 'reopen' is NOT part of the current workflow ???
                    $do_flow_manage=0;
                }
                
            } while (0);
            
            
            if ( $do_flow_manage ) {
                
               // a flow exists ...    
               $fl_output = $this->_flow_manage($oldStatusID, $old_WflStName, $newStatusID, $newStatusName);
               if ($fl_output['ok']<0) return $fl_output;
               
               $possible_States = $fl_output['pos_states'];
               
               // build form inits array
               foreach($table_states as $state_p_id => $state_name) {
                   
                   if ( $formOpt["noReopen"] == 1 && $state_name == "reopen" ) {
                       continue;
                   } else {
                       if (in_array($state_p_id, $possible_States)) {
                           $iniarr[$state_p_id] = $state_name;
                       }
                   }
                   
               }
                   
            } else {
                
                // no flow exists ...   
                
                // build form inits array
                foreach($table_states as $state_p_id => $state_name) {
                    if ( $formOpt["noReopen"] == 1 && $state_name == "reopen" ) {
                        continue;
                    } else {
                        $iniarr[$state_p_id] = $state_name;
                    }
                }
            }

           
           
            
            // preset $newStatusID
            if (sizeof($iniarr) == 1 && $newStatusID== NULL){
                $newStatusID    = key($iniarr);
                $preSetStatusID = $newStatusID;
            }
        } while (0);
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
            debugOut('(144) : oldStatusID: '.$oldStatusID . ' newStatusID:'.$newStatusID.
                ' UseTheWorkflow? '.$do_flow_manage. '; formOpt[ignore]:'.$formOpt["ignore"].'; ', $FUNCNAME, 1);
            debugOut('(144) : iniarr: '.print_r($iniarr,1), $FUNCNAME, 1);
            //debugOut('(145) : stateArr:'.print_r($wfl_states,1), $FUNCNAME, 1);
        }
        
        return array('ok'=>1, 'fieldx'=>NULL, 'ok_states'=>$iniarr, 'status.preset'=>$preSetStatusID);
        
        
    }
}
