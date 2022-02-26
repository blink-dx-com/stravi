<?php
/**
 * manage insert of objects or elements to database
 * @namespace core::
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com), Mac
 * @var VARSTRUCT $accRights = array(GROUP_ID => accArray) : array of access rights
 * 		accArray: array("select"=>1, "update"=>1, "delete"=>1, "insert"=>1, "entail"=>1)
 * @var VARSTRUCT $ins_opt_STRUCT = NULL (compare also with glob.obj.update.inc:upd_opt_STRUCT) <pre>
	"types" = array [COL_NAME] => type 
			    "DATE1" : value in format: YYYY-MM-DD HH24:MI:SS
				"TIME"  : Unix-time-stamp
	"touchmom" = 0|1 : touch mother object, if it is an assoc-object
	--- only for insert_row() ---
	"rights" = VARSTRUCT $accRights
	"db_user_id" : alternative $_SESSION['sec']['db_user_id']
	"touch" : {'warning': STRING } -- add touch with warnings
	</pre>
 * @version $Header: trunk/src/phplib/insert.inc 59 2018-11-21 09:04:09Z $
 * @Unittest exists
 */

require_once('access_check.inc');
require_once 'glob.obj.touch.inc';

/**
 * get the group-umasks for a user (mainly for GUI activities)
 * - do not show SINGLE user groups
 * - do not show INACTIVE groups
 * @return array:
    user_group_id => value-array:
 *  ['user_in_group'] => 1/0
 *  ['active'] => 1/0
 *  ['select'] => 1/0
 *  ['update'] => 1/0
 *  ['insert'] => 1/0
 *  ['delete'] => 1/0
 *  ['entail'] => 1/0
 */
function userGroupsGet(  &$sql,    $user_id) {

  $ret = array();

  $sql->query('SELECT var_name, value FROM user_pref WHERE db_user_id = '.$sql->addQuotes($user_id).' AND var_name LIKE \'umask.%\'');
  while($sql->ReadRow()) {
    $gid = substr($sql->RowData[0], 6);
    $ret[$gid] = array();
    $ret[$gid]['active'] = (bool)strstr($sql->RowData[1], 'active');
    $ret[$gid]['select'] = (bool)strstr($sql->RowData[1], 'select');
    $ret[$gid]['insert'] = (bool)strstr($sql->RowData[1], 'insert');
    $ret[$gid]['update'] = (bool)strstr($sql->RowData[1], 'update');
    $ret[$gid]['delete'] = (bool)strstr($sql->RowData[1], 'delete');
    $ret[$gid]['entail'] = (bool)strstr($sql->RowData[1], 'entail');
    $ret[$gid]['user_in_group'] = 0;
  }

  $found_groups = array_keys($ret);
  


  // do not show SINGLE_USER groups ...
  $sql->query( "SELECT ug.user_group_id FROM db_user_in_group ug, user_group g ".
  	" WHERE ug.db_user_id = ".$sql->addQuotes($user_id). 
	" AND   ug.user_group_id=g.user_group_id AND (g.SINGLE_USER is NULL or g.SINGLE_USER <= 0)"
    );
  while($sql->ReadRow()) {
    if (in_array($sql->RowData[0], $found_groups)) {
      $ret[$sql->RowData[0]]['user_in_group'] = 1;
    } else {
      $ret[$sql->RowData[0]] = array();
      $ret[$sql->RowData[0]]['active'] = 0;
      $ret[$sql->RowData[0]]['select'] = 0;
      $ret[$sql->RowData[0]]['insert'] = 0;
      $ret[$sql->RowData[0]]['update'] = 0;
      $ret[$sql->RowData[0]]['delete'] = 0;
      $ret[$sql->RowData[0]]['entail'] = 0;
	  $ret[$sql->RowData[0]]['user_in_group'] = 1;
    }
  }
  
  
  // unset INACTIVE groups
  foreach($found_groups as $one_group) {
  	$inactive = glob_elementDataGet( $sql, 'USER_GROUP', 'USER_GROUP_ID', $one_group, 'INACTIVE');
  	if ($inactive>0) {
  		// remove inactive group
  		unset($ret[$one_group]);
  		$index = array_search($one_group, $found_groups);
  		unset($found_groups[$index]);
  	}
  }
  
  return $ret;
}

/**
 * - get the active group-umasks for a user from his settings in table USER_PREF VAR_NAME LIKE 'umask.%'
 * - check, if groups exist
 * - check, if group is active
 * - get also the default SINGLE_USER-group, if exists
 * - TBD: build singleton-methods to check existance of groups in USER_PREF only once per insert!!!
 * @param  object $sql
 * @param  int $user_id
 * @global $_SESSION['globals'][o.DB_USER.".$user_id.".mygroup]
 * @return  array: <pre>
 *     user_group_id => value-array:
 * 		         ['select'] => 1/0
 * 		         ['update'] => 1/0
 * 		         ['insert'] => 1/0
 * 		         ['delete'] => 1/0
 * 		         ['entail'] => 1/0
 */
function userActiveGroupsGet( &$sqlo, $user_id) {
  
  
  $ret = NULL;
  // get preferences of user
  $sqlo->query("SELECT var_name, value FROM USER_PREF WHERE db_user_id = ".$sqlo->addQuotes($user_id)." AND VAR_NAME LIKE 'umask.%'");
  while($sqlo->ReadRow()) {
  	$rig_mask = $sqlo->RowData[1];
    if (strstr($rig_mask, 'active') === false) continue;

    $gid = substr($sqlo->RowData[0], 6);
    $ret[$gid] = array();

    $ret[$gid]['select'] = strstr($rig_mask, 'select') ? 1 : 0;
    $ret[$gid]['insert'] = strstr($rig_mask, 'insert') ? 1 : 0;
    $ret[$gid]['update'] = strstr($rig_mask, 'update') ? 1 : 0;
    $ret[$gid]['delete'] = strstr($rig_mask, 'delete') ? 1 : 0;
    $ret[$gid]['entail'] = strstr($rig_mask, 'entail') ? 1 : 0;
  }
  
  if ($ret!=NULL) {
  	
  	// check, if groups exist and is active; unset group, if not exists ...
  	$keys = array_keys($ret);
  	foreach( $keys as $groupid) {
  		$sqlo->Quesel('1 from USER_GROUP where USER_GROUP_ID='.$groupid. ' and (INACTIVE is NULL or INACTIVE<=0)');
		if ( !$sqlo->ReadRow() ) {
			unset( $ret[$groupid]);
		}
	}
	reset ($keys);
  }
  
  if ( $_SESSION['globals']["security_write"] ) {
	$usrGrpId = access_getPersGrp ($sqlo, $user_id);
	if ($usrGrpId>0) { // positive value ???
		if ( !isset($ret[$usrGrpId]) ) $ret[$usrGrpId] = access_getInsRights();
	}
		
  }
  
  return $ret;
}

/**
 * touch mother object of an assoc element!
 */
function insert_touchmom(&$sql, $tablename, &$argu) {
	
	$mothertab = mothertable_get2($tablename);
	if ($mothertab=="") return;
	$pk_name = PrimNameGet2($tablename);
	$pk_val  = $argu[$pk_name];
	if ($pk_val=="") return;
	
	$actarr = array('d'=>array('x'=>array('ass'=>array('t'=>$tablename))  ), 'a'=>array('key'=>'new') );
	globObjTouch::touch( $sql, $mothertab, $pk_val, $actarr ); 
}

/**
 * get SQL string for insert row
 * - params see insert_row_s
 * @param array $options
 *    "types"
 *    'prim_name'
 */
function insert_row_Get( &$sql, $tablename, $argu,	$options=NULL ) {
    if ($options['prim_name']) $prim_name = $options['prim_name'];
	else $prim_name = PrimNameGet2($tablename);  // get most important primary key
	
	$sqls      = 'VALUES (';
	$sql_cols  = '(';
	$i         = 0;
	
	if (empty($prim_name)) {
		sys_error_my('Primary key name not defined for table: '.$tablename.'.');
		exit;
	}
	
	
	foreach( $argu as $xcol=>$xval) {
	
		if ($i) {// beim ersten val KEIN komma
		  $sqls     .=', ';
		  $sql_cols .=', ';
		}
		if (($prim_name === $xcol) &&  !$xval){
			$xval = ''; // make 0 to NULL for triggers
		}	 
		$sql_cols .= $xcol;
		if ( $xval === "" or  $xval === NULL) $tmpvalSql = 'NULL';
		else {
			$tmpvalSql = $sql->addQuotes($xval);
		
			if ($options["types"]!="") {
				$xtype=$options["types"][$xcol];
				if ($xtype!="") {
					if ($xtype=="DATE1") {
						$tmpvalSql = $sql->DateString2Sql($xval,1);
					}
					if ($xtype=="TIME") {
						$tmpvalSql = $sql->Timestamp2Sql ( $xval ); 
					}
				}
			}
		}
		
		$sqls .= $tmpvalSql;
		$i++;
	}
	
	
	$sqls = $sql_cols.') '.$sqls.')'; // assemble whole query
	return ($sqls);
}

/**
 * - insert row without checking for CCT_ACCESS (see also insert_row)  
 * - ! for ASSOC elements: use class fAssocUpdate()
 * - does touch()
 * @param &$sql,       
 * @param string $tablename db-name of table for insert (in capital letters)  
 * @param array $argu       array ("COLUMN_NAME" => value, ...) (column_names in capital letters!)
 * @param array ins_opt_STRUCT $options
 *     prim_name -- extra optional the PKNAME
 * @return int >0 or "text": OK -> this is the primary key
 *	           =0 error
 */
function insert_row_s(&$sql, $tablename, $argu, $options = NULL ) {
    if ($options['prim_name']) $prim_name = $options['prim_name'];
    else $prim_name = PrimNameGet2($tablename);
    
    if($prim_name==NULL) $prim_name = PrimNameGet2($tablename);
    $sqls   = insert_row_Get( $sql, $tablename, $argu, $options );
    $retval = $sql->queryInsert($tablename, $sqls, $prim_name); // do insert
    if ( $options["touchmom"] ) insert_touchmom($sql, $tablename, $argu);
    return ($retval);
}

/**
 * create an CCT_ACCESS entry + group rights
 * @param object $sql
 * @param string $tablename
 * @param array  $options   <pre>
 *   "rights" =  VARSTRUCT $accRights
 *               if a right is not set: default : 0
 *   "db_user_id" : alternative user_id for owner
 *   "touch" : {'warning': STRING } -- add touch with warnings
 * @return number $access_id
 */
function insert_access( &$sql, $tablename, $options=NULL ) {
	
	
	$dest_user_id = $_SESSION['sec']['db_user_id'];
	if ($options["db_user_id"]>0)  $dest_user_id = $options["db_user_id"];
	
    //OLD:  $access_id = '0'; // only for TRIGGERED database
    $access_id = $sql->queryInsert("CCT_ACCESS", '( db_user_id, crea_date, table_name) '.
		'VALUES ('.$dest_user_id.', '.$sql->Timestamp2Sql().', '.$sql->addQuotes($tablename).')','cct_access_id');
    if (!$access_id) {
		return 0; 
	}

    // assign to active groups for group-rights
    if ( is_array($options) && array_key_exists( "rights", $options) ) { // can also contain NULL, to deny group rights
        $groups = $options["rights"];
    } else {
        $groups = userActiveGroupsGet($sql, $dest_user_id);
    }
	
    if ( !empty($groups) ) {
		$defRights = access_getInsRights();
        foreach( $groups as $group_id=>$rig) {
			foreach( $defRights as $key=>$dummy) { // for all defined features ...
				if ( $rig[$key]=="" )  $rig[$key]="0";     // set to a defined value !!!
			}
			reset ($defRights);
        	$sql->query('INSERT INTO cct_access_rights (cct_access_id, user_group_id, select_right, insert_right, update_right, delete_right, entail_right) '.
				  'VALUES ('.$access_id.', '.$group_id.', '.$rig['select'].', '.$rig['insert'].', '. $rig['update'].', '.$rig['delete'].', '.$rig['entail'].')');
        }
    }
	
	return ($access_id);
}

/**
 * insert row including creation of CCT_ACCESS and group-rights
 * @param $tablename ... db-name of table for insert (in capital letters)
 * @param $argu      ... array ("COLUMN_NAME" => value, ...) (column_names in capital letters!)
                       use argu['PRIM_KEY_ID'] = 0  for new auto-generated primary key
 * @param array ins_opt_STRUCT $options + extra_keys: "rights", "db_user_id"	
              
 * @return int  PK of object
 *   > 0 :  OK (can also be a string for CCT_TABLE?)
 *     0 : ERROR
 */
function insert_row(  
	&$sql,       
	$tablename,      
	$argu, 
	$options = NULL 
	) {

  
  //$retVal = 0; // default: error

  $old_autocommit_val = $sql->SetAutoCommit(false);
  
  if (cct_access_has2($tablename)) { // check if cct_access_id
  	$accopt = NULL;
    If (isset($options["rights"])) $accopt["rights"] = $options["rights"];
	if ($options["db_user_id"]>0)  $accopt["db_user_id"] = $options["db_user_id"];
    $access_id = insert_access( $sql, $tablename, $accopt);
	if (!$access_id) {
		$sql->Rollback();
		$sql->SetAutoCommit($old_autocommit_val);
		return 0;
	}
    $argu['CCT_ACCESS_ID'] = $access_id;
	
  }
  
  $ret = insert_row_s($sql, $tablename, $argu, $options);
  if (!$ret) { // got error
	$sql->Rollback();
  } elseif($old_autocommit_val == true){ // commit only when auto-commmit was on otherwise leave uncommitted
	$sql->Commit();
  }
  $sql->SetAutoCommit($old_autocommit_val);
  return $ret;
}   

/**
 *  insert object and check also role rights
 */
function insert_row_role(  &$sql, $tablename, $argu, $options = NULL ) {    

    $error = & ErrorHandler::get();
    
    $t_rights = tableAccessCheck( $sql, $tablename );
    
    if ( $t_rights["insert"] != 1 ) {
        $error->set('insert_row_role', 1, 'no role right to insert an object in "'.$tablename.'"!'); 
        return 0;
    }
    return ( insert_row( $sql, $tablename, $argu, $options ) );
    
}

