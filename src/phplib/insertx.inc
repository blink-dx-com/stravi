<?php
/**
 * meta functions to insert objects (mainly BO)
 * @namespace core::db
 * @package insertx.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
                      
require_once ('insert.inc');
require_once ('object.info.inc');
require_once ('o.S_VARIO.mod.inc');
require_once ('f.update.inc');


/**
 * insert object with SINGLE PK (mainly BO)
 */
class insertC {

    /**
     * create a new object
     * @param array $args TYPEOF objFeatStruct <pre>
     *   array( "vals"=>..., "xobj"=>... "vario"=>... )
     *   "vals" => main object parameters
     *     array [COL] = val
     *     should NOT contain CCT_ACCESS_ID
     *     should NOT contain a PK, if it is a single PK-object
     *     EXTRA_OBJ_ID will be removed
    	 "xobj" => array ( 
    		'extra_class_id' => ... , 
    		"values"  => array(attrib_id=>val) 
    		"optname" => [0],1 : values_by_name ??? 
    		)
    	 "vario"=> array( key => val ) VARIO-Parameters
    	 "acc"  => array( key=>val )   CCT_ACCESS parameters
    	    Info: we did not use the key "access" to prevent accedentially copying old parameters from 
    	    objInfoC::getParamsRaw()
    	 
     * </pre>
     * @param array  $insopt (TYPEOF insert.inc:ins_opt_STRUCT )
     * @return int ID of new object
     */
    static function new_meta(&$sql, string $tablename, $args,	$insopt = NULL) {
    	global $error, $varcol;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $error  = & ErrorHandler::get();
        $varcol = & Varcols::get();
        $pkname = PrimNameGet2($tablename);
        
        if (is_array($args["vals"])) unset ($args["vals"]["EXTRA_OBJ_ID"]);
        
        $tmp_pk = insert_row($sql, $tablename, $args["vals"], $insopt);
        if (!$tmp_pk) {
            $error->set($FUNCNAME,1, "Creation insert_row() failed");
            return;
        }
    
        if (is_array($args["xobj"])) {
            if ( $args["xobj"]['extra_class_id'] != '' ) {
            	$varcol = & Varcols::get();
            	$dest_idname = $tablename."_ID"; // TBD: use function
        		$id_values = NULL;
        		if ( is_array($args["xobj"]['values']) ) {
        			$id_values = &$args["xobj"]['values'];
        			if ( $args["xobj"]["optname"] ) { // index of value_array are NAMES
        				$valstemp = $args["xobj"]['values'];
        				$id_values = NULL;
        				foreach( $valstemp as $attrib_name=>$val) {
        					$attrib_id = $varcol->attrib_name_to_id ($attrib_name, $args["xobj"]['extra_class_id']);
        					if ( $error->got(CCT_ERROR_READONLY) ) return;
        					$id_values[$attrib_id] = $val;
        				}
        				reset($valstemp);
        			}
        		}
        		$EXTRA_OBJ_ID = $varcol->insert ($args["xobj"]['extra_class_id'], $id_values ); 
        		if ( $error->got(READONLY) or !$EXTRA_OBJ_ID) return 0;
        		
                $sqls = "extra_obj_id = ".$EXTRA_OBJ_ID." WHERE ".$dest_idname." = ".$tmp_pk;
                $sql->Updatex($tablename, $sqls);
                if ($error->got(READONLY)) $error->set($FUNCNAME, 2, "creation of extra_class parameters for object [$tmp_pk] failed");
            }
        }
        
    	if ( is_array($args['vario']) ) {
    			
    		$varioLib = new oS_VARIO_mod();
    		$varioOpt = array('noAccCheck'=>1);
    		$varioLib->setObject( $sql, $tablename, $tmp_pk, $varioOpt );
    		
    		reset($args['vario']);
    		foreach( $args['vario'] as $key=>$val) {
    			$varioLib->updateKeyVal($sql, $key, $val);
    		}
    		reset ($args['vario']);
    		 
    		if ($error->Got(READONLY))  {
    			$error->set( $FUNCNAME, 2, 'new object [ID:'.$tmp_pk.']: error on vario-update.' );
    			return $tmp_pk;
    		}
    			
    	}
    	
    	if ( is_array($args['acc']) ) {
    		// CCT_ACCESS params
    		$accid = glob_elementDataGet( $sql, $tablename, $pkname, $tmp_pk, 'CCT_ACCESS_ID');
    		if ($accid) {
    			gObjUpdate::update_row_s( $sql, 'CCT_ACCESS', $args['acc'], array('CCT_ACCESS_ID'=>$accid) );
    		}
    	}
        
        return ( $tmp_pk );
    }
    
    }
    
    
/**
  * this class does NOT do any check for table policies (HARDCORE) 
  * FUTURE: put also to class insertC ?
  */
class insertRawC {
    	
        static function insertR( &$sqlo, $tablename, &$argu, $prim_name) {

        	$sqls      = ' VALUES (';
        	$sql_cols  = '(';
        	$i         = 0;

        	foreach( $argu as $xcol=>$xval) {
        	
        		if ($i) {  // beim ersten val KEIN komma
        			$sqls     .=', ';
        			$sql_cols .=', ';
        		}
        			 
        		$sql_cols .= $xcol;
        		if ( $xval === "" or $xval ===NULL ) $tmpvalSql = 'NULL';
        		else {
        			$tmpvalSql = $sqlo->addQuotes($xval);
        		}
        		
        		$sqls .= $tmpvalSql;
        		$i++;
        	}
        	
        	
        	$sqls  = $sql_cols.') '.$sqls.')'; // assemble whole query
        	$objid = $sqlo->queryInsert($tablename, $sqls, $prim_name); // do insert
        	
        	return ($objid);
        }
	
}

// insert-update support
class insertUpdateC {
    
    // support SINGLE primary key table
    static function ins_upd_simple($sqlo, $table, $argu, $pkname) {
        
        $id = $argu[$pkname];
        $exists = gElement_exists2($sqlo, $table, array($pkname=>$id));
        if (!$exists) {
            $xopt=array('prim_name'=>$pkname);
            $new_id = insert_row_s($sqlo, $table, $argu, $xopt);
        } else {
            $new_id = $id;
            unset($argu[$pkname]);
            gObjUpdate::update_row_s($sqlo, $table, $argu, array($pkname=>$id));
        }
        return $new_id;
    }
}

/**
 * class to copy assoc-elements from SRC to DEST
 */
class fCopyAssoc {
	
    function __construct() {
    	
    	$this->insertLib  = new insertC();
    }
    
    
    /**
     * copy associated table elements
     * @param object $sql
     * @param object $sql2
     * @param string $tablename
     * @param int $src_id
     * @param int $dest_id
     * @param array $assoc_arr array ["TABLE_NAME"] = 1
     * @param array $realAssObjPar  OPTIONAL  - copy also assocciated objects, optional: DEEP copy
    		 array ( [TABLE]  => array ( 
    		   [COL_NAME] => array(
    		       "tab"  => TABLE, 
    		       "deep" => 0 : not deep copy / 1 : deep copy 
    		       )		      
    		 )
    		 
    		 e.g. = array ([EXP_HAS_PROTO] => array("key"=>"CONCRETE_PROTO_ID", "tab"=>TABLE, "deep"=>1) )
    
     * @param array $option  
     *    ["sqlhandle"] : need for deep $realAssObjPar
     * @return void|NULL|number
     */
    function new_assoc(&$sql, &$sql2, $tablename, $src_id, $dest_id, $assoc_arr,  $realAssObjPar = NULL,  $option = NULL ) {
    	global $error;
    	
    	$retarr=NULL;
    	$xopt  = NULL;
    	$xopt["xObjParById"] = 1;
    	$sql3  = &$option["sqlhandle"];
    	
    	$assoc_cnt = sizeof ($assoc_arr);
    	if ( !$assoc_cnt ) return;
    
    	foreach( $assoc_arr as $assoc_name=>$dummy) {
    
    		// $assoc_nicename = tablename_nice2($assoc_name);
    		// echo "<LI><I>$assoc_nicename</I>: ";
    		$idname      = PrimNameGet2($assoc_name);
    		$thisRealObj = $realAssObjPar[$assoc_name]; // do a real obj copy ???
    		
    		if (!empty($thisRealObj)) {
        		$thisKeyName = key($thisRealObj);  // e.g. "CONCRETE_PROTO_ID"=> ("tab"=>..., "deep"=>...)
        		$tmparr1     = current($thisRealObj);
        		$thisRemTab  = $tmparr1["tab"];
        		$thisDeep    = $tmparr1["deep"];
    		}
    		
    		$sqls= "select * from ".$assoc_name." where ".$idname."=".$src_id;
    		$sql->query("$sqls");
    		$cnt_elem		= 0;
    		//$save_cnt_elem	= 0;
    		
    		while ( $sql->ReadArray() ) {
    
    			$args		   = $sql->RowData;
    			$args[$idname] = $dest_id;      /* set dest ID */
    			
    			if (is_array($thisRealObj)) {
    			    
    				// do a real object copy !!!
    				$thisNewId   = NULL;
    				$real_src_id = $args[$thisKeyName];
    				$idname_deepObj  = PrimNameGet2($thisRemTab);
    				
    				do {
    					if (!$real_src_id) break;
    					if ( $_SESSION['userGlob']["g.debugLevel"]>2 ) {
       						 echo "DEBUG: new_assoc() do object_copy of $thisRemTab:$real_src_id <br>\n";
    					}
    					
    					// get deep object e.g. for EXP: EXP_HAS_PROTO
    					
    					$thisSrcParam = objInfoC::getParamsRaw($sql2, $thisRemTab, $real_src_id, $xopt); 
    					unset( $thisSrcParam['vals'][$idname_deepObj] ); // unset PRIMARY KEY !
    					
    					$thisNewId    = $this->insertLib->new_meta( $sql2, $thisRemTab, $thisSrcParam );
    					if (!$thisNewId) {
    						$error->set("insertC::new_assoc()", 2, "object ".$thisRemTab." not created!");
    						return;
    					}
    				
    					
    					if ($thisDeep) {
    						if ($thisRemTab=="CONCRETE_PROTO") {
    							$tmpDeepTab=array("CONCRETE_PROTO_STEP"=>1);
    							$retarr = $this->new_assoc($sql2, $sql3, $thisRemTab, $real_src_id, $thisNewId, 
    										$tmpDeepTab );
    						} 
    					}
    				} while (0);
    				
    				if ( $thisNewId ) $args[$thisKeyName] = $thisNewId; // update link to NEW object
    			}
    			
    			$retVal = insert_row_s( $sql2, $assoc_name, $args );
    			if ($retVal<=0) {
    				$error->set( "insertC::new_assoc()", 1, "element ".$cnt_elem." for '$assoc_name' not created.");
    				return;
    			}
    			
    			
    			$cnt_elem++;
    
    			
    		}
    		$retarr[$assoc_name]=$cnt_elem;
    	}
    
    	return ($retarr);
    
    }
    
    
    /**
     * copy of foreign-assoc-table elements, 
      	where PRIMARY_KEY of  $tablename is NOT PRIMARY_KEY of $ASSOC_TABLE
      EXAMPLE: $tablename="DB_USER"; $assoc_arr= array ( "USER_ROLE", "DB_USER_GROUP" )
     * @param object $sql
     * @param object $sql2
     * @param string $tablename
     * @param int $src_id  PRIMARY_KEY of $tablename
     * @param int $dest_id PRIMARY_KEY of $tablename
     * @param array $assoc_arr array[$ASSOC_TABLE] = 1
     * @return void|NULL|number
     */
    function new_assocFrgn(&$sql, &$sql2, $tablename, $src_id, $dest_id, $assoc_arr ) {
    	global $error;
    	
    	$retarr=NULL;
    // 	if ( $_SESSION['userGlob']["g.debugLevel"]>2 AND $realAssObjPar) {
    //    		echo "DEBUG: new_assocFrgn() object_copy params: \n"; print_r($realAssObjPar)."<br>";
    // 	}
    	
    	$assoc_cnt = sizeof ($assoc_arr);
    	if ( !$assoc_cnt ) return;
    	
    	$idname  = PrimNameGet2($tablename);
    
    	foreach( $assoc_arr as $assoc_name=>$dummy) {
    
    		$sqls= "select * from ".$assoc_name." where ".$idname."=".$src_id;
    		$sql->query("$sqls");
    		$cnt_elem		= 0;
    		//$save_cnt_elem	= 0;
    		
    		while ( $sql->ReadArray() ) {
    
    			$args		   = $sql->RowData;
    			$args[$idname] = $dest_id;      /* set dest ID */
    
    			$retVal = insert_row_s( $sql2, $assoc_name, $args );
    			if ($retVal<=0) {
    				$error->set( "insertC::new_assocFrgn()", 1, "element ".$cnt_elem." for '$assoc_name' not created.");
    				return;
    			}
    			$cnt_elem++;
    
    		}
    		$retarr[$assoc_name]=$cnt_elem;
    	}
    
    	return ($retarr);
    }
 
}
