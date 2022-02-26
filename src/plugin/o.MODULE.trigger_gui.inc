<?php
require_once ('func_form.inc');
require_once 'o.MODULE.trigger.inc';
/**
 * change workflow of object
 *    
 * @package o.MODULE.trigger_gui.inc 
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param $_REQUEST['tablename'] tablename
 * @param $_REQUEST['id'] id of object
 
 * 
 */
class o_MODULE_trigger_gui_XPL extends gPlugin {
	var $CLASSNAME='o_MODULE_trigger_XPL';
	var $cct_access_id;
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= $_REQUEST['tablename'];
		$title		= 'change workflow assignment';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = 'modify';
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array() )
				);
				
		
	}
	
	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		$this->objid   = $_REQUEST['id'];
		$this->tablename = $_REQUEST['tablename'];
		$objid = $this->objid;
		$sqlo = &$this->_sqlo;
		$sqltcct = "CCT_ACCESS_ID from ".$this->tablename." where ".$this->tablename."_ID = ". $objid;
		$sqlo->Quesel($sqltcct);
		$sqlo->ReadRow();
		$cct_access_id  = $sqlo->RowData[0];
		$this->cct_access_id = $cct_access_id;
		//$this->showtriggertable($sqlo);
		if ($this->_go) {
			$this->parx = $_REQUEST['parx'];
			$parx = $this->parx;
			$this->newtrigger = $parx["newtrigger"];
			$this->changetrigger($sqlo, $this->newtrigger);
			
			$error->printAll();
			
		}
		$trigger = $this->getacttrigger($sqlo, $cct_access_id);
		if ($trigger["id"] == ""){
			echo "<br />\n No workflow set for ".$this->tablename."_ID " . $objid 
			. " (" .$trigger["bo"]. ").<br />\n<br />\n";
		}else {
			echo "<br />\nCurrent workflow for ".$this->tablename."_ID " . $objid
			. " (".$trigger["bo"].") is " . $trigger["name"] . ".<br />\n<br />\n";
		}
		$this->form($trigger["id"], $sqlo);
		
	}
	
	/**
	 * function for showing workflow table
	 * @param $sqlo
	 * @return -
	 */
	function showtriggertable(&$sqlo){
		
		$sqltrig = "MXID, NAME, LOCATION, NOTES from MODULE";
		$sqlo->Quesel($sqltrig);
		// show a table
		$headx  =array('ID', 'name of workflow', 'location', 'notes');
		$headOpt=array("title"=>'available workflows');
		$this->_html->_table3_init($headx, $headOpt);
		while ( $sqlo->ReadRow() ) {
			$dataArr=array($sqlo->RowData[0], $sqlo->RowData[1], $sqlo->RowData[2], $sqlo->RowData[3]);
			$this->_html->_table3_row($dataArr);
		}
		$this->_html->_table3_close();
		
	}
	/**
	 * function asks workflow values for BO
	 * @param $sqlo
	 * @param int $cct_access_id CCT_ACCESS_ID of BO
	 * @return $trigger array of workflow values id, name, bo workflow is def for, bo ID
	 */
	function getacttrigger(&$sqlo, $cct_access_id){
		$sqltrig = "WFLOW_ID, TABLE_NAME from CCT_ACCESS where CCT_ACCESS_ID = ". $cct_access_id;
		$sqlo->Quesel($sqltrig);
		$sqlo->ReadRow();
		$trigger=array();
		$trigger ["id"] = $sqlo->RowData[0];
		$trigger ["table"] = $sqlo->RowData[1];
		$sqlbo = $trigger ["table"] . "_ID, NAME from ". $trigger ["table"] . " where CCT_ACCESS_ID = ". $cct_access_id;
		$sqlo->Quesel($sqlbo);
		$sqlo->ReadRow();
		$trigger["bo_id"]  = $sqlo->RowData[0];
		$trigger["bo"]  = $sqlo->RowData[1];
		if ($trigger ["id"] !=""){
			$sqltrig = "NAME from MODULE where MXID = ". $trigger ["id"];
			$sqlo->Quesel($sqltrig);
			$sqlo->ReadRow();
			$trigger["name"]  = $sqlo->RowData[0];
		}
		return $trigger;
		
	}
	/**
	 * function changes value of WFLOW_ID in CCT_ACCESS
	 * @param $sqlo
	 * @param int $newtrigger new trigger ID for BO
	 * @param $cct_access_id
	 * @return -
	 */
	function changetrigger(&$sqlo, $newtrigger){
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		if ($newtrigger != NULL){
			$right = $this->checkrights($sqlo);
			if ($right == 1){
				return;
			}
		}

		oMODULE_trigger::obj_mod_trigger($sqlo, $newtrigger, $this->tablename, $this->objid);
		if ($error->Got(READONLY))  {
		    $error->set( $FUNCNAME, 1, 'error on update.' );
		    return;
		}
		
		
		echo "<br />\n<b>Successfully changed! </b><br />\n";
		
	}
	/**
	 * function shows the selection field for new workflow
	 * @param $triggervalue
	 * @param $sqlo
	 * @return -
	 */
	function form($triggervalue, &$sqlo){
		$initarr   = NULL;
		$initarr["title"]       = "modify workflow";
		$initarr["submittitle"] = "change";
		$initarr["tabwidth"]    = "AUTO";
		$initarr["dblink"] = 1;
		
		$hiddenarr = NULL;	
		$hiddenarr['id'] = $this->objid;
		$formobj = new formc( $initarr, $hiddenarr, 0 );
		$fieldy=array();
		
		$fieldpos = 1;
   		// WARNING:  need $initarr["dblink"] = 1;
    	$fieldy = array ("title" => "workflow selection", "name"  => "newtrigger", 
                     "val"   => $triggervalue, 
                     "notes" => "select", 
                     "inits" => array( 'table'=>'MODULE', 
                     					'getObjName'=>1, 
    			                     	'sqlo'=>&$sqlo, 
    			                     	'pos' =>$fieldpos, 
    			                     	'projlink'=> 1, 
    			                     	'urlfull' => 1),
    				"object" => "dblink");       
    	$formobj->fieldOut( $fieldy ); 
		
		
		
		$formobj->close(TRUE);
		
	}
	/**
	 * function checks, if right f.g.triggerEdit is set for role of DB_USER
	 * @param $sqlo
	 * @return $right BOOLEAN, 0 if right is set, 1 if not
	 */
	function checkrights(&$sqlo){
		$role_right_name = "g.triggerEdit";
		$role_right      = role_check_f($sqlo, $role_right_name);
		$right = 0;
 		if ($role_right!="execute" ) {
			echo "<br>";
			htmlErrorBox("Action denied!", "Sorry, you must have role right '$role_right_name'. to use this tool."  );
			htmlFoot();
			$right = 1;
		}
		return $right;
	}
	
}