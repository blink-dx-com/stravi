<?php
require_once("sql_query_dyn.inc");
require_once("f.sql_query.inc");
require_once('o.CCT_TABLE.info.inc');

/**
 * Quick search: sub functions LEVEL2 for glob.obj.qsearch.php
 * @package glob.obj.qsearch2.inc
 * @swreq UREQ:0001029: g > glob.obj.qsearch.php : Quick search 
 * @author  Steffen Kube (steffen@blink-dx.com)
 */


/**
 * search ACTION helper class
 */
class fSearchActC {
    
    static function str2sql($searchstr) {
        $sqlstr = str_replace ("'", "''",$searchstr);
        return ($sqlstr);
    }
    
    /**
     *
     * @param object $sql
     * @param string $table  name of table
     * @param string $pkname
     * @param string $where
     * @param array $option
     *   "bool" => "AND", "OR" => if set,  add new CONDITION to old condition
     * @return array
     */
    static function searchx( &$sql, $table,	 $pkname, $where, $option=NULL ) {
        
        $found_matches=0;
        
        $sqlAfter = $table." x where ".$where;
        
        if ($option["bool"]!="") {
            // add new CONDITION to old condition
            $sqlAfter = get_selection_as_sql( $table );
            $sqlAfter .= " AND (".$where.")";
        }
        
        $sqls = "select ".$pkname." from ".$sqlAfter;
        // echo "SQL: $sqls<br>";
        $sql->query($sqls);
        if ( $sql->ReadRow() ) {
            $tmpid = $sql->RowData[0];
            $found_matches = 1;
        }
        if ( $sql->ReadRow() ) {
            $found_matches = 2;
        }
        return array($found_matches, $tmpid);
    }
}


//
// ---- object dependent classes -------------------------------------------------------------------------------------
//



/**
 * object dependent ABSTRACT class
 *
 */
class _table_qsearch_ABS {
    
    protected $searchXOpt;
    protected $tablename;
    protected $sea_info_arr;
    
    function __construct($searchXOpt, $tablename) {
        
        $this->tablename= $tablename;
        $this->searchXOpt =  $searchXOpt;
        $this->sea_info_arr = array();
        $this->pkname   = "x." . PrimNameGet2($tablename);
    }
    
    protected function add_info_str($text) {
        // $searchdone
        $this->sea_info_arr[]=$text;
    }
    
    /**
     * do special search
     * @param object $sql
     * @param string $idx - search string
     * @return array( $found_matches, $found_id, $where, $searchdone);
     */
    function specialSearch(&$sql, $idx) {}
    
    /**
     * get all objects, which contain the EXACT string with wildcards around ...
     * @param object $sqlo
     * @param string $pkname
     * @param string $name_column
     * @param string $idx
     * @return array
     */
    protected function objs_contain_str( $sqlo, $pkname, $name_column, $idx, $max_cache ) {
        
        
        $sqlsel    = $pkname.", ".$name_column." from ".$this->tablename." x where UPPER(".$name_column.") like UPPER('%".fSearchActC::str2sql($idx)."%') order by ".$name_column;
        
        $sqlo->Quesel($sqlsel);
        
        $found_matches=0;
        $found_objs=array();
        
        while ( $sqlo->ReadRow() ) {
            if ($found_matches>=$max_cache) {
                break;
            }
            $tmpid     = $sqlo->RowData[0];
            $found_objs[$tmpid] = array('n'=>$sqlo->RowData[1]);
            $found_matches++;
        }
        return $found_objs;
    }
}

/**
 * object dependent class
 *
 */
class _oABSTRACT_SUBST_qsearch extends _table_qsearch_ABS {
    
    function __construct($searchXOpt) {
        
        parent::__construct($searchXOpt, 'ABSTRACT_SUBST');
    }
    
    /**
     * search for NAME, SYNONYMX and other ...
     * if $idx='PRO:...' search in product number
     * @param object $sqlo
     * @param string $idx
     * @param string $pkname
     * @return array search_result_STRUCT
     *    'matches'=>$found_matches, 
     *    'id'     =>$found_id, 
     *    'where'  =>$where, 
     *    'info'   =>$search_info
     *    'break'  => 0,1 : following search should break, even if no match found here
     */
    function standard_search(&$sqlo, $idx, $pkname) {
        
        $searchXOpt = array();
        $do_break = 0;
        $found_matches=0;
        $found_id=0;
        
        do {
        
            // special search for PROD_NO ?
            $PRO_key = 'PRO:';
            $PRO_key_len = strlen($PRO_key);
            if (substr($idx,0,$PRO_key_len)==$PRO_key) {
                
                $product_no = trim(substr($idx,$PRO_key_len));
                if ($product_no=='') {
                    // do nothing ...
                } else {
                    $sql_val    = fSearchActC::str2sql($product_no);
                    $where      = "UPPER(x.PROD_NO)    like UPPER('".$sql_val."')";
                    list ($found_matches, $found_id) = fSearchActC::searchx( $sqlo, $this->tablename, $pkname, $where, $searchXOpt);
                }
                $this->add_info_str("PRODUCT-number: '".$product_no."'");
                $do_break = 1;
                break;
            }
            
            $sql_val    = fSearchActC::str2sql($idx);

            // try with WILDCARDs, many fields
            
            $use_fields = array('NAME', 'SYNONYMX', 'CAS', 'NOTES');
            $tmpor='';
            foreach($use_fields as $field) {
                $where .= $tmpor . "(UPPER(x.".$field.")  like UPPER('%".$sql_val."%'))";
                $tmpor=' OR ';
            }
            

            list ($found_matches, $found_id) = fSearchActC::searchx( $sqlo, $this->tablename, $pkname, $where, $searchXOpt);
            $this->add_info_str("parts of ".implode(", ",$use_fields).": like '*".$idx."*'");
            
            
        } while (0);
        
        
        $search_info = implode(', ',$this->sea_info_arr);
        
        
        return array( 'matches'=>$found_matches, 'id'=>$found_id, 'where'=>$where, 'info'=>$search_info, 'break'=>$do_break );
    }
    
    /**
     * search in other fields of ABSTRACT_SUBST: do a LIKELY search!
     * @param object $sql
     * @param string $idx
     * @param string $pkname
     * @param array  $options
     *    'MAX_CACHE' => int
     * @return array search_result_STRUCT
     *   extra field Index:4:  array  of ranked objects, max 50
     *      [OBJ-ID] = array('n'=>object name, 's'=>SCORE );
     */
    function specialSearch(&$sqlo, $idx, $options=array() ) {
       
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $MIN_SCORE = 55;
        $MAX_CACHE = 50;
        if ($options['MAX_CACHE']>0) $MAX_CACHE=$options['MAX_CACHE'];
        
        $pkname = $this->pkname;
        $table='ABSTRACT_SUBST';
        $idxSql = fSearchActC::str2sql($idx);
        $where_sql='';
        
        // echo "DDX: word: '$idx' <br><br>";
        
        /**
         * 1. contained in ...
         * 2. wild search
         * @var Ambiguous $select_column
         */
        $name_column = 'NAME';
        $obj_arr_1 = $this->objs_contain_str( $sqlo, $pkname, $name_column, $idx, $MAX_CACHE-20 );
        
        
        
        // a) EDIT_DISTANCE_SIMILARITY
        // b) jaro_winkler_similarity
        $select_column='';
        $method_nice='';
        if ($_SESSION['sec']['_dbtype']=='CDB_OCI8') {
            // jaro-winkler
            $select_column = "ABSTRACT_SUBST_ID, UTL_MATCH.jaro_winkler_similarity( UPPER('".$idxSql."') , UPPER(NAME) ) as DIST, NAME";
            $method_nice='jaro_winkler';
            $method_ord='DESC';
        }
        if ($_SESSION['sec']['_dbtype']=='CDB_PG') {
            # important levenshtein-weighting factors : INS, DEL, SUB
            $select_column = "ABSTRACT_SUBST_ID, levenshtein( UPPER('".$idxSql."') , UPPER(NAME),1,1000,1000 ) as DIST, NAME";
            $method_nice='levenshtein';
            $method_ord='ASC';
        }
        if ($select_column=='') {
            throw new Exception('This search method is not supported!');
        }
        
        // $where = "1=1";
   
        /*
        list ($found_matches, $found_id) = fSearchActC::searchx(
            $sql, 'ABSTRACT_SUBST', $select_column, $where, $this->searchXOpt);
        */
        $max_cache = $MAX_CACHE - sizeof($obj_arr_1); // max 50, but without the objects containing the FULL SEARCH-string
        $sqlsel    = $select_column." from ".$table." order by DIST ".$method_ord;
        
        debugOut("MIN_SCORE:$MIN_SCORE max_cache:$max_cache DDSQL: $sqlsel", $FUNCNAME, 1);
        
        
        $sqlo->Quesel($sqlsel);
        $found_matches =0;
        while ( $sqlo->ReadRow() ) {
            
            if ($found_matches>=$max_cache) {
               break;
            }
            $tmpid     = $sqlo->RowData[0];
            $tmp_score = $sqlo->RowData[1];
            $tmp_name  = $sqlo->RowData[2];
            
            debugOut("- tmpid:$tmpid tmp_score: $tmp_score", $FUNCNAME, 3);
            
//             if ($tmp_score < $MIN_SCORE ) {
//                 break;
//             }
            
            if ( is_array($obj_arr_1[$tmpid]) ) {
                // nothing
            } else {
                $obj_arr_1[$tmpid] = array('n'=>$tmp_name, 's'=>$tmp_score);
            }
            
            $found_matches++;
        }
        
        debugOut('Wild-Search: results: matches: '.$found_matches, $FUNCNAME, 1);
        
        $found_matches = sizeof($obj_arr_1);
        $obj_ids = array();
        
        if ($found_matches) {
           
            $obj_ids   = array_keys($obj_arr_1);
            $found_id  = $obj_ids[0];
            $where_sql = $pkname .' in ('.implode(", ", $obj_ids).')';
        }
        
        $search_info = 'Wild-Search ('.$method_nice.') of  "'.$idx.'"; shows the first '.$found_matches.' matches';
        
        // DEBUG
        /*
        echo "DDD: $found_matches, $found_id <br>";
        echo print_r($where_sql,1);
        echo "DEBUG-STOP";
        */
        // exit;
        
        return array($found_matches, $found_id, $where_sql, $search_info, $obj_arr_1);
    }
}

/**
 * class for CONCRETE_SUBST specific searches
 */
class _oCONCRETE_SUBST_qsearch extends _table_qsearch_ABS {
    
    function __construct($searchXOpt) {
        parent::__construct($searchXOpt, 'CONCRETE_SUBST');
    }
    
    /**
     * search in other fields of Contact
     * @param object $sql
     * @param string $idx
     
     * @return multitype:unknown string
     */
    function specialSearch(&$sql, $idx) {

        $pkname = $this->pkname;
        $idxSql = fSearchActC::str2sql($idx);
        
        $col1 = "NAME";
        $cond2= "ABSTRACT_SUBST_ID in (select ABSTRACT_SUBST_ID from ABSTRACT_SUBST where (UPPER(NAME) like UPPER('%".$idxSql."%')) )";
        
        $where =     "(UPPER(".$col1.") like UPPER('%".$idxSql."%')) ".
            " OR ( ". $cond2 ." )";
        
        list ($found_matches, $found_id) = fSearchActC::searchx($sql, $this->tablename, $pkname, $where, $this->searchXOpt);
        
        $searchdone = "NAME of MAC or NAME of MAA: like '%".$idx."%'";
        
        return array($found_matches, $found_id, $where, $searchdone);
    }
}


/**
 * class for CONTACT specific searches
 */
class _oCONTACT_qsearch extends _table_qsearch_ABS {
    
    function __construct($searchXOpt) {
        parent::__construct($searchXOpt, 'CONTACT');
    }
    
    /**
     * search in other fields of Contact
     * @param object $sql
     * @param string $idx
  
     * @return multitype:unknown string
     */
    function specialSearch(&$sql, $idx) {
        
       
        $pkname = $this->pkname;
        $idxSql = fSearchActC::str2sql($idx);
     
        $col1 = "CONTACT_PERSON";
        $col2 = "EMAIL";
        $col3 = "NAME";
        // or CONTACT has a mother contact with the name
        $cond4= "CON_CONTACT_ID in (select CONTACT_ID from CONTACT where (UPPER(NAME) like UPPER('%".$idxSql."%')) )";
        
        
        $where =     "(UPPER(".$col1.") like UPPER('%".$idxSql."%')) ".
            " OR (UPPER(".$col2.") like UPPER('%".$idxSql."%'))".
            " OR (UPPER(".$col3.") like UPPER('%".$idxSql."%'))".
            " OR ".$cond4 ;
        
        list ($found_matches, $found_id) = fSearchActC::searchx($sql, $this->tablename, $pkname, $where, $this->searchXOpt);
        
        $searchdone = "NAME, PERSON or EMAIL: like '%".$idx."%'";
        
        return array($found_matches, $found_id, $where, $searchdone);
    }
}

/**
 * class for DB_USER specific searches
 */
class _oDB_USER_qsearch extends _table_qsearch_ABS {
    
    function __construct($searchXOpt) {
        parent::__construct($searchXOpt, 'DB_USER');
    }
    
    /**
     * search in other fields of Contact
     * @param object $sql
     * @param string $idx
     * @return array
     */
    function specialSearch(&$sql, $idx) {

        $pkname = $this->pkname;
        $idxSql = fSearchActC::str2sql($idx);
        
        $col1 = "NICK";
        $col2 = "EMAIL";
        $col3 = "FULL_NAME";
        $where =     "(UPPER(".$col1.") like UPPER('%".$idxSql."%')) ".
            " OR (UPPER(".$col2.") like UPPER('%".$idxSql."%'))".
            " OR (UPPER(".$col3.") like UPPER('%".$idxSql."%'))";

        list ($found_matches, $found_id) = fSearchActC::searchx($sql, $this->tablename, $pkname, $where, $this->searchXOpt); 
        $searchdone = "NICK, EMAIL or FULL_NAME: like '%".$idx."%'";
        return array($found_matches, $found_id, $where, $searchdone);
    }
}

/**
 * class for CYTCART specific searches
 */
class _oCYTCART_qsearch extends _table_qsearch_ABS {
    
    function __construct($searchXOpt) {
        parent::__construct($searchXOpt, 'CYTCART');
    }
    
    /**
     * search in other fields of Contact
     * @param object $sql
     * @param string $idx
     * @param string $pkname
     * @return multitype:unknown string
     */
    function specialSearch(&$sql, $idx) {
        
        global $varcol;
        $pkname = $this->pkname;
        $varcol = & Varcols::get();
        $idxSql = fSearchActC::str2sql($idx);
        $search_col='BARCODE';
        $where  = $search_col." like ('".$idxSql."')";
        
        list ($found_matches, $found_id) = fSearchActC::searchx($sql, $this->tablename, $pkname, $where, $this->searchXOpt);
        $searchdone = $search_col.": like '".$idx."'";
        return array($found_matches, $found_id, $where, $searchdone);
    }
}