<?php
/**
 * Partisan-Frankenstein
 * - sub functions
 * - generate new database structure SQL-strings for PARTISAN
 * $Header: trunk/src/www/pionir/rootsubs/db_transform/f.dbStructGui.inc 59 2018-11-21 09:04:09Z $
 * @package f.dbStructGui.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @var  $_SESSION['s_formState']["f.dbStructCrea"] = array (
				"cct_table"  => array (),
				"cct_column" => array( $col=>array() )
				"settings" => array()
				   sql_exp_type
				   db_type: ORACLE, POSTGRES
			);
 */
require_once ("func_form.inc");
require_once ("visufuncs.inc");
require_once ("f.dbStructCrea.inc");
	
class fCbStructGui {
    
    const SQL_EXP_TYPE_INS='insarr';
    const SQL_EXP_TYPE_pure='pure';
	
	var $tabStruc; // main table structure
	var $go; /** 
			defines the action
			"0"=>"Define type",
			1=>"Give columns", 
			2=>"Create structure", 
			3=>"Edit a column",
			4=>"Delete a column",
			5=>"Create SQL-commands",
			6=>"Remove all structures",
			7=> "drop table"
			*/

function __construct($scriptname) {
	
	
	$this->scriptname = $scriptname;
	$this->tabStruc   = $_SESSION['s_formState'][$scriptname];
	$this->settings   = $_SESSION['s_formState'][$scriptname]['settings'];
	
	$this->structLib  = new fDbStructCrea();
	$this->cct_cols   = &$this->structLib->cct_cols;
	
	
	
	$this->structLib->init($this->tabStruc, $this->settings);
	
}

function _info ($text, $notes=NULL ) {
    // FUNCTION: print out info text
    if ($notes!="")  $notes = " &nbsp;<I>".$notes."</I>";
    echo "<font color=gray>INFO</font> <B>".$text."</B>".$notes."<br>\n";

}


function _getTabContent( &$sql, $tablename ) {
	$sqls = "select ".$tablename."_ID, NAME from ".$tablename." order by NAME";
	$sql->query($sqls);
	while ( $sql->ReadRow() ) {
		$retarr[$sql->RowData[0]] = $sql->RowData[1];
	}
	return ($retarr);
}

function _setColumn($parx) {

	if ($parx['MOST_IMP_COL']==NULL)  $parx['MOST_IMP_COL']=0;

	$this->tabStruc["cct_column"][$parx["COLUMN_NAME"]] = $parx;
	
}

function init($go, $parx, $subaction) {
	$this->go=$go;
	$this->parx = $parx;
	$this->subaction = $subaction;
	
	$goArray   = array( 
		"0"=>"Define type",
		1=>"Give columns", 
		2=>"Create structure", 
		3=>"Edit a column",
		4=>"Delete a column",
		5=>"Create SQL-commands",
		6=>"Remove all structures",
		7=>"drop table",
		8=>"Reset columns",
		9=>"Export a Frankenstein-file",
		10=>"Import a Frankenstein-file",
	    12=>"Frankenstein-Settings",
		);
	$extratext = "[<a href=\"".$_SERVER['PHP_SELF']."?go=3\">Home</a>] [<a href=\"".$_SERVER['PHP_SELF']."?go=0\">Start again</a>] go:".$go;
	
	$formPageLib = new FormPageC();
	$formPageLib->init( $goArray, $extratext );
	$formPageLib->goInfo( $go ); 
	echo "<br>";
}

/**
 * "cct_table"  => array (),
	"cct_column" => 
 */
function saveStruct() {
	
    $_SESSION['s_formState']["f.dbStructCrea"]["cct_table"]  = $this->tabStruc["cct_table"];
    $_SESSION['s_formState']["f.dbStructCrea"]["cct_column"] = $this->tabStruc["cct_column"];
   
}

private function saveSettings() {
    $_SESSION['s_formState']["f.dbStructCrea"]['settings']   = $this->settings;
}

function resortCols($posset=0) {
	if (!$this->tabStruc["cct_column"]) {
		return;
	}
	$colStrucPoi = &$this->tabStruc["cct_column"];
	$posarr=NULL;
	foreach( $colStrucPoi as $db_col=>$colarr) {
		$posarr[ $colarr["POS"] ] = $db_col;
	}
	reset ( $colStrucPoi );
	
	ksort($posarr);
	reset($posarr);
	
	$newpos = 1;
	$cnt	= 0;
	$newStruct = NULL;
	// $this->tabStruc["cct_column"][$db_col]["POS"]
	foreach( $posarr as $pos=>$db_col) {
		$newStruct[$db_col] = $this->tabStruc["cct_column"][$db_col];
		if ($posset>0) $newStruct[$db_col]["POS"] = $newpos;

		$cnt++;
		$newpos++;
	}
	reset ( $posarr );
	$this->tabStruc["cct_column"] = $newStruct;
}

function form0() {
	// FUNCTION: define type
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Define CCT_TABLE features";
	$initarr["submittitle"] = "Submit";
	$initarr["tabwidth"]    = "AUTO";

	$formobj = new formc($initarr, $hiddenarr, 0);
	
	$showcols    = array( "TABLE_NAME", "NICE_NAME","CCT_TABLE_NAME", 
				    "TABLE_TYPE", 'IS_VIEW', 'INTERNAL', 'EXIM', "notes" );
	$colFormType = array( "TABLE_TYPE"=>"select" );
	
	$selarr["TABLE_TYPE"] = array("", "BO"=>"BO", "BO_ASSOC"=>"BO_ASSOC", "SYS"=>"SYS", "SYS_META"=>"SYS_META" );

	foreach( $showcols as $dummy=>$colname) {
		
		$tmpType = "text";
		if ( $colFormType[$colname]!="" ) $tmpType = $colFormType[$colname];
		$tmpInits = $selarr[$colname]=="" ? NULL : $selarr[$colname];
			
		$fieldx = array ( 
		"title" => strtolower($colname), 
		"name"  => $colname,
		"object"=> $tmpType,
		"val"   => $this->tabStruc["cct_table"][$colname], 
		"inits" => $tmpInits,
		"notes" => ""
		 );
		$formobj->fieldOut( $fieldx );
		
	}
	reset ($showcols); 

	$formobj->close( TRUE );

	glob_printr( $this->tabStruc, "tabStruc" );	

	
}


// ----------------------------
		
function _cctColsShow(&$sql) {
	// show defined columns
	if (!$this->tabStruc["cct_column"]) {
		return;
	}
	
	$tabobj  = new visufuncs();
	$headOpt = array( "title" => "defined columns" );
	
	$headx  = NULL;
	$headx  = $this->cct_cols["names"];
	$tabobj->table_head($headx,   $headOpt);
	
	$selarrAPP_DATA = $this->_getTabContent( $sql, "APP_DATA_TYPE" );
	
	foreach( $this->tabStruc["cct_column"] as $db_col=>$colarr) {
		$dataArr=NULL;
		foreach( $headx as $dummy=>$colname) {
			$val = $colarr[$colname];
			switch ($colname) {
				case "COLUMN_NAME":
					$outtext = "<a href=\"".$_SERVER['PHP_SELF']."?go=3&parx[COLUMN_NAME]=".$db_col."\">".$db_col."</a>";
					break;
				case "APP_DATA_TYPE_ID":
					$outtext = $val.' ['.$selarrAPP_DATA[$val].']';
					break;
				default:
					$outtext = $val;
			}
			
			$dataArr[] = $outtext;
			
		}
		reset($headx);
		$tabobj->table_row ($dataArr);
	}
	reset ($this->tabStruc["cct_column"]);
	
	$tabobj->table_close();
	echo "<br>";
}


		
function save1() {
	
	global $error;
	$FUNCNAME= "save1";
    echo "... save params <br>";
	
    $allow = array("BO" ,"SYS", 'BO_ASSOC');
	if ( !in_array( $this->parx["TABLE_TYPE"], $allow) ) {
		 $error->set( $FUNCNAME, 1, "TABLE_TYPE ".$this->parx["TABLE_TYPE"].'not allowed' );
		 return;
	} 
	$this->tabStruc["cct_table"] = $this->parx;
	
	$this->structLib->init($this->tabStruc,  $this->settings);
	
	if ( !$this->tabStruc["cct_column"] ) {
		$this->structLib->_generateFirstCols();
		$this->tabStruc = $this->structLib->getStruct();
	}
	
	$this->saveStruct();
	
}

function form1( &$sql, $db_col ) {
	// columns
	// FUNCTION: define type
	
	
	
	
	$this->_cctColsShow($sql);
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Define columns";
	$initarr["submittitle"] = "Submit";
	$initarr["tabwidth"]    = "AUTO";

	$formobj = new formc($initarr, $hiddenarr, 1);
	
	$colFormType = array( "APP_DATA_TYPE_ID"=>"select" );
	
	$selarr["APP_DATA_TYPE_ID"] = $this->_getTabContent( $sql, "APP_DATA_TYPE" );
	
	$valOld = NULL;
	if ( $db_col!="" ) {
		$valOld = $this->tabStruc["cct_column"][$db_col];
	}

	foreach( $this->cct_cols["names"] as $dummy=>$colname) {
		
		$tmpType = "text";
		if ( $colFormType[$colname]!="" ) $tmpType = $colFormType[$colname];
		$tmpInits = $selarr[$colname]=="" ? NULL : $selarr[$colname];
			
		$thisVal = "";
		if ($colname=="COLUMN_NAME")  {
			$thisVal = $db_col;
		} else {
			$thisVal = $valOld[$colname];
		}
		
		$fieldx = array ( 
		"title" => strtolower($colname), 
		"name"  => $colname,
		"object"=> $tmpType,
		"val"   => $thisVal, 
		"inits" => $tmpInits,
		"notes" => ""
		 );
		$formobj->fieldOut( $fieldx );
		
	}
	reset ( $this->cct_cols["names"] ); 

	$formobj->close( TRUE );
	
	// echo "<br>\n";
	if ( $db_col!="" ) 
	{
		echo "<a href=\"".$_SERVER['PHP_SELF']."?go=4&parx[COLUMN_NAME]=".$db_col."\">Delete column ".$db_col."</a><br>";
	}
	echo "[<a href=\"".$_SERVER['PHP_SELF']."?go=8&\">Reset column POS</a>]<br>";
	echo "<br>\n";
	glob_printr( $this->tabStruc["cct_table"] , "CCT_TABLE");
}

function org1(&$sql) {
	global $error;
	$FUNCNAME= "org1";
	
	$this->save1();
	if ($error->Got(READONLY))  {
    	$error->set( $FUNCNAME, 1, "Error occurred." );
		return;
	}
	
	$colname=NULL;
	$this->form1($sql, $colname);
}

// -------------------------------
		
		
// save a column ... 
function save2() {
	global $error;
	$FUNCNAME= "save2";
    $this->_info ("... save params");
	
	if ($this->parx["COLUMN_NAME"]=="") {
		 $error->set( $FUNCNAME, 1, "Need COLUMN_NAME." );
		 return;
	} 
	$this->_setColumn($this->parx);
	
	$this->resortCols();
	$this->saveStruct();
}

function org2(&$sql) {
	global $error;
	$FUNCNAME= "org2";
	
	$this->save2();
	if ($error->Got(READONLY))  {
    	$error->set( $FUNCNAME, 1, "Error occurred." );
		return;
	}
	$colname="";
	$this->form1($sql, $colname);
}

function editCol(&$sql) {
	$colname=NULL;
	if ( $this->parx["COLUMN_NAME"]!="" ) $colname = $this->parx["COLUMN_NAME"];
	$this->form1($sql, $colname);
}

function createSQL(&$sql) {
	$this->structLib->createSQL($sql);
	
	if ($this->settings['sql_exp_type']==self::SQL_EXP_TYPE_INS ) {
	   $this->structLib->showForInsert();
	} else {
	    $this->structLib->show_SQL_pure();
	}
}

function removeStructs($sql) {
	$this->tabStruc=NULL;
	$this->saveStruct();
	
	echo "<b><a href=\"".$_SERVER['PHP_SELF']."?go=0\">Start again</a></b>";
}

function deleteCol(&$sql) {

	if ( $this->parx["COLUMN_NAME"]=="" ) {
		echo "Pleas give a name!";
		return;
	}
	
	$colname = $this->parx["COLUMN_NAME"];
	unset ($this->tabStruc["cct_column"][$colname]);
	$this->saveStruct();
	
	echo "<br><b>[<a href=\"".$_SERVER['PHP_SELF']."?go=3\">Back to columns</a>]</b>";
	
}

function DropTable(&$sql) {
	$this->structLib->dropSQL($sql);
	$this->structLib->showForInsert();
}

// resort columns
function do_resortCols(&$sql) {
	$this->_info ('Resort columns');
	$this->resortCols(1);
	$this->saveStruct();
}

/**
 * export $this->tabStruc as serialized(variable)
 */
function do_exportArray(&$sql) {
	global $error;
	$FUNCNAME= 'do_exportArray';
	
	require_once ('f.workdir.inc');
	$workdirObj = new workDir();
	$tmpdir   = $workdirObj->getWorkDir ( $this->scriptname );
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, '' );
		return;
	}
	
	$table= $this->tabStruc["cct_table"]['TABLE_NAME'];
	$filename = $table.'.franken.dat';
	$datafileFull = $tmpdir.'/'.$filename;
	// save as file
	$fp = fopen( $datafileFull, 'w');
	
	$content = json_encode($this->tabStruc,  JSON_PRETTY_PRINT);
	$retVal  = fputs( $fp, $content ); 
	fclose( $fp );
	
	echo '<a href="../../f_workfile_down.php?file='.urlencode($this->scriptname.'/'.$filename).
		'&mimetype=serialized-php">=&gt; <b>Download file</b></a><br><br>'."\n";
	
}

function do_importArrForm() {
	require_once ('func_form.inc');
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Import Frankenstein-file";
	$initarr["submittitle"] = "Submit";
	$initarr["tabwidth"]    = "AUTO";
	$initarr["ENCTYPE"]     = "multipart/form-data";

	$hiddenarr = NULL;
	$hiddenarr["go"]     = 11;

	$formobj = new formc($initarr, $hiddenarr, 0);

	$fieldx = array ( 
		"title" => "File", 
		"name"  => "userfile",
		"namex" => TRUE,
		"object"=> "file",
		"notes" => "the dat-file"
		 );
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );
}

// import the array
function do_importArrDo() {
	global $error;
	$FUNCNAME= 'do_importArrDo';
	
	require_once ('f.upload.inc');
	
	$uploadObj = new uploadC();
	$filenameShort = 'frankenstein.dat';
	
	$fileFull  =  $_FILES['userfile']['tmp_name'];
	
	// open in string
	$FH = fopen($fileFull, 'r');
	if ( !$FH ) {
	  $error->set( $FUNCNAME, 1, 'Cant open file '.$fileFull );
	  return;
	}
	echo 'size: '.filesize ($fileFull).'<br>';
	$contents = fread ($FH, filesize ($fileFull));
	
	$temp = json_decode($contents, TRUE);
	if (!isset($temp['cct_table'])) {
		$error->set( $FUNCNAME, 2, 'Uploaded file not valid. No array[cct_table] found.' );
		return;
	}
	$this->tabStruc = $temp;
	$this->saveStruct();
	
	echo '<br>o.k.<br>';
}

function do_fr_settings_form() {
    $initarr   = NULL;
    $initarr["action"]      = $_SERVER['PHP_SELF'];
    $initarr["title"]       = "Frankenstein settings";
    $initarr["submittitle"] = "Submit";
    $initarr["tabwidth"]    = "AUTO";
    $initarr["goNext"]      = 12;
    
    $hiddenarr=array(
        'subact'=>'save'
    );
    
    $formobj = new formc($initarr, $hiddenarr, 0);
    
    $sql_exp_type=$this->settings['sql_exp_type'];
    if ($this->settings['sql_exp_type']=='') $sql_exp_type='insarr';
    $tmpInits=array('insarr'=>'PHP-array', 'pure'=>'pure-SQL');
    $fieldx = array (
        "title" => 'SQL-Export-type',
        "name"  => 'sql_exp_type',
        "object"=> 'select',
        "val"   => $sql_exp_type,
        "inits" => $tmpInits,
        "notes" => ""
    );
    $formobj->fieldOut( $fieldx );
    
    //  db_type: ORACLE, POSTGRES
    $db_type = $this->settings['db_type'];
    if ($this->settings['db_type']=='') $db_type='POSTGRES';
    $tmpInits=array( 'ORACLE'=>'ORACLE', 'POSTGRES'=>'POSTGRES' );
    $fieldx = array (
        "title" => 'DB type',
        "name"  => 'db_type',
        "object"=> 'select',
        "val"   => $db_type,
        "inits" => $tmpInits,
        "notes" => ""
    );
    $formobj->fieldOut( $fieldx );
    
    
    $formobj->close( TRUE );
}

function do_fr_settings() {
    $this->settings = $this->parx;
    $this->saveSettings();
}

function doit( &$sql ) {
	global $error;
	$FUNCNAME= "doit";
	
	echo " [<a href=\"".$_SERVER['PHP_SELF']."?go=12\">Settings</a>]\n";
	
	if ( $this->go>0 AND $this->tabStruc["cct_column"] ) {
		echo "[<a href=\"".$_SERVER['PHP_SELF']."?go=5\">The art of gold pouring - Create SQL-commands!</a>] ";
		echo "[<a href=\"".$_SERVER['PHP_SELF']."?go=9\">Export Frankenstein-array</a>] ";
	} 
	
	echo " [<a href=\"".$_SERVER['PHP_SELF']."?go=10\">Import Frankenstein-array</a>]\n";
	
	if ( $this->go>0 AND $this->tabStruc["cct_column"] ) {
		echo "<br><br>";
		echo "[<a href=\"".$_SERVER['PHP_SELF']."?go=7\">Drop table and sub-structures</a>]\n";
		echo "[<b>Warning:</b> <a href=\"".$_SERVER['PHP_SELF']."?go=6\">Remove ALL form-data!</a>]<br>\n";
	}	
	
	echo "<br>\n";
	
	
	switch ($this->go) {
		case 0:
			$this->form0();
			break;
		case 1:
			$this->org1($sql);
			break;
		case 2:
			$this->org2($sql);
			break;
		case 3:
			$this->editCol($sql);
			break;
		case 4:
			$this->deleteCol($sql);
			break;
		case 5:
			$this->createSQL($sql);
			break;
		case 6:
			$this->removeStructs($sql);
			break;
		case 7:
			$this->DropTable($sql);
			break;
		case 8:
			$this->do_resortCols($sql);
			break;
		case 9:
			$this->do_exportArray($sql);
			break;
		case 10:
			$this->do_importArrForm();
			break;
		case 11:
			$this->do_importArrDo();
			break;
		case 12:
		    if ($this->subaction=='save') {
		        $this->do_fr_settings();
		    } else {
		        $this->do_fr_settings_form();
		    }
		    break;
	}
	
}

}