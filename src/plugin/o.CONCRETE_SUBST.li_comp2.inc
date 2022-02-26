<?php
/**
 * search protocols by parameters
 * GLOBAL:  $_SESSION["s_formState"]['o.CONCRETE_PROTO.li_search'] li_search_conf_STRUCT
 *   'options' : array
 *   'pra_ids' : array
 *   'steps: array 
 *       [STEP_NR][TYPE]=array(
 *         'c': condition
 *         'v': value
 *      )
 * @package o.CONCRETE_SUBST.li_comp2.inc
 * @author  Steffen Kube
 * @param $_REQUEST['go'] 


 */
require_once ("glob.obj.update.inc");
require_once ("javascript.inc");


require_once 'func_form.inc';
// require_once 'o.ABSTRACT_PROTO.stepx.inc';
require_once 'o.CONCRETE_SUBST.proto.inc';
require_once 'o.AS_HAS_PR.subs.inc';
require_once 'o.CS_HAS_PR.subs.inc';
require_once 'gui/o.CONCRETE_PROTO.li_show1.inc';


class o_CONCRETE_SUBST_li_comp2_XPL extends gPlugin {
    
    const MAX_OBJECTS=300;

	function register() {
		$tablename	= 'CONCRETE_SUBST';
		$this->title		= 'Protocol Compare2';
		
		$this->infoarr		= NULL;
	
		$this->infoarr['title']    = 'Show protocol steps: Variant 2';
		$this->infoarr['title_sh'] = $this->title;
		$this->infoarr['form_type']= 'list'; 
		
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_cnt']  = 1;

		$this->infoarr2 = array( 
			'accCheck' => array('tab'=>array('read', 'write'), 'obj'=>array() ),
			'listChkSelect'   => 1,
		    'listCheckNumMax' => self::MAX_OBJECTS,
			);
				
		
	}
	
	static function SUC_in_EXP_via_CART($sqlo, $suc_id) {
	    // now concrete data
	    // cid
	    
	    $output=array('cnt'=>0);
	    
	    $sqlsel_pre = "select distinct(SOCKET_ID) from CART_H_SUC where CONCRETE_SUBST_ID=".$suc_id;
	    $sqlsel = 'EXP_ID from EXP where SOCKET_ID in ('.$sqlsel_pre .') order by EXP_ID';
	    
	    $sqlo->Quesel($sqlsel);
	    $cnt=0;
	    $CNT_MAX=3;
	    while ($sqlo->ReadRow()) {
	        if($cnt>$CNT_MAX) break;
	        $output['cnt']=$cnt+1;
	        $output['eid']=$sqlo->RowData[0];
	        $cnt++;
	    }
	    return $output;
	}
	
	function _track_for_exp($sqlo, $suc_id) {
	    $exp_id = 0;
	    // oSOCKET_subs::SUC_is_part_of_cart($sqlo, $suc_id);
	    $exp_answer = self::SUC_in_EXP_via_CART($sqlo, $suc_id);
	    if ($exp_answer['cnt']>0) {
	        $exp_id =  $exp_answer['eid'];
	    }
	    return $exp_id;
	}
	
	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		$tablename='CONCRETE_SUBST';
		    
		
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
		

		
		// $this->settings = $_SESSION["s_formState"]['o.CONCRETE_PROTO.li_search'];
		debugOut('(416) IN:settings: '.print_r($this->settings,1), $FUNCNAME, 2);
		
		if ($_REQUEST['action']=='settings') {
		    if (!$go) {
		        $hiddenarr=array( 'mod'=>$this->_mod );
		        $tmp = o_CONCRETE_PROTO_li_show1::settings_get();
		        $step_no=0;
		        o_CONCRETE_PROTO_li_show1::form_settings($tmp, $step_no, $hiddenarr);
		        return;
		    } else {
		        o_CONCRETE_PROTO_li_show1::settings_save($parx);
		        $go = 0;
		    }
		}
		$this->settings = o_CONCRETE_PROTO_li_show1::settings_get();
		
		$sqlAfter = $this->listLib->getSqlAfter();
		        
        $sqlsel = "x.CONCRETE_SUBST_ID from ".$sqlAfter; 
        $proto_arr = array();
        $mo_arr    = array();
        
        
        $MAX_OBJECTS=self::MAX_OBJECTS;
        
        $cnt=0;
        
        
        $sqlo2->Quesel($sqlsel);
        while ( $sqlo2->ReadRow() ) {
            
            if ($cnt>$MAX_OBJECTS) {
                
                $this->_html->line_out('');
                $this->_html->messageBox2('warning', 'Too many objects found. Will show only '.$cnt.' objects.');
                $this->_html->line_out('');
                break;
            }
            
            $suc_id = $sqlo2->RowData[0];
            $mo_arr[]=$suc_id;
            
            $cnt++;
        }
        
        $suc_proto_lib = new oCONCRETE_SUBST_proto();
        
        foreach( $mo_arr as $suc_id ) {

            $suc_proto_lib->set_obj($suc_id);
            $plog = $suc_proto_lib->get_c_protos($sqlo);
            
            $first  = current($plog);
            $prc_id = 0;
            if( $first['cp'] ) {
                $prc_id = $first['cp'];
            }
            $proto_arr[] = $prc_id;
        }
        
        // search mothers
        $mo2_arr = array();
        $mo2_has = 0;
        foreach( $mo_arr as $suc_id ) {
            $exp_id = $this->_track_for_exp($sqlo, $suc_id);
            if ($exp_id) $mo2_has=1;
            $mo2_arr[] = $exp_id;
        }
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>=2 ) {
           
            debugOut('MO-Exp-IDs:'.print_r($mo2_arr,1), $FUNCNAME, 1);
            
        }
        
        
        $mo_table=$tablename;
        $show_objects = array('mo_arr'=>&$mo_arr,  'mo_table'=>$mo_table);
        if ($mo2_has) {
            $show_objects['mo2_table'] = 'EXP';
            $show_objects['mo2_arr']   = $mo2_arr;
        }
        
        $tool_opts = $this->settings;
        
        if(!is_array($tool_opts)) $tool_opts=array();
        $tool_opts['form_url'] = $this->_plug_get_URL();
//         if (sizeof($all_proto_arr)>1) {
//             $tool_opts['many_PRA_flag'] = 1;
//         }
        
        $prot_edit_lib = new o_CONCRETE_PROTO_li_show1($sqlo, $show_objects, $tool_opts);
        $prot_edit_lib->open_table($sqlo);
        //foreach($all_proto_arr as $one_cp_arr) {
        $prot_edit_lib->show_one_pra($sqlo, $proto_arr);
        //}
        $prot_edit_lib->table_close();
        
	}
	
}
