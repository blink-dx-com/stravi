<?php
/**
 * - sub funcs for wiid and roid (table: H_WIID)
   - the $wiid_num is related to 	$_SESSION['globals']["magasin_serial"]
 * @package f.wiid_roid.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class fWiidRoidC {

function __construct() {
	$this->magasinBase = ''; //OLD: "http://www.clondiag.com/magasin/?db=";
}

function wiid2intern(
	&$sql, 
	$wiid_num // Partisan-database WIID-number
) {
	//RETURN: internal WIID
	$sqls = "select WIID from H_WIID where NAME='".$this->magasinBase.$wiid_num."'";
	$sql->query($sqls);
	$sql->ReadRow();
	$wiid_intern = $sql->RowData[0];
	
	return ($wiid_intern);
}

/**
 *  WIID-number of magasin
 * @param object $sql
 * @param int $wiidintern : Partisan-database WIID-number
 * @return void|string
 */
function intern2wiid(
	&$sql, 
	$wiidintern 
) {

	$sqls = "select NAME from H_WIID where WIID=".$wiidintern;
	$sql->query($sqls);
	$sql->ReadRow();
	$wiid_name = $sql->RowData[0];
	if ($wiid_name=="") {
	    return;
	}
	
	if ( strstr($wiid_name, $this->magasinBase)==NULL ) return $wiid_name;
	$wiid = substr($wiid_name, strlen($this->magasinBase));
	
	return ($wiid);
}

/**
 * get object-ID from WIID:ROID identifiers
 * @param object $sql
 * @param string $table
 * @param int $wiid_num // magasin-serial-number; can also be the number of the current database
 * @param int $roid // remote-object-ID
 * @return int
 */
function getObjID (&$sql, $table, $wiid_num, $roid      ) {
	global $error;
	
		
	if ( $_SESSION['globals']["magasin_serial"] == $wiid_num ) {
		// object comes from the current database
		return ( $roid );
	}
	
	$cntnum = 0;
	$wiid_intern = $this->wiid2intern($sql, $wiid_num);
	if (!$wiid_intern) {
		$error->set("getObjID", 1, "WIID '".$wiid_num."' not found in this database.");
		return;
	}
	
	$sqls = "select CCT_ACCESS_ID from CCT_ACCESS where TABLE_NAME='".$table."' AND WIID=".$wiid_intern." AND ROID=".$roid;
	$sql->query($sqls);
	$sql->ReadRow();
	$cct_access_id = $sql->RowData[0];
	
	if ($sql->ReadRow()) {
		$error->set("getObjID", 2, "More than ONE entry found in CCT_ACEESS for TABLE='".$table."' WIID='".$wiid_num."' ROID='".$roid."'.");
		return;
	}
	
	if (!$cct_access_id) {
		$error->set("getObjID", 3, "object TABLE='".$table."' WIID='".$wiid_num."' ROID='".$roid."' not found in this database.");
		return;
	}
	$pk_name = PrimNameGet2($table);
	$sqls = "select ".$pk_name." from ".$table." where CCT_ACCESS_ID=".$cct_access_id;
	$sql->query($sqls);
	$sql->ReadRow();
	$obj_id = $sql->RowData[0];
	$cntnum = 1;
	
	if ($sql->ReadRow()) {
		$error->set("getObjID", 4, "More than ONE entry found in '".$table."' for CCT_ACCESS_ID=".$cct_access_id.".");
		$cntnum=2;
		return;
	}
	
	return ( $obj_id );
	
}

/**
 * return WIID, ROID from object
 * @param object $sql
 * @param string $table
 * @param int $id
 * @return array or NULL
 *    array($wiid, $roid)
 */
function getWiroiByID( &$sql, $table, $id ) {
	
	$pk_name = PrimNameGet2($table);
	$sqlsel = "CCT_ACCESS_ID from ".$table." where ".$pk_name."=".$id;
	$sql->Quesel($sqlsel);
	$sql->ReadRow();
	$cct_access_id = $sql->RowData[0];
	
	$sqlsel = "WIID, ROID from CCT_ACCESS where CCT_ACCESS_ID=".$cct_access_id;
	$sql->Quesel($sqlsel);
	$sql->ReadRow();
	$wiid_loc = $sql->RowData[0];
	$roid = $sql->RowData[1];
	
	if (!$roid AND !$wiid_loc) return;
	
	$wiid = $this->intern2wiid($sql, $wiid_loc);
	return array($wiid, $roid);
	
}

}