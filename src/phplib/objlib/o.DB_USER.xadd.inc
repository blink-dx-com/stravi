<?php
require_once ("o.DB_USER.subs.inc");
require_once('glob.obj.create.ABS.inc');

/**
 * object specific modification class
 * @package oDB_USER.xadd.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oDB_USER_xmodC  extends gObjCreateC_ABS  {

    /* special settings, tests for DB_USER
         @swreq SREQ:0001698: o.DB_USER > create a user
        
         check unique NAME
         @swreq SREQ:0001698:003: ensure unique nick name in the system
 
    */
    public function copy_prep(&$sqlo, &$obj_blueparam ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $tmpName = $obj_blueparam["vals"]["NICK"];
        $sqlsel  = 'NICK from DB_USER where UPPER(NICK) like UPPER('.$sqlo->addQuotes($tmpName).')';
        $sqlo->Quesel($sqlsel);
        if ( $sqlo->ReadRow()) {
            $error->set( $FUNCNAME, 1, 'User nick "'.$tmpName.'" already exists.' );
            return;
        }
        if ( $obj_blueparam["vals"]["EMAIL"]==NULL ) {
            $error->set( $FUNCNAME, 1, 'Input-Error: EMAIL missing.' );
            return;
        }
        if ( $obj_blueparam["vals"]["FULL_NAME"]==NULL ) {
            $error->set( $FUNCNAME, 1, 'Input-Error: FULL_NAME missing.' );
            return;
        }
        
        $tmpPassword = fPasswordC::createPasswd();
        $cryptPasswd = fPasswordC::hash_pw($tmpPassword);
        $obj_blueparam["vals"]["PASS_WORD"]  = $cryptPasswd;
        $obj_blueparam["vals"]["CONTACT_ID"] = NULL;
        
        //         if ( $option["optspecial"]["showpasswd"] ) {
        //             $this->creaInfo[] = "Email: ".$obj_blueparam["vals"]["EMAIL"]."<br>".
        //                 "User: " .$obj_blueparam["vals"]["NICK"]."<br>".
        //                 "Password: ". $tmpPassword;
        //         }
    	
    	
    }
    
    function post_crea( &$sql, &$sql2, $objid,  $blueObjId, $option  = array()	) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;

        // if the user was copied from former user ... delete personal groups
        $this->_add_log("remove user from personal groups ..."); //TBD: add to a log ?
        $sqls = "delete from DB_USER_IN_GROUP u ".
            "where u.DB_USER_ID=".$objid. " AND u.USER_GROUP_ID in (select USER_GROUP_ID from USER_GROUP where SINGLE_USER=1)";
        $sql->query($sqls);
        
        // 1. create rights, 2. create project !!!
        
        if ( $_SESSION["globals"]["security_write"]>0 ) {
            $this->_add_log("create a single user group ...");
            DB_userC::persGroupCrea($sql, $objid);
            if ( $error->Got(READONLY) )  {
                $error->set($FUNCNAME, 3, "error during creation of a single user group!");
                return;
            }
        }
        
        $this->_add_log("create a home project ...");
        DB_userC::homeProjCreate($sql, $objid);
        
        if ( $error->Got(READONLY) )  {
            $error->set($FUNCNAME, 2, "error during home folder creation!");
            return;
        }
  
    }

}
