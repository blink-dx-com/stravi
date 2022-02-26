<?php
/*MODULE:  f.textAna2.inc
  DESCR:   text support -- text to array
  AUTHOR:  qbi
  VERSION: 0.1 - 20020904
*/

class fTextAnaTab {

function openText( &$text ) {
	$this->text = $text;
}

function setTextArray($textArr) {
	$this->_textArr = $textArr;
}

function text2array( 
		$sepchar // "AUTO", " ", \t
	) {
	/*  
	FUNCTION:- transform TEXT => ARRAY
			 - ignore empty lines
	RETURN: array("txt"=>, "row"=>, "col"=> )
	*/
	$this->_textArr = explode("\n", $this->text);
	$colMax = 0;
	$resarr = NULL;
	
	if ($sepchar=="AUTO") {
		$sepchar="\t";
		list(,$oneline) = each($this->_textArr);
		reset($this->_textArr);
		
		if ( strstr( $oneline,"\t" )== NULL ) {
			$sepchar=" ";
		}
	} 
	
	$pregCmd = "[".$sepchar."]+";
	if ($sepchar==" ") {
		$pregCmd = "[ ]+"; // split multiple SPACES
	}
	
	foreach( $this->_textArr as $dummy=>$oneline) {
		if ( trim($oneline)== NULL ) continue;
		if ($sepchar!=" ") $oneline = trim($oneline," ");
		if ($oneline=="") continue;
		 
		$subArr   = preg_split("/[".$sepchar."]+/", $oneline);
		$colLen   = sizeof($subArr);
		$resarr[] = $subArr;
		$colMax   = max($colMax, $colLen); 
	}
	reset($this->_textArr);
	
	$retvals = array("txt"=>$resarr, "row"=>sizeof($resarr), "col"=>$colMax );
	
	return ($retvals);
}


}
