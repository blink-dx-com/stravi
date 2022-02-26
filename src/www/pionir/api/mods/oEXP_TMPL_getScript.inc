<?php
/**
 * @namespace core::xmlrpc::meta
 */

/**

 * Request: 0000785: p.HIV-NAT_getScript
 * Get the WetScript (python file) for a given EXP_TMPL_ID
 * @author  Michael Brendel (michael.brendel@clondiag.com) 
 * @param
 * <pre>array ($f_param
 *   [0]	'$objectId'   			=> the exp_tmpl_id,
 *   [1]	'$protokind' [optional] => type of the protocol, determines the script to be downloaded (e.g. "Image acquisition" => WetScript) )
 * </pre>
 *
 * @return
 * <pre>array (
 * 	[0]		ERRORCODE		=> 0 if no error occured
 *  [1]		RETVAL			=> 
 *  	if ($protokind!= NULL) {
 *  			returns the Id of the Script for the given protocol type
 *  			(e.g. the wetscript in case of protocol type 'Image acquisition'),
 *  	}
 *  	else {
 *  		returns an array of all connected scripts and their protocol type
 *  		array("documentId"=>ID, "protoKind"=>string)
 *  	}
 * </pre>
 * @throws exception
 * @see    "usedby:external:ICONOCLUST"
 * 
 */
require_once("object.info.inc");
require_once("globals.inc");
require_once("class.filex.inc");

class oEXP_TMPL_getScript_XRPC extends XRPC_BASE {
	//constructor
	function __construct() {
	}
	
	/**
	 * (non-PHPdoc)
	 * @see www/pionir/xmlrpc/xml_functions/XRPC_BASE#__checkParams()
	 */
	function __checkParams() {
		global $error;
		$FUNCNAME= '__checkParams';
		
		$sqlo = $this->_sqlo;
		
		$objectId = $this->_params[0];

		//1. Existenz-Checks
		
		//2. Access-Checks
		
		//3. Data-Checks
		
		$t_rights = tableAccessCheck( $sqlo, $tableName );
		if ( $t_rights["read"] != 1 ) {
			$answer = getRawTableAccMsg( $tableName, 'read' );
			$error->set( $FUNCNAME, 1, $answer );
			return;
		}
		
		if (!$objectId) {
			$error->set( $FUNCNAME, 2, 'id is not set.' );
			return;
		}
		return 1;
	}
	
	function &run() {
		global $error;
		$FUNCNAME= 'run';
		
		$objInfo = new objInfoC();
		$obj = new fileC();
		$sqlo = $this->_sqlo;
		
		$objectId = trim($this->_params[0]);
		$protoKind = trim($this->_params[1]);
		$tableName = "H_PROTO_KIND";
		$link_id = 0;
		$ret[] = null;
		
		if ($protoKind == null) {
			//return an array with all scripts if protokind is null
			//select a.ABSTRACT_PROTO_ID, b.NAME from ABSTRACT_PROTO a, H_PROTO_KIND b
	  		//	where a.ABSTRACT_PROTO_ID in (select ABSTRACT_PROTO_ID from EXP_TMPL_HAS_PROTO
	  		//		where EXP_TMPL_ID = 4189) and a.H_PROTO_KIND_ID=b.H_PROTO_KIND_ID
			$sqlo->Quesel(	'a.LINK_ID, b.NAME from ABSTRACT_PROTO a, H_PROTO_KIND b '.
								'where a.ABSTRACT_PROTO_ID in (select ABSTRACT_PROTO_ID from EXP_TMPL_HAS_PROTO '.
									'where EXP_TMPL_ID = '.$objectId.') '.
										'and a.H_PROTO_KIND_ID=b.H_PROTO_KIND_ID '.
										'and a.LINK_ID IS NOT NULL');
			while ($sqlo->ReadRow()) {
				$ret[] = array(array("documentId"=>$sqlo->RowData[0], "protoKind"=>$sqlo->RowData[1]));
			}
			return $ret;
		}
		
		/**
		 * select LINK_ID from ABSTRACT_PROTO
		 *   where ABSTRACT_PROTO_ID in (
		 *     select ABSTRACT_PROTO_ID from EXP_TMPL_HAS_PROTO where exp_tmpl_id = 3907)
		 *   AND H_PROTO_KIND_ID = 160 (//Image acquisition)
		 */
		
		list( $objcnt, $objid, $objname ) = $obj->objGetByName( 
			$sqlo, 
			$tableName, 
			importantNameGet2($tableName), 
			$protoKind, 
			$opt=NULL //  "like" = 1 add wildcards: %$objName%
					//         = 2 only like ...
					//  "caseinsense" => 1 : case insensitive search
			);
		
		if ($objcnt < 1) { //not found
			$error->set( $FUNCNAME, 3, 'no object '.$protoKind.' was found in table '.$tableName );
			return;
		}
		else if ($objcnt == 1) { //found one
			$sqlo->Quesel("LINK_ID from ABSTRACT_PROTO".
					 		" where ABSTRACT_PROTO_ID in (".
								" select ABSTRACT_PROTO_ID from EXP_TMPL_HAS_PROTO where EXP_TMPL_ID = '".$objectId."')".
					 		" AND H_PROTO_KIND_ID = '".$objid."'" );
			$sqlo->ReadRow();
			$link_id = $sqlo->RowData[0];	
		}
		
		else if ($objcnt > 1) { //found multiple
			$error->set( $FUNCNAME, 4, 'multiple objects with name '.$protoKind.' were found in table '.$tableName );
			return;
		}
			
		if (!$link_id) {
			$error->set( $FUNCNAME, 5, "No Script of type '".$protoKind."' for Object ".$objectId." was found." );
			return;
		}
		
		//TODO: handle multiple found scripts...not allowed atm
		/*
		if ($link_id > 1) {
			$error->set( $FUNCNAME, 5, "Multiple Scripts for Object ".$objectId." found." );
			return;
		}*/
		
		return $link_id;
	}
}