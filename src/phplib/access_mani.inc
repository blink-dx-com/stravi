<?
/**
 * - Contains functions for BO access manipulation
 * - started: 20010823
 * @package access_mani.inc
 * @author  Adrian Ciocildau, Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @var $rights_STRUCT: same struct like access_getInsRights()
 * 	   array("select"=>1, "update"=>1, "delete"=>1, "insert"=>1, "entail"=>1);
 * errors defined here:
      access_rights 100 could not set rights(...) because of restrictions of entail-right (warning level 3)
      access_rights 101 user not allowed to set/change rights
      access_rights 102 got error from set access rights
 */

require_once("access_check.inc");


/**
 * get column names for CCT_ACCESS_RIGHTS
 * @param array  nicerights_STRUCT $nicerights
 * @return $colnames
 */
function access__getColNames($nicerights) {
	
	if (!sizeof($nicerights)) return NULL;
	
	$transarr = array('read'=>'select', 'write'=>'update');
	reset($nicerights);
	$colnames = NULL;
    foreach( $nicerights as $dummy=>$nice_name) {
    	if ($transarr[$nice_name]!=NULL) $newname=$transarr[$nice_name];
    	else $newname=$nice_name;
        $colnames[]= $newname.'_right';
    }
    reset ($nicerights);
    return $colnames;
}


/**
 * remove cct_access_rights for (CCT_ACCESS_ID=$cct_access_id AND GROUP_ID=$group_id)
 * @param object $db_h
 * @param int $cct_access_id cct_access_id of BO
 * @param int $group_id for group id 
 * @param number $checkAccess flag: check, if access should be checked
 */
function access_del_group(  object &$db_h,  int $cct_access_id, int $group_id, $checkAccess =0 ) {
	$db_h->Deletex( 'CCT_ACCESS_RIGHTS', 'cct_access_id='.$cct_access_id." AND user_group_id=".$group_id);  
}

/**
 * set/delete right entry for one GROUP
 * attention: does not check, if the user is alloed to change the rights!!!
 * @param object $db_h db_access handle
 * @param int   $cct_access_id cct_access_id of BO
 * @param array  $rights new rights see $rights_STRUCT
 * @param int    $group_id
 * @param int    $f_del_group  flag: 0,1:  delete rights for group, even if rights are set
 * @return -
 */
function access_write( 
	  &$db_h,         
	  $cct_access_id, 
	  $rights,        
	  $group_id,
	  $f_del_group=0 
	  ) {
  
    
    $FUNCNAME="access_write";

    $error  = & ErrorHandler::get();
    
    if (empty($rights)) return; // do nothing if nothing to do

    $error->reset(); // make sure that no old errors interfere with this function

    if (!$f_del_group) { // check if all rights are not set, then also delete entry
        $cnt_rights = array_count_values($rights);
        $only_set_to_no_right = ((count($cnt_rights) == 1) && isset($cnt_rights[0]));
        $f_del_group = ($only_set_to_no_right && (count($rights) == 5));
        //                                          ^-- all 5 rights have to be in array otherwise do not automatically delete
    }
    
    if ($f_del_group) { // delete the rights
        $db_h->Deletex( 'CCT_ACCESS_RIGHTS', "cct_access_id =$cct_access_id AND user_group_id = $group_id");
        if ($error->got(READONLY)) return;
    } else { // try update first 
	
	
        $mquery = "";
		$tmpkomma = "";
        reset($rights);
        foreach( $rights as $r_name=>$r_val) {
            switch ($r_name) {
            	case "read" : $r_name = "select"; break;
            	case "write": $r_name = "update"; break;
            }
            $mquery  .= $tmpkomma . $r_name.'_right = '.$r_val;
			$tmpkomma = ", ";
        }
        reset ($rights);
        
        
        // check, if entry exists
        $db_h->Quesel('cct_access_id from CCT_ACCESS_RIGHTS where cct_access_id='.$cct_access_id.' and user_group_id='.$group_id);
        $db_h->ReadRow();
        $has_entry = $db_h->RowData[0];       
		
        $db_h->Updatex("CCT_ACCESS_RIGHTS",  $mquery." WHERE cct_access_id = $cct_access_id AND user_group_id = $group_id");	
        if ($error->got(READONLY)) return;

        if (!$has_entry && !$only_set_to_no_right) { 
            
            // no update because entry not yet existing --> do insert
            $inscols = "cct_access_id, user_group_id";
            $mquery2 = "$cct_access_id, $group_id";
            
            foreach($rights as $r_name => $r_val) {
                switch ($r_name) {
                case "read":  $r_name = "select"; break;
                case "write": $r_name = "update"; break;
                }
                $inscols .= ", ".$r_name."_right";
                $mquery2 .= ", $r_val";
            }
            $db_h->Insertx("CCT_ACCESS_RIGHTS", $inscols, $mquery2, $FUNCNAME);
            if ($error->got(READONLY)) return;	
        }
    }
}

/**
 * set/delete right entry for one GROUP, check perimission
 * @param $sql
 * @param $cct_access_id ... cct_access_id of BO
 * @param array $rights      ... new rights as array see $rights_STRUCT
 * @param $group_id    ... group id of rights
 * @param $f_del_group ... flag: delete rights for group, even if rights are set
 * @param $table
 * @return -
 * # errors:
       db_access       2 error in query
       access_rights  10 business-object with given cct_access_id does not exist
       access_rights 100 could not set rights(...) because of restrictions of entail-right (warning level 3)
       access_rights 101 user not allowed to set rights
       access_rights 102 got error from set access rights
 */
function access_write_with_check(  
	&$sql,    
	$cct_access_id,     
	&$rights,        
	$group_id,     
	$f_del_group=0,
	$table=NULL
	) {

  

  $error        = & ErrorHandler::get();
  $access_allow = access__allowed_to_chmod($sql, $cct_access_id, $table);
  if ($error->got(READONLY)) return;

  switch ($access_allow) {
	case "no":
		$error->set('access_rights', 101, 'Not allowed to set rights.');
		break;
	case "yes":
		access_write($sql, $cct_access_id, $rights, $group_id, $f_del_group);
		break;
	case "entail": // all allowed
		access_write($sql, $cct_access_id, $rights, $group_id, $f_del_group);
		break;
	default: htmlfoot("ERROR", "programming-error in access_write_with_check");
  }
}


// delete all rights from cct_access_id
function access_delAll( &$sql, $cct_access_id) {
	$sql->Deletex( 'CCT_ACCESS_RIGHTS', "cct_access_id=".$cct_access_id );	
}

// remove all manipulation rights of an object(also entail-right!)
function access_upDelMani(&$sqlo, $cct_access_id) {
	$maniRights = access_getManiRights();
	$colnames   = access__getColNames($maniRights);
	reset ($colnames);
	$komma =NULL;
	$mquery=NULL;
	foreach( $colnames as $dummy=>$colname) {
		$mquery .= $komma. $colname.'=0';
		$komma=', ';
	}
	reset ($colnames);
	$sqlo->Updatex("CCT_ACCESS_RIGHTS",  $mquery." WHERE cct_access_id =".$cct_access_id);
}
