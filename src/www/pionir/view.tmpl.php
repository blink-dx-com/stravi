<?php
/**
 * show table view for ONE DB-table, identified by $t
 * GLOBALS:  $_SESSION['userGlob']["o.". $tablename.".condX"]
   INCLUDES: "obj.".$TABLENAME_LOW.".xfunc.inc"
 			 "obj.".$TABLENAME_LOW.".xview.inc"
   INTERN STRUCTS:
  	STRUCT:searchArr_struc $searchArr = array( 
  		"alias"=>$searchAlias, 
  		"cond"=>$tableSCond, 
  		"column"=>$searchCol, 
  		"stext"=>$searchtxt, 
		"op"=>$searchBool, 
		"condclean" => $condclean,
		"infoCond" => optional condition info
		"news" => optional news, which will be shown
		);
  
     STRUCT:colNames_sh  $colNames_show[] = array ( 
            "cc"=>$column_code, 
            "ce"=>$column_ext_name", 
            "show" => $show_flag, 
            "nice" => $niceCol, 
            "app" => $app_type );
            
 * @package view.tmpl.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   
 *  $t: tablename (upper case), MANDATORY
	$action		: "select" otherwise contains a destination URL like  "import1.php" (with our without "?")
		- $cctgo ... manage a selection of an object
		- if connection PARENT window is lost, reload this window AND clear cctgoba !!!
	$cctgoba	: if SET; GO BACK TO PARENT, contains id of form (0,1,2,..), parent input field ($cctgoba)
    $cctgobacol	: if $cctgoba: OPTIONAL return ID of column $cctgobacol
	$cctgoclear : =1: clear all back information 
	$cctgoCleClo: =1: $cctgoclear=1 and close window
    $cctgo['sel']	: GO BACK TO php-script stored in this var; shows SUBMIT
	$cctgo['info']  : info to forwarded selection
	$listshow	: show list, no matter if search condition is set [0,1]
	$listall    : show FULL list (more than 500 sets per page)
    $viewmode	: "lean" -> export friendly output (slim html table), no forms, no images
			    : "lean_csv" -> CSV format
	for search:
	  $sel[]	    : select some elements with primary ids; example: sel[id1]=1&sel[id2]=1&sel[idxxx]=1...
	  $condclean	: =1 clean all conditions
	  $tableSCond 	: add search condition, e.g. "exp_id=300" 
	  $searchCol    : NAME of Column; specials:
	  					EXTRA_ATTRIB: "class.CLASS_ID.MAP_COL" e.g. "class.1083.S02"
	  					VARIO: "vario.COLNAME"
	  $searchtxt    : searchable text or ID: "searchTxt", 345
	  $searchBool   : > < == !=
	  $searchOp_i   : search OP by interactive template view.tmpl.php, "OR", "AND"  "NEW" => special command
	  $searchCase   : no |[yes] case sensitive ??? TBD
	  $searchAlias  : alias substitutes $tableSCond and $condclean, can be a string (SYS) or a number (USER_DEF)
	                : predefined: absID, my_today, my_data
	  $searchMothId : if this is a associated table, gives the ID of the mother object
	  $searchClass  : CLASS_NAME
      $searchX      : extended SQL after main select see $_SESSION['s_tabSearchCond']["y"]
      $searchidx    : qucick search string
	  
	temporary options: (activ for this call)
	  $view_opt["setsPerView"]: sets per view "all", NULL
      $view_opt["pureids"]: output pure database IDs instead of NAME of object-ID
      $view_opt["ShowFK"] : show FKs for this page
      $view_opt["colCoNa"]: show CODE name of column, e.g. EXP_ID instead of "exp id"
      $view_opt["fromArch"] : set value for $_SESSION['s_tabSearchCond'][$tablename]['arch']
      		[0] : no action
      		1   : take data from optional ARCHDB
      		-1  : daactivate for session
      
	show options:
	  $exp_raw_desc_id: ID of EXP_RAW_DESC (for visualisazion of dynamic columns)
	  $viewPage	: counter for viewed pages
	  $userprefset[variable]    : =value (set userGlob["o.TABLENAME.variable"]= value )
	     - e.g. sortcrit='', showsets=50

 */ 
session_start(); 


require_once("db_access.inc");
require_once("globals.inc");
require_once('func_head2.inc');
require_once('func_head.inc');
require_once("object.subs.inc");
require_once("access_check.inc");
require_once("table_access.inc");

require_once("view.sub.html.inc");
require_once("view.tmplGui.inc");
require_once("subs/view.tmpl.extra2.inc");

require_once("sql_query_dyn.inc");
require_once("db_x_obj.inc");
require_once("f.clipboard.inc");
require_once("down_up_load.inc");
require_once('f.s_historyL.inc'); 
require_once 'o.CCT_TABLE_DYN.subs.inc';

$error = & ErrorHandler::get();
$varcol= & Varcols::get();
$sql  = logon2( $_SERVER['PHP_SELF'] );
$sql2 = logon2(  );

$tablename=NULL;
$t = $_REQUEST['t']; 
$viewmode  = $_REQUEST['viewmode']; 
$cctgobacol= $_REQUEST['cctgobacol']; 
$cctgoba   = $_REQUEST['cctgoba']; 
$cctgo     = $_REQUEST['cctgo']; 
$cctgoclear= $_REQUEST['cctgoclear'];
// cctgoCleClo .. later
// listall     .. later
$view_opt  = $_REQUEST['view_opt']; 
$searchX   = $_REQUEST['searchX']; 
$searchClass = $_REQUEST['searchClass']; 
$exp_raw_desc_id = $_REQUEST['exp_raw_desc_id'];

$viewPage=$_REQUEST['viewPage'];
$searchMothId=$_REQUEST['searchMothId'];
$condclean=$_REQUEST['condclean'];
$sel=$_REQUEST['sel'];
$action=$_REQUEST['action'];
$listshow=$_REQUEST['listshow'];
$searchCol=$_REQUEST['searchCol'];
$searchBool=$_REQUEST['searchBool'];
$searchAlias=$_REQUEST['searchAlias'];
$searchOp_i=$_REQUEST['searchOp_i'];
$searchtxt=$_REQUEST['searchtxt'];
$tableSCond=$_REQUEST['tableSCond'];

$userprefset=$_REQUEST['userprefset'];


if ( $t !="" ) $tablename=$t;  // or take this ...

if (empty($viewPage))      $viewPage     = 1;
if (!isset($searchMothId)) $searchMothId = NULL;
if (empty($condclean))     $condclean    = 0;
if (!isset($sel))          $sel          = NULL;
if (!isset($action))       $action       = NULL;
if (!isset($listshow))     $listshow     = 0;
if (!isset($searchCol))    $searchCol    = NULL;
if (!isset($searchBool))   $searchBool   = NULL;
if (!isset($searchAlias))  $searchAlias  = NULL; 
if (!isset($searchOp_i))   $searchOp_i   = NULL; 
if (!isset($cctgoclear))   $cctgoclear   = NULL; 
if (!isset($userprefset))  $userprefset  = NULL; 



$viSubObj = new viewSubHtml($tablename);
$viGuiObj = new viewGuiC($viSubObj, $tablename);

$view_optg = unserialize($_SESSION['userGlob']["g.viewf.opt"]);
$view_opt["ShowFK"]  = $view_optg["listShowFK"];
$view_opt["CelSize"] = $view_optg["CelSize"];

$sql_work  = &$sql;

// check for TEMPORARY other ARCHIVE-database ...
// check, if TABLE is situated in an EXTERNAL database
// this objects identify OPTIONAL a DB-link to an other database; e.g. MYSQL:krake
// @var object $sql_work, $sql2_work - DB-links
$FOREIGN_DB =  globTabMetaByKey($tablename, 'FOREIGN_DB');
if ($FOREIGN_DB!=NULL) {
	// e.g. $FOREIGN_DB='krake'
	// echo "INFO:EXTRNAL_DB:".$tablename."<br>";
	if (!is_array($_SESSION['globals']['ext_database'][$FOREIGN_DB])) {
		$viGuiObj->viewHeadError($sql, 'ERROR', "Table <B>$tablename</B>: access to external database '".$FOREIGN_DB."' is not configured. ".
				"Please ask the administrator!", $tablename);
		exit;
	}
	
	$dbconfig = $_SESSION['globals']['ext_database'][$FOREIGN_DB];
	$sql_work  = logon_to_db( $dbconfig['user'], $dbconfig['passwd'], $dbconfig['host'], $dbconfig['dbtype'], NULL, false, $dbconfig['dbname']);
	// $sql2_work = logon_to_db( $dbconfig['user'], $dbconfig['passwd'], $dbconfig['host'], $dbconfig['dbtype'], NULL, false, $dbconfig['dbname']);
	if ($error->Got(READONLY))  {
		$error->set( 'MAIN', 1, 'error on login to DB:'.$FOREIGN_DB );
		$error->printAll();
		exit;
	}
} else {
	$sql_work  = &$sql;
	// $sql2_work = &$sql2;
}


$nicename    = tablename_nice2($tablename);
if ($nicename=="") {
 	$viGuiObj->viewHeadError($sql, 'ERROR', "Table <B>$tablename</B> is not described by the system. ".
 		"Please ask the administrator!", $tablename);
	exit;
}

$colNames  = columns_get_pos($tablename);
if ( !sizeof ($colNames) ) { // colNames exists?
  $viGuiObj->viewHeadError($sql, 'ERROR', 'No column definition found for table "'.$tablename.
  	'"! Please contact the admin!',  $tablename);
  exit;
}

$t_rights      = tableAccessCheck($sql, $tablename);
if ( $tablename == "PROJ" ) { // projects need special functions !!!
    $t_rights["insert"] = 0;
    $t_rights["delete"] = 0;
} 
if ( $_SESSION['sec']['appuser'] == 'root' ) {
    $t_rights["insert"] = 1;
    $t_rights["delete"] = 1;
}
if ( $t_rights["read"] != 1 ) {
	$viGuiObj->viewHeadError($sql, 'INFO', getTableAccessMsg( $nicename, 'read' ), $tablename);
	exit;
}

if ( $cctgoba!="" OR $cctgo['sel']!=NULL ) {	
    $_SESSION['s_formback'][$tablename] = array( $cctgoba, $cctgobacol, $cctgo['sel'], $cctgo['info']);
}    

if ( $_REQUEST['cctgoCleClo']>0 ) {
	unset($_SESSION['s_formback'][$tablename]);
	echo "<script language=\"JavaScript\"> \n";
	// only, if window has a parent ...
	echo "if (window.opener != null) \n"; 
	echo "  window.close();\n";
	echo "</script> \n";
}
if ($cctgoclear)  unset($_SESSION['s_formback'][$tablename]);

/**
 * @var array $formback 
 * 		['fid'] : ID of REMOTE form element, usually an open window
 *		['xcol']: [optional] xcol
 *		['url'] : [optional] back url; no extra window
 *		['info'] : info to selection
 */
$formback=NULL;

if ( is_array($_SESSION['s_formback'][$tablename]) ) { 
  $formback['fid'] = $_SESSION['s_formback'][$tablename][0];
  $formback['xcol']= $_SESSION['s_formback'][$tablename][1];
  $formback['url'] = $_SESSION['s_formback'][$tablename][2];
  $formback['info']= $_SESSION['s_formback'][$tablename][3];
  
  // if connection PARENT window is lost, reload this window AND clear cctgoba !!!
  if ( $formback['fid']!=NULL ) {
	  echo "<script language=\"JavaScript\"> \n";
	  echo "if (window.opener == null) { \n"; 
	  echo "  location.href='".$_SERVER['PHP_SELF']."?t=".$tablename."&cctgoclear=1'; \n";
	  echo "} \n";
	  echo " </script> \n";
  }
}  else {
  $formback=NULL;
}

$viGuiObj->initVars ( $sql, $formback );
if ($error->Got(READONLY))  {
	$errLast   = $error->getLast();
	$error_txt = $errLast->text;
	// $error_id  = $errLast->id;
	$viGuiObj->viewHeadError($sql, 'ERROR', 'Table <B>'.$tablename-'</B>: ARCHIVE-search: Init failed. '.$error_txt);
	exit;
}
$viSubObj->prSetPar1( $sql, $formback );
$tablename_SQLuse = $viGuiObj->get_tablename_SQLuse();

$tmpCheckbox  = $viGuiObj->selectBoxes;
$internOpt    = array("checkbox"=>$tmpCheckbox);
$viewmode_base="";
$rparams = array(
	"viewmode"=>$viewmode,
	"modebase"=>$viewmode_base,
	"view_opt"=>&$view_opt,
	"internOpt"=>$internOpt
	);
$viSubObj->prViewModeSet($rparams);

// page head

$phopt = array( 
	"noBody"=> 1, 
	"css"   => '.xLite { font-size:0.8em; color:gray; }',
	"jsFile"=> 'view.tmpl.js',
	"headIn"=> '<link rel="stylesheet" type="text/css" href="res/css/glob.menu2.css?d=1" />'."\n",
    "logModulExt"=> 't='.$tablename
	);
$title = " List: ".$nicename;
$htmlPgLib = new gHtmlHead();
$htmlPgLib->_PageHead($title, $phopt);


/* start of HTML-Head */

if (!empty($userprefset)) {
  $th0 = key ($userprefset);
  $th1 = current ($userprefset);
  $_SESSION['userGlob']['o.'.$tablename.'.'.$th0] = $th1;
}


$tablename_l=strtolower($tablename);

// load object dependent functions
$xtra_file = 'obj.'.$tablename_l.'.xview.inc';
if ( file_exists($xtra_file) ) {
  require_once ( $xtra_file );   		/* object oriented extra functions */
} 

$xtra_file = $_SESSION['s_sessVars']['AppLabLibDir'].'/obj.'.$tablename_l.'.xview.inc';
if ( file_exists($xtra_file) ) {
  require_once ( $xtra_file );   		/* LAB object oriented extra functions */
}

$viGuiObj->htmlBody( $t_rights );

$searchDoFlag    = 1; /* do the search ? can be denied, if no search criteria is given */

// COUNT elements ...
$fullBaseCnt = NULL; /* number of elements in table: NULL means no INFO */
if ($viGuiObj->doCount) {
  // count only tables with low counts
  $sql_work->query('SELECT COUNT(1) FROM '.$tablename_SQLuse); // SQL: count result sets
  if ($sql_work->ReadRow()) $fullBaseCnt = $sql_work->RowData[0]; // number of all sets DB
}
 

/********************/
/* hide columns ??? */
/********************/
 

$selectstr = "";
$sqlfromXtra="";
$useJoin   = "";


$mother_idM = $_SESSION['s_tabSearchCond'][$tablename]["mothid"]; /* prepare this for colFeaturesGet() */
if ( $searchMothId ) $mother_idM = $searchMothId;
if ( $condclean    ) $mother_idM = "";

$classname=''; // not yet set ...
$exp_raw_desc_id = $viSubObj->getDynaColMother($sql, $mother_idM, $exp_raw_desc_id);
list($selectCols, $useJoin) = $viSubObj->colinfoget($sql, $tablename, $colNames, 
	$viGuiObj->access_id_has, $viGuiObj->class_tab_has, $classname, $exp_raw_desc_id, 1);

// prepare a new SEARCH condition

$selectstr = $viSubObj->sqlColString( $selectCols );
$searchtxt = trim($searchtxt, "\x00\x09\x0A\x0B\x0D\x20\xA0");
$oldSearchInfo = $_SESSION['s_tabSearchCond'][$tablename]["info"];
$searchArr = array( "alias"=>$searchAlias, "cond"=>$tableSCond, "column"=>$searchCol, 
 		            "condclean" => $condclean,	"stext"=>$searchtxt, "op"=>$searchBool );

if ( $searchArr['alias']=='q_search' ) {
    
    $sess_f_settings = $_SESSION['s_formState'][$tablename.'.view.search.f'];
    if (!is_array($sess_f_settings)) $sess_f_settings=array();
    $sess_f_settings['gui']='slim';
    $_SESSION['s_formState'][$tablename.'.view.search.f'] = $sess_f_settings; // save ...
    
    $viSubObj->searchQuick( $sql, trim($_REQUEST['searchidx']) );
    if ($error->Got(READONLY))  {
        echo "<br />\n";
        $error->printAllEasy();
        $error->reset();
    }
    $searchArr=array();
    
} else {
    
    $viSubObj->searchCheckFullFlag( $searchArr );
    $viSubObj->prSearchPrepMain( $sql,  $searchArr, $searchOp_i, $oldSearchInfo);
    if ($error->Got(READONLY))  {
    	echo "<br />\n";
    	$error->printAllEasy();
    	$error->reset();
    }
}

$tmpArch = $_SESSION['s_tabSearchCond'][$tablename]['arch']; // save old value ...
$sopt = NULL;
$sopt["useJoin"] = $useJoin;
$sopt["sMothId"] = $searchMothId;
$sopt["sClass"]  = $searchClass;
if ($searchArr['infoCond']!=NULL)  $sopt['infoCond'] = $searchArr['infoCond'];
if ($searchX!="")  $sopt["xsql"] = $searchX;

list ( $sqlfromXtra, $tableSCondM, $sqlWhereXtra, $sel_info, $classname, $mother_idM, $xSql ) =
       selectGet($sql, $tablename,  $searchArr["condclean"], $searchArr["cond"], $_SESSION['s_tabSearchCond'], $sopt, 
	   			 $searchArr["stext"], $searchArr["column"], $sel,  $searchArr["op"], $searchArr["opBool"]);


// final saving of search condition for table
$_SESSION['s_tabSearchCond'][$tablename] = 
 	array ( "f"=>$sqlfromXtra, "w"=>$tableSCondM, "x"=>$sqlWhereXtra, 
 		"c"=>$classname, "info"=>$sel_info, "mothid"=>$mother_idM, "y"=>$xSql, "arch"=>$tmpArch ); 


if ( $classname ) {
	$viSubObj->getClassParams($sql, $classname);
}
 
 $sqlAfter = full_query_get( $tablename_SQLuse, $sqlfromXtra, $tableSCondM, $sqlWhereXtra, $xSql );
 $tmp_listflag = "o.".$tablename.".listShowAny";
 
 if ($searchArr["condclean"] > 0 ) $_SESSION['s_sessVars'][$tmp_listflag] = ''; /* clear flag */
 if ($listshow)       $_SESSION['s_sessVars'][$tmp_listflag] = $listshow;
 $listshow = $_SESSION['s_sessVars'][$tmp_listflag];

 if ( $fullBaseCnt && ($fullBaseCnt < 1000) ) $listshow = 1;
 if ( ($sel_info=="") && !$listshow ) $searchDoFlag=0;
	
 if (table_is_view($tablename)) {
 	if ( $sel_info=="") $searchDoFlag=0; // never allowed without condition, because can be very time-consuming
	if ( strstr($sel_info, $viGuiObj->pk_name ) === FALSE ) $searchDoFlag=0; /* first PK must be contained in condition */
 }
 
 $selectedBaseCnt = 0;

 if ($searchDoFlag) { 
 	
	$selectedBaseCnt = $viGuiObj->doCondCount($sql_work, $sqlAfter, $sel_info);
     
	/**********************************************************/
	/* Sort criterium */
	/*********************************************************/
	//$sortcritX       = NULL;
	$sortcritXText   = query_sort_org( $tablename );
	$viSubObj->getSortMatrix( $sortcritXText );

	if ( trim($sortcritXText)!="" ) {
		$sqlSort = " ORDER BY ".$sortcritXText; // $sortcritX["col"]." ".$sortcritX["dir"];
	}
	$sqlSortFin = $sqlAfter . $sqlSort;

	$sqls_main = "select $selectstr from ". $sqlSortFin; /* now get data */
 }
 
 // calculate set per view (also for toolbar.php)
 $viewPgCnt = $viGuiObj->setPageSets( $view_opt, $viewPage, $selectedBaseCnt );
 
/* ############################################### */

if ( $viewmode_base!="lean") {
	$toolOpt = array();
	if ( $viGuiObj->is_archdbMode()) {
		$toolOpt['info'] = '<span style="background-color: #EEEE00; margin:4px; padding:3px;">'.
			'DATA from ARCHDB !</span><br />[<a href="'.$_SERVER['PHP_SELF'].'?t='.$tablename.
			'&view_opt[fromArch]=-1">Switch back to NORMAL</a>]'.
			"<br />\n";
	}
	$viGuiObj-> toolbar( $sql, $selectCols, $viSubObj->class_params_show, $fullBaseCnt, $sel_info, $t_rights, $toolOpt );
}

if ( $_SESSION['userGlob']["g.debugLevel"] > 2 ) {
	echo "<B>DEBUG</B> query-string:$sqls_main <BR>";
}

if ($searchArr['news']!=NULL) {
	$viGuiObj->showNews($searchArr['news']);
}
flush(); /* for long searches */
   
$viGuiObj->forwardSelect( $sel, $action );


	
//########################################
//       Start of the main form+table        

if ($viewmode_base != 'lean') {     
  $viGuiObj->startViewForm($sql2, $formback);
}

if ( !$searchDoFlag ) {
	$viGuiObj->msgNoSearch($fullBaseCnt);
	$viGuiObj->pageClose();
	return 0;
}  

if ( ($viewmode!="lean_csv") && ( ($viGuiObj->viewPgSetsShow=="all") && ($viGuiObj->viewPgSets > 500) && !$_REQUEST['listall']) ) { 
    $viGuiObj->msgManyDataShow();
    $viGuiObj->pageClose();
    return 0;
}

      
$sql_work->query($sqls_main);
$sql_work->ReadRow();

$queryResults = $selectedBaseCnt;
if ( !$sql_work->RowData ) $queryResults=0;

list($colNames_show, $foreign_keys) = $viSubObj->headRowPrep();


/***************************/
/*** Start of data table ***/
/***************************/

// get extented column information
$viSubObj->xColsManage( $sql2 );
$viSubObj->prSetPar2( $foreign_keys, $colNames_show);
$viSubObj->prHeadRow( $sql2 );

	
//  START DATA LINES NOW
// reset($sql_work->RowData);           
$cnt=0;
if ( !$queryResults ) { 
	echo "</table>\n";
	echo "</FORM>";
	echo "<br><center>\n";
	
	htmlInfoBox( "NO data sets for this condition!", "", "open", "WARN");
	echo "<font color=gray>Condition:</font><br>\n";
	echo htmlspecialchars($_SESSION['s_tabSearchCond'][$tablename]["info"]);
	htmlInfoBox( "", "", "close" );
	echo "</center><br>\n";
	
} else {
	
	/* read out till current show page */
	if ( $viewPgCnt >= $selectedBaseCnt ) $viewPgCnt = $selectedBaseCnt-1; /* security reason */
	$oristartcnt=$viewPgCnt; 
	while ( $cnt < $viewPgCnt ) {
		$sql_work->ReadRow();
		$cnt++;
	}
	
	/* update $viewPgCnt */
	$viewPgCnt = $viewPgCnt + $viGuiObj->viewPgSets;
	if ( $viewPgCnt >= $selectedBaseCnt ) $viewPgCnt = $selectedBaseCnt;
	
	
	
	$showcnt=0; /* cnt number of sets on display  */
	while( $cnt < $viewPgCnt)  {
		$viSubObj->print_row($sql2, $sql_work->RowData, $cnt);
		$cnt++;
		if ( !$sql_work->ReadRow() ) $cnt = $viewPgCnt; /* extra ordinary end */
		$showcnt++;
	}

	if ( $viewmode!="lean_csv" ) {
		echo "</nobr>\n";
		echo "</table>\n";
	}
}
     
if ( $viewmode_base!="lean") {
	
    if ($queryResults) {
    	$pgOpt=NULL;
		if ($formback['url']==NULL) $pgOpt['showSELECT']=1;
		$viGuiObj->showPagNav( $oristartcnt, $cnt, $selectedBaseCnt, $viewPage, 
			$viewPgCnt, $viGuiObj->viewPgSets, $showcnt, $pgOpt );
    }
    echo "</FORM>\n";
    
	echo "</body>\n";
	echo "</html>";
}
$sql_work->close();
