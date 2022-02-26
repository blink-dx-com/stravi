<?php

/**
 * methods to support search for objects by their NAME in database, object redundancy check
 * - usually used, when object names are parsed in files
 * - used to search for objects with same single features
 * @package class.filex.inc
 * @swreq GLOBAL
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @unittest exists
 */
class fileC {  

    
    /**
     * transform an object name to a valid file-name
     * @param string $objectname
     */
    static function objname2filename( $objectname ) {
    	$badchars       = array( "\\","/", "*", "?", "\"", ":", ">", "<", "|", "#" );
    	$objectnameNew  = trim($objectname);                // trim
    	$objectnameNew  = substr($objectnameNew,0,200);     // max 200 chars
    	$filename = str_replace( $badchars, "_", $objectnameNew); // replace bad chars
    	
    	return ($filename);
    }
    
    /**
     * do a redundancy Test
     * @param  $opt=NULL 
     *		"like" : 0|1
     *		"wild" : 0|1 add wildcards
     *		"caseinsense" => 1 : case insensitive search
     *		"isEXOB" : the $searchCol is an EXTRA_OBJ attribute
     *		'classid': see "isEXOB"
     * @return number of found objects
     */
    static function redundancyTest( &$sql, $tablename, $searchCol, $objName, $opt=NULL ) {
        global $varcol;
    
        $objNameSql = str_replace ("'", "''", $objName);
        $operator = "=";
        if ( $opt["like"] ) { 
            $operator = "LIKE";
        }      
        
        $searchCol2 = $searchCol;
    	if ( $opt["caseinsense"]>0) {
    		$searchCol2="UPPER(".$searchCol.")";
    		$operator = "LIKE";
    		$objNameSql = strtoupper($objNameSql);
    	} 
    	
    	if ( $opt["wild"] ) {
    		 $objNameSql = "%".$objNameSql."%";
    	}
    
    	$jointab = NULL;
    	$addWhere= NULL;
    	if ( $opt['isEXOB'] ) {
    		$answer  = $varcol->sql_get_fromWhereAdd( $opt['classid'] );
    		$jointab = $answer['j'];
    		$addWhere= ' AND ' .$answer['w'];
    	}
    
        $sqls   = "select count(*) from ".$tablename .' x' . $jointab. " where " .$searchCol2 
    			  . " ".$operator." '". $objNameSql ."'".$addWhere;
        $sql->query($sqls);
        $sql->ReadRow();
        $tmpcnt = $sql->RowData[0];
        return ($tmpcnt);
    } 
    
    /**
     * simple redundancy test: search if object with parameters exists, except this object with $objid
     * @param object $sql
     * @param string $tablename
     * @param array $search_array
     * @param int $objid [OPTIONAL] -- ignore this  object-ID
     
     * @return int of found other objects
     */
    static function redundancyTest2( &$sqlo, $tablename, $search_array, $objid=NULL ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;

    	$pkname = PrimNameGet2($tablename);
    	
    	if ($pkname==NULL) sys_error_my( $FUNCNAME.': no PK for table defined.' );
    	// if (!$objid) sys_error_my( $FUNCNAME.': no input param objid given.' );
    	
    	$where_arr=array();
    	foreach ($search_array as $key=>$val ) {
    	    if ( $val==NULL ) {
    	        $error->set( $FUNCNAME, 1, 'No value given for '.$key );
    	        return;
    	    }
    		$where_arr[] = $key.' = '.$sqlo->addQuotes($val);
    	}
    	$where_str=implode(' and ',$where_arr);
    	
    	$obj_except_str=NULL;
    	if ($objid!=NULL) {
    		$obj_except_str= ' and ' .$pkname. ' != '.$objid;
    	}
    	
    	
    	$sqlsel  = 'count(1) from '.$tablename.' where '. $where_str . $obj_except_str;
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadRow();
    	$cnt = $sqlo->RowData[0];
    	
    	return $cnt;
    }
    
    /**
     * find an object by name (or other parameters)
     * @param $sql
     * @param $tablename
     * @param $searchCol
     * @param $objName
     * @param array $opt : 
     * 		"like" = 1 add wildcards: %$objName%
    			   = 2 only like ...
    			   = 3 REGULAR expression in $objName; e.g. $objName = '^[0]*1788$'; search string with leading zeros
    		"caseinsense" => 1 : case insensitive search
    		'isEXOB' = 0,1 : use a class-search ?
    		'classid'=> ID of class (need 'isEXOB')
     * @return array( $cnt, $objid, $objname )
    	 		number of hits: 
                0  no hit
                1  exactly one hit
                n  more
     */
    static function objGetByName( &$sql, $tablename, $searchCol, $objName, $opt=NULL ) {
        global $varcol;
        
        $tmpcnt = 0;
        $objid  = 0;
        $impColNameSql = "";
        $objname =  "";
    	$nameCol = importantNameGet2($tablename);
    	$pkname  = PrimNameGet2($tablename);
         
        // $objNameSql = str_replace ("'", "''", $objName);
        $objNameSql = $objName;
        $operator = "=";
        if ($opt["like"] ) { 
            $operator = "LIKE";
            if ($opt["like"]==1) $objNameSql = "%".$objNameSql."%";
    		else $objNameSql = $objNameSql;
    		
        }
    	
    	$searchCol2 = $searchCol;
    	if ( $opt["caseinsense"]>0) {
    		$searchCol2="UPPER(".$searchCol.")";
    		$operator = "LIKE";
    		$objNameSql = strtoupper($objNameSql);
    	} 
    
    	$jointab = NULL;
    	$addWhere= NULL;
    	if ( $opt['isEXOB'] ) {
    		$answer  = $varcol->sql_get_fromWhereAdd( $opt['classid'] );
    		$jointab = $answer['j'];
    		$addWhere= ' AND ' .$answer['w'];
    	}
                                             
        $sqlAfter = " from ".$tablename. ' x'. $jointab . " where " .
    		$searchCol2 . " ".$operator ." ". $sql->addQuotes($objNameSql). $addWhere ;
        
    	if ($opt["like"]==3) {
    	    // REGULAR EXPRESSION
    	    $sqlAfter_base = " from ".$tablename.  ' x'. $jointab . " where ";
    	    $sqlAfter =  $sqlAfter_base . " REGEXP_LIKE (".$searchCol.", '".$objName."')";
    	    if ($_SESSION['sec']['_dbtype']=='CDB_PG') {
    	        $sqlAfter =  $sqlAfter_base . $searchCol . ' SIMILAR TO ' . $sql->addQuotes($objName) ;
    	    }
    	    $sqlAfter = $sqlAfter .  $addWhere;
    	}
    			
        $impColNameSql = ", ".$nameCol;
        
        $sqls   = "select ". $pkname . $impColNameSql . $sqlAfter;
        $sql->query($sqls);
        if ( $sql->ReadRow() ) {
        	// at least once found ...
            $tmpcnt  = 1;
            $objid   = $sql->RowData[0];
            $objname = $sql->RowData[1];
        
    	    if ( $sql->ReadRow() )  {  // more than 1 found ?
    	        $sqls   = "select count(*) ".$sqlAfter;
    	        $sql->query($sqls);
    	        $sql->ReadRow();
    	        $tmpcnt = $sql->RowData[0];
    	    }
        }
        return array($tmpcnt, $objid, $objname);
    }

}
