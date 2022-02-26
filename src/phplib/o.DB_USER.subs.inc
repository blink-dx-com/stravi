<?php
require_once 'f.password.inc';
require_once ('f.update.inc');
require_once 'o.S_VARIO.subs.inc';
require_once 'o.ROLE.mod.inc';

/**
 * sub methods for DB_USER
 * @package o.DB_USER.subs.inc
 * @swreq UREQ:GLOBAL
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class DB_userC {
	
	private $userid;
	public $out_buffer=array();

    function setUserID($userid) {
    	$this->userid = $userid;
    	$this->out_buffer=array();
    }
    
    static function getIcon( $id ) {
        $img_dir = $_SESSION['globals']['data_path'];
        return ( $img_dir."/DB_USER.".$id.".jpg" );
    }
    
    static function user_logout_save($sqlo, $userid) {
        // do not change modification date of user ...
        $sqlo->Updatex("DB_USER",'LOGOUT_LAST = '.$sqlo->Timestamp2Sql().' WHERE DB_USER_ID = '.$sqlo->addQuotes($userid) );
    }
    
    /**
     * get full name of user
     * - if not set: get "NICK:".$nick
     * @param  $sqlo
     * @param  $userid
     * @return string $fullname
     */
    static function getFullname(&$sqlo, $userid) {
    	if (!is_numeric($userid)) sys_error_my( "Input-param userid not given." );
    	if (!($userid)) sys_error_my( "Input-param userid not given." );
    	
    	$fullname='ID:'.$userid; // default fallback
    	$userFeats = glob_elemDataGet3( $sqlo, "DB_USER", array("DB_USER_ID"=>$userid), 
    		array('NICK', 'FULL_NAME') ); 
    	$fullname = $userFeats['FULL_NAME'];
    	if ($fullname==NULL) $fullname ='[NICK:'.$userFeats['FULL_NAME'].']'; // fall back
    	return $fullname;
    }
    
    /**
     * 
     * @param  $sqlo
     * @param  string $nick
     * @return int ID
     */
    static function get_ID_by_Nick(&$sqlo, $nick) {
    
        $sqlsel = 'DB_USER_ID from DB_USER where UPPER(NICK) like UPPER('.$sqlo->addquotes($nick).')';
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $user_id = $sqlo->RowData[0];
        
        return $user_id;
    }
    
    /**
     * set LOCAL PASSWORD for a user 
     * @param object $sqlo
     * @param int $user_id
     * @param string $pw -- password
     */
    static function password_set($sqlo, $user_id, $pw) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
        if ( $pw=="" ) {
            $error->set( $FUNCNAME, 1, "Password is empty." );
            return;
        }
    
        
        $pw_crypt="";
        $pw_crypt=fPasswordC::hash_pw($pw);
        
        // Disable SQL logging before sending the update statement to SQL the query object:
        // (SQL logging will be automatically resumed if a new SQL query object is
        //  requested by calling 'logon_to_db()'. This typically happens within the
        //  next PHP script executed.)
        
        $log = & SQL_log::getHandle();
        $old_flag = $log->isLogging();
        $log->close();
        
//         $sqls = "PASS_WORD =".$sqlo->addQuotes($pw_crypt)." where DB_USER_ID=".$user_id;
//         $ret  = $sqlo->Updatex("DB_USER",$sqls); //TBD: use function , which logs the update
        
        $argu=array(
            'DB_USER_ID'=>$user_id,
            'PASS_WORD' =>$pw_crypt
        );
        gObjUpdate::update_row($sqlo, "DB_USER", $argu);
        
        $log->setLogging($old_flag); // reopen SQL logging
        
        if ($error->Got(READONLY)) {
            $error->set( $FUNCNAME, 3, "Update of password failed!");
            return;
        }
    }
    

    /**
     * create user home project
     * @param $sql
     * @param $user_id
     * @param $option  "hasHomeIgnore" => 1
     * @return int project_id
     * @global table GLOBALS.NAME='o.PROJ.userhome' : alternative base-project
     */
    static function homeProjCreate(&$sql, $user_id, $option = NULL) {
    	global $error;
    	$error = & ErrorHandler::get();
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$proj_tab_nice = tablename_nice2('PROJ');
    	$baseProjectID = glob_elementDataGet( $sql, 'GLOBALS', 'NAME', 'o.PROJ.userhome', 'VALUE');
    	
    	$sqlCondAdd = 'is NULL';
    	if ($baseProjectID>0) {
    		$sqlCondAdd = '='.$baseProjectID;
    		if ( !gObject_exists ($sql, 'PROJ', $baseProjectID) ) {
    			$error->set($FUNCNAME, 5, "BaseProject ID:".$baseProjectID. ' does not exist.');
    			return;
    		}
    	}
    	
    	$sql->query("select nick from db_user where db_user_id = ".$user_id);
    	$sql->ReadRow();
    	$nick = $sql->RowData[0];
    	
    	if ($nick == "") {
    		$error->set($FUNCNAME, 1, "No nick for user $user_id");
    		return;
    	}
    	
    	
    	if ( $option["hasHomeIgnore"] != 1 ) {
    	
    		// test if project entry exists 
    		$sql->query("SELECT proj_id FROM proj WHERE name = '".$nick."' AND pro_proj_id ".$sqlCondAdd);
    		$sql->ReadRow();
    		$proj_id = $sql->RowData[0];
    		
    		if ($proj_id>0) {
    			$error->set($FUNCNAME, 2, "Folder already exists [ID:$proj_id].");
    			return;
    		}
    	}
    	
    	$argu = array();
    	// $argu["PROJ_ID"]		= ""; // NULL value for AUTO-sequence column not allowed in POSTGRES!
    	$argu["PRO_PROJ_ID"]	= $baseProjectID;
    	$argu["NAME"]			= $nick;
    	$argu["NOTES"]			= "Welcome to ".$nick."'s home ".tablename_nice2('PROJ').".";
    	
    	$xopt = NULL;
    	$xopt["db_user_id"]     = $user_id;  // owned by $user_id, not by ROOT
    	$prim_id_proj = insert_row( $sql, "PROJ", $argu, $xopt );
    	if ($prim_id_proj <= 0) {
    	    $error->set($FUNCNAME, 3, "SQL insert for new ".$proj_tab_nice." failed.");
    		return;
    	}
    	
    	// set home project in db_user
    	$sql->Updatex("DB_USER", "proj_id = $prim_id_proj WHERE db_user_id = $user_id");
    	
    	if ($error->Got(READONLY))  {
    	    $error->set($FUNCNAME, 4, "Update of ".$proj_tab_nice." rights failed.");
    	}
    	return ($prim_id_proj);
    }
    
    static function persGroupCrea(&$sql, $user_id) {
    	// FUNCTION: create a single user group
    	global $error;
    	
    	$sql->query("select nick from db_user where db_user_id = ".$user_id);
    	$sql->ReadRow();
    	$nick = $sql->RowData[0];
    	
    	$sql->query("select 1 from USER_GROUP where name='".$nick."'");
    	if ( !$sql->ReadRow() ) {
    		// group not exists
    		$argu = NULL;
    		$argu["NAME"] 		 = $nick;
    		$argu["SINGLE_USER"] = 1;
    		$argu["DB_USER_ID"]  = $user_id;
    		$dbgrp_id = insert_row($sql, "USER_GROUP", $argu);
    		
    		if (!$dbgrp_id) {
    			$error->set("persGroupCrea", 1, "Error during creation of group for user '$nick'");
    			return;
    		}
    		$argux = NULL;
    		$argux["USER_GROUP_ID"] = $dbgrp_id;
    		$argux["DB_USER_ID"]    = $user_id;
    		$retval = insert_row($sql, "DB_USER_IN_GROUP", $argux);
    		
    	} else {
    		$error->set("persGroupCrea", 1, "Group '$nick' already exists.");
    		return;
    	}
    	return ($dbgrp_id);
    }
    
    /**
     * get value from vario
     * @param  $sqlo
     * @param  $key
     */
    function getVarioVal(&$sqlo, $key, $defaultVal=NULL) {
    	
    	if (!$this->userid) {
    	    sys_error_my( __CLASS__. ' not initialized.' );
    	}
    	
    	$varioLib = new oS_VARIO_sub('DB_USER');
    	
    	$value = $varioLib->getValByKey($sqlo, $this->userid, $key);
    	
    	if ($value===NULL) $value = $defaultVal;
    	
    	return $value;
    }
    
    private function _add_out_buf($text) {
        $this->out_buffer[]=$text;
    }

    /**
     * remove groups and roles
     * @param object $sqlo
     */
    function deactivate(&$sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$this->userid) {
            sys_error_my( __CLASS__. ' not initialized.' );
        }
        
        $userid = $this->userid;
        
        $debug = $_SESSION["userGlob"]["g.debugLevel"];

        $UpdateLib = new globObjUpdate();
        $args= array(
            'vals'=>array('LOGIN_DENY'=>1)
        );
        
        $UpdateLib->update_meta( $sqlo, 'DB_USER', $userid, $args );
        
        // remove from role
        if ($debug) $this->_add_out_buf('remove from role');
        $wherestr = "DB_USER_ID=".$userid;
        $sqlo->Deletex ( 'USER_ROLES', $wherestr );
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'an error occured: USER_ROLES' );
            return;
        }
        
        // remove from group, except own SINGLE_USER group (analyse SINGLE_USER flag in USER_GROUP)
        if ($debug) $this->_add_out_buf('remove from groups, except personal group');
        $wherestr = "DB_USER_ID=".$userid. ' and USER_GROUP_ID in ('.
            'select USER_GROUP_ID from USER_GROUP where (SINGLE_USER=0 or SINGLE_USER is NULL)'.
            ')';
        $sqlo->Deletex ( 'DB_USER_IN_GROUP', $wherestr );
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 2, 'an error occured: DB_USER_IN_GROUP' );
            return;
        }
        
        // deactivate personal group
        if ($debug) $this->_add_out_buf('deactivate personal group');
        $nick   = glob_elementDataGet( $sqlo, 'DB_USER', 'DB_USER_ID', $userid, 'NICK');
        $sqlsel = "USER_GROUP_ID from USER_GROUP where name=".$sqlo->addQuotes($nick);
        $sqlo->Quesel($sqlsel);
        if ( $sqlo->ReadRow() ) {
            $grp_id = $sqlo->RowData[0];
            $args = array( 'vals' => array('INACTIVE'=>1) );
            $UpdateLib->update_meta( $sqlo, 'USER_GROUP', $grp_id, $args );
        }
        
        // USER_PREF
        if ($debug) $this->_add_out_buf('delete all USER_PREF');
        $wherestr = "DB_USER_ID=".$userid;
        $sqlo->Deletex ( 'USER_PREF', $wherestr );
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 2, 'an error occured: USER_PREF' );
            return;
        }
        
        // delete image
        if ($debug) $this->_add_out_buf('delete user image-file');
        $pfilename = datadirC::datadir_filename( "DB_USER", $userid ,"jpg" );
        if ( file_exists($pfilename) ) {
            echo "... delete user image file.<br>";
            unlink ($pfilename);
        }
        
    }

    function add_role(&$sqlo, $role_id) {
        if (!$this->userid) {
            sys_error_my( __CLASS__. ' not initialized.' );
        }
        
        $user_id = $this->userid;
        $role_lib = new oROLE_mod($sqlo, $role_id);
        $role_lib->add_user_role($sqlo, $user_id);
    }

}

