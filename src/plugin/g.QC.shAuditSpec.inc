<?php
require_once 'gui/f.pageEntriesOrg.inc';
require_once ('o.CCT_ACCLOG.subs.inc');
require_once ("visufuncs.inc");
require_once ("f.objview.inc");
require_once ("o.DB_USER.subs.inc");


/**
 * [ShowSpecialRelease] show special releases  
 * @package g.QC.shAuditSpec.inc 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0002153: g > QC > show batches with special release 
 *   
 */
class g_QC_shAuditSpec_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		//$tablename	= 'CART_BATCH';
		$title		  = 'QC: [ShowSpecialRelease]';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'tool'; 
		$this->infoarr['design']   = 'norm';
		// $this->infoarr['obj_name'] = $tablename;
		$this -> infoarr ['locrow'] = array ( // defines the tool location - path
			array ('home.php', 'home')
			);
		
		$this->infoarr2 = array( 
				// 'accCheck' => array('tab'=>array('read'), 'obj'=>array() )
				);
		
	}
	
	function pluginInit2(&$sqlo) {
		$this->accLogLib = new oAccLogC();
		$this->userLib = new DB_userC();
		$this->objLinkLib = new fObjViewC();
	}
	
	/**
	 * show one row
	 *  '#',
		'SUC-ID',
		'substance (conc.)',
		'status (now)',
		'status (problem)',
		'date',
		'audit user',
		'notes',
		'problems'
	 * @param $sqlo
	 * @param $tabobj
	 * @param $thisData
	 */
	function oneEntryOut(&$sqlo, &$tabobj, $thisData, $cnt) {
		
		$sucid    = $thisData[0];
		$cctaccid = $thisData[1];
		$tablename='CONCRETE_SUBST';
		
		$suc_nice = obj_nice_name ( $sqlo, $tablename, $sucid ); 
		
		$this->accLogLib->setObject( $sqlo, $tablename, $sucid );
		$lastLogID = $this->accLogLib->getLastLog($sqlo);
		
		// search the LAST problem of this object (!)
		$sqlSel = '* from CCT_ACCLOG WHERE CCT_ACCESS_ID='.$cctaccid.
			" and (QM_VAL < 1 or PROBLEM is not NULL) order by POS DESC";
		
		$sqlo-> Quesel( $sqlSel );
		
		$problemTupel = NULL;
		if ( $sqlo-> ReadArray() ) {
			$problemTupel = $sqlo->RowData;
		}
		
		$dataArrPre=NULL;
		if (!is_array($problemTupel)) {
			// he, what is this?, expected an answer ...
		} else {
		    $dataArrPre['status'] = oH_ALOG_ACT_subs::statusID2Name($sqlo, $problemTupel['H_ALOG_ACT_ID']);
			$dataArrPre['date']   = $problemTupel['ACT_DATE'];
			$dataArrPre['user']   = $this->userLib->getFullname($sqlo, $problemTupel['DB_USER_ID']);
			$dataArrPre['notes']  = $problemTupel['NOTES'];
			$dataArrPre['prob']   = $problemTupel['PROBLEM'];
		}
		
		$suc_link = $this->objLinkLib->bo_IDLink($tablename, $sucid, $sucid);
		
		$dataArr = array(
			$cnt,
			$suc_link,
			$suc_nice,
		    oH_ALOG_ACT_subs::statusID2Name($sqlo, $lastLogID),
			$dataArrPre['status'] ,
			$dataArrPre['date'],
			$dataArrPre['user'],
			$dataArrPre['notes'],
			$dataArrPre['prob']
		);
		
		$tabobj->table_row ($dataArr);
	}
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$this->objid   = $_REQUEST['id'];
		$page   = $_REQUEST['page'];
		$parx   = NULL;
		
		$sqlo  = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2 ();
		
		$this->pluginInit2($sqlo);
		
		$sqlBase = 
		    "from CONCRETE_SUBST x where x.CCT_ACCESS_ID in (
				select CCT_ACCESS_ID from CCT_ACCESS x WHERE 
				    x.TABLE_NAME = 'CONCRETE_SUBST' and 
					x.CCT_ACCESS_ID IN (
						select x.CCT_ACCESS_ID from CCT_ACCLOG x WHERE (x.QM_VAL < 1 or x.PROBLEM is not NULL)
					)
					 
		    )";
		$orderstr= ' order by x.CONCRETE_SUBST_ID DESC';
		
		$sqlsel = 'count(1) '.$sqlBase;
		$sqlo-> Quesel( $sqlsel );
		$sqlo-> ReadRow();
		$entries = $sqlo->RowData [0];
		
		if (!$entries) {
			$this->htmlFoot('No entries found.');
		}
		
		
		$tableHeader = array(
			'#',
			'SUC-ID',
			'substance (conc.)',
			'status (now)',
			'status (problem)',
			'date',
			'audit user',
			'notes',
			'problems'
		);
		
		$showinf=array(
			'startShow'    => $page,
			'foundEntries' => $entries
		);
		$pageNavLib = new f_pageEntriesGui();
	    $info = $pageNavLib->setParams( $showinf, $parx );
	    $cnt = 0;
		$moreExists = 0;
		$startShow = $info['start'];
		$endShow   = $info['end'];
		
		$tabobj  = new visufuncs();
		$headOpt = array( "title" => "Critical Substances" );
		$tabobj->table_head($tableHeader, $headOpt);
		
		$sqlsel = "x.CONCRETE_SUBST_ID, x.CCT_ACCESS_ID ". $sqlBase.$orderstr;
		$sqlo2->Quesel ( $sqlsel );
		
		while ( $sqlo2->ReadRow() ) {
			$showit = 0;
			
			$thisData = $sqlo2->RowData;
			if ( $cnt >= $startShow ) $showit=1;
			if ( $cnt > $endShow )   {
				$moreExists = 1;
				break;
			}
			
			if ($showit) {
				$this->oneEntryOut($sqlo, $tabobj, $thisData, ($cnt+1));
			}
			$cnt++;
		}
		$tabobj->table_close();
		
		$pageNavLib->setLastShownEntry($cnt-1);
		$desturl= 'p.php?mod='.$this->_mod . '&page=';
		$pageNavLib->showEntryNav ( $desturl, $page );
	}
	
}