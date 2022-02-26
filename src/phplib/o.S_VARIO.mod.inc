<?php

require_once ('insert.inc');
require_once ('f.update.inc');
require_once 'glob.obj.touch.inc';

/**
 * S_VARIO modification functions
 * @package o.S_VARIO.mod.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @example
 * $vario_mod_lib = new oS_VARIO_mod();
 * $vario_mod_lib->setObject( $sqlo, 'CONCRETE_SUBST', 32);
 * $vario_mod_lib->updateKeyVal($sqlo, $key, $val);
 */
class oS_VARIO_mod {
    
	var $CLASSNAME='oS_VARIO_mod';
	private $isTouched;
	private $mothIsBo;

    /**
     * init object
     * @param object $sqlo
     * @param string $tablename
     * @param int $id
     * @param array $options  array('noAccCheck'=>0,1)
     * @return -
     */
    function setObject( &$sqlo, $tablename, $id, $options=NULL ) {
    	global $error;
    	$FUNCNAME= $this->CLASSNAME.':';
    	$this->tablename = $tablename;
    	$this->id = $id;
    	$this->mothIsBo  = cct_access_has2($tablename); 
    	$this->isTouched = 0;
    	
    	// check access
    	if ($options['noAccCheck']>0) {
    		// nothing
    	} else {
    		$o_rights = access_check($sqlo, $tablename, $id);
    		$right='insert';
    		if ( !$o_rights[$right]) {
    			$error->set( $FUNCNAME, 1, 'You do not have '.$right.
    				' permission on object '.$tablename.' ID:'. $id.' !' );
    			return;
    		}
    	}
    }
    
    // @param int $flag 0,1
    function set_touched($flag) {
        $this->isTouched = $flag;
    }
    
    // delete an entry by KEY
    function delete_entry($sqlo, $key) {
        $tablename=$this->tablename;
        $id=$this->id;
        $sqlo->Deletex ( 'S_VARIO', "TABLE_NAME='".$tablename."' AND OBJ_ID=".$id." AND KEY=". $sqlo->addQuotes($key));
    }
    
    /**
     * update one value
     * - check if key exists:
     *  - yes: update
     *  - no:  insert
     * @param $sqlo
     * @param $key
     * @param $val
    
     */
    function updateKeyVal(&$sqlo, $key, $val) {
    	$tablename=$this->tablename;
    	$id=$this->id;
    	
    	$sqlo->Quesel("VALUE FROM S_VARIO WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$id.
    		 " AND KEY=". $sqlo->addQuotes($key) );
    	if ( $sqlo->ReadRow() ) {
    		
    		// exists: UPDATE or DELETE
    	    if ($val===NULL or $val==='') {
    	        // DELETE
    	        $this->delete_entry($sqlo, $key);
    	    } else {
    	        // UPDATE
        		$argu=NULL;
        		$argu['VALUE'] =$val;
        		$idarr= array('TABLE_NAME'=>$tablename, 'OBJ_ID'=>$id, 'KEY'=>$key);
        		$sqls = gObjUpdate::update_row_sGet($sqlo, 'S_VARIO', $argu, $idarr);
        		if ( $sqls!="" ) $sqlo->query($sqls);
    	    }
    	} else {
    		// INSERT
    		$argu=NULL;
    		$argu['TABLE_NAME']=$tablename;
    		$argu['OBJ_ID']=$id;
    		$argu['KEY']   =$key;
    		$argu['VALUE'] =$val;
    		$sqlAfter = insert_row_Get( $sqlo, 'S_VARIO', $argu );
    		$sqls   = "insert into S_VARIO ".$sqlAfter;
    		$sqlo->query($sqls);
    		
    	}
    	
    	$this->_touchTest( $sqlo );
    }
    
    /**
     * test for a touch-action
     * @param array $opts : 
     *		"do" => 0,1 -- do TOUCH ?
     * @global  $this->_noTouch
     */
    function _touchTest( &$sqlo,$opts=NULL ) {
    	
    	if ( !$this->mothIsBo ) {
    		return;
    	}
    	
    	$tablename = $this->tablename;
    	$id = $this->id;
    	$doTouch = 0;
    	
    	if ( !$this->isTouched ) {
    		$doTouch = 1;
    	}
    	
    	if ( $doTouch ) {
    		$actarr = array('d'=>array('x'=>array('var'=>NULL)), 'a'=>array('key'=>'mod') );
    		globObjTouch::touch( $sqlo, $tablename, $id, $actarr );
    	}
    	$this->isTouched++;
    	
    }


}