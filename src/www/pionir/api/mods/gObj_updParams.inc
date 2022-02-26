<?php
/**
 * @namespace core::xmlrpc::meta
 */
require_once("glob.obj.update.inc");

/** 
 * update parameters of an BO
 * 
 * - use class globObjUpdate 
 * - test: access, object existance, allowed columns
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   array $f_param <pre>
 *	 [0]	'table'=> tablename,
 *   [1]	'id'   => the ID 	
 *   [2]    $args : TYPEOF objFeatStruct : array( 
 *   	'vals'  => array(columns=>vals),
 *   	'xobj'  => array ( 
					'class_name' => name of class ,
					'values'     => array(attrib_name=>val) 
					 )
 *   	)
 * 	</pre>
 * Example: update EXP 56
 * 		IN: array('table'=>'EXP_TMPL', 'id'=>56, array('vals'=>array('NOTES'=>'hallo')) )
 * 		
 * @return -
 * @throws exception
 * @see    "usedby:external:array_production"
 */
class gObj_updParams_XRPC extends XRPC_BASE {
	
	private $_extra_class_id = 0; // cache extra_class_id
	
	//constructor
	function __construct() {
	}
	
	/**
	 * analyse columns
	 * @return 
	 * @param object $sqlo
	 * @param object $mainFeat
	 */
	function _anaColumns(&$sqlo, &$mainFeat) {
		
		$PK_name  = PrimNameGet2($this->tablename);
		
		$editArr=NULL;
		foreach( $mainFeat as $colx=>$dummy) {
			$featx = colFeaturesGet( $sqlo, $this->tablename, $colx);
			$editArr[$colx]=$featx['EDITABLE'];
		}
		reset ($mainFeat); 
		
		// not allowed!
		$editArr['CCT_ACCESS_ID']=0;
		$editArr['EXTRA_OBJ_ID'] =0;
		$editArr[$PK_name] =  0;
		
		$badcol=NULL;
		foreach( $mainFeat as $colx=>$dummy) {
			if ( $editArr[$colx]<1) {
				$badcol .= $komma . $colx;
				$komma=',';
			}
		}
		reset ($mainFeat);
		return $badcol;
	}
	
	/**
	 * 
	 * @see www/pionir/xmlrpc/xml_functions/XRPC_BASE#__checkParams()
	 */
	function __checkParams() {
		global $error, $varcol;
		$FUNCNAME= '__checkParams';
		
		$sqlo = $this->_sqlo;
		$varcol     = & Varcols::get();
		
		$tableName= $this->_params[0];
		$objectId = $this->_params[1];
		$args  	  = $this->_params[2];
		
		$this->tablename = $tableName;
		$this->objid = $objectId;
		$this->_extra_class_id = 0;

		//1. Existenz-Checks
		//2. Access-Checks
		//3. Data-Checks
		
		$t_rights = tableAccessCheck( $sqlo, $tableName );
		if ( $t_rights["write"] != 1 ) {
			$answer = getRawTableAccMsg( $tableName, 'write' );
			$error->set( $FUNCNAME, 1, $answer );
			return -1;
		}
		
		if ( !cct_access_has2($tableName) ) {
			$error->set( $FUNCNAME, 7, 'Only Business objects allowed.' );
			return -7;
		}
		
		
		//id is not set
		if (!$objectId) {
			$error->set( $FUNCNAME, 2, 'id is not set.' );
			return -2;
		}
		
		//id is not numeric
		if (!is_numeric($objectId) ) {
			$error->set( $FUNCNAME, 3, 'id is not numeric.' );
			return -3;
		}
		
		// object doesn't exist
		if ( !gObject_exists ($sqlo, $tableName, $objectId) ) {
			$error->set( $FUNCNAME, 4, 'no object with '.$objectId.' was found in table '.$tableName );
			return -4;
		}
		
		$o_rights = access_check($sqlo, $tableName, $objectId);
		if ( !$o_rights["write"] ){
			$error->set( $FUNCNAME, 8, 'you have no "write" permission on '.
					$tableName.' ID:'.$objectId.'!' );
			return -8;
		}
    
		// check columns
		if (!sizeof($args)) {
			$error->set( $FUNCNAME, 5, 'no column-args given' );
			return -5;
		}
		
		// check column names
		if ( sizeof($args['vals']) ) {
			
			$mainFeat = &$args['vals'];
			$badcol = $this->_anaColumns($sqlo, $mainFeat);
			
			if ( $badcol!=NULL ) {
				$error->set( $FUNCNAME, 6, 'column(s) "'.$badcol.'" not possible.'  );
				return -6;
			}
		}
		
		if ( sizeof($args['xobj']) ) {
			
			$objFeat = &$args['xobj'];
			if ($objFeat['class_name']==NULL) {
				$error->set( $FUNCNAME, 9, 'xobj-params are given, class-name missing.'  );
				return -9;
			}
			$this->_extra_class_id = $varcol->class_name_to_id($tableName, $objFeat['class_name']);
			
		}
			
		return 1;
	}
	
	function &run() {
		$objInfo = new objInfoC();
		$sqlo = $this->_sqlo;
		
		$tableName= $this->_params[0];
		$objectId = $this->_params[1];
		$args  	  = $this->_params[2]; 
		
		
		if ( isset($args['xobj']['class_name'] ) ) {
			$args['xobj']['extra_class_id'] = $this->_extra_class_id; // provide needed ID
		}
  
		$UpdateLib = new globObjUpdate();
		$opt=NULL;
		$opt['typeAuto']=1;
		$UpdateLib->update_meta( $sqlo, $tableName, $objectId, $args, $opt );
		

	}
}