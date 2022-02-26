<?php
// unittest for XMLRPC-method obj_get_params

class obj_get_params_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;

		$optInputParams = $this->params;
		$subtest  = $this->subtest;
		
		switch  ( $subtest ) {

			case "normal":
				$exp_id	 =  $this->_getDefObjID( "EXP", "default" );
				$params = array( 'EXP',$exp_id, array('NAME','NOTES'));
				break;
			case "input":
				if ($optInputParams==NULL) {
					$error->set( $FUNCNAME, 2, 'Need PARAMETERS: TABLE:ID; OUTPUT: NAME,NOTES' );
					break;
				}
				
				$paramarr = explode(':',$optInputParams);
				$tablename=$paramarr[0];
				$objid    =$paramarr[1];
				if ( !glob_table_exists($tablename) ) {
					$error->set( $FUNCNAME, 2, 'table "'.$tablename.'" does not exist.' );
					break;
				}
				
				if ( !is_numeric($objid) ) {
					$error->set( $FUNCNAME, 2, 'objid is not numeric.' );
					break;
				}
				
				$params = array( $tablename, $objid, array('NAME','NOTES'));
				
				break;
				
			case "input2":
					if ($optInputParams==NULL) {
						$error->set( $FUNCNAME, 2, 'Need PARAMETERS: TABLE:ID; OUTPUT: NAME,NOTES,_CLASS' );
						break;
					}
				
					$paramarr = explode(':',$optInputParams);
					$tablename=$paramarr[0];
					$objid    =$paramarr[1];
					if ( !glob_table_exists($tablename) ) {
						$error->set( $FUNCNAME, 2, 'table "'.$tablename.'" does not exist.' );
						break;
					}
				
					if ( !is_numeric($objid) ) {
						$error->set( $FUNCNAME, 2, 'objid is not numeric.' );
						break;
					}
				
					$params = array( $tablename, $objid, array('NAME','NOTES','USER', '_CLASS'));
				
					break;
		}
		return $params;
	}
}
