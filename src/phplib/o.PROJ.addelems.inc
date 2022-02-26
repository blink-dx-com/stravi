<?php
/**
 * functions to ADD, UNLINK object links to projects
 * @namespace core::obj::PROJ
 * @package  o.PROJ.addelems.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/o.PROJ.addelems.inc 59 2018-11-21 09:04:09Z $
 */

require_once('insert.inc');
require_once ("f.assocUpdate.inc");
require_once ("class.history.inc");

/**
 * functions to ADD, UNLINK object links to projects
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @example <pre>
 *   $projLib = new oProjAddElem( $sqlo, $projid );
 *   $projLib->addObj( $sqlo, $tablename, $id); 
 * </pre>
 */
class oProjAddElem {
	
    /**
     * init class and (optional) project  and  check project access
     */
    function __construct( &$sql, $projid=NULL ) {
    	//global $error;
    	//$FUNCNAME= "oProjAddElem";
    	
    	$this->assoclib = new  fAssocUpdate();
    	if ($projid!=NULL) $this->setProject($sql, $projid );
    }
    
    /**
     * user has  has insert right for this PROJECT
     * @param object $sqlo
     * @param int $proj_id
     * @return number 0,1
     */
    static function has_insert_rights($sqlo, $proj_id) {
        $o_rights = access_check($sqlo, 'PROJ', $proj_id);
        $right='insert';
        $answer = 0;
        if ( $o_rights[$right]>0) {
            $answer = 1;
        }
        return $answer;
    }
    
    /**
     * activate project
     * @param $sql
     * @param $projid
     * @param $option e.g. 'noAccCheck'=>0,1
     * @return -
     */
    function setProject( &$sql, $projid, $option=NULL ) {
    	
    	$this->inserted = 0;
    	$this->objfound = 0;
    
    	$this->assoclib->setObj( $sql, 'PROJ_HAS_ELEM', $projid, $option );
    	$this->proj_id = $projid;
    }
    
    function unlinkObj( &$sql, $tablename, $objid, $opt=NULL ) {
        if (!$this->proj_id) {
            throw new Exception('class not initialized.');
        }
    	$idarr = array('TABLE_NAME'=>$tablename, 'PRIM_KEY'=>$objid );
    	$this->assoclib->delOneRow( $sql, $idarr );
    }
    	
    /**
     * add one object to project
     * 
     *  - insert one element to project 
        - default: full insert_row() of project
        - if element exists in PROJ_HAS_ELEM, set it as link
    	- on first call for project => touch project !
    	- do not touch when $opt["notouch"]=>1
    	- save $proj_id as class-var
     * @param $opt  
        "order"   !="" set  ELEM_ORDER
    	"notouch"] => 1 : never touch project!
     * @return int
     *         0 ok
    	       1 element already exists
    	       <0 error
     */
    function addObj( &$sql, $tablename, $id, $opt=NULL  ) {
    	global $error;
    	$FUNCNAME= 'addObj';
        $error  = & ErrorHandler::get();
        
        if (!$this->proj_id) {
            throw new Exception('class not initialized.');
        }
        if ( !cct_access_has2($tablename) ) {
            $error->set($FUNCNAME, 2, 'Insertation failed. Table '.$tablename.' is not a BO.'  );
            return -3;
        }
        
        
        $retval = 0;
    	$proj_id= $this->proj_id;
    	
        $sql->query('SELECT proj_id FROM PROJ_HAS_ELEM WHERE proj_id = '.$proj_id. 
    			' AND TABLE_NAME ='.$sql->addquotes($tablename).' AND PRIM_KEY = '.$id);
        if ($sql->ReadRow() && $sql->RowData[0] != '') {
            $this->objfound++;
        } else {
            $sql->query('SELECT proj_id FROM PROJ_HAS_ELEM WHERE table_name = '.
    				$sql->addquotes($tablename).' AND PRIM_KEY = '.$id);
            $is_link = (($sql->ReadRow() && $sql->RowData[0] != '')) ? 1 : 0; // object already existing in a project
    	
            $argu=array();
            $argu['TABLE_NAME'] = $tablename;
            $argu['PRIM_KEY']   = $id;
            $argu['IS_LINK']    = $is_link;
    	    if ($opt["order"]!="") $argu['ELEM_ORDER'] = $opt["order"];
    		
            $prim_id = $this->assoclib->insert($sql, $argu);
            if (!$prim_id) {
                $error->set($FUNCNAME, 0, 'Insertation failed. Project (ID): ' . 
    					$proj_id . '. Object: ' . $tablename . '=' . $id .' (as link: '. $is_link . ')');
                return (-2);
            }
    		$this->inserted++;
    		
        }
        return $retval;
    }
    
    /**
     * @param array $argu
     *    ELEM_ORDER
     *    IS_LINK
     *    DEACTIVATED
     */
    function mod_entry($sqlo, $tablename, $id, $argu) {
        
        if (!$this->proj_id) {
            throw new Exception('class not initialized.');
        }
        $idarr=array('TABLE_NAME'=>$tablename, 'PRIM_KEY'=> $id);
        $this->assoclib->update($sqlo, $argu, $idarr);
    }
    
    function getNumInsert() {
    	return ($this->inserted);
    }
    
    /**
     * get object by name from project
     * @param  $table
     * @param  $name  (object name)
     * @return array ($found, $objid)
     * 		$found : 0 : not found
     * 				 1 : exact one
     * 				 2 : more than one found
     */
    function getObjByName( &$sqlo, $table, $name ) {
    	global $error;
    	$FUNCNAME= 'getObjByName';
    
    	$proj_id = $this->proj_id;
    	if ( !$proj_id ) {
    		$error->set($FUNCNAME, 1, 'class not initialized.');
            return;
    	}
    
    	$found  = 0;
    	$pkname = PrimNameGet2($table);
    	$sqls   = "select x.$pkname from proj_has_elem p, ".$table." x ".
    			" where PROJ_ID=".$proj_id." AND p.TABLE_NAME='".$table."' ".
    			" AND p.PRIM_KEY=x.".$pkname." and x.name=".  $sqlo->addQuotes( $name ) ."";
    	$sqlo->query($sqls);
    	if (!$sqlo->ReadRow() ) {
    		return array($found, NULL);
    	}
    	$objid = $sqlo->RowData[0];
    	$found  = 1;
    	if ( $sqlo->ReadRow() ) {
    		$found  = 2;
    	}
    	return array($found, $objid);
    }
    
    /**
     * check, if the user has selected a valid project to link data to
     * @return array( project_id , name)
     * 				  project_id = 0 : error occured
     * @param object $sql
     * @param object $project_id
     */
    function projectTest( &$sql, $project_id=0 ) {
    	global   $error;
    	$FUNCNAME= 'projectTest';
    	
    	$error  = & ErrorHandler::get();
    	$proj_name  = NULL;
    	
    	if (!$project_id) {
    		$hist_obj   = new historyc();
    		$project_id = $hist_obj->last_bo_get('PROJ'); // if exists
    	}
    	
    	if ( !$project_id ) {
    		$error->set( $FUNCNAME, 2, 'You must select a destination project to create the object.');
    		return array( $project_id, $proj_name );
    	}
    	
    	$sql->query('SELECT name FROM proj WHERE proj_id = '.$project_id);
    	$sql->ReadRow();
    	$proj_name = $sql->RowData[0]; /* just loaded from DB */
    	$o_rights  = access_check($sql, 'PROJ', $project_id );
    		
    	if (!$o_rights['insert']) {
    		$error->set( $FUNCNAME, 1, 'You have no permission to add an object to the project  '.$project_id.
    			' (insert right).');
    		$project_id = 0;
    		return array( $project_id, $proj_name );
    	}
    	
    	
    	
    	return array( $project_id, $proj_name );
    }

	
}

