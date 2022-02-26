<?php
/**
 * contains functions to act with EXTRA_OBJ, EXTRA_CLASS, EXTRA_ATTRIB
 * @package db_x_obj.inc
 * @swreq SREQ:0000252: o.EXTRA_OBJ > helper classes/methods 
 * @author  Steffen Kube (steffen@blink-dx.com)
 */

require_once('varcols.inc');
require_once('globals.inc');
require_once ('o.S_VARIO.subs.inc');


class fVarcolConabs {
    
    /**
     * get planned extra class for an concrete objects from an abstract object
     * @param object $sqlo
     * @param string $tablename
     * @param int $objid
     */
    public function get_class_for_conc($sqlo, $tablename, $objid) {
        global $error;
        
        $key='ClassID_for_Concrete';
        $vario_lib = new oS_VARIO_sub($tablename);
        $class_id  = $vario_lib->getValByKey( $sqlo, $objid, $key );
        
        if (!$class_id) return;
        
        $varcol     = & Varcols::get();
        $classname  = $varcol->class_id_to_name( $class_id );
        if ($error->got()) {
            $error->reset();
            return;
        }
        if ($classname==NULL) {
            return;
        }
        
        return $class_id;
    }
}


/**
 * manage class parameters, update class parameters with this class
 * @author steffen
 *
 */
class fVarcolMeta {
	
	var $EXTRA_OBJ_ID; // EXTRA_OBJ_ID of current object
	
	function __construct(&$sqlo, $tablename=NULL, $id=NULL ) {
	    $this->debuglevel = array_key_exists('g.debugLevel', $_SESSION['userGlob']) ? $_SESSION['userGlob']['g.debugLevel'] : 0;
	    if ($tablename!=NULL) {
	        $this->setObj( $sqlo, $tablename, $id );
	    }
	}

    function setObj( &$sqlo, $tablename, $id ) {
    	$this->tablename = $tablename; 
    	$this->id 		 = $id;
    	
    	
    }
    
    // object is initialized
    function _isInit() {
    	if ($this->tablename==NULL or $this->id==NULL) return 0;
    	else return 1;
    }
    
    // get EXTRA_OBJ info from $extra_obj_id (OLD: get_xtraobjid_from_argu)
    static function get_args_by_id( &$sql,  $extra_obj_id) {
        global $varcol;
        $varcol = & Varcols::get();
        
        $extra_class_id = NULL;
        $arguobj        = NULL;
        
        if ($extra_obj_id) {
            $e_vals  = $varcol->select_by_name ($extra_obj_id);
            $arguobj = $e_vals["values"];
            $extra_class_id = $e_vals["extra_class_id"];
        }
        return ( array(
            "extra_obj_id"  => $extra_obj_id,
            "extra_class_id"=> $extra_class_id,
            "arguobj"       => $arguobj // by name !!!
        ));
    }
    
    private function _obj_extraid_update(&$sql, $tablename, $extra_obj_id, $idname, $id) {
        # return: void, errors through error handler
        # errors:
        #         extra_object 100 .. id of object having extra_object is equal to or smaller than zero.
        #                      104 .. can't update extra-object-id in main_obj_table
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        $error = & ErrorHandler::get();
        
        if ($id <= 0) {
            $error->set($FUNCNAME, 100, 'ID of object having extra_object is equal to or smaller than zero.');
            return;
        }
        $sql->Updatex( $tablename, 'extra_obj_id = '.$extra_obj_id.' WHERE '.$idname.' = '.$id);
        if ($error->got()) {
            $error->set($FUNCNAME, 104, 'Can\'t update extra-object-id of '.tablename_nice2($tablename).' for primary key = '.$id);
            return;
        }
    }
    
    
    /**
    # update extra_obj and main_obj, which has link to extra_obj
    # input:
    #        EXTRA_OBJ_ID .. existing extra_obj_id; can be NULL
    #        class_id     .. if NULL -> delete; otherwise -> insert/update
    #        tablename    .. name of main_obj-table
    #        idname       .. primary key name of main_obj-table
    #        id           .. primary key value of main_obj-table
    #        arguobj      .. array (attribute_id => attribute_value, ...) attribute-IDs: (INT)
    # return: void, errors through error-handler
    # errors:
    #         extra_object 100 .. id of object having extra_object is equal to or smaller than zero.
    #                      101 .. can't delete extra object
    #                      102 .. error at insert of extra object
    #                      103 .. extra object not existing
    #                      104 .. can't update extra-object-id in main_obj_table
    #                      105 .. error at update of extra object
    */
    function extra_update( &$sql, $EXTRA_OBJ_ID,   $class_id,   $arguobj   ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $debuglevel = $this->debuglevel;
        $varcol     = & Varcols::get();
        $error      = & ErrorHandler::get();
        
        if ( !$this->_isInit() ) {
            $error->set($FUNCNAME, 90, 'no object initialized.' );
            return;
        }
        
        
        $tablename    = $this->tablename;
        $id			  = $this->id;
        $idname     = PrimNameGet2($tablename);
        
        if (!$class_id) { // delete extra_object
            if ($EXTRA_OBJ_ID) {
                if ($debuglevel>3) echo 'DEBUG: extra_update: delete EXTRA_OBJ<br>';
                $this->_obj_extraid_update( $sql, $tablename, 'NULL', $idname, $id);
                if ($error->got(READONLY)) return;
                $varcol->delete($EXTRA_OBJ_ID);
                if ($error->got()) {
                    $error->set($FUNCNAME, 101, 'Can\'t delete extra object of '.tablename_nice2($tablename).' where primary key = '.$id);
                    return;
                }
            }
        } else {	// class_id existing, so do update or insert
            if (!$EXTRA_OBJ_ID) { // do insert
                if ($debuglevel>3) echo 'DEBUG: extra_update: insert EXTRA_OBJ<br>';
                $EXTRA_OBJ_ID = $varcol->insert ($class_id, $arguobj);
                if ($error->got()) {
                    $error->set($FUNCNAME, 102, 'Error at insert of extra object for '.tablename_nice2($tablename).' where primary key = '.$id);
                    return;
                }
                $this->_obj_extraid_update( $sql, $tablename, $EXTRA_OBJ_ID, $idname, $id);
                if ($error->got(READONLY)) return;
            } else { // do update
                if ( $debuglevel>3 ) echo 'DEBUG: extra_update: update EXTRA_OBJ<br>';
                // check first, if same class
                $class_id_old = $varcol->obj_id_to_class_id($EXTRA_OBJ_ID);
                if ($error->got()) {
                    $error->set($FUNCNAME, 103, 'Extra-object not existing.');
                    return;
                }
                if ($class_id_old != $class_id) { // replace extra object
                    $this->_obj_extraid_update( $sql, $tablename, 'NULL', $idname, $id);
                    if ($error->got(READONLY)) return;
                    $varcol->delete ($EXTRA_OBJ_ID);
                    if ($error->got()) {
                        $error->set($FUNCNAME, 101, 'Can\'t delete extra object of '.tablename_nice2($tablename).' where primary key = '.$id);
                        return;
                    }
                    $EXTRA_OBJ_ID = $varcol->insert ($class_id, $arguobj);
                    if ($error->got()) {
                        $error->set($FUNCNAME, 102, 'Error at insert of extra object for '.tablename_nice2($tablename).' where primary key = '.$id);
                        return;
                    }
                    $this->_obj_extraid_update( $sql, $tablename, $EXTRA_OBJ_ID, $idname, $id);
                    if ($error->got(READONLY)) return;
                } else { // do normal update
                    $varcol->update ($EXTRA_OBJ_ID, $arguobj, $class_id);
                    if ($error->got()) {
                        $error->set($FUNCNAME, 105, 'Error at update of extra object of '.tablename_nice2($tablename).' where primary key = '.$id);
                        return;
                    }
                }
            }
        }
    }
    
    
    /**
     * - update an BO with extra_object params
     * - ( OLD class can be empty ) OR ( OLD class == NEW class )
     * - global-input: $this->EXTRA_OBJ_ID
     * @todo: for future: on CLASS=NULL => do NOT delete extra_obj-id, try intelligent update ...
     * @param 
     *		 $class_name, 	string or NULL (than delete the old extra_data)
     *		 $vals           attributes [name] = val 
     * @return  void
     * @throws Exception int 
     *  100 .. id of object having extra_object is equal to or smaller than zero.
         101 .. can't delete extra object
         102 .. error at insert of extra object
         103 .. extra object not existing
         104 .. can't update extra-object-id in main_obj_table
         105 .. error at update of extra object
         106 .. no object initialized.
     */
    function updateByClsName( &$sqlo, $class_name, $vals ) {
    	global  $error, $varcol;
    	$varcol     = & Varcols::get();
    	$error      = & ErrorHandler::get();
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	// get current EXTRA_OBJ_ID
    	$tablename    = $this->tablename;
    	$id			  = $this->id;
    	$idname 	= PrimNameGet2($tablename);
    	$this->EXTRA_OBJ_ID = glob_elementDataGet( $sqlo, $tablename, $idname, $id, 'EXTRA_OBJ_ID');
    	
    	$EXTRA_OBJ_ID = $this->EXTRA_OBJ_ID;
    	
    
    	if ( !$this->_isInit() ) {
    		$error->set($FUNCNAME, 106, 'no object initialized.' );
    		return;
    	}
    	$idname 	= PrimNameGet2($tablename);
    	
    	if ($class_name==NULL) { // delete extra_object
    	    if ($EXTRA_OBJ_ID ) {
    	        if ($this->debuglevel>3) echo 'DEBUG:'.$FUNCNAME.': delete EXTRA_OBJ<br>';
    	        $this->_obj_extraid_update( $sqlo, $tablename, 'NULL', $idname, $id);
    	        if ($error->got(READONLY)) return;
    	        $varcol->delete($EXTRA_OBJ_ID);
    	        if ($error->got()) {
    	            $error->set($FUNCNAME, 101, 'Can\'t delete extra object of (T:'.tablename_nice2($tablename).',ID:'.$id.')');
    	            return;
    	        }
    	    }
    	    return;
    	}
      
    	if (!$EXTRA_OBJ_ID) { // do insert
    	
    	  $EXTRA_OBJ_ID = $varcol->insert_by_name ($tablename, $class_name, $vals);
    	  if ($error->got()) {
    		$error->set($FUNCNAME, 102, 'Error at insert of extra object for '.$tablename.' where ID='.$id);
    		return;
    	  }
    	  $this->_obj_extraid_update( $sqlo, $tablename, $EXTRA_OBJ_ID, $idname, $id);
    	  if ($error->got(READONLY)) return;
    	  
    	} else { // do update
    	
    		// check first, if same class	
    		$class_name_old = $varcol->obj_id_to_class_name($EXTRA_OBJ_ID);
    		if ($error->got()) {
    			$error->set($FUNCNAME, 103, 'Extra-object not existing.');
    			return;
    		}
    		if ($class_name_old != $class_name) { // OLD != NEW class		
    			$error->set($FUNCNAME, 106, 'old class [name:$class_name_old] is unlike the new class [name:$class_name] for '.$tablename.' where ID='.$id);
    			return;
    		} 
    	 
    		$varcol->update_by_name($EXTRA_OBJ_ID, $vals);	
    		if ($error->got()) {
    			$error->set($FUNCNAME, 105, 'Error at update of extra object of '.$tablename.' where ID='.$id);
    			return;
    		}
    	  
    	}
      
    }

}
