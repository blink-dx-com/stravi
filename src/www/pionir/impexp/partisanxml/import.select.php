<?php
/**
 * GUI for paxml import
 * @package import.select.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $proj_id
			
			$cctopt[asarchive]
			$formopt[]   
				"shPrefs" : 0|1 show prefs
				"notar"   : 0 : from tar
						    1 : import from Export-dir
				"projHidden" : (0|1)
 * @version $Header: trunk/src/www/pionir/impexp/partisanxml/import.select.php 59 2018-11-21 09:04:09Z $
 */


extract($_REQUEST); 
session_start(); 


require_once ("func_head.inc");
require_once ("role.inc");
require_once ("db_access.inc");
require_once ("PaXMLReader.inc");
require_once 'f.util_IniAna.inc';

class fPaxmlImpSel {

function __construct($formopt, $proj_id) {
	$this->formopt=$formopt;
	$this->proj_id=$proj_id;
	
	if ( $formopt["projHidden"]!="1" ) $this->projHidden = 1;
	
	$this->colorarr = NULL;
	$this->colorarr["important"] = "#DDDDFF";
}

function formOpen() {
	$proj_id = $this->proj_id;
	
	$enctype = "enctype=multipart/form-data";
	if ( $this->formopt["notar"]>0 ) $enctype = "";
	
	echo "<br>\n";
	echo "<form action=\"import.php\" method=\"post\" $enctype>";
	?>
	<center>
	<table width=90% cellspacing=0 cellpadding=1 border=0 bgcolor=black>
	 <tr>
      <td width=50%>
	   <table width=100% cellspacing=0 cellpadding=10 border=0 bgcolor=white>
		
        <?php

        if ($proj_id == null)
            $proj_id = -1;

        if ($this->projHidden == 1)
            echo "<input type=hidden name=cct_proj value=\"$proj_id\">\n";
        else
        {

        	?>
            <tr><td nowrap valign=top><font style=font-family:Arial,Helvetica;font-size:12px;>
            &nbsp; &nbsp; project folder: &nbsp;</font>
            </td><td valign=top>
            <input type=text name=cct_proj value="<?=$proj_id; ?>" size=20 style=font-family:Arial,Helvetica;font-size:12px><br>
            <font style=font-family:Arial,Helvetica;font-size:10px;color:gray>'-1' means no project folder</font><br>
            </td></tr>
        	<?

        } 
}

function _oneRow($title, $varname, $notes, 
	$type // "file", "text"
	) {
		 echo "<tr bgcolor=".$this->colorarr["important"].">\n<td nowrap>";
         echo "<font style=font-size:12px;font-weight:bold> &nbsp;&nbsp; ".$title.": &nbsp;</font>";
		 echo "</td><td nowrap colspan=2>";
		 echo "<input type=$type size=50 name=".$varname." value=\"\" style=font-family:Arial,Helvetica;font-size:12px>";
		 echo "&nbsp; &nbsp;";
         echo "<font style=font-size:12px;color:gray>$notes</td>";
		 echo "</tr>\n";
}

function formMainRow() {
	
		
		$upload_max_filesize = ini_get_bytes('upload_max_filesize');
		$post_max_size = ini_get_bytes('post_max_size');
		$max_SUM_upload= min($upload_max_filesize, $post_max_size);
		$max_SUM_upload_mega = round($max_SUM_upload/1000000,1);

		$title = "paxml file";
		$varname  = "cct_file";
		$notes    = "Select a source file (*.tar, *.xml or *.gz). Max-Upload-Size: ".$max_SUM_upload_mega." Mbytes";
		
		$type  = "file";
		
		if ( $this->formopt["notar"]>0 ) {
			$title = "paxml file on TEMP";
			$type  = "text";
		}
		
		$this->_oneRow($title, $varname, $notes, $type);
		
		if ( $this->formopt["notar"]>0 ) {
			$this->_oneRow("SubDir", "cctopt[tempdir]", "'SubDir' (after WORK_PATH) of the export-dir on server", "text");
		}
		
}

}	

function thisRowOpen($kextext) {
	echo "<tr><td nowrap valign=top>";
	if ($kextext!="") 
		echo "<font style=font-size:12px;><font color=gray> &nbsp;&nbsp; ".$kextext.": &nbsp;</font></font>";
	echo "</td>";
}

function thisRow($kextext, $button, $notes) {
	thisRowOpen($kextext);
	echo "<td nowrap valign=top>";
	echo $button. "&nbsp;&nbsp; ".$notes."<br>";
	echo "</td>";
	echo "</tr>\n";
	
}

$error = & ErrorHandler::get();
$sql   = logon2(  );
if ($error->printLast()) htmlFoot();

$title   = "Paxml Import";
$infoarr = NULL;
$infoarr["help_url"] = "partisanxml.import.html";
$infoarr["title"]    = $title;
if ($proj_id) {
	$infoarr["form_type"]= "obj"; 
	$infoarr["obj_name"] = "PROJ";
	$infoarr["obj_id"]   = $proj_id;
}
	
$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sql, $infoarr);

$mainLib = new fPaxmlImpSel($formopt, $proj_id);

echo "<font style=color:gray>&nbsp;Info: This is the import tool for Paxml.</font><br>\n";

$paxml = new PaXMLReader($_SESSION['sec']['dbuser'], $_SESSION['sec']['passwd'], $_SESSION['sec']['db'], $_SESSION['sec']['_dbtype'], 0);


$role_right_name="f.PaXml_import";
$role_ok    = 0;
$role_right = role_check($sql, $role_right_name);
if ($role_right["execute"] > 0)
	$role_ok = 1;
if ( !$role_ok && ($_SESSION['sec']['appuser']!="root") )
{
	echo "<br>&nbsp;&nbsp;Sorry, you must have role right '$role_right_name' or be root.";
	return 0;
}


$mainLib->formOpen();

$mainLib->formMainRow();  
        
		
		if ( $formopt["shPrefs"] ) {
			thisRow("Start on project", "<input type=text name=\"cctopt[isp]\" value=\"\">", "start import on 'WIID:ROID'");
			thisRow("Options", "<input type=checkbox name=\"cctopt[update][files]\" value=1>", "update file attachments of existing objects");
			thisRow("", "<input type=checkbox name=\"cctopt[asarchive]\" value=1>", "import as archive (import user-IDs and other critical IDs)");
		
		
			thisRowOpen("Info level");
			?>
			<td nowrap valign=top><font style=font-family:Arial,Helvetica;font-size:12px;>
			<input type=radio name=cct_output value=0 checked>quiet &nbsp;&nbsp;&nbsp;(show nothing)<br>
			<input type=radio name=cct_output value=0.5 >tiny &nbsp;&nbsp;&nbsp;(show only projects)<br>
			<input type=radio name=cct_output value=1>minimal &nbsp;&nbsp;&nbsp;(show only objects)<br>
			<input type=radio name=cct_output value=2>show objects, class and system data<br>
			<input type=radio name=cct_output value=3>maximum for users (warning, this may a lot)<br>
			<?
	
			if ($_SESSION['sec']['appuser'] == "root")
				echo "<input type=radio name=cct_output value=4>see what an developer can see (only for root)!<br>";
	
			?>
			</font></td>
			<td rowspan=3 width=50% valign = top><font style=font-size:12px;color:gray>
			<p align=justify>If you get an unusual behavior, maybe an error, don't worry about your data. Objects, 
			which are not well described, will not be imported.
			</p>
			</font>
			</td>
			</tr>
			<?
		}
		?>
		<tr><td nowrap>
		<?
		 if (!$formopt["shPrefs"]) {
		 	$preflink = 1;
			$preftext = "options";
		 } else {
		 	$preflink = 0;
			$preftext = "unshow options";
		 }
		 
		?>
		<input type=button name=cct_options value="<?echo $preftext?>" style="color: #303030; border: thin #DFDFDF; border-style:solid; border-width:1px;" 
			onclick='location.href="<?echo $_SERVER['PHP_SELF']."?proj_id=$proj_id&formopt[projHidden]=".
			$mainLib->projHidden."&formopt[shPrefs]=".$preflink;?>"'>
		
		 <br><br>
		</td><td valign=top>
		<input type=submit name=cct_export value="IMPORT" style=font-family:Arial,Helvetica;font-size:12px>
		
		<? 
		if ( glob_isAdmin() ) {
			if ( $mainLib->formopt["notar"]<=0 )  {
				echo "&nbsp;&nbsp;&nbsp;[<a href=\"".$_SERVER['PHP_SELF']."?proj_id=".$proj_id."&formopt[notar]=1&formopt[shPrefs]=1\">import from Export-dir</a>]";
			} else {
				echo "&nbsp;&nbsp;&nbsp;[<a href=\"".$_SERVER['PHP_SELF']."?proj_id=".$proj_id."&formopt[notar]=0&formopt[shPrefs]=1\">import TAR-file</a>]";
		
			}
		}
		?>
		<br><br>
		</td></tr></table>
	</td></tr>
	</table>
	</center>
	</form>
	
	</body>
	
</html>
