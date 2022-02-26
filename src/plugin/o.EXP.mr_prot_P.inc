<?php
/**
 * experiment multireactor PROTOCOL editor
 * @package o.EXP.mr_prot_P.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 
 * @param   int $id    [REQUIRED]
 * @param int $step_no [REQUIRED]
 * @param parx
 * @param $action 
 *   ['show']
 *   'update'
 *   'settings'  -- tool settings
 * @params $q, $cs
 */
require_once 'javascript.inc';
require_once ("f.objview.inc");
require_once 'o.PROTO.steps.inc';
require_once 'o.PROTO.subs.inc';
require_once 'o.ABSTRACT_PROTO.stepx.inc';
require_once 'o.EXP.proto.inc';
require_once 'o.EXP.proto_mod.inc';
require_once 'gui/o.CONCRETE_PROTO.edit.inc';
require_once 'gui/o.CONCRETE_PROTO.li_show1.inc';
require_once 'gui/o.PROTO.stepout1.inc';
require_once($_SESSION['s_sessVars']['AppLabLibDir'].'/lablib/gui/o.EXP.plate.inc');


class o_EXP_mr_proto_up_ONE {
    
    static $problems=array();
    
    function __construct(object $sqlo, int $mo_exp_id, int $exp_id, int $step_no, $pra_id, array $inp, array &$pra_step_all ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
  
        $this->mo_id = $mo_exp_id;
        $this->id = $exp_id;
        $this->step_no= $step_no;
        $this->pra_step_all = &$pra_step_all;
        $this->pra_id = $pra_id;
        $this->inp= &$inp;
        
        $this->exp_lib = new oEXP_mr($mo_exp_id);
        
        $exp_proto_lib  = new oEXPprotoC();
        $this->ori_cpid = $exp_proto_lib->getCProtoByStep($sqlo, $exp_id, $step_no);
        
        $this->updateCnt=0;
    }
    
    static function flagProblem($exp_id_l, $step_nr, $text) {
        self::$problems[$exp_id_l][$step_nr]=$text;
    }
    
    /**
     * 
     * @param object $sqlo
     * @param int $exp_id_l
     * @param array $pra_step_all
     * @param array $inp
     * @return array
     *   [STEP] = array("quantity"=>, "subst"=>)
     */
    static function inp2st_param($sqlo, $exp_id_l, &$pra_step_all, &$inp) {
        
        // $exp_id_l = $this->id;
        
        $errcnt=0;
        $up_params=array();
        
        foreach($pra_step_all as $row) {
            
            $step_nr=$row['STEP_NR'];
            
            $new=array();
            
            $tmp_q = trim($inp['q'][$exp_id_l][$step_nr]);
            if ($tmp_q==='') $tmp_q=NULL; // force value = NULL not ""
            $new["quantity"] = $tmp_q;
            
            $tmp_suc = trim($inp['cs'][$exp_id_l][$step_nr]);
            if(!$tmp_suc) $tmp_suc=NULL; // force value = NULL not ""
            $new["subst"]    = $tmp_suc;
           
            
            do {
                if ($new["quantity"]!="") {
                    if (!is_numeric($new["quantity"])) {
                        self::flagProblem($exp_id_l, $step_nr, 'Not a number');
                        $errcnt++;
                        break;
                    }
                }
                
                // check each substance if exists in DB
                if ($new["subst"]!=NULL) {
                    if (!is_numeric($new["subst"])) {
                        self::flagProblem($exp_id_l, $step_nr, 'NaN');
                        $errcnt++;
                        break;
                    }
                    if ( !gObject_exists ($sqlo, 'CONCRETE_SUBST', $new["subst"]) ) {
                        self::flagProblem($step_nr, 'MAC not exists.');
                        $errcnt++;
                        break;
                    }
                }
                
                $up_params[$step_nr] = $new;
                
                
            } while (0);
        }
        return $up_params;
    }
    
   
    
    private function proto_update(&$sqlo, &$up_params) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;

        $conc_proto_id = $this->cpid;
        
        if (!$conc_proto_id) throw new Exception('Input cpid missing.');
        
        $a_proto_id  = $this->pra_id;
        
        $protoManiLib = new oCprotoMani();
        $protoManiLib->setCprotoID($sqlo,  $conc_proto_id, $a_proto_id);
        
        foreach($up_params as $row) {
            $step_nr=$row['st'];
            $protoManiLib->oneStepSave( $sqlo, $row['params'], $step_nr );
            if ($error->Got(READONLY))  {
                $error->set( $FUNCNAME, 1, 'Error on step:'.$step_nr );
                return;
            }
            $this->updateCnt++;
        }
        
        $protoManiLib->close($sqlo);
        
    }
    
    
}

/**
 * protocol bulk update ...
 * intelligent: create new protcols, if shared protocols are different now
 *
 */
class o_EXP_mr_proto_bulkup {
    
    /**
     * 
     * @var array
     *  'q'
     *  'cs'
     */
    private $inp;
    private $proto_ext_arr;
    
    function __construct(object $sqlo, int $id, int $step_no ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
      
        $this->id = $id;
        $this->step_no= $step_no;
        $this->infox=array();
        $this->_log=array();
        $this->updateCnt=0;
        $this->find_create_cnt+0;
        
        $this->exp_mr_lib = new oEXP_mr($this->id);
        $exp_arr_raw  = $this->exp_mr_lib->get_children($sqlo);
        $this->exp_arr_slim = oEXP_mr::transExpSt2Exp($exp_arr_raw);
        
        $proto_arr_raw = $this->exp_mr_lib->subexp_protosByStep($sqlo, $step_no);
        
        $pra_arr=array();
        foreach($proto_arr_raw as $cpid) {
            if ($cpid) {
                $pra_id = glob_elementDataGet( $sqlo, 'CONCRETE_PROTO', 'CONCRETE_PROTO_ID', $cpid, 'ABSTRACT_PROTO_ID');
                $pra_arr[$pra_id]=1;
            }
        }
        if (empty($pra_arr)) {
            $error->set( $FUNCNAME, 1, 'No protocol found in experiments.' );
            return;
        }
        $this->pra_id = key($pra_arr);
        
        $this->pra_step_all=array();
      
        $pra_lib = new oABSTRACT_PROTO_stepx($pra_id);
        $this->pra_steps = $pra_lib->get_steps($sqlo);
        
        foreach($this->pra_steps as $step_nr) {
            $one_st_inf = $pra_lib->step_info($sqlo, $step_nr);
            $this->pra_step_all[]=$one_st_inf;
        }

        $this->proto_ext_arr=array();
        
        
        foreach($proto_arr_raw as $cpid) {
            $loop_cp_data=NULL;
            if ($cpid) {
                $loop_cp_data = $this->_get_proto_data($sqlo, $cpid);
            }
            $this->proto_ext_arr[]=array('cpid'=>$cpid, 'data'=>$loop_cp_data );
        }
        
        
    }
    
    /**
     * 
     * @param object $sqlo
     * @param int $cpid
     * @param array $pra_steps
     * @return array 
     *    array('st'=>$step_nr, 'params'=> array("quantity"=>, "subst"=>))
     */
    private function _get_proto_data($sqlo, int $cpid) {
        
        $proto_data=array();
        
        // prefill, if protcol is still empty ...
        foreach($this->pra_steps as $step_nr) {
            $proto_data[$step_nr]= array( 'quantity'=>NULL, 'subst'=>NULL);
        }
        
        $sqlsel = "* from CONCRETE_PROTO_STEP where CONCRETE_PROTO_ID=".$cpid." ORDER by STEP_NR";
        $sqlo->Quesel($sqlsel);
        
        while ( $sqlo->ReadArray() ) {
            $step_nr= $sqlo->RowData['STEP_NR'];
            $suc    = $sqlo->RowData['CONCRETE_SUBST_ID'];
            $q      = $sqlo->RowData['QUANTITY'];
            $proto_data[$step_nr] = array( 'quantity'=>$q, 'subst'=>$suc);
        }
        return $proto_data;
    }
    
    private function add_log($text) {
        $this->_log[]=$text;
    }
    
    private function proto_update(&$sqlo, int $conc_proto_id, array &$up_params) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;

        
        if (!$conc_proto_id) throw new Exception('Input cpid missing.');
        
        $a_proto_id  = $this->pra_id;
        
        $protoManiLib = new oCprotoMani();
        $protoManiLib->setCprotoID($sqlo,  $conc_proto_id, $a_proto_id);
        
        foreach($up_params as $step_nr=>$param_l) {
            
            $protoManiLib->oneStepSave( $sqlo, $param_l, $step_nr );
            if ($error->Got(READONLY))  {
                $error->set( $FUNCNAME, 1, 'Error on step:'.$step_nr );
                return;
            }
            // $this->updateCnt++;
        }
        
        $protoManiLib->close($sqlo);
        
    }
    
    /**
     * create new protocol and attach to experiment
     * @param object $sqlo
     * @param object $sqlo2
     * @param int $exp_id
     * @param int $ori_cpid
     * @return number
     */
    private function crea_find_new_proto($sqlo, $sqlo2, $exp_id, $ori_cpid, &$this_params, $this_index) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $proto_lib = new oEXP_proto_mod();
        $proto_lib->set_exp($sqlo, $exp_id);
        
        
        $this_params_serial = serialize($this_params);
        
        // echo "DDDX: ORI-data: ".print_r($this_params,1)." SERI:$this_params_serial <br>";
        
        do {

            // ... search for compatiple protocol ...
            $found=0;
            $i=0;

            foreach($this->proto_ext_arr as $row) {
                
                if ($i==$this_index) {
                    // this object, continue ...
                } else {
                    $data_loop   = $row['data'];
                    $loop_serial = serialize($data_loop);
                    
                    if ($this_params_serial==$loop_serial) {
                        // found one
                        $use_cpid = $row['cpid'];
                        $proto_lib->unlink_prot($sqlo, $this->step_no); // remove old link ...
                        $proto_lib->add_existing($sqlo, $use_cpid, $this->step_no); // new link
                        $found = 1;
                        $this->add_log('---  NEW_CP: FOUND_OTHER_existing: CPID:'.$use_cpid.'.');

                        break;
                    } else {
                        // echo "--- DDDX: ($i): data:  ".print_r($data_loop,1)." SERI:$loop_serial<br>"; 
                    }
                }
                $i++;
            }
            
            if ($found) {
                break;
            }
            
            // create a real new CPID
      
            $proto_lib->unlink_prot($sqlo, $this->step_no); // remove old link ...
            $creopt = array("checkStepNo"=>1);
            
            $use_cpid = $proto_lib->create($sqlo, $sqlo2, $this->pra_id, $this->step_no, array(), $ori_cpid, $creopt);
            if ($error->Got(READONLY))  {
                $error->set( $FUNCNAME, 1, 'Error on proto create for exp:'.$exp_id );
                return;
            }
            $this->proto_update($sqlo, $use_cpid, $this_params);
            $this->add_log('---  CREATE: CPID_NEW:'.$use_cpid.'.');
            
        } while (0);
        
        $this->find_create_cnt++;
        
        return $use_cpid;
    }
    
    private function params_have_data(&$this_params) {
        $has_data=0;
        foreach($this_params as $row) {
            foreach($row as $val) {
                if ( !($val==='' or $val===NULL) ) {
                    $has_data=1;
                    break;
                }
            }
        }
        return $has_data;
    }
    
    /**
     * analyse experiment protocols, if there are SHARED protocols
     * @param object $sqlo
     * @param object $sqlo2
     */
    function analyse_for_diff($sqlo, $sqlo2)  {

        $i=0;
        foreach($this->exp_arr_slim as $exp_id_l) {
            
            $cpid_l      = $this->proto_ext_arr[$i]['cpid'];
            $this_params = $this->all_params[$i];
            $use_cpid    = $cpid_l;
            
            $this->add_log('EXP '.$exp_id_l.': params:'.print_r($this_params,1) );
            
            do {
                
                $cp_created = 0;
            
                if (!$cpid_l) {
                    
                    $this->add_log('--- no CPID.');
                    if ( !$this->params_have_data($this_params) ) {
                        // no data: do not create a protocol ...
                        $this->add_log('--- no data: do nothing.');
                        break;
                    }
                    $new_cpid = $this->crea_find_new_proto($sqlo, $sqlo2, $exp_id_l, 0, $this_params, $i);
                    $this->proto_ext_arr[$i]['cpid'] = $new_cpid; // update CPID array ...
                    $use_cpid = $new_cpid;
                    $cp_created = 1;
                    
                    $this->add_log('--- NEW_CPID: got initial protocol');
                }
                
                $exps_used = $this->exp_mr_lib->proto_used_by_others($sqlo, $cpid_l, $exp_id_l); // other experiments ...
                if (empty($exps_used)) {
                    
                    // ok is NOT_SHARED
                    $this->add_log('--- OK: has no dependencies.');
                    break;
                    
                } 
                    
                $exps_used_slim = oEXP_mr::transExpSt2Exp($exps_used);
                $now_serial  = serialize($this_params);

                foreach($this->all_params as $sub_i=>$params_loop) {
                    
                    if ($sub_i==$i) continue; // current experiment ..
                    $sub_loop_exp = $this->exp_arr_slim[$sub_i];
                    
                    if (!in_array($sub_loop_exp, $exps_used_slim)) {
                        continue; // not in this used array ...
                    }
                    
                    $serial_loop = serialize($params_loop);
                    if ($now_serial==$serial_loop) {
                        // $this->add_log('---  OTHER-EXP:'.$sub_loop_exp.' P:'.print_r($params_loop,1).' :OK: the same params.');
                    } else {
                        
                        // diff found ...
                        // create new cpid
                        $new_cpid = $this->crea_find_new_proto($sqlo, $sqlo2, $exp_id_l, $cpid_l, $this_params, $i);
                        $this->proto_ext_arr[$i]['cpid'] = $new_cpid; // update CPID array ...
                        $use_cpid = $new_cpid;
                        $cp_created = 1;
                        $this->add_log('---  NEW_CPID_NEED:'.$sub_loop_exp.' P:'.print_r($params_loop,1).' : new_CPID:'.$new_cpid.'.');
                        
                        break; // ready
                    }
                }
     
                
            } while (0);
            
            if ($use_cpid) { // there are still empty CPIDs possible
                if (!$cp_created) {
                    $this->add_log('---  OK: no CPID-Change, only update-params.');
                    $this->proto_update($sqlo, $use_cpid, $this_params);
                }
                
                // update protcol data cache
                $new_cp_data = $this->_get_proto_data($sqlo, $use_cpid);
                $this->proto_ext_arr[$i]['data'] = $new_cp_data;
            }
            
            $i++;
        }
    }
   
    
    /**
     * TBD: handle input errors !!!
     * @param object $sqlo
     */
    function update_all(&$sqlo, $sqlo2) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->inp=array();
        $this->inp['q']  = $_REQUEST['q'];
        $this->inp['cs'] = $_REQUEST['cs'];
        
        
        $this->all_params=array();
        $i=0; 
        foreach($this->exp_arr_slim as $exp_id_l) {

            $cpid_l   = $this->proto_ext_arr[$i]['cpid'];
            $up_params = o_EXP_mr_proto_up_ONE::inp2st_param($sqlo, $exp_id_l, $this->pra_step_all, $this->inp);

            // echo "(384)DDX: exp_id_l:$exp_id_l cpid_l:$cpid_l  P:".print_r($up_params,1)."<br>";
            
            $this->all_params[] = $up_params;
            $i++;
        }
        
        // analyse protocols ...
        // create UNIQUE protocols here ???
        $this->analyse_for_diff($sqlo, $sqlo2);
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
            echo 'DEBUG: UPDATE-LOG:<ul>';
            foreach($this->_log as $text_l) {
                echo '- '.$text_l.'<br>';
            }
            echo '</ul><hr>'."\n";
        }
       
       
    }
    
}





class o_EXP_mr_prot_P_XPL extends gPlugin {
    
    
    function register() {
        $tablename	= 'EXP';
        $title		= 'Multi-Protocol Editor';
        
        $this->infoarr			 = NULL;
        $this->infoarr['title']    = $title;
        $this->infoarr['form_type']= 'obj';
        $this->infoarr['obj_name'] = $tablename;
        $this->infoarr['obj_id']   = $_REQUEST['id'];
        $this->infoarr['checkid']  = 1;
        $this->infoarr['locrow'] = array (
            array ('p.php?mod=DEF/o.EXP.mr_mod_P&id='.$_REQUEST['id'], 'multireactor Editor') ,
            
        );
        
        $this->infoarr2 = array(
            'accCheck' => array('tab'=>array('write'), 'obj'=>array('read') )
        );
    }
    
    
    function startMain() {
        global $error;
        $FUNCNAME='startMain';
        
        $tablename='EXP';
        $sqlo = &$this->_sqlo;
        $sqlo2= &$this->_plug_regSql2();
        
        js_formAll();
        
        $this->objid   = $_REQUEST['id'];
        $go     = $_REQUEST['go'];
        $parx   = $_REQUEST['parx'];
        $step_no= $_REQUEST['step_no'];
        $action = $_REQUEST['action'];
        
        $o_rights = access_check($sqlo, $tablename, $this->objid);
        
        if(!$step_no) {
            $this->htmlFoot('ERROR', 'Input step_no missing.');
        }
        
        if ($_REQUEST['action']=='settings') {
            if (!$go) {
                $hiddenarr=array( 'id'=>$this->objid, 'mod'=>$this->_mod );
                $tmp = o_CONCRETE_PROTO_li_show1::settings_get();
                o_CONCRETE_PROTO_li_show1::form_settings($tmp, $step_no, $hiddenarr);
                return;
            } else {
                o_CONCRETE_PROTO_li_show1::settings_save($parx);
                $go = 0;
            }
        }
        $tool_opts = o_CONCRETE_PROTO_li_show1::settings_get();
       
        
       
        $edit_flag = 0;
        if ($action=='update') {
            
            if ( !$o_rights["write"]) {
                $error->set( $FUNCNAME, 1, 'You have no "write" right on this object.' );
                return;
            }
            $edit_flag=1;
        }
        
       
        
        $form_url=$this->_plug_get_URL() . '&id='.$this->objid.'&step_no='.$step_no;
        
        debugOut('parx:'.print_r($parx,1), $FUNCNAME, 2);

        
        if ($go and $action=='update') {
            
            if ( !$o_rights["write"]) {
                $error->set( $FUNCNAME, 1, 'You have no "write" right on this object.' );
                return;
            }
            
            echo '... update.<br>';
            echo '... CURENTLY not supported STOP.<br>';
            $DO_UP=0;
            if ($DO_UP) {
                $prot_bulkup_lib = new o_EXP_mr_proto_bulkup($sqlo, $this->objid , $step_no);
                $prot_bulkup_lib->update_all($sqlo, $sqlo2);
                if ($error->Got(READONLY))  {
                    $error->set( $FUNCNAME, 1, 'Errors' );
                    return;
                }
            }
            
        }
        
       
        $tool_opts['form_url'] = $form_url;
        
        $exp_mr_lib = new oEXP_mr( $this->objid);
        $exp_arr_raw   = $exp_mr_lib->get_children($sqlo);
        $proto_arr = $exp_mr_lib->subexp_protosByStep($sqlo, $step_no);
        
        $exp_arr =array();
        foreach($exp_arr_raw as $row) {
            $exp_arr[] = $row[0];
        }
        
        $show_objects = array('mo_arr'=>&$exp_arr,  'prc_arr'=>&$proto_arr, 'mo_table'=>'EXP');
        
        $prot_edit_lib = new o_CONCRETE_PROTO_li_show1($sqlo, $show_objects, $tool_opts);
        $prot_edit_lib->form($sqlo, $edit_flag);
        
    }
}

