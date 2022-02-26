<?php
require_once 'gui/f.htmlGraph2.inc';

/**
 * analyse SQL_GLOBLOG (e.g. parti.1.sqllog.txt)
 * format:
 * 	 0: time:  2013-01-03 04:40:21     
 *   1: user:   User: 1376      
 *   2: script: Script: /pionir/view.tmpl.php     
 *   3: procnum:  48      
 *   4: procnum2: 64            
 *   5: query:   select x.EXP_ID,x.NAME,a.CREA_DATE,a.MOD_DATE,a.DB_USER ...
 *   6: duration: Time: 5294 ms
 
 *    
 * @package g.analyse_sqllog.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   UREQ:0004148 g > admin > Analyse SQL_GLOBLOG
 * @param   $parx:
 * 		mintime : show events of min-time X
 *  	dayOnly : string show only of this day; "2013-04-04"
 *  	userOnly: user only
 * 		action
 *   		['details'] : day details
 *   		'day'       : day statistics
 * 		filex  : the log-file
 * @version $Header: trunk/src/plugin/root/g.analyse_sqllog.inc 59 2018-11-21 09:04:09Z $  
 */
class g_analyse_sqllog_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$title		= 'Analyse SQL_GLOBLOG';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title_sh'] = $title;
		$this->infoarr['title']    = $title.' UREQ:0004148';
		$this->infoarr['form_type']= 'tool'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['inforow']   = 'Analyse SQL-log-file SQL_GLOBLOG. It contains very time-consuming SQL-commands.';
		
		$this->infoarr['locrow']= array (
			array ('home.php', 'home'),
			array ('rootsubs/rootFuncs.php', 'Administration' )
			);
		
		$this->infoarr2 = array( 
			'onlyAdmin'=>1
		);
				
		
	}
	
	/**
	 * init some variables
	 */
	function init2() {
		// use standard file
		$sqllog = &SQL_log::getHandle();
		$this->standardLogfile = $sqllog->getGlobLogFile();
	}

	
	function form0() {
		$parx = $this->parx;
		$initarr["title"]       = "Method selection";
		$initarr["submittitle"] = "Submit";
		$initarr["tabwidth"]    = "AUTO";
		$initarr["ENCTYPE"]     = "multipart/form-data";
		
		
		$hiddenarr = NULL;
		$formobj = $this->_html->formInit($initarr, $hiddenarr);
		
		/*
		$fieldx = array (
				"namex"=>TRUE,
				"title" => "Logfile",
				"name"  => "filex",
				"object"=> "file",
				"val"   => $parx["filex"],
				'inits'=> array('details'=>'Details', 'day'=>'Tagesstatistik'),
				"notes" => "Optional, if not given, use standard LOG-file: ".$this->standardLogfile
		);
		$formobj->fieldOut( $fieldx );
		*/
		
		$fieldx = array ( 
			"title" => "Aktion", 
			"name"  => "action",
			"object"=> "select",
			"val"   => $parx["action"], 
			'inits'=> array('details'=>'Details', 'day'=>'Tagesstatistik'),
			"notes" => ""
			 );
		$formobj->fieldOut( $fieldx );
		
		$fieldx = array (
				"title" => "Zeige MinTime",
				"name"  => "mintime",
				"object"=> "text",
				"val"   => $parx["mintime"],
				"notes" => "[ms] Zeige nur Logs mit Duration>=MinTime"
		);
		$formobj->fieldOut( $fieldx );
		
		$fieldx = array (
				"title" => "Zeige nur Tag X",
				"name"  => "dayOnly",
				"object"=> "text",
				"val"   => $parx["dayOnly"],
				"notes" => "Zeige nur diesen Tag; z.B. 2013-04-12"
		);
		$formobj->fieldOut( $fieldx );
		
		$fieldx = array (
				"title" => "Zeige nur User-ID",
				"name"  => "userOnly",
				"object"=> "text",
				"val"   => $parx["userOnly"],
				"notes" => "Zeige nur diesen User; z.B. 1205"
		);
		$formobj->fieldOut( $fieldx );
		
		$fieldx = array (
				"title" => "FullSqlText",
				"name"  => "fullsql",
				"object"=> "checkbox",
				"val"   => 1,
				"notes" => "Zeige vollen String ?"
		);
		$formobj->fieldOut( $fieldx );
		
		
		$formobj->close( TRUE );
	}
	
	
	/**
	 * decide, whether use stzandard log file or uploaded file
	 * @param  -
	 * @throws Exception
	 * @return $filename
	 */
	function getFile() {
		
		/*
		if ( is_array($_FILES['filex']) and $_FILES['filex']['tmp_name']!=NULL) {
			
			$fileName = $_FILES['filex']['tmp_name'];
			
		} else {
			// use standard file
			$fileName = $this->standardLogfile;
			
		}
		*/
		
		$fileName = $this->standardLogfile;
		
		if (!file_exists($fileName)) {
			throw new Exception('File "'.$fileName.'" nicht gefunden.');
		}
		
		return $fileName;
	}
	
	private function file_line_count($fileName) {

		$linecount = 0;
		$handle = fopen($fileName, "r");
		
		if ($handle==NULL) return;
		
		$MAX=20000;
		while(!feof($handle)){
			$line = fgets($handle);
			$linecount++;
		}
		
		fclose($handle);
		return $linecount;
	}
	
	/**
	 * @param array $outarr of col-Width
	 */
	function _tableColDef($outarr) {
		echo "<colgroup>";
	
	
		foreach( $outarr as $dummy=>$width) {
			echo '<col width="'.$width.'" />';
		}
		echo "</colgroup>\n";
	}
	
	/**
	 * $option = array 'type'=>'inforow'
	 */
	function rowOut($outarr, $option=NULL) {
		reset ($outarr);
	
		$tropt=NULL;
		if ($option['type']=='inforow') {
			$tropt=' style="background-color:#E0E0FF";';
		}
		echo "<tr ".$tropt.">";
		foreach( $outarr as $key=>$val) {
			echo "<td>".$val."</td>";
		}
		echo "</tr>\n";
	
	}
	
	
	function analyseFile($tmpFileName) {
	
		$parx = $this->parx;
		$mintimeDEF = $parx['mintime'];
		$dayOnlyDEF = $parx['dayOnly'];
		$userOnlyDEF = $parx['userOnly'];
	
		$LINE_LENGTH = 32000;
		$FH = fopen($tmpFileName, 'r');
		if ( !$FH ) {
			echo "<B>Error:</B> Can't open file '$tmpFileName'<br>\n";
			return;
		}
	
		$imgMaxVal = 100;
		$graphLib = new fHtmlGraph2C($imgMaxVal, 300);
	
		$cssAdStr ==NULL;
		if (!$parx['fullsql']) {
			$cssAdStr ='white-space: nowrap;';
		}
	
		echo '<table width=100% style="font-size:0.8em; '.$cssAdStr.'">'."\n";
	
		$colarr =array('30','100', '60', '300', '300', '50');
	
		$this->_tableColDef($colarr);

		$outarr=array('#','Time', 'Usr', 'Script', 'SQL', 'Duration [ms]', 'graph');
		$this->rowOut($outarr);

		$lastDay=NULL;
		$i=1;
		while( !feof ( $FH ) ) {
	        $line = fgets($FH, $LINE_LENGTH);
	        $valArr = explode("\t",$line);
									 
	        $timeTag = $valArr[0];
	        $var1 = $valArr[1]; // text
	        $usrArr =  explode(" ",$var1);
	        $usr    = $usrArr[1];
	        
	        // SQL-text
	        $var5 = $valArr[6]; // text
	        if (!$parx['fullsql']) {
	        	$var5 = substr($var5,0, 60).'...';
	        }
	        $var6 = $valArr[7];
	        $timeArr = explode(" ",$var6);
	        $timex = $timeArr[1];
	        $day   = substr($timeTag,0,10);
	         
	        $showRow=1;
	        if ($mintimeDEF>0) {
		        $showRow=0;
		        if ($timex>$mintimeDEF) $showRow=1;
	        }
	         
	        if ($dayOnlyDEF!=NULL) {
		        $showRow=0;
		        if ($dayOnlyDEF==$day) {
		        	$showRow=1; // found.
		        }
	        }
	        if ($userOnlyDEF!=NULL) {
	        	$showRow=0;
	        	if ($userOnlyDEF==$usr) {
	        		$showRow=1; // found.
	        	}
	        }
	         
	        if ($timeTag!=NULL) {

		        if ($day!=$lastDay and $dayOnlyDEF==NULL) {
		        	// show this DAY-row, if day-option is inactive
		        	$outarr=array('',$day,'','','');
		        	$rowOpt=array('type'=>'inforow');
		        	$this->rowOut($outarr, $rowOpt);
		        }
		        $lastDay=$day;
	        }
	         
	        if ($showRow) {
		        if ($timex>0) {

		        	$logTime = log($timex/1000.0, 1.2);
		        	$color=NULL;
		        	if ($logTime>$imgMaxVal) {
		        		$logTime = $imgMaxVal;
		        		$color='orange';
		        	}
		        	$imgStr  = $graphLib->getImgHtml($logTime, $color);
	        	} else $imgStr  = NULL;

	        	$outarr = array( ($i+1), $timeTag, $usr, $valArr[2], $var5, $timex, $imgStr);
	       		$this->rowOut($outarr);
	        }
	        $i++;
		        
		}
		fclose ( $FH);
		echo '</table>';
		echo 'INFO: '.$i.' lines analysed.<br>';
	}
	
	private function _oneRowDay($sqlPerDay, $daycnt, $userarr, $lastDay) {
		$color = NULL;
		
		$showImgVal=$sqlPerDay;
		if ($showImgVal>100) {
			$showImgVal=100;
			$color='orange';
		} 
		$usrstr=NULL;
		foreach($userarr as $id=>$sql_num) {
			$usrstr .= $id.':'.$sql_num.' ;';
		}
		
		$imgStr  = $this->graphLib->getImgHtml($showImgVal, $color);
		$outarr  = array(($daycnt+1), $lastDay, $sqlPerDay, $usrstr, '',$imgStr);
		$this->rowOut($outarr);
	}
	
	/**
	 * analyse, how many big queries occured per day
	 */
	function analyseFileDay($tmpFileName) {
	
		$parx = $this->parx;
		$mintimeDEF = $parx['mintime'];
	
		$LINE_LENGTH = 32000;
		$FH = fopen($tmpFileName, 'r');
		if ( !$FH ) {
			echo "<B>Error:</B> Can't open file '$tmpFileName'<br>\n";
			return;
		}
	
		$this->graphLib = new fHtmlGraph2C(100, 300);
	
		$cssAdStr ==NULL;
		if (!$parx['fullsql']) {
				$cssAdStr ='white-space: nowrap;';
		}
	
		echo '<table width=100% style="font-size:0.8em; '.$cssAdStr.'">'."\n";

		$colarr =array('20','40', '40', '40','20', '400');

		$this->_tableColDef($colarr);

		$outarr=array('#','day', 'Num queries', 'Users', 'Duration [ms]', 'graph');
		$this->rowOut($outarr);

		$lastDay=NULL;
		$i=1;
		$daycnt = 0;
		$sqlPerDay = 0;
		$userarr=array();

		while( !feof ( $FH ) ) {
			$line = fgets($FH, $LINE_LENGTH);
			$valArr = explode("\t",$line);
				
			$timeTag = $valArr[0];
			$var1 	 = $valArr[1]; // text
			$usrArr  =  explode(" ",$var1);
			$user     = $usrArr[1];
			 
			$var6 = $valArr[7];
			$timeArr = explode(" ",$var6);
			$timex = $timeArr[1];
			 
			$showRow=0;
	
			$userarr[$user]++;
			 
			if ($timeTag!=NULL) {
				$day = substr($timeTag,0,10);
				 
				if ($day!=$lastDay) {
					$this->_oneRowDay($sqlPerDay, $daycnt, $userarr, $lastDay);
					$daycnt=$daycnt+1;
					$sqlPerDay = 0;
					$userarr=array();
				}
				$lastDay=$day;
			 
			}
	
			$sqlPerDay++;
					 
	
			$i++;
		}
		$this->_oneRowDay($sqlPerDay, $daycnt, $userarr, $lastDay);
		
		fclose ( $FH);
		echo '</table>'."\n";
	}
	
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
		
		
		$this->parx = $parx;
		
		$this->init2();
		
		$tmpFileName = $this->getFile();
		if ($tmpFileName==NULL) {
			$this->htmlFoot('ERROR', 'No log file');
		}
		$xcnt = $this->file_line_count($tmpFileName);
		$this->_html->line_out( '<b>File-Name:</b> '.$tmpFileName.' Lines:'.$xcnt );
		
		$this->form0();
		$this->_html->line_out( '' );
		
		if ( !$go ) {
			return;
		}
		
		
		try {
		
			
			
			$mintime = $parx['mintime'];
			if ( $mintime!=NULL )  {
				if (!is_numeric($mintime)) {
					throw new Exception('Param MinTime ist falsch.');
				}
			}
		
			
			if ($parx['action']=='day') {
				$this->analyseFileDay($tmpFileName);
			} else {
				$this->analyseFile($tmpFileName);
			}
		
		} catch (Exception $e) {
			echo '<b>Exception</b> abgefangen: ',  $e->getMessage(), "\n";
			return;
		}
		
		
		//
		

		
	}
	
}
