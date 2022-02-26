<?php
/**
 * XMLRPC Unittests
 * @swreq   0000778: Concept for XML-RPC-Unittests 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)		
 * @package  f.xmlrpc.test.inc  
 */



/**
 * abstract class for META-methods in directory "xmlrpc_sub"
 * @author steffen
 *
 */
class BASE_TSTxrpc {
	
	/**
	 * @var array STRUCT_xmlrpc_infoarr
	 */
	private $testinfo;
	public  $subtest; // string : sub test
	public  $params;  // string extra parameters
	
	function _init(&$sqlo, &$XmlCliObj, $subtest, $params, $metacallFunc, &$useObjexts) {
		$this->sqlo    = &$sqlo;
		$this->subtest = $subtest;
		$this->params  = $params;
		$this->metacallFunc = $metacallFunc;
		$this->useObjexts = &$useObjexts;
		$this->XmlCliObj  = &$XmlCliObj;
	}
	
	function _getDefObjID( $table, $synonym ) {
		$objid = $this->useObjexts[$table][$synonym];
		return ($objid);
	}
	
	function outinfo($text) {
		echo "- $text<br>\n";
	}
	
	function getMetacallFunc() {
		return $this->metacallFunc;
	}
	
	/**
	 * abstract method
	 * get the main parameter dictionary for the XML-RPC method
	 * @return $paramFunc
	 */
	function getXmlrpc_params() {}
	
	/**
	 * abstract method; returns optional array of subtest keys
	 * example: array('CAB', 'TT1');
	 * @return array $subTestKeys
	 */
	function getSubTestKeys() {}
	
	/**
	 * abstract method; returns parameter information string
	 * example: 'SubTest: give ID of EXP_TMPL'
	 * @return  array STRUCT_xmlrpc_infoarr $infoarr
	 *  'nice' => nice name of test
	 * 	'info' => general info
	 *  'subtest' =>
	 *  'params'  =>
	 *  'expectErr' => expected error code of XMLRPC-method
	 */
	function getParamInfo() {
		return $this->testinfo;
	}

}




/**   
 * - test for xmlrpc-functions
   - define test-data for xmlrpc-test
	
  add new test for a XML-RPC function:
  - add function name to $this->funcs
  - if you need an object from the DB, add it to $this->useObjexts	
  - add a switch/case entry for the function in the method doAction()
  - the special "case" entry must return $params = array( xxx );

 */
class fXmlrcpFuTest {

var $CLASSNAME     = "fXmlrcpFuTest";
private  $test_base_dir=NULL;

/**
 * init
 * @param  array $XmlCliObj VARSTRUCT_fXmlrpcCli , can be NULL
 * @param $parx
 *   'param' - optional method params
 */
function __construct( &$XmlCliObj, $parx, $debug=NULL ) {
	
	$this->parx  = $parx;
	$this->debug = 0;
	$this->test_base_dir = dirname(__FILE__);
	
	if ( $debug>0 ) {
		$this->debug = $debug;
	}
	
	$this->useObjexts = array(
		//"ABSTRACT_PROTO"=> 	array("assay" => 3900 ),
		"ABSTRACT_PROTO"=> 	array("default" => 12327, "assay" => 12327 ),
		"ABSTRACT_ARRAY"=> 	array("default" => 120 ), // with alias names
		"EXP" => array(
				"default" => 55500,
				"cytoRes" => 55500,
				"update"  => 1789,
				"exp_update_script" => 7660,
				"noWriteAccess" => 295030 
					  ),	
		"EXP_TMPL" => array("default" => 100 ),		
	    "IMG" => array("default" => 120206 ),			 
		"LINK" => array("exp_update_script" => 15670 ),
		
		"CHIP_READER"=> 	array("keyence" => 100, 'CAB'=>929, 'TT1'=>1015 ),
		"CONCRETE_PROTO"=> 	array("default" => 49940 ), // with steps
		"CONCRETE_SUBST"=> 	array("default" => 1 ), // with steps
		"CART_BATCH"=> 	array("default" => 674, "cart_batch_cartInf1"=>7, "expiryTest"=>268 ),
		"CYTO_ORDER"=> array("default" =>44),	
		"PROJ"	=> 		array("default" => 100 ),	
		"RESX"	=> 		array("update" => 11 ),
		"SOCKET"  => array("default" => 1 ),	
		"W_WAFER" => array(
			"default" => 100,
			"default2" => 1,
			)
 
			);
	if ( $XmlCliObj!=NULL ) $this->setClient($XmlCliObj);
}

function getFuncList() {
	
	/* array(funcname) = 
	 *   1:                one test
	 *   array(subtests) : many tests exist
	 * 
	 */
	$this->funcs = array(
		"connect" => 1,
		"connect2" => 1,
		"batch_qa_set"=>1,
		"cronjob"=>1,
		"array_socket_connect"=>1,
		"cart_batch_barget"=>1,
		'cart_batch_cartInf1'=>1,
		"cart_batch_qaset"=>1,
		"count_spots"=>1,
		"count_spots_layout"=>1,
  		"delete_spot_res"=>1,
		"delete_raw_res"=>1,
		"delete_ref_points"=>1,
		"exp_create_macro"=> array("bad-cartname", "allnew", "129.PIMA.labor", "129.PIMA.assemb", "HIVNAT", "wrongProj"),
		"exp_create"      => array("no-project"),
		"exp_get_scrdev"  => array("nischt"),
		"exp_get_script"=>1,
		"exp_get_script_device"=>1,
		"exp_img_save"=>1,
		"exp_update_scrdev"=>array("unlink","newhash"),
		"exp_update_script"=>array("NewDocument", "NewDocument2"),
		"exp_proto_upd" => array('err1', 'err2', 'err3', 'err4'),
		"g_getfunclist" => 1,
		"g_sessionalive"=> array('err'),
		"get_a_probe_on_an_array"=>1,
		"get_a_probe_on_an_array_subst"=>1,
		"get_array_lay"=>1,
		"get_exp_imgs"=>1,
		"get_exp_notes"=>1,
		"get_exp_tmpl_notes" => 1,
		"get_href_pos_sys2"  =>array('oneSpot'),
		"get_href_pos_sys"  =>1,
		"get_proto_list"=>1,
		"get_refp_pos"=>1,
		"get_version" => 1,
		'gObj_getIdByName'=>1,
  		
  		"insert_ref_points2"=>1,
		"insert_spot_res_bulk"=>1,
		"metacall"=>1,
		"obj_get_params"=>array("normal", "input"),
		"query"=>1,
		"socket_getarr"=>1,
		"update_exp_chan_res"=>1,
		"update_exp_norm_result"=>1,
		"update_exp_params"=>1,
		"update_exp_raw_result"=>array("noData", "missData", "2channels"),
		"update_exp_raw_result_bulk"=>array("noData", "missData"),

    	"wafer_seclab"  => 1,
    	"wafer_step_updoc"=>1,
		"XMLFUNC_NOT_EXISTS"=>1,
		"_module_test"=>array('no_answer_from_server', 'db_login_error'),
		);
		
		$this->metacall=array(
			'DEF/oCHIP_READER_createProto'=> 1,
			'DEF/oEXP_get_absArray'=> 1,
		
			'DEF/oEXP_get_absArray'=>1,
			'DEF/gAdmin_getSessVars'=>1,
			'DEF/gObj_addAttach'=>1,
			'DEF/gObj_getParams'=>1,
			'DEF/gObj_getAssParams'=>1,
			'DEF/gObj_updParams'=>1,
			'DEF/oABSTRACT_ARRAY_getSubstIds'=>1, 
			'DEF/oEXP_test'=>1, 
			'DEF/oGLOBALS_update'=>1,
			'DEF/oEXPTMPL_getNotes'=>1, 
			'DEF/oEXP_TMPL_getScript'=>1,
			'DEF/oPROJ_getObjects'   =>1,
			'ERR/test'=>1,
			
			'LAB/gSAPWE'=>1,
			'LAB/gSAP_MM_getstock'=>1,
			'LAB/gSPC_QC_detect'=>1,
			'LAB/oCHIP_READER_getLastProto'=>1,
			'LAB/oOEE_one_day'=>1,
			'LAB/oRESX_updWafers'=>1,
			'LAB/p129_createSocket'=>1,	
			'LAB/p129_devGetParams'=>1,
			'LAB/p129_devSaveData'=> 1,
			'LAB/p129_get_cyto_order'=>1,
			'LAB/p129_getPosOfProc'=>1,
			'LAB/p129_finish_order'=>1,
			'DEF/oEXP_projProto_connect'=>1,
			'DEF/oPROJ_getIdByDate'=>1,
			'DEF/oPROJ_getElems'=>1,

			//'LAB/p131_CABNATInfo'=>1,
			'LAB/p098_printLabel'=> array('PARAM', 'prod', 'userQA0', 'userQA1', 'userQAN', 'userNonHVCart', 'wrap'),
			'LAB/p098_getNameParamsBySerial'=>array('PARAM', 'noParam', 'invalidParam', 'noCarttype', 'noArrayBatch'),
			'LAB/p098_getDMC'    => array('default'),
			'LAB/oSUC_computer_exp'=>1,
			'LAB/test_get_data'=>1,
			'DEF/oEXP_getResults'=> array('PARAM'),
			'DEF/oIMG_getURL'=> array('PARAM'),
			'DEF/oEXP_close'=>1,
			'DEF/oEXP_isClosed'=>1,
			
				
			/*
			'LAB/p131_printLabel'=>1,
			'LAB/p098_getNameParamsBySerial'=>1,
			*/
			'LAB/oPROJ_getIdByDate'=>1
				
				
		);
		
		$this->funcs['--- metacall ---']=1;
		foreach ( $this->metacall as $key=>$val ) {
			$this->funcs['META/'.$key]=$val;
		}
		 
		
		
	return ($this->funcs);
}

function setClient(&$XmlCliObj) {
	$this->XmlCliObj = &$XmlCliObj;
}

function outinfo($text) {
	echo "- $text<br>\n";
}

function _getDefObjID( $table, $synonym ) {
	$objid = $this->useObjexts[$table][$synonym];
	return ($objid);
}

/**
 * analyse METACALL
 * - include a module: $metacallFunc ".inc" (replace "/" by "_")
 * - base-class name_TSTxrpc
 * @param $sqlo
 * @param $metacallFunc
 * @param $subtest : use 'PARAM', if $this->parx['params'] should be taken
 * @param $opt
 * @return array( 
 * 	 $metacallFunc, 
 *   $paramFunc, 
 *   $infoarr  STRUCT_xmlrpc_infoarr info-structure
 *   )
 * 	 
 *   
 */
function _doMeta(
	&$sqlo, 
	$metacallFunc, 
	$subtest = NULL, // e.g. "noData"
	$opt     = NULL	
	) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;	
	
	$baseName   = str_replace("/", "_", $metacallFunc);
	$className  = $baseName.'_TST';
	$modul_name = $this->test_base_dir . "/UT_META/" .$baseName. ".inc";	
	
	if (!file_exists($modul_name)) {
		$error->set($FUNCNAME, 1, 'ModulTest nicht definiert. Modul-Name: "'.$modul_name.'"' );
		return;
	}
		
	require_once($modul_name);
	
	$testClass = new $className();
	$testClass->_init($sqlo, $this->XmlCliObj, $subtest, $this->parx['params'], $metacallFunc, $this->useObjexts);
	
	$funcInfoArr = $testClass->getSubTestKeys();
	if (is_array($funcInfoArr)) {
		$this->outinfo("Available Subtests: <font color=#8080FF><b>". implode(", ",$funcInfoArr).'</b></font>');
	}
	
	
	$paramFunc    = $testClass->getXmlrpc_params();
	$metacallFunc = $testClass->getMetacallFunc();
	$infoarr	  = $testClass->getParamInfo();
	
	return array( $metacallFunc, $paramFunc, $infoarr );
}

/**
 * analyse normal test
 * - include a module: "xmlrpc/" + $methodName ".inc" (replace "/" by "_")
 * - base-class name_TSTxrpc
 * @param $sqlo
 * @param $methodName
 * @param $subtest : use 'PARAM', if $this->parx['params'] should be taken
 * @param $opt
 * @return array( 
 * 	 $methodName, 
 *   $paramFunc, 
 *   $infoarr
 *   )
 * 	 
 *   
 */
private function _doNormal(
	&$sqlo, 
	$methodName, 
	$subtest = NULL, // e.g. "noData"
	$opt     = NULL	
	) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;	
	
	$baseName   = str_replace("/", "_", $methodName);
	$className  = $baseName.'_TST';
	$modul_name = $this->test_base_dir . "/UT_xml_functions/" .$baseName. ".inc";
	
	if (!file_exists($modul_name)) {
		$error->set($FUNCNAME, 1, 'ModulTest nicht definiert. Modul-Name: "'.$modul_name.'"' );
		return;
	}
	
	require_once($modul_name);
	
	$testClass = new $className();
	$testClass->_init($sqlo, $this->XmlCliObj, $subtest, $this->parx['params'], $methodName, $this->useObjexts);
	
	$funcInfoArr = $testClass->getSubTestKeys();
	if (is_array($funcInfoArr)) {
		$this->outinfo("Available Subtests: <font color=#8080FF><b>". implode(", ",$funcInfoArr).'</b></font>');
	}
	
	
	$paramFunc    = $testClass->getXmlrpc_params();
	$methodName   = $testClass->getMetacallFunc();
	$infoarr	  = $testClass->getParamInfo(); // $infoarr  STRUCT_xmlrpc_infoarr info-structure
	
	
	if ($this->debug>0) {
		$this->outinfo("Method-Params:<pre>" . 
				'Method-Name: '.$methodName."\n".
				'Method-Params: '."\n".print_r($paramFunc,1). "</pre>");
	}
	
	return array( $methodName, $paramFunc, $infoarr);
}

/**
 * - analyse $xmlfuncName
 * - perform the xmlrpc request
 * @param $sqlo
 * @param $xmlfuncName   e.g. "update_exp_params"
 * @param string  $subtest  define a subtest : e.g. "noData"
 * @param $opt
 * @return array($answer, $testinfo)
 *   $testinfo: array of
 *     'nice' => nice name of Test
 *     'info  => info to test
 */
function doAction(
	&$sqlo,
	$xmlfuncName,  
	$subtest = NULL,
	$opt     = NULL	  
	) {
	global $error;
	$FUNCNAME = "doAction";
	
	$testinfo=NULL;
	
	$optInputParams = $this->parx['params'];
	
	$this->outinfo("Action: <b>".$xmlfuncName."</b> Subtest: <b>".$subtest."</b>");
	$this->getFuncList();
	$funcInfoArr = $this->funcs[$xmlfuncName];
	if (is_array($funcInfoArr)) {
		$this->outinfo("Available Subtests: <font color=#8080FF><b>". implode(", ",$funcInfoArr).'</b></font>');
	}
	
	if (substr($xmlfuncName,0,5)=='META/') {
		$xmlfuncNameUse = 'metacall';
		$metacallFunc   = substr($xmlfuncName,5);
		$params = $this->_doMeta($sqlo, $metacallFunc, $subtest, $opt);
		if ($error->Got(READONLY)) {
			return;
		}
		$testinfo = $params[2];
		if ($this->debug>0) {
			$this->outinfo("Method-Params:<pre>" . 
				'Method-Name: '.print_r($params[0],1)."\n".
				'Method-Params: '."\n".print_r($params[1],1). "</pre>");
		}
		
		$answer = $this->XmlCliObj->funcCall( $xmlfuncNameUse, $params ); // can set $error	
		return array($answer,$testinfo);
	}
	
	
	// normal XMLRPC-method	
	$params = $this->_doNormal($sqlo, $xmlfuncName, $subtest, $opt);
	if ($error->Got(READONLY)) {
		return;
	}
	$testinfo = $params[2];
	
	$answer = $this->XmlCliObj->funcCall( $params[0], $params[1] ); // can set $error	
	
	return array($answer,$testinfo);
	
}
	
}