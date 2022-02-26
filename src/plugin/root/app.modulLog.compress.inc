<?php
require_once 'f.advmod_log.inc';
require_once ("f.objview.inc");

/**
 * g > app.modulLog.dat > compress
 * - read all lines of app.modulLog.dat till time stamp $parx['timePat'] 
 * - compress one year
 * - compress the data 
 * - write a new TEMP-File: ...
 * - write PART1 (old) to TEMP
 * - write PART_NEW (compressed) to TEMP
 * - write rest of app.modulLog.dat to that file
 * - copy  TEMP-File back to app.modulLog.dat
 *    
 * @package app.modulLog.compress.inc
 * @swreq   SREQ:4686 g > app.modulLog.dat > compress
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $parx['timePat'] compress till time pattern: e.g. 2013-01-29:12
 * 
 * @format
 * YYYY-MM-DDTHH:mm:ss user module IP NUM
 */

class app_modulLog_compress_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		
		$title		= 'Compress the ADMOD_LOG file';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		
		$this->infoarr['form_type']= 'tool'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['locrow']   = array( array('rootsubs/rootFuncs.php', 'home') );
				
		
	}
	
	/**
	 * append original lines at end of new TEMP-file
	 * @param unknown $fp
	 * @param unknown $ori_filename
	 * @param unknown $lineStart
	 * @param int $lineEnd : if -1: until END
	 */
	private function appendFile($fp, $ori_filename, $lineStart, $lineEnd) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		$LINE_LENGTH = 100000;
		
		$fp_ori = fopen($ori_filename, 'r');
		if (!$fp_ori) {
			$error->set( $FUNCNAME, 1, 'Cant open advmod_file for read: '.$ori_filename );
			return;
		}
		
		$linecnt=1;
		while( $linecnt<$lineStart and !feof ($fp_ori) ) {
			fgets($fp_ori, $LINE_LENGTH);
			$linecnt++;
		}
		
		// append content
		while( !feof ( $fp_ori ) ) {
			
			if ( ($lineEnd>=0) and ($linecnt>$lineEnd) ) {
				// until $lineEnd
				break;
			}
			$line     = trim( fgets($fp_ori, $LINE_LENGTH));
			if ($line==NULL) {
				$linecnt++;
				continue;
			}
			$retVal = fputs( $fp, $line."\n" );
			$linecnt++;
		}
		
		fclose($fp_ori);
		
	}
	
	function analyse() {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$sqlo = &$this->_sqlo;
		
		$parx = $this->parx;
	
		$analines = $parx['lines'];
		if ($analines < 10)  $analines = 5;
		

		
		
		$timePat=NULL;
		$patternLen = 0;
		if ($parx['timePat']!=NULL) {
			$timePat=$parx['timePat'];
			$patternLen = strlen($timePat);
		}
		

		$filename = $_SESSION["globals"]["app.log.dir"].'/app.modulLog.dat';
		
		if (!file_exists($filename)) {
			return; // file does not exist
		}
		
		$LINE_LENGTH = 100000;
	    $fp_ori = fopen($filename, 'r');
		if (!$fp_ori) {
			$error->set( $FUNCNAME, 1, 'Cant open advmod_file for read: '.$filename );
			return;
		}
		
		$sizex = filesize($filename);
		
		$cnt=1;
		while( !feof ( $fp_ori ) ) { 
			
	        $line = fgets($fp_ori, $LINE_LENGTH);
	        // echo "CNT:$cnt; $line<br>";
		    $cnt++;
		}
		rewind ($fp_ori);
		
		$this->_html->line_out('Lines of file: '.($cnt-1).' Size of file: '.$sizex. ' bytes');

		// $this->_html->line_out('Search: last lines: '.$analines);
		$this->_html->line_out('Date pattern: '.$timePat);
		

		
		$start_analyse = 0;
		$lineID=1;
		$doShow=0;
		$rowopt = array("trOpt"=>'valign=top');
		$cnt_compress_lines=0;
		$pos_info=array();
		
		/**
		 * array of modules
		 * (INDEX = module] = array('d'=> date, 'u'=>list of users, komma sep., 'n'=>number of occurence)
		 */
		$modCacheFirst = array();  // just for the first occurence
		$moduleCache = array();
		
		while( !feof ( $fp_ori ) ) { 
	        $line     = fgets($fp_ori, $LINE_LENGTH);
	        
	        if ( trim($line)==NULL ) {
	        	$lineID++;
	        	continue; // is empty
	        }
        	$linearr = explode("\t",$line);
        	
        	$date_loop = $linearr[0];
        	$user_loop = $linearr[1];
        	$module_loop  = trim($linearr[2]);
        	$occur    = $linearr[3];
        	$ipx      = trim($linearr[4]);
        	
        	
        	$date_loop_YMD = substr($date_loop,0,$patternLen);
        	
        	if ($date_loop_YMD > $timePat) {
        		$pos_info['end']=$lineID-1;
        		break; // end of analysis
        	}
        	if ($date_loop_YMD != $timePat) {
        		$lineID++;
        		continue;
        	} 
        	
        	do {
        	
        		if (!$cnt_compress_lines) {
        			$pos_info['start']=$lineID;
        		}
        		$cnt_compress_lines++;
	        	if ( isset($moduleCache[$module_loop]) ) {
	        		
	        		$moduleCache[$module_loop]['n'] = $moduleCache[$module_loop]['n']+1;
	        		if (strstr($moduleCache[$module_loop]['u'], $user_loop.',') ==NULL) {
	        			if (strlen($moduleCache[$module_loop]['u'])<60) { // do not append, if too many users added
	        				$moduleCache[$module_loop]['u'] .= $user_loop.','; // add USER and KOMMA
	        			}
	        		}
	        		$moduleCache[$module_loop]['d'] = $date_loop;
	        		
	        	} else {
	        		if (!$occur) $occur = 1;
	        		
	        		if (!isset($modCacheFirst[$module_loop])) {
	        			$modCacheFirst[$module_loop] = array('d'=> $date_loop, 'u'=>$user_loop, 'n'=>$occur, 'ip'=>$ipx);
	        			// break; // o.k.
	        		}
	        		
	        		$moduleCache  [$module_loop] = array('d'=> $date_loop, 'u'=>$user_loop, 'n'=>$occur);
	        		if (strstr($moduleCache[$module_loop]['u'],',') ==NULL) {
	        			$moduleCache[$module_loop]['u'] .= ','; // add KOMMA
	        		}
	        	}
	        	
        	} while (0);
	       	

		    $lineID++;
		}
		fclose ( $fp_ori);
		
		if (!$pos_info['end'] and $cnt_compress_lines) {
			$pos_info['end']=$lineID-1;
		}
		
		$newStrArr = array();
		
		
		foreach( $modCacheFirst as $key=>$modularr) {
			$newStrArr[] =  $modularr['d']."\t". $modularr['u']."\t".$key."\t". $modularr['ip'];
		}
		
		foreach( $moduleCache as $key=>$modularr) {
			$newStrArr[] =  $modularr['d']."\t". $modularr['u']."\t".$key."\t". $modularr['n'];
		}
		
		sort($newStrArr);
		
		if ($this->_localDebug>1) {
			glob_printr( $newStrArr, basename(__FILE__).':newStrArr');
			
		}
		
		echo "new number of file lines: ".sizeof($newStrArr)."<br>";
		echo "Start-Pos: ".$pos_info['start']."<br>\n";
		echo "End-Pos: ".$pos_info['end']."<br>\n";
		
		
		if ( !sizeof($newStrArr) ) {
			echo "Nothing to compress.<br>";
			return;
		}
		

		
		$tempname = tempnam("/tmp", "FOO");
		//$tempname = '/tmp/mod_temp';
		$fp = fopen($tempname, "w");
		
		if (!$fp) {
			return 6; // problem on writing file
		}
		
		
		//  PART1
		$start = 1;
		$earlier_data_pos = $pos_info['start']-1;
		echo "Start:".$start."; End:".$earlier_data_pos." <br>";
		$this->appendFile($fp, $filename, $start, $earlier_data_pos);
		
		//  PART2
		foreach( $newStrArr as $dummy=>$info) {
			$retVal = fputs( $fp, $info."\n" ); 
		}
		
		//  PART3
		// append rest of ORI-log-file
		$startpos=$pos_info['end']+1;
		$endpos=-1;
		echo "Start:".$startpos."; End:".$endpos." <br>";
		$this->appendFile($fp, $filename, $startpos, $endpos);
		
		fclose( $fp );

		
		$answer = copy ($tempname, $filename);
		if (!$answer) {
			$error->set( $FUNCNAME, 1, 'error on file copy to '.$filename );
		}
		
		

		
		
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


		
		$fieldx = array ( // form-field definition
			"title"   => "Date pattern", 
			"name"  => "timePat",
			"object"=> "text",
			"val"    => $parx['timePat'], 
			"notes" => "do Compressing till this Date; e.g. 2013-01-29 "
			 );
		$formobj->fieldOut( $fieldx ); // output the form-field

		$formobj->close( TRUE ); // close form, sshow submit-button
	}
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$this->_localDebug = $_SESSION["userGlob"]["g.debugLevel"];
		
		$this->parx   = $_REQUEST['parx'];
		$go   = $_REQUEST['go'];
		$sqlo = &$this->_sqlo;
		
		$this->_form($this->parx);
		
		if (!$go) {
			return;
		}
		
		if ($this->parx['timePat']==NULL) {
			$this->htmlFoot('ERROR', 'Need the time pattern.');
		}
		
		$this->analyse();
		
	}
	
}
