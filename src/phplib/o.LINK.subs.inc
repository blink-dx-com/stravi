<?php
/**
 * functions for LINK
 * @package o.LINK.subs.inc
 * @swreq GLOBAL
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/o.LINK.subs.inc 59 2018-11-21 09:04:09Z $
 */
require_once ('f.update.inc');

function linkpath_get( $id ) {
	
	$pather = $_SESSION['globals']["data_path"] . "/o.LINK.".$id;
	return ($pather);
}

// ---------------------------------------------------------

/**
 * LINK upload methods
 * @author steffen
 *
 */
class oLinkUpload {

/**
 * copy uploaded file to LINK-object
 * @param unknown $sql
 * @param string $filename
 * @param unknown $id
 * @param unknown $userfile_name : original name of file
 * @param unknown $mime_type
 * @param string $option 
 *   "ignRights" => [0], 1
	  "extHash"   => "ec66..." external calculated HASH
	  "iconoapp"  => 1 : call comes from iconoclust
 * @return int
 *  0 OK
 * @throws errors 
	< 0 error
	=-1  Hash exists (warning)
	=-2  upldate MIME_TYPE failed
	=-3  document[s] already exists in database 
	<-10 upload error
 */	
function link_cp_file( &$sql, $filename, $id, $userfile_name, $mime_type,  $option  = NULL ) {
	 
	
	global $error;
	$FUNCNAME = "oLinkUpload::link_cp_file";

	$error = & ErrorHandler::get();
	$main_ret=0;
	$ext   = NULL;
	
	if ( $filename=="none" ) {
		$error->set($FUNCNAME, 10, "ERROR: File '$userfile_name' not found or too big.");
		return;
	}
	if ( !file_exists($filename) OR (!filesize($filename) )  ) {
		$error->set($FUNCNAME, 12, "File '$userfile_name' failed for upload! May be not found.");
		return;
	}
	
	// test for iconoclust scripts
	if ( ($extpos = strrpos($userfile_name, ".")) > 0 )
		$ext = substr ($userfile_name, $extpos+1);
		
	$iconomime="";
	if ( ($ext=="icrun") || ($mime_type=="application/iconoclust-run") ) {
		$iconomime = "application/iconoclust-run";
	}
	if ( ($ext=="icpck") || ($mime_type=="application/iconoclust-pck") ) {
		$iconomime = "application/iconoclust-pck";
		if ( !$option["iconoapp"] ) {
			$error->set($FUNCNAME, 13, "Upload of ICPCK-files only allowed via ICONOCLUST.");
			return;
		}
	}
	
	$updateOpt = NULL;
	if ($option["ignRights"]) $updateOpt["noRightCheck"] = 1;

	$dest_name = linkpath_get( $id );

	if ( !copy( $filename, $dest_name ) ) {
		$error->set($FUNCNAME, 11, "Failed to copy uploaded file.");
		return;
	} 
	
	$argu["LINK_ID"]  = $id;
	$argu["MIME_TYPE"]= $mime_type;
	
	$retval = gObjUpdate::update_row($sql, "LINK", $argu, $updateOpt);
	if ($retval<0) {
		$error->set($FUNCNAME, 2, "Problems on update MIME_TYPE.");
		return;
	}
	
	if ( $iconomime!="" ) { // Iconoclust specials
	
		// calc HASH ???
		if ($option["extHash"]!="" ) {
			$hash_code = $option["extHash"];
		} else {
			$hash_code = md5_file($filename);
		}
		$argu = array();
		$argu["LINK_ID"]	= $id;
		$argu["HASH"]   	= $hash_code;
		$argu["MIME_TYPE"] 	= $iconomime;
		$retval = gObjUpdate::update_row($sql, "LINK", $argu, $updateOpt);
		if ($retval<0) {
			$error->set($FUNCNAME, 1, "Problems on update HASH and MIME_TYPE.");
			return;
		}
		// check, if HASH exists
		$sql->query("SELECT count(link_id) FROM LINK WHERE HASH='$hash_code' AND LINK_ID!=".$id);
		$sql->ReadRow();
		$numval = $sql->RowData[0];
		if ($numval>0) {
			$error->set($FUNCNAME, 3, "<font color=red><b>WARNING</B></font>: file uploaded, but $numval
					document[s] already exists in database (checked by HASH).");
			return;
		}

	}

	return (0);
}

}
