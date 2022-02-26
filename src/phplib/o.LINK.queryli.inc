<?php
/**
 * - specials for view.tmpl.php : prepare a user-defined SQL-query
 * - rarly used
 * - used for a special view-column
 * $Header: trunk/src/phplib/o.LINK.queryli.inc 59 2018-11-21 09:04:09Z $
 * @package o.LINK.queryli.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

require_once ("o.LINK.c_query_subs.inc");

class fViewQueSpec {

function userQueryPrep( &$sql, $tablename, $pk_name) {
	// prepare a special user query
	// INPUT:  $_SESSION['userGlob']["o.".$tablename.".l.docQuery"]
	// RETURN: $userSqlObj
			
	global $error;
	$FUNCNAME = "userQueryPrep";
	
	$queryLinkId   = $_SESSION['userGlob']["o.".$tablename.".l.docQuery"];
	if (!$queryLinkId) {
		$error->set($FUNCNAME, 5, "no QueryObjectID given for 'user defined'");
		return;
	}
	$userSqlObj = new myquery();
	$sqlvars  = $userSqlObj->queryObjGetVars( $sql, $queryLinkId );
	if ($error->got(READONLY)) {
		$error->set($FUNCNAME, 6, "document $queryLinkId: error during sql-parameter get");
	}
	$uservars = $userSqlObj->get_uservars( $sqlvars["sqltext"] );

	if (sizeof($uservars)!=1) {
		$error->set($FUNCNAME, 1, "document $queryLinkId: number of user vars not 1");
		return;
	}

	list(, $tmparr) = each($uservars);
	$varname = $tmparr[0];
	if ($varname!=$pk_name) {
		$error->set($FUNCNAME, 2, "document $queryLinkId: expected '".$pk_name."' as #USER_VAR in 'sqltext'");
		return;
	}
	$sqltext_compiled = $sqlvars["sqltext"];
	list($sqlRetArr, $infox) = $userSqlObj->createSqlArr( $sqlvars, $sqltext_compiled );
	$tmptable   = $sqlvars["table"];
	$tmparr2    = &$infox["option"];
	$tmpColName = $tmparr2["outcol"];
	if ($tmpColName=="") {
		$error->set($FUNCNAME, 3, "document $queryLinkId: parameter 'options' :: expected column_name for keyword 'outcol'");
		return;
	}
	
	$sqlAfter  = full_query_get( $tmptable, $sqlRetArr["f"], $sqlRetArr["w"], $sqlRetArr["x"], $sqlRetArr["y"]);
	// store it !
	$userSqlObj->fullQuery = "select ".$tmpColName." from ".$sqlAfter;
	
	return ($userSqlObj);
}
	
	
}