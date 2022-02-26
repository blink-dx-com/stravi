<?php
/**
 * show/delete ALL preferences for object type
 * @package glob.obj.pref2.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $tablename
  	 $id
	 [$go]
	 $clearPrefs
 */

session_start(); 

require_once("db_access.inc");
require_once("globals.inc");
require_once("func_head.inc");
require_once("subs/f.prefsgui.inc");
require_once("object.subs.inc");
 
function this_clearpref( $tablename ){ 

    $cntis=0;
    $tabpat = "o.". $tablename. ".";
    $userGlobTmp = $_SESSION['userGlob'];
    
    foreach( $userGlobTmp as $key=>$val) {     
  
        if ( strpos($key, $tabpat) !== FALSE ) {
            unset ($_SESSION['userGlob'][$key]);
            $cntis++; 
        } 
    }
    echo "<br><font color=gray>Cleared preferences:</font> $cntis<br>";
  
}

$tablename=$_REQUEST['tablename'];
$id=$_REQUEST['id'];
$clearPrefs=$_REQUEST['clearPrefs'];
$go=$_REQUEST['go'];

$sql     = logon2( $_SERVER['PHP_SELF'] );
$title   = "Object preferences: advanced view";
$keypref = "g.sof.opt";

$infoarr			 = NULL;
$infoarr["scriptID"] = "";
$infoarr["title"]    = $title;
$infoarr["title_sh"] = "Single object preferences";
$infoarr["obj_name"] = $tablename;
$infoarr["help_url"] = "preferences.html"; 
$infoarr["design"]   = "slim"; 

if ( !empty($id) ) {
	$infoarr["form_type"]= "obj";
	$infoarr["obj_id"]   = $id;
	$infoarr["show_name"]= 0; 
} else {
	$infoarr["form_type"]= "list";
}
$infoarr["icon"]     = "images/ic24.userprefs.png";

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

if ( $tablename == "" ) {
    htmlFoot("Error", "Need an object type");
}

$prefsGuiLib = new fPrefsGuiC();		
$prefsGuiLib->showHeadTabs( "OBJ", $tablename  );


//$settings_possible = 0;
$nice_name =  tablename_nice2($tablename);

echo "<UL>";

if ($clearPrefs ) {
  if ($go <= 0) {
        echo '<B>Are you sure,<br>you want to delete the preferences for object type "'.$nice_name.'"?</B><br>&nbsp;<br>';
	    echo '[<a href="'.$_SERVER['PHP_SELF'].'?tablename='.$tablename.'&clearPrefs=1&go=1">YES</A>] | ';
	    echo '<a href="' .$_SERVER['PHP_SELF'].'?tablename='.$tablename.'">no</A>';
        htmlFoot('</blockquote>');
  } else {
        this_clearpref( $tablename );
  }
    
}


$tabpat = "o.". $tablename. ".";

echo "Preferences only for this object type:";   
echo "<br>";
$cntis = "0";

if (sizeof($_SESSION['userGlob'])) {
  echo "<table cellpadding=1 cellspacing=1 border=0 bgcolor=#EFEFEF>";
  foreach( $_SESSION['userGlob'] as $key=>$val) {     
  
    $put_out = 0;
	   
    if ( strpos($key, $tabpat) !== FALSE ) {
        $put_out = 1; 
    } 

    if ( $put_out ) {
        echo "<tr><td>".$key."</td><td>".$val."</td></tr>\n"; 
        $cntis++;
    }
  }
  echo "</table>";
  reset($_SESSION['userGlob']);
   
  echo "<font color=gray>Number of preferences:</font> ".$cntis;
  if ($cntis) echo " &nbsp; [<a href=\"".$_SERVER['PHP_SELF']."?tablename=".$tablename."&clearPrefs=1\">delete them</a>]";
  echo "<br>";
  
} 
?>
<br>
<hr size=1 noshade>
<B>Info about the object type</B><br><br>
<?
echo '<font color=gray>Codename of table:</font> ',$tablename,'<br>';
echo '<font color=gray>Notes of the object type:</font> ',table_remark2($tablename), '<br>';

echo "</UL>";
$pagelib->htmlFoot();
