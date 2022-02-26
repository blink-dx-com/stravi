<?php
require_once('validate.inc');
require_once 'f.sql_query.inc';

/**
 * library for user search form preparation
 * used by view.tmpl.inc, view.tmpl.php, ...
 * @package f.search_ulib.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/f.search_ulib.inc 59 2018-11-21 09:04:09Z $
 * 
 * @var $searchArr
 *   'cond'
 *   'infoCond'
 *   "column"
	 "stext" : column value
	 "op"
 */
class f_search_ulib {
	
	
	function __construct($tablename) {
		$this->tablename = $tablename;
	}
	
	/**
	 * do a QUICK search
	 * @param object $sqlo
	 * @param string $idx
	 */
	function quick_search($sqlo, $idx) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    require_once 'lev1/glob.obj.qsearch1.inc';
	    
	    $mainLib = new fSearchLib();
	    $parx=array();
	    $mainLib->init($parx, $idx, $this->tablename, 1);
	    $methods_info  = $mainLib->mainSearch_default($sqlo);
	    $sea_info_arr  = $mainLib->get_sea_info_arr();
	    
	    if ($methods_info['userError']!='') {
	        $error->set( $FUNCNAME, 1, 'Search-Form-Error:'.$methods_info['userError'] );
	        return;
	    }
	    
	    $utilLib = new fSqlQueryC($this->tablename);
	    $utilLib->cleanCond();
	    $utilLib->addCond( $methods_info['where'], '', implode('; ',$sea_info_arr) );
	    $utilLib->queryRelase(); // save in session vars : make it user-global
	    
	}
	
	/**
	 * set default search conditions fro special tables
	 */
	private function _setDefaultCond( &$searchArr ) {
		$tableSCondM = "";
		$sel_info    = "";
		$retval = "";
	
		switch ($this->tablename) {
			case "USER_GROUP":
				$tableSCondM = "(SINGLE_USER=0 OR SINGLE_USER is NULL)"; // only real groups
				$sel_info = "groups with more than one member";
				break;
			case "DB_USER":
				$tableSCondM = "LOGIN_DENY=0"; // only active
				$sel_info    = "active users";
				break;
			case "ROLE":
				if ( !glob_column_exists('ROLE', 'SINGLE_USER') ) break; // because this is a new column (2009-07-23)
				$tableSCondM = "(SINGLE_USER=0 OR SINGLE_USER is NULL)"; // only active
				$sel_info    = "non-single-user roles";
				break;
		}
	
		if ($tableSCondM!=NULL) {
			$searchArr['cond'] = $tableSCondM;
			$searchArr['infoCond'] = $sel_info;
			$searchArr['news'] = 'activate default condition "'.$sel_info.'".';
		}
		return $retval;
	}
	
	/**
	 * special user search ...
	 * @param object $sql
	 * @param array $searchArr searchArr_struc 
	 */
	private static function _USER_specials(&$sql, &$searchArr) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$tmpOper = $searchArr["op"];
		// only special operators allowed
		$allowOper = array('!=', '=', 'LIKE');
		if ( !in_array($tmpOper,$allowOper))  {
			$searchArr["stext"]=NULL; // repair
			$error->set( $FUNCNAME, 1, 'for "user" only following operators allowed:'. implode(', ',$allowOper) );
			return;
		}
		
		$sql->query('SELECT db_user_id FROM db_user WHERE nick = '.$sql->addQuotes($searchArr["stext"]));
		$user_id = $sql->ReadRow() ? $sql->RowData[0] : 0;
		
		if ($user_id<=0) {
			$error->set( $FUNCNAME, 2, 'User "'.$searchArr["stext"].'" unknown!' );
			$searchArr["stext"]  = NULL; // repair
			return;
			 
		} else {
			$searchArr["column"] = 'a.DB_USER_ID';
			$searchArr["stext"]  = $user_id;
			if ($searchArr["op"] != '!=' ) $searchArr["op"] = '='; // set boolean only if "!="
		}
	}
	
	/**
	 * check $value data type
	 * @param object $sql
	 * @param string $tablename
	 * @param string $colname_ext (e.g. x.NAME, a.CREA_DATE)
	 * @param string $value
	 */
	static function check_data_type(&$sql, $tablename, $colname_ext, $value) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$prefix  = substr($colname_ext,0,2);
		$colname = substr($colname_ext,2);
		
		
		switch ($prefix) {
			case "x.":
				$app_id = appDataTypeGet2($tablename, $colname);
				$validate_lib = new fValidate();
				$validate_lib->_check_datatype($tablename, $colname, $value, $app_id);
				break;
				
			case "a.":
				$colNameLen=strlen($colname);
				if ( substr($colname,$colNameLen-5) == "_DATE") {
					$validate_lib = new fValidate();
					$val_info = $validate_lib->check_type($value, 'DATE');
					if ($val_info['ok']<1) {
						//$searchArr["stext"] = NULL;
						$error->set( $FUNCNAME, 2, 'Bad format: '.$val_info['info']);
						return;
					}
				}
			    break;
		}
		 
	}
	
	/**
	 * prepare an ALIAS search
	 * @param object $sql
	 * @param string $tablename
	 * @param array $searchArr
	 */
	private static function alias_search(&$sql, $tablename, &$searchArr) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
		switch ($searchArr["alias"]) {
		
			case "my_data":
				// objects of user this user
				$searchArr["column"] = "a.DB_USER_ID";
				$searchArr["op"]     = "=";
				$searchArr["stext"]  = $_SESSION['sec']['db_user_id'];
				break;
		
			case "my_today":
				// objects of user this user and created today
				$human_date = date ("Y-m-d");
				$sqldate = $sql->DateString2Sql ( $human_date, 1);
				$searchArr["cond"] = "(a.DB_USER_ID=".$_SESSION['sec']['db_user_id']." AND a.CREA_DATE >= ".$sqldate.")";
				break;
			case "absID":
				// get CONCRETE objects from abstract
				// @swreq SRS:0001017:EDI.01:001 needed for Protocol-Editor
				$concAbsLib = new gConcAbsC();
				$absTabName = $concAbsLib->getTemplate($tablename);
				if ($absTabName==NULL) {
					$error->set( $FUNCNAME, 5, 'alias:absID not possible. This table has no abstract type. ');
					return;
				}
		
				// must be NUMERIC
				if ($searchArr["stext"]==NULL or !is_numeric($searchArr["stext"])) {
					$error->set( $FUNCNAME, 5, 'alias:absID: Missing valid ID of abstract object.');
					return;
				}
		
				$absObjID   = $searchArr["stext"];
				$absColName = $absTabName.'_ID';
				$searchArr["cond"] = "x.".$absColName.'='.$absObjID;
				if ($tablename=='CONCRETE_SUBST') {
					// check for META
					// @swreq SRS:0001017:EDI.01:002: select children of META
					$isMETA = glob_elementDataGet($sql, $absTabName, 'ABSTRACT_SUBST_ID', $absObjID, 'IS_META');
					if ($isMETA) {
						$searchArr["cond"] = "x.".$absColName.' in '.
								'(select CHILD_SUBST_ID from ABS_CHILD_SUBST where  ABSTRACT_SUBST_ID='.$absObjID.')';
						$searchArr["column"] = NULL;
						$searchArr["stext"]  = NULL;
						$searchArr["infoCond"] = 'all children of META-substance SUA-ID:'.$absObjID;
					}
				}
		
				break;
			case "q_search":
			    break;
		}
	}
	
	/**
	 *  post preparation of SQL query
	 *  - manage alias searches
	 *  - check "column" and "stext" for syntax
	 *  - returns on Error
	 *  @param array searchArr_struc $searchArr : definition see view.tmpl.php
	 *  @return array AS REFERENCE: searchArr_struc $searchArr
	       $retval <=0 ok
	 */
	static function searchPrepare(&$sql, $tablename, &$searchArr) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		// $searchtxt_ori = $searchArr["stext"];

		if (!$searchArr["op"])
			$searchArr["op"] = "=";
		 
		if ( $searchArr["column"] == "a.user_name" ) {
			self::_USER_specials($sql, $searchArr);
			if ($error->Got(READONLY))  return;
		}
		
		// check data type for "x." columns ...
		if ( $searchArr["column"]!=NULL and $searchArr["stext"]!=NULL) {
			self::check_data_type($sql, $tablename, $searchArr["column"], $searchArr["stext"]);
			if ($error->Got(READONLY)) {
				$searchArr["stext"] = NULL; // reset data
				$error->set( $FUNCNAME, 1, 'Column: '.$searchArr["column"] );
				return;
			}
		}
	
		// handle a DATE
		$colNameLen   = strlen($searchArr["column"]);
		$DATE_KEY     ='_DATE';
		$DATE_KEY_len = strlen($DATE_KEY);
		$col_date_rest_len = $colNameLen-$DATE_KEY_len;
		
		if ( $col_date_rest_len>0 ) {
    			if (
    			    (substr($searchArr["column"],$colNameLen-$DATE_KEY_len) == $DATE_KEY) and
    			     ($searchArr["op"] == '=')
    			) {
    			
    			
        			$DATE_LEN=10;
        			if (strlen($searchArr["stext"])==$DATE_LEN) {
        				$searchArr["cond"] = "(".$searchArr["column"]."  between ".
        						$sql->addQuotes($searchArr["stext"])." and '".$searchArr["stext"]." 23:59:59')";
        				$searchArr["column"] = NULL;
        				$searchArr["stext"]  = NULL;
        			} else {
        				$searchArr["op"] = NULL; // now works for date-cols which do not have '_DATE' in the name
        			}
    	
    		}
		}
	
		if ( strstr( $searchArr["column"], "_DATE") and  ($searchArr["op"] == '='))
			$searchArr["op"] = 'LIKE'; // now works for date-cols which do not have '_DATE' in the name
		
		
	
		if ($searchArr["alias"]!="") {
			self::alias_search($sql, $tablename, $searchArr);
		}
		
		
		return 0;
	}
	
	
	
	/**
	 * - prepare search operators
	 * @param  array searchArr_struc $searchArr (modified)
	 * @param  int $searchOp_i ( "NEW", "OR", "AND" )
	 * @global $_SESSION['userGlob']["o.". $tablename.".condX"] (modified)
	 */
	function prSearchPrepMain( &$sql,  &$searchArr, $searchOp_i, $oldSearchInfo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		$_condGlobKey = "o.". $this->tablename.".condX";
		$_condX = null;
		if ( $_SESSION['userGlob'][$_condGlobKey]!=NULL ) $_condX = unserialize( $_SESSION['userGlob'][$_condGlobKey] );
	
		if ( $searchArr["column"]!="" ) $_condX["col"] = $searchArr["column"];
		if ( $searchArr["op"]!="" )     $_condX["op"]  = $searchArr["op"];
	
		$this->searchPrepare( $sql, $this->tablename, $searchArr );
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'Error on search prepare.' );
			return;
		}
	
		$searchArr["opBool"] = NULL;
		if ( $searchOp_i != "" ) {
			if ( $searchOp_i == "NEW" ) {
				$searchArr["condclean"] = 1;
				$searchArr["opBool"]  = "";
				$tmp_flag  = "NEW";
			} else {
				$tmp_flag="";
				if ( $searchOp_i=="OR" ) $tmp_flag="OR";
				$searchArr["opBool"] = $searchOp_i;
			}
			$_condX["add"] = $tmp_flag;
		}
	
		// analyse condition and set default condition, if no condition is set
		$hasCond=0;
		if ( is_array($searchArr)) {
		    
		    foreach( $searchArr as $key=> $val ) {

				if ($key=="condclean") {
					if ($val>0) $oldSearchInfo=NULL; // will be cleaned
					continue;
				}
				if ($val!=NULL and $key!="op" ) {
					$hasCond=1;
				}
			}
	
	
		}
		# echo "aaa<br>aaa<br>hasCond:$hasCond oldSearchInfo:$oldSearchInfo<br>"; print_r($searchArr);
		if (!$hasCond and $oldSearchInfo==NULL) {
			$this->_setDefaultCond($searchArr);
		}
		
	
		if ($_condX!=NULL) $_SESSION['userGlob'][$_condGlobKey] = serialize($_condX);
	}
	
}

/**
 * handle the modern f.sql_sel2.inc:sql_filter_STRUCT 
 */
class f_search_ulib2 {

    function __construct($tablename) {
        $this->tablename = $tablename;
    }
    
    /**
     * special user search ...
     * @param object $sql
     * @param array $searchArr searchArr_struc
     */
    static function USER_specials(&$sql, &$filter_one) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $new_filter_one=array();
        
        $tmpOper = $filter_one["con"];
        // only special operators allowed
        $allowOper = array('!=', '=', 'LIKE');
        if ( !in_array($tmpOper,$allowOper))  {
            $filter_one["val"]=NULL; // repair
            $error->set( $FUNCNAME, 1, 'for "user" only following operators allowed:'. implode(', ',$allowOper) );
            return;
        }
        
        $sql->query('SELECT db_user_id FROM db_user WHERE nick = '.$sql->addQuotes($filter_one["val"]));
        $user_id = $sql->ReadRow() ? $sql->RowData[0] : 0;
        
        if ($user_id<=0) {
            $error->set( $FUNCNAME, 2, 'User "'.$filter_one["val"].'" unknown!' );
            $filter_one["val"]  = NULL; // repair
            return;
            
        } else {
            $new_filter_one["col"] = 'a.DB_USER_ID';
            $new_filter_one["val"] = $user_id;
            $new_filter_one['con'] = $tmpOper;
            if ($tmpOper != '!=' )  $new_filter_one['con'] = '='; // set boolean only if "!="
        }
        
        return $new_filter_one;
    }
    
    /**
     * check $value data type
     * @param object $sql
     * @param string $tablename
     * @param string $colname_ext (e.g. x.NAME, a.CREA_DATE)
     * @param string $value
     */
    static function check_data_type(&$sql, $tablename, $colname_ext, $value) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $prefix  = substr($colname_ext,0,2);
        $colname = substr($colname_ext,2);
        
        
        switch ($prefix) {
            case "x.":
                $app_id = appDataTypeGet2($tablename, $colname);
                $validate_lib = new fValidate();
                $validate_lib->_check_datatype($tablename, $colname, $value, $app_id);
                if ($error->Got(READONLY))  {
                    $error->set( $FUNCNAME, 1, 'Column: '.$colname );
                }
                break;
                
            case "a.":
                $colNameLen=strlen($colname);
                if ( substr($colname,$colNameLen-5) == "_DATE") {
                    $validate_lib = new fValidate();
                    $val_info = $validate_lib->check_type($value, 'DATE');
                    if ($val_info['ok']<1) {
                        //$searchArr["stext"] = NULL;
                        $error->set( $FUNCNAME, 2, 'Bad format: '.$val_info['info']);
                        return;
                    }
                }
                break;
        }
        
    }
    
    private function filter_one($sql, $use_f_one) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $new_filter_one = $use_f_one;
        
        if (!$new_filter_one["con"])
            $new_filter_one["con"] = "=";
            
        if ( $use_f_one["col"] == "a.user_name" ) {
            $new_filter_one = self::USER_specials($sql, $use_f_one);
            if ($error->Got(READONLY))  return;
            return $new_filter_one;
        }
        
        // check data type for "x." columns ...
        $cond_check_val=1;
        if($use_f_one['con']=='IN LIST') $cond_check_val=0;
        
        if ( $new_filter_one["col"]!=NULL and $new_filter_one["val"]!==NULL and $cond_check_val) {
            self::check_data_type($sql, $this->tablename, $new_filter_one["col"], $new_filter_one["val"]);
            if ($error->Got(READONLY)) {
                $new_filter_one["val"] = NULL; // reset data
                return;
            }
        }
        if ( strstr($use_f_one['con'],"LIKE") ) {
            $new_filter_one["val"] = strtoupper($use_f_one['val']);
        }
        
        // handle a DATE
        
        $DATE_KEY     ='_DATE';
        if ( str_endswith($new_filter_one["col"], $DATE_KEY) and  ($new_filter_one["con"] == '=')  ) {
                    
            $DATE_LEN=10;
            if (strlen($new_filter_one["val"])==$DATE_LEN) {
                $new_filter_one["val"] = "(".$use_f_one["col"]."  between ".
                    $sql->addQuotes($use_f_one["val"])." and '".$use_f_one["val"]." 23:59:59')";
                $new_filter_one["con"] = 'extra';
                $new_filter_one["col"] = NULL;
               
            } else {
                //$searchArr["op"] = NULL; // now works for date-cols which do not have '_DATE' in the name
            }       
        }
        
        if ( strstr( $new_filter_one["col"], "_DATE") and  ($new_filter_one["con"] == '='))
            $new_filter_one["con"] = 'LIKE'; // now works for date-cols which do not have '_DATE' in the name
        
        return $new_filter_one;
    }
    
    /**
     *  post preparation of SQL query
     *  - returns on Error
     *  @param array $usr_filter_all like inc:sql_filter_STRUCT, but more options ...
     *     "col" == "a.user_name" : take NICK
     *     if "con"=='LIKE' : make column TOUPPER ...
     *  @return array $filter_new f.sql_sel2.inc:sql_filter_STRUCT
     */
    function filter(&$sql, &$usr_filter_all) {
        $filter_new=array();
        foreach($usr_filter_all as $use_f_one) {  
            $filter_new[] = $this->filter_one($sql, $use_f_one);
        }
        return $filter_new;
    }
    
}