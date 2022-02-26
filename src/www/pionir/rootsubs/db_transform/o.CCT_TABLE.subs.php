<?php
/*MODULE: o.CCT_TABLE.subs.php
  DESCR:  specials for table CCT_TABLE and CCT_COLUMN
  AUTHOR: qbi
  INPUT:  $tablename
  		  $parx["format"] = ["php"], -- for php
		  				    "sql"
  VERSION: 0.1 - 20050921
*/

/*
	application data types:
 	1 	id 	INT
	2 	name 	STRING
	3 	notes 	STRING
	4 	password 	STRING
	5 	float 	FLOAT
	6 	string 	STRING
	7 	boolean 	INT
	8 	natural number 	INT
	9 	url 	STRING
	10 	url_query_string 	STRING
	11 	email 	STRING
	12 	date 	DATE
	14 	quality_score 	FLOAT
	15 	integer 	INT
*/

extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("visufuncs.inc");
require_once ("f.dbStructCrea.inc");


class fCct_tableDefs {

function __construct ($tablename, $parx ) {
	$this->tablename=$tablename;
	$this->parx=$parx;
	$this->StrucCreaLib = new fDbStructCrea();
}

function postMani( &$colInfo ) {
	// FUNCTION: post manipulation
	
	$this->newCols=NULL;
	
	if (!sizeof($colInfo)) {
		return;
	}
		foreach( $colInfo as $key=>$tmparr) {
			if ($key=="__table_data__") continue;
			
			$newCols[$key] = $tmparr;
			
			if ($tmparr["PRIMARY_KEY"])  $infox["pkcnt"] = $infox["pkcnt"] +1;
			else {
				if ($tmparr["NOT_NULL"] =="N" AND $key!="CCT_ACCESS_ID" AND $key!="NAME"  ) {
					$infox["pkcnt"] = $infox["pkcnt"] +1;
					$newCols[$key]["PRIMARY_KEY"] = $infox["pkcnt"];
				}
			}
			if ($tmparr["MOST_IMP_COL"]) $infox["Mostcnt"] = $infox["Mostcnt"] +1;
			if ($tmparr["NOT_NULL"] =="Y") $newCols[$key]["NOT_NULL"] = "0";
			if ($tmparr["NOT_NULL"] =="N") $newCols[$key]["NOT_NULL"] = "1";
			
			if ($tmparr["POS"] =="") {
				$infox["latestpos"] = $infox["latestpos"] + 1;
				$newCols[$key]["POS"] = $infox["latestpos"];
			} else {
				if ($tmparr["POS"]>$infox["latestpos"]) $infox["latestpos"] = $tmparr["POS"];
			}
			if ( $tmparr["EDITABLE"] =="") {
				$newCols[$key]["EDITABLE"] = "1";
			}
			
			
			// test for MOTHER table
			$pos_tmpid = strlen($key)-3;
			if ( substr($key, $pos_tmpid) == "_ID" ) {
				$mothertab = substr($key, 0, $pos_tmpid);
				if ($tmparr["CCT_TABLE_NAME"]=="") $newCols[$key]["CCT_TABLE_NAME"] = $mothertab;
			}
			
			// APP_DATA_TYPE_ID
			if ( substr($key, $pos_tmpid) == "_ID" ) {
				// test for data type
				if ( $tmparr["APP_DATA_TYPE_ID"]==6 ) {
					$newCols[$key]["APP_DATA_TYPE_ID"] = 1; // "ID" !!!
				}
			}
			if ( strstr($key, "DATE") != NULL ) {
				$newCols[$key]["APP_DATA_TYPE_ID"] = 12; // date
			}
			
			if ( $key == "NOTES" ) {
				$newCols[$key]["APP_DATA_TYPE_ID"] = 3; // notes
			}
		}
		reset($colInfo);
	
	
	if (!$infox["pkcnt"]) {
		// guess PK
		list($first_key, $tmparr) = each( $newCols );
		$newCols[$first_key]["PRIMARY_KEY"] = 1;
		reset($newCols);
	}
	if ( !$infox["Mostcnt"] ) {
		if ($newCols["NAME"]!= NULL) {
			$newCols["NAME"]["MOST_IMP_COL"] = 1;
		} 
	}
	
	$this->newCols=$newCols;
	$this->infox=$infox;
	
	return($infox);
}

/* depricated
function _insertSQL(&$sql, $tablename, $argu) {
  // fUNCTION: get SQL-command for insert
  
  $sqls      = ' VALUES (';
  $sql_cols  = 'INSERT INTO '.$tablename.' (';
  $i         = 0;
  
  reset($argu);
  foreach( $argu as $xcol=>$xval) {
  
    if ($i) {// beim ersten val KEIN komma
      $sqls     .=', ';
      $sql_cols .=', ';
    }
	if (($prim_name === $xcol) &&  !$xval){
	  $xval = ''; // make 0 to NULL for triggers
    }	 
	$sql_cols .= $xcol;
	if ( $xval === "") $tmpvalSql = 'NULL';
	else $tmpvalSql = $sql->addQuotes($xval);
	
	if ($options["types"]!="") {
		$xtype=$options["types"][$xcol];
		if ($xtype!="") {
			if ($xtype=="DATE1") {
				$tmpvalSql = $sql->DateString2Sql($xval,1);
			}
		}
	}
	
	if ($xcol=="NICE_NAME" OR $xcol=="CCT_TABLE_NAME") {
		$tmpvalSql = str_pad( $tmpvalSql, 15, " ", STR_PAD_RIGHT );
	}
	 
	$sqls     .= $tmpvalSql;
    $i++;
  }
  
  $sqls = $sql_cols.') '.$sqls.')'; // assemble whole query
  
  if ($this->parx["format"] == "php" ) {
  	$sqls = "\$insarr[] = \"".$sqls."\";";
  }
  
  return ($sqls);
}
*/

/* depricated
 
function showSQLTab( &$sql, $ccttabData) {

	$desttable = "CCT_TABLE";
	$tablename = $this->tablename;
	echo "<br><b>Show CCT_TABLE INSERT-SQL-commands</b><pre>";
	
	if ( $ccttabData["TABLE_TYPE"]=="" ) {
		if ( $this->infox["pkcnt"]==1 ) $ccttabData["TABLE_TYPE"] = "BO";
		if ( $this->infox["pkcnt"]>1  ) $ccttabData["TABLE_TYPE"]  = "BO_ASSOC";
	} 
	
	$argu = array();
	$argu["TABLE_NAME"] = $tablename; 
    $argu["NICE_NAME"] 	= strtolower($ccttabData["NICE_NAME"]);
    $argu["TABLE_TYPE"] = $ccttabData["TABLE_TYPE"];
    $argu["IS_VIEW" ] 	= $ccttabData["IS_VIEW" ];
    $argu["INTERNAL" ] 	= $ccttabData["INTERNAL"];
    $argu["EXIM" ] 		= $ccttabData["EXIM"];
	
	$sqls = $this->_insertSQL($sql, $desttable, $argu);
	echo htmlspecialchars($sqls)."\n";
  
}


function showSQL(&$sql, $newCols) {
	// fUNCTION: show entries as SQL commands
	
	$tablename = $this->tablename;
	$desttable = "CCT_COLUMN";
	$pk_name   = "TABLE_NAME";
	$pk2name   = "COLUMN_NAME";
	
	echo "<br><b>Show INSERT-SQL-commands</b><pre>";
	
	foreach( $newCols as $key=>$tmparr) {
		$newCols[$key][$pk_name] = $tablename;
		$newCols[$key][$pk2name] = $key;
		unset($newCols[$key]["COMMENTS"]); // is not a column in database
	}
	reset($newCols);
	
	foreach( $newCols as $key=>$tmparr) {
		$sqls = $this->_insertSQL($sql, $desttable, $tmparr);
		echo htmlspecialchars($sqls)."\n";
	}
	reset($newCols);
	echo "\n&nbsp;</pre>";
}

*/

function colsshow(&$infarr1, &$colInfo) {

	$tabobj = new visufuncs();
	$headOpt= array( "title" => "column description");
	$rowOpt = array("bgcolor" => "#EEEEEE");
	$headx  = array_flip($infarr1);
	
	$newCols = NULL;
	$infox   = NULL;
	
	
	//list($first_key, $tmparr) = each( $colInfo );
	//list($first_key, $tmparr) = each( $colInfo );
	//print_r($tmparr);
	//reset($colInfo);
	
	if ( sizeof($colInfo) ) {
		$tabobj->table_head($headx,   $headOpt);
		foreach( $colInfo as $key=>$tmparr) {
			if ($key=="__table_data__") continue;
			
			// bring the data in the right order (as $headx)
			$dataArr[$infarr1["COLUMN_NAME"]] = $key;
			foreach( $tmparr as $key=>$colval) {
				$dataArr[$infarr1[$key]] = $colval;
			}
			ksort($dataArr);
			$tabobj->table_row ($dataArr);
		}
		reset($colInfo);
		$tabobj->table_close();			 
	}
}

function showCreaStruc  ( &$sql, $ccttabData ) {
	
	$ccttabData['TABLE_NAME'] = $this->tablename;
	unset($ccttabData['PRIM_KEYS']);
	unset($ccttabData['IS_BUSINESS_OBJECT']);
	unset($ccttabData['MOST_IMP_COL']);
	unset($ccttabData['COMMENTS']);
	
	$tabStruc = array( 
		"cct_table"  => $ccttabData,
  		"cct_column" => $this->newCols 
		);
	
		
	$this->StrucCreaLib->init( $tabStruc );
	$this->StrucCreaLib->_newCCT_TABLE_meta( $sql );
	
	echo "<br><br><b>META-data</b><br><br>\n";
	
	$this->StrucCreaLib->show_META();

	// $this->showSQLTab( $sql, $ccttabData);
	// $this->showSQL   ( $sql, $this->newCols);
}

function formshow() {
	require_once ('func_form.inc');
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Select a table";
	$initarr["submittitle"] = "Submit";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;

	$formobj = new formc($initarr, $hiddenarr, 0);

	$fieldx = array ( "title" => "Tablename", "name"  => "tablename",
			"object" => "text",
			"val"   => "", "namex" => 1,
			"notes" => "tabelname code" );
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );

}

}

global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

if ($parx["format"]=="") $parx["format"]="php";


$title       = "AppDefs: define META-columns: ".$tablename;

$infoarr["title"]    = $title;
$infoarr["form_type"]= "tool";
$infoarr["locrow"]   = array( array('index.php', 'transform home' ) );
$infoarr["obj_name"] = $tablename;


$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

$mainLib = new fCct_tableDefs( $tablename, $parx );

if ( $tablename == "" ) {
  // echo "ERROR: No tablename given!<br>";
  echo "<ul>";
  $mainLib->formshow();
  htmlFoot();
}

echo "1. Show CACHE info for table ";
echo " [<a href=\"".$_SERVER['PHP_SELF']."\">Start again</a>]";
$ccttabData = $_s_i_table[$tablename]["__table_data__"];

echo "<pre>";
print_r( $ccttabData );
echo "</pre>";


$colInfo = &$_s_i_table[$tablename];

$infarr1 = array(
			"COLUMN_NAME" => 0,
			"COMMENTS"    => 1, 
            "NICE_NAME"   => 2,  
            "PRIMARY_KEY" => 3, 
            "MOST_IMP_COL"=> 4,  
            "UNIQUE_COL"  => 5,  
            "NOT_NULL"    => 6,  
            "VISIBLE"     => 7,  
            "CCT_TABLE_NAME"  => 8,  
            "APP_DATA_TYPE_ID"=> 9,
			"POS" 		 => 10,
			"EDITABLE"   => 11,
			"EXIM"       => 12
			 );

			 
echo "first column data:";
			 
$mainLib->colsshow( $infarr1, $colInfo);

$infox = $mainLib->postMani( $colInfo );


echo "<br>";
$vopt=array("showid" => 1,"title" =>"result:");
$header=null;
if (sizeof($infox)) visufuncs::table_out( $header, $infox, $vopt);

echo "<br>New columns<br>";

$mainLib->colsshow  ( $infarr1, $mainLib->newCols);
$mainLib->showCreaStruc  ( $sql, $ccttabData);



htmlFoot("<hr>");




