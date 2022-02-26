<?php
/**
 * Configure protocol parameters for MixedVisu
 * @package obj.exp.res_visu_conf.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $expid  (experiment ID)
   		   $backparam = $proj_id OR ""
  		   $go
		    --- OPTIONAL: ---
		   $parx["aprotoid"]
		   $step[]   selected step numbers array[STEPNR] = 1
 * @version0 2006-07-04
 * @app_type_info will be overwritten by type:2021_abbott
 */ 
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("o.EXP.proto.inc");
require_once ('func_form.inc');
require_once ("gui/o.PROTO.stepout2.inc");

class mainScriptC {

function __construct($expid, $backparam) {
	$this->expid = $expid;
	$this->backparam = $backparam;
}

function form0($protoArray) {
	
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "1. Select protocol";
	$initarr["submittitle"] = "Submit";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["expid"]     = $this->expid;
	$hiddenarr["backparam"] = $this->backparam;

	$formobj = new formc($initarr, $hiddenarr, 0);
	
	$val = current($protoArray);
	$firstproto = $val[0];
	reset($protoArray);
	
	$initarr = NULL;
	foreach( $protoArray as $key=>$val) {
		$initarr[$val[0]] = $val[1];
	}
	reset ($protoArray); 

	$fieldx = array ( 
			"title" => "Protocol", 
			"name"  => "aprotoid",
			"object" => "select",
			"val"   => $firstproto, "inits" => $initarr,
			"notes" => "the protocol" );
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );
}

function form1(&$sql, &$sql2, $parx) {
	
	
	if ( !$parx["aprotoid"] ) {
		htmlFoot("Error", "Please select a protocol!");
	}
	
 	echo "<br><B><font color=blue size=+1>2. Select single steps from protocol (abstract)</font></B><br><br>";
 	$pOptions=array();
	$pOptions["checkboxes"] = 1;
	echo '<form name="xtraform" ACTION="'.$_SERVER['PHP_SELF'].'" METHOD=POST>'; 
	echo "<input type=hidden name='expid' value='".$this->expid."'>\n";
	echo "<input type=hidden name='parx[aprotoid]' value='".$parx["aprotoid"]."'>\n";
	echo "<input type=hidden name='go' value='2'>\n";
	echo "<input type=hidden name='backparam' value='".$this->backparam."'>\n";
	
	$sqls= "select name  from  ABSTRACT_PROTO  where ABSTRACT_PROTO_ID=".$parx["aprotoid"];
	$sql->query("$sqls");
	$sql->ReadRow();
	$a_proto_name=$sql->RowData[0];
	
	echo "<font color=gray>Protocol (abstract):</font> <B>$a_proto_name</B> [ID:".$parx["aprotoid"]."]<br>\n";
	
	$sqls= "select count(*) from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$parx["aprotoid"];
	$sql->query("$sqls");
	$sql->ReadRow();
	$steps=$sql->RowData[0];
	
	if ( $steps ) {	
	
		$protoShowObj = new oProtocolShowC();
		$protoShowObj->writeJavascript();
		$protoShowObj->showAll( $sql, $sql2, 0, $parx["aprotoid"], $pOptions ); 
		echo "<input type=submit value=\"Select steps\"> ";
	} else {
		 htmlInfoBox( "Protocol step problem", "Protocol contains no steps!", "", "WARN" );
	}
	echo "</form>\n";
}

}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( ); // give the URL-link for the first db-login
$sql2  = logon2( );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$expid = $_REQUEST["expid"];
$backparam = $_REQUEST["backparam"];
$go = $_REQUEST["go"];
$parx = $_REQUEST["parx"];
$step = $_REQUEST["step"];

$expProtoObj   = new oEXPprotoC();
$mainScriptObj = new mainScriptC($expid, $backparam);


$title       = "MixedVisu: Configure protocol visualization";


$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$newurl = "obj.exp.res_visu_arr.php";
if ( $backparam>0 ) $newurl .= "?proj_id=".$backparam;
/*
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;
$infoarr["show_name"]= 1;
*/
// $infoarr["version"] = '1.0';	// version of script


$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);


echo "<ul>";

if (!$expid) htmlFoot("Error","Need an Reference-experiment!");

// get protocols
$protoArray = $expProtoObj->getAbsProtocols ($sql, $expid);

if ( !sizeof($protoArray) )  htmlFoot("Error","Reefrence experiment ID:$expid has no protocols!");

if ( !$go ) {
	$mainScriptObj->form0($protoArray);
	htmlFoot();
}

if ( $go==1 ) {
	$mainScriptObj->form1($sql, $sql2, $parx);
	htmlFoot();
}

if ( $go==2 ) {
	// save parameters
	$doforward = 1;
	if ( sizeof($step) ) {
		$_SESSION['s_formState']["obj.exp.res_visu_arr"]["protoparm"] = NULL;
		$_SESSION['s_formState']["obj.exp.res_visu_arr"]["protoparm"] = array("aprotoid"=>$parx["aprotoid"], "steps"=>$step);
	} else {
		htmlInfoBox( "Parameters problem", "You did'nt select steps!", "", "ERROR" );
		$doforward = 0;
	}
	$newurl = "obj.exp.res_visu_arr.php";
	if ( $backparam>0 ) $newurl .= "?proj_id=".$backparam;
	
	echo "<br><a href=\"$newurl\">... Go back to MixedVisu ...</a> <br>";
	if ( $doforward ) {
		?>
		<script language="JavaScript">
			location.replace("<?php echo $newurl?>");            
		</script>
		<? 	
	}
	htmlFoot();
}

htmlFoot();