<?php
/**
 * list more functions
 * @package obj.exp.list_funcs.php
 * @swreq UREQ:xxxxxxxxxxxx
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/obj.exp.list_funcs.php 59 2018-11-21 09:04:09Z $
 */

session_start(); 


require_once ('db_access.inc');
require_once ('globals.inc');
require_once ('func_head.inc');
require_once ('access_check.inc');
require_once ('table_access.inc');
require_once ("f.textOut.inc");

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();


$title = 'List of functions for a set of experiments';

$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "list";
$infoarr['help_url']     = 'o.EXP.html';
$infoarr["obj_name"] = "EXP";

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>";  

$glist = NULL;
$glist["ana"][] = array("ty"=>"head", "txt"=>"Visualization and analysis", "lay"=>"1" );
//$glist["ana"][] = array("ty"=>"lnk", "li"=>"br", "txt"=>"Image thumbnail gallery", "href"=> "obj.proj.images_show.php?tablename=EXP", "iicon"=>"f.obj.proj.img.png", "icbord"=>1);
$glist["ana"][] = array("ty"=>"lnk", "li"=>"br", "href"=> "obj.exp.res_visu_arr.php" , "txt"=>"MixedVisu: mixed visualization (2D, bar, ...)", "iicon"=>"f.obj.exp.res_arr.png", "icbord"=>1);


$labextfunc = "../".$_SESSION['globals']["lab_path"]."/obj.exp.list_funcs.inc";
if (file_exists($labextfunc)) {
	require_once($labextfunc);
	lab_exp_list($glist);
}

$glist["ana"][] = array("ty"=>"headclose");

$textoutObj = new textOutC();   
$fopt = array("ulbr"=>1,  "ulclass"=>"yul2");


$flist[] = array("ty"=>"head", "txt"=>"Result export", "lay"=>"1" );
//$flist[] = array("ty"=>"lnk",  "txt"=>"<B>OneFeatureExport (CSV)</B>", "href"=> "objtools/EXP.filter_eisen.php", "iicon"=>"f.obj.exp.lexport.png", "icbord"=>1);
//$flist[] = array("ty"=>"lnk",  "txt"=>"AllFeatureExport (single files are collected in results.tar.gz)", "href"=>"obj.exp.res_export_gui.php?opt_list=1");
// $flist[] = array("ty"=>"lnk",  "txt"=>"Image file export (TAR-file)", "href"=>"obj.img.list_export.php?tablemom=EXP");

$flist[] = array("ty"=>"headclose");  
$textoutObj->linksOut($flist, $fopt); 


echo "<table cellpadding=0 cellspacing=0 border=0><tr valign=top><td>";
$glist["ana"][] = array("ty"=>"headclose");
$textoutObj->linksOut($glist["ana"], $fopt); 
echo "</td></tr></table>\n";

if ( is_array($glist["prod"]) ) {
	$textoutObj->linksOut($glist["prod"], $fopt); 
}

$flist = NULL;

// ------------------------ 

$flist[] = array("ty"=>"head", "txt"=>"Misc", "lay"=>"1" );
// $flist[] = array("ty"=>"lnk",   "href"=> "obj.exp.l_qccheck.php" , "txt"=>"[QC_checker] ");

$flist[] = array("ty"=>"lnk",   "href"=> "obj.exp.imp_sample.php" , "txt"=>"[ProtoImporter] Import protocol parameters (e.g. samples)", "icon"=>"images/f.obj.exp.impproto.gif");
// $flist[] = array("ty"=>"lnk",   "href"=> "obj.exp.prot_samp.php" , "txt"=>"[ExpName2Proto] Update protocol parameters from exp-name", "icon"=>"images/f.obj.exp.upproto.gif");

//$flist[] = array("ty"=>"lnk",   "href"=> "obj.exp.list_cpref.php" , "txt"=>"Copy reference points to other experiments");
//$flist[] = array("ty"=>"lnk",   "href"=> "objtools/EXP.create_by_img.gui.php" , "txt"=>"Create batch of experiments by images");
//$flist[] = array("ty"=>"lnk",   "href"=> "obj.exp.l_impmat.php" , "txt"=>"Import virtual experiment matching results [vimares]");
//$flist[] = array("ty"=>"lnk",   "href"=> "objtools/EXP.uniqueimg.php", "txt"=>"Create unique image-objects" );
$flist[] = array("ty"=>"lnk",   "href"=> "p.php?mod=DEF/o.EXP.result_archive", "txt"=>"Archive experiment results to file" );
$flist[] = array("ty"=>"lnk",   "href"=> "p.php?mod=DEF/o.EXP.li_arch", "txt"=>"Archive experiment results to ArchDB" );
$flist[] = array("ty"=>"lnk",   "href"=> "p.php?mod=DEF/o.EXP.li_arch&mode=recover", "txt"=>"Recover experiment results from ArchDB" );

$flist[] = array("ty"=>"headclose"); 

// ------------------------ 

// $flist[] = array("ty"=>"head", "txt"=>"Expression profiling", "lay"=>"1" );
// $flist[] = array("ty"=>"lnk",   "href"=> "objtools/EXP.rank.php" , "txt"=>"Show rank of experiment results");
// $flist[] = array("ty"=>"lnk",   "href"=> "objtools/EXP.correlation.php" , "txt"=>"Scatter plot (correlation)");
//$flist[] = array("ty"=>"lnk",   "href"=> "objtools/EXP.miame_test_li.php" , "txt"=>"MIAME test");
//$flist[] = array("ty"=>"lnk",   "href"=> "objtools/EXP.biocop.php" , "txt"=>"AT Expressionist");
//$flist[] = array("ty"=>"headclose"); 
 
$textoutObj->linksOut($flist, $fopt); 

echo "</ul>"; 

htmlFoot("<hr>");
