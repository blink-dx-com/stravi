<?php
require_once ('insertx.inc');
require_once ('o.PROJ.addelems.inc');

/**
 * highLevel object creation
 * - based on class insertC()
 * - can be extended by AppLibDir/
 * - adds object to project
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 * @package glob.obj.create.inc
 */
class gObjCreateC {
	
	private $objModLib;
	
	function __construct($tablename) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	$this->insertXLib = new insertC();
    	$this->objModLib = NULL;
    	
    	$xtra_file = $_SESSION['s_sessVars']['AppLibDir']. "/objlib/o.".$tablename.".xadd.inc";
    	if ( file_exists($xtra_file) ) {
    	    require_once($xtra_file);  //  TABLE specific extra methods
    	    $dynClassName = 'o'.$tablename.'_xmodC';
    	    $this->objModLib    = new $dynClassName();
    	    debugOut("xadd-Extension for $tablename found.", $FUNCNAME, 2);
    	}
    }
    
   
    
    /**
     *  - create a business object
    	- perform object-type dependend tests
    	  TBD:  - have a flag to IGNORE INSERT_PRE_TEST 
    		    - reason: CONCRETE_SUBST: user wants the ARGUMENT-date, not the CURRENT date
        - WITHOUT "post_crea"
       @param array $obj_param, 	 TYPEOF objFeatStruct
       @param $proj_id = NULL optional add object to project
     */
    function create_objX_RAW ( &$sql, 	$tablename, 	$obj_param, $proj_id = NULL ) {
        global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
      	$error  = & ErrorHandler::get();
      	
      
    	
      	if (is_object($this->objModLib)) {
    		if ( method_exists($this->objModLib, 'crea_prep') ) {	
    			$this->objModLib->crea_prep( $sql, $obj_param ); // call specific add preparation
    			if ($error->Got(READONLY)) {
    				$error->set($FUNCNAME, 2, "Parameter preparation failed.");
    				return;
    			}
    		}
      	}
        
    	$obj_new_id = $this->insertXLib->new_meta( $sql, $tablename, $obj_param );
    	
    	if (!$obj_new_id OR $error->got(READONLY)) {
    		$error->set( $FUNCNAME, 1, "object creation problem");
    		return ($obj_new_id);
    	}
    	
    	if ($proj_id) {
    		$projLib = new oProjAddElem( $sql, $proj_id );
    		if ($error->got(READONLY)) {
    			$error->set($FUNCNAME, 7, "Copy to project ".$proj_id." failed.");
    	    	return;
    	    }
    		$projLib->addObj( $sql,  $tablename, $obj_new_id); 
    	}
      
      	return ( $obj_new_id );
    }

    /**
     *  - create a business object
     - perform object-type dependend tests
     TBD:  - have a flag to IGNORE INSERT_PRE_TEST
     - reason: CONCRETE_SUBST: user wants the ARGUMENT-date, not the CURRENT date
     @param array $obj_param, 	 TYPEOF objFeatStruct
     @param int $blueObjId  blue print onject
     @param $proj_id = NULL optional add object to project
     */
    function create_objX ( &$sql, &$sql2, $tablename, $obj_param, $blueObjId=0, $proj_id = NULL, $crea_opt=array() ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $obj_new_id = $this->create_objX_RAW ( $sql, $tablename, $obj_param, $proj_id  );
        if ($error->got(READONLY)) {
            return;
        }
        
        if (is_object($this->objModLib)) {
            if ( method_exists($this->objModLib, 'post_crea') ) {
                debugOut("(98) post-creation-method exists for $tablename.", $FUNCNAME, 2);
                $this->objModLib->post_crea( $sql,  $sql2, $obj_new_id,  $blueObjId, $crea_opt);
                if ($error->Got(READONLY)) {
                    $error->set($FUNCNAME, 2, "post_crea failed");
                    return;
                }
            }
        }
        
      
        return $obj_new_id;
    }
    
    // get log array
    function get_log() {
        if (is_object($this->objModLib)) {
            return $this->objModLib->get_log();
        }
    }

}
