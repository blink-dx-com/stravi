<?php
/**
 * main module for a single unittest
 * - must call initClass() before using
 * @package unittest_onetest.inc
 * @swreq UREQ:30; FS-ID:FS-REG02-t2 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
require_once 'test_config.inc';
require_once ("f.xmlrpc_cli.inc");
require_once ("insertx.inc");
require_once ('f.wiid_roid.inc');

/**
 * abstract class for UNIT tests
 */
class gUnitTestSub {

	private $_resultCache; // array of results, updated in _saveTestResult()
	private $module_noPreLoad=0;  // 0,1 - do NOT preload ? can be set in constructor of class
	private $GUI_test_flag=0;   // 0,1 - is this a GUI based test, which needes user interaction ?
	private $config_unittest;
	var $useObjexts;
	
	/**
	 * 
	 * @var $parx
	 *  'infolevel'
	 */
	var $parx;

	function __construct() {
	}
	
	function _setParx($parx) {
		$this->parx = $parx;
	}
	
	/**
	 * get config values, identified by test-key
	 * - get from $this->config_unittest[$testkey]
	 * @param string $testkey
	 */
	function _get_test_configval($testkey) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		//Old: ['testparams']
		if (!isset($this->config_unittest[$testkey])) {
			$error->set( $FUNCNAME, 1, 'KEY "'.$testkey.'" not set in UnitTest-Config.' );
			return;
		}
		return $this->config_unittest[$testkey];
	}

	/**
	 * 
	 * @param object $sqlo
	 * @param object $sqlo2
	 * @param array $config_unittest
	 */
	function initClass(&$sqlo, &$sqlo2, &$config_unittest) {

		$configlib = new unittest_config();
		
		$this->useObjexts = $configlib->get_objects();
		$this->useObjectFeatures = $configlib->get_object_features();
		$this->_infotxt = NULL;

		$this->config_unittest = &$config_unittest;

		$this->slqoInt  = &$sqlo;
		$this->slqo2Int = &$sqlo2;

		$this->_resultCache=array();
		
		
	}
	
	/**
	 * lazy init, load default config-file
	 * @param object $sqlo
	 * @param object $sqlo2
	 */
	function initClass_lazy(&$sqlo, &$sqlo2) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $config_unittest = array();
	    $configFile = $_SESSION['s_sessVars']['AppRootDir'].'/config/config.unittest.inc';
	    if (!file_exists($configFile)) {
	        $error->set( $FUNCNAME, 5, "UnitTest-configfile not found." );
	        return;
	    }
	    include($configFile); // load $config_unittest
	    
	    
	    $this->initClass($sqlo, $sqlo2, $config_unittest); // $config_unittest comes from config.unittest.inc
	}
	
	/**
	 * XML-RPC call, including connect
	 * @param string $funcname
	 * @param array $params
	 * @return array
	 */
	function _xmlrpc_call($xml_funcname, $params) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		
		require_once ("../../../../config/xml_rpc_tests.inc");
		$connect_sys_array = xml_rpc_testLogins();
		if (!is_array($connect_sys_array)) {
			$error->set( $FUNCNAME, 1, 'no xmlrpc connect configured in xml_rpc_tests.inc.' );
			return;
		}
		$target = $this->_get_test_configval('xmlrpc.target');
		if ($target==NULL) {
			$error->set( $FUNCNAME, 2, 'no target configured in UNITTEST-config.' );
			return;
		}
		
		$connect_params = $connect_sys_array[$target];
		if (!is_array($connect_params)) {
			$error->set( $FUNCNAME, 3, 'target "'.$target.'" not configured.' );
			return;
		}
		
		
		$obj1 = new fXmlrpcCli( $connect_params );
		$obj1->connect(0); 
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 4, 'Connect error.' );
			return;
		}
		
		$answer = $obj1->funcCall( $xml_funcname, $params ); // can set $error
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 5, 'run error.' );
			return;
		}
		return $answer;
	}
	
	/**
	 *
	 * check, if object exists (check COLUMN: $check_column=$check_val
	 * if not, create it
	 * @param object $sqlo
	 * @param string $table
	 * @param string $check_column e.g. NAME
	 * @param string $check_val e.g. UnitTest_TestXYZ
	 * @param array $ins_params
	 *      array(
				"vals"=> array(
						'NAME'=>xxx,
						'ABS_WORKLIST_ID'=>xxx
				)
	 */
	function _createObj_checkExists($sqlo, $table, $check_column, $check_val, $ins_params) {
		
		$pkname = PrimNameGet2($table);
		$obj_id = glob_elementDataGet( $sqlo, $table, $check_column, $check_val, $pkname);
		
		if ($obj_id) {
			return $obj_id;
		}
		
		// create document
		$insertlib = new insertC();
		$obj_id   = $insertlib->new_meta($sqlo, $table, $ins_params);
		return $obj_id;
	}

	function _getFuncStack() {

		$retarr=NULL;
		$tmparr = debug_backtrace();
		next($tmparr); // remove one level

		$cnt=0;
		foreach( $tmparr as $id=>$content) {
			$filex = basename($content["file"]);

			$outarr=NULL;
			$outarr[1]=$filex;
			$outarr[2]=$content["function"];
			$outarr[3]=basename($content["line"]);
			$retarr[] = 'file:'.$outarr[1]. ' func:'.$outarr[2].
			' line:'.$outarr[3];
				
			$cnt++;
		}
		reset($tmparr);

		return $retarr;
	}

	/**
	 * show error immediatly, but NO stop ...
	 * */
	function _stop($info=NULL) {
		$stack = $this->_getFuncStack();
		htmlInfoBox( "Unit test error", "", "open", "ERROR" );
		if ($info!=NULL) echo 'Info: '.$info.'<br>';
		glob_printr( $stack, "stack info" );
		htmlInfoBox( "", "", "close" );
		
		throw new Exception("Unit test error: ".$info);
	}

	/**
	 * save result in test cache
	 * @param string $testid
	 * @param int $result : 0: error, 1: ok
	 */
	function _saveTestResult($testid, $result, $infotext=NULL) {
		$this->_resultCache[] = array($testid, $result, $infotext);
	}
	
	/**
	 * store, that this test will be defined later ...
	 * @param string $testid
	 * @param int $result : 0: error, 1: ok
	 */
	function _saveTest_def_later($testid, $infotext=NULL) {
	    $testid .= ':TBD';
	    $result  = 1;
	    $this->_resultCache[] = array($testid, $result, $infotext);
	}
	
	/**
	 * compare values ($val_expect,$val_result)  and save result in test cache
	 * @param string $testid
	 * @param $val_expect
	 * @param $val_result
	 * @param string $infotext
	 */
	function _compareTestResult($testid, $val_expect, $val_result, $infotext=NULL) {
		if ($val_expect===$val_result)  $result=1;
		else $result=0;
		$this->_resultCache[] = array($testid, $result, $infotext);
	}
	
	/**
	 * @DEPRECATED
	 * get object ID by TABLE, SYNONYM
	 * - no further checks
	 * @param string $table
	 * @param string $synonym
	 * @return int
	 */
	private function _getDefObjID_RAW( $table, $synonym ) {
		
	    die('Method is not supported anymore.');
	    /*
		$objid = $this->useObjexts[$table][$synonym];
		if (!$objid) {
			$this->_stop('no object defined in useObjexts[] for table "'.$table.'" and  synonym "'.$synonym.'"');
		}
		return ($objid);
		*/
	}

	/**
	 * get object ID by TABLE, SYNONYM
	 * - forward now to function _getExistObjID()
	 * @param string $table
	 * @param string $synonym
	 * @return int
	 */
	function _getDefObjID( $table, $synonym ) {
		/*
		$objid = $this->useObjexts[$table][$synonym];
		if (!$objid) {
			$this->_stop('no object defined in useObjexts[] for table "'.$table.'" and  synonym "'.$synonym.'"');
		}
		*/
		$objid = $this->_getExistObjID( $table, $synonym );
		return ($objid);
	}
	
	/**
	 * get predefined object features for a $synonym
	 * @param string $table
	 * @param string $synonym
	 * @param string $feature
	 * @return string
	 */
	function _getDefObjAttribute( $table, $synonym, $feature ) {
		
		if ( !is_array($this->useObjectFeatures[$table][$synonym]) ) {
			$this->_stop('no data found for table "'.$table.'", synonym "'.$synonym.'"');
			return NULL;
		}
		
		if ( !array_key_exists($feature, $this->useObjectFeatures[$table][$synonym]) ) {
			$this->_stop('no feature defined for table "'.$table.'", synonym "'.$synonym.'", feature:'.$feature.'"');
			return NULL;
		}
		$value = $this->useObjectFeatures[$table][$synonym][$feature];
		
		return ($value);
	}
	
	/**
	 * get object by VARIO synonym
	 * @param object $sqlo
	 * @param string $tablename
	 * @param string $synonym
	 * @return int
	 */
	function _getDefObj_by_vario_key(&$sqlo, $tablename, $synonym) {

	    $VARIO_KEY = 'UnitTest_Synonym';
	    $sqlo->Quesel("OBJ_ID FROM S_VARIO WHERE TABLE_NAME=". $sqlo->addQuotes($tablename).
	        " AND KEY="   . $sqlo->addQuotes($VARIO_KEY)  .
	        " AND VALUE= ". $sqlo->addQuotes($synonym) ) ;
	    $sqlo->ReadRow();
	    $objid = $sqlo->RowData[0];
	    return $objid;
	    
	}

	/**
	 * get objid; check, if object exist
	 * UT-object defined by: VARIO: UT_NAME: $synonym
	 * @param string $table
	 * @param string $synonym
	 * @return int ID
	 */
	function _getExistObjID( $table, $synonym ) {
		
	    $objid = $this->_getDefObj_by_vario_key($this->slqoInt, $table, $synonym);

	    if (!$objid) $this->_stop('Object t:'.$table.' synonym:'.$synonym.' does not exist in DB.');
		
		return $objid;
	}
	
	/**
	 * get TEST DATA path
	 * @return string
	 */
	static function _getTestDir_path( ) {
		$pathx = realpath(dirname(__FILE__).'/../test_data');
		return ($pathx);
	}
	


	/**
	 * create a test object
	 * @param string $table
	 * @param array $data
	 * @return int ID
	 */
	function _createObj( $tablename, $args ) {
		require_once ("insertx.inc");

		$newlib = new insertC();
		$objid = $newlib->new_meta($this->slqoInt, $tablename, $args);

		return $objid;
	}

	/**
	 * print info NOW, without any other ending chars
	 * - show on 'infolevel'>=1
	 * @param $text
	 */
	function _infoRaw( $text ) {
		if ($this->parx['infolevel']<1) return;
		echo $text;
	}

	/**
	 * print info NOW + BR
	 * - show on 'infolevel'>=1
	 * @param $status : ['ok'], 'error'
	 */
	function _infoNow( $key, $text, $status=NULL ) {
		if ($this->parx['infolevel']<1) return;
		
		$status_nice='';
		if ($status=='error') $status_nice='<span style="background-color:red; padding:3px;">Error</span> ';
		if ($status=='ok') $status_nice='<span style="background-color:#C0FFC0; padding:3px;">OK</span> ';
		
		echo '<b>INFO:</b> '.$status_nice.$key.': '. $text."<br />\n";
	}

	function _info( $key, $text ) {
		$this->_infotxt[] = array($key, $text);
	}
	
	/**
	 * - show on 'infolevel'>=1
	 * @param array $outArray
	 *   e.g. key=>val
	 */
	function _infoTable( $title, $outArray ) {
	    if ($this->parx['infolevel']<1) return;
	    
	    echo "TABLE: $title<br>";
	    echo '<table>'."\n";
	    foreach($outArray as $key=>$val) {
	        echo '<tr>';
	        echo '<td>'.$key.'</td>';
	        if (is_array($val)) {
	            $val_out = print_r($val,1);
	        } else {
	            $val_out = $val;
	        }
	        echo '<td>'.$val_out.'</td>';
	        echo '</tr>'."\n";
	    }
	    echo '</table><br>'."\n";
	}

	/**
	 * - show on 'infolevel'>=1
	 * @param array $outArray
	 */
	function _infoTabRow( $outArray ) {
		if ($this->parx['infolevel']<1) return;

		echo '<tr>';
		foreach( $outArray as $val) {
			echo '<td>'.$val.'</td>';
		}
		
		echo '</tr>'."\n";
	}

	function _getSqlo2() {
		return $this->slqo2Int;
	}

	/**
	 * - compare array values
	 * @param $arr1 - input
	 * @param $arr2 - output
	 */
	function _compareArrayVals($arr1, $arr2) {
	    
	    $diffarr = array_diff_assoc($arr1, $arr2);
	    if (!sizeof($diffarr)) $diffarr = NULL;
		return ($diffarr);
	}
	
	/**
	 * - compare array values
	 * @param $arr1 - input
	 * @param $arr2 - output
	 */
	function _compare_Array_floatVals($arr1, $arr2) {
	    
	    $diffarr = NULL;
	    
	    if (!is_array($arr1)) {
	        $arr1 = array();
	    }
	    
	    foreach($arr1 as $key => $val) {
	        
	        if ( is_numeric($val) ) {
	            if (floatval($val) != floatval($arr2[$key]) ) {
	                $diffarr[$key] = $arr2[$key];
	            }
	        } else {
	            if ($val != $arr2[$key]) {
	                $diffarr[$key] = $arr2[$key];
	            }
	        }
	    }
	    return ($diffarr);
	}

	/**
	 * - array has NON-Null values ?
	 * @param array &$inarr
	 */
	function _checkArrNotNull( &$inarr ) {
		$hasVals = 0;
		if ( !is_array($inarr) ) return $hasVals;

		reset($inarr);
		foreach( $inarr as $key=>$val) {
			if ( $val !== NULL ) {
				$hasVals = 1;
				break;
			}
		}
		reset ($inarr);
		return $hasVals;
	}

	/**
	 * open a browser window
	 * @param string $relurl the relative URL from PARTIROOT/www
	 */
	function _openWindow( $relurl, $params=NULL ) {
		

		$fullurl  = $_SESSION['s_sessVars']['DocRootURL'];
		$fullurl .= '/' . $relurl;
		if ( $params!=NULL )  $fullurl .= '?'.$params;

		echo "\n".
				"<script language=\"JavaScript\"> \n".
				"	xopenwin(\"".$fullurl."\"); \n".
				"</script>\n";

	}

	public function getTestResults() {
		return $this->_resultCache;
	}

	function getInfoArr() {
		return ($this->_infotxt);
	}

	/**
	 *
	 * @param $sqlo
	 * @param $options <pre>
	 *  'subTest' = name of subtest
	 * </pre>
	 * @return int 0,1
	 */
	function dotest( &$sqlo, $options ) {}

}

class gUnitTestC {
	var $sqloIntern;  // inter SQL-handle
	var $sqloIntern2; // inter second SQL-handle
	
	/**
	 * @var array $parx
	 *  "modpath"
	 *  "subTest" name of sub-test for module
     *  "infolevel" int; level of info: 0-low, 3-high 
	 */
	var $parx;

	function __construct( &$sqlo, &$sqlo2, $parx, $go ) {

		$this->sqloIntern  = &$sqlo;
		$this->sqloIntern2 = &$sqlo2;
		$this->parx = $parx;
		$this->go   = $go;
		$this->testDir = dirname(__FILE__).'/../src'; // base of UnitTest src modules
		$this->oriDir  = dirname(__FILE__).'/../..';  // base of original src files
	}
	
	/**
	 * initialize test
	 * @param object $sqlo
	 * @return void
	 */
	function test_init(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->unitClasObj = NULL;
		
		$modpath = $this->parx["modpath"];
		$UT_moName  = basename($modpath);
		$UT_dirName = dirname  ( $modpath );
		$UT_moName  = $UT_dirName . '/UT_'.$UT_moName;
		
		$mo_fullpath = $this->oriDir  ."/".$modpath;
		$ut_fullpath = $this->testDir ."/".$UT_moName;
		$moHuman = realpath($this->oriDir);
		$utHuman = realpath($this->testDir);
		
		if ( !file_exists($mo_fullpath) ) {
			$error->set( $FUNCNAME, 1, "OriginalModule '".$mo_fullpath."' not found. dir:'".$moHuman."'" );
			return;
		}
		
		if ( !file_exists($ut_fullpath) ) {
		    $error->set( $FUNCNAME, 3, "ModuleTest '".$ut_fullpath."' not found in test-environment.dir:'".$utHuman."'" );
			return;
		}
		
		// before here include ...
		require_once($ut_fullpath);
		
		$testClass_1   = basename($modpath);
		$testClassName = "UT_" . str_replace(".","_", $testClass_1);
		
		$unitClasObj = new $testClassName();
		if ($unitClasObj==NULL) {
			$error->set( $FUNCNAME, 2, "Could not create class '$testClassName'." );
			return;
		}
		
		$config_unittest = array();
		$configFile = $_SESSION['s_sessVars']['AppRootDir'].'/config/config.unittest.inc';
		if (!file_exists($configFile)) {
			$error->set( $FUNCNAME, 5, "UnitTest-configfile not found." );
			return;
		}
		include($configFile); // load $config_unittest
		
		
		$unitClasObj->initClass($sqlo, $this->sqloIntern2, $config_unittest); // $config_unittest comes from config.unittest.inc
		$unitClasObj->_setParx($this->parx);
		
		// import modules
		$guimod	 = $unitClasObj->module_noPreLoad;
		if ( !$guimod ) require_once($mo_fullpath);
		
		$this->unitClasObj = &$unitClasObj;
	}
	
	/**
	 * execute a test
	 * @param object $sqlo
	 */
	function test_execute(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$unitClasObj = &$this->unitClasObj;
		
		$options=NULL;
		if ($this->parx['subTest']!=NULL) {
			$options['subTest']=$this->parx['subTest'];
		}
		$answer = $unitClasObj->dotest( $sqlo, $options );
		
		$infoarr = $unitClasObj->getInfoArr();
		if ($infoarr!=NULL and $this->parx['infolevel']>=1) {
			glob_printr( $infoarr, "UnitTest-info" );
		}
		
		// analyse cache
		$errorcnt   = 0;
		
		$testResults = $unitClasObj->getTestResults();
		if ($this->parx['infolevel']>1) {
			echo 'TestResults: <br>'."\n";
		}
		
		if (sizeof($testResults)>0) {
			$badTests=NULL;
			
			foreach( $testResults as $dummy=>$valarr) {
				if ($valarr[1]<1) {
					$errorcnt++;
					$badTests[]=$valarr[0];
				}
		
				if ($this->parx['infolevel']>1) {
					echo ' - ';
					echo $valarr[0].': '.$valarr[1].': '.$valarr[2]."<br>\n";
				}
			}
		
		}
		
		if ($answer!=1 or $errorcnt) {
			if ($errorcnt) {
				$answer = 'BadSubTests: '. implode(', ',$badTests);
			}
			$error->set( $FUNCNAME, 4, "UnitTest failed. ERR-No: ".$answer );
			return;
		}
	}



	/**
	 * - INIT + EXCEUTE a unit test
	 * - include the unittest-module
	 * @param object &$sqlo
	 */
	function dotest(&$sqlo) {
		global $error;
		$FUNCNAME= "dotest";

		$this->test_init($sqlo);
		if ($error->Got(READONLY))  {
			return;
		}

		$this->test_execute($sqlo);
	}
	
	function is_gui_module() {
		return $this->unitClasObj->GUI_test_flag;
	}

}
