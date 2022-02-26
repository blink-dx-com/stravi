<? 
/**
 * contains sub functions for generating a valid key allowing to execute advanced querys
 * $Header: trunk/src/www/pionir/obj.link.c_KeyGen_sub2.inc 59 2018-11-21 09:04:09Z $
 * @package obj.link.c_KeyGen_sub2.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param string INPUT 
 */
 
require_once('o.LINK.c_queryAdv_sub.inc');

class oLINK_c_KeyGen_gui {
	var $id; // LINK object ID
	var $roleRightName = 'o.LINK.c_queryAdvKeyGen';
	
	function __construct( $id, $go, $myquvarval ) {
		$this->setObj($id);
		$this->go = $go;
		$this->myquvarval=$myquvarval;
		
		$this->values = NULL;
		
		$this->keyObj = new oLINK_c_queryAdv();
	}	
	
	function _info($text) {
		echo "Info: ".$text. "<br />\n";
	}
	
	/*
	 *
	 */
	function getRoleRightName() {
		return $this->roleRightName;
	}
	
	/* checks if an user have the neccessary role-right "o.LINK.c_queryAdvKeyGen" to generate a key
	 * @param SQL-handle-reference &$sqlo
	 * @return boolean 1=(TRUE) if the check was successful, otherwise 0=(FALSE)
	 */
	function roleCheck ( &$sqlo ) {
		//$this->roleRightName = 'o.LINK.c_queryAdvKeyGen';
		$role_right      = role_check_f( $sqlo, $this->roleRightName);
		if ($role_right!="execute" ) {
			return 0;
		}
		return 1;
	}	
	
	/* tries to generate an authorization key for an advanced query (extra class object) and update/insert it into the database
	 * neccessary role-rights and keychecks are done before
	 * @param sql-handle &$sqlo
	 * @return boolean 1=(TRUE) if the authorization key update was successful, otherwise 0=(FALSE)
	 */
	function updateKey (&$sqlo) {
		global $error;
		$FUNCNAME= 'updateKey';
		
		$values = &$this->values;
		$sqlsel = $values["sqlArray"];
		$authKey = $values["authKey"];
		
		if ( $this->keyObj->checkKey($sqlsel, $authKey) ) {
			//error handling and abortion
			$this->_info('A valid authorization key already exist in the database.');
			return 0;
		}
		
		if (!$this->roleCheck($sqlo) ) {
			//error handling and abortion
			$error->set( $FUNCNAME, 1, "You do not have the required role-right '".$this->getRoleRightName()."' to generate an authorization key.<br />
				Please contact your administrator, if you need to generate authorization keys.");
			return 0;
		} else {
			$key = $this->keyObj->createKey($sqlsel);
			if ($this->keyObj->writeKeyToObject($sqlo, $key)) {
				$this->_info( 'Authorization key '.$key.' generated and written into database' );
				return 1;
			} else {
				$error->set( $FUNCNAME, 2, "Error occured while updating the key in the database.");
				return 0;
			}
		}
	}
	
	function setObj($id) {
		$this->id = $id;
	}
	
	function initChecks( &$sqlo ) {
		$id = $this->id;
		
		$this->keyObj->setObj( $sqlo, $id );
		$this->keyObj->checkParams( $sqlo );
		$this->values = $this->keyObj->getValues();
	}
}
