<?php
/**
 * - plugin to create a QC-protocol for CHIP_READER
 * @package o.CHIP_READER.qcProtCrea.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $_REQUEST['id'] id of CHIP_READER
 *   
 */

require_once ("insertx.inc");
require_once ("javascript.inc" );
require_once ("glob.obj.update.inc");
require_once ("o.CHIP_READER.qcprot.inc");

class ocCHIP_READER_qcProtoCreaG {
	var $CLASSNAME='ocCHIP_READER_qcProtoCreaG';
	var $infox; /*
		'STEP_NR' sample : step-NR
	*/
	
	
	function __construct(&$sqlo, $device_id) {
		global $error;
		$FUNCNAME = $this->CLASSNAME.':__init__';
		$this->device_id = $device_id;
		$this->tablename = 'CHIP_READER';
		$this->tableNice = tablename_nice2('CHIP_READER');
		$this->absTableNice = tablename_nice2('A_CHIP_READER');
		
	}
	
	/**
	 * check parameters
	 * @param $sqlo
	 * @return unknown_type
	 */
	function checkParams( &$sqlo ) {
		global $error;
		$FUNCNAME = $this->CLASSNAME.':checkParams';
		
		$sqlsel = '* from CHIP_READER where CHIP_READER_ID='.$this->device_id;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadArray();
		$feats = $sqlo->RowData;
		
		$this->infox['A_CHIP_READER_ID'] = $feats['A_CHIP_READER_ID'];
		$this->infox['QC_CPROTO_ID']     = $feats['QC_CPROTO_ID'];
		if ($this->infox['QC_CPROTO_ID']) {
			$error->set( $FUNCNAME, 3, $this->tableNice.' already has a QC-protocol.' );
			return;
		}
		$absDeviceID = $this->infox['A_CHIP_READER_ID'];
		if (!$absDeviceID) {
			$error->set( $FUNCNAME, 2, 'device ['.$this->device_id.'] has nor abstract device.' );
			return;
		}
		
		$aDeviceQcLib = new oA_CHIP_READER_QCprot( $sqlo );
		$absQcProtoID = $aDeviceQcLib->getQcProtoId($sqlo, $absDeviceID );
		
		if (!$absQcProtoID) {
			$error->set( $FUNCNAME, 4, $this->absTableNice.' ['.$absDeviceID.'] has no planned QC-protocol.' );
			return;
		}
		$answer = $aDeviceQcLib->isValid($sqlo, $absQcProtoID);
		if ($answer[0]<1) {
			$error->set( $FUNCNAME, 5, 'protocol (abstract) ['.$absQcProtoID.'] :QC-protocol check failed: '.$answer[1] );
			return;
		}
		
		$this->infox['QC_ABS_PROTO_ID'] = $absQcProtoID;
		
	}
	
	/**
	 * create protocol
	 * - put DUMMY_SAMPLE to the sample-step
	 * - add protocol to substance
	 * @param $sqlo
	 * @return -
	 */
	function create(&$sqlo) {
		global $error;
		$FUNCNAME = $this->CLASSNAME.':create';
		
		$aProtoID = $this->infox['QC_ABS_PROTO_ID'];
		$device_id  = $this->device_id;
		$error_SUM= NULL;
		
		$InsertLib = new insertC(); 
		$argu=NULL;
		$argu['ABSTRACT_PROTO_ID'] = $aProtoID;
		$protoID = $InsertLib->new_meta($sqlo, 'CONCRETE_PROTO', array( "vals"=>$argu ) );
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'error on creation of protocol' );
			return;
		}
		echo '- protocol ID:'.$protoID.' created.<br />';
		
		// attach protocol to device $device_id
		$UpdateLib = new globObjUpdate();
		$args = array( 'vals'=> array('QC_CPROTO_ID'=>$protoID) );
		$UpdateLib->update_meta( $sqlo, $this->tablename, $device_id, $args );
		
		if ($error_SUM!=NULL) {
			$error->set( $FUNCNAME, 1, 'Previous error: '.$error_SUM );
			return;
		}
	}
}


class o_CHIP_READER_qcProtCrea_XPL extends gPlugin {
	var $CLASSNAME='o_CHIP_READER_qcProtCrea_XPL';
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'CHIP_READER';
		$title		= 'Create a QC-protocol';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = 'Create QC-protocol';
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read','write'), 'obj'=>array('insert') )
				);
				
		
	}
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$this->objid   = $_REQUEST['id'];
		$sqlo = &$this->_sqlo;
		
		$MainLib = new ocCHIP_READER_qcProtoCreaG($sqlo, $this->objid);
		if ($error->Got(READONLY))  {
			return;
		}
		
		$MainLib->checkParams($sqlo) ;
		if ($error->Got(READONLY))  {
			return;
		}
		
		$MainLib->create($sqlo);
		if ($error->Got(READONLY))  {
			return;
		}
		
		$newurl='edit.tmpl.php?t=CHIP_READER&id='.$this->objid;
		js__location_replace($newurl, 'substance' ); 
		
	}
	
}