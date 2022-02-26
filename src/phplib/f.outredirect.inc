<?php
/*MODULE: f.outredirect.inc 
  DESCR:  redirect output to a file
  AUTHOR: qbi
  VERSION: 0.1 - 20050720
*/
 


// start buffering to file
function  mb_output_handler1($text, $status) {
	global $s_outputHndlFp;
	fwrite($s_outputHndlFp, $text);
}

class outputSaveC {

function openFile($filename) {
	if ( ($fhd = fopen($filename , "w")) == FALSE) {
		echo "ERROR: could not write $filename\n";
		return (-1);
	}
	return ($fhd);
}

function closeFile( &$fhd ) {
  while (@ob_end_flush());
	fclose($fhd);
}

}