<?php
/**
 * report for 'device log' (REA_LOG)
 * @package obj.rea_log.report.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param 
     go
     parx 
 */
session_start(); 

require_once ('reqnormal.inc');
require_once ('glob.objtab.page.inc');
require_once ("f.sql_query.inc");
require_once ('subs/obj.rea_log.report.inc');

class oREA_LOG_report {
	
function __construct($go, $parx) {
	$tablename='REA_LOG';
	$this->go = $go;
	$this->parx=$parx;
	
	$sqlQuerLib = new fSqlQueryC($tablename);
	$sqlopt["order"] = 1;
	$this->sqlAfter = $sqlQuerLib->get_sql_after($sqlopt); 
	
	$this->ReportLib = new oCHIP_READER_logSh();
	echo $this->ReportLib->getCss();
}

function show(&$sqlo, &$sqlo2) {
	$this->ReportLib->tableStart();
	
	$sqlsLoop = "SELECT x.* FROM ".$this->sqlAfter;
	$sqlo2->query($sqlsLoop);
	$cnt=0;
	$rowOpt = array('withDevice'=>1);
	
	while ( $sqlo2->ReadArray() ) {
		$datax = $sqlo2->RowData;
		
		$id = $datax["CHIP_READER_ID"];
		$name = obj_nice_name ( $sqlo, 'CHIP_READER', $id );
		$datax['readerName'] = $name;
		
	    $this->ReportLib->oneRow( $sqlo, $datax, $rowOpt );
	    $cnt++;
	}
	$this->ReportLib->tableEnd();
	
}

}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
$sqlo2 = logon2( );

if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$go 		= $_REQUEST['go'];
$parx 		= $_REQUEST['parx'];
$tablename	= 'REA_LOG';
$title		= 'Report for device log';

$infoarr			 = NULL;
$infoarr['scriptID'] = '';
$infoarr['title']    = $title;
$infoarr['design']   = 'norm';
$infoarr["obj_cnt"]  = 1;

$headopt = array( "obj_cnt"=>1 );
$mainObj = new gObjTabPage($sqlo, $tablename );
$mainObj->showHead($sqlo, $infoarr, $headopt);
$mainObj->initCheck($sqlo);

$MainLib = new oREA_LOG_report($go, $parx);
$MainLib->show($sqlo, $sqlo2);

htmlFoot('<hr>');
