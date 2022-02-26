<?php
require_once ('glob.obj.advmod.inc');
require_once ('f.update.inc');
require_once 'glob.obj.touch.inc';

/**
 *  handle INSERT/DELETE/UPDATE of associated elements of an object
 *  - to use ADVMOD: you MUST use the close() method at the end of actions !
 * @namespace core::db
 * @package f.assocUpdate.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @example 
 * 
A) SIMPLE (without writing the advanced mod log

$assoclib = new fAssocUpdate();
// init mother object, mother of  'SPOT_RESULT' is 'EXP'
$expid=897;
$assoclib->setObj( $sqlo, 'SPOT_RESULT', $expid ); 
if ($error->Got(READONLY))  { // check access rights
	$error->set( 'assocTest', 1, 'error on init of experiment '.$expid );
	return;
}
// insert one row
$argu=NULL;
$argu["SPOT_ID"]= 5;
$argu["VALUE"]	= 55.4;
$argu["FLAGS"]	= 0;
$assoclib->insert( $sqlo, $argu );

B) COMPLEX

$assoclib = new fAssocUpdate();
// init mother object, mother of  'SPOT_RESULT' is 'EXP'
$expid=897;
$assoclib->setObj( $sqlo, 'SPOT_RESULT', $expid ); 
if ($error->Got(READONLY))  { // check access rights
	$error->set( 'assocTest', 1, 'error on init of experiment '.$expid );
	return;
}
foreach($data as $row) {
    // insert one row
    $assoclib->insert( $sqlo, $row );
}
$assoclib->close($sqlo); // finish the ADVANCED modification log ...

 */
class fAssocUpdate {
    
    const MASS_LOG_FIRST  =0;
    const MASS_LOG_ACT_CHANGE=1;
    const MASS_LOG_POS_RANGE =2;
    const MASS_LOG_ALL_POS   =3;
    const MASS_LOG_ALL       =4;
	
    var $CLASSNAME="fAssocUpdate";
    var $assoctable;// name of assoc table; e.g. 'SPOT_RESULT'
    var $objid;     // ID of mother object; e.g. $assoctable='SPOT_RESULT', $objid=2345 (ID of experiment)
    var $mothertab; // name of mother table e.g. "EXP"
    var $touchAct;  // last logged action
    
    private $isTouched; // object is touched ?
    private $_noTouch;  // 0: ok, 1: do NOT touch
    private $_pos_cache=array();
    /**
     * 
     * @var integer $_log_level
     *  0 : log ONE (first) mod action of many
     *  1 : log only action changes
     *  2 : log all POS-values in ONE log action (as action=mod)
     *  3 : log every action in separate log entry
     
     */
    private $_log_level = 0;
    
    function __construct() {
    	$this->advmodLib   = new obj_advmod();
    }
    
    /**
     * - init an update process
     * - on first call of a MODIFICATION-function: TOUCH the mother once
     * - standard: do ACCESS-check
     * @param object &$sqlo
     * @param string $assoctable name of assoc table, e.g. 'SPOT_RESULT'
     * @param int    $objid ID of mother object
     * @param array  $opt : <pre>
     *		'noAccCheck'=>0,1
     *		'oneTouch' => 0,1 DEPRECATED! use set_log_level()
     *          - only one touch in the object-session 
     * 			(prevent many touches, when action changes)
     *      'mothertab' : alternative mother table, e.g. DB_USER
     * </pre>
     * @exception: error on access-violation
     */
    function setObj( &$sqlo, $assoctable, $objid, $opt=NULL ) {
    
    	global $error;
    	$FUNCNAME = $this->CLASSNAME.":setObj";
    	
    	$this->assoctable= $assoctable;
    	$this->objid     = $objid;
    	$this->mothertab = mothertable_get2($assoctable);
    	if ( is_array($opt) ) {
    	    if ($opt['mothertab']) {
    	        $this->mothertab = $opt['mothertab'];
    	    }
    	}
    	
    	$this->primas    = primary_keys_get2($assoctable);
    	if ( !sizeof($this->primas) ) {
    		$error->set( $FUNCNAME, 1, "Table ".$assoctable." is not defined in system." );
    		return;
    	}
    	$this->assocPK1  = PrimNameGet2($assoctable);
    	$this->tablePosKey  = $this->primas[1];
    	if (!empty($this->primas[2])) {
    	    $this->tablePosKey  = $this->primas[2];
    	}
    	$this->mothIsBo = cct_access_has2($this->mothertab); // mother is BO ?
    	
    	$this->_log_level = 0;
    	$this->isTouched = 0;
    	$this->_noTouch  = 0;
    	$this->touchTxt  = NULL;
    	$this->touchAct  = ''; 
    	$this->_do_advmod= 0; // do ADVMOD ?

    	if ($this->mothIsBo and $opt['noAccCheck']!=1 ) $this->doAccCheck($sqlo);
    	if ($opt['oneTouch']!='') $this->_log_level=self::MASS_LOG_FIRST;
    	
    	if ($this->mothIsBo) {
    	   
    		$advmod_info = $this->advmodLib->setObj($sqlo, $this->mothertab, $objid);
    		if ($advmod_info['dolog']>0) {
    			$this->_do_advmod = 1;
    			$adv_opt=array();
    			
    			if ($assoctable=='USER_ROLES'and $this->mothertab=='DB_USER') $adv_opt['primas_switch']=1;
    			$this->advmodLib->getOriData_ass($sqlo, $assoctable, $adv_opt);
    		}
    	}
    }
    
    
    
    // $noTouch = 0,1
    function setNoTouch($noTouch) {
    	$this->_noTouch  = $noTouch;
    }
    
    // $flag = 0,1 OLD: set_log_level
    function set_log_level($flag) {
        $this->_log_level  = $flag;
        $this->_pos_cache=array();
        if ($flag==self::MASS_LOG_POS_RANGE) {
            $this->_pos_cache=array(0,0);
        }
    }
    
    // do access check
    function doAccCheck(&$sqlo) {
    	global $error;
    	$FUNCNAME = 'doAccCheck';
    
    	$objid = $this->objid;
    	$o_rights = access_check($sqlo, $this->mothertab, $objid);
    	$right='insert';
    	if ( !$o_rights[$right]) {
    		$error->set( $FUNCNAME, 1, 'You do not have '.$right.
    			' permission on object '.$this->mothertab.' ID:'. $objid.' !' );
    		return;
    	}
    }
    
    /**
     * check $idarr
     * @param array $idarr [NAME]=>VAL
     * @return string $where-string, if EMPTY: error
     */
    function _checkPKs( &$idarr ) {
    	   
    	global $error;
    	$FUNCNAME= $this->CLASSNAME.":_checkPKs";
    			
    	$tablename  = $this->assoctable;
    
        if (!sizeof($idarr)) { 
    		$error->set( $FUNCNAME, 1, 'idarr empty' );
            return;
        }
      
    	$primcnt = 0; // counter for primary keys
    	$where   = "";
    	$AND     = "";
    	
    	
    	foreach( $this->primas as $i=>$prim_name) {
    		if ( $idarr[$prim_name] == NULL ) {
    		    $error->set( $FUNCNAME, 1, 'one PK ('.$prim_name.') empty' );
    			$where='';
    			break;
    		}
    		$where .= $AND . $prim_name."='".$idarr[$prim_name]."'";
    		$AND=" AND ";
    	}
    	
    	
    	return ($where);
    }
    
    private function _do_touch(object &$sqlo, string $act, array $dx_ass, $opts=NULL ) {
        
        $this->touchTimeStamp = NULL;
        
        $ass = array('t'=>$this->assoctable);
        $ass = array_merge($ass, $dx_ass);
        if ( is_array($opts["add"]["d"]) ) {
            $ass = array_merge( $ass, $opts["add"]["d"]);
        }
        
        $actarr = array(
            'd'=>array('x'=>array( 'ass'=>$ass  ) ),
            'a'=>array('key'=>$act)
        );
        
        if ( is_array($opts['add']['a']) ) {
            $actarr['a'] = array_merge( $actarr['a'], $opts["add"]["a"]);
        }
        
        if ( is_array($opts['x']) ) {
            $actarr = array_merge($actarr, array('x'=>$opts['x']) );
        }
        $touchRet = globObjTouch::touch( $sqlo, $this->mothertab, $this->objid,  $actarr);
        $this->touchTimeStamp = $touchRet['timest'];
        
    }
    
    /**
     * test for a touch
     * TBD: new parameter: array $dx_ass (merge for  $actarr['d']['x']['ass']); 
     *     keys: po, p0, p1
     * @param array $opts : 
     *	"do" => 0,1 -- do TOUCH ?
     *	"add" = array("d"=> dataadd ) -- add data-info 
     *			      "a"=> actionadd )	
     *  "x"  => extend array , e.g. "dest"=> "url:/data/1>"
     * @global  $this->_noTouch
     * @return int
     *   0 : no touch 
     *   1 : touched
     */
    private function _touchTest( object &$sqlo, string $act, array $dx_ass, $opts=NULL ) {
    	
    	$doTouch = 0; // default 
    	
    	if ($this->_noTouch>0)  {
    	    return 0;
    	}
    	
    	if ( $this->mothIsBo) {
    	    
    	    do {
    	        
    	        if ($this->_log_level==self::MASS_LOG_FIRST) {
            		if ( !$this->isTouched ) {
            			$doTouch = 1;  // first action will be logged 
            		} else {
            		    // already touched ...
            		}
            		break;
    	        }
 
    	        if ( $this->_log_level== self::MASS_LOG_ACT_CHANGE) {
    	            if ($act != $this->touchAct) $doTouch = 1; // action changed
    	            break; 
    	        }
        		
        		// $act not equal last $act ....
    	        if ($this->_log_level == self::MASS_LOG_POS_RANGE) {
        		    if ( !empty($dx_ass['po']) ) {
        		        // calc MIN, MAX
        		        if (!$this->_pos_cache[0]) $this->_pos_cache[0]=$dx_ass['po'];
        		        if ($dx_ass['po']< $this->_pos_cache[0]) {
        		            $this->_pos_cache[0] = $dx_ass['po'];
        		        }
        		        if ($dx_ass['po']>$this->_pos_cache[1]) {
        		            $this->_pos_cache[1] = $dx_ass['po'];
        		        }
        		    }
        		    break;
        		}
        		
        		if ( $this->_log_level==self::MASS_LOG_ALL_POS) {
        		    if ( !empty($dx_ass['po']) )  $this->_pos_cache[]=$dx_ass['po'];
        		    break;
        		}
        		
        		// self::MASS_LOG_ALL ...
        		$doTouch = 1;
        		
        		
    	    } while (0);
    	}
    	
    	if ( $opts["do"] ) 	$doTouch = 1;
    	$this->touchAct = $act;
    	$this->isTouched++;
    	
    	if ( $doTouch ) {
    	    $this->_do_touch($sqlo, $act, $dx_ass, $opts );
    	}
  
    	return $doTouch;
    }
    
    /**
     * tranbsform argu-array to SQL string
     * @param array $argu
     * @return string de-html_entitied string (no entities at all)
     */
    private function argu2sqls( &$sqlo, &$argu, $options=NULL ) {
        
        $sqls = NULL;
        $i=0;
        foreach( $argu as $xcol=>$xval) {
            
            if ($i) { // beim ersten val KEIN komma
                $sqls     .=', ';
            }
            
            if ( $xval === "" or $xval === NULL ) $tmpvalSql = 'NULL';
            else {
                $tmpvalSql = $sqlo->addQuotes($xval);
                if ($options["types"]!="") {
                    $xtype=$options["types"][$xcol];
                    if ($xtype!="") {
                        if ($xtype=="DATE1") {
                            $tmpvalSql = $sqlo->DateString2Sql($xval,1);
                        }
                    }
                }
            }
            
            $sqls .= $xcol."=".$tmpvalSql;
            $i++;
        }
       
        
        return ($sqls);
    }
    
    /**
     * - insert one ROW of an assocElem without checking for CCT_ACCESS (see also insert_row)
     * - warning $argu will be changed
     * @param $sql
     * @param $argu array ("COLUMN_NAME" => value, ...) (column_names in capital letters!)
     with PK1
     * @param array $options array of [COL_NAME] <pre>
     * 	 "types" = array [COL_NAME] => type
     "DATE1" : value in format: YYYY-MM-DD HH24:MI:SS
     "TIME"  : Unix-time-stamp
     </pre>
     * @global $this->assoctable
     * @return  >0 or "text": OK -> this is the primary key
     *         =0 error
     */
    private function _insert_row( &$sql, &$argu, $options = NULL) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
            
        $tablename = $this->assoctable;
        $sqls      = ' VALUES (';
        $sql_cols  = 'INSERT INTO '.$tablename.' (';
        $i         = 0;
        
        if ( empty($argu[$this->assocPK1]) ) {
            $error->set( $FUNCNAME, 1, "No assocPK1 set" );
            return;
        }
        
       
        foreach( $argu as $xcol=>$xval) {
            
            if ($i) {// beim ersten val KEIN komma
                $sqls     .=', ';
                $sql_cols .=', ';
            }
            
            //if (($this->assocPK1 === $xcol) && $xval==0 ) {
            //    $xval = ''; // make 0 to NULL for triggers
            //}
            
            $sql_cols .= $xcol;
            if ( $xval === "" or $xval === NULL) $tmpvalSql = 'NULL';
            else {
                $tmpvalSql = $sql->addQuotes($xval);
                
                if ($options["types"]!="") {
                    $xtype=$options["types"][$xcol];
                    if ($xtype!="") {
                        if ($xtype=="DATE1") {
                            $tmpvalSql = $sql->DateString2Sql($xval,1);
                        }
                        if ($xtype=="TIME") {
                            $tmpvalSql = $sql->Timestamp2Sql( $xval );
                        }
                    }
                }
            }
            
            $sqls .= $tmpvalSql;
            $i++;
        }
       
        
        $sqls   = $sql_cols.') '.$sqls.')'; // assemble whole query
        $retval = $sql->Query( $sqls, $FUNCNAME ); // do insert
        
        return ($retval);
    }
    
    // PUBLIC methods
    
    public function touch_test( object &$sqlo, string $act, array $dx_ass, $opts=NULL ) {
        $this->_touchTest( $sqlo, $act, $dx_ass,  $opts);
    }
    
    /**
     * get touch info dict
     * @return array ('act'=>$this->touchAct, 'timest'=>$this->touchTimeStamp)
     */
    function getTouchDict() {
    	return array('act'=>$this->touchAct, 'timest'=>$this->touchTimeStamp);
    }
    function getTouchStats() {
        return array('act'=>$this->touchAct, 'isTouched'=>$this->isTouched, 'log_level'=>$this->_log_level, 
            'noTouch'=>$this->_noTouch, 'pos_cache'=>$this->_pos_cache);
    }
    
    function resetTouched() {
    	$this->isTouched = 0;
    	$this->set_log_level($this->_log_level);  // some more resets
    }
    
    /**
     * @param array argu : without mother-PK
     * @return int > 0 o.k. or $error
     */
    function insert( &$sqlo, $argu, $inopt = NULL ) {
    
    	$argu[$this->assocPK1] = $this->objid;
    	$retval = $this->_insert_row ( $sqlo, $argu, $inopt );
    	$this->_touchTest( $sqlo, "new", array('po'=>$argu[$this->tablePosKey]) );
    	return ($retval);
    }
    
    /**
     * update one element
     * @param object $sqlo
     * @param array  $argu args(KEY=>VAL)
     * @param array  $idarr primary keys, without mother-ID, e.g. for 'SPOT_RESULT' : ("SPOT_ID"=>3)
     * @param array  $option update options
     *   'touchopt' => array() FUTURE
     * @return -
     */
    function update( &$sqlo, &$argu, $idarr, $option=NULL ) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    	$idarr[$this->assocPK1] = $this->objid;
    	
    	$sqls = gObjUpdate::update_row_sGet($sqlo, $this->assoctable, $argu, $idarr, $option);
        if ( $sqls!="" ) $ret  = $sqlo->query($sqls);
    	if ( $ret<1 ) {
    		$error->set( $FUNCNAME, 1, "Error on assoc-update on OBJ:".$this->assoctable.":".$this->objid );
    		return;
    	}
    	
    	//if ($option['touchopt']!=NULL) $touchOpt=$option['touchopt'];
    	//else $touchOpt=NULL;
    	$tOpt=array();
    	$this->_touchTest( $sqlo, "mod", array('po'=>$idarr[$this->tablePosKey]), $tOpt );
    	
    	return;
    }
    
    /**
     * update selected elements from one object
     * @param object $sqlo
     * @param array  $argu
     * @param string $whereAdd additional where condition
     * @param array $opts // "touch" : 
     *     array("p0"=>number, "p1"=> number ) : position range
     *     "ainfo" STRING
     * @return -
     */
    function updateMore( &$sqlo, &$argu, $whereAdd, $opts=NULL ) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    	if ( is_array($argu) ) {
    		$sqlArgu  = $this->argu2sqls( $sqlo, $argu );
    		$wherestr = $this->assocPK1."='". $this->objid."' and (".$whereAdd.")";
    		$sqlo->Updatex( $this->assoctable, $sqlArgu." where ".$wherestr );
    	}
    
    	$topt = NULL;
    	if ($opts["touch"] !=NULL ) {
    	    $topt["add"]=array();
    		if ($opts["touch"]["p0"] !==NULL ) {
    		    $topt["add"]['d'] = array( 'p0'=>$opts["touch"]["p0"], 'p1'=>$opts["touch"]["p1"] );
    		}
    		if ($opts["touch"]["ainfo"] !==NULL ) {
    			$topt["add"]["a"] = $opts["touch"]["ainfo"];
    		}
    	} 
    	$this->_touchTest( $sqlo, "mod", array(), $topt ); // 'po' is unknown
    
    }
    
    // delete ONE row
    function delOneRow( &$sqlo, &$idarr ) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$idarr[$this->assocPK1] = $this->objid;
    	$wherestr = $this->_checkPKs( $idarr );
    	
    	if ( $wherestr==NULL or $error->Got(READONLY) ) {
    		$error->set( $FUNCNAME, 1, "ID-array of obj:".$this->assoctable.":".$this->objid." not valid." );
    		return;
    	}
    	
    	$sqlo->Deletex ( $this->assoctable, $wherestr );
    	$this->_touchTest( $sqlo, "del", array('po'=>$idarr[$this->tablePosKey]) );
    		
    }
    
    /**
     * delete all/selected elements from one mother-object
     * @param object $sqlo
     * @param string [$addWhere] optional where condition
     * @return -
     */
    function delMore( &$sqlo, $addWhere=NULL ) {
    	//global $error;
    	//$FUNCNAME= $this->CLASSNAME.":delOne";
    	
    	$wherestr = $this->assocPK1."='". $this->objid."'";
    	if ($addWhere!="") {
    		$wherestr .= " AND (".$addWhere.")";
    	} 
    	$sqlo->Deletex ( $this->assoctable, $wherestr );
    	$this->_touchTest( $sqlo, "del", array() ); // 'po' is unknown
    }
    
    
    
    /**
     * - INSERT or UPDATE one element
     * - if element NOT exists: INSERT
     * - if element exists: UPDATE
     * @param array $argu (without PKs)
     */
    function updateInsert( &$sqlo, &$argu, $idarr, $options = NULL) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    	$tablename = $this->assoctable;
    	$fullPks   = $idarr;
    	$fullPks[$this->assocPK1] = $this->objid;
    
    	$data = gElement_exist2( $sqlo, $tablename, $fullPks );
    	if ( $data==NULL ) {
    		$newargu = $idarr;
    		$newargu = array_merge($newargu, $argu);
    		$retval  = $this->insert( $sqlo, $newargu, $options );
    		
    		if ($retval<1) {
    			$error->set( $FUNCNAME, 1, "error on insert on PKs:". glob_array2String( $fullPks ) );
    			return;
    		}
    	} else {
    		$this->update($sqlo, $argu, $idarr);
    	}
    }
    
    /**
     * close modification for this object
     * check ADVMOD
     */
    function close(&$sqlo) {
    	global $error;
    	//$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	if ( $this->_log_level==self::MASS_LOG_POS_RANGE) {
    	    $this->_do_touch($sqlo, $this->touchAct, array('p0'=>$this->_pos_cache[0], 'p1'=>$this->_pos_cache[1], 'pN'=>$this->isTouched) );
    	}
    	if ($this->_log_level==self::MASS_LOG_ALL_POS) {
    	    $this->_do_touch($sqlo, $this->touchAct, array('px'=>$this->_pos_cache) );
    	}
    	
    	if ($this->_do_advmod) {
    		$touchInfo = array( 'timest'=>$this->touchTimeStamp ); // TBD: must come in FUTURE from gObjUpdate::update_row !
    		$this->advmodLib->getPostData_ass($sqlo, $touchInfo);
    		if ($error->Got(READONLY))  {
    			$error->logError(); // but no user output ...
    		}
    	}
    }

}
