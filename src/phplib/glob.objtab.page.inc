<?php
/*MODULE: glob.objtab.page.inc
  DESCR:  html page intro for list selection
  AUTHOR: qbi
  RETURN: 
  INPUT: 
  VERSION: 0.1 - 20020904 	
  USAGE:
  
  $mainObj = new gObjTabPage($sqlo, $tablename );
  $mainObj->showHead($sqlo, $infoarr, $headopt);
  $mainObj->initCheck($sqlo);
*/

require_once("sql_query_dyn.inc");
require_once ("f.visu_list.inc");     // sub function LIST visualization


class gObjTabPage extends gHtmlHead {
	
	var $headarr; /* "obj_cnt" */

function __construct(
	&$sql,  
	$tablename
	) {
					
	$this->tablename=$tablename;
	// $this->pagelib = NULL;
	
	if ( $tablename == "" ) {
		// echo "ERROR: No tablename given!<br>";
		echo "<ul>";
		htmlFoot("Error","Please give a tablename");
	}
	
	$sqlopt["order"] = 1;
	$this->sqlAfter       = get_selection_as_sql( $tablename, $sqlopt);
	$this->sqlAfterNoOrd  = get_selection_as_sql( $tablename);
}

// show HTML page head
function showHead(
	&$sql,		  	
	&$infoarr,  /* typical:
				["obj_cnt"]  = 1;   
				*/
 	$options = NULL /*
				"noHead" => 0,1
				*/
	) {
	$tablename = $this->tablename;
	$infoarr["form_type"] = "list";
	$infoarr["obj_name"] = $tablename;
	
	$this->infoarr = &$infoarr;
	
	if ( $options["noHead"]!=1 ) {
		// $this->pagelib = new gHtmlHead();
		parent::__construct(); // create extended class !
		$this->headarr = $this->startPage($sql, $this->infoarr); // $this->pagelib
	}
}

function getSqlAfter() {
	return ($this->sqlAfter);
}

function getSqlAfterNoOrd() {
	return ($this->sqlAfterNoOrd);
}

function getHeadArr() {
	return ($this->headarr);
}

/**
 * 
 * @param $sql
 * @param $option : "checkNumMax"
 * @return 
 */
function initCheck( &$sql, $option=NULL ) {
	// check TABLE selection
	$tablename =$this->tablename;
	
	$listVisuObj = new visu_listC();
	$tablenice   = tablename_nice2($tablename);
	
	$copt=NULL;
	if ( $this->headarr["obj_cnt"]>0 ) $copt["elemNum"] = $this->headarr["obj_cnt"];
	if (!$copt["elemNum"]) $copt["doCount"] = 1;
	if ($option["checkNumMax"]>0) $copt["checkNumMax"] = $option["checkNumMax"];
	
	list ($stopFlag, $stopReason)= $listVisuObj->checkSelection( $sql, $tablename, $copt );
	if ( $stopFlag<0 ) {
		htmlFoot("Attention", $stopReason." Please select elements of '".$tablenice."'!");
	}
}

/*
function chkErrStop() {
	$this->pagelib->chkErrStop();
}

function htmlFoot() {
	$this->pagelib->htmlFoot();
}
*/

}