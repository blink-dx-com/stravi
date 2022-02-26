<?php
/** 
 - compare/update single concrete_proto_steps of experiments / substances /  protocols
 - save some vars in $_SESSION['s_formState']["proto.m_comp2e"]
        'parx'
        'step'
 - save initial compare-params in 
 - show "select step" - GUI ($selsteps=1)
 - if ONE step selected: allow update of PROTOCOL_STEP with same value
 * @swreq UREQ:0001101: g > compare protocols of selected objects 
 * @package obj.concrete_proto.m_comp2e.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param  
 *   [$go]  0, 
  			1 : do update
	  $tablename			: name of object-type:
  								"EXP", "CONCRETE_PROTO", "CONCRETE_SUBST", "CHIP_READER", "W_WAFER"
      
	  [$selsteps]			: 0 : nothing
	  						  1 : select single steps
	  $sel[OBJ_ID]		    : [OBJ_ID]=1 ;  if ($go>0) => the sel[] must be given as parameter
      $editmode 			: "edit", "view"
      $proj_id       		 : alterantive: if not  "tablename" ... (use experiments in project)
	  
	  $parx
    	  "ref_exp"      : ID of reference object
          "proto_no"     : STEP_NO of exp_has_proto or W_WAFER_STEP or EXP_HAS_PROTO 
          "proto_step_nr": step_nr for  $action
          "action"]      : 
              ["normal"] - individual values per protocol
               "selpra"  - select PRA
    	  	   "all_same"  - update of same values
    					 - need: $parx["a_proto_id"] 
    		   protocomp2 - xxx
          "a_proto_id"   : ID of TARGET abstract_protocol_id  
      
	  $parx3
	    "format"    : [html] , "csv", "print"
	    "direct"	: [vertical], horizontal
      $step[]               : array of STEP_NR in concrete_proto_step
							: array[STEP_NR] = 1
	  $ref_obj_id			: overwrites $parx["ref_exp"]
	  $fromCache		    : =1 => get ALL parameters from cache $_SESSION['s_formState']["proto.m_comp2e"]:
  								allParam = array(
								 "proj_id" => $proj_id
								)
	  $step_no    ??? - still used ?
	  

      #  params to update, where the index = object_id
      $quanti    [OBJ][step]
      $concsubst [OBJ][step]
	  $devids    [OBJ][step]
      $newnote   [OBJ][step]
      $not_done  [OBJ][step]
  GLOBAL_VARS: $_SESSION['s_formState']["proto.m_comp2e"]
  			parx[a_proto_id] ... also
 */

session_start(); 



require_once ('reqnormal.inc');
require_once ( "javascript.inc" );
require_once ('f.msgboxes.inc');
require_once ("sql_query_dyn.inc"); 
require_once ("f.head_proj.inc");
require_once ('func_form.inc');
require_once ("down_up_load.inc");

require_once ("o.PROTO.subs.inc");
require_once ("gui/o.PROTO.stepout1.inc");
require_once ("gui/o.PROTO.stepout2.inc");
require_once ('o.ABSTRACT_PROTO.stepx.inc');
require_once  'o.CS_HAS_PR.subs.inc';

require_once("subs/obj.concrete_proto.cmpgui.inc");
require_once('subs/obj.concrete_proto.m_comp2e.su1.inc');

class oPROTOcomp_Static {
    
    /**
     * get ONE protocol
     * @param object $sql
     * @param int $one_obj_id
     * @param int $step_no
     * @return int
     */
    static function get_pra(&$sql, $tablename, $one_obj_id, $step_no)  {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        
        $sqls      = "";
        $a_proto_id= 0;
        
        switch ($tablename) {
            case "EXP":
                if ( !$step_no) {
                    echo "ERROR: you must select an protocol !!!<br>";
                    return;
                }
                $sqls= "select c.ABSTRACT_PROTO_ID, ec.CONCRETE_PROTO_ID  from EXP_HAS_PROTO ec, CONCRETE_PROTO c ".
                    " where ec.EXP_ID=".$one_obj_id. " AND ec.STEP_NO=".$step_no . " AND ec.CONCRETE_PROTO_ID=c.CONCRETE_PROTO_ID";
                break;
                
            case "CONCRETE_PROTO":
                $sqls= "select c.ABSTRACT_PROTO_ID, c.CONCRETE_PROTO_ID  from CONCRETE_PROTO c ".
                    " where c.CONCRETE_PROTO_ID=".$one_obj_id;
                break;
                
            case "CONCRETE_SUBST":
                $sqls='';
                $suc_lib = new oCS_HAS_PR_subs($one_obj_id);
                $proto_log = $suc_lib->getProtoLog($sql);
                if (is_array($proto_log)) {
                    $first_elem = current($proto_log);
                    $a_proto_id=$first_elem['ap'];
                }
                if (!$a_proto_id) {
                    $error->set( $FUNCNAME, 1, "No Protocol found on first Object." );
                    return;
                }
                
                break;
            case "CHIP_READER":
                $sqls= "select c.ABSTRACT_PROTO_ID, ec.CONCRETE_PROTO_ID  from CHIP_READER ec, CONCRETE_PROTO c ".
                    " where ec.CHIP_READER_ID=".$one_obj_id. " AND ec.CONCRETE_PROTO_ID=c.CONCRETE_PROTO_ID";
                break;
                
            default:
                $error->set( $FUNCNAME, 2, "table $tablename not allowed here" );
                return;
                
        }
        
        if (!$a_proto_id) {
            $sql->query($sqls);
            $sql->ReadRow();
            $a_proto_id=$sql->RowData[0];
        }
        
        return $a_proto_id;
    }
    
    /**
     * get ALL protocols
     * @param object $sql
     * @param int $one_obj_id
     * @return array $pra_arr of IDs
     */
    static function get_all_PRAs(&$sql, $tablename, $one_obj_id)  {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $sqls      = NULL;
        $pra_arr=array();
        
        switch ($tablename) {
            case "EXP":
                $sqls= "select c.ABSTRACT_PROTO_ID, ec.CONCRETE_PROTO_ID  from EXP_HAS_PROTO ec, CONCRETE_PROTO c ".
                    " where ec.EXP_ID=".$one_obj_id. " AND ec.CONCRETE_PROTO_ID=c.CONCRETE_PROTO_ID";
                break;
                
            case "CONCRETE_PROTO":
                $sqls= "select c.ABSTRACT_PROTO_ID, c.CONCRETE_PROTO_ID  from CONCRETE_PROTO c ".
                    " where c.CONCRETE_PROTO_ID=".$one_obj_id;
                break;
                
            case "CONCRETE_SUBST":
                $sqls='';
                $suc_lib = new oCS_HAS_PR_subs($one_obj_id);
                $proto_log = $suc_lib->getProtoLog($sql);
                if (is_array($proto_log)) {
                    foreach($proto_log as $row) {
                        $pra_arr[]=$row['ap'];
                    }
                }
                break;
            case "CHIP_READER":
                $sqls= "select c.ABSTRACT_PROTO_ID, ec.CONCRETE_PROTO_ID  from CHIP_READER ec, CONCRETE_PROTO c ".
                    " where ec.CHIP_READER_ID=".$one_obj_id. " AND ec.CONCRETE_PROTO_ID=c.CONCRETE_PROTO_ID";
                break;
                
            default:
                $error->set( $FUNCNAME, 2, "table $tablename not allowed here" );
                return;
                
        }
        
        if (empty($pra_arr)) {
            if ($sqls!=NULL) {
                $sql->query($sqls);
                while ($sql->ReadRow() ) {
                    $pra_id = $sql->RowData[0];
                    $pra_arr[] = $pra_id;
                }
            }
        }
        
        return $pra_arr;
    }
    
    static function form_horizontal($tablename, $proj_id, $step, $parx, $parx3) {
        // do http-forward
        echo '<html>'."\n";
        echo '<body>'."\n";
        echo '<form name="xtraform" ACTION="obj.concrete_proto.m_comp3.php" METHOD=POST>'."\n";
        foreach( $step as $stepx=>$dummy) {
            echo "<input type=hidden name=\"step[".$stepx."]\" value=\"1\">\n";  // add the object ID
        }
        
        if ($proj_id) echo "<input type=hidden name=\"proj_id\"  value=\"".$proj_id."\">\n";
        echo "<input type=hidden name=\"tablename\"  value=\"".$tablename."\">\n";
        echo "<input type=hidden name=\"parx[format]\"  value=\"".$parx3["format"]."\">\n";
        echo '<input type=hidden name="parx[a_proto_id]"  value="'. $parx["a_proto_id"].'">'."\n";
        echo '<input type=hidden name="parx[ref_exp]"  value="'.  $parx["ref_exp"].'">'."\n";
        echo "</form>\n";
        echo "\n";
        echo '<script type="text/javascript">',"\n";
        echo 'document.xtraform.submit();',"\n";
        echo '</script>',"\n";
        echo '</html>'."\n";
        
    }
    
    static function form_select_pra($sqlo, $tablename, $pra_ids) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $tablename='ABSTRACT_PROTO';
        
        if (empty($pra_ids)) {
            $error->set( $FUNCNAME, 1, 'No protocols to select from.' );
            return;
        }
        
        $pra_init=array();
        foreach($pra_ids as $pra_id) {
            $nice = obj_nice_name ( $sqlo, $tablename, $pra_id );
            $pra_init[$pra_id] = $nice;
        }
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
            debugOut('(831) pra_init:'.print_r($pra_init,1), $FUNCNAME, 1);
        }
        
        
        $initarr   = NULL;
        $initarr["action"]      = $_SERVER['PHP_SELF'];
        $initarr["title"]       = "Select ".tablename_nice2($tablename);
        $initarr["submittitle"] = "Select";
        $initarr["tabwidth"]    = "AUTO";
        
        $hiddenarr = NULL;
        $hiddenarr["tablename"]     = $tablename;
        $hiddenarr["selsteps"]     = 1;
        
        $formobj = new formc($initarr, $hiddenarr, 0);
        
        $fieldx = array (
            "title" => tablename_nice2($tablename),
            "name"  => "a_proto_id",
            "object"=> "select",
            "val"   => '',
            "inits" => $pra_init,
            "notes" => ""
        );
        $formobj->fieldOut( $fieldx );
        
        $formobj->close( TRUE );
        
    }
    
    static function form_protocomp2($sqlo, $pra_ids) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $tablename='ABSTRACT_PROTO';
        
        if (empty($pra_ids)) {
            $error->set( $FUNCNAME, 1, 'No protocols to select from.' );
            return;
        }
        
        $pra_init=array();
        foreach($pra_ids as $pra_id) {
            $nice = obj_nice_name ( $sqlo, $tablename, $pra_id );
            $pra_init[$pra_id] = $nice;
        }
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
            debugOut('(831) pra_init:'.print_r($pra_init,1), $FUNCNAME, 1);
        }
        
        
        $initarr   = NULL;
        $initarr["action"]      = 'p.php';
        $initarr["title"]       = "Select ".tablename_nice2($tablename);
        $initarr["submittitle"] = "Select";
        $initarr["tabwidth"]    = "AUTO";
        
        $hiddenarr = array();
        $hiddenarr['mod'] = 'DEF/o.CONCRETE_PROTO.li_search';
        $formobj   = new formc($initarr, $hiddenarr, 0);
        
        $fieldx = array (
            "title" => tablename_nice2($tablename),
            "name"  => "pra_id",
            "object"=> "select",
            "val"   => '',
            "inits" => $pra_init,
            "notes" => ""
        );
        $formobj->fieldOut( $fieldx );
        
        $formobj->close( TRUE );
        
    }
}


class oPROTOcompSelstep extends oProtoCompLib {

  
	var $a_arr;
	var $proj_id;
	var $tablename;
	var $parx;
	var $parx3; // not saved in session var
	var $protoShowObj;
	var $tablenice; 
	var $csvKeys;
	var $step; // array of CONCRETE_PROTO_STEP : array[STEP_NR] of (0,1)
	# private $first_a_proto;
	public $infox=array();
	
    function __construct($proj_id, $tablename, $parx, $parx3, $step, $editmode) {
    	
    	
    	$this->proj_id   = $proj_id;
    	$this->tablename = $tablename;
    	$this->parx = $parx;
    	$this->parx3= $parx3; // not saved in session var
    	$this->step = $step; 
    	$this->protoShowObj = new oProtocolShowC();
    	$this->tablenice    = tablename_nice2($tablename); 
    	$this->csvKeys = array("objid", "stepnr", "inact", "quant", "SUBST_NAME", "exist", "notes");
    	$this->infox = array();
    	
    	parent::__construct($proj_id, $tablename, $parx, $step);
    	$this->oProtoOrgLib = new gProtoOrg();
    	
    	// set editmode
    	if ( isset($editmode) )   
    		$_SESSION['s_sessVars']['o.CONCRETE_PROTO.editmode'] = $editmode;
    	$editmode  = $_SESSION['s_sessVars']['o.CONCRETE_PROTO.editmode'];
    	if ( $editmode == "" )  $editmode = "view";
    	$this->editmode = $editmode;
    	
    	if ( $parx3["format"] == "print" ) {
    		$this->editmode = "view";
    	}
    	
    }
    
    function outJavascript() {
    	$this->protoShowObj->writeJavascript();
    }
    
    function get_ref_obj() {
        return $this->parx["ref_exp"];
    }
    
    /**
     * set $this->refobj
     * @param object $sql
     * @return array
     */
    function get_ref_obj_inf( &$sql) {
    	$ref_obj_id = $this->parx["ref_exp"];
    	$proto_arr_loop = $this->obj_one_proto_get( $sql, $ref_obj_id );
    	$ref_protoid       =$proto_arr_loop['cpid'];
    	$a_proto_typical_id=$proto_arr_loop['apid'];
    	$a_proto_name      =$proto_arr_loop['ap_name'];
    	
    	$this->refobj = NULL;
    	$this->refobj["cpid"] = $ref_protoid;
    	return array($a_proto_typical_id, $a_proto_name);
    }
    
    function setGo($go) {
    	$this->go = $go;
    }
    
    function setParxVal( $key, $val ) {
    	$this->parx[$key] = $val;
    }
    
    function info_out ( $key, $val ) {
        echo "<font color=gray>$key:</font> <B>$val</B><br>\n";
    }
    function info_out2 ( $key, $val ) {
        echo "<tr><td><font color=gray>$key:</font></td><td><B>$val</B></td></tr>\n";
    }   
    function set_target_a_proto_id($target_a_proto_id) {
    	$this->target_a_proto_id = $target_a_proto_id;
    }
    function set_a_proto_id($a_proto_id) {
        $this->parx['a_proto_id'] = $a_proto_id;
    }
    
    function setOneObject($one_obj_id)  {
        $this->one_obj_id=$one_obj_id;
        if (!$this->parx["ref_exp"]) {
            $this->parx["ref_exp"] = $this->one_obj_id;
        }
    }
    
    function showHeadPrintLinks () {
    	echo " <b>print view</b> | ";
    	echo " [<a href=\"".$_SERVER['PHP_SELF']."?proj_id=".$this->proj_id."&parx3[format]=html\">normal view</a>]";
    	echo "<br>";
    }
    
    function showHeadLinks() {
     	
    	$tablename = $this->tablename;
    	
    	echo "<UL>\n";
    
    	$base_url = $_SERVER['PHP_SELF']."?proj_id=".$this->proj_id.'&tablename='.$tablename;
    	$tmpout = formc::editViewBut( $this->editmode, 1, $base_url, "editmode" );
    	echo $tmpout;
    	
    	echo "&nbsp;[<a href=\"".$base_url."&selsteps=1\">Select other steps</a>] ";
    	echo " [<a href=\"".$base_url."&parx3[format]=print\">print view</a>]";
    	// echo " &nbsp;&nbsp;&nbsp;[<a href=\"obj.exp.imp_sample.php?tablename=".$tablename."\">ProtoImporter: import parameters</a>]";
    	echo "<br><img src=0.gif height=5 width=1><br>\n";
    	
    	if ( $_SESSION['userGlob']["g.debugLevel"]>0 ) {
    		echo "<B>INFO:</B> DEBUG-mode active<br>\n";
    	}
    }
    
    
    
    
    
    /**
     * select single steps, show a GUI
     * @param object $sql
     * @param int $step_no
     * @param int $one_obj_id
     * @param int $aprotoid  
     */
    function selSingleSteps( &$sql, &$sql2, $step_no, $one_obj_id, 	$a_proto_id=NULL ) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	    
    	$tablename = $this->tablename;
    	$pra_table_nice = tablename_nice2('ABSTRACT_PROTO');
    	echo '[<a href="'.$_SERVER['PHP_SELF'].'?tablename='.$tablename.'&parx[action]=selpra">Select other '.$pra_table_nice.'</a>]<br><br>'."\n";
    	
    	// $a_proto_id = $this->get_pra($sql, $one_obj_id, $step_no);
    // 	if ($error->Got(READONLY))  {
    // 	    $error->set( $FUNCNAME, 1, '' );
    // 	    return;
    // 	}
    	
        if ($a_proto_id) {  
        
            echo "<br><B><font color=blue size=+1>Select single steps from ".$pra_table_nice ."</font></B><br><br>";
            // if ($tablename=="EXP") echo "<font color=gray>Step_No inside the experiment log:</font> $step_no<br>\n\n";
            
            $options=array();
            $options["checkboxes"] = 1;
            echo '<form name="xtraform" ACTION="'.$_SERVER['PHP_SELF'].'?parx[ref_exp]='.$one_obj_id.'" METHOD=POST>';  //OLD: &parx[proto_no]='.$step_no. '
    
            $sqls= "select name  from  ABSTRACT_PROTO  where ABSTRACT_PROTO_ID=".$a_proto_id;
            $sql->query($sqls);
            $sql->ReadRow();
            $a_proto_name=$sql->RowData[0];
            echo "<font color=gray>".$pra_table_nice.":</font> <B>$a_proto_name</B> [ID:$a_proto_id]<br>\n";
            $this->protoShowObj->showAll( $sql, $sql2, 0, $a_proto_id, $options ); 
            echo "<input type=hidden name=tablename value=\"".$tablename."\">";
    		if ($this->proj_id) echo "<input type=hidden name=proj_id value=\"".$this->proj_id."\">";
    		
    		$feld = array("html"=>"normal", "csv"=> "Excel", "print"=>"printer");
    		$selopt = formc::selectFget("parx3[format]",	$feld, ""); 
    		
    		$feld = array("vertical"=>"[vertical]", "horizontal"=> "horizontal");
    		$selopt2 = formc::selectFget("parx3[direct]",	$feld, "vertical"); 
    		
            echo '<input type=submit class="yButton" value="Compare steps"> ';
    		echo "&nbsp;&nbsp; &nbsp;optional: Format: ".$selopt;
    		echo "&nbsp;&nbsp; &nbsp;Direction: ".$selopt2;
    		echo "<br>";
            echo "</form>\n";
        } else {   
            
            $error->set( $FUNCNAME, 3, "first object must contain a protocol (concrete)" );
            return;
        }
        return;
    
    }
    
    function isHtmlFormat() {
    	if ( $this->parx3["format"]=="html") {
    		return (1);
    	} else return (0);
    }
    
    private function proto_info( &$sql, $c_proto_id, $a_proto_tmp, $a_proto_typical_id, $proto_step_nr ) {
        // RETURN: $c_arr : is_array() => STEP exists 
        //                  NULL       => STEP NOT exists 
        $wrong_aproto = 0;
        $c_arr        = NULL;
        $invalid      = "";
        $tcolor       = "";
        
        if ($c_proto_id) {  
            if ($a_proto_tmp==$a_proto_typical_id) { 
                  $c_arr = $this->oProtoOrgLib->cproto_step_info( $sql, $c_proto_id, $proto_step_nr );
                  if ($c_arr["NOT_DONE"]==1) $tcolor="bgcolor=#E0C0C0";
            } else {
                // abstract protos different
                $invalid="different protocol: $a_proto_tmp";
                $tcolor ="bgcolor=#E0C0C0";
                $wrong_aproto = 1;
            }
        } else {        
            $invalid="no protocol";
            $tcolor ="bgcolor=#E0D0D0";
            $wrong_aproto = 1;
        }
        return ( array($invalid, $tcolor, $c_arr, $wrong_aproto) ); 
    }
    
    function getAbsProtoSteps(&$sql, &$step) {
    	// FUNCTION: store parameters of selected abstract_proto_steps
    	// OUTPUT:   $this->a_arr
    
    	$aprotoid = $this->infox["target_a_proto_id"];
    	$aProtoSublib = new oABSTRACT_PROTO_stepx();
    	$aProtoSublib->setObj($aprotoid);
    
    	$this->a_arr = NULL;
    	foreach( $step as $th0=>$th1) { 
    	
    		$proto_step_nr = $th0;
    		$retarr = $aProtoSublib->step_info( $sql, $proto_step_nr, 1);
    	
    		if ( $retarr == NULL ) {
    			htmlFoot( "ERROR", "Step_ID: <B>$proto_step_nr</B> does not exist.");
    		}
    		$this->a_arr[] = $retarr;
    	
    	}
    	
    }
    
    function _InfoProtoStepsOnly(&$sql, $infox ) {
    	$tmpcnt = 0;
    	$step = &$this->step;
    	
    	
    	foreach( $step as $dummy) {
    		
    		$a_subst_name = $this->a_arr[$tmpcnt]["ABSTRACT_SUBST_NAME"]; 
    		$a_dev_name   = $this->a_arr[$tmpcnt]["ABS_DEV_NAME"];
    		
    		$info3  = "";
    		if ($a_subst_name!="")  $info3  .=  "Subst: <b>".$a_subst_name."</B>";
    		if ($a_dev_name!="")    $info3  .=  " Dev: <b>".$a_dev_name."</B>";
    		
    		if ($this->a_arr[$tmpcnt]["QUANTITY"]!="") $info3 .= " Quant: <b>".$this->a_arr[$tmpcnt]["QUANTITY"]."</B>";
    		$notes = $this->notes_make_html($this->a_arr[$tmpcnt]["NOTES"]);   
    		if ($notes!="") $info3 .= " Notes: <I>".$notes."</I>";
    	
    		
    		$this->info_out2( "&nbsp;&nbsp;<b>#".($tmpcnt+1)."</b> ".
    			"[STEP:".$this->a_arr[$tmpcnt]["STEP_NR"]."]", $this->a_arr[$tmpcnt]["NAME"].
    			"</B> &nbsp;".$info3);   
    		$tmpcnt++;    
    	}
    	
    }
    
    function protoStepInfo( &$sql, $infox ) {
    	// FUNCTION: show proto step info
    	
    	htmlInfoBox( "Selected protocol steps", "", "open", "CALM" );
    	
    	echo "<table cellpadding=1 cellspacing=1 border=0>";
    	$this->info_out2( tablename_nice2('ABSTRACT_PROTO'), "<img src=\"images/icon.ABSTRACT_PROTO.gif\"> ".$infox["target_a_proto_name"]." <font color=gray>[ID:". $infox["target_a_proto_id"]."] </font>");       
    	$this->_InfoProtoStepsOnly($sql, $infox );
    	echo "</table>";
    	htmlInfoBox( "", "", "close" );
    	echo "<br>";
    }
    
    function notes_make_html($notes) {      
        if ( strlen($notes) > 40 ){ 
            $notes = substr( $notes, 0, 40 ) . "&nbsp;...";
        }
        return ($notes); 
    } 
    
    function bulkPowerInit() {
    	// need proj_id
    	$tmpProjObj = new gHtmlHeadProjC();
    	// get selected objects from the stored $_SESSION['s_tabSearchCond']
    	$this->sqlAfter    = $tmpProjObj->getSqlAfter($this->tablename, $this->proj_id);
    	return ($this->sqlAfter);
    }
    
    function _cvsLine($datx) {
    	$tmptab="";
    	
    	foreach( $this->csvKeys as $key) {
    		echo $tmptab . $datx[$key];
    		$tmptab="\t";
    	}
    	
    	echo "\n";
    }
    
    function showSteps ( &$sql, &$sel, &$infox, &$parx, &$step ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	$a_arr = &$this->a_arr;
    	$tablename  = $this->tablename;
    	
    	debugOut('(500) step-nrs:'.print_r($step,1). ' infox:'.print_r($infox,1), $FUNCNAME, 2);
    	
    	if ($this->parx3["format"]=="csv") {
    	
    		$headkeys = array("objid"=>"OBJID", "stepnr"=>"stepnr", "inact"=>"inact", "quant"=>"quant",
    			 "SUBST_NAME"=>"SUBST_NAME", "DEV_NAME"=>"DEV_NAME", "exist"=>"exist", "notes"=>"notes");
    		$this->_cvsLine($headkeys);
    		
    	} else {
    		
    		$tableBorder="0";
    		if ($this->parx3["format"]=="print") {
    			$tableBorder=1;
    		}
    		
    		echo "<table bgcolor=#EFEFEF cellpadding=1 cellspacing=1 border=".$tableBorder.
    			 " style=\"empty-cells:show\">\n
    				<tr bgcolor=#D0D0D0>
    				<td><B>".$this->tablenice."</B></td>"; 
    		echo "  <td><B>#</B></td>";
    		echo "  <td><B></B></td>";
    	
    		if ( $infox["step_cnt"] > 1 )   {  // more steps ???
    			echo "<td><B>Step name</B></td>";
    		} 
    	
    		echo "  <td><B>Substance</B></td>
    				<td><B>Device</B></td>
    				<td><B>Quant</B></td>
    				<td><B>Notes</B></td>
    				</tr>\n";
    	}
        $badAProtoCnt      = 0;
    	$total_proto_steps = 0; 
        $numSteps = sizeof($step);
     
    	foreach( $sel as $obj_id=>$dummy) {
    		
    	    $proto_arr_loop = $this->obj_one_proto_get( $sql, $obj_id, $parx["a_proto_id"] );
    	    $obj_name    =$proto_arr_loop['name'];
    	    $c_proto_id  =$proto_arr_loop['cpid'];
    	    $a_proto_tmp =$proto_arr_loop['apid'];
    	   
    	    
    	    debugOut('(549) obj_id:'.$obj_id.' c_proto_id:'.$c_proto_id. ' a_proto_tmp:'.$a_proto_tmp, $FUNCNAME, 2);
    		
    		$protoStepCnt = 0;
    		foreach( $step as $proto_step_nr_tmp=>$dummy) {
    	
    			$existIcon = "";
    			list ( $invalid, $tcolor, $c_arr, $flagBadAproto ) = $this->proto_info( $sql, $c_proto_id, $a_proto_tmp, $infox["target_a_proto_id"], $proto_step_nr_tmp );
    	
    			if ( $flagBadAproto ) $badAProtoCnt++;
    			if ($this->parx3["format"]=="csv") {
    				$datx=NULL;
    				if ( $c_arr["NOT_DONE"]==1) $datx["inact"] = 1;
    				if ( is_array($c_arr) )  $datx["exist"] = 1;
    				$datx["objid"]  = $obj_id;
    				$datx["stepnr"] = $a_arr[$protoStepCnt]["STEP_NR"];
    				$datx["notes"]  = $c_arr["NOTES"];
    				
    				$datx["quant"] = $c_arr["QUANTITY"];
    				if ($c_arr["QUANTITY"]=="") {
    					$datx["quant"] = $a_arr[$protoStepCnt]["QUANTITY"];
    				}
    				$datx["SUBST_NAME"] = $c_arr["SUBST_NAME"];
    				$datx["DEV_NAME"]   = $c_arr["SUBST_NAME"];
    				
    				$this->_cvsLine($datx);
    			
    			} else {
    			
    				if ( is_array($c_arr) ) $existIcon = "<IMG src=\"images/but.gocp13.gif\" border=0>";
    		
    				if (!$protoStepCnt && ($infox["step_cnt"]>1)) {  // first line ??? 
    					if ($tcolor=="") $tcolor="bgcolor=#C0C0C0";
    				}
    				echo "<tr $tcolor><td>";
    				if (!$protoStepCnt){ 
    					echo "<a href=\"edit.tmpl.php?t=".$tablename."&id=$obj_id\">$obj_name</a>";
    				}  else { 
    					echo "&nbsp;";
    				} 
    		
    				if  ($c_arr["NOT_DONE"]==1) echo " <font color=red>inactive</font>";
    				if  ($invalid!="") echo " <font color=red>$invalid</font>";
    				echo "</td>";
    				echo "<td>";
    				if ($numSteps>1) echo "#". ($protoStepCnt+1);
    				echo "</td>";
    				echo "<td>". $existIcon ."</td>";
    		
    				if ($infox["step_cnt"]>1) echo "<td><font color=gray>".$a_arr[$protoStepCnt]["NAME"]."</font></td>"; // Step-name
    				$notes = $this->notes_make_html($c_arr["NOTES"]);
    				
    				$quantiout = $c_arr["QUANTITY"];
    				
    				if ($c_arr["QUANTITY"]=="") {
    					$quantiout = $a_arr[$protoStepCnt]["QUANTITY"];
    				} else {
    					$quantiout = "<B>".$c_arr["QUANTITY"]."</B>";
    				}
    				
    				
    				echo "<td>";
    				if ($c_arr["CONCRETE_SUBST_ID"])
    					echo "<B>".$c_arr["SUBST_NAME"]."</B> [ID:".$c_arr["CONCRETE_SUBST_ID"]."]";
    				echo "</td>";
    				echo "<td>";
    				if ($c_arr["DEV_ID"])
    					echo "<B>".$c_arr["DEV_NAME"]."</B> [ID:".$c_arr["DEV_ID"]."]";
    				echo "</td>";
    				echo "<td>"	  .$quantiout.	"</td>";
    				echo "<td>"	  .$notes .				"&nbsp;</td>";    
    				echo "</tr>\n";
    			}
    			$protoStepCnt++; 
    			$total_proto_steps++; 
    		}
    		
    		
    	}
    	
    	if ($this->parx3["format"]!="csv") echo "</table>\n\n";
    	
    	$retinfo = array ( "badAProtoCnt" => $badAProtoCnt, "total_steps" => $total_proto_steps);
    	return ($retinfo);
    }
    
    function showRefProto( &$sql, &$sql2, $infox) {
    	// show REFERENCE protocol
    	// INPUT: $this->refobj["cpid"]
    			
    	$cpid = $this->refobj["cpid"];
    	if ( !$cpid ) {
    		htmlErrorBox( "Error", "No reference-protocol given." );
    		return;
    	} 
    	$optname=array("noID"=>1);   
    	$protoname = obj_nice_name ( $sql, "CONCRETE_PROTO", $cpid, $optname );
    	echo "<br>";
    	htmlInfoBox( "Reference protocol (concrete)", "", "open", "CALM" );
    	echo "<table cellpadding=1 cellspacing=1 border=0>";
    	$this->info_out2( "Name", $protoname. " [ID:$cpid]");       
    	$this->info_out2( tablename_nice2('ABSTRACT_PROTO'), "<img src=\"images/icon.ABSTRACT_PROTO.gif\"> ".$infox["target_a_proto_name"]." <font color=gray>[ID:". $infox["target_a_proto_id"]."] </font>");       
    	echo "</table>";
    	htmlInfoBox( "", "", "close" );
    	echo "<br>";
    	
    	$options=array();
    	$options["out_type"] = "printer";
    	$protoShowObj = new oProtocolShowC();
    	$a_proto_id   = 0;
    	$protoShowObj->showAll( $sql, $sql2, $cpid, $a_proto_id, $options );
    	echo "<br>\n";
    	
    	htmlInfoBox( "Selected steps", "", "open", "CALM" );
    	echo "<table cellpadding=1 cellspacing=1 border=0>";
    	$this->_InfoProtoStepsOnly( $sql, $infox );
    	echo "</table>";
    	htmlInfoBox( "", "", "close" );
    	echo "<br>";
    }
    
    
    function showButSameVal( $stepcnt ) {
    	// FUNCTION: show special button
    
    	
        echo '<input type="button" class="yButton" value="Update steps with same value &gt;&gt;" onclick="document.formUp2.submit();">'. "\n";  
    	#} else {
    	#    echo '<span style="color:blue;">Info:</span> Select only ONE step to Update ALL with SAME values.';
    	#}
    	
    	echo '<form style="display:inline;" method="post"  name="formUp2"   action="'.$_SERVER['PHP_SELF'].'" >'."\n";
    	echo "<input type=hidden name =\"parx[action]\"  value=\"all_same\">\n";
    	echo "<input type=hidden name =\"proj_id\"       value=\"".$this->proj_id."\">\n";
    	echo "<input type=hidden name =\"tablename\"     value=\"".$this->tablename."\">\n";
    	echo "<input type=hidden name =\"parx[ref_exp]\"        value=\"".$this->parx["ref_exp"]."\">\n";
    	echo "<input type=hidden name =\"parx[proto_no]\"       value=\"".$this->parx["proto_no"]."\">\n";
    	echo "<input type=hidden name =\"parx[proto_step_nr]\"  value=\"".$this->parx["proto_step_nr"]."\">\n";
    	echo "<input type=hidden name =\"parx[a_proto_id]\"  value=\"".$this->target_a_proto_id."\">\n";
    	foreach( $this->step as $key=>$val) {
    	    echo "<input type=hidden name =\"step[".$key."]\"  value=\"".$val."\">\n";
    	}
    	echo "</form>\n"; 
    	
    }
    
    
    /**
     * scan ALL objects, get first C_PROTO_ID 
     * @param object $sql
     * @param array $sel
     */
    function manage_first_proto($sql, $sel ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        
        foreach($sel as $objid=>$val) {
            $proto_info = $this->obj_one_proto_get( $sql, $objid  );
            if ($proto_info['cpid']) {
                break; // found
            }
        }
        $this->infox['first_c_proto'] = $proto_info['cpid'];
        
        if (!$proto_info['cpid']) {
            $error->set( $FUNCNAME, 1, 'No Object with a conceret protocol found !' );
            return;
        }
    }



}

class oPROTOcomp_Forms {
    
    var $proj_id;
    var $tablename;
    var $parx;
    
    function __construct( $proj_id, $tablename, $parx, $step ) {

        $this->proj_id   = $proj_id;
        $this->tablename = $tablename;
        $this->parx = $parx;
       
        $this->compare_help = new oProtoCompLib($proj_id, $tablename, $parx, $step);
    }
    
    private function _add_hidden($sel) {
        // add sel[] at the end to leave the html-INPUT fields in the original order for the edit-form actions
        foreach( $sel as $obj_id=>$dummy) {
            echo "<input type=hidden name=\"sel[".$obj_id."]\" value=\"1\">\n";  // add the object ID
        }
        
        echo '<input type=hidden name="tablename"  value="'.$this->tablename.'">'."\n";
        echo '<input type=hidden name="go"  value="1">'."\n";
        echo "<input type=hidden name=\"parx[a_proto_id]\"     value=\"".$this->parx["a_proto_id"]."\">\n";
        if ($this->proj_id) echo "<input type=hidden name=\"proj_id\"  value=\"".$this->proj_id."\">\n";
    }
    
    function form_edit_steps($sql, $sql2, &$sel, &$step, $infox, $updateInfo) {
        
        if (!$infox["target_a_proto_id"]) {
            cMsgbox::showBox("error", "Input-Error: PRA missing.");
            return;
        }
        
        $tablename = $this->tablename;
        $tabIcon   = "<img src=\"images/icon.".$tablename.".gif\">";
        //$parx2["a_proto_id"] = $infox["first_a_proto"]; // define the template protocol
        //$editAllowed = 1;
        echo "\n\n";
        echo '<form name="xtraform" ACTION="'.$_SERVER['PHP_SELF'].'" METHOD=POST>';
        $xopt=array();
        $xopt["noStepScan"]     = 0; # olD: 1
        $xopt["abstrStepsShow"] = 1; // TBD: !!!
        $xopt["linexHead"]  = 1;
        $xopt["showStepNo"] = 1; 
        $protoobj = new protostep( $sql, $sql2, $infox["target_a_proto_id"], 1, 0, $infox['first_c_proto'], $xopt );
        if ($protoobj==NULL) {
            cMsgbox::showBox("error", "Internal: could not init object protostep()");
            return;
           
        }
        
        $badAProtoCnt      = 0;
        $total_proto_steps = 0;
        $protoobj->table_init( ); // tableSingStep_init( $tablenice )
        $inf_colspan = $protoobj->colspan;
        if (!$inf_colspan) $inf_colspan = 7;
        $objectCnt   = 0;
        $rememberProto=array();
        
        foreach( $sel as $obj_id=>$dummy) {
            
            $proto_arr_loop = $this->compare_help->obj_one_proto_get( $sql, $obj_id, $infox["target_a_proto_id"] );
            $c_proto_id =$proto_arr_loop['cpid'];
            $obj_name   =$proto_arr_loop['name'];
            
            
            $extraInfo   = "";
            $lineProblem = 0;
            $problemText = "";
            
            if (!$c_proto_id) {
                $lineProblem = 1;
                $problemText = " <font color=red>Warning:</font> contains no protocol";
                if ($tablename=="EXP") $problemText .= " =&gt; <a href=\"obj.exp.protocol_comp.php\">use this tool</a>";
            } else {
                if ( $rememberProto[$c_proto_id] > 0 ) {
                    $problemText = " <font color=red>Warning:</font> more than one protocol with ID:$c_proto_id found in list. Edit not allowed.";
                    $lineProblem = 1;
                }
            }
            // TBD: check, if protocol is used by other objects !!!
            $loopObjInfo = $tabIcon. "<a href=\"edit.tmpl.php?t=".$tablename."&id=".$obj_id."\">".$obj_name."</a>";
            
            if (!$lineProblem) {
                $protoStepCnt = 0;
                
                if ($infox["step_cnt"]>1) { // object info in extra line, if more than ONE step per object
                    echo "<tr bgcolor=#E8E8FF><td colspan=".$inf_colspan.">&nbsp;".$loopObjInfo."</td></tr>\n";
                }
                
                foreach( $step as $proto_step_nrx=>$dummy) {
                    $tmpSubStep="";
                    $protoobj->conc_proto_id = $c_proto_id;  		  // set other protocol
                    $protoobj->main_step_cnt = $protoStepCnt+1;  // for visualization
                    $protoobj->sub_step_cnt  = $protoStepCnt+1;  // for visualization
                    if ($infox["step_cnt"]>1) $tmpSubStep = " <font color=gray>step ".($protoStepCnt+1)."</font>";
                    $xopt["xtratxt"] = "";
                    if (!$protoStepCnt AND $infox["step_cnt"]<=1) $xopt["xtratxt"]  = $loopObjInfo;
                    if ($infox["step_cnt"]>1) $xopt["xtratxt"]   .= "&nbsp;&nbsp;".$tmpSubStep." ";
                    $xopt["xtratxt"]        .= $updateInfo[$obj_id].$extraInfo; // print also the update_info
                    $xopt["step_nr_par"]     = $obj_id."][".$proto_step_nrx; // TBD: !!!
                    $protoobj->outstep( $proto_step_nrx, $sql, $xopt );
                    $rememberProto[$c_proto_id] = 1;
                    $protoStepCnt++;
                    $total_proto_steps++;
                }
                
                
            } else  {
                echo "<tr bgcolor=#FFC0C0><td colspan=".$inf_colspan.">".$loopObjInfo;
                echo " ".$problemText."</td></tr>\n";
            }
            $objectCnt ++;
        }
        
        
        $infores = array ( "badAProtoCnt" => $badAProtoCnt, "total_steps" => $total_proto_steps);
        
       
 
        echo "<tr bgcolor=#E0E0E0><td><img src=\"0.gif\" height=35 width=1></td><td valign=middle colspan=".($inf_colspan-1).">";
        echo '&nbsp;&nbsp;<input type=submit class="yButton" value="Update">'."\n";
        echo "</td></tr>\n";
        echo "</table>\n\n";
        
        $this->_add_hidden($sel); # normal
        echo '<input type=hidden name="parx[action]"  value="normal">'."\n";
        
        echo "</form>\n";
        
        return $infores;
    }
    
    /**
     * 'QUANTITY'=>'qu',
        'CONCRETE_SUBST_ID'=>'cs' ,
        'DEV_ID'  =>'dev',
        'NOTES'   =>'cn' ,
        'NOT_DONE'=>'dea',
     * @param object $sqlo
     * @param string $col
     */
    private function _cell($sqlo, $col, $step) {
        

        $col_short = oCprotoMani::REAL2SHORT_KEYS[$col];
        $check = '<input type=checkbox name="chk['.$step.']['.$col_short.']" value="1">';
        $out= $check.' ' ;
        
        $varname = 'vx['.$step.']['.$col_short.']';
        
        switch($col) {
            case 'QUANTITY':
                $out .= '<input type=text name="vx['.$step.']['.$col_short.']" value="" size=8>';
                break;
            case 'NOTES':
                $out .= '<input type=text name="vx['.$step.']['.$col_short.']" value="" size=30>';
                break;
            case 'NOT_DONE':
                
                $sel_array= array(
               
                '0'=>'is active',
                '1'=>'NOT active!'
                );
                
                $out = formc::selectFget($varname, $sel_array, NULL);
                break;
                
            case 'CONCRETE_SUBST_ID':
                
                $butID  = 'x'.$step;
                $this->jsFormLib->setTable('CONCRETE_SUBST', $butID);
                $olopt = array('butxtra'=> 'class=tbut');
                $out   .= $this->jsFormLib->getObjLnk( $varname, 0, '', $olopt);
                $osopt  = NULL;
                $out   .=  $this->jsFormLib->getObjSelector( $osopt );
                # $butIdTmp = $this->jsFormLib->butid;

                break;
                
            case 'DEV_ID':
                
                $butID  = 'xd'.$step;
                $this->jsFormLib->setTable('CHIP_READER', $butID);
                $olopt = array('butxtra'=> 'class=tbut');
                $out   .= $this->jsFormLib->getObjLnk( $varname, 0, '', $olopt);
                $osopt  = NULL;
                $out   .=  $this->jsFormLib->getObjSelector( $osopt );
                break;
        }
        return $out;
    }
    
    private function _one_step($sqlo, $one_a_step_arr) {
        $step = $one_a_step_arr['STEP_NR'];
        $data=array();
        $data[]=$one_a_step_arr['stepnr.nice'];
        $data[]=$one_a_step_arr['NAME'];
        $data[]= $this->_cell($sqlo, 'NOT_DONE', $step);
        $data[]= $this->_cell($sqlo, 'CONCRETE_SUBST_ID', $step);
        $data[]= $this->_cell($sqlo, 'DEV_ID', $step);
        $data[]= $this->_cell($sqlo, 'QUANTITY', $step);
        $data[]= $one_a_step_arr['H_UNIT.name'];
        $data[]= $this->_cell($sqlo, 'NOTES', $step);
        
        echo '<tr>'."\n";
        foreach($data as $val) {
            echo '<td style="padding-left:20px;">'.$val.'</td>'."\n";
        }
        echo '</tr>'."\n";
    }
    
    /**
     *   Update Protocols with the SAME values
     *   VARS: 
     *     chk[step][q]=0,1 (PROTO_STEP_short_keys)
     *     vx[step][q] = 123 (PROTO_STEP_short_keys)
     **/
    function form_one_VAL_all($sqlo, $sqlo2, &$sel, &$step, $infox) {
        #global $error;
        #$FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        echo '<h3>Update Protocols with the SAME values</h3>'."\n";
        
        echo "<br>";
        cMsgbox::showBox("info", "Please use the checkboxes to select values for update!");
        echo "<br>";
        
        if (!$infox["target_a_proto_id"]) {
            cMsgbox::showBox("error", "Input-Error: PRA is missing.");
            return;
        }
        if (!$infox['first_c_proto']) {
            cMsgbox::showBox("error", "Input-Error: first PRC is missing.");
            return;
        }
        
        $this->jsFormLib = new gJS_edit();
        
        $pra_id = $infox["target_a_proto_id"];
        
        $obj_cnt=sizeof($sel);

        $pra_steps = gProtoOrg::get_pra_all_steps($sqlo, $pra_id);
        gProtoOrg::stepnr_nice_add($pra_steps);
        
        echo "\n\n";
        echo '<form name="xtraform" ACTION="'.$_SERVER['PHP_SELF'].'" METHOD=POST>';
       

        echo '<table>'."\n";
        $cols= array(
            '#',
            'Description',
            'Deactivated',
            'Material NOW',
            'Equipment NOW',
            'Quantity NOW',
            'Meas. Unit',
            'Notes NOW',
        );
        echo '<tr bgcolor=#E0E0E0>'."\n";
        foreach($cols as $col) {
            echo '<th>'.$col.'</th>'."\n";
        }
        echo '</tr>'."\n";
        
        
        $protoStepCnt = 0;
  
        foreach( $step as $step_nr=>$dummy) {
            
            $one_a_step_arr= $pra_steps[$step_nr];
 
            $H_UNIT_ID  = $one_a_step_arr['H_UNIT_ID'];
            if ($H_UNIT_ID) {
                $one_a_step_arr['H_UNIT.name'] = glob_ObjDataColGet($sqlo, 'H_UNIT', $H_UNIT_ID, 'NAME');
            } 
            $this->_one_step($sqlo, $one_a_step_arr);
            
            $protoStepCnt++;
        }
        
        
        
        echo "<tr bgcolor=#E0E0E0><td colspan=".sizeof($cols).">";
        echo '&nbsp;&nbsp;<input type=submit class="yButton" value="Update '.$obj_cnt.' Object-Protocols">'."\n";
        echo "</td></tr>\n";
        echo "</table>\n\n";
        
        $this->_add_hidden($sel);
        echo '<input type=hidden name="parx[action]"  value="all_same">'."\n";
        
        echo "</form>\n";
    
    }
    
}


// --------------------------------------------------------------------
$FUNCNAME='MAIN';
$title="Compare/Update SINGLE protocol steps";

$go       =$_REQUEST['go'];
$parx     =$_REQUEST['parx'];
$parx3    =$_REQUEST['parx3'];
$proj_id  =$_REQUEST['proj_id'];
$ref_obj_id=$_REQUEST['ref_obj_id'];
$sel      =$_REQUEST['sel'];
$selsteps =$_REQUEST['selsteps'];
$step_no  =$_REQUEST['step_no'];
$tablename=$_REQUEST['tablename'];
$fromCache=$_REQUEST['fromCache'];
$editmode =$_REQUEST['editmode'];
//$aprotoid =$_REQUEST['aprotoid'];


$error  = & ErrorHandler::get();
$sql    = logon2( $_SERVER['PHP_SELF'] );
$sql2   = logon2( $_SERVER['PHP_SELF'] );
//$retval   = 0; 
$num_elem = 0;
$infoarr  = array();
# $infox    = NULL;   // useful variables of script

// manage form session
if (!is_array($_SESSION['s_formState']["proto.m_comp2e"])) $_SESSION['s_formState']["proto.m_comp2e"]=array();
if (!is_array($_SESSION['s_formState']["proto.m_comp2e"]['parx'])) $_SESSION['s_formState']["proto.m_comp2e"]['parx']=array();
if (!is_array($_SESSION['s_formState']["proto.m_comp2e"]['step'])) $_SESSION['s_formState']["proto.m_comp2e"]['step']=array();
if (is_array($parx)) {
    foreach($parx as $key =>$val) {
        $_SESSION['s_formState']["proto.m_comp2e"]["parx"][$key]=$val;
    }
}
if (is_array($_REQUEST['step'])) {
    $_SESSION['s_formState']["proto.m_comp2e"]["step"] = $_REQUEST['step'];
}
$action_now = 'normal';
if (is_array($parx)) $action_now = $parx['action'];

$parx = $_SESSION['s_formState']["proto.m_comp2e"]["parx"];  // get from session
$step = $_SESSION['s_formState']["proto.m_comp2e"]["step"];
$parx['action'] = $action_now;

if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
    debugOut('(909) parx:'.print_r($parx,1), $FUNCNAME, 1);
}
 

if ( $ref_obj_id ) $parx["ref_exp"] = $ref_obj_id;
if ( $fromCache ) {
	$proj_id = $_SESSION['s_formState']["proto.m_comp2e"]["allParam"]["proj_id"];
	$parx["action"] = "";
}
if ( $tablename!="" ) $parx["tablename"] = $tablename;  
$parx["proj_id"] = $proj_id;  
$tablename       = $parx["tablename"];

if ($parx3["direct"]=="horizontal" ) {
    oPROTOcomp_Static::form_horizontal($tablename, $proj_id, $step, $parx, $parx3);
    return;
	
}

if ( $parx3["format"]=="csv" ) {
	set_mime_type("application/vnd.ms-excel",  "protocolComp.txt");
}

if ($tablename == "") {
	$infoarr=array();
	$pagelib = new gHtmlHead();
	$pagelib->_PageHead ( $title,  $infoarr );
	htmlFoot("Error", "tablename missing");
}



if ($parx3["format"]=="") $parx3["format"]="html"; 

$mainScriptObj = new oPROTOcompSelstep( $proj_id, $tablename, $parx, $parx3, $step, $editmode);
$mainScriptObj->setGo($go);
$editmode = $mainScriptObj->editmode;

// manage a script header with alternativ calling from PROJ or with TABLE-selection
//   
$compGuiAll    = new oPROTO_cmpgui();
$compGuiAll->init($proj_id, $tablename);
// manage a script header with alternativ calling from PROJ or with TABLE-selection
//   


if ($parx3["format"]!="csv") { 
	$shheaOpt = NULL;
	if ($parx3["format"]=="print") $shheaOpt = array("norider"=>1);
	$obj_ids=NULL;
	
	$keyx="selected";
	if ($parx['action']=='protocomp2') {
	    $keyx="sea";
	}
	$compGuiAll->showHeader( $sql, $title,  $obj_ids, $infoarr, $keyx, $shheaOpt );
	$mainScriptObj->outJavascript();
} else {
	$sqlAfter    = $mainScriptObj->bulkPowerInit();
	$compGuiAll->setSqlAfter($sqlAfter);
}

$sqlAfterOrder  = $compGuiAll->sqlAfter;
$one_obj_id     = $compGuiAll->getOneObject($sql);
$mainScriptObj->setOneObject($one_obj_id);
$pra_arr_of_one_obj = oPROTOcomp_Static::get_all_PRAs($sql, $tablename, $one_obj_id);

if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
    debugOut('one_obj_id:'.$one_obj_id. ' ALL_PRAs:'.print_r($pra_arr_of_one_obj,1), $FUNCNAME, 1);
}

if ($parx["action"]=='selpra') {
    echo "<br><br>\n";
    oPROTOcomp_Static::form_select_pra($sql, $tablename, $pra_arr_of_one_obj);
    $error->printAll();
    htmlFoot();
    return;
}

if ($parx["action"]=='protocomp2') {
    echo "<br><br>\n";
    oPROTOcomp_Static::form_protocomp2($sql, $pra_arr_of_one_obj);
    $error->printAll();
    htmlFoot();
    return;
}

list($step_no, $parx['a_proto_id']) = $compGuiAll->anaFormState($step_no, $parx['a_proto_id']);  

if (!$parx['a_proto_id']) {
    if (empty($pra_arr_of_one_obj)) {
        htmlFoot('INFO','First object contains no protocol.');  
    }
    
    if (sizeof($pra_arr_of_one_obj)==1) {
        $a_proto_id = current($pra_arr_of_one_obj);
        $parx['a_proto_id'] = $a_proto_id;
        $mainScriptObj->set_a_proto_id($a_proto_id);
    } else { 
        echo "<br><br>\n";
        oPROTOcomp_Static::form_select_pra($sql, $tablename, $pra_arr_of_one_obj);
        $error->printAll();
        htmlFoot();
        return;
    }
}

$_SESSION['s_formState']["proto.m_comp2e"]["parx"]['a_proto_id']=$parx['a_proto_id'];


// from this point: $parx['a_proto_id'] is available ...

// list($step_no, $aprotoid) = $compGuiAll->anaFormState($step_no, $aprotoid);  

if ( $selsteps ) {
	echo "<ul>";
	$mainScriptObj->selSingleSteps( $sql, $sql2, $step_no, $one_obj_id, $parx['a_proto_id'] ); 
	echo "</ul>";
	$error->printAll();
    htmlFoot();  
}

$tablenice = tablename_nice2($tablename); 


$num_elem = $mainScriptObj->getSel($sql, $sel, $sqlAfterOrder, $go);

if (!$num_elem or !sizeof($sel) ) {
	htmlFoot('No elements selected. (Step:getSel)');
}

if ( $editmode == "edit" ) js_formAll(); // print javascript functions for form

if ( $parx3["format"] == "html" ) {
    echo "<br>\n";
	$mainScriptObj->showHeadLinks();
}
if ( $parx3["format"] == "print" ) {
	$mainScriptObj->showHeadPrintLinks();
}

$mainScriptObj->infox["target_a_proto_id"]   = $parx['a_proto_id'] ;
$mainScriptObj->infox["target_a_proto_name"] = obj_nice_name ( $sql, 'ABSTRACT_PROTO',  $parx['a_proto_id'] );

$mainScriptObj->InitialChecks();
if ($error->printAll()) {
	htmlFoot();
}



if ( $go and $parx['action']=='normal' ) {  // save protocol parameters
    $protoUpLib = new oCprotoUpStepC( $mainScriptObj );
    $updateInfo = $protoUpLib->update_individualVals( $sql, $sel, $step, $parx ); 
}
if ( $go and $parx['action']=='all_same' ) {  // save protocol parameters
    $protoUpLib = new oCprotoUpStepC( $mainScriptObj );
    $updateInfo = $protoUpLib->update_SameVals( $sql, $sel, $step, $parx, $_REQUEST['chk'], $_REQUEST['vx'] );
    $parx["action"] ='normal'; // switch to normal
    $go=0;
}

$mainScriptObj->get_ref_obj_inf( $sql );


if (!$mainScriptObj->infox["target_a_proto_id"])  {
    $tmp = $mainScriptObj->get_ref_obj();
    echo "ERROR: At least first ".$tablenice." [ID:".$tmp."] must contain a protocol.<br>";         
    return;
} 

$mainScriptObj->set_target_a_proto_id($mainScriptObj->infox["target_a_proto_id"]);

$mainScriptObj->infox["step_cnt"] = sizeof($step);

$mainScriptObj->getAbsProtoSteps($sql, $step );
if ($parx3["format"]=="html") { 
	$mainScriptObj->protoStepInfo($sql, $mainScriptObj->infox );
}
                            
#$first_obj = key($sel);
#reset($sel);

// $mainScriptObj->infox["target_a_proto_id"]
$mainScriptObj->manage_first_proto($sql, $sel  );
if ($error->Got(READONLY))  {
    $error->printAll();
    htmlFoot();
}

$first_step = key($step);
reset($step);
$mainScriptObj->setParxVal( "proto_step_nr", $first_step) ;

if ($parx["action"] == "all_same") {
    
   
    $forms_lib = new oPROTOcomp_Forms( $proj_id, $tablename, $parx, $step);
    $infores   = $forms_lib->form_one_VAL_all($sql, $sql2, $sel, $step, $mainScriptObj->infox);
    
    $error->printAll();
    return;
} 

if ( $mainScriptObj->parx3["format"]=="print" ) {
	$mainScriptObj->showRefProto($sql, $sql2, $mainScriptObj->infox);
}

if ($editmode != "edit") {

	$infores = $mainScriptObj->showSteps( $sql, $sel, $mainScriptObj->infox, $mainScriptObj->parx, $step );
	if ($parx3["format"]=="csv") { 
		return;
	}

} else {  
    // ALLOW EDIT
    $forms_lib = new oPROTOcomp_Forms( $proj_id, $tablename, $parx, $step);
    $infores   = $forms_lib->form_edit_steps($sql, $sql2, $sel, $step, $mainScriptObj->infox, $updateInfo);
}
	
$mainScriptObj->info_out("Number of shown steps", $infores["total_steps"]);  
if ($infores["badAProtoCnt"]) 
    $mainScriptObj->info_out("Number of steps with other ".tablename_nice2('ABSTRACT_PROTO'), "<font color=red>". $infores["badAProtoCnt"]."</font>"); 
echo "<br>\n";

if ( $mainScriptObj->isHtmlFormat() and ( $editmode=="edit" ) ) {
	$mainScriptObj->showButSameVal( $mainScriptObj->infox["step_cnt"] );
}
echo "</UL>\n";
htmlFoot();
