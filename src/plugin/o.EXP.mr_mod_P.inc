<?php
/**
 * experiment multireactor plugin
 * @package o.EXP.mr_mod_P.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @swreq REQ-0017 Create/Edit a WellPlate experiment
 * @param   int $id 
 * @param $action
 *   'well_def' -- define which wells are used: create sub exps
 *   'sub_exp_coll' -- collect existing sub experiments
 *   'proto_new_one'   create one new proto for ALL wells, need parx[STEP_NO], parx[PRA]
 *   'proto_new_some'  create one new proto for SELECTED wells, need parx[STEP_NO], parx[PRA]
 *   'proto_edit' -- edit one protocol : need parx['cpid']
 *   'proto_edit_sel' -- show select to prepare protocol edit : need parx[STEP_NO]
 * @param parx
 *   ['well'][POS] : 0,1 for $action='well_def'
 *   ['cpid']  -- one protocol
 */
require_once 'javascript.inc';
require_once 'f.directorySub.inc';
require_once ("f.objview.inc");
require_once ("visufuncs.inc");
require_once 'o.ABS_SOCKET.subs.inc';
require_once 'o.EXP.proto_mod.inc';
require_once 'down_up_load.inc';
require_once 'o.EXP.proto.inc';
require_once 'gui/o.CONCRETE_PROTO.edit.inc';
require_once 'gui/o.PROTO.stepout1.inc';
require_once ($_SESSION['s_sessVars']['AppLabLibDir'].'/lablib/gui/o.EXP.plate.inc');
require_once ($_SESSION['s_sessVars']['AppLabLibDir'].'/lablib/o.EXP.akw_api.inc');
require_once ($_SESSION['s_sessVars']['AppLabLibDir'].'/lablib/blk_akw/f.exp_status_json.inc');

class oABS_SOCKET_plate_FORM {
    
    function __construct($abs_cart_id) {
        $this->abs_cart_id=$abs_cart_id;
    }
    
    /**
     * wells for new experiment
     * @param object $sqlo
     */
    function show_new_exp($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        
        $show_lib = new oABS_SOCKET_showcart($sqlo, $this->abs_cart_id);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on init.' );
            return;
        }
        $MAX_ENTRIES = $show_lib->get_feature('MAX_ENTRIES');
        $js = $show_lib->get_javascript();
        echo "<script>\n" .$js. "</script>\n";
        
        $free_wells =array();
        $well_colors=array();
        for ($i=0; $i<$MAX_ENTRIES; $i++) {
            $pos=$i+1;
            echo '<input type=hidden name="parx[well]['.$pos.']" value="0">'."\n";
            $free_wells[]=$pos;
            $well_colors[$pos]=array('c'=>'#E0E0E0');
        }

        $sopt=array(
            'legend_show'=>-1,
            'well_js_action'=> array('var'=>'well'),
            'well_act_pos'  => $free_wells,
            'well.show'     =>'extern',
            'content'       => $well_colors,
        );
        $show_lib->show($sqlo, $sopt);
        echo "<br>";
    }
    
    /**
     * select wells inside the used wells
     * @param object $sqlo
     */
    function show_sub_sel($sqlo, $used_wells, $free_wells) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $well_colors=array();
        foreach($used_wells as $pos) {
            $well_colors[$pos]=array('c'=>'#ffcccb');
        }
        foreach($free_wells as $pos) {
            $well_colors[$pos]=array('c'=>'#B7D2FF');
        }
        

        $sopt=array(
            'legend_show'=>-1,
            'well_js_action'=> array('var'=>'well'),
            'well_act_pos'  => $free_wells,
            'content'    => $well_colors,
            'well.show'  =>'extern'
        );
        $show_lib = new oABS_SOCKET_showcart($sqlo, $this->abs_cart_id);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on init.' );
            return;
        }
        $js = $show_lib->get_javascript();
        echo "<script>\n" .$js. "</script>\n";
        
        
        foreach($free_wells as $pos) {
            echo '<input type=hidden name="parx[well]['.$pos.']" value="0">'."\n";
        }
        
        $show_lib->show($sqlo, $sopt);
        echo "<br>";
    }
}


class o_EXP_mr_proto_diff {
    
    function __construct(object $sqlo, int $id, int $step_no, $form_url ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->form_url=$form_url;
        $this->id = $id;
        $this->step_no= $step_no;
        
        $this->exp_mr_lib = new oEXP_mr($this->id);
        $this->exp_arr   = $this->exp_mr_lib->get_children($sqlo);
        $this->proto_arr = $this->exp_mr_lib->subexp_protosByStep($sqlo, $step_no);
    }
    
    private function one_proto($sqlo, $cpid, $cnt) {
        
        $pra_id = glob_elementDataGet( $sqlo, 'CONCRETE_PROTO', 'CONCRETE_PROTO_ID', $cpid, 'ABSTRACT_PROTO_ID');
        
        $part_exp_arr_raw = $this->exp_mr_lib->proto_belongs_to($sqlo, $cpid);
        $part_exp_arr=array();
        foreach($part_exp_arr_raw as $row) {
            $part_exp_arr[]=$row[0];
        }
        
        $url=$this->form_url.'&id='.$this->id.'&action=proto_edit&parx[cpid]='.$cpid;
        echo '<h2><a href="'.$url.'"><img src="images/but.edit.norm.gif" width=30></a>'.($cnt+1).'. PRC-ID:'.$cpid.'</h2>';
        echo 'Experiments: '.implode(', ',$part_exp_arr)."<br><br>";
        
        $editAllow=0;
        $prot_opt=array( "header.show"=>-1);
        $proto_step_c = new  protostep( $sqlo, 0, $pra_id, $editAllow, 0, $cpid, $prot_opt );
        $proto_step_c->tableOut($sqlo);
        echo "<br><br>\n";
    }
    
    function show_details($sqlo, int $edit_flag) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        
        $diff_array = oEXP_plate_static_GUI::calc_diffs($sqlo, $this->proto_arr);
        
        $cpid_first=key($diff_array);
        if(!$cpid_first) {
            next($diff_array);
            $cpid_first=key($diff_array);
        }
        if (!$cpid_first) {
            $error->set( $FUNCNAME, 1, 'No protocol found in selection.' );
            return;
        }
        
        
        $pra_id = glob_elementDataGet( $sqlo, 'CONCRETE_PROTO', 'CONCRETE_PROTO_ID', $cpid_first, 'ABSTRACT_PROTO_ID');
        $html_tmp = fObjViewC::bo_display( $sqlo, 'ABSTRACT_PROTO', $pra_id );
        echo tablename_nice2('ABSTRACT_PROTO').': '. $html_tmp."<br><br>";
        
        $this->edit_flag=$edit_flag;
        
        
        $cnt=0;
        foreach($diff_array as $cpid=>$row) {
            
            if (!$cpid) continue;
            $this->one_proto($sqlo, $cpid, $cnt);
            $cnt++;
        }
    }
}


class o_EXP_mr_mod_P_XPL extends gPlugin {
	
	
	function register() {
		$tablename	= 'EXP';
		$title		= 'multireactor Editor';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		if ($_REQUEST['action']!='') {
		    $this->infoarr['title']    = 'Actions';
    		$this->infoarr['locrow'] = array ( 
    		    array ($this->_plug_get_URL().'&id='.$_REQUEST['id'], $title) ,
    		    
    		);
		}
		$this->infoarr2 = array( 
		     
		    // the WRITE-access check will be done later, if actions are given ...
			'accCheck' => array('tab'=>array('write'), 'obj'=>array('read') )
		);
	}
	
	function proto_sel_well_form($sqlo, $parx, &$proto_arr) {
	    
	    $initarr   = array();
	    $initarr["action"]      = $this->form_url;
	    $initarr["title"]       = "Define wells for new protocols";
	    $initarr["submittitle"] = "Submit";
	    $initarr["tabwidth"]    = "AUTO";
	    
	    $hiddenarr = array();
	    $hiddenarr["id"]      = $this->objid;
	    $hiddenarr["action"]  = 'proto_new_some';
	    $hiddenarr["parx[STEP_NO]"] = $parx["STEP_NO"];
	    $hiddenarr["parx[PRA]"]     = $parx["PRA"];
	    
	
	    
	    echo '<h2>Define, which wells are used for the NEW protocol.</h2>'."\n";
	    
	    
	    $formobj = new formPure($initarr, $hiddenarr, 0);
	    $formobj->start();
	    
	    $used_wells=array();
	    $free_wells=array();
	    
	    $i=0;
	    foreach($this->exp_arr as $row) {
	        $pos=$row[1];
	        $cpid = $proto_arr[$i];
	        if ($cpid) {
	            $used_wells[]=$pos;
	        } else {
	            $free_wells[]=$pos;
	        }
	        $i++;
	    }
	    
	    
	    
	    $abs_cart_id = $this->exp_feats['ABS_SOCKET_ID'];
	    $plate_gui_lib = new oABS_SOCKET_plate_FORM($abs_cart_id);
	    $plate_gui_lib->show_sub_sel($sqlo, $used_wells, $free_wells);
	    
	    echo "<input class='yButton' type=submit value=\"Save\">\n";
	    $formobj->close( );
	}
	
	
	/**
	 * output:
	 * $parx[well][{POS}]= 0,1
	 * @param object $sqlo
	 */
	function sub_exp_miss_form($sqlo) {
	    
	    $initarr   = NULL;
	    $initarr["action"]      = $this->form_url;
	    $initarr["title"]       = "Define used wells";
	    $initarr["submittitle"] = "Submit";
	    $initarr["tabwidth"]    = "AUTO";
	    
	    $hiddenarr = NULL;
	    $hiddenarr["id"]      = $this->objid;
	    $hiddenarr["action"]  = 'well_def';
	    
	    echo '<h2>Define, which wells are used.</h2>'."\n";
	    
	    
	    $formobj = new formPure($initarr, $hiddenarr, 0);
	    $formobj->start();
	    
	    $abs_cart_id = $this->exp_feats['ABS_SOCKET_ID'];
	    $plate_gui_lib = new oABS_SOCKET_plate_FORM($abs_cart_id);
	    $plate_gui_lib->show_new_exp($sqlo);
	    
	    echo "<input class='yButton' type=submit value=\"Save\">\n";
	    $formobj->close( );
	}
	
	function sub_exp_miss_DO($sqlo, $well_def) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	   
	    $tablename='EXP';
	    $exp_mod_lib  = new oEXP_mr_mod($sqlo, $this->objid);
	    
	    
	    $ABS_SOCKET_ID = $this->exp_feats['ABS_SOCKET_ID'];
	    $abs_cart_lib  = new oABS_SOCKET_subs();
	    $abs_cart_lib->init($sqlo, $ABS_SOCKET_ID);
	    
	   
	    
	    foreach($well_def as $pos=>$val) {
	        
	        if ($val>0) { 
	            
	            
	            
	            $well_alias = $abs_cart_lib->wellPos2Alias($pos);
	            $new_name = 'EXP'.$this->objid.' '.$well_alias;
	            
	            $new_args = array();
	            $new_args['NAME']    = $new_name;
	            $new_args['MO_POS']  = $pos;
	            
	            debugOut('(128) new_args:'.print_r($new_args,1), $FUNCNAME, 2);
	            $exp_mod_lib->add_sub_exp($sqlo, $new_args);
	            if ($error->Got(READONLY))  {
	                $error->set( $FUNCNAME, 1, 'sub-experiment creation pos='.$pos.' failed.' );
	                return;
	            }
	            
	        }
	    }
	}
	
	/**
	 * create protocol for ALL sub_exp: all the same ...
	 * @param object $sqlo
	 * @param object $sqlo2
	 * @param int $abstract_proto_id
	 * @param int $step_no
	 */
	function proto_create($sqlo, $sqlo2, $abstract_proto_id, $step_no) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $creopt = array("checkStepNo"=>1);
	    $blue_conc_prot=0;
	    $proto_lib = new oEXP_proto_mod();
	    $proto_lib->set_exp($sqlo, $this->objid);
	    
	    //debugOut("(150) abstract_proto_id:$abstract_proto_id, step_no:$step_no", $FUNCNAME, 1);
	    $cp_id = $proto_lib->create($sqlo, $sqlo2, $abstract_proto_id, $step_no, array(), $blue_conc_prot, $creopt);
	    
	    $sub_proto_lib = new oEXP_proto_mod();
	    
	    // ... attach to all sub_exp
	    foreach( $this->exp_arr as $row) {
	        $exp_loop_id=$row[0];
	        $sub_proto_lib->set_exp($sqlo, $exp_loop_id);
	        $sub_proto_lib->add_existing($sqlo, $cp_id, $step_no);
	        if ( $error->Got(READONLY))  {
	            $error->set( $FUNCNAME, 1, 'Error on sub-exp:'.$exp_loop_id );
	            return;
	        }
	    }
	    
	    return $cp_id;
	}
	
	/**
	 * create protocol for ALL sub_exp: all the same ...
	 * @param object $sqlo
	 * @param object $sqlo2
	 * @param int $abstract_proto_id
	 * @param int $step_no
	 * @param array $pos_arr  -- selected positions
	 */
	function proto_create_some($sqlo, $sqlo2, $abstract_proto_id, $step_no, $pos_arr) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $creopt = array("checkStepNo"=>1);
	    $blue_conc_prot=0;
	    $proto_lib = new oEXP_proto_mod();
	    $proto_lib->set_exp($sqlo, $this->objid);
	    
	    if ($proto_lib->step_exists($sqlo, $step_no)) {
	        // there can be already an protocol on mOTHER object
	        $proto_lib->unlink_prot($sqlo, $step_no);
	    }
	    $cp_id = $proto_lib->create($sqlo, $sqlo2, $abstract_proto_id, $step_no, array(), $blue_conc_prot, $creopt);
	    
	    $sub_proto_lib = new oEXP_proto_mod();
	    
	    $selected_pos=array();
	    foreach($pos_arr as $pos=>$val) {
	        if ($val>0) { 
	            $selected_pos[]=$pos;
	        }
	    }
	    
	    // ... attach to SELECTED sub_exp
	    foreach( $this->exp_arr as $row) {
	        
	        $exp_loop_id=$row[0];
	        $pos  = $row[1];
	        
	        if(!in_array($pos, $selected_pos)) {
	            continue;
	        }
	        
	        $sub_proto_lib->set_exp($sqlo, $exp_loop_id);
	        
	        debugOut('(317) pos: '.$pos.' exp_loop_id:'.$exp_loop_id, $FUNCNAME, 2);
	        $sub_proto_lib->add_existing($sqlo, $cp_id, $step_no);
	        if ( $error->Got(READONLY))  {
	            $error->set( $FUNCNAME, 1, 'Error on sub-exp:'.$exp_loop_id );
	            return;
	        }
	    }
	    
	    return $cp_id;
	    
	}
	
	/**
	 * check:
	 * - cpid belongs to collectiom
	 * - insert rigths
	 * @param object $sqlo
	 * @param int $cpid
	 */
	function proto_edit_check($sqlo, int $cpid) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $exp_lib = new oEXP_mr($this->objid);
	    
	    $sub_exp_arr = $exp_lib->proto_belongs_to($sqlo, $cpid);
	    if (empty($sub_exp_arr)) {
	        $error->set( $FUNCNAME, 1, 'The protocol does not belong to this Experiment-Collection.' );
	        return;
	    }
	    $this->select_exp_arr=$sub_exp_arr;
	    
	    $o_rights = access_check($sqlo, 'CONCRETE_PROTO', $cpid);
	    if ( $o_rights["insert"]<1 ) {
	        $error->set( $FUNCNAME, 2, 'You have no "insert" right on this protocol.' );
	        return;
	    }
	    
	    $this->cpid=$cpid;
	    
	}
	
	function proto_edit_f0($sqlo) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $cpid   = $this->cpid;
	    $exp_id = $this->objid;
	    
	    $exp_no = sizeof($this->select_exp_arr);
	    
	    echo '<h2>Edit protocol for a collection of '.$exp_no.' experiments.</h2>';
	    
	    $pra_id = glob_elementDataGet( $sqlo, 'CONCRETE_PROTO', 'CONCRETE_PROTO_ID', $cpid, 'ABSTRACT_PROTO_ID');
	    $html_tmp = fObjViewC::bo_display( $sqlo, 'ABSTRACT_PROTO', $pra_id );
	    echo tablename_nice2('ABSTRACT_PROTO').': '. $html_tmp."<br><br>";

	    $edFormLib = new oConcProtoEdform($sqlo, $cpid);
	   

	    $devopt = array(
	        "backurl"    => $this->_plug_get_URL() . "&id=".$exp_id,
	        "showRequ"   => 0,
	    );
	    $edit_allow = 1;
	    $dummy=NULL;
	    $edFormLib->showform( $sqlo, $edit_allow, $devopt );
	}
	
	function proto_edit_sel_form($sqlo, $step_no) {

	    echo '<h2>Select a protocol for edit.</h2>'."\n";

	    $diff_lib = new o_EXP_mr_proto_diff($sqlo, $this->objid, $step_no, $this->form_url);
	    $edit_flag=1;
	    $diff_lib->show_details($sqlo, $edit_flag);

	}
	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$tablename='EXP';
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		js_formAll();
		
		$this->objid   = $_REQUEST['id'];
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
		$action   = $_REQUEST['action'];
		$this->form_url='p.php?mod=DEF/o.EXP.mr_mod_P';
		
		debugOut('parx:'.print_r($parx,1), $FUNCNAME, 1);
		
		$do_reload  = 0;
		$exp_gui_lib  = new oEXP_plate_MO_GUI($sqlo, $this->objid, 1);
		$exp_gui_lib->get_css();
		$this->exp_feats = $exp_gui_lib->get_main_features();
		$this->exp_arr   = $exp_gui_lib->get_exp_arr();
		$this->socket_det_arr   = $exp_gui_lib->get_socket_details($sqlo);
		
		$init_out        = $exp_gui_lib->init_check($sqlo);
		$o_rights = access_check($sqlo, $tablename, $this->objid);
		
		if($action!=NULL) {
		    
		    if ( !$o_rights["write"]) {
		        $error->set( $FUNCNAME, 1, 'You have no "write" right on this object.' );
		        return;
		    }
		    
    		switch($action) {
    		    case 'well_def':
    		        if ($go==1) {
    		            if( empty($parx['well']) ) {
    		                $this->htmlFoot("ERROR","Input parx missing");
    		            }
    		            $this->sub_exp_miss_DO($sqlo, $parx['well']);
    		            if ($error->Got(READONLY))  {
    		                $error->set( $FUNCNAME, 1, 'sub-experiments creation failed.' );
    		                return;
    		            }
    		            $this->_html->messageBox2('ok', 'Sub-exp creation ok.');
    		            $do_reload=1;
    		        }
    		        break;  
    		    
    		    case 'proto_edit':
    		        if( !$parx['cpid'] ) {
    		            $this->htmlFoot("ERROR","Input cpid missing");
    		        }
    		        $this->proto_edit_check($sqlo, $parx['cpid']);
    		        if (!$go) {
    		            $this->proto_edit_f0($sqlo);
    		        }
    		        break;
    		        
    		    case 'proto_edit_sel':
    		        if( !$parx['STEP_NO'] ) {
    		            $this->htmlFoot("ERROR","Input STEP_NO missing");
    		        }
    		        $this->proto_edit_sel_form($sqlo, $parx['STEP_NO']);
    		        break;
    		        
    		    case 'proto_new_one':
    		        if ($go==1) {
    		            if( !$parx['STEP_NO'] ) {
    		                $this->htmlFoot("ERROR","Input STEP_NO missing");
    		            }
    		            if( !$parx['PRA'] ) {
    		                $this->htmlFoot("ERROR","Input PRA missing");
    		            }
    		            $cpid = $this->proto_create($sqlo, $sqlo2, $parx['PRA'], $parx['STEP_NO']);
    		            if ($error->Got(READONLY))  {
    		                $error->set( $FUNCNAME, 1, 'protocol creation failed.' );
    		                return;
    		            }
    		            $this->_html->messageBox2('ok', 'Protocol creation ok.');
    		            $this->_html->pageForward($this->_plug_get_URL().'&id='.$this->objid.'&action=proto_edit&parx[cpid]='.$cpid, 'forward to edit.');
    		            return;
    		        }
    		        break;
    		        
    		    case 'proto_new_some':
    		        
    		        if( !$parx['STEP_NO'] ) {
    		            $this->htmlFoot("ERROR","Input STEP_NO missing");
    		        }
    		        if( !$parx['PRA'] ) {
    		            $this->htmlFoot("ERROR","Input PRA missing");
    		        }
    		        
    		        if (!$go) {
    		            $proto_arr = $exp_gui_lib->subexp_protosByStep($sqlo, $parx['STEP_NO']); 
    		            $this->proto_sel_well_form($sqlo, $parx, $proto_arr);
    		        } else {
    		            
    		            if( empty($parx['well']) ) {
    		                $this->htmlFoot("ERROR",'Input "well" missing');
    		            }
    		            
    		            $cpid = $this->proto_create_some($sqlo, $sqlo2, $parx['PRA'], $parx['STEP_NO'], $parx['well'] );
    		            if ($error->Got(READONLY))  {
    		                $error->set( $FUNCNAME, 1, 'protocol creation failed.' );
    		                return;
    		            }
    		            $this->_html->messageBox2('ok', 'Protocol creation ok.');
    		            $do_reload=1;
    		            $this->_html->pageForward($this->_plug_get_URL().'&id='.$this->objid.'&action=proto_edit&parx[cpid]='.$cpid, 'forward to edit.');
    		            return;
    		        }
    		        break;
    		    default:
    		        $this->htmlFoot("ERROR",'Action "'.$action.'" is unknown.');
    		}
    		if ($do_reload) {
    		    $this->_html->pageForward($this->_plug_get_URL().'&id='.$this->objid, 'reload.');
    		}
    		return;
		}
		
		if ($init_out['status']!=1) {
		    switch($init_out['key']) {
		        case 'abs_sock_miss':
		            cMsgbox::showBox("warning", $init_out['info']);
		            break;
		        case 'sub_exp_miss':
		            $this->sub_exp_miss_form($sqlo);
		           break;
		        default:
		            cMsgbox::showBox("warning", 'Unknown warning.');
		            return;
		    }
		    return;
		}
		
		echo '<table><tr valign=top>';
		echo '<td>';
		$exp_gui_lib->show_plate($sqlo);
		echo '</td>';
		echo '<td>'."\n";
		$exp_gui_lib->show_exp_list($sqlo);
		echo '</td>';
		echo '</tr></table>'."\n";
		
		$exp_gui_lib->show_proto_log($sqlo);
	}
}
