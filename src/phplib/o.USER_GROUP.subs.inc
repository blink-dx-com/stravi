<?php
/**
 * USER_GROUP sub functions
 * @package o.USER_GROUP.subs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq SREQ:0003004: o.DB_USER, o.USER_GROUP > provide system methods
 * @unittest phplib/o.USER_GROUP.subs.inc
 */
require_once 'f.assocUpdate.inc';

/**
 * analyse user group
 */
 class oUSER_GROUP_subs {
 	
 	static function getUsersOfGroup(&$sqlo, $groupid) {
 		
 		$users=array();
	 	$sqlsel = "DB_USER_ID from DB_USER_IN_GROUP where USER_GROUP_ID=".$groupid. ' order by DB_USER_ID';
		$sqlo->Quesel($sqlsel);
		while ( $sqlo->ReadRow() ) {
		    $userid = $sqlo->RowData[0];
			$users[] = $userid;
		} 
		
		return ($users);
 	}

 	/**
 	 * - analyse if user is member of group
 	 * @param $sqlo
 	 * @param $userid
 	 * @param $groupid
 	 * @return $isMember (0,1)
 	 */
 	static function userIsMember(&$sqlo, $userid, $groupid) {
	 	$sqlsel = "DB_USER_ID from DB_USER_IN_GROUP where USER_GROUP_ID=".$groupid. ' and DB_USER_ID='.$userid;
		$sqlo->Quesel($sqlsel);
		if ($sqlo->ReadRow() ) {
			$isMember = 1;
		} else $isMember = 0;
		
		return ($isMember);
 	}
 	
 	static function getGroupsOfUser(&$sqlo, $userid) {
 	    $groups=array();
 	    $sqlsel = "USER_GROUP_ID from DB_USER_IN_GROUP where DB_USER_ID=".$userid. ' order by DB_USER_ID';
 	    $sqlo->Quesel($sqlsel);
 	    while ( $sqlo->ReadRow() ) {
 	        $grp_id = $sqlo->RowData[0];
 	        $groups[] = $grp_id;
 	    }
 	    return $groups;
 	}
 }
 
 /**
  * modify a USER group
  */
 class oUSER_GROUP_mod {
     
     private $modified;
     
     function __construct($sqlo, $user_group_id) {
         $this->user_group_id = $user_group_id;
 
         $this->assoclib = new fAssocUpdate();
         $this->assoclib->setObj( $sqlo, 'DB_USER_IN_GROUP', $this->user_group_id ); 
     }
     
     function add_user($sqlo, $user_id) {
         $argu=array('DB_USER_ID'=>$user_id);
         $this->assoclib->insert($sqlo, $argu);
     }
     
     function remove_user($sqlo, $user_id) {
         $idarr=array('DB_USER_ID'=>$user_id);
         $this->assoclib->delOneRow($sqlo, $idarr);
     }
     
     function close($sqlo) {
         $this->assoclib->close($sqlo);
     }
 }
 