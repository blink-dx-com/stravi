<?php
require_once('o.MODULE.trigger.inc');

/**
 * set worklflow for glob.obj.crea_wiz.php
 *    
 * @package g.crea-wiz.prefs
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0000958: glob.obj.crea_wiz.php > setze Workflow beim Erstellen 
 * @param $_REQUEST['t'] tablename
 * @param $_REQUEST['go'] 0,1
 * @param $_REQUEST['parx']['MX_ID'] Module-ID, NULL allowed
 *   
 */
class g_crea_wiz_prefs_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$title		= 'Wizard > set Workflow';
		$tablename=$_REQUEST['t'];
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = 'Set Workflow';
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['locrow']   = array(array('glob.obj.crea_wiz.php?tablename='.$tablename,'Object creation wizard'));
	}
	
	function _collectTriggers(&$sqlo) {
		$tablename=$this->tablename;
		$triggerLib = new oMODULE_trigger();
		$moduleArr = $triggerLib->getTriggersOfTable($sqlo, $tablename);
		
		if (empty($moduleArr)) {
			return;
		}
		$niceModArr=NULL;
		foreach( $moduleArr as $mxid) {
			$name= obj_nice_name ( $sqlo, 'MODULE', $mxid );
			$niceModArr[$mxid]=$name;
		}
		
		return  $niceModArr;
	}
	
	/**
	 * - creates a form with one text field "Min-value"
	 * - creates a HTML-page parameter $_REQUEST['parx']['minval'] after SUBMIT
	 */
	function form1(&$sqlo) {
		global $error;
		$FUNCNAME='form1';
		
		$niceModArr = $this->_collectTriggers($sqlo);
		if (!$niceModArr) {
			$error->set( $FUNCNAME, 1, 'No workflows for this table-type exist. Please ask your admin. (set MOD_META: KEY="TABLE_NAME", VALUE=[table])' );
			return;
		}
		
		$initarr   = NULL;
		$initarr["title"]       = "Set Workflow"; // title of form
		$initarr["submittitle"] = "Set"; // title of submit button
		$initarr["tabwidth"]    = "AUTO";   // table-width: AUTO

		$hiddenarr = NULL; // hidden form variables
		$hiddenarr["t"]     = $this->tablename;

		// init form, uses class formc
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);

		$fieldx = array (
			"title"   => "Workflow", 
			"name"    => "MX_ID",
			"object"  => "select",
			"val"     => $this->old_MXID,
			'inits'=> $niceModArr, 
			"notes"   => "the Workflow"
			 );
		$formobj->fieldOut( $fieldx ); // output the form-field

		$formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	function _setTrigger(&$sqlo, $MX_ID) {
		$this->wizardOpts['trigger']=$MX_ID;
		$_SESSION['userGlob']["o.".$this->tablename.".fCreaOpt"]=serialize($this->wizardOpts);
		
	}
	
	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$this->tablename   = $_REQUEST['t'];
		$go   = $_REQUEST['go'];
		$sqlo = &$this->_sqlo;
		$tablename=$this->tablename;
		
		if ($this->tablename==NULL) {
			$error->set( $FUNCNAME, 1, 'tablename missing' );
			return;
		}
		
		if ($_SESSION['userGlob']["o.".$tablename.".fCreaOpt"]!=NULL) {
			$this->wizardOpts = unserialize($_SESSION['userGlob']["o.".$tablename.".fCreaOpt"]);
			$this->old_MXID=$this->wizardOpts['trigger'];
		}
		
		if (!$go) {
			 $this->form1($sqlo);
			 return;
		}
		
		if ($go==1) {
			$parx   = $_REQUEST['parx'];
			$this->_setTrigger($sqlo, $parx['MX_ID']);
		}
		
		require_once ( "javascript.inc" );
		js__location_replace('glob.obj.crea_wiz.php?tablename='.$tablename, "Creation wizard" ); 
		
	}
	
}