<?php

require_once ("insertx.inc");
require_once 'object.info.inc';
require_once('o.PROJ.addelems.inc');


/** 
 * update parameters of an BO
 * 
 * - test: access, allowed columns
 * 
 * ```
 *  INPUT:
 *	 ['t']	'table'=> tablename,
 *   ['args']   $args : TYPEOF objFeatStruct : array( 
 *   	'vals'  => array(columns=>vals),
 *   	'xobj'  => array ( 
 *					'class_name' => name of class ,
 *					'values'     => array(attrib_name=>val) 
 *			)
 *	 ['proj'] => [OPTIONAL] if ID is set, add object to project
 *	             if no write access: throw error
 *	 ['attach'] => attachment
 *	         'binary' => binary stream
 *	         'name'   => MUST set
 *	         'notes'  =>
 *	         'key'    => 
 *   	)
 * 	
 * Example: update EXP 56
 * 	 IN: array('table'=>'EXP_TMPL', 'id'=>56, 
 *          array('vals'=>array('NOTES'=>'hallo')) )
 * 		
 * OUT: array('objid'=>ID)
 * ```
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @module gObj_create.inc
 * @package WEBAPI\gObj_create
 */
class gObj_create_XRPC extends XRPC_BASE {
	
	private $_extra_class_id = 0; // cache extra_class_id
	
	
	
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
		
		$tableName= $this->_params['t'];
		$args  	  = $this->_params['args'];
		
		$this->tablename = $tableName;
		$this->_extra_class_id = 0;

		//1. Existenz-Checks
		//2. Access-Checks
		//3. Data-Checks
		
		$t_rights = tableAccessCheck( $sqlo, $tableName );
		if ( $t_rights["insert"] != 1 ) {
			$answer = getRawTableAccMsg( $tableName, 'insert' );
			$error->set( $FUNCNAME, 1, $answer );
			return -1;
		}
		
		if ( !cct_access_has2($tableName) ) {
			$error->set( $FUNCNAME, 7, 'Only Business objects allowed.' );
			return -7;
		}
		

    
		// check columns
		if (empty($args)) {
			$error->set( $FUNCNAME, 5, 'no column-args given' );
			return -5;
		}
		
		// check column names
		if ( !empty($args['vals']) ) {
			
			$mainFeat = &$args['vals'];
			$badcol = $this->_anaColumns($sqlo, $mainFeat);
			
			if ( $badcol!=NULL ) {
				$error->set( $FUNCNAME, 6, 'column(s) "'.$badcol.'" not possible.'  );
				return -6;
			}
		}
		
		if ( !empty($args['xobj']) ) {
			
			$objFeat = &$args['xobj'];
			if ($objFeat['class_name']==NULL) {
				$error->set( $FUNCNAME, 9, 'xobj-params are given, class-name missing.'  );
				return -9;
			}
			$this->_extra_class_id = $varcol->class_name_to_id($tableName, $objFeat['class_name']);
			
		}
		
		if ( $this->_params['proj'] ) {
		    $this->__objectCheck($sqlo, 'PROJ', $this->_params['proj']);
		    if ( $error->got(READONLY) ) {
		        return -10;
		    }	
		    
		    $act = array(
		        'obj' => array('insert')
		        );
		    $this->__objAccChk($sqlo, 'PROJ', $this->_params['proj'], $act );
		    if ( $error->got(READONLY) ) {
		        return -11;
		    }
		}
			
		return 1;
	}
	
	function &run() {
	    
		$sqlo = $this->_sqlo;
		
		$tablename= $this->_params['t'];
		$args  	  = $this->_params['args']; 

		
		if ( isset($args['xobj']['class_name'] ) ) {
			$args['xobj']['extra_class_id'] = $this->_extra_class_id; // provide needed ID
		}
		
		$insertlib = new insertC();
		$objid = $insertlib->new_meta($sqlo, $tablename, $args);
		
		if ( $objid and $this->_params['proj'] ) {
		    
		    $proj_id = $this->_params['proj'];
		    $proj_lib = new oProjAddElem($sqlo, $proj_id);
		    $proj_lib->addObj($sqlo, $tablename, $objid);
		}
  
		
		return array('objid'=>$objid);

	}
}