<?php
require_once("object.subs.inc");


/**
 * analyze associated objects of a mother object
 * @package glob.obj.assocObj.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class objAssocObj {

    var $objPool;	/* [ASSOC_TAB][COL] = objects_array  */
    var $foreigns;	/* [$assocTab] = array( "cols"=> array(COLNAME=>FOREIGN_TAB), 
    									    "pkname"=> $pkname )
    				*/
    
    function initTable(&$sql, $tablename) {
    	// FUNCTION: get ASSOC-tables with linked objects
    	// RETURN:   $this->foreigns
    	
    	$this->tablename = $tablename;
    	$assocs = get_assoc_tables2( $sql, $tablename );
    	
    	if (empty($assocs)) return;
    	$this->foreigns = array();
    	
    	foreach( $assocs as $dummy=>$valarr) {
    	
    		$assoc1 = $valarr[0];
    		// analyse table
    		$sqls = "select COLUMN_NAME, CCT_TABLE_NAME, PRIMARY_KEY from CCT_COLUMN where TABLE_NAME='".$assoc1.
    			"'AND CCT_TABLE_NAME is not NULL ";
    		$sql->query($sqls);
    		$foreignCols = array();
    		$pkname      = PrimNameGet2($assoc1) ;
    		
    		while ( $sql->ReadRow() ) {
    			$colname 	  = $sql->RowData[0];
    			$foreignTable = $sql->RowData[1];
    			$pk_pos		  = $sql->RowData[2];
    			if ($pk_pos==1) continue; // not the mother object
    			
    			$forTabIsBo   = cct_access_has2($foreignTable);
    			
    			if (!$forTabIsBo) continue; //is not a BO
    			
    			$foreignCols[$colname] = $foreignTable;
    		}
    		if ( !empty($foreignCols) ) $this->foreigns[$assoc1] = array("cols"=>$foreignCols, "pkname"=>$pkname);
    	}
    	
    
    }
    
    function getAssocsNice() {
    
    	$tmpkomma = "";
    	$retStr   = "";
    	
    	foreach( $this->foreigns as $assocTab=>$valarr) {		
    		$colarr = &$valarr["cols"];
    		foreach( $colarr as $colname=>$foreignTable) {
    			$forNice = tablename_nice2($foreignTable);
    			$retStr  .= $tmpkomma ."<b>".$forNice."</b> (in ".tablename_nice2($assocTab).")";
    			$tmpkomma = "<br>\n";
    		} 
    	}
    	return ($retStr);
    }
    
    function getObjCntNice() {
    
    	$tmpkomma = "";
    	$retStr   = "";
    	if ( $this->objPool==NULL ) return;
    	
    	foreach( $this->objPool as $assocTab=>$colarr) {
    		foreach( $colarr as $colname=>$valarr) {
    			$colcnt = sizeof($valarr);
    			$retStr  .= $tmpkomma .$colname.":".$colcnt;
    		}
    		
    	}
    	return ($retStr);
    }
    
    function typeHasAssocs() {
    	$retval = 0;
    	if ( !empty($this->foreigns) ) $retval = 1;
    	return ($retval);
    }
    
    function initObj($objid) {
    	$this->objid   = $objid;
    	$this->objPool = NULL;
    }
    
    function _getObjectsOneCol( &$sql, $assocTab, $colname, $foreignTable, $pkname ) {
    	// ONE column: get foreign objects from $assocTab : colname=$colname
    	
    	$onPool = NULL;
    	$sqls = "select ".$colname. " from ".$assocTab. " where ".$pkname."=".$this->objid. 
    		    " AND ".$colname.">0";
    	$sql->query($sqls);
    	while ( $sql->ReadRow() ) {
    		$onPool[] = $sql->RowData[0];
    	}
    	
    	if ( !empty($onPool) ) $this->objPool[$assocTab][$colname] = $onPool;
    }
    
    function getForeignObs(&$sql) {
    	// get all foreign objects
    	
    	$this->objPool = NULL;
    	
    	foreach( $this->foreigns as $assocTab=>$valarr) {
    		$foreignCols = &$valarr["cols"];
    		$pkname      = $valarr["pkname"];
    		foreach( $foreignCols as $colname=>$foreignTable) {
    			
    			$this->_getObjectsOneCol($sql, $assocTab, $colname, $foreignTable, $pkname);
    		}
    	}
    }
    
    function &getObjPool() {
    	return ($this->objPool);
    }
    function &getObjDef() {
    	return ($this->foreigns);
    }
    
    function hasObjects() {
    	$retval = 0;
    	if ( !empty($this->objPool) ) $retval = 1;
    	return ($retval);
    }

}
