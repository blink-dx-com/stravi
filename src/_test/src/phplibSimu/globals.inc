<?php

require_once('ErrorHandler.inc');

class sqlClass {



}

class Varcols
{

function &get() {

  $newobj = new Varcols();
  return ($newobj);

}


}


function &logon2( $dummy=NULL ) {
	$objlib = new sqlClass();
	return $objlib;
}



function tablename_nice2($tablename) {
  
  $nicename = strtoupper($tablename);
  return $nicename;
}

function obj_nice_name( &$sql, $tablename, $t_obj_id, $nicopt) {
	return ( "name_".$t_obj_id);
}

function glob_isadmin() {
  return (1);
}

function glob_printr($vararr, $info) {
	// FUNCTION: print an array for debugging
	// See also: glob_array2String()
	echo "glob_printr() <b>$info</b>";
	echo "<pre>";
	print_r ($vararr);
	echo "</pre>\n";
}