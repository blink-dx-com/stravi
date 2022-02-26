<?   
require_once ('insert.inc'); 
require_once ('f.update.inc');
require_once ('date_funcs.inc'); 
require_once ("f.assocUpdate.inc");
require_once ("glob.obj.update.inc");
require_once 'o.CONCRETE_PROTO.mod.inc';

/**
 * TBD: deprecated use standard lib !!!
 * sub functions to manage protocol actions like "create" 
 * @swreq UREQ:0002522: o.CONCRETE_PROTO > create new protocol/create copy and attach to object  
 * @package glob.obj.proto_act.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * 
 */
class obj_proto_act {
	
function __construct() {
	$this->sqlDateNow = date_unix2sql( time() );
}


/**
 * add a protocol to "CONCRETE_SUBST" or "CHIP_READER"
 * @param  $sql
 * @param  $sql2
 * @param  $tablename // "CONCRETE_SUBST" | "CHIP_READER"
 * @param  $id        // destination concrete_subst_id, chip_reader_id
 * @param  $abstract_proto_id
 * @param  $blue_conc_prot
 * @param  $opt  
 *  'xord', -- MUST be set for CONCRETE_SUBST !
 *  'type'
 */
function csubst_create_proto(  &$sql,&$sql2,$tablename, $id,  $abstract_proto_id, $blue_conc_prot, $opt=NULL ) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    $error = & ErrorHandler::get();
    
    $retval=0;
	$pk_name = $tablename . "_ID";
    $cargu=array();
	$cargu['ABSTRACT_PROTO_ID'] = $abstract_proto_id;
	$cargu['EXEC_DATE'] 		= $this->sqlDateNow;
		 
	$c_proto = insert_row( $sql, "CONCRETE_PROTO", $cargu );  /* TBD: use insertC::new_meta() */
	
	if ($c_proto<=0) {
		$error->set($FUNCNAME, 1, "Problems at SQL-creation of the protocol.");
		return -1;
	}
    
	if ($tablename=='CONCRETE_SUBST') {
		$xorder = $opt['xord'];
		$typex  = $opt['type'];
		
		$this->attachProto($sql, $id, $c_proto, $xorder, $typex);
	} else {
		// CHIP_READER
		$argu = NULL;
		$argu[$pk_name] = $id;
		$argu['CONCRETE_PROTO_ID'] = "$c_proto";
			 
		$ret_id = gObjUpdate::update_row( $sql, $tablename, $argu );
		if ($ret_id<1) {
			$error->set($FUNCNAME, 2, "Problems at connection of protocol to the $tablename.");
			return -2;
		}
	}
	if ($error->Got(READONLY)) { 
        $error->set($FUNCNAME, 2, "Problems at connection of protocol to the $tablename.");
		return -2;
	}
	
	if ( $blue_conc_prot ) {
	    
	    $cropt=array( 'exec_date'=>$this->sqlDateNow);
	    oCONCRETE_PROTO_mod::assoc_copy( $sql, $sql2, $blue_conc_prot, $c_proto, $cropt );
	}
    return ($retval);
}

	/**
	 * add protocol to substance
	 * allows also to add protocols for $xorder>9999
	 * @param $sqlo
	 * @param $protoID
	 * @param $xorder 
	 * @param $typex  :default-value: 1 : Prep-protocol 
	 */
	function attachProto(&$sqlo, $substid, $protoID, $xorder, $typex) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    if (!$xorder) {
	        $error->set( $FUNCNAME, 1, 'INPUT xorder missing.' );
	        return;
	    }
		if (!$typex) $typex = 1;  // fallback
		
		
		// add to CP_HAS_PR
		$params = array(
			'STEP_NO'=> $xorder,
			'CONCRETE_PROTO_ID'=>$protoID,
			'TYPE'    => $typex
		 );
		 
		$assoclib = new  fAssocUpdate();
		$assoclib->setObj( $sqlo, 'CS_HAS_PR', $substid );
		$assoclib->insert( $sqlo, $params );
		$assoclib->close($sqlo);
		
	}	

}

