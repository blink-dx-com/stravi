<?php
/**
 * contains methods for convert.php
 * @package convertData.inc
 * @swreq  UREQ:0014545 (FS-ARC02-02) [DB_Haeckelianer] Support upgrade of the database model;
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/rootsubs/db_transform/convertData.inc 59 2018-11-21 09:04:09Z $
 * @var  convDataArray_STRUCT (e.g. dataArray) - contains the data for a single conversion-version
		["info"] "main" => main INFO
		["reqs"] "DbVersionNeed",  (can be EMPTY!)
				   "DbVersionNew"
		['extconvert'] :  => array( 'tables'=>array(tables), 'parts'=> [0],1,2,3,  'transform'=>0,1 )  
				- extended convert
				'tables' => if data in tables 'tables', needs special transformation-script
				'parts'  => number of parts for FULL transform: first do part=0, than parts=1, part=2, ...
				'transform'=> 0,1 call $this->transform(&$sqlo, &$sqlo2, $partno)  ?
		["sql"] : sql commands to CREATE a structure
				: SQL-comments MUST start with SLASH_STAR
		["rem_sql"] : sql commands to DELETE a structure
		["meta"] : intelligent sql : array( act=>("upd", "del", "ins"), "tab"=>..., "vals"=>array(key=>val) )
 */
require_once("o.CCT_TABLE.info.inc");

/**
 * handle many DB-versions
 * @author steffen
 *
 */
class dbConvertC {
	
	private $LAB_transform_dir = NULL;
	
	function __construct() {
		
		$lab_path = $_SESSION['s_sessVars']['AppLabLibDir'];
		$db_convert_dir = $lab_path. '/root/db_transform';
		
		if (file_exists($db_convert_dir)) {
			$this->LAB_transform_dir = $db_convert_dir;
		}
	}

function getVersions() {

	require_once("db_versions.inc");
	$versarr = db_versionsGet_BASIC();
	
	if ($this->LAB_transform_dir!=NULL) {
		require_once($this->LAB_transform_dir . "/db_versions.inc");
		$versarr_lab = db_versionsGet_LAB();
		
		$versarr = array_merge($versarr, $versarr_lab);
	}
	
	return $versarr;
}

/**
 * get all versions for a special type
 * @param string $typex : "norm" : normal version
 						  "intern" : intern versions; remove "i" from string
 */
function getTypeVersions( $typex ) {
	$tmparr    = db_versionsGet();
	$ExtVers   = NULL;
	$newTmpArr = NULL;
	foreach( $tmparr as $dummy=>$versstr) {
		if ( $typex=="norm" and is_numeric(substr($versstr,0,1)) ) {
			$ExtVers[] = $versstr;
		}
		if ( $typex=="intern" and substr($versstr,0,1)=="i" ) {
			$ExtVers[] = $versstr;
		}
	}
	reset($tmparr);
	
	return ($ExtVers);
}

function checkVersion(&$sql, $wantVersion, $needVersion) {
	$errtxt = "";
	$sqls = "select value from globals where name='DBVersion'";
	$sql->query($sqls);
	$sql->ReadRow();
	$thisFullVersion = $sql->RowData[0];
	$main_version = substr($thisFullVersion, 0, 7);
	
	$sqls = "select value from globals where name='DBVersion_LAB'";
	$sql->query($sqls);
	$sql->ReadRow();
	$thisFullVersion = $sql->RowData[0];
	$intern_version = substr($thisFullVersion, 0, 7);
	
	if ( strstr($wantVersion,"i")!="" ) $checkVersion = $intern_version;
	else $checkVersion = $main_version;
	
	// echo "want: $wantVersion need:$needVersion is: $checkVersion<br>";
	
	if ($needVersion=="") return; // need no special version
	
	if ($checkVersion != $needVersion) {
		$errtxt = "This script is designed for DB-version $needVersion";
	} 
	return ($errtxt);
}

function getInternVersion(&$sql) {
	$sqls = "select value from globals where name='DBVersion_LAB'";
	$sql->query($sqls);
	$sql->ReadRow();
	$thisFullVersion = $sql->RowData[0];
	return ($thisFullVersion);
}

function getDBVersion(&$sql) {
	$sqls = "select value from globals where name='DBVersion'";
	$sql->query($sqls);
	$sql->ReadRow();
	$FullVersion = $sql->RowData[0];
	return ($FullVersion);
}



}


/**
 * abstract class for one version in dir data/
 * is called like that:
 * 
 * ... for one transformation-PART:
	$dataarr = $this->GetData( $datoption );
	perform_sql_statements($dataarr);
	$this->transform($sqlo, $sqlo2, $part, $go);
	
}
 * 
 * 
 * @author steffen.kube@gmx.de, steffen@blink-dx.com
 *
 */
class gDB_transAbstract {
    
   
    var $dataArray; // convDataArray_STRUCT
	
	function GetData( &$dataArray, $datoption=NULL ) {}
	
	/**
	 * keep in mind, that in your code, $go must be analysed, otherwise, in check-status (go=0) of the script,
	 * the transform-code will be executed
	 * @param object $sqlo
	 * @param object $sqlo2
	 * @param int $part
	 * @param int $go (0,1)
	 */
	function transform($sqlo, $sqlo2, $part, $go) {}
	
	/**
	 * get basic information, like user-info, required version, ...
	 * @return array 
	 *   ["info"] "main" => main INFO
		 ["reqs"] "DbVersionNeed",  (can be EMPTY!)
				  "DbVersionNew"
	 */
	function getBasicInfo() {}
	
	/**
	 * put some info out to display
	 * @param string $text
	 */
	function infoLine($text) {
		echo 'INFO:'.__CLASS__.':'.$text."<br>\n";
	}
}

/*
 * activate one VERSION CLASS
 */
class dbConvOneVersCls {
	
function _info($text) {
	$this->infolog[]=$text;
}
	
/**
 * init one DB-version
 * - load the DB-Version-Transform-file "${version}.inc"
 * - the DBVTF can be situated in directory "data" or "{LAB}/root/db_transform/data"
 * - initiate the class
 * @param string $version
 * @return number
 */
function initVers( $version ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$this->verclass = NULL;
	$this->infolog  = NULL;
	
	$lab_path = $_SESSION['s_sessVars']['AppLabLibDir'];
	$LAB_db_convert_dir = $lab_path. '/root/db_transform';
	
	//
	// try to find the code file int the BASIC or LAB dir
	//
	$BASIC_file = "data/".$version .".inc";
	$file_found = 0;
	if ( file_exists( $BASIC_file) ) {
		require_once( $BASIC_file );
		$file_found = 1;
	} else {
		if ( file_exists($LAB_db_convert_dir) ) {
			$LAB_file = $LAB_db_convert_dir . '/'. $BASIC_file;
			if ( file_exists( $LAB_file) ) {
				require_once( $LAB_file );
				$file_found = 1;
			}
		}
	}
	if (!$file_found) {
		$error->set( $FUNCNAME, 1, 'For BASIC or LAB: no DB-Version-Transform-file "'.$BASIC_file.'" found.' );
		return;
	}
	
	$classVersion = str_replace( ".","_", $version);
	$classnamex   = "gDB_tranData_" . $classVersion;
	$this->verclass = new $classnamex();
	
	return (1);
}

/**
 * get SQL-data 
 * @param  array $datoption=NULL 
 * 			 "onlyparts" = 1,2,3 ...
 * 			['sql']  = &$sqlo; 
 *			['go']   = 0,1
 * @return array convDataArray_STRUCT $dataArray()
 * 
 * 
 * 
							  
*/
function GetData( $datoption=NULL ) {
	
	$dataArray = NULL;
	
	$dataBasic = array();  // basic information 
	if ( method_exists($this->verclass,'getBasicInfo') ) {
		$dataBasic = $this->verclass->getBasicInfo();
	}
	
	$this->verclass->getData( $dataArray, $datoption );
	if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
		echo "DEBUG: GetData(): size(sql): ".sizeof($dataArray["sql"]).
		  " onlyparts: ".$datoption["onlyparts"]."<br>\n";
	}
	$dataArray = array_merge($dataArray,$dataBasic);
	
	return ($dataArray);
}

/**
 * 
 * @return int $retval 0,1 transform exists?
 * @param object $sqlo
 * @param object $sqlo2
 * @param object $part
 * @param object $go 0,1
 */
function checkTransform( &$sqlo, &$sqlo2, $part, $go ) {
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    $t_exists=0;
	$retval = 0;
	if ( method_exists($this->verclass,'transform') ) {
	    $t_exists=1;
		$this->_info('transformation exists');
		$this->verclass->transform($sqlo, $sqlo2, $part, $go);
		$retval = 1;
	} 
	debugOut('TRANSFORM_exists ? '.$t_exists, $FUNCNAME,2 );
	
	return ($retval);
}

}
