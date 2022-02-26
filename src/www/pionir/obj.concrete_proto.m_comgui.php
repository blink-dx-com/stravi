<?
/**
 * - compare concrete_proto_steps : WIZARD
   - set the protocol step on EXP or W_WAFER
   - if first object does NOT contain "tablename"= OBJ_TABLE AND "aprotoid" AND "step_no" 
	removes all saved parameters in $_SESSION['s_formState']["oPROTOcmp"] ???
 * @package obj.concrete_proto.m_comgui.php
 * @swreq   SREQ:0002523: g > list of objects > compare protocols 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @global $_SESSION['s_formState']["oPROTOcmp"] = array ( "tablename"=> "aprotoid"=> , "step_no"=> ) 
 * @param  
 *   $tablename  ["EXP"], 
	  			  "CONCRETE_PROTO", 
				  "W_WAFER" 	   (need $aprotoid)
				  "CHIP_READER"
				  "CONCRETE_SUBST"
	   $proj_id	  if not  $tablename ... (use experiments in project)
	  [$obj_ids]
	  [$go]	  	  for "EXP" / "W_WAFER"
	  [$step_no]  for "EXP" / "W_WAFER"
	  [$step_no2]  for "EXP" / "W_WAFER" : for [$action] = "sel_step"  
      [$step_id]  for protocol_steps => switches to obj.concrete_proto.m_comp2e.php
      [$action]   "sel_step"    => select single step
                  ["overview"]
                  "all_steps"
      
	  [$aprotoid] ABSTRACT_PROTO_ID => can overwrite $step_no
 */ 
session_start(); 

require_once ('reqnormal.inc');
require_once ("gui/o.PROTO.stepout1.inc");
require_once ("func_form.inc");    
require_once ("sql_query_dyn.inc");
require_once ( "javascript.inc" );
 

require_once ("subs/obj.concrete_proto.m_comp.inc");
require_once ("subs/obj.concrete_proto.cmpgui.inc");
require_once ("f.head_proj.inc");

class PROTOcmpGuiC {

function __construct($proj_id, $tablename) {
	
	$this->proj_id = $proj_id;
	$this->tablename = $tablename;
	if ($proj_id AND $tablename=="") $this->tablename = "EXP";
	
}

function expwafer_form1(
	&$sql, 
	$one_obj_id,
	$tablename // "EXP", "W_WAFER"
	){
	
	
	
	   
	$initsarr    = NULL;
	$a_proto_tmp = NULL;
	// $tablename   = "EXP";
		
	if ($tablename=="EXP") { 	
		$sqls= "select e.STEP_NO, c.ABSTRACT_PROTO_ID from EXP_HAS_PROTO e, CONCRETE_PROTO c where e.EXP_ID=".$one_obj_id. 
				" AND e.CONCRETE_PROTO_ID=c.CONCRETE_PROTO_ID ORDER by STEP_NO";
	} else { // W_WAFER
		$sqls= "select e.STEP_NR, c.ABSTRACT_PROTO_ID from W_WAFER_STEP e, CONCRETE_PROTO c where e.W_WAFER_ID=".$one_obj_id. 
				" AND e.CONCRETE_PROTO_ID=c.CONCRETE_PROTO_ID ORDER by STEP_NR";
	}
	$sql->query("$sqls");
	while ( $sql->ReadRow() ) {
		$a_proto_tmp[$sql->RowData[0]] = $sql->RowData[1];
	}

	if ( sizeof($a_proto_tmp) ) {
		
		foreach( $a_proto_tmp as $tmp_step=>$tmp_abs_proto) {
			$sqls= "select NAME from ABSTRACT_PROTO where ABSTRACT_PROTO_ID=".$tmp_abs_proto;
			$sql->query("$sqls");
			$sql->ReadRow();
			$name=$sql->RowData[0];
			$initsarr[$tmp_step] = $name;
		}

	}
	
	if ( !sizeof($initsarr) ) {
		echo "<br>";
		htmlErrorBox("Warning", "First object contains no protocol");
		echo "<br>";
	} 

	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Select compare mode"; 
	$initarr["submittitle"] = "Submit"; 
	$initarr["tabwidth"]    = "AUTO"; 
	$initarr["tabnowrap"]   = "1";
	$hiddenarr = NULL;
	$hiddenarr["tablename"] = $tablename; 
	$hiddenarr["proj_id"]   = $this->proj_id; 
	
	if (sizeof($initsarr)==1) {
		$oneStepNo =  key($initsarr);
		$hiddenarr["step_no"]    = $oneStepNo;
		$hiddenarr["step_no2"]   = $oneStepNo; 
	} 
	
	$formobj = new formc($initarr, $hiddenarr, 0);
	
	$fieldx = array ("title" => "Overview", "name"  => "action", "object" => "radio",
					"val"   => $action, "inits" => "overview", "namex" => 1,
					"notes" => "show only the protocol names" ); 
	$formobj->fieldOut( $fieldx );
	
	//$fieldx = array ("title" => "Actions for ONE selected protocol", "object" => "info"); 
	//$formobj->fieldOut( $fieldx ); 
	
	if ( sizeof($initsarr) ) {
		$tmpval  =  key($initsarr);
		$tmpName =  current($initsarr);
		reset ($initsarr);
		
		$tmpnotes = "Select one protocol";
		$tmpObject= "select";
		
		if (sizeof($initsarr)==1) {
			$tmpObject= "info2";
			$initsarr = $tmpName; // name of the protocol
			$protoSelAll = $tmpName.":";
			$protoSelSel = $tmpName.":";
			$tmpnotes = "the protocol";
		} else {
			$protoSelAll = $formobj->selectFget( "step_no" , $initsarr, $tmpval );
			$protoSelSel = $formobj->selectFget( "step_no2", $initsarr, $tmpval );
		}
		

		$fieldx = array ("title" => "ALL steps", "name"  => "action", "object" => "radio",
						"val"   => $action, "inits" => "all_steps", "namex" => 1,
						"notes" => $protoSelAll ." show all steps", "bgcolor"=>"OPTIONAL"); 
		$formobj->fieldOut( $fieldx );
		
		$fieldx = array ("title" => "SELECTED steps", "name"  => "action", "object" => "radio",
						"val"   => $action, "inits" => "sel_step", "namex" => 1,
						"notes" => $protoSelSel ." show only selected steps", "bgcolor"=>"OPTIONAL"); 
		$formobj->fieldOut( $fieldx ); 
	}
	$formobj->close( TRUE );
	
}


function form_subst1($tablename = "CONCRETE_SUBST") {
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Select compare mode"; 
	$initarr["submittitle"] = "Submit"; 
	$initarr["tabwidth"]    = "AUTO";
	$initarr["colwidth"]    = array ("30%", "5%", "65%"); 
	
	$hiddenarr = NULL;
	$hiddenarr["tablename"]     = $tablename; 
	$action = "overview";
	
	$formobj = new formc($initarr, $hiddenarr, 0);
	
	$fieldx = array ("title" => "Overview", "name"  => "action", "object" => "radio",
					"val"   => $action, "inits" => "overview", "namex" => 1,
					"notes" => "show only the protocol names" ); 
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ("title" => "ALL step details", "name"  => "action", "object" => "radio",
					"val"   => $action, "inits" => "all_steps", "namex" => 1,
					"notes" => "show all steps and parameters" ); 
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ("title" => "Selected steps", "name"  => "action", "object" => "radio",
					"val"   => $action, "inits" => "sel_step", "namex" => 1,
					"notes" => "show only selected steps and parameters" ); 
	$formobj->fieldOut( $fieldx ); 
	
	$formobj->close( TRUE );
}

}

//***********************************************************************************************



$error      = & ErrorHandler::get(); 
$sql        = logon2( $_SERVER['PHP_SELF'] );	
// $sql2       = logon2( $_SERVER['PHP_SELF'] );

$obj_ids= $_REQUEST['obj_ids'];
$tablename = $_REQUEST['tablename'];
$proj_id = $_REQUEST['proj_id'];
$obj_ids = $_REQUEST['obj_ids'];
$go = $_REQUEST['go'];
$step_no = $_REQUEST['step_no'];
$step_no2 = $_REQUEST['step_no2'];
$step_id = $_REQUEST['step_id'];
$action = $_REQUEST['action'];
$aprotoid = $_REQUEST['aprotoid'];


if ( $action=="" ) $action = "overview";

$toption    = NULL;
$toption["width"] = "AUTO";
if ($proj_id AND $tablename=="") $tablename = "EXP";

$tablenice  = tablename_nice2($tablename);
$infox 		= NULL;
$infoarr    = NULL;
$title      = "Compare protocols for ".$tablenice;
$mainScriptObj = new PROTOcmpGuiC($proj_id, $tablename);
$compGuiAll    = new oPROTO_cmpgui();
$compGuiAll->init($proj_id, $tablename);


// manage a script header with alternativ calling from PROJ or with TABLE-selection
//   
$compGuiAll->showHeader( $sql, $title, $obj_ids, $infoarr, "wizard" );
$sqlAfter = $compGuiAll->sqlAfter;	

echo "<br>\n";
echo "<UL>\n"; 

//
// get one object
//
$sql->query("SELECT ".$tablename."_ID FROM $sqlAfter"); 
$sql->ReadRow();
$tmpobj = $sql->RowData[0];
$infox["first_obj_id"] = $tmpobj;

if ( $action == "sel_step" )  {
	$step_no = $step_no2;
}

list($step_no, $aprotoid) = $compGuiAll->anaFormState($step_no, $aprotoid);


if ( !$go ) {
	switch ($tablename) {
		case "EXP":
			$mainScriptObj->expwafer_form1($sql, $infox["first_obj_id"], "EXP");
			break;
		case "W_WAFER":
			$mainScriptObj->expwafer_form1($sql, $infox["first_obj_id"], "W_WAFER");
			break;
		default:
			$mainScriptObj->form_subst1($tablename);
			
	}
	htmlFoot();
}


    
    if ( $action == "overview" ) {
		if ($tablename=="CONCRETE_SUBST")  {
			$newurl =  "p.php?mod=DEF/o.CONCRETE_SUBST.comp";
	        js__location_replace( $newurl, "overview" ); 
	        return;
    	}
    
		if ( $tablename=="CHIP_READER")  {
			$newurl =  "obj.chip_reader.comp.php?tablename=".$tablename;
	        js__location_replace( $newurl, "overview" ); 
	        return;
	    }
	    
	    if ( $tablename=="CONCRETE_PROTO")  {
	    	echo "No action for this object type defined.<br>";
	    	htmlFoot();
	    }
	    if ($tablename=="EXP") {
    		$newurl =  "obj.exp.protocol_comp.php?proj_id=" . $proj_id;
    		js__location_replace( $newurl, "overview" );
    		return;
	    }
	    if ($tablename=="W_WAFER") {
	    	$newurl =  "obj.w_wafer.list_pcomp.php";
	    	js__location_replace( $newurl, "overview" );
	    	return; 
	    }
	 }


if ( $action == "all_steps" )  { 
	$newurl = "obj.concrete_proto.m_comp.php?tablename=".$tablename."&proj_id=".$proj_id."&step_no=".$step_no;
	js__location_replace( $newurl, "all_steps" ); 	
	return;
}

if ( $action == "sel_step" )  { 
	$newurl = "obj.concrete_proto.m_comp2e.php?tablename=".$tablename."&proj_id=".$proj_id."&step_no=".$step_no."&selsteps=1";
	js__location_replace( $newurl, "sel_step" );
	return;
}


htmlFoot();