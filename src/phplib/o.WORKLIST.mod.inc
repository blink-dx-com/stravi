<?php
require_once ("f.assocUpdate.inc");

/**
 * modification methods for WORKLIST
 * @package o.WORKLIST.mod.inc
 * @swreq UREQ:0000932 db.WORKLIST: Messauftrag organisieren
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oWORKLIST_mod {
	function __construct() {
		$this->assoclib = new  fAssocUpdate();
		$this->objid=NULL;
	}
	
	/**
	 * initialize worklist, prepare for manipulation
	 * @param $sqlo
	 * @param $objid
	 * @return -
	 */
	function setObj(&$sqlo, $objid) {
		$this->objid=$objid;
		$this->assoclib->setObj( $sqlo, 'WORKLIST_ENTRY', $objid );
	}
	
	/**
	 * update a cell for given params
	 * - if cell doeas not exist: create it
	 * @param $sqlo
	 * @param $xy array(x,y)
	 * @param $params array of:  TABLE_NAME, OBJID, KEY, VALUE
	 * @return -
	 */
	function updateCell(&$sqlo, $xy, $params) {
		if (!$this->objid) {
			sys_error_my( 'class is not initialized.' );
		}
		$x=$xy[0];
		$y=$xy[1];
		
		$sqlo->Quesel('1 from WORKLIST_ENTRY where WORKLIST_ID='.$this->objid.' AND XPOS='.$x.' and YPOS='.$y);
		if ($sqlo->ReadRow()) $exists=1;
		else $exists=0;
		
		if ($exists) {
			unset($params['YPOS']);
			unset($params['XPOS']);
			$idarr=array();
			$idarr['YPOS']=$y;
			$idarr['XPOS']=$x;
			$this->assoclib->update( $sqlo, $params, $idarr );
		} else {
			$params['YPOS']=$y;
			$params['XPOS']=$x;
			$this->assoclib->insert( $sqlo, $params );
		}
	}
	
	/**
	 * needed to close the modification process
	 */
	function close(&$sqlo) {
		$this->assoclib->close($sqlo);
	}
}
