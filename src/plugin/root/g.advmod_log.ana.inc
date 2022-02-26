<?php
require_once 'f.advmod_log.inc';
require_once ("f.objview.inc");

/**
 * analyse advmod_log file
 *    
 * @package g.advmod_log.ana.inc
 * @swreq   SREQ:0003894
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $parx['lines'] last x lines
 * @param   $parx['timePat'] time pattern: e.g. 2013-01-29:12
 *   
 */

class g_advmod_log_ana_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		
		$title		= 'Analyse the ADMOD_LOG file';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		
		$this->infoarr['form_type']= 'tool'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['locrow']   = array( array('rootsubs/rootFuncs.php', 'home') );
				
		
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
		
		if ($timePat==NULL and $analines > 500 ) $analines = 500; // reduce it
		
		$testLib  = new f_advmod_log();
		$filename = $testLib->getFileName();
		
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
		
		$this->_html->line_out('Search: last lines: '.$analines.'; Date pattern: '.$timePat);
		
		
		$headOpt = array( "title" => "show ADVMOD_LOG");
		$headx   = array ("Line", "CCT_ACCESS_ID /<br />Date", "Dict");
		$this->_html->_table3_init($headx, $headOpt);
		
		$objLinkLib = new fObjViewC();
		$start_analyse = $cnt - $analines;
		$lineID=1;
		$doShow=0;
		$rowopt = array("trOpt"=>'valign=top');
		while( !feof ( $fp ) ) { 
	        $line     = fgets($fp, $LINE_LENGTH);
	        
	        
	        if ($lineID>=$start_analyse) {
	        	
	        	$objInfo = '???';
	        	
		        if ( trim($line)==NULL ) continue; // is empty
	        	$linearr = explode("\t",$line);
	        	$date_loop = $linearr[1];
	        	$doShow=0;
	        	if ($timePat!=NULL) {
	        		if (substr($date_loop,0, $patternLen)==$timePat) {
	        			$doShow=1;
	        		}
	        	} else $doShow=1;
	        	
	        	if ($doShow) {
	        		
		        	$entryDict = json_decode(base64_decode($linearr[2]), TRUE);
		        	$cct_acc   = $linearr[0];
					if ($cct_acc) {
						$objInfo = 'CCT_ACCESS_ID:'.$cct_acc; // default ...
						
						// get object
						$accFeas = glob_elemDataGet3($sqlo, 'CCT_ACCESS', array('CCT_ACCESS_ID'=>$cct_acc), array('TABLE_NAME'));
						if ($accFeas['TABLE_NAME']!=NULL) {
							$pkname  = PrimNameGet2($accFeas['TABLE_NAME']);
							$objid   = glob_elementDataGet( $sqlo, $accFeas['TABLE_NAME'], 'CCT_ACCESS_ID', $cct_acc, $pkname);
							if ($objid) {
								$objInfo = $objLinkLib->bo_display( $sqlo, $accFeas['TABLE_NAME'], $objid );
							}
						}
					}
					
					// format $entryDict
					$outtmp = 
						'old: '.htmlentities( print_r($entryDict['old'],1) ). "<br />\n" .
						'new: '.htmlentities( print_r($entryDict['new'],1) );
		        	
		        	$looparr = array($lineID, $objInfo."<br />".$date_loop, $outtmp);
		        		
		        	$this->_html->_table3_row( $looparr, $rowopt );
	        	}
	        }    
	        	
	        
		    $lineID++;
		}
		fclose ( $fp);
		
		$this->_html->_table3_close( );
		
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
		
		$fieldx = array ( // form-field definition
			"title"   => "Date pattern", 
			"name"  => "timePat",
			"object"=> "text",
			"val"    => $parx['timePat'], 
			"notes" => "e.g. 2013-01-29 or 2013-01-29:12:28"
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
		
		$this->_form($parx);
		
		if (!$go) {
			return;
		}
		
		$this->analyse();
		
	}
	
}
