<?php
/*
 *  get features of an object or assoc tables
 *  see also for general TABLE info: table.info.inc
 *  @package object.info.inc
 */

require_once ('object.subs.inc');
require_once ('o.S_VARIO.subs.inc');

/** 
 * @brief get features of an object or assoc tables
 * 
 * - read features of ONE object or ASSOC-table-elements from database
 * - objFeatStruct is used by class insertC

 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @var TYPEOF objFeatStruct -- Object Feature Structure (short: OFS)  <pre>
 *   = array (  
		"vals" => data, 
	    "xobj" => extra_obj_data, = ('extra_class_id' => xxx, 'values' => array(attribName => value, ...) )
	    "access" => access_data
	    "ass"  => associated elements
	            array( $ASSOC_TABLE=> array(key=>val) )
		)
  </pre>
*/
class objInfoC {

/**
 * get RAW features of object ("vals", "xobj", "access", "vario")
 *
 * - does NOT check CCT_COLUMN (e.g. hidden columns)
 * - to get "xobj": set col:EXTRA_OBJ_ID
 * - to get "access": set col:CCT_ACCESS_ID
 * @param $sql
 * @param $table
 * @param int $id ID of object
 * @param $option <pre>
 *   "xObjParById" : [0]  get by ATTRIB_NAME
 *          1 : get ATTRIB_ID
 *    "pk2" : PK2
 *    "pk3" : PK3
 *    "types" : convert a column to a special format (e.g. date)
 *   			- array (COL => TYPE)  TYPE= "DATE1", ....
 *				- is ignored, if "typeAuto" is set
 *    "typeAuto" : [0], 1 : analyse format automatically, e.g. for "DATE"
 *				   ignores "types"
 *    "cols" : give feature columns
 *    "vario": 0,1 get vario data ?
 * </pre>
 * @return  array objFeatStruct (TYPEOF)
 */
static function getParamsRaw(&$sql, $table, $id, $option=NULL ) {
   global $error, $varcol;
   $FUNCNAME= __CLASS__.':'.__FUNCTION__;
   
   $komma   = "";
   $colsql  = "";
   $xobj_id = 0;
   $xparam  = NULL;
   $morepk  = "";
   $retvals = NULL;
   $pkColName = PrimNameGet2($table);
   $retDic  = array();
   
   if ($pkColName==NULL) {
       $error->set( $FUNCNAME, 1, 'Table "'.$table.'": no PK-col found.' );
       return;
   }
   
   if ($option["pk2"]!="" OR $option["pk3"]!="") {
   		$pk_arr  = primary_keys_get2($table);
		if ($option["pk2"]!="") $morepk .= " AND ".$pk_arr[1] ."='".$option["pk2"]."'";
		if ($option["pk3"]!="") $morepk .= " AND ".$pk_arr[2] ."='".$option["pk3"]."'";
   }
   
	if ($option["cols"]==NULL) $cols = columns_get2($table); 
	else $cols = $option["cols"];
	
	$optTypeArr = $option["types"];
	// generate auto-format, e.g. for DATE
	if ($option["typeAuto"]>0) {
		$optTypeArr = NULL;
		// TBD: may cache this array or use cached function !!! ...
		$colTypes = appRawTypeGet($sql,$table); 
		
		foreach( $colTypes as $col=>$typex) {
			if ($typex=='DATE') $optTypeArr[$col]="DATE1";
		}
		
	}
   
   foreach( $cols as $colname) {
   
   		if ( is_array($optTypeArr) ) {
			// colname could NOT be changed by TYPE-conversion!
			$xtype = $optTypeArr[$colname];
			if ($xtype!="") {
				if ($xtype=="DATE1") {
					$colname = $sql->Sql2DateString($colname, 1).' '.$colname;  // alias name!
				}
			}
		}
   		$colsql .= $komma . $colname;
        $komma = ", ";
   }
  
   
   $sqls = "select ".$colsql." from ".$table. " where ".$pkColName. "=".$id. $morepk;
   $sql->query($sqls);
   // readout array[COL] = VAL
   if ( !$sql->ReadArray() ) {
   		$error->set('objInfoC::getParams()', 1, "Object [ID:$id] not found");
        return;
   } 
   $retvals = $sql->RowData;
   
   $retDic["vals"] = $retvals;

   if ( !empty($retvals['EXTRA_OBJ_ID']) ) {
        $xobj_id = $retvals['EXTRA_OBJ_ID'];
   }
   
   if ($xobj_id) {
   		$varcol  = &Varcols::get();
        if ($option["xObjParById"]) $xparam = $varcol->select_by_id  ($xobj_id);   // by ATTRIB_ID
		else 					    $xparam = $varcol->select_by_name($xobj_id); // by ATTRIB_NAME
		$retDic["xobj"] = $xparam;
   }
   
   if ( !empty($retvals['CCT_ACCESS_ID']) ) {
		$sqlsel = "* from CCT_ACCESS where CCT_ACCESS_ID=".$retvals['CCT_ACCESS_ID'];
		$sql->Quesel($sqlsel);
		$sql->ReadArray();
		$retDic["access"] = $sql->RowData;
   }
   
   if ($option['vario']>0) {
   	    $varioLib = new oS_VARIO_sub($table);
   	    $varioarr = $varioLib->getAllVals( $sql, $id );
   	    $retDic['vario']=$varioarr;
   }
   
   return ($retDic);
  
}


/**
 * get CLASS-ID (of EXTRA_OBJ_ID) of object
 */
function get_class_id(&$sqlo, $tablename, $id) {
    
    $class_id = 0;
    $pkname = PrimNameGet2($tablename);
    $extra_obj_id = glob_elementDataGet( $sqlo, $tablename, $pkname, $id, 'EXTRA_OBJ_ID');
    if ($extra_obj_id) {
        $varcol  = &Varcols::get();
        $class_id  = $varcol->obj_id_to_class_id( $extra_obj_id );
    }
    return $class_id;
}

/**
 * get all parameters from BO:EXTRA_OBJ
 * @param  $sql
 * @param  string $table tablename of object
 * @param  int $id    id of object
 * @return $vals = array (
 //			 "class_name" =>
 //			 "extra_class_id" =>
 //			 "values"     =>
 //			 "extra_obj_id" =>
 //			)
 */
function class_params( &$sql, $table, $id  ) {
    global $error, $varcol;
    
    $error  = & ErrorHandler::get();
    $varcol = & Varcols::get();
    
    $vals = NULL;
    $pkname =  $table."_ID";
    $sqls = "select extra_obj_id from ".$table." where ".$pkname."=".$id;
    $sql->query($sqls);
    if ( $sql->ReadRow() ) {
        $extra_obj_id = $sql->RowData[0];
        if ($extra_obj_id) {
            $vals =  $varcol->select_by_name ($extra_obj_id);
            $vals["extra_obj_id"] = $extra_obj_id;
            if ( $vals["extra_class_id"] ) {
                $sqls= "select name from EXTRA_CLASS where TABLE_NAME='".$table."' AND EXTRA_CLASS_ID=".$vals["extra_class_id"];
                $sql->query("$sqls");
                $sql->ReadRow();
                $tmpname = $sql->RowData[0];
                $vals["class_name"] = $tmpname;
            }
        }
    }
    
    return ($vals);
}   

/**
 * get all associated values (from columns $colnames) from $assocTable
 * - no check: if columns exists
 * - example: $a = getAssocVals($sqlo, 'EXP', 6362, 'EXP_HAS_PROTO', array('STEP_NO', 'CONCRETE_PROTO_ID') )
 * @param object $sqlo
 * @param $table
 * @param $id
 * @param string $assocTable associated table name
 * @param $colnames array of column names
 * @return array of rows(column=>val)
 */
function getAssocVals(&$sqlo, $table, $id, $assocTable, $colnames) {
	
	$colstr = implode(', ',$colnames);
	$pkname = PrimNameGet2($assocTable);
	
	$sqlsel = $colstr . " from ".$assocTable." where ".$pkname."=".$id;
	$sqlo->Quesel($sqlsel);
	$rows=NULL;
	while ( $sqlo->ReadArray() ) {
		$valarr = $sqlo->RowData;
		$rows[] = $valarr; 
	}
	
	return $rows;
}


/**
 *  // array[COLUMN] = ID/VAL
 *  // array( 'extra_class_id' => val, 'values' => array(attrib_id=>val) )
 */


/**
 *  return nice-names/values 
    		 - remove CCT_ACCESS_ID 
   			 - change "EXTRA_OBJ_ID" to class info
    		 - niceval is empty, if no special foreign value !!!   
 * @param &$sql, 
 * @param string $table, 
 * @params array &$params 
 *   "vals", "xobj", "access", "vario"
 * @return   array( 'vals'=>$params, 'xobj'=>extra_obj params ) 
		$params[COL]      = array('valnice'=>, 'colnice'=> )
    	$retXarr[ATTR_ID] = array('valnice'=>, 'colnice'=> attribute name)
 */
function paramRaw2nice(&$sql, $table, &$params ) { 

    global $varcol;
    
    $retarr  = NULL;  // extra_obj_params
    
    if (!empty($params['vals'])) $retarr['vals']=array();
    foreach( $params['vals'] as $colName=>$val) {
        
   		if ( $colName=="CCT_ACCESS_ID" OR $colName=="EXTRA_OBJ_ID") continue;
        $colFeat     = colFeaturesGet( $sql, $table, $colName);
        $linked_tab  = $colFeat["CCT_TABLE_NAME"];
        $colNameNice = $colFeat["NICE_NAME"];
        
        if ( $linked_tab!="" and $val!=NULL ) {
        	$nicename = obj_nice_name( $sql, $linked_tab, $val );
        } else { 
        	$nicename = $val;
        }
        $retarr['vals'][$colName] = array('valnice'=>$nicename, 'colnice'=>$colNameNice);
    }
    
    
    if (!empty($params['xobj'])) {
        if ( sizeof($params['xobj']) ) {
            $varcol   = &Varcols::get();
            $class_id = $params['xobj']['extra_class_id'];
            $attrvals = &$params['xobj']['values'];
            $attrNames= $varcol->get_attrib_names($class_id);
            $classNice= $varcol->class_id_to_nice_name($class_id);
            
            $retarr['vals']["EXTRA_OBJ_ID"] = array( 'valnice'=>$classNice, 'colnice'=>"class" ); 
            
            if (sizeof($attrvals)) foreach( $attrvals as $attrid=>$val) {
                $retarr['xobj'][$attrid] = $attrNames[$attrid];
            }
           
        }
    }
    if (!empty($params['access'])) {
        $retarr['access']=array();
        
        foreach( $params['access'] as $colName=>$val) {
            $colFeat     = colFeaturesGet( $sql, 'CCT_ACCESS', $colName);
            $linked_tab  = $colFeat["CCT_TABLE_NAME"];
            $colNameNice = $colFeat["NICE_NAME"];
            
            if ( $linked_tab!="" and $val!=NULL ) {
                $nicename = obj_nice_name( $sql, $linked_tab, $val );
            } else {
                $nicename = $val;
            }
            $retarr['access'][$colName] = array('valnice'=>$nicename, 'colnice'=>$colNameNice);
        }
    }
    
    return $retarr;
}

/**
 * get (foreign table, primary key, important name column) of (tablename,$column)
 * @param  string $column
 * @param  string $tablename
 * @return array ()
 *    'fk_table' : string foreign key table
 *    'pk_name' : name of primary key column
 *    'imp_name' : name of most important column
 */
static function fk_info($tablename,$column) {
    global $_s_i_table;
    
    if ($fk_table = $_s_i_table[$tablename][$column]['CCT_TABLE_NAME']) {
        $ftab_prim_name = PrimNameGet2($fk_table) ? PrimNameGet2($fk_table) : $column;
        $ftab_imp_name  = importantNameGet2($fk_table) ? importantNameGet2($fk_table) : $ftab_prim_name;
        return array('fk_table'=>$fk_table,  'pk_name'=>$ftab_prim_name, 'imp_name'=>$ftab_imp_name);
    }
    return NULL;
}

/**
 * get VALUE of COLUMN $colname for this object
 * @param string $tablename
 * @param string $objid
 * @param string $colname
 * @return $value
 */
static function oneColVal($sqlo, $tablename, $objid, $colname) {
    $pkname = PrimNameGet2($tablename);
    $value  = glob_elementDataGet( $sqlo, $tablename, $pkname, $objid, $colname);
    
    return $value;
}
 
}
