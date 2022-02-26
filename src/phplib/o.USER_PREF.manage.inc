<?php
require_once ("role.inc");
require_once ('f.update.inc');
require_once ('insert.inc');

/**
 * sub function for table USER_PREF and $_SESSION['userGlob']
 * @package o.USER_PREF.manage.inc
 * @swreq UREQ:
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/o.USER_PREF.manage.inc 59 2018-11-21 09:04:09Z $
 */
class oUSER_PREF_manage {

	
	/**
	 * save userGlob as user preferences
	 * THIS IS A NEW METHOD: optimizes for LOW DELETE/INSERT SQL-Methods !
	 * - update USER_PREF:UPFLAG = 0
	 * - insert / update user + USER_PREF:UPFLAG = 1
	 * - delete where UPFLAG = 0
	 * @param $sql
	 */
	function savePrefsNew( &$sqlo ) {
		global  $userGlob; 
	  
	  	$tmpallow = role_check_f ( $sqlo, "g.appPrefSave" );
	  	if ( $tmpallow == "deny" ) {
	  		return -1;
	  	}
	
	  
	  	/* compare DB with $_SESSION['userGlob'] => iterate DATABASE
	  	 * $tmpUserGlob[KEY] = 
	  	 *      0 : ??? ERROR
	  	 * 		1 : value conflict
	  	 *      2 : equal match
	  	 *      3 : need new value
	  	 */
	  	$tmpUserGlob=array();
	  	$sqlsel = 'var_name, value FROM user_pref WHERE db_user_id = '.$sqlo->addQuotes($_SESSION['sec']['db_user_id']).' ORDER BY var_name';
		$sqlo->Quesel($sqlsel);
		while ($sqlo->ReadRow()) {
			$val = $sqlo->RowData[1];
			$key = $sqlo->RowData[0];
			$val_sess = $_SESSION['userGlob'][$key];
			if ( $val_sess === NULL or $val_sess === '') {
				$tmpUserGlob[$key] = 0;
	  			continue; // ignore NULL values => will lead to a DELETE
	  		}
			if ($val === $_SESSION['userGlob'][$key]) {
				$tmpUserGlob[$key]=2;
			} else {
				$tmpUserGlob[$key]=1;
			}
		}
		
		/* compare DB with $_SESSION['userGlob'] => iterate SESSION-variable
	  	 * $tmpUserGlob[KEY] = 1 : conflict
	  	 * $tmpUserGlob[KEY] = 2 : equal match
	  	 */
	  	foreach($_SESSION['userGlob'] as $key => $val) {
			
	  		if ( $val === NULL or $val === '') {
	  			continue; // ignore NULL values
	  		}
			if (!$tmpUserGlob[$key]) { // not saved yet in DB
				$tmpUserGlob[$key] = 3; // need to insert ...
			}
		}
		
		/* debug analysis ...
		echo '<table>';
		foreach($_SESSION['userGlob'] as $key => $val) {
	  		echo "<tr><td>".$key.'</td><td>flag:'.$tmpUserGlob[$key]. "</td><td>VAL:".htmlspecialchars($val)."</td></tr>\n";
		}
		echo '</table>';
		*/
		
		/**
		 * just update/insert values, where $tmpUserGlob[$key]= 1 or 2
		 */
		
		$sqlo->SetAutoCommit(false);
	  
	  	// INIT USER_PREF : UPFLAG=0
	  	$sqlo->query('UPDATE USER_PREF set UPFLAG=0 WHERE db_user_id = '.$sqlo->addQuotes($_SESSION['sec']['db_user_id']));
	  	
	  	// insert/update values
		foreach($tmpUserGlob as $key => $flag) {
			
			$sqlString = NULL;
			
	  		switch ($flag) {
	  			case 1: // update VALUE
	  				$val = $_SESSION['userGlob'][$key];
	      			$sqlString ='update user_pref set '.
	      			  ' value='.$sqlo->addQuotes($val).', UPFLAG=1'.
	                  ' where DB_USER_ID='.$_SESSION['sec']['db_user_id'].' and var_name='.$sqlo->addQuotes($key) ;
	  				break;
	  			case 2: // set only the UPDATE flag
	  				$sqlString ='update user_pref set '.
	      			  ' UPFLAG=2'.
	                  ' where DB_USER_ID='.$_SESSION['sec']['db_user_id'].' and var_name='.$sqlo->addQuotes($key) ;
	  				break; 
	  			case 3: // insert
	  				$val = $_SESSION['userGlob'][$key];
	      			$sqlString = 'INSERT INTO user_pref (db_user_id, var_name, value, UPFLAG) VALUES ('.
	                  $_SESSION['sec']['db_user_id'].', '.$sqlo->addQuotes($key).', '.$sqlo->addQuotes($val).', 3)' ;
	  				break;
	  			default:
	  				// nothing ...
	  		}
	  		
	  		if ($sqlString!=NULL) {
	  			$sqlo->query($sqlString);
	  		}
	  		
		}
	  
	  // delete entries, where UPFLAG=0 ( never mentioned ....
	  $sqlo->query('DELETE FROM USER_PREF WHERE db_user_id = '.$sqlo->addQuotes($_SESSION['sec']['db_user_id']).' and UPFLAG=0');

	  $sqlo->commit();
	  $sqlo->SetAutoCommit(true);
	  
	  return 0;
	}
	
	function loadprefs(&$sql, &$userGlob, $xcctuser, $user_id) {
	  $sql->query('SELECT var_name, value FROM user_pref WHERE db_user_id = '.$sql->addQuotes($user_id).' ORDER BY var_name');
	  $cnt = 0;
	  while ($sql->ReadRow()) {
		$userGlob[$sql->RowData[0]] = $sql->RowData[1];
		$cnt++;
	  }
	  return $cnt;
	}
	
	/**
	 * INSERT/UPDATE one key/value pair of a USER
	 * @param object $sqlo
	 * @param int $user_id
	 * @param string $key
	 * @param string $value
	 */
	static function entry_update_user( &$sqlo, int $user_id, string $key, $value) {
	    
	    $sqlsel = 'VALUE FROM USER_PREF WHERE DB_USER_ID=' . $sqlo->addQuotes($user_id). ' and VAR_NAME='.$sqlo->addQuotes($key);
	    $sqlo->Quesel($sqlsel);
	    if ($sqlo->ReadRow()) {
	       // $sqlo->RowData[0];
	       if ($value==='' or $value===NULL) {
	          // delete ...
	           $sqlo->deletex('USER_PREF', 'DB_USER_ID='.$sqlo->addQuotes($user_id).' and VAR_NAME='. $sqlo->addQuotes($key));
	       } else {
	           $argu= array('VALUE'=>$value);
	           gObjUpdate::update_row_s($sqlo, 'USER_PREF', $argu, array('DB_USER_ID'=>$user_id, 'VAR_NAME'=>$key));
	       }
	    } else {
	        if ($value==='' or $value===NULL) {
	            // nothing 
	        } else {
	           $argu=array('VALUE'=>$value, 'DB_USER_ID'=>$user_id, 'VAR_NAME'=>$key);
	           insert_row_s($sqlo, 'USER_PREF', $argu);
	        }
	    }
	    
	}
	
	/**
	 * INSERT/UPDATE one key/value pair of LOGGEDIN user
	 * @param object $sqlo
	 * @param string $key
	 * @param string $value
	 */
	static function entry_update(&$sqlo, $key, $value) {
	    self::entry_update_user($sqlo, $_SESSION['sec']['db_user_id'], $key, $value);
	}
	
	/**
	 * - set key val in $_SESSION['userGlob']
	 * - save key,val in DB only on chancge of value: this saves DB-updates
	 * @param object $sqlo
	 * @param string $key
	 * @param string $value
	 */
	static function entry_update_on_change(&$sqlo, $key, $value) {
	    $old_val = $_SESSION['userGlob'][$key];
	    $_SESSION['userGlob'][$key]=$value;
	    if ($old_val!=$value) {
	        oUSER_PREF_manage::entry_update($sqlo, $key, $value);
	    }
	}
	
	/**
	 * remove on VAR from ALL users
	 * @param object $sqlo
	 * @param string $varname
	 */
	static function remove_var_all($sqlo, string $varname) {
	    $sqlwhere = 'VAR_NAME='.$sqlo->addQuotes($varname);
	    $sqlo->deletex('USER_PREF', $sqlwhere);
	}

}
