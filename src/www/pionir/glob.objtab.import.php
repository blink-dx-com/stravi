<?php
/**
 * insert/update data from data-file (CSV) for one table
   - bei CLASS-Attributen: Objekte muessen die Ziel-Class schon haben!
   
 * @todo  - DIESES MODULE muss DRINGEND REFAKTURIERT WERDEN! (Steffen, 2011-04-14)
 * @package glob.objtab.import.php
 * @swreq   UREQ:0001359: g > import object-attributes from CSV-file 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @unittest see data file: _tests\test_data\www\pionir\glob.objtab.import.Update.01.xlsx
 * @param   
     $tablename             : "EXP" or other
     $go                    : 
       	[0], : initial form
       	 1,  : Prepare import (upload file to workdir)
       	 2   : Prepare2
       	 3   : do insert/update
       	 4   : clean temp-files
     $parx :
	   "showOpts"	  : show options [0]
       "action"        : ["update"] | "insert"  
       "class_id"      : import class parameters
       "WIID"          : [number]  if given, use for this WIID 
       "ignoreMissObj" : 0|1  ignore missing object 
       "shFirstObj"    : show first object 
       "infolevel"     : 0,1,2,3,   
       "testredunt"    : [0] | 1 redundancy test ?
       redundancy_cols_in     : string one column name
       "i_shownum"     : [5] or more; number of shown data sets  
       "errcase"       : "continue", 0,5,20,30 ... 
       "motherid"      : if ASSOC elements and no PRIMARY_KEY column given
       "motherneed"    : flag, if motherid is needed [0] | 1
	   "trimDouQout"   : trim DOUBLE QUOTE from cell-values
	   "projid"        : destination project-id; for $parx["action"] = "insert" ; for Business objects
	   "filename"      : filename (without directory [given, if go>1]
	   "colname.type"  : ["RAW"], "HUMAN", "AUTO" 
	   
	   $_FILES['userfile']    : uploaded file (on $go=1)
 *	@version $Header: trunk/src/www/pionir/glob.objtab.import.php 59 2018-11-21 09:04:09Z $
 */
session_start(); 


require_once ('reqnormal.inc'); 
require_once ('sql_query_dyn.inc');
require_once ('db_x_obj.inc');  
require_once ('func_form.inc');
require_once ('subs/glob.objtab.import.inc');  
require_once ("f.objview.inc");	  
 

$error  = & ErrorHandler::get();
$varcol = & Varcols::get();
$FUNCNAME= 'MAIN';

$go        = $_REQUEST['go'];
$parx      = $_REQUEST['parx'];
$tablename = $_REQUEST['tablename'];



$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( );

if ($error->printLast()) htmlFoot(); 
$title       = "Bulk import: import objects from File";
$title_short = "Bulk import";
$titleAdd='???';
if ($parx['action']=='insert') $titleAdd='INSERT objects';
if ($parx['action']=='update') $titleAdd='UPDATE objects';

$infoarr=array();
$infoarr["title_sh"] = $title_short.' : '.$titleAdd;
$infoarr["title"]    = $title;
$infoarr["form_type"]= "list";
$infoarr["help_url"] = "g.Bulk_import.html";
$infoarr["obj_name"] = $tablename;

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

if ($tablename=='') {
    $pagelib->htmlFoot('ERROR', 'Table missing.'); 
}

$isassoc     = 0;
$importMode  = NULL; // "singleInsert", "objupdate"
$importMode  = "singleInsert";
$mothertable = mothertable_get2($tablename);
if ( $mothertable!='') {
    $isassoc = 1;
}
if ($parx["action"]=="update" OR $isassoc ) $importMode = "objupdate";

if ($importMode=="singleInsert") {
    $mainlib = new gObjtabImpIns($tablename, $go, $parx);
} else {
    $mainlib = new gObjtabImpUpd($tablename, $go, $parx);
}

$guilib  = new gObjtabImpGui($tablename);
//$helperLib  = new gObjtabImpSubs();

// $columnInfo["objName2ID"] = 
//	array ( POS => array ("tab"=>..., // remote tab
//                        "column" => ... // main column name
//						)
echo "<ul>";

if ($parx['action']==NULL) {
	$guilib->form1_action( );
	htmlFoot();
}


if (empty($go))  $go = 0;

$doImport    = 1;   // allow import of file, can be denied by test

$mainlib->initialCheck($sql);
$pagelib->chkErrStop();

$mothertable_nice = NULL;

$isassoc     = $mainlib->isassoc;
$mothertable = $mainlib->mothertable;
//$motherImpCol= $mainlib->motherImpCol;
$htmlActNice = $mainlib->htmlActNice;




if ( !$go )  {
  gHtmlMisc::func_hist("glob.objtab.import", $title_short, $_SERVER['PHP_SELF']."?tablename=".$tablename);
}

$goArray = array(
	0=> "Input information",
	1=> "Prepare import: (check file)",
	2=> "Prepare import II",
	3=> $htmlActNice." now ",
	4=> "Finish: Clean temporary files ",
);
$extraText = "[<a href=\"".$_SERVER['PHP_SELF']."?tablename=".$tablename."&parx[action]=".$parx['action']."\">Start again</a>]";

$formWizardLib = new FormPageC();
$formWizardLib->init($goArray, $extraText);


// show LINK ...
$url_arr = array(
    "colCoNa"  => 1,
    "add_nice_head"=>1,
    "pureids"  => 1,
    "stop_cnt" => 1
);
$url_str = NULL;
foreach($url_arr as $key=>$val) {
    $url_str .= '&view_opt[' . $key .']='.$val;
}
echo ' [<a href="glob.objtab.exp.php?t=' . $tablename . '&format=xlsxml' . $url_str. '">Create import template</a>]<br><br>'."\n";

$formWizardLib->goInfo($go); 

echo "<P>";

if ( $go == 4 ) { 

  echo "<br>... remove temporary file ... <br>";
  $mainlib->removeFile();
  echo "... done ...";
  $pagelib->htmlFoot(); 
  
}

// collect information ...

$classInfoArr = array();  
 
if ( $parx["class_id"] ) {
    $classInfoArr = $mainlib->_objClassParamsGet( $parx["class_id"] );
    if ( $error->got(READONLY) ) {  
        $error->set('analyze Class', 5, 'Failed!');
        $pagelib->chkErrStop(); // STOP on error  
    } 
    if ($parx["infolevel"]  > 1 && $parx["class_id"] ) { 
        $tmpvals = print_r($classInfoArr, TRUE);
        $mainlib->store_info("Possible extra_class columns:", "<pre>".$tmpvals."</pre>");
    }    
    $mainlib->store_info("Class", $classInfoArr[1] . " [ID:".$parx["class_id"]."] ");
}                                                        
 
if ( $parx["infolevel"] ) $mainlib->store_info("Info-level:", $parx["infolevel"] ); 


if ( !$go  ) { 
    
    

	$foptions = array();
	if ($mainlib->isbo) {
		$foptions['showProjID']=1;
	}
	$foptions['redund_cols'] = $guilib->get_redund_column_arr($sql);
	
	
	$param_ini = array();
	$guilib->form1( $sql, $mainlib->parx, $go, $importMode, $param_ini, $foptions );
    echo "<br>\n";
    if ($parx['action']=='insert') $guilib->help_insert();
    if ($parx['action']=='update') $guilib->help_update();
    
    htmlFoot();
}

// $ go > 0
	
$formok     = 1;

///// FILE !!!!

$mainlib->setParxVal( "doUpload", 0);  // do not do upload anymore
$mainlib->store_info("Action"     , $parx["action"]  );
$mainlib->store_info("File name"  , $mainlib->parx["filename"]  );
$mainlib->store_info("Character-Encoding", $mainlib->encoding_key );
$mainlib->store_info("File-Type", $mainlib->get_file_type() );

$tmpval = $parx["errcase"]; 
if ($tmpval!="continue") {
     $tmpval = "Stop after ".$tmpval. " error(s)";
}    
$mainlib->store_info("On_error", $tmpval );
$mainlib->store_info("Show lines", $parx["i_shownum"] );
if ( $parx["trimDouQout"]>0 ) $mainlib->store_info("Trim double quotes", 'YES' );

$mothertable_nice = tablename_nice2($mothertable);
if  ($isassoc) $mainlib->store_info ("Mother-table", $mothertable_nice); 
if  ($parx["projid"]) {
	
	$objLinkLib = new fObjViewC();
	$temphtml = $objLinkLib->bo_display( $sql, 'PROJ', $parx["projid"] );
	$mainlib->store_info ("Destination project", $temphtml); 
}

if ( $go>=2 AND $parx["motherneed"] ) {
	$mainlib->check_motherneed($sql, $parx);
    $pagelib->chkErrStop(); // STOP on error      
}

if ( $importMode == "singleInsert" ) {

    if ( isset($parx["testredunt"]) )  { // and $parx["infolevel"]>0
        $tmpval = "NO";
        if ( $parx["testredunt"] ) $tmpval = "YES";
        $mainlib->store_info("Redundancy check", $tmpval);  
        
        // check parameters
        if ($parx['redundancy_cols_in']==NULL) {
            $error->set( $FUNCNAME, 1, 'Input Error: redundancy column missing.' );
            $pagelib->chkErrStop();  
        }
        $col_red_feat = colFeaturesGet2($tablename, $parx['redundancy_cols_in']);
        $col_red_nice = $col_red_feat['NICE_NAME'];
        $mainlib->store_info("Redundancy column", $col_red_nice);  
     }
      
} else {  
    
    if ( $go>=2 ) {
        if ($parx["WIID"]) {
            $mainlib->store_info("WIID", $parx["WIID"]. " (ID of the origin-database, transforms PRIMARY ID's)");
        }
    
        $tmpinfo = "break whole update";
        if ( $parx["ignoreMissObj"] ) $tmpinfo = "continue with next object";
        $mainlib->store_info("Not_in_database", $tmpinfo, "what to do, if the mother-object, is not in the database" );
    } 
}   

if ( isset($parx["toclip"]) and $parx["infolevel"]>0 )  {                    
    $tmpinfo="no";
    if ($parx["toclip"]) $tmpinfo="yes"; 
    $mainlib->store_info("To clipboard?", $tmpinfo, "Put objects to clipboard");
}

$mainlib->showInfoTable();
 
//
// check header !!!
//
$hoption = array();
$hoption["infolevel"] = $parx["infolevel"];
$hoption["motherid"]  = $parx["motherid"];

// $FH = fopen($mainlib->userfile, 'r');

if ($go<=1) $goparse=0;
else        $goparse=1;


$head_lib   = new gObjtabImpHead($tablename, $mainlib->userfile, $mainlib->file_type);
$headerInfo = $head_lib->parseHead( $sql, $parx,  $classInfoArr, $goparse, $hoption );                
$pagelib->chkErrStop();

							  
if ( $parx["infolevel"] >= 3 ) {
	echo "DEBUG HEADER_INFO:<pre>";
	print_r($headerInfo);
	echo "</pre>";
}
$colHasClass = sizeof($headerInfo["colClass_arr"]); 
$headError = $headerInfo["headError"];
$header_arr= $headerInfo["header_arr"];
if ( $headError<0 ) {
    echo "<br>"; 
    $tmptxt= "Header:<pre>".print_r($header_arr, TRUE)."</pre>";
    htmlErrorBox("Error", "at parsing the header.", $tmptxt);       
    $formok = -1;
}          

if ( $colHasClass ) {
    if ( $parx["infolevel"] > 1 ) {
        $mainlib->_debug_out ("found extra-class-columns in file header");
        //echo "<pre>";
        //print_r($colClass_arr); 
        //echo "</pre>";
    }              
}  

//$parxerr=NULL;
if ( $go >= 2 ) {
   if ( $importMode == "objupdate" AND $headerInfo["primcol_pos"]=="" ) {
        // test, if mother exists
        if ($parx["motherid"]=="") {    // error =>
            //$parxerr["motherid"] = "Please give a mother-object-id";
            $go     = 1;
            $formok = -1;
        }     
   }
}           


if ( $go == 1 ) {
 
	$mainlib->setParxVal("motherneed",0);  // hidden parameter
    if ( $importMode == "objupdate" AND $headerInfo["primcol_pos"]  == "") {
        $mainlib->setParxVal("motherneed",1); 
    }
    $tmpfile=NULL; // FUTURE: use may be later 
    $mainlib->form2( $go, $formok, $importMode, $tmpfile );
    echo "<br>\n";
    if ( $formok <0 ) {
        htmlFoot("Stopped", "Actions stopped due to previous error(s)");                               
    }
      
}   


if ( $go == 2 ) {
  
    $initarr   = NULL;
    $initarr["action"]      = $_SERVER['PHP_SELF'];
    $initarr["title"]       = "Final choise"; 
    $initarr["submittitle"] = "$htmlActNice now!";
    $initarr["tabwidth"]    = "400px";
     
    $hiddenarr = NULL;
    $hiddenarr["tablename"]     = $tablename;   
    
    
    
    if ( sizeof($parx) ) {
        foreach( $parx as $idx=>$valx) {
              $hiddenarr['parx['.$idx.']']    = $valx;
        }
        
    }    
      
    $formobj = new formc( $initarr, $hiddenarr, $go );   
             
    if ( $formok >=0 ) $showSubmit = TRUE;
    else  $showSubmit = FALSE;        
    
    $formobj->close( $showSubmit );
    echo "<br>";
    
    if ( $formok <0 ) {
        
        htmlFoot("Stopped", "Actions stopped due to previous error(s)");                               
    }
      
} 

if ( $go == 3 ) {


    $initarr   = NULL;
    $initarr["action"]      = $_SERVER['PHP_SELF'];
    $initarr["title"]       = "Finishing"; 
    $initarr["submittitle"] = "Finish - clean temp files";
    $initarr["tabwidth"]    = "AUTO";
     
    $hiddenarr = NULL;
    $hiddenarr["tablename"]     = $tablename;
    
    if ( sizeof($parx) ) {
        foreach( $parx as $idx=>$valx) {
              $hiddenarr['parx['.$idx.']']    = $valx;
        }
        reset($parx); 
    }
    
    if ( $importMode == "singleInsert" ) $initarr["goBack"] = 1;    
      
    $formobj = new formc( $initarr, $hiddenarr, $go );   
        
    $formobj->close( TRUE );
    echo "<br>\n";
}  



/**
 * @var array $option INSERT_OPTS_STRUCT
 *  "WIID"       
    "shFirstObj"    
    "ignoreMissObj" 
    "testredunt"    
    "i_shownum"        
    "errcase"       
    "motherid"      
    "toclip"   
 */
$option = NULL;
$option["WIID"]          = $parx["WIID"];
$option["shFirstObj"]    = $parx["shFirstObj"];    
$option["ignoreMissObj"] = $parx["ignoreMissObj"];
$option["testredunt"]    = $parx["testredunt"]; 
$option["redundancy_cols"] = array($parx["redundancy_cols_in"]); 
$option["i_shownum"]     = $parx["i_shownum"];         
$option["errcase"]       = $parx["errcase"];
$option["motherid"]      = $parx["motherid"];
$option["toclip"]        = $parx["toclip"]; 

$paramx = NULL; 
$paramx["isassoc"]      = $isassoc    ; 
$paramx["mothertable"]  = $mothertable;  

if ( $doImport ) {
	
    //$mainlib->openFile($mainlib->userfile, $file_type);
    $retval = $mainlib->import_do( $sql, $sql2, $go, $classInfoArr, $option, $paramx, $headerInfo );
    
    
    if ( ($retval<0) || $error->got(READONLY) ) { 
        echo "<br>";
        $error->printAll();    
    }
    
}


htmlFoot("<hr>");

