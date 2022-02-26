<?php
require_once 'insert.inc';
require_once 'o.S_OBJLINK.subs.inc';
require_once 'glob.obj.touch.inc';

/**
 * S_OBJLINK modification functions (add, delete)
 * @package o.S_OBJLINK.mod.inc
 * @swreq UREQ:0001011: o.S_OBJLINK > add/remove any child-object to object 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 */
class oS_OBJLINK_mod {

function __construct() {
	$this->objid=NULL;
	$this->moTable=NULL;
}

/**
 * set mother object
 */
function setObj(&$sqlo, $moTable, $objid) {
	$this->moTable = $moTable;
	$this->objid   = $objid;
	$this->isTouched=0;
}

function getChiObjByKey(&$sqlo, $key) {
    if (!$this->_is_init()) {
        sys_error_my( __CLASS__.':'.__FUNCTION__.' not initialized.' );
    }
    return oS_OBJLINK_subs::getChiObjByKey($sqlo, $this->moTable , $this->objid, $key);
}

// class is initialized ? 
function _is_init() {
	if ($this->objid==NULL) return 0;
	else return 1;
}

/**
 * add object link
 * - test, if object already linked => then do nothing
 * @return boolean added ? 
 */
function addLink(&$sqlo, $chTable, $chID, $key) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
	if (!$this->_is_init()) {
		sys_error_my( __CLASS__.':'.__FUNCTION__.' not initialized.' );
	}
	if (!$chID) {
	    $error->set( $FUNCNAME, 1, 'Input-Error: Child-ID not given.' );
	    return;
	}
	
	if ($this->moTable==$chTable) {
	    if ($this->objid==$chID) {
	        $error->set( $FUNCNAME, 2, 'Input-Error: You cannot link the object to itself!' );
	        return;
	    }
	}
	
	// check, if child object already on mother ...
	$sqlsel = "1 from S_OBJLINK where CH_TABLE=".$sqlo->addquotes($chTable).
		' and MO_ID='.$this->objid. ' and MO_TABLE= '.$sqlo->addquotes($this->moTable).
		' and CH_ID='.$chID       . ' and CH_TABLE= '.$sqlo->addquotes($chTable);
	$sqlo->Quesel($sqlsel);
	
	if ($sqlo->ReadRow()) {
		// exists, do not add child again
		return 0;
	}
	$sqlsel = "max(POS) from S_OBJLINK where ".
		' MO_ID='.$this->objid. ' and MO_TABLE= '.$sqlo->addquotes($this->moTable);
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$lastPos  = $sqlo->RowData[0];
	if (!$lastPos) $lastPos=0;
	$newPos=$lastPos+1;

	$argu = array(
		'MO_TABLE'=>$this->moTable,
		'MO_ID' => $this->objid,
		'CH_TABLE'=>$chTable,
		'CH_ID'=>$chID,
		'POS'=>$newPos,
		'KEY'=>$key
	 );
	insert_row_s( $sqlo, 'S_OBJLINK', $argu); 
	return 1;
}

/**
 * remove child-link by POS
 * - touch mother object only for the first time
 * @param  $sqlo
 * @param  string $key : KEY in S_OBJECT
 */
function removeLink_by_key(&$sqlo, string $key) {
    if (!$this->_is_init()) {
        sys_error_my( __CLASS__.':'.__FUNCTION__.' not initialized.' );
    }
    
    $table   = 'S_OBJLINK';
    $wherestr= 'MO_ID='.$this->objid. ' and MO_TABLE= '.$sqlo->addquotes($this->moTable).
        ' and KEY='.$sqlo->addQuotes($key);
    $sqlo->Deletex( $table, $wherestr);
    
    if (!$this->isTouched) {
        $actarr = array('d'=>array('x'=>array('oli'=>NULL)), 'a'=>array('key'=>'del') );
        globObjTouch::touch( $sqlo, $this->moTable, $this->objid, $actarr);
    }
    $this->isTouched=1;
}

/**
 * remove child-link by POS
 * - touch mother object only for the first time
 * @param  $sqlo
 * @param  int  $pos : pos of child in S_OBJECT
 */
function removeLink(&$sqlo, $pos) {
	if (!$this->_is_init()) {
		sys_error_my( __CLASS__.':'.__FUNCTION__.' not initialized.' );
	}	
	
	$table   ='S_OBJLINK';
	$wherestr='MO_ID='.$this->objid. ' and MO_TABLE= '.$sqlo->addquotes($this->moTable).
		' and POS='.$pos;
	$sqlo->Deletex( $table, $wherestr);
	
    if (!$this->isTouched) {
        $actarr = array('d'=>array('x'=>array('oli'=>NULL)), 'a'=>array('key'=>'del') );
        globObjTouch::touch( $sqlo, $this->moTable, $this->objid, $actarr);  
	}
	$this->isTouched=1;        
}

/**
 * remove child-link by POS
 * - touch mother object only for the first time
 * @param  $sqlo
 * @param  int  $pos : pos of child in S_OBJECT
 */
function removeLinkByObjID(&$sqlo, $chTable, $chID) {
	if (!$this->_is_init()) {
		sys_error_my( __CLASS__.':'.__FUNCTION__.' not initialized.' );
	}	
	
	$table   ='S_OBJLINK';
	$wherestr='MO_ID='.$this->objid. ' and MO_TABLE= '.$sqlo->addquotes($this->moTable).
		' and CH_ID='.$chID . ' and CH_TABLE='.$sqlo->addquotes($chTable);
	$sqlo->Deletex( $table, $wherestr);
	if (!$this->isTouched) {
	    $actarr = array('d'=>array('x'=>array('oli'=>NULL)), 'a'=>array('key'=>'del') );
	    globObjTouch::touch( $sqlo, $this->moTable, $this->objid, $actarr);  
	}
	$this->isTouched=1;    
}


}
