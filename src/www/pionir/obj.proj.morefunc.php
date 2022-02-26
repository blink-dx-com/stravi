<?php
/**
* more project functions
* @package obj.proj.morefunc.php
* @author  Steffen Kube (steffen@blink-dx.com)
* @param   $id   (PROJ_ID)
*/
session_start(); 


require_once ('db_access.inc');
require_once ('globals.inc');
require_once ('func_head.inc');  
 
class oPROJ_morefunc{
 
function linespace() {
	echo '<br>';
}
 
}

function this_addlink( $text, $url, $inactive=NULL ) {
	$outtext = this_addlinkPure( $text, $url, $inactive  );
    echo "<li>$outtext</li>\n"; 
}

function this_addlinkPure( $text, $url, $inactive=NULL ) {
	if ($inactive) $outtext = $text;
	else $outtext = "<a href=\"".$url."\">".$text."</a>";
	return ($outtext); 
}

function this_headx($text) {
	echo "<B>$text</B><br><ul style=\"padding-top:8px; padding-bottom:8px; padding-left:20px;\">\n"; 
}


$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$id 				 = $_REQUEST["id"];

$title = 'Project: more functions';
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = "PROJ";
$infoarr["obj_id"]   = $id;
$infoarr["checkid"]  = 1;

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>\n";

$mainlib = new oPROJ_morefunc();

$boximg = "<img src=\"images/icon._box.gif\">";


this_headx($boximg." Clipboard functions");
this_addlink( "paste objects from history to project", "obj.proj.pastehist.php?id=". $id);
this_addlink( "copy selected objects from list-view to project", "obj.proj.pasteList.php?id=". $id);
this_addlink( "transform a clipboard-copy to a CUT action", "obj.proj.elementact.php?proj_id=". $id."&actio_elem=tocut");
$mainlib->linespace();
this_addlink( "remove objects-in-clipboard from project", "obj.proj.elementact.php?proj_id=". $id."&actio_elem=minusclip");
this_addlink( "remove broken links from project", "obj.proj.elementact.php?proj_id=".$id."&actio_elem=delBroken");
echo "</ul>\n";

this_headx("<img src=\"images/icon.PROJ.gif\"> Project analysis");
this_addlink(  'Activity analysis', 'obj.proj.treeana2.php?id='.$id);
this_addlink(  'View tree', 'obj.proj.viewTree.php?id='.$id);
echo "</ul>\n";

// this_headx("<img src=\"images/icon.EXP.gif\"> Experiment functions");
// this_addlink( "[ExpImporter] Import experiments from device file", "obj.proj.series2.php?id=". $id);
// this_addlink( "[QcExCreaWiz] QC Experiment wizard", "obj.proj.qcWiz.php?id=". $id);
// this_addlink( "[ExProtoConnect] Connect protocols", "obj.exp.protoMulMa.php?id=". $id);
// this_addlink( "[ExCreatorZoo] Create a new experiment (and upload image)", "obj.exp.new1.php?parx[proj_id]=". $id);
// echo "</ul>\n";


  
echo "</ul>\n"; 
//this_headx($boximg." QC functions ");
//this_addlink( "[QC-Project-Cleaner]", "obj.proj.qcRem.php?id=".$id );
// echo "</ul>\n"; 

$lab_special_file = '../lab/obj.proj.funcs_list.inc';
if (file_exists($lab_special_file)) { 
    this_headx("$boximg Lab functions");;
    require_once ($lab_special_file);
    $tmpfunc=lab_funcs($id);
    foreach( $tmpfunc as $dummy=>$th) {
        echo "<li><a href=\"../lab/" .$th["url"]. "\">".$th["txt"]."</a><br>\n";
    }
    echo "</ul>"; 
}

echo "</ul>\n";
htmlFoot('<hr>');
