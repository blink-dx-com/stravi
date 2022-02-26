<?php
/**
 * - plugin for device (table: CHIP_READER)
 * - activate device-protocol + archive
 * - used for: CHIP_READER: (CONCRETE_PROTO_ID, QC_CPROTO_ID)
 * 
 * $Header: trunk/src/plugin/o.CHIP_READER.protArch.inc 59 2018-11-21 09:04:09Z $
 * @package o.CHIP_READER.protArch
 * @swreq UREQ:FS-LIM08 Support device/instrument development
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $_REQUEST['id']
 * @param $_REQUEST['action']=
 		'commit' : commit protocol CONCRETE_PROTO_ID
 		'new'    : create a new protocol as copy from last for CONCRETE_PROTO_ID
 		'commitqc' : commit protocol QC_CPROTO_ID
 		'newqc'    : create a new protocol as copy from last for QC_CPROTO_ID
 */
 
require_once ( 'access_lock.inc');
require_once ( "f.assocUpdate.inc");
require_once ( "javascript.inc" );
require_once ( 'glob.obj.copyobj1.inc');
require_once ( "glob.obj.update.inc");

class o_CHIP_READER_protArch_XPL extends gPlugin {
	var $CLASSNAME='o_CHIP_READER_protArch_XPL';
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'CHIP_READER';
		$title		= 'activate device-protocol (compo or QC) + archive';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('write'), 'obj'=>array('insert') )
				);
	}
	
	function finish() {
		global $error;
		// check for errors
		if ($error->Got(READONLY))  {
			return;
		}
		echo 'o.k.'."<br>\n";
		
		$url='edit.tmpl.php?t=CHIP_READER&id='.$_REQUEST['id'];
		js__location_replace($url, 'Back to '.tablename_nice2('CHIP_READER') );
	}
	
	/**
	 * - check: protocol already on archive ???
	 * - lock the protocol (only read)
	 * - add protocol to archive
	 * @param string $proto_type 'COMPO', 'QC'
	 */
	function commitX($proto_type) {
		global $error;
		$FUNCNAME= 'commitX';
		
		$sqlo  = &$this->_sqlo;
		$objid = $_REQUEST['id'];
		
		$assoctable='REA_PRO_ARCH';
		$archNiceTable = tablename_nice2($assoctable);
		$col_name = NULL;
		
		switch ($proto_type) {
			case 'COMPO':
				$col_name = 'CONCRETE_PROTO_ID';
				break;
			case 'QC':
				$col_name = 'QC_CPROTO_ID';
				break;
			default:
				$error->set( $FUNCNAME, 2, 'wrong input parameter proto_type!' );
				return;	
		}
		
		$sqlsel = $col_name. ' from CHIP_READER where CHIP_READER_ID='.$objid;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$cpid = $sqlo->RowData[0];
		if (!$cpid) {
			$error->set( $FUNCNAME, 1, 'No protocol on device '.$objid );
			return;
		}
		$sqlsel = 'CONCRETE_PROTO_ID from REA_PRO_ARCH '.
			' where CHIP_READER_ID='.$objid.' and CONCRETE_PROTO_ID='.$cpid;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$archCpid = $sqlo->RowData[0];
		if ($archCpid) {
			$error->set( $FUNCNAME, 2, 'Protocol '.$cpid.' already on '.$archNiceTable );
			return;
		}
		
		// check exec-date
		$sqlsel = 'EXEC_DATE from CONCRETE_PROTO where CONCRETE_PROTO_ID='.$cpid;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$execdate = $sqlo->RowData[0];
		if ($execdate==NULL) {
			$error->set( $FUNCNAME, 3, 'exec_date must be given in protocol '.$cpid );
			return;
		}
		
		
		$sqlsel = 'max(VERSID) from REA_PRO_ARCH where CHIP_READER_ID='.$objid;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$lastVers = $sqlo->RowData[0];
		if (!$lastVers) $lastVers=0;
		$newvers = $lastVers+1;
		
		$objLockLib = new gObjaccLock();
		$objLockLib->accessLockOne( $sqlo, 'CONCRETE_PROTO', $cpid);
		
		
		$assoclib = new  fAssocUpdate();
		$assoclib->setObj( $sqlo, $assoctable, $objid );
		$argu=array( 'VERSID'=>$newvers, 'CONCRETE_PROTO_ID'=>$cpid );
		$assoclib->insert( $sqlo, $argu );
		
		$this->finish();
		
	}
	
	
	/**
	 * create a new protocol as copy from last
	 * @param string $proto_type 'COMPO', 'QC'
	 */
	function newX($proto_type) {
		global $error;
		$FUNCNAME= 'newX';
		
		$sqlo  = &$this->_sqlo;
		$objid = $_REQUEST['id'];
		$this->_plug_regSql2();
		$sqlo2 = &$this->_sqlo2;
		
		switch ($proto_type) {
			case 'COMPO':
				$col_name = 'CONCRETE_PROTO_ID';
				break;
			case 'QC':
				$col_name = 'QC_CPROTO_ID';
				break;
			default:
				$error->set( $FUNCNAME, 2, 'wrong input parameter proto_type!' );
				return;
		}
		
		$cpid = glob_elementDataGet( $sqlo, 'CHIP_READER', 'CHIP_READER_ID', $objid, $col_name);
		if (!$cpid) {
			$error->set( $FUNCNAME, 1, 'No protocol on device '.$objid );
			return;
		}
		
		// check if committed
		$sqlsel = 'CONCRETE_PROTO_ID from REA_PRO_ARCH '.
			' where CHIP_READER_ID='.$objid.' and CONCRETE_PROTO_ID='.$cpid;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$archCpid = $sqlo->RowData[0];
		if (!$archCpid) {
			$error->set( $FUNCNAME, 2, 'Protocol '.$cpid.' not yet committed.' );
			return;
		}
		
		// make copy
		$copyLib = new gObjCopyS();
		$new_cpid = $copyLib->copyOne( $sqlo, $sqlo2, 'CONCRETE_PROTO', $cpid );
		if (!$new_cpid) {
			$error->set( $FUNCNAME, 3, 'Error on create of new protocol copy.' );
			return;
		}
		
		// change link of CHIP_READER to CONCRETE_PROTO
		$args=array( 'vals'=> array($col_name => $new_cpid) );
		$UpdateLib = new globObjUpdate();
		$UpdateLib->update_meta( $sqlo, 'CHIP_READER', $objid, $args );
		
		$this->finish();
	}
	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		switch ($_REQUEST['action']) {
			case 'commit':
				$this->commitX('COMPO');
				break;
			case 'new':
				$this->newX('COMPO');
				break;
			case 'commitqc':
					$this->commitX('QC');
					break;
			case 'newqc':
					$this->newX('QC');
					break;
			default:
				$error->set( $FUNCNAME, 1, 'Action unknown.' );
				return;
		}
	}
}
