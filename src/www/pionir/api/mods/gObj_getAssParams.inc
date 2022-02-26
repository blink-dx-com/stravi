<?php
/**
 *  @package gObj_getAssParams.inc
 */
require_once("object.info.inc");
require_once("globals.inc");


/**
 * get associated parameters of an BO
 * @author  Steffen Kube
 * $Header: trunk/src/www/pionir/xmlrpc/mods/gObj_getAssParams.inc 59 2018-11-21 09:04:09Z $
 */
 class gObj_getAssParams_XRPC extends XRPC_BASE {
	
	//constructor
	function __construct() {
	}
	
	/**
	 * 
	 * @see www/pionir/xmlrpc/xml_functions/XRPC_BASE#__checkParams()
	 */
	function __checkParams() {
		global $error;
		$FUNCNAME= '__checkParams';
		
		$sqlo = $this->_sqlo;
		
		$tableName = $this->_params[0];
		$objectId = $this->_params[1];
		$columns  = $this->_params[2];
		$xcolumns = &$this->_params[3];
		

		$this->__objectCheck($sqlo, $tableName, $objectId);
		if ( $error->got(READONLY) ) {
			return -1;
		}	
    
		// check columns
		if (!sizeof($columns)) {
			$error->set( $FUNCNAME, 5, 'no columns given' );
			return -5;
		}
		
		// check column names
		reset ($columns);
		$badcol=NULL;
		$komma=NULL;
		foreach( $columns as $dummy=>$colx) {
			$featx = colFeaturesGet( $sqlo, $tableName, $colx);
			if ( !isset($featx['VISIBLE']) or $featx['VISIBLE']<1) {
				$badcol .= $komma . $colx;
				$komma=',';
			}
		}
		reset ($columns); 
		if ( $badcol!=NULL ) {
			$error->set( $FUNCNAME, 6, 'column(s) "'.$badcol.'" not possible.'  );
			return -6;
		}
			
		return 1;
	}
	
	/**
	 * get parameters of an BO
	 * - support also ASSOC tables
	 * - example: <pre>
	 *   get Notes from EXP_HAS_PROTO for EXP_TMPL_ID 56
	 * 		IN: array('table'=>'EXP_HAS_PROTO', 'id'=>56, 'vals'=>array('STEP_NO', 'CONCRETE_PROTO_ID') )
	 * 		OUT: array( array('STEP_NO'=>2, 'CONCRETE_PROTO_ID'=>92929), array('STEP_NO'=>3, 'CONCRETE_PROTO_ID'=>444444) )
	 * </pre>
	 * @param  array $this->f_param: <pre>
	 *	 [0]	'table'=> tablename,
	 *   [1]	'id'   => the ID 	
	 *   [2]    'vals' => array(columns),
	 * 	</pre>
	 * @return 
     *      array of rows(column=>value)
	 *      - if no element exists: return NULL
	 * 
     * @throws exception
	 */
	function &run() {
		$objInfo = new objInfoC();
		$sqlo = $this->_sqlo;
		
		$tableName = $this->_params[0];
		$objectId = $this->_params[1];
		$column   = $this->_params[2];
    
    	$opt=NULL;
		$opt['cols'] = $column ;
    	$opt["typeAuto"]= 1;
    	
    	if ( countPrimaryKeys($tableName)<2 ) {
    		$error->set( $FUNCNAME, 4, 'This methods supports only ASSOC tables. Input:'.$tableName  );
    		return -4;
    	}
		
    	$mother = mothertable_get2($tableName);
    	return $objInfo->getAssocVals($sqlo, $mother, $objectId, $tableName, $column);
    	
	}
}