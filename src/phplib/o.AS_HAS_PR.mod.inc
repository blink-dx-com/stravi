<?php
require_once ("f.assocUpdate.inc");

/**
 * methods for AS_HAS_PR for one ABSTRACT_SUBST
 * 
 * @package o.AS_HAS_PR.mod.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oAS_HAS_PR_mod {
	
	function __construct($sqlo, $objid=NULL) {
	    if ($objid) $this->setObj($sqlo, $objid);
	}
	
	function setObj($sqlo, $objid) {
		$this->objid = $objid;
		$this->assoclib = new  fAssocUpdate();
		$this->assoclib->setObj( $sqlo, 'AS_HAS_PR', $this->objid );
	}

	/**
	 * add protocol
	 * @param object $sqlo
	 * @param array $params
	 *    STEP_ID, ABSTRACT_PROTO_ID, XORDER, TYPE
	 */
	function add_protocol($sqlo, $params) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $STEP_ID = $params['STEP_ID'];
	    if (!$STEP_ID) {
	        throw new Exception('Input STEP_ID missing.');
	    }
	    $sqls="STEP_ID from AS_HAS_PR where ABSTRACT_SUBST_ID=" . $this->objid ." and STEP_ID=".$STEP_ID;
	    $sqlo->Quesel($sqls);
	    if ( $sqlo->ReadRow() ) {
	        $error->set( $FUNCNAME, 1, 'STEP_ID '.$STEP_ID.' already exists.' );
	        return; 
	    }
	    
	    $this->assoclib->insert( $sqlo, $params );
	}
	
	function get_last_STEP_ID($sqlo) {
	    $sqls="max(STEP_ID) from AS_HAS_PR where ABSTRACT_SUBST_ID=" . $this->objid;
	    $sqlo->Quesel($sqls);
	    $sqlo->ReadRow();
	    $last_STEP_ID= $sqlo->RowData[0];
	    if (!$last_STEP_ID) $last_STEP_ID = 0;
	    return $last_STEP_ID;
	}
	
	/**
	 * delete one protocol
	 * @param object $sqlo
	 * @param int $STEP_ID
	 */
	function del($sqlo, $step_id) {
	    $idarr=array('STEP_ID'=>$step_id);
	    $this->assoclib->delOneRow($sqlo, $idarr);
	}
	
	// needed to support ADVMOD
	function close($sqlo) {
	    $this->assoclib->close($sqlo);
	}
	
}
