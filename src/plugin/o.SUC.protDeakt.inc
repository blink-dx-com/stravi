<?php
require_once 'o.CS_HAS_PR.subs.inc';
require_once ("f.assocUpdate.inc");
require_once ("glob.obj.update.inc");
require_once 'o.DB_USER.subs.inc';
require_once 'date_funcs.inc';

/**
 * o.CONCRETE_SUBST > one proto > substitute a protocol 
 *    
 * @package o.SUC.protDeakt.inc
 * @swreq UREQ:0002725: o.CONCRETE_SUBST > one proto > substitute a protocol 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $_REQUEST['id'] id of SUC
 * @param   $parx['xorder'] deactivate protocol with xorder
 * @param   $go : 
 *   0 : prepare
 *   1 : do it
 *   
 */
class o_SUC_protDeakt_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'CONCRETE_SUBST';
		$title		= 'Deactivate a protocol';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['help_url'] = 'plugin/o.suc.protdeakt'; // link im Wiki: 
		$this->infoarr['help_base']= 'wiki';  // use wiki
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		$this->infoarr['inforow'] = 'UREQ:0002725: o.CONCRETE_SUBST > one proto > deactivate a protocol';
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read', 'write'), 'obj'=>array() )
				);
				
		
	}
	
	/**
	 * give parameter
	 */
	function form1() {
		
		$initarr   = NULL;
		$initarr["title"]       = "Waehle zu deaktivierendes Protocol"; // title of form
		$initarr["submittitle"] = "Deaktivieren"; // title of submit button
		$initarr["tabwidth"]    = "AUTO";   // table-width: AUTO

		$hiddenarr = NULL; // hidden form variables
		$hiddenarr["id"]     = $this->objid; // is a required input parameter

		// init form, uses class formc
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);

		$fieldx = array ( 
			"title"   => "xorder", 
			"name"  => "xorder",
			"object"=> "text",
			"val"    => "", 
			"req"   => 1,
			"notes" => "Relativer ID des Protocols in der Substanz"
			 );
		$formobj->fieldOut( $fieldx ); // output the form-field
		$fieldx = array ( 
			"title"   => "Grund", 
			"name"  => "reason",
			"object"=> "text",
			"val"    => '', 
			"req"   => 1,
			"notes" => "Grund der Aenderung (max 80 Chars)"
			 );
		$formobj->fieldOut( $fieldx ); // output the form-field

		$formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	function deactivate_it(&$sqlo, $xorder, $reason) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		$sucid = $this->objid;
		
		// check, if xorder exists
		$cslib = new oCS_HAS_PR_subs($sucid);
		$protolog = $cslib->getProtoLog($sqlo);
		
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'Error on protocol-log analysis.' );
			return;
		}
		
		if (!sizeof($protolog)) {
			$this->_html->line_out( "Es gibt keine Protokolle!");
			return;
		}
		
		$found=0;
		
		foreach( $protolog as $valarr) {
			if ($valarr['or']==$xorder) {
				$found=1;
				break;
			}
		}
 
		if (!$found) {
			$this->_html->line_out( "Kein Protokoll gefunden mit xorder=".$xorder." !");
			return;
		}
		
		$this_cpid = $valarr['cp'];
		
		// deactivate !
		// 1. find empty space after xorder=10000
		$next_xorder = $cslib->getFreeDeactOrder($sqlo);
		
		if (!$this_cpid) {
			$this->_html->line_out( "Kein Protokoll gefunden mit xorder=".$xorder." !");
			return;
		}
		
		// update notes-field
		$notesOld = glob_elementDataGet( $sqlo, 'CONCRETE_PROTO', 'CONCRETE_PROTO_ID', $this_cpid, 'NOTES');
		if (strlen($notesOld)>3900) {
			$this->_html->line_out( "Notes-Feld des Protocols schon zu lang fuer die Mod-Meldung !");
			return;
		}
		
		$userName = DB_userC::getFullname($sqlo, $_SESSION ['db_user_id']);
		$newNotes = $notesOld . "\n" . date_unix2datestr(time(), 1).' User:'.$userName.'; Info:Protocol deactivated; Reason:'.$reason;
		$UpdateLib = new globObjUpdate();
		$upOpt = array("noRightCheck"=>1); // ignore "write"-right !
		$args  = array('vals'=> array('NOTES'=>$newNotes) );
		$UpdateLib->update_meta( $sqlo, 'CONCRETE_PROTO', $this_cpid, $args, $upOpt );
		
		$this->_html->line_out( "- Protokoll (ID:".$this_cpid.", xorder:".$xorder.", new_xorder:".$next_xorder.") wird deaktiviert ...");
		
		
		// move protocol to the end
		$idarr = array(
			'CONCRETE_PROTO_ID'=>$this_cpid
		 );
		 $argu = array(
		 	'STEP_NO'=>$next_xorder
		 );
		 
		$assoclib = new  fAssocUpdate();
		$assoclib->setObj( $sqlo, 'CS_HAS_PR', $sucid );
		$assoclib->update( $sqlo, $argu, $idarr);
		
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'Update failed! Inform the Admin.' );
			return;
		}
		$assoclib->close($sqlo);
		
		$this->_html->line_out( "O.k., Protokoll (ID:".$this_cpid.") wurde deaktiviert!");
	}

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$MAX_ORDER=10000;
		$this->objid   = $_REQUEST['id'];
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
		$sqlo = &$this->_sqlo;
		// $sqlo2= &$this->_plug_regSql2();
		
		$role_right_name = "g.QM_expert";
		$role_right = role_check_f ($sqlo , $role_right_name );
		if ( $role_right !='execute' ) {
			echo "Sorry , you must have role right ". $role_right_name .
				" to use this tool.";
			return ;
		}

		if (!$go) {
			 $this->form1();
			 return;
		}
		
		$parx['xorder']=trim($parx['xorder']);
		if ( !is_numeric($parx['xorder']) ) {
			$this->htmlfoot('ERROR', 'xorder muss eine Zahl sein');
		}
		if ( $parx['xorder']>$MAX_ORDER ) {
			$this->htmlfoot('ERROR', 'xorder muss kleiner '.$MAX_ORDER.' sein');
		}
		if ( $parx['reason']==NULL ) {
			$this->htmlfoot('ERROR', 'Gib den Grund an!');
		}
		if ( strlen($parx['reason'])>80 ) {
			$this->htmlfoot('ERROR', 'Anzeahl der Zeichen im Grund muss kleiner 80 sein!');
		}
		
		$this->deactivate_it($sqlo, $parx['xorder'], $parx['reason']);
	}
	
}