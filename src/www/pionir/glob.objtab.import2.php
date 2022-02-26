<?php
/**
 * import slim CSV data
 * - only for ADMIN
 * @package  glob.objtab.import2.php
 * @author   Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $go
 * @param $tablename	
 * @param $parx	  
 * 	  $parx["type"]  : type of import
 * 		insert : normal insert
 *      move   : move assoc_elements: columns: PK1_SRC, POS_SRC, PK2_DEST, POS_DEST
	  $parx["datafile"]
	  $parx['infolevel']
	  FUTURE: $parx["action"]        : ["move"] | "insert"
 * @param 	 $go : 0,1,2,
 * @global  $_SESSION["userGlob"]["g.debugKey"] : 'noInsert' : does NOT insert values
 
 */ 
session_start(); 

require_once ("reqnormal.inc");
require_once ("func_form.inc");
require_once ('insert.inc');
require_once ("f.directorySub.inc");
require_once ("class.filex.inc");
require_once ("f.upload.inc");
require_once ('f.workdir.inc');
require_once ("f.visuTables.inc");
require_once ('f.msgboxes.inc');
require_once ('f.progressBar.inc');
require_once ('f.fileana.inc');

class gObjImport2One {

function __initLib( $headerPos, $doInsert, $insertOpt, $debug) {
	$this->headerPos = $headerPos;
	$this->doInsert=$doInsert;
	
	$this->insertOpt=$insertOpt;
	$this->debug=$debug;
}

function __setTable($tablename) {
	$this->tablename = $tablename;
}

function getExpectCols() {}

// can throw errors
function initCheck() {}

}

/**
 * one object, several ASSOC_ELEMs
 * 
 */
class gObjImport2One_move extends gObjImport2One {

	function getExpectCols() {
		
		$pkArr = primary_keys_get2($this->tablename);
		$this->expectArr   = array(
			'SRC:'.$pkArr[0]=>2, 
			'SRC:'.$pkArr[1]=>2, 
			'DST:'.$pkArr[0]=>2, 
			'DST:'.$pkArr[1]=>2
			);
		
		return $this->expectArr;
	}

	// can throw errors
	function initCheck() {
		global $error;
		$FUNCNAME= 'initCheck';
		$pkArr = primary_keys_get2($this->tablename);
		
		$this->realColsArr = array( $pkArr[0], $pkArr[1], $pkArr[0], $pkArr[1] );
		
		
		foreach( $this->headerPos as $dbcol=>$pos) {
			$expectCol = key($this->expectArr);
			next($this->expectArr);
			
			if ($expectCol!=$dbcol) {
				$error->set( $FUNCNAME, 1, 'Column "'.$dbcol.'" is not valid, expected name: "'.$expectCol.'"' );
				return;
			}
		}
	
		reset ($this->expectArr);
		
	}
	
	function _oneColGet($pos, $dataarr) {
		$outstr =  $this->realColsArr[$pos]."=".$dataarr[$pos];
		return $outstr;
	}
	
	/**
	 * import one line: MOVE element1,POS1 to element2,POS
	 * @param object $sqlo
	 * @param  $dataarr
	 * @param $cnt
	 * @return -
	 */
	function importLine( &$sqlo, $dataarr, $cnt ) {
		global $error;
		$FUNCNAME= 'importLine';
		
		$headerPos=$this->headerPos;
		$tablename=$this->tablename;
		
		$updateStr = $this->_oneColGet(2, $dataarr).", ".$this->_oneColGet(3, $dataarr).
					 " where ".$this->_oneColGet(0, $dataarr)." and ".$this->_oneColGet(1, $dataarr);

		if ( $this->doInsert ) {
			$retval = $sqlo->updatex($tablename, $updateStr); // only allowed for admin ...
		}
		else {
			$retval = 1;
		}
		if ( $this->debug>0 ) {
			echo ($cnt +1).". DEBUG_data: ".$updateStr. "<br>\n";
		}
		
		if ($retval<=0) {
			$error->set( $FUNCNAME, 1, 'update failed of object: '.  htmlspecialchars( glob_array2String( $argu)) );
			return;
		}
	}
	
	function help () {
	
		?>
		<ul>
			<li>update data columns in database, special Primary Key Move</li>
			<li>this tool imports a CSV file of assocciated tables (BO_ASSOC)</li>
			<li>each row contains one data set</li>
			<li>the data set will be UPDATED to the destination table</li>
			<li>update, move primary keys: PK1=SRC_1, PK2=SRC_2 ==> PK1=DEST_1, PK2=DEST_2</li>
			<li>security: the tool does not check access rights, does NOT touch the mother object</li>
			<li>only usable by an admin-user</li>
			
		</ul>
	<br><br>
		<b>CSV-File format</b>
		<ul>
			<li>TAB-separated values</li>
			<li>first line contain CODE-column-names</li>
			<li>next lines contain the data</li>
		</ul>
		<br><br>
		<b>Example for the CSV-file for CONCRETE_PROTO_STEP</b>
		<br><br>
	<TABLE CELLSPACING=0 COLS=6 BORDER=3>
		<TBODY>
			<TR>
				<TD WIDTH=86 HEIGHT=17 ALIGN=LEFT>CART_BATCH_ID</TD>
				<TD WIDTH=86 ALIGN=LEFT>POS</TD>
				<TD WIDTH=86 ALIGN=LEFT>CART_BATCH_ID</TD>
				<TD WIDTH=86 ALIGN=LEFT>POS</TD>
				
			</TR>
			<TR>
				<TD HEIGHT=17 ALIGN=RIGHT SDVAL="840" >840</TD>
				<TD ALIGN=RIGHT SDVAL="340" >5</TD>
				<TD ALIGN=RIGHT SDVAL="28" >890</TD>
				<TD ALIGN=RIGHT SDVAL="3940" >5</TD>
				
			</TR>
			<TR>
				<TD HEIGHT=17 ALIGN=RIGHT SDVAL="940" >840</TD>
				<TD ALIGN=RIGHT SDVAL="340" >6</TD>
				<TD ALIGN=RIGHT SDVAL="28" >890</TD>
				<TD ALIGN=RIGHT SDVAL="3940" >7</TD>
				
			</TR>
			
		</TBODY>
	</TABLE>
		<?
		
	} 

}

/**
 * one object, several ASSOC_ELEMs
 * 
 */
class gObjImport2One_insert extends gObjImport2One {

function getExpectCols() {
	
	$tablename=$this->tablename;
	$columns = columns_get_pos($tablename); 
	$this->colsExpect = NULL;
	foreach( $columns as $dummy=>$key) {
		$this->colsExpect[$key] = array('col.name'=>$key, 'req'=>1 );
	}
	reset ($columns); 
	
	$pks = primary_keys_get2($tablename);
	foreach( $pks as $dummy=>$key) {
	    $this->colsExpect[$key] = array('col.name'=>$key, 'req'=>2 );
	}
	reset ($pks); 
	return $this->colsExpect;
}

/**
 * import one line
 * @param object $sqlo
 * @param  $dataarr
 * @param $cnt
 * @return -
 */
function importLine( &$sqlo, $dataarr, $cnt ) {
	global $error;
	$FUNCNAME= 'importLine';
	
	$argu=NULL;
	reset ($this->headerPos);
	foreach( $this->headerPos as $dbcol=>$pos) {
		$argu[$dbcol]  = trim ( $dataarr[ $this->headerPos[$dbcol] ], '"');
	}
	reset ($this->headerPos); 
	
	if ( $this->doInsert ) insert_row_s( $sqlo, $this->tablename, $argu, $this->insertOpt); 
	
	if ( $this->debug>0 ) {
		echo ($cnt +1).". DEBUG: ".glob_array2String( $argu). "<br>\n";
	}
	
	if ( $error->Got(READONLY) ) {
		$error->set( $FUNCNAME, 1, 'insert failed of object: '.  htmlspecialchars( glob_array2String( $argu)) );
		return;
	}
}

function help () {
	
	?>
	<ul>
		<li>inserts data columns in database</li>
		<li>this tool imports a CSV file of NON business object data (SYS and BO_ASSOC)</li>
		<li>each row contains one data set</li>
		<li>the data set will be INSERTED to the destination table</li>
		<li>security: the tool does not check access rights, does NOT touch the mother object</li>
		<li>only usable by an admin-user</li>
		<li>Date-format: YYYY-MM-DD</li>
	</ul>
<br><br>
	<b>CSV-File format</b>
	<ul>
		<li>TAB-separated values</li>
		<li>first line contain CODE-column-names</li>
		<li>next lines contain the data</li>
	</ul>
	<br><br>
	<b>Example for the CSV-file for CONCRETE_PROTO_STEP</b>
	<br><br>
<TABLE CELLSPACING=0 COLS=6 BORDER=3>
	<TBODY>
		<TR>
			<TD WIDTH=86 HEIGHT=17 ALIGN=LEFT>CONCRETE_PROTO_ID</TD>
			<TD WIDTH=86 ALIGN=LEFT>ABSTRACT_PROTO_ID</TD>
			<TD WIDTH=86 ALIGN=LEFT>STEP_NR</TD>
			<TD WIDTH=86 ALIGN=LEFT>CONCRETE_SUBST_ID</TD>
			<TD WIDTH=77 ALIGN=LEFT>NOT_DONE</TD>
			<TD WIDTH=86 ALIGN=LEFT>NOTES</TD>
		</TR>
		<TR>
			<TD HEIGHT=17 ALIGN=RIGHT SDVAL="840" >840</TD>
			<TD ALIGN=RIGHT SDVAL="340" >340</TD>
			<TD ALIGN=RIGHT SDVAL="28" >28</TD>
			<TD ALIGN=RIGHT SDVAL="3940" >3940</TD>
			<TD ALIGN=RIGHT SDVAL="0" >0</TD>
			<TD ALIGN=LEFT><BR></TD>
		</TR>
		<TR>
			<TD HEIGHT=17 ALIGN=RIGHT SDVAL="940" >940</TD>
			<TD ALIGN=RIGHT SDVAL="340" >340</TD>
			<TD ALIGN=RIGHT SDVAL="28" >28</TD>
			<TD ALIGN=RIGHT SDVAL="3940" >3940</TD>
			<TD ALIGN=RIGHT SDVAL="0" >0</TD>
			<TD ALIGN=LEFT>"Hello"</TD>
		</TR>
		<TR>
			<TD HEIGHT=17 ALIGN=RIGHT SDVAL="1280" >1280</TD>
			<TD ALIGN=RIGHT SDVAL="340" >340</TD>
			<TD ALIGN=RIGHT SDVAL="28" >28</TD>
			<TD ALIGN=RIGHT SDVAL="3940" >3940</TD>
			<TD ALIGN=RIGHT SDVAL="0" >0</TD>
			<TD ALIGN=LEFT>"Note2 yuhhu"</TD>
		</TR>
	</TBODY>
</TABLE>
	<?
	
} 

}


/**
 * the IMPORT library for a file
 * 
 */
class gObjImp2C {

var $maxDataLines;

function __construct($type) {
	$this->impOneLib=NULL;
	if ($type=='insert') {
		$this->impOneLib = new gObjImport2One_insert();
	}
	if ($type=='move') {
		$this->impOneLib = new gObjImport2One_move();
	}
	$this->uploadLib = new fFileanaC();
	
	$this->insertOpt = NULL;
	$this->debug = NULL;
	$this->doInsert = 1;
	if ( $_SESSION["userGlob"]["g.debugKey"]=='noInsert' ) {
		echo "<B>INFO:</B> DEBUG-mode: g.debugKey: noInsert<br>\n";
		$this->doInsert = 0;
	}
	if ( $_SESSION["userGlob"]["g.debugLevel"]>2 ) {
		echo "<B>INFO:</B> DEBUG-mode: show insert data<br>\n";
		$this->debug = 1;
	}
}

function init( &$sqlo, $tablename, $go, $datafileFull, $parx) {
	global $error;
	$FUNCNAME= 'init';
	
	$this->tablename   = $tablename;
	$this->go   = $go;
	$this->parx = $parx;
	$this->datafileFull = $datafileFull;
	
	$this->impOneLib->__setTable($tablename);
	$this->colsExpect = $this->impOneLib->getExpectCols();
	
	$pks = primary_keys_get2($tablename);
	$this->primKeys = $pks;
	
	$this->maxDataLines = $this->uploadLib->countDataLines( $datafileFull );
	if ( $this->maxDataLines<=0 ) {
		$error->set( $FUNCNAME, 2, 'No data lines found.');
		return;
	}
	
	$this->uploadLib->setDelimiter("\t");
	
    $this->uploadLib->setHeaderParms($this->colsExpect);
	$FH = $this->uploadLib->openFile( $sqlo, $datafileFull );
    
	if ( $error->Got(READONLY) ) {
		$error->set( $FUNCNAME, 1, "open File analysis failed. '".$datafileFull."'");
		return;
	}
	$this->headerPos = $this->uploadLib->headerPos;
	
	reset ($this->headerPos);
	$types=NULL;
	foreach( $this->headerPos as $dbcol=>$pos) {
		if (substr($dbcol,-5)=='_DATE') {
			$types[]=array($dbcol=>'DATE1');
		}
	}
	reset ($this->headerPos);
	if (sizeof($types)>0) {
		$this->insertOpt = array('types'=>$types);
	}
	
	$this->impOneLib->__initLib($this->headerPos, $this->doInsert, $this->insertOpt, $this->debug);
	$this->impOneLib->initCheck();
	
	return ($FH);
}

function getNumLines() {
	return ($this->maxDataLines);
}

function getHeader() {
	return ($this->headerPos);
}


/**
 * analyse header
 * 
 */
function anaHeader( &$sqlo ) {
}

function _valByDB_col( &$dataarr, $dbcol ) {
    return ($dataarr[ $this->headerPos[$dbcol] ]);
}


function doImport( &$sqlo, &$FH ) {
	global $error;
	$FUNCNAME= 'doImport';
	
	$LINE_LENGTH=32000;
	
	$flushLib = new fProgressBar( ); 
	$prgopt['objname']='rows';
	$prgopt['maxnum']= $this->maxDataLines;
	$flushLib->shoPgroBar($prgopt);
	
	$this->linecount = 0;
    $posPK1 = $this->headerPos[$this->primKeys[0]];
    $cnt = 0;
	$this->sum=NULL;
	
	
    while( !feof ( $FH ) ) { 
    
	    $tmperr = "";
		$line   = fgets($FH, $LINE_LENGTH);
		
		if ( trim($line)==NULL) {
			return;
		}
		
		$dataarr = $this->uploadLib->getLineArr( $line );
	
		$this->impOneLib->importLine($sqlo, $dataarr, $cnt);
		if ( $error->Got(READONLY) ) {
			
			echo '<b>ERROR:</b> Line:'. ($cnt+1).'<br>'."\n";
			$error->printall();
			$error->reset();
			$this->sum['err']++;
			
			if ($this->sum['err']>5) {
				echo 'Too many errors... BREAK<br>'."\n";
				break;
			}
		}
	
		$flushLib->alivePoint($cnt);
		$cnt++;
	}
	$flushLib->alivePoint($cnt,1); // finish
	$this->sum['cnt'] = $cnt;
	fclose($FH);
	
	glob_printr( $this->sum, "SUM" );
}

function help() {
	$this->impOneLib->help();
}

}

class gObjtabImp2gui {

function __construct( &$sqlo, $tablename, $parx, $go, $scriptid ) {
	$this->scriptid  = $scriptid;
	$this->tablename = $tablename;
	$this->go   = $go;

	if ( $parx["infolevel"]==NULL ) {
		$parx["infolevel"]=1; // default
	}
	$this->parx = $parx;
	$this->workFileName = 'datafile.dat';
	$this->datafileFull = NULL;
	
	$this->dataImpLib = new gObjImp2C($parx['type']);
	$this->_infolevels = array( 0=>'silent', 1=>'[only errors]', 2=>'warnings+errors', 3=>'all objects' );
}

function GoInfo($go, $coltxt=NULL) {
	
	$goArray   = array( "0"=>"Upload file", 1=>"Prepare Import", 2=>"Do Import" );
	$extratext = '[<a href="'.$_SERVER['PHP_SELF'].'?id='.$this->proj_id.'">Start again</a>]';
	
	$formPageLib = new FormPageC();
	$formPageLib->init( $goArray, $extratext );
	$formPageLib->goInfo( $go ); 

}

function showParams() {
	$parx = $this->parx;

	$tabobj = new visufuncs();
	$dataArr= NULL;

	$lev = $parx['infolevel'];
	$tmpHeader = $this->dataImpLib->getHeader() ;
	$dataArr[] = array( 'InfoLevel:', $this->_infolevels[$lev].' ['.$lev.']' );
	$dataArr[] = array( 'DataLines:',  $this->dataImpLib->getNumLines() );
	$dataArr[] = array( 'Header:',  glob_array2String( $tmpHeader )  );
	$headOpt = array( "title" => "Parameters", "headNoShow" =>1);
	$headx   = array ("Key", "Val");
	$tabobj->table_out2($headx, $dataArr,  $headOpt);
	echo "<br>";
}


function form1(&$sqlo) {
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Upload data file";
	$initarr["submittitle"] = "Next &gt;&gt;";
	$initarr["tabwidth"]    = "AUTO";
	$initarr["ENCTYPE"]		= "multipart/form-data";

	$hiddenarr = NULL;
	$hiddenarr["tablename"]     = $this->tablename;
	$hiddenarr["parx[type]"]	= $this->parx["type"];

	$formobj = new formc($initarr, $hiddenarr, 0);

	$fieldx = array ( 
		"namex" => TRUE,
		"title" => "Data file", 
		"name"  => "datafile",
		"object"=> "file",
		"val"   => $parx["datafile"], 
		"notes" => "the CSV data file"
		 );
	$formobj->fieldOut( $fieldx );

	$fieldx = array ( 
		"title" => "InfoLevel", 
		"name"  => "infolevel",
		"object"=> "select",
		"inits" => $this->_infolevels,
		"val"   => $this->parx["infolevel"], 
		"notes" => "infolevel"
		 );
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );

	echo "<br>";
	$this->help();
}

function form2(&$sqlo) {
	
	global $error;
	$FUNCNAME='form2';

	$uploadObj = new uploadC();
	$this->datafileFull  =  $uploadObj->mvUpload2Tmp( 
		$this->scriptid, $this->workFileName, $_FILES['datafile']['tmp_name'], $_FILES['datafile']['name'],
		 $_FILES['datafile']['size'] );
	$this->workDirFull = $uploadObj->getWorkDir();

	echo "<br>\n";
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Prepare creation";
	$initarr["submittitle"] = "Submit";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["tablename"]     = $this->tablename;
	$hiddenarr["parx[infolevel]"] = $this->parx["infolevel"];
	$hiddenarr["parx[type]"]	= $this->parx["type"];
	$formobj = new formc($initarr, $hiddenarr, 1);
	$formobj->close( TRUE );
	
	$FH = $this->dataImpLib->init($sqlo, $this->tablename, $this->go, $this->datafileFull, $this->parx);
	$this->showParams();
	if ($error->Got(READONLY))  {
		return;
	}
	
	$this->dataImpLib->anaHeader( $sqlo );
	
}

function form3(&$sqlo) {
	
	global $error;
	$FUNCNAME='form3';
	
	$workdirObj = new workDir();
	$this->workDirFull   = $workdirObj->getWorkDir ( $this->scriptid, 1 );
	if ($error->Got(READONLY))  {
		return;
	}
	$this->datafileFull =  $this->workDirFull . DIRECTORY_SEPARATOR . $this->workFileName;
	
	$FH = $this->dataImpLib->init($sqlo, $this->tablename, $this->go, $this->datafileFull, $this->parx);
	$this->showParams();
	if ($error->Got(READONLY))  {
		return;
	}
	$this->dataImpLib->doImport( $sqlo, $FH );
}

function help () {
	htmlInfoBox( "Short help", "", "open", "HELP" );
	$this->dataImpLib->help();
	htmlInfoBox( "", "", "close" );
} 

}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$go		    = $_REQUEST["go"];
$tablename	= $_REQUEST["tablename"];
$parx	    = $_REQUEST["parx"];

$title		= "CSV Data Importer II - ".$parx['type'];
$flushLib = new fProgressBar( ); 

$infoarr			 = NULL;
$infoarr["scriptID"] = "glob.objtab.import.php";
$infoarr["title"]    = $title;
$infoarr["title_sh"] = 'Importer II';
$infoarr["form_type"]= "list"; // "tool", "list"
$infoarr["obj_name"] = $tablename;
$infoarr['design']   = 'norm';
$infoarr['css'] 			= $flushLib->getCss();
$infoarr['javascript'] = $flushLib->getJS(); 

$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo, $infoarr);
$pagelib->chkErrStop();
if ($tablename==NULL) {
	$pagelib->htmlFoot('ERROR', 'No table given');
}
if ( $parx["type"]==NULL ) {
	$pagelib->htmlFoot('ERROR', 'Import-type missing.');
}

if ( !glob_isAdmin() ) {
	$pagelib->htmlFoot("ERROR",   
     "Only root can execute this!",
     "For security reason it is not allowed for common users" );
}

$mainlib = new gObjtabImp2gui( $sqlo, $tablename, $_REQUEST['parx'], $go, $infoarr["scriptID"]  );

$mainlib->GoInfo($go);

if ( !$go ) {
	$mainlib->form1($sqlo);
	$pagelib->chkErrStop();
	$pagelib->htmlFoot();
}

if ( $go==1 ) {
	$mainlib->form2($sqlo);
	$pagelib->chkErrStop();
	$pagelib->htmlFoot();
}

if ( $go==2 ) {
	$mainlib->form3($sqlo);
	$pagelib->chkErrStop();

	echo "<br>";
	cMsgbox::showBox("ok", "O.K."); 

	$pagelib->htmlFoot();
}


$pagelib->htmlFoot();
