<?php

require_once ("f.objview.inc");
require_once 'gui/f.htmlGraph2.inc';

/**
 *load analysis
 *    
 * @package g.load.ana.inc
 * @swreq UREQ:0003895 load analysis
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $parx['lines'] last x lines

 *   
 */

class SessAnaCls {
	
	/**
	 * 
	 * @param unknown $varname
	 * @param string $type: 
	 * 	sess : super session var
	 *  s string
	 *  i integer
	 * @param unknown $resultarr
	 */
	private function getVariable($varname, $type, &$resultarr) {
		
		$sessContent = &$this->sessContent;
		$val=NULL;
		$found=0;
		$userPos1 = strpos($sessContent,$varname);
		$varnameLen=strlen($varname);
		
		if ($userPos1) {
			
			if ($type=='sess') {
				$userPosx = $userPos1+$varnameLen+1; // VARNAME|s:4:"hallo"
			} else {
				
				$userPosx = $userPos1+$varnameLen+2; // "VARNAME";s:4:"hallo"
			}
			
			$type =  substr($sessContent,$userPosx,1);
			$userPos2 = $userPosx;
			$valun = NULL;
			
			
			if ($type=='s') {
				$userPos2 = strpos($sessContent,'"',$userPos2+1);
				$userPos3 = strpos($sessContent,'"',$userPos2+1);
				$varlen=$userPos3-$userPos2-1;
				$val = substr($sessContent,$userPos2+1,$varlen);
			}
			if ($type=='i') {
				$userPos2 = $userPos2 + 1;
				$userPos3 = strpos($sessContent,';',$userPos2+1);
				$vallen = $userPos3-$userPos2-1;
				$val = substr($sessContent,$userPos2+1,$vallen);
			}
			if ($type=='a') {
				
				$userPos3 = strpos($sessContent,'}',$userPos2+1);
				$userPos3 = $userPos3+1;
				$vallen   = $userPos3-$userPos2;
				$valun = substr($sessContent,$userPos2,$vallen);
				$val = unserialize($valun);
			}
			$found=1;
			
			/*
			echo "DEBBB: $varname:$type startVal:".substr($sessContent,$userPosx,20)."| VALRAW:".$valun."| VAL:";
			var_dump($val);
			echo "<br>";
			*/
		}
		
		// echo "DDDD: $varname:$found<br>";
		
		if ($found) {
			$resultarr[$varname] = $val;
		}
		
	}
	public function analyseSessfile($filefull) {
		
		$resultarr=array();
		
		
		$this->sessContent =  file_get_contents ( $filefull );
		
		/*
		$varname='s_sessVars';
		$varlen=strlen($varname);
		$userPos1 = strpos($this->sessContent,$varname);
		if ( $userPos1!==false ) {
			$strpos2=$userPos1+$varlen+1;
			$tmpSessval = substr($this->sessContent,$strpos2);
			$tmpSessval = substr($tmpSessval,0,strlen($tmpSessval)-2);
			echo " DDDD:".htmlentities($tmpSessval)."|XXX\n<br>";
			$test = unserialize($tmpSessval);
			echo " OHADEBBB:_".print_r($test);
			echo "LINUX: ".$test['linuxProcID'];
		}
		*/
		
		
		
		$this->getVariable('cctuser'    , 'sess',$resultarr);
		$this->getVariable('linuxProcID',  'i', $resultarr);
		$this->getVariable('REMOTE_ADDR',  's', $resultarr);
		$this->getVariable('g.modulesUse', 'a', $resultarr);
		
		$this->getVariable('g.moduleNowT',  's', $resultarr);
		$this->getVariable('g.moduleNowP',  's', $resultarr);
		$this->getVariable('g.moduleNowF',  's', $resultarr);
		
		if ($resultarr['g.moduleNowP']) {
			$pid=$resultarr['g.moduleNowP'];
			$cpuuse = trim(`ps --pid $pid --no-headers -opcpu`);  // TBD: only for Linux
			$resultarr['cpuuse']=$cpuuse;
		}
		
		
		return $resultarr;
	}
}

class g_load_ana_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		
		$title		= 'Load analysis';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title_sh']    = $title;
		$this->infoarr['title']    = 'Load analysis: Analysis of session-load on app-server';
		
		$this->infoarr['form_type']= 'tool'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['locrow']   = array( array('rootsubs/rootFuncs.php', 'home') );
				
		$this->conf=array();
		$this->conf['sessdir'] = session_save_path();
		$this->conf['sessPrefix'] = 'sess_';
		
	}
	
	private function getSessionFIles() {
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$dirbase = $this->conf['sessdir'];
		$sessPrefix = $this->conf['sessPrefix'];
		$prefixlen = strlen($sessPrefix);
		
		if ( ($dh = opendir ( $dirbase )) === false ) {
			throw new Exception($FUNCNAME.': Cant read dir '.$dirbase.'.');
		}
		
		$filearr = array();
		
		$extlen = strlen($file_ext);
		while (false !== ($file = readdir ($dh))) {
			$takeit = 0;
			$flen   = strlen ($file);
			 
			if ($file=="." OR $file=="..") continue;
				
			$filePrefix = substr ($file, 0, $prefixlen);
			if ($filePrefix==$sessPrefix) $takeit = 1;
				
			if ($takeit) $filearr[]=$file;
			$filecnt++;
		}
		
		if ( sizeof($filearr) ) {
			sort ($filearr);
			reset ($filearr);
		}
		closedir($dh);
		 
		return $filearr;
	}
	
	function showAnatable(&$analysisArr) {
		
	
		$this->_html->line_out('');
		$this->_html->line_out('Zeige nur lebende Prozesse an ...');
		$headx = array('#','session', 'linuxProcID',  'Action', 'access-date', 'CPU-use');
		$headOpt=array('title'=>'Linux Process Analysis');
		$this->_html->_table3_init($headx, $headOpt);
		
		
		reset ($analysisArr);
		$cnt=0;
		$oldProcCnt=0;
		foreach( $analysisArr as $procid=>$datarr) {
			
			$cpuuse = NULL;
			if ($procid)
				$cpuuse = trim(`ps --pid $procid --no-headers -opcpu`);
			
			if (!is_numeric($cpuuse)) {
				$oldProcCnt++;
				continue;
			}
			
			$dataArr= array( ($cnt+1), 
					'<a href="#'.$datarr['sess'].'">'.$datarr['sess'].'</a>', $procid, 
					$datarr['act'], $datarr['date'], $cpuuse);
			$this->_html->_table3_row($dataArr);
			$cnt++;
		}
		$this->_html->_table3_close();
		$this->_html->line_out('Anzahl toter Prozesse: '.$oldProcCnt);
		
	}
	
	private function showGraph($maxPix, $maxval, $value) {
		$graphlib = new fHtmlGraph2C($maxval, $maxPix);
		$imgtag = $graphlib->getImgHtml($value, '', 1);
		return $imgtag;
	}
	
	function analyseTOP() {
		$topstr = `top -b -n 1`;
		$toparr = explode("\n",$topstr);
		$info= array();
		
		if (!sizeof($toparr)) {
			return;
		}
		
		// top - 09:03:07 up 106 days, 18:38,  1 user,  load average: 0.34, 0.23, 0.19
		$firstLineLoad = explode('load average:',$toparr[0]);
		$loadvals      = explode(',',$firstLineLoad[1]);
		$loadStr  = trim($loadvals[0]);
		$graphTag = NULL;
		
		if ( $loadStr===NULL or $loadStr==='' ) {
			$info['load'] = '???';
		} else {
			$info['load'] = floatval($loadStr);
			$loadLimit = 40;
			$graphTag  = ' '.$this->showGraph(300, $loadLimit, $info['load']). ' (Load-Limit:' . $loadLimit.')';
		}
		$this->_html->rawHtmlCR( "<b>TOP-Info:</B> Load:".$info['load']. $graphTag);
		$this->_html->line_out('');
		
		
	}
	
	function analyse() {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$sessfiles = $this->getSessionFIles();
		if (!sizeof($sessfiles)) return;
		
		$this->_html->line_out('');
		$this->showLegende();
		$this->_html->line_out('');
		
		$sessionLib = new SessAnaCls();
		$headOpt=array();
		$headx = array('#','session', 'user', 'IP-address',  'Snapshot', 'Modules');
		$this->_html->_table3_init($headx, $headOpt);
		
		
		$analysisArr = array(); // array of process-ids
		
		reset ($sessfiles);
		$cnt=0;
		foreach( $sessfiles as $dummy=>$sessionname) {
			
			$filefull =  $this->conf['sessdir'] . DIRECTORY_SEPARATOR . $sessionname;
			
			if (!is_readable($filefull)) continue; // can occure, if file will be removed by automatism
			if (!filesize($filefull)) { // empty?
				continue;
			}
			
			$answer = $sessionLib->analyseSessfile($filefull);
			
			$procid = $answer['g.moduleNowP'];
			
			if ($procid!=NULL and $answer['g.moduleNowT']!=NULL) {
				//build analysis array
				$olddate  = $analysisArr[$procid]['date'];
				$proc_date = $answer['g.moduleNowT'];
				
				if ($proc_date>$olddate) {
					$analysisArr[$procid]=array('date'=>$proc_date, 'sess'=>$sessionname, 'act'=>$answer['g.moduleNowF']);
				}
				
			}
			
			$dataArr= array( 
					($cnt+1), 
					'<a name="'.$sessionname.'"></a>'.$sessionname, 
					$answer['cctuser'], $answer['REMOTE_ADDR'], 
					    'T: '.$answer['g.moduleNowT'].'<br />'.
					    ' F: '.$answer['g.moduleNowF'].
					    ' P: '.$answer['g.moduleNowP'],
						print_r($answer['g.modulesUse'],1)
					);
			$this->_html->_table3_row($dataArr);
			$cnt++;
		}
		$this->_html->_table3_close();
		
		$this->showAnatable($analysisArr);
		
	}
	
	private function showLegende() {
		
		$dataArr = array(
			array('session','ID of PHP-Session (found in session chache; no info, if the session is still used)'),
			array('user','User of session'),
			array('IP-address','IP-adress of client'), 
			array('Snapshot','last logged PHP-Script: with timestamp (T), method (F), process-ID (P); WARN: process-ID was only valid for this time-stamp!'),
			array('Modules','list of last called PHP-Scripts (max 20 stored)')
			);
		
		$headx = array('','');
		$headOpt=array("headNoShow"=>1, 'title'=>'legend');
		$this->_html->_table_out2($headx, $dataArr, $headOpt);
	}
	
	function _form() {
		
		$parx = $this->parx;
		$initarr   = NULL;
		$initarr["title"]       = "Select Params"; // title of form
		$initarr["submittitle"] = "Submit"; // title of submit button
		$initarr["tabwidth"]    = "AUTO";   // table-width: AUTO

		$hiddenarr = NULL; // hidden form variables
		$hiddenarr["id"]     = $this->objid; // is a required input parameter

		// init form, uses class formc
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);

		if (!$parx['lines']) $parx['lines']=10;
		$fieldx = array ( // form-field definition
			"title"   => "Show Lines", 
			"name"  => "lines",
			"object"=> "text",
			"val"    => $parx['lines'], 
			"notes" => "search in the last X lines of log-file"
			 );
		$formobj->fieldOut( $fieldx ); // output the form-field
		
		

		$formobj->close( TRUE ); // close form, sshow submit-button
	}
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$this->parx   = $_REQUEST['parx'];
		$go   = $_REQUEST['go'];
		$sqlo = &$this->_sqlo;
		
		echo "mypid: ".getmypid().' session-dir: '.$this->conf['sessdir']."<br>";
		
		$this->analyseTOP();
		
		// $test=array('hello'=>array(456,'agaggga'), 'yux'=>234);
		//echo "DEBUG_SER: ".serialize($test)."<br>";
		
		/*$this->_form($parx);
		
		if (!$go) {
			return;
		}
		*/
		
		$this->analyse();
		
	}
	
}
