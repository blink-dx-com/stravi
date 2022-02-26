<?php
/**
 * Import values into H_VAL_INIT
 * @package obj.cct_column.pref_import.php
 * @swreq UREQ:-
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $tablename
 *   $content string - content of values
  	 $column string
  	 $go = 0|1
 * @version $Header: trunk/src/www/pionir/obj.cct_column.pref_import.php 59 2018-11-21 09:04:09Z $
 */


//extract($_REQUEST); 
session_start(); 


require_once ('db_access.inc');
require_once ('globals.inc');
require_once ('func_head.inc');
require_once ('access_check.inc');

$error = & ErrorHandler::get();
$sql   =  logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();

$maxvalnum=200;

$tablename = $_REQUEST['tablename'];
$column    = $_REQUEST['column'];
$go        = $_REQUEST['go'];
$content   = $_REQUEST['content'];


$title = 'Import preffered values for column to H_VAL_INIT';
$infoarr			 = NULL;
$infoarr["scriptID"] = "obj.cct_column.pref_import.php";
$infoarr["title"]    = $title;
$infoarr["title_sh"]  = "Import values to H_VAL_INIT";
$infoarr["form_type"]= "tool"; // "tool", "list"

$infoarr["locrow"] = array( 
	array('edit.tmpl.php?t=CCT_COLUMN&id='.$tablename.'&primas[1]=COLUMN_NAME&primasid[1]='.$column,
		    "col: $column")
	);

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);



echo "&nbsp;[<a href=\"view.tmpl.php?t=H_VAL_INIT\">List view of H_VAL_INIT</a>]<br>";

echo "<blockquote>";
if ($tablename=="" || $column=="") {
	echo "ERROR: no TABLE_NAME or COLUMN_NAME given.<br>";
	return;
}

if ($_SESSION['sec']['appuser']!="root") {
	echo "ERROR: only root can do it!<br>";
	return;
}

echo "Import for column: <B>$tablename</B>::<B>$column</B><br>\n";
echo "MAX Import-values: ".$maxvalnum."<br>";

$sql->query("SELECT nice_name, CCT_TABLE_NAME, PRIMARY_KEY FROM CCT_COLUMN WHERE TABLE_NAME='" .$tablename. "' AND COLUMN_NAME='".$column."'");
if ($sql->ReadRow()) {
	$nicename 	= $sql->RowData[0];
	//$mother_tab 	= $sql->RowData[1];
	$pk_is 		= $sql->RowData[2];
	echo "Nicename: $nicename<br>\n";
} else {
	echo "<B>ERROR:</B> entry in CCT_COLUMN not found!<br>\n";
	return;
}

/*
if ($mother_tab!="") {
	echo "<B>INFO</B>: The column is a foreign key to $mother_tab. No predefined values possible.<br>\n";
	return;
} 
*/

if ($pk_is>0) {
	echo "<B>INFO</B>: The column is primary key. No predefined values possible.<br>\n";
	return;
} 

echo "<br><br>\n";

$numprefs=0;
echo "Defined entries ins H_VAL_INIT: ";
$sql->query("SELECT count(*) FROM H_VAL_INIT WHERE TABLE_NAME='" .$tablename. "' AND COLUMN_NAME='".$column."'");
if ($sql->ReadRow()) {
	$numprefs = $sql->RowData[0];
	$tableSCond= urlencode("TABLE_NAME='" .$tablename. "' AND COLUMN_NAME='".$column."'");
	echo "<a href=\"view.tmpl.php?t=H_VAL_INIT&condclean=1&tableSCond=$tableSCond\"><B>$numprefs</B></a>";
} else {
	echo "0";
}
echo "<br>\n";

if ( $go && strlen($content)) {
	$text_arr = explode("\n", $content);
	foreach( $text_arr as $line) {
	    
		$line_arr = explode("\t", $line);
		$tmpval   = trim($line_arr[0]);
		$tmpnotes = trim($line_arr[1]);
		echo "[$tmpval] $tmpnotes : \n";
		
		if ($tmpval==='' or $tmpval===NULL) {
		    echo " --- EMPTY value !<br>\n";
		    continue;
		}
		
		$sql->query("SELECT VALUE FROM H_VAL_INIT WHERE TABLE_NAME='" .$tablename. "' AND COLUMN_NAME='".$column."' AND VALUE='".$tmpval."'");
		if ($sql->ReadRow()) {
			echo "exists.";
		} else {
			$colstr = "TABLE_NAME, COLUMN_NAME, VALUE, NOTES";
			$valstr = "'". $tablename ."','" .$column.	"','" .$tmpval.	"','" .$tmpnotes. "'";
			$retval = $sql->Insertx ( "H_VAL_INIT", $colstr, $valstr);
			if (!$retval) {
				echo "<font color=red>insert failed.</font>";
			} else {
				echo "<font color=green>inserted.</font>";
				$numprefs++;
			}
		}
		echo "<br>\n";
		if ($numprefs>$maxvalnum) {
			echo "<b>WARNING:</B> Too many pref vals. Import stopped.<br>\n";
			break;
		}
	} 
	echo "ready.<br><br>\n";
}

?>
<I>Please give not more than <? echo $maxvalnum;?> values.</I><br><br>

Give VALUE &lt;TAB&gt; NOTES<br>
<form method="post" action="<?echo $_SERVER['PHP_SELF']?>?go=1&tablename=<? echo $tablename?>&column=<?echo $column?>" > 
<textarea name=content rows="15" cols="72" >
</textarea>
<br>
<INPUT type=submit value="submit" >        
</form>
<?

htmlFoot();

