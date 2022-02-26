<?php
/**
 * experiment multireactor functions
 * @namespace core::obj::EXP
 * @package o.EXP.mr.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq P2-SDS-006 DataStructure: WellPlate-Experiment 
 */

require_once ("insertx.inc");
require_once 'f.update.inc';
require_once 'class.filex.inc';

require_once 'o.ABS_SOCKET.subs.inc';
require_once 'o.SOCKET.subs.inc';
require_once 'o.H_PROTO_KIND.subs.inc';
require_once 'o.EXP.proto.inc';
require_once 'o.EXP.proto_mod.inc';
require_once 'o.PROTO.upsteps.inc';

// STATIC methods
class oEXP_mr_STAT {
    
    const MOCHI_TYPE_MO =1;
    const MOCHI_TYPE_CHI=2;
    
    static function get_mochi_type($sqlo, $exp_id) {
        $mochi_type=self::MOCHI_TYPE_MO;
        $exp_feats = glob_elemDataGet3(  $sqlo, 'EXP', array('EXP_ID'=> $exp_id), array('SOCKET_ID', 'EXP_TMPL_ID', 'MO_EXP_ID')  );
        if ($exp_feats['MO_EXP_ID']) {
            // is child
            $mochi_type=self::MOCHI_TYPE_CHI;
        }
        return $mochi_type;
    }
    
    
}

/**
 * experiment multireactor ANALYSIS functions
 */
class oEXP_mr {
    
    private $ch_exps;
    private $exp_feats;
    const SOCK_TYPE_MULTIREACTOR='07';
    
    function __construct(int $exp_id) {
        $this->exp_id=$exp_id;
        $this->ch_exps=array();
    }

    /**
     * cache $this->ch_exps
     * @param object $sqlo
     * @return array
     */
    function get_children($sqlo) {
       $this->ch_exps=array();
       $sqlsel = "EXP_ID, MO_POS from EXP where MO_EXP_ID=".$this->exp_id.' order by MO_POS';
       $sqlo->Quesel($sqlsel);
       while ( $sqlo->ReadRow() ) {
           $exp_id_l = $sqlo->RowData[0];
           $pos = $sqlo->RowData[1];
           $this->ch_exps[]=array($exp_id_l, $pos);
       }
       return $this->ch_exps;
    }
    
    function is_child($sqlo) {
        $is_child=0;
        $MO_EXP_ID = glob_elementDataGet( $sqlo, 'EXP', 'EXP_ID', $this->exp_id, 'MO_EXP_ID');
        if ($MO_EXP_ID) $is_child=1;
        return $is_child;
    }
    
    
   
    /**
     * protocol belongs to SUB-experiments
     * @param object $sqlo
     * @param int $cpid
     * @return array of (exp_ids, MO_POS)
     */
    function proto_belongs_to(object $sqlo, int $cpid) {
        $exp_arr=array();
        $subsel='select EXP_ID from EXP_HAS_PROTO where CONCRETE_PROTO_ID='.$cpid ;
        $sqlsel= 'EXP_ID, MO_POS from EXP where MO_EXP_ID='.$this->exp_id. '  and EXP_ID in ('.$subsel.') order by MO_POS';
        $sqlo->Quesel($sqlsel);
        while ($sqlo->ReadRow()) {
           $exp_arr[]=array($sqlo->RowData[0], $sqlo->RowData[1]);
        }
        return $exp_arr;
    }
    
    /**
     * find an aub-experiment by ending string 
     * @param object $sqlo
     * @param int $cpid
     * @return $exp_id or 0 or -1
     */
    function find_exp_by_NAME_pattern(object $sqlo, string $pattern) {
        $exp_id = 0;
       
        $sqlsel= 'EXP_ID, MO_POS from EXP where MO_EXP_ID='.$this->exp_id. " and NAME like ".$sqlo->addQuotes($pattern);
        $sqlo->Quesel($sqlsel);
        if ($sqlo->ReadRow()) {
            $exp_id = $sqlo->RowData[0];
        }
        if ($sqlo->ReadRow()) {
            $exp_id=-1;  // more than 1 expeirment found ...
        }
        return $exp_id;
    }
    
    /**
     * protocol used by other SUB-experiments than THIS-EXP
     * @param object $sqlo
     * @param int $cpid
     * @return array of (exp_ids, MO_POS)
     */
    function proto_used_by_others(object $sqlo, int $cpid, int $this_exp_id) {
        $exp_arr=array();
        $subsel='select EXP_ID from EXP_HAS_PROTO where CONCRETE_PROTO_ID='.$cpid ;
        $sqlsel= 'EXP_ID, MO_POS from EXP where MO_EXP_ID='.$this->exp_id. 
        '  and EXP_ID in ('.$subsel.') and EXP_ID!='.$this_exp_id.' order by MO_POS';
        $sqlo->Quesel($sqlsel);
        while ($sqlo->ReadRow()) {
            $exp_arr[]=array($sqlo->RowData[0], $sqlo->RowData[1]);
        }
        return $exp_arr;
    }
    
    // tranbsform array(array(exp_id, step_no)) => array(exp_id)
    static function transExpSt2Exp($expStepArr) {
        $out=array();
        foreach($expStepArr as $row) {
            $out[]=$row[0];
        }
        return $out;
    }
    
    /**
     * get protocols of all sub-experiments at $step_nr
     * - uses cache $this->ch_exps
     * @param object $sqlo
     * @param int $step_nr
     * @return number[]
     */
    function subexp_protosByStep($sqlo, int $step_nr) {
        
        if (empty($this->ch_exps)) {
            $this->get_children($sqlo);
        }
        $exp_lib = new oEXPprotoC();
        
        $exp_proto_arr=array();
        foreach($this->ch_exps as $row) {
            $exp_id=$row[0];
            $cpid = $exp_lib->getCProtoByStep($sqlo, $exp_id, $step_nr);
            if (!$cpid) $cpid=0;
            $exp_proto_arr[]=$cpid;
        }
        return $exp_proto_arr;
    }
    
    static function proto_usage_no($sqlo, $cpid) {
        $sqlsel= "count(1) from EXP_HAS_PROTO where CONCRETE_PROTO_ID=".$cpid;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $cp_no = $sqlo->RowData[0];
        return $cp_no;
    }
    
    /**
     * cache result ...
     * @param object $sqlo
     * @return array
     */
    function get_mr_details($sqlo) {
        
        $id = $this->exp_id;
        
        
        $this->exp_feats = glob_elemDataGet3(  $sqlo, 'EXP', array('EXP_ID'=> $id), array('SOCKET_ID', 'EXP_TMPL_ID', 'MO_EXP_ID')  );
        if (!$this->exp_feats['EXP_TMPL_ID']) {
            // throw new Exception('Need an '.tablename_nice2('EXP_TMPL'));
            return $this->exp_feats;
        }
        
        do {
            if ($this->exp_feats['SOCKET_ID']) {
                $this->exp_feats['ABS_SOCKET_ID'] = glob_elementDataGet( $sqlo, 'SOCKET', 'SOCKET_ID', $this->exp_feats['SOCKET_ID'], 'ABS_SOCKET_ID' );
                break;
            }
            $this->exp_feats['ABS_SOCKET_ID'] = glob_elementDataGet( $sqlo, 'EXP_TMPL', 'EXP_TMPL_ID', $this->exp_feats['EXP_TMPL_ID'], 'ABS_SOCKET_ID' );
            break;
            
        } while (0);
        
        if(!$this->exp_feats['ABS_SOCKET_ID']) {
            //throw new Exception('Need a '.tablename_nice2('ABS_SOCKET'));
            return $this->exp_feats;
        }
        
        $sock_type_key = oABS_SOCKET_subs::get_socket_type_key_STAT($sqlo, $this->exp_feats['ABS_SOCKET_ID']);
        $this->exp_feats['SOCKET.type.key'] = $sock_type_key;
        return $this->exp_feats;
    }
    
    /**
     * get details regarding to groups
     * @param object $sqlo
     * @return array
     *    array('grp'=>, 'suc_id'=>)
     */
    function get_socket_grp_details($sqlo) {
        $output=array();
        $this->get_mr_details($sqlo);
        if (!$this->exp_feats['SOCKET_ID']) {
            return $output;
        }
        $sock_lib = new oSOCKET_subs();
        $sock_lib->init($sqlo, $this->exp_feats['SOCKET_ID']);
        $grp_list = $sock_lib->get_unique_GRP_list($sqlo);
        return $grp_list;
    }
    
    /**
     * is this a multireactor MOTHER experiment ?
     * - derived from SOCKET, ABS_SOCKET ...
     * @param object $sqlo
     * @throws Exception
     * @return number 0,1
     */
    function is_multireactor($sqlo) {
        $output = 0;
        
        $exp_feats = $this->get_mr_details($sqlo);
        $sock_type_key = $exp_feats['SOCKET.type.key'];
        if ($sock_type_key==self::SOCK_TYPE_MULTIREACTOR) {
            $output=1;
        }
        
        return $output;
    }
}

class oEXP_mr_mod {
    
    private $ori_args;
    
    function __construct($sqlo, int $exp_id) {
        $this->exp_id=$exp_id;
        
        $ori_args_r = glob_elemDataGet3( $sqlo, 'EXP', array('EXP_ID'=>$exp_id), array('*') );
        
        // remove these columns ...
        // SOCKET_ID : do not reference the original SOCKET_ID !
        $del_cols = array(
            'EXP_ID', 
            'CCT_ACCESS_ID', 
            'EXTRA_OBJ_ID', 
            'SOCKET_ID', 
            'EXT_ID',
            'DATAURL'
            
        );
        
        // debugOut('(112) ori_args_r:'.print_r($this->ori_args_r,1), $FUNCNAME, 2);
        
        $this->ori_args=array();
        foreach($ori_args_r as $col=>$val) {
            if (in_array($col,$del_cols)) continue;
            if ($val===NULL) continue;
            $this->ori_args[$col] = $val;
        }
    }
    
    function sub_exp_exists(object $sqlo, int $pos) {
        $pos_exp_id=0;
        $sqlsel = "EXP_ID from EXP where MO_EXP_ID=".$this->exp_id.' and MO_POS='.$pos;
        $sqlo->Quesel($sqlsel);
        if ( $sqlo->ReadRow() ) {
            $pos_exp_id = $sqlo->RowData[0];
        }
        return $pos_exp_id;
    }
    
    /**
     * add new sub experiment
     * - initial args: for most args: use args of MOTHER !
     * @param object $sqlo
     * @param array $args
     *   'vals'
     * @return int new exp id
     */
    function add_sub_exp(object $sqlo, array $arg_val) {
        if(!$arg_val['MO_POS']) throw new Exception('Input POS missing.');
        
        if ( $this->sub_exp_exists($sqlo, $arg_val['MO_POS']) ) throw new Exception('MO_POS:'.$arg_val['MO_POS'].' already exists.');
        
        // check, if mother is already flagged ...
        $ori_args_r = glob_elemDataGet3( $sqlo, 'EXP', array('EXP_ID'=>$this->exp_id), array('MO_POS') );
        if ($ori_args_r['MO_POS']==='' or $ori_args_r['MO_POS']===NULL) {
            // MO_POS=0 must be set for mother 
            $argu_mo=array('MO_POS'=>0);
            $idarr  =array('EXP_ID'=>$this->exp_id);
            gObjUpdate::update_row_s($sqlo, 'EXP', $argu_mo, $idarr);
        }
        
        $arg_use =  $this->ori_args;
        $arg_use = array_merge($arg_use, $arg_val);
        
        $arg_use['MO_EXP_ID'] = $this->exp_id;
        $insertlib = new insertC();
        $args = array( "vals"=>$arg_use );
        $new_exp_id = $insertlib->new_meta($sqlo, 'EXP', $args);
        return $new_exp_id;
    }
    
    /**
     * add a standard protocol for the sample
     * @param object $sqlo
     * @param int $suc_id
     * @return number
     */
    function add_sample_protocol(object $sqlo, $suc_id) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$suc_id) {
            $error->set( $FUNCNAME, 1, 'Input SUC-ID missing.' );
            return;
        }
        
        // $key='e.STD';
        // $h_id = oH_PROTO_KIND_subs::key2ID($sqlo, $key);
        
        $pra_id  = 921;
        $exp_lib = new oEXP_proto_mod();
        $exp_lib->set_exp($sqlo, $this->exp_id);
        $sqlo_dummy=NULL;
        $prc_id = $exp_lib->create($sqlo, $sqlo_dummy, $pra_id, 10);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 2, 'Error on protocol creation.' );
            return;
        }
        
        // add sample ...
        
        $step_params = array(
            1 => array('CONCRETE_SUBST_ID'=>$suc_id),
        );
        $proto_mod_lib = new oConcProtoUpSteps();
        $proto_mod_lib->setProto($prc_id);
        $proto_mod_lib->protoUpdate($sqlo, $pra_id, $step_params);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 5, 'Error on protocol update.' );
            return;
        }
        
    }
    
}