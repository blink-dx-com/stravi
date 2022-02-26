<?php
/*
 * This utility class is a collection of often used utility functions 
 * @package 
 * @author Michael Brendel michael.brendel@clondiag.com
 * @version $Header: trunk/src/www/_tests/_test_lib/test_utilities.inc 59 2018-11-21 09:04:09Z $
 */ 
class test_utilities {
	
	
	/*
	 * Generates an alphanumeric random string(incl. special chars) of length $length.
	 * Useful for password generators, arbitrary gibberish strings, ...
	 * @param int $length
	 * @return string $randstr (random String)
	 */
	function generateRandStr($length){
		$randstr = "";
		for($i=0; $i<$length; $i++){
			$randnum = mt_rand(0,94);
        	$randstr .= chr($randnum+32);
        	//$randstr .= chr($i+32);
      	}
      	return $randstr;
      
	}
}