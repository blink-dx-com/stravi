<?php

require_once 'o.proj.profile.inc';
require_once 'o.PROJ.subs.inc';
require_once ('func_form.inc');
require_once ("f.objview.inc");
require_once 'o.PROTO.quant.inc';
require_once 'f.sql_query.inc';

/**
 * protoquant CONFIGURATION GUI helper
 * @author skube
 * 
 * o.PROTO.conf_gui2.inc
 */
class oProtoQuant_ConfGuiC {
    
    private $PROFILE_DIR_NAME='Protoquant';
    
    function __construct($sqlo, $q_table) {
        $proj_lib = new profile_funcs();
        $this->q_table = $q_table;
        $this->profile_proj_id  = $proj_lib->getProj($sqlo,  $this->PROFILE_DIR_NAME, "CREATE" );
    }
    
   
    
    
    function form_seldoc(&$sqlo, $parx) {
        
        
        $objLinkLib = new fObjViewC();
        $html_tmp = $objLinkLib->bo_display( $sqlo, 'PROJ',  $this->profile_proj_id );
        echo 'Configuration folder: '.$html_tmp."<br>";
        
        $docarr_tmp = array();
        
        $proj_lib = new cProjSubs();
        $sql_after = $proj_lib->getTableSQLInSort( $this->profile_proj_id, 'LINK' );
        $sqlsel = "LINK_ID, NAME from ".$sql_after;
        $sqlo->Quesel($sqlsel);
        while ( $sqlo->ReadRow() ) {
            $docarr_tmp[$sqlo->RowData[0]] = $sqlo->RowData[1];
        }
        
        // show only docs of type for this q_table
        $docarr = array();
        foreach($docarr_tmp as $one_doc_id => $doc_name) {
            $params = fQuant_doc::get_ana_params($sqlo, $one_doc_id);
            if ($params['table']==$this->q_table) {
                $docarr[$one_doc_id]=$doc_name;
            }
        }
        
        $initarr   = array();
        $initarr["action"]      = 'obj.protocol.quant.php'; // $_SERVER['PHP_SELF'];
        $initarr["title"]       = "Select configuration";
        $initarr["submittitle"] = "Submit";
        $initarr["tabwidth"]    = "AUTO";
        
        $hiddenarr = NULL;
        $hiddenarr['action'] = 'sel_config';
        $hiddenarr['q_table'] = $this->q_table;
        
        $formobj = new formc($initarr, $hiddenarr, 0);
        
        $fieldx = array (
            "title" => "Configuration",
            "name"  => "docid",
            "object"=> "select",
            "val"   => $parx["docid"],
            "inits" => $docarr,
            "notes" => ""
        );
        $formobj->fieldOut( $fieldx );
        
        $formobj->close( TRUE );
        
    }
    
    
    
    
    
    /**
     * analyse first object in SELECTION, get all PROFILE Docs or analyse cache $_SESSION['s_formState']['o.PROTO.quant']['SUA_arr']
     * @param object $sqlo
     */
    static function get_doc_id_by_MAA($sqlo) {
        $tablename  = "CONCRETE_SUBST";
        
        $utilLib  = new fSqlQueryC($tablename);
        $sqlAfter = $utilLib->get_sql_after();
        
        $sqlsel = "x.CONCRETE_SUBST_ID, x.ABSTRACT_SUBST_ID from ".$sqlAfter;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $sua_id = $sqlo->RowData[1];
        
        if ( is_array($_SESSION['s_formState']['o.PROTO.quant']['SUA_arr']) ) {
            $doc_id =  $_SESSION['s_formState']['o.PROTO.quant']['SUA_arr'][$sua_id];
            if ($doc_id) return $doc_id;
        }
        
        $_SESSION['s_formState']['o.PROTO.quant'] = array();
        $_SESSION['s_formState']['o.PROTO.quant']['SUA_arr'] = array();
        
        $all_docs = fQuant_doc::get_docs_of_profile($sqlo);
        foreach($all_docs as $one_doc_id) {
            $params = fQuant_doc::get_ana_params($sqlo, $one_doc_id);
            // get first SUA-ID ...
            $sua_id    = $params['abs_obj_id'];
            $doc_table = $params['table'];
            if ($doc_table=='CONCRETE_SUBST' and $sua_id) {
                
                $_SESSION['s_formState']['o.PROTO.quant']['SUA_arr'][$sua_id] = $one_doc_id;
            }
        }
        
        $doc_id =  $_SESSION['s_formState']['o.PROTO.quant']['SUA_arr'][$sua_id];
        
        return $doc_id;
        
    }
    
}
