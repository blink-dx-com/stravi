<?php
/**
 * [protoQuant] help for Definition GUI
 * @package o.PROTO.def.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 *@var $_SESSION['userGlob']["o.proto.Quant_sel"] o_proto_Quant_sel_STRUCT
 *  'current' => DOC_ID
 *  {TABLENAME} => array(
 *     'doc.sel' => 'AUTO'
 *     'docid'   => DOC_ID
 *  )
 */
require_once 'o.PROTO.def_min.inc';
require_once 'o.PROTO.quant_help.inc';
require_once 'f.sql_query.inc';
require_once 'glob.obj.conabs.inc';

/**
 * small helper methods
 * @author skube
 *
 */
class fQuant_helper {
    
    static function get_abs_obj_html($sqlo, $mo_table, $abs_id) {
        
        $abs_conLib = new gConcAbsC();
        $objLinkLib = new fObjViewC();
        
        $abs_table  = $abs_conLib->getTemplate( $mo_table );
        if ($abs_table) {
            $html_tmp = $objLinkLib->bo_display( $sqlo, $abs_table,  $abs_id );
        } else {
            $html_tmp = $abs_id.'?';
        }
        return $html_tmp;
    }
    
    static function set_session_doc(int $docid, $q_table) {
        /**
         * OLD:
         * $user_globals_qu = unserialize($_SESSION['userGlob']["o.proto.Quant_sel"]); // see o_proto_Quant_sel_STRUCT
        
        if (!is_array($user_globals_qu)) {
            $user_globals_qu=array();
        }
        if (!is_array($user_globals_qu[$this->q_table] )) {
            $user_globals_qu[$this->q_table] = array();
        }
        
        $user_globals_qu[$this->q_table]['docid']  = $doc_id;
        $user_globals_qu['current']  = $doc_id;
        
        $_SESSION['userGlob']["o.proto.Quant_sel"] = serialize($user_globals_qu);
         */
        
        
        $user_globals_qu=array();
        $user_globals_qu['doc.sel']='doc.def';
        $user_globals_qu['docid']  = $docid;
        
        if( empty($_SESSION['s_formState']['f.protoquant']) ) {
            $_SESSION['s_formState']['f.protoquant']=array();
        }
        $_SESSION['s_formState']['f.protoquant']['current'] = $docid;
        
        // this variable should be deprecated, please use s_formState below ...
        // OLD: $_SESSION['userGlob']["o.proto.Quant_sel"] = serialize($user_globals_qu);
        // e.g. 'o.CONCRETE_SUBST.pq'
        $_SESSION['s_formState']['o.'.$q_table.'.pq'] = $user_globals_qu;
    }
}

/**
 * create new config
 * @author skube
 *
 */
class fQuantConfNew {
    
    const DOC_CLASS_NAME='protoquant';
    
    function __construct($sqlo, $parx, $q_table ) {
        $PROFILE_DIR_NAME=fQuant_defs::PROFILE_DIR_NAME;
        
        $this->parx = $parx;
        $this->q_table = $q_table;
        
        $proj_lib = new profile_funcs();
        $this->profile_proj_id  = $proj_lib->getProj($sqlo,  $PROFILE_DIR_NAME, 'CREATE' );
    }
    
    function form1( &$sqlo  ) {
        
        
        $objLinkLib = new fObjViewC();
        $html_tmp = $objLinkLib->bo_display( $sqlo, 'PROJ', $this->profile_proj_id );
        echo 'Configs are saved in folder: '.$html_tmp.'<br><br>';
        
        $initarr   = NULL;
        $initarr["action"]      = $_SERVER['PHP_SELF'];
        $initarr["title"]       = "Create configuration";
        $initarr["submittitle"] = "Submit";
        $initarr["tabwidth"]    = "AUTO";
        $initarr["tabnowrap"]   = 1;
        $initarr["dblink"]      = 1;
        
        $hiddenarr = NULL;
        $hiddenarr["action"]  = 'crea_config';
        $hiddenarr["q_table"] = $this->q_table;
        
        $formobj = new formc($initarr, $hiddenarr, 0);
        
        $fieldx = array (
            "title" => "Name of config",
            "name"  => "name",
            "object"=> "text",
            "val"   => '',
            "notes" => "" );
        $formobj->fieldOut( $fieldx );
        $formobj->close( TRUE );
    }
    
    function analyse(&$sqlo ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        
        $this->parx['name'] = trim($this->parx['name']);
        $parx = $this->parx;
        
        
        if ($parx['name']=='') {
            $error->set( $FUNCNAME, 1, 'name is missing' );
            return;
        }
        
        $newname = $parx['name'];
        
        
        $proj_lib = new cProjSubs();
        $num_objects = $proj_lib->objectByNameInProject( $sqlo, $this->profile_proj_id, 'LINK', $newname );
        
        if ($num_objects>0) {
            $error->set( $FUNCNAME, 1, 'Config with name "'.$newname.'" already exits.' );
            return;
        }

    }
    
    // TBD: do same for EXPERIMENT ...
    private function get_first_object_ABS($sqlo) {
        
        $utilLib  = new fSqlQueryC($this->q_table);
        $sqlAfter = $utilLib->get_sql_after();
        
        $conabs_lib = new gConcAbsC();
        // get ABS_NAME
        $abs_table = $conabs_lib->getTemplate($this->q_table);
        $pk_name = PrimNameGet2($abs_table);
        
        $sqlsel = "x.".$pk_name." from ".$sqlAfter; 
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $abs_obj_id = $sqlo->RowData[0];
        return $abs_obj_id;
    }
    
    function create_doc( &$sqlo ) {
        global $error, $varcol;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        require_once ("insertx.inc");
        require_once 'o.PROJ.addelems.inc';
        
        $varcol = & Varcols::get();
        $class_id	= $varcol->class_name_to_id( 'LINK', self::DOC_CLASS_NAME );
        
        $insertlib = new insertC();
        $args= array(
            "vals"=>array(
                'NAME'=> $this->parx['name'] 
            ),
            "xobj" => array (
                'extra_class_id' => $class_id,
            )
            
        );
        $doc_id = $insertlib->new_meta($sqlo, 'LINK', $args);
        
        $protquat_lib = new oProtoQuantC();
        $protquat_lib->set_docid($sqlo, $doc_id, 1);
        
        $o_struct_lib = new oPQ_OBJ_STRUCT();
        $o_struct_lib->create($this->q_table, -1, array() );
        $start_sub_arr = $o_struct_lib->get_structure(); 
        
        $abs_obj_id = 0;
        switch ($this->q_table) {
            case 'CONCRETE_SUBST':
                $abs_obj_id = $this->get_first_object_ABS($sqlo);
                break;
            case 'CONCRETE_PROTO':
                $abs_obj_id = $this->get_first_object_ABS($sqlo);
                $start_sub_arr['src']['abs_id'] = $abs_obj_id; // set ABSTRACT_PROTO_ID ...
                break;
        }
        
        $main_struct = array('table'=>$this->q_table, 'abs_obj_id'=>$abs_obj_id);
        $protquat_lib->set_globset_main($main_struct);
        $protquat_lib->set_sub_struct(1, $start_sub_arr);
 
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on creation: preparation.' );
            return;
        }
        
        $protquat_lib->save_globset($sqlo);
        
        if ($doc_id) {
            $proj_mod_lib = new oProjAddElem($sqlo, $this->profile_proj_id);
            $proj_mod_lib->addObj($sqlo, 'LINK', $doc_id);
        }
        
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 2, 'Error on creation.' );
            return;
        }
        
        $objLinkLib = new fObjViewC();
        $html_tmp1  = $objLinkLib->bo_display( $sqlo, 'LINK',  $doc_id );
        $html_tmp   = $objLinkLib->bo_display ( $sqlo, 'PROJ',  $this->profile_proj_id );
        echo 'Config "'.$html_tmp1.'" saved in Configuration folder: '.$html_tmp."<br>";
        
        return $doc_id;
        
        
        
    }
    
    /**
     * create and save sub structure
     * @param object $sqlo
     * @param int $doc_id
     * @param array $parx
     *   'ty'
     *   'col'
     *   't'
     *   'mo_id'
     *   'fkt' : OPTIONAL foreign table
     */
    function create_sub_struct($sqlo, $doc_id, $parx) {
        global $error;
        
        
        $protquat_lib = new oProtoQuantC();
        $protquat_lib->set_docid($sqlo, $doc_id);
        $last_so_id = $protquat_lib->get_sub_maxid();
        
        $o_struct_lib = new oPQ_OBJ_STRUCT();
        
        $src = array('ty'=>$parx['ty'], 'col'=>$parx['col']);
        if ($parx['fkt'])      $src['t']    = $parx['fkt'];
        if ($parx['abs_id'])   $src['abs_id'] = $parx['abs_id'];
        if ($parx['step'])     $src['step'] = $parx['step'];
        if ($parx['col_nice']) $src['col_nice'] = $parx['col_nice'];
        
        $o_struct_lib->create( $parx['t'], $parx['mo_id'], $src );
        $start_sub_arr = $o_struct_lib->get_structure();
        
        $new_so_id = $last_so_id + 1;
        
        $protquat_lib->set_sub_struct($new_so_id, $start_sub_arr);
        
        // $temp = $protquat_lib->get_globset();
       
        $protquat_lib->save_globset($sqlo);
    }
    
}

/**
 * ABSTRACT class : manage ProtQuant settings, e.g. get/set settings from DOCUMENT
 * @author skube
 *
 */
class fQuantSettings {
    
    var $userglob;
    var $docid;
    var $quantLib;
    protected $so_id; // ID in objects
    
    function __construct($sqlo, $docid) {
        
        $this->docid   = $docid;
        $this->quantLib = new oProtoQuantC();
        
        $this->userglob =  $this->quantLib->set_docid($sqlo, $docid);
        
    }
    
    protected function _show_log($text) {
        echo "-INFO: ".$text."<br>";
    }
    
    function get_userglob() {
        return $this->userglob;
    }
    
    
    function saveGlobals(&$sqlo, $vararray ) {
        
        if ( !sizeof($vararray) ) {
            $vararray=array();
        }
        
        $this->quantLib->save_ana_params($sqlo, $this->docid, $vararray);
        
    }
    
    function save_sub_set($sqlo, $so_id , $sub_struct) {
        $this->quantLib->set_sub_struct($so_id , $sub_struct);
        $this->quantLib->save_globset($sqlo);
    }
    
    /**
     * return OBJ_STRUCT_sub
     * @param int $so_id
     * @return array
     */
    function get_sub_struct($so_id) {
        return $this->quantLib->get_sub_struct($so_id);
    }
    
    
    
}



/**
 * PROTOCOL STEP definition
 * @author skube
 *
 */
class oAbsProtoStepDet extends fQuantSettings {
    
    private $use_SUB_STRUCT;
    private $apid;
    private $STEP_COLTYPEs;
    
    /**
     *
     * @param object $sql
     * @param int $docid
     * @param array $x
     * @param string $mode_in
     *     'main',
     *     'sub_struct'
     */
    function __construct(&$sql, $docid, $x, $mode_in='') {

        
        $this->docid   = $docid;
        // $this->backurl = $backurl;
        
        $this->params     = $x;
        $this->numDetails = 8; // subst, subst-ID, a-subst-name, quant, notes, device, device-ID
        
        $this->mode = 'main';
        if ($mode_in!='') $this->mode = $mode_in;
        
        parent::__construct($sql, $docid);
        
        $this->use_SUB_STRUCT = array();
        
        $this->STEP_COLTYPEs = oProtoQuant_help::get_STEP_COLTYPEs();
    }
    
    /**
     * get APID from SUA of mo_apid, mo_stepnr
     * @param object $sqlo
     * @return void|number
     */
    private function get_mo_sua(&$sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $output = array('apid'=>0, 'sua_id'=>0);
        
        $sqlsel = "ABSTRACT_SUBST_ID from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$this->mo_apid. " and STEP_NR=".$this->mo_stepnr;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $sua_id = $sqlo->RowData[0];
        if (!$sua_id) {
            $error->set( $FUNCNAME, 1, 'No SUA for this step (PRT-ID:'.$this->mo_apid.').' );
            return;
        }
        
        $output['sua_id'] = $sua_id;
        
        // get proto
        require_once 'o.AS_HAS_PR.subs.inc';
        $sua_prot_lib = new oAS_HAS_PR_subs($sua_id);
        $proto_log = $sua_prot_lib->getProtoLog($sqlo);
        
        if (!sizeof($proto_log)) {
            // $error->set( $FUNCNAME, 1, 'No PRA found for MAA-ID:'.$sua_id.'.' );
            return $output;
        }
        
        $row = current($proto_log);
        $apid = $row['ap'];
        
        if (!$apid) {
            // $error->set( $FUNCNAME, 2, 'No PRA found for MAA-ID:'.$sua_id.'.' );
            return $output;
        }
        
        return array('apid'=>$apid, 'sua_id'=>$sua_id);
    }
    
 
    
    function set_sub_struct($so_id, $SUB_STRUCT) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->so_id   = $so_id;

        $this->use_SUB_STRUCT = $SUB_STRUCT;

        $this->apid    = $this->use_SUB_STRUCT['src']['abs_id'];
        $this->q_table = $this->use_SUB_STRUCT['table'];
        
        if (!$this->apid) {
            $error->set( $FUNCNAME, 1, 'APID missing.' );
            return;
        }
        
        $obj_id = $this->use_SUB_STRUCT['obj_id'];
        
        if ($this->q_table=='ABSTRACT_SUBST') {
            $objLinkLib = new fObjViewC();
            if ($obj_id)    $html_tmp = $objLinkLib->bo_display( $sqlo, 'ABSTRACT_SUBST',  $obj_id );
            else  $html_tmp = '?';
            echo "Sub-Material: ".$html_tmp." (Proto-ID:".$this->apid.")<br>";
        }
        
    }
    
    
    

    
    /**
     * save sub structure
     * @param object $sql
     */
    function saveVars( &$sqlo ) {
        
        $out=NULL;
        if ( !sizeof($this->params) ) {
            $this->params = array();
        }
        
        $stepdets=0;
        foreach( $this->params as $step=>$valarr) {
            foreach( $valarr as $key=>$val) {
                $out[$step] .= $key;
                $stepdets++;
            }
        }
        reset ($this->params);
        
        $sub_struct = $this->use_SUB_STRUCT;
        $sub_struct["steps"] = $out;
        
        $this->save_sub_set($sqlo, $this->so_id , $sub_struct);

        echo "Saved <b>$stepdets</b> step details.<br>";
    }
    
    function _checkboxOut( $key, $stepnr ){
        $ischecked = "";
        if ( strstr($this->use_SUB_STRUCT["steps"][$stepnr], $key)!=NULL ) $ischecked = " checked";
        return ( "<input type=checkbox name=x[".$stepnr."][".$key."] value=1".$ischecked.">" );
    }
    
    function _get_sub_url($apid, $stepnr) {
        $url = $_SERVER['PHP_SELF'].'?action=sub_struct&parx[mo_apid]='.$apid.'&parx[stepnr]='.$stepnr;
        return $url;
    }
    
    function _oneStepAna( &$sqlo, $oneStep ) {
        $outarr = array();
        
        $stepnr  = $oneStep["STEP_NR"];
        $apid = $this->apid;
        $sua_name='';
        
        if ($oneStep["ABSTRACT_SUBST_ID"]>0) {
            $sua_name = obj_nice_name ( $sqlo, "ABSTRACT_SUBST", $oneStep["ABSTRACT_SUBST_ID"] );
        }
        
        if ($oneStep["SUB_STEP_NR"]>0) $linecnt = "&nbsp;&nbsp;&nbsp;".$this->stSubCnt;
        else {
            $linecnt = $this->stMainCnt;
            $this->stSubCnt = 1;
        }
        $outarr[] = $linecnt.". ".$oneStep["NAME"];
        $outarr[] = $sua_name;
        
        foreach($this->STEP_COLTYPEs as $key) {
            $outarr[] = $this->_checkboxOut( $key, $stepnr );
        }
        /*
        $outarr[] = $this->_checkboxOut( "s", $stepnr );
        $outarr[] = $this->_checkboxOut( "S", $stepnr );
        $outarr[] = $this->_checkboxOut( "a", $stepnr );
        $outarr[] = $this->_checkboxOut( "q", $stepnr );
        $outarr[] = $this->_checkboxOut( "n", $stepnr );
        $outarr[] = $this->_checkboxOut( "d", $stepnr );
        $outarr[] = $this->_checkboxOut( "D", $stepnr );
        */ 
        
        if ($oneStep["ABSTRACT_SUBST_ID"]>0) {
            
            $sua_id = $oneStep["ABSTRACT_SUBST_ID"];
            $tablename = $this->q_table;

            
            // $so_id exists ???
            $found_so_id = $this->quantLib->sub_struct_sea( 'CONCRETE_SUBST', $this->so_id, array( 'ty'=>'ass', 't'=>'CONCRETE_PROT_STEP', 'col'=>'CONCRETE_SUBST_ID', 'step'=>$stepnr ) );
            if ($found_so_id) {
                $sub_action ='[<a href="'.$_SERVER['PHP_SELF'].'?action=sub_struct&so_id='.$found_so_id.'">Define</a>]';
            } else {
                $sub_action ='[<a href="'.$_SERVER['PHP_SELF'].'?action=sub_struct_new&parx[mo_id]='.$this->so_id.
                   '&parx[ty]=ass&parx[t]=CONCRETE_SUBST&parx[col]=CONCRETE_SUBST_ID&parx[fkt]=CONCRETE_PROTO_STEP&parx[step]='.$stepnr.
                   '&parx[abs_id]='.$sua_id.'">Define new</a>]';
            }
            
            
            $outarr[] = $sub_action;
        } else {
            $outarr[] = '';
        }
        if ($oneStep["SUB_STEP_NR"]>0) $this->stSubCnt++;
        else $this->stMainCnt++;
        
        return ($outarr);
    }
    
    /** show for for SUC => abstract_proto
     *
     * @param object $sqlo
     * @param object $sqlo2
     */
    function form_suc_pra_select( &$sqlo, &$sqlo2 ) {
        
        $tablename  = "CONCRETE_SUBST";
        $sqlopt=array();
        $sqlopt["order"] = 1;
        $sqlAfter  = get_selection_as_sql( $tablename, $sqlopt);
        
        $helplib    = new oConcSubstProtQuant_H($sqlAfter);
        $abstractProtos = $helplib->getAbsProtos($sqlo, $sqlo2);
        $formparams = array();
        $this->quantLib->setAbstractProtos($abstractProtos);
        $this->quantLib->abstractProtoSel( $sqlo, $formparams );
        
    }
    
    function form1( &$sql, $sql2  ) {
        
        if ($this->q_table=='CONCRETE_SUBST') {
            $this->form_suc_pra_select( $sql, $sql2 );
            return;
        }
        
        $apid = $this->userglob["apid"];
        
        $tablename = "ABSTRACT_PROTO";
        
        $initarr   = NULL;
        $initarr["action"]      = $_SERVER['PHP_SELF'];
        $initarr["title"]       = "Select a special protocol";
        $initarr["submittitle"] = "Submit";
        $initarr["tabwidth"]    = "AUTO";
        $initarr["tabnowrap"]   = 1;
        $initarr["dblink"]      = 1;
        $hiddenarr = NULL;
        $hiddenarr["backurl"] = $this->backurl;
        // $hiddenarr["docid"]   = $this->docid;
        
        
        $formobj = new formc($initarr, $hiddenarr, 0);
        
        if (!$apid) {
            $apname = "---";
        } else {
            $apname = obj_nice_name  ( $sql, $tablename, $apid);
        }
        
        $inits = array( "table"=>$tablename, "objname"=>$apname, "pos"=>0, "projlink"=>1 );
        
        $fieldx = array (
            "title" => "Abstract protocol",
            "name"  => "apid",
            "namex" => TRUE,
            "object"=> "dblink",
            "val"   => $apid,
            "inits" => $inits,
            "notes" => "the protocol" );
        $formobj->fieldOut( $fieldx );
        $formobj->close( TRUE );
    }
    
    
    /**
     * show the STEP matrix
     * INPUT: $this->apid
     * @param object $sql
     * @param object $sql2
     * @param string $fo_opt
     *   'action' : 'sub_struct'
     *      'mo_apid':
     *      'step_nr':
     */
    function form2( &$sql, &$sql2, $fo_opt=array() ) {
        
        $apid = $this->apid;
        
        $col_offset = 2; // Step-name + Material<br>template
        
        echo "<form style=\"display:inline;\" method=\"post\"  name=\"editform\"  action=\"".$_SERVER['PHP_SELF']."\" >\n";
        
        $tabobj = new visufuncs();
        $headOpt = array( "title" => "Select shown step details" );
        $headx  = array (
            "Step-name",
            "Planned Material<br>template",
            "Material<br>NAME",
            "Material<br>ID",
            "Mat-Template<br>NAME",
            "Quantity",
            "Notes",
            "Device<br>Name",
            "Device<br>ID",
            "Material-DETAILS"
            
        );
        $tabobj->table_head($headx,   $headOpt);
        
        $sqls = "select * from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$apid. " ORDER by MAIN_STEP_NR";
        $sql2->query($sqls);
        
        $this->stMainCnt = 1;
        $this->stSubCnt  = 1;
        $stepAll=0;
        while ( $sql2->ReadArray() ) {
            $oneStep   = $sql2->RowData;
            $dataArr = $this->_oneStepAna($sql, $oneStep);
            $tabobj->table_row ($dataArr);
            $stepAll++;
        }
        
        $dataArr=NULL;
        $dataArr[]="";
        $dataArr[]="";
        for ($i=0; $i<$this->numDetails ; $i++) {
            $dataArr[] = "<input type=button name='checker_".$i."' value='all'" .
                " onclick=javascript:checkall(".$i.",".$stepAll.")></td>\n";
        }
        $tabobj->table_row ($dataArr);
        
        $dataArr = array("<br><input type=submit value=\"Submit\"><br><br>" );
        $copt = array("colspan"=>$this->numDetails + $col_offset, "bgcolor"=>"#E0E0FF", "other"=>"align=center");
        $tabobj->table_row ($dataArr, $copt);
        
        $tabobj->table_close();
        
        
        echo "<input type=hidden name='action' value='steps'>\n";
        echo "<input type=hidden name='go' value='2'>\n";
        echo "<input type=hidden name='so_id' value='".$this->so_id."'>\n";
        
        echo "</form>";
    }
    
    function children_select($sqlo) {
        
        $objLinkLib = new fObjViewC();
        $children = &$this->userglob["children"];
        foreach($children as $key=>$row) {
            
            $child_arr = explode(":",$key);
            $mo_apid    = $child_arr[0];
            $mo_step_nr = $child_arr[1];
            $apid = $row['apid'];
            
            $dest_url = $this->_get_sub_url($mo_apid, $mo_step_nr);
            $html_tmp = obj_nice_name( $sqlo, 'ABSTRACT_PROTO', $apid );
            echo '- proto: <a href="'.$dest_url.'">'.$html_tmp."</a> (APID:$apid)<br>";
        }
    }
    
}


/**
 * select ABSTRACT_PROTO
 * @author skube
 *
 */
class fQuant_sel_ap extends fQuantSettings {
    
    /**
     * 
     * @param object $sqlo
     * @param string $tablename
     * @param int $so_id index of OBJ_STRUCT
     */
    function __construct(&$sqlo, $tablename, $so_id, $docid) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $main_quant_table = $this->userglob['table'];
        $this->tablename = $tablename;
        $this->so_id     = $so_id;

        parent::__construct($sqlo, $docid);
        
        
        if ( $tablename =='EXP') {
            require_once ('o.EXP.quant.inc');
            
            $this->objdep_lib = new oEXP_Quant();
            $this->objdep_lib->init_quant_loop($this->quantLib,$sqlo);
            // $this->objdep_lib->getAbsProtos($sqlo);
            $this->abstractProtos = $this->objdep_lib->getAbsProtos($sqlo);

        }
        
        if ($this->tablename == 'CONCRETE_SUBST') {
            require_once ('o.CONCRETE_SUBST.quant.inc');
            
            $this->objdep_lib = new oCONCRETE_SUBST_Quant();
            $this->objdep_lib->init_quant_loop( $this->quantLib, $sqlo); //TBD:
            // if ($this->userglob['table']=='CONCRETE_SUBST') {
                // $this->objdep_lib->getAbsProtos($sqlo);
                
            debugOut('so_id: '.$this->so_id, $FUNCNAME, 2);
            
            if ($this->so_id==1) {  // root of analysis tree
                $this->abstractProtos = $this->objdep_lib->getAbsProtos($sqlo);
            } else {
                
                $obj_struct_sub = $this->get_sub_struct($this->so_id);
                $abs_id         = $obj_struct_sub['src']['abs_id'];
                
                $tmp = $this->quantLib->get_globset();
                
                
                if (!$abs_id) {
                    $error->set( $FUNCNAME, 1, 'Input abs_id missing.' );
                    return;
                }
                
                $this->abstractProtos = $this->objdep_lib->get_apids_of_abs($sqlo, $abs_id);
            }
            
        }
        
        if ($this->tablename == 'CONCRETE_PROTO') {
            require_once ('o.CONCRETE_PROTO.quant.inc');

            $this->objdep_lib = new oCONCRETE_PROTO_Quant();
            debugOut('(781) so_id: '.$this->so_id, $FUNCNAME, 2);
            
            if ($this->so_id==1) {  // root of analysis tree
                
                $this->objdep_lib->init_quant_loop( $this->quantLib, $sqlo); 
                $this->abstractProtos = $this->objdep_lib->getAbsProtos($sqlo);
            } else { 
                $obj_struct_sub = $this->get_sub_struct($this->so_id);
                $abs_id         = $obj_struct_sub['src']['abs_id'];
                //$tmp = $this->quantLib->get_globset();

                if (!$abs_id) {
                    $error->set( $FUNCNAME, 1, 'Input abs_id missing.' );
                    return;
                }
                
                $this->abstractProtos = array($abs_id);
            }
            
        }
        
        debugOut("(800) $tablename:abstractProtos:".print_r($this->abstractProtos,1), $FUNCNAME, 1);
        // echo "DEBUGX(685): abstractProtos: ".print_r($this->abstractProtos,1).' x:'.print_r($obj_struct_sub,1)."<br>";
    }
    
    function get_pra_by_abs(&$sqlo, $abs_id) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$abs_id) {
            $error->set( $FUNCNAME, 1, 'Input abs_id missing.' );
            return;
        }
        return $this->objdep_lib->get_apids_of_abs($sqlo, $abs_id);
    }
    
    function get_abs_protos() {
        return $this->abstractProtos;
    }
    
   
}

/**
 * TBD: extra OBJECT faetures
 * @author skube
 *
 */
class fQuant_obj_def_extra {
    
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
        
        if ($tablename == 'CONCRETE_PROTO') {
            require_once ('o.CONCRETE_PROTO.quant.inc');
            $this->objdep_lib = new oCONCRETE_PROTO_Quant();
        }
        
        if ($tablename == 'SOCKET') {
            require_once ('o.SOCKET.quant.inc');
            $this->objdep_lib = new oSOCKET_Quant();
        }
    }
    
    function get_columns() {
        if (!$this->objdep_lib) return;
        
        return $this->objdep_lib->get_columns();
    }
    
    
    
}