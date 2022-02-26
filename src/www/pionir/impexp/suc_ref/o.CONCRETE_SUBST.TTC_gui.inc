<?php
require_once ("f.objview.inc");
require_once 'o.ABSTRACT_PROTO.stepx.inc';
/**
 * GUI for differences ...
 * @package o.CONCRETE_SUBST.TTC_gui.inc
 * @swreq REQ:
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @created 2021-06-05
 */
class o_CONCRETE_SUBST_TTC_gui {

    function __construct($prod_bead, $ref_doc) {
        $this->prod_bead=$prod_bead;
        $this->ref_doc  =$ref_doc;
    }
    
    static function compare_one_PRA( $sqlo, int $pra_id, array $diffarr ) {

        $pra_step_data = oABSTRACT_PROTO_stepx::getStepArray_STAT($sqlo, $pra_id);
        $analyse_keys=array('sua', 'q');
        
        foreach($diffarr as $diff_row) {
            
            // get object step
            $step_nr = $diff_row['st'];
            
            //$reff_bead_st_data = o_SUC_TTC_proto::get_stepdata_by_step($ref_bead_steps, $step_nr);
            //$this_bead_st_data = o_SUC_TTC_proto::get_stepdata_by_step($this_bead_steps, $step_nr);
            
            
//             if (empty($this_bead_st_data)) {
//                 $this_bead_st_data=array(); // make an enmpty array
//             } 
           
            $diff_txt=NULL;
            $this_val =  $diff_row['now'];
            $ref_val  =  $diff_row['ref'];
            $diff_key =  $diff_row['key'];
    
            switch ($diff_key) {
                case 'sua':
                    $diff_txt .= 'MAA: expected '.
                        fObjViewC::bo_display( $sqlo, 'ABSTRACT_SUBST', $ref_val).
                        ', given: '.fObjViewC::bo_display( $sqlo, 'ABSTRACT_SUBST', $this_val).' . ';
                    break;
                case 'q':
                    $diff_txt .= 'Quantity: expected '. $ref_val.', given: '.$this_val.' . ';
                default:
                    $diff_txt = $diff_row['text'];
            }
 
            
            
            $step_name = $pra_step_data[$step_nr]['NAME'];
            echo 'Step:'.$step_nr.' <b>'.$step_name.'</b>: '.$diff_txt."<br>\n";
            
        }
        
    }
    
    /*
     * 
     * 'st'=>$step_nr,
     * 'text'=>$diff_txt,
     * 'key'=>$key,
     * 'ref'=>$ref_val,
     * 'now'=>$now_val,
     * */
    function _one_sua($sqlo, $suc_id, $diffarr) {
          
        $suc_link = fObjViewC::	bo_display( $sqlo, 'CONCRETE_SUBST', $suc_id);
        echo $suc_link.': <ul>'."\n";
        
        foreach($diffarr as  $ord_no=> $diff_log_row) {
            
            $pra_id       = $diff_log_row['ap'];
            $diffs_of_PRA = $diff_log_row['diff'];
        
            if (empty($diffs_of_PRA)) {
                continue;
            }
            
            $pra_link = fObjViewC::	bo_display( $sqlo, 'ABSTRACT_PROTO', $pra_id);
            echo '- Protocol: ' .$pra_link."<ul>";
            self::compare_one_PRA( $sqlo, $pra_id, $diffs_of_PRA );
            echo '</ul>'."\n";
            
        }
        echo '</ul>'."\n";
    }
    
    /**
     * 
     * @param object $sqlo
     * @param array $diffarr see o.CONCRETE_SUBST.bead_TTC.inc:PROTO_DIFF_STRUCT
     */
    function diff_details(object $sqlo, array $diffarr) {
        
        if (!empty($diffarr)) {
            echo '<span class="yGgray">Show Difference between '.o_LINK_TTC::CONFIG_NAME_USER.' '.
                ' and this MAC</span> '.
                '<br>'."\n";
        }
         
        foreach($diffarr as $suc_id => $diff_osuc) {
            $this->_one_sua($sqlo, $suc_id , $diff_osuc);
        }
        
        
    }
}