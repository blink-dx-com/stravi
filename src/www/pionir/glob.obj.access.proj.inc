<?php
require_once ('o.CCT_ACCLOG.subs.inc');

/**
 * - access rights: manage right-setting for a project tree
 * - support recursive settings
 * @package glob.obj.access.proj.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class gObjAccRigProj {
	
    function __construct(&$sqlo) {
    	$this->accLogLib = new oAccLogC();
    	$this->errcnt=0;
    }
    
    function getGroups( &$sql ) {
    	global $error;
    	
    	$this->groups = array(); // array (user_group_id => user_group_name)
    	$sql->query("SELECT user_group_id, name FROM user_group");
    	while($sql->readRow())
    	  $this->groups[$sql->RowData[0]] = $sql->RowData[1];
    }
    
    function _statusShowCnt($got_error) {
    	if (!$got_error) {
    		echo '<font color="#00ff00">okay</font>';
    	} else {
    		$this->errcnt++;
    	}
    }
    
    function getErrCnt() {
    	return($this->errcnt);
    }
    
    /**
     * - check audit state
     * - if an audit state is set (except 'reopen'): deny change
     * @global $this->specialAct
     * @return $allow 0,1
     */
    function _checkAuditState(&$sqlo, $access_id) {
    	
    	$allow=0;
    	$this->accLogLib->setObject( $sqlo, NULL, NULL, $access_id );
    	$lastStatus = $this->accLogLib->getLastLogName($sqlo);
    	
    	if ($lastStatus=='') return 1;
    	
    	if ($lastStatus=='reopen' or $_SESSION['sec']['appuser']=="root" ) {
    		$allow=1;
    	}
    	return $allow;
    }
    
    
    
    /**
     *  deletes the group-right-sets which are existing but are not in $rights
     * @param object $sql
     * @param int $cct_access_id  ... cct_access_id of the object
     * @param array $rights  ... array (group_id => array(rights like return of access_check), ...)
     * @param string $tablename
     * @return void|number wether error occured or not
     */
    function delete_groups_after_set(  &$sql,    int $cct_access_id,     array &$rights, string $tablename) {
    
        $error     = & ErrorHandler::get();
        $got_error = 0;
    
      $sqlsel = 'user_group_id FROM cct_access_rights WHERE cct_access_id = '.$cct_access_id;
      if ($error->got(READONLY)) return;
      $existing_groups=array();
      $sql->Quesel($sqlsel);
      while ( $sql->ReadRow() ) {
          $existing_groups[] = $sql->RowData[0];
      }
      
      
      foreach( $existing_groups as $existing_group_id) {
    	if (!array_key_exists($existing_group_id, $rights)) {
    	  $access_allow = access__allowed_to_chmod($sql, $cct_access_id, $tablename);
    	  switch ($access_allow) {
        	  case 'no':
        		echo '<font color="#ff6699">'.$this->groups[$existing_group_id].': not allowed to delete right set.</font> ';
        		$got_error = 1;
        		break;
        	  case 'yes':
        	      access_del_group($sql, $cct_access_id, $existing_group_id);
        		  if ($error->got(READONLY)) return;
        		  break;
        	  case 'entail':
        		$o_rights_of_user = access_check ($sql, NULL, NULL, $cct_access_id);
        		if ($error->got(READONLY)) return;
        		if (count(array_unique($o_rights_of_user)) == 1) { // user has all rights (all rights are one because user has entail right)
        		  access_del_group($sql, $cct_access_id, $existing_group_id);
        		  if ($error->got(READONLY)) return;
        		} else {
        		  echo '<font color="#ff6699">'.$this->groups[$existing_group_id].': not allowed to delete right set.</font> ';
        		  $got_error = 1;
        		  break;
        		}
        	  default: htmlfoot("ERROR", "programming-error in delete_groups_after_set");
    	  }
    	}
      }
      return $got_error;
    }
    
    #   void access_write_rec(obj sql, int proj_id, array proj_data, array rights, string method, array proj_tree)
    
    /**
     * - writes the access rights recursivly through a project
     * - TBD: is $tablename=="PROJ" ???
     * - warning! recursive function
     * 
     * @param proj_id   ... id of the current project
     * @param proj_data ... array("name" => proj.name, "reading_allowed" => 0/1, "cct_access_id" => proj.cct_access_id)
     * @param reading_allowed ... is reading of the current project allowed
     * @param rights    ... array (group_id => array(rights like return of access_check), ...)
     * @param method    ... "add" to existing rights or "set" as rights
     * @param proj_tree ... array containing whole project tree
     */
    function access_write_recursive( &$sql, $proj_id, &$rights,  &$proj_data, &$proj_tree, &$method ) {
      global $error, $varcol;
      
      $varcol    = & Varcols::get();
      $error     = & ErrorHandler::get();
      $got_error = 0;
      $icon = "images/icon.PROJ.gif";
      
    
      echo '<li><a href="glob.obj.access.php?t=PROJ&id='.$proj_id.'" target="cctinfo">';
      echo '<img src="'.$icon.'" border="0"> '.$proj_data['name'].'</a> -- ';
      
      // do rights setting
      $tablename = "PROJ";
      reset ($rights);
      foreach( $rights as $group_id=>$group_rights) {
    	access_write_with_check($sql, $proj_data['cct_access_id'], $group_rights, $group_id, 0, $tablename);
    	$got_error = $this->error_handler_helper($group_id);
    	if ($error->got(CCT_ERROR_READONLY)) return; // other error than handeled in error_handler_helper()
      }
    
      if ($method == "set") { // remove existing rights which are not set
    	$got_error |= $this->delete_groups_after_set($sql, $proj_data['cct_access_id'], $rights, $tablename);
    	if ($error->got(READONLY)) return;	
      }
      
      $this->_statusShowCnt($got_error);
      
    
      echo '<ul style="list-style-type: none;">';
      if (!$proj_data['reading_allowed']) {
        echo '<li><font color="#ff0000">Access for project denied.</font></li></ul>'."\n";
        return;
      }
    
    
       //print_r ($proj_tree); // [$proj_id]["elements"]
    
      foreach( $proj_tree[$proj_id]['elements'] as $tab_name=>$tab_elems) {
    	$tab_name_icon = ($tab_name == 'PROJ') ? 'PROJ_link' : $tab_name;
        $icon          = file_exists('images/icon.'.$tab_name_icon.'.gif') ? 'images/icon.'.$tab_name_icon.'.gif' : 'images/icon.UNKNOWN.gif';
    
    	foreach( $tab_elems as $elem_id=>$elem_data) {
    	  $tmp_imp_name = empty($elem_data["name"]) ? $elem_id : $elem_data["name"];
    
    	  echo '<li><a href="glob.obj.access.php?t='.$tab_name.'&id='.$elem_id.'" target="cctinfo">';
    	  echo '<img src="'.$icon.'" border="0"> '.$tmp_imp_name.'</a> -- ';
    
    	  if (!empty($elem_data['cct_access_id'])) { // otherwise it is a broken link which has no rights to set/add
    		
    	  	$audAllow = $this->_checkAuditState($sql, $elem_data['cct_access_id']);
    	  	if ($audAllow) {
    		  	reset ($rights);
    			foreach( $rights as $group_id=>$group_rights) {
    				
    				  access_write_with_check($sql, $elem_data['cct_access_id'], $group_rights, $group_id, 0, $tab_name);
    				  $got_error =  $this->error_handler_helper($group_id);
    				  if ($error->got(CCT_ERROR_READONLY))
    					return; // other error than handeled in error_handler_helper()
    			}
    	  	} else { // not allowed ...
    	  		$this->_warn_out('object has an audit-status.');
    	  		$got_error = 1;
    	  	}
    		if ($method == "set") { // remove existing rights which are not set
    		  $got_error |= $this->delete_groups_after_set($sql, $elem_data['cct_access_id'], $rights, $tab_name);
    		  if ($error->got(READONLY)) return;
    		}
    	  }
    
    	  $this->_statusShowCnt($got_error);
    
    	  echo '</li>';
    	}
      }
    
      foreach( $proj_tree[$proj_id]["projects"] as $proj_id_sub=>$proj_data_sub) // show subprojects
      	 $this->access_write_recursive($sql, $proj_id_sub, $rights, $proj_data_sub, $proj_tree, $method);
      echo "</ul></li>\n";
    
      
      
    }
    
    
    /** 
     * print a warn text
     */
    function _warn_out($text) {
    	echo '<font color="#CC6633">denied: '.$text.'</font> ';
    }
    
    #   bool error_handler_helper(int group_id)
    function error_handler_helper(   $group_id) {
    # descr: checkes the error-handler for errors occuring using access_write__recursive_proj
    # input: $group_id ... id of group fro which rigths where set
    # return: wether an error occured or not
    
    
      $error     = & ErrorHandler::get();
      $got_error = 0;
      
      if ($error->got(CCT_WARNING_READONLY)) {
    	$err = $error->getLast(); // problems on entail
    	$this->_warn_out($this->groups[$group_id].': '.$err->text);
    	$got_error = 1;
      } 
      if ($error->got(CCT_ERROR_READONLY, 'access_rights', 101)) {// not allowed to chmod
    	$err = $error->getLast('access_rights', 101); 
    	echo '<font color="#ff0000">denied: '.$err->text.'</font> ';
    	$got_error = 1;
      }
      return $got_error;
    }

}