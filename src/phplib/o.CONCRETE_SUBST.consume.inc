<?php
/**
 * calc consumed QUANTITY of a substance
 * @package o.CONCRETE_SUBST.consume.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0001018: o.CONCRETE_SUBST > consumption calculation
 */

require_once ('o.H_UNIT.siSubs.inc');

class oCONCRETE_SUBST_consOne{
	var $_CLASSNAME='oCONCRETE_SUBST_consOne';
	var $feat; /* substance features
	['aid']  ABSTRACT_SUBST_ID
	['quan'] QUANTITY
	['hunitID']  meas-unit of the abstract subst
	['hunitNa'] 
	['hunitFac']
	*/
	
	function oCONCRETE_SUBST_consOne() {
		$this->UnitLib = new oH_UNIT_siSubs();
		
		
	}
	
	function initLib(&$sqlo) {
		
	}
	
	/**
	 * - check initial parameters
	 * - get abstract_subst, measunit, amount
	 * - get SI-factor
	 * @return 
	 * @param object $sqlo
	 */
	function setOne(&$sqlo, $id) {
		global $error;
		$FUNCNAME= $this->_CLASSNAME.':setOne';
		$this->id   = $id;
		$this->feat     = array();
		$this->absProts = NULL;
		
		$sqlsel = 'ABSTRACT_SUBST_ID, QUANTITY from CONCRETE_SUBST where CONCRETE_SUBST_ID='.$id;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$feat['aid']  = $sqlo->RowData[0];
		$feat['quan'] = $sqlo->RowData[1];
		$feat['hunitID']  = glob_elementDataGet( $sqlo, 'ABSTRACT_SUBST', 'ABSTRACT_SUBST_ID', 
					$feat['aid'], 'H_UNIT_ID');
		
		if ( $feat['hunitID']  ) {
			$feat['hunitNa']  = glob_elementDataGet( $sqlo, 'H_UNIT', 'H_UNIT_ID', 
					$feat['hunitID'], 'NAME');
			$feat['hunitFac'] = $this->UnitLib->getFactor($feat['hunitNa']);
		}
		$this->feat = $feat;
		
		// checks
		$errtxt=NULL;
		if ( $feat['quan']<=0 ) $warntxt='no quantity';
		if ( $feat['hunitNa']==NULL ) $errtxt='no unit given in abstract object';
		else {	
			if ( $feat['hunitFac']==NULL )$errtxt='no valid measurement factor for "'.$feat['hunitNa'].'".';
		}
		if ($errtxt!=NULL) {
			$error->set( $FUNCNAME, 1, $errtxt );
		}
	}
	
	function getSubstFeatures() {
		return($this->feat);
	}
	
	/**
	 * generate cache for abstract_proto_steps
	 * @return 
	 * @param object $sqlo2
	 * @param object $aid
	 * @param object $stepnr
	 * @global $this->absProts
	 */
	function _getFacByAbsProt( &$sqlo, $aid, $step) {
		$sqlsel = "H_UNIT_ID, QUANTITY from ABSTRACT_PROTO_STEP where ".
				" ABSTRACT_PROTO_ID=".$aid." and STEP_NR=".$step;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$hunit  = $sqlo->RowData[0];
		$defQuant  = $sqlo->RowData[1];
		
		if (!$hunit) {
			$this->absProts[$aid][$step] = array('f'=>0, 'q'=>$defQuant);
			return;
		}
		
		$fac = $this->UnitLib->getFactorByID($sqlo, $hunit);
		$this->absProts[$aid][$step]=array('f'=>$fac, 'q'=>$defQuant);
		
		return ($fac);
	}
	function getAbsProtoCache() {
		return ($this->absProts);
	}
	
	function startQuery(&$sqlo) {
		$id = $this->id;
		$sqlsel = "CONCRETE_PROTO_ID, ABSTRACT_PROTO_ID, QUANTITY, STEP_NR from CONCRETE_PROTO_STEP where ".
				"CONCRETE_SUBST_ID=".$id. " and NOT_DONE=0 order by CONCRETE_PROTO_ID";
		$sqlo->Quesel($sqlsel);
	}
	
	function ReadRow(&$sqlo) {
		return ( $sqlo->ReadRow() );
	}
	
	/**
	 * - analyse one protocol step 
	 * - if QUANTITY=NULL : use quantity of  abstract_proto_step
	 * @return 
	 * @param object $sqlo
	 * @param object $sqlo2
	 */
	function getRow( &$sqlo, &$sqlo2) {
		
		$this->_oneRow = NULL;
		$this->_oneRow['cid']  = $sqlo->RowData[0];
		$aid  = $sqlo->RowData[1];
		$quan = $sqlo->RowData[2];
		$step = $sqlo->RowData[3];
		
		if (!isset($this->absProts[$aid][$step])) {
			$fac = $this->_getFacByAbsProt($sqlo2, $aid, $step);
		}
		$fac = $this->absProts[$aid][$step]['f'];
		
		if ($quan===NULL) { // no value set
			$quan = $this->absProts[$aid][$step]['q']; // use default value
		}
		
		$this->_oneRow['aid'] = $aid;
		$this->_oneRow['quan']= $quan;
		$this->_oneRow['step']= $step;
		$this->_oneRow['fac'] = $fac;
		if ($fac>0 and $quan>=0) {  
			$this->_oneRow['quaSi'] = $quan * $fac;
		} else $this->_oneRow['quaSi'] = NULL;
		
		return ($this->_oneRow);
	}
	
	/**
	 * - calc consumed amount of material
	 * - could be thousands of steps !
	 * - search in CONCRETE_PROTO_STEP
	 * @return 
	 * @param object $sqlo
	 */
	function calcConsum(&$sqlo, &$sqlo2) {
		$infotxt   = NULL;
		$quanSiSum = 0;
		$this->absProts = array(); // array(id) = Factor
		
		$this->startQuery($sqlo);
		$cnt=0;
		while ( $this->ReadRow($sqlo) ) {
			$result = $this->getRow($sqlo, $sqlo2);
			if ($result['quaSi']>=0) {  // what to do ?
				$quanSiSum = $quanSiSum + $result['quaSi'];
			}
			$cnt++;
		}
		
		
		
		
		$retarr = array('sum'=>$quanSiSum, 'cnt'=>$cnt, 'info'=>$infotxt);
		return ($retarr);
	}

}
