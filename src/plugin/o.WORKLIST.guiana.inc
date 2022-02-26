<?php
/**
 * - analysis plugin table WORKLIST
 * - [WorklistAnalysis] 
 *    
 * @package o.WORKLIST.guiana.inc
 * @swreq UREQ:0001789: o.WORKLIST > [WorklistAnalysis] extract objects and optional start analysis-plugin 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $_REQUEST['id'] id of CYTCART
 * @param $_REQUEST['act'] 
 *   'select'   - select objects, need $parx[colx]
 *   'analysis' - FUTURE PROJECT: select the column by abstract_worklist: VARIO: WorklistAnalysis= COL-ID:PLUGIN-NAME,COL-ID,PLUGIN-NAME
 * @param $parx['colx']  column id (if action="select")

 */

require_once ("o.WORKLIST.subs.inc");
require_once ("o.ABS_WORKLIST.subs2.inc");
require_once ('access_lock.inc');
require_once ('f.sql_query.inc');

/**
 * @author steffen
 *
 */
class o_WORKLIST_guiana_XPL extends gPlugin {
	
	var $absEntries; // tablenames
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'WORKLIST';
		$title		= '[WorklistAnalysis]';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
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
	 * set collect command
	 * @param $sqlo
	 * @param $colx
	 * @return $backurl
	 */
	function _act_SELECT(&$sqlo, $colx) {
		global $error;
		$FUNCNAME= __CLASS__.':_act_SELECT';
		
		$thisAbsInfo = $this->absEntries[$colx];
		
		if ( $thisAbsInfo['typ']!='OBJ' or $thisAbsInfo['t']==NULL ) {
			$error->set( $FUNCNAME, 1, 'column not defined  as object.' );
			return;
		}
		$tablename=$thisAbsInfo['t'];
		$tabnice = tablename_nice2($tablename);
		
		$sqlSelAfter = 'from WORKLIST_ENTRY where  WORKLIST_ID='.$this->objid.
			' and XPOS='.$colx. ' and OBJID>0';
		$sqlsel = 'count (1) '.$sqlSelAfter;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$objcnt = $sqlo->RowData[0];
		
		if (!$objcnt) {
			$error->set( $FUNCNAME, 2, 'Column "'.$tabnice.'" contains no objects.' );
			return;
		}
		
		$pkname     = PrimNameGet2($tablename); 
		$sqlCondNew = $pkname.' in (select OBJID '.$sqlSelAfter.')';
		$wliName    = obj_nice_name ( $sqlo, 'WORKLIST', $this->objid ); 
		
		$utilLib    = new fSqlQueryC($tablename);
	 	$utilLib->cleanCond();
		$utilLib->addCond( $sqlCondNew, '', 'WLI-ID:'.$this->objid.' Name:'. $wliName ); 
		$utilLib->queryRelase();
		
		$backurl = 'view.tmpl.php?t='.$tablename; //'&condclean=1&tableSCond='.urlencode($sqlCondNew);
		return $backurl;
	}
	
	


	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$this->objid   = $_REQUEST['id'];
		$sqlo = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		$parx   = $_REQUEST['parx'];
		$action = $_REQUEST['act'];
		
		if ($action==NULL) {
			$this->htmlFoot('ERROR','Parameter "action" missing');
		}
		
		$this->_html-> line_out('Action: '.$action);
		
		$this->workAnaLib = new oWORKLIST_subs($this->objid);
		$initAnswer       = $this->workAnaLib->init($sqlo);
		$this->absWorkId  = $initAnswer['absWorklistId'];
		$this->maxy = $initAnswer['maxy'];
		
		$this->absEntries = $this->workAnaLib->getPlannedAttribs($sqlo);
		
		
		switch ($action) {
			case 'select':
				if ($parx['colx']<=0) {
					$this->htmlFoot('ERROR','Parameter "colx" missing.');
				}
				$forwardUrl = $this->_act_SELECT($sqlo, $parx['colx']);
				if ($error->got(READONLY))  {
					return;
				}
				js__location_replace($forwardUrl, 'list view with selection' );  
				break;
			/*
			case 'analysis':
				$forwardUrl = $this->_act_ANALYSIS($sqlo);
				if ($error->got(READONLY))  {
					return;
				}
				js__location_replace($forwardUrl, 'list view with selection' );  
				break;
			*/
		}
		
		if ($error->got(READONLY))  {
			return;
		}
		
		$url='edit.tmpl.php?t=WORKLIST&id='.$this->objid;
		js__location_replace($url, "object" );
	}
	
}
