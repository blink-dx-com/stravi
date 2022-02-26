<?php
/**
 * show info of selected tables
 * -a analyse $_s_i_table
 * @package obj.cct_table.info.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   none
 * @version0 2002-09-04
 */ 
session_start(); 

require_once ('reqnormal.inc');
require_once('javascript.inc');

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();

$title = 'Info for CCT_TABLE';
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = "CCT_TABLE";

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);



echo "<blockquote>";

  echo '<P>';
  echo '<li><a href="glob.cache.refresh.php?back='.$_SERVER['PHP_SELF'].js__get_param_to_url().'&amp;auto_back=1">Refresh global table data cache</a></LI>'; 
  
  $print_headline = 1;
  
  foreach( $_s_i_table as $table=>$dummy) {
	if (tablename_nice2($table) === $table) {
	  if ($print_headline) {
		echo '<li><b>Tables which are not yet defined in cct_table:</b> (or not yet in cache)</LI><ul>';
		$print_headline = 0;
	  }
	  echo '<li>',$table,'</li>';
	}
  }
  if (!$print_headline)
	echo '</ul>';    
    
echo "</blockquote>";

htmlFoot();
