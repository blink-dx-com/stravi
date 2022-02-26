<?php
require_once ('f.update.inc');
/**
 * it contains code, which will be included in edit.tmpl.php, when the rider is set to "MODE"
 * $Header: trunk/src/www/pionir/obj.db_user.xmode.mng.inc 59 2018-11-21 09:04:09Z $
 * @package    edit.tmpl.php
 * @subpackage obj.DB_USER.xmode.mng.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0000126:003: only restricted group of managers can do this
 */
class oDB_USER_xmode_mng {
	
function __construct( &$editLib ) {
	$this->editLib = &$editLib;
}

function _form(&$sqlo, $id, $oldAttribs) {
	
	$tablename='DB_USER';
	$fieldArr = array(
		'FULL_NAME' =>array( "object"=> "text", "fsize"=>40), 
		'LOGIN_DENY'=>array( "object"=> "checkbox"), 
		'EMAIL'=>array( "object"=> "text", "fsize"=>40), 
		'NOTES'=>array( "object"=> "textarea", "inits"=>array('rows'=>9, 'cols'=>80), 'colspan'=>2 ), 
	);
	
	require_once ('func_form.inc');
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "main user features";
	$initarr["submittitle"] = "Update";
	$initarr["tabwidth"]    = "500";

	$hiddenarr = NULL;
	$hiddenarr["tablename"]    = $tablename;
	$hiddenarr["id"]           = $id;
	$hiddenarr["tabArgu[go]"]  = 1;

	$formobj = new formc($initarr, $hiddenarr, 0);

	reset ($fieldArr);
	foreach( $fieldArr as $key=>$fieldx) {
		
		$oneRow = $fieldx;
		$oneRow['name']  = 'tabArgu['.$key.']';
		$oneRow['namex'] = TRUE;
		$oneRow['title'] = columnname_nice2($tablename, $key);
		$oneRow['val']   = $oldAttribs[$key];
		
		if ($this->_editMode!='edit') $oneRow['view']  = 1;
		
		$formobj->fieldOut( $oneRow );
	}
	reset ($fieldArr); 

	$formOpt=array();;
	if ($this->_editMode!='edit') $formOpt["noSubmitButton"]=1;
	$formobj->close( TRUE, $formOpt );
	
}

/**
 * update vals
 * @param unknown_type $sqlo
 * @param unknown_type $id
 * @param unknown_type $tabArgu
 */
function _update( &$sqlo, $id, $tabArgu) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	reset ($this->editCols);
	$argu   = array('DB_USER_ID'=>$id );
	$errText= NULL;
	foreach( $this->editCols as $dummy=>$key) {
		$val = trim($tabArgu[$key]);
		switch ($key) {
			case 'FULL_NAME':
				if ($val==NULL) {
					$errText='full name needed';
				}
				break;
			case 'EMAIL':
				if ($val==NULL) {
					$errText='email needed';
				}
				break;
			case 'LOGIN_DENY':
				if ($val==NULL) {
					$val=0;
				}
				break;
		} 
		
		if ($errText!=NULL) {
			$error->set( $FUNCNAME, 1, $errText );
			return;
		}
		
		$argu[$key]=$val;

	}
	
	$upOpt=array("noRightCheck"=>1); // because x_rights was set specially in the class
	$answer = gObjUpdate::update_row($sqlo, 'DB_USER', $argu, $upOpt);
	
	if ($answer<1) {
		$error->set( $FUNCNAME, 1, 'error on Update.' );
		return;
	}
}

// PLEASE use this function; still optional 
function xmode_start( &$sqlo, &$sqlo2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {
	global $error, $varcol;
	
	$tablename='DB_USER';
	$this->readCols = array('FULL_NAME', 'LOGIN_DENY', 'EMAIL', 'NOTES');
	$this->editCols = array('FULL_NAME', 'LOGIN_DENY', 'EMAIL', 'NOTES');
	
	$this->_editMode = $this->editLib->getEditMode();
	
	echo '<ul>'."\n";
	// manage SYS-table-objects, where table-rights ($t_rights) are set
	echo 'Console for user-manager. (need table-right: o.DB_USER=write)<br>';
	
	$t_rights = tableAccessCheck($sqlo, $tablename);
	$x_rights = $t_rights;

	if (!$x_rights['write']) {
		echo 'You have no table-right "write".<br>';
		return;
	}
	
	if ($tabArgu['go']) {
		// last check before update
		if (!$x_rights['write']) {
			echo 'You have no table-right "write".<br>';
			return;
		}
		
		$this->_update($sqlo, $id, $tabArgu);
		if ($error->Got(READONLY))  {
			$error->printAllPrio();
			$this->_form($sqlo, $id, $tabArgu);
			return;
		} else {
			echo 'o.k.<br>';
		}
	}
	
	$oldAttribs = glob_elemDataGet3( $sqlo, $tablename, array('DB_USER_ID'=>$id), $this->readCols );
	$this->_form($sqlo, $id, $oldAttribs);
	
	
	
}

}

