<?php
/**
 * - compare concrete_proto_steps
   - show ALL steps
   GLOBAL:  $_SESSION['s_formState']["oPROTOcmp"] = array ("aprotoid"=> , "step_no"=> )
 * @package obj.concrete_proto.m_comp.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   
 *   $tablename  ["EXP"], 
	  			  "CONCRETE_PROTO", 
				  "W_WAFER" 	   (need $aprotoid)
				  "CHIP_READER"
				  "CONCRETE_SUBST"
	  [$proj_id]	  if not  $tablename ... (use experiments in project)
	  [$obj_ids] : optional: use this IDs for selection
	  
	  [$go]	  	  for "EXP" / "W_WAFER"
	  [$step_no]  for "EXP" / "CONCRETE_SUBST" ID in protocol-log
      [$step_id]  for protocol_steps => switches to obj.concrete_proto.m_comp2e.php
      [$action]   "sel_step"    => select single step
                  ["overview"]
                  "all_steps"
      [$sub_action]  
         'show_pra_sel'
      [$ref_proto_in]  - reference protocol ID
	  [$aprotoid] ABSTRACT_PROTO_ID => can overwrite $step_no
      $setti      compare options: "show_quant", "show_subst", "show_st_notes", 
	  				"show_dprot" : show only different protocols
					"show_diff"
      $prefgo     0|1 
 */

session_start(); 



require_once ("db_access.inc");
require_once ("globals.inc");
require_once ("gui/o.PROTO.stepout1.inc");
require_once ("func_head.inc"); 
require_once ("func_form.inc");    
require_once 'f.sql_query.inc';
require_once ("visufuncs.inc");
require_once ('f.msgboxes.inc');
require_once 'o.CONCRETE_SUBST.proto.inc';
require_once 'o.EXP.proto.inc';

 

require_once ("subs/obj.concrete_proto.m_comp.inc");
require_once ("subs/obj.concrete_proto.cmpgui.inc");
require_once ("f.head_proj.inc");

class oPRC_m_comp_C{
    
    /**
     * 
     * @var array $infox
     *   "target_a_proto_id" => int
     *   "first_obj_id"
     *   "ref_cproto_id"  reference concrete_protocol ID
         "abstract_proto_name"
     */
    var $infox;
    private $ori_objects; // array of obj-IDs
    private $proto_ids;
    private $proto_log; //  array('cp'=>ID of protocol, 'ap' => ID of ABSTRACT_PROTO, 'or'=>order number)
    private $step_no;
    
    function init($sqlo, $obj_ids) {
        
        $this->tablename = $_REQUEST['tablename'];
        $this->proj_id = $_REQUEST['proj_id'];
        $this->go= $_REQUEST['go'];
        $this->step_no= $_REQUEST['step_no'];
        // $step_id= $_REQUEST['step_id'];
        $this->action= $_REQUEST['action'];
        $this->ref_proto_in= $_REQUEST['ref_proto_in'];
        $this->aprotoid = $_REQUEST['aprotoid'];
        
        if (!empty($obj_ids)) {
            $this->obj_ids2sql($sqlo, $obj_ids);
        }
        
        if ( $this->action=="" ) $this->action = "overview";
        
        $tablenice  = tablename_nice2($this->tablename);
        $this->compare_Opts= unserialize($_SESSION['userGlob']["o.CONCRETE_PROTO.cmpmode"]);
        $this->infox 		= array(); 
        
        $title="Compare protocols for ".$tablenice;
        $this->main_comp_lib = new oCONCR_PROTOcomp( $this->action, $this->proj_id );
        $this->mainGUIobj    = new oPROTO_cmpgui();
        $this->mainGUIobj->init($this->proj_id, $this->tablename);
        
        // manage a script header with alternativ calling from PROJ or with TABLE-selection
        //
        
        $infoarr=array();
        $dummy = array();
        $this->mainGUIobj->showHeader( $sqlo, $title, $dummy, $infoarr, "all" );
        $this->sqlAfter = $this->mainGUIobj->sqlAfter;	
    }
    
    private function obj_ids2sql($sqlo, $obj_ids) {
        
        $ori_objects=array();
        foreach($obj_ids as $obj_id) {
            if (!gObject_exists ($sqlo, $this->tablename, $obj_id)) {
                continue;
            }
            $ori_objects[]=$obj_id;
        }
        
        if (empty($ori_objects)) {
            $utilLib = new fSqlQueryC('EXP');
            $utilLib->cleanCond();
            $utilLib->queryRelase();
            return;
        }
        
        $sqlselWhere = 'x.EXP_ID in ('.implode(', ',$ori_objects).')';
          
        $utilLib = new fSqlQueryC('EXP');
        $utilLib->cleanCond();
        $utilLib->addCond( $sqlselWhere, '', 'selected objects' );
        $utilLib->queryRelase();
        
    }
    
    function set_prefs($setti) {
        
        $this->compare_Opts=array();
        if ($setti["show_quant"]) $this->compare_Opts["show_quant"]  = $setti["show_quant"];
        if ($setti["show_subst"]) $this->compare_Opts["show_subst"]  = $setti["show_subst"];
        if ($setti["show_st_notes"]) $this->compare_Opts["show_st_notes"]  = $setti["show_st_notes"];
        if ($setti["show_diff"])     $this->compare_Opts["show_diff"]      = 1;
        if ($setti["show_dprot"])    $this->compare_Opts["show_dprot"]     = 1;
        if ($setti["show_navert"])   $this->compare_Opts["show_navert"]    = 1;
        $_SESSION['userGlob']["o.CONCRETE_PROTO.cmpmode"] = serialize($this->compare_Opts); 
    }
    
    function head_links() {
        echo '[<a href="'.$_SERVER['PHP_SELF'].'?tablename='.$this->tablename.'&sub_action=show_pra_sel">Select an other protocol</a>]';
        echo "<br>";
    }
    
    // TBD: $obj_ids was initialized ...
    function select_objects($sqlo)  {
        //
        // get selected objects
        //
        $this->ori_objects  = array();
    
    
        $sqlo->Quesel( $this->tablename."_ID FROM ".$this->sqlAfter);
        while ( $sqlo->ReadRow() ) {
            $this->ori_objects[] = $sqlo->RowData[0];
           
        }
            
        
        $tmpobj = current($this->ori_objects);
        
        $this->infox["first_obj_id"] = $tmpobj;
        
        list($this->step_no, $this->aprotoid) = $this->mainGUIobj->anaFormState($this->step_no, $this->aprotoid);
    }
    
    function form_sel_pra($sqlo) {
        
        $sel_array=array();
        foreach($this->proto_log as $row) {
            if ($row['ap']) {
                $sel_array[ $row['or'] ] = obj_nice_name ( $sqlo, 'ABSTRACT_PROTO', $row['ap'] );
            }
        }
        
        $initarr   = array();
        $initarr["title"]       = "Select Protocol"; // title of form
        $initarr["action"]      = $_SERVER['PHP_SELF'];
        $initarr["submittitle"] = "Select"; // title of submit button
        $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
        
        $hiddenarr = NULL; // hidden form variables
        $hiddenarr["tablename"]     = $this->tablename;
        
        
        $formobj = new formc($initarr, $hiddenarr, 0);
        
        $fieldx = array ( // form-field definition
            "title"   => "Protocol",
            "name"  => "step_no",
            "namex" => TRUE,
            "object"=> "select",
            "inits" => $sel_array,
            "val"    => 0,
            "notes" => "protocol to analyse"
        );
        $formobj->fieldOut( $fieldx ); // output the form-field
        
        $formobj->close( TRUE ); // close form, sshow submit-button
    }
    
    private function _get_pra_from_STEP_NO($step_nr) {
        
        $pra_id=0;
        
        if (empty($this->proto_log)) return;
        
        foreach($this->proto_log as $row) {
            if ($step_nr==$row['or']) {
                $pra_id = $row['ap'];
                break;
            }
        }
        return $pra_id;
    }
    
    function get_proto_log($sqlo, $obj_id) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $tablename=$this->tablename;
        $this->proto_log = array();
        
        if ($tablename=="EXP") {
            $exp_lib = new oEXPprotoC();
            $proto_log_raw = $exp_lib->get_c_protos_details($sqlo, $obj_id);
            
            if(empty($proto_log_raw)) return array();
            
            // transform log
            foreach($proto_log_raw as $row) {
                $this->proto_log[] =  array('cp'=>$row['cp'], 'ap'=>$row['ap'], 'or'=>$row['st'] );
            }
        }
        
        if ($tablename=="CONCRETE_SUBST") {
            
            $suc_lib = new oCONCRETE_SUBST_proto($obj_id);
            $this->proto_log = $suc_lib->get_c_protos($sqlo);
            
        }
        
        debugOut('(210) proto_log:'.print_r($this->proto_log,1), $FUNCNAME, 1); 
        
        return  $this->proto_log;
    }
    
    /**
     * get protocols
     * @param object $sqlo
     * @return int status 
     *      1 ok
     *      -1: please select a protocol
     */
    function get_protocols($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $tablename=$this->tablename;
       
       
        
        if ( $tablename=="CONCRETE_PROTO" ) { 
              $this->conc_proto_name_arr=array();
              return 1;
        }
            
       
        $this->proto_ids		= array();  // fill the $sel with concrete protocols
        $cnt		= 0;
        
        if ($tablename=="EXP") {
   
            $proto_log = $this->get_proto_log($sqlo,  $this->infox["first_obj_id"]);
            
            $len_log = sizeof($proto_log);
            if ($len_log<1) {
                $error->set( $FUNCNAME, 1, "No protocol found om first object." );
                return -2;
            }
            
            if ($len_log>1 and !$this->step_no) {
                return -1;
            }
            
            if (!$this->step_no)  return -1; // htmlFoot("Error", "Please select one protocol");
            
            
            $aprotoid = $this->_get_pra_from_STEP_NO($this->step_no);
            $this->infox["target_a_proto_id"] = $aprotoid;
        }
        
        if ($tablename=="CONCRETE_SUBST") {
            
            $proto_log = $this->get_proto_log($sqlo,  $this->infox["first_obj_id"]);
            $len_log = sizeof($proto_log);
               
            if ($len_log<1) {
                $error->set( $FUNCNAME, 1, "No protocol found om first object." );
                return -2;
            }
 
            if ($len_log>1) {
                if (!$this->step_no) return -1;
                $aprotoid = $this->_get_pra_from_STEP_NO($this->step_no);
 
            } else {
            
                // take FIRST one 
                $first_row = current($proto_log);
                $aprotoid = $first_row['ap'];
                $this->step_no = $first_row['or'];
            }
            
            $this->infox["target_a_proto_id"] = $aprotoid;
        }
        
        debugOut('aprotoid:'.$aprotoid.' step-no:'.$this->step_no, $FUNCNAME, 1);
        
        $this->conc_proto_name_arr=array();
        
        foreach( $this->ori_objects as $tmp_obj_id) {
            
            $name = obj_nice_name ( $sqlo, $tablename, $tmp_obj_id ); 
            
            if ($tablename=="CONCRETE_SUBST") {
                $suc_lib = new oCONCRETE_SUBST_proto($tmp_obj_id);
                $proto_row = $suc_lib->get_proto_by_step($sqlo, $this->step_no);
                $c_proto_tmp=$proto_row['cp'];
            }
            
            
            if ($tablename=="CHIP_READER") {
                $sqls= "select CONCRETE_PROTO_ID from $tablename where ".$tablename."_ID=".$tmp_obj_id;
                $sqlo->query("$sqls");
                $sqlo->ReadRow();
                $c_proto_tmp= $sqlo->RowData[1];
            }
            
            
            if ($tablename=="EXP") {
                
                $sqls= "select CONCRETE_PROTO_ID from EXP_HAS_PROTO ".
                    " where EXP_ID=".$tmp_obj_id. " AND STEP_NO=".$this->step_no;
                $sqlo->query("$sqls");
                $sqlo->ReadRow();
                $c_proto_tmp=$sqlo->RowData[0];
            }
            
            //
            // save concrete protocol per object
            //
            $this->proto_ids[$cnt] = $c_proto_tmp;
            $this->conc_proto_name_arr[$cnt] = $name;
            if ( $_SESSION['userGlob']["g.debugLevel"]>1 ) echo "DEBUG: obj:$tmp_obj_id name:$name proto_id:$c_proto_tmp<br>\n";
            $cnt++;
        }
        
        
        
        return 1;
    }
    
    function sess_save_settings() {
        
        $this->mainGUIobj->set_cache_val("step_no", $this->step_no);
        $this->mainGUIobj->set_cache_val("aprotoid", $this->infox["target_a_proto_id"]);
    }
    
    function analyse_protocols($sqlo) {
        global $error;
        
        // get reference protocol
        if ( !$this->ref_proto_in ) {
            $this->infox["ref_cproto_id"] = current($this->proto_ids);
        } else {
            $this->infox["ref_cproto_id"] = trim($this->ref_proto_in);
        }
        
        $this->main_comp_lib->analyseConcProtoRef($sqlo, $this->infox["ref_cproto_id"] );
        if ($error->Got(READONLY))  {
            return;
        }
        
        $this->main_comp_lib->analyseAbstractProtos( $sqlo, $this->proto_ids );
        
        // echos
        $this->infox["abstract_proto_name"] = obj_nice_name( $sqlo, "ABSTRACT_PROTO", $this->main_comp_lib->a_proto_id);
    }
    
    function analyse_protocols_part2($sqlo) {
        
        $infoarr=array();
        $infoarr[] = array("<font color=gray>analysed protocol (abstract):</font>",
            "<img src=\"images/icon.ABSTRACT_PROTO.gif\"> <b>".$this->infox["abstract_proto_name"]."</b>");
        
        $this->comp_options=array();
        $this->comp_options["show_st_notes"] = $this->compare_Opts["show_st_notes"];
        $this->comp_options["show_quant"] = $this->compare_Opts["show_quant"];
        $this->comp_options["show_subst"] = $this->compare_Opts["show_subst"];
        
        $this->step_arrayX  = $this->main_comp_lib->getStepArray();
        $this->infox['step_count']   = sizeof( $this->step_arrayX ) ;
        $this->infox['showDetails']  = 1; 
        $numProtosDiff = sizeof($this->proto_ids);
        
        if ( $this->compare_Opts["show_dprot"] ) { // show only different protocols ?
            
            //
            // calculate a new $this->conc_proto_name_arr, which contains only different protocols !
            //
            $numProtos = sizeof($this->proto_ids);
            list($this->proto_ids, $this->conc_proto_name_arr, $dummy) = 
                $this->main_comp_lib->getDiffProtos($sqlo, $this->conc_proto_name_arr, $this->proto_ids );
            $numProtosDiff = sizeof($this->proto_ids);
            
        }
        
        $this->main_comp_lib->analyseAbstractProtos( $sqlo, $this->proto_ids ); // reanalyse protocols
        
        if ( $this->compare_Opts["show_diff"] OR $this->compare_Opts["show_dprot"] ) {
            $methodText = "";
            if ( $this->compare_Opts["show_diff"] ) $methodText .= "show_only_different_steps | ";
            if ( $this->compare_Opts["show_dprot"] ) {
                $methodText .=  " Show only different protocols (show <B>".$numProtosDiff."</B> of ".$numProtos.")\n";
            }
            
            $infoarr[] = array("<font color=gray>Methods:</font>", $methodText );
            if ( $this->compare_Opts["show_dprot"] AND !$numProtosDiff) {
                cMsgbox::showBox("O.K.", "No difference between protocols ($numProtos) found.");
                echo "<br><br>\n";
                $this->infox['showDetails'] = 0;
            }
        }
        
        
        
        $tabobj = new visufuncs();
        $headOpt = array( "title" => "Analysis Info", "headNoShow" =>1);
        $headx   = array ("Key", "Val");
        $tabobj->table_out2( $headx, $infoarr,  $headOpt );
    }
    
    function show_details($sqlo) {
        
        $aopt=array();
        $aopt["nameAsImg"] = "";
        if ($this->compare_Opts["show_navert"]) $aopt["nameAsImg"] = "1";
        
        $this->main_comp_lib->proto_step_c->cproto_arr_ini( 
            $this->proto_ids, $this->conc_proto_name_arr, $this->infox["ref_cproto_id"], $this->comp_options );
        $this->main_comp_lib->proto_step_c->table_arr_init( $sqlo, $aopt );
        $this->step_arrayX  = $this->main_comp_lib->proto_step_c->getStepArray(); // TBD: where doeas it come from ???
        
        $diffCount = 0;
        $i         = 0;
        while ( $i<$this->infox['step_count'] ) {
            
            $step_nr = $this->step_arrayX[$i];
            if ( $this->compare_Opts["show_diff"] ) {
                list ( $isdiff, $dprotarr, $sub_step ) = $this->main_comp_lib->CompareProtStep( $sqlo, $this->proto_ids, $step_nr );
                
                if ($isdiff) {
                    $this->main_comp_lib->proto_step_c->outstep_arr( $step_nr, $sqlo ); // differences !!!
                    $diffCount++;
                } else {
                    $this->main_comp_lib->proto_step_c->line_finish( $step_nr, $sub_step );
                }
            } else {
                $this->main_comp_lib->proto_step_c->outstep_arr( $step_nr, $sqlo );
            }
            $i++;
        }
        echo "</table>";
        
        echo "<ul>";
        if ( $this->compare_Opts["show_diff"] ) {
            echo "<br>";
            if ( !$diffCount ) {
                $diffText = "<font color=green><B>No differences</B></font> found in step parameters.<br>\n";
            } else {
                $diffText =  "<B>".$diffCount." different step(s)</B> found in protocol steps.<br>\n";
            }
            htmlInfoBox( "Difference Message", $diffText, "", "INFO" );
        }
        echo "</ul>\n";  
    }
    
    function page_end() {
        echo "<ul><br>\n";
        $this->main_comp_lib->prefs_formshow($this->tablename, $this->go, $this->step_no, $this->compare_Opts);
        
        echo "<br>";
        htmlInfoBox( "Statistics", "", "open", "CALM" );
        $tmpname = $this->infox["abstract_proto_name"];
        
        $this->main_comp_lib->infoout("Protocol abstract", "<a href=\"edit.tmpl.php?t=ABSTRACT_PROTO&id=".
            $this->main_comp_lib->a_proto_id."\">".$tmpname."</a>");
        $this->main_comp_lib->infoout("Number of steps per protocol", $this->infox['step_count']);
        if ($this->infox["ref_cproto_id"]) $this->main_comp_lib->infoout( "Reference_protocol_ID",$this->infox["ref_cproto_id"] );
        
        htmlInfoBox( "", "", "close" );
        
        echo "<br>";
        echo "</UL>";
    }
    
    function objs_exists() {
        return sizeof($this->ori_objects);
    }
}

//***********************************************************************************************



$error      = & ErrorHandler::get(); 
$sql        = logon2( $_SERVER['PHP_SELF'] );	
//$sql2       = logon2( $_SERVER['PHP_SELF'] );

$_SESSION['s_formState']["proto.m_comp2e"]=NULL; // reset settings of other tool

$help_lib = new oPRC_m_comp_C();
$help_lib->init($sql, $_REQUEST['obj_ids']);

$prefgo = $_REQUEST['prefgo'];
if ($prefgo) {
    $setti= $_REQUEST['setti'];
    $help_lib->set_prefs($setti);
}

echo "<UL>\n"; 

$help_lib->head_links();
$help_lib->select_objects($sql);

if (empty($help_lib->objs_exists())) {
    htmlFoot("Error", "No objects were selected.");
}

$help_lib->get_proto_log($sql,  $help_lib->infox["first_obj_id"]);


$proto_answer = $help_lib->get_protocols($sql);
if ($error->Got(READONLY))  {
    $help_lib->main_comp_lib->exitError();
}

if ( $_REQUEST['sub_action']=='show_pra_sel') {
    $help_lib->form_sel_pra($sql);
    return;
}


if ($proto_answer==-1) {
    cMsgbox::showBox("warning", "Please select a protocol.");
    echo "<br>\n";
    $help_lib->form_sel_pra($sql);
    return;
}


$help_lib->sess_save_settings();
$help_lib->analyse_protocols($sql);
if ($error->Got(READONLY))  {
    $help_lib->main_comp_lib->exitError();
}

$help_lib->analyse_protocols_part2($sql);

echo "</UL>";  // remove all INSERTS

if ( $help_lib->infox['showDetails'] ) {
    $help_lib->show_details($sql);
} 	

$help_lib->page_end();


htmlFoot();
