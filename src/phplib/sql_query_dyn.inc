<?php
/**
 * - Contains functions for dynamic sql query creation
 * - for new code, PLEASE USE MODULE: f.sql_query.inc  !!!
 * - CONSTRAINS: 
  	  - some tables are related to ALIASES in the query:
  		x - the main table $tablename, e.g. EXP
		a - CCT_ACCESS
		o - EXTRA_OBJ
		c - EXTRA_CLASS
 * $Header: trunk/src/phplib/sql_query_dyn.inc 59 2018-11-21 09:04:09Z $
 * @package sql_query_dyn.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @var array $_SESSION['s_tabSearchCond'][TABLE] = array (     // stores the condition
         "f" => from_extra, 	// tablenames must be LOWER case !!!
         "w" => condition, 
         "x" => sqlWhereXtra, 
         "c" => classname, 
         "info"  => $sel_info, 
         "mothid"=> $mother_idM
         "y" => extra_statement after core-sql e.g. MINUS select concrete_array_id from EXP
      ); 

 */
require_once("globals.inc");

class UtilSqlSynHelp {
	
	function __construct($searchOp) {
		$this->searchOp = $searchOp;
	}
	
	/**
	 * manage a typical radio-button selection
	 * @param array $sel array[pk1][pk2][pk3] = 1
	 * @return $tableSCond
	 */
	function manageSel(&$sql, &$sel, &$primas) {
		$sqlcond = NULL;
	 	$selcnt  = 0;
		// $lener   = sizeof($sel);
		$delemiter = "";
		$col_prefix='x.';
		
		// build up an intelligent SQL query with optimal AND and OR connectors
		foreach( $sel as $pk1=>$tmp1  ) {
		  
		  $sub1_delim ='';
		  $sqlcond   .= $delemiter.'('. $col_prefix.$primas[0]. ' = '.$sql->addQuotes($pk1);
		  
		  if (is_array($tmp1)) {
			// SECOND PK
			$sqlcond= $sqlcond . " AND ((";
			foreach( $tmp1  as $pk2=>$tmp2  ) {
			    $sqlcond .= $sub1_delim . $col_prefix.$primas[1]. ' = '.$sql->addquotes($pk2);
			  $sub1_delim = ') OR (';
			  $sub2_delim = '';
			  if (is_array($tmp2)) {
				$sqlcond .= ' AND (';
				foreach( $tmp2 as $pk3=>$tmp3  ) {
				    $sqlcond .= $sub2_delim . $col_prefix.$primas[2].' = '.$sql->addQuotes($pk3);
				  $sub2_delim = ' OR ';
				}
				$sqlcond .= ')';
			  }
			}
			$sqlcond .= '))';
		  }
		  $sqlcond .= ')';
		  $delemiter = ' OR ';
		}
		
	 	$tableSCond = "(".$sqlcond.")"; // selects are OR connectors and should be in BRACES to allow following AND conditions       
	
	    return $tableSCond;
	}
	
	/**
	 * make SQL-string: COLUMN CONDTION TEXT (e.g. EXP_ID > 100)
	 * @param string $conditionCode -- the condition CODE (e.g. LIKE)
	 * @param string $searchCol    column
	 * @param string $afterColName string after the column-name
	 * @return string $searchText
	 */
	function mkColumnCond(&$sql, $conditionCode, $searchCol, $afterColName) {
		$addBrace=0;
		if (strstr($conditionCode,"LIKE")) $searchColNew = $sql->strToUpper($searchCol); //DB system dependent function
		else $searchColNew = $searchCol;
		
		if ($conditionCode=='NOT LIKE') { // extend this condition
			$afterColName .= ' or '.$searchCol.' is NULL';
			$addBrace=1;
		}
		$searchText = $searchColNew . " " .$afterColName;
		if ($addBrace) $searchText = '('.$searchText.')';
		return $searchText;
	}
	
	function _whereAdd( &$sqlWhereXtra, $input, $tbool="AND" ) {
	  $sqlWhereXtra = ($sqlWhereXtra == "") ? $input : $sqlWhereXtra." ".$tbool." ".$input;
	}
	
	/**
	 * build EXTRA_OBJ search
	 * @param $sql
	 * @param $searchCol = "class.CLASS_ID.COL_MAP" : extra_class column e.g. class.25.S4 (CLASS_ID=25, COL_MAP=S1)
	 * @param $searchBool
	 * @param $afterColName
	 * @param $whereXtra
	 * @param $sel_info
	 * @return array
	 */
	function mkExtraObj( &$sql, $searchCol, $searchBool, $afterColName, $whereXtra, $sel_info) {
		
		$searchOp = $this->searchOp;
		
		$tmp_pos1 = 6; // len of "class." 
		$tmp_pos2 = strpos( $searchCol, ".", $tmp_pos1+1  );
		$attrib_len= $tmp_pos2 - $tmp_pos1;
	    //echo "<br><br><br>DEBUG_XX1: $searchCol | class-info:$attrib_len<br>";
		if ( $attrib_len > 0) {
		  $class_id = substr( $searchCol, $tmp_pos1, $attrib_len);	  
		  $tmp_pos3 = strlen( $searchCol);
		  $col_map_len= $tmp_pos3 - $tmp_pos2;
		  
		  if ( $col_map_len > 0) {
			$col_map=substr( $searchCol, $tmp_pos2+1, $col_map_len);
		  }
	
		  $tmp_sel_add = $this->mkColumnCond($sql, $searchBool, $col_map, $afterColName);
		  
		  $tmp_where="x.EXTRA_OBJ_ID in (select EXTRA_OBJ_ID from EXTRA_OBJ where EXTRA_CLASS_ID=" .$class_id. " AND (".
	      $tmp_sel_add ."))"; // please let the "))" together; they are needed for parsing !!!
		  $tmp_pos = strpos( $whereXtra, "x.EXTRA_OBJ_ID"  );
	      if ( $tmp_pos === FALSE ) {
	        $this->_whereAdd($whereXtra, $tmp_where, $searchOp);
	      } else {
	        // bring before "))" the new condition
	        $tmp_pos_2 = strpos( $whereXtra, "))", $tmp_pos  );
	        $whereXtra = substr( $whereXtra,0,$tmp_pos_2 ) ." ".$searchOp." ".$tmp_sel_add. substr( $whereXtra,$tmp_pos_2 );
	      }     
		  $this->_whereAdd($sel_info,  $tmp_sel_add, ' '.$searchOp);
		}
		return array($whereXtra, $sel_info);
	}
	
}

function selectWhereAdd( &$sqlWhereXtra, $input, $tbool="AND" ) {
  $sqlWhereXtra = ($sqlWhereXtra == "") ? $input : $sqlWhereXtra." ".$tbool." ".$input;
}

function selectGetEasy( &$sql, $tablename, $sql_newcond) {
    // FUNCTION: - set a new query 
    //           - save selection array
    
    
    $condclean = 1;
    $opt       = array();
	$dummy     = array();
    
    list ( $sqlfromXtra, $tableSCondM, $sqlWhereXtra, $sel_info, $classname, $mother_idM ) = 
         selectGet( $sql, $tablename, $condclean, $sql_newcond, $dummy, $opt ); 
    
    $_SESSION['s_tabSearchCond'][$tablename] = 
        array ( "f"=>$sqlfromXtra, "w"=>$tableSCondM, "x"=>$sqlWhereXtra, "c"=>$classname, "info"=>$sel_info, "mothid"=>$mother_idM ); 
}


/**
 *  - create new select conditions
 *  - special joined columns like "a.DB_USER" MUST be set in $searchCol for FROM-clause calcs
 *  - calculates automatically fromClause and $whereXtra fro use of joined tables
 *  - each condition causes a text in $_SESSION['s_tabSearchCond'][TAB]["info"]
 *	  
 * @param object $sql
 * @param string $tablename table name 
 * @param int $condclean flag for previous condition clean 
 * @param string $tableSCond condition string e.g. "exp_id=300"
 * @param array $tabSearchCond : dict of saved conditions for tables
 * @param array $opt
 *  "useJoin" comma seperated list of names of joined tables e.g. "EXTRA_CLASS,CCT_ACCESS"
             use this, if there is no $searchCol which shows, that the user wants to show a special join
   "sMothId" if this is a assoc table (more than 1 PK), gives the ID of first PK
             this clears the condition and $searchCol,$searchtxt 
   "sClass"  "CLASS_NAME": if exists => add condition to class
   "xsql"   extra SQL statement after (select * from TAB)  like 'MINUS select * from EXP'
   "infoCond" optional nice human text, if $tableSCond is given, will be copied to $sel_info
 * @param string $searchtxt value to be searched e.g. "text" or "783478"; "" is ignored! 
 * @param string $searchCol
 *   columnname, reserved aliases: 
      "a.XXXX" : cct_access column
      "class.CLASS_ID.COL_MAP" : extra_class column e.g. class.25.S4 (CLASS_ID=25, COL_MAP=S1)
 * @param array $sel
 *   OPTIONAL
                [] one PK
                [][] two PKs
                [][][] three PKs 
 * @param string $searchBool
 *    the CODE-condition : can be translated to a real SQL-condition
			 "==" ">" "<" "!="  
		     "LIKE" : wildcards : "%" - some chars; "?" - single char
		     "NOT LIKE" : TRANSFORM to: analyse also NULL-values
		     "NOT LIKE strict" : do NOT analyse NULL values
 * @param array $searchOp OPTIONAL, search operator: [""], AND, OR
 * @return  $fromClause  : extra string after FROM (like cct_access a)
    	     $whereClause : string after WHERE (without joins)
             $whereXtra   : extra whereclause for joined tables like (a.cct_access_id=x.cct_access_id)
             $sel_info    : user friendly
             $class_name  : CLASS name
    	     return list( $fromClause, whereClause, $whereXtra, $sel_info; $class_name, $moth_obj_sav)
 */
function selectGet(  &$sql,	$tablename,	$condclean, $tableSCond, &$tabSearchCond, $opt, $searchtxt =NULL, 
		$searchCol =NULL, $sel = NULL, $searchBool=NULL, $searchOp  =NULL ) {
    
            
            
  if ( $searchOp   == '' )  $searchOp = 'AND'; // default: AND
  
  $helpLib = new UtilSqlSynHelp($searchOp);
  
  $usejoin      = $opt["useJoin"];
  $moth_obj_id  = $opt["sMothId"]; 
  $use_class    = $opt["sClass"]; 
  
  // if flags for "sMothId" or $sel:
  // primary keys 0 based index [0=>"first_name", 1=>"second_name",..]
  $primas		= primary_keys_get2($tablename);  
  												 
  if (!is_array($tabSearchCond) ) $tabSearchCond=array();  //fall back
  
  if ( $condclean ) $tabSearchCond[$tablename] = array(); // clean conditions
  
  if ($moth_obj_id !== NULL) {
    $tabSearchCond[$tablename]=array();
	$searchCol="x.".$primas[0];
	$searchtxt=$moth_obj_id;
	$searchBool="";
	$tabSearchCond[$tablename]["mothid"] = $moth_obj_id;
	$moth_obj_sav=$moth_obj_id;
  }
  
  
  
  if ( $searchBool == '' ) {
	$searchBool = '=';
  }
  
  $afterColName = NULL;   // string after COLUMN
  $useBool = $searchBool; // use this string for the condtion build
  if (strstr ($searchBool, 'NULL') ) $afterColName = $searchBool; /* special SQL query, like "is NULL" */
  else {
  	$afterSeaTxt = ""; // can be ESCAPE
 	if (strstr($searchBool,'LIKE')) {
		// escape wildcards
		$CHAR_DOLLAR = chr(36);
		$WILDCHARD   = "?";
		$searchtxt = str_replace( $CHAR_DOLLAR.$WILDCHARD, "###", $searchtxt ); // save $WILDCHARD, if  "$?"
		$searchtxt = str_replace( "_", $CHAR_DOLLAR . "_", $searchtxt );
		$searchtxt = str_replace( $WILDCHARD, "_", $searchtxt );
		$searchtxt = str_replace( "###", $WILDCHARD, $searchtxt ); 		// get back $WILDCHARD-char
		$searchtxt = strtoupper ( $searchtxt ); /* activate also sql-function UPPER() later*/
		if ( strstr($searchtxt,$CHAR_DOLLAR)!=NULL ) $afterSeaTxt .= "  ESCAPE '$'";
	}
	
	// transform a simple 'NOT LIKE' to (not like ... or COL is NULL)
    if ($searchBool=='NOT LIKE strict') {
		$useBool = 'NOT LIKE'; // the real SQL 'not like' 
	}
	
 	if ($searchtxt!="") {
 	  // $searchBool was translated to $useBool
	  $afterColName = $useBool. " '". $searchtxt. "'" . $afterSeaTxt;
 	} else {
	  $searchCol=""; // when searchtxt=="" => ignore condition
 	}
  }
  
  if ( !empty($sel) ) { // check selection array
  	$tableSCond = $helpLib->manageSel($sql, $sel, $primas);
	$tabSearchCond[$tablename] = array(); // clean previous selections, because the slect boxes ignore other searches
  }
  
  /* conditions  */
  
  if ( !empty($tabSearchCond[$tablename]) ) {
 	//reset ($tabSearchCond[$tablename]) ;
 	$fromClause 	= $tabSearchCond[$tablename]["f"];
 	$tableSCondOld  = $tabSearchCond[$tablename]["w"];
 	$whereXtra 	    = $tabSearchCond[$tablename]["x"]; 
	$class_name 	= $tabSearchCond[$tablename]["c"] ;
	$sel_info 	    = $tabSearchCond[$tablename]["info"];
	$moth_obj_sav	= $tabSearchCond[$tablename]["mothid"];
    $xsql           = $tabSearchCond[$tablename]["y"];
    
  } else {
 	$fromClause 	= NULL;
 	$tableSCondOld  = NULL;
 	$whereXtra 	    = NULL;
	$class_name 	= NULL;
	$sel_info 	    = NULL;
	$moth_obj_sav	= NULL;
    $xsql           = NULL;
  }
   
  
  if ( strpos( $searchCol,"a." ) === TRUE ) {
	
	if ( strstr( $usejoin, "CCT_ACCESS") =="" ) $usejoin= $usejoin."CCT_ACCESS,";
	$tmp_table="xx.cct_access_id";
	if ( strstr( $searchCol,"aa." ) =="" ) $searchCol = "a". $searchCol;
	$tmp_access_pos = strpos( $whereXtra, $tmp_table  );
	$tmp_where_add_pre = " " . $helpLib->mkColumnCond($sql, $searchBool, $searchCol, $afterColName);
	
	$tmp_where_add = $tmp_where_add_pre ." )";
	selectWhereAdd($sel_info, $tmp_where_add_pre, " ".$searchOp);
	
	if ( $tmp_access_pos === FALSE ) {
	  $tmp_where= "x.".$tablename."_ID in (select xx.".$tablename."_ID from $tablename xx, cct_access aa where aa.cct_access_id = xx.cct_access_id AND (".$tmp_where_add."  )";
	  selectWhereAdd($whereXtra, $tmp_where );
	} else {
	  $tmp_access_pos = strpos( $whereXtra, ")", $tmp_access_pos  ); # search end of CCT_ACCESS condition 
	  $whereXtra = substr_replace( $whereXtra, $searchOp.$tmp_where_add , $tmp_access_pos,  1 );
	}
	$searchCol = ''; // clear $searchCol
  }
  if ( strstr( $searchCol,"class." ) !="" ) { 
    list($whereXtra, $sel_info) = $helpLib->mkExtraObj( $sql, $searchCol, $searchBool, $afterColName, $whereXtra, $sel_info);
 	$searchCol=""; /* clear $searchCol */
  }
  
  if ($searchCol!=NULL and $afterColName!=NULL){
  	if ( $tableSCond!=NULL ) {
	  if ( $searchOp == "OR" or $searchOp == "AND") $tableSCond="(".$tableSCond.")";
	  $tableSCond = $tableSCond . " ".$searchOp." ";
	}
	
	$searchText = $helpLib->mkColumnCond($sql, $searchBool, $searchCol, $afterColName);
	$tableSCond = $tableSCond . $searchText ;
	
  }
  
 if ( strstr($usejoin, "EXTRA_CLASS")!="" || ($use_class!="") ) {
   if ( strstr($fromClause, "extra_obj") =="" ) {
	 $fromClause .= ', extra_obj o, extra_class c';
	 selectWhereAdd($whereXtra, ' x.extra_obj_id = o.extra_obj_id AND c.extra_class_id = o.extra_class_id');
   }
   if ( strstr( $tableSCondOld, "c.NAME")=="" && ($use_class!="")) {
	 selectWhereAdd($tableSCond, " c.NAME='$use_class'" );
	 $class_name = $use_class;
   }
 }
 if ( strstr($usejoin, "CCT_ACCESS")!="" ) {
   if ( strstr($fromClause, "cct_access")=="" ) {
	 $fromClause = $fromClause . ", cct_access a";
	 selectWhereAdd($whereXtra, " a.cct_access_id=x.cct_access_id" );
   }
 }
 $tableSCondM=$tableSCondOld;
 
 if ( $tableSCond!=NULL ) {
   $ret= strstr( $tableSCondOld, $tableSCond );
 	if ( !$ret ) { /* condition already in string */
	  if ( $tableSCondOld!=NULL ) {
		if ( $searchOp == "OR" or $searchOp == "AND") $tableSCondOld="(".$tableSCondOld.")";
		$tableSCondOld = $tableSCondOld . " ".$searchOp." ";
		$sel_info = $sel_info ." ". $searchOp ." ";
	  }
	  $tableSCondM = $tableSCondOld . $tableSCond;
	  if ( $opt["infoCond"]==NULL ) {
	  	$sel_info = $sel_info . " " .$tableSCond;
	  } else {
	  	$sel_info = $sel_info. " " .$opt["infoCond"]; // add other info
	  }
	}
 } 
 
 if ($opt["xsql"]!="") {
    $xsql    .= " ".$opt["xsql"]; // no special APPEND-string 
    $sel_info = $sel_info . " " .$xsql;
 }
 
 
 
 return array ( $fromClause, $tableSCondM, $whereXtra, $sel_info, $class_name, $moth_obj_sav, $xsql );
}

function full_query_get( $tablename, $sqlfromXtra, 
    $tableSCondM, 
    $sqlWhereXtra, 
    $xsql=NULL     // "y"
    ) {
  /* FUNCTION: get full query string after FROM */
  $sqls = $tablename. " x" . $sqlfromXtra;
  if ( $tableSCondM || $sqlWhereXtra ) { /* extra search condition */
	$sqls = $sqls . " WHERE ";
	if ( $tableSCondM != "" )  $sqls = $sqls . " (" . $tableSCondM. ")";
	if ( $tableSCondM && $sqlWhereXtra )  $sqls = $sqls . " AND ";
	$sqls = $sqls . $sqlWhereXtra;
  }
  if ($xsql) $sqls .= " ".$xsql; // extended statement: no special APPEND string
  
  return $sqls;
}

 
/**
 * returns the from- & where-clause of the selection query-string wich is set for the table $tablename
 * @param string $tablename
 * @param array $opt "order" [0] | 1   add order string
 * @return string SQL-condition
 */
function get_selection_as_sql( 
    $tablename, 
    $opt=NULL  
    ) {
  
  
  $fromClause  = isset($_SESSION['s_tabSearchCond'][$tablename]["f"]) ? $_SESSION['s_tabSearchCond'][$tablename]["f"] : "";
  $tableSCondM = isset($_SESSION['s_tabSearchCond'][$tablename]["w"]) ? $_SESSION['s_tabSearchCond'][$tablename]["w"] : "";
  $whereXtra   = isset($_SESSION['s_tabSearchCond'][$tablename]["x"]) ? $_SESSION['s_tabSearchCond'][$tablename]["x"] : "";
  $xSql        = isset($_SESSION['s_tabSearchCond'][$tablename]["y"]) ? $_SESSION['s_tabSearchCond'][$tablename]["y"] : "";
  $query_str = full_query_get($tablename, $fromClause, $tableSCondM, $whereXtra, $xSql);
  
  if ( $opt["order"] ) { // TBD: before the extended SQL $xSql ?
    $sortcrit = query_sort_org( $tablename );
    $tmpSort  = query_sort_get( $tablename, $sortcrit );  // with order_by
    $query_str .=  $tmpSort;
  }
  
  return $query_str;
} 

function query_set_sort($tablename, $sortSQL) {
	
	$sortcrit_name= "o.".$tablename.".sortcrit";
	$_SESSION['userGlob'][$sortcrit_name] = $sortSQL;
}

function query_sort_org( $tablename ) {
	// FUNCTION: - get sort criteria
	//			 - check, if sort_criteria is possible => check aliases : o. a. c. pc.
	//			 - relevant for view.tmpl.php
	
	$sortcrit_name= "o.".$tablename.".sortcrit";
    $sortcrit = $_SESSION['userGlob'][$sortcrit_name];
	
	// check for relevance
	if (strstr($sortcrit, ".")!=NULL) {	// contains ALIAS-columns ???
		$fromClause  = $_SESSION['s_tabSearchCond'][$tablename]["f"];
		if ( strstr($sortcrit, "o.")!=NULL AND strstr($fromClause, "extra_obj")==NULL)  $sortcrit = ""; // delete criteria !!!
		if ( strstr($sortcrit, "a.")!=NULL AND strstr($fromClause, "cct_access")==NULL)  $sortcrit = ""; // delete criteria !!!
		if ( substr($sortcrit,0,2)== "c." AND strstr($fromClause, "extra_class")==NULL)  $sortcrit = ""; // delete criteria !!!
		if ( strstr($sortcrit, "pc.")!=NULL AND strstr($fromClause, "PROJ_HAS_ELEM")==NULL)  $sortcrit = ""; // delete criteria !!!
	}
    return ($sortcrit);
}

function query_get_info($tablename) {
	// FUNCTION: get current info string
	
	$select_info = $_SESSION['s_tabSearchCond'][$tablename]["info"];
	return ($select_info);
}

function query_sort_get( $tablename, $sortcrit ) {  // "e.g. "EXP_ID ASC"            
// FUNCTION: hwat is, if no sort criteria was given ???
    $retstr = "";
    
    if ($sortcrit!="") {
       $retstr = " order by ". $sortcrit; 
    } 
    return ($retstr);
}

function query_add_join( &$sql, $tablename, $joinstr ) {
	// FUNCTION: add a join
	
	
	$condclean = 0;
	$tableSCond = "";
	$tabSearchCond = &$_SESSION['s_tabSearchCond'];
	
	$optx = array(	"useJoin" => $joinstr);
	list ( $sqlfromXtra, $tableSCondM, $sqlWhereXtra, $sel_info, $classname, $mother_idM ) = 
	     selectGet( $sql, $tablename, $condclean, $tableSCond, $tabSearchCond, $optx );	
    
    $_SESSION['s_tabSearchCond'][$tablename] = 
        array ( "f"=>$sqlfromXtra, "w"=>$tableSCondM, "x"=>$sqlWhereXtra, "c"=>$classname, "info"=>$sel_info, "mothid"=>$mother_idM ); 
   
}