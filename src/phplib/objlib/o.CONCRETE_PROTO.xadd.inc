<?php
require_once('glob.obj.create.ABS.inc');

/**
 * object specific modification class
 * @package oCONCRETE_PROTO.xadd.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq 
 */
class oCONCRETE_PROTO_xmodC extends gObjCreateC_ABS {
	
	var $infox;

/**
 * before CREATION of object check/prepare arguments
 * manipulates this array, format see also object.info.inc 
  format: array ( "vals" => data, "xobj" => extra_obj_data, "access" => access_data)
 */
function crea_prep(&$sqlo, &$obj_param ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$sqlDateNow = date_unix2sql( time() );
	$obj_param["vals"]["EXEC_DATE"] = $sqlDateNow; // set current date	
}

}
