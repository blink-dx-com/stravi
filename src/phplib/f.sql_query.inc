<?php
/**
 * Contains functions for dynamic sql query creation
 * - do not use sql_query_dyn.inc anymore !
 * @package f.sql_query.inc
 * @namespace core::db
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @var $_SESSION['s_tabSearchCond'] (see module: "sql_query_dyn.inc" ) <pre>
	$_SESSION['s_tabSearchCond'][TABLE] = array ( 
		"f" => from_extra, 	
		"w" => condition, 
		"x" => sqlWhereXtra, 
		"c" => classname, 
		"info"  => $sel_info, 
		"mothid"=> $mother_idM
		"y" => extra_statement after core-sql e.g. MINUS select concrete_array_id from EXP,
		"arch" => 0,1 : use data from ARCHDB (see SREQ:4856)
	);
	</pre>
 * @example <pre>
 	$utilLib = new fSqlQueryC('CONCRETE_SUBST');
 	$utilLib->cleanCond();
	$utilLib->addCond( $sqlselWhere, '', 'aktuelles Labor-Blut' ); 
	$sqlAfter = $utilLib->get_sql_after( );
	</pre>
 * @version $Header: trunk/src/phplib/f.sql_query.inc 59 2018-11-21 09:04:09Z $
 */
require_once ("sql_query_dyn.inc");

class fSqlQueryC {
	
    function __construct($tablename=NULL) {
    	if ($tablename!=NULL) $this->setDefs( $tablename ); 
    }
    	
    /**
     * load session table condition array
     * @param string $tablename
     */
    function setDefs( string $tablename ) {
    	$this->tablename  = $tablename;
    	$this->tabSeaCond = $_SESSION["s_tabSearchCond"][$tablename]; // make a copy
    }
    
    // set new search condition (after WHERE)
    function settabSeaCond( &$tabSearchCond ) {
    	$this->tabSeaCond = $tabSearchCond; // make a copy
    }
    
    /**
     * clean condition
     */
    function cleanCond() {
    	$this->tabSeaCond = NULL;
    }
    
    function _whereAddPure($old, $add, $tbool="AND") {
    	$new = ($old == "") ? $add : $old." ".$tbool." ".$add;
    	return ($new);
    }
    
    /**
     * add WHERE-condition to "w" and "info"
     * @param $addwhere
     * @param $tbool
     * @param string $infostr human readable condition
     * @return $newwhere
     */
    function _WhereAdd( $addwhere, $tbool="AND", $infostr=NULL ) {
        if ($tbool=='') $tbool='AND';
    	$oldwhere = &$this->tabSeaCond["w"];
    	$this->tabSeaCond["w"] = ($oldwhere == "") ? $addwhere : $oldwhere." ".$tbool." ".$addwhere;
    	
    	$oldwhere = &$this->tabSeaCond["info"];
    	$addinfo  = ($infostr == "") ? $addwhere : $infostr ;
    	$this->tabSeaCond["info"] = ($oldwhere == "") ? $addinfo : $oldwhere." ".$tbool." ".$addinfo;
    }
    
    /**
     * make SQL-string: COLUMN CONDTION TEXT (e.g. EXP_ID > 100)
     
     * @param string $searchCol    column
     * @param string $condition -- the condition CODE (e.g. LIKE)
     *     '' ==> '='
     *     'extra' => accept PURE value like : 'EXEC_DATE between(....)'
     *     'IN'  : $value: '234,2344' or 'select X from Y'
     * @param string $value string after the column-name
     * @return string $searchText
     */
    private function _column_cond(&$sqlo, $searchCol, $condition,  $value) {
        $addBrace=0;
        
        $value_use  = NULL;
        if ($value!==NULL) $value_use = $sqlo->addQuotes($value);
        $searchColNew  = $searchCol;
        $condition_use = $condition;
        
        do {
            // condition check 

            if ($condition=='') {
                $condition_use='=';
                break;
            }
     
            if ($condition=='extra') {
                // DB system dependent function
                $searchColNew  = NULL; 
                $condition_use = NULL;
                $value_use     = $value; // NO addquotes !
                break;
            }
            if ($condition=='IN') {
                $condition_use = ' '.$condition.' ';
                $value_use     = '('.$value.')'; // NO addquotes !
                break;
            }
            
           
            if (strstr($condition,"LIKE")) {
                $searchColNew = $sqlo->strToUpper($searchCol); //DB system dependent function
                $condition_use = ' '.$condition.' ';
            } 
    
            if ($condition=='NOT LIKE') { // extend this condition
                $value_use .= ' or '.$searchCol.' is NULL';
                $addBrace=1;
            }
        } while(0);
        
        $searchText = $searchColNew . $condition_use . $value_use;
        if ($addBrace) $searchText = '('.$searchText.')';
        return $searchText;
    }
    
    /**
     * ad a column condition
     * @param object $sqlo
     * @param string $searchCol
     * @param string $conditionCode
     * @param string $value
     * @param string $tbool
     */
    function add_column_cond(&$sqlo, $searchCol, $conditionCode,  $value, $tbool='') {
        $addwhere = $this->_column_cond($sqlo, $searchCol, $conditionCode,  $value);
        $this->_WhereAdd($addwhere, $tbool);
    }
    
    /**
     * - add a joined table
     * - must be used for joining CCT_ACCESS, EXTRA_OBJ
     * @param $usejoin e.g. "CCT_ACCESS"
     * @return NULL
     */
    function addJoin($usejoin) {
    	$tsc = &$this->tabSeaCond;
    	if ( strstr($usejoin, "CCT_ACCESS")!="" ) {
    	   if ( strstr($tsc['f'], "cct_access")=="" ) {
    		 $tsc['f'] = $tsc['f'] . ", cct_access a";
    		 $answer   = $this->_whereAddPure( $tsc['x'],"a.cct_access_id=x.cct_access_id" );
    		 $tsc['x'] = $answer;
    	   }
    	}
    	if ( strstr($usejoin, "EXTRA_OBJ")!="" ) {
    		if ( strstr($tsc['f'], "extra_obj")=="" ) {
    			$tsc['f'] = $tsc['f'] . ", extra_obj o";
    			$answer   = $this->_whereAddPure( $tsc['x'],"x.extra_obj_id = o.extra_obj_id" );
    			$tsc['x'] = $answer;
    		}
    	}
    	if ( strstr($usejoin, "EXTRA_CLASS")!="" ) {
    	    // needs also JOIN EXTRA_OBJ !
    	    if ( strstr($tsc['f'], "extra_class")=="" ) {
    	        $tsc['f'] = $tsc['f'] . ", extra_class c";
    	        $answer   = $this->_whereAddPure( $tsc['x'],"o.extra_class_id = c.extra_class_id" );
    	        $tsc['x'] = $answer;
    	    }
    	}
    }
    
    /**
     * - get full query string after FROM-keyword
     * - WITHOUT sort !
     * INFO: using an ARCHDB, you could use also $tablename='ARCH.EXP_RAW_RESULT@ORAMIC'
     * @param string  $tablename
     * @return string $sqls
     */
    public function full_query_get( $tablename ) {
      
      $tsc = &$this->tabSeaCond;
      
      $sqls = $tablename. " x" . $tsc["f"];
      if ( $tsc["w"] OR  $tsc["x"] ) {   // extra search condition
    	$sqls = $sqls . " WHERE ";
    	if ( $tsc["w"] != "" )  $sqls = $sqls . " (" . $tsc["w"]. ")";
    	if ( $tsc["w"] AND $tsc["x"] )  $sqls = $sqls . " AND ";
    	$sqls = $sqls . $tsc["x"];
      }
      if ($tsc["y"]) $sqls .= " ".$tsc["y"]; // extended statement: no special APPEND string
      
      return $sqls;
    }
    
    /**
     * - get sort criteria
     * - check, if sort_criteria is possible => check aliases : o. a. c. pc.
     * - relevant for view.tmpl.php
     */
    function _query_sort_org(  ) {
    	
    	
    	
    	$tablename = $this->tablename;
    	$sortcrit_name= "o.".$tablename.".sortcrit";
        $sortcrit = $_SESSION['userGlob'][$sortcrit_name];
    	
    	// check for relevance
    	if (strstr($sortcrit, ".")!=NULL) {	// contains ALIAS-columns ???
    		$fromClause  = $this->tabSeaCond["f"];
    		if ( strstr($sortcrit, "o.")!=NULL AND strstr($fromClause, "extra_obj")==NULL)  $sortcrit = ""; // delete criteria !!!
    		if ( strstr($sortcrit, "a.")!=NULL AND strstr($fromClause, "cct_access")==NULL)  $sortcrit = ""; // delete criteria !!!
    		if ( substr($sortcrit,0,2)== "c."  AND strstr($fromClause, "extra_class")==NULL)  $sortcrit = ""; // delete criteria !!!
    		if ( strstr($sortcrit, "pc.")!=NULL AND strstr($fromClause, "PROJ_HAS_ELEM")==NULL)  $sortcrit = ""; // delete criteria !!!
    	}
        return ($sortcrit);
    }
    
    function _query_sort_get( $sortcrit ) {  // "e.g. "EXP_ID ASC"            
    	// FUNCTION: hwat is, if no sort criteria was given ???
        $retstr = "";
        
        if ($sortcrit!="") {
           $retstr = " order by ". $sortcrit; 
        } 
        return ($retstr);
    }
    
    // PUBLIC functions
    	
    /**
     * add an easy condition to default search
     * @param string $cond
     * @param string $boolop  // "AND", "OR", ...
     * @param string $infostr // ALTERNATIVE iNFO-STR
     */
    function addCond( $cond, $boolop="AND", $infostr=null	) {	
    	if ($this->tablename=="") sys_error_my( 'Class fSqlQueryC not initialized.' );		
    	$this->_WhereAdd( $cond, $boolop, $infostr );
    }
    
    /**
     * returns the SQL-statement after FROM-keyword (from- & where-clause)
     * for the table $tablename
     * @param $opt  : "order" [0] | 1   add order string
     * @return sql-string
     */
    function get_sql_after( $opt=NULL  ) {
      $tablename = $this->tablename;
      $query_str = $this->full_query_get($tablename);
      
      if ( $opt["order"] ) { // TBD: before the extended SQL $xSql ?
        $sortcrit = $this->_query_sort_org(  );
        $tmpSort  = $this->_query_sort_get( $sortcrit );  // with order_by
        $query_str .=  $tmpSort;
      }
      
      return $query_str;
    } 
    
    /**
     * get current info string of $this->tabSeaCond
     */
    function get_sql_info() {
    	$select_info = $this->tabSeaCond["info"];
    	return ($select_info);
    }
    
    /**
     * release the query as standard query for the current user session
     * @return -
     */
    function queryRelase() {
    	$_SESSION["s_tabSearchCond"][$this->tablename] = $this->tabSeaCond;	
    }

}
