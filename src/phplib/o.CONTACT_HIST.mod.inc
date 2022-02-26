<?php
require_once ("date_funcs.inc");
require_once ("f.assocUpdate.inc");



/**
 * modify entry for CONTACT_HIST
 * @package o.CONTACT_HIST.mod.inc
 * @swreq UREQ:9506: o.CONTACT > Kommunikations-Historie abbilden
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/o.CONTACT_HIST.mod.inc 59 2018-11-21 09:04:09Z $
 */
class oCONTACT_HIST_mod {
	
function __construct() {
	$this->assoclib    = new fAssocUpdate();
	$this->validateLib = new fValidate();
	
	$this->allowKeys = array ('SUBJECT','EVENT_DATE','NOTES');
	
}
	
function setContact($objid) {
	$this->contact_id=$objid;
}

function checkAccess(&$sqlo) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$o_rights = access_check($sqlo, 'CONTACT', $this->contact_id);
	if ( !$o_rights["insert"]) {
		$error->set( $FUNCNAME, 1,"You do not have insert permission on this contact!");
		return;
	}
	
}

/**
 * check parameters for plausibility
 * 
 * @return NULL : o.k.
          array(errflag, $errtext) ERROR
 */
function paramCheck(&$sqlo, $argu) {
	$now = time();
	
	if ($argu['SUBJECT']==NULL ) {
		return array(1, 'Short description missing.' );
	}
	
	if (strlen($argu['SUBJECT'])>100 ) {
		return array(2, 'Short description: too long; max: 100 chars.' );
	}
	
	
	if ($argu['EVENT_DATE']==NULL) {
		return array(3, 'EVENT_DATE: no value.');
	}
	
	if ($argu['EVENT_DATE']!=NULL) {
		// check date
		$info = $this->validateLib->check_type($argu['EVENT_DATE'], 'DATE2');
		if ($info['ok']<1) {
			return array(4, 'EVENT_DATE: Format-error: '.$info['info']);
		}
	}
	
	if (strlen($argu['NOTES'])>4000) {
		return array(3, 'notes: too long. Max 4000 chars.');
	}

	
}


/**
 * insert e new entry of CONTACT_HIST
 * @param $sqlo
 * @param $parx :"SUBJECT", "NOTES", [EVENT_DATE]
 * 	  keys are not allowed: POS, DB_USER_ID
 * @return $newpos of entry
 */
function addEntry( &$sqlo, $parx) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$id = $this->contact_id;
	
	
	
	// get last entry
	$sqls="select max(POS) from CONTACT_HIST where CONTACT_ID=".$id;
	$sqlo->query($sqls);
	$sqlo->ReadRow();
	$lastpos = $sqlo->RowData[0];
	if ($lastpos=="" OR $lastpos<=0) $lastpos = 0;
	$newpos = $lastpos + 1;
	
	
	$nowdateStr = date_unix2datestr( time(), 2);
	
	// copy allowed parameters 
	$argu=NULL;
	
	foreach($this->allowKeys as $key) {
		if ($parx[$key]!==NULL) $argu[$key] = $parx[$key];
	}
	 
	
	$argu["POS"] 	= $newpos;
	$argu["DB_USER_ID"] 	= $_SESSION['sec']['db_user_id'];
	$argu["MOD_DATE"] = $nowdateStr;
	
	
	$this->assoclib->setObj( $sqlo, "CONTACT_HIST", $id );
	if ($error->Got(READONLY)) {
		$error->set( $FUNCNAME, 1, 'error on object set' );
		return;
	}
	
	$retval = $this->assoclib->insert( $sqlo, $argu, $inopt );
	if ($retval<=0) {
		$error->set( $FUNCNAME, 1, 'SQL insert failed!' );
		return;
	}
	return $newpos;
}

/**
 * update one entry
 */
function updateEntry(&$sqlo, $pos, $parx) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$id = $this->contact_id;
	
	$argu=NULL;
	
	foreach($this->allowKeys as $key) {
		if ($parx[$key]!==NULL) $argu[$key] = $parx[$key];
	}
	
	$this->assoclib = new  fAssocUpdate();
	$this->assoclib->setObj( $sqlo, "CONTACT_HIST", $id );
	if ($error->Got(READONLY)) {
		$error->set( $FUNCNAME, 1, 'error on object set' );
		return;
	}
	
	// plaus check; check date ???
	$nowdateStr = date_unix2datestr( time(), 2);
	
	$idarr = array('POS'=>$pos);
	$argu["DB_USER_ID"] 	= $_SESSION['sec']['db_user_id']; // update user
	$argu["MOD_DATE"] = $nowdateStr;
	
	$this->assoclib->update( $sqlo, $argu, $idarr );
	if ($error->Got(READONLY)) {
		$error->set( $FUNCNAME, 1, 'SQL update failed!' );
		return;
	}
}

}

 