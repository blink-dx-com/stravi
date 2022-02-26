<?php
require_once ("insertx.inc");
require_once ('get_cache.inc');
require_once 'o.S_VARIO.mod.inc';
require_once 'o.PROJ.addelems.inc';
require_once 'o.IMG.file.inc';
require_once 'o.AS_HAS_PR.mod.inc';
require_once 'o.ROLE.mod.inc';
require_once ('get_cache.inc');
require_once 'init_cache.inc';
require_once 'f.password.inc';
require_once 'o.CONCRETE_SUBST.proto.inc';
require_once 'o.PROTO.steps.inc';
require_once 'o.PROTO.upsteps.inc';

require_once(dirname(__FILE__).'/../unittest_onetest.inc');
require_once(dirname(__FILE__).'/../test_config.inc');
require_once(dirname(__FILE__).'/subs/o.EXP.create.inc');


/**
 * create missing objects
 * @package create_UT_objects.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   
 */

class create_UT_lib {
    
    function set($tablename, $synonym ) {
        $this->tablename= $tablename;
        $this->synonym  = $synonym;
        
        // $this->defs_of_synonym = $this->object_dict[$this->tablename][$this->synonym];
    }
    
    function _getTestDir_path() {
        $pathx = gUnitTestSub::_getTestDir_path();
        return ($pathx);
    }
    
    function _create_UT_obj_RAW(&$sqlo, &$args, $synonym, $tablename) {
        //global $error;
        //$FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $NAME_col = importantNameGet2($tablename);
        if ($args['vals'][$NAME_col]==NULL) {
            $args['vals'][$NAME_col] = 'UnitTest_'.$synonym;
        }
        $args['vario'] = array('UnitTest_Synonym' => $synonym );
        
        $newlib = new insertC();
        $objid = $newlib->new_meta($sqlo, $tablename, $args);
        
        
        return $objid;
        
    }
    
    /**
     * update object with unittest identifier
     * @param object $sqlo
     * @param int $objid
     */
    function obj_update_UT(&$sqlo, $objid) {
        
        $varioLib = new oS_VARIO_mod();
        $varioOpt = array('noAccCheck'=>1);
        $varioLib->setObject( $sqlo, $this->tablename, $objid,$varioOpt );
        
        $key = 'UnitTest_Synonym';
        $varioLib->updateKeyVal($sqlo, $key, $this->synonym );
        
    }
    
    static function infoout($text) {
        echo 'Info: '.$text."<br>\n";
    }
}

class oROLE_create_UT extends create_UT_lib {
    function create(&$sqlo) {
        
        switch ($this->synonym) {
            
            case 'login_norm':
                
                $role_name = 'UnitTest_login_norm';
                $db_role_id = glob_elementDataGet( $sqlo, 'ROLE', 'NAME', $role_name, 'ROLE_ID');
                if (!$db_role_id) {
                    $args = array('vals'=>array(
                        'NAME'      => $role_name,
                    ));
                    $db_role_id = $this->_create_UT_obj_RAW($sqlo, $args, $this->synonym, 'ROLE');
                }
                // check rigths
                
                
                $sqlsel = 'USER_RIGHT_ID from RIGHT_IN_ROLE where ROLE_ID='.$db_role_id. ' and USER_RIGHT_ID in '.
                    "(select  USER_RIGHT_ID from USER_RIGHT where NAME='f.g.login.NORM' and CCT_RIGHT='execute')";
                $sqlo->Quesel($sqlsel);
                $sqlo->ReadRow();
                $retid = $sqlo->RowData[0];
                if (!$retid) {
                    
                    $rightid = glob_elementDataGet2( $sqlo, 'USER_RIGHT', array('NAME'=>'f.g.login.NORM', 'CCT_RIGHT'=>'execute'), 'USER_RIGHT_ID');
                    $assoclib = new  fAssocUpdate();
                    $assoclib->setObj( $sqlo, 'RIGHT_IN_ROLE', $db_role_id );
                    $argu = array('ROLE_ID'=>$db_role_id, 'USER_RIGHT_ID'=>$rightid );
                    $assoclib->insert( $sqlo, $argu );
                }
                
               
                $obj_id = $db_role_id;
                break;
        }
        return $obj_id;
    }
    
}

/**
 * one object
 * 
 * @example to create a sub object:
 *  $one_obj_lib = new create_UT_oneObj($sqlo);
    $one_obj_lib->set('ABSTRACT_PROTO', 'default' );
    $pra_id = $one_obj_lib->ana_crea($sqlo);
 */
class create_UT_oneObj {
    
    private $defs_of_synonym;
    private $unittest_lib;
    
    function __construct($sqlo) {
        
        $this->unittest_lib = new gUnitTestSub();
        $config_unittest = array(); // can be empty
        
        $this->create_UT_obj = new create_UT_lib();
        
        $UT_conf_lib = new unittest_config();
        $this->object_dict = $UT_conf_lib->get_objects();
        
        $SQL_DUMMY= NULL;
        $this->unittest_lib->initClass($sqlo, $SQL_DUMMY, $config_unittest);
    }
    
    private static function cache_refresh() {
        global $error;
        $CacheLib = new gInitCache();
        $CacheLib->init_cache( $_SESSION['sec']['dbuser'], $_SESSION['sec']['passwd'], $_SESSION['sec']['db'], $_SESSION['sec']['_dbtype']);
        if ($error->Got(READONLY))  {
            $error->reset();
        }
    }
    
    function set_mother_proj($proj_id) {
        $this->proj_id = $proj_id;
    }
    
    function set($tablename, $synonym ) {
        $this->tablename= $tablename;
        $this->synonym  = $synonym;
        
        $this->create_UT_obj->set($tablename, $synonym );
       
        
        $this->defs_of_synonym = $this->object_dict[$this->tablename][$this->synonym];
    }
    
    function _create_UT_obj_RAW(&$sqlo, &$args) {
        return $this->create_UT_obj->_create_UT_obj_RAW($sqlo, $args, $this->synonym, $this->tablename );
    }
    
    /**
     * update object with unittest identifier
     * @param object $sqlo
     * @param int $objid
     */
    private function obj_update_UT(&$sqlo, $objid) {
        return $this->create_UT_obj->obj_update_UT($sqlo, $objid);
    }
    
    
    private function _crea_ABSTRACT_PROTO_ass($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!file_exists($_SESSION['s_sessVars']['AppLabLibDir'].'/lablib/bASSAY/assay_mod.inc')) {
            return 0;
        }
        
        require_once($_SESSION['s_sessVars']['AppLabLibDir'].'/lablib/bASSAY/assay_mod.inc');
        
        $pathx = $this->create_UT_obj->_getTestDir_path();
        $test_data_dir = $pathx;
        $json_file = $test_data_dir.'/www/lab_blk/api/mods/test01.assay.json';
        if (!file_exists($json_file)) {
            throw new Exception('Test-file "'.$json_file.'" not exists.');
        }

        
        $assay_content = file_get_contents($json_file);
        $assay_dict    = json_decode($assay_content, TRUE);
        
        
        //
        // modify 'id' of ASSAY
        //
        $new_assay_id = 'UT_' . rand(10000, 10000000);
        $assay_dict['id'] = $new_assay_id;
        // $new_assay_content = json_encode($assay_dict);
        

        $assay_lib = new \bASSAY\extassay_mod($new_assay_id);
        $exist_arr = $assay_lib->assay_exists($sqlo);
        if ($exist_arr['cnt']!=0) {
            $pra_id = $exist_arr['pra_id'];
            
        } else {
            $json_str='';
            $data_type='dict';
            $pra_id = $assay_lib->create_proto($sqlo, $assay_dict, $json_str, $data_type);
            if ($error->Got(READONLY))  {
                $error->set( $FUNCNAME, 1, 'Error on creation of proto for assay_id:'.$new_assay_id );
                return;
            }
            
        }
        
        $this->obj_update_UT($sqlo, $pra_id);
        
        return $pra_id;
    }
    
    private function _crea_ABSTRACT_SUBST(&$sqlo) {
        global $error, $varcol;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $varcol = & Varcols::get();
        
        switch ($this->synonym) {
            
            case 'default':
                
                $args = array(
                );
                $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                break;
                
            case "qcsteps":
                
                $args = array();
                $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);

                $one_obj_lib = new create_UT_oneObj($sqlo);
                $one_obj_lib->set('ABSTRACT_PROTO', 'default' );
                $pra_id_PREP = $one_obj_lib->ana_crea($sqlo);
                
                $one_obj_lib = new create_UT_oneObj($sqlo);
                $one_obj_lib->set('ABSTRACT_PROTO', 'QC' );
                $pra_id_qc = $one_obj_lib->ana_crea($sqlo);
                
                $sua_pra_lib = new oAS_HAS_PR_mod($sqlo, $obj_id);
                
                $params=array(
                    'STEP_ID'=>5,
                    'ABSTRACT_PROTO_ID'=>$pra_id_PREP,
                    'XORDER'=>5,
                    'TYPE'=>1
                );
                $sua_pra_lib->add_protocol($sqlo, $params);
                
                $params=array(
                    'STEP_ID'=>10,
                    'ABSTRACT_PROTO_ID'=>$pra_id_qc,
                    'XORDER'=>10,
                    'TYPE'=>2
                );
                
                $sua_pra_lib->add_protocol($sqlo, $params);
                
                break;
                
                
                
            case 'with_Class':
                
               
                $class_name='UT_class';
                $class_id= 0;
                $class_fea = glob_elemDataGet3( $sqlo, 'EXTRA_CLASS', array('NAME'=>$class_name), array('*') );
                if (!empty($class_fea)) {
                    $class_id=$class_fea['EXTRA_CLASS_ID'];
                }
                //$class_id	= $varcol->class_name_to_id( 'ABSTRACT_SUBST', $class_name );
                
                create_UT_lib::infoout('analyze class "'.$class_name.'" ID:'.$class_id);
                
                if (!$class_id) {
                    $args = array(
                        'NAME'=>$class_name,
                        'NICE_NAME'=>$class_name,
                        'TABLE_NAME'=>'ABSTRACT_SUBST'
                    );
                    $class_id = insert_row($sqlo, 'EXTRA_CLASS', $args);
                    if ($error->Got(READONLY))  {
                        $error->set( $FUNCNAME, 1, 'EXTRA_CLASS for class:'.$class_name );
                        return;
                    }
                    
                    create_UT_lib::infoout('created class "'.$class_name.'" ID:'.$class_id);
                    
                    $class_fea = glob_elemDataGet3( $sqlo, 'EXTRA_CLASS', array('EXTRA_CLASS_ID'=>$class_id), array('*') );
                    create_UT_lib::infoout('class-features ID:'.$class_id.': '.print_r($class_fea,1));
                    
                    // cache reset
                    get_cache(1, 0);
                    if ($error->Got(CCT_ERROR_READONLY))  {
                        return;
                    }
                    // reset warnings
                    $error->reset();
                    
                    
                }
                
                $attr_names = $varcol->get_attrib_names($class_id);
                if (!sizeof($attr_names)) {
                    
                    // add attribute
                    $args = array(
                        'NAME'    => 'val1',
                        'NICE_NAME' => 'Value 1',
                        'EXTRA_CLASS_ID' => $class_id,
                        'POS' => 1,
                        'APP_DATA_TYPE_ID' => 5,
                        'MAP_COL' => 'N01',
                        'VISIBLE2'=>1,
                        'EDITABLE' => 1,
                        
                    );
                    $attribid = insert_row($sqlo, 'EXTRA_ATTRIB', $args);
                    if ($error->Got(READONLY))  {
                        $error->set( $FUNCNAME, 2, 'EXTRA_ATTRIB for class:'.$class_id );
                        return;
                    }
                }
                
                self::cache_refresh();
                
                $args = array(
                    'vals'=>array('NOTES'=>'UT'),
                    "xobj" => array ('extra_class_id' => $class_id)
                );
                $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                if ($error->Got(READONLY))  {
                    $error->set( $FUNCNAME, 3, 'ABSTRACT_SUBST with class:'.$class_name );
                    return;
                }
                
                
                break;
                
            case 'class_ASR':
                
                
                $class_name='ASR';
                $class_id	= $varcol->class_name_to_id( 'ABSTRACT_SUBST', $class_name );
                if ($error->Got(READONLY))  {
                    
                    return;
                }
                
                
                $args = array(
                    'vals'=>array('NOTES'=>'UT'),
                    "xobj" => array ('extra_class_id' => $class_id)
                );
                $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                
                
                //$obj_id = 131; // TBD: create this object
                
                break;
            case 'class_BLDK':
                
                $class_name='BLDK';
                $class_id	= $varcol->class_name_to_id( 'ABSTRACT_SUBST', $class_name );
                if ($error->Got(READONLY))  {
                    return;
                }
                $args = array(
                    'vals'=>array('NOTES'=>'UT'),
                    "xobj" => array ('extra_class_id' => $class_id)
                );
                $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                break;
                
        }
        return $obj_id;
    }
    
    private function _crea_CONCRETE_SUBST(&$sqlo) {
        global $error, $varcol;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $varcol = & Varcols::get();
        
        switch ($this->synonym) {
            
            case 'default': # simple, no protocol
                $one_obj_lib = new create_UT_oneObj($sqlo);
                $one_obj_lib->set('ABSTRACT_SUBST', 'default' );
                $sua_id = $one_obj_lib->ana_crea($sqlo);
                
                if (!$sua_id) return;
                
                $args = array(
                    'vals'=>array(
                        'ABSTRACT_SUBST_ID'=> $sua_id,
                    )
                );
                $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                break;
                
            case 'UT1':
                $one_obj_lib = new create_UT_oneObj($sqlo);
                $one_obj_lib->set('ABSTRACT_SUBST', 'default' );
                $sua_id = $one_obj_lib->ana_crea($sqlo);
                
                if (!$sua_id) return;
                
                $args = array(
                    'vals'=>array(
                        'ABSTRACT_SUBST_ID'=> $sua_id,
                        'NAME' => 'UnitTest_UT1'
                    )
                );
                $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                break;
            case 'UT2':
                $one_obj_lib = new create_UT_oneObj($sqlo);
                $one_obj_lib->set('ABSTRACT_SUBST', 'default' );
                $sua_id = $one_obj_lib->ana_crea($sqlo);
                
                if (!$sua_id) return;
                
                $args = array(
                    'vals'=>array(
                        'ABSTRACT_SUBST_ID'=> $sua_id,
                        'NAME' => 'UnitTest_UT2'
                    )
                );
                $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                break;
                
            case 'proto':
                
                $one_obj_lib = new create_UT_oneObj($sqlo);
                $one_obj_lib->set('ABSTRACT_SUBST', 'qcsteps' );
                $sua_id = $one_obj_lib->ana_crea($sqlo);
                if (!$sua_id) return;
                
                $one_obj_lib = new create_UT_oneObj($sqlo);
                $one_obj_lib->set('CONCRETE_SUBST', 'default' );
                $suc_id_SIMPLE = $one_obj_lib->ana_crea($sqlo);
                if (!$suc_id_SIMPLE) return;

                
                $args = array(
                    'vals'=>array(
                        'ABSTRACT_SUBST_ID'=> $sua_id,
                        'NAME' => 'UnitTest_UTproto'
                    )
                );
                $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                
                $suc_lib = new oCONCRETE_SUBST_proto($obj_id);
                $ca_logs = $suc_lib->get_ca_protos($sqlo);
                
                foreach($ca_logs as $row) {
                    $features=array();
                    
                    $prc_id = $suc_lib->create_proto($sqlo, $features, $row['ap'], $row['st'], $row['ty']);
                    
                    $pra_steps = gProtoOrg::get_pra_all_steps($sqlo, $row['ap']);
                    $step_params = array();
                    $cnt=0;
                    foreach($pra_steps as $st_row) {

                        $step_loop = $st_row['STEP_NR'];
                        $step_params[$step_loop] =array('QUANTITY'=>$step_loop+0.1);
                        if (!$cnt and $row['ty']==1) {
                            $step_params[$step_loop]['CONCRETE_SUBST_ID'] = $suc_id_SIMPLE;
                        }
                        $cnt++;
                    }
                    
                    $proto_mod_lib = new oConcProtoUpSteps();
                    $proto_mod_lib->setProto($prc_id);
                    $proto_mod_lib->protoUpdate($sqlo, $row['ap'], $step_params);
                }
                break;
        }
        return $obj_id;
    }
    
    function _create_object($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        global $varcol;
        
        $tablename = $this->tablename;
        $obj_id = 0;
        
        switch ($this->tablename) {
            
            case 'A_CHIP_READER':
                switch ($this->synonym) {
                    case 'default':
                        $args = array(
                            'vals'=>array(
                                'NAME' => 'UT_object'   
                            )
                        );
                        $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                        break;
                }
                break;
            
            case 'ABS_SOCKET':
                switch ($this->synonym) {
                    
                    case 'default':
                        // create a compact cartridge
                        require_once ("o.ABS_SOCKET.mod.inc");
                        
                        $H_SOCK_TYPE_ID	 = glob_elementDataGet( $sqlo, 'H_SOCK_TYPE', 'NAME', '02', 'H_SOCK_TYPE_ID'); 
                        
                        $args = array( 
                            'vals'=>array(
                              'H_SOCK_TYPE_ID' => $H_SOCK_TYPE_ID
                              )
                        );
                        $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                        
                        $abs_socket_id = $obj_id;
                        
                        $one_obj_lib = new create_UT_oneObj($sqlo);
                        $one_obj_lib->set('ABSTRACT_SUBST', 'class_ASR' );
                        $sua_id_ASR = $one_obj_lib->ana_crea($sqlo);
                        if (!$sua_id_ASR) return;
                        
                        /*
                        $one_obj_lib = new create_UT_oneObj($sqlo);
                        $one_obj_lib->set('ABSTRACT_SUBST', 'class_BLDK' );
                        $sua_id_BLDK = $one_obj_lib->ana_crea($sqlo);
                        if (!$sua_id_BLDK) return;
                        */
                        
                        $mod_lib = new oABS_SOCKET_mod();
                        $mod_lib->set_obj($sqlo, $abs_socket_id);
                        $mod_lib->update_one_pos($sqlo, 1, $sua_id_ASR, 'REAG');
                        

                        
                        break;
                }
                break;
            
            case 'ABS_WORKLIST':
                switch ($this->synonym) {
                    
                    case 'default':

                        $args = array(
                           
                        );
                        $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                        
                        $entries = array(
                            array( 'ABS_WORKLIST_ID'=>$obj_id, 'XPOS'=>1, 'TABLE_NAME'=>'EXP', 'TYPE'=>'OBJ'),
                            array( 'ABS_WORKLIST_ID'=>$obj_id, 'XPOS'=>2, 'TYPE'=>'NOTES'),
                            
                        );
                        foreach( $entries as $entry ) {
                            insert_row_s($sqlo, 'ABS_WORKLIST_ENTRY', $entry);
                        }
                        
                        break;
                }
                break;
            
            case 'ABSTRACT_PROTO':
                switch ($this->synonym) {
                    case 'default':

                        $args = array(
                           'vals'=>array('NOTES'=>'UT')
                        );
                        $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                        
                        if (!$obj_id) return;
                        
                        // create STEP
                        $tmplib = new fAssocUpdate();
                        $tmplib->setObj( $sqlo, 'ABSTRACT_PROTO_STEP', $obj_id );
                        
                        $argus = array( 
                            array( 'STEP_NR'=>10, 'MAIN_STEP_NR'=> 1, 'SUB_STEP_NR'=>0, 'NAME'=>'Step 1'  ),
                            array( 'STEP_NR'=>20, 'MAIN_STEP_NR'=> 2, 'SUB_STEP_NR'=>0, 'NAME'=>'Step 2'  ),
                            array( 'STEP_NR'=>30, 'MAIN_STEP_NR'=> 3, 'SUB_STEP_NR'=>0, 'NAME'=>'Step 3'  ),
                            array( 'STEP_NR'=>40, 'MAIN_STEP_NR'=> 4, 'SUB_STEP_NR'=>0, 'NAME'=>'Step 4'  ),
                            array( 'STEP_NR'=>50, 'MAIN_STEP_NR'=> 5, 'SUB_STEP_NR'=>1, 'NAME'=>'Step 5'  ),
                            array( 'STEP_NR'=>60, 'MAIN_STEP_NR'=>6, 'SUB_STEP_NR'=>1, 'NAME'=>'Step 6'  ),
                        );    
                        
                        foreach($argus as $argu) { 
                            $tmplib->insert($sqlo, $argu);
                        }
                        
                        break;
                        
                        
                    case 'QC':
                        
                        $H_PROTO_KIND_ID = glob_elementDataGet( $sqlo, 'H_PROTO_KIND', 'NAME', 'QC test', 'H_PROTO_KIND_ID');
                        $args = array(
                           'vals'=>array(
                                'H_PROTO_KIND_ID'=> $H_PROTO_KIND_ID,
                                'NOTES'=>'QC-Proto'
                               )
                        );
                        $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                        
                        if (!$obj_id) return;
                        
                        // create STEP
                        $tmplib = new fAssocUpdate();
                        $tmplib->setObj( $sqlo, 'ABSTRACT_PROTO_STEP', $obj_id );
                        
                        $argus = array(
                            array( 'STEP_NR'=>10, 'MAIN_STEP_NR'=> 1, 'SUB_STEP_NR'=>0, 'NAME'=>'Step 1'  ),
                            array( 'STEP_NR'=>20, 'MAIN_STEP_NR'=> 2, 'SUB_STEP_NR'=>0, 'NAME'=>'Step 2'  ),
                            array( 'STEP_NR'=>30, 'MAIN_STEP_NR'=> 3, 'SUB_STEP_NR'=>0, 'NAME'=>'Step 3'  ),
                            array( 'STEP_NR'=>40, 'MAIN_STEP_NR'=> 4, 'SUB_STEP_NR'=>0, 'NAME'=>'Step 4'  ),
                            array( 'STEP_NR'=>50, 'MAIN_STEP_NR'=> 5, 'SUB_STEP_NR'=>1, 'NAME'=>'Step 5'  ),
                            array( 'STEP_NR'=>60, 'MAIN_STEP_NR'=> 6, 'SUB_STEP_NR'=>1, 'NAME'=>'STATUS'  ),
                        );
                        
                        foreach($argus as $argu) {
                            $tmplib->insert($sqlo, $argu);
                        }
                        
                        break;
                        
                    case 'assay-file':
                        $obj_id = $this->_crea_ABSTRACT_PROTO_ass($sqlo);
                        break;
                        
                    case  'Assaying':
                        
                        $H_PROTO_KIND_ID = glob_elementDataGet( $sqlo, 'H_PROTO_KIND', 'NAME', 'Assaying', 'H_PROTO_KIND_ID');
                        $args = array(
                          'vals'=>array(
                             'H_PROTO_KIND_ID'=> $H_PROTO_KIND_ID,
                             'NOTES'=>'UT'
                             )
                        );
                        $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                        break;
                    case  'Experiment protocol':
                        
                        $H_PROTO_KIND_ID = glob_elementDataGet( $sqlo, 'H_PROTO_KIND', 'NAME', 'Experiment protocol', 'H_PROTO_KIND_ID');
                        $args = array(
                            'vals'=>array(
                                'H_PROTO_KIND_ID'=> $H_PROTO_KIND_ID,
                                'NOTES'=>'UT'
                            )
                        );
                        $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                        break;
                        
                    case 'Exp_with_samples':
                        
                        $args = array(
                        'vals'=>array('NOTES'=>'UT')
                        );
                        $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                        
                        if (!$obj_id) return;
                        
                        // create STEP
                        $tmplib = new fAssocUpdate();
                        $tmplib->setObj( $sqlo, 'ABSTRACT_PROTO_STEP', $obj_id );
                        
                        $argus = array(
                            array( 'STEP_NR'=>10, 'MAIN_STEP_NR'=> 1, 'SUB_STEP_NR'=>0, 'NAME'=>'Sample 1', 'IS_SAMPLE'=>1 ),
                            array( 'STEP_NR'=>20, 'MAIN_STEP_NR'=> 2, 'SUB_STEP_NR'=>0, 'NAME'=>'Sample 2', 'IS_SAMPLE'=>1 ),
                            array( 'STEP_NR'=>30, 'MAIN_STEP_NR'=> 3, 'SUB_STEP_NR'=>0, 'NAME'=>'Sample 3', 'IS_SAMPLE'=>1 ),
                            array( 'STEP_NR'=>40, 'MAIN_STEP_NR'=> 4, 'SUB_STEP_NR'=>0, 'NAME'=>'Step 4'  ),
                            array( 'STEP_NR'=>50, 'MAIN_STEP_NR'=> 5, 'SUB_STEP_NR'=>1, 'NAME'=>'Step 5'  ),
                            array( 'STEP_NR'=>60, 'MAIN_STEP_NR'=>6, 'SUB_STEP_NR'=>1,   'NAME'=>'Step 6'  ),
                        );
                        
                        foreach($argus as $argu) {
                            $tmplib->insert($sqlo, $argu);
                        }
                        
                        break;
                    default:
                        $error->set( $FUNCNAME, 1, 'No rule for TABLE:'. $this->tablename.' synonym:'.$this->synonym );
                        return;
                }
                break;
                
            case 'ABSTRACT_SUBST':
                $obj_id = $this->_crea_ABSTRACT_SUBST($sqlo);
                break;
                
            case 'CHIP_READER':
                
                $one_obj_lib = new create_UT_oneObj($sqlo);
                $one_obj_lib->set('A_CHIP_READER', 'default' );
                $a_dev_id = $one_obj_lib->ana_crea($sqlo);
                
                switch ($this->synonym) {
                    case 'default':
                        $args = array(
                        'vals'=>array(
                            'NAME' => 'UT_object 1',
                            'A_CHIP_READER_ID' => $a_dev_id
                            )
                        );
                        $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                        break;
                }
                break;
                
            case 'CONCRETE_PROTO':
                switch ($this->synonym) {
                    
                    case 'default':
                        
                        $one_obj_lib = new create_UT_oneObj($sqlo);
                        $one_obj_lib->set('ABSTRACT_PROTO', 'default' );
                        $pra_id = $one_obj_lib->ana_crea($sqlo);
                        
                        if (!$pra_id) return;
                        
                        $args = array(
                            'vals'=>array('ABSTRACT_PROTO_ID'=> $pra_id)
                        );
                        $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                        
                        break;
                        
                    
                    case 'assocUpdate':
                        
                        
                        $one_obj_lib = new create_UT_oneObj($sqlo);
                        $one_obj_lib->set('ABSTRACT_PROTO', 'default' );
                        $pra_id = $one_obj_lib->ana_crea($sqlo);
                        
                        if (!$pra_id) return;
                        
                        $args = array(
                            'vals'=>array('ABSTRACT_PROTO_ID'=> $pra_id)
                        );
                        $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);

                        
                        
                        break;
                    default:
                        $error->set( $FUNCNAME, 1, 'No rule for TABLE:'. $this->tablename.' synonym:'.$this->synonym );
                        return;
                }
                break;
                
            case 'CONCRETE_SUBST':
                $obj_id = $this->_crea_CONCRETE_SUBST($sqlo);
                break;
                
            case 'CONTACT':
                switch ($this->synonym) {
                    
                    case 'default': 
                        $args = array(
                          'vals'=> array(
                               'NAME'=>'UnitTest_default',
                           )
                        );
                        $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                        break;
                }
                break;
                
            case 'DB_USER':
                
                switch ($this->synonym) {
                    
                    case 'default':
                        
                        $args = array( 
                             'vals'=> array(
                                'NICK'=>'UnitTest_default',
                                'PASS_WORD'=>'XXX',
                                'EMAIL'=>'dummy@xxx.com'
                                )   
                        );
                        $obj_id = $this->_create_UT_obj_RAW($sqlo, $args); 
                        break;
                        
                    case 'test':
                        // for several tests
                        
                        $role_name='Automat';
                        $role_id = glob_elementDataGet( $sqlo, 'ROLE', 'NAME', $role_name, 'ROLE_ID');
                        if (!$role_id) {
                            $error->set( $FUNCNAME, 10, 'Role "'.$role_name.'" missing.' );
                            return;
                        }
                        
                        $pw = $this->defs_of_synonym['PASS_WORD'];
                        $cryptPasswd = fPasswordC::hash_pw($pw);
                        $args = array(
                            'vals'=> array(
                            'NICK'=>'test01',
                            'PASS_WORD'=>$cryptPasswd,
                            'EMAIL'=>'dummy@xxx.com'
                            )
                        );
                        $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                        if (!$obj_id) return;
                        
                        $role_lib = new oROLE_mod($sqlo, $role_id);
                        $role_lib->add_user_role($sqlo, $obj_id);
                        
                        break;
                        
                    case  "real_user":
                        $nick = $this->defs_of_synonym['NICK'];
                        if ($nick=='') {
                            $error->set( $FUNCNAME, 20, 'Object-Def: NICK missing.' );
                            return;
                        }
                        $obj_id = glob_elementDataGet( $sqlo, 'DB_USER', 'NICK', $nick, 'DB_USER_ID');
                        if (!$obj_id) {
                            $error->set( $FUNCNAME, 30, 'Object-Def: NICK:"'.$nick.'" not found.' );
                            return;
                        }
                        
                        $this->obj_update_UT($sqlo, $obj_id);
                        break;
                }
                break;
                
            case 'EXP':
                $explib = new oEXP_create_UT();
                $explib->set($this->tablename, $this->synonym);
                $explib->create($sqlo);
                break;
                
            case 'EXP_TMPL':
                
                require_once(dirname(__FILE__).'/subs/o.EXP_TMPL.create.inc');
                $explib = new oEXP_TMPL_create_UT();
                $explib->set($this->tablename, $this->synonym);
                $explib->create($sqlo);
                break;
            case 'IMG':
                switch ($this->synonym) {
                    
                    case 'default':
                        $args = array( );
                        $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                        break;
                }
                break;
                
            case 'LINK':
                switch ($this->synonym) {
                    
                    case 'default':
                        $args = array( );
                        $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                        break;
                }
                break;
                
            case 'PROJ':
                switch ($this->synonym) {
                    
                    case 'default':
                        
                        if (!$this->proj_id) {
                            $error->set( $FUNCNAME, 1, 'Setting missing: proj_id.' );
                            return;
                        }
                        
                        $args = array(
                            'vals'=>array(
                                'PRO_PROJ_ID'=> $this->proj_id,
                                'NOTES' => 'UnitTest project'
                            )
                         );
                        $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                        break;
                        
                    
                }
                break;
                
            case 'ROLE': 
                $explib = new oROLE_create_UT();
                $explib->set($this->tablename, $this->synonym);
                $explib->create($sqlo);
                break;
                
            case 'USER_GROUP':
                switch ($this->synonym) {

                    case 'default':
                        
                        $one_obj_lib = new create_UT_oneObj($sqlo);
                        $one_obj_lib->set('DB_USER', 'default' );
                        $user_id = $one_obj_lib->ana_crea($sqlo);
                        
                        if (!$user_id) return;
                        
                        
                        $args = array( 'vals'=>array( 
                            'NAME'=>'UnitTest_grp01',
                            'DB_USER_ID' => $user_id
                            )
                        );
                        $obj_id = $this->_create_UT_obj_RAW($sqlo, $args);
                        break;
                }
                break;
                
            default:
                $error->set( $FUNCNAME, 1, 'No rule for '. $this->tablename );
                return;
        }
        
        return $obj_id;
    }
    
    private function _post_actions($sqlo, $objid) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $tablename = $this->tablename;
        $obj_id = 0;
        
        switch ($this->tablename) {
            
            case 'IMG':
                switch ($this->synonym) {
                    
                    case 'default':
                        
                       // check attachment
                       $img_path = oIMG_fileC::imgPathFull($objid);
                       if (!file_exists($img_path)) {
                           // add file 
                           $src_file = $this->unittest_lib->_getTestDir_path() .'/example_img.png';
                           if (!file_exists($src_file)) {
                               $error->set( $FUNCNAME, 1, 'Src-file "'.$src_file.'" not found.' );
                               return;
                           }
                           
                           copy($src_file, $img_path);
                           if (!file_exists($img_path)) {
                               $error->set( $FUNCNAME, 1, 'Could not create an image file on IMG:'.$objid.'.' );
                               return;
                           }
                           
                       }
                        
                       break;
                }
                break;
        }
    }
    
    /**
     * analyse and create object
     */
    function ana_crea($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $synonym   = $this->synonym;
        $tablename = $this->tablename;
        
        
        // get the real object ID and check, if object exists
        $objid = $this->unittest_lib->_getDefObj_by_vario_key($sqlo, $tablename, $synonym);
        if (!$objid)  {
            // create
            echo '... create; ';
            $objid = $this->_create_object($sqlo);
            if ($error->Got(READONLY))  {  
                return;
            }
            
            
        } else {
            echo '... exists; ';
        }
        
        // post actions ???
        if ($this->defs_of_synonym['_post_actions']>0) {
            $this->_post_actions($sqlo, $objid);
        }
        
        
        return $objid;
        
    }
}




class create_UT_objects_XPL extends gPlugin {
	
	
	function register() {
		
		$title		= 'create UnitTest objects';
		
		$this->infoarr			     = NULL;
		$this->infoarr['title']      = $title;
		$this->infoarr['form_type']  = 'tool'; 
		$this->infoarr2['onlyAdmin'] = 1;
		
		$this -> infoarr ['locrow'] = array (
		    array ('../_tests/www/test/index.php', 'Unittest home')
		    );
	}
	
	/**
	 * form
	 */
	function form1() {
	    
	    $initarr   = NULL;
	    $initarr["title"]       = "Set Destination Project"; // title of form
	    $initarr["submittitle"] = "Submit"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    
	    $hiddenarr = NULL; // hidden form variables
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
	    
	    $fieldx = array ( // form-field definition
	        "title"   => "Project ID",
	        "name"  => "proj_id",
	        "object"=> "text",
	        "val"    => 0,
	        "notes" => ""
	    );
	    $formobj->fieldOut( $fieldx ); // output the form-field
	    
	    $formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];

		
		
		$UT_conf_lib = new unittest_config();
		$object_dict = $UT_conf_lib->get_objects();
		
		if (!$go or !$parx['proj_id']) {
		    $this->form1();
		    return;
		}
		
		echo 'PROJECT ID:'. $parx['proj_id']."<br>";
		
		$proj_mod_lib = new oProjAddElem($sqlo);
		$proj_mod_lib->setProject($sqlo, $parx['proj_id']);
		if ($error->Got(READONLY))  {
		    $error->set( $FUNCNAME, 1, 'Error on Init.' );
		    return;
		}
		
		$one_obj_lib = new create_UT_oneObj($sqlo);
		$one_obj_lib->set_mother_proj($parx['proj_id']);
		
		
		foreach($object_dict as $tablename => $row) {
		    
		    // is a single PK object ?
		    if ( countPrimaryKeys($tablename)<=1 ) {
		        
		        foreach($row as $key => $objid_ORI) {
		            
		            echo '... analyse '. $tablename.' Key:'. $key." ";
		            $one_obj_lib->set($tablename, $key );
		            $objid = $one_obj_lib->ana_crea($sqlo);

		            
		            if ($error->Got(READONLY))  {
		                $error->set( $FUNCNAME, 1, 'Error on creating object.');
		                break;
		            }
		            
		            
		            
		            if ($objid and  cct_access_has2($tablename) ) {
		                echo '; add ID:'. $objid;
		                $proj_mod_lib->addObj($sqlo, $tablename, $objid);
		                if ($error->Got(READONLY))  {
		                    $error->set( $FUNCNAME, 1, 'Error on adding: '. $tablename.' ID:'. $objid);
		                    return;
		                }
		                
		            }
		            
		            echo "<br>\n";
		            
		        }
		        if ($error->Got(READONLY))  {
		            break;
		        }
		    }
		}
		
		echo "READY.<br>\n";
	}
	
}
