<?php
/**
 * DB_USER sub methods 2
 * @namespace core::obj::DB_USER
 * @package o.DB_USER.subs2.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 
 */
class oDB_USER_sub2 {

    /*   int oDB_USER_sub2::userHomeProjGet(obj sql, int id) */
    static function userHomeProjGet(  &$sql, $id=0) {
        $sqls = "SELECT proj_id FROM db_user WHERE ";
        
        if ($id) $sqls .= "db_user_id = $id";
        else     $sqls .= "nick = ".$sql->addQuotes($_SESSION['sec']['appuser']);
        
        $sql->query($sqls);
        $sql->ReadRow();
        return $sql->RowData[0];
    }
    
    /**
     * return array of groups, where user is member
     * @param object $sql
     * @param int $user_id
     * @return array
     */
    static function groupListGet( &$sql, $user_id) {
        $grarr = null;
        $sql->query("SELECT g.user_group_id, g.name FROM db_user_in_group l, USER_GROUP g ".
            " WHERE l.db_user_id = ".$user_id." AND l.USER_GROUP_ID=g.USER_GROUP_ID order by g.NAME"  );
        while($sql->ReadRow()) {
            $grarr[$sql->RowData[0]] = $sql->RowData[1];
        }
        return($grarr);
    }  


    // return array of user roles ID=>NAME
    static function roleListGet(&$sql, $user_id) { 
 
        $role_arr=array();
        $sqls= "select u.ROLE_ID, r.NAME from user_roles u, ROLE r where u.db_user_id=".$user_id. " AND u.ROLE_ID=r.ROLE_ID order by r.NAME";
        $sql->query($sqls);
        while ($sql->ReadRow()) {
            $role_arr[$sql->RowData[0]] = $sql->RowData[1];
        }
        return($role_arr);
    }
    
    // has ROLE_ID ? : 0,1
    static function has_Role(object &$sql, int $user_id, int $role_id) {
        $has_role = 0;
        $sqls= "select u.ROLE_ID from user_roles u where u.db_user_id=".$user_id. " AND u.ROLE_ID=".$role_id;
        $sql->query($sqls);
        if ($sql->ReadRow()) {
            $has_role=1;
        }
        return $has_role;
    }
    
    /**
     * 
     * @param object $sql
     * @param int $user_id
     * @param int $group_id
     * @param string $umask or NULL
     *      if NULL: remove mask ...
     *      "active select update insert delete entail"
     * @param int $set_session_var update $_SESSION['userGlob']['umask..'] ???
     */
    static function writeUmask (  &$sql, $user_id, $group_id,  $umask, $set_session_var=false ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;

        if ($umask==NULL) {
            // remove ...
            
            $sql->Deletex("USER_PREF", "(db_user_id = $user_id AND var_name = 'umask.".$group_id."')");
            if ($set_session_var) unset($_SESSION['userGlob']['umask.'.$group_id]);  
            return;
        }
        
    	$sql->query("SELECT 1 FROM user_pref WHERE db_user_id = $user_id AND var_name = 'umask.$group_id'");
    	$sql->ReadRow();
    	if ($sql->RowData[0] == 1) {
    		$sql->Updatex("USER_PREF", "value = '$umask' WHERE db_user_id = $user_id AND var_name = 'umask.$group_id'");
    	} else {
    		$sql->Insertx("USER_PREF",  "db_user_id, var_name, value", "$user_id, 'umask.$group_id', '$umask'", $FUNCNAME );
    	}
    	if ($set_session_var) $_SESSION['userGlob']['umask.'.$group_id] = $umask;
        
    }
}

