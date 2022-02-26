<?php
/**
 * analyse activity of projects
 * @package obj.proj.treeana2.php
 * @swreq UREQ:xxxxxxxxxxxx
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @version
 * @param   
 *    $id          : proj.proj_id for start of tree view
	  $parx["action"] = ["dateana"]
	  					"copyprojs"
						"cutprojs"
	  $parx["treelevel"] = 0 : all sub projects
						 = 1 : show for all sub-trees level 1
	  					 = 2 : show for depth1 sub-trees
	  $parx["BOs"] = 1 : analyze all BOs
	  $sel
		  
 */
session_start(); 


require_once("func_head.inc");
require_once("db_access.inc");
require_once("globals.inc");
require_once("access_check.inc");
require_once ("o.PROJ.tree.inc");
require_once("varcols.inc");
require_once ('o.PROJ.paths.inc');
require_once ("visufuncs.inc");
require_once ("f.clipboard.inc");
require_once ("o.PROJ.subs.inc");


class mainTreeAnaC{
	var $showopt;
					
function init(
	&$sql,
 	$projid,
	$showopt=NULL   // "objects" = ["all"] - show projects and objects
				 			  // 			  "projs"- show only projects
	) {
			
	$this->projid  = $projid;
	$this->projPathObj = new oPROJpathC();
	$this->showopt = $showopt;
	
	$this->superLatest = NULL;
	$this->superProjId = NULL;
	$this->superProjcnt= 0;
	
	$this->barYears  = 3.0;
	$this->barMaxLen = 100;
	$this->barDays = $this->barYears * 356;
	
	$this->dateNowUnx = time();
	
	$this->projSubLib = new cProjSubs();
			
	$this->levelInfo = array( "0"=>"show latest project", 1=>"analyse ONE sub-level", 2=>"analyse ALL sub-levels");

}

function showLinks() {
	
	require_once ('func_form.inc');

	$parx = $this->showopt;
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Select mode";
	$initarr["submittitle"] = "Submit";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["id"]     = $this->projid ;
	$hiddenarr["parx[action]"]     = "dateana";
	$formobj = new formc($initarr, $hiddenarr, 0);

	$fieldx = array ( 
		"title" => "Mode", 
		"name"  => "treelevel",
		"object"=> "select",
		"val"   => $parx["treelevel"], 
		"inits" => $this->levelInfo,
		"notes" => "select mode"
		 );
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ( 
		"title" => "Analyse objects", 
		"name"  => "BOs",
		"object"=> "checkbox",
		"val"   => $parx["BOs"], 
		"notes" => "time consuming"
		 );
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );
	
	
}

function printPath(&$sql, $projid) {
	$desturl = "edit.tmpl.php?t=PROJ&id=";
	$pathstr = $this->projPathObj->showPathSlim($sql, $projid, $desturl);
	echo "<b>Base-project:</b> ".$pathstr."<br>\n";
}

function getIcon($tab_name) {
	$tab_name_icon = ($tab_name == "PROJ") ? "PROJ_link" : $tab_name;
	if ( $this->tabIcons[$tab_name_icon]!="" ) {
		$icon = $this->tabIcons[$tab_name_icon];
	} else {
    	$icon = file_exists("images/icon.$tab_name_icon.gif") ? "images/icon.$tab_name_icon.gif" : "images/icon.UNKNOWN.gif";
		$this->tabIcons[$tab_name_icon] = $icon;
	}
	return ($icon);
}

/**
 * Get MAX-date( all BOs of project )
 */
function _anaObjectsOfProj( &$sql, $proj_id) {

	$sqls = "select distinct(TABLE_NAME) from PROJ_HAS_ELEM where PROJ_ID=".$proj_id;
	$sql->query($sqls);
	$tabs = NULL;
	while ($sql->ReadRow() ) {
		$tabs[] = $sql->RowData[0];
	}
	if ($tabs==NULL)  return;
	
	$dateMax = NULL;
	foreach( $tabs as $dummy=>$table) {
		$sqlPre = $this->projSubLib->getTableSQL ( $proj_id, $table );
		$pkname = PrimNameGet2($table);
		$sqls = "select max(".$sql->Sql2DateString ( "a.crea_date", 0).") ".
				   " from $table x JOIN CCT_ACCESS a ON x.CCT_ACCESS_ID=a.CCT_ACCESS_ID".
				   " where ".$pkname." in (".$sqlPre.")" ;
		$sql->query($sqls);
		$sql->ReadRow();
		$datestr = $sql->RowData[0];
		if ( $datestr>$dateMax ) $dateMax = $datestr;
	}
	reset ($tabs); 
	return ($dateMax);
}

/**
 * Analyze one project
 */
function analyseDateOne( &$sql, $proj_id) {

	$this->projcnt++;
	$this->superProjcnt++;
	$sqls = "select ".$sql->Sql2DateString ( "a.crea_date", 0)." from PROJ p, CCT_ACCESS a where p.PROJ_ID=".$proj_id." AND p.CCT_ACCESS_ID=a.CCT_ACCESS_ID";
	$sql->query($sqls);
	$sql->ReadRow();
	$datestr = $sql->RowData[0];
	
	if ( $this->showopt["BOs"] ) {
		$dateBoStr = $this->_anaObjectsOfProj($sql, $proj_id);
		if ($dateBoStr > $datestr) $datestr = $dateBoStr;
	}
	
	//echo "DEBUG_one: ".$proj_id."<br>\n";
	return ($datestr);
}

function tabstart() {
	
	$this->tabobj = new visufuncs();
	echo "\n<form style=\"display:inline;\" method=\"post\" ".
		 " name=\"editform2\"  action=\"".$_SERVER['PHP_SELF']."\" >\n";
		 
	echo '<style type="text/css">'."\n";
	echo "table.ydataTab td  { white-space: nowrap; }\n";
    echo '</style>'."\n";
	
	$headOpt = array( "title" => "Analysed sub-projects level 1", "cssclass"=>"ydataTab" );
	$headx  = array ("Level 1 project", "Activity<br><img src=\"images/point_gray.gif\" height=12 width=".$this->barMaxLen.">",
		 "Date", "Proj<br>jects", "Newest sub-project");
	$this->tabobj->table_head($headx,   $headOpt);
	
}

function tabstop() {
	$this->tabobj->table_close();
	echo "<input type=hidden name='id' value='".$this->projid ."'>\n";
	echo "<input type=hidden name='parx[action]' value=''>\n";
	echo "<input type=submit value=\"Copy\" OnClick=\"setAction( 'copyprojs' )\"> <I>Copy selected projects to clipboard</i><br>\n"; // SUBMIT
	echo "<input type=submit value=\"Cut \" OnClick=\"setAction( 'cutprojs' )\"> <I>Cut selected projects from analysis project</i><br>\n"; // SUBMIT
	echo "</form>\n";
	
	$this->legend();
}

function _showPathLine( &$sql, $projSubID, $projname, $latestID, $latestDate, $projcnt, $sumflag=0 ) {
	
	$dataout = NULL;
	$desturl = "edit.tmpl.php?t=PROJ&id=";
	$pathstr = "<a href=\"".$desturl.$projSubID."\">".$projname."</a>";
	$checkbox = "<input type=checkbox name='sel[".$projSubID."]' value=1>&nbsp;";
	$dataout[0] = $checkbox.$pathstr;
	
	if ( $latestID ) {
		$pathstr  = $this->projPathObj->showPathSlim( $sql, $latestID, $desturl, $this->projid);
		$dateUnx  = mktime ( 0,0, 0, substr($latestDate,5,2), substr($latestDate,8,2), substr($latestDate,0,4) );
		$debtime  = date("Y-m-d",$dateUnx);
		$datediffDays = ($this->dateNowUnx - $dateUnx) / (60*60*24); // in days
		$barwidth = intval( ($this->barDays - $datediffDays) / $this->barDays * $this->barMaxLen);	
		if ($barwidth<0) 	$barwidth=0;
		$dategraph  = "<img src=\"images/point.gif\" height=12 width=".$barwidth.">";
		$dataout[1] = $dategraph;
		$dataout[2] = $latestDate;
		$dataout[3] = $projcnt;
		$dataout[4] = $pathstr;
	} else {
		$dataout[1] = "&nbsp;";
		$dataout[2] = "&nbsp;";
		$dataout[3] = "&nbsp;";
		$dataout[4] = "&nbsp;";
	}
	
	if ($sumflag) {
		$optrow = array( "bgcolor" => "#E0E0FF");
		$dataout[0] = "<b>Latest:</b>";
	}
	else $optrow=NULL;
	$this->tabobj->table_row($dataout, $optrow);
}

function _oneProjAnaX(&$sql, $proj_id_sub, $projname ) {

	if ( $projname!=".profile" ) { // ignore ".profile"-projects
			
		$datex = $this->analyseDateOne($sql, $proj_id_sub);
		
		if ($datex > $this->dateLatest)  {
			$this->dateLatest = $datex;
			$this->dateProjId = $proj_id_sub;
		}
		
		if ($datex > $this->superLatest)  {
			$this->superLatest = $datex;
			$this->superProjId = $proj_id_sub;
		}
	}
	
}	

function analyse_date(   
	&$sql, 
	$proj_id, 
	$proj_name,   
	&$proj_arr,
	$deeplevel=0
	) {
# descr: analyse date of project array
#        warning! recursive function
# input: proj_id   ... proy id of the subtree to print
#        proj_name ... name of the top-proj of the sub-tree
#        proj_arr  ... array containing whole project tree

	
    $varcol = & Varcols::get();
    $error  = & ErrorHandler::get();
	$deeplevel++;
	
	$nowSubAnalyse = 0;
	if ($deeplevel==1 AND $this->showopt["treelevel"]>0)  {
		$nowSubAnalyse = 1;
	}
	
	if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
		echo "<B>analyse_date()</B> DEBUG: [".$proj_id."] ".$proj_name ." : sizeSubs: ".sizeof($proj_arr[$proj_id])."<br>\n";
	}

    if ( !sizeof($proj_arr[$proj_id]) ) {
		$this->_oneProjAnaX( $sql, $proj_id, $proj_name );
		return;
	}
	
    foreach( $proj_arr[$proj_id] as $proj_id_sub=>$projname) { // show subprojects
		
		$this->_oneProjAnaX($sql, $proj_id_sub, $projname );
		
		if ($nowSubAnalyse) {
			
			$this->dateLatest = "";
			$this->dateProjId = "";
			$this->projcnt = 0;
		}
		
		$this->analyse_date($sql, $proj_id_sub, $projname, $proj_arr, $deeplevel);
		
		
		if ($nowSubAnalyse) {
			$this->_showPathLine( $sql, $proj_id_sub, $projname, $this->dateProjId, $this->dateLatest, $this->projcnt);
		}
		
    }
	
}

function analyse_date_start(   
	&$sql, 
	$proj_id, 
	$proj_name,   
	&$proj_arr
	) {
	
	$this->dateStart = "";
	$this->dateProjId = 0;
	$this->projcnt   = 0;
	
	echo "<b>Method:</b> ".$this->levelInfo[$this->showopt["treelevel"]]."<br>";
	if ( $this->showopt["BOs"]>0 ) echo "<b>SubAnalysis:</b> Also Objects<br>";
	
	if ( $this->showopt["treelevel"]>0 ) {
		
		$this->tabstart();
		$this->analyse_date($sql, $proj_id, $proj_name, $proj_arr);
		
		$this->_showPathLine( $sql, "", "", $this->superProjId, $this->superLatest, $this->superProjcnt, 1);
		
		$this->tabstop();
		
	} else {
	
		$this->analyse_date($sql, $proj_id, $proj_name, $proj_arr);
		echo "<b>Analysed Projects:</b> ".$this->projcnt ."<br>"; 
		if ($this->dateLatest!="") { 
			echo "<b>Latest Project:</b> ";
			// ana $this->dateProjId
			$desturl = "edit.tmpl.php?t=PROJ&id=";
			$pathstr = $this->projPathObj->showPathSlim($sql, $this->dateProjId, $desturl);
			echo $pathstr."<br>\n";
			echo "<b>Creation date:</b> ".$this->dateLatest."<br>";
		} else {
			echo "No sub-projects found.<br>";
		}
	} 
}

function doAnalyse( &$sql ) {
	$treeDepth = 0;
	if ( $this->showopt["treelevel"]==1 ) {
		$treeDepth = 1;
	}
	
	$proj_arr  = &oPROJ_tree::tree2array($sql, $this->projid, $treeDepth);
	// glob_printr($proj_arr, "proj_tree" );
	
	$this->analyse_date_start($sql, $this->projid, $proj_data['name'], $proj_arr);
}

function legend() {
	echo "<br>";
	$tabobj = new visufuncs();
	$dataArr= NULL;
	$dataArr[] = array( "<b>Activity</b>", "the stripe ".
			"identifies activity over the last ".$this->barYears." years.");
	
	$headOpt = array( "title" => "Legend", "headNoShow" =>1);
	$headx   = array ("Key", "Val");
	$tabobj->table_out2($headx, $dataArr,  $headOpt);
}

function help() {
	echo "<br>";
	htmlInfoBox( "Short help", "", "open", "HELP" );
	?>
	<ul>
			<LI>Analyse project tree</LI>
			<LI>Show the LAST created project in a project</LI>
	</ul>		
	<?
	htmlInfoBox( "", "", "close" );
}

}

//**********************************************************************************
//**********************************************************************************
//**********************************************************************************

$title = "Project Activity Analysis";
$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );

$id     = $_REQUEST['id'];
$parx 	= $_REQUEST['parx'];
$sel    = $_REQUEST['sel'];
if (empty($id)) $id=0;


$javascript = "
		function setAction( flagval ) {
			document.editform2.elements['parx[action]'].value = flagval;
			document.editform2.submit();
		}
";
		

$tablename="PROJ";
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["help_url"] ="o.PROJ.treeana2.html";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;
$infoarr["javascript"]   = $javascript;

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

if (empty($id)) htmlFoot("ALERT", "no project ID given.");

if ( $parx["action"] == "copyprojs" OR $parx["action"] == "cutprojs" ) {
	
	$infoTxt =array("copyprojs"=>"Copy selected projects to clipboard.",
					"cutprojs"=>"Cut selected projects from analysis project."
				   );
	echo "<ul><b>".$infoTxt[$parx["action"]]."</B><br>";
	
	$sizeproj = sizeof($sel);
	
	if (!$sizeproj) htmlFoot("ERROR", "No projects selected"); 
	
	$clipobj = new clipboardC();
	$clipobj->resetx();
	
	$clipobj->obj_put ( "PROJ_ORI", $sel ); 
	
	if (  $parx["action"] == "cutprojs" ) {
		$clipobj->setCutProject($id);
		echo "Info: Set CUT-projects.<br>";
	}
	echo "Info: <b>".$sizeproj."</b> projects copied to clipboard.<br>";
	htmlFoot("<hr>"); 
}

$projTreeObj = new mainTreeAnaC();
$projTreeObj->init($sql, $id, $parx);

$projTreeObj->showLinks();

echo "<ul>\n";

if ($parx["treelevel"]=="") {
	htmlInfoBox( "Please select", "Please select one level", "", "INFO" );
	mainTreeAnaC::help();
	
	htmlFoot("");
} 



$reading_allowed = access_reading_allowed($sql, "PROJ", $id);
if ($error->printLast()) htmlFoot();

$projTreeObj->printPath($sql, $id);
echo "<br>"; 

$projTreeObj->doAnalyse($sql);

$error->printLast();
htmlFoot('<hr>');

// ******************** end of page ********************
