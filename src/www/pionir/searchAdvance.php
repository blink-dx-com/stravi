<?php
/**
 * advanced search GUI
 * @package searchAdvance.php
 * @swreq SREQ:0002447: g > object list > advanced search 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   string $tablename 
 */

//extract($_REQUEST); 
session_start(); 


require_once("db_access.inc");
require_once("globals.inc");
require_once("sql_query_dyn.inc");
require_once("func_head.inc");
require_once("view.tmpl.inc");


class searchAdvC {

function __construct($tablename) {
	$this->tablename = $tablename;
	$this->numPKs = countPrimaryKeys($tablename);
	
}

function _showtab(&$toolArr) {
	echo "<table border=0 cellspacing=3 cellpadding=3><tr valign=top bgcolor=#EFEFFF align=center>";
	
	foreach( $toolArr as $val) {
		$xcolorSpecial="";
		if ($val["color"]!="") $xcolorSpecial="bgcolor=".$val["color"];
		
		echo "<td ".$xcolorSpecial."><img src=\"images/".$val["ic"]."\" border=1 height=40 TITLE=\"".$val["ictitle"]."\">".
		"<br><a href=\"".$val["href"]."\">".$val["txt"]."</a></td>\n";
	
	}
	
	
	echo "</tr>\n</table>\n";
}

function toolsList() {
	global $_s_i_table;
	
	$tablename = $this->tablename;
	//$xcolorSpecial = "#EFEFC0";
	$access_id_has = cct_access_has2($tablename); 
	$cctpre = substr($tablename,0,4);
	$isCCT_table = 0;
	if ( $cctpre=="CCT_" ) $isCCT_table = 1;
	
	$toolArr=array();
	$toolArr[] = array("ic"=>"ic.myqueryLogo.40.png", "href"=>"obj.link.c_query_mylist.php", "txt"=>"My Search<br>Center");
	$toolArr[] = array("ic"=>"search.usage.gif", "href"=>"glob.objtab.trackgui.php?tablename=".$tablename, "txt"=>"Object<br>tracking");
	
	if ($access_id_has) $toolArr[] = array("ic"=>"ic.quicks.gif", "href"=>"glob.obj.qsearch.php?tablename=".$tablename, "txt"=>"Quick<br>search");
	$toolArr[] = array("ic"=>"i40.prosearch.gif", "href"=>"glob.objtab.search_1.php?t=".$tablename, "txt"=>"Pro<br>search");
	$toolArr[] = array("ic"=>"i40.prosearch.gif", "href"=>"p.php?mod=DEF/g.objtab.sea2&t=".$tablename, "txt"=>"Pro<br>search II");
	
	
	
	// Information about the classes
	if (array_key_exists("EXTRA_OBJ_ID", $_s_i_table[$tablename])) {
		$toolArr[] = array("ic"=>"ic.classinfo.gif", "href"=>"glob.extra_obj_info.php?tablename=$tablename&extra_obj_exist=1", "txt"=>"Class<br>info");
	}
	
	$toolArr[] = array("ic"=>"search.impsea.gif", "href"=>"glob.objtab.impsea.php?tablename=".$tablename, "txt"=>"Existence<br>check");
	$toolArr[] = array("ic"=>"ic47.equal.gif",    "href"=>"glob.objtab.cmp.php?tablename=".$tablename, "txt"=>"Basic<br>compare");
	$toolArr[] = array("ic"=>"search.attrib.gif", "href"=>"glob.objtab.coluniq.php?tablename=".$tablename, "txt"=>"Unique<br>columns");
	if ($access_id_has OR $isCCT_table) $toolArr[] = array("ic"=>"i39.timedia.gif",   "href"=>"glob.objtab.timedia.php?tablename=".$tablename, "txt"=>"Object<br>TimeGraph");
	if ($access_id_has ) $toolArr[] = array("ic"=>"i39.timedia.gif",   "href"=>"p.php?mod=DEF/g.objtab.auditlog&t=".$tablename, "txt"=>"Audit log<br>list");
	
	if ($this->numPKs==1)  $toolArr[] = array("ic"=>"i40.vario.gif", "href"=>"p.php?mod=DEF/g.objtab.s_vario.sh&t=".$tablename, "txt"=>"Vario<br />columns");
	
	$this->_showtab($toolArr);
	
	if ($access_id_has ) {
		// attachments
		$toolArr = array();
		$toolArr[] = array("ic"=>"i40.attach_ana.gif",   "href"=>"p.php?mod=DEF/g.objtab.attach_v&t=".$tablename, "txt"=>"Attachment<br />search");
		$toolArr[] = array("ic"=>"i40.attach_ana.gif",   "href"=>"glob.objtab.sattach_exp.php?tablename=".$tablename, "txt"=>"Attachment<br />export");
		$this->_showtab($toolArr);
	}
}

function toolsExtra() {
	$tablename = $this->tablename;
	$toolArrX = NULL;
	switch ($tablename) {
		
		case "EXP": 
			// $toolArrX[] = array("ic"=>"ic.exp_searchsubst.gif", "href"=>"obj.exp.search.substs.php", "txt"=>"Search substance in<br>experiment results");
			$toolArrX[] = array("ic"=>"i40.seaCross.gif", "href"=>"glob.objtab.seaCross.php?t=EXP", "txt"=>"cross search<br>selected samples");
			
			break;
		case "LINK":
			$toolArrX[] = array("ic"=>"icon.LINK.gif", "ictitle"=>"search content inside documents", "href"=>"obj.link.searchDoc.php", "txt"=>"Search<br>content");
			break;
		default: 
	}
	
	if ($toolArrX==NULL) return;
	
	echo "<br><br><b>Object special searches, minings</b><br>";
	$this->_showtab($toolArrX);
}

}



$sql = logon2( $_SERVER['PHP_SELF'] );
$tablename=$_REQUEST['tablename'];
//$addusercond_allow=1;


$title= "Advanced search, data mining";
$infoarr=array();
$infoarr["help_url"] = "Search_advanced.html"; 

$infoarr["title"] = $title;
$infoarr["title_sh"] = "Advanced search";
$infoarr["form_type"]= "list";
$infoarr["icon"]     = "images/ic20.searadv.gif";
if ($tablename!="") {
	$infoarr["obj_name"] = $tablename;
	$infoarr["obj_cnt"]  = 1; 
}

$mainObj  = new searchAdvC($tablename);

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

if ( $tablename == "" ) {
  // echo "ERROR: No tablename given!<br>";
  echo "<ul>";
  require_once("f.visu_list.inc");
  visu_listC::selTable($sql, $_SERVER['PHP_SELF']); 
  htmlFoot();
}

$tablename_nice = tablename_nice2($tablename);
$title_short = "Adv search: ".$tablename_nice;
gHtmlMisc::func_hist("searchAdvanced.".$tablename, $title_short, $_SERVER['PHP_SELF']."?tablename=".$tablename );

echo "<UL>";

$mainObj->toolsList();
$mainObj->toolsExtra();

//------------------------------------------------------------------------------

echo "<br>";

//TBD: put to module as function
if ($_SESSION['s_tabSearchCond'][$tablename]["info"]!="") {
	echo "<br>";
	htmlInfoBox( "Current filter condition", "", "open", "CALM" );
	$tmpstr = $_SESSION['s_tabSearchCond'][$tablename]["info"];
	if (strlen($tmpstr)>80)  $tmpstr = substr($tmpstr,0,80)." ...";
	$tmpstr = htmlspecialchars($tmpstr);
	echo $tmpstr;
	htmlInfoBox( "", "", "close" );
	echo "<br>";
}
                

echo "<br>";
echo "<font color=gray>More functions: </font> ";
echo "[<a href=\"glob.objtab.info.php?tablename=".$tablename."\">Detailed table info</a>] \n";
echo "[<a href=\"glob.objtab.idAna.php?tablename=".$tablename."\">Analyse ID continuity</a>] \n";
echo "[<a href=\"glob.objtab.trackproj.php?tablename=".$tablename."\">Project External Tracking</a>] \n";


echo "<br>\n";
echo "<hr size=1>";

htmlFoot();
