<?php 
require_once 'gui/f.pageEntriesOrg.inc';
require_once 'visufuncs.inc';

/**
 * php-bean to show a report
 * @package f.reportShow.inc
 * @swreq SREQ:0002248: g.Reporting : php-bean to show a report 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @unittest exists
 * @version $Header: trunk/src/phplib/gui/f.reportShow.inc 59 2018-11-21 09:04:09Z $
 * @example
 * 
 * $reportLib = new fReportShow_X();
 * $reportLib->init($infoarr);
 * $reportLib->show($sqlo, $sqlo2);
 */
class fReportShow_abs {
	
	
	/**
	 *  ABSTRACT method
	 *  @return array() for output
	 */
	function getOneRow(&$sqlo, $rowData) {}
	
	/**
	 * 
	 * @param $infoLib must contain method getOneRow()
	 * @param $infoarr
	 *   'title'   => table title
	 *   'entries' => number [required]
	 *   'entryPerPage' => number [optional]
	 *   'sqlCmd'  => e.g. "x.CONCRETE_SUBST_ID, x.CCT_ACCESS_ID ". $sqlBase.$orderstr";
	 *   'toolUrl' => e.g. 'p.php?mod='.$this->_mod ; will be extended by '&page=' ...
	 *   'tableHeader' => header array ; e.g. array (
	 *      'SUC-ID',
			'substance (conc.)',
			'status (now)',
			'status (problem)',)
	 */
	function init( &$infoarr) {
		$this->_page   = $_REQUEST['page'];
		$this->_parx   = NULL;
		$this->_infoarr = $infoarr;
		
		if ( $infoarr['entryPerPage']>0 ) $this->_parx['entryPerPage'] = $infoarr['entryPerPage'];
	}
	
	function show(&$sqlo, &$sqlo2) {
		
		$parx = $this->_parx;
		$tableHeader = $this->_infoarr['tableHeader'];
		
		$showinf=array(
			'startShow'    => $this->_page,
			'foundEntries' => $this->_infoarr['entries']
		);
		$pageNavLib = new f_pageEntriesGui();
	    $info = $pageNavLib->setParams( $showinf, $parx );
	    $cnt = 0;
		$moreExists = 0;
		$startShow = $info['start'];
		$endShow   = $info['end'];
		
		$tabobj  = new visufuncs();
		$headOpt = array( "title" => $this->_infoarr['title'] );
		$tabobj->table_head($tableHeader, $headOpt);
		
		$sqlsel = $this->_infoarr['sqlCmd'];
		$sqlo2->Quesel ( $sqlsel );
		
		while ( $sqlo2->ReadRow() ) {
			$showit = 0;
			
			$rowData = $sqlo2->RowData;
			if ( $cnt >= $startShow ) $showit=1;
			if ( $cnt > $endShow )   {
				$moreExists = 1;
				break;
			}
			
			if ($showit) {
				$outarr = $this->getOneRow($sqlo, $rowData);
				$tabobj->table_row ($outarr);
			}
			$cnt++;
		}
		$tabobj->table_close();
		
		$pageNavLib->setLastShownEntry($cnt-1);
		$desturl= $this->_infoarr['toolUrl'].'&page=';
		$pageNavLib->showEntryNav ( $desturl, $page );
	}
}
 