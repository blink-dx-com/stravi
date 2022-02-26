<?php
require_once ("subs/glob.objtab.importHead.inc");  
require_once ("subs/glob.objtab.import_sub2.inc");  
require_once ("class.filex.inc");
require_once ('f.fileana.inc');
require_once ("object.subs.inc");
require_once ("o.PROJ.addelems.inc");
require_once ('f.workdir.inc');
require_once 'validate.inc';
require_once ('insert.inc');   
require_once ('f.update.inc');
require_once 'db_x_obj.inc';
require_once ("glob.obj.update.inc");
require_once 'class.filex.inc';

/**
 * - used by glob.objtab.import.php
 * - update selected tables
   - objecte muessen die Ziel-Class schon haben!
 * @package glob.objtab.import.inc
 * @swreq UREQ:0001359: g > import object-attributes from CSV-file 
 * @swreq UREQ:0002637: f.import > csv import > allow escape sequences in NOTES
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/glob.objtab.import.inc 59 2018-11-21 09:04:09Z $
 * 
 * $headerInfo >STRUCT
 *    ["primcol_pos"];
      ["columnInfo"] ;
      ["transformViaWiid"] ;
      ["Wiid_array"];
      ['extra_ob_sql'];
 */

/**
 * UPDATE ONE object
 * @author skube
 *
 */
class OneObjUpdate {
    
    var $datafields; // input argument array from line
    
    function __construct($sqlo, $tablename, $colClass_arr,  $classInfoArr, $parx, $headerInfo, $option, $isassoc, $go) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->tablename = $tablename;
        $this->obj_last=array();
        $this->obj_last[0] =  "";   // NEW_obj
        $this->obj_last[1] =  "";   // ORIGINAL_obj (from other DB)
        $this->obj_last[2] =  0;   // ID ok ???
        
        $this->isbo = cct_access_has2($tablename); // is business object ?
        
        $this->libsub  = new gObjtabImpSubs();
        $this->colClass_arr = $colClass_arr;
        $this->headerInfo = $headerInfo;
        $this->classInfoArr = $classInfoArr;
        $this->parx    = $parx;
        $this->option  = $option;
        $this->isassoc = $isassoc;
        $this->go = $go;
        $this->infolevel      = $parx["infolevel"];
        $this->objcnt    = 0;   // real analyzed lines
        $col_arr     = $headerInfo["col_arr"];
        $this->numColsTotal = sizeof($col_arr) + sizeof($colClass_arr);
        
        $this->wiidTransArr = array();
        
        $this->projAddLib =  NULL;
        if ($parx['projid']) {
            $this->projAddLib = new oProjAddElem($sqlo, $parx['projid']);
            if ($error->Got(READONLY))  {
                $error->set( $FUNCNAME, 1, 'Error on init of destination project.' );
                return;
            }
        }
        
        $this->validate_lib = new fValidate();
        
    }
    
    /**
     * update one object
     * @param object $sqlo
     * @param object $sqlo2
     * @param array $line_arr
     * @return array $obj_last
     */
    function update( &$sqlo, &$sqlo2, $line_arr) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        global $varcol;
        
        $tablename = $this->tablename;
        $obj_last      = $this->obj_last;
        $isassoc   = $this->isassoc;
        $option    = $this->option;
        $primcol_pos   = $this->headerInfo["primcol_pos"];
        $columnInfo    = $this->headerInfo["columnInfo"] ;
        $transformViaWiid = $this->headerInfo["transformViaWiid"] ;
        $iniarr        = $this->headerInfo["Wiid_array"];
        $extra_ob_sql  = $this->headerInfo['extra_ob_sql'];
        $mothertable   = $this->headerInfo['mothertable'];
        $colHasClass   = sizeof($this->headerInfo["colClass_arr"]); // data header has CLASS infos
        $col_arr       = $this->headerInfo["col_arr"];
        $class_id      = $this->classInfoArr[0];
        // $action    = $this->parx['action'];
        
        $is_valid      = 0;
        
        /**
         * if < -100 : flags a severe error and stops
         * @var integer $funcret
         */
        $funcret       = 0;
        $tmpinfo       = "";
        $tmpinfo2      = "";
        $thisLoopOk    = 1;
        $extra_obj_id  = NULL;
        $datafields    = array();
        $primcol        = PrimNameGet2($tablename);
        $flagIgnMisObj  = $option["ignoreMissObj"];
        
     
        if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {    
            debugOut(' headerInfo'.print_r( $this->headerInfo,1), $FUNCNAME);  
        }

        foreach( $line_arr as $key=>$value) {
            $value = trim($value); // trim NEWLINE
            $value = trim($value, "\" "); // trim quotes, white spaces
            $datafields[]   = $value;
        }
        $this->datafields = $datafields;
        
        // $datafieldNum = sizeof($datafields);
        $argu       = array();
        $class_argu = array(); // EXTRA_OBJ arguments
        
        if (  $this->infolevel > 2 AND $this->objcnt<2) {
            echo "DEBUG: first arguments:<pre>";
            var_dump($datafields);
            echo "</pre>";
        }
        
        do {   // UPDATE checks ON_ERROR construct
            
            if ( $isassoc AND $option["motherid"] ) {
                $from_obj_id = $option["motherid"];
            } else {
                
                $from_obj_id = $datafields[$primcol_pos];
                
                if ($columnInfo["objName2ID"][$primcol_pos]) {
                    // the identifier is a NAME => transform to an ID
                    
                    $obj2id_info = gObjtabImpSubs::obj_name2ID($sqlo, $from_obj_id, $columnInfo["objName2ID"][$primcol_pos] );
                    if ( $obj2id_info['retval'] < 0 ) {
                        $tmpinfo  .= "<span style='color:red'>object-name problem</span>";
                        $tmpinfo2 .= $obj2id_info['info'];
                        $argu[$primcol] = $from_obj_id;  // for debugging
                        $funcret        = -2;
                        break;
                    }
                    
                    $from_obj_id =  $obj2id_info['objid'];
                    $datafields[$primcol_pos] = $from_obj_id; // change datafield
                }
            }
            
            
            if ( $from_obj_id=="" ) {
                $tmpinfo  .= "<span style='color:red'>data check failed</span>";
                $tmpinfo2 .= 'object_ID in file is empty!';
                $funcret   = -2;
                break;
            }
            
            $obj_id =  $from_obj_id;
            
            
            
            if ( $obj_last[1] != $from_obj_id ) {
                //
                // NEW mother object detected
                //
                
                if ($transformViaWiid) {
                    $obj_id = $this->libsub->objTransformWiid($sqlo, $iniarr, $from_obj_id, $this->wiidTransArr );
                    
                    if ($obj_id=="") {
                        $thisLoopOk =  0;    // failed !!!
                        $funcret    = -3;
                        $tmptxt     = 'was transformed by WIID, but no transformed object found!';
                        $tmpinfo2  .= $tmptxt;
                        if ( !$flagIgnMisObj ) { // ignore missing object ???
                            $error->set($FUNCNAME, 1, $tmptxt);
                            $tmpinfo  .= "<span style='color:red'>data check failed</span>";
                            $funcret = -123; // unrecoverable
                        }
                        break;
                    }
                    
                    $datafields[$primcol_pos] = $obj_id;  // change thew ID in the datafields
                    if (  $this->infolevel > 1 ) {
                        //$printout = 0;
                        //$tmpdeb = "";
                        //if ( $infolevel > 2 ) $printout = 1;
                        //if ( $printout ) $this->_debug_out( $tmpdeb."origin-id: '$from_obj_id' destination-id: '$obj_id'");
                    }
                    
                }
                
                
                if ( $obj_id!="" ) {
                    // search for OBJ_ID or if ASSOC => in mothertable
                    $sqls = "select ".$primcol .$extra_ob_sql. " from ".$mothertable. " where ".$primcol."='".$obj_id."'";
                    $sqlo->query($sqls);
                    if ( !$sqlo->ReadRow() ) {
                        if (!$flagIgnMisObj) { // ignore missing object ???
                            $tmpinfo  .= "<span style='color:red'>column check failed</span>";
                            $tmpinfo2 .= '(transformed-ID:'.$obj_id.') not found in database!';
                            $funcret   = -2;
                            break;
                        }
                    }
                    
                    if ( $option["toclip"] ) $_SESSION['s_clipboard'][] = array ( "tab"=>$mothertable, "ida"=>$obj_id );
                    
                    if ($colHasClass) {
                        $extra_obj_id  = $sqlo->RowData[1];
                        
                    }
                    $o_rights = access_check( $sqlo, $mothertable, $obj_id );
                    if ( $isassoc ) {
                        if (!$o_rights['insert']) {
                            $argu[$primcol] = $obj_id;  // for debugging
                            $tmpinfo  .= "no insert access";
                            $funcret   = -8;
                            break;
                        }
                    } else {
                        if (!$o_rights['write']) {
                            $argu[$primcol] = $obj_id;  // for debugging
                            $tmpinfo  .= "no write access";
                            $funcret   = -8;
                            break;
                        }
                    }
                    
                    // EXTRA_OBJ CHECK
                    
                    if ($colHasClass) {
                        if (!$extra_obj_id) {
                            $argu[$primcol] = $obj_id;  // for debugging
                            $tmpinfo  .= "object has no class defined";
                            $funcret   = -9;
                            break;
                        }
                        $tmp_class_id = $varcol->obj_id_to_class_id($extra_obj_id);
                        if ($tmp_class_id!=$class_id) {
                            $argu[$primcol] = $obj_id;  // for debugging
                            $tmpinfo  .= "class is wrong: expected:$class_id; is:$tmp_class_id";
                            $funcret   = -10;
                            break;
                        }
                    }
                    
                    // need touch row here only for ASSOC elements; normal objctes will be touched in the gObjUpdate::update_row function
                    if ($this->go == 3 and $isassoc) {
                        //NEW: touch will be done by lower update functions
                    }
                }
                
            } else {
                
                // take from remembered object
                $obj_id     = $obj_last[0];
                $thisLoopOk = $obj_last[2];
                $funcret    = $obj_last[4];
                if ($funcret<0) $tmpinfo    = $obj_last[3]; // only on error case !
                $argu[$primcol] =  $obj_id;  // for debugging
            }
            
            
            if ( $thisLoopOk<1 ) {
                break;
            }
            
            // add the transformation NAME => OBJ_ID
            foreach( $datafields as $colpos=>$value) {
                
                $colinfo     = $col_arr[$colpos];
                $column_name = $colinfo['col'];
                
                if ( $colinfo['col'] != "" ) {
                    
                    // PRIMARY_KEY was transformed before !!!
                    if ($columnInfo["objName2ID"][$colpos] AND $colpos!=$primcol_pos) {
                        $nameori = $value;
                        list($rettmpx, $value, $tmpname, $tmpinfox) = $this->libsub->_name2IdExt( $sqlo, $columnInfo["objName2ID"][$colpos], $nameori);
                        if ($rettmpx<0) {
                            $funcret = -5;
                            $tmpinfo = "Object name error";
                            $argu[$column_name] = "<span style='color:red'>Error:</span> object '".$nameori."': ".$tmpinfox;
                            break;
                        }
                    }
                    $argu[$column_name] =  $value;
                } else {
                    $attribTmpId = $this->colClass_arr[$colpos];
                    if ( $colHasClass && ( $attribTmpId != "") ) {
                        $class_argu[$attribTmpId] = $value;
                    }
                }
                
            }
            if ( $isassoc AND $option["motherid"] ) {
                // $primcol was not in $datafields
                $argu[$primcol] = $option["motherid"];
            }
            
            
            if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
                debugOut('datafields:'.print_r($datafields,1), $FUNCNAME);
                debugOut('argu:'.print_r($argu,1), $FUNCNAME);
            }
            
            if ( $funcret<0 ) {
                break;
            }
            
            $this->validate_lib->check_columns($sqlo2, $tablename, $argu, 1);
            if ( $error->Got(READONLY) ) {
                $err_arr   = $error->getTextAsArray();
                $tmpinfo2 .= "validation errors (features):" . implode("<br>",$err_arr);
                $tmpinfo .= "preparation failed";
                $error->reset(); // reset error-stack
                $funcret = -25;
                break;
            }
            
            if ($colHasClass) {
                
                $class_argu_names=array();
                
                $this->validate_lib->check_extra_values ($class_id, $class_argu);
                if ( $error->Got(READONLY) ) {
                    $err_arr   = $error->getTextAsArray();
                    $tmpinfo2 .= "validation errors (class parameters):" . implode("<br>",$err_arr);
                    $tmpinfo  .= "preparation failed";
                    $error->reset();
                    $funcret = -26;
                    break;
                }
                // finally
                if(!empty($class_argu)) {
                    foreach($class_argu as $att_id=>$val) {
                        $att_name = $varcol->attrib_id_to_name($class_id, $att_id);
                        $class_argu_names[$att_name] = $val;
                    }
                }
            }
            
            if ($this->go == 3)  {
                
//                 if ($action=="insert") {
//                     $retvaltmp = insert_row_s( $sqlo2, $tablename, $argu, $this->modiRowOption );
//                     if ( $error->Got(READONLY) ) {
//                         $tmpinfo .= 'object-ID '.$obj_id.': SQL-insert failed!';
//                         $funcret= -1;
//                         break;
//                     }                 
//                 } else {
//                 }

                if ($this->isbo) {
                    
                    $args=array(
                        'vals'=>$argu
                    );
                    if ($colHasClass) {
                        // $varcol->update ( $extra_obj_id, $class_argu, $class_id );
                        $args['xobj']= array(
                            'extra_class_id' => $class_id,
                            'values' => $class_argu_names
                        );
                    }
                    
                    $UpdateLib = new globObjUpdate();
                    $UpdateLib->update_meta( $sqlo, $tablename, $obj_id, $args, $this->modiRowOption );

                    if ($error->Got(READONLY))  {
                        $tmpinfo  .=  "update failed";
                        $errLast   = $error->getLast();
                        $error_txt = $errLast->text;
                        $error->reset();
                        $tmpinfo2 .=  $error_txt."; Params: ". $this->libsub->_arrayToString($argu);
                        $funcret= -121;
                        break;
                    }
                    
                    if ($this->parx['projid']) {
                        $this->projAddLib->addObj( $sqlo2, $tablename, $obj_id );
                        if ( $error->got(READONLY) ) {
                            $tmpinfo .=  'object-ID '.$obj_id.': add to project failed!';
                            $funcret  = -125;
                            break;
                        }
                    }
     
                } else {
                    $retvaltmp = gObjUpdate::update_row( $sqlo2, $tablename, $argu, $this->modiRowOption );
                    if ( $retvaltmp <= 0) {
                        $tmpinfo  .= "assoc_update failed";
                        $tmpinfo2 .=  'object-ID '.$obj_id.': SQL-update failed!<br>Update-Parameters: '. $this->libsub->_arrayToString($argu);
                        $funcret= -121;
                        break;
                    }
                }
                    
                if ($funcret>=0) {
                    $tmpinfo .= "<span style='color:green'>updated</span>";
                    $is_valid=1;
                }
            } else {
                $tmpinfo .= "<span style='color:green'>ok</span>";
                $is_valid=1;
            }
            
            
        } while (0);  // END: do check loop
        

        $obj_last[0] =  $obj_id;
        $obj_last[1] =  $from_obj_id;
        $obj_last[2] =  $thisLoopOk;
        $obj_last[3] =  $tmpinfo;
        $obj_last[4] =  $funcret;
        $obj_last[5] =  $tmpinfo2;
        $obj_last[6] =  $is_valid; // $validCnt

        $this->argu       = $argu;
        $this->class_argu = $class_argu;
        $this->obj_last   = $obj_last;
        
        return $obj_last;
    }
}


/**
 * insert ONE object
 * @author skube
 *
 */
class OneObjInsert {
    
    /**
     * 
     * @param object $projAddLib
     * @param string $tablename
     * @param array $parx
     * @param array $col_arr
     * @param int $class_id
     * @param array $colClass_arr
     * @param array $columnInfo
     * @param array $options INS_ONE_OBJECT_STRUCT
     */
    function __construct(&$projAddLib, string $tablename, $parx, array $col_arr, $class_id, $colClass_arr, $columnInfo, $options) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->projAddLib = &$projAddLib;
        $this->tablename  = $tablename;
        $this->go     =  $options["go"];
        $this->parx   = $parx;
        $this->col_arr=$col_arr;
        $this->class_id=$class_id;
        $this->colClass_arr=$colClass_arr;
       
        $this->columnInfo = $columnInfo;
        $this->options = $options;
        
        if ( $this->options["testredunt"] ) {
            $this->redundancy_col   = current($this->options['redundancy_cols']);
            $col_red_feat = colFeaturesGet2($tablename, $parx['redundancy_cols_in']);
            $this->redundancy_col_nice = $col_red_feat['NICE_NAME'];
            
            if (!$this->file_col_exists($this->redundancy_col)) {
                $error->set( $FUNCNAME, 1, 'Redundancy-column "'.$this->redundancy_col_nice.'" missing in the file.' );
                return;
            }
            
        }
        $this->validate_lib = new fValidate();
    }
    
    function file_col_exists($col_name) {
        $found=0;
        foreach($this->col_arr as $colinfo) {
            $col_loop  = $colinfo['col'];
            if ( $col_loop==$col_name) {
                $found=1;
                break;
            }
        }
        return $found;
    }
    
    /**
     * prepare argus
     * - TRIM values
     * - TRANFORM NAMES => IDs (optional)
     * @return array($argu, $class_argu)
     */
    function _prepArgus($sqlo, $datafields, $col_arr, $class_id, &$colClass_arr) {
        
        $parx = $this->parx;
        $argu         = array();
        $class_argu   = array();
        $columnInfo   = $this->columnInfo;
        
        foreach( $datafields as $colpos=>$value) {
            $value = trim ($value);
            if ( $parx["trimDouQout"]>0 ) $value = trim ( $value, '"' );
            $value = trim ($value, chr(160)); // strange char from OpenOffice
            $colinfo = $col_arr[$colpos];
            $column  = $colinfo['col'];
            if ( $column != "" ) $argu[$column] =  $value;
            $attribTmpId = $colClass_arr[$colpos];
            if ( $class_id && ( $attribTmpId != "") ) $class_argu[$attribTmpId] = $value;
            
            if ($columnInfo["objName2ID"][$colpos]) {
                // the identifier is a NAME => transform to an ID
                
                $obj2id_info = gObjtabImpSubs::obj_name2ID($sqlo, $value, $columnInfo["objName2ID"][$colpos] );
                if ( $obj2id_info['retval'] < 0 ) {
                    throw new Exception( $obj2id_info['info'] );
                }
  
                $from_obj_id =  $obj2id_info['objid'];
                $argu[$column] = $from_obj_id; // change datafield
            }
        }

        return array($argu, $class_argu);
    }
    
    function insert(&$sql, &$datafields, $tab_len) {
        global $varcol;
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $ins_info=array();
        $go = $this->go;
        
        $tmpinfo='';
        $tmpinfo2='';
        $funcret = 0;  // if  < -10 : severe error => early break
        
        $tablename = $this->tablename;
        $doInsert      = 1;
        $this->datafields = &$datafields;
        $infolevel = $this->parx["infolevel"]; 
        $colinfo  = $this->col_arr[$this->columnInfo["importantNamePos"]];
        $mainColName  = $colinfo['col'];
        

        // break loop
        do {
            

            try {
                list($argu, $class_argu) = $this->_prepArgus( $sql, $this->datafields, $this->col_arr, $this->class_id, $this->colClass_arr);
            } catch (Exception $e) {
                $tmpinfo  .= "<span style='color:red'>Prep-Arguments failed</span>";
                $tmpinfo2 .= $e->getMessage();   
                $funcret        = -13;
                break;
                
            }
            
           
            
            $thisObjName  =  $argu[$mainColName];
            
            // check
            if ( $thisObjName == "" ) {
                $tmpinfo2 .= "object has not IMPORTANT NAME!";
                $tmpinfo .= "<span style='color:#808000'>data check failed</span>";
                $funcret = -1;
                $doInsert = 0;
                break;
            }
            
            if ( $this->options["testredunt"] ) {
                
                
                // do redundancy test:
                $redundancy_col   = $this->redundancy_col;
                $redund_val  = $argu[$redundancy_col];
                
                if ($redund_val!=='' and $redund_val!==NULL) { // ignore NULL value
                
                    $search_array= array($redundancy_col=>$redund_val); 
                    $found_cnt = fileC::redundancyTest2($sql, $tablename, $search_array);
       
                    if  ($found_cnt>0) {
                        $tmpinfo .= ' <span style="color:#808000;">"'.$this->redundancy_col_nice.'" found in DB, no import</span>';
                        $doInsert = 0;
                        $ins_info['found'] = 1;
                        break;
                    }
                }
            }
            
            
            if ($infolevel>2) {
                echo "DEBUG: LEVEL>2: real-SQL-data:<pre>";
                foreach( $argu as $key=>$value) {
                    echo "$key => \"".$value."\" lastchar-ascii: ";
                    if ($value!="") echo ord(substr($value,strlen($value)-1,1));
                    echo "\n";
                }
                echo "</pre>";
            }
            
            
            $this->validate_lib->check_columns($sql, $tablename, $argu, 0);
            if ( $error->Got(READONLY) ) {
                $err_arr   = $error->getTextAsArray();
                $tmpinfo2 .= "validation errors:" . implode("<br>",$err_arr);
                $tmpinfo .= "preparation failed";
                $error->reset(); // reset error-stack
                $funcret = -1;
                break; 
            }
            
            if ($go == 3)  {
                $this_PK_VAL = 0;
                $this_PK_VAL = insert_row( $sql, $tablename, $argu );
                if ( $error->Got(READONLY) ) {
                    $errLast   = $error->getLast();
                    $error_txt = $errLast->text;
                    $error_id  = $errLast->id;
                    $tmpinfo2 .= "object: '".$thisObjName."' SQL-insert action failed! ".
                        "(Err:$error_id:$error_txt)";
                    $tmpinfo .= "insert failed";
                    $error->reset(); // reset error-stack
                    $funcret = -1;
                    break; // !!!
                }
                
                $tmpinfo .= "<span style='color:green'>inserted</span> ";
                
                if ($this->parx['projid']) {
                    $this->projAddLib->addObj( $sql, $tablename, $this_PK_VAL );
                }
                
                
                if ( ($funcret>=0) AND $this->class_id ) {

                    $XobjLib = new fVarcolMeta($sql); 
                    $XobjLib->setObj( $sql, $tablename, $this_PK_VAL );
                    $dummy=0;
                    $XobjLib->extra_update( $sql, $dummy, $this->class_id, $class_argu);

                    if ( $error->got(READONLY) ) {
                        $error->set($FUNCNAME, 1, 'line '.($tab_len).': object-ID: '.$this_PK_VAL.' name:'.$thisObjName.': class-argument creation failed!');
                        $tmpinfo .= ", extra_obj insert failed";
                        $funcret = -11;
                        break;
                    }
 
                }
                if ($funcret>=0) $ins_info['valid'] = 1;
                
            } else { // do insert in the next form-state ...
                $tmpinfo .= "<span style='color:green'>ok</span>";
                $ins_info['valid'] = 1;
            }
            
        } while (0);
        
        $ins_info['funcret'] = $funcret;
        $ins_info['info'] = $tmpinfo;
        $ins_info['info2'] = $tmpinfo2;
        $ins_info['doInsert'] = $doInsert; 
        $ins_info['argu'] = $argu;
        $ins_info['class_argu'] = $class_argu;
        
        
        return $ins_info;
    }
}



class gObjtabImp {
	
	var $tablename;
	var $userfile;  // uploaded file
	var $modiRowOption; // options for insert_row, gObjUpdate::update_row
	var $isassoc;
	var $mothertable;
	var $motherImpCol;
	var $isbo;    // is business object ? 0,1
	var $infoarr; // user info text array
	var $encoding_key;
	var $parx;
	var $file_type;

    function __construct($tablename, $go, $parx ) {
    
    	if (empty($parx["action"])  ) $parx["action"] = "update";    
    	if (empty($parx["errcase"]) ) $parx["errcase"]= "5";
    	
    	$this->file_type = '';
    	$this->userfile  = NULL;
    	$this->tablename = $tablename;
    	$this->go   = $go;
    	$this->parx = $parx;
    	
    	$this->guilib  = new gObjtabImpGui($tablename);
    	
    	$this->infoarr = array();
    	
    	$this->modiRowOption = NULL;
    	
    }
    
    function openFile($filename, $file_type) {
        $this->filename=$filename;
        $this->file_type=$file_type;
    }
    
    /**
     * do initial checks
     * - out: 
     *   -  $this->userfile
     * @param $sqlo
     */
    function initialCheck(&$sqlo) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$parx      = $this->parx;
    	$tablename = $this->tablename;
    	$go		   = $this->go;
    	
    	$this->isbo = cct_access_has2($tablename); // is business object ?
    	
    	$this->htmlActNice = "Update";
    	if ( $parx["action"]=="insert" ) { 
    	    $this->htmlActNice = "Insert";
    	}
    	
    	if (empty($tablename)) {
    		$error->set( $FUNCNAME, 1, "input: table name missing!" );
    		return;
    	}
    	$tablenice = tablename_nice2($tablename);
    	
    	$role_right = role_check_f( $sqlo, "objtab.import");
    	if ( $role_right != "execute"  ) {
    		$error->set( $FUNCNAME, 2, "You must have the role 'f.objtab.import' to perform this. Please ask your admin." );
    		return;
    	} 
    	
    	$t_rights = tableAccessCheck( $sqlo, $tablename );
    	if ( $t_rights['write'] != 1 ) {
    		$msg = getTableAccessMsg( $tablenice, 'write' );
    		$error->set( $FUNCNAME, 3, $msg );
    		return;
    	}
    	
    	if ($parx['projid']) {
    		
    		if (!$this->isbo) {
    			$error->set( $FUNCNAME, 7, 'Object must be of type "business object" to add to project!');
    			return;
    		}
    		
    		$proj_t_rights = tableAccessCheck( $sqlo, 'PROJ' );
    		if ( $proj_t_rights['write'] != 1 ) {
    			$msg = getTableAccessMsg( 'project', 'write' );
    			$error->set( $FUNCNAME, 4, $msg );
    			return;
    		}
    		
    		$o_rights = access_check($sqlo, 'PROJ', $parx['projid']);
    		if ( !$o_rights["insert"] ) {
    			$error->set( $FUNCNAME, 5, 'no "insert" permissions on destination project ID:'.$parx['projid'].' !');
    			return;
    		}
    	}
    	
    	
    	
    	$this->_getMother();
    	
    	// manage file

    	if ($go==1) {
    		$userfile      = $_FILES['userfile']['tmp_name'];
    		$userfile_name = $_FILES['userfile']['name'];
    		$temp_file_short = $userfile_name;
    		
    		$FH = fopen($userfile, 'r');
    		if ( !$FH ) {
    		  $error->set( $FUNCNAME, 9, 'Could not find the uploaded file!');
    		  return;
    		}
    		fclose($FH); 
    		
    		$this->parx['filename'] = $temp_file_short;
    		
    		$workdirObj = new workDir();
    		$tmpdir   = $workdirObj->getWorkDir ( "glob.objtab.import" );
    		if ($error->Got(READONLY))  {
    			$error->set( $FUNCNAME, 8, "Problem with WORKDIR-init." );
    			return;
    		}
    		
    		$new_userfile = $tmpdir . '/' . $temp_file_short;
    		
    	    if (!move_uploaded_file($userfile, $new_userfile))  {
    		   $error->set($FUNCNAME, 9, 'Error during copying temporary file!');
    	       return;  
    	    }
    	    $this->userfile = $new_userfile;
    	   
    
    	}  
    	
    	if ($go>=2) {
    	    
    	    $temp_file_short = $parx['filename'];
    	    if (!$temp_file_short) {
    	        $error->set( $FUNCNAME, 14, "Filename missing." );
    	        return;
    	    }
    	    
    		$workdirObj = new workDir();
    		$tmpdir   = $workdirObj->getWorkDir ( "glob.objtab.import", 1 ); // do NOT clean
    		if ($error->Got(READONLY))  {
    			$error->set( $FUNCNAME, 10, "Problem with WORKDIR-init." );
    			return;
    		}
    		$new_userfile = $tmpdir . '/' . $temp_file_short;
    		$this->userfile = $new_userfile;   
    		
    		if (!file_exists($this->userfile)) {
    			$error->set( $FUNCNAME, 11, 'Uploaded file not found in cache!');
    		  	return;
    		}
    
    		$FH = fopen($this->userfile, 'r');
    		if ( !$FH ) {
    		  $error->set( $FUNCNAME, 12, 'Could not find the uploaded file!');
    		  return;
    		}
    		fclose($FH); 
    	}
    	
    	if ($this->userfile!=NULL) {
    	    $this->file_init();
    	}
    	
    	
    } 
    
    public function setParxVal($key, $val) {
    	$this->parx[$key] = $val;
    }
    
    /**
     * check mother-object settings; produce errors on problem
     * @param object $sql
     * @param array $parx
     */
    public function check_motherneed(&$sql, $parx) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$tablename = $this->tablename;
    	$primcol   = PrimNameGet2($tablename);
    	
    	if ( $parx["motherid"] ) {
            
            $sqls = "select ".$this->motherImpCol." from ".$this->mothertable." where ".$primcol."=".$parx["motherid"];
            $sql->query($sqls);
            if ( $sql->ReadRow() ) {
                $tmpval   = $sql->RowData[0]. " [".$parx["motherid"]."]";  // the name of the object
                $o_rights = access_check( $sql, $this->mothertable, $parx["motherid"] );
                if (!$o_rights['insert']) {
                    $error->set( $FUNCNAME, 1, 'Mother-object: '.$tmpval.': no "insert" access' );
    				return;
                }
                
            } else {  
            	$error->set( $FUNCNAME, 1, 'Mother-object: Object '.$parx["motherid"].' NOT found.' );
    			return;
                
            }
            
        } else { 
        	$error->set( $FUNCNAME, 1, 'Mother-object: Input-ID missing.' );
    		return;
        }
        
        $this->store_info("Mother-Object", $tmpval, "all features come from ONE object" );
    }
    
    
    function form2( $go, $formok, $importMode, $tmpfile ) {
    	$this->guilib->form2( $this->parx, $go, $formok, $importMode, $tmpfile );
    }
    
    
    function store_info($key, $val, $notes=NULL) {
    	$this->infoarr[] = array($key, $val, $notes);
    }
     
    function _info_out ($varname, $text, $notes=NULL, $mode=NULL) {
        echo "<span style='color:gray'>$varname: </span>$text<br>\n";
    }
    
    function showInfoTable() {
    	echo "<table>\n";
    	foreach($this->infoarr as $row) {
    		echo "<tr valign=top><td><span style='color:gray'>".$row[0].":</span></td><td><B>".$row[1]."</B></td>";
            echo "<td><span style='color:gray'>".$row[2]."</span></td></tr>\n";
    	}
    	echo "</table>\n";
    }
    
    function _debug_out ($text) {
        echo "<span style='color:#808000'>INFO: </span>$text<br>\n";
    } 
    

    
    // ------------------------------------------------------------------------------
    
    function _getMother(){ 
    	$tablename  = $this->tablename;
        $isassoc    = 0;
        $motherImpCol = "";     
        $mothertable= mothertable_get2($tablename);
        
        if ($mothertable=="") $mothertable = $tablename;
        else {
            $isassoc    = 1;
            $motherImpCol = importantNameGet2($mothertable);
        }
        
        $this->isassoc     = $isassoc;
        $this->mothertable = $mothertable;
        $this->motherImpCol  = $motherImpCol;
        
        return array($isassoc, $mothertable, $motherImpCol); 
    } 
    
     
    
    function _objClassParamsGet( $extra_class_id ) {
     // RETURN: extra_class_id, class_name, argu_ids   
        global $error, $varcol;
        $tablename = $this->tablename;
        $extra_class_name = $varcol->class_id_to_name( $extra_class_id );
        if ( $error->got(READONLY) ) {  
            $error->set('objClassParamsGet', 1, 'Problem with getting the class info!');
            return array(0,0, NULL);  
        }
        // returns arr[attrib_id] = name 
        $attribNames = $varcol->get_attrib_names($extra_class_id);
        return array($extra_class_id, $extra_class_name, $attribNames); 
    }
    
    
    
    //####################################################################################
    //
    // T H E   I M P O R T 
    //
    
    /**
     * OPEN file + analyse encoding
     */
    function file_init() {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        
        if (!file_exists($this->userfile)) {
            $error->set( $FUNCNAME, 10, 'file "'.$this->userfile.'" not exists.' );
            return;
        }
        
        $this->file_type='csv';
        
        $file_info = pathinfo($this->userfile);
        if ($file_info['extension']=='xlsx') {
            $this->file_type='excel';
        }
        
        $this->file_lib = new f_read_file_meta($this->userfile, $this->file_type);
        $this->file_lib->open_file();
        
        $this->encoding_key = $this->file_lib->get_encoding_info();
        
        $this->_file_init = 1;
    }
    
    
    function get_file_type() {
        return $this->file_type;
    }
    
    
    public function removeFile() {
    	if ($this->userfile!="") unlink ($this->userfile);
    }

}

/**
 * INSERT
 * @author skube
 *
 */
class  gObjtabImpIns extends gObjtabImp {
    
    function __construct($tablename, $go, $parx ) {
        
        $this->libsub  = new gObjtabImpSubs();
        parent::__construct( $tablename, $go, $parx );
    }
    
    /**
     * INSERT of objects with SINGLE primary key ( no ASSOC elements )
     * global:
     *   $this->parx["projid"]   : destination project ID
     * @param object $sql
     * @param object $FH
     * @param array $header_arr
     * @param array $col_arr    :  headerInfo["col_arr"] STRUCT
     * @param array $classInfoArr
     * @param array $colClass_arr
     * @param array $columnInfo
     * @param array $options [OPTIONAL] INS_ONE_OBJECT_STRUCT
     *  "infolevel"
        "go"
        "testredunt" : 0, 1 has test before for NAME existens
        redundancy_cols : array of columns
        "i_shownum" [5]
        "errcase"  :
     
     @return array
     ["objcnt"]
     ["valids"]
     ["bad"]
     ["found"]
     ["lines"]
     ["retval"]
     */
    function _insertObj( &$sql, &$header_arr, &$col_arr, &$classInfoArr, &$colClass_arr, &$columnInfo, $options = NULL ) {
        
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $parx = $this->parx;
        $tablename = $this->tablename;
        
        $retInsert = 0;
        $validCnt  = 0;
        $funcret   = 0;   // 0     : ok;
        // <0    : error in this line
        // <=-10 : unrecoverable error
        $cntBad    = 0;
        $cntFound  = 0;
        $earlyBreak= 0;
        $tab_len   = 1;   // start count behind header !!!
        $objcnt    = 0;   // real analyzed objects
        
        
        $printPoints = 0;
        $infolevel =  $parx["infolevel"];
        //$go        =  $options["go"];          // 3 - do insert
        $class_id  =  $classInfoArr[0];
        
        $parx 	   = $this->parx;
        
        // $mainColName  = $col_arr[$columnInfo["importantNamePos"]];
        
        $projAddLib =  NULL;
        if ($parx['projid']) {
            $projAddLib = new oProjAddElem($sql, $parx['projid']);
            if ($error->Got(READONLY))  {
                $error->set( $FUNCNAME, 10, 'Error on init of destination project.' );
                return;
            }
        }
        
        
        
        
        $obj_lib = new OneObjInsert($projAddLib, $tablename, $parx, $col_arr, $class_id, $colClass_arr, $columnInfo, $options);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 20, 'Insert init problem.' );
            return;
        }
        
        $this->guilib->_print_header( $header_arr, $col_arr, $colClass_arr, $classInfoArr[2] );
        
        while( $this->file_lib->read_line() )  {
            
            
            $tab_len++;
            $objcnt++; // COUNTER FOR REAL ANALYZED LINES
            
            $datafields   = $this->file_lib->get_data_row();
            if ( $infolevel > 1 ) {
            //    $this->_debug_out( 'DATAROW: '.print_r($datafields,1) );
            }
            
            $ins_info = $obj_lib->insert($sql, $datafields, $tab_len);
            $cntFound = $cntFound + $ins_info['found'];
            $validCnt = $validCnt + $ins_info['valid'];
            
            $printout = 0;
            
            if ( $ins_info['funcret']<0   )                       $printout = 1;  // on ERROR
            if ( $infolevel>2 )                       $printout = 1;  // on Info-level
            if ( $objcnt <= $options["i_shownum"] )   $printout = 1;  // info show
            if ( $options["i_shownum"] == "ALL" )     $printout = 1;
            
            if ( $printout ) {
                
                $this->guilib->_infoRow( $printPoints, $col_arr, $tab_len, $ins_info['info'], $ins_info['argu'],
                    $class_id, $ins_info['class_argu'], $ins_info['info2'], $ins_info['funcret']);
                $printPoints = 0;
                
            } else {
                
                $this->guilib->_printProg( $printPoints, $tab_len, $this->numColsTotal, $validCnt );
                $printPoints++;
            }
            
            
            if ($ins_info['funcret']<0) {
                
                // if STOP_ON_ERROR or unrecoverable error
                $cntBad++;
                $earlyBreak=1;
                
                // stop after N errors
                if ( $cntBad<$options["errcase"] )      $earlyBreak=0;
                if ( $options["errcase"] == "continue") $earlyBreak=0;
                
                // hard error !!!
                if ( $ins_info['funcret']<=-10 ) $earlyBreak=1;
                if ( $earlyBreak ) {
                    break;
                }
            }
        }
        
        if ( $printPoints ) {  // still open ???
            echo "</td></tr>\n";
            $printPoints = 0;
        }
        echo "</table>\n";
        
        if ( $earlyBreak ) {
            echo "<br>";
            htmlErrorBox("Error", "Early break! End of file not reached" );
            $retInsert = -1;
        }
        $retarr = array();
        $retarr["objcnt"]  = $objcnt;
        $retarr["valids"]  = $validCnt;
        $retarr["bad"]     = $cntBad;
        $retarr["found"]   = $cntFound;
        $retarr["lines"]   = $tab_len;
        $retarr["retval"]  = $retInsert;
        
        return ($retarr);
    }  
    
    /**
     * 
     * @param object $sql
     * @param object $sql2
     * @param int $go
     * @param array $classInfoArr
     * @param array $option INSERT_OPTS_STRUCT
     * @param array $paramx
     * @param array $headerInfo
     * @return void|number
     */
    function import_do(&$sql, &$sql2, $go,  &$classInfoArr, $option, $paramx, $headerInfo ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        
        $retval    = 0;
        $parx      = $this->parx;
        $infolevel = $parx["infolevel"];
        
        if (!$this->_file_init) {
            $error->set( $FUNCNAME, 11, 'Internal: file_init missing.' );
            return;
        }
        
        $columnInfo     = $headerInfo["columnInfo"] ;
        $colClass_arr   = $headerInfo["colClass_arr"];    // POS => ATTRIB_ID
        $col_arr        = $headerInfo["col_arr"];
        $header_arr     = $headerInfo["header_arr"];
        
       
        
        
        // $this->_info_out("Action", "Insert objects", "", 1 );
        
        $ioptions = NULL;
        //$ioptions["infolevel"] = $infolevel;
        $ioptions["go"]    = $go;
        $ioptions["testredunt"]      = $option["testredunt"];
        if ($option["testredunt"]) {
            $ioptions["redundancy_cols"] = $option["redundancy_cols"];
        }
        $ioptions["i_shownum"] = $option["i_shownum"];
        $ioptions["errcase"]   = $option["errcase"];
        
        $retarr = $this->_insertObj( $sql, $header_arr, $col_arr,
            $classInfoArr, $colClass_arr, $columnInfo, $ioptions );
        
        
        
        
        $tmpimpinfo = "imported";
        if ($go==1) $tmpimpinfo = "valid would be imported";
        $this->guilib->showResultStats($retarr, $tmpimpinfo);
        
        $retval = $retarr["retval"];
        
        return ($retval);
    }
}

/**
 * UPDATE
 * @author skube
 *
 */
class  gObjtabImpUpd extends gObjtabImp {
    
    
    function __construct($tablename, $go, $parx ) {
        parent::__construct( $tablename, $go, $parx );
    }
    
    /**
     * do the import, update or import CSV file NOW!
     * - changes $_SESSION['s_clipboard']
     * @param  $sql
     * @param  $sql2
     * @param  $FH
     * @param int $go // 0: prepare
     // 1: prepare
     // 2: prepare 2
     // 3: do update/insert
     * @param array $classInfoArr
     * @param array $option // "testredunt" = [0] | 1
     // "motherid"
     * @param array $paramx
     "isassoc"
     "mothertable"
     * @param string $action  "update", "insert"
     * @param  array $headerInfo see this_parseHead()
     * @throws Exception errors
     *    10: file handle not valid
     *
     *   return-val-errors:
     *    -25 'file handle not valid.'
     
     */
    function import_do( &$sql, &$sql2, $go, &$classInfoArr, $option = NULL, $paramx,$headerInfo ) {
            
            global $error, $varcol;
            $FUNCNAME= __CLASS__.':'.__FUNCTION__;
            
            if (!$this->_file_init) {
                $error->set( $FUNCNAME, 11, 'Internal: file_init missing.' );
                return;
            }
            
            $tablename = $this->tablename;
            $retval    = 0;
            $parx      = $this->parx;
            $argu      = array();
            $action    = $parx['action'];
            
            
            
            $infolevel      = $parx["infolevel"];
            
            $WIID             = $option["WIID"];
            $flagIgnMisObj    = $option["ignoreMissObj"];
            $flagShowFirstObj = $option["shFirstObj"];
            
            $primcol        = PrimNameGet2($tablename);
            
            $class_id       = $classInfoArr[0];
            $class_name     = $classInfoArr[1];
            $attribNames_arr= $classInfoArr[2];
            
            $isassoc     = $paramx["isassoc"];
            $mothertable = $paramx["mothertable"];
            
            $columnInfo     = $headerInfo["columnInfo"] ;
            $primcol_pos    = $headerInfo["primcol_pos"];
            $colClassAttribName_arr = $headerInfo["colClassAttribName_arr"] ;
            $colClass_arr   = $headerInfo["colClass_arr"];    // POS => ATTRIB_ID
            $col_arr     = $headerInfo["col_arr"];
            $header_arr     = $headerInfo["header_arr"];
            $colHasClass    = sizeof($headerInfo["colClass_arr"]);
            
            // set extra values ...
            $headerInfo["mothertable"] = $mothertable;
            
            
           
            
            if ( $infolevel >=3 ) {
                echo "DEBUG_INFO:<ul>\n";
                if ($columnInfo["objName2ID"][$primcol_pos])
                    echo "- the OBJECT-identifier is a NAME => transform to an ID<br>\n";
                    echo "- isAssoc? $isassoc<br>";
                    echo "</ul>";
            }
            
            
            // Update or assoc
            $options=array();
            $options["infolevel"] = $infolevel;
            $options["go"]        = $go;
            $options["i_shownum"] = $option["i_shownum"];
            $options["errcase"]   = $option["errcase"];
            
            $tmpinfo = "";
            if ( $isassoc ) $tmpinfo = ": insert new features";
            $this->_info_out("Action", "Update objects".$tmpinfo.", go:$go", "", 1 );
            
            //
            // UPDATE checks
            //
            
            if ( $colHasClass ) {
                if ($isassoc) {
                    htmlErrorBox("Error", "ASSOC elements can not have EXTRA_CLASS params");
                    $colHasClass = 0;
                } else {
                    $headerInfo['extra_ob_sql']  = ", EXTRA_OBJ_ID";
                }
            }
            
            if ( $infolevel > 1 ) {
                $this->_debug_out("main table-columns in file:");
                print_r($col_arr);
                echo "<br>";
            }
            
            
            
            $tab_len          = 1;
            $valids           = 0;
            $transformViaWiid = 0;
            
            if ( $WIID ) {
                
                if (cct_access_has2($mothertable)) {
                    // check for the WIID in the database
                    $sqls = "select WIID from H_WIID where NAME='http://www.clondiag.com/magasin/?db=".$WIID."'";
                    $sql->query($sqls);
                    if ( !$sql->ReadRow() ) {
                        $error->set( $FUNCNAME, 1, "The expected database-ID '$WIID' is not in the destination database" );
                        return;
                    }
                    $iniarr=array();
                    $iniarr["dbwiid"] = $sql->RowData[0];
                    $headerInfo["transformViaWiid"]   = 1;
                    $iniarr["mother"] = $mothertable;
                    $iniarr["pname"]  = $primcol;
                    $headerInfo["Wiid_array"] = $iniarr;
                }
            }
            
            echo "<hr>\n";
            
            // ------------------------------------
            // START DATA fields
            // ------------------------------------
            
            $obj_id_last = "";
            $obj_from_id_last = "";
            
            
            $cntBad    = 0;
            $cntFound  = 0;
            $earlyBreak= 0;
            $tab_len   = 1;   // start count behind header !!!
            $objcnt = 0;
            
            $validCnt  = 0;
            $_SESSION['s_clipboard']= NULL; // array ( tab=>"ABSTRACT_PROTO_STEP", ida=>$id, idb=>$th[0], idc=>"" )
            $flagShowName = 1;  // show name of the object ???
            $printPoints  = 0;
            //$wiidTransArr = array();
            
            if ($isassoc) $flagShowName=0;
            
            //$infolevel =  $options["infolevel"];
            //$go        =  $options["go"];
            // $class_id  =  $classInfoArr[0];
            //$numColsTotal = sizeof($col_arr) + sizeof($colClass_arr);
            
            $this->guilib->_print_header( $header_arr, $col_arr, $colClass_arr, $classInfoArr[2] );
            $update_lib = new OneObjUpdate($sql, $tablename, $colClass_arr,  $classInfoArr, $parx, $headerInfo, $option, $isassoc, $go);
            
            while ( $this->file_lib->read_line() ) {
                

                $tab_len++;

                $line_arr = $this->file_lib->get_data_row();
                $objcnt++; 		// COUNTER FOR REAL ANALYZED LINES
                
                $obj_last = $update_lib->update( $sql, $sql2, $line_arr);
                $obj_id   = $obj_last[0];
                $funcret  = $obj_last[4];
                $tmpinfo2 = $obj_last[5];
                $datafields = $update_lib->datafields;
                $class_argu = $update_lib->class_argu;
                $argu       = $update_lib->argu;
                
                $validCnt = $validCnt + $obj_last[6];
                
                $printout = 0;
                
                if ( $funcret<0   ) {
                    //$thisLoopOk = -1;  // save it for the next loop
                    $printout   =  1;  // on ERROR
                    $tmpinfo2   .= '; '.$obj_last[3];
                    if ($error->Got(READONLY))  {
                        $errLast   = $error->getLast();
                        if ($tmpinfo2==NULL) $tmpinfo2 .= '; '.$errLast->text; // show error details
                        $error->reset();   // reset error stack !
                    }
                }
                if ( $infolevel>2 )                       $printout = 1;  // on Info-level
                if ( $objcnt <= $options["i_shownum"] )   $printout = 1;  // info show
                if ( $options["i_shownum"] == "ALL" )     $printout = 1;
                
                if ( $printout ) {
                    // TBD: print also name of object ...
                    
                    if ( $funcret>=0 AND $flagShowName ) {
                        $tmpObjName = obj_nice_name( $sql, $tablename, $obj_id );
                        $argu[$primcol] .= ":".$tmpObjName;
                    }
                    foreach( $col_arr as $pos=>$colinfo) {
                        
                        $col = $colinfo['col'];
                        if ($argu[$col] != $datafields[$pos] AND $pos!=$primcol_pos ) {
                            // if ARGU is different from ori: show them together !!
                            $argu[$col] = "<span style='color:gray'>OLD:".$argu[$col]." NEW:</span> ".$datafields[$pos];
                        }
                    }
                    
                    
                    $this->guilib->_infoRow( $printPoints, $col_arr, $tab_len, $tmpinfo, $argu, $class_id, $class_argu, $tmpinfo2, $funcret );
                    $printPoints = 0;
                    
                } else {
                    $this->guilib->_printProg( $printPoints, $tab_len, $this->numColsTotal, $validCnt );
                    $printPoints++;
                }
                
                
                if ($funcret<0) {
                    // if STOP_ON_ERROR or unrecoverable error
                    $cntBad++;
                    $earlyBreak=1;
                    
                    // stop after N errors
                    if ( $cntBad < $options["errcase"]       ) $earlyBreak=0;
                    if ( $options["errcase"] == "continue" )   $earlyBreak=0;
                    
                    // hard error => recover not possible, break !!!
                    if ( $funcret<=-100 ) $earlyBreak=1;
                    if ( $earlyBreak ) {
                        break;
                    }
                    
                }
                
            }
            
            if ( $printPoints ) {  // still open ???
                echo "</td></tr>\n";
                $printPoints = 0;
            }
            
            // if last line was not printed out ...
            if (!$printout) $this->guilib->_infoRow( $printPoints, $col_arr, $tab_len, $tmpinfo, $argu, $class_id, $class_argu, $tmpinfo2, $funcret);
            
            
            echo "</table>\n";  // INFO-table
            
            if ( $earlyBreak ) {
                $retval = -1;   // error !!!
                echo "<br>";
                htmlErrorBox("Warning", "Early break! End of file not reached" );
            }
            $retarr=array();
            $retarr["objcnt"]  = "$objcnt";
            $retarr["valids"]  = "$validCnt";
            $retarr["bad"]     = "$cntBad";
            $retarr["found"]   = "$cntFound";
            $retarr["lines"]   = "$tab_len";

            
            if ($go==3) $tmpimpinfo = "imported";
            else $tmpimpinfo = "valid (would be imported)";
            
            echo "<br>\n\n";
            htmlInfoBox( "Import result", "", "open", "INFO" );
            
            echo "<B>"                    .$retarr["objcnt"]."</B> objects analyzed. <br>";
            echo "<span style='color:green'><B>"  .$retarr["valids"]."</B></span> data-sets ".$tmpimpinfo.".<br>";
            if ( $retarr["bad"] ) echo " <span style='color:red'><B>".$retarr["bad"]."</B> data set(s) failed.</span><br>";
            if ( $option["toclip"] )  {
                echo "<br><span style='color:gray'><B>".sizeof($_SESSION['s_clipboard'])."</B> objects added to clipboard.</span><br>\n";
            }
            htmlInfoBox( "", "", "close");
            
            
            return $retval;
    }
}