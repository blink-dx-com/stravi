<?php
require_once ("o.ACCEPT_PROT.subs.inc");

/**
 * abstract subst : QM analysis
 * $Header: trunk/src/phplib/o.ABSTRACT_SUBST.qmana.inc 59 2018-11-21 09:04:09Z $
 * @package o.ABSTRACT_SUBST.qmana.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   SREQ:0002755: o.ABSTRACT_SUBST > sublib for QM analysis 
 */
class oABSTRACT_SUBST_qmana {

function __construct() {
	
}
	
function setObj(&$sqlo, $suaid) {
	$this->suaid = $suaid;
	$this->acceptLib = new oACCEPT_PROT_subs($sqlo);
}

/**
 * get protoblems
 * - analyse acceptance proto
 * @param object $sqlo
 * @param $absprotoid
 * @return array of strings
 */
function getQM_prob_qcproto(&$sqlo, $absprotoid) {
	// get acceptance protocols from ABSTRACT_PROT (QC)
	$errorarr = NULL;
	
	$accOpt    = array( 'onlyReleased'=>1 );
	$accProtoArr = $this->acceptLib->getAcceptByAbstract($sqlo, $absprotoid, $accOpt);
	// $acc_num     = $this->acceptLib->getNumAcceptByAbs(  $sqlo, $absprotoid);
	
	if (empty($accProtoArr)) {
		$errorarr[]='protocol has no released acceptance protocol!';
	}
	return $errorarr;
}    	
	
}