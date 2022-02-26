<?php
/**
 * QC analysis of an object
 * - starts a plugin which is configured in table GLOBALS
 * - each concrete subst has an abstract subst, this abstract subst can have a configured QC-plugin 
 * - configuration: ABSTRACT-OBJECT:VARIO:DataAnalysisPlugin = {NAME of PLUGIN}
 * - see developper documentation
 * @package g.QC_analysis2.inc
 * @swreq:  Task:0001501: g.QC-analysis, Substanzen > generisches Konzept, um Plugin aufzurufen 
 * @param $_REQUEST['t']  tablename
 * @param $_REQUEST['id'] id of object
 * @version $Header: trunk/src/plugin/g.QC_analysis2.inc 59 2018-11-21 09:04:09Z $
 */
require_once 'glob.obj.conabs.inc';
require_once 'o.S_VARIO.subs.inc';

class g_QC_analysis2_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= $_REQUEST['t'];
		$title		= 'Start QC-Analysis-Plugin';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		#$this->infoarr['help_url'] = '';
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
		$table  = $_REQUEST['t'];
		$sqlo   = &$this->_sqlo;
		
		// check tables
		$allowArr = array('CONCRETE_SUBST', 'WORKLIST');
		if ( !in_array($_REQUEST['t'],$allowArr) ) {
			$this->htmlFoot('ERROR', 'This table "'.$_REQUEST['t'].'" is not supported for this tool. Contact the admin.');
		}
		
		$conAbsLib = new gConcAbsC();
		$absTable  = $conAbsLib->getTemplate($table);
		
		
		$pkname     = PrimNameGet2($table);
		$absTablePK = PrimNameGet2($absTable);
		
		$absID = glob_elementDataGet( $sqlo, $table, $pkname, $this->objid, $absTablePK );
		
		if (!$absID) {
			$this->htmlFoot('ERROR', 'abstract parent object missing on this object.');
		}
		
		$key='DataAnalysisPlugin';
		$varioLib = new oS_VARIO_sub($absTable);
		$pluginName = $varioLib->getValByKey( $sqlo, $absID, $key );
		
		
		if ($pluginName==NULL) {
			$this->_html->messageBox('No plugin defined for this substance', '','INFO', 'no plugin defined for the related '.
			tablename_nice2($absTable) .' ID:'.$absID.'. '.
				' Ask your admin to set a plugin. He can set plugin in:  '.tablename_nice2($absTable).':VARIO:'.$key.'; '.
				' Find more information in the developper docu.');
			$this->htmlFoot();
		}
		
		$needle = '{ID}';
		if ( strstr($pluginName, $needle)==NULL ) {
			$this->htmlFoot('ERROR', tablename_nice2($absTable) .' ID:'.$absID.': Plugin-string incorrect; {ID}-pattern missing!');
		}
		
		$newPlugin = str_replace("{ID}", $this->objid, $pluginName);
		
		$url='p.php?mod='.$newPlugin;
		$this->_html->pageForward($url,'QC-plugin'); 
		
	}
	
}
