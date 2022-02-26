<?php
require_once ("date_funcs.inc");
require_once 'insertx.inc';
require_once ("o.CS_HAS_PR.subs.inc");
require_once ("o.AS_HAS_PR.subs.inc");
require_once 'o.PROTO.upsteps.inc';
require_once 'o.ACCEPT_PROT.subs.inc';


/**
 * sub functions for concrete subst,  protocol modification/creation things
 * @package o.CONCRETE_SUBST.proto.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @unittest exists
 */
class oCONCRETE_SUBST_proto {

    
    function __construct($suc_id=0) {
        $this->AS_HAS_PR_lib   = new oAS_HAS_PR_subs();
        $this->CS_HAS_PR_lib   = new oCS_HAS_PR_subs();
        
        $this->suc_id = $suc_id;
        $this->CS_HAS_PR_lib->setObj($suc_id);
    }
    
    function set_obj($suc_id) {
        $this->suc_id = $suc_id;
        $this->CS_HAS_PR_lib->setObj($suc_id);
    }
    

    /**
     * get protocol log
     * @param object $sqlo
     * @throws Exception
     * @return array $proto_log : see  o.CS_HAS_PR.subs.inc:cprotoLog_STRUCT
     */
    function get_c_protos(&$sqlo) {
        
        if ( !$this->suc_id ) {
            throw new Exception('Class not inizialized.');
        }
        
        $proto_log = $this->CS_HAS_PR_lib->getProtoLog($sqlo);
        return $proto_log;
    }
    
    /**
     * get protocol log of SUC and SUA merged !
     * @param object $sqlo
     * @return array $proto_log : see  o.CS_HAS_PR.subs.inc:cprotoLog_STRUCT
     */
    function get_ca_protos($sqlo) {
        
        $sua_id = glob_elementDataGet( $sqlo, 'CONCRETE_SUBST', 'CONCRETE_SUBST_ID', $this->suc_id,  'ABSTRACT_SUBST_ID');
        $this->AS_HAS_PR_lib->setObj($sua_id);
        
        $cp_array = $this->CS_HAS_PR_lib->getProtoLog($sqlo);
        $ap_array = $this->AS_HAS_PR_lib->getProtoLog($sqlo);
        
        $protos = $this->CS_HAS_PR_lib->mergeProtoArr($cp_array, $ap_array);
        return $protos;
    }
    
    /**
     * get first cpid by pra_id
     * @param object $sqlo
     * @throws Exception
     * @return int cp_id
     */
    function get_c_proto_by_pra(&$sqlo, $pra_id) {
        
        if ( !$this->suc_id ) {
            throw new Exception('Class not inizialized.');
        }
        $cpid_out=0;
        $proto_log = $this->CS_HAS_PR_lib->getProtoLog($sqlo);
        foreach($proto_log as $row) {
            if ($row['ap']==$pra_id) {
                $cpid_out=$row['cp'];
                break;
            }
        }
        return $cpid_out;
    }
    
    /**
     * get STEP_ID by PRA_ID from AS_HAS_PR
     * @param object $sqlo
     * @param int $pra_id
     * @return int $STEP_ID
     */
    function get_a_stepinfo_by_pra(&$sqlo, $pra_id) {
        $sua_id = glob_elementDataGet( $sqlo, 'CONCRETE_SUBST', 'CONCRETE_SUBST_ID', $this->suc_id,  'ABSTRACT_SUBST_ID');
        $this->AS_HAS_PR_lib->setObj($sua_id);
        $log = $this->AS_HAS_PR_lib->getProtoLog($sqlo);
       
        $found=0;
        foreach($log as $row) {
            if ($row['ap']==$pra_id) {
                $found=1;
                break;
            }
        }
        if ($found) return $row;
        return NULL;
    }
    
    /**
     * get protocol by step_no
     * @param object $sqlo
     * @param int $step_no
     * @throws Exception
     * @return array $proto_log : see  o.CS_HAS_PR.subs.inc:cprotoLog_STRUCT
     */
    function get_proto_by_step(&$sqlo, $step_no) {
        
        if ( !$this->suc_id ) {
            throw new Exception('Class not inizialized.');
        }
        
        $proto_row=array();
        $proto_log = $this->CS_HAS_PR_lib->getProtoLog($sqlo);
        foreach($proto_log as $row) {
            if ($step_no==$row['or']) {
                $proto_row=$row;
                break;
            }
        }
        
        return $proto_row;
    }
    
    /**
     * get all distinct SUCs from all protocols, which have ABSTRACT_SUBST_ID=$sua_id
     * @param object $sqlo
     * @param int $sua_id
     */
    function get_sub_sucs_by_sua(object $sqlo, int $sua_id) {
        $MAX_STORE=20;
        $sqlsel_1 = "select CONCRETE_PROTO_ID from CS_HAS_PR where CONCRETE_SUBST_ID=".$this->suc_id;
        $sqlsel_2 = "select CONCRETE_SUBST_ID from CONCRETE_SUBST where ABSTRACT_SUBST_ID=".$sua_id;
        $sqlsel = "from CONCRETE_PROTO_STEP where ".
          "CONCRETE_PROTO_ID in (".$sqlsel_1.") and ".
          "CONCRETE_SUBST_ID in (".$sqlsel_2.")";
        
        $sqlo->Quesel("distinct(CONCRETE_SUBST_ID) ".$sqlsel);
        $i=0;
        $suc_arr=array();
        while ($sqlo->ReadRow() ) {
            if ($i>$MAX_STORE) break;
            $suc_loop = $sqlo->RowData[0];
            $suc_arr[]= $suc_loop;
            $i++;
        }
        return $suc_arr;
    }
    
    /**
     * do object tracking of SUC => SUC for to the next SUC level
     * @param object $sqlo
     * @return array
     *   'sucs'=> array of SUC-IDs
     *   'cnt'=> real number of hits
     */
    function obj_tracking_suc(&$sqlo, $MAX_STORE=50) {
        if ( !$this->suc_id ) {
            throw new Exception('Class not inizialized.');
        }

        
        $sua_arr = array();
        
        $sqlsel_1 = 'select CONCRETE_PROTO_ID from CONCRETE_PROTO_STEP where CONCRETE_SUBST_ID='.$this->suc_id;
        $sqlsel_2 = "from CS_HAS_PR where CONCRETE_PROTO_ID in (".$sqlsel_1.")";
        
        $sqlo->Quesel("count(1) ".$sqlsel_2);
        $sqlo->ReadRow();
        $cnt = $sqlo->RowData[0];
        
        $sqlo->Quesel("CONCRETE_SUBST_ID ".$sqlsel_2);
        $i=0;
        while ($sqlo->ReadRow() ) {
            if ($i>$MAX_STORE) break;
            $sua_loop = $sqlo->RowData[0];
            $sua_arr[]= $sua_loop;
            $i++;
        }
        
        return array('cnt'=>$cnt, 'sucs'=>$sua_arr);
    }
    
    /**
     * get RELEASED acceptance protocol (ACCEPT_PROT) from ABSTRACT_PROT (QC)
     * @param object $sqlo
     * @param int $pra_id
     */
    private function accept_proto($sqlo, $pra_id) {
       
        $acceptLib = new oACCEPT_PROT_subs($sqlo);
        
        $accOpt      = array( 'onlyReleased'=>1 );
        $accProtoArr = $acceptLib->getAcceptByAbstract($sqlo, $pra_id, $accOpt);
        $ACCEPT_PROT_ID = 0;
        
        if (is_array($accProtoArr)) {
            if (sizeof($accProtoArr)==1) {
                // take that one
                $currentAccID   = current ($accProtoArr);
                $ACCEPT_PROT_ID = $currentAccID;
            } else {
                // @todo give warning here ???
                // WARN: mehrere Acceptances:
                $currentAccID = current ($accProtoArr);
                $ACCEPT_PROT_ID = $currentAccID;
            }
        }
        
        return $ACCEPT_PROT_ID;
    }
    
    
    /**
     * create protocol 
     * - lazy selection of the ABSTRACT_PROTO_ID => if $pra_id not given, use the FIRST abstract proto of ABSTRACT_SUBST_ID
     * - support linking of ACCEPT_PROT
     * @param object $sqlo
     * @param array $features  table features of CONCRETE_PROTO
     *   'vals'=>array() :
     * @param int $pra_id  [OPTIONAL] - ABSTRACT_PROTO_ID
     * @param int $step_nr [OPTIONAL] 
     * @param int $type 1,2
     * @return int $prc_id
     */
    function create_proto(&$sqlo, $features, $pra_id=0, $step_nr=0, $type=1) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if ( !$this->suc_id ) {
            throw new Exception('Class not inizialized.');
        }
        
        $sua_id = glob_elementDataGet( $sqlo, 'CONCRETE_SUBST', 'CONCRETE_SUBST_ID', $this->suc_id,  'ABSTRACT_SUBST_ID');
        $this->AS_HAS_PR_lib->setObj($sua_id);
        // 'st'=>$step_id, 'ap'=>$a_proto_id, 'or'=>$XORDER, 'ty'=>$type)
        $sua_proto_log = $this->AS_HAS_PR_lib->getProtoLog($sqlo);
        if (!sizeof($sua_proto_log)) {
            $error->set( $FUNCNAME, 1, 'MAM-ID:'.$sua_id.' has no defined protocols.');
            return;
        }
        
        if (!$pra_id) {
            // get first planned proto_entry
            $first_proto = current($sua_proto_log);
            $pra_id  = $first_proto['ap'];
            $step_nr = $first_proto['or'];
            $type    = $first_proto['ty'];
        }
        
        if (!$step_nr) {

            // look for PRA-ID
            $found = 0;
            foreach($sua_proto_log as $row) {
                if ($pra_id==$row['ap']) {
                    $found = 1;
                    break;
                }
            }
            
            if ($found) {
                $step_nr = $row['or'];
                $type    = $row['ty'];
            } else {
                // NOT found: 
                $first_proto = current($sua_proto_log);
                $pra_id  = $first_proto['ap'];
                $step_nr = $first_proto['or'];
                $type    = $first_proto['ty'];
            }
        }
       
        $insertlib = new insertC();
        
        if (empty($features)) $features=array('vals'=>array());
        $features['vals']['ABSTRACT_PROTO_ID'] = $pra_id;
        
        if ( table_exists( $sqlo, 'ACCEPT_PROT' )) {
            $acc_id = $this->accept_proto($sqlo, $pra_id);
            if ($acc_id)  {
                $features['vals']['ACCEPT_PROT_ID'] = $acc_id;
            }
        }
        
        $prc_id = $insertlib->new_meta($sqlo, 'CONCRETE_PROTO', $features);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Protocol creation (PRA-ID:'.$pra_id.') ) failed.');
            return;
        }
        
        if (!$step_nr) $step_nr=10; // fallback
        if (!$type) $type=1; // fallback
        $this->CS_HAS_PR_lib->add_protocol($sqlo, $step_nr, $prc_id, $type);
        
        return $prc_id;
    }
    
    function add_protocol($sqlo, int $step_nr, int $prc_id, int $type) {
        $this->CS_HAS_PR_lib->add_protocol($sqlo, $step_nr, $prc_id, $type);
    }
    
    /**
     * update steps by STEP_NAME
     * @param object $sqlo
     * @param int $cpid
     * @param array $step_params
     */
    function mod_steps_by_name(&$sqlo, $cpid, $step_params_name) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $proto_mod_lib = new oConcProtoUpSteps();
        $proto_mod_lib->setProto($cpid);
        
        $pra_id = glob_elementDataGet( $sqlo, 'CONCRETE_PROTO', 'CONCRETE_PROTO_ID', $cpid ,  'ABSTRACT_PROTO_ID'); 
        
        $step_params = $proto_mod_lib->step_arr_name2no($sqlo, $step_params_name);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on Step-names-Input.' );
            return;
        }
        
        $proto_mod_lib->protoUpdate($sqlo, $pra_id, $step_params);
    }
    
    /**
     * update steps by STEP_NO
     * @param object $sqlo
     * @param int $cpid
     * @param array $step_params
     */
    function mod_steps_by_no(&$sqlo, $cpid, $step_params) {
        //global $error;
        //$FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $proto_mod_lib = new oConcProtoUpSteps();
        $proto_mod_lib->setProto($cpid);
        
        $pra_id = glob_elementDataGet( $sqlo, 'CONCRETE_PROTO', 'CONCRETE_PROTO_ID', $cpid ,  'ABSTRACT_PROTO_ID');
        $proto_mod_lib->protoUpdate($sqlo, $pra_id, $step_params);
    }
    
   
}

class oCONCRETE_SUBST_proto_copy {
    
    function __construct($src_id, $dst_id) {
        $this->src_id=$src_id;
        $this->dst_id=$dst_id;
    }
    
    /**
     * copy all PREP protos, ignore QC-protos
     * @param object $sqlo
     */
    function copy_PREP_protocols($sqlo, $sqlo2) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        debugOut("(372) copy Protos from SUC: ".$this->src_id. ' to '. $this->dst_id, $FUNCNAME,1);
        $prc_lib     = new oCONCRETE_SUBST_proto($this->src_id);
        $prc_log_ORI = $prc_lib->get_c_protos($sqlo);
        
        $prc_lib_DST = new oCONCRETE_SUBST_proto($this->dst_id);
        $sqlo3_dummy=NULL;
        $deep_copy=array('CONCRETE_PROTO_STEP'=>1);
        $crea_opt =array();
        $objCreaLib  = new objCreaWiz('CONCRETE_PROTO');
         
        foreach($prc_log_ORI as $row) {
            
            $type = $row['ty'];
            if ($type != oCS_HAS_PR_subs::TYPE_PREP) continue; // ignore 
            
            $prc_SRC = $row['cp'];
            $new_params=array();
            $prc_id_NEW = $objCreaLib->objCreate($sqlo, $sqlo2, $sqlo3_dummy, $prc_SRC, $new_params, $deep_copy, $crea_opt);
            if ($error->Got(READONLY))  {
                $error->set( $FUNCNAME, 1, 'Creation of PRC failed.' );
                return;
            }

            $prc_lib_DST->add_protocol($sqlo, $row['or'], $prc_id_NEW, $type);

        }
    }
}

/**
 * advanced object tracking
 * - search protocols, where SUC is used
 */
class oCONCRETE_SUBST_proto_use {
    
    function __construct($suc_id) {
        $this->suc_id=$suc_id;
        $this->cnt=0; // used for some analysis tools
    }
    
    function get_cnt() {
        return $this->cnt;
    }
    
    /**
     * get all PRCs, where SUC is sample
     * warn, if too many ???
     * MAX-CACHE:1000
     * @param object $sqlo
     * @return array of IDs
     */
    function get_all_protos_where_sample($sqlo) {
        
        $MAX_STORE=1000;
        
        $prc_list=array();
        $sqlsel_PRE=" FROM abstract_proto_step aps,
              concrete_proto_step cps
        where aps.is_sample = 1
         and aps.abstract_proto_id = cps.abstract_proto_id
         and aps.step_nr = cps.step_nr
         and cps.concrete_subst_id=".$this->suc_id;

        $sqlsel="count(distinct(cps.concrete_proto_id)) ".$sqlsel_PRE;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $this->cnt = $sqlo->RowData[0];

        $sqlsel="distinct(cps.concrete_proto_id) ".$sqlsel_PRE.' order by cps.concrete_proto_id';
        
        $sqlo->Quesel($sqlsel);
        $i=0;
        while ( $sqlo->ReadRow() ) {
            if ($i>$MAX_STORE) break;
            $tmpid = $sqlo->RowData[0];
            $prc_list[]=$tmpid;
            $i++;
        }
        return $prc_list;
        
    }
    
    /**
     * get all PRCs, where SUC is sample
     * warn, if too many ???
     * MAX-CACHE:1000
     * @param object $sqlo
     * @param array $params
     *  'pra' => only of type pra
     * @return array of IDs
     */
    function get_SUCs_where_used($sqlo, $params=array() ) {
        
        $MAX_STORE=1000;
        
        $suc_list=array();
        $sqlsel_PRE1="select cps.concrete_proto_id  FROM 
           concrete_proto_step cps
        where cps.concrete_subst_id=".$this->suc_id;
        
        if ($params['pra']) {
            $sqlsel_PRE1 .= " and cps.ABSTRACT_PROTO_ID=".$params['pra'];
        }
        
        $sqlsel_PRE="from CS_HAS_PR where CONCRETE_PROTO_ID in (".$sqlsel_PRE1.')';
       
        $sqlsel="count(distinct(CONCRETE_SUBST_ID)) ".$sqlsel_PRE;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $this->cnt = $sqlo->RowData[0];
        
        $sqlsel="distinct(CONCRETE_SUBST_ID) ".$sqlsel_PRE.' order by CONCRETE_SUBST_ID';
        
        $sqlo->Quesel($sqlsel);
        $i=0;
        while ( $sqlo->ReadRow() ) {
            if ($i>$MAX_STORE) break;
            $tmpid = $sqlo->RowData[0];
            $suc_list[]=$tmpid;
            $i++;
        }
        return $suc_list;
        
    }
}
