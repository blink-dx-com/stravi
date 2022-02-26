<?php
/**
 * preferences for single object sheet
 * $Header: trunk/src/www/pionir/glob.obj.pref.php 59 2018-11-21 09:04:09Z $
 * @package glob.obj.pref.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $tablename
  	 $id
	 $id1 TBD: not yet supported
	 $id2 TBD: not yet supported
	 [$go]
	 [$setti] : the prefernces array
 * @uses: ROBO/o.[TABLE].preferences.html (example: o.EXP.preferences.html)
 */

session_start(); 

require_once("db_access.inc");
require_once("globals.inc");
require_once("func_head.inc");
require_once("subs/f.prefsgui.inc");
require_once("object.subs.inc");
require_once("f.help.inc");
require_once ( "javascript.inc" );

class o_xspref_ABS {
    
    function pref_full_row($title, $field, $notes, $opt=NULL) {
        $this->prefNameShow( $title );
        echo "<td>".$field."</td>";
        $this->prefClose($notes);
    }
    
    function prefNameShow( $text, $align="" ) {
        echo "<tr $align><td><font color=gray>".$text.":</font></td>\n";
    }
    
    function prefClose( $info="" ) {
        echo "<td>&nbsp;<I>".$info."</I></td></tr>\n";
    }
    
    static function head1($text, $opttext=NULL) {
        echo '<tr style="background-color:#E8E8FF;"><td colspan=3 style="padding:10px;"><b>'.$text.'</b> '. $opttext.'</td></tr>'."\n";
    }
    static function head2($text) {
        echo '<tr "><td colspan=3 style="padding-top:10px;"></td></tr>'."\n";
        echo '<tr style="margin-top:20px; "><td colspan=3 style="padding:10px; background-color:#EFEFEF;"><b>'.$text.'</b></td></tr>'."\n";
    }
    
    function singlePref( &$sql, $info ) {}
    function prefSave( &$sql, $info, $setti) {}
}

function prefNameShow( $text, $align="" ) {
	echo "<tr $align><td><font color=gray>".$text.":</font></td>\n";
}  
  
function prefClose( $info="" ) {
	echo "<td>&nbsp;<I>".$info."</I></td></tr>\n";
}

function prefInfoRow($text, $style=0) {
	if (!$style) 
		echo "<tr><td colspan=3 bgcolor=#EFEFEF><B>".$text."</B></td></tr>\n";
	if ($style==1) 
		echo "<tr><td colspan=3><B>".$text."</B></td></tr>\n";
}

function prefEmptLine() {
	echo "<tr><td colspan=3 bgcolor=#FFFFFF><img src=\"0.gif\" height=5></td></tr>\n";
}

function prefSeparator() {
	echo "<tr><td colspan=3 bgcolor=#E8E8FF><img src=\"0.gif\" height=1 width=1></td></tr>\n";
}


$sql     = logon2( $_SERVER['PHP_SELF'] );

$tablename = $_REQUEST['tablename'];
$id= $_REQUEST['id'];
//$id1 = $_REQUEST['id1'];
//$id2 = $_REQUEST['id2'];
$go= $_REQUEST['go'];
$setti = $_REQUEST['setti'];

$nicename= tablename_nice2($tablename);
$title   = "Preferences for ".$nicename;
$keypref = "g.sof.opt";
$headerColor = "#E8E8FF";


$infoarr			 = array();
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
 
 // $objinfo = " <img src=\"".htmlObjIcon($tablename)."\"> ";
$infoarr["icon"]     = "images/ic24.userprefs.png";
$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

$prefsGuiLib = new fPrefsGuiC();
$prefsGuiLib->showHeadTabs( "SINGLE", $tablename );
echo "<ul>\n";
 
/* object dependent functions */

 $obj_ref_lib = NULL;
 $obj_ref_lab_lib=NULL;
 $tablename_l=strtolower($tablename);
 $classname = 'o_'.$tablename.'_XSPREF';
 
 $xtra_file="obj.".$tablename_l.".xspref.inc";
 if ( file_exists($xtra_file) ) {
    require_once($xtra_file);   		/* object oriented extra functions: singlePrefSave() */
    $obj_ref_lib = new $classname();
 }
 
 $xtra_file_lab = "../lab/".$xtra_file;
 if ( file_exists($xtra_file_lab) ) {
     require_once($xtra_file_lab);  
     $obj_ref_lab_lib = new $classname();
 }
 
 

$flagShowGlobPrefs = TRUE;
if ( $tablename=="PROJ" ) $flagShowGlobPrefs = FALSE; // do not show SingleForm options
 
$sqls = "select COLUMN_NAME from CCT_COLUMN where TABLE_NAME='$tablename' AND VISIBLE=2"; // look for advanced columns
$sql->query("$sqls");
$tmpcol=array();
while ($sql->ReadRow() ) {
	$tmpcoln = $sql->RowData[0];
	$tmpfeat = colFeaturesGet( $sql, $tablename, $tmpcoln, 0);
	$tmpcol[$tmpcoln]=$tmpfeat["NICE_NAME"];	
}


/**************************/


$settings_possible = 0;

echo "<UL>";
if ( $go ) {

	if ($flagShowGlobPrefs) {
		$tmpFormOpt = NULL;
		$prefname   = "colvSlim";
		$tmpFormOpt[$prefname] = $setti[$keypref."_".$prefname];

// 		$prefname = "colShowAdvanced";
// 		$tmp_val = $setti[$keypref."_".$prefname] ;
// 		$tmpFormOpt[$prefname] = $tmp_val;

		$prefname = "formslim";
		$tmp_val = $setti[$keypref."_".$prefname] ;
		$tmpFormOpt[$prefname] = $tmp_val;
		
		$_SESSION['userGlob']["editRemarks"]="1";
		if ( !$setti["editRemarks"] ) $_SESSION['userGlob']["editRemarks"] = "";

		$_SESSION['userGlob'][$keypref] = serialize($tmpFormOpt);
	}

	$info=NULL; // not used!
	if ( $obj_ref_lib!==NULL ) {  
	   $obj_ref_lib->prefSave( $sql, $info, $setti );
	}
    
	if ( $obj_ref_lab_lib!==NULL ) {  // from $xtra_file
	    $obj_ref_lab_lib->prefSave( $sql, $info, $setti );
	}
    echo "<font color=green>...updated</font><br>\n"; 
	
	if ( $id ) {
		$newurl = "edit.tmpl.php?t=".$tablename."&id=".$id;
		js__location_replace($newurl, "forward to object");  
		return;
	}	
}


?>
<form  method="post" action="<?echo $_SERVER['PHP_SELF']?>?go=1&id=<?php echo $id;?>" > 	
<input type=hidden name=tablename value=<?echo $tablename?> >
<?php	


echo "<table style=\"border:solid; border-color:".$headerColor."; border-spacing:0px; margin:0px; border-width:1px;\">"; // border-color:".$headerColor."; 

if ($flagShowGlobPrefs) {
    
    o_xspref_ABS::head1('Global settings');
    

	$formopt = unserialize($_SESSION['userGlob'][$keypref]);

	prefNameShow( "Slim form head" );
	$prefname = "formslim";
	$infotmp  = $formopt[$prefname];
	if (empty($infotmp))  $tmp_val  = '';
	else $tmp_val  = 'checked';
	echo "<td><input type=checkbox name=\"setti[".$keypref."_".$prefname."]\" value=\"1\" $tmp_val >";
	echo "</td>";
	prefClose ("hide menu and other navigation parts");

	prefNameShow( "Slim data row view" );
	$prefname = "colvSlim";
	$infotmp  = $formopt[$prefname];
	if (empty($infotmp))  $tmp_val  = '';
	else $tmp_val  = 'checked';
	echo "<td><input type=checkbox name=\"setti[".$keypref."_".$prefname."]\" value=\"1\" $tmp_val >";
	echo "</td>";
	prefClose ("hide empty columns for slim visualization");   

	if (count($tmpcol)) { 
		$tmpstr = "(e.g. ";
		foreach( $tmpcol as $nicename) {
			$tmpstr .= "'<B>".$nicename."</B>' "; 
		}
		$tmpstr .=  ")";
	}

// 	$prefname = "colShowAdvanced";
// 	$infotmp= $formopt[$prefname];
// 	if (empty($infotmp)) {
// 		$tmp_val  = '';
// 	} else {
// 		$tmp_val  = 'checked';
// 	}
// 	prefNameShow( "Show advanced parameters" );
// 	echo "<td><input type=checkbox name=\"setti[".$keypref."_".$prefname."]\" value=\"1\" $tmp_val >";
// 	echo "</td>";
// 	prefClose ("show advanced columns ".$tmpstr); 
	
	prefNameShow( "Show remarks" );
	$checked="";
	if  ($_SESSION['userGlob']["editRemarks"]) $checked="checked";
	echo "<td><input type=checkbox name=setti[editRemarks] value=1 $checked ></td>";		
	prefClose ("Show remarks of columns");

	prefEmptLine();
	$helpLib = new fHelpC();
	$helplink = $helpLib->getTableEntryHelp($tablename, 'preferences');
	if ($helplink!=NULL) $helpOuttext = " [<a href=\"".$helplink."\" target=_help>help</a>]";   
	else $helpOuttext=NULL;
	
	o_xspref_ABS::head1('Settings for this object type',$helpOuttext);
	
	
}

if ( $obj_ref_lib!==NULL ) {   // from $xtra_file
    $obj_ref_lib->singlePref( $sql, $info );
	$settings_possible = 1;
}
if (!$settings_possible) echo "<tr><td colspan=3><font color=gray>No special settings for this object type possible.</font></td></tr>\n";

if ( $obj_ref_lab_lib!==NULL ) {   // from $xtra_file_lab 
	prefEmptLine();
	o_xspref_ABS::head1('Lab specific settings');
   
    $obj_ref_lab_lib->singlePref( $sql, $info );
}

prefEmptLine();
// <img src=\"0.gif\" width=1 height=23>
echo "<tr bgcolor=".$headerColor."><td></td><td valign=middle>".
    "<input type=\"submit\" value=\"Submit preferences\" class='yButton'></td><td></td></tr>\n";
echo "</table>\n";
	
echo '</form>'."\n";	
echo '</UL>';

htmlFoot();
