<?php
/**
 * reopen CONCRETE_PROTO of CONCRETE_SUBST > Wenn ein protocol gelocket ist, kann es trotzdem von einem QC-Mitarbeiter wieder geöffnet werden; Nebenregel:
SUBREQ:
 001: Mitarbeiter hat Rollenrecht QC;
 002: Mitarbeiter muss Schreibrecht auf Substanz haben
 *    
 * @package o.SUC.cp_reopen.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   UREQ:0001352:006: Wenn ein protocol gelocket ist, kann es trotzdem von einem QC-Mitarbeiter wieder geöffnet werden
 * @param   $_REQUEST['id']   SUC-ID
 * @param   $_REQUEST['cpid'] CONCRETE_PROTO_ID
 * @param $go 
 *   0 : analyse
 *   1 : do reopen
 *   
 */
require_once ('func_form.inc');
require_once ('f.signature.sub.inc');
require_once ('o.CCT_ACCLOG.gui.inc');
require_once ('lev1/o.CCT_ACCESS.reopen.inc');
require_once 'o.CS_HAS_PR.subs.inc';

class o_SUC_cp_reopen_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'CONCRETE_SUBST';
		$title		= 'Reopen protocol of a substance';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		//$this->infoarr['help_url'] = 'plugin/o.suc.protdeakt'; // link im Wiki: 
		$this->infoarr['help_base']= 'wiki';  // use wiki
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		$this->infoarr['inforow'] = 'UREQ:0001352: reopen protocol of substance. Bedingungen: <br />'.
			'001: Mitarbeiter braucht das Rollenrecht "g.QC_expert_lev2"<br />'.
			'002: Mitarbeiter muss Schreibrecht auf Substanz haben.';
		
		// @swreq SUBREQ:001
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read', 'write'), 'obj'=>array('insert') )
				);
				
		
	}
	
	/*	
	 * Builds the form used to reopen a locked audit-trail object.
	 */
	function formReopen(&$sqlo, $parx) {
		
		$table= 'CONCRETE_PROTO';
		$cpid  = $this->cpid;
		$sucid = $this->sucid;
		
		$accLogGuigLib = new oAccLogGuiC();
		$accLogGuigLib->setObject( $sqlo, $table, $cpid );

		$initarr   = NULL;
		$initarr["action"]      = $_SERVER['PHP_SELF'];
		$initarr["title"]       = "Reopen and add reason";
		$initarr["submittitle"] = "Reopen";
		$initarr["tabwidth"]    = "AUTO";

		$hiddenarr = NULL;
		
		$hiddenarr["id"]  = $sucid;
		$hiddenarr["cpid"]= $cpid;
		$hiddenarr["action"]     = $this->action;
		$hiddenarr["backurl"]    = $this->backurl;
		
		$parx["action"] = $this->action;
		
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
		
		$fieldx = array ( 
			"title" => "Reopen reason:",
			"name"  => "notes",
			"req"   => 1,
			"object"=> "textarea",
			"inits" => array("cols"=>60),
			"val"   => $parx["notes"]
			 );

		$formobj->fieldOut( $fieldx );		
		$formobj->close( TRUE );
	}
	
	/**
	 * - reopen object
	 * - no checko of entail-right
	 * @global 
	 * 		$this->cct_access_id
	 * 		$this->tablename 
	 * 		$this->id
	 */
	function doReopen(&$sqlo, $parx) {
		global $error;
		$FUNCNAME= "doReopen";
		
		$tablename     = 'CONCRETE_PROTO';
		$cpid  = $this->cpid;
		
		$cct_access_id = glob_elementDataGet( $sqlo, $tablename, 'CONCRETE_PROTO_ID', $cpid, 'CCT_ACCESS_ID');
		$cct_access_id = $cct_access_id;
		
		if (!$cct_access_id) {
			 throw new Exception('cp has no cct_access_id.');
		}
		
		$reopenLib = new gObjAccReopen($sqlo);
		if ($error->Got(READONLY))  {
			return;
		}
		
		$roptions = array('ignoreRights'=>1);
		$reopenLib->doReopen($sqlo, $cct_access_id, $tablename, $cpid, $parx, $roptions);
	}
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$go   		  = $_REQUEST['go'];
		$parx  		  = $_REQUEST['parx'];
		$this->sucid  = $_REQUEST['id'];
		$this->cpid   = $_REQUEST['cpid'];
		$sqlo  = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		if (!$this->cpid) {
			$this->htmlFoot('WARN', 'No protocol-id given.');
		}
		
		// @swreq SUBREQ:002
		$role_right_name = "g.QC_expert_lev2";
		$role_right      = role_check_f($sqlo, $role_right_name);
		if ( $role_right!="execute" ) {
			$this->htmlFoot('WARN', 'Sorry, you must have role right "'. $role_right_name . '" to use this tool.');
		}
		
		// proto info
		$cpopt = array("absName"=>1);
		$objNice = obj_nice_name ( $sqlo, 'CONCRETE_PROTO', $this->cpid, $cpopt ); 
		$this->_html->line_out('Reopen protocol: '.$objNice );
		$this->_html->line_out('' );
		
		// check, if protocol is in substance
		$sucHasCP_lib = new oCS_HAS_PR_subs($this->sucid);
		
		/* array of
 		*    array('cp'=>ID of protocol, 'or'=>order number, 'ty'=>1,2)
 		*    */
		$protoArr = $sucHasCP_lib->getProtoLog($sqlo);
		
		if (!sizeof($protoArr)) $this->htmlFoot('WARN', 'substance has no protocols.');
		
		$protoFound = 0;
		foreach( $protoArr as $dummy=>$valarr) {
			$cpInSuc = $valarr['cp'];
			if ($cpInSuc == $this->cpid) {
				$protoFound = 1;
				break;
			}
		}
		reset ($protoArr); 
		
		if (!$protoFound) {
			$this->htmlFoot('WARN', 'protocol is not part of this substance.');
		}
		
		if (!$go) {
			$this->formReopen($sqlo, $parx);
			return;
		}
		
		$this->doReopen($sqlo, $parx);
		if ($error->Got(READONLY))  {
			$error->printAllPrio();
			return;
		}
		
		$this->_html->pageForward( 'edit.tmpl.php?t=CONCRETE_SUBST&id='.$this->sucid , 'back to substance');
	}
	
}
