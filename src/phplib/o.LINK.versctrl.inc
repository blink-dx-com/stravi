<?php
/*MODULE:  o.LINK.versctrl.inc
  DESCR:   - version control for documents
  		   - version-documents are found in directory: $_SESSION['globals']["data_path"] . "/vers
  AUTHOR:  qbi
  INPUT:   
  VERSION: 0.1 - 20060418
*/

require_once("date_funcs.inc");
require_once("insert.inc");
require_once("o.LINK.subs.inc");
require_once ('f.update.inc');

class oLINKversC {

var $lastVersPos;	// must call getlastVersPos()

function isPossible() {
	// FUNCTION:- version control possible?
	//			- check DB-version and version directory
	
	
	if ($_SESSION['globals']["DBVersionShort"]<"1.0.3.4") return 0;
	if (!file_exists($_SESSION['globals']["data_path"] . "/vers")) return 0;
	return 1;
}

function getlastVersPos(&$sql, $id) {
	
	$sqls = "select MAX(POS) from LINKV where link_id=".$id;
	$sql->query($sqls);
	$sql->ReadRow();
	$retid = $sql->RowData[0];
	$this->lastVersPos = $retid;
	return ($retid);
}

function countVersions(&$sql, $id) {
	
	$sqls = "select count(POS) from LINKV where link_id=".$id;
	$sql->query($sqls);
	$sql->ReadRow();
	$xcount = $sql->RowData[0];
	return ($xcount);
}

function getLastBefore( &$sql, $id, $pos) {
	$sqls = "select MAX(POS) from LINKV where link_id=".$id." AND POS<".$pos;
	$sql->query($sqls);
	$sql->ReadRow();
	$retid = $sql->RowData[0];
	return ($retid);
}

function isUnderControl(&$sql, $id) {
	$lastpos = $this->getlastVersPos($sql, $id);
	if ($lastpos>0) return 1;
	else return (0); 
}

function initDoc( &$sql, $id) {
	$this->getlastVersPos($sql, $id);
}

function getInfo(&$sql, $id, $pos) {
	global $error;
	
	$sqls ="select LINK_ID, POS, DB_USER_ID, MAIN_VER, SUB_VER, NOTES from LINKV where link_id=".$id." AND POS=".$pos;
	$sql->query($sqls);
	if ( !$sql->ReadArray() ) {
		$error->set("oLINKversC::getInfo", 1, "document version (ID:$id, POS:$pos) not found.");
		return;
	}
	$outdata = $sql->RowData;
	
	$sqls ="select ".$sql->Sql2DateString('cdate', 1)." from LINKV where link_id=".$id." AND POS=".$pos;
	$sql->query($sqls);
	$sql->ReadRow();
	$outdata["CDATE"] = $sql->RowData[0];
	return($outdata);
}

function createSub( &$sql, $id, $newpos, $argu) {
	global  $error;
	
	$datenow = time();
	$datesql = date_unix2datestr($datenow, 1);
	
	$argu["LINK_ID"]= $id;
	$argu["POS"]	= $newpos;
	$argu["DB_USER_ID"] = $_SESSION['sec']['db_user_id'];
	$argu["CDATE"]	= $datesql;
	
	if ( ($argu["MAIN_VER"]=="") ) {
		if ( $this->lastVersPos ) {
			// get new version ...
			$oldArgu = $this->getInfo($sql, $id, $this->lastVersPos);
			$argu["MAIN_VER"] = $oldArgu["MAIN_VER"];
			$argu["SUB_VER"]  = $oldArgu["SUB_VER"]+1;
		} else {
			$argu["MAIN_VER"] = 1;
			$argu["SUB_VER"]  = "0";
		}
	} 
	
	$ioptions = array("types" =>array("CDATE"=>"DATE1"));
	$retval = insert_row($sql, "LINKV", $argu, $ioptions);
	
	if ($retval<=0) {
 		$error->set("oLINKversC::create", 1, "SQL insert failed.");
	}
	
}

function create( &$sql, $id, $argu, 
	$testfile=0 // test, if attachment exists; set error, if not exists
	) {
	// FUNCTION: create an entry an copy document to right position
	global $error;
	
	$lastpos = $this->getlastVersPos($sql, $id);
	$newpos  = $lastpos+1;
	
	if ($testfile) {
		$pathori  = linkpath_get( $id );
		
		if ( !file_exists($pathori) ) {
			$error->set("oLINKversC::create", 1, "Original document-file (ID:$id) does not exist.");
			return;
		}
	}

	$this->createSub( $sql, $id, $newpos, $argu);
	if ($error->Got(READONLY))  {
		return;
	}
	
	if ($lastpos) {
		// copy last file
		$this->copyLastVersion($sql, $id, $lastpos ); 
		if ($error->Got(READONLY))  {
			return;
		}
	}
	return ($newpos);
}

function updateFeatures( &$sql, $id, $pos, $arguin) {
	global $error;
	
	$argu=NULL;
	$argu["LINK_ID"] = $id;
	$argu["POS"] = $pos;
	$argu["MAIN_VER"] = $arguin["MAIN_VER"];
	$argu["SUB_VER"]  = $arguin["SUB_VER"];
	$argu["NOTES"] 	  = $arguin["NOTES"];
	
	$upopt  = array("noRightCheck"=>1);
	$retval = gObjUpdate::update_row( $sql, "LINKV", $argu, $upopt ); 
	if ($retval<0)  $error->set("oLINKversC::updateFeatures", 1, "error during SQL-update ($retval).");
}

function getVersFilePath($id, $pos) {
	// FUNCTION: return document-path, if it is NOT the LAST version
	
	
	$pathout = $_SESSION['globals']["data_path"] . "/vers/o.LINKV.".$id.".".$pos;
	
	return ($pathout);
}

function getVersPathEasy($id, $pos) {
	// FUNCTION: return document-path:
	//			- if NOT last version: o.LINKV.ID.POS
	//			- if is  last version: o.LINK.ID
	
	if ($pos == $this->lastVersPos) {
		$pathOut  = linkpath_get( $id );
	} else {
		$pathOut = $this->getVersFilePath($id, $pos);
	}
	return($pathOut);
}

function checkNeighours(&$sql, $id, $pos, $mainver, $subver) {
	// FUNCTION: check, if new version is valid, check against the neighbor version numbers
	
	// BEFORE pos: new_version must be bigger
	$ok=0;
	$sqls ="select MAIN_VER, SUB_VER from LINKV where link_id=".$id." AND POS<".$pos." order by POS DESC";
	$sql->query($sqls);
	if ( $sql->ReadRow() ) {
		if ( $mainver < $sql->RowData[0] ) $ok=-1;
		if ( $mainver == $sql->RowData[0] AND $subver<=$sql->RowData[1] ) $ok=-2;
		$before=$sql->RowData[0].".".$sql->RowData[1];
	}
	
	// AFTER pos: new_version must be smaller 
	$sqls ="select MAIN_VER, SUB_VER from LINKV where link_id=".$id." AND POS>".$pos." order by POS ASC";
	$sql->query($sqls);
	if ( $sql->ReadRow() ) {
		if ( $mainver > $sql->RowData[0] ) $ok=-3;
		if ( $mainver == $sql->RowData[0] AND $subver>=$sql->RowData[1] ) $ok=-4;
		$nextx = $sql->RowData[0].".".$sql->RowData[1];
	}
	
	return array($ok,$before,$nextx);
}

function copyLastVersion(&$sql, $id, $pos ) {
	// FUNCTION: copy current attachment of LINK to $lastpos-version
	$pathori  = linkpath_get( $id );
	$pathDest = $this->getVersFilePath($id, $pos);
	
	if ( !file_exists($pathori) ) {
		$error->set("oLINKversC::copyLastVersion", 1, "Original document-file (ID:$id) doeas not exist.");
		return;
	}
	if ( !copy($pathori, $pathDest) )  {
		$error->set("oLINKversC::copyLastVersion", 2, "Copy of document-ID:$id, Pos:$pos failed.");
		return;
	}
}

function deleteVers( &$sql, $id, $pos) {
	if ($pos != $this->lastVersPos) { // is not the last version
		$pathFile = $this->getVersFilePath($id, $pos);
		unlink($pathFile);
	} else {
		// the last version-position: copy last before entry to o.LINK.id
		$lastbefore = $this->getLastBefore($sql, $id, $pos);
		if ($lastbefore) {
			$pathLink  = linkpath_get( $id );
			$lastfile = $this->getVersFilePath($id, $pos);
			if ( file_exists($lastfile) ) {
				if ( !copy( $lastfile, $pathLink ) )  {
					$error->set("oLINKversC::copyLastVersion", 2, "Copy of document-ID:$id, Pos:$pos failed.");
					return;
				}
			}
		}
		
	}
	$sql->Deletex("LINKV", "LINK_ID=".$id." AND POS=".$pos);
}

}