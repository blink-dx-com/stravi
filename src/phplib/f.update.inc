<?php
require_once 'glob.obj.touch.inc';

/**
 * simple update of an TABLE object
 * usage: gObjUpdate::update_row
 * require_once ('f.update.inc');
 * @package f.update.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class gObjUpdate {
    
    /**
     * - update a business object or a simple PK-object
     * - DEPRICATED: associated table elements (use: class fAssocUpdate)
     * - checks ownership
     * - touches the object (if is an BO_ASSOC => touches the mother)
     * - delete EXTRA_OBJ, if  EXTRA_OBJ_ID=NULL
     * @param object &$sql, 		  sql handle
     * @param string $tablename,     name of table
     * @param array $argu, 	      [key]=value   (must contain the primary key value pairs)
     * @param array $option = NULL <pre>
     *		"types" : see updateSqlOpt_STRUCT
     *		"noRightCheck" [0], 1 
     *      "touch" : -1, [1] deny, allow
     *</pre>
     * @return int answer<pre>
     1  ok
     -1 update forbidden due to user own
     -2 no primary keys </pre>
     */
    static function update_row(&$sql, $tablename, $argu, $option = NULL) {
            global   $error, $varcol;
            $FUNCNAME="update_row";
            
            $varcol = & Varcols::get();
            $error  = & ErrorHandler::get();
            
            $allow_touch=1;
            $ret               = 0;
            $tmp_argu          = $argu;
            $argucnt           = 0;    // count arguments for SET
            $extra_obj_delFlag = 0;
            $sqls              = 'UPDATE '.$tablename.' SET ';
            $id                = '';
            $where_clause      = '';
            $primas            = primary_keys_get2($tablename);
            $num_prims         = sizeof($primas);
            
            if ($option["touch"]<0) $allow_touch=0;
            
            if (!$num_prims) return -1;
            if (!sizeof($tmp_argu)) {
                return -7;
            }
            
            $primcnt = 0; // counter for primary keys
            
            foreach( $primas as $i=>$prim_name) {
                if (isset($tmp_argu[$prim_name])) {
                    if ($i > 0) $where_clause .= ' AND ';  // beim ersten mal kein AND
                    else  $id = $tmp_argu[$prim_name];     // get primary key
                    $where_clause .= $prim_name .'='.$sql->addQuotes($tmp_argu[$prim_name]).' ';
                    unset ( $tmp_argu[$prim_name] );
                    $primcnt++;
                } else {
                    // echo '<font color=red><B>WARNING:</B></font> Primary keys not given for object '.$tablename.' ...<br>';
                    $error->set($FUNCNAME, 1, 'Primary keys not given for  object '.$tablename);
                    return -2;
                }
            }
            
            
            $separ   = '';
            foreach( $tmp_argu as $xcol=>$xval) {
                
                if (($xcol == 'EXTRA_OBJ_ID') AND $xval == '') {
                    $extra_obj_delFlag=1;
                }
                $sqls .= $separ . $xcol;
 
                if ( $xval !== '' and $xval !== NULL ) {
                    $valsql = $sql->addQuotes($xval);
                    if ($option["types"]!="") {
                        $xtype = $option["types"][$xcol];
                        if ($xtype!="") {
                            if ($xtype=="DATE1") {
                                $valsql = $sql->DateString2Sql($xval,1);
                            }
                            if ($xtype=="TIME") {
                                $valsql = $sql->Timestamp2Sql ( $xval );
                            }
                        }
                    }
                    $sqls .= ' ='.$valsql;
                } else {
                    $sqls .= ' = NULL';
                }
                $argucnt++;
                $separ = ', ';
            }
           
            
            $sqls .= ' WHERE '.$where_clause;
            
            if ($id == '') {
                return -3;
            }
            
            $o_rights = access_check($sql, $tablename, $id);
            
            $updateAllow = $o_rights['write'];
            if ( $option["noRightCheck"] ) $updateAllow=1;
            //$update_info = 'ok';
            
            if (!$updateAllow) {
                return (-4); // not allowed by right
            }
            
            if ($allow_touch) {
                $fea_cols = array_keys($tmp_argu);
                globObjTouch::touch( $sql, $tablename, $id, array('d'=>array('x'=>array('fea'=>$fea_cols)), 'a'=>array('key'=>'mod') )  );
            }
            
            if ($num_prims>1) { // is assoc table ?
                $mothertab = mothertable_get2($tablename);
                if ($mothertab!="") {
                    if ($allow_touch) globObjTouch::touch ( $sql, $mothertab, $id, array('d'=>array('x'=>array('ass'=>array('t'=>$tablename))), 'a'=>array('key'=>'mod') )  );
                }
            }
            
            if ( $extra_obj_delFlag ) { // delete extra object
                $ret = $sql->query('SELECT extra_obj_id FROM '.$tablename.' WHERE '.$where_clause, $FUNCNAME.":gXobj1");
                $ret = $sql->ReadRow();
                $extra_obj = $sql->RowData[0];
                
                if ($argucnt) $ret = $sql->query($sqls, $FUNCNAME.":gXobj2"); // first update, then delete extra_obj
                else $ret = 1;
                
                if ($extra_obj) {
                    $varcol->delete($extra_obj);
                    $ret = $error->printLast() ? -6 : 1;
                }
            } else {
                if ($argucnt) $ret = $sql->query($sqls,$FUNCNAME );
                else $ret = 1;
            }
            
            return $ret;
    }
    
    /**
     * - get SQL-string for update_row_s()
     * - NEW: for ASSOC elements: use class fAssocUpdate()
     * @param object $sql  sql handle
     * @param string $tablename
     * @param array $argu argument array [NAME] = VAL
     *   - make EMPTY strings => NULL value
     * @param $idarr array of PRIMARY_KEYS  [NAME] = VAL
     * @param $option <pre>
     * 	"types" : see updateSqlOpt_STRUCT
     * </pre>
     * @return string SQL-statement
     */
    static function update_row_sGet( &$sql, $tablename, &$argu, $idarr, $option=NULL ) {
        global $error;
        $FUNCNAME="update_row_sGet";
        
        $error        = & ErrorHandler::get();
        $i            = 0;
        $separ        = "";
        $where_clause = "";
        $sqls         = 'UPDATE '.$tablename.' SET ';
        $WHERE_SEP    = "";
        
        if (sizeof($idarr)) {
            foreach( $idarr as $keyx=>$valx) {
                if ($valx===NULL or $valx==='') {
                    $error->set($FUNCNAME, 3, "primary key values not valid.");
                    return;
                }
                $where_clause .= $WHERE_SEP . $keyx. "='".$valx."'";
                $WHERE_SEP = " AND ";
            }
            reset($idarr);
        } else {
            $error->set($FUNCNAME, 1, "no primary keys given");
            return;
        }
        
        foreach( $argu as $keyx=>$valx) {
            
            $sqls .= $separ . $keyx;
            if ( $valx === NULL or $valx === '') {
                // make also empty string to NULL !
                $sqls .= ' = NULL';
            } else {
                $valsql = $sql->addQuotes($valx);
                if ($option["types"]!="") {
                    $xtype = $option["types"][$keyx];
                    if ($xtype!="") {
                        if ($xtype=="DATE1") {
                            $valsql = $sql->DateString2Sql($valx,1);
                        }
                    }
                }
                $sqls .= ' ='. $valsql;
            }
            $i++;
            $separ = ', ';
        }
        reset ($argu);
        
        $sqls .= ' WHERE '.$where_clause;
        return ($sqls);
    }
    
    /**
     * - simple update
     - no access check
     - no check of PRIMARY_KEYS
     - no handling of EXTRA_OBJ
     * @param  $sql
     * @param  $tablename
     * @param  array $argu  argument array [NAME] = VAL
     * @param  $idarr array of PRIMARY_KEYS  [NAME] = VAL
     * @param  $option
     * @return number
     */
    static function update_row_s ( &$sql, $tablename, &$argu, $idarr, $option=NULL )  {
        $sqls = self::update_row_sGet($sql, $tablename, $argu, $idarr, $option);
        if ( $sqls!="" ) $ret  = $sql->query($sqls);
        else $ret=-1;
        
        return ($ret);
    }
}