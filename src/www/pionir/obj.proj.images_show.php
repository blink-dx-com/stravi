<?php
/*MODULE: obj.proj.images_show.php
  DESCR: - show images of a list of experiments/images
  		 - support of IMG, EXP, PROJ, W_WAFER
  AUTHOR: Steffen Kube
  VERSION: 2020-11-05 (samll refactoring)
  INPUT:  
  	[$proj_id]       = project-id
	$tablename  	 ="PROJ" or "EXP" or "IMG"
	$go				 =  0 just show
						1 set params
						2 set also advanced params
	$img_params 	 = (display params for ImageMagick)
	$img_para_arr[]	 = "-normalize", "-negate" = 0,1
	$opt["zoomFact"] = -2,-1,0,1,2   

   OPTIONAL:
   
	[$actionx] 		 = ["show"] | "print" | "copy"
	[$advanced_show] = 0|1
	[$img_per_row]   = NUM
	[$optShowSample] = 0|1
	$opt[refpoints]  = 0|1
	$opt[namelen]    = 0 | LEN
	
	$opt[maxshow]    = [200] : set maximum of shown images
	$opt[forcecalc]  = [0]|1 : force recalculation
	
	$selx[expid]     = if actionx="copy" => copy selx[exp_ids] to clipboard
	
   GLOBALS: 	$_SESSION['userGlob']["o.PROJ.expseries"] : prefs
   DEPRICATED:
    $dim[0], [1] 	 = x,y size  (2004-01-15)
  	
*/

// extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc");

require_once ("sql_query_dyn.inc");
require_once ('class.history.inc');
require_once ("o.EXP.subs.inc");
require_once ('f.clipboard.inc');
require_once ("glob.image.inc");
require_once ('f.imgparams.inc');

function this_expshow(&$sql2, $exp_id, $exp_name, 
	&$scriptOptions, 
	&$imgOpt /*
			"dimen"	 	  dimension
			"img_params"  imaging parameters
			"imgOptx"     other visualization options
			*/
	) {


	this_objShow("EXP", $exp_id, $exp_name,  $scriptOptions["nameLen"]);
      
	if ( $scriptOptions["showSample"] ) {
		$samplenames = this_getSamples($sql2, $exp_id);
		this_objShow("SAMPLE", "", $samplenames,  $scriptOptions["nameLen"]);
	}
	$sqls= "select x.img_id, y.name from exp x, img y where x.exp_id=".$exp_id. " AND x.img_id=y.img_id";
	$sql2->query("$sqls");
	$sql2->ReadRow();
	$image_id = $sql2->RowData[0];
	
	if ( $image_id ) {

		$image_name_url = $sql2->RowData[1];
		this_objShow("IMG", $image_id, $image_name_url,  $scriptOptions["nameLen"]);
		$ret = org_img_show ( $sql2, $image_id, $imgOpt["dimen"], $imgOpt["img_params"], $image_name_url, $imgOpt["imgOptx"] );

	}
	echo "<hr size=1 NOSHADE color=#E0E0FF></td>";
}

function this_showZoom( $tmpChkArr, $advanced, $zoomfactor ) {
	  if ( !$advanced_show )     
	  
	  $separator = " &nbsp;&nbsp;";
	  if ($advanced) $separator = "<br>";
	  
	  foreach( $tmpChkArr as $val=>$txt) {
	  		$tmpcheck="";
			if ($zoomfactor==$val) $tmpcheck=" checked";
	  		$tmptxt .= "<input type=radio name=\"opt[zoomFact]\" value=\"".$val."\" ".$tmpcheck.">".$txt.$separator;
	  }
	  return ($tmptxt);
}

function this_objShow(
	$tablename,  // "EXP", "IMG", "SAMPLE", "W_WAFER"
	$objid, 
	$objname, 
	$nameLenMax  // 0 : auto
	 ) {
	 
	$lener     = $nameLenMax;
	$shortName = $objname;
	
	if ( $nameLenMax>0 AND strlen($objname) > $nameLenMax ) {
		$start = strlen ($objname) - $nameLenMax;
		$shortName = "...". substr($objname, $start, $nameLenMax);
	}
	echo "<img src=\"images/icon.".$tablename.".gif\" TITLE=\"".htmlspecialchars($objname)."\"> ";
	if ($tablename!="SAMPLE") echo "<a href=\"edit.tmpl.php?t=".$tablename."&id=".$objid."\"> ";
	echo htmlspecialchars($shortName);
	if ($objname=="" AND $objid!="") echo "[$objid]";
	echo "</a>";
	if ($tablename=="IMG") echo " <a href=\"obj.img.showHuge.php?img_id=$objid&opx[mode]=slim&opx[profile]=galery\" target=_img><img src=\"images/ic.zoom.gif\" border=0></a>";
	if ($tablename=="EXP") echo " <input type=checkbox name=selx[".$objid."] value=1>";
	echo "<br>\n";
}

function this_getSamples( &$sql, $exp_id ) {
    $samples  = oEXP_subs::getSamplesWithName($sql, $exp_id);
	if (!sizeof($samples)) return;
	$komma = "";
	$nameall = "";
	foreach( $samples as $id=>$name) {
		$nameall .= $komma. $name;
		$komma = ", ";
	}
	return ($nameall);
}

function print_separator ($advanced_show) {
	if ($advanced_show) echo "</tr>\n<tr bgcolor=#EFEFEF>";
} 

function this_printout( $key, $text, $advanced_show, 
	$notes=NULL, 
	$opt  =NULL // "headline" 0|1
		 	    // "bgcolor"  e.g. #FFFFF
	) {
	
    $bgcolor="";
	
	if ($opt["headline"]) {
		$bgcolor="#E0E0FF";
		if ($opt["bgcolor"]!="") $bgcolor=$opt["bgcolor"];
		echo "<tr bgcolor=$bgcolor><td colspan=3><B>".$text."</B></td>";
		echo "</tr>\n";
		return;
	}
	
    if (!$advanced_show) {
        echo "<td align=center bgcolor=#EFEFEF>";
        echo "<font size=-1 color=gray><B>".$key."</B><br>".$text;
        echo "</font></td>\n";
    } else {    
        echo "<tr bgcolor=#EFEFEF><td>";
        echo "<font color=gray><B>".$key."</B></font></td><td>".$text;
        echo "</td>";
		echo "<td><I>".$notes."</I></td>";
        echo "</tr>\n";
    }
    
}


// ------------------------------------------------------------------
$sql  = logon2( $_SERVER['PHP_SELF'] );
$sql2 = logon2( $_SERVER['PHP_SELF'] );
// $sql3 = logon2( $_SERVER['PHP_SELF'] );

$proj_id=$_REQUEST['proj_id'];
$tablename=$_REQUEST['tablename'];
$go=$_REQUEST['go'];
$img_params=$_REQUEST['img_params'];
$img_para_arr=$_REQUEST['img_para_arr'];
$opt=$_REQUEST['opt'];
$optShowSample=$_REQUEST['optShowSample'];
$actionx=$_REQUEST['actionx'];
$advanced_show=$_REQUEST['advanced_show'];
$img_per_row=$_REQUEST['img_per_row'];
$selx=$_REQUEST['selxexpid'];


if ($tablename=="" ) $tablename="PROJ"; 

$title = "Image gallery";
$infoarr = NULL;
$infoarr["scriptID"] = "";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "list";
$infoarr['help_url'] = 'o.PROJ.img_galery.html';
$infoarr['back_url'] = 'view.tmpl.php?t='.$tablename;
$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1; 

if ($tablename=="PROJ") {
	$infoarr["obj_id"]   = $proj_id;
	$infoarr["form_type"]= "obj";
	$infoarr["obj_cnt"]  = 0; 
}
$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sql, $infoarr);

$infTabAll = array ("PROJ"=>1,  "IMG"=>1, "EXP"=>1);
$dimIni=array();
$dimIni[0]  = 350;
$dimIni[1]  = 250;
$show_limit_ini = 100;

$tableShow  = $tablename;
$show_limit = $show_limit_ini;

$i	    = 0; 
//$retVal = 0;
$scriptOptions  = unserialize($_SESSION['userGlob']["o.PROJ.expseries"]);
$images_per_row = 2;

if ( $go ) {
	$scriptOptions["showSample"]    = $optShowSample;
	if ($img_per_row>0)		   $scriptOptions["imgPerRow"] = $img_per_row;
	if ($opt["namelen"] !="")  $scriptOptions["nameLen"]   = $opt["namelen"];
	if ($opt["zoomFact"]!="")  $scriptOptions["zoomFact"]  = $opt["zoomFact"];
	$scriptOptions["refpoints"] = $opt["refpoints"];
	if ($scriptOptions["nameLen"]=="") $scriptOptions["nameLen"] = 40;
	
	if ( $go > 1 ) { // advanced params
		
		if ($opt["maxshow"]>0 OR $opt["maxshow"]=="") $scriptOptions["maxshow"]   = $opt["maxshow"];
	}
	
	$_SESSION['userGlob']["o.PROJ.expseries"]   = serialize($scriptOptions); // write back
}
if ( $scriptOptions["imgPerRow"] > 0 ) {
	$images_per_row = $scriptOptions["imgPerRow"];
}
if ($scriptOptions["refpoints"]) $opt["refpoints"] = $scriptOptions["refpoints"];

$dimen[0]=$dimIni[0];
$dimen[1]=$dimIni[1];
if ($scriptOptions["maxshow"] > 0)  $show_limit = $scriptOptions["maxshow"];
/*if ($dim[0] && $dim[1]) {
	$dimen[0]=$dim[0];
	$dimen[1]=$dim[1];
}
*/

if (!empty($img_para_arr) ) {
	$img_params = fImgParams::paramArray2str( $img_para_arr );
}

if (!$scriptOptions["zoomFact"]) $scriptOptions["zoomFact"] = "0";

if ($scriptOptions["zoomFact"]==0) {	// reset
	$dimen=$dimIni;
}

if ($scriptOptions["zoomFact"]==1) {	// zoom
	$dimen[0]=$dimIni[0]*2.0;
	$dimen[1]=$dimIni[1]*2.0;
}

if ($scriptOptions["zoomFact"]==2) {	// zoom
	$dimen[0]=$dimIni[0]*4.0;
	$dimen[1]=$dimIni[1]*4.0;
}

if ($scriptOptions["zoomFact"]<0) { // zoom in
	$factor = 1.0 / (float)(abs($scriptOptions["zoomFact"])+1); // -1, -2
	$dimen[0]=(int)($dimIni[0]*$factor);
	$dimen[1]=(int)($dimIni[1]*$factor);
}
$img_params_enc = urlencode($img_params); 
    


$imgOpt = NULL;	/*
			"dimen"	 	  dimension
			"img_params"  imaging parameters
			"imgOptx"     other visualization options
				*/
$imgOpt["dimen"] 	  = $dimen;
$imgOpt["img_params"] = $img_params;

         
gHtmlMisc::func_hist("obj.proj.images_show", $title, 'obj.proj.images_show.php?tablename='.$tablename );


$found=0;
foreach( $infTabAll as $key=>$val) {
	if ($tablename == $key) $found=1;
}
if (!$found) {
	htmlFoot("Error", "Table '$tablename' not allowed for this function");
}

$sqlAfterNoSort='';

if ($tablename=="PROJ") {
	/* get  project name*/  
    
	$i_tableNiceName = "experiment in project";
    
    if ( !$proj_id ) {
		htmlFoot("Error","Need a project!");
	}
	
	$sqlAfterNoSort  = "exp x where x.exp_id IN (select PRIM_KEY from PROJ_HAS_ELEM where PROJ_ID=".$proj_id." AND TABLE_NAME='EXP' )";
	$sqlAfterSort    = $sqlAfterNoSort. " order by x.name";
	
	// 
	//   check   R I G H T S
	//
	$t_rights = tableAccessCheck( $sql, $tablename );
	if ( $t_rights["read"] != 1 ) {
		tableAccessMsg( $i_tableNiceName, "read" );
		htmlFoot();
	}
	
	$o_rights = access_check($sql, $tablename, $proj_id);
	if ( !$o_rights["read"] ) htmlFoot("ERROR", "no read permissions on ".$i_tableNiceName."!");
    
}


if ($tablename=="EXP") {

	$i_tableNiceName = "experiment from list";

	$tablename_tmp="EXP";

	$tmp_info    = $_SESSION['s_tabSearchCond'][$tablename_tmp]["info"];
	
	$sqlAfterNoSort  = get_selection_as_sql( $tablename);
	$sqlopt=array();
	$sqlopt["order"] = 1;
	$sqlAfterSort  = get_selection_as_sql( $tablename, $sqlopt);
	
	if ( $tmp_info=="" ) {
		htmlFoot("Error","Please select experiments from list!");
	}
	
	// 
	//   check   R I G H T S
	//
	$t_rights = tableAccessCheck( $sql, $tablename );
	if ( $t_rights["read"] != 1 ) {
		tableAccessMsg( $i_tableNiceName, "read" );
		htmlFoot();
	}
	
	
}	


if ($tablename=="IMG")  {
	$i_tableNiceName = "image from list";
	$tablename_tmp="IMG";
	
	$tmp_info    = $_SESSION['s_tabSearchCond'][$tablename_tmp]["info"];
	
	$sqlAfterNoSort  = get_selection_as_sql( $tablename);
	$sqlopt=array();
	$sqlopt["order"] = 1;
	$sqlAfterSort  = get_selection_as_sql( $tablename, $sqlopt);
    
	if ( $tmp_info=="" ) {
		htmlFoot("Error","Please select elements from list!");
	}
	
	// 
	//   check   R I G H T S
	//
	$t_rights = tableAccessCheck( $sql, $tablename );
	if ( $t_rights["read"] != 1 ) {
		tableAccessMsg( $i_tableNiceName, "read" );
		htmlFoot();
	}
	
}


if ( !$advanced_show ) {

    $sqls= "select count(1) from ".$sqlAfterNoSort;
	$sql->query($sqls);
	$sql->ReadRow();
	$obj_num=$sql->RowData[0];
	
}



if ( !$advanced_show) {
	$gotmp = 1;
	echo "[<a href=\"".$_SERVER['PHP_SELF']."?proj_id=$proj_id&tablename=$tablename&img_params=$img_params_enc&actionx=print\">Printer view</a>]&nbsp;";
	echo "<font color=gray>Selection:</font> <b>$i_tableNiceName</b> ";
	echo "\n";
} else { 
	$gotmp = 2;
}

?> 
<font color=gray>Parameters:</font> 
<? if ( !$advanced_show ) { ?>
	[<a href="<?echo $_SERVER['PHP_SELF']?>?proj_id=<?echo $proj_id?>&tablename=<?echo $tablename?>&advanced_show=1">advanced</a>] 
<? } else { ?>
	<B>advanced</B> [<a href="<?echo $_SERVER['PHP_SELF']?>?proj_id=<?echo $proj_id?>&tablename=<?echo $tablename?>">easy</a>] 
<? } 
echo "<I>".$img_params."</I>\n"; 
if ($opt["forcecalc"]>0) echo " Action: calculate fresh thumbnails, do not use cached thumbnails.";
echo "<br>";

echo "<ul>";
?>
<table bgcolor=#DFDFDF border=0 cellspacing=1 cellpadding=1>
<form method="post" name=editform action="<?echo $_SERVER['PHP_SELF']?>?proj_id=<?echo $proj_id?>&tablename=<?echo $tablename?>">
<?
echo '<input type="hidden" name="actionx" value="show">'."\n";
echo "<input type=\"hidden\" name=\"go\" value=\"$gotmp\">\n";

if ( $actionx!="print" ) {
	if ( $advanced_show ) $theSeperator = "<br>";
	else   $theSeperator = "";  
	
	$tmpChkArr = array (-2 => 0.25, -1=>0.5, "0"=>"[1]", 1=>2, 2=>4);
	$tmptxt = this_showZoom( $tmpChkArr, $advanced_show, $scriptOptions["zoomFact"] );
	
	this_printout( "Zoom", $tmptxt, $advanced_show, "Set the zoom factor"); 
	$tmpChecked = $scriptOptions["showSample"] ? " checked" : ""; 
	if ($tablename!="IMG") // only useful for PROJ or EXP
		this_printout( "Sample","<input type=checkbox name=optShowSample value=\"1\" $tmpChecked>", $advanced_show, "Show sample ?");
	this_printout( "Images per row","<input name=img_per_row value=\"".$images_per_row."\" size=3>", $advanced_show, "Number of images per row [1..5]");
     

    if ( $advanced_show ) {
		$tmpChecked =  $scriptOptions["refpoints"] ? " checked" : "";
	    this_printout( "Show reference spots","<input type=checkbox name=opt[refpoints] value=\"1\" ".$tmpChecked.">", $advanced_show);
        
		this_printout( "Length of names","<input type=text name=opt[namelen] size=3 value=\"".$scriptOptions["nameLen"]."\" >", $advanced_show,"maximum length of names (full name : 0; typical: 40)");
        
		
		this_printout( "ViewLimit","<input type=text  name=opt[maxshow]  value=\"".$scriptOptions["maxshow"]."\" size=5 >", $advanced_show, "Set the maximum number of images; default: ".$show_limit_ini);
        this_printout( "ForceCalc","<input type=checkbox  name=opt[forcecalc]  value=1>", $advanced_show, "Force recalculation of thumbnails, do not use cached thumbnails");
        
		
		
		$tmpOpt = array ("headline"=>1, "bgcolor" => "#FFFFFF");
		this_printout( "","&nbsp;",1,"", $tmpOpt);
		$tmpOpt = array ("headline"=>1);
		this_printout( "","Options for image processing",1, "", $tmpOpt);
		this_printout( "Normalize", "<input type=checkbox name=img_para_arr[\"-normalize\"] value=\"on\" >", $advanced_show); 
		this_printout( "Invers","<input type=checkbox name=img_para_arr[\"-negate\"] value=\"on\" >", $advanced_show);
        this_printout( "Threshold",	"<input type=text size=6 name=img_para_arr[\"-threshold\"] value=\"\" >", $advanced_show);
        this_printout( "Flip", "<input type=checkbox name=img_para_arr[\"-flip\"] value=\"on\" >",$advanced_show);
        this_printout( "Gamma","<input type=text size=6 name=img_para_arr[\"-gamma\"] value=\"\" >", $advanced_show, "value 0.1 ... +3");
        this_printout( "Swirl","<input type=text size=6 name=img_para_arr[\"-swirl\"] value=\"\" >", $advanced_show, "value 0..90");
        this_printout( "Rotate","<input type=text size=6 name=img_para_arr[\"-rotate\"] value=\"\" >", $advanced_show, "value 0..90");
        this_printout( "Shear ","<input type=text size=6 name=img_para_arr[\"-shear\"] value=\"\" >", $advanced_show, "&lt;x_degrees&gt;x&lt;y_degrees&gt; degrees=(0..90)");
        this_printout( "Colors","<input type=text size=6 name=img_para_arr[\"-colorize\"] value=\"\" >", $advanced_show, "  (value R/G/B) (e.g. 0/0/70)");
        this_printout( "&nbsp;","<input type=submit value=\" Set \">", $advanced_show, "");
        
		echo "<tr>"; // for SUBMIT button
    } else {
		echo "<td align=center><br>";
		echo "<input type=submit value=\"Set\">";
		echo "</td>\n";
		echo '<td align=center bgcolor=#EFEFEF><br><input type=button value="copy selected"  onclick="document.editform.actionx.value=\'copy\'; document.editform.submit();"></td>'."\n";
	
	}
	
}
    
echo "</tr>";
echo "</table>\n&nbsp;<br>";	

if ( $advanced_show ) {
	echo "</ul>";
	return 0;
}  
  
if ( $actionx == "copy" ) {
 	echo "<ul>.. copy <b>".sizeof($selx)."</b> selected experiments to clipboard ...<br>";
	if ( !sizeof($selx) ) {
		 htmlInfoBox( "Copy failed", "No experiments selected!", "", "INFO" );
	} else {
		clipboardC::obj_put ( "EXP", $selx );
		echo ".. ready<br><br><hr>";
 	}
	echo "</ul>";
	htmlFoot();
}


$options_imgShow = array();	
if ($opt["refpoints"]) $options_imgShow["show_ref_points"] = 1;
if ( $tableShow!="IMG" ) {
	$options_imgShow["noImageLogo"]=1;
	$options_imgShow["noImgBotTxt"]=1;
}

if ($opt["forcecalc"]>0) $options_imgShow["refresh"] = 1;
$imgOpt["imgOptx"] = $options_imgShow;

echo "<table>";

do {    

	if (($tableShow=="EXP") || ($tableShow=="PROJ")) {
		
		$sqls= "select x.exp_id, x.NAME from ". $sqlAfterSort;
		$sql->query("$sqls");
		
		
		$i=0;
		while ( $sql->ReadRow() ) {
			
			$exp_id  = $sql->RowData[0];
			$exp_name= $sql->RowData[1];
	
			echo "<td valign=top>";
			this_expshow($sql2, $exp_id, $exp_name, $scriptOptions, $imgOpt);
			$i++ ;
			
			echo "</td>";
			if ( ($i/$images_per_row) == (int)($i/$images_per_row) ) {
					echo "</tr>";
					$newtr=1;
			}
			if ( $i>=$show_limit ) break;
	
		}
		break;
	}
	
	
		
	if ($tableShow=="IMG") {
	
		/* get  images */
		$sqls= "select x.img_id, x.name from ".$sqlAfterSort;
		$sql->query("$sqls");
	
		echo "<table>";
		$i=0;
		$newtr=1;
		
		while ( $sql->ReadRow() ) {
		
			if ($newtr) {
				echo "<tr>";
				$newtr=0;
			}
			echo "<td valign=top>"; 
	
			$image_id      =$sql->RowData[0]; 
			$image_name_url=$sql->RowData[1];
	
			$ret = org_img_show ( $sql2, $image_id, $dimen, $img_params, $image_name_url, $options_imgShow );
	
			echo "</td>";
			
			$i++;
			if ( ($i/$images_per_row) == (int)($i/$images_per_row) ) {
				echo "</tr>";
				$newtr=1;
			}
			if ( $i>=$show_limit ) break;
			
		}
		
		break;
	}
	
} while (0);

echo "</table>";
echo "</form>\n";

if ( $i>=$show_limit ) {
	echo "<br>";
	htmlInfoBox( "Displaying stopped.", "Too many images selected. ViewLimit is set to <b>$show_limit</b>. You can set the ViewLimit in the advanced options.", "", "WARN" );
	echo "<br>";
}

echo "selected objects: <B>$obj_num</b>";
echo "<hr size=1 NOSHADE>\n";
htmlFoot();
	
