<?php
require_once ('access_check.inc');
require_once ('table_access.inc');
require_once ('globals.inc');
require_once ('object.subs.inc');
require_once ('o.SATTACH.subs.inc');
require_once ('o.PROJ.subs.inc');
require_once 'o.PROJ.addelems.inc';
require_once 'glob.obj.touch.inc';

/**
 * - delete one object
 * - doeas not important class variables till now, except:
 *   - private $info_flag; 
 *   - $this->proj_elems_lib
 * - functions to delete BOs (also PROJ) and assoc elements
 * - check role right object
 * @package g_delete.inc
 * @swreq SREQ:0001166: g > delete one object from DB 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * $Header: trunk/src/phplib/g_delete.inc 59 2018-11-21 09:04:09Z $
 */
class fObjDelC {
	
	private $info_flag; // show infos?
	
function __construct() {
	$dummy=NULL;
	$this->proj_elems_lib = new oProjAddElem($dummy);
}

static function _del_VARIO($sqlo, $tablename, $prim_id) {
    $sqlsel = "count(1) FROM S_VARIO WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$prim_id;
    $sqlo->Quesel($sqlsel);
    $sqlo->ReadRow();
    $cnt = $sqlo->RowData[0];
    if (!$cnt) return 0;
    
    $sqlo->Deletex ( 'S_VARIO', "TABLE_NAME='".$tablename."' AND OBJ_ID=".$prim_id);
    return $cnt;
}
 
/**
 * delete associated objects
 * @param string $table table name of assoc objects
 * @param array $assocX array of object-ids
 */
function _delAssObj( &$sql, $table, &$assocX ) {
	global $error;
	$FUNCNAME="_delAssObj";
    $xopt = NULL;
    $infolevel = $this->info_flag;
    
    if ($infolevel) {
    	echo "Info ".$FUNCNAME.": Delete attached ".$table."(s): ";
    	$xopt["info"] =  $infolevel;
    }
    foreach( $assocX as $sub_id) {
        if ($infolevel) echo " [".$sub_id."] ";
        $this->obj_delete ( $sql, $table, $sub_id, NULL, NULL, $xopt); // TBD: use new object tp allow CLASS_variables ?
        if ($error->Got())  $error->set($FUNCNAME, 12, 'Error at deleting attached '.$table.' ['.$sub_id.']',1 ); // is a WARNING
    }
    
    return;
}

/**
 * check, if object has links to other projects
 * @param int $projID can be 0
 * @return int 0 : no
 * 			   1 : yes
 */
function _checkProjLinks( &$sqlo, $tablename, $objid, $projID ) {
	$sqlAfter = cProjSubs::getObjLinkExcept( $sqlo, $projID, $tablename, $objid );
	$sqlsel   = 'PROJ_ID' . $sqlAfter;
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$hasExtLink = $sqlo->RowData[0];
	if ($hasExtLink>0) return $hasExtLink;
	else return 0;
}

/**
 * delete on BO
 * - special actions on CONCRETE_ARRAY, PROJ
 * @param object $sql
 * @param string $tablename
 * @param int $prim_id
 * @return void|number
 */
function _deleteBO( &$sql, $tablename, $prim_id ) {
	global $error;
	$FUNCNAME="_deleteBO";
	
	$cct_access_id  = cct_access_get($sql, $tablename, $prim_id);
	$PROJ_mother_id = 0;
	
  	if ( $tablename == "CONCRETE_ARRAY" ) {
	
		// check if cct_access_id is shared with other objects
		$sqls = "select count(1) from CONCRETE_ARRAY WHERE cct_access_id = ".$cct_access_id;
		$sql->query($sqls, $FUNCNAME);
		$sql->ReadRow();
		$used = $sql->RowData[0];
	
		if ($used>1) {
			// delete the real object (not via CCT_ACCESS_ID)
			if ( $sql->query('DELETE FROM CONCRETE_ARRAY WHERE CONCRETE_ARRAY_ID='.$prim_id, $FUNCNAME) <= 0 ) {
				$error->set($FUNCNAME, 10, 'delete failed for ['.$tablename.':'.$prim_id.'], may be used by other object!');
				return;
			}
			return 0;
		}
  	}
  	if ( $tablename == "PROJ" ) {
  		// get mother project
  		$sql->Quesel('PRO_PROJ_ID from PROJ where PROJ_ID='.$prim_id);
  		$sql->ReadRow();
  		$PROJ_mother_id = $sql->RowData[0];
  	}
	
	// a trigger deletes the Object, after delete of cct_access_id
	if ($sql->query('DELETE FROM cct_access WHERE cct_access_id = '.$cct_access_id, $FUNCNAME) <= 0) {
		$error->set($FUNCNAME, 10, 'delete failed for ['.$tablename.':'.$prim_id.'], may be used by other object!');
		return;
	}
	
	if ( $tablename == "PROJ" and $PROJ_mother_id) {
		// touch mother project
	    $actarr = array('d'=>array('x'=>array('ass'=>array('t'=>'PROJ'))), 'a'=>array('key'=>'del') );
		globObjTouch::touch( $sql, 'PROJ', $PROJ_mother_id, $actarr );
	}
}

/**
 * delete attachments
 */
function _delSattach( &$sql, $tablename, $prim_id ) {

	$attachElems = NULL; // attachment IDs
	$sql->query("SELECT REL_ID FROM SATTACH WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$prim_id);
	while ($sql->ReadRow() ) {
		$attachElems[] = $sql->RowData[0];
	}
	
	if ( !empty($attachElems) ) { 
		$sql->query("delete FROM SATTACH WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$prim_id); // delete all entries
		
		foreach( $attachElems as $dummy=>$relid) {
			$filename = cSattachSubs::getDocumentPath($tablename, $prim_id, $relid);
			if (file_exists( $filename )) {
				unlink($filename);
			}
		}
	}	
}

/**
 * - delete an project-Object 
   - MUST NOT have SUB-PROJECTS => error
   - MUST NOT havwe PROJ_HAS_ELEM elements
 */
function _delete_PROJ(&$sql, $proj_id) { 
	global $error;
	$FUNCNAME="_delete_PROJ";
	

	// check elements (bo's) in proj
	$sql->query('SELECT COUNT(1) FROM proj_has_elem WHERE proj_id = '.$proj_id); 
	if ($error->got(READONLY)) return;
	$sql->ReadRow();

	if ($sql->RowData[0] > 0) {
		$error->set($FUNCNAME, 5, 'Project not deletable because it is not empty.');	
		return;
	}
    
	
	// check for SUB_PROJECTS
    $sql->query('SELECT COUNT(1) FROM proj WHERE pro_proj_id = '.$proj_id); 
    if ($error->got(READONLY)) return;
    $sql->ReadRow();
  
    if ($sql->RowData[0] > 0) {
        $error->set($FUNCNAME, 5, 'Project not deletable because it contains sub-projects.');	
        return;
    }
}

/**
 * manage project links
 * - check links
 * - delete links, if option is set
 * @param object $sql
 * @param string $tablename
 * @param int $prim_id
 * @param int $projID
 * @param array $option
 * @return $error_number
 */
private function _handleProjLinks(&$sqlo, $tablename, $prim_id, $projID, $option) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
  	$hasLinks = $this->_checkProjLinks($sqlo, $tablename, $prim_id, $projID);
  	if ($hasLinks) {
  		if ( $option["projLinksIgnore"]>0 ) {
  			//
  			// delete all project links of object
  			//
  			$sqlAfter = cProjSubs::getObjLinkExcept( $sqlo, 0, $tablename, $prim_id );
  			$sqlsel   = 'PROJ_ID' . $sqlAfter;
  			$sqlo->Quesel($sqlsel);
  			$proj_arr=array();
  			while ($sqlo->ReadRow()) {
  				$proj_id_loop = $sqlo->RowData[0];
  				$proj_arr[]=$proj_id_loop;
  			}
  			if ($this->info_flag) {
  				echo "Info obj_delete(): Delete ".sizeof($proj_arr)." project links ...<br>";
  			}
  			
  			foreach($proj_arr as $proj_id_loop) {
  				$this->proj_elems_lib->setProject( $sqlo, $proj_id_loop );
  				if ($error->Got(READONLY))  {
  					$error->set( $FUNCNAME, 15, 'Access problem on project modification PROJ-ID:'.$proj_id_loop.'!' );
  					return -15;
  				}
  				$this->proj_elems_lib->unlinkObj( $sqlo, $tablename, $prim_id );
  				if ($error->Got(READONLY))  {
  					$error->set( $FUNCNAME, 16, 'Problem on delet of project link on PROJ-ID:'.$proj_id_loop.'!' );
  					return -16;
  				}
  			}
  		} else {
	  		if ($projID) $tmpTxtAdd='an other ';
	  		else $tmpTxtAdd='a ';
	  		$error->set($FUNCNAME, 14, 'object is linked to '.$tmpTxtAdd.'project!');
			return -14;
  		}
  	}
  	return 1;
}

/**
 * - delete an object
 * - check role right object
 * - check, if object is connected to a project (despite $projID)
 * - WARN: will be called recursively
 * @param &$sql,
 * @param $tablename, 
 * @param int $prim_id,     ID of primary key
 * @param string $pkext [optional]  array ( 0 => second PK, 1 => third  PK)
 * @param int $projID [optional] ID of connected project
 * @param $option=NULL  
 *   "deep" => [0] only object
 *  			1  also defined, attached objects
 *   "info" => 1 show HTML info
 *   "projLinksIgnore" : 0,1
 * @return: int error:
 * 		  0 ok
 *       -1 no permission
 *	     -2 no role right
 *       -3 object does not exist
 *       -10 other error
 *       -11 attachment error  
 *       -13 _delete_PROJ
 *       -14 has link to other project
 *		... errors > 50 : warnings!
 * 		 51, 'document attachment exists	
 *		 52, 'error on _delAssOb
 */
function obj_delete ( &$sql, $tablename, $prim_id,  $pkext= NULL, $projID = NULL, $option=NULL ) {
	global   $error;
	$FUNCNAME = "obj_delete";
	
	$tmperr = NULL; // temporary WARNING stack
	  
	if ($_SESSION['userGlob']["g.debugLevel"]>0) $option["info"] = 1;
	$this->info_flag = $option["info"];
	
	if ($this->info_flag) {
		echo "Info obj_delete(): object: ".$tablename.":".$prim_id." ...<br>";
	}
		
	
	$infox = NULL;
	$error = &ErrorHandler::get();
	$isbo  = cct_access_has2($tablename);
  
  // check table-access-rights for user
  $t_rights = tableAccessCheck($sql, $tablename);
  if ( $t_rights["delete"] != 1 ) {
	$error->set($FUNCNAME, 1, "no role right to delete object-type ".$tablename);
	return -2;
  }
  
  // generate combined array of all PKs
  $pk_vals = array(0 => $prim_id);
  if (is_array($pkext))
	$pk_vals = array_merge($pk_vals, $pkext);

  // check if object for deleting exists
  if (!gElement_exists($sql, $tablename, $pk_vals)) {
    $error->set($FUNCNAME, 1, 'object does not exist');
	return -3;
  }
  // check access rights
  $o_rights = access_check($sql, $tablename, $prim_id);	
  if (!$o_rights['delete']) {
    $error->set($FUNCNAME, 1, 'delete failed, no "delete" permission!');
	return -1;
  }
  
  if ( $isbo ) {
  	$tmp_result = $this->_handleProjLinks($sql, $tablename, $prim_id, $projID, $option);
  	if ($tmp_result<0) return $tmp_result;
  }
  
  // do delete
   
  $assocX1 = NULL;
  $assocX2 = NULL;
  
  if ( $tablename == "EXP" ) {  // delete attached images 
	
    //  $sql->query('select img_id FROM img_of_exp WHERE exp_id='.$prim_id);
    // 	while ( $sql->ReadRow() ) {
    // 		$assocX1[] = $sql->RowData[0];
    // 	}
	$sql->query('select concrete_proto_id FROM exp_has_proto WHERE exp_id='.$prim_id);
	while ( $sql->ReadRow() ) {
		$assocX2[] = $sql->RowData[0];
	}
  }
  
  if ($tablename == "PROJ") {
  		$this->_delete_PROJ($sql, $prim_id); // check something
		if ($error->got(READONLY)) {
			$error->set($FUNCNAME, 13, 'delete of Project-object failed!');
			return -13;
		}
  }
  
  if ($isbo) { // delete business-object
  
  	$this->_deleteBO($sql, $tablename, $prim_id); 
	if ($error->Got(READONLY))  {
    	$error->set( $FUNCNAME, 10, "Error occurred on _deleteBO." );
		return -10;
	}
  } else { // delete non-business object
  
	if ($sql->query('DELETE FROM '.$tablename.' WHERE '.primary_select_str2($sql, $tablename, $pk_vals)) <= 0) {
	  $error->set($FUNCNAME, 12, 'delete failed for ['.$tablename.':'.$prim_id.'], may be used by other object!');
      return -12;		
	}
	$tmp_mothertable = mothertable_get2($tablename);
	$actarr = array('d'=>array('x'=>array('ass'=>array('t'=>$tablename))), 'a'=>array('key'=>'del') );
	globObjTouch::touch($sql, $tmp_mothertable, $prim_id, $actarr);
	
  }  
  
  // POST delete actions: object was really deleted!
  
  if ( $tablename == "IMG" ) {  // delete attachment 
    require_once("glob.image.inc");
    if ( imgOnServerExists( $prim_id ) ) {    
         if ($this->info_flag) {
            echo "Info obj_delete(): IMG-ID:$prim_id IMG-attachment detected, will be deleted<br>";
         }
         $tmp_name = imgPathFull( $prim_id );
         if ( unlink ($tmp_name) !== TRUE ) {
         	$tmperr[] = array (53, 'attachment exists, but can not delete!');
         }
         
    }
  }  
  
  if ( $tablename == "LINK" ) {  // delete document-file
  	$tmpPath = $_SESSION['globals']["data_path"] . "/o.LINK.".$prim_id;
    if ( file_exists( $tmpPath ) ) {    
         if ( unlink ($tmpPath) !== TRUE ) {
               $tmperr[] = array (51, 'document attachment exists, but can not delete!');
         }
    }
  }    
  
  if ( $isbo) {  // must be called after the object was really deleted
   		$this->_delSattach($sql, $tablename, $prim_id);
  }  
  
  self::_del_VARIO($sql, $tablename, $prim_id);
  
  if ( $tablename == "EXP" AND $option["deep"] ) {  // delete attached images 

	if ( sizeof($assocX1) ) $this->_delAssObj( $sql, "IMG", $assocX1);
	if ($error->Got(READONLY))  {
		$errLast   = $error->getLast();
     	$error_txt = $errLast->text;
    	$tmperr[] = array (52, 'error on _delAssObj: '.$error_txt);
		$error->reset();
	}
	if ( sizeof($assocX2) ) $this->_delAssObj( $sql, "CONCRETE_PROTO", $assocX2);
  	if ($error->Got(READONLY))  {
		$errLast   = $error->getLast();
     	$error_txt = $errLast->text;
    	$tmperr[] = array (52, 'error on _delAssObj: '.$error_txt);
		$error->reset();
	}
  }

  // if NO severe error, append warnings ...
  if ( is_array($tmperr) AND !$error->Got(READONLY) ) {
	foreach( $tmperr as $valarr) {
		$error->set( $FUNCNAME, $valarr[0], $valarr[1] );
	}
	 
  }

  return 0;
}
      
}
 
