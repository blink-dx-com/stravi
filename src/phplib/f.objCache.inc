<?php
require_once ("glob.obj.usagegui.inc");

/**
 * - manage the object cache $_SESSION['s_objCache']
 * - format: array[table][id] => array( 
 *     't'=>mother table, 'id'=> , 'found' => int
 *    )
 *    
 * @var moObj_STRUCT:
 *    array('t'=>$moTable, 'id'=>$moObjIdGot, 'found'=> 0, 1 or 2)
 *    
 * $Header: trunk/src/phplib/f.objCache.inc 59 2018-11-21 09:04:09Z $
 * @package f.objCache.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class fObjCache {
 	
var $MAX_ENTRIES = 20;

/**
 * set 's'=>1, flags a SCAN miss
 * @return 
 * @param object $table
 * @param object $id
 */
function _setScanMiss( $table, $id ) {
	$cachePoi = &$_SESSION['s_objCache'][$table];
 	//$subarr = $cachePoi[$id];
	
 	if (is_array($cachePoi)) {
	   $numObjPerTab = sizeof( $cachePoi );
 	} else $numObjPerTab=0;
 	
	if ($numObjPerTab>$this->MAX_ENTRIES) {
		// remove one element
		$firstid = key($cachePoi);
		unset ($cachePoi[$firstid]);
		
	}
	$cachePoi[$id] = array('s'=>1);
}

function _doObjCheck( &$sqlo, $table, $id, $moTable ) {
	$sqlsel = "x.column_name from cct_column x ".
			" WHERE x.cct_table_name='". $table ."' AND x.table_name='".$moTable."'";
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$objColname = $sqlo->RowData[0];
	
	$moPkName = PrimNameGet2($moTable);
	// check object-link
	$sqlsel = $moPkName ." from ".$moTable." where ".$objColname."=".$sqlo->addQuotes($id);
	$sqlo->Quesel($sqlsel);
	$found = 0;
	if ( $sqlo->ReadRow() ) { // TBD: check all mothers ???
		$moObjIdGot = $sqlo->RowData[0];
		$found = 1;
		if ( $sqlo->ReadRow() ) $found = 2;
	}
	return array($moObjIdGot, $found);
}
 	
function getMother( &$sqlo, $table, $id ) {
 	$cachePoi = &$_SESSION['s_objCache'];
 	$subarr = $cachePoi[$table][$id];
 	return ($subarr);
}
 
function _checkExists() {
 	$cachePoi = &$_SESSION['s_objCache'];
 	
}
 
 /**
  * set mother object without test
  * @return 
  * @param object $sqlo
  * @param object $table
  * @param object $id
  * @param object $moTable
  * @param object $moId
  */
function setMother( $table, $id, $moTable, $moId, $found=0 ) {
 	$cachePoi = &$_SESSION['s_objCache'][$table];
 	$subarr = $cachePoi[$id];
 	if ($subarr['id']==$moId ) return;
	
 	if (is_array($cachePoi)) {
    	$numObjPerTab = sizeof( $cachePoi );
    	if ($numObjPerTab>$this->MAX_ENTRIES) {
    		// remove one element
    		$firstid = key($cachePoi);
    		unset ($cachePoi[$firstid]);
    		
    	}
 	}
 	$cachePoi[$id] = array('t'=>$moTable, 'id'=>$moId, 'found'=>$found);
}

/**
 * - guess the mother, if given $moTable, $moId are valid: return them
 * 1. if MOTHER is input: use it !
 * 2. get from cache
 *   - if scan-flag==1 : do not scan again
 *   - check relation
 * 3. scan database
 * @param object $sqlo
 * @param object $table
 * @param object $id
 * @param object $moTable
 * @param object $moId
 * @return array $moObj
 *    array('t'=>$moTable, 'id'=>$moObjIdGot, 'found'=> 0, 1 or 2);

 */
function guessMother( &$sqlo, $table, $id, $moTable, $moId, $found=0 ) {
	
	if ( $moTable!=NULL and $moId!=NULL  ) {
		// get mother from cache
	    $this->setMother( $table, $id, $moTable, $moId, $found );
	}
	
	$cachePoi = &$_SESSION['s_objCache'];
 	$subarr = $cachePoi[$table][$id];
	
	if ( $subarr['t']!=NULL ) {
		$moTable = $subarr['t'];
		list($moObjIdGot, $found) = $this->_doObjCheck( $sqlo, $table, $id, $moTable );
		if ($moObjIdGot>0) {
			// if ($moObjIdGot!=$moId) {
			// TBD: echo "WARN:hasMother() mother changed.<br>";
		    $moObj = array('t'=>$moTable, 'id'=>$moObjIdGot, 'found'=>$found);
			$this->setMother( $table, $id, $moTable, $moObjIdGot );
			return ($moObj);
		}
	}
	if ( $subarr['s']>0 ) {
		// was scanned before: NEGATIVE
		return;
	}
	
	// full scan
	$objSearch = new object_usage($sqlo, $table, $id, 0 );
	$objSearch->analyseAll( $sqlo );
	$obj_num_all = $objSearch->obj_num_all;
	
	if (!$obj_num_all) {
		$this->_setScanMiss( $table, $id );
		return;
	}
	
	$usedTabs = $objSearch->getUsedTabs();
	$moTable  = key($usedTabs);
	
	if ($obj_num_all>1) {
		// more than one mother
	    $moObj = array('t'=>$moTable, 'found'=>2);
		return ($moObj);
	}
	// $numUse==1
	
	// $cnt= current($usedTabs);
	
	
	list($moObjIdGot, $found) = $this->_doObjCheck( $sqlo, $table, $id, $moTable );
	if ($moObjIdGot>0) {
		// if ($moObjIdGot!=$moId) {
		// TBD: echo "WARN:hasMother() mother changed.<br>";
	    $moObj = array('t'=>$moTable, 'id'=>$moObjIdGot, 'found'=>$found);
		$this->setMother( $table, $id, $moTable, $moObjIdGot );
		return ($moObj);
	}
}

/**
 * 
 * @param object $sqlo
 * @param string $table
 * @param int $id
 * @param string|array $moTable one or more mother tables
 * @return array moObj_STRUCT or NULL
 */
public function guessMotherOneTab(&$sqlo, $table, $id, $moTable) {
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    $cachePoi = &$_SESSION['s_objCache'];
    $subarr = $cachePoi[$table][$id];
    
    if(!is_array($moTable)) {
        $moTables = array($moTable);
    } else $moTables = $moTable;
    
    // look into cache  ...
    foreach($moTables as $moTable) {
        
        if ( $subarr['t']==$moTable ) {
            $moTable = $subarr['t'];
            if ($subarr['id']) {
                return $subarr;
            }
        }
    }
    
    // deep scan ...
    foreach($moTables as $moTable) {
        
        if ( $subarr['s']>0 ) {
            // was scanned before: NEGATIVE
            return;
        }

        list($moObjIdGot, $found) = $this->_doObjCheck( $sqlo, $table, $id, $moTable );
        debugOut("DeepScan: $table:$id mo:$moTable result:$found", $FUNCNAME, 3);
        
        if ($moObjIdGot>0) {
            $moObj = array('t'=>$moTable, 'id'=>$moObjIdGot, 'found'=>$found);
            $this->setMother( $table, $id, $moTable, $moObjIdGot, $found );
            return ($moObj);
        }
    }
    
    
}
 
}
