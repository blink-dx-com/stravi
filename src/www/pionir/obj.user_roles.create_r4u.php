<?php
/**
 *  Add one role (table ROLE) to a selection of DB_USER
 * $Header: trunk/src/www/pionir/obj.user_roles.create_r4u.php 59 2018-11-21 09:04:09Z $
 * @package obj.user_roles.create_r4u.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param int $role_id (connected ROLE_ID)
 * @param int $go 
 * 		0 : give params
 * 		1 : prepare
 * 		2 : do it (insert)
 */

session_start(); 


require_once ('reqnormal.inc');
require_once ('sql_query_dyn.inc');
require_once ('glob.objtab.page.inc'); 
require_once 'o.ROLE.mod.inc';

/* This module is called while executing the function "Add role to user"
 * only allowed by Administrator(root).
 * It provides a function to select and grant a set of previously selected users a role.
 */
class UserRoleAdder { 

	var $CLASSNAME='UserRoleAdder';
	function __construct($go, $role_id, $sqlAfter) {
		$this->sqlAfter = $sqlAfter;
		$this->go = $go;
		$this->role_id = $role_id;
	}
	
	function form1(&$sqlo) {
		require_once ('func_form.inc');
	
		$role_id 	= $this->role_id;
		
		$initarr   = NULL;
		$initarr["action"]      = $_SERVER['PHP_SELF'];
		$initarr["title"]       = "Select role";
		$initarr["submittitle"] = "Prepare";
		$initarr["tabwidth"]    = "AUTO";
		$initarr["tabnowrap"]   = 1;
		$initarr["dblink"]      = 1;
	
		$hiddenarr = NULL;
	
		$formobj = new formc($initarr, $hiddenarr, 0);
	
		$fieldx = array ( 
			"title" => "Role", 
			"name"  => "role_id",
			"object"=> "dblink",
			"namex" => TRUE,    
			"val"   => $role_id, 
			"inits" => array( "table"=>"ROLE", 'pos' =>0, 'getObjName'=>1, 'sqlo'=>&$sqlo ),
			"notes" => "the connected role"
			 );
		$formobj->fieldOut( $fieldx );
	
		$formobj->close( TRUE );
	
	}

	/**
	 * insert new entries for USER_ROLES
	 * @param $sql
	 * @param $role_id
	 * @param $db_user_tmp
	 * @param $go
	 
	 */
	function _insert_user_role( &$sql, $db_user_tmp ) {
	    $go = $this->go;
		$role_id 	= $this->role_id;
		
		$role_lib = new oROLE_mod($sql, $role_id);
		
		$sqls = "select * from USER_ROLES where role_id=$role_id and db_user_id=$db_user_tmp";
		$sql->query($sqls);
		if ( !$sql->ReadRow() ) {	
			if ($go==2) {
				echo "<font color=green>add role</font>";
				$role_lib->add_user_role($sql, $db_user_tmp);
			}
		} else {
			echo "<font color=#D0D000>role exists</font>";
		}
		echo "<br>\n";
		
	}
	
	/**
	 * manage insert process
	 */
	function manageInsert(&$sqlo, &$sqlo2) {
		global $error;
		$FUNCNAME= 'manageInsert';
		
		$role_id 	= $this->role_id;
		$go 		= $this->go;
		$sqlAfter 	= $this->sqlAfter;
		
		$sqls = "select name from role where role_id=".$role_id;
		$sqlo->query($sqls);
		if ( $sqlo->ReadRow() ) {
			$role_name = $sqlo->RowData[0];
		} else {
			echo "ERROR: ROLE_ID does not exist.<P>";
			return 0;
		}
		
		echo "<font color=gray>Assign role:</font> <B>".$role_name."</B><P>";
		if ($go==1) {
			echo '<form method="post" action="'.$_SERVER['PHP_SELF'].'?go=2&role_id='.$role_id.'" >'."\n";
			echo '<input type=submit value="Submit" class="yButton"></td><td>';
			echo "</form><br>\n";
		}
		$sqls = "select db_user_id, nick from " . $sqlAfter;
		$sqlo->query($sqls);
		while ( $sqlo->ReadRow() ) {
			$db_user_tmp = $sqlo->RowData[0];
			$nick 		 = $sqlo->RowData[1];	
			echo "$nick ";

			$this->_insert_user_role( $sqlo2, $db_user_tmp);
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 1, 'problem on insert role for user "'.$nick.'"' );
				return;
			}
		}
	}
}




// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
$sqlo2  = logon2(  );
if ($error->printLast()) htmlFoot();


$role_id 	= $_REQUEST['role_id'];
$go 		= $_REQUEST['go'];
$tablename="DB_USER";

$title = "Assign a role to selection of users";
$infoarr=NULL;
$infoarr["title"] = $title;
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1;
$infoarr['design']   = 'norm';


// $headopt = array( "obj_cnt"=1 )
$mainObj = new gObjTabPage($sqlo, $tablename );
$mainObj->showHead($sqlo, $infoarr);
$mainObj->initCheck($sqlo);


if ( !glob_isAdmin() ) {
	echo "ERROR: only root is allowed to call this function.<P>";
	return 0;
}

$sqlAfter  = $mainObj->getSqlAfter();

$mainLib = new UserRoleAdder($go, $role_id, $sqlAfter);


if ( !$go ) {
	$mainLib->form1($sqlo);
	$mainObj->htmlFoot();
	
} else {

	if (!$role_id) {
		$mainObj->htmlFoot('ERROR', 'Please give ROLE_ID.');
	}
	$mainLib->manageInsert($sqlo, $sqlo2);
	
}

$mainObj->chkErrStop();
$mainObj->htmlFoot();

