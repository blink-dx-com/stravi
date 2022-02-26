<?php
/**
 * translate CCT_ACC_UP to human readable text
 * $Header: trunk/src/phplib/o.CCT_ACC_UP.human.inc 59 2018-11-21 09:04:09Z $
 * @package o.CCT_ACC_UP.human
 * @author  qbi
 */
 
class oCCT_ACC_UP_hum  {

function __construct() {

	$this->actdef = array("new"=>"new", "mod"=>"modified", "del"=>"deleted" );
	// ["features", "assoc elements","extra class params"]
}

function getNiceAct($act) {
	$retval = $this->actdef[$act];
	if ($retval=="")  $retval = $act;
	return ($retval);
}

/**
 * search for next element
 * @param int  $pos: start of search (no ":")
 * @return array (key, val, int) : $key, $val, $nextPos: pos after ":"
 *						$nextpos = NULL : no next pos
 */
function _nextElem($field, $pos) {

    $colonPos = strpos($field,':', $pos);
    
    if (substr($field,$colonPos+1,1)=='"') {
        // this is a STRING "string"
        $field   = str_replace('\\"',"'", $field); // un-escape \"
        $string_endPos = strpos($field,'"', $colonPos+2); // end of text
        $nextPos = strpos($field, " ", $string_endPos);
    } else {
	   $nextPos = strpos($field, " ", $pos);
    }
    
	if ($nextPos==FALSE) {
		$text    = substr($field, $pos);
		$nextPos = strlen($field);
	}
	$nexlen = $nextPos-$pos;
	$text   = substr($field, $pos, $nexlen);
	//$keyval = explode(":",$text);
	$colonPos = strpos($text,':');
	$nextPos++;
	if ($nextPos>=strlen($field)) $nextPos=NULL;
	$key = substr($text, 0,$colonPos);
	$val = substr($text, $colonPos+1);
	return array( $key, $val, $nextPos );
}

// search for next element
function _getRest($field, $pos) {
	$text    =  substr($field, $pos);
	return ($text);
}
/**
 * will be deprecated later ...
 * transform $entry to array
 * @return array (
 *    "d"=>array("x"=>{"fea"=>, "ass"=>, ...}, "det"=>DETAIL_TXT, "ex"=> extra_info after "det" )
	  "a"=>array("key"=>ACT) ,
	  "x"=> array("ty"=> TEXT ) 
						)
 */
function _extract_OLD($entry) {
	$infarr = explode("<",$entry);
	$outarr = NULL;
	
	foreach( $infarr as $dummy=>$field) {
		if (substr($field,-1,1)=='>') {
			$field = substr($field,0,-1);
		}
		
		list($key, $val, $nextPos) = $this->_nextElem($field, $nextKeyPos);
		
		switch ($key) {
			case "d": // DATAKEY: <d:ass t:SPOT_RES pr:0,2
				$darr = NULL;
				$darr ["x"] = array($val=>NULL);
				
				while ( $nextPos!=NULL ) {
					list( $key, $nexttxt, $nextPos) =  $this->_nextElem($field, $nextPos);
					$darr [$key] = $nexttxt;
				}
				$outarr["d"] = $darr;
				break;
			case "a": //ACTION: new|mod|del
				$aarr = NULL;
				$aarr["key"]  = $val;
				while ( $nextPos!=NULL ) {
					list( $key, $nexttxt, $nextPos) =  $this->_nextElem($field, $nextPos);
					$aarr [$key] = $nexttxt;
				}
				$outarr["a"] = $aarr;
				break;
			case "x": // EXTRA
				$xarr = array('x'=>substr($field, $nextPos)); // the rest ....
				$outarr["x"] = $xarr;
				break;
		}
	}
	
	return ($outarr);
}

function extract($entry) {
    if ($entry==NULL) return array();
    if (substr($entry,0,1)=='<') {
        return $this->_extract_OLD($entry);
    }
    
    $entry_arr = json_decode($entry,TRUE);
    if (!is_array($entry_arr)) return array(); // error
    
    return $entry_arr;
}

}