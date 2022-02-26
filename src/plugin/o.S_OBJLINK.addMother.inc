<?php
require_once 'o.S_OBJLINK.subs.inc';
require_once 'o.S_OBJLINK.mod.inc';
require_once 'o.CCT_TABLE.info.inc';
require_once ( "javascript.inc" );
require_once ("f.objview.inc");

/**
 * o.S_OBJLINK > add/delete any mother-object to/from object 
 * - called by glob.obj.feat.php
 * @swreq UREQ:0001429: g > SAP > Child-Material zu Mother-Material 
 * @package o.S_OBJLINK.addMother.inc 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $_REQUEST['t']  child tablename
 * @param   $_REQUEST['id'] id of object
 * @param   $_REQUEST['go'] 
 * 	0: give table
 *  1: give object + key
 *  2: update
 * @param   $_REQUEST['parx']
 * 	 action 
 *  	'add' add object
 *  	'del' delete link of child; need 'childpos'
 * 	 motab : [OPTIONAL] mother-table (can only be set, if user has special role-right o.S_OBJLINK.modify)
 *   moid  : ID of mother
 *   childkey : KEY for S_OBJLINK
 *   childpos : needed for action=='del'
 * @param   $_REQUEST['backurl'] [OPTIONAL] encoded URL
 */
class o_S_OBJLINK_addMother_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= $_REQUEST['t'];
		$act = $_REQUEST['parx']['action'];
		
		$title		= 'Mother/child action'; // default
		if ($act=='add') $title		= 'Add a mother-object to this object';
		if ($act=='del') $title		= 'Delete mother-object-link from this object';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = 'child-object action';
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		$this->infoarr['inforow']  = 'Add/delete a child object. A user needs the role right "o.S_OBJLINK.modify" to use this tool.';
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read','write'), 'obj'=>array('insert') )
				);
				
	}
	
	function _forward() {
		if ($_REQUEST['backurl']!=NULL) {
			$backurl = urldecode($_REQUEST['backurl']);
		} else $backurl = 'glob.obj.feat.php?t='.$this->chTable.'&id='.$this->objid;
		js__location_replace($backurl, 'back');
	}
	
	function _getHiddenParams() {
		$hiddenarr = NULL ; // hidden form variables
		$hiddenarr["id"] = $this->objid;
		$hiddenarr["t"] = $this->moTable;
		$hiddenarr["parx[action]"] = $this->action;
		return $hiddenarr;
	}
	
	function _form0(&$sqlo) {
		$initarr = NULL ;
		$initarr ["title"] = "Select table"; // title of form
		$initarr ["submittitle"] = "Next"; // title of submit button
		$initarr ["tabwidth"] = "AUTO"; // table - width : AUTO
		
		$hiddenarr = $this->_getHiddenParams();
		
		$TabLib = new oCCT_TABLE_info();
    	$tableArray    = $TabLib->getTablesNiceByType($sqlo, 'BO');
		
		$formobj = $this ->_html->formInit ( $initarr , $hiddenarr , 0);
		$fieldx = array ( 
			"title" => "table",
			"name"  => "childtab",
			"object"=> "select",
			"inits" => $tableArray,
			"val"   => '',
			"notes" => "table name"
			);
		$formobj -> fieldOut ( $fieldx ); // output the form - field
		$formobj -> close ( TRUE ); // close form , sshow submit - button
	}
	
	function _form1(&$sqlo, $parx) {
		global $error ;
		$FUNCNAME = __CLASS__.':'.__FUNCTION__;
		
		if ($parx['childtab']==NULL) {
			$error->set ( $FUNCNAME , 1, 'child tablename missing' );
			return;
		}
		
		$childtab=$parx['childtab'];
		
		$initarr = NULL ;
		$initarr ["title"]    = "Select child object"; // title of form
		$initarr ["submittitle"] = "Update"; // title of submit button
		$initarr ["tabwidth"] = "AUTO";    // table - width : AUTO
		$initarr ["dblink"]   = 1;
		
		$hiddenarr = $this->_getHiddenParams();
		$hiddenarr["parx[childtab]"] = $childtab;
		
		$childTabNice = tablename_nice2($childtab);
		
		// init form , uses class formc
		$formobj = $this ->_html->formInit ( $initarr , $hiddenarr , 1);
		$fieldx = array ( 
			"title" => $childTabNice." object",
			"name"  => "moid",
			"object"=> "dblink",
			"val" => '',
			"inits" => array( "table"=>$childtab, "objname"=>'', "pos" =>"0", "projlink"=>1, 'noshDel'=>1),
			"notes" => ""
			);
		$formobj->fieldOut ( $fieldx ); // output the form - field
		
		$fieldx = array ( 
			"title" => "Key",
			"name"  => "childkey",
			"object"=> "text",
			"val" => '',
			"notes" => "[optional] key"
			);
		$formobj -> fieldOut ( $fieldx ); // output the form - field
		
		$formobj->close ( TRUE ); // close form , sshow submit - button
	}
	
	/**
	 * insert new MOTHER-CHILD-connection
	 * @param $sqlo
	 * @param $parx
	 */
	function _update(&$sqlo, $parx) {
		global $error ;
		$FUNCNAME = __CLASS__.':'.__FUNCTION__;
		
		if ($parx['motab']==NULL) {
			$parx['motab'] = $this->chTable;
		}
		
		if ($parx['motab']==NULL) {
			$error->set ( $FUNCNAME , 1, 'mother tablename missing' );
			return;
		}
		if (!$parx['moid']) {
			$error->set ( $FUNCNAME , 2, 'mother ID missing' );
			return;
		}
		if ($parx['childkey']==NULL) {
			$error->set ( $FUNCNAME , 3, 'KEY missing' );
			return;
		}
		if ($parx['motab']==$this->chTable and $parx['moid']==$this->objid) {
			$error->set ( $FUNCNAME , 4, 'mother and child must be different.' );
			return;
		}
		
		$objLinkLib = new fObjViewC();
		$htmlTmp = $objLinkLib->bo_display( $sqlo, $parx['motab'], $parx['moid']);
		echo 'mother object: '.$htmlTmp."<br><br>";
		
		$objLinkSubLib = new oS_OBJLINK_subs();
		$pos = $objLinkSubLib->chiObjLinked($sqlo, $parx['motab'], $parx['moid'], $this->chTable, $this->objid);
		if ($pos>0)  {
			$error->set( $FUNCNAME, 3, 'mother-object already linked on mother.' );
			return;
		}
		
		$sObjLib = new oS_OBJLINK_mod();
		$sObjLib->setObj( $sqlo, $parx['motab'], $parx['moid']);
		$sObjLib->addLink($sqlo,  $this->chTable, $this->objid, $parx['childkey']);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 4, 'error during adding mother.' );
			return;
		}
	}
	
	function _del_init(&$sqlo, $parx) {
		
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		if (!$parx['pos']) {
			$error->set( $FUNCNAME, 1, 'POS of child missing' );
			return;
		}
		
		$pos = $parx['pos'];
		$objLinkSubLib = new oS_OBJLINK_subs();
		$childObjArr = $objLinkSubLib->getChiObjByPos($sqlo, $this->moTable, $this->objid, $pos);
		
		if ( !is_array($childObjArr) ) {
			$error->set( $FUNCNAME, 1, 'no child found on POS '.$pos );
			return;
		}
			
	
		$objLinkLib = new fObjViewC();
		$htmlTmp = $objLinkLib->bo_display( $sqlo, $childObjArr[0], $childObjArr[1]);
		echo 'child object: '.$htmlTmp."<br><br>";
		
		return;
	}
	
	function _del_form0(&$sqlo, $parx) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		
		$initarr = NULL ;
		$initarr ["title"]    = "Delete child-object-link ?"; // title of form
		$initarr ["submittitle"] = "Delete child link"; // title of submit button
		$initarr ["tabwidth"] = "AUTO"; // table - width : AUTO
		
		$hiddenarr = $this->_getHiddenParams();
		$hiddenarr['parx[pos]']=$parx['pos'];
		$formobj = $this->_html->formInit ( $initarr , $hiddenarr , 0);
		$formobj->close ( TRUE ); // close form , sshow submit - button
	}
	
	function _del_delete(&$sqlo, $parx) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		$sObjLib = new oS_OBJLINK_mod();
		$sObjLib->setObj( $sqlo, $this->moTable, $this->objid);
		$sObjLib->removeLink($sqlo, $parx['pos'] );
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'error during delete child on pos'.$parx['pos'].' .' );
			return;
		}
		$this->_html->rawHtmlCR('child-link removed ...');
		
	}
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$this->chTable = $_REQUEST['t'];
		$this->objid   = $_REQUEST['id'];
		$sqlo = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		$tablename = $this->chTable;
		
		/*
		$role_right_name = "o.S_OBJLINK.modify";
		$role_right      = role_check_f($sqlo, $role_right_name);
		if ($role_right!="execute" ) {
			htmlFoot('ERROR',  "Sorry, you must have role right '$role_right_name'. to use this tool.");
		}
		*/
		

		
		$parx = $_REQUEST['parx'];
		$go   = $_REQUEST['go'];
		
		if ($parx['action']==NULL) {
			$this->htmlFoot('ERROR', 'action missing');
		}
		$this->action = $parx['action'];
		
		if ($parx['action']=='add') {
			/*
			 * STILL not supported
			if (!$go) {
				$this->_form0($sqlo);
				return;
			}
			
			if ($go==1) {
				$this->_form1($sqlo, $parx);
				return;
			}
			*/
			
			if ($go==2) {
				$this->_update($sqlo, $parx);
				if ($error->Got(READONLY))  {
					return;
				}
				
				$this->_forward();
			}
		}
		if ($parx['action']=='del') {
			/*
			 * STILL not supported
			$this->_del_init($sqlo, $parx);
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 1, 'init of delete failed.' );
				return;
			}
			
			if (!$go) {
				$this->_del_form0($sqlo, $parx);
				return;
			}
			if ($go==1) {
				$this->_del_delete($sqlo, $parx);
				if ($error->Got(READONLY))  {
					return;
				}
				$this->_forward();
			}
			*/
		}
	}
	
}