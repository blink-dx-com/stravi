<?php 
require_once 'o.CS_HAS_PR.subs.inc';
require_once 'o.AS_HAS_PR.subs.inc';
require_once 'object.info.inc';
require_once 'glob.obj.conabs.inc';

/**
 * @package o.PROTO.quant_help.inc
 * @author skube
 * @var array STEP_COLTYPE_STRUCT : array( 
 *   's', 
 *   'S', 
 *   'd', 
 *   'D', 
 *   'q', 
 *   'n'  
 *   ); 
 */
class oProtoQuant_help{
    
    // this array defines the KEYS and order of columns in output table
    static $known_types = array( 's', 'S', 'a', 'q', 'n', 'd', 'D'  ); // STEP_COLTYPE_STRUCT
    static $types_map   = array( 's'=>'MAC.name', 'S'=>'MAC.id', 'a'=>'MAA.name', 'q'=>'Quantity', 'n'=>'Notes', 'd'=>'Equipment.name', 'D'=>'Equipment.id'  ); // STEP_COLTYPE_STRUCT
    
    
    
    public static function get_STEP_COLTYPEs() {
        return self::$known_types;
    }
    public static function get_types_map() {
        return self::$types_map;
    }
    
     /** - get selected steps ()
    - mark them as QUANTITY

    * @param object $sql
    * @param int $aprotoid
    * @param array $selsteps : $this->globset["steps"]
    * @return array
    */
    static function _getAbsStepsSel( &$sql, $aprotoid, &$selsteps ) {
        
        $known_types = self::$known_types;
        
        $steparr=array();
        $stepNum = 0;
        $sqls = "select * from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$aprotoid. " order by MAIN_STEP_NR";
        $sql->query($sqls);
        while ( $sql->ReadArray() ) {
            $onestep = $sql->RowData;
            
            $step_nr = $onestep["STEP_NR"];
            
            unset($onestep["MAIN_STEP_NR"]);
            unset($onestep["IS_SAMPLE"]);
            unset($onestep["MANDATORY_VALUES"]);
            
            // check detail-type
            $selinfo = $selsteps[$step_nr];
            if ( $selinfo != NULL ) {
                foreach($known_types as $key) {
                    if ( strstr($selinfo,$key)!=NULL ) {
                        $onestep["TYPE"] = $key;
                        $steparr[] = $onestep;
                    }
                }
            }
            $stepNum ++;
        }
        
        
        return ($steparr);
    }
    
    /*
     get all steps
     - mark them as QUANTITY
     */
    static function _getAbsStepsNorm(&$sql, $aprotoid) {
        
        $steparr=NULL;
        $stepNum = 0;
        $sqls = "select * from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$aprotoid. " order by MAIN_STEP_NR";
        $sql->query($sqls);
        while ( $sql->ReadArray() ) {
            $onestep = $sql->RowData;
            
            $step_nr = $onestep["STEP_NR"];
            
            unset($onestep["MAIN_STEP_NR"]);
            unset($onestep["IS_SAMPLE"]);
            unset($onestep["MANDATORY_VALUES"]);
            $onestep["TYPE"] = "q";
            
            $steparr[] = $onestep;
            $stepNum ++;
        }
        if ( is_array($steparr) ) reset ($steparr);
        
        return ($steparr);
    }
    
    /**
     * get APID from SUA
     * @param object $sqlo
     * @param int $sua_id
     * @return number|mixed
     */
    static function get_apid_fr_sua($sqlo, $sua_id) {
        
        
        $sua_prot_lib = new oAS_HAS_PR_subs($sua_id);
        $proto_log = $sua_prot_lib->getProtoLog($sqlo);
        
        if (empty($proto_log)) {
            return 0;
        }
        
        $row = current($proto_log);
        $apid = $row['ap'];
        
        return $apid;
    }
    
    static function get_abs_obj_html($sqlo, $mo_table, $abs_id) {
        
        $abs_conLib = new gConcAbsC();
        $abs_table  = $abs_conLib->getTemplate( $mo_table );
        if ($abs_table) {
            $html_tmp = obj_nice_name( $sqlo, $abs_table,  $abs_id );
        } else {
            $html_tmp = $abs_id.'?';
        }
        return $html_tmp;
    }
    
}

// -----------------------------------------


/**
 * TBD: extra OBJECT faetures
 * @author skube
 *
 */
class fQuant_obj_extra {
    
    function __construct($tablename) {
        
        $this->tablename = $tablename;
        $this->objdep_lib = NULL;
        
        
        if ($tablename=='EXP') {
            require_once ('o.EXP.quant.inc');
            $this->objdep_lib = new oEXP_Quant();
        }
        
        if ($tablename == 'CONCRETE_SUBST') {
            require_once ('o.CONCRETE_SUBST.quant.inc');
            $this->objdep_lib = new oCONCRETE_SUBST_Quant();
        }
        
        if ($tablename == 'SOCKET') {
            require_once ('o.SOCKET.quant.inc');
            $this->objdep_lib = new oSOCKET_Quant();
        }
    }
    
    function get_objtrc2_obj($sqlo, $name, $origin_table, $mo_obj_id, $options) {
        if (!$this->objdep_lib) return;
        
        return $this->objdep_lib->get_objtrc2_obj($sqlo, $name, $origin_table, $mo_obj_id, $options);
    }
    
    function get_objtrc2_data($sqlo, $name,  $obj_id, $options) {
        if (!$this->objdep_lib) return;
        
        return $this->objdep_lib->get_objtrc2_data($sqlo, $name, $obj_id, $options);
    }
    
}

class oPROTO_ana_cpid {
    
    var $cpfeature;
    var $cStepArr;

    
    /**
     * 
     * @param int $cpid
     * @param array $aProtoInfo
     * @param array $options
     *   "aprotoDiff"
     */
    function __construct(&$aProtoInfo, $options) {
        
        $this->dateformat = 3;
        $this->aProtoInfo = &$aProtoInfo;
        $this->options    = $options;

    }
    
    /**
     * init the Protocol
     * @param int $cpid
     */
    function set_cpid($cpid) {
        $this->cpid = $cpid;
        $this->cStepArr  = NULL;
        $this->cpfeature = NULL;
   
    }
 
    /**
     * analyse one concrete_proto
     * RETURN:
     *  $this->cStepArr  ( data of Concrete-Protocol-Steps )
     $this->cpfeature
    
     $this->cpid
     * @param object $sql
    
     */
    function ana_protocol(&$sql ) {
        
        
        $this->cStepArr  = NULL;
        $this->cpfeature = NULL;
        
        
        $cpid = $this->cpid;
        if (!$cpid) {
            return;
        }
        
        $sqls = "select ABSTRACT_PROTO_ID, NAME, ".$sql->Sql2DateString('EXEC_DATE', $this->dateformat).", CCT_ACCESS_ID from CONCRETE_PROTO where CONCRETE_PROTO_ID=".$cpid;
        $sql->query($sqls);
        $sql->ReadRow();
        
        $this->cpfeature["ABSTRACT_PROTO_ID"] = $sql->RowData[0];
        $this->cpfeature["C_NAME"]			  = $sql->RowData[1];
        $this->cpfeature["EXEC_DATE"] 	 	  = $sql->RowData[2];
        $CCT_ACCESS_ID 	 	  			      = $sql->RowData[3];
        $this->cpfeature["CONCRETE_PROTO_ID"] = $cpid;
        
        if ($this->cpfeature["EXEC_DATE"]=="") {
            // get creation date
            $acc_data = access_data_getai( $sql, $CCT_ACCESS_ID, $this->dateformat );
            $this->cpfeature["EXEC_DATE"] 	= $acc_data["crea_date"];
        }
        
        $aprotid = $this->cpfeature["ABSTRACT_PROTO_ID"];
        
        // TBD: read protocol, even if it doeas NOT match the default prot ???
        if ( empty($this->aProtoInfo[$aprotid]) ) {
            // $this->getAprotoInfos($sql, $aprotid);
            if ($this->options["aprotoDiff"]<=0) return;
        }
        
        
        // get steps
        $steparr = array();
        $sqls = "select * from CONCRETE_PROTO_STEP where CONCRETE_PROTO_ID=".$cpid. " order by STEP_NR";
        $sql->query($sqls);
        while ( $sql->ReadArray() ) {
            $onestep = $sql->RowData;
            $stepnr  = $onestep["STEP_NR"];
            $steparr[$stepnr] = $onestep;
        }
        
        if ( !empty($steparr)) {
            $steparr_poi = &$steparr;
            foreach( $steparr_poi as $step_nr=>$valarr) {
                if ($valarr["CONCRETE_SUBST_ID"]) {
                   
                    $sql->Quesel("NAME, ABSTRACT_SUBST_ID from CONCRETE_SUBST where CONCRETE_SUBST_ID=".$valarr["CONCRETE_SUBST_ID"]);
                    $sql->ReadRow();
                    $s_name = $sql->RowData[0];
                    $steparr[$step_nr]["CONCRETE_SUBST"] = $s_name;
                    $steparr[$step_nr]["sua.id"] = $sql->RowData[1];
                    
                    $sql->Quesel("NAME from ABSTRACT_SUBST where ABSTRACT_SUBST_ID=". $steparr[$step_nr]["sua.id"] );
                    $sql->ReadRow();
                    $steparr[$step_nr]["sua.name"] = $sql->RowData[0];
                }
                if ($valarr["DEV_ID"]) {
                    $sqlsel = "NAME from CHIP_READER where CHIP_READER_ID=".$valarr["DEV_ID"];
                    $sql->Quesel($sqlsel);
                    $sql->ReadRow();
                    $d_name = $sql->RowData[0];
                    $steparr[$step_nr]["DEV"] = $d_name;
                }
            }
           
        }
        
       
        
        $this->cStepArr = $steparr;
    }
}

/**
 * handle one export OBJECT, e.g. one EXP
 * @author skube
 *
 */
class fProtoQ_one_obj {
    
    private $debug=0;
    private $options; /**
        "aprotoMeas"
        "aprotoDiff"
    */
    private $cpid;
    private $cpid_other; // alternative cpid 
    private $SUB_STRUCT;
    private $aProtoOther;
    private $obj_special_lib;
    
    private $aProtoInfo; // aProtoInfo_STRUCT
    
    /**
     * 
     * @var array $obj_cache = array of 
     *   INDEX (so_id) => {
     *     'obj_id' => 
     *     'fea'=> {
     *       'vals'=> 
     *      }
     *     'cStepArr' => cStepArr_STRUCT
     */
    private $obj_cache;
    
    /**
     * temp step array
     * @var array  $cStepArr cStepArr_STRUCT
     * "CONCRETE_SUBST_ID" :
     * "CONCRETE_SUBST" : name of SUC
     * "sua.id"   : ID of SUA
     * "sua.name" : name of SUA
     * "DEV_ID"
     * "DEV" : name of device
     * "QUANITITY"
     *  STEP_NR
     */
    var $cStepArr; // 
    
    /**
     * 
     * @param array $aProtoInfo aProtoInfo_STRUCT
     * @param array $globset
     * @param array $options
     */
    function __construct(&$aProtoInfo, $globset, $options) {

        $this->globset    = $globset;
        $this->aProtoInfo = &$aProtoInfo;
        $this->options    = $options;
        $this->obj_cache  = array();
        $this->aProtoOther = array();
        
        $this->obj_info_lib = new objInfoC();
        $this->proto_ana_lib = new oPROTO_ana_cpid($this->aProtoInfo, $options);

    }
    
    private function _set_error_text($text) {
        $this->err_text = $text;
    }
    
    function init($init_obj_id) {
        $this->obj_cache = array();
        $this->obj_cache[1]['obj_id'] = $init_obj_id;
    }
    
    function set_obj_cache_val($index, $key, $val) {
        $this->obj_cache[$index][$key] = $val;
    }
    
    /**
     * set data for one object
     * calculate $this->obj_id
     * set: 
     *   - $this->mo_obj_id
     * @param object $sqlo
     * @param array $SUB_STRUCT
     * @param int $so_id
     */
    function set_data( $sqlo, $SUB_STRUCT, $so_id ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->SUB_STRUCT = $SUB_STRUCT;
        $this->cStepArr = array();
        $this->cpfeature= array();
     
        // $mo_struct=array();
        $this->so_id = $so_id;
        
        $this->_set_error_text('');
        
        // get obj_id from last MOTHER-Object Protocol-struct
        // debugOut("cahce::".print_r($this->obj_cache,1), $FUNCNAME, 1);
        
        $mo_so_id      = $SUB_STRUCT['from'];
        if ($mo_so_id>0) {
            // $mo_struct     = $this->globset['objects'][$mo_so_id];
            $last_obj_info = $this->obj_cache[$mo_so_id];
        }
        $loop_table    = $SUB_STRUCT['table'];
        $src  = $SUB_STRUCT['src'];
        
        $this->obj_special_lib = new fQuant_obj_extra($loop_table);
        
        // get OBJ-ID
        $this->mo_obj_id = 0;
        
        if ($mo_so_id<=0) {
            // on start
            $obj_id   = $this->obj_cache[1]['obj_id'];
        } else {
            
            $mo_obj_id = $last_obj_info['obj_id'];
            $this->mo_obj_id = $mo_obj_id ;
            
            switch ($src['ty']) {
                
                case 'ass':
                    // associated table
                    $fkt = $src['t'];
                    $pks = primary_keys_get2($fkt);
                    $pk0 = $pks[0];
                    $pk1 = $pks[1];
                    do {
                        if ($fkt=='EXP_HAS_PROTO') {
                            
                            $obj_info  = $this->get_cpid_by_exp($sqlo, $mo_obj_id, $src['abs_id']);
                            $obj_id    = $obj_info['cpid'];
                            break;
                        }
                        if ($fkt=='CS_HAS_PR') {
                            $obj_info  = $this->get_cpid_by_suc($sqlo, $mo_obj_id, $src['abs_id']);
                            $obj_id = $obj_info['cpid'];
                            break;
                        }
                        if ($fkt=='CONCRETE_PROTO_STEP') {
                            $pk1='STEP_NR';
                        }
                        
                        $obj_col  = $src['col'];
                        $obj_id   = glob_elementDataGet2( $sqlo, $fkt, array($pk0=>$mo_obj_id, $pk1=>$src['step']), $obj_col );
                        
                    } while (0);
                    break;
                    
                case 'objtrc':
                    // object tracking
                    if ($mo_obj_id) {
                        $fkt = $src['t'];
                        $pk_name = PrimNameGet2($fkt);
                        $obj_col  = $src['col'];
                        $obj_id   = glob_elementDataGet2( $sqlo, $fkt, array($obj_col=>$mo_obj_id), $pk_name );
                    }
                    break;
                case 'objtrc2':
                    // object tracking extreme
                    
                    
                    if ($mo_obj_id) {
                        $fkt = $src['t'];
                        $tr_opt = array('depend_id'=> $src['depend_id']);
                        $column_name = $src['col'];
                        
                        debugOut(":column_name:$column_name ", $FUNCNAME, 1);
                        
                        $obj_id =  $this->obj_special_lib->get_objtrc2_obj($sqlo, $column_name, $fkt, $mo_obj_id, $tr_opt);
                    }
                    break;
                default:
                    // features
                    $colname = $src['col'];
                    $obj_id   = $this->obj_cache[$mo_so_id]['fea']['vals'][$colname];

            }
            
        }
        
        // echo "DDX3: mo_obj_id:$mo_obj_id colname:$colname SRC:".print_r($src,1)."<br>";
        
        if (!$obj_id) {
            $error->set( $FUNCNAME, 1, 'No OBJ-ID found.' );
            $this->_set_error_text('No OBJ-ID found.');
        }
        
        $this->obj_cache[$so_id]['obj_id'] = $obj_id;
        $this->obj_id = $obj_id;
    }
    
    /**
    * analyse one concrete_proto
    * RETURN:
    *  $this->cStepArr  ( data of Concrete-Protocol-Steps )
       $this->cpfeature
       $this->cpid
    * @param object $sql
    * @param int $cpid  : can be NULL
    */
    function ana_protocol(&$sql ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (empty($this->SUB_STRUCT['steps'])) {
            return array();
        }
        
        $cpid = $this->obj_id;
        $this->cStepArr  = NULL;
        $this->cpfeature = NULL;
      

        
        if (!$cpid) {
            // check alternative cpid ...

            if (!$this->cpid_other) {
                return array();
            }
            $cpid = $this->cpid_other;
        }
        if (!$cpid) {
            return array();
        }
        
        $this->proto_ana_lib->set_cpid($cpid);
        $this->proto_ana_lib->ana_protocol($sql);
        
        $this->cStepArr  = $this->proto_ana_lib->cStepArr;
        $this->cpfeature = $this->proto_ana_lib->cpfeature;
    
        
        // fill cache
        $this->obj_cache[$this->so_id]['cStepArr'] = $this->cStepArr;
    }
    
    /**
     * gain data for one SPECIAL cell
     * @param object $sqlo
     * @param array $col_desc
     * @param int $mo_obj_id
     * @return array
     */
    private function _one_special_cell( $sqlo, $col_desc ) {
        
        $obj_id = $this->obj_id;
        $new_data='';
        switch ($col_desc['ty']) { 
            
            case 'objtrc':
                // object tracking
                if ($this->obj_id) {
                    $table    = $this->SUB_STRUCT['table'];
                    $fkt      = $col_desc['fkt'];
                    $pk_name  = PrimNameGet2($fkt);
                    $obj_col  = PrimNameGet2($table);
                    $new_data   = glob_elementDataGet2( $sqlo, $fkt, array( $obj_col => $this->obj_id ), $pk_name );      
                }
                break;
                
            case 'objtrc2':
                // object tracking extreme
                if ($this->obj_id) {
                    $tr_opt = array('depend_id'=> $col_desc['depend_id']);
                    $column_name = $col_desc['name'];
                    $new_data =  $this->obj_special_lib->get_objtrc2_data($sqlo, $column_name, $this->obj_id, $tr_opt);
                }
                break;
                
            case 'ass':
                // associated data
 
                $fkt = $col_desc['fkt'];
                $pks = primary_keys_get2($fkt);
                $pk1 = $pks[0];
                $pk2 = $pks[1];
                $pk2_val = $col_desc['step'];
                
                if ( $col_desc['pk1'] ) {
                    $pk1     = $col_desc['pk1'];
                }
                
                if ( $col_desc['pk2'] ) {
                    // if set to 'none' : no PK2 will be set 
                    $pk2     = $col_desc['pk2'];
                    $pk2_val = $col_desc['pk2_val'];
                }
                
                $where_arr = array( $pk1=>$obj_id );
                
                if ($pk2!='none') {
                    $where_arr[$pk2] = $pk2_val;
                }
                
                $obj_col  = $col_desc['col'];
                $new_data   = glob_elementDataGet2( $sqlo, $fkt, $where_arr, $obj_col );    
                // echo "DDX3: out:$new_data fkt:$fkt, WHERE: ".print_r($where_arr,1).", $obj_col  )) SRC:".print_r($col_desc,1)."<br>";
                
                break;
            default:
                // features
                $colname  = $col_desc['col'];
                $mo_so_id = $this->SUB_STRUCT['from'];
                $new_data   = $this->obj_cache[$mo_so_id]['fea']['vals'][$colname];      
        }
        return $new_data;
    }
    
    /**
     * get features
     * save data in $this->obj_cache[$this->so_id]['fea']
     * @param object $sqlo
     */
    function ana_features( $sqlo ) {
        global $varcol;
        $objid = $this->obj_id;
        $table = $this->SUB_STRUCT['table'];
        
        $objvals = $this->obj_info_lib->getParamsRaw($sqlo, $table, $objid);
        $this->obj_cache[$this->so_id]['fea'] = $objvals;
        
       
        $cols_poi = &$this->SUB_STRUCT['cols'];
        $X_PATTERN = 'x.';
        $XOBJ_PATTERN = 'ca.';
        $PATT_LEN  = strlen($X_PATTERN);
        $XOBJ_PATT_LEN  = strlen($XOBJ_PATTERN);
        
        // extra-object: class name
        if ($objvals['xobj']['extra_class_id']) {
            $varcol = & Varcols::get();
            $tmp = $varcol->class_id_to_nice_name( $objvals['xobj']['extra_class_id'] );
            $this->obj_cache[$this->so_id]['fea']['vals']['cx.class_name']=$tmp;
            
        }
        // attributes
        foreach($cols_poi as $col_desc) {
            $colname   = $col_desc['name'];
            $db_col_name=substr($colname,3);
            if (substr($colname,0, $XOBJ_PATT_LEN) == $XOBJ_PATTERN) {
                $this->obj_cache[$this->so_id]['fea']['vals'][$colname] = $objvals['xobj']['values'][$db_col_name];
            }
        }
        
        if ( !empty($cols_poi) ) {
            // check extra features: write the data also to obj_cache[$this->so_id]['fea']
            foreach($cols_poi as $col_desc) {
                $colname   = $col_desc['name'];
                if (substr($colname,0, $PATT_LEN) == $X_PATTERN) {
                    $obj_data = $this->_one_special_cell( $sqlo, $col_desc );
                    $this->obj_cache[$this->so_id]['fea']['vals'][$colname] = $obj_data;
                }
            }
 
        }
    }
    
    /**
     * get feature values
     * @param object $sqlo
     * @return array
     */
    function get_fea_outarr($sqlo) {
        
        if (empty($this->SUB_STRUCT['cols'])) return array();
        
        $tmp_point = &$this->obj_cache[$this->so_id]['fea']['vals'];
        $table = $this->SUB_STRUCT['table'];
        
        $out=array();

        $info_cell   = $this->obj_id; // info row
        
        if ( $this->err_text!=NULL ) {
            $info_cell .= ' ERR: '.$this->err_text;
        }
        $out[]   = $info_cell;
        

        foreach($this->SUB_STRUCT['cols'] as $row) {
            
            $val_out = '';
            $colname   = $row['name'];
           
            $val_out   = $tmp_point[$colname];
            
            if ( $row['nice'] and $val_out) {
                // get foreign object
                
                if ($row['t']) {
                    $fkt_loop = $row['t'];
                } else {
                    $col_feat = colFeaturesGet2($table, $colname);
                    $fkt_loop = $col_feat['CCT_TABLE_NAME'];
                }
                
               
                if ($fkt_loop) {
                    $val_out = obj_nice_name($sqlo, $fkt_loop, $val_out);
                }
            }
            
            $out[]   = $val_out;
        }
        return $out;
    }
    
    /**
     * get CPID from SUC for ABSTRACT_PROTO=$apid
     * @param object $sqlo
     * @param int $suc_id
     * @param int $apid
     * @return array
     *   'cpid'       : matching cpid
     *   'other_cpid' : other cpid
     */
    static function get_cpid_by_suc($sqlo, $suc_id, $apid) {
        $cpid = 0;
        
        if (!$suc_id) return 0;
        $suc_cs_lib = new oCS_HAS_PR_subs();
        $suc_cs_lib->setObj($suc_id);
        $proto_log = $suc_cs_lib->getProtoLog($sqlo);
        
        if ( empty($proto_log) ) return 0;
        
        $other_cpid=0;
        foreach($proto_log as $row) {
            if ($apid == $row['ap']) {
                $cpid = $row['cp'];
            } else {
                $other_cpid = $row['cp'];
            }
        }
        return array('cpid'=>$cpid, 'other_cpid'=>$other_cpid);
    }
    
    /**
     * get CPID from EXP for ABSTRACT_PROTO=$apid
     * @param object $sqlo
     * @param int $exp_id
     * @param int $apid
     * @return array
     *   'cpid'       : matching cpid
     *   'other_cpid' : other cpid
     */
    static function get_cpid_by_exp($sqlo, $exp_id, $apid) {
        
        $sqlsel = " e.CONCRETE_PROTO_ID from EXP_HAS_PROTO e, CONCRETE_PROTO c where e.EXP_ID=". $exp_id.
            " AND e.CONCRETE_PROTO_ID=c.CONCRETE_PROTO_ID AND c.ABSTRACT_PROTO_ID=".$apid.
            " order by STEP_NO ";
        
        $sqlo->Quesel($sqlsel);
        
        while ( $sqlo->ReadRow() ) {
            $cpid = $sqlo->RowData[0];
        }

        
        return array('cpid'=>$cpid, 'other_cpid'=>0);
    }
    
    /**
     * get info of other protocol
         RETRUN:   $this->aProtoOther[$aProtoID] = $struct = array ("name", "isMeasure")
     * @param object $sqlo
     * @param int $aProtoID
     */
    function _getOtherInfo( &$sqlo, $aProtoID) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $struct = NULL;
        $sqlsel = "NAME, H_PROTO_KIND_ID from ABSTRACT_PROTO where ABSTRACT_PROTO_ID=".$aProtoID;
        $sqlo->Quesel($sqlsel, $FUNCNAME);
        $sqlo->ReadRow();
        $struct["name"] = $sqlo->RowData[0];
        $kindid = $sqlo->RowData[1];
        if ($kindid) {
            $kindname = glob_elementDataGet( $sqlo, "H_PROTO_KIND", "H_PROTO_KIND_ID", $kindid, "NAME" );
            if ($kindname=="Measurement")  $struct["isMeasure"] = 1;
        }
        
        $this->aProtoOther[$aProtoID] = $struct;
    }
    
    private function _get_empty_proto_arr( &$aProtStepFeat, $info_text) {
        $dataarr=array();
        $dataarr[] = $info_text;
        if ( !empty($aProtStepFeat) ) {
            foreach($aProtStepFeat as $dummy) {
                $dataarr[] = '';
            }
        }
        return $dataarr;
    }
    
    /**
     * get Protocol parameters (one ROW)
     *  INPUT:  $this->cStepArr
   
     $this->cpfeature
     * @param object $sql
     * @param array $extraCols  : array[] = value; must match with $this->colextra
     * @param number $showNoMatching
     * @return array
     */
    function get_step_outarr( &$sql ) {
        
        $showNoMatching=0; //TBD: later dynamic ...
        
        $apid = $this->SUB_STRUCT['src']['abs_id'];
        
        $thisAprotoInfo = &$this->aProtoInfo[$apid];
        $aProtStepFeat  = &$thisAprotoInfo["features"];
        $dataarr   = NULL;
        $info_text = NULL;
        
        $aProtoMatch  = 0;
        $otherStruct  = NULL;
        
        $thisAprotoID = $this->cpfeature["ABSTRACT_PROTO_ID"];
        
        if ($thisAprotoID) {
            
            if ( $this->debug>1 ) {
                glob_printr( $this->cpfeature, "showSteps: cpfeature" );
            }
            
            if ( $thisAprotoID == $this->SUB_STRUCT['src']['abs_id'] ) {
                $aProtoMatch = 1;
                $info_text = "[PRA-ID:".$thisAprotoID."]";

            } else {
                // get other proto-name
                if ( $this->aProtoOther[$thisAprotoID]== NULL ) {
                    $this->_getOtherInfo($sql,$thisAprotoID);
                }
                $otherStruct = &$this->aProtoOther[$thisAprotoID];
                $info_text = 'Other:'.$otherStruct["name"]. " [PRA-ID:".$thisAprotoID."]";
            }
            
            if ( !$aProtoMatch AND $this->options["aprotoDiff"]>0) {
                if ( ($this->options["aprotoMeas"]>0) AND !$otherStruct["isMeasure"] ) {
                    $aProtoMatch = 0; // is NOT a measurement protocol
                } else $aProtoMatch = 1; // ignore different protos
                
            }
        }
        
        if (!$aProtoMatch AND !$showNoMatching ) {
            // error
            $info_text2 = '(1) no matching protocol found. '.$info_text;
            $dataarr = $this->_get_empty_proto_arr( $aProtStepFeat, $info_text2);
            return $dataarr;
        }
        
        
        
        
        if ( $aProtoMatch ) {
            
            $dataarr[] = $info_text; // normally: empty

            if ( !empty($aProtStepFeat) ) {
 
                
                foreach ($aProtStepFeat as $aVals) {
                    
                    $outval = "";
                    $stepnr = $aVals["STEP_NR"];
                    switch ( $aVals["TYPE"] ) {
                        case "s":
                            $outval = $this->cStepArr[$stepnr]["CONCRETE_SUBST"];
                            break;
                        case "S":
                            $outval = $this->cStepArr[$stepnr]["CONCRETE_SUBST_ID"];
                            break;
                        case "a":
                            $outval = $this->cStepArr[$stepnr]["sua.name"];
                            break;
                        case "d":
                            $outval = $this->cStepArr[$stepnr]["DEV"]; 
                            break;
                        case "D":
                            $outval = $this->cStepArr[$stepnr]["DEV_ID"];
                            break;
                        case "q":
                            $outval = $this->cStepArr[$stepnr]["QUANTITY"];
                            break;
                        case "n":
                            $outval = $this->cStepArr[$stepnr]["NOTES"];
                            break;
                    }
                    $dataarr[] = $outval;    
                    
                }  
                
            }
        } else {
            $info_text2 = "<font color=red>no valid proto</font>"; // info string
            $dataarr = $this->_get_empty_proto_arr( $aProtStepFeat, $info_text2);
        }
        return $dataarr;
    }
    
    function get_cache($so_id) {
        return $this->obj_cache[$so_id];
    }
}
