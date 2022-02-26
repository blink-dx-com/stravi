<?php

require_once ("f.objview.inc");

/**
 * analyse xmlrpc log file
 *    - START: 2016-08-05 15:54:39; END: 13174
 * @package g.xmlrpc_log.ana.inc
 * @swreq   SREQ:
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $parx['timePat'] time pattern: e.g. 2013-01-29:12
 *   
 */

class g_xmlrpc_log_ana_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		
		$title		= 'Analyse the xmlrpc log file';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		
		$this->infoarr['form_type']= 'tool'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['locrow']   = array( array('rootsubs/rootFuncs.php', 'home') );
				
		
	}
	
	private function serialize_x($value) {
		echo serialize($value)."<br>\n";
	}
	
	private function getXML_init() {
		$this->full_in_out_str=NULL; // 1: started
		$this->outcnt = 0;
	}
	
	private function getXML_bulk(&$fp, &$linecnt) {
		
		$outputType = $this->outputType;
		$endline = $this->parx['endline'];
		
		$action= 'CALL';
		$LINE_LENGTH = 100000;
		$KEY1 = '</methodCall>';
		$KEY1_len = strlen($KEY1);
		$KEY2 = '<methodName>';
		$KEY2_len = strlen($KEY2);
		$KEY3 = '</methodName>';
		$KEY3_len = strlen($KEY3);
		
		$KEY5 = '</methodResponse>';
		$KEY5_len = strlen($KEY5);
		
		$KEY4 = '<OPENCOMM';
		$KEY4_len = strlen($KEY4);
		
		$full_string = NULL;
		
		while( !feof ( $fp ) ) {
				
			$line     = fgets($fp, $LINE_LENGTH);
			$linecnt++;
			
			if ($endline>0 and $linecnt>=$endline) break; // stop it now
			
			// echo "-LINE:".htmlspecialchars($line)."<br>";
			if (substr($line,0,$KEY4_len)==$KEY4) {
				if ($outputType=='normal') echo "-L:".$linecnt.": OPENCOM:".htmlspecialchars($line)."<br>";
				continue;
			}

			$full_string .= $line;
			
			if (substr($line,0,$KEY2_len)==$KEY2) {
				$pos1 = strpos($line, $KEY2);
				$str2 = substr($line,$pos1+$KEY2_len);
				
				$pos2 = strpos($str2, $KEY3);
				$meth_string = substr($str2,0, $pos2);
				
				if ($outputType=='normal') {
					echo "- method: <b>".$meth_string."</b><br>";
				} 
			}
			
			if ( strstr($line,$KEY1)!=NULL or  strstr($line,$KEY5)!=NULL ) {
				// end found
				if (strstr($line,$KEY5)!=NULL) $action= 'RESPONSE';
				break;
			}
			
		}
		
		// analyse string
		
		$value_dict = xmlrpc_decode($full_string);
		
		// echo "DEBUG_LLL: |".htmlspecialchars($full_string)."|<br>";
		switch ($outputType) {
			case 'normal':
				var_dump($value_dict);
				echo "<br><br>";
				break;
				
			case 'code':
				echo $this->serialize_x(array('method'=>$meth_string));
				echo $action.' ';
				echo $this->serialize_x($value_dict);
				break;
				
			case 'code1':
				
				$meth_string_out = $meth_string;
				if ($meth_string=='metacall') {
					$subfunc = $value_dict[1];
					$meth_string_out .= ' ('.$subfunc.')';
				}
				
				if ($meth_string!=NULL) {
					echo 'M: '.$meth_string_out."<br>";
				}
				break;
			case 'code2': // show analysed serial array
				/**
				 * $this->full_in_out_str : array(
				 * 'meth'=>method
				 * 'in' => array
				 * 'out'=>array
				 * )
				 */
				// $this->full_in_out_str=NULL
				if ($action=='CALL') {
					$this->full_in_out_str['meth']=$meth_string;
					$this->full_in_out_str['in']  =$value_dict;
				} else {
					$this->full_in_out_str['out']=$value_dict;
					// echo base64_encode(serialize($this->full_in_out_str));
					echo htmlspecialchars( '$testx['.$this->outcnt.'] = '. var_export($this->full_in_out_str, TRUE) ).';';
					$this->outcnt++;
					// echo htmlspecialchars( print_r($this->full_in_out_str,1) );
					echo "\n<br>";
					
					$this->full_in_out_str= NULL;
				}
				
				
				break;
		}
		
		
		
	}
	
	function analyse() {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		
		$parx = $this->parx;

		$timePat=NULL;
		$patternLen = 0;
		if ($parx['timePat']!=NULL) {
			$timePat=$parx['timePat'];
			$patternLen = strlen($timePat);
		}
		
		$this->getXML_init();
		
		
		$filename = $_SESSION['globals']['xml_rpc.debug_dir'] .'/xml_rpc_log.log';
		echo "Fielname:".$filename."<br>";
		
		if (!file_exists($filename)) {
			return; // file does not exist
		}
		
		$LINE_LENGTH = 100000;
	    $fp = fopen($filename, 'r');
		if (!$fp) {
			$error->set( $FUNCNAME, 1, 'Cant open advmod_file for read: '.$filename );
			return;
		}
		
		$sizex = filesize($filename);
		
		$cnt=1;
		while( !feof ( $fp ) ) { 
	        fgets($fp, $LINE_LENGTH);
		    $cnt++;
		}
		rewind ($fp);
		
		$this->_html->line_out('Lines of file: '.$cnt.' Size of file: '.$sizex. ' bytes');
		
		$this->_html->line_out('Search: Date pattern: '.$timePat);
		
		/*
		$headOpt = array( "title" => "show XML RPC LOG");
		$headx   = array ("Line", "CCT_ACCESS_ID /<br />Date", "Dict");
		$this->_html->_table3_init($headx, $headOpt);
		*/
		
		$objLinkLib = new fObjViewC();
	
		$lineID=1;
		$doShow=0;
		$rowopt = array("trOpt"=>'valign=top');
		$KEY1 = '<OPENCOMM';
		$KEY1_len = strlen($KEY1);
		$bulkcnt=0;
		$linecnt=0;
		$endline = $this->parx['endline'];
		
		while( !feof ( $fp ) ) { 
	        
	        if (!$doShow) {
	        	
	        	$line     = fgets($fp, $LINE_LENGTH);
	        	$linecnt++;
	        	
	        	if ( trim($line)==NULL ) continue; // is empty
	        	
	        	if (substr($line,0,$KEY1_len)==$KEY1) {
	        		if (strstr($line, $timePat)!=NULL) {
	        			$doShow=1;
	        		}
	        	}
	        }
        	
        	if ($doShow) {
        		
	        	$lineinfo = $this->getXML_bulk($fp, $linecnt);
	        	// $looparr = array($lineID, $lineinfo);	
	        	// $this->_html->_table3_row( $looparr, $rowopt );
	        	
	        	if ($endline>0 and $linecnt>=$endline) break; // stop it now
	        	
	        	$bulkcnt++;
	        	
	        	// if ($bulkcnt>12) exit;
        	}
	         
	        	
	        
		    $lineID++;
		}
		fclose ( $fp);
		
		// $this->_html->_table3_close( );
		
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
			"notes" => "e.g. 2013-01-29 or 2013-01-29:12:28"
			 );
		$formobj->fieldOut( $fieldx ); // output the form-field
		

		$fieldx = array ( // form-field definition
				"title"   => "Output",
				"name"  => "output",
				"object"=> "text",
				"val"    => $parx['output'],
				"notes" => "code | normal"
		);
		$formobj->fieldOut( $fieldx ); // output the form-field
		
		$fieldx = array ( // form-field definition
				"title"   => "Break-On-Line",
				"name"  => "endline",
				"object"=> "text",
				"val"    => $parx['endline'],
				"notes" => "Stop analysis on line; e.g. 3456"
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
		
		$this->outputType = $this->parx['output'];
		if ($this->parx['output']==NULL) $this->outputType='normal';
		
		$this->_form($parx);
		
		if (!$go) {
			return;
		}
		
		$this->analyse();
		
	}
	
}
