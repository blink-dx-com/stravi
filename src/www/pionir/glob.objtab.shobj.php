<?php

/**
 * full details of a selection of objects
 * - load optional lib "obj.".$tablename_l.".xedit.php"
 * 
 * @package glob.objtab.shobj.php 
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param $t ( table_name )
 * @param $parx
 */
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("f.sql_query.inc");
require_once ("f.visu_list.inc");     // sub function LIST visualization
require_once ('object.oneview.inc');

class globObjListShow2 {

    function __construct($tablename, $parx) {
    	$this->tablename = $tablename;
    	$this->oneObjLib = new objOneviewC($tablename);
    	// $this->oneObjLib->init();
    	$this->parxNow = $parx;
    	if ( $this->parxNow["page"]<=0 ) $this->parxNow["page"] = 1;
    	
    	$tablename_l= strtolower($tablename);
//     	$xtra_file  = "obj.".$tablename_l.".xedit.inc";
//     	if ( file_exists($xtra_file) ) {
//     	    require_once($xtra_file);       //TBD:  include object oriented extra functions
//     	} 
    }
    
    function setEntryNum($foundEntries) {
    	$this->foundEntries = $foundEntries;
    }
    
    function loopy ( &$sql, &$sql2, &$sql3, $sqlAfter) {
    	global $error;
    	
    	$tablename = $this->tablename ;
    	$primary_key = PrimNameGet2($tablename);
    
    	$sqlsLoop = "SELECT x.".$primary_key." FROM ".$sqlAfter;
    	$sql3->query($sqlsLoop);
    	$obopt = array("showAccess"=>1);
    	$obopt["withAssocs"]     = 1;
    	
    	// TBD: test
    	$this->parx["entryPerPage"] = 20;
    	
    	$cnt = 0;
    	$this->moreExists=0;
    	$moreExists = 0;
    	$startShow = ($this->parxNow["page"]-1) * $this->parx["entryPerPage"] + 1;
    	$endShow   = $startShow + $this->parx["entryPerPage"] - 1;
    	
    	while ( $sql3->ReadRow() ) {
    		$showit = 0;
    		$cnt++;
    		
    		if ( $cnt >= $startShow ) $showit=1;
    		if ( $cnt > $endShow )   {
    			$moreExists = 1;
    			$cnt--;
    			break;
    		}
    		
    		if ($showit) {
    			$objid = $sql3->RowData[0];
    			$obopt["headLinePreStr"] = $cnt.". ";
    			$obopt['showObjLink'] = 1;
    			$this->oneObjLib->set_obj($objid);
    			$this->oneObjLib->oneOut( $sql, $sql2, $obopt);
    			if ($error->Got(READONLY))  {
    			    echo "Object ID: ".$objid." not found.<br>\n";
    				$error->reset();
    			}
    			echo "<br>\n";
    		}
    		
    		if ($moreExists) {
    			$this->moreExists=1;
    		}
    		
    		// $this->stopTable();
    		
    		$this->showinf["lastshow"]  = $cnt;
    		$this->showinf["startShow"] = $startShow;
    		$this->showinf["endShow"]   = $endShow;
    		
    	}
    }
    
    function postLoopy () {
    	// finish listViewPageControl()
    	
    	
    	$tablename = $this->tablename ;
    	echo "&nbsp;&nbsp;&nbsp;Entries: <b>".$this->showinf["startShow"]."</B>...<b>".$this->showinf["lastshow"]."</B> of ".$this->foundEntries;
    	echo "&nbsp;&nbsp;&nbsp;&nbsp;Page: ";
    	
    	$destUrl   = $_SERVER['PHP_SELF']."?t=".$tablename;
    	$pageCnt   = 1;
    	$firstPoints     = 0;
    	$pagesShowAround = 5;
    	$thisPage  = $this->parxNow["page"];
    	if ($thisPage<=5) $pagesShowAround = 10 - $thisPage + 1;
    	
    	$pagemax = ceil ($this->foundEntries / $this->parx["entryPerPage"]);
    	
    	if ( $pagemax>1 AND $this->parxNow["page"]>1) echo  "&nbsp;<a href=\"".$destUrl."&parx[page]=".($this->parxNow["page"]-1)."\">&lt;&lt;prev</a>&nbsp;&nbsp;";
    	
    	while ($pageCnt <= $pagemax) {
    	
    		$pagePrintFlag=0;
    		$pageOut = "<a href=\"".$destUrl."&parx[page]=".$pageCnt."\">".$pageCnt."</a>";
    		// echo "DEBBB: " . abs($pageCnt-$thisPage).":$pagesShowAround ";
    		if ( $pageCnt==$thisPage ) $pageOut = "<b>" . $pageOut . "</b>";
    		
    		if ( (abs($pageCnt-$thisPage)<$pagesShowAround) OR ($pageCnt==1)  ) {
    				echo $pageOut . " ";
    				$pagePrintFlag=1;
    		}
    		
    		if ( ($pageCnt<$thisPage) AND !$pagePrintFlag AND !$firstPoints) {
    			echo " ... ";
    			$firstPoints=1;
    		}
    		
    		if ( ($pageCnt>$thisPage) AND !$pagePrintFlag ) {
    			echo " ... ";
    			break;
    		}
    		$pageCnt++;
    	}
    	if ( $pagemax>1 AND $this->parxNow["page"]<$pagemax) echo  "&nbsp;&nbsp;<a href=\"".$destUrl."&parx[page]=".($this->parxNow["page"]+1)."\">next &gt;&gt;</a> ";
    	
    	echo "<br><br>\n";
    	echo "<ul>";
    	// $this->prefForm();
    }


}
// --------------------------------------------------- 
global $error;

$error = & ErrorHandler::get();
$sqlo   = logon2( ); // give the URL-link for the first db-login
$sqlo2  = logon2( );
$sqlo3  = logon2( );
if ($error->printLast()) htmlFoot();
//$varcol = & Varcols::get();

$tablename = $_REQUEST['t'];
$parx    = $_REQUEST['parx'];

$xcss    = fObjFormSub::datatab_css();

$title       = "Show detailed features of selected objects";
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1;          // show number of objects
$infoarr["css"]      = $xcss;

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);

echo "<ul>";



// ----------------------------------------------------------------
// SPECIALS for LIST SELECTION
// ----------------------------------------------------------------
$listVisuObj = new visu_listC();
$tablenice   = tablename_nice2($tablename);

if ( $tablename=="") htmlFoot("Error","Please give a table.");

$t_rights = tableAccessCheck( $sqlo, $tablename );
if ( $t_rights["read"] != 1 ) {
	tableAccessMsg( $tablenice, "read" );
	htmlFoot();
}
$sqlopt=array();
$sqlopt["order"] = 1;
$utilLib = new fSqlQueryC($tablename);
$sqlAfter = $utilLib->get_sql_after( $sqlopt );


// check TABLE selection
$copt = array ("elemNum" => $headarr["obj_cnt"] ); // prevent double SQL counting
list ($stopFlag, $stopReason)= $listVisuObj->checkSelection( $sqlo, $tablename, $copt );
if ( $stopFlag<0 ) {
    htmlFoot("Attention", $stopReason." Please select elements of '".$tablenice."'!");
}



$numpks = countPrimaryKeys($tablename);
if ($numpks!=1) htmlFoot("Sorry", "This function only works for 'Business objects'. (object-types with ONE primary-key)"); 


$mainObjLib = new globObjListShow2($tablename, $parx);
$mainObjLib->setEntryNum($headarr["obj_cnt"]);
$mainObjLib->loopy ( $sqlo, $sqlo2, $sqlo3, $sqlAfter);
$mainObjLib->postLoopy ();
htmlFoot("</ul><hr>");
