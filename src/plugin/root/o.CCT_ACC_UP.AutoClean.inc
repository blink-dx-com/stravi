<?php
require_once 'lev1/o.CCT_ACC_UP.AutoCleanMod.inc';

/**
 * AutoCleaner for CCT_ACC_UP 
 *    
 * @package o.CCT_ACC_UP.AutoClean.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   UREQ:0001088: o.CCT_ACC_UP > AutoCleaner 
 * @param   $_REQUEST['go'] 0,1
 * @version $Header: trunk/src/plugin/root/o.CCT_ACC_UP.AutoClean.inc 59 2018-11-21 09:04:09Z $
 *   
 */
class o_CCT_ACC_UP_AutoClean_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'CCT_ACC_UP';
		$title		= 'AutoCleaner, UREQ:0001088';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['help_base']= 'lab';
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = 'AutoCleaner';
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;

		
		$this->infoarr2 = array( 
			'accCheck'    => array('tab'=>array('read', 'delete'), 'obj'=>array() ),
			'onlyAdmin'   =>1,
			'progressBar' =>1
		);
				
		
	}
	
	function form0() {
		$initarr["title"]       = "AutoClean confirm";
		$initarr["submittitle"] = "Now!";
		$initarr["tabwidth"]    = "AUTO";
		$hiddenarr = NULL;
		$formobj = $this->_html->formInit($initarr, $hiddenarr);
		$formobj->close( TRUE );
	}
	
	function clean_data_types($sqlo, $sqlo2) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->_html->line_out( 'Drop user defined TYPES (table:USER_OBJECTS)' );
		
		$sqlsel = "OBJECT_NAME from USER_OBJECTS where OBJECT_TYPE='TYPE' and OBJECT_NAME like ".
			$sqlo->addQuotes('CCTBU%');
		$sqlo2->Quesel($sqlsel);
		$cnt=0;
		while ($sqlo2->ReadRow() ) {
			
			$typename = $sqlo2->RowData[0];
			
			$result = $sqlo->query("drop type ".$typename);
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 1, 'Error on drop type '.$typename );
				return;
			}
			$cnt++;
			
		}
		$this->_html->line_out( $cnt.' TYPES dropped.' );
	}
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$go   = $_REQUEST['go'];
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		$mainLib = new oCCT_ACC_UP_AutoCleanMod();
		$mainLib->init($sqlo);
		
		$tmpNum = $mainLib->getMAX_STORE_NUM();
		$this->_html->line_out( 'Info: Remove all older entries in CCT_ACC_UP, 
			where an object has more than '.$tmpNum. ' CCT_ACC_UP entries.' );
		
		$mainLib2 = new oCCT_ACC_UP_CleanModDate();
		$mainLib2->init($sqlo);
		$startdate = $mainLib2->get_startDateHum();
		$this->_html->line_out( 'Info: Remove all older entries in CCT_ACC_UP, 
			where an CONCRETE object has entries older than '.$startdate. '.' );
		$this->_html->line_out( 'This tool keeps entries with ARCHIVE-info: UPINFO like "%a:arch%"');
		$this->_html->line_out( 'Info: Drop user defined TYPES (table:USER_OBJECTS)' );
		
		if (!$go)  {
			$this->form0();
			return;
		}
		
		
		

		if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
		    echo "<B>INFO:</B> DEBUG-mode: DELETE deactivated!.<br>\n";
		    $mainLib->_doDelete =0;
		    $mainLib2->_doDelete=0;
		}
		
		$objCnt = $mainLib->cleanStart( $sqlo );
		
		$prgopt = array ('maxnum'=> $objCnt );
		$this->_html->progressInit( $prgopt );
		$loopcnt= 0;
		$cntSum = 0;
		while ( $mainLib->oneRow($sqlo, $sqlo2) ) {
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 1, 'error on loopcnt='.$loopcnt );
				return;
			}
			$oneRow = $mainLib->getRowInfo();
			if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
				glob_printr( $oneRow, ($loopcnt+1) . ". row:" );
			}
			$this->_html->progressAlive ( $loopcnt );
			
			$cntSum = $cntSum + $oneRow['cnt'];
			$loopcnt++;
			
		}
		$this->_html->progressAlive ( $loopcnt,1 );
		
		$this->_html->line_out( 'removed entries: '.$cntSum );
		
		///////////////
		//$this->_html->line_out( 'Second cleaning inactive. Because of PERFORMANCE problems' );
		//return;
		
		
		$objCnt   = $mainLib2->cleanStart( $sqlo );
		$info_lib = $mainLib2->getInfox();
		$this->_html->line_out( 'mod_date analysis: objects: '.$objCnt.
			' ;max(CCT_ACCESS_ID): '.$info_lib['CCT_ACCESS_max'] );
		
		$prgopt = array ('maxnum'=> $objCnt );
		$this->_html->_flushLib->setNewLimits( $prgopt );
		$loopcnt= 0;
		$cntSum = 0;
		while ( $mainLib2->oneRow($sqlo, $sqlo2) ) {
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 1, 'error on loopcnt='.$loopcnt );
				return;
			}
			$oneRow = $mainLib2->getRowInfo();
			if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
				echo ($loopcnt+1) . ". row: ". $oneRow['cnt']. ' acc:'.$oneRow['c'].
					' tab: '.$oneRow['t']."<br>\n";
			}
			$this->_html->progressAlive ( $loopcnt );
			
			$cntSum = $cntSum + $oneRow['cnt'];
			$loopcnt++;
			
			
		}
		$this->_html->progressAlive ( $loopcnt,1 );
		
		$this->_html->line_out( 'removed entries: '.$cntSum );
		
		$this->clean_data_types($sqlo, $sqlo2);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'error on cleaning TYPEs');
			return;
		}
		
		
	}
	
}