<?php
require_once("o.DB_USER.subs2.inc");
require_once("insert.inc"); 
require_once('o.PROJ.subs.inc');
 
/**
 * contains sub functions for ".profile" project
 * user session-cache because this changes rarly ...
 *    $_SESSION['s_sessVars'][o.PROJ.user.home.id] 
 *    $_SESSION['s_sessVars'][o.PROJ.user.profile.id]
 *    $_SESSION['s_sessVars'][o.PROJ.user.bookmarks.id]
 *
 * @package o.proj.profile.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class profile_funcs {
    
    private $_log=array();
    
    function get_log() {
        return $this->_log;
    }
    
    private function _logadd($text) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        //$this->_log[]=$text;
        debugOut($text, $FUNCNAME, 4);
    }
    
    private function _create_proj($sqlo, $mo_proj_id, $name) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $o_rights = access_check( $sqlo, "PROJ", $mo_proj_id);
        if ($o_rights["insert"] != 1) {
            $error->set($FUNCNAME, 1, 'no "insert" right on project [ID:'.$mo_proj_id.']!');
            return (-1);
        }
        $argu=array();
        $argu["NAME"] = $name;
        $argu["PRO_PROJ_ID"] = $mo_proj_id;
        $new_proj_id = insert_row_role(  $sqlo, "PROJ", $argu); // with role check !!!
        
        return $new_proj_id;
    }

    /**
     * get/make /HOME/.profile/$projName  project
     * @param object $sql
     * @param string $projName
     * @param string $action ["CREATE"], 
                       at "READONLY": if no project exists returns just 0, no error
       @param int $other_user_id !!! WARNING, PLEASE JUST USE FOR UNITTESTS !!! 
                      if you set this, it uses/changes the current USER cache !!!
     * @return int
     */
    function getProj( &$sql, $projName, $action="CREATE", $other_user_id=0 ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        $error = & ErrorHandler::get();
        
        $out_proj_id = 0;
        $profilename = ".profile";
        
        
        // try fast cache hit ....
        if ($projName=='bookmarks' and $_SESSION['s_sessVars']['o.PROJ.user.bookmarks.id']>0) {
            $this->_logadd('(56) first hit');
            return $_SESSION['s_sessVars']['o.PROJ.user.bookmarks.id'];
        }
        
        if ($_SESSION['s_sessVars']['o.PROJ.user.home.id']) {
            $home_proj_id = $_SESSION['s_sessVars']['o.PROJ.user.home.id'];
            $this->_logadd('(66) home cache hit');
        } else {
            $home_proj_id =  oDB_USER_sub2::userHomeProjGet( $sql, $other_user_id );
            $_SESSION['s_sessVars']['o.PROJ.user.home.id'] = $home_proj_id; // update session var
        }
        
       
        if ( !$home_proj_id ) { 
            $error->set($FUNCNAME, 1, 'no home project!');
            return -1;
        }
        
        //
        // get ".profile"
        //
        
        
        $get_from_DB=1;
        if ( array_key_exists('o.PROJ.user.profile.id', $_SESSION['s_sessVars']) ) {
            // try from cache ...
            $tmp = $_SESSION['s_sessVars']['o.PROJ.user.profile.id'];
            if (!$tmp and $action!='CREATE') {
                $this->_logadd('(88) profile cache hit: NONE');
                return 0;
            }
            if ($tmp) {
                $get_from_DB= 0;
                $profiledir = $tmp;
                $this->_logadd('(94) profile cache hit: YES');
            }
        } 
        
        if ($get_from_DB) {
            // get from database OR create ...
            $sqlsel= "proj_id from proj where pro_proj_id=".$home_proj_id. " AND NAME='".$profilename."'";
            $sql->Quesel($sqlsel);
            if ( $sql->ReadRow() ) {
                $profiledir = $sql->RowData[0];
            } else {
                if ( $action == "READONLY" ) {
                    $_SESSION['s_sessVars']['o.PROJ.user.profile.id'] = 0;
                    return 0; 
                }
                $profiledir = $this->_create_proj($sql, $home_proj_id, $profilename);
            }   
            if ($error->got(READONLY)) return 0;
            
            $this->_logadd('(113) profile from DB.');
            $_SESSION['s_sessVars']['o.PROJ.user.profile.id'] = $profiledir;
        }
        
        // get the SUB-dir, e.g. "bookmarks"
        
        $book_get_from_DB=1;
        if ($projName=='bookmarks') {
            if ( array_key_exists('o.PROJ.user.bookmarks.id', $_SESSION['s_sessVars']) ) {
                // try from cache
                $tmp = $_SESSION['s_sessVars']['o.PROJ.user.bookmarks.id'];
                if (!$tmp and $action!='CREATE') {
                    $this->_logadd('(113) bookmarks cache hit NONE.');
                    return 0;
                }
                if ($tmp) {
                    $book_get_from_DB= 0;
                    $out_proj_id = $tmp;
                    $this->_logadd('(113) bookmarks cache hit YES.');
                    return $out_proj_id;
                    
                }
            } 
        }
        
        if ($book_get_from_DB) {
            $sqlsel= "proj_id from proj where pro_proj_id=".$profiledir. " AND NAME='".$projName."'";
            $sql->Quesel($sqlsel);
            if ( $sql->ReadRow() ) {
                $out_proj_id = $sql->RowData[0];
            } else {
                if ( $action == "READONLY" ) return 0; 
                $out_proj_id = $this->_create_proj($sql, $profiledir, $projName);
                if ($error->got(READONLY)) return 0;
            }
            if ($projName=='bookmarks') {
                $_SESSION['s_sessVars']['o.PROJ.user.bookmarks.id'] = $out_proj_id;
                $this->_logadd('(150) bookmarks from DB.');
            }
        }
        
        return $out_proj_id;
    
    }  
    
    /**
     * 
     * @param object $sql
     * @param string $synonym  predefined synonyms; e.g. "bookmarks"
     * @param string $action
     * @return number
     */
    function getProjBySyn(&$sql, $synonym, $action=NULL ) {
    	
    	$syntab = array("bookmarks"=>"bookmarks");
    	
    	$realproject = $syntab[$synonym];
    	if ($realproject=="") $realproject = $synonym;
    	$projid = $this->getProj( $sql, $realproject, $action); 
    	return ( $projid );
    }
    
    function getTabList ( &$sql, $proj_id, $tablename, $options=NULL) {        
       // RETURN: array[id] = name     
        $pk_name = $tablename. "_ID";
        $first_name = "NAME";
        $obj_list=array();
        
        $sqls ="select x.".$pk_name.", x.".$first_name .
               " from ".$tablename." x ".
               " where (x.".$pk_name." in ". 
                 " (select prim_key FROM proj_has_elem WHERE proj_id = ".$proj_id." AND table_name='".$tablename."')) ".
                 "  ORDER by x.".$first_name;      
    
        $sql->query($sqls);
        while( $sql->ReadRow() ) {
            $tmpid   = $sql->RowData[0]; 
            $tmpname = $sql->RowData[1];  
            $obj_list[$tmpid] = $tmpname;
        } 
        
        return ($obj_list);
    }
  
}

/**
 * bookmark actions
 *
 */
class oPROJ_bookmarks {
    
    // object is in bookmarks project ?
    // @return int 0,1
    function obj_is_in_proj($sqlo, $tablename, $objid) {
        $answer = 0;
        $profileLib = new profile_funcs();
        $projid     = $profileLib->getProj($sqlo, 'bookmarks', 'READONLY');
        if (!$projid) return 0;
        
        if ( cProjSubs::objectInProject($sqlo, $projid, $tablename, $objid) ) {
            $answer = 1;
        }
        
        return $answer;
        
    }
}

