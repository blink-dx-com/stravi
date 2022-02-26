<?php
/**
 * [DB_Haeckelianer] upgrade database scheme from OLD to NEW (e.g. v1032 to v1033)
 * - include PHP-code from files "data/${version}.inc" (alternative location: "{LAB}/root/db_transform/data"
   GLOBAL $_SESSION["s_formState"]['convert.php']=array()
 * @package convert.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq  UREQ:0014545 (FS-ARC02-02) [DB_Haeckelianer] Support upgrade of the database model;
 * @param   $go = 
  				0  - prepare
  				1  - show commands
				2  - do update till error
				3  - do update, commit, also if an error occures
  			
			$parx["toversion"]   : e.g. 1.0.3.3 , "i003" - production ...
			$parx["partsonly"] : [0],1,2,3
			$parx["partsAll"]  : -1,[1]  -- perform all parts, if $dataArray['extconvert']['parts'] > 0 !!! 
			$parx["lineStart"]     : for "sql"  : start at line-number 
			$parx["metaLineStart"] : for "meta" : start at line-number
			$parx["showCom"]   : 0|1 : show commands
			$parx["showopt"] : "", 
				    		  1    show pure SQL commands
			$parx["method"] : ["ONE"] - transform ONLY this version
							   "ALL"  - transform ALL versions up to this one
			$parx["showAdvanced"] : [0], 1 show advanced options
 */
session_start(); 


require_once ("reqnormal.inc");
require_once ("insert.inc");
require_once ('get_cache.inc');
require_once ('validate.inc');
require_once ('convertData.inc');
require_once ('func_form.inc');
require_once ("visufuncs.inc");
require_once ('f.update.inc');

/**
 * convert one version
 * @author steffen
 *
 */
class gDB_ConvOne {
	
	// @var convertData.inc :: convDataArray_STRUCT
	var $dataArray;
	

	
function __construct($go) {
	$this->allowTransform = 1;
	$this->go = $go;
	$this->dbConvertObj = new dbConvertC();
	$this->dbConvDoLib  = new dbConvOneVersCls();
	$this->debug = $_SESSION["userGlob"]["g.debugLevel"];
	$this->_initMetaStruct();
	
}

function setParx($parx) {
	$this->parx = $parx;
	$this->dataArray    = NULL;
}

function _initMetaStruct() {
	global $_s_i_table;
	
	$_s_i_table['CCT_TABLE']['__table_data__']['PRIM_KEYS'][0]='TABLE_NAME';
	
	$_s_i_table['CCT_COLUMN']['__table_data__']['PRIM_KEYS'][0]='TABLE_NAME';
	$_s_i_table['CCT_COLUMN']['__table_data__']['PRIM_KEYS'][1]='COLUMN_NAME';
	
	$_s_i_table['APP_DATA_TYPE']['__table_data__']['PRIM_KEYS'][0]='APP_DATA_TYPE_ID';
	
}

/**
 * show info text
 * @return 
 * @param int $prio 0:low 5:high
 * @param object $text
 */
function _info( $prio, $text ) {
	$showit = 0;
	if ( ($prio>=3) or ($this->debug>1) ) $showit = 1;
	if ( $showit ) echo 'Info: ' . $text . '<br>'."\n";
}

/**
 * check primary keys in $valarr
 * @param  
 * @return array $idarr
 */
function _getIdArr($table, $valarr) {
	$primkeys = primary_keys_get2($table);
	if ($primkeys==NULL) return;

	$idarr = NULL;
	foreach( $primkeys as $key) {
		if ( $valarr[$key]==NULL ) {  // error !!!
			$idarr=NULL;
			break;
		}
		$idarr[$key] = $valarr[$key];
	}
	 

	return ($idarr);
}

function _deleteElemArr(&$sqlo, $table, $idarr) {
	global $error;
	$FUNCNAME= "_deleteElemArr";
	
	if (!sizeof($idarr)) {
		$error->set( $FUNCNAME, 1, "idarr is empty" );
		return;
	}
	$tmpand   = NULL;
	$sqlwhere = NULL;
	foreach( $idarr as $key=>$val) {
		if ( $val==NULL or $key==NULL ) {
			$error->set( $FUNCNAME, 2, "val or key are empty." );
			return;
		}
		$sqlwhere .= $tmpand . $key."=". $sqlo->addQuotes($val);
		$tmpand = ' and ';
	}
	reset ($idarr); 
	
	$sqls = 'delete from '.$table. ' where '. $sqlwhere;
	return ($sqls);
}

/**
 * element exists ?
 * @return int 
 *    0 : does NOT exist
 * 	  1 : does exist
 */
function _checkExists(&$sqlo, $table, $idarr) {
	$tmpand   = NULL;
	$sqlwhere = NULL;

	if ( $idarr==NULL )  return 0; // fallback ...

	foreach( $idarr as $key=>$val) {
		$sqlwhere .= $tmpand . $key."='". $val."'";
		$tmpand = ' and ';
	}
	reset ($idarr); 
	$sqlo->query('SELECT 1 FROM '.$table.' WHERE '.$sqlwhere);
	$retval = 0;
	if ( $sqlo->ReadRow() ) $retval = 1;

  	return $retval;
}

/**
 * perform SQL-queries from $this->dataArray["sql"]
 */
function doXSqlRaw( &$sql ) {
	/*
	$this->insarr
	$this->parx
	*/
	//$FUNCNAME = 'doSqlRaw';

	$insarr = &$this->dataArray["sql"];

	if ($this->go>=2 AND $this->allowTransform) {
		// echo "<b>SET AUTOCOMMIT OFF!</B><br><br>";
		// $sql->setautocommit(false); 
	}
	
	if ( $this->parx["showCom"] ) {
		echo "<br><B>List of SQL-commands:</B>";
		echo "<br><br>\n";
		echo "<font size=-1>";
	}
	
	$error_flag = 0;
	$cnt		= 0;
	if ($this->parx['lineStart']>0) {
		$lineStart = $this->parx['lineStart'];
	} else $lineStart = 0;
	//
	// EXECUTE NOW !
	//
	if ( $insarr!=NULL ) {
		reset($insarr);
		foreach( $insarr as $i=>$sqltmpcomd) {

			if ($lineStart>0 AND ( ($cnt+1)<$lineStart ) ) {
				$cnt++;
				continue;
			}	

			if ( $this->parx["showCom"] ) {
				if ( !$this->parx["showopt"] ) echo "<font color=gray>".($cnt+1)."</font>. ";
				$showcom = htmlspecialchars($sqltmpcomd);
				$showcom = str_replace("\n", "<br>\n", $showcom);
				echo $showcom.";<br>\n";
			}
			
			if ($this->go>=2 AND $this->allowTransform) {
				do {
					if ($sqltmpcomd=="") break;
					if (substr($sqltmpcomd,0,2)=="/*") break; // a comment ... 
					$retval = $sql->query($sqltmpcomd);
					// if ($this->go>=3) $sql->setautocommit(true);
					if (!$retval) {
						$error_flag++;
						echo "<font color=red>Error:</font> sqls: ".$sqltmpcomd."</br>\n";
					}
				} while (0);
			}

			$cnt++;
		}
		reset ($insarr);
	}
	
	$retarr = array( "cnt"=>$cnt, "error_flag"=>$error_flag );
	
	echo "</font>";
	echo "<B>".$cnt."</B> SQL-commands.<br>\n";
	// echo "<hr>";
	
	return ($retarr);
}

/**
 * - transform meta => sql
 * - "act"]=='insupd' if element exists: do update
 * @param  array  &$vararr : array( 
 *     'act' =>(
 *       "upd", : need 'ids'=array(KEY=>VAL,)
 *       "del", : need 'ids'=array(KEY=>VAL,)
 *       "ins", 
 *       "insupd"
 *       ), 
 * 		'tab'=>..., 
 *      "vals"= >array(key=>val) 
 *    )
 * @example 
 *   "del": $meta[]=array( 'act'=>'del', 'tab'=>'CCT_COLUMN', 'ids'=>array('COLUMN_NAME'=>'EXTRA_OBJ2_ID', 'TABLE_NAME'=>'ABSTRACT_SUBST') );
 * @return sql-command
 */
function _meta2sql( &$sql, &$vararr ) {
	
	global $error;
	$FUNCNAME= "_meta2sql";

	if ($vararr["tab"]=="") {
		 $error->set( $FUNCNAME, 1, "Tablename missing" );
		 return;
	} 
	
	$table  = $vararr["tab"];
	$action = $vararr["act"];

	do {
		if ( $action=='upd' and is_array($vararr["vals"]) AND is_array($vararr["ids"]) ) {
			$sqls = gObjUpdate::update_row_sGet($sql, $vararr["tab"], $vararr["vals"], $vararr["ids"] );  
			if ($error->Got(READONLY))  {
	    		 $error->set( $FUNCNAME, 2, "SQL preparation failed." ); 
				 return;
			}  
			break;
	    }
	    
		if ( $action=='insupd' ) {
	
			$idarr  = $this->_getIdArr($table, $vararr["vals"]);
			$exists = $this->_checkExists($sql, $table, $idarr);
	
			if (!$exists) { 
				$sqlsPre = insert_row_Get( $sql, $vararr["tab"], $vararr["vals"] );
				$sqls = 'insert into '.$vararr["tab"]. ' '. $sqlsPre;
	 		} else {
	 			// unset primary keys in arguments
	 			$valsTmp = $vararr["vals"];
	 			foreach( $idarr as $key=>$val) {
	 				unset($valsTmp[$key]);
	 			}
	 			reset($idarr);
				$sqlsPre = gObjUpdate::update_row_sGet($sql, $vararr["tab"], $valsTmp, $idarr );
				$sqls = $sqlsPre;
			}
	
			if ($error->Got(READONLY) or $sqlsPre=='' )  {
	    		$error->set( $FUNCNAME, 2, "SQL preparation failed." ); 
				return;
			}  
			break;
	    }
	    
	    if ( $action=='del' ) {
	    	$idarr  = $vararr["ids"];
	    	// $exists = $this->_checkExists($sql, $table, $idarr);
	    	$sqls = $this->_deleteElemArr($sql, $table, $idarr);
	    	if ($error->Got(READONLY) )  {
	    		$error->set( $FUNCNAME, 3, "delete prepare failed." ); 
				return;
			} 
	    }
	    
	} while (0);
	
	return ($sqls);

}

/**
 * insert META DATA
 * @param object $sql
 * @return void|number[]
 */
function doSqlMeta( &$sql ) {

	$meta = &$this->dataArray["meta"];
	
	if ($meta==NULL) {
		return;
	} 

	if ($this->go>=2 AND $this->allowTransform) {
		//echo "<b>SET AUTOCOMMIT OFF!</B><br><br>";
		//$sql->setautocommit(false); 
	}
	
	if ($this->parx['metaLineStart']>0) {
	    $lineStart = $this->parx['metaLineStart'];
	} else $lineStart = 0;
	
	if ( $this->parx["showCom"] ) {
		echo "<br>List of META-commands:";
		echo "<br><br>\n";
		echo "<font size=-1>";
	}
	
	$error_flag = 0;
	$cnt		= 0;
	
	//
	// EXECUTE NOW !
	//
	foreach( $meta as $i=>$onearr) {
	    
	    if ($lineStart>0 AND ( ($cnt+1)<$lineStart ) ) {
	        $cnt++;
	        continue;
	    }	
	
		$sqlCmd = $this->_meta2sql($sql, $onearr );
	
		if ( $this->parx["showCom"] ) {
			if ( !$this->parx["showopt"] ) echo "<font color=gray>".($cnt+1)."</font>. ";
			echo htmlspecialchars($sqlCmd).";<br>\n";
		}
		
		if ($this->go>=2 AND $this->allowTransform) {
			do {
				if ($sqlCmd=="") break;
				if (substr($sqlCmd,0,2)=="/*") break; // a comment ... 
				$retval = $sql->query($sqlCmd);
				// if ($this->go>=3) $sql->setautocommit(true);
				if (!$retval) $error_flag++;
			} while (0);
		}
		$cnt++;
	}
	
	$retarr = array( "cnt"=>$cnt, "error_flag"=>$error_flag );
	
	echo "</font>";
	echo "<B>".$cnt."</B> META-commands.<br>\n";
	// echo "<hr>";
	
	return ($retarr);
}

// tables contain data ?
function _checkTableContent( &$sqlo, $tables ) {
	
	$cnt=0;
	foreach( $tables as $table) {
		
		// table exists ???
		if ( table_exists( $sqlo, $table) ) {
			$sqlsel = 'count(1) from '.$table;
			$sqlo->Quesel($sqlsel);
			$sqlo->ReadRow();
			$cnt = $sqlo->RowData[0];
			if ($cnt>0) return $cnt; 
		}
	}
	
	return ($cnt);
}

/**
 * get data of version
 * @param array versopt_TYPE $versopt
 */
function _GetDataSub( &$versopt )  {
	global $error;
	$FUNCNAME= "_GetDataSub";
	
	$toversion = $this->parx["toversion"];
	
	if ( $this->_versInit != $toversion ) {
		$retval = $this->dbConvDoLib->initVers( $toversion );
		if (!$retval) {
			$error->set( $FUNCNAME, 1, "Init of ConvertClass for vers ".$toversion." failed." );
		 	return;
		}
		$this->_versInit = $toversion;
	}
	

	$dataArray = $this->dbConvDoLib->GetData( $versopt );
	if ($dataArray==NULL){
		 $error->set( $FUNCNAME, 1, "No data for version ".$toversion."." );
		 return;
	}
	
	return ($dataArray);
}

/**
 * get data of version
 * @param array versopt_TYPE $versopt
 */
function GetData( $versopt, &$sqlo )  {
	global $error;
	//$FUNCNAME= "GetData";
	
	$this->dataArray = NULL;
	$go=0;
	if ( $this->go>=2 ) $go=1;
	$versopt['sql']  = &$sqlo; 
	$versopt['go']   = $go;
	$dataArray = $this->_GetDataSub( $versopt );
	if ($dataArray==NULL) return; 
	$this->dataArray = $dataArray;
	return ($dataArray);
}


/**
 * - get SQL-code
 * - perform SQL
 * ['extconvert'] :  => array( 'tables'=>array(tables), 'parts'
 */
function _doSqlOnePart(&$sqlo, &$sqlo2, $part) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$versopt = array( 'onlyparts'=>$part, 'sql'=>&$sqlo );
	$dataarr = $this->_GetDataSub( $versopt );
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, 'problem at getting SQL-code' );
		return;
	}

	$this->dataArray['sql']  = $dataarr['sql'] ;
	$this->dataArray['meta'] = $dataarr['meta'] ;

	$this->_info(2, 'doXSqlRaw: SQL-commands: '.sizeof($dataarr['sql']) );
	$retarr = $this->doXSqlRaw ( $sqlo );
	$this->doSqlMeta( $sqlo );
	
	// call hard PHP-code
	$go = 0;
	if ( $this->go>=2 ) $go=1;
	$this->dbConvDoLib->checkTransform($sqlo, $sqlo2, $part, $go);
	
	return ($retarr);
}

/**
 * - get SQL-data and perform QUERIES
 * @param $option
 * 				 ['allparts'] : [0], only one 'onlyparts'
 *							       1  all parts from $dataArray['extconvert']['parts'] 
 *				['onlyparts'] : [0],1,2,3 ...
 */
function doSql( &$sqlo, &$sqlo2, $option=NULL ) {
	global $error;
	$FUNCNAME= "doSql";
	
	$numparts = $this->dataArray['extconvert']['parts'];
	
	if ( ($option['allparts']>0) and ($numparts>0) ) {
		
			$part=0;
			$this->_info(3, 'More than one PART for convert.');
			$this->_info(3, 'Check PART 1.');
			
			if ( $this->dataArray['extconvert']['tables']!=NULL ) {
				$tables = $this->dataArray['extconvert']['tables'];
				$extraConvertNeed = $this->_checkTableContent($sqlo, $tables);
				if ($extraConvertNeed>0) {
					$error->set( $FUNCNAME, 1, 'some table-data need extra conversion; tables: '.
						implode(',', $tables) );
					return;
				}
			}
			// start sql
			$retarr = array();
			while ( $part<$numparts ) {
				$retarr1 = $this->_doSqlOnePart($sqlo, $sqlo2, $part);
				$retarr  = array_merge($retarr, $retarr1);  // TBD: does it work ???
				if ($error->Got(READONLY))  {
					$error->set( $FUNCNAME, 1, 'problem at PART:'.$part );
					return;
				}
				$part++;
			}
			
		
	} else {
		if ( $option['onlyparts']>0 ) {
			$part = $option['onlyparts'];
		} else $part=0;
		$this->_info(2, 'Do only one Part.');
		$retarr = $this->_doSqlOnePart($sqlo, $sqlo2, $part);
	}
	
	return ($retarr);
}

function getInternVersion(&$sql) {
	 return ( $this->dbConvertObj->getInternVersion($sql) );
}


function checkVersion( &$sql ) {
	$errtxt = $this->dbConvertObj->checkVersion($sql, $this->parx["toversion"], $this->dataArray["reqs"]["DbVersionNeed"] );
	if ($errtxt!="") {
		htmlInfoBox( "Warning", $errtxt, "", "ERROR" );
		echo "<br>\n";
	}
}

function getNeedVersion() {
	return ( $this->dataArray["reqs"]["DbVersionNeed"] );
}

function getExtconvert() {
	return ( $this->dataArray['extconvert'] );
}

function getTransInfo() {
	return ( $this->dataArray["info"] );
}

}


// -----

class mainScriptC {

function __construct($go, $parx) {
	$this->allparts = 0;
	
	$temparr=array();
	if ( $parx["method"]=="" ) $parx["method"]="ONE"; // default
	if ( $parx["partsAll"]==NULL ) $parx["partsAll"] = 1;
	if ( $parx["partsAll"]>0 ) $this->allparts = 1;
	
	// save in formState var
	if ($parx["showAdvanced"]!==NULL) {
		$temparr["showAdvanced"]=$parx["showAdvanced"];
		$_SESSION["s_formState"]['convert.php'] = $temparr;
	}
	
	$temparr = $_SESSION["s_formState"]['convert.php'];
	if ( $temparr["showAdvanced"]>0 ) {
		$parx["showAdvanced"] = $temparr["showAdvanced"];
	}
	
	
	$this->go   = $go;
	$this->parx = $parx;
	
	$this->dbConvOne    = new gDB_ConvOne($go);
	$this->dbConvOne->setParx($parx);
	
	$this->dbConvertObj = new dbConvertC();

}

function init( &$sql ) {
	$sqls = "select value from globals where name='DBVersion'";
	$sql->query($sqls);
	$sql->ReadRow();
	$this->vers["Full"] = $sql->RowData[0];
	$this->vers["main"] = substr($this->vers["Full"], 0, 7);
	$this->vers["intern"] = $this->dbConvertObj->getInternVersion($sql);
	$strpos1 = strpos($this->vers["intern"]," ");
	$this->vers["internShort"] = substr($this->vers["intern"], 0, $strpos1);
}

/**
 * get initial data of this version
 */
function GetVersInitData( $versopt, &$sqlo ) {
	$this->dbConvOne->GetData( $versopt, $sqlo );
}

function getNeedVersion() {
	return ( $this->dbConvOne->getNeedVersion() );
}

function getTransInfo() {
	return ( $this->dbConvOne->getTransInfo() );
}

function getExtconvert() {
	return ( $this->dbConvOne->getExtconvert() );
}

function getInitData(&$sqlo) {
	global $error;
	$FUNCNAME="getInitData";
	
	$parx = $this->parx;
	$go = $this->go;
	
	$tabobj  = new visufuncs();
	$headx   = array(1,2);
	$headopt = array("headNoShow" => 1);
	$tabobj->table_head($headx, $headopt);
	
	$tabobj->table_row ( array("This DB-Version:",  "<B>".$this->vers["Full"]."</B>") );
	$tabobj->table_row ( array("Intern DB-Version:","<B>".$this->vers["intern"]."</B>") );
	if ($parx["toversion"] != "") 
		$tabobj->table_row ( array("New selected DB-Version:","<B>".$parx["toversion"]."</B>") );
	
	if ($parx["toversion"] == "" or !$go) {
		$tabobj->table_close();
		echo "<br>";
		$this->selectVersion($parx["toversion"]);
		htmlFoot(); // !!!
	} 
	
	$versopt = NULL;

	if ($parx["partsonly"]>0) {
		$versopt["onlyparts"] = $parx["partsonly"];
	}
	
	$this->versopt = $versopt;
	$this->GetVersInitData( $versopt, $sqlo );
	if ( $error->Got(READONLY) ) {
		$tabobj->table_close();
		echo "<br>";
		$error->set( $FUNCNAME, 1, 'error occured' );
		$error->printAll();
		htmlFoot();
	} else {
		$needVersion = $this->getNeedVersion();
	}
	$extConvert = $this->getExtconvert();
	
	$tabobj->table_row ( array("Transform-Info:","<B>".$this->getTransInfo()."</B>") );
	$tabobj->table_row ( array("Need DB-Version:","<B>".$needVersion."</B>") );
	
	$levels = 0;
	if ( $extConvert!=NULL ) {
		$levels = $extConvert['parts'];
		$tabobj->table_row ( array("Extended: levels","<B>".$levels."</B>", 'Need to run more than one PART (0,1,..)') );
		if ($extConvert['tables']!=NULL) { 
			$tables = $extConvert['tables'];
			$tabsout = implode(',', $tables);
			$tabobj->table_row ( array("<b>Extended: tables</b>","<B>".$tabsout ."</B>", 'tables with content need special convert tool!') );
		}
	}
	
	if ($this->parx["partsAll"]>0 and $levels>0) {
		$tabobj->table_row ( array("Update ALL parts:","<B>YES</B>") );
	} else {
		$tabobj->table_row ( array("Update only part:","<B>".$this->parx["partsonly"]."</B>") );
	}
	
	if ($this->parx["lineStart"]>0) {
		$tabobj->table_row ( array("SQL: Start at line:","<B>".$this->parx["lineStart"]."</B>") );
	}
	if ($this->parx["metaLineStart"]>0) {
	    $tabobj->table_row ( array("META: Start at line:","<B>".$this->parx["metaLineStart"]."</B>") );
	}
	$tabobj->table_close();
	echo "<br>";

}

function GoInfo($go, $coltxt=NULL) {

	
	$goArray   = array(
		"0"=>"Select version", 
		1=>"Prepare SQL-commands", 
		2=>"Do update (no autocommit on error)",
		3=>"Do update (autocommit, ignore errors)"
		);
	$extratext = "[<a href=\"".$_SERVER['PHP_SELF']."\">Start again</a>]";
	
	$formPageLib = new FormPageC();
	$formPageLib->init( $goArray, $extratext );
	$formPageLib->goInfo( $go ); 
	
}

function selectVersion($version=NULL) {

	
	
	$parx = $this->parx;
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Prepare - Select new DB-version";
	$initarr["submittitle"] = "Next &gt;&gt;";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;

	$formobj = new formc($initarr, $hiddenarr, 0);

	$tmparr    = $this->dbConvertObj->getVersions();
	$newTmpArr = NULL;
	foreach( $tmparr as $val) {
		$newTmpArr[$val] = $val;
	}
	
	$fieldx = array ( 
			"title" => "New version", 
			"name"  => "toversion",
			"object"=> "select",
			"val"   => $version, 
			"inits" => $newTmpArr,
			"notes" => "select the version" );
	$formobj->fieldOut( $fieldx );
	
	$tmpsel = array( "ONE"=>"[one]", "ALL"=>"all to this" );
	$fieldx = array ( 
			"title" => "one/all", 
			"name"  => "method",
			"object"=> "select",
			"inits" => $tmpsel,
			"val"   => $parx["method"], 
			"notes" => "transform [ONE] version or to ALL versions"
			);
	$formobj->fieldOut( $fieldx );
	
	
	$fieldx = array ( 
			"title" => "SQL: Start at line", 
			"name"  => "lineStart",
			"object"=> "text",
			"val"   => $parx["lineStart"], 
			"notes" => "start at line-number (e.g. 10)",
			"optional" => 1 );
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array (
	    "title" => "META: Start at line",
	    "name"  => "metaLineStart",
	    "object"=> "text",
	    "val"   => $parx["metaLineStart"],
	    "notes" => "start at line-number (e.g. 10)",
	    "optional" => 1 );
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ( 
			"title" => "Show commands", 
			"name"  => "showCom",
			"object"=> "checkbox",
			"val"   => $parx["showCom"], 
			"notes" => "show?",
			"optional" => 1 );
	$formobj->fieldOut( $fieldx );
	
	
	if ( $parx["showAdvanced"] >0 ) {
		$fieldx = array ( "object"=> "hr" );
		$formobj->fieldOut( $fieldx );
		
		
		$tmparr = array(-1=>'only one part', 0=>'[auto]', 1=>'all parts');
		$fieldx = array ( 
				"title" => "Parts?", 
				"name"  => "partsAll",
				"object"=> "select",
				"val"   => $parx["partsAll"], 
				"inits" => $tmparr,
				"optional" => 1,
				"notes" => "update only parts" );
		$formobj->fieldOut( $fieldx );
		
		$tmparr = array(0=>'0',1=>1, 2=>2, 3=>3, 4=>4);
		$fieldx = array ( 
				"title" => "Advanced: only parts", 
				"name"  => "partsonly",
				"object"=> "select",
				"val"   => 0, 
				"inits" => $tmparr,
				"optional" => 1,
				"notes" => "update only parts" );
		$formobj->fieldOut( $fieldx );
	
		
		$fieldx = array ( 
				"title" => "Show pure SQL", 
				"name"  => "showopt",
				"object"=> "checkbox",
				"val"   => $parx["showCom"], 
				"notes" => "show pure-SQL-code without html-code?",
				"optional" => 1 );
		$formobj->fieldOut( $fieldx );
	}
	
	if ( $parx["showAdvanced"] >0 ) {
		 $addObjects='<b>Advanced</b> | <a href="'.$_SERVER['PHP_SELF'].'?parx[showAdvanced]=0">Normal</a>';
	} else {
		$addObjects='<a href="'.$_SERVER['PHP_SELF'].'?parx[showAdvanced]=1">Advanced</a> | Normal';
	
	}
	
	$closopt=NULL;
	$closopt["addObjects"] = $addObjects;
	$formobj->close( TRUE, $closopt );

}

function go1() {
	
	$parx = $this->parx;
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Execute transformation";
	$initarr["submittitle"] = "Update now!";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	foreach( $parx as $key=>$val) {
		$hiddenarr["parx[".$key."]"]=$val;
	}
	reset ($parx); 

	$formobj = new formc($initarr, $hiddenarr, 1);
	$formobj->close( TRUE );
	
	echo "<br>";
}

function doOneVersion ( &$sql, &$sql2, $go ){
	global  $error;
	$FUNCNAME='doOneVersion';

	$parx = $this->parx;
	$allparts = $this->allparts;
	
	
	$this->dbConvOne->checkVersion( $sql );
	
	if ($go==1) {
		$this->go1();
	}

	if ($go>=2) echo "<B>Execute SQL-commands...</B><br>";
	
	$versopt = $this->versopt;
	$this->dbConvOne->GetData( $versopt, $sql );
	if ($error->Got(READONLY)){
		$error->set( $FUNCNAME, 1, "No data for version ".$versOne."." );
		return;
	}
	
	/*
	   $option
    * 				 ['allparts'] : [0], only one 'onlyparts'
    *							       1  all parts from $dataArray['extconvert']['parts'] 
    *				['onlyparts'] : [0],1,2,3 ...
    */
	$conopt = array('allparts'=> $allparts, 'onlyparts'=>$parx["partsonly"]);
	$ansarr = $this->dbConvOne->doSql( $sql, $sql2, $conopt );
	echo "<hr>";
	

	if ($go>=2) {
		if ( $ansarr["error_flag"] OR $error->Got(READONLY) ) {
			
			if ($ansarr["error_flag"]) {
				 $error->set( $FUNCNAME, 1, "<B>".$ansarr["error_flag"]."</B> errors occured." );
			}
			return;
		}
		
	}
}

/**
 * get versions till this one
 */
function _getVersTill( $toversion ) {

	$typex = "norm";
	$nowversion = $this->vers["main"] ;
	if ( substr($toversion,0,1)=="i" ) {
		$typex = "intern";
		$nowversion = "i".$this->vers["internShort"];
	}
	$versarr    = $this->dbConvertObj->getTypeVersions($typex);
	$useVersions = NULL;
	foreach( $versarr as $dummy=>$vers) {
		if ( $vers > $nowversion AND ($vers <= $toversion) ) {
			$useVersions[] = $vers;
		}
	}
	reset($versarr);
	return ($useVersions);
}

function doAll ( &$sql, &$sql2, $go ){
	global  $error;
	$FUNCNAME="doAll";
	
	$versions = $this->_getVersTill( $this->parx["toversion"] );
	glob_printr( $versions, "versions to be transformed." );
	
	$versopt = NULL;
	if ($this->parx["partsonly"]>0) {
		$versopt["onlyparts"] = $this->parx["partsonly"];
	}
	
	$this->dbConvOne->checkVersion( $sql );
	
	if ($go==1) {
		$this->go1();
	}
	
	if ($go>=2) echo "<B>Execute Transformation</B> (go=".$go.")<br>";
	
	//$looperror=0;
	if ( !sizeof($versions)) {
		$error->set( $FUNCNAME, 1, "This version is higher than ". $this->parx["toversion"]." !");
		return;
	}
	
	foreach( $versions as $versOne) {
	
		echo "<b>Version: $versOne</b><ul>\n";
		$parx = $this->parx;
		$parx["toversion"] = $versOne;
		$this->dbConvOne->setParx($parx);
		
		$this->dbConvOne->GetData( $versopt, $sql );
		if ($error->Got(READONLY)){
			$error->set( $FUNCNAME, 1, "No data for version ".$versOne."." );
			break;
		}
		
		$dosqlOpt = array('allparts'=>1);
		$ansarr = $this->dbConvOne->doSql( $sql, $sql2, $dosqlOpt );
		
		if ($ansarr["error_flag"] OR $error->Got(READONLY)) {
			
			$error->set( $FUNCNAME, 2, "Version:". $versOne." <B>".$ansarr["error_flag"].
				  "</B> errors occured.");
			break;
		}
			
		
		echo "</ul>";
	}
	
	
	
	
}

}


// ---------------------------------------------

$error = & ErrorHandler::get();
$sql   =  logon2( $_SERVER['PHP_SELF'] );
$sql2  =  logon2(  );
if ($error->printLast()) htmlFoot();

$go = $_REQUEST['go'];
$parx = $_REQUEST['parx'];

$title = '[DB_Haeckelianer] Upgrade DB-version';
$titleShort          = 'DB_Haeckelianer';
$infoarr=array();
$infoarr["title"]    = $title;
$infoarr["title_sh"] = $titleShort;
$infoarr['scriptID'] = 'convert.php';
$infoarr["form_type"]= "tool";
$infoarr["locrow"]   = array( array("index.php", "db-transform") );

$mainScObj    = new mainScriptC($go, $parx);

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>";
		 
if ( !glob_isAdmin() )  htmlFoot('Sorry', 'Only an Admin is allowed to use this tool.');  

gHtmlMisc::func_hist( $infoarr['scriptID'], $titleShort, $_SERVER['PHP_SELF'] );

$mainScObj->init( $sql );
$mainScObj->GoInfo( $go, "[<a href=\"".$_SERVER['PHP_SELF']."\">Start again</a>]" );

$mainScObj->getInitData($sql);

if ($mainScObj->parx["method"] == "ALL" ) {
	$mainScObj->doAll ( $sql, $sql2, $go );
} else {
	$mainScObj->doOneVersion($sql, $sql2, $go);
}

if ( $error->printAll() ) {
	//  nothing
} else {
	if ( $go>=2 ) {
		echo "<br>ready.<br>";
		echo 'refreshing cache ...<br>',"\n";
		flush();
		get_cache(1,0);
		$error->reset();
			
		echo "<B>PLEASE <a href=\"../../logout.php\" target=_top>LOGOUT</a> AGAIN!!!</B><br>";
	}
}

htmlFoot("<hr>");