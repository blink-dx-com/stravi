<?php

require_once 'lev1/o.SUA.versman.mod.inc';
require_once 'lev1/o.SUA.versman.sub.inc';

/**
 * o.ABSTRACT_SUBST > [VersMan] > neue Version anlegen 
 *    
 * @package o.SUA.versman.new.inc
 * @swreq   SREQ:0003349: o.ABSTRACT_SUBST > [VersMan] > neue Version anlegen 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $_REQUEST['id'] SUA_ID
 * @param   $_REQUEST['go'] 
 * 	0: prepare
 * 	1: do create
 * @version $Header: trunk/src/plugin/o.SUA.versman.new.inc 59 2018-11-21 09:04:09Z $  
 */
class o_SUA_versman_new_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'ABSTRACT_SUBST';
		$title		= '[VersMan] Create new Version';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		
		//$this->infoarr['help_url'] = 'p129/xxxx'; // link im Wiki: 
		//$this->infoarr['help_base']= 'wiki';  // use wiki
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array() )
				);	
		
	}
	
	/**
	 * init tool
	 * @param $sqlo
	 * @return array('ok'=>0,1)
	 */
	function initx(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		$this->versModLib = new oSUA_versman_mod();
		$this->versModLib->setSubst($sqlo, $this->objid);
		
		$this->versSubLib = new oSUA_versman_sub();
		$this->versSubLib->getSUA_type($sqlo, $this->objid);
		
		$this->versType = $this->versSubLib->getSUA_type($sqlo, $this->objid);
		if ($this->versType!=2) {
			return array('ok'=>-1,'info'=>'This substance is not an activate version. 
				First activate the substance for PLM.');
		}
		
		$this->master_id = $this->versSubLib->getMasterID($sqlo, $this->objid);
		if (!$this->master_id) {
			return array('ok'=>-2, 'info'=>'Can\'t find the VersionMaster.');
		}
		
		// @swreq UREQ:3349:005 check insert rights on the VersionMaster-SUA
		$o_rights = access_check($sqlo, 'ABSTRACT_SUBST', $this->master_id);
		if ( !$o_rights["insert"] ) {
			return array('ok'=>-3, 'info'=>'Permission denied. You need "insert"-rights on the VersionMaster-SUA [SUA-ID:'.$this->master_id.']');
		}
		
		// @swreq UREQ:3349:004 check: max X versions without DATES allowed (DATE_START, DATE_END)
		/*
		$sqlsel = 'count(1) from SUA_VERS where ABSTRACT_SUBST_ID='.$this->master_id.' and DATE_START is NULL';
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$numEmpty = $sqlo->RowData[0];
		$MAX_COUNT_EMPTY=3;
		if ($numEmpty>=$MAX_COUNT_EMPTY) {
			return array('ok'=>-4, 'info'=>'Creation denied. There are already at least '.$MAX_COUNT_EMPTY.' NON-DATED versions for this VersionMaster.
			 Please use these versions first or delete them.');
		}
		*/
		
		return array('ok'=>1);
	}
	
	/**
	 * form
	 */
	function form1() {
		
		$initarr   = NULL;
		$initarr["title"]       = "Create new version"; // title of form
		$initarr["submittitle"] = "Create"; // title of submit button
		$initarr["tabwidth"]    = "AUTO";   // table-width: AUTO

		$hiddenarr = NULL; // hidden form variables
		$hiddenarr["id"]     = $this->objid; // is a required input parameter

		// init form, uses class formc
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
		
		$fieldx = array ( // form-field definition
				"title"   => 'Notes', 
				"name"    => 'NOTES',
				"object"  => "textarea",
				"val"     => '',
				"req"     => 1, 
				"notes"   => 'Version notes'
				 );
				 
		$formobj->fieldOut( $fieldx ); 

		$formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	/**
	 * create SUA
	 * @param $sqlo
	 * @param $sqlo2
	 */
	function createx(&$sqlo, &$sqlo2, $parx) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$flagError=0;
		$errLast  =NULL;
		
		$newSuaID = $this->versModLib->createCopy($sqlo, $sqlo2);
		if (!$newSuaID) {
			$error->set( $FUNCNAME, 1, 'creation of version faild.' );
			return;
		}
		if ($error->Got(READONLY))  {
			$errLast   = $error->getLast();
			$error->reset();// reset error stack
			$flagError=1;   // throw error later again
		}
		
		$this->_html->line_out('New SUA created: SUA-ID:'.$newSuaID);
		
		$versModLib = new oSUA_versman_mod();
		$versModLib->setSubst($sqlo, $newSuaID);
		$params     = array('NOTES'=>$parx['NOTES']);
		$versModLib->addToMaster($sqlo, $this->master_id, $params);
		
		if ($flagError) {
			$error->set( $FUNCNAME, 2, $errLast->text );
		}
		
		return $newSuaID;
	}
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$this->objid   = $_REQUEST['id'];
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		$answer = $this->initx($sqlo);
		if ($answer['ok']<1) {
			$this->htmlFoot('WARN',$answer['info']);
		}
		
		if (!$go) {
			$this->form1();
			return;
		}
		
		if ($parx['NOTES']==NULL) {
			$this->htmlFoot('WARN','Parameter "Notes" missing.');
		}
		
		$newid = $this->createx($sqlo, $sqlo2, $parx);
		if ($error->Got(READONLY))  {
			return;
		}
		
		$this->_html->pageForward('edit.tmpl.php?t=ABSTRACT_SUBST&id='.$newid,'go to new version');
	}
	
}