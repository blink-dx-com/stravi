<?php
/**
 * analyze application log
 * 
 * @package f.appLogAna.inc
 * @swreq SREQ:0001004 g > f.appLogAna.php application log analysis 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * $Header: trunk/src/www/pionir/rootsubs/f.appLogAna.inc 59 2018-11-21 09:04:09Z $
 */

/**
 * decorator for fappErrLog
 * @author steffen
 */
class fappLogBeauty {
	
	function __construct() {
		$this->xmlpars   = xml_parser_create();
		$this->cnt=0;
		//$this->parx=$parx;
		
	}
	
	
	
	/**
	 * beautify the pure info-string 
	 * 
	 * - transform '[/ERR]'
	 * @param $pureInfo
	 * @return $outstr
	 */
	function _anaInfoText($pureInfo) {
		$key='[/ERR]';
		
		$outstr = htmlEntities( $pureInfo );
		if ( strstr($pureInfo, $key)!=NULL ) {
			$outarr = explode($key,$outstr);
			$komma = '<br />- ';
			$outstr='';
			foreach( $outarr as $dummy=>$val) {
				$outstr .= $komma . $val;
				
			}
			reset ($outarr); 
		}
		return $outstr;
	}
	
	/**
	 * beautify info line
	 * @param $infoCol
	 * @return string
	 */
	function infoBeauty($infoCol) {
		
		if ( substr($infoCol,0,1) != '<' ) {
			return (htmlEntities($infoCol));
		}
		
		$xmlArr = NULL;
		$this->xmlpars   = xml_parser_create();
		$result = xml_parse_into_struct  ( $this->xmlpars, $infoCol, $xmlArr );
		if ($xmlArr == NULL) {
			// fallback ...
			//$errcode = xml_get_error_code ( $this->xmlpars );
			//$errstr  = xml_error_string($errcode);
			return (htmlEntities($infoCol));
		}
		
		$mainStruc = &$xmlArr;
		$attribArr = $xmlArr[0]['attributes'];
		$urlStr    = $attribArr['PHPFILE'];
		$logfunc   = $attribArr['FUNC'];
		$pureInfo  = $xmlArr[1]['value'];
		
		if ($logfunc==NULL or $pureInfo==NULL) {
			// fallback ...
			return (htmlEntities($infoCol));
		}
		
		// analyse $pureInfo
		$newInfo = $this->_anaInfoText($pureInfo);
		

		$rest   = '<font color=#808080>Log:</font>'.$logfunc.' <font color=#3030FF>URL:</font> <b>'.$urlStr.'</b>; '. 
			'<br /><font color=#808080>Info:</font>'.$newInfo;
			
		
		return ($rest);
	}
	
}

/**
 * application log
 * @author steffen
 *
 */
class fappErrLog extends fappLogAnaC {
	
	function __construct ($parx) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		//parent::__construct($parx);
		parent::__construct($parx);
		$this->fileName = 	$_SESSION['globals']['app.logfile'];
		if ($this->fileName==NULL) {
			$error->set( $FUNCNAME, 1, 'Not configured: $_SESSION[globals][app.logfile]' );
			return;
		}
		$this->tableTitle='application log';
		$this->headx  = array ( 'Line', 'Date', 'IP', 'user', 'infoKey', 'prio', 'Info' );
		$this->infoBeautyLib = new fappLogBeauty();
		
		$this->ignoreArr = NULL;
		if ($this->parx['ignore']!=NULL) {
			$this->ignoreArr = explode(',',$this->parx['ignore']);
		}
		$this->whiteKeyArr = NULL;
		if ($this->parx['white']!=NULL) {
			$this->whiteKeyArr = explode(',',$this->parx['white']);
		}
		
		$this->ignore_text_Arr = array();
		$this->parx['ignoretxt']=trim($this->parx['ignoretxt']);
		if ($this->parx['ignoretxt']!=NULL) {
		    $this->ignore_text_Arr = explode(',',$this->parx['ignoretxt']);
		}
		
	}
	
	public function getMoreFields($parx) {
	
		$fieldxArr=array();
		$fieldxArr[] = array ( 
			"title" => "Ignore keys", 
			"name"  => "ignore",
			"object"=> "text",
			"val"   => $parx["ignore"], 
			"notes" => "ignore entries with keys: e.g.: USERERROR,INFO,INTERROR"
			 );
		
		$fieldxArr[] = array (
				"title" => "Ignore text",
				"name"  => "ignoretxt",
				"object"=> "text",
				"val"   => $parx["ignoretxt"],
				"notes" => "ignore lines with this text; text pieces are separated by komma; e.g. 'hello,other error,next'"
		);
		
		$fieldxArr[] = array ( 
			"title" => "Only keys", 
			"name"  => "white",
			"object"=> "text",
			"val"   => $parx["white"], 
			"notes" => "show only entries with keys: e.g.: INTERROR"
			 );
		
		
		$fieldxArr[] = array ( 
			"title" => "Any text", 
			"name"  => "seaText",
			"object"=> "text",
			"val"   => $parx["seaText"], 
			"notes" => "search for any text in log"
			 );
		
		
		$fieldxArr[] = array ( 
			"title" => "min prio", 
			"name"  => "prio",
			"object"=> "text",
			"val"   => $parx["prio"], 
			"notes" => "e.g. 2; show info with min priority; 1:high, 5:low"
			 );
		return $fieldxArr;
		
	}
	
	/**
	 * - decide/show one line
	 * @param $lineArr
	 * @return int 0 : shown, 1: not shown
	 */
	function _rowout($lineno, $line) {
		
		$splitLimit=7;
		$lineArr = explode( "\t", $line, $splitLimit); 
		
		$ip       = $lineArr[1];
		$user     = $lineArr[3];
		$errkey   = $lineArr[4];
		$prio     = $lineArr[5];
		$restText = $lineArr[6];
		
		$exitFunc=0;
		if (is_array($this->whiteKeyArr)) {
			if ( !in_array($errkey, $this->whiteKeyArr) ) {
				return 1;
			}
		}
		
		// analyse search params
		if (is_array($this->ignoreArr)) {
			if (in_array($errkey, $this->ignoreArr)) {
				return 1;
			}
		}
		
		if ( !empty($this->ignore_text_Arr) ) {
		    foreach($this->ignore_text_Arr as $ignoretxt) {
		        if ( strstr($line, $ignoretxt) ) {    // ignore lines, containing $this->parx['ignorertxt']
    				return 1;
    			}
		    }
		}
		
		if ($this->parx['prio']>0) {
			if ($prio>$this->parx['prio']) { // not the rigth prio
				return 1;
			}
		}
		
		if ($this->parx['seaText']!=NULL) {
			if ( !stristr($restText, $this->parx['seaText']) ) return 1;
		}
		
		if ($this->last['ip']!=$ip or $this->last['user']!=$user) {
			$ipOut   = '<b>'.$ip.'</b>';
			$userOut = '<b>'.$user.'</b>';
		} else {
			$ipOut = $ip;
			$userOut = $user;
		}
		
		$infoColOut = $this->infoBeautyLib->infoBeauty($restText);
		
		$dataArr = array (
			$lineno,
			$lineArr[0],
			$ipOut,
			// $lineArr[2], DBID
			$userOut,
			$lineArr[4],
			$lineArr[5],
			$infoColOut
			);
		$this->tabobj->table_row ($dataArr);
		
		$this->last['ip'] = $ip;
		$this->last['user'] = $user;
		return 0;
	}
	
}

/**
 * show PHP-error-logs
 * @author steffen
 *
 */
class fphpErrLog extends fappLogAnaC {
	
	function __construct ($parx) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		parent::__construct($parx);
		
		$this->tableTitle='PHP error log';
		$this->fileName = 	$_SESSION['globals']["php.log.errors"]; //'/var/log/phplog.txt';
		if ($this->fileName==NULL) {
			$error->set( $FUNCNAME, 1, 'Not configured: $_SESSION[globals][php.log.errors]' );
			return;
		}
		
		$this->headx  = array ( 'Line', 'Date', 'Type', 'Info' );
		
		$this->storeLine  =NULL;
		$this->storeLineno=NULL;
	}
	
	function _oneLineOut($lineno, $line) {
		
		$datePos = strpos($line,']');
		$datelen = $datePos-1;
		$dateString = substr($line,1,$datelen);
		
		$lineArr=array();
		$lineArr[0] = $dateString;
		$warnPos = strpos($line,':', $datePos+1);
		$warnLen  = $warnPos-$datePos+1;
		$warnType = substr($line,$datePos+1,$warnLen);
		if (strstr($warnType,'error')!=NULL) $warnTypeOut='<font color=red>'.$warnType.'</font>';
		else $warnTypeOut=$warnType;
		
		$lineArr[1] = $warnTypeOut;
		
		$lineArr[2] = substr($line,$warnPos+1);
		
		$dataArr = array (
			$lineno,
			$lineArr[0],
			$lineArr[1],
			htmlentities($lineArr[2])
			);
		$this->tabobj->table_row ($dataArr);
	}
	
	/**
	 * - show one line
	 * @param array $lineArr
	 * @return NULL
	 */
	function _rowout($lineno, $line) {
		
		/**
         * FORMAT:
         * [YYYY-MM-DD HH:MM:SS] WARNTTYPE TEXT
		 */
		$lineArr = NULL;
		$datePos = strpos($line,']');
		$datelen = $datePos-1;
		$dateString = substr($line,1,$datelen);
		$dayArr = explode('-',$dateString);
		$dayNumber = $dayArr[0];
		
		/* - manage the problem, that the log of PHP-errors can store ONE error in more than ONE log-file-line
		 * - detect the DATE in the line, if no DATE, store this error-part and show later
		 */
		if (is_numeric($dayNumber)) {
			// YES: print last line
			if ($this->storeLine!=NULL) {
				$this->_oneLineOut($this->storeLineno, $this->storeLine);
				$this->storeLine  =NULL;
				$this->storeLineno=NULL;
			}
			$this->storeLineno =$lineno;
			$this->storeLine =$line;
			
		} else {
			if (!$this->storeLineno) $this->storeLineno=$lineno;
			if ($this->storeLine!=NULL) $this->storeLine .="\n";
			$this->storeLine .= $line;
		}
		
	}
	
	function finishLoop() {
		if ($this->storeLine!=NULL) {
			$this->_oneLineOut($this->storeLineno, $this->storeLine);
		}
	}
	
}





