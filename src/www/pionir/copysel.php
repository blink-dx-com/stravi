<?php
/**
 * copy selected objects to clipboard
 * @package copysel.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $tablename
 */
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once('sql_query_dyn.inc');
require_once('javascript.inc');

define('MAX_CLIP_ELEMS', 10000); // maximum elements in clipboard

$tablename=$_REQUEST['tablename'];

$pagelib = new gHtmlHead();
$pagelib->PageHeadLight ('Clipboard: copy elements');

echo '<blockquote><h3>Clipboard: copy elements</h3>';

if (empty($tablename)) htmlFoot('ALERT', 'No tablename found!</blockquote>');

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
if ($error->PrintLast()) htmlFoot('</blockquote>');

$primas         = primary_keys_get2($tablename);
$primas_str     = implode (', ', $primas);
$fromClause     = $_SESSION['s_tabSearchCond'][$tablename]['f'];
$tableSCondM    = $_SESSION['s_tabSearchCond'][$tablename]['w'];
$whereXtra      = $_SESSION['s_tabSearchCond'][$tablename]['x']; 
$sqlAfter       = full_query_get($tablename, $fromClause, $tableSCondM, $whereXtra);
$_SESSION['s_clipboard']    = array();
$i              = 0;

$sql->query('SELECT '.$primas_str.' FROM '.$sqlAfter);
if ($error->printLast()) htmlFoot('</blockquote>');

while ($sql->ReadRow() && $i < MAX_CLIP_ELEMS ) {
  $ids = array('tab' => $tablename, 'ida' => $sql->RowData[0]);
  if (isset($primas[1]))
	$ids['idb'] = $sql->RowData[1];
  else
	$ids['idb'] = '';
  if (isset($primas[2]))
	$ids['idc'] = $sql->RowData[2];
  else
	$ids['idc'] = '';

  $_SESSION['s_clipboard'][] = $ids;
  $i++;
}

if ($i >= MAX_CLIP_ELEMS) {
  info_out('WARNING', 'Copied only '.$i.' elements due to clipboard restrictions.');
  echo '[<a href="view.tmpl.php?&t=',$tablename,'">Go back</a>]';
} else {
   js__location_replace('view.tmpl.php?t='.$tablename, 'back');
}

htmlFoot('</blockquote>');
