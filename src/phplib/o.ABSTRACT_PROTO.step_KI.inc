<?php
/**
 * protocol template KI: search IDs of ABSTRACT_PROTO by STEP_NR + NAME
 * @package o.ABSTRACT_PROTO.step_KI.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 *
 */
class o_ABSTRACT_PROTO_step_KI_1 {
    
    /**
     * 
     * @var array
     *   'name' => step name
     *   'st'   => step
     */
    private $step_arr;
    private $detail_arr;
    
    function set_steps($step_arr) {
        $this->step_arr=$step_arr;
    }
    
    /**
     * 
     * @param object $sqlo
     * @return array
     *   'ids'  => array (ABSTRACT_PROTO_ID => count of found steps)
     *   'status'=> text
     */
    function analyse($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $MAX_PRAS = 200;
        $this->detail_arr = array();
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>2 ) {
            glob_printr( $this->step_arr, $FUNCNAME." step_arr:", 1);
        }
  
        // analyse result LEVEL0        
        foreach($this->step_arr as $i=> $row) {
            
            $step_name = $row['name'];
            $step_nr   = $row['st'];
            
            $sqlsse = "count(*)  from ABSTRACT_PROTO_STEP where NAME=".$sqlo->addQuotes($step_name) . ' and STEP_NR='.$step_nr;
            $sqlo->Quesel($sqlsse);
            $sqlo->ReadRow();
            $cnt_all = $sqlo->RowData[0];
            
            $info_row=array();
            $sqlsse = "distinct(ABSTRACT_PROTO_ID)  from ABSTRACT_PROTO_STEP where NAME=".$sqlo->addQuotes($step_name) . ' and STEP_NR='.$step_nr;
            $sqlo->Quesel($sqlsse);
            $cnt=0;
            while($sqlo->ReadRow()) {
                if($cnt>$MAX_PRAS) {
                    break;
                }
                $pra_loop = $sqlo->RowData[0];
                $info_row[] = $pra_loop;
                $cnt++;
            }
            
            $this->detail_arr[] = array('ids'=>$info_row, 'cnt'=>$cnt_all, 'st'=>$step_nr, 'stn'=>$step_name);
            
        }
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>2 ) {
            debugOut('KI:result:LEVEL0: ', $FUNCNAME, 1);
            foreach($this->detail_arr as $i => $row) {
                echo ($i+1).': Step-NR: '.$row['st'].' Step-Name: '.$row['stn'].' &nbsp; <b>PRA-ids:</b> '.implode(', ',$row['ids'])."<br>\n";
            }
        }
        
        $status_txt=NULL;
        $all_found = 1;
        
        // analyse result LEVEL1
        foreach($this->detail_arr as $i=>$row) {
            
            $step_row = $this->step_arr[$i];
            $step_nr  = $step_row['st'];
            
            if (empty($row['ids'])) {
                $status_txt='Step "'.$step_nr.'" not found';
                $all_found = -1;
                break;
            }
        }
        if ($all_found<0) {
            return array('ids'=>array(), 'status'=>$status_txt);
        }
        
        // analyse result LEVEL2
        $pra_arr = array();
        foreach($this->detail_arr as $i=>$row) {
            
            $step_row = $this->step_arr[$i];

            $loop_ids = &$row['ids'];
            if (sizeof($loop_ids)<$row['cnt']) {
                // not ALL were cached ...
                // TBD: what to do now ?
            }
            foreach($loop_ids as $pra_loop) {
                if(!$pra_arr[$pra_loop]) $pra_arr[$pra_loop]=0;
                $pra_arr[$pra_loop]++;
            }
        }
        
        // analyse result LEVEL3
        $expect_step_cnt = sizeof($this->step_arr);
        $pra_final_arr = array();
        foreach($pra_arr as $pra_loop => $pra_step_cnt) {
            if ($pra_step_cnt<$expect_step_cnt) {
                // at least one missing ...
            } else {
                $pra_final_arr[$pra_loop] = 0;
            }
        }
        
        // analyse result LEVEL4
        // look for EXACT number of steps
        
        
        $one_pra_has_same_no = 0;
        $pra_ids = array_keys($pra_final_arr);
        
        foreach($pra_ids as $pra_loop) {
            
            $pra_lib = new oABSTRACT_PROTO_stepx($pra_loop);
            $pra_loop_step_cnt = $pra_lib->step_cnt($sqlo);
            $pra_final_arr[$pra_loop] = $pra_loop_step_cnt;
            
            if ($expect_step_cnt==$pra_loop_step_cnt) {
                $one_pra_has_same_no = 1;
            }
        }
        
        // analyse result LEVEL5
        // look for EXACT number of steps
        // remove protocols with MORE steps ...
        $TAKE_ONLY_EXACT_MATCH=0;
        if ($TAKE_ONLY_EXACT_MATCH) {
            if ($one_pra_has_same_no) {
                
                $pra_ids = array_keys($pra_final_arr);
                foreach($pra_ids as $pra_loop) {
                    $pra_loop_step_cnt = $pra_final_arr[$pra_loop];
                    if ($pra_loop_step_cnt!=$expect_step_cnt)  {
                        unset($pra_final_arr[$pra_loop]);
                    }
                }
            }
        }
  
        if ( $_SESSION["userGlob"]["g.debugLevel"]>2 ) {
            
            debugOut('KI:result:LEVEL5: ', $FUNCNAME, 1);
            foreach($this->detail_arr as $i=>$row) {
                echo ($i+1).': CNT: '.$row['cnt'].' Step-NR: '.$row['st'].' NAME: '.$row['stn'].' <b>PRA-ids:</b>'.implode(', ',$row['ids'])."<br>\n";
            }
            
        }
        
        debugOut('pra_final_arr: '.print_r($pra_final_arr,1), $FUNCNAME, 1);
        
        return array('ids'=>$pra_final_arr, 'status'=>$status_txt);
        
    }
    
    function show_KI_details() {
        if (!is_array($this->detail_arr)) {
            echo 'No Details set.<br>';
            return;
        }
        
        foreach($this->detail_arr as $i=>$row) {
            echo ($i+1).': Step-NR: '.$row['st'].' NAME: '.$row['stn'].' <b>PRA-ids:</b>'.implode(', ',$row['ids'])."<br>\n";
        }
    }
    
    function get_detail_arr() {
        return $this->detail_arr;
    }
}
