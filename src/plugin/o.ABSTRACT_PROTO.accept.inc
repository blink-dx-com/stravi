<?php
require_once 'glob.obj.copyobj1.inc';
require_once ("f.assocUpdate.inc");
require_once ("glob.obj.update.inc");

/**
 * o.ABSTRACT_PROTO > create new acceptance proto as copy from old 
 *    
 * @package o.ABSTRACT_PROTO.accept.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   SREQ:0003414: o.ABSTRACT_PROTO > create new acceptance proto as copy from old 
 *   
 */
class o_ABSTRACT_PROTO_accept_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'ABSTRACT_PROTO';
		$title		= 'Create copy of acceptance proto';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array() )
				);
		
	}
	
	function form1() {
		
		$initarr   = NULL;
		$initarr["title"]       = "Select source acceptance protocol"; // title of form
		$initarr["submittitle"] = "Create!"; // title of submit button
		$initarr["tabwidth"]    = "400";   // table-width: AUTO
		$initarr["dblink"]		= 1;

		$hiddenarr = NULL; // hidden form variables
		$hiddenarr["id"]     = $this->objid; // is a required input parameter
		

		// init form, uses class formc
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);

		$fieldx = array ( // form-field definition
			"title" => "Copy from", 
			"name"  => "acsrc",
			"object"=> "dblink",
			"val"   => NULL, 
			"inits" => array( "table"=>"ACCEPT_PROT", "objname"=>NULL, "pos" =>"0", "projlink"=> 1),
			"notes" => "source"
			 );
		$formobj->fieldOut( $fieldx ); // output the form-field

		$formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	function create(&$sqlo, &$sqlo2, $srcid) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$copyLib = new gObjCopyS();
		$newobj  = $copyLib->copyOne($sqlo, $sqlo2, 'ACCEPT_PROT', $srcid);
		if (!$newobj) {
			$error->set( $FUNCNAME, 1, 'creation of object ACCEPT_PROT failed' );
			return;
		}
		
		$this->_html->line_out('New protocol created. ID:'.$newobj);
		
		// any steps copied ?
		$sqlsel = 'count(1) from ACCEPT_PROT_STEP where ACCEPT_PROT_ID='.$newobj;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$newAccCnt = $sqlo->RowData[0];
		
		if (!$newAccCnt) return; // no action needed
		
		$oldAccFeats = glob_elemDataGet3($sqlo, 'ACCEPT_PROT', array('ACCEPT_PROT_ID'=>$srcid), array(ABSTRACT_PROTO_ID));
		$oldPraID    = $oldAccFeats['ABSTRACT_PROTO_ID'];
		
		
		// correct values:
		// - remove non-existing steps of current ABSTRACT_PROTO_ID
		// - exchange old ABSTRACT_PROTO_ID by new ABSTRACT_PROTO_ID in ACCEPT_PROT_STEP
		
		// get step-numbers of new PRA
		$newpra_steps = array();
		$sqlsel = "STEP_NR from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$this->objid;
		$sqlo->Quesel($sqlsel);
		while ( $sqlo->ReadRow() ) {
		    $newpra_steps[] = $sqlo->RowData[0];
		}
		
		$badSteps=array();
		// check new acceptance steps
		$sqlsel = "STEP_NR from ACCEPT_PROT_STEP where ACCEPT_PROT_ID=".$newobj;
		$sqlo->Quesel($sqlsel);
		while ( $sqlo->ReadRow() ) {
		    $newstep = $sqlo->RowData[0];
		    if (!in_array($newstep, $newpra_steps)) $badSteps[]=$newstep;
		}
		
		$assoclib = new  fAssocUpdate();
		$assoclib->setObj($sqlo, 'ACCEPT_PROT_STEP', $newobj);
		
		if (sizeof($badSteps)) {
			
			// remove bad steps
			$stepSQLstr = 'STEP_NR in ('.implode(', ',$badSteps).')';
			$assoclib->delMore($sqlo, $stepSQLstr);
			
		}
		
		
		$UpdateLib = new globObjUpdate();
		$args  = array( 'vals'=>array('ABSTRACT_PROTO_ID'=>$this->objid)  );
		$UpdateLib->update_meta( $sqlo, 'ACCEPT_PROT', $newobj, $args );
		
		// update ABSTRACT_PROTO_ID, because during copy protcess the OLD ABSTRACT_PROTO_ID was copied!
		
		$sqlWhere = "ACCEPT_PROT_ID=".$newobj;
		$argu     = array('ABSTRACT_PROTO_ID'=>$this->objid);
		$assoclib->updateMore($sqlo, $argu, $sqlWhere);
		
	}
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$accTableNice = tablename_nice2('ACCEPT_PROT'); 
		$this->objid  = $_REQUEST['id'];
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		$t_rights = tableAccessCheck ( $sqlo , 'ACCEPT_PROT' );
		if ( $t_rights ["insert"] != 1 ) {
			$this->htmlFoot('ERROR', 'You have no role - right INSERT on table ACCEPT_PROT.');
		}
		
		if (!$go) {
			$this->form1();
			return;
		}
		
		if ($parx['acsrc']==NULL) {
			$this->htmlFoot('ERROR', 'Need a source '.$accTableNice);
		}
		
		
		$this->create($sqlo, $sqlo2, $parx['acsrc']);
		
		if ($error->Got(READONLY))  {
			return;
		}
		
		$this->_html->pageForward('edit.tmpl.php?t=ABSTRACT_PROTO&id='.$this->objid,'Back to '. tablename_nice2('ABSTRACT_PROTO'));
		
		
	}
	
}