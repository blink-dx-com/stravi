<?
/**
    tests if the session's cache-variables are still in memory
    - refresh cache
  * @package test_caches.php
  * @author  Adrian
    @param $back
    @param auto_back
    @param $print_r
   @version $Header: trunk/src/www/pionir/rootsubs/test_caches.php 59 2018-11-21 09:04:09Z $
 */
extract($_REQUEST); 
session_start(); 


require_once('globals.inc');
require_once('utilities.inc');
require_once('func_head.inc');
require_once('varcols.inc');
require_once('javascript.inc');

if (!isset($print_r)) {
  $back_url = 'rootFuncs.php';
  $back_txt = 'Administration';
} else {
  $back_url = $_SERVER['PHP_SELF'];
  $back_txt = '???';
}
$sqloDummy = NULL; // needed ???
		
$title = 'Test class- &amp; table-data cache';
$infoarr			 = NULL;
$infoarr["scriptID"] = "";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "tool";
$infoarr['help_url'] = 'g.cache-refresh.html';
$infoarr["locrow"] = array( array($back_url, $back_txt) );

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqloDummy, $infoarr);

global $_s_i_misc;
global $_s_i_app_data_type;
global $_s_i_table;

echo '<blockquote>';

get_cache();

if (!isset($print_r)) { // nomal mode
  echo '<tt><a href="'.$_SERVER['PHP_SELF'].'?print_r=_s_i_varcol">$_s_i_varcol</a></tt>: ';
  if (!isset($_s_i_varcol)) {
    echo '<font color="#ff0000">not existing!</font>';
  } else {
    echo 'existing; ';
	if (!function_exists('shm_attach')) { // we have the data sored in session
	  if ( isset($_SESSION['_s_i_varcol']) )
		echo 'existing in session; ';
	  else 
		echo '<font color="#ff0000">not found in session!</font> ';
	}
	if (count($_s_i_varcol))
	  echo 'contains '.count($_s_i_varcol).' elements';
	else
	  echo '<font color="#ff0000">contains '.count($_s_i_varcol).' elements</font>';
  }
  
  echo '<br> <tt><a href="'.$_SERVER['PHP_SELF'].'?print_r=_s_i_table">$_s_i_table</a></tt>: ';
  if (!isset($_s_i_table))
    echo '<font color="#ff0000">not declared</font>';
  elseif (!is_array($_s_i_table))
    echo 'no array';
  else {
    echo 'existing; ';
	if (!function_exists('shm_attach')) { // we have the data sored in session
	  if ( isset($_SESSION['_s_i_table']) )
		echo 'existing in session; ';
	  else 
		echo '<font color="#ff0000">not in session!</font> ';
	}
    echo count($_s_i_table). ' elements';
  }


  echo '<br> <tt><a href="'.$_SERVER['PHP_SELF'].'?print_r=_s_i_app_data_type">$_s_i_app_data_type</a></tt>: ';
  if (!isset($_s_i_app_data_type))
    echo '<font color="#ff0000">not declared</font>';
  elseif (!is_array($_s_i_app_data_type))
    echo 'no array';
  else {
    echo 'existing; ';
	if (!function_exists('shm_attach')) { // we have the data sored in session
		
	  if ( isset($_SESSION['_s_i_app_data_type']) )
		echo 'existing in session; ';
	  else 
		echo '<font color="#ff0000">not in session!</font> ';
	}
    echo count($_s_i_app_data_type). ' elements';
  }
  
  echo '<br> <tt><a href="'.$_SERVER['PHP_SELF'].'?print_r=_s_i_misc">$_s_i_misc</a></tt>: ';
  if (!isset($_s_i_misc))
      echo '<font color="#ff0000">not declared</font>';
      elseif (!is_array($_s_i_misc))
      echo 'no array';
      else {
          echo 'existing; ';
          if (!function_exists('shm_attach')) { // we have the data sored in session
              
              if ( isset($_SESSION['_s_i_misc']) )
                  echo 'existing in session; ';
                  else
                      echo '<font color="#ff0000">not in session!</font> ';
          }
          echo count($_s_i_misc). ' elements';
      }

  echo '<br><br>[<b><a href="../glob.cache.refresh.php?back='.$_SERVER['PHP_SELF'].js__get_param_to_url().'&amp;auto_back=0">Refresh global class and table-data cache</a></b>]';
//   echo "<br><br><a href=$_SERVER['PHP_SELF']?rehash=1>Reinitialize Session-Variables</a>";

} else { // print variable
  switch ($print_r) {
  case '_s_i_varcol':
  case '_s_i_table':
  case '_s_i_app_data_type': glob_printr($$print_r,'test_caches'); break;
  default: info_out('ALERT', $print_r.' is not allowed for display.');
  }
}

  echo "<br><br> <hr>";
  echo '[<a href="db_transform/o.CCT_TABLE.subs.php">Generate automatic entries in CCT_TABLE, CCT_COLUMN</a>]';
 
htmlfoot('</blockquote>');
?>
