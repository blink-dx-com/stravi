<?php
/**
 * [MultiCreator] create X objects as blueprint of object ID
	TBD: for CONCRETE_SUBST, CHIP_READER: create a real copy of the PROTOCOL_CONCRETE !!!
 * @package glob.obj.creaMulti.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $t   - tablename
  		  $id
		  $projid
		  $go    : 1 - create
		  $parx["num"] -- MUST BE SET
		  $parx["start"]
		  $parx["pre"]	- preposition
 		  $parx["advanced"] -- advanced
		  
		  $deep_copy[assoc_name] = 1 : deep copy of assoc table
 * @version0 2008-04-01
 */
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ('glob.obj.copyobj2.inc');
require_once ("f.objview.inc");	
require_once ('func_form.inc');

class gObjCreaMulti{

function __construct(&$sql, $tablename, $id, $projid, &$parx, &$deep_copy) {

	 $this->tablename=$tablename;
	 $this->id = $id;
	 $this->projid = $projid;
	 $this->parx = $parx;
	 $this->deep_copy=$deep_copy;
	 
	 $this->copyLib = new objCreaWiz($tablename);
	 
	 $this->assoc_arr = get_assoc_tables2( $sql, $tablename );
	 
}

function selObject(&$sql) {
	// select an object
	
	
	$tablename = $this->tablename;
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Select the original object";
	$initarr["submittitle"] = "Select";
	$initarr["tabwidth"]    = "AUTO";
	$initarr["dblink"]      = 1;
	$initarr["tabnowrap"]   = 1;
	$initarr["goNext"]		= "0";

	$hiddenarr = NULL;
	$hiddenarr["t"]     = $this->tablename;
	$hiddenarr["projid"]= $this->projid;
	
	$i_tableNiceName 	 = tablename_nice2($tablename);

	$formobj = new formc($initarr, $hiddenarr, 0);

	$fieldx = array ( 
		"title" => $i_tableNiceName, 
		"namex" => TRUE,
		"name"  => "id",
		"object"=> "dblink",
		"val"   => "", 
		"inits" => array("table"=>$this->tablename,
						"objname"=> " --- select ---- ",
						"pos" => 0,
						"projlink"=> 1 ) ,
		"notes" => "select the original object"
		 );
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );		
}

function initChecks(&$sql) {
	global $error;
	
	$FUNCNAME="initChecks";
	$tablename = $this->tablename;
	$id        = $this->id;
	$i_tableNiceName 	 = tablename_nice2($tablename);
	
	if ($tablename=="" OR $id=="") {
		$error->set( $FUNCNAME, 1, "Please give table and ID of object.");
		return;
	} 
	
	if ( !$this->projid) {
		$error->set( $FUNCNAME, 2, "Please give a project-ID.");
		return;
	}
	
	if ( !cct_access_has2($tablename) ) {
		$error->set( $FUNCNAME, 3, "This tool works only for business objects.");
		return;
	}
	
	$nicopt = array("absName"=>1);
	echo  "<b>Original $i_tableNiceName:</B> ".fObjViewC::bo_display($sql, $tablename, $id, $nicopt);
	echo "<br><br>\n";
	
	$t_rights = tableAccessCheck( $sql, $tablename );
	if ( $t_rights["insert"] != 1 ) {
		$answer = getTableAccessMsg( $name, $right );
		$error->set( $FUNCNAME, 4, $answer);
		return;
	}
	
	//project
	$o_rights = access_check($sql, "PROJ", $this->projid);
	if ( !$o_rights["insert"] ) {
		$error->set( $FUNCNAME, 5, "no insert permissions on this project ".$this->projid."!");
		return;
	}
}

function checkParams() {
	global $error;
	$FUNCNAME= "checkParams";
	
	$parx = &$this->parx;
	
	if ($parx["num"]<=0 ) {
		$error->set($FUNCNAME, 1,"Give a number");
		return;
	} 
	if ( $parx["num"]>1000) {
		$error->set($FUNCNAME, 1,"Number ".$parx["num"]." too big.");
		return;
	} 
	if ( $parx["start"]!="" AND !is_numeric($parx["start"]) ) {
		$error->set($FUNCNAME, 1,"Start-Number '".$parx["start"]."' must be a number.");
		return;
	} 

}

function getAssocBoxes(&$sql) {

	$tablename = $this->tablename;
	$text = "";
	$brtag = "";
	foreach( $this->assoc_arr as $dummy=>$th) {
	
		$allow	= 1;
		$assoc_name		= $th[0];
		$assoc_nicename	= $th[1];
		
		if ( ($tablename=="W_WAFER") && ($assoc_name=="W_CHIP_FROM_WAFER") ) $allow=0; // not allowed
		if ($tablename=="CONCRETE_PROTO" OR $tablename=="ABSTRACT_PROTO") $checked = " checked";
		else $checked = "";
		
		if ($allow) {
			$text .= $brtag . "<input type=checkbox name=deep_copy[".$assoc_name."] ".
				$checked." value=\"1\"> ".
				"<font color=gray>".$assoc_nicename."</font>";
			$brtag = "<br>";
		}
	}
	reset($this->assoc_arr);
	
	return ($text);
	
}

function form1(&$sql) {
	$parx = $this->parx;
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Give creation parameters";
	$initarr["submittitle"] = "Prepare";
	$initarr["tabwidth"]    = "AUTO";
	$initarr["tabnowrap"]   = 1;

	$hiddenarr = NULL;
	$hiddenarr["t"]     = $this->tablename;
	$hiddenarr["id"]    = $this->id;
	$hiddenarr["projid"]= $this->projid;
	$hiddenarr["parx[advanced]"]= $parx["advanced"];

	$formobj = new formc($initarr, $hiddenarr, 0);

	$fieldx = array ( 
		"title" => "Number of objects", 
		"name"  => "num",
		"object"=> "text",
		"val"   => $parx["num"], 
		"notes" => "Number of objects"
		 );
	$formobj->fieldOut( $fieldx );
	
	if ( $parx["advanced"]) {
		$fieldx = array ( 
		"title" => "Start-number", 
		"name"  => "start",
		"object"=> "text",
		"val"   => $parx["start"], 
		"notes" => "e.g. '05'"
		 );
		$formobj->fieldOut( $fieldx );
		
		$fieldx = array ( 
		"title" => "PreName", 
		"name"  => "pre",
		"object"=> "text",
		"val"   => $parx["pre"], 
		"notes" => "e.g. 'prenom_'"
		 );
		$formobj->fieldOut( $fieldx );
	}
	
	if ( !empty ($this->assoc_arr) ) {
		$text = $this->getAssocBoxes($sql);
		
		$fieldx = array ( 
			"title" => "Copy lists", 
			"name"  => "lists",
			"object"=> "info2",
			"inits" =>  $text
			
			);
		$formobj->fieldOut( $fieldx );
	}
	$clopt = NULL;
	if ( !$parx["advanced"] ) $clopt["addObjects"] = "[<a href=\"javascript:advanced(1)\">advanced</a>]";
	else  $clopt["addObjects"] = "[<a href=\"javascript:advanced(0)\">normal</a>]";
		$formobj->close( TRUE , $clopt);
	// $parx["advance"]
}

function form2() {
	$parx = $this->parx;
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Prepare creation";
	$initarr["submittitle"] = "Create now";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["t"]     = $this->tablename;
	$hiddenarr["id"]    = $this->id;
	$hiddenarr["projid"]= $this->projid;
	
	
	if ( !empty($this->deep_copy) ) {
		foreach( $this->deep_copy as $key=>$val) {
			$hiddenarr["deep_copy[".$key."]"] = $val;
		}
		reset ($this->deep_copy); 
	}

	$formobj = new formc($initarr, $hiddenarr, 1);
	$formobj->addHiddenParx( $parx );
	$formobj->close( TRUE );
}

function doLoop( &$sql, &$sql2, $go ) {

    $sql3_dummy=NULL;

	if ( $go<2 ) {
		echo "<b>Prepare creation</b>";
	} else {
		echo "<b>Create now</b>";
	}
	echo "<br><br>";
	
	$parx = $this->parx;
	$tablename = $this->tablename;
	$pkname    = PrimNameGet2($tablename);
	
	$deep_copy = 0;
	$cnt = 0;
	$new_params = array('vals'=>array());
	$nameColumn =  importantNameGet2($tablename);
	
	$coption   = NULL; 
	$coption["proj_id"]    = $this->projid;
	$coption["info"] 	   = 1;
	
	$num_of_chars = max( strlen($parx["num"]), strlen($parx["start"]) );
	$startNum     = 1;
	if ($parx["start"]!="")  $startNum = $parx["start"];
	$nameInt = $startNum;
	
	while ( $cnt < $parx["num"] ) {
	
		$newname = str_pad( $nameInt, $num_of_chars, "0", STR_PAD_LEFT );
		$new_params['vals'][$nameColumn] = $parx["pre"] . $newname;
		
		echo ($cnt+1).". ";
		
		if ($go==2) {
			$newid = 
			$this->copyLib->objCreate( $sql, $sql2, $sql3_dummy, $this->id, $new_params, $this->deep_copy, $coption );
				
			}
		echo "<b>".$new_params['vals'][$nameColumn]. "</b> [ID:".$newid."] ";
		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
			$infostr = $this->copyLib->getInfo();
			if ($infostr!="") echo $infostr;
		}
		echo "<br>\n";
		$cnt++;
		$nameInt++;
		
	}
	echo "...ready<br>\n";
}

}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
$sql2  = logon2( );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$id = $_REQUEST["id"];
$tablename=$_REQUEST['t'];
$go=$_REQUEST['go'];
$parx=$_REQUEST['parx'];
$projid=$_REQUEST['projid'];
$deep_copy=$_REQUEST['deep_copy'];


$title       		 = "Create X copies from an object";
$infoarr			 = NULL;

$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = "PROJ";
$infoarr["obj_id"]   = $projid;
$infoarr["show_name"]= 1;



$pagelib = new gHtmlHead();
$pagelib->_PageHead ( $infoarr["title"],  $infoarr );
?>
	<script>
		function advanced( flag ) {
			document.editform.go.value = 0;
			document.editform.elements['parx[advanced]'].value = flag;
			document.editform.submit();
		}
	
	</script>
	<? 
$pagelib->_startBody($sql, $infoarr);
echo "<ul>";

$mainlib = new gObjCreaMulti($sql, $tablename, $id, $projid, $parx, $deep_copy);

if (!$go AND !$mainlib->id AND $mainlib->tablename!="") {
	$mainlib->selObject($sql);
	htmlFoot("<hr>");
}


$mainlib->initChecks($sql);
if (  $error->printAll() ) {
	htmlFoot("<hr>");
}

if ( !$go ) {
	$mainlib->form1($sql);
	htmlFoot();
}

$mainlib->checkParams();
if (  $error->printAllEasy() ) {
	htmlFoot("<hr>");
}

if ( $go==1 ) {
	$mainlib->form2();
	
}

$mainlib->doLoop( $sql, $sql2, $go );
if (  $error->printAll() ) {
	htmlFoot("<hr>");
}

htmlFoot("<hr>");
