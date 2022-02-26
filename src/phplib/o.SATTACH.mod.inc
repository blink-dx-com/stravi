<?php
require_once ("o.SATTACH.subs.inc");
require_once ("insert.inc");
require_once ("glob.obj.touch.inc");
require_once ("date_funcs.inc");
require_once ('f.update.inc');

/**
 * attachment modification functions
 * TBD: introduce globObjTouch::touch() !!!
 * @package o.SATTACH.mod.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq SREQ:0001151: o.SATTACH > anlegen, editieren eines Attachments 
 */
class oSattachMod {
	
	var $newColsExist; // 0,1 exist columns: XDATE and DB_USER_ID ?

function __construct() {
	$this->satObj = new cSattachSubs();
	$this->touchLib = new globObjTouch();
	
	$newColsExist = glob_column_exists('SATTACH', 'ARCHIVE');
	$this->newColsExist = $newColsExist;
}

function setObj($tablename, $objid) {
	$this->tablename = $tablename;
	$this->objid = $objid;
	$this->touched=0;
	
}

function _objIsInitialized() {
	if ($this->objid>0) return 1;
	return 0;
}

function getDocumentPath( $rel_id) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    if (!$this->_objIsInitialized()) {
        $error->set( $FUNCNAME, 1, 'class not initialized.' );
        return;
    }
    
    $path = $this->satObj->getDocumentPath($this->tablename , $this->objid , $rel_id);
    return $path;
}


/**
 * touch object
 * - if already touched; do not touch anymore
 * @param $action 'new', 'del'
 */
function touchObject(&$sqlo, $action, $rel_id) {

	if ($this->touched) return;
	
	$actarr=array( 'd'=>array(   'x'=>array(  'atx'=>array('po'=>$rel_id)  )   ), 'a'=>array('key'=>$action) );
	$this->touchLib->touch( $sqlo, $this->tablename, $this->objid, $actarr );
	
	$this->touched=1;
}

/**
 * check, if such a name exists
 * @param $name
 * @param int $rel_id if set, do check despite the REL_ID
 * @return 1: name exists
 */
function checkNameExists(&$sqlo, $name, $rel_id=0) {
	$sqlsel = "REL_ID from SATTACH where TABLE_NAME='".$this->tablename."' AND OBJ_ID=".
		$this->objid. ' and NAME like '.$sqlo->addQuotes($name);
	if ($rel_id>0) $sqlsel .= ' and REL_ID !='.$rel_id;
	$sqlo->Quesel($sqlsel);
	if ($sqlo->ReadRow()) {
		$nameExists=1;
	} else $nameExists=0;
	return $nameExists;
}

function checkNameExistsSubPos($sqlo, $sub_pos, $name) {
    return cSattachSubs::checkNameExistsSubPos($sqlo, $this->tablename, $this->objid, $sub_pos, $name);
}

function get_rel_id_by_key($sqlo, $key) {
    return $this->satObj->get_LateRelIDByKey($sqlo, $this->tablename, $this->objid, $key);
}
function get_rel_id_by_name(  &$sqlo, $name, $options=NULL ) {
    return $this->satObj->getRelIDbyName($sqlo, $this->tablename, $this->objid, $name, $options);
}

/**
 * insert new attachment entry
 * - if $argin[DB_USER_ID] not set, set it automatically
 * @param array $argin parameters in the form of (KEY=>VAL)
 */
function insertAtt( &$sqlo, $argin ) {
	global $error;
	$FUNCNAME= 'insertAtt';
	
	$ColNotAllow= array( "TABLE_NAME","OBJ_ID","REL_ID" );
	$tablename = $this->tablename;
	$id = $this->objid;
	if ( !$this->_objIsInitialized() ) {
		$error->set( $FUNCNAME, 1, 'class not initialized.' );
		return;
	}
	
	$rel_id = $this->satObj->getNextRelID($sqlo, $tablename, $id);
	
	$argu = $argin;
	$argu["TABLE_NAME"] = $tablename;
	$argu["OBJ_ID"]     = $id;
	$argu["REL_ID"]		= $rel_id;
	
	if ($this->newColsExist) {
		// only set, if database has the latest DB-model
		$argu["XDATE"]      = date_unix2datestr(time(),1);
		if (!$argin["DB_USER_ID"]) $argu["DB_USER_ID"] = $_SESSION['sec']['db_user_id'];
	} else {
		unset ($argu['ARCHIVE']); // does not exist in old version
	}
	
	$retval = insert_row($sqlo, "SATTACH", $argu);
	
	if ($retval<0) {
		$error->set( $FUNCNAME, 1, "Failed to create attachment ($tablename, $id, $rel_id).");
		return;
	}
	
	$this->touchObject($sqlo, 'new', $rel_id);
	
	return ($rel_id);
}
    
/**
 * copy file from $srcfile to attachment with relative-id $rel_id
 */
function copyFile($rel_id, $srcfile ) {
	global $error;
	$FUNCNAME= 'copyFile';
	
	if (!$this->_objIsInitialized()) {
		$error->set( $FUNCNAME, 1, 'class not initialized.' );
		return;
	}
	$destAttachFile = $this->satObj->getDocumentPath($this->tablename, $this->objid, $rel_id);
	if ( !copy(  $srcfile, $destAttachFile ) ) {
		$error->set( $FUNCNAME, 2, 'copy of attachment attachment ($tablename, $id, $rel_id) failed.' );
		return;
	}
}

/**
 * update attributes, attachment identified  by REL_ID
 * - if wrong column names are given in $parx, remove them silent!
 * @param array parx of key=>value; can be selected fields of SATTACH
 */
function update(&$sqlo, $rel_id, $parx) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$tablename  = $this->tablename;
	$id   = $this->objid;
	if ( !$this->_objIsInitialized() ) {
		$error->set( $FUNCNAME, 1, 'class not initialized.' );
		return;
	}
	
	$argu = array();
	$pkarr= array();
	$colnames = columns_get2 ( "SATTACH" ) ;
	
	$pkarr["TABLE_NAME"] = $tablename;
	$pkarr["OBJ_ID"]     = $id;
	$pkarr["REL_ID"]	 = $rel_id;
	
	// update only existing columns
	
	foreach( $parx as $colname=>$val) {
		if ( !isset($pkarr[$colname]) and in_array($colname, $colnames) ) {
			$argu[$colname] = $val;
		}
	}
	
	
	$retval = gObjUpdate::update_row_s($sqlo, "SATTACH", $argu, $pkarr);
	
	if ($retval<0) {
		$error->set( $FUNCNAME, 1, "Failed to update attachment ($tablename, $id, $rel_id).");
		return;
	}
	
	$this->touchObject($sqlo, 'mod', $rel_id);
	
}

/**
 * update attachment by KEY
 * - attachment must exist; else: Error
 */
function updateByKey( &$sqlo,  $key, $argus ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$tablename  = $this->tablename;
	$id   = $this->objid;
	if ( !$this->_objIsInitialized() ) {
		$error->set( $FUNCNAME, 1, 'class not initialized.' );
		return;
	}
	if ($key==NULL) {
		$error->set( $FUNCNAME, 2, 'input-param missing: key' );
		return;
	}
	
	$relid = $this->satObj->getRelIDbyKey( $sqlo, $tablename, $id, $key );
	
	if ( !$relid ) {
		$error->set( $FUNCNAME, 1, "Failed to update attachment ($tablename, $id, key=$key).");
		return;
	} else {
		$this->update($sqlo, $relid, $argus);
	}
}

// delete one attachment
function delAttach( &$sqlo, $rel_id) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$tablename  = $this->tablename;
	$id   = $this->objid;
	if ( !$this->_objIsInitialized() ) {
		$error->set( $FUNCNAME, 1, 'class not initialized.' );
		return;
	}
		
	$sqls = "TABLE_NAME='".$tablename."' AND OBJ_ID=".$id. " AND REL_ID=".$rel_id;
	$retval = $sqlo->Deletex('SATTACH', $sqls);
	
	$this->touchObject($sqlo, 'del', $rel_id);
	
	$filename = $this->satObj->getDocumentPath($tablename, $id, $rel_id);
	if (file_exists( $filename )) {
		unlink($filename);
	}
}

}