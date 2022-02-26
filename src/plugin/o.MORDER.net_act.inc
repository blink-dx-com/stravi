<?php
require_once 'o.MORDER.network.inc';
// require_once 'glob.obj.copyobj2.inc';
require_once 'lev1/glob.obj.create_subs.inc';
require_once "insertx.inc";
require_once 'lev1/glob.obj.status.mod.inc';
require_once ("f.objview.inc");	

/**
 * create SUB order
 * - create new MORDER with SUA=$sua_id
 * - add workflow (STD or workflow of SUA)
 * - start workflow
 * @package o.MORDER.net_act.inc
 * @author  Steffen Kube
 * @param $id id of object
 * @param $mo_id mother
 * @param $sua_id SUA-ID 
 * @param string $act
 *   'new'    : create new sub-order
 *   'mo_mod' : modify mother, need $mo_id
 *   'mo_del' : delete many delx[MODID] = 1
 *   'mo_new' : create new mother order
 * 
 */
class o_MORDER_net_act_XPL extends gPlugin {

	
	function register() {
		$tablename	= 'MORDER';
		$title		= 'Manage sub orders';
		
		$this->infoarr			 = NULL;
		
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read', 'write'), 'obj'=>array('write') )
				);

	}
	
	/**
	 * add MORDER: CHI to MO, start a workflow
	 * @param object $sqlo
	 * @param int $mo_id
	 * @param int $chi_id
	 * @param string $mo_chi_key
	 *   'MO' or 'CHI'
	 */
	private function _add_CHI_to_MO($sqlo, $mo_id, $chi_id, string $mo_chi_key) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    $tablename	= 'MORDER';
	    
	    $start_mord_id=0;
	    if ($mo_chi_key=='MO') {
	        $start_mord_id=$mo_id;
	    }
	    if ($mo_chi_key=='CHI') {
	        $start_mord_id=$chi_id;
	    }
	    
    	$ords_mod_lib = new oMORDER_network_mod($sqlo, $mo_id);
    	if ($error->Got(READONLY))  {
    	    $error->set( $FUNCNAME, 1, 'Error on Prep.' );
    	    return;
    	}
    	
    	// add to mother
    	$ords_mod_lib->add($sqlo, $chi_id);
    	if ($error->Got(READONLY))  {
    	    $error->set( $FUNCNAME, 3, 'Error on linking.' );
    	    return;
    	}
    	
    	if ($start_mord_id) {
    	
        	// add workflow and start
        	$wf_manage_lib = new gObjCreaSubs();
        	$wf_manage_lib->addUserWorkflow($sqlo, $tablename, $start_mord_id); // no special workflow, take default
        	if ($error->Got(READONLY))  {
        	    $error->set( $FUNCNAME, 5, 'Error on workflow start' );
        	    return;
        	}
        	
        	// set to beauftragt ...
        	$statusName='beauftragt';
        	$auditNotes='';
        	$workflow_lib = new gObj_StatusMod($sqlo, $tablename, $start_mord_id);
        	$workflow_lib->status_change($sqlo, $statusName, $auditNotes);
        	$infos = $workflow_lib->get_acc_log_info();
        	if (!empty($infos)) {
        	    echo 'Status-Change-Info:<br>'.implode('<br> - ',$infos);
        	    echo "<br>";
        	}
        	if ($error->Got(READONLY))  {
        	    $error->set( $FUNCNAME, 10, 'Error on status change.' );
        	    return;
        	}
    	}
	}
	
	function _new($sqlo, &$help_lib) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $tablename	= 'MORDER';
	    $table_nice = tablename_nice2($tablename);
	
	    $suas       = $help_lib->get_sub_suas($sqlo);
	    $sub_orders = $help_lib->get_orders($sqlo);
	    
	    if (empty($suas)) {
	        $this->htmlFoot('WARN','(1) There are no MAAs for planned sub orders.');
	    }
	    $sua_v_ord = $help_lib->suas_versus_subs_ords($sqlo, $suas, $sub_orders);
	    
	    if (!$_REQUEST['sua_id']) {
	        $this->htmlFoot('WARN','(2) Input SUA-ID missing.');
	    }
	    $sua_id = $_REQUEST['sua_id'];
	    
	    if (is_array($sua_v_ord[$sua_id]) ) {
	        if($sua_v_ord[$sua_id]['ord']) {
	            $this->htmlFoot('WARN','(3) '.$table_nice.' for MAA '.$sua_id.' alrready exists.');
	        }
	    }
	    
	    $copy_val_keys = array('NAME', 'PROD_LINE_ID', 'CERT_FLAG');
	    $mord_old_feat = glob_elemDataGet3( $sqlo, $tablename, array('MORDER_ID'=>$this->objid ), $copy_val_keys );
	    
	    //$new_name = 'Sub-order for POR:'.$this->objid.' MAA:'.$sua_id;
	    
	    $new_vals=array();
	    foreach($copy_val_keys as $key) {
	        $new_vals[$key]=$mord_old_feat[$key];
	    }
	    
	    $new_name = $new_vals['NAME'].' (sub)';
	    $new_vals['NAME'] = $new_name;
	    $new_vals['ABSTRACT_SUBST_ID']=$sua_id;
	    
	    $new_params = array(
	        'vals'=>$new_vals
	    );
	    
	    $insertlib = new insertC();
	    $new_id = $insertlib->new_meta($sqlo, $tablename, $new_params);
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 2, 'Error on creation.' );
	        return;
	    }
	    
	    $this->_add_CHI_to_MO($sqlo, $this->objid, $new_id, 'CHI');
	    
	    
	}
	
	function form_mo_del($sqlo, $old_mo_ids) {

	    
	    $tablename	= 'MORDER';
	    //$table_nice = tablename_nice2($tablename);
	    $initarr   = array();
	    $initarr["title"]       = "Remove current mothers";
	    $initarr["submittitle"] = "Remove";
	    $initarr["tabwidth"]    = "AUTO";
	    
	    
	    $hiddenarr = NULL;
	    $hiddenarr["id"]     = $this->objid;
	    $hiddenarr["act"]     = 'mo_del';
	    
	    
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
	    
	    $i=1;
	    foreach($old_mo_ids as $old_mo_id) {
	        
	        $obj_link = fObjViewC::	bo_display( $sqlo, 'MORDER', $old_mo_id);
	        $fieldx = array ( // form-field definition
	            "title" => "Mother ".$i,
	            "name"  => "delx[".$old_mo_id."]",
	            "object"=> "checkbox",
	            "namex" => TRUE,
	            "val"   => 0,
	            'notes' => $obj_link
	            
	        );
	        $formobj->fieldOut( $fieldx );
	        $i++;
	    }
	    
	    
	    $formobj->close( TRUE );
	}
	
	function form_mo_mod() {
	    
	    $tablename	= 'MORDER';
	    $table_nice = tablename_nice2($tablename);
	    $initarr   = array();
	    $initarr["title"]       = "Add an existing mother ".$table_nice; 
	    $initarr["submittitle"] = "Add";
	    $initarr["tabwidth"]    = "AUTO";  
	    $initarr["dblink"]    = 1;  
	    
	    $hiddenarr = NULL; 
	    $hiddenarr["id"]     = $this->objid; 
	    $hiddenarr["act"]     = 'mo_mod'; 
	    
	    
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
	    
	    $fieldx = array ( // form-field definition
	        "title" => "Mother",
	        "name"  => "mo_id",
	        "object"=> "dblink",
	        "namex" => TRUE,
	        "val"   => 0,
	        "inits" => array( "table"=>"MORDER", "objname"=>'', "pos" =>"0", "projlink"=> 1),
	        "notes" => ""
	    );
	    $formobj->fieldOut( $fieldx ); 
	    
	    $formobj->close( TRUE );
	}
	
	function form_mo_new($sqlo, $sua_id) {
	    
	    $tablename	= 'MORDER';
	    $table_nice = tablename_nice2($tablename);
	    $initarr   = array();
	    $initarr["title"]       = "Add a NEW mother ".$table_nice;
	    $initarr["submittitle"] = "Create";
	    $initarr["tabwidth"]    = "AUTO";
	    $initarr["dblink"]      = 1;
	    
	    $hiddenarr = NULL;
	    $hiddenarr["id"]     = $this->objid;
	    $hiddenarr["act"]     = 'mo_new';
	    
	    
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
	    
	    $fieldx = array ( // form-field definition
	        "title" => "Mother",
	        "object"=> "info2",
	        "val" => fObjViewC::	bo_display( $sqlo, 'ABSTRACT_SUBST', $sua_id)
	    );
	    $formobj->fieldOut( $fieldx );
	    
	    $formobj->close( TRUE );
	}
	
	
	function _mo_mod($sqlo, oMORDER_network &$help_lib, $mo_id, $go) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    $table_nice = tablename_nice2('MORDER');
	    
	    
	    
	    $old_mo_ids = $help_lib->get_parents($sqlo);
	    
	    if (!$go) {
	        
	        if (!empty($old_mo_ids)) {
	            $this->form_mo_del($sqlo, $old_mo_ids);
	            echo "<br>";
	        }
	        
	        $this->form_mo_mod();
	        return;
	    } 
	    
	    if (!$mo_id) {
	        $error->set( $FUNCNAME, 1, 'No mother given.' );
	        return;
	    }
	    
	    $this->_html->line_out('Add a mother '.$table_nice.' ...');
	    $this->_html->line_out('');
	    $this->_html->line_out('New mother '.$table_nice.': '.$mo_id);
	    $ords_mod_lib = new oMORDER_network_mod($sqlo, $mo_id);
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 1, 'Error on Prep.' );
	        return;
	    }
	    $ords_mod_lib->add($sqlo, $this->objid);
	}
	
	function _mo_del($sqlo, oMORDER_network &$help_lib, $mo_ids, $go) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    $table_nice = tablename_nice2('MORDER');
	    
	    $this->_html->line_out('Modify mother '.$table_nice.' ...');
	    $this->_html->line_out('');

	    
	    $this->_html->line_out('Remove mothers '.$table_nice.'.');
	    $ords_mod_lib = new oMORDER_network_mod($sqlo, $this->objid);
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 1, 'Error on Prep.' );
	        return;
	    }
	    
	    foreach($mo_ids as $old_mo_id=>$flag) {
	        if (!$flag) continue;
	        $ords_mod_lib->del_by_ID($sqlo, $old_mo_id);
	    }
	    
	    $this->_html->messageBox2('ok', 'some mothers were removed.');
	    
	}
	
	
	/**
	 * create new mother
	 * @param object $sqlo
	 * @param object $help_lib
	 * @param int $go
	 */
	function _mo_new($sqlo, oMORDER_network $help_lib, $go) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $tablename	= 'MORDER';
	    $table_nice = tablename_nice2($tablename);
	    
	    // mother already exists ???
	    if ($help_lib->has_parent($sqlo)) {
	        $error->set( $FUNCNAME, 1, 'This '.$table_nice.' already has a mother '.$table_nice );
	        return;
	    }

	    $mother_suas = $help_lib->get_mo_suas($sqlo);
	    if (empty($mother_suas)) {
	        $error->set( $FUNCNAME, 10, 'No planned Mother-MAA configured. You must configure: Mother-MAA:VARIO: '.
	            oMORDER_network::VARIO_SUA_CHILD_SUA.'={ID of Child-MAA}' );
	        return;
	    }
	    
	    $mo_sua = current($mother_suas);
	    
	    if (!$go) {
	        $this->form_mo_new($sqlo, $mo_sua);
	        return;
	    }
	    
	    // create order
	    $copy_val_keys = array('NAME', 'PROD_LINE_ID', 'CERT_FLAG');
	    $mord_old_feat = glob_elemDataGet3( $sqlo, $tablename, array('MORDER_ID'=>$this->objid ), $copy_val_keys );
	    
	    //$new_name = 'Sub-order for POR:'.$this->objid.' MAA:'.$sua_id;
	    
	    $new_vals=array();
	    foreach($copy_val_keys as $key) {
	        $new_vals[$key]=$mord_old_feat[$key];
	    }
	    
	    $new_name = $new_vals['NAME'].' (mother)';
	    $new_vals['NAME'] = $new_name;
	    $new_vals['ABSTRACT_SUBST_ID'] = $mo_sua;
	    
	    $new_params = array(
	        'vals'=>$new_vals
	    );
	    
	    $insertlib = new insertC();
	    $new_id = $insertlib->new_meta($sqlo, $tablename, $new_params);
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 2, 'Error on creation.' );
	        return;
	    }
	    
	    $this->_add_CHI_to_MO($sqlo, $new_id, $this->objid, 'MO');
	    
	    return $new_id;
	}
	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$this->objid   = $_REQUEST['id'];
		$action   = $_REQUEST['act'];
		$go       = $_REQUEST['go'];
		
		$sqlo  = &$this->_sqlo;

		
		$help_lib   = new oMORDER_network($this->objid);
		
		switch ($action){
		    case 'new':
		      $this->_new($sqlo, $help_lib);
		      break;
	
		   case 'mo_mod':
		      $this->_mo_mod($sqlo, $help_lib, $_REQUEST['mo_id'], $go);
		      if (!$go) {
		          return;
		      }
		      break;
		   case 'mo_del':
		       $this->_mo_del($sqlo, $help_lib, $_REQUEST['delx'], $go);
		       if (!$go) {
		           return;
		       }
		       break;
		   case 'mo_new':
		       $this->_mo_new($sqlo, $help_lib, $go);
		       if (!$go) {
		           return;
		       }
		       
		       break;
		   default:
		       $this->htmlFoot('ERROR', 'No action found.');
		}
		
		if ($error->Got(READONLY))  {
		    return;
		}
		
		$this->_html->pageForward('edit.tmpl.php?t=MORDER&id='.$this->objid, 'back to parent order.');
	}
	
	
	
}