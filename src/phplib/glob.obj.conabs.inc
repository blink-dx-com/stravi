<?php
/**
 * handle abstract - concrete - abstract objects (templates)
 * @package glob.obj.conabs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/glob.obj.conabs.inc 59 2018-11-21 09:04:09Z $
 */
class gConcAbsC {
	
function __construct() {
	// OBJ_TEMPLATE mapper  
	$this->tab_mapper['BCBATCH']    	= 'BCBATCH_TMPL' ;
	$this->tab_mapper['CHIP_READER']    = 'ABS_CHIP_READER' ;
	$this->tab_mapper['CONTAINER']      = 'ABS_CONTAINER' ;
	$this->tab_mapper['EXP']            = 'EXP_TMPL' ;
	$this->tab_mapper['CONCRETE_ARRAY'] = 'ABSTRACT_ARRAY' ;
	$this->tab_mapper['CONCRETE_PROTO'] = 'ABSTRACT_PROTO' ;
	$this->tab_mapper['CONCRETE_SUBST'] = 'ABSTRACT_SUBST' ;
	$this->tab_mapper['CYTCART']        = 'ABS_CYTCART' ;
	$this->tab_mapper['CHIP_READER']    = 'A_CHIP_READER' ;
	$this->tab_mapper['SOCKET']         = 'ABS_SOCKET' ;
	$this->tab_mapper['WORKLIST']    	= 'ABS_WORKLIST' ;

	$this->tab_absmap = array_flip($this->tab_mapper);
}

function abs2concrete($abstractTable) {
	return $this->tab_absmap[$abstractTable];
}

/**
 * get conc_abs type: 'conc' or 'abs' or '?'
 * @param string $table
 */
function table_type($table) {
    
    $output='?';
    if (in_array($table, $this->tab_absmap)) {
        $output='conc';
        return $output;
    }
    if (in_array($table, $this->tab_mapper)) {
        $output='abs';
        return $output;
    }
    return $output;
}

function getTemplate($tablename) {
	return ( $this->tab_mapper[$tablename] );
}

/**
 * get ABSTRACT-object from CONCRETE-object
 * @swrequ UREQ:12963 Material Monitoring Concept 2018
 * @param object $sqlo
 * @param string $table  CONCRETE table
 * @param int $c_obj_id
 * @return array $abs_info
 *   ('t'=>abstract table, 'id'=>$absObjID );
 */
function getAbsFrConc(&$sqlo, $table, $c_obj_id) {
	
	$tmplTable = $this->getTemplate($table);
	if ($tmplTable==NULL) return array();
		
	$pkName = PrimNameGet2($table);
	$fkName = PrimNameGet2($tmplTable);
	$fkColFeatures = colFeaturesGet2($table, $fkName );
	
	if ($fkColFeatures==NULL) return array(); // column not defined
	
	// get abstract objid
	$absObjID = glob_elementDataGet( $sqlo, $table, $pkName, $c_obj_id, $fkName);
	return array('t'=>$tmplTable, 'id'=>$absObjID);
}

/**
 * get TMPL_TABLE,COLNAME of TMPL_OBJECT in TABLE
 * @param string $table
 * @return void|mixed
 */
function getAbsColName($table) {
    $tmplTable = $this->getTemplate($table);
    if ($tmplTable==NULL) return;
    $fkName = PrimNameGet2($tmplTable);
    return array($tmplTable, $fkName);
}

function getConcFrAbsSql($tablename, $id) {
	// RETURN: $sqlAfter: e.g. " EXP where TMPL_ID=xxx"
	// USGAE: $sqls = "select x.EXP_ID from ".$sqlAfter
			
	$tmpl_name = $this->tab_mapper[$tablename];
	if ($tmpl_name=="") return;
	 
	$c_id = $tablename."_ID";
	$a_id = $tmpl_name."_ID";
	
	$sqlAfter = $tablename." x, ".$tmpl_name." a ".
		    " where a.".$a_id."=".$id. " AND x.".$a_id."=a.".$a_id;
	return ($sqlAfter);

}

}