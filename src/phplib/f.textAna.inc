<?php
/*MODULE:  f.textAna.inc
  DESCR:   text support
  AUTHOR:  qbi
  VERSION: 0.1 - 20020904
*/

class fTextAnalysis {

function openText( &$text ) {
	$this->text = $text;
}

function setTextArray($textArr) {
	$this->_textArr = $textArr;
}

function getLine () {
	
	$ok = 1;
	
	if ($this->_textArr==NULL) {
		$this->_textArr = explode("\n", $this->text);
	}
	
	$resarr = each($this->_textArr);
	
	if ($resarr===NULL) 
		$ok=0;
	else $line = $resarr[1];
	
	return array($ok, $line);
}

function getWords( 
	$line, 
	$delimiter // [0] - start
			   // [1] - end
	) {
	
	$words=NULL;
	$lineLen = strlen($line);
	$pos = 0;
	while ( $pos<$lineLen ) {
		$charx = substr($line, $pos, 1);
		
		if ($charx==$delimiter[0]) {
			// found
			$posStart = $pos;
			$pos++;
			$saveword="";
			while ( $pos<$lineLen ) {
				$charx = substr($line, $pos, 1);
				if ($charx==$delimiter[1]) {
					$words[$posStart]=$saveword;
					break;
				}
				if ($charx==$delimiter[0]) { // start char again
					$pos--; // jump back one char
					break;
				}
				
				$saveword .= $charx;
				$pos++;
			}
		}
		$pos++;
	}
	return ($words);
}

}
