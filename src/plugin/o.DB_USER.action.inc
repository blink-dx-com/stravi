<?php
/**
 * - modify actions for a user
 *    
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:12915 o.DB_USER > deactivate user
 * @param $_REQUEST['go'] 0,1
 * @param $_REQUEST['id'] : user ID
 * @param $_REQUEST['action'] : action string
 *    'deactivate'
 *    'grp_edit'
 *    'role_edit'
 * @param $_REQUEST['parx'] : params
 * @module  o.DB_USER.action.inc
 */
require_once ("glob.obj.update.inc");
require_once 'f.data_dir.inc';
require_once ('f.msgboxes.inc');
require_once 'o.DB_USER.subs.inc';
require_once 'o.DB_USER.subs2.inc';
require_once 'o.USER_GROUP.subs.inc';
require_once 'o.ROLE.mod.inc';
require_once ("f.objview.inc");

class o_DB_USER_action_XPL extends gPlugin {

    private $act_arr = array(
        'deactivate'=>array('title'=>'deactivate', 'notes'=>'Removes groups , roles, user preferences, image from user'),
        'grp_edit'  =>array('title'=>'Group management', 'notes'=>'add/remove groups'),
        'role_edit'  =>array('title'=>'Role Management', 'notes'=>'add/remove roles'),
    );
	
	function register() {
	    
		$tablename	= 'DB_USER';
		$title		= 'Actions for a user';
		$action     = $_REQUEST['action'];
		
		if ($action) {
		    $act_title= $this->act_arr[$action]['title'];
		    $title    = $act_title . ' : '.$this->act_arr[$action]['notes'];
		    $title_sh = $act_title;
		}
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = $title_sh;
		
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		
		$this->infoarr2 = array( 
		    'accCheck' => array('tab'=>array('write', 'admin'), 'obj'=>array() ), // must have admin rights !
		);
				
		
	}
	
	function deactivate_form0() {
		
		
		$initarr   = NULL;
		$initarr["title"]       = "Deactivate the user";
		$initarr["submittitle"] = "Submit";
	
		$hiddenarr = NULL;
		$hiddenarr['id'] =  $this->objid;
		$hiddenarr['action'] =  'deactivate';
		
		$formobj = $this->_html->formInit($initarr, $hiddenarr);
		$formobj->close( TRUE );
	}
	
	function _obj_list($sqlo, $table, $obj_ids) {
	    
	    if (empty($obj_ids)) return;
	    switch($table) {
	        case 'USER_GROUP':
	            $main_act='grp_edit';
	            $del_var_name='grp_id';
	            $list_url = 'view.tmpl.php?t=DB_USER_IN_GROUP&searchCol=DB_USER_ID&condclean=1&searchtxt='.$this->objid;
	            break;
	        case 'ROLE':
	            $main_act='role_edit';
	            $del_var_name='role_id';
	            $list_url = 'view.tmpl.php?t=USER_ROLES&searchCol=DB_USER_ID&condclean=1&searchtxt='.$this->objid;
	            break;
	    }
	    $list_link = ' [<a href="'.$list_url.'">Show in list view</a>]';
	    
	    $t_nice = tablename_nice2($table);
	    
	    $nicearr=array();
	    foreach($obj_ids as $loop_id) {
	        $nice = obj_nice_name ( $sqlo, $table, $loop_id ); 
	        $nicearr[$loop_id] = $nice;
	    }
	    
	    asort($nicearr);

	    $dataArr=array();
	    foreach($nicearr as $loop_id => $nice) {
	        $dataArr[] = array( 
	            fObjViewC::	bo_display( $sqlo, $table, $loop_id), 
	            '<a href="'.$this->_plug_get_URL().'&action='.$main_act.'&parx[subact]=del&id='.
	              $this->objid.'&parx['.$del_var_name.']='.$loop_id.'&go=1">'.
	              '<img src="res/img/x.svg" height=20 hspace=4> Remove</a>' 
	            
	        );
	    }
	 
	    $tabobj = new visufuncs();
	    $headOpt = array( "title" => "user has ".$t_nice.'s', "headNoShow" =>1, 'title_notes'=>$list_link);
	    $headx=array();
	    $tabobj->table_out2($headx, $dataArr,  $headOpt);
	}
	
	function group_add_form($sqlo) {
	    
	    $initarr   = array();
	    $initarr["title"]       = "Add a group"; // title of form
	    $initarr["submittitle"] = "Add"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    $initarr["dblink"]=1;
	    
	    $hiddenarr = NULL; // hidden form variables
	    $hiddenarr["id"]     = $this->objid; // is a required input parameter
	    $hiddenarr['action'] =  'grp_edit';
	    $hiddenarr['parx[subact]'] =  'add';
	   
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
	    $fieldx = array ( // form-field definition
	        "title"   => "Group",
	        "name"   => "grp_id",
	        "object" => "dblink",
	        "val"    => 0,
	        "inits"  => array( "table"=>"USER_GROUP", "pos" =>"0", "projlink"=> 1, "noshDel"=>1 ),
	    );
	    $formobj->fieldOut( $fieldx ); // output the form-field
	    $formobj->close( TRUE ); // close form, sshow submit-button
	    
	    echo "<br>";
	    $groups = oUSER_GROUP_subs::getGroupsOfUser($sqlo, $this->objid);
	    $this->_obj_list($sqlo, 'USER_GROUP', $groups);
	}
	
	function role_add_form($sqlo) {
	    
	    $initarr   = array();
	    $initarr["title"]       = "Add a role"; // title of form
	    $initarr["submittitle"] = "Add"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    $initarr["dblink"]=1;
	    
	    $hiddenarr = NULL; // hidden form variables
	    $hiddenarr["id"]     = $this->objid; // is a required input parameter
	    $hiddenarr['action'] =  'role_edit';
	    $hiddenarr['parx[subact]'] =  'add';
	    
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
	    $fieldx = array ( // form-field definition
	        "title"   => "Role",
	        "name"   => "role_id",
	        "object" => "dblink",
	        "val"    => 0,
	        "inits"  => array( "table"=>"ROLE", "pos" =>"0", "noshDel"=>1 ),
	    );
	    $formobj->fieldOut( $fieldx ); // output the form-field
	    $formobj->close( TRUE ); // close form, sshow submit-button
	    
	    echo "<br>";
	    $user_roles_raw   = oDB_USER_sub2::roleListGet ($sqlo, $this->objid);
	    $user_roles = array_keys($user_roles_raw);
	    $this->_obj_list($sqlo, 'ROLE', $user_roles);
	}
	


	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$this->go   = $_REQUEST['go'];
		$action   = $_REQUEST['action'];
		$this->objid   = $_REQUEST['id'];
		$parx   = $_REQUEST['parx'];
		$sqlo = &$this->_sqlo;
		
		

		$go = $this->go;
		
		switch ($action) {
		    
		    case 'deactivate':
       
        		if ( !$go ) {
        		    $this->deactivate_form0();
        			return;
        		}
        		
        		$user_lib = new DB_userC();
        		$user_lib->setUserID($this->objid);
        		$user_lib->deactivate($sqlo);
        		
        		$out_buffer= $user_lib->out_buffer;
        		if (!empty($out_buffer)) {
        		    echo "Info:<br>";
        		    echo implode("<br>", $out_buffer);
        		    echo "<br>\n";
        		}
        		
        		if (!$error->Got(READONLY))  {
        			cMsgbox::showBox("ok", "User deactivated.");
        		}
        		break;
        		
		    case 'grp_edit':
		        if ( !$go ) {
		            $this->group_add_form($sqlo);
		            return;
		        }
		        $subaction= $parx['subact'];
		        $grp_id   = $parx['grp_id'];
		        if (!$grp_id) {
		            $this->htmlFoot('ERROR', 'No group given.');
		        }
		        $grp_lib = new oUSER_GROUP_mod($sqlo, $grp_id);
		        
		        $act_nice=NULL;
		        if ($subaction=='add') {
		            $act_nice='added';
		            $grp_lib->add_user($sqlo, $this->objid);
		            $grp_lib->close($sqlo);
		        }
		        if ($subaction=='del') {
		            $act_nice='deleted';
		            $grp_lib->remove_user($sqlo, $this->objid);
		            $grp_lib->close($sqlo);
		        }
		        if ($act_nice==NULL) {
		            $this->htmlFoot('ERROR', 'Action '.$subaction.'unknown.');
		        }
		        if ($error->Got(READONLY))  {
		            return;
		        }
		        
	            cMsgbox::showBox("ok", "Group ".$act_nice.".");
	            echo "<br>";
	            $this->group_add_form($sqlo);

		        break;
		        
		    case 'role_edit':
		        if ( !$go ) {
		            $this->role_add_form($sqlo);
		            return;
		        }
		        
		        $subaction= $parx['subact'];
		        $role_id   = $parx['role_id'];
		        if (!$role_id) {
		            $this->htmlFoot('ERROR', 'No role given.');
		        }
		        
		        $act_nice=NULL;
		        
		        $role_lib = new oROLE_mod($sqlo, $role_id);
		        if ($subaction=='add') {
		            $act_nice='added';
		            $role_lib->add_user_role($sqlo, $this->objid);
		            $role_lib->close($sqlo);
		        }
		        if ($subaction=='del') {
		            $act_nice='deleted';
		            $role_lib->remove_user_role($sqlo, $this->objid);
		            $role_lib->close($sqlo);
		        }
		        if ($act_nice==NULL) {
		            $this->htmlFoot('ERROR', 'Action '.$subaction.'unknown.');
		        }
		        
		        if ($error->Got(READONLY))  {
		            return;
		        }
		        
		       
		        cMsgbox::showBox("ok", "Role ".$act_nice.".");
		        echo "<br>";
		        $this->role_add_form($sqlo);
		        
		        break;
		        
		    default:
		        $this->htmlFoot('ERROR', 'Action "'.$action.'" is unknown.');
		}
		
// 		if ($go and !$error->Got(READONLY))  {
// 		    $this->_html->pageForward('edit.tmpl.php?t=DB_USER&id='.$this->objid, 'back to user', 2000);
// 		}
		
	}
	
}
