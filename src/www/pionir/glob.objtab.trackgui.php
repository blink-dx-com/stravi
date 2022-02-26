<?php
/**
 * GUI to select tablename where to track selection
 * following scripts:
 *   - glob.objtab.search_usage.php
  	 - view.tmpl.php
   - since: 20020211
 * @package glob.objtab.trackgui.php
 * @swreq   UREQ:0001242: g > object tracking > list of objects
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   
 * 		 $tablename
  		 $advanced 0|1
		 $actionx = "", 
		 		 = "forwProjBo" - forward a project -> BO selection
		 		 = 'forwNorm'   - forward to view.tmpl.php (prepare parameters)
	OPTIONAL:
		 $parx[tableBO]  - destination table for $action = "forwProjBo"
		   - for $actionx = 'forwNorm' 
		 $parx[destTab]  - destination table
		 $parx[col2]     - track for column col2 in destTab
		 $parx[destVia]  - track destVia => destTab
		 $parx[foreignOpt] - 0,1 : show selection for columns, where the $tablename is
		   linked to more than one COLUMN in a destination TABLE $parx[destTab]
		 $parx['special'] : 0,1 very special forward
		
 */
session_start(); 


require_once ('reqnormal.inc');
require_once("func_form.inc");
require_once("subs/glob.objtab.track.inc");
require_once("sql_query_dyn.inc");
require_once ( "javascript.inc" );

class gObjTabTrackGui {

function __construct($tablename, $parx) {
	
	
	$this->tablename = $tablename;
	$this->parx = $parx;
	$fromClause  = $_SESSION['s_tabSearchCond'][$tablename]["f"];
	$tableSCondM = $_SESSION['s_tabSearchCond'][$tablename]["w"];
	$whereXtra   = $_SESSION['s_tabSearchCond'][$tablename]["x"]; 
	$sel_info    = $_SESSION['s_tabSearchCond'][$tablename]["info"];
	
	$this->tabPkName   = PrimNameGet2($tablename);
	$this->selcond_ori = full_query_get( $tablename, $fromClause, $tableSCondM, $whereXtra );
	$this->gTrackUsedLib = new gObjTrackSubGui($tablename, $parx);
}

function forwardNorm( &$sqlo ) {
	$this->gTrackUsedLib->forwardDo($sqlo, $this->parx);
}

function _showMethHead($text, $icon) {
	// ... are used by other objects
	echo "<br>\n";
	echo "<table border=0 cellpadding=4 cellspacing=0 bgcolor=#EFEFEF width=600>";
	echo '<tr><td valign=top  width=50><img src="images/'.$icon.'" TITLE="'.$text.'"> ';
	echo "</td><td valign=top>";
	echo '<B>'.$text.'</B><br><img height=5 width=1><br>';
	echo "</td></tr></table>\n";
}

function _show_element($icon, $text, $url, $notes=NULL) {
    echo "<a href=\"".$url."\">";
    echo "<img src=\"images/$icon\" border=0> ";
    echo $text. "</a> ". $notes;
}

function projSubElems(&$sql) {
	
	
	$desturl = "";
	$boTables = NULL;
	$sqls = "select TABLE_NAME, NICE_NAME from CCT_TABLE where TABLE_TYPE='BO' order by NICE_NAME";
	$sql->query($sqls);
	while ( $sql->ReadRow() ) {
		$boTables[$sql->RowData[0]] = $sql->RowData[1];
	}
	$preselected = NULL;
	$seltext = formc::selectFget( 
		"parx[tableBO]", 	  // FORM-variable-name
		$boTables, 		  // array ( ID => "nice name")
		$preselected 
		); 
	echo "\n";
	echo "<form style=\"display:inline;\" method=\"post\"  name=\"eroes\"  action=\"".$_SERVER['PHP_SELF']."\" >\n";
	echo "<img src=\"images/icon.PROJ_HAS_ELEM.gif\" border=0> Contained objects:\n";
	echo $seltext;
	
	echo "<input type=hidden name=\"actionx\" value=\"forwProjBo\">\n";
	echo "<input type=hidden name=\"tablename\"  value=\"PROJ\">\n";
	echo "<input type=hidden name=\"go\"  value=\"1\">\n";
	echo "<input type=submit value=\"Submit\">\n";
	
	echo "</form>\n";
	
}

function forwardProjBO(&$sql) {
	// FUNCTION: forward a project -> BO selection
	
	
	echo "... forward query to list-view-page ... <br>\n";
	
	$tableBO = $this->parx["tableBO"];
	if ( $tableBO == "" ) {
		htmlFoot("Error", "Please select a destination table!");
	}
	
	$main_col = PrimNameGet2($tableBO);
	$selcond_tmp =
	    "x.".$main_col.
		" IN ( select PRIM_KEY from PROJ_HAS_ELEM where " .
		"PROJ_ID IN (select x.PROJ_ID from ".$this->selcond_ori." )" .
		" AND TABLE_NAME='".$tableBO."'".
		" ) ";

		
	//	$selcond_tmp = "x.".$main_col." IN (select x.$col_name from ".$tableBO." x where".
	//	"x.".$this->tabPkName." IN (select x.PRIM_KEY from ".$this->selcond_ori." )"

	$forwardURL = "view.tmpl.php?condclean=1&t=".$tableBO."&tableSCond=". urlencode( $selcond_tmp );
	js__location_replace( $forwardURL, "list view" );
	exit; 	 
}

/**
 * extra form for tracking options
 */
function form1() {
	
	$tablename = $this->tablename;
	# [<a href='glob.objtab.search_usage.php?tablename=$tablename&areused=yes'>only used once</a>] 
    #  [<a href='glob.objtab.search_usage.php?tablename=$tablename&areused=no'>only unsed</a>]";
	
	
	$initarr   = NULL;
	$initarr["action"]      = "glob.objtab.search_usage.php";
	$initarr["title"]       = "Select advanced tracking options";
	$initarr["submittitle"] = "Submit";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["tablename"]     = $tablename;

	$formobj = new formc($initarr, $hiddenarr, 0);

	$tmparr = array("yes" => "used", "no" => "unused");
	$fieldx = array ( "title" => "Search objects which are ...", "name"  => "areused",
			"object" => "select",
			"val"   => "yes", "inits" => $tmparr,
			"notes" => "only used/unused" );
	$formobj->fieldOut( $fieldx );
	
	$tmparr = array("yes" => "yes", "no" => "no");
	$fieldx = array ( "title" => "Is object linked in projects? <img src=\"images/icon.PROJ.gif\">", "name"  => "withproj",
			"object" => "select",
			"val"   => "yes", "inits" => $tmparr,
			"notes" => "reflect links in projects?" );
	$formobj->fieldOut( $fieldx );
	
	
	$fieldx = array ( "title" => "Show project path? <img src=\"images/icon.PROJ.gif\">", "name"  => "shProjPath",
			"object" => "checkbox",
			"val"   => "0", 
			"inits" => 1,
			"notes" => "show path, if one project linked?" );
	$formobj->fieldOut( $fieldx );
	
	
	$fieldx = array ( "title" => "Max use count", "name"  => "maxuse",
			"object" => "text", "fsize" => 3,
			"notes" => "If given, maximum number of usages (e.g. 10)" );
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );
}

function s_used_by( &$sql ) {
	$tablename = $this->tablename;
	$this->gTrackUsedLib->track_gui( $sql );
	
}

function s_subObjects(&$sql) {
	// show sub-objects
	global $error;
	
	$FUNCNAME= "s_subObjects";
	
	$tablename = $this->tablename;
	echo "<table border=0 cellpadding=4 cellspacing=0 bgcolor=#EFEFEF width=600>";
	echo '<tr><td valign=top width=50><img src="images/search.sub_elem.gif" TITLE="used sub elements">';
	echo "</td><td valign=top align=left>";
	echo '<B>... use sub elements</B>';
	echo "</td></tr></table>\n";
	echo "<blockquote>";
	
	$tmparr=array();
	$tab2=array();
		
	$sqls = "select x.cct_table_name, y.nice_name, x.COLUMN_NAME, x.NICE_NAME ".
		" from cct_column x, cct_table y ".
		" WHERE x.TABLE_NAME='".$tablename."' AND ".
		"   x.cct_table_name>' ' AND x.cct_table_name=y.table_name order by x.POS"  ;
		
	$sql->query( $sqls, $FUNCNAME );
	$optioncnt=0;
	while ( $sql->ReadRow() ) {
	
		$tabName_tmp     = $sql->RowData[0];
		$tabNameNice_tmp = $sql->RowData[1];
		$col_name        = $sql->RowData[2];      
		$nice_col        = $sql->RowData[3];  
		$tmparr[]        = array( $tabName_tmp, $tabNameNice_tmp, $col_name, $nice_col);
		$optioncnt++;  
	
	}  
	
	if (sizeof($tmparr)) {

		// SORT objects by name and by put H_tables back
		//  
		$hcount = 100;
		$icount = 0;
		foreach( $tmparr as $idx=>$rowarr) {
		
			list( $tabName_tmp, $tabNameNice_tmp, $col_name, $nice_col ) =  $rowarr;
			if ( strpos($tabName_tmp, "H_") !== FALSE ) {
				$tab2[$hcount] = $rowarr;  
				$hcount++;
			} else  {
				$tab2[$icount] = $rowarr; 
				$icount++;
			}
		}
		ksort ($tab2); // sort by key, H_tables to the bottom
	
		$tableCache = array(); // flag, that table appeared again
		
		foreach( $tab2 as $idx=>$rowarr) {
	
			$tmpTabAllow = TRUE;
			list( $tabName_tmp, $tabNameNice_tmp, $col_name, $nice_col ) =  $rowarr;
	
			if ( ($tabName_tmp == "CCT_ACCESS") OR ($tabName_tmp == "EXTRA_OBJ") ) {
				$tmpTabAllow = FALSE;
			}
			if ( $_SESSION['sec']['appuser']=="root" AND $tabName_tmp == "CCT_ACCESS" ) $tmpTabAllow = TRUE;
			
			if ( $tmpTabAllow ) {
	
				$main_col = $tabName_tmp."_ID";
				$selcond_tmp = "x.".$main_col." IN (select x.$col_name from ".$this->selcond_ori." )";
	
				$icon="images/icon.".$tabName_tmp.".gif";
				if ( !file_exists($icon) ) $icon="images/icon.UNKNOWN.gif";
	
				echo "<a href=\"view.tmpl.php?condclean=1&t=".$tabName_tmp."&tableSCond=". urlencode( $selcond_tmp )."\">";
				echo "<img src=\"$icon\" border=0> ";
				echo $tabNameNice_tmp ."</a>";
				if ( $tableCache[$tabName_tmp]>0) {
					echo " (".$nice_col.")";
				}
				echo "<br>\n";
				
				$tableCache[$tabName_tmp]=1;
			}
			$optioncnt++;
		
		}
	}
	
	if ( $tablename == "PROJ" ) {
		$this->projSubElems($sql);
	}
	
	$tabisbo 	 = cct_access_has2($tablename);
	if ($tabisbo) {
		echo '<a href="p.php?mod=DEF/g.objtrack.S_OBJECT&t='.$tablename.'&type=mo"><img src="images/icon.UNKNOWN.gif" border=0> object link</a>';
	}
	
	echo "</blockquote>";
}

function projTrackLink() {
	// track in projects
			
	$tablename = $this->tablename;
	$tabName_tmp     = "PROJ";
	$tabNameNice_tmp = "project";
	
	$sql_getObjInProj = "PRIM_KEY IN (select ".$this->tabPkName." from ".$this->selcond_ori.
		" ) AND TABLE_NAME='".$tablename."'";
	$selcond_tmp = "PROJ_ID in (select PROJ_ID from PROJ_HAS_ELEM where ".$sql_getObjInProj.")";

	$icon="images/icon.PROJ.gif";
	echo "<br>";
	echo "<a href=\"view.tmpl.php?condclean=1&t=".$tabName_tmp."&tableSCond=". urlencode( $selcond_tmp )."\">";
	echo "<img src=\"$icon\" border=0> ";
	echo $tabNameNice_tmp ."</a>";
	
	// link to PROJ_HAS_ELEM
	$tabName_tmp     = "PROJ_HAS_ELEM";
	$tabNameNice_tmp = "project links";
	
	
	$icon="images/icon.PROJ_HAS_ELEM.gif";
	echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
	echo "<a href=\"view.tmpl.php?condclean=1&t=".$tabName_tmp."&tableSCond=". urlencode( $sql_getObjInProj )."\">";
	echo "<img src=\"$icon\" border=0> ";
	echo "<font color=gray>".$tabNameNice_tmp ."</font></a>";
	
	echo "<br>\n";
}

function sObjLinkChi() {
	$tablename = $this->tablename;
	echo '<a href="p.php?mod=DEF/g.objtrack.S_OBJECT&t='.$tablename.'&type=chi"><img src="images/icon.UNKNOWN.gif" border=0> object link</a>';
}

/**
 * SUBSTANCE specials
 */
function _SUC_special() {
	
    // search in table CS_HAS_PR, not in the field CONCRETE_SUBST:CONCRETE_PROTO_ID !!!
	$sql_getProto = "select CONCRETE_PROTO_ID from CONCRETE_PROTO_STEP x WHERE (".
		" x.CONCRETE_SUBST_ID IN (select CONCRETE_SUBST_ID from ".$this->selcond_ori.")".
		")";
	$sql_getSubst = "CONCRETE_SUBST_ID in (select CONCRETE_SUBST_ID from CS_HAS_PR where CONCRETE_PROTO_ID in (".$sql_getProto.") )";	
	$icon="icon.CONCRETE_SUBST.gif";
	echo "<br>";
	
	$nice_tmp = tablename_nice2('CONCRETE_SUBST');
	$this->_show_element($icon,  'Used in '.$nice_tmp, "view.tmpl.php?condclean=1&t=CONCRETE_SUBST&tableSCond=". urlencode( $sql_getSubst ), '');
	
	echo "<br>";
	
	// subst in exp 
	//
	$sql_getProto = "select CONCRETE_PROTO_ID from CONCRETE_PROTO_STEP x WHERE (".
	   	" x.CONCRETE_SUBST_ID IN (select CONCRETE_SUBST_ID from ".$this->selcond_ori.")".
	   	")";
	$sql_exp_log = "EXP_ID in (select EXP_ID from EXP_HAS_PROTO x WHERE CONCRETE_PROTO_ID in (".$sql_getProto.") )";
	
	$icon="icon.EXP.gif";
	$this->_show_element($icon,  'Used in experiment', "view.tmpl.php?condclean=1&t=EXP&tableSCond=". urlencode( $sql_exp_log ));
	
}

function s_show_assocs( &$sql ) {

	if ( $this->gTrackUsedLib->assocsExist() ) { // only at first occurence
		$this->_showMethHead("... associated lists", "search.sub_elem.gif");
		echo "<blockquote>";
		$this->gTrackUsedLib->trackShowAssocs( $sql );
		echo "</blockquote>\n";
	} 
	
}

function s_formEnd() {
	// echo "</form>\n";
}

}


global $error;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();

$tablename= $_REQUEST["tablename"];
$advanced = $_REQUEST["advanced"];
$actionx= $_REQUEST["actionx"];
$parx= $_REQUEST["parx"];

$title = "Object tracking for a list of objects";
$infoarr=array();
$infoarr["help_url"] = "object_tracking.html";

$infoarr["title"]    = $title;
$infoarr["title_sh"] = "Object tracking";
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;
$infoarr["scriptID"] = "glob.objtab.trackgui";
if ($advanced) {
	$infoarr["locrow"] =   array( array($_SERVER['PHP_SELF']."?tablename=".$tablename, "Object tracking") );
}

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

//glob_printr($_POST, "POST vars");
if ( $tablename == "" ) htmlFoot("ERROR", 'please give a tablename!');
echo "[<a href='glob.objtab.search_usage.php?tablename=$tablename'>Scan database</a>] 
      [<a href='".$_SERVER['PHP_SELF']."?tablename=$tablename&advanced=1'>Advanced</a> <font color=gray>(used/unused objects)</font>]";
echo "<br>";

$mainPageLib = new gObjTabTrackGui($tablename, $parx);
$tabisbo 	 = cct_access_has2($tablename);

// echo "DEBUG: actionx:$actionx  ";

if ( $actionx == "forwProjBo" ) {
	$mainPageLib->forwardProjBO($sql);
	htmlFoot();
	exit;
}

if ( $actionx == "forwNorm" ) {
	if ( $parx['destTab']==NULL ) {
		$pagelib->htmlFoot('ERROR', 'No destination table given.');
	}
	echo '<ul>';
	$mainPageLib->forwardNorm($sql);
	$pagelib->chkErrStop();
	exit;
}

if ($advanced) {
	echo "<br>\n";
	echo "<table border=0 cellpadding=4 cellspacing=0 bgcolor=#EFEFEF width=600>";
	echo '<tr><td valign=top  width=50><img src="images/search.usage.gif" TITLE="used by other objects"> ';
	echo "</td><td valign=top>";
	echo '<B>Search if selected objects are used by other objects</B><br><img height=5 width=1><br>';
	echo "</td></tr></table>\n";
	echo "<ul>";
	$mainPageLib->form1();
	htmlFoot();
}

$mainPageLib->_showMethHead("... are used by other objects", "search.usage.gif");

echo '<blockquote>';

$mainPageLib->s_used_by($sql);

if ($tabisbo) {
	// TBD: here put the project
	$mainPageLib->projTrackLink();
	$mainPageLib->sObjLinkChi();
}

if ($tablename == "CONCRETE_SUBST" ) {
    $mainPageLib->_SUC_special();
}

echo "</blockquote>";

// search sub-objects ...
$mainPageLib->s_subObjects( $sql );

$mainPageLib->s_show_assocs( $sql );
$mainPageLib->s_formEnd();

//echo "</blockquote>";     
echo "<hr size=1 noshade>"; 
htmlFoot();

