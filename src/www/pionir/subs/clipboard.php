<?php
/**
 *  do action with clipboard
 *  TBD: transform history.back to location.replace(); user MUST then give a $backUrl
 * @package clipboard.php

 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $action 	( "copy", "copyplus" )
	 $tablename
	 $ida	first primary key 
	 [$idb]
	 [$idc]
 */

session_start(); 


require_once ('reqnormal.inc');
require_once("javascript.inc" );

$action = $_REQUEST['action'];
$tablename = $_REQUEST['tablename'];
$ida = $_REQUEST['ida'];
$idb = $_REQUEST['idb'];
$idc = $_REQUEST['idc'];

$pagelib = new gHtmlHead();
$pagelib->PageHeadLight('clipboard'); 

if ( $action == "copyplus" ) {
	$action = "copy";
	$actio_extra_param="plus";
}

if ( $action == "copy" ) {

	$insertelem=1;
	if ( $actio_extra_param != "plus" ) $_SESSION['s_clipboard']=array();
	else {	
		/* check if element allways in clipboard */
	    foreach($_SESSION['s_clipboard'] as $th1) {
		

   			$tmp_tablename=current($th1);
			$id0=next($th1);
			$id1=next($th1);
			$id2=next($th1);
			if ( ($tmp_tablename==$tablename) && ($id0==$ida) && ($id1==$idb) && ($id2==$idc) ) $insertelem=0;

		}
	}
	
	
	if ( ($tablename != "") && ( $ida>0 ) && $insertelem) { /* data check */ 
		 $_SESSION['s_clipboard'][]= array ( "tab"=>$tablename, "ida"=>$ida, "idb"=>$idb, "idc"=>$idc );
		 
	}
} 


js__history_back2();

$pagelib->htmlFoot(' '); 

