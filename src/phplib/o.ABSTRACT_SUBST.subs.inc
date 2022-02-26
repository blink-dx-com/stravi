<?php

/**
 * analysis methods of ABSTRACT_SUBST
 * @package o.ABSTRACT_SUBST.subs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oABSTRACT_SUBST_subs {
	
	/**
	 * get META children in ABS_CHILD_SUBST of mother
	 * @param $sqlo
	 * @param $absSubstID
	 * @return array
	 */
	public static function get_META_children(&$sqlo, $absSubstID) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;

		$MAXCNT = 1000;
		$sqlsel = "CHILD_SUBST_ID from ABS_CHILD_SUBST where ABSTRACT_SUBST_ID=".$absSubstID.' order by POS';
		$sqlo->Quesel($sqlsel);
		$cnt=0;
		$substarr=NULL;
		while ( $sqlo->ReadRow() ) {
			if ($cnt>$MAXCNT) {
				$error->set( $FUNCNAME, 1, 'Too many objects (>'.$MAXCNT.')' );
				return;
			}
		    $objid = $sqlo->RowData[0];
		    $substarr[]=$objid;
		}
		return $substarr;
	}
	
	/**
	 * analyse, if $child is real part of $mother
	 * @param  $sqlo
	 * @param  $mother
	 * @param  $child
	 * @return 0,1
	 */
	public function suaIsChild(&$sqlo, $mother, $child) {
		$sqlsel = "CHILD_SUBST_ID from ABS_CHILD_SUBST where ABSTRACT_SUBST_ID=".$mother.
			' and CHILD_SUBST_ID='.$child;
		$sqlo->Quesel($sqlsel);
		if ($sqlo->ReadRow() ) {
			return 1;
		}
		return 0;
	}
}