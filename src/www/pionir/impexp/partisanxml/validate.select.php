<?php
/*
INPUT: $proj_id
		$projHidden (0|1)
*/

extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc");
require_once ("PaXMLValidator.inc");

$sql = logon2( $_SERVER['PHP_SELF'] );

$title = "Paxml Validator";
$infoarr			 = NULL;
$infoarr["scriptID"] = "validate.select.php";
$infoarr["title"]    = $title;
$infoarr["help_url"] = "export_of_objects.html";

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sql, $infoarr);


$paxml = new PaXMLValidator($_SESSION['sec']['dbuser'], $_SESSION['sec']['passwd'], $_SESSION['sec']['db'], $_SESSION['sec']['_dbtype'], 0);


$role_right_name="f.PaXml_import";
$role_ok = 0;
$role_right=role_check($sql, $role_right_name);
if ($role_right["execute"] > 0)
	$role_ok = 1;
if ( !$role_ok && ($_SESSION['sec']['appuser']!="root") )
{
	echo "<br>&nbsp;&nbsp;Sorry, you must have role right '$role_right_name' or be root.";
	return 0;
}


echo "<br>";
	
?>
 
	
	<form action="validate.php" method="post" enctype=multipart/form-data>

	<center>
	<?
	htmlInfoBox( "WARNING: Paxml-validator only available for old paxml-versions", "The validator is not longer supported.<br> It supports Paxml-files, which are older then 2005-01-03.<br> So use it on your own risk.", "", "INFO" );
	
	?>
	<br>
	<table width=90% cellspacing=0 cellpadding=1 border=0 bgcolor=black>
	 <tr>
      <td width=50%>
	   <table width=100% cellspacing=0 cellpadding=10 border=0 bgcolor=white>
		<tr><td colspan=3 align=center>
         <br>
		 <font style=font-family:Arial,Helvetica;font-size:16px;color:#0049BB><b>Validate Paxml file vs. Database</b></font>
		 <hr noshade width=75%><br>
		</td></tr>
		<tr bgcolor=#EEEEEE><td nowrap>
         <font style=font-family:Arial,Helvetica;font-size:12px;> &nbsp; &nbsp; file: &nbsp;</font>
		</td><td nowrap colspan=2>
		 <input type=file size=50 name=cct_file value="" style=font-family:Arial,Helvetica;font-size:12px><br>
         <font style=font-family:Arial,Helvetica;font-size:12px;color:black>Select a partisan xml source file (*.xml or *.gz or *.tar).
		</td>
		</tr>
		<tr><td nowrap valign=top>
         <font style=font-family:Arial,Helvetica;font-size:12px;>&nbsp; &nbsp; output level: &nbsp;</font>
		</td><td nowrap valign=top><font style=font-family:Arial,Helvetica;font-size:12px;>
		<input type=radio name=cct_output value=0.5 checked>tiny output (only objects)<br>
		<!-- <input type=radio name=cct_output value=1>minimal output (only objects)<br> //-->
		<input type=radio name=cct_output value=2>objects, class and system data for output<br>
		<input type=radio name=cct_output value=3>maximum output for users (warning, this may a lot)<br>
        <?

            if ($_SESSION['sec']['appuser'] == "root")
                echo "<input type=radio name=cct_output value=4>see what an developer can see (only for root)!<br>";

         ?>
		</font></td>
		<td rowspan=3 width=50% valign = top><font style=font-family:Arial,Helvetica;font-size:12px;color:gray>
		<p align=justify>This is a validation tool. No data will be written. The result may be interpreted quite 
		differently depending on execution before import or after import xml file.<br>
		Validation before import shows you, which data is actually new to your database. Also it shows which data 
		may not imported, although it's new data.<br>
		Validation after import shows you which data of xml file has been found in database. Also it shows data,
		not found, because of an import abort.
        </p>
		</font>
		</td>
		</tr>
<?php



    if ($proj_id == null)
        $proj_id = -1;

    //if ($projHidden == 1)
        echo "<input type=hidden name=cct_proj value=\"$proj_id\">\n";
    /*
	else
    {


		<tr><td nowrap valign=top><font style=font-family:Arial,Helvetica;font-size:12px;>
		&nbsp; &nbsp; project folder: &nbsp;</font>
		</td><td valign=top>
		<input type=text name=cct_proj value="<?=$proj_id; ?>" size=20 style=font-family:Arial,Helvetica;font-size:12px><br>
		<font style=font-family:Arial,Helvetica;font-size:10px;color:gray>'-1' means no proeject folder</font><br>
		</td></tr>


    }
    */
?>
		<tr><td nowrap>&nbsp;
		</td><td valign=top>
		<input type=submit name=cct_export value="validate" style=font-family:Arial,Helvetica;font-size:12px><br><br>
		</td></tr></table>
	</td></tr>
	</table>
	</center>
	</form>
	
	</body>
	
</html>
