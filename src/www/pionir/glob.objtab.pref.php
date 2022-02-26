<?php
/**
 * preferences for OBJECT list view
 * - can be extended by file 'obj.'.$tablename_l.'.xfunc.inc'
 * global vars:
 *   $_SESSION['userGlob']
 *      ["g.viewf.opt"]
 *      ['o.'.{TABLE}.'.sortcrit']
 *      ['o.'.$tablename.'.condShow']
 *      ['g.listHideMenu']
 *   $_SESSION['userGlob']['o.{TABLE}.viewcols']  -- prefs for shown columns
 *       serialized  tab_cols_pref_STRUCT
 * 
 * 
 * @package glob.objtab.pref.php
 * @swreq UREQ:0002512: g > list view preferences 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param    
 *    $tablename
	  $go	0|1
	  $viewcol     (if form was submitted )
      $reset_col 0|1  - reset the column prefs
      $setti : parameter array
	  [$colopt]
	  

 */

session_start(); 


require_once ('reqnormal.inc');
require_once ("subs/f.prefsgui.inc");
require_once ("gui/glob.objtab.pref.inc");


/****************************************************************************/
global $error;
$error = & ErrorHandler::get();


$tablename = $_REQUEST['tablename'];
$go        = $_REQUEST['go'];
$colopt    = $_REQUEST['colopt'];


$sql   = logon2( $_SERVER['PHP_SELF'] );
$title2 = 'List view preferences ';
$objinfo = "";
if ($tablename != "") {
	$objinfo = " for ".tablename_nice2($tablename);
	// $objinfo2 = " for <img src=\"".htmlObjIcon($tablename)."\"> ".tablename_nice2($tablename);
}
$title  =  $title2.$objinfo;
$infoarr			 = NULL;
$infoarr["scriptID"] = "";
$infoarr["title"]    = $title;
$infoarr["title_sh"] = $title2;
$infoarr["form_type"]= "tool";
$infoarr['help_url'] = 'g.listViewPrefs.html';
$infoarr["icon"]     = "images/ic24.userprefs.png";
$infoarr["design"]   = "slim"; 

			
if ($tablename != "") {  
    $infoarr["form_type"]= "list";
	$infoarr["obj_name"] = $tablename;
} else {
	$infoarr["locrow"]   = array(
			array("obj.db_user.settings.php", "Main preferences"),
			);
}

$infoarr["css"] = "th.x1 { color:gray; }"; // color:#808080; background-color:#CCDDFF

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

if (empty($go)) $go = 0;
 
if ( $tablename=="" ) {
    htmlFoot("Error", "table name missing [<a href=\"obj.db_user.settings.php\">Back main preferences</a>]");
}

$prefsGuiLib = new fPrefsGuiC();
$prefsGuiLib->showHeadTabs( 'LIST', $tablename );	
echo "<ul>\n";

$mainlib = new glob_objtab_pref($tablename, $colopt);
$mainlib->default_settings();

;

if ( $go>0 ) {
    $mainlib->save_as_default($sql);
    // reinit
    $mainlib = new glob_objtab_pref($tablename, $colopt);
    $mainlib->default_settings();
}
if ($_REQUEST['reset_col']) {
    $mainlib->col_prefs_reset();
    $mainlib->column_saveAsSTD($sql);
}

// echo "<br>";
$mainlib->show($sql);



// <hr size=1 noshade><br>
htmlInfoBox( "Information about the table", "", "open", "CALM" );
echo '<font color=gray>Codename of table:</font> ',$tablename,'&nbsp;&nbsp;&nbsp;&nbsp;',
     '[<a href="edit.tmpl.php?t=CCT_TABLE&id=',$tablename,'">column definitions</a>]';
echo ' [<a href="view.tmpl.php?t=EXTRA_CLASS&searchCol=TABLE_NAME&searchtxt='.$tablename.'&condclean=1">classes</a>]<br>'."\n";
echo '<font color=gray>Notes:</font> ',table_remark2($tablename), '<br>';     
echo "<font color=gray>Current search condition:</font> ";      
$tmpinfo = $_SESSION['s_tabSearchCond'][$tablename]["info"];
if ($tmpinfo!="") echo htmlspecialchars($tmpinfo);    
else echo "<font color=gray>nothing</font>";
echo "<br>";
htmlInfoBox( "", "", "close", "" );  

$mainlib->JS_out();

htmlFoot('</ul>');
