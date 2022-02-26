<?php
/**
* produce SQL-insert query text
* @package export.php
* @swreq UREQ:xxxxxxxxxxxx
* @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
* @param   
*    $tablename	: tablename
     $go 0|1
     $colcheck
     $longsql    : 1/0
* @version $Header: trunk/src/www/pionir/rootsubs/glob.objtab.export.php 59 2018-11-21 09:04:09Z $
*/

extract($_REQUEST); 
session_start(); 


require_once ('reqnormal.inc');
require_once ("down_up_load.inc");
require_once ("object.subs.inc");

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );


if ($tablename == '') die("ERROR: give tablename.");

$cols = columns_get2($tablename);

$title		= 'Create SQL-commands to insert elements from TABLE';

$infoarr			 = NULL;
$infoarr['scriptID'] = '';
$infoarr['title']    = $title;
$infoarr['form_type']= 'list';
$infoarr['design']   = 'norm';
$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1;
$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sql, $infoarr);

if (!glob_isAdmin())  {
	$pagelib->htmlFoot('USERERROR', 'Only Admin can call this tool.');
}


if (!$go) {

	
	$sqls        = 'FROM '.$tablename.' x ';
	$fromClause  = $_SESSION['s_tabSearchCond'][$tablename]['f'];
	$tableSCondM = $_SESSION['s_tabSearchCond'][$tablename]['w'];
	$whereXtra   = $_SESSION['s_tabSearchCond'][$tablename]['x']; 

	if ( $tableSCondM ) {
	  $sqls .= ' WHERE '.$tableSCondM;
	}
	$sql->query('select count(*) '.$sqls);
	$sql->ReadRow();
	$num  = $sql->RowData[0];
	echo "<B>$num</B> elements selected.<br><br>";
	?>
	
	<form  method="get" action="<?echo $_SERVER['PHP_SELF']?>">
	<input type="hidden" name="tablename" value="<? echo $tablename;?>">
	<input type="hidden" name="go" value="1">
	<?
	
	foreach( $cols as $id=>$coltmp) {
		echo "<input type=checkbox name=colcheck[$coltmp] checked value=1> $coltmp<br>\n";
	}
	reset ($cols);
	?>
	
	<input type=submit>
	</form>
	</blockquote>            
	<?
	$pagelib->htmlFoot();
}


echo '<pre>'."\n";


$sqls        = 'FROM '.$tablename.' x ';
$fromClause  = $_SESSION['s_tabSearchCond'][$tablename]['f'];
$tableSCondM = $_SESSION['s_tabSearchCond'][$tablename]['w'];
$whereXtra   = $_SESSION['s_tabSearchCond'][$tablename]['x']; 

if ( $tableSCondM ) {
  $sqls .= ' WHERE '.$tableSCondM;
}
 
$sql->query('select * '.$sqls);
if ($error->got()) {
  $e = $error->getLast();
  $e->printText();
  die();
}
	 
while ($sql->ReadArray()) {

  reset ($cols);
  $sql_val ="";
  $sql_cols="";
  foreach( $cols as $dummy=>$col) {
	if ($colcheck[$col]) { // if checked for export
		$value    = $sql->RowData[$col];
		$sql_cols.= $col.', ';
		$sql_val .= ($value != '') ? $sql->addquotes($value).', ' : 'null, ';
	}
  }
  $sql_val  = substr ($sql_val , 0, -2);
  $sql_cols = substr ($sql_cols, 0, -2);

  $sql_end = 'INSERT INTO '.$tablename.' (' .$sql_cols. ') VALUES (' .$sql_val. ');' . "\n";


  echo $sql_end;
}
echo "\n\n \n";
echo '</pre>';
?>
