<?php
/**
 * Analyze unique columns for a selction of objects
 * @package glob.objtab.coluniq.php 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $tablename
  		  $parx[colx] column name
		  $parx["sort"] "sortoc", "nosort"
		  $go
 */
 
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("sql_query_dyn.inc");
require_once ("glob.objtab.sub.inc");
require_once ('func_form.inc');
require_once ('visufuncs.inc');

class colUniqueC {

function init($tablename) {
	$this->infox       = NULL;
	$this->infox["MAX_ANA_NUM"]  = 1500;
	$this->infox["MAX_SHOW_NUM"] = 200;
	$this->tablename=$tablename;
	$this->isfk		= 0;
}

function init2($colx, $sqlAfter, $sortx) {
	$this->colx = $colx;
	$this->sqlAfter = $sqlAfter;
	$this->sortx = $sortx;
}

function info ($key, $text, $notes=NULL ) {
    // FUNCTION: print out info text
    if ($notes!="")  $notes = " &nbsp;<I>".$notes."</I>";
    echo "<font color=gray>".$key.":</font> <B>".$text."</B>".$notes."<br>\n";

}

function GoInfo($go, $coltxt=NULL) {
	// FUNCTION: headline for a site tool
	
	
	$tablename = $this->tablename;
	echo "<B><font size=+1 color=#606060>";
	if ( !$go )   echo "1. Select column";
	if ( $go==1 )   echo "2. Analyze unique data";
	if ( $go>0 )echo "  of column <font color=#000050 size=+2>". $coltxt."</font> ";
	echo "</b></font>";
	if ( $go ) echo " [<a href=\"".$_SERVER['PHP_SELF']."?tablename=".$tablename."\">Start</a>]";
	echo "<br><br>\n";
}

function GetNiceName(&$sql, $tablename, $id) {
	$niceName = obj_nice_name($sql, $tablename, $id);
	return($niceName);
}

function preAnalyse( &$sql ) {

	$colx = $this->colx;
	$tablename = $this->tablename;
	
	$this->infox["colNice"] = columnname_nice2($tablename, $colx);
	$this->GoInfo($go, $this->infox["colNice"]);
	
	$ftab_prim_name = "";
	$ftab_imp_name  = "";
	$this->fktable = fk_check2( $colx, $tablename, $ftab_prim_name, $ftab_imp_name );
	if ($colx=="a.NICK")  $this->fktable = "DB_USER";
	
	if ( $this->fktable!="" ) {
		$this->isfk = 1;
		$this->infox["fkTabNice"] = tablename_nice2($this->fktable);
		$this->info("Column is a foreign key to table", $this->infox["fkTabNice"]  );
	}
	if ($this->sortx == "sortoc") {
		$this->info("Sort criteria: ", "by number of occurence"  );
	}
	
	$sqlsLoop = "SELECT count(distinct(x.".$colx.")) FROM ".$this->sqlAfter;
	if ($colx=="a.NICK")  $sqlsLoop = "SELECT count(distinct(a.DB_USER_ID)) FROM ".$this->sqlAfter;
	$sql->query($sqlsLoop);
	$sql->ReadRow();
	$this->infox["numUniqueData"] = $sql->RowData[0];
	
	// $mainObj->info("Number of unique data values", $this->infox["numUniqueData"] );
	
	if ($this->infox["numUniqueData"] > $this->infox["MAX_ANA_NUM"]) {
		$this->info("Number of unique data values", $this->infox["numUniqueData"] );
		htmlFoot("INFO","Too many different data sets for a deeper analysis. Less than ".$this->infox["MAX_ANA_NUM"]." can be analysed");
	}
}

function analyze(&$sql, &$sql2) {
	
	
	$colx = $this->colx;
	$tablename = $this->tablename;
	
	
	$tabobj = new visufuncs();
	$headx = array("data", "number of occurences");
	$numColID = 1; // column 1
	if ($this->isfk) {
		$headx = array("ID", "Name", "number of occurences");
		$numColID = 2;
	}
	$headOpt = array("colopt" => array( $numColID => " align=right") );
	$tabobj->table_head($headx, $headOpt);
	
	$topt     = NULL;
	$cntSum   = 0;
	$sqlOrder = "";
	if ($this->sortx == "sortoc") {
		$sqlOrder = " order by count(x.".$colx.") DESC";
		if ($colx=="a.NICK")  $sqlOrder = " order by count(a.DB_USER_ID) DESC";
	}
	
	$cntrow = 0;
	$sqlsLoop = "SELECT max(x.".$colx."), count(x.".$colx.") FROM ".$this->sqlAfter. " group by x.".$colx. $sqlOrder;
	if ($colx=="a.NICK")  $sqlsLoop = "SELECT max(a.DB_USER_ID), count(a.DB_USER_ID) FROM ".$this->sqlAfter. " group by a.DB_USER_ID". $sqlOrder;
	
	$sql->query($sqlsLoop);
	while ( $sql->ReadRow() ) {
	
		$colval = $sql->RowData[0];
		$colCnt = $sql->RowData[1];
		
		if (!$colCnt) continue;		// also NULL values occur in the selection, but give no value for COUNT
		if ($this->isfk) {
			$nicename = $colval;
			$idcol = "<a href=\"edit.tmpl.php?t=".$this->fktable."&id=".$colval."\">".$colval."</a>";
			$nicename = $this->GetNiceName($sql2, $this->fktable, $colval);
			$dataArr = array($idcol, $nicename, $colCnt);
		} else {
			if (strlen($colval)>40) $colval = substr($colval,0,40)."...";
			$dataArr = array( htmlspecialchars($colval), $colCnt );
		}
		$cntSum = $cntSum + $colCnt;
		$tabobj->table_row($dataArr, $topt);
		if ($cntrow>$this->infox["MAX_SHOW_NUM"]) break;
		$cntrow++;
	}
	
	// count NULL values
	$sqlAdvanced = "x.".$colx." is NULL";
	if ($colx=="a.NICK")  $sqlAdvanced = "a.DB_USER_ID is NULL";
	$sqlopt = NULL;
	
	list ( $fromClause, $tableSCondM, $whereXtra, $sel_info, $classname, $mother_idM ) = 
			selectGet( $sql, $tablename, 0, $sqlAdvanced, $_SESSION['s_tabSearchCond'], $sqlopt );
			
	$sqlAfterNew = full_query_get($tablename, $fromClause, $tableSCondM, $whereXtra);
	
	$sqlsLoop = "SELECT count(1) FROM ".$sqlAfterNew;
	$sql->query($sqlsLoop);
	$sql->ReadRow();
	$cntNull = $sql->RowData[0];
	$dataArr = array("<font color=gray>NULL</font>", $cntNull);
	if ($this->isfk) {
		$dataArr = array("<font color=gray>NULL</font>","&nbsp;", $cntNull);
	}
	$tabobj->table_row($dataArr, $topt);
	$tabobj->table_close();
	
	echo "<br><br>\n";
	
	if ($cntrow > $this->infox["MAX_SHOW_NUM"]) {
		echo "<font color=red><B>WARNING:</B></font> List stopped, because too many elements.<br>\n";
	}
	
	
	
	$tabobj = new visufuncs();
	$header = NULL;
	$tmparr = array("Number of distinct values:"=> $this->infox["numUniqueData"], "Total number of set values:"=>$cntSum);
	$resopt = array ("title"=>"Statistics", "showid"=>1);
	$tabobj->table_out( $header, $tmparr,  $resopt);
}

function attribCompare(&$sql) {

	$tablename = $this->tablename;
	
    $errcode=0;
    do {
    	
    	
        echo '<form method="post" name=xform action="glob.objtab.search_2.php?go=1&tablename='.$tablename.'">';
		echo "<br><b>Analyse unique class parameters:</b>";
        echo "<table bgcolor=#EFEFEF cellpadding=5><tr><td>\n";
        
        $sqls = "SELECT extra_class_id, name FROM extra_class WHERE table_name='".$tablename."'";
        $sql->query($sqls);
        $class_arr = array();
        while ($sql->readRow()) {
            $class_arr[$sql->RowData[0]]=$sql->RowData[1];
        }
        
        if ( !sizeof($class_arr) ) {
            $errcode=1;
            break;
        }
        
        
        echo "<select name=extra_class_id>";
        echo "<option value=\"\">--- Select class ---\n";
        foreach( $class_arr as $class_id=>$class_name) {
            echo '<option value="' .$class_id. '">' .$class_name. "\n";
        }
        reset ($class_arr);
        echo "</select><br>\n";
        
        
        echo "</td></tr></table>\n";
        echo '<input type=submit value="=&gt; Next">';
        echo "</form>";
        return;
        
        
    } while (0);
    
    if ($errcode) 
        switch ($errcode) {
        case 1:
            echo "INFO: table '$nicetable' has no classes.<br>";
            
            break;
        default: echo "Error $errcode occured.";
            
            break;
        }
        echo "</td></tr></table>\n";
        return;
    return;

}

function Form1($tablename, $colarr, $parx) {

	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Select column";
	$initarr["submittitle"] = "Analyse";
	$initarr["tabwidth"]    = "AUTO";
	$initarr["tabnowrap"]   = 1;
	$hiddenarr = NULL;
	$hiddenarr["tablename"]     = $tablename;

	$formobj = new formc($initarr, $hiddenarr, 0);
	
	$tmpinits = NULL;
	foreach( $colarr as $key=>$val) {
		$tmpinits[$key] = $val;
	}
	$tmpinits["a.NICK"] = "owner";
	
	$tmpinitsSort = array("nosort"=>"no sorting", "sortoc"=>"sort by occurence");

	$fieldx = array ( "title" => "Column", "name"  => "colx",
			"object" => "radio",
			"val"   => $selcol, 
			"inits" => $tmpinits, 
			"notes" => "Select column",
			"optx"  => array ("rowbr"=>1)  );
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ( "title" => "", "name"  => "",
			"object" => "hr"  );
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ( "title" => "Sort", "name"  => "sort",
			"object" => "radio",
			"val"   => $parx["sort"], 
			"inits" => $tmpinitsSort, 
			"notes" => "Select sort crietria",
			"optx"  => array ("rowbr"=>1)  );
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );
}

}

// ----------------------------------------------------------------

global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$tablename=$_REQUEST['tablename'];
$go=$_REQUEST['go'];
$parx=$_REQUEST['parx'];

$tablenice = tablename_nice2($tablename);


$title       = "Show unique column data";

$infoarr=array();
$infoarr["title"] = $title;
$infoarr["title_sh"] = "unique column data";
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1;     
$infoarr["locrow"]   = array(
			array("searchAdvance.php?tablename=".$tablename, "Search advanced")
			);


$mainObj = new colUniqueC();



if ( $parx["sort"] == "" ) $parx["sort"] = "sortoc";

$mainObj->init($tablename);

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sql, $infoarr);
if ( $go ) echo "&nbsp; [<a href=\"".$_SERVER['PHP_SELF']."?tablename=$tablename\">Start again</a>]<br>\n";
echo "<ul>";



$t_rights = tableAccessCheck( $sql, $tablename );
if ( $t_rights["read"] != 1 ) {
	tableAccessMsg( $tablenice, "read" );
	htmlFoot();
}

$sqlopt=NULL;
$sqlAfter  = get_selection_as_sql( $tablename, $sqlopt);


$stopReason = "";
$tmp_info   = $_SESSION['s_tabSearchCond'][$tablename]["info"];
if ($tmp_info=="") $stopReason = "No elements selected.";
if ($headarr["obj_cnt"] <= 0) $stopReason = "No elements selected.";




if ($stopReason!="") {

	 htmlInfoBox( "Info", "To analyse column data, please select elements of '".$tablenice."'! (". $stopReason.")", "", "WARN");
	
	if ( !$go ) {
		$mainObj->attribCompare($sql);
	}
	htmlFoot();
    
}


$objtabobj = new objtabSubC();

$colopt=array("withpk"=>1);
$colarr = $objtabobj->colsGetNorm( $sql, $tablename, $colopt );


if ( !$go ) {
	$mainObj->GoInfo($go);
	$mainObj->Form1($tablename, $colarr, $parx);
	
	$mainObj->attribCompare($sql);
	htmlFoot();
}

$colx = $parx["colx"];
$mainObj->init2($colx, $sqlAfter, $parx["sort"]);

if ($colx=="") htmlFoot("Error", "Please select a column!");

$mainObj->preAnalyse( $sql );

$mainObj->analyze($sql, $sql2);


htmlFoot("<hr>");



