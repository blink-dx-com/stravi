<?php
require_once ("role.inc");
require_once ('f.update.inc');
require_once ('insert.inc');

/**
 * sub function for table USER_PREF2 and $_SESSION['userGlob']
 * @package o.USER_PREF2.subs.inc
 * @swreq UREQ:
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oUSER_PREF2_subs {

	const table='USER_PREF2';
	
	static function my_get_key_val(&$sqlo, $key) {
	    
	    $result = glob_elemDataGet3( $sqlo, self::table, array('KEY'=>$key ,
	        'DB_USER_ID'=>$_SESSION['sec']['db_user_id']), array('VALUE') );
	    if ($result['VALUE']===NULL) return;
	    
	    $out = json_decode($result['VALUE'], TRUE);
	    return $out;
	}
	
	static function my_get_by_id(object &$sqlo,  int $pref_id) {
	    $result = glob_elemDataGet3( $sqlo, self::table, array('USER_PREF2_ID'=>$pref_id ,
	        'DB_USER_ID'=>$_SESSION['sec']['db_user_id']), array('VALUE') );
	    if ($result['VALUE']===NULL) return;
	    $out = json_decode($result['VALUE'], TRUE);
	    return $out;
	}
	
	/**
	 * does NOT test uniqunes of KEY !
	 * @param object $sqlo
	 * @param string $key
	 * @param array $value
	 * @return number
	 */
	static function my_insert_entry( &$sqlo, string $key, $value) {
	    $argu=array('VALUE'=>json_encode($value), 'DB_USER_ID'=>$_SESSION['sec']['db_user_id'], 'KEY'=>$key);
	     $pref_id =  insert_row_s($sqlo, self::table, $argu);
	     return $pref_id;
	}
	static function my_update_by_id( &$sqlo, int $pref_id, $value) {
	    $idarr=array('USER_PREF2_ID'=>$pref_id , 'DB_USER_ID'=>$_SESSION['sec']['db_user_id']);
	    $argu =array('VALUE'=>json_encode($value));
	    gObjUpdate::update_row_s( $sqlo, self::table, $argu,$idarr);
	}
	
	
	/**
	 * INSERT/UPDATE one key/value pair of a USER
	 * 
	 * @param object $sqlo
	 * @param int $user_id
	 * @param string $key
	 * @param string $value
	 */
    static function user_update_key( &$sqlo, int $user_id, string $key, $value) {

	    $sqlsel = 'VALUE FROM USER_PREF2 WHERE DB_USER_ID=' . $sqlo->addQuotes($user_id). ' and KEY='.$sqlo->addQuotes($key);
	    $sqlo->Quesel($sqlsel);
	    if ($sqlo->ReadRow()) {
	       // $sqlo->RowData[0];
	       
           $argu= array('VALUE'=>json_encode($value) );
           gObjUpdate::update_row_s($sqlo, self::table, $argu, array('DB_USER_ID'=>$user_id, 'KEY'=>$key));
	       
	    } else {
	        if ($value==='' or $value===NULL) {
	            // nothing 
	        } else {
	           $argu=array('VALUE'=>json_encode($value), 'DB_USER_ID'=>$user_id, 'KEY'=>$key);
	           insert_row_s($sqlo, self::table, $argu);
	        }
	    }
	    
	}
	
	/**
	 * INSERT/UPDATE one key/value pair of LOGGEDIN user
	 * @param object $sqlo
	 * @param string $key
	 * @param string $value
	 */
	static function my_update_key_val(&$sqlo, $key, $value) {
	    self::user_update_key($sqlo, $_SESSION['sec']['db_user_id'], $key, $value);
	}

	
	/**
	 * remove on VAR from ALL users
	 * @param object $sqlo
	 * @param string $varname
	 */
	static function remove_all_of_user($sqlo, int $user_id) {
	    $sqlwhere = 'DB_USER_ID=' . $sqlo->addQuotes($user_id);
	    $sqlo->deletex(self::table, $sqlwhere);
	}

}
