<?php
require_once("f.modulLog.inc");
require_once 'f.api.inc';

/**
 * base class for RPC::metacall-plugin manager
 * @package metacall.inc
 * @swreq   g > rpc > metacall 
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class XRPC_BASE {
	var $_sqlo;
	var $_params;
	
	function __setInit(&$sqlo, &$params) {
		$this->_params = &$params;
		$this->_sqlo = &$sqlo;
	}
	
	// module internal method, can be overwritten ..
	// called after __setInit()
	function __mod_init() {}
	
	/** check parameters
	 * @return : >0 : o.k.
	 */
	function __checkParams() {
		return 1;
	}
	
	// register the second SQL-handle
	function &_plug_regSql2() {
		if ($this->_sqlo2==NULL) {
			$this->_sqlo2  = logon2(  );
		}
		return $this->_sqlo2;
	}
	
	/**
     * parameter and object check
     * - supports also ASSOC-tables: check existance of mother object
	 */
	function __objectCheck(&$sqlo, $tablename, $objectId) {
	    f_api_C::objectCheck($sqlo, $tablename, $objectId);
	}
	
	
	/**
	 * do access check: standard: write-test
	 * @param array $act 'tab' => array(rights)
	 *					 'obj' => array(rights)
	 				leave 'tab' or 'obj' as empty array, if you do not want to check			  
	 */
	function __objAccChk(&$sqlo, $tablename, $id, $act=NULL ) {
		if ( !is_array($act['tab']) ) $act['tab'] = array('write');
		if ( !is_array($act['obj']))  $act['obj'] = array('read', 'write');
		access_tab_obj_rights($sqlo, $tablename, $id, $act);
	}
	
	/**
	 * main run-method
	 * @return mixed $answer
	 */
	function run() {}
	
}

class metacall {
    
    private $p_name;
    private $f_param;
    
    function __construct($p_name, &$f_param) {
        $this->p_name  = $p_name;
        $this->f_param = &$f_param;
    }

    /** 
     * @access  private
     */
    function _meta_include($mod) {
    		global $error;
    		$FUNCNAME = '_meta_include';
    		
    		/**
    		 * locations of code
    		 * @var array $basekeys
    		 */
    		$basekeys = array(
    			'DEF'=> dirname(__FILE__).'/../mods', // TBD: move this location ?
    		    'LAB'=> dirname(__FILE__).'/../../../' . $_SESSION['globals']["lab_path"].'/api/mods'
    		);
    		$BASELEN  = 3; // 3 letters !!!
    		
    		if ($mod==NULL) {
    			$error->set( $FUNCNAME, 1, 'no Plugin name given.' );
    			return;
    		}
    		if (strstr($mod,'..')!=NULL) {
    			$error->set( $FUNCNAME, 2, 'Plugin-path invalid.' );
    			return;
    		}
    		$basekeyX = substr($mod,0,$BASELEN);
    		if ( $basekeys[$basekeyX]==NULL ) {
    			$error->set( $FUNCNAME, 3, 'No support for Base:'.$basekeyX.'.' );
    			return;
    		}
    		$pureModPath = substr($mod,$BASELEN+1); // without '/'
    		$BASE_DIR = $basekeys[$basekeyX];
    		$relFile  = $BASE_DIR.'/'.$pureModPath.'.inc';
    		
    		if (!file_exists($relFile)) {
    			$error->set( $FUNCNAME, 4, 'Plugin "'.$relFile.'" not found.' );
    			return;
    		}
    		require_once($relFile);
    		
    		$classname = $pureModPath.'_XRPC';
    		return $classname;
    	}
    
    
    /**
     * manage XML-RPC meta-calls : the metacall interface
     * 
     * - call a module-method
     * - dynamic binding of a module
     * - module location:  "../mods/module_name.inc" > class module_name_XRPC
     * 
     * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
     * @swreq   0001179: g > xml-rpc > metacall 
     * @param   $p_name : plugin name
     * @param    array $f_param <pre>
     * [0] long   sess_id
     * [1] string moduleURL : NAMESPACE/MODULE
     *      + NAMESPACEs: DEF, LAB
     *      + moduleURL-example: 'DEF/oEXP_test'
     * [2] array  params
     * 	</pre>
     * @throws exception <pre>
     * 3 - invalid module-name
     * </pre>
     * @return array jsonrpc_result_STRUCT  
     */	
    function run(){
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        global $error, $varcol;
        
        $params= &$this->f_param;
    	
    	$errprio     = 3;
    	$module_name = $this->p_name;
    	$meta_msg_call_info= '';
    	if (array_key_exists('id', $params)) {
    	    $meta_msg_call_info= '; Input:ID:'.$params['id'];
    	}
	
    	$error  = & ErrorHandler::get();
    	$varcol = & Varcols::get();
    	$sqlo   = logon2();
    	
    	$className = $this->_meta_include($module_name);
    	if ($error->Got(READONLY))  {
    		$error->set( $FUNCNAME, 3, 'metacall: invalid module:"'.$module_name.'"' );
    		$errtext = $error->getAsTextAndLog( $FUNCNAME, $errprio );
    		$error->reset();
    		return array( 'error'=>array('num'=>3, 'text'=>$errtext ) );
    	}
    	
    	
    	$plugLib = new $className();
    	$plugLib->__setInit($sqlo, $params);
    	$plugLib->__mod_init();
    	$answer = $plugLib->__checkParams();
    	
    	if ($answer<=0 or $error->Got(READONLY)) {
    	    
    		$error->set( $FUNCNAME, 4, 'metacall: module:"'.$module_name.'" checkParams()' );
    		$error_stack=NULL;
    		$errTxt = $error->getAllAsText($error_stack, "[/ERR]");
    		
    		$logerrTxt = $errTxt;
    		if ($_SESSION['s_sessVars']['blink.device.serial']!='') {
    		    // add device serial to the error-log
    		    $logerrTxt = 'DEVICE.serial:'.$_SESSION['s_sessVars']['blink.device.serial'].'; '.$errTxt;
    		}
    		
    		$error->logxMeta( "ERROR", $errprio, 'metacall:checkParams', $FUNCNAME.':'.$module_name,  $logerrTxt.$meta_msg_call_info );
    		
    		
    		return array( 'error'=>array('num'=>4, 'text'=>$errTxt ) );
    	}
    	
    	$answer  = &$plugLib->run();
    	if ($error->Got(READONLY))  {
    		
    		// @swreq: UREQ:0001171: g > XML_RPC > Fehlerkonzept 
    		$errLast   = $error->getLast('',0,READONLY);
    		$error_id  = $errLast->id;
    	
    		$erroridUse= 5;
    		if ($error_id>10) $erroridUse=$error_id; // errors bigger 10 will be returned with original ID
    		$error->set( $FUNCNAME, $erroridUse, 'module:"'.$module_name.'" run failed.' );
    		
    		$error_stack=NULL;
    		$errTxt = $error->getAllAsText($error_stack, "[/ERR]");
    		
    		$logerrTxt = $errTxt;
    		if ($_SESSION['s_sessVars']['blink.device.serial']!='') {
    		    // add device serial to the error-log
    		    $logerrTxt = 'DEVICE.serial:'.$_SESSION['s_sessVars']['blink.device.serial'].'; '.$errTxt;
    		}
    		$error->logxMeta( "ERROR", $errprio, 'metacall:run', $FUNCNAME.':'.$module_name,  $logerrTxt. $meta_msg_call_info);
    		

    		return array( 'data'=>NULL, 'error'=>array('num'=>$erroridUse, 'text'=>$errTxt ) );
    	}
    	
    	if ($error->Got(CCT_WARNING_READONLY))  {
    	    // save warnings ...
    	    $errtext = $error->getAsTextAndLog( $FUNCNAME.':'.$module_name, 20 );
    	}
    	
    	return array( 'data'=>$answer );
    }
}