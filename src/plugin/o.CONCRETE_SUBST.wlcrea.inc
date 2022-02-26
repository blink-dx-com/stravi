<?php

require_once ("o.S_OBJLINK.mod.inc");
require_once ("o.S_OBJLINK.subs.inc");
require_once ("insertx.inc");
require_once ("lev1/glob.obj.create_subs.inc");

/**
 * creat a worklist
 *    
 * @package o.CONCRETE_SUBST.wlcrea.inc 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0002203: o.WORKLIST : create worklist by substance (concrete) 
 * @param $id : substance id
 * @param $awlid : abstract worklist-ID; if false ABS_WORKLIST_ID: no problem, check it
 * @param $go : 
 * 		0 : check
 * 		1 : create and go back
 */
class o_CONCRETE_SUBST_wlcrea_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'CONCRETE_SUBST';
		$title		= 'create worklist from substance (concrete)';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('write'), 'obj'=>array('insert') )
				);
		
	}
	
	function initTool(&$sqlo) {
		$objLinkLib = new oS_OBJLINK_subs();
		$moTable = 'CONCRETE_SUBST';
		
		$objid = $this->objid;
		$abs_worklist_id = $this->awlid;
		

		$chiObjArr  = $objLinkLib->getChiObjByChiTab($sqlo, $moTable, $objid, 'WORKLIST');
		$numChildren = sizeof($chiObjArr);
		
		$nameWorklist = 'SUC-ID:'.$objid.'('.($numChildren+1).')'; // name of worklist
		
		$this->_html->line_out('Lege neue Worklist "'.$nameWorklist.'" an.');
		
		$this->nameWorklist = $nameWorklist;
		$this->numChildren  = $numChildren;
	}
	
	/**
	* - creates a form with one text field "Min - value "
	* - creates a HTML - page parameter $_REQUEST ['parx '][ ' minval '] after SUBMIT
	*/
	function form1() {
		$initarr = NULL ;
		$initarr ["title"] = "Neue Worklist anlegen"; // title of form
		$initarr ["submittitle"] = "Anlegen"; // title of submit button
		$initarr ["tabwidth"] = "AUTO"; // table - width : AUTO
		$hiddenarr = NULL ; // hidden form variables
		$hiddenarr ["id"]    = $this->objid ; // is a required input parameter
		$hiddenarr ["awlid"] = $this->awlid ; // is a required input parameter
		
		// init form , uses class formc
		$formobj = $this->_html->formInit ( $initarr , $hiddenarr , 0);
		
		$formobj->close( TRUE ); // close form , sshow submit - button
	}
	
	
	/**
	 * - create a worklist
	 * - connect cartridges
	 * - add a trigger and change status to started
	 * @param $sqlo
	 * @param $key
	 * @param array $cartArr of CartIDs; can be EMPTY
	 * @param $nameSuffix = NULL
	 * @return long $ID worklist ID
	 */
	function newWorkList(&$sqlo, $key=NULL) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$objLinkLib = new oS_OBJLINK_subs();
		$moTable = 'CONCRETE_SUBST';
		
		$objid = $this->objid;
		$abs_worklist_id = $this->awlid;

		$nameWorklist = $this->nameWorklist;

		
		$param = array(
			'vals'=>array(
				'NAME'=> $nameWorklist , 
				'ABS_WORKLIST_ID'=>$abs_worklist_id)
		);
		$worklistID = insertC::new_meta ( $sqlo , "WORKLIST", $param );
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'error on insert worklist; name:'. $nameWorklist);
			return;
		}
		
		$crea2Lib = new gObjCreaSubs();
		$crea2Lib->addUserWorkflow($sqlo, "WORKLIST", $worklistID);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'Workflow problem for worklist-ID:'.$worklistID);
			return;
		}
		
		// set mother
		$modlib = new oS_OBJLINK_mod();
		$modlib->setObj($sqlo, $moTable, $objid);
		$retval = $modlib->addLink($sqlo, 'WORKLIST', $worklistID, 'MOTHER');
		
		
		$this->_html->line_out('Created a new worklist '.$nameWorklist);
		
		return $worklistID;
	}
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$this->objid   = $_REQUEST['id'];
		$this->awlid   = $_REQUEST['awlid'];
		$go   = $_REQUEST['go'];
		$sqlo = &$this->_sqlo;
		
		if (!$this->awlid) {
			$this->htmlFoot('ERROR','Give an abstract worklist as param.');
		}
		
		$this->initTool($sqlo);
		
		if (!$go) {
			$this->form1();
			$this->htmlFoot();
		}
		
		$wlistid = $this->newWorkList($sqlo);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, '' );
			return;
		}
		
		$this->_html->line_out('');
		$this->_html->line_out('<a href="edit.tmpl.php?t=WORKLIST&id='.$wlistid.'">=&gt; goto new worklist</a>');
		//$this->_html->pageForward('edit.tmpl.php?t=CONCRETE_SUBST&id='.$this->objid, 'back to object');
	}
	
}