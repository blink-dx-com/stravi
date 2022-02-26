<?php
require_once 'o.H_PROTO_KIND.subs.inc';
require_once 'o.CONCRETE_SUBST.subs.inc';
require_once 'o.CONCRETE_SUBST.qcProt.inc';
require_once 'f.qc_check.inc';
// $Header: trunk/src/phplib/o.CONCRETE_PROTO.policy.inc 59 2018-11-21 09:04:09Z $
/**
 * special release policy for CONCRETE_PROTO
 * called by e.g. www/lab/pol.objRelease.inc
 * @package o.CONCRETE_PROTO.policy.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   UREQ:0002045: f.object.release.policy > special for CONCRETE_PROTO
 */
class oPRC_policy {
	function __construct( &$sqlo ) {
		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
		    $this->_debug = $_SESSION["userGlob"]["g.debugLevel"];
		}
	}
	
	/**
	 * get release info
	 *  - get protocol KIND
	 *  - check, if it is a 'QC test'-Protocol
	 * @param $sqlo
	 * @param $objid
	 * @param array $polOption
 	 *   'mo.CERT_FLAG' : [1],2,3,4 planned Life Cycle Flag of mother object
	 * @return array ( 'txt'=>$qc_infoText, 'err'=>$qc_problem )
	 * 	'txt' => string
	 *  'err' <= 0 o.k.
	 *  	  >  0 error
	 */
	function policyInfo(&$sqlo, $objid, $infolevel=0, $polOption=NULL) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		$pol_problem = 0;
		$outtext     = NULL;
		
		
		
		$protoKindLib   = new oH_PROTO_KIND_subs();
		$qc_test_KindID = $protoKindLib->name2ID($sqlo, oH_PROTO_KIND_subs::TYPE_QC );
		
		$protoFeats = glob_elemDataGet3( $sqlo, 'CONCRETE_PROTO', array('CONCRETE_PROTO_ID'=>$objid), array('ABSTRACT_PROTO_ID', 'ACCEPT_PROT_ID') );
		
		if ($protoFeats['ABSTRACT_PROTO_ID']) {
			$aprotoFeats = glob_elemDataGet3( $sqlo, 'ABSTRACT_PROTO', array('ABSTRACT_PROTO_ID'=>$protoFeats['ABSTRACT_PROTO_ID']), 
				array('H_PROTO_KIND_ID', 'NAME') );
		}
		
		if ($aprotoFeats['H_PROTO_KIND_ID']==$qc_test_KindID) {
		    //
			// do special QC-Protocol-test
			//
			
			if ( $this->_debug>1 ) {
				debugOut('perform special check of QC-protocol features.', $FUNCNAME);
			}
			
			$cSubstQCLib = new ocSUC_qcProtoMeta($sqlo);
			$answer      = $cSubstQCLib->checkOneCproto( $sqlo, $objid );
			if ($answer['all']<0) {
				$pol_problem = 1;
				$outtext     = $answer['text'];
			}
		} else {
		    //
			// normal Prep-Protocol-check
			//
			
		    if ( $polOption['mo.CERT_FLAG']== oConcSubst_subC::CERT_FLAG_research ) {
		        // no check for LCF="Research" !
		        return array ( 'txt'=>$outtext, 'err'=>$pol_problem );
		    }
		    
			$cSubstPrepLib = new fQC_checkC();
			$levelPrep='full';
			if ($infolevel>0) $levelPrep='full2';
			
			$prepOpt = NULL;
			// do special check, dependent from $polOption['mo.CERT_FLAG']
			if ($polOption['mo.CERT_FLAG']>0) $prepOpt = array( 'CERT_FLAG'=>$polOption['mo.CERT_FLAG'] );
			
			$answer  = $cSubstPrepLib->protoCheckBase( $sqlo, $objid, $levelPrep, $prepOpt );
			if ($answer['all']<0) {
				$pol_problem = 2;
				$outtext     = $answer['text'];
			}
		}
		return array ( 'txt'=>$outtext, 'err'=>$pol_problem );
	}
	
	/**
	 * 
	 * show Non-QM-info
	 * @swreq UREQ:0001032 check if concrete object is derived from abstract object
	 * @param  $sqlo
	 * @param  $cpid
	 * @return array ( 'txt'=>$qc_infoText, 'err'=>$qc_problem )
	 * 	'txt' => string
	 *  'err' <= 0 o.k.
	 *  	  >  0 error
	 *  or NULL
	 */
	public function nonQmInfo( &$sqlo, $cpid ) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$pol_problem = 0;
		$outtext     = NULL;
		
		$protoKindLib   = new oH_PROTO_KIND_subs();
		$qc_test_KindID = $protoKindLib->name2ID($sqlo, 'QC test');
		
		$protoFeats = glob_elemDataGet3( $sqlo, 'CONCRETE_PROTO', array('CONCRETE_PROTO_ID'=>$cpid), 
			array('ABSTRACT_PROTO_ID', 'ACCEPT_PROT_ID') );
		
		if ($protoFeats['ABSTRACT_PROTO_ID']) {
			$aprotoFeats = glob_elemDataGet3( $sqlo, 'ABSTRACT_PROTO', array('ABSTRACT_PROTO_ID'=>$protoFeats['ABSTRACT_PROTO_ID']), 
				array('H_PROTO_KIND_ID', 'NAME') );
		}
		
		if ($aprotoFeats['H_PROTO_KIND_ID']!=$qc_test_KindID) {
			// check abstract substs; only for NON-"QC test" protocols
			$qc2Lib = new oCONCRETE_PROTO_qc2();
			$qc2Lib->setCpid( $sqlo, $cpid, $protoFeats['ABSTRACT_PROTO_ID'] );
			$infoarr = $qc2Lib->absVersConcSubst($sqlo);
			$outtext = $infoarr['txt'];
			$pol_problem =  $infoarr['err'];
		}
		
		return array ( 'txt'=>$outtext, 'err'=>$pol_problem );
	}
}