<?php
/**
 * GUI for export objects paxml
 * @package export.select.php
 * @author  Rogo, Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param    [$cct_table]
		 [$id]
		 [$prefs]   	0|1 show preferences
		 [$asarchive]   0|1 save paxml with archive-option
 * @version $Header: trunk/src/www/pionir/impexp/partisanxml/export.select.php 59 2018-11-21 09:04:09Z $
 */


extract($_REQUEST); 
session_start(); 


require_once("func_head.inc");
require_once("PaXML_guifunc.inc");
require_once ('func_form.inc');


function this_rowout(
	$key, $val, $notes, 
	$opt=NULL // "optional" = 1
	) {
	echo "<tr>";
	echo "<td nowrap>";
	if ($opt["optional"]) echo "<font color=gray>";
	echo $key.":"; 
	if ($opt["optional"]) echo "</font>";
	echo " </td>";
	echo "<td nowrap>";
	echo $val;
	echo "&nbsp;<font color=gray>".$notes."</font>";
	echo "</td>";
	echo "</tr>\n";
	  	
}
	  
	  

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );

$title = "Paxml Export";
$infoarr = NULL;
$infoarr["help_url"] = "export_of_objects.html";
$infoarr["title"]    = $title;
if ($cct_table=="PROJ" AND $cct_id) {
	$infoarr["form_type"]= "obj"; 
	$infoarr["obj_name"] = "PROJ";
	$infoarr["obj_id"]   = $cct_id;
}
		
$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sql, $infoarr);

$colorarr = NULL;
$colorarr["important"] = "#DDDDFF";
	
echo "<br>";
$tabnice = "";
$icony   = '../../images/icon.UNKNOWN.gif';
if ($cct_table!="" AND $cct_id!="") { 
	$objnicename = obj_nice_name($sql, $cct_table, $cct_id);
	$tabnice     = tablename_nice2($cct_table);
	if ($objnicename == "") $objnicename = "[$id]";
	$iconx = '../../images/icon.'.$cct_table.'.gif';
    if ( file_exists($iconx) ) $icony=$iconx;
}


$retval = paxmlHelpC::exportCheckRole($sql, 1);
if (!$retval) htmlFoot();
?>

  <ul>
  <form name=exportform action="export.php" method="post">
   
	<table width=50% cellspacing=0 cellpadding=1 border=0 bgcolor=black>
	 <tr>
      <td>

       <table width=100% cellspacing=0 cellpadding=10 border=0 bgcolor=WHITE>
		
	<?
	if ($cct_table=="" OR $cct_id=="") { 
	?>
		<tr>
         <td nowrap>
          <font style=font-size:12px;> &nbsp; &nbsp; root table: &nbsp;</font>
		 </td>
         <td nowrap>
		  <input type=text size=30 name=cct_table style=font-size:12px>
		 </td>
		 <td rowspan=4 width=100% valign = top bgcolor=white>
          <font style=font-size:14px;color:gray>
          <p align=justify></p>
		 </font>
		</td>
       </tr>
	   <tr>
        <td nowrap>
         <font style=font-size:12px;> &nbsp; &nbsp; id: &nbsp;</font><br>
		</td>
        <td>
		 <input type=text size=30 name=cct_id style=font-size:12px><br>
		</td>
       </tr>
	   <tr>
	   
	  <?
	  } else {
	  ?>
	  <tr bgcolor=<?echo $colorarr["important"]?>>
         <td nowrap>
          <font style=font-size:12px;> &nbsp; &nbsp; <?echo $tabnice?>: &nbsp;</font>
		 </td>
         <td nowrap>
		   <? 
		   echo "<input type=hidden name='cct_table' value='$cct_table'>\n";
		   echo "<input type=hidden name='cct_id' value='$cct_id'>\n";
		   echo "<b>";
		   echo "<img src=\"$icony\"> ".$objnicename. " ";
		   ?></B>
		 </td>
       </tr>
	  	<?
	  }
	  
	  if ($prefs) {
	  	$optx = NULL;
	  	$optx["optional"] = 1;
	  	this_rowout("as archive", "<input type=checkbox name='asarchive' value='1'>", "save also database-user information", $optx);
		this_rowout("no tar",     "<input type=checkbox name='opt[notar]' value='1'>", "do not produce a TAR-file (for advanced users only)", $optx);
		this_rowout("no raw files", "<input type=checkbox name='opt[doNotTakeAttachments]' value='1'>", "do not export raw files of images, documents", $optx);
	  
		$selarr=array(
		    
		    "0.5"=>'tiny (only projects)',
		    "1"  =>'minimum (only objects)',
		    "2"  =>'normal (objects and class)',
		    "3"  =>'maximum (detailed output)',
		);
		if ($_SESSION['sec']['appuser'] == "root")
		    $selarr[4] ="debug output (only root)";
		
	  	$optx = NULL;
	  	$optx["optional"] = 1;
		$buttext = formc::selectFget('cct_out', $selarr, "0.5");
	  	this_rowout("info level", $buttext, "", $optx);
	 
	  
	  }
	  
	  ?>
	   
	   <tr>
        <td>
		 <?
		 if (!$prefs) {
		 	?>
         	<input type=button name=cct_options value="options" style="color: #303030; border: thin #DFDFDF; border-style:solid; border-width:1px;" onclick='location.href="<?echo $_SERVER['PHP_SELF']."?cct_id=$cct_id&cct_table=$cct_table&prefs=1";?>"'>
		 	<?
		 }
		 ?>
		 <br><br>
		</td>
        <td>
		 <input type=button name=cct_export value="   EXPORT   " style=font-size:12px onclick=javascript:document.exportform.submit()><br><br>
		</td></tr>
	   </table>
   	  </td>
     </tr>
	</table>
	<font color=gray>|</font> <a href="info.php"><font color=gray>view DB specific paxml settings</font></a>  <font color=gray>|</font>
	
   </center>
  <input type=hidden name='prefs' value='0'>
  </form>
  <?php 
  
  $pagelib->htmlFoot();
