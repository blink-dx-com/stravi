<?php
/**
 * Import values into EXTRA_PREF_VAL
 * @package obj.extra_attrib.import_pref.php
 
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   
 *   $id   (attrib_id)
  	 $go = 0|1
  	 $content
 */
session_start(); 


require_once ('db_access.inc');
require_once ('globals.inc');
require_once ('func_head.inc');
require_once ('access_check.inc');

$error = & ErrorHandler::get();
$sql   =  logon2( $_SERVER['PHP_SELF'] );
// $sql2  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();

$id = $_REQUEST['id'];
$go = $_REQUEST['go'];
$content = $_REQUEST['content'];

$title = 'Import preffered values for attribute to EXTRA_PREF_VAL';

$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";

$infoarr["obj_name"] = "EXTRA_ATTRIB";
$infoarr["obj_id"] = $id;

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
$maxvalnum=40;

echo "<blockquote>";
if (!$id) {
	echo "ERROR: no ATTRIB_ID.<br>";
	return;
}

if ($_SESSION['sec']['appuser']!="root") {
	echo "ERROR: only root can do it!<br>";
	return;
}

$sql->query('SELECT name, nice_name FROM extra_attrib WHERE extra_attrib_id = '. $id);
if ($sql->ReadRow()) {
	$attrib_name = $sql->RowData[0];
	$attrib_name_nice = $sql->RowData[1];
} else {
	echo "ERROR: ATTRIB_ID $id not found!<br>";
	return;
}

echo "Attribute: <b>$attrib_name_nice</B> (Code: $attrib_name) (ID:$id)<br><br>";
$numprefs=0;
echo "Defined entries ins EXTRA_PREF_VAL: ";
$sql->query('SELECT count(*) FROM EXTRA_PREF_VAL WHERE EXTRA_ATTRIB_ID = '. $id);
if ($sql->ReadRow()) {
	$numprefs = $sql->RowData[0];
	echo "<a href=\"view.tmpl.php?t=EXTRA_PREF_VAL&condclean=1&searchCol=EXTRA_ATTRIB_ID&searchtxt=$id\">$numprefs</a>";
} else {
	echo "0";
}
echo "<br>\n";

if ( $go and strlen($content)) {
	$text_arr = explode("\n", $content);
	$cnt=0;
	foreach( $text_arr as $line) {

		$line_arr = explode("\t", $line);
		$tmpval   = trim($line_arr[0]);
		$tmpnotes = trim($line_arr[1]);
		
		if ($tmpval==='' or $tmpval===NULL) {
		    echo "Line ".($cnt+1)." : no value given ...<br>";
		    $cnt++;
		    continue;
		}
		
		echo "[$tmpval] $tmpnotes : \n";
		$sql->query("SELECT VALUE FROM EXTRA_PREF_VAL WHERE EXTRA_ATTRIB_ID = ". $id. " AND VALUE=".$sql->addQuotes($tmpval) );
		if ($sql->ReadRow()) {
			echo "exists.";
		} else {
			$colstr = "EXTRA_ATTRIB_ID, VALUE, NOTES";
			$valstr = $id .",'" .$tmpval."','" .$tmpnotes. "'";
			$retval = $sql->Insertx ( "EXTRA_PREF_VAL", $colstr, $valstr);
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
		$cnt++;
	} 
	echo "ready.<br><br>\n";
}

?>
Please give not more than <b><?=$maxvalnum?></b> values<br><br>

Give VALUE &lt;TAB&gt; NOTES<br>
<form method="post" action="<?echo $_SERVER['PHP_SELF']?>?go=1&id=<?echo $id?>" > 
<textarea name=content rows="15" cols="72" >
</textarea>
<br>
<INPUT type=submit value="submit" >        
</form>
<?

htmlFoot();
