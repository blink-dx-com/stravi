<?php
/**
 *  - upoad a single document file
  	- can handle version control management
 * @package obj.link.versedit.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @version
 * @param   $id of LINK
  			$pos
			$action = "del"
			$go
			$parx
 */ 
session_start(); 


require_once ("o.LINK.subs.inc");
require_once ("reqnormal.inc"); 
require_once("o.LINK.versctrl.inc");

class oLINKVersEdit {

function init(&$sql, $id, $pos) {
	$this->idx  = $id;
	$this->pos  = $pos;
	
	$this->versionObj = new oLINKversC();
	$this->versionObj->initDoc($sql, $id); 
	$this->loaddata = $this->versionObj->getInfo($sql, $id, $pos);
	$this->deleteRight=0;
}

function setData($parx) {
	$this->loaddata = $parx;
}

function analyseAccess($o_rights) {
	
	$editmode   = 1;
	$editReason = "";
	
	if (!$o_rights["insert"]) {
		$editmode=0;
		$editReason ="no 'insert'-right on the document";
	}
	
	if (!$o_rights["write"]) { // analyze more rights
		if ( $this->loaddata["DB_USER_ID"] != $_SESSION['sec']['db_user_id'] ) {
			$editmode   = 0;
			$editReason = "you have 'insert' access, but you are not the creator";
		}
	}
	
	$this->deleteRight=0;
	if ($o_rights["delete"]) {
		$this->deleteRight=1;
	}
	return (array($editmode,$editReason)); 
}

function showButtons() {
	
	
	echo '<a href="obj.link.verhist.php?id='.$this->idx.'">';
	echo '<img src="images/but.list2.gif" TITLE="view version history" border="0"></a>&nbsp;&nbsp;'."\n";
		
	if ($this->deleteRight) {
		echo '<a href="'.$_SERVER['PHP_SELF'].'?id='.$this->idx.'&pos='.$this->pos.'&action=del">';
		echo '<img src="images/but.delete.gif" TITLE="delete version" border="0"></a>';
	} else {
		echo '<img src="images/but.delete.low.gif" TITLE="delete version: no delete right">';
	}
	echo "<br>\n";
}


function showForm(&$sql, $argu, $editmode, $opt=NULL) {
	require_once ('func_form.inc');
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Update version features";
	$initarr["submittitle"] = "Update";
	$initarr["tabwidth"]    = "AUTO";
	$initarr["tabnowrap"]	= 1;
	if (!$editmode) {
		$initarr["title"]       = "Show version features";
		$initarr["submittitle"] = "";
	}
	
	$hiddenarr = NULL;
	$hiddenarr["id"]     = $this->idx;
	$hiddenarr["pos"]    = $this->pos;

	$formobj = new formc($initarr, $hiddenarr, 0);
	$infox=array();
	$infox["user"] = obj_nice_name    ( $sql, "DB_USER", $argu["DB_USER_ID"] );
	$fieldx=NULL;
	$fieldx[] = array ( "title" => "Creator",		"view"=>1, "object" => "text", "val"=>$infox["user"]);
	$fieldx[] = array ( "title" => "Date",			"view"=>1, "object" => "text", "val"=>$argu["CDATE"]);
	$fieldx[] = array ( "title" => "Main version",	"name"=>"MAIN_VER", "object" => "text", "fsize" => 3, "val"=>$argu["MAIN_VER"] );
	$fieldx[] = array ( "title" => "Sub version",	"name"=>"SUB_VER", "object" => "text", "fsize" => 3, "val"=>$argu["SUB_VER"]  );
	$fieldx[] = array ( "title" => "Notes",		    "name"=>"NOTES",  "object" => "textarea", "val"=>$argu["NOTES"], "inits"=>array("rows"=>10, "cols"=>70) );
                                  //             cols=$fieldx["init"]["cols"]  

	foreach( $fieldx as $key=>$val) {
		if (!$editmode) $val["view"]=1;
		$formobj->fieldOut( $val );
	}
	reset ($fieldx); 	
	
	$cloption	 = NULL;
	if (!$editmode) {
		$cloption["noSubmitButton"] = 1;
	}					  
	$formobj->close( TRUE, $cloption );
	
	if ( $opt["access"]!="" ) {
		echo " <font color=gray><img src=\"images/ico.password.gif\"> Access info:</font> DENIED: ".$opt["access"]."<br>";
	}
	echo "<br>";
	
	$filePath = $this->versionObj->getVersPathEasy($this->idx, $this->pos);
	if ( !file_exists($filePath) ) {
		echo "<br>";
		htmlInfoBox( "Error", "Version-File '".$filePath."' not found!", "", "ERROR" );
		echo "<br>";
	} else {
		$sizenow = filesize($filePath);
		echo "File-size: <b>$sizenow</b> bytes<br>\n";
		$downurl = "obj.link.download.php?id=".$this->idx."&pos=".$this->pos;
		echo "<a href=\"". $downurl . "\"><img src=\"images/ic.docdown.big.gif\" border=0 TITLE=\"Download\"></a>".
			"&nbsp;<b>Download</b><br>\n";
	}
	echo "<hr>";
}

function updateFeatures( &$sql, $id, $pos, $arguin) {
	global $error;
	
	// test parameters
	list($isok, $before, $nextx) = $this->versionObj->checkNeighours($sql, $id, $pos, $arguin["MAIN_VER"], $arguin["SUB_VER"]);
	if ( $isok<0 ) {
 		 $error->set("oLINKversC::updateFeatures", 1, "Parameter error.<br>The new version-numbers must be $before &lt; new_version &lt; $nextx");
	} else {
		$this->versionObj->updateFeatures( $sql, $id, $pos, $arguin);
	}
}

function deleteVers( &$sql) {
	$this->versionObj->deleteVers( $sql, $this->idx, $this->pos);
}

}

$error = & ErrorHandler::get();
$sql 	  = logon2( $_SERVER['PHP_SELF'] );

$id 	= $_REQUEST["id"];
$pos =  $_REQUEST["pos"];
$action = $_REQUEST["action"];
$go = $_REQUEST["go"];
$parx= $_REQUEST["parx"];

$o_rights = access_check( $sql, "LINK", $id );
$tablename= "LINK";
$errsave  = NULL;
$posnow   = "";

if ( $id ) {
	$versFormObj = new oLINKVersEdit();
	$versFormObj->init( $sql, $id, $pos);
	if ($error->got(READONLY)) {
		$errsave = $error;
	} else {
		$posnow = $versFormObj->pos;
	}
}

$title       = "Edit version features";

#$infoarr['help_url'] = 'o.EXAMPLE.htm';
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;
$infoarr["checkid"]  = 1;
if ($posnow) $infoarr["obj_more"] = "&nbsp;version-entry: $posnow";


$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>";

if ($errsave) {
	$errsave->printAll();
	htmlFoot( "ERROR", "Entry not found.");
}

list($editmode,$editReason)  = $versFormObj->analyseAccess($o_rights);

if ( $action=="del") {

	if (!$versFormObj->deleteRight) {
		htmlFoot("Error", "No delete right!");
	}	

	$vertext = $versFormObj->loaddata["MAIN_VER"].".".$versFormObj->loaddata["SUB_VER"];
	if ( !$go ) {
		$iopt="";
		$iopt["icon"] = "ic.del.gif";
		htmlInfoBox( "Delete version ".$vertext, "", "open", "INFO", $iopt );
		echo "<center>";
		echo "[<a href=\"".$_SERVER['PHP_SELF'].'?id='.$versFormObj->idx.'&pos='.$versFormObj->pos.
			 "&action=del&go=1\"><b>YES</b></a>]".
		     "&nbsp;&nbsp;&nbsp;[<a href=\"".$_SERVER['PHP_SELF'].'?id='.$versFormObj->idx.'&pos='.$versFormObj->pos."\">no</a>]";
		echo "</center>";
		htmlInfoBox( "", "", "close" );
		htmlFoot();
	} else {
		// delete version
		$versFormObj->deleteVers($sql);
		if ($error->printAll()) {
			htmlFoot();
		}
		$newurl="obj.link.verhist.php?id=".$id;
		?>
		<script language="JavaScript">
			location.replace("<?php echo $newurl?>");            
		</script>
		<? 	
	} 
}

if ($go AND $editmode) {
	// update info
	
	$versFormObj->updateFeatures($sql, $id, $pos, $parx);
	if ($error->got(READONLY)) {
		$error->set("update()", 1, "Update failed!");
		$error->printAllEasy();
		echo "<br>";
		
		$versFormObj->setData($parx);
		
	} else {
		echo "<b><font color=green>updated.</font></b><br>";
		$versFormObj->init( $sql, $id, $pos);
	}
}

$butopt=NULL;
if ($editReason!="") $butopt["access"]=$editReason; 
$versFormObj->showButtons();
$versFormObj->showForm( $sql, $versFormObj->loaddata, $editmode, $butopt );

htmlFoot();

