<?php
require_once 'glob.obj.copyobj1.inc';
require_once ("glob.obj.update.inc");
require_once ("f.assocUpdate.inc");
require_once 'o.X_ERP_ARTMAP.mod.inc';


/**
 * lib for SUA version management; MODIFICATION methods
 * @package o.SUA.versman.mod.inc
 * @swreq   SREQ:0003315: o.ABSTRACT_SUBST > [VersMan] > library 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/lev1/o.SUA.versman.mod.inc 59 2018-11-21 09:04:09Z $
 */
class oSUA_versman_mod {
	
	private $suaid;
	
	public function __construct() {
		$this->versmanActive=0;
		if (glob_column_exists('ABSTRACT_SUBST', 'VERSFLAG')) {
			$this->versmanActive=1;
		}
		
		$this->versSubLib = new oSUA_versman_sub();
	}
	
	public function versman_active() {
		return $this->versmanActive;
	}
	
	public function setSubst(&$sqlo, $suaid) {
		$this->suaid = $suaid;
	}
	
	/**
	 * check, if a new version is possible
	 * @param unknown_type $sqlo
	 */
	public function copyCheck(&$sqlo) {
		
	}
	
	private function getNewVersNum(&$sqlo, $masterid) {
		$sqlsel = 'max(XVERS) from SUA_VERS where ABSTRACT_SUBST_ID='.$masterid;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$XVERS = $sqlo->RowData[0];
		
		if (!$XVERS) $XVERS=0;
		$newVers = floor($XVERS + 1.0);
		return $newVers;
	}
	
	/**
	 * add substance as version to a master
	 * - create a version entry with AUTO version number
	 * @param  $sqlo
	 * @params array $params version parameters
	 *   'NOTES' - e.g. notes, can be NULL
	 * @param array $options
	 *   "noRightCheck" => 0,1 : for $masterid
	 */
	public function addToMaster(&$sqlo, $masterid, $params, $options=NULL) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		if (!$this->suaid) sys_error_my('class not initialized.');
		
		// check first
		if ($this->versSubLib->getMasterID($sqlo, $this->suaid)>0) {
			$error->set( $FUNCNAME, 1, 'SUA is already part of VersionMaster' );
			return;
		}
		
		// get new version-ID, add to master
		$newVers = $this->getNewVersNum($sqlo, $masterid);
		
		$assoclib = new  fAssocUpdate();
		$assoc_opt=array();
		if ($options["noRightCheck"]>0) {
			$assoc_opt["noAccCheck"] = 1;
		}
		$assoclib->setObj( $sqlo, 'SUA_VERS', $masterid, $assoc_opt );
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'error on master('.$masterid.') init.' );
			return;
		}
		
		$argu = $params;
		$argu['XVERS']    = $newVers;
		$argu['CHILD_ID'] = $this->suaid;
		$assoclib->insert( $sqlo, $argu );
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'error on connecting version('.$this->suaid.') to master('.$masterid.')' );
			return;
		}
		
		
		$UpdateLib = new globObjUpdate();
		$args = array( 'vals'=>array( 'VERSFLAG'=>2 ) );
		$upopt= array( "noRightCheck"=>1 );
		$UpdateLib->update_meta( $sqlo, 'ABSTRACT_SUBST', $this->suaid, $args, $upopt );
		
	}
	
	/**
	 * create a new copy of an existing SUA
	 * - do NOT copy Attachment of type "DMR_sub"
	 * prerequisits:
	 * - Version-Management of SUA must be active,
	 * @swreq   SREQ:0003349: o.ABSTRACT_SUBST > [VersMan] > neue Version anlegen 
	 * @param  $sqlo
	 * @param  $srcid
	 * @return long $newid
	 */
	public function createCopy(&$sqlo, &$sqlo2) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		if (!$this->suaid) sys_error_my('class not initialized.');
		
		$tablename = 'ABSTRACT_SUBST';
		$objCopyLib = new gObjCopyS();
		$creopt = array('attach'=>1, 'attIgnorKey'=>array("DMR_sub") );
		$newid = $objCopyLib->copyOne($sqlo, $sqlo2, $tablename, $this->suaid, $creopt);
		
		if (!$newid) return; // on error
		
		
		return $newid;
	}
	
	
}
