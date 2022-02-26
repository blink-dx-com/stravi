<?php
/**
 * Contains analysis functions for table right and BO checking
 * - for manipulation of rights use: access_mani.inc
 * @package access_check.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @global  $_SESSION['globals']["security_write"]
 * 
 * @var nicerights_STRUCT = array("read", "write", "delete", "insert", "entail")
 *      nice representation of object-access rights (e.g. "read" will be translated to "SELECT_RIGHT" in CCT_ACCESS_RIGHTS)
 */

require_once("globals.inc");
require_once("table_access.inc");


// return all access rights
function access_getRights () {
	return array("read", "write", "delete", "insert", "entail");
}

/** 
 * return all manipulation-right-names
 * @return array $rights nicerights_STRUCT 
 */
function access_getManiRights () {
	return array("write", "delete", "insert", "entail");
}

/** 
 * return all access-right-names (names like in table CCT_ACCESS_RIGHTS) for insert.inc ...
 * @return array ($rights => active (0,1) )
 */
function access_getInsRights () {
	return array("select"=>1, "update"=>1, "delete"=>1, "insert"=>1, "entail"=>1);
}



/**
 * get the SUMMARY rights for one user of one object
 		  - by default the READ-flag == 1
		  - when $_SESSION['globals']["security_level"] == "select_on" => analyse the READ-flag
 * @param object $sql
 * @param string $tablename
 * @param int $prim_id
 * @param int $cct_access_id // optional, if you know it, you can save some db-calls
 * @param array $opt "useReadFlag" 0,1 -- get the real READ-flag from cct_access_rights,
					                  even if $_SESSION['globals']["security_level"] != "select_on"
 * @return array $rights nicerights_STRUCT 
 */
function access_check ( 
	&$sql, 
	$tablename, 
	$prim_id, 
	$cct_access_id=0, 
	$opt = NULL 	  
	) {



  if (glob_isAdmin()) {
    return array("read"=>1, "write"=>1, "delete"=>1, "insert"=>1, "entail"=>1);
  }
  
  $tmp_useReadFlag = 0;
  if ( ($_SESSION['globals']["security_level"] == "select_on") OR ($opt["useReadFlag"]>0) ) $tmp_useReadFlag = 1;
  $tmp_mothertable = ""; // no mother table
  $o_rights = array("read"=>1, "write"=>0, "delete"=>0, "insert"=>0, "entail"=>0);  // do NOT change the order!
         // right to read,      update,     delete,      insert or    entail an element

  
  if ($cct_access_id == 0) {
    if (empty($prim_id)) return $o_rights;
	$cct_acces_has = cct_access_has2($tablename);

	if ($cct_acces_has) { 
	  $mothertable = $tablename;
	} else { // has mother table?
	  if ($tmp_mothertable = mothertable_get2($tablename)) { // mother has => cct_access has!
		$mothertable=$tmp_mothertable;
		$cct_acces_has = cct_access_has2($mothertable);
	  }
	} // else: it is an object in a system table
	
	if ( $cct_acces_has ) {
	  $cct_access_id=cct_access_get($sql, $mothertable, $prim_id);
	  if (empty($cct_access_id)) $cct_access_id = 0;
	} else {
	  return $o_rights;
	}
  }
  
  $sql_tmp= "SELECT 1 FROM cct_access a WHERE cct_access_id = $cct_access_id AND  db_user_id = ".$_SESSION['sec']['db_user_id'];
  $sql->query($sql_tmp , "access_check:1"); 
  $isowner = $sql->ReadRow(); // if one row read: user is owner
  
  if ($isowner AND !$_SESSION['globals']["security_write"])  {  // ??? owner of object AND no special DB write_level
	return array("read"=>1, "write"=>1, "delete"=>1, "insert"=>1, "entail"=>1);
  } 
  
  
    // check for group rights (e.g. if security_write==1)
	$sql->query("SELECT MAX(select_right), MAX(update_right), MAX(delete_right), MAX(insert_right), MAX(entail_right) ".
				"FROM cct_access_rights WHERE cct_access_id = $cct_access_id AND user_group_id IN ".
				"(SELECT user_group_id FROM db_user_in_group WHERE db_user_id = ".$_SESSION['sec']['db_user_id'].")", "access_check:2");
	$sql->ReadRow();
	
	if ($tmp_useReadFlag) {
		$o_rights["read"] = $sql->RowData[0];
	}	
	if ($tmp_mothertable=="") {  // default BO
		$o_rights["write"]  = $sql->RowData[1];
		$o_rights["delete"] = $sql->RowData[2];
		$o_rights["insert"] = $sql->RowData[3];
		$o_rights["entail"] = $sql->RowData[4];
	} else {  // an associated elements translates the "insert" right of the BO
				// to the "insert", "write", "delete" right of the assoc element
		$o_rights["write"]  = $sql->RowData[3];
		$o_rights["delete"] = $sql->RowData[3];
		$o_rights["insert"] = $sql->RowData[3];
		$o_rights["entail"] = $sql->RowData[4];
	}
  
  return $o_rights;
}

/**
 * get 'single user group' of user $user_id
 * - TBD: use CACHE ???
 * @param object $sql
 * @param int $user_id
 * @return $group_id
 */
function access_getPersGrp (&$sql, $user_id) {
	
	
	$group_id = 0;
	$user_key = "o.DB_USER.".$user_id.".mygroup";
	if ($_SESSION['globals'][$user_key]=="") { // TBD: o.DB_USER.mygroup is cached ???
		$sql->query("SELECT USER_GROUP_ID FROM USER_GROUP WHERE db_user_id=".$user_id. " AND SINGLE_USER=1");
		if ( $sql->ReadRow() ) {
			$group_id = $sql->RowData[0];
			$_SESSION['globals'][$user_key] = $group_id;
		} else {
			$_SESSION['globals'][$user_key] = "0"; // no entry in database, set cache to NOT_EXIST
		}
	} else {
		$group_id = $_SESSION['globals'][$user_key];
	}
	return ($group_id);
}

/**
 *  get the access rights of the object for ONE group
 *  @return $o_rights
 */
function access_ch_group ( &$sql, $tablename, $prim_id, $group_id ) {	
  $sql->Query("SELECT select_right, update_right, delete_right, insert_right, entail_right FROM cct_access_rights WHERE cct_access_id IN ".
			  "(SELECT cct_access_id FROM $tablename WHERE ".PrimNameGet2($tablename)." = $prim_id) AND user_group_id = $group_id",
			"access_ch_group");
  $sql->ReadRow();
  $o_rights=array();
  $o_rights["read"] = $sql->RowData[0];
  $o_rights["write"] = $sql->RowData[1];
  $o_rights["delete"] = $sql->RowData[2];
  $o_rights["insert"] = $sql->RowData[3];
  $o_rights["entail"] = $sql->RowData[4];
  return $o_rights;
}

/**
 *  tests, if the user is allowed to read a table-entry (distinguishes between normal 
 *  & high security version of partisan)
 *  @return boolean 0,1
 */
function access_reading_allowed(  &$sql, $tablename, $prim_id) {
  

  if (array_key_exists("security_level", $_SESSION['globals']) && ($_SESSION['globals']["security_level"] == "select_on")) {
    $acc = access_check ($sql, $tablename, $prim_id);
    return $acc["read"];
  } // otherwise no high security --> reading always allowed
  return true;
}

/**
 * get all groups of an CCT_ACCESS_ID
 * @param $sql
 * @param $cct_access_id
 * @return array (grpid=>1)
 */
function access__getGroups(&$sql, $cct_access_id) {
	$groups = NULL;
	$sqls = "SELECT user_group_id FROM cct_access_rights WHERE cct_access_id =".$cct_access_id;
	$sql->Query($sqls);
	while ( $sql->ReadRow() ) {
		$groups[$sql->RowData[0]]=1;
	}
	return ($groups);
}


/**
 * get info, if user is part of a group in right-mask
 * @param  $sql
 * @param  $cct_access_id
 * @return $isReflected 0,1
 */
function access_userIsReflected( &$sql, $cct_access_id) {
	
	
	$isReflected=0;
	$sqlUsrGrps = "SELECT USER_GROUP_ID FROM DB_USER_IN_GROUP WHERE DB_USER_ID=" . $_SESSION['sec']['db_user_id'];  // all groups of user
	$sqls = "SELECT 1 FROM CCT_ACCESS_RIGHTS ".
		    " WHERE CCT_ACCESS_ID =".$cct_access_id. " and USER_GROUP_ID in (".$sqlUsrGrps.")";
	$sql->Query($sqls);
	if ( $sql->ReadRow() ) {
		$isReflected=1;
	}
	return ($isReflected);
}


/**
 * get all groups + rights of an CCT_ACCESS_ID
 * @return $g_rights array( group_id => nicerights_STRUCT )
 */
function access_getAllRights( &$sql, $cct_access_id ) {
	
	$g_rights = NULL;
	$sql->Query( 
		"SELECT user_group_id, select_right, update_right, delete_right, insert_right, entail_right FROM cct_access_rights ".
		"WHERE cct_access_id=".$cct_access_id );
	while ( $sql->ReadRow() ) {
		$o_rights=NULL;
		$o_rights["read"]   = $sql->RowData[1];
		$o_rights["write"]  = $sql->RowData[2];
		$o_rights["delete"] = $sql->RowData[3];
		$o_rights["insert"] = $sql->RowData[4];
		$o_rights["entail"] = $sql->RowData[5];	
		$g_rights[$sql->RowData[0]] = $o_rights;
	}
	return ($g_rights);
}

/**
 * get all groups + rights of an CCT_ACCESS_ID
 * right amsk from access_getAllRights must be converted
 * @return $g_rights array( group_id => VARSTRUCT_accRights )
 */
function access_getAllRightsIns( &$sqlo, $cct_access_id) {
	$rigths = access_getAllRights( $sqlo, $cct_access_id );
	if (!sizeof($rigths)) return;
	reset ($rigths);
	foreach( $rigths as $grpid=>$rigarr) {
		$rigths[$grpid]['update']=$rigarr['write'];
		$rigths[$grpid]['select']=$rigarr['read'];
		unset ($rigths[$grpid]['write']);
		unset ($rigths[$grpid]['read']);
	}
	reset ($rigths); 
	return $rigths;
}

/**
 * check, if $cct_access_id has manipulation rights
 * @param  $sql
 * @param  $cct_access_id
 * @return bool $rightsum : 0 - no, 1 - yes
 */
function access_hasManiRights( &$sql, $cct_access_id ) {
	$sql->Query( 
		"SELECT max(update_right),  max(delete_right),  max(insert_right),  max(entail_right) ".
		" FROM cct_access_rights ".
		"WHERE cct_access_id=".$cct_access_id, "access_hasManiRights" );
	$sql->ReadRow();
	$rightsum = $sql->RowData[0] + $sql->RowData[1] + $sql->RowData[2] + $sql->RowData[3];
	return ($rightsum);
}

/**
 * check, if RIGHT-MASK has manipulation rights
 * @param array $o_rights  array[RIGHT] = 0,1
 * @return  $rightsum : 0 - no,  1 - yes
 */
function access_isManiMask( $o_rights ) {	
    
	$maniarr = array("write", "delete", "insert", "entail");
	$rightsum = 0;
	foreach( $maniarr as $key) {
		$rightsum = $rightsum + $o_rights[$key];
	}

	return ($rightsum);
}

/**
 * check, if user has entail-right on object
 * @param  $sql
 * @param  $cct_access_id
 * @return int $hasEntail
 *   0 : no
 *   1 : yes
 */
function access__userHasEntail( &$sql, $cct_access_id ) {
	global $error;
    $error = & ErrorHandler::get();
    
	// now get the ENTAIL-info from cct_access_rights
    $sql->query("SELECT MAX(entail_right) FROM cct_access_rights WHERE cct_access_id = $cct_access_id AND user_group_id IN ".
                "(SELECT user_group_id FROM db_user_in_group WHERE db_user_id = ". $_SESSION['sec']['db_user_id'] .")" );
    if ($error->got(READONLY)) return;
    $sql->readRow();
    if ($sql->RowData[0])
        return  1;
    else return 0;
}

/**
 *  tests if the current user is allowed to set rigths for the business-object specified by its cct_access_id
 * @param $sql
 * @param $cct_access_id
 * @param $tablename
 * @return 
 * 	"no"     .. user not allowed to set rights for bo
 *  "yes"    .. user is allowed to set rights because he is root or (NO OHIPPO-Mode and he owns the object)
 *  "entail" .. user is allowed to add rights because he has the entail rights on the object
 * @errors 
         db_access      2 error in query
         access_rights 10 business-object with given cct_access_id does not exist
 */
function access__allowed_to_chmod(  &$sql,    $cct_access_id, $tablename=NULL) {
    global   $error;

    $error = & ErrorHandler::get();
	
	$ohippoFlag = $_SESSION['globals']["security_write"];
    if ($_SESSION['sec']['appuser'] == "root") return "yes";
    
    $sql->query("SELECT db_user_id FROM cct_access WHERE cct_access_id = $cct_access_id");
    if (!$sql->readRow()) {
        $error->set("access_rights", 10, "Business-object with cct_access_id = $cct_access_id does not exist.");
        return;
    }
	
	// is owner and system runs NOT in security_write_mode?
    if ( ($sql->RowData[0] == $_SESSION['sec']['db_user_id']) AND ($ohippoFlag<=0) ) {
		return "yes";
	}
	
	// check role-right "admin"
	if ($tablename!="") {
		$isAdmin = role_admin_check ( $sql, $tablename ); 
		if ($isAdmin) return "yes";
	}
 
    // now get the ENTAIL-info from cct_access_rights
    if ( access__userHasEntail( $sql, $cct_access_id )==1)
        return "entail";
        
    return "no";
}

/**
 * do combined access check of TABLE and OBJECT: 
 * throws error missing right
 * standard: READ-test
 * @param array $act 'tab' => array(rights)
 *					 'obj' => array(rights)
         leave 'tab' or 'obj' as empty array, if you do not want to check
 */
function access_tab_obj_rights(&$sqlo, $tablename, $id, $act=NULL) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    // table rights
    $t_rights = tableAccessCheck( $sqlo, $tablename );
    if ( is_array($act['tab']) ) $tRightArr = $act['tab'];
    else $tRightArr = array('read');
    
    foreach( $tRightArr as $rightx) {
        if ( $t_rights[$rightx] != 1 ) {
            $answer = getTableAccessMsg( $tablename, $rightx );
            $error->set( $FUNCNAME, 1, $answer );
            return;
        }
    }
    
    // object rights
    if (is_array($act['obj'])) $oRightArr = $act['obj'];
    else $oRightArr = array('read', 'write');
    
    if ( sizeof($oRightArr) ) {  // only when keys are available ...
        $o_rights = access_check($sqlo, $tablename, $id);
        $tabnice  = tablename_nice2($tablename);
        foreach( $oRightArr as $righto) {
            if ( !$o_rights[$righto] ) {
                $error->set( $FUNCNAME, 2, 'no "'.$righto.'" permission on this '.$tabnice.' object (ID:'.$id.')!');
                return;
            }
        }
        
    }
}

