<?php
/*MODULE:  glob.obj.col.inc
  DESCR:   sub functions for bulk actions realted to table-columns
  AUTHOR:  qbi
  VERSION: 0.1 - 20070427
*/
require_once("object.subs.inc");

class globTabColGuiC {

function initTab($tablename) {
	$this->table = $tablename;
}

function analyzeColumn(&$sql, $colname ) {
	// FUNCTION: - analyze column for modification-access 
	//			 - test, if the column can be shown in a form
	//
	
	$showcol = 1; 
	
	$colInfos = colFeaturesGet( $sql, $this->table, $colname, 0 ); 
            
	if (  $colInfos["VISIBLE"]=="0" ) $showcol = 0;
	if ( !$colInfos["EDITABLE"] )     $showcol = 0;
	if ( $_SESSION['sec']['appuser'] == 'root' )         $showcol = 1;  // all allowed
	if ( $colInfos["PRIMARY_KEY"]>0 ) $showcol = 0; // but no primary keys
	if ( $colname == "CCT_ACCESS_ID") $showcol = 0;
	if ( $colname == "EXTRA_OBJ_ID")  $showcol = 0;
      
	$retarr=NULL;
	$retarr["showcol"] = $showcol;
	$retarr["nice"]    = $colInfos["NICE_NAME"];
	$retarr["mother"]  = $colInfos["CCT_TABLE_NAME"];
	
	return ($retarr);
}

function anaColAll(&$sql, $colname ) {
	// FUNCTION: - analyze column just for READ-access 
	//			 - used for object-info
	//			 - show also primary keys
	
	$showcol = 1; 
	
	$colInfos = colFeaturesGet( $sql, $this->table, $colname, 0 ); 
            
	if (  $colInfos["VISIBLE"]=="0" ) $showcol = 0;
	if ( $_SESSION['sec']['appuser'] == 'root' )         $showcol = 1;  // all allowed
	if ( $colname == "CCT_ACCESS_ID") $showcol = 0;
	if ( $colname == "EXTRA_OBJ_ID")  $showcol = 0;
      
	$retarr=NULL;
	$retarr["showcol"] = $showcol;
	$retarr["nice"]    = $colInfos["NICE_NAME"];
	$retarr["mother"]  = $colInfos["CCT_TABLE_NAME"];
	
	return ($retarr);
}

}
