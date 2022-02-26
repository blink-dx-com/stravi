<?php
require_once ('o.PROJ.subs.inc');
require_once ('o.CCT_ACCLOG.subs.inc');
require_once ("f.objview.inc");	
require_once ("visufuncs.inc");
require_once ("date_funcs.inc");
require_once ("o.CHIP_READER.logs.inc"); 

/**
 * show status of devices in a list
 * - column "OK-Status" : show only STOP or Working
 * - column "Log-Status" : show KEY, Notes of last Status of LAST action (if not Working)
 * @package o.CHIP_READER.statusView.inc
 * @swreq  SREQ:001094: o.CHIP_READER (device) > snapin to show a service log status 
 * @author Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/gui/o.CHIP_READER.statusView.inc 59 2018-11-21 09:04:09Z $
 */
class oCHIP_READER_statusView {
	
	function __construct() {
		$logSubLib = new oREA_LOG_subs();
		$this->devKeyColors = $logSubLib->getActColors();
	}
	
	function _getProjLink($projid) {
	    $projstr = '(in '.tablename_nice2('PROJ').' ID:<a href="edit.tmpl.php?t=PROJ&id='.$projid.'">'.$projid.'</a>)';
		return $projstr;
	}
	
	function _prodLastProblem($key, $name) {
	    $infoProblem=$name;
		if (strlen($name)>30) $infoProblem=substr($name,0,30).' ...';
		$lastProblem = htmlspecialchars($infoProblem) ;
		return $lastProblem;
	}
	
	/**
	 * get device link and room
	 * @param $sqlo
	 * @param $objid
	 */
	function _deviceInfo( &$sqlo, $objid ) {
		$sqlsel = "* from CHIP_READER where CHIP_READER_ID=". $objid;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadArray();
		$name    = $sqlo->RowData['NAME'];
		$loction = $sqlo->RowData['LOCATION'];
		$objlink = fObjViewC::bo_IDLink( 'CHIP_READER', $objid, $name);
		// $locarr  = explode('-',$loction);
		// not needed, because we have more halls: $loc_room = $locarr[2];
		$loc_room = $loction;
		return array($objlink,$loc_room);
	}
	
	function _getLastKey(&$sqlo, $objid, $key=NULL) {
		
		$subSql=NULL;
		if ($key!=NULL) $subSql = " and KEY='" . $key . "'";
		$sqlsel = "KEY, ".$sqlo->Sql2DateString('XDATE', 4 ) .", NAME from REA_LOG where CHIP_READER_ID=". $objid. 
			' and XDATE<= ' . $sqlo->Timestamp2Sql($this->dateNow) . $subSql . " order by XDATE DESC";
		$sqlo->Quesel($sqlsel);
		if (!$sqlo->ReadRow() ) return;
		return $sqlo->RowData;
	}
	
	
	
	/**
	 * - get last Status Info of device $objid
	 * - only logs from PAST, not from FUTURE !
	 * @param object $sqlo
	 * @param int   $objid device id
	 * @return array(
	 *  'OK.key' => last OK-Status: Working or STOP or unknown 
	 * 	'key'=>$keyOut,
	 *  'date'=>$datex, 
	 *  'prob'=>$lastProblem
	 *  );
	 */
	function _getLastStatus( &$sqlo, $objid) {
		
		$lookBack=0;
		$lastProblem = NULL;
		
		// get last working
		$answerArrST = $this->_getLastKey($sqlo, $objid, 'STOP');
		$answerArrWO = $this->_getLastKey($sqlo, $objid, 'Working');
		
		$keyx='unknown';
		
		if ($answerArrST[1]!=NULL) {
			$keyx='STOP';
		}
		if ($answerArrWO[1]!=NULL and $answerArrWO[1]>$answerArrST[1]) {
			$keyx='Working';
		}
		
		$lookBack=1;
		
		if ($this->devKeyColors[$keyx]!=NULL) {
			$keyOut = '<span style="color: '.$this->devKeyColors[$keyx].'">'.$keyx.'</span>';
		} else $keyOut = $keyx;
		
		$keyLast=NULL;
		$lastProblem = NULL;
		$datex = NULL;
		$datex = NULL;
		
		// next log-entry, look for problem before
		if ( $lookBack  ) { 
			$answerArr = $this->_getLastKey($sqlo, $objid); // get last entry
			// $keyx  = $sqlo->RowData[0];
			$datex    = $answerArr[1];
			$keyLast  = $answerArr[0];
			$lastName = $answerArr[2];
			$lastProblem = $this->_prodLastProblem($keyBefore, $lastName);
		 
		}
		
		
		return array('OK.key'=>$keyOut,'key'=>$keyLast, 'date'=>$datex, 'prob'=>$lastProblem, 'OK.key.raw'=>$keyx);
	}
	
	/**
	 * show current state of devices, collected in project $srcProjID
	 * @param int $srcProjID source project ID
	 */
	private function showLog_Start(&$sqlo, $sqlsAfter, $infox, $title_opt=NULL) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$table    ='CHIP_READER';
		$pkname   =PrimNameGet2($table);
		$tablenice=tablename_nice2($table) ;
		
		$datenow    = time();
		$dateLast24 = $datenow - date_unixHour2Sec(24);
		$this->dateNow = $datenow;
		$this->objLinkLib = new fObjViewC();
	
		$sqlsel = $pkname.' from '.$sqlsAfter;
		$sqlo->Quesel($sqlsel);
		$objarr=NULL;
		while ($sqlo->ReadRow()) {
			$retid = $sqlo->RowData[0];
			$objarr[]=$retid;
		}
		
		if ($objarr==NULL) {
		    echo 'No '.$tablenice.' found for condition: '.$infox['condition'].'<br>';
			return;
		}
		
		
		echo '<style type="text/css">'."\n";
		echo "table.ydataTab { }\n";
		echo "table.ydataTab td:nth-child(1)  { width:60px; }\n";
		echo "table.ydataTab td:nth-child(2)  { width:105px;  }\n";
		echo "table.ydataTab td:nth-child(3)  { width:160px;  }\n";
		echo "table.ydataTab td:nth-child(4)  { width:80px;  }\n";
		echo "table.ydataTab td:nth-child(5)  { width:80px;  }\n";
		echo "table.ydataTab td:nth-child(6)  { width:150px; }\n";
		echo "table.ydataTab td:nth-child(7)  { width:300px;  }\n";
		echo '</style>'."\n";
		
		$title  = 'list of '.$tablenice;
		if ($title_opt!=NULL) $title = $title_opt;
		$tabobj = new visufuncs();
		
		$title_out='<img src="images/icon.'.$table.'.gif"> '. $title .' </b>';
		if ($infox['srcProjID']=='PROJ') {
		    $title_out .= $this->_getProjLink($infox['srcProjID']) .
		      ' [<a href="p.php?mod=DEF/o.CHIP_READER.logSearch&projid='.$infox['srcProjID'].'">Suche im Log</a>]';
		}
		$headOpt = array( "title" => $title_out, 'cssclass'=>'ydataTab' );
			
	   
		
		
		$headx  = array ("ID", "Raum", "Name", "OK-Status", "Log-Status", "Log-Datum", "Letzter Eintrag");
		$tabobj->table_head($headx,   $headOpt);
	
		
		$last_room = NULL;
		$loopcnt=0;
		

		foreach( $objarr as $objid) {
			
			$statusArr = $this->_getLastStatus($sqlo,  $objid);
			list($objName, $location)   = $this->_deviceInfo($sqlo, $objid );
			
			// analyse date
			$dateEntry    = $statusArr['date'];
			$dateEntryUnx=0;
			if ($dateEntry) {
			     $dateEntryUnx = date_str2unix( $dateEntry, 5 );
			}
			if ( $dateEntry!=NULL and $dateEntryUnx>$dateLast24 ) {
				$dateFormatted = '<b>'.$dateEntry.'</b>';
			} else $dateFormatted=$dateEntry;
			
			$dataArr = array($objid, '', $objName, $statusArr['OK.key'], $statusArr['key'], $dateFormatted, $statusArr['prob'] );
			
			if($last_room!=$location) {
				$rowopt=NULL;
				$rowopt["bgcolor"]='#D0D0FF';
				$newLocarr= array(NULL, "R: ".$location, NULL, NULL, NULL, NULL, NULL );
				$tabobj->table_row ($newLocarr, $rowopt);
			} 
			$tabobj->table_row ($dataArr);
			
			$last_room = $location;
			$loopcnt++;
		}
		reset ($objarr); 
		$tabobj->table_close();
		echo "<br />\n";
	}
	
	
	/**
	 * show current state of devices, collected in project $srcProjID
	 * @param int $srcProjID source project ID
	 */
	function showLogList(&$sqlo, $srcProjID, $title_opt=NULL) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    // get devices, order by: LOCATION, NAME
	    $table    ='CHIP_READER';
	    $pkname   =PrimNameGet2($table);
	    $projSubLib = new cProjSubs();
	    $sqlafterPre  = $projSubLib->getTableSQL ( $srcProjID, 'CHIP_READER' );
	    $sqlsAfter    = $table." x where x.".$pkname." IN (".$sqlafterPre.") order by x.NAME";
	    
	    $infox=array();
	    $infox['condition'] = tablename_nice2('PROJ').' '.$srcProjID;
	    $infox['srcProjID'] = $srcProjID;
	    
	    $this->showLog_Start($sqlo, $sqlsAfter, $infox, $title_opt);
	}
	
	/**
	 * 
	 * @param object $sqlo
	 * @param array $filter
	 * @param string $title_opt
	 */
	function showLog_byFilter(&$sqlo, array $filter, $title_opt=NULL) {
	    
	    $table    ='CHIP_READER';
	    $infox=array();
	    $infox['condition'] = 'Filter';
	    
	    
	    $where_arr=array();
	    foreach($filter as $row) {
	        $where_arr[]=$row['col'].'='.$row['val'];
	    }
	    $where=implode(' and ',$where_arr);
	    
	    $sqlsAfter    = $table." x where ".$where." order by x.LOCATION, x.NAME";
	    
	    $this->showLog_Start($sqlo, $sqlsAfter, $infox, $title_opt);
	    
	}
	
	
	
}
