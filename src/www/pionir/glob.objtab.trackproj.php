<?php
/**
 * get objects, which are in PROJECTS, which are NOT in linked projects in a ref-project $refprojid
 * @package glob.objtab.trackproj.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $tablename
  		   $refprojid
		   $parx["infolevel"] :
		   			2 - show external objects
					3 - show external and internal objects
		   $parx["tempproj"]
		   $parx["updatenow"] - switch $go = 2
		   $go : 0 - configure
		   		 1 - analyse objects
				 2 - remove external objects from project
		   $_SESSION['s_formState']"["glob.objtab.trackproj"]
 * @version0 2002-09-04
 */
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("sql_query_dyn.inc");
require_once ("f.visu_list.inc"); 
require_once ('func_form.inc');
require_once ('o.PROJ.subs.inc');
require_once ('f.clipboard.inc');
require_once ("visufuncs.inc");
require_once ('o.PROJ.paths.inc');
require_once ('o.PROJ.addelems.inc');
 
class gTrackProjC {

function __construct( &$sql, $tablename, $refprojid, $parx) {

	$this->tablename = $tablename;
	$this->refprojid = $refprojid;
	$this->primary_key = PrimNameGet2($tablename);
	$this->impNameCol  = importantNameGet2($tablename);
	$this->infolevel = 1;
	
	$this->parx = $parx;
	$this->doSaveObjects = 1;
	
	$this->clipbobj = new clipboardC();
	$this->clipbobj->resetx();
	
	$this->projPathObj = new oPROJpathC();
	$this->projManiLib = new oProjAddElem($sql);
	
	if ( $parx["infolevel"]!="" )$this->infolevel = $parx["infolevel"];
	$sqlopt=array();
	$sqlopt["order"] = 1;
	$this->sqlAfter  = get_selection_as_sql( $tablename, $sqlopt);
	
	
}

function initProj(&$sql) {
	
	
	$lastParams = $_SESSION['s_formState']["glob.objtab.trackproj"];
	if ($lastParams["refprojid"]) {
		$this->refprojid = $lastParams["refprojid"];
	}

	if ($this->refprojid) {
		$sqls = "select NAME from PROJ where PROJ_ID=".$this->refprojid;
		$sql->query($sqls);
		$sql->ReadRow();
		$this->projname = $sql->RowData[0];
		
		// count sub-projects
		$sqls = "select count(1) from PROJ_HAS_ELEM where PROJ_ID=".$this->refprojid." AND TABLE_NAME='PROJ'";
		$sql->query($sqls);
		$sql->ReadRow();
		$this->subProjsCnt = $sql->RowData[0];
	}
}

function _objectget(&$sql, $objid) {
	$sqls = "select ".$this->impNameCol." from ".$this->tablename." where ".$this->primary_key ."=".$objid;
	$sql->query($sqls);
	$sql->ReadRow();
	$objname = $sql->RowData[0];
	return ($objname);
}

// @param $this->projLast -- last touched project
function _unlinkFromIntern(&$sql, &$sql2, $objid) {
	// RETURN: number of failed delete_object_actions
	global $error;
	$FUNCNAME= '_unlinkFromIntern';
	
	$tablename = $this->tablename;
	$refprojid = $this->refprojid;
	$noDelCnt = 0;
	
	// get the relevant INTERNAL projects
	$sqls = "select PROJ_ID from PROJ_HAS_ELEM where PRIM_KEY=".$objid." AND TABLE_NAME='".$tablename."'".
			" AND PROJ_ID in (select PRIM_KEY from PROJ_HAS_ELEM where PROJ_ID=".$refprojid.")" .
			" ORDER by PROJ_ID";
	$sql2->query($sqls, $FUNCNAME);
	
	while ( $sql2->ReadRow() ) {
		$subprojid = $sql2->RowData[0];
		// check access
		// check access rights
		$nodel=0;
		$o_rights = access_check($sql, "PROJ", $subprojid);	
		if (!$o_rights['insert']) {
			$nodel=1;
		} else {
			if ( $subprojid != $this->projLast ) {
				$this->projManiLib->setProject( $sql, $subprojid );
				$this->projLast = $subprojid;
			}
			$this->projManiLib->unlinkObj( $sql, $tablename, $objid );
		}
		if ($nodel) $noDelCnt++;
	}
	return ($noDelCnt);
}

function info ($key, $text, $notes=NULL ) {
    // FUNCTION: print out info text
    if ($notes!="")  $notes = " &nbsp;<I>".$notes."</I>";
    echo "<font color=gray>".$key.":</font> <B>".$text."</B>".$notes."<br>\n";

}

function GoInfo($go, $coltxt=NULL) {
	// FUNCTION: headline for a site tool
	echo "<B><font size=+1 color=#606060>";
	if ( !$go )   echo "1. Configure tracking";
	if ( $go==1 ) echo "2. Analyse objects";
	if ( $go==2 ) echo "3. Unlink external objects";
	echo "</font></b>";
	if ( $go>0 )echo "&nbsp;&nbsp;&nbsp;<font color=#000050>". $coltxt."</font>";
	echo "<br><br>\n";
}

function projectInfoOut( &$sql) {
	$this->info("Reference project", "<a href=\"edit.tmpl.php?t=PROJ&id=".$this->refprojid."\">".$this->projname."</a>");
	if ($this->tempProj) $this->info("Save project", "<a href=\"edit.tmpl.php?t=PROJ&id=".$this->tempProj."\">".$this->tempProjname."</a>");
	$this->info("Number of sub projects", $this->subProjsCnt);
}

function oneObjectScan(&$sql, $objid) {
	// RETURN; foundcnt, lastFoundProject
	$tablename = $this->tablename;
	$refprojid = $this->refprojid;
	$projs = NULL;
	$otherFound=0;
	
	$sqls = "select PROJ_ID from PROJ_HAS_ELEM where PRIM_KEY=".$objid. " AND TABLE_NAME='".$tablename."'".
			" AND PROJ_ID not in (select PRIM_KEY from PROJ_HAS_ELEM where PROJ_ID=".$refprojid.")";
	$sql->query($sqls);
	while ( $sql->ReadRow() ) {
		$projid = $sql->RowData[0];
		$otherFound++;
	}
	return array($otherFound, $projid);
}

function rowOut($dataArr) {
	// "#", "Object", "Hits", "action","Project"
	if ($this->tabOutRowOpen) {
		echo "</td></TR>\n";
		$this->tabOutRowOpen = 0;
	}
	$this->tabobj->table_row ($dataArr);
}

function tabOutStar( $text=NULL ) {

	if (!$this->tabOutRowOpen) {
		echo "<TR><td></td><td>\n";
	}

	if ($text=="<BR>") {
		echo "</td></TR>\n";
		$this->tabOutRowOpen = 0;
		return;
	}
	
	$this->tabOutRowOpen = 1;
	echo "*";
}

function getPath(&$sql, $projid) {

	$desturl = "edit.tmpl.php?t=PROJ&id=";
	$rettext = $this->projPathObj->showPathSlim( $sql, $projid, $desturl);
	return ($rettext);
}

function loopObjects( &$sql, &$sql2, &$sql3, $go ) {

    $lastProj = array();
    
	$tablename = $this->tablename;
	$externCount = 0;
	$sqls = "select ".$this->primary_key." from ".$this->sqlAfter;
	$sql2->query($sqls);
	
	$this->tabobj = new visufuncs();
	$headOpt = array( "title" => "Analysis table");
	$headx  = array ("#", "Object", "Hits", "Action", "Project" );
	$this->tabobj->table_head($headx, $headOpt);
	$this->tabOutRowOpen=0;

	$cnt			= 0;
	$starCount 		= 0;
	$brCnt   		= 0;
	$cntshow 		= 0;
	$this->projLast = 0; // must be set for _unlinkFromIntern()
		
	while ( $sql2->ReadRow() ) {
	
		$infotmp  =  "";
		$outarr   = NULL;
		$showObj  = 0;
		$nowLevel = $this->infolevel;
		$objid = $sql2->RowData[0];
		list($otherFound, $oneProjID) = $this->oneObjectScan($sql, $objid);
		
		if ( ($starCount+1)*0.01 == (int)(($starCount+1)*0.01) ) {
			$this->tabOutStar();
			ob_end_flush ( );
			while (@ob_end_flush()); // send all buffered output
			
			$brcnt++;
			
			if ( $brCnt>40 ) {
				$this->tabOutStar("<BR>");
				$brCnt=0;
			}
		}
		
		
		if ($otherFound) {
			$externCount++;
			if ( $this->doSaveObjects ) {
				$this->clipbobj->obj_addone ( $tablename, $objid );
			}
			
			if ( $go == 2 ) {
				$delFailAns = $this->_unlinkFromIntern($sql, $sql3, $objid);
				if ($delFailAns>0) $nowLevel++; // increase info-level
			}
			
			if ($nowLevel>=2) {
				$showObj = 1;
			}
			
			if ( $go == 2 ) {
				if ($delFailAns<=0) {
					$infotmp =  "<font color=green>unlinked.</font>";
				} else {
					$infotmp = "<font color=red>unlink failed. access right problems.</font>";
				}
			}
			

		} else {
			if ($nowLevel>=3) {
				$showObj = 1;
			}
		}
		
		
		if ( $showObj ) {
		
			$objname   = $this->_objectget($sql, $objid);
			if ($lastProj["ID"]!=$oneProjID) {
				$nowpath = $this->getPath($sql, $oneProjID);
			} else {
				$nowpath = $lastProj["path"];
			}
			
			$outarr[0] = $cntshow+1;
			$outarr[1] = "<a href=\"edit.tmpl.php?t=".$tablename."&id=".$objid."\">".$objname."</a>";
			$outarr[2] = $otherFound;
			$outarr[3] = $infotmp;
			$outarr[4] = $nowpath;
			$this->rowOut($outarr);
			$lastProj = array("ID"=>$oneProjID, "path"=> $nowpath);
			$cntshow++;
			$starCount = 0;
		} else {
			$starCount++;
		}
		
		$cnt++;
	}
	
	$this->tabobj->table_close();
	echo "<br>";
	$this->info("Analysed objects","<b>".$cnt."</b>");
	$this->info("Found external objects","<b>".$externCount."</b>");
	$this->info("Found external objects","are saved in the clipboard ...");
}

function formProj(&$sql) {
	
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Select reference project";
	$initarr["submittitle"] = "Submit";
	$initarr["tabwidth"]    = "AUTO";
	$initarr["dblink"]		= 1;
	$hiddenarr = NULL;
	$hiddenarr["tablename"]     = $this->tablename;

	$formobj = new formc($initarr, $hiddenarr, 0);
	
	

	$objname =  "------";
	if ($this->refprojid) $objname = $this->projname; 
	$inits = array( "table"=> "PROJ",
                              "objname" => $objname,
							  "pos"       => 0,
							  "projlink"  => 1
							  );
	$fieldx = array ( "title" => "reference project", 
			"name"  => "refprojid",
			"namex" => 1,
			"object" => "dblink",
			"val"   => $this->refprojid, 
			"inits" => $inits,
			"notes" => "contains project links" );
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ( "title" => "unlink now!", 
			"name"  => "updatenow",
			"object" => "checkbox",
			"val"   => 0, 
			"notes" => "no preparation, unlink objects now" );
	$formobj->fieldOut( $fieldx );
	
	$inits2 = array( "2"=>"2- show external objects", "3"=>"3- show external and internal objects" );
	$fieldx = array ( "title" => "info level", 
			"name"  => "infolevel",
			"object" => "select",
			"val"   => $this->parx["infolevel"], 
			"inits" => $inits2,
			"notes" => "infolevel (0..3)" );
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );
}

function form2 ( &$sql ) {

	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Unlink all External objects from Internal projects";
	$initarr["submittitle"] = "Unlink";
	$initarr["tabwidth"]    = "AUTO";
	$hiddenarr = NULL;
	$hiddenarr["tablename"]     = $this->tablename;
	$hiddenarr["refprojid"]     = $this->refprojid;
	$hiddenarr["parx[infolevel]"] = $this->parx["infolevel"];
	

	$formobj = new formc($initarr, $hiddenarr, 1);

	$formobj->close( TRUE );
	echo "<br>";
}

function help() {
	htmlInfoBox( "Short help", "", "open", "HELP" );
	
	echo "Track and unlink objects from 'internal projects', which are linked in External projects.<br>";
	echo "These found objects are saved in the clipboard.<br><br>";
	echo "This tool considers all project-links in the Reference project as 'internal projects'.<br>";
	echo "Produce this list of 'internal sub projects' by using the 'search' function in a project X.<br>";
	echo "Purpose: Prevent external objects from deletion. After unlinking external objects, only intern used objects stay in the sub projects.<br>\n";
	htmlInfoBox( "", "", "close" );
}

}
 
// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( ); // give the URL-link for the first db-login
$sql2  = logon2( );
$sql3  = logon2( );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$tablename = $_REQUEST["tablename"];
$refprojid = $_REQUEST["refprojid"];
$parx = $_REQUEST["parx"];
$go = $_REQUEST["go"];

$title  = "External Project-objects tracking";
#$infoarr['help_url'] = 'o.EXAMPLE.htm';
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1;          // show number of objects

// gives back the number of objects
$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sql, $infoarr);

if ($parx["updatenow"] == 1 ) $go=2;



echo "<I>Notes: Track and unlink objects from 'internal projects', which are linked in External projects.";
echo "</I><br>";

echo "<ul>";


$mainObj     = new gTrackProjC($sql, $tablename, $refprojid, $parx);
$mainObj->initProj($sql);


$listVisuObj = new visu_listC();
$tablenice   = tablename_nice2($tablename);

// check TABLE selection
$copt = array ("elemNum" => $headarr["obj_cnt"] ); // prevent double SQL counting
list ($stopFlag, $stopReason)= $listVisuObj->checkSelection( $sql, $tablename, $copt );
if ( $stopFlag<0 ) {
    htmlFoot("Attention", $stopReason." Please select elements of '".$tablenice."'!");
}

$tmpinf = "[<a href=\"".$_SERVER['PHP_SELF']."?tablename=".$tablename."&refprojid=$refprojid&parx[infolevel]=". $parx["infolevel"]."\">Restart</a>]";

$mainObj->GoInfo($go, $tmpinf);

if (!$refprojid OR $go<=0) {
	// get project
	$mainObj->formProj($sql);
	echo "<br>";
	$mainObj->help();
	htmlFoot();
} 

if ( $go==1 ) {
	if ($refprojid) $_SESSION['s_formState']["glob.objtab.trackproj"]["refprojid"] = $refprojid;
	$mainObj->form2 ( $sql );
}

$mainObj->projectInfoOut( $sql);
echo "<br>";


ob_end_flush ( );
while (@ob_end_flush()); // send all buffered output

$mainObj->loopObjects( $sql, $sql2, $sql3, $go );

htmlFoot("<hr>");