<?php
/**
 * Role access functions
     - function rights are identified by NAME: "f." REST
     - positive functions: CCT_RIGHT: "execute"   
     - deny functions    : CCT_RIGHT: "deny"
 * @package role.inc
 * @swreq   SREQ:0000310: g > role right analysis: helper methods 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/role.inc 59 2018-11-21 09:04:09Z $
 */


/**
 * Please use this function only for object.rights !!!
   Check all the rights a user has on a specific object
 * @param  $sql
 * @param  $object object name
 */
function role_check (&$sql, $object ) {
  
  $m_rights=array();
		
  $sql->query("SELECT cct_right FROM user_right WHERE name = ".$sql->addQuotes($object)." AND user_right_id IN ".
			  "(SELECT user_right_id FROM right_in_role WHERE role_id IN ".
			  "(SELECT role_id FROM user_roles WHERE db_user_id = ".$_SESSION['sec']['db_user_id']."))");
  while ($sql->ReadRow())
	$m_rights[$sql->RowData[0]] = 1;
  return $m_rights;
}  

/**
 * Get flag, if user is table-admin
 * @param  $sql
 * @param  $tablename
 * @return int 0,1 : user is table-admin ?
 */
function role_admin_check ( &$sql, $tablename ) {
  
		
  $retval = 0;
  $sql->query("SELECT 1 FROM user_right WHERE name = 'o.".$tablename."' AND cct_right='admin' AND user_right_id IN ".
			  "(SELECT user_right_id FROM right_in_role WHERE role_id IN ".
			  "(SELECT role_id FROM user_roles WHERE db_user_id = ".$_SESSION['sec']['db_user_id']."))");
  if ($sql->ReadRow()) $retval = 1;
  return ($retval);
}

/**
 * - check FUNCTION role right
 - user root gets NOT automatically "execute" !!!
 - possible error-echo : due to your personal role-right 'g.appPrefSave'=DENY ...
 * @param  $sql
 * @param  $funcname : the function-name without "f." e.g. "PaXml_import"
 * @param  $user_id  : user_id
 * @return string "execute" or "deny" or "" [means deny ?]
 */
function role_check_f_pure ( &$sql, $funcname, $user_id ) {
	$retval = "";

	$sql->query("SELECT cct_right FROM user_right WHERE name = 'f.".$funcname."' AND user_right_id IN ".
			"(SELECT user_right_id FROM right_in_role WHERE role_id IN ".
			"(SELECT role_id FROM user_roles WHERE db_user_id = ".$user_id."))");
	if ($sql->ReadRow()) {
	    $retval = $sql->RowData[0];
	}

	return ($retval);
}

/**
 * - check FUNCTION role right 
   - user root gets allways "execute" !!!
   - possible error-echo : due to your personal role-right 'g.appPrefSave'=DENY ...
 * @param  $sql
 * @param  $funcname : the function-name without "f." e.g. "PaXml_import"
 * @param  $user_id  : optional user_id, otherwise $_SESSION['sec']['db_user_id']
 * @return string "execute" or "deny" or "" [means deny ?]
 */
function role_check_f (
    &$sql, 
    $funcname,      
	$user_id = NULL 
    ) {
  
  
  $retval = NULL;

  if ( glob_isAdmin() ) {
      return ("execute");  
  }	
  if ( !$user_id ) $user_id = $_SESSION['sec']['db_user_id'];
  
  $sql->query("SELECT cct_right FROM user_right WHERE name = 'f.".$funcname."' AND user_right_id IN ".
			  "(SELECT user_right_id FROM right_in_role WHERE role_id IN ".
			  "(SELECT role_id FROM user_roles WHERE db_user_id = ".$user_id."))");
  if ($sql->ReadRow()) {
	 $retval = $sql->RowData[0];
  }
  
  return ($retval);
}

/**
 * - check PLUGIN role right
 - user root gets allways "execute" !!!
 * @param  $sqlo
 * @param  $plugin_name : the function-name without "p." e.g. "p98/o.CYTORD.take"
 * @param  $user_id  : optional user_id, otherwise $_SESSION['sec']['db_user_id']
 * @return string "execute" or "deny" or "" [means deny ?]
 */
function role_check_p ( &$sqlo, $plugin_name, $user_id = NULL ) {
	

	$retval = "";

	if ( glob_isAdmin() ) {
		return ("execute");
	}
	if ( !$user_id ) $user_id = $_SESSION['sec']['db_user_id'];

	$sqlo->query("SELECT cct_right FROM user_right WHERE name = ". $sqlo->addQuotes('p.'.$plugin_name).
			" AND user_right_id IN ".
			"(SELECT user_right_id FROM right_in_role WHERE role_id IN ".
			"(SELECT role_id FROM user_roles WHERE db_user_id = ".$user_id."))");
	if ($sqlo->ReadRow()) {
	 $retval = $sqlo->RowData[0];
	}

	return ($retval);
}


/**
 * check, if right exists
 * @param object $sqlo
 * @param string $plugin_name
 * @return int 0,1 - exists ?
 */
function role_plugin_right_exists ( &$sqlo, $plugin_name ) {
	
	$retval = 0;
	$sqlo->Quesel("cct_right FROM user_right WHERE name = ". $sqlo->addQuotes('p.'.$plugin_name) );
	if ($sqlo->ReadRow()) {
		if ($sqlo->RowData[0]=='execute') $retval=1;
	}

	return ($retval);
}


// FUNCTION: get the sql string after SELECT for searching a users of a role
function role_f_get_users ($funcname // the function-name without "f." e.g. "PaXml_import"
	) {
	$sqlsel = "distinct(uro.db_user_id) FROM user_right ur, right_in_role rir, user_roles uro ".
			"WHERE ur.name = 'f.".$funcname."' ".
			"AND ur.user_right_id=rir.user_right_id ".
			"AND rir.role_id=uro.role_id ";
	return ($sqlsel);
}

/**
 * get the sql string for searching a users of a role-right
 * @param  $tablename
 * @param  $right
 */
function role_o_get_users (
	$tablename, // e.g. "ABSTRACT_ARRAY"
	$right		// e.g. "read"
	) {
	
	$sqlsel = "distinct(uro.db_user_id) FROM user_right ur, right_in_role rir, user_roles uro ".
			"WHERE ur.name = 'o.".$tablename."' and ur.CCT_RIGHT='".$right."' ".
			"AND ur.user_right_id=rir.user_right_id ".
			"AND rir.role_id=uro.role_id ";
	return ($sqlsel);
}

