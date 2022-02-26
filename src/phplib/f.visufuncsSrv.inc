<?php
/**
 * helper methods for slim HTML output on server
 * 
 * - this is mainly used for saved HTML-reports
 * - has same interface like in visufuncs.inc for: table_head(), table_row(), table_close()
 * - tables
 * - links
 * @namespace core::gui
 * @package f.visufuncsSrv.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 * @example 
 * 	$this->visuObj = new visufuncsSrv();
 *  $filename = 'tmp/test.html';
 *  $this->visuObj->initFile($filename); 
 *  $this->visuObj->startPage($infoarr);
 *  
	$headOpt = array( "title" =>  $title, 'cssclass' => 'xTabReport');	
	$this->visuObj->table_head($headx,   $headOpt);
	$dataTmp=array('hello', 'world');
	$this->visuObj->table_row ($dataTmp);
	$this->visuObj->table_close();
	
	$this->visuObj->closePage();
	$this->visuObj->closeFile();
 * 
 */

class visufuncsSrv { 
	var $options;
	var $filePtr=NULL;
	var $CLASSNAME='visufuncsSrv';

function initFile($filename) {
	global $error;
	$FUNCNAME= $this->CLASSNAME.':initFile';
	
	$this->filename=$filename;
	$this->filePtr = fopen($filename, 'w');
	if ( !$this->filePtr ) {
		$error->set( $FUNCNAME, 1, 'Cannot open file "'.$filename.'" for write.' );
		return;
	}
	
}

/**
 * start of page, e.g. HTML-page
 * @param array $infoarr: 'title'
 * @return -
 */
function startPage($infoarr) {
	$this->_outRaw('<html><body>'."\n");
	$this->_outRaw('<head>'."\n");
	$this->_outRaw('<title>'.$infoarr['title'].'</title>'."\n");
	$this->_outRaw('</head>'."\n");
	
}

function closePage() {
	$this->_outRaw('<hr />'."\n");
	$this->_outRaw('</body></html>'."\n");
}

function closeFile() {
	fclose( $this->filePtr );
}
	
function _outRaw($text) {
	if ($this->filePtr==NULL) {
		sys_error_my( 'class visufuncsSrv: File-pointer not initialized.' );
	}
	$retVal = fputs( $this->filePtr, $text );
	
}

/**
 * create head of a table
 * @param $header // can be NULL
 * @param $opt // "title"   = title-text
	 		   // "headNoShow" = [0], 1 -- do not show header
			   // "cssclass"     e.g. "datatab"
			   // "colThCss" = array of styles for header
			   // "format  = ["html"] | "csv"
 * @return -
 */
function table_head( &$header,  $opt=NULL ) {
    
	$borderColor="#B0B0B0";
	if ($opt["borderColor"]!="") $borderColor=$opt["borderColor"];
	$tmpcellspace = 1;
	if ($opt["noCellBorder"]) $tmpcellspace = 0; 
	$thisclass = "";
	if ($opt["cssclass"]!="") {
		$thisclass = "class=\"".$opt["cssclass"]."\" ";
	}
	$this->cellColor = "#F3F3F3";
	if ( $opt["cellColor"]!="" )  $this->cellColor = $opt["cellColor"];
	
	$this->options = $opt;
	
	if ($this->options["format"]=="csv") {
	
		// todo
		
	} else {
	
		$this->_outRaw( "<table cellpadding=1 cellspacing=$tmpcellspace border=0 ".$thisclass."bgcolor=".$borderColor.">\n");
		
			
		if ($opt["title"]!="") {
			$colspan=2;
			if (sizeof($header)) $colspan=sizeof($header);
			$this->_outRaw( "<tr><td colspan=$colspan><font color=white><b>".$opt["title"]."</b></font></td></tr>");
		}
		
		if ($opt["noCellBorder"]) $this->_outRaw( "<tr><td><table cellpadding=1 cellspacing=0 border=0 width=100% bgcolor=".$borderColor.">\n");
		
		if (is_array($header) AND !$opt["headNoShow"]) {
			
			$this->_outRaw( "<tr bgcolor=\"#D0D0D0\" style=\"color:#707070; font-weight:bold;\">");
			$i=0;
			foreach( $header as $dummy=>$tmptxt) {
				$tdopt = $opt["headopt"][$i];
				if ($opt["colThCss"][$i]!="" ) {
					$tdopt .=" class=\"".$opt["colThCss"][$i]."\"";
				}
				$this->_outRaw( "<td $tdopt>".$tmptxt."</td>");
				$i++;
			}  
			reset ( $header );
		}
		$this->colopt  = $opt["colopt"];
		$this->_outRaw( "</tr>\n");
	} 
}	

/**
 * @param $dataArr
 * @param $opt=NULL	<pre>
 * 	"bgcolor" = "#EEEEEE"
 *  "colspan" = NUMBER
 *  "other"   = variable text per column
 *  "trOpt"   = optional text in TR-tag
 *  </pre>
 */
function table_row( $dataArr, $opt=NULL) {
	
	if ($this->options["format"]=="csv") {
	
		// todo
		
	} else {
	
		$thisbgcolor = $this->cellColor;
		if ($opt["bgcolor"] !="") $thisbgcolor = $opt["bgcolor"];
		$tmptd = "";
		if ($opt["colspan"] !="") $tmptd  = " colspan=\"".$opt["colspan"]."\"";
		if ($opt["other"] !="")   $tmptd .= " ".$opt["other"];
		
		$this->_outRaw( "<tr bgcolor=".$thisbgcolor.' '.$opt["trOpt"].'>'); 
		$i=0;
		foreach( $dataArr as $dummy=>$tmptxt) {
			$this->_outRaw( "<td ".$this->colopt[$i].$tmptd.">".$tmptxt."</td>");
			$i++;
		}
		$this->_outRaw( "</tr>\n");
	}
}

function table_close() {
	if ($this->options["format"]=="csv") {
		// nothing ....
	} else {
		if ($this->options["noCellBorder"]) $this->_outRaw(  "</table></td></tr>\n");
		$this->_outRaw(  "</table>\n" );
	}
}  

/**
 * error stack as HTML-table to file
 * @return -
 */
function errorOut() {
	global $error;
	if (!$error->Got(READONLY)) return;
	
	$tabopt = array('title'=>'ERROR', "borderColor"=>'#FF0000', "headNoShow"=>1);
	$header=array('text');
	$this->table_head( $header, $tabopt);
	
	$textArray = $error->getTextAsArray();
	reset ($textArray);
	$textAll=NULL;
	$tmpbr  =NULL;
	foreach( $textArray as $dummy=>$text) {
		$textAll .= $tmpbr . $text;
		$tmpbr='<br />';
	}
	reset ($textArray);
	$dataArr = array($textAll);
	$this->table_row( $dataArr );
	$this->table_close();
}

}