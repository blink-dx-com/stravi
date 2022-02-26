<?php
/**
 * - plugin for CHIP_READER: export protocol for EXCEL, CSV
 *    
 * @package o.CHIP_READER.proto_exp.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $parx["protocolType"] 
 *	'normal'=>'[normal protocol]', 
 *	'archive'=>'active archive protocol'
 *   
 */
 
require_once ('gui/g.exportList.inc');
require_once ( "o.PROTO.steps.inc" );
require_once ("o.REA_PRO_ARCH.subs.inc");
 
class oCHIP_READER_proto_export {
	var $CLASSNAME='oCHIP_READER_proto_export';
	var $step_array; // STEP-array
	
	function __construct($objCnt, $sqlAfter, $parx) {
		$this->modname = 'temp';
		$this->filename= 'deviceProtocols.xls';
		
		$this->objCnt=$objCnt;
		$this->sqlAfter = $sqlAfter;
		$this->parx = $parx;
		
		$this->protocolType='normal';
		if ($this->parx['protocolType']!=NULL) $this->protocolType=$this->parx['protocolType'];
		
		$this->protArchLib = NULL;
		if ($this->protocolType=='archive') {
			$this->protArchLib = new oREA_PRO_ARCH_subs();
		}
	}
	
	/**
	 * get header-array for device protocol
	 *  0 : ID
	 *  1 : device-name
	 *  2 : 
	 * @global $this->step_array
	 * @return $headerarr
	 * @param object $sqlo
	 */
	function getHeaderArray(&$sqlo) {
		$AbsConcSumCols = array('NAME', 'ABSTRACT_SUBST_ID', 'ABS_DEV_ID', 'QUANTITY', 'H_UNIT_ID');
		
		$headerarr = NULL;
		$absProtoID = $this->absProtoID;
		
		$headStartArr = array();
		$headStartArr[0]='';
		$headStartArr[1]='';
		$headStartArr[2]='';
		$headStartArr[3]='';
		$headStartArr[4]='';
		
		// 1. ----------
    
		$headCols = array();
		$headCols[0]='ID';
		$headCols[1]='Name';
		$headCols[2]='PRC-ID';
		$headCols[3]='Exec date';
		$headCols[4]='Info';
	
		
		$concreteColNames = $this->protLib->getCSumCols();
		
		reset ($this->step_array);
		foreach( $this->step_array as $dummy=>$stepnr) {
			$sqlsel = '* from ABSTRACT_PROTO_STEP where '.
				' ABSTRACT_PROTO_ID='.$absProtoID.' and STEP_NR='.$stepnr;
			$sqlo->Quesel($sqlsel);
			$sqlo->ReadArray();
			$absParams = $sqlo->RowData;
			
			//if ($absParams['ABSTRACT_SUBST_ID']>0) 
			$headCols[]=$absParams['NAME']; // Step-Name
			//if ($absParams['ABS_DEV_ID']>0) 
			$headCols[]=NULL; // dev
			$headCols[]=NULL;
			$headCols[]=NULL;
			
		}
		reset ($this->step_array); 
		$headerarr[] = $headCols;
		
		// 2. ----------
		$headCols = $headStartArr; 
		
		reset ($this->step_array);
		foreach( $this->step_array as $dummy=>$stepnr) {
			
			//if ($absParams['ABSTRACT_SUBST_ID']>0) 
			$headCols[]='SUBST';
			//if ($absParams['ABS_DEV_ID']>0) 
			$headCols[]='DEV';
			$headCols[]='QUANT';
			$headCols[]='NOTES';
			
		}
		reset ($this->step_array); 
		
		$headerarr[] = $headCols;
		
		// 3. ----------
		
		$headCols = $headStartArr; // 4 or 5 empty cols
		
		reset ($this->step_array);
		foreach( $this->step_array as $dummy=>$stepnr) {
			$sqlsel = '* from ABSTRACT_PROTO_STEP where '.
				' ABSTRACT_PROTO_ID='.$absProtoID." and STEP_NR=".$stepnr;
			$sqlo->Quesel($sqlsel);
			$sqlo->ReadArray();
			$absParams = $sqlo->RowData;
			
			$absSubstName = NULL;
			if ($absParams['ABSTRACT_SUBST_ID']>0) {
				$absSubstName = obj_nice_name ( $sqlo, 'ABSTRACT_SUBST', $absParams['ABSTRACT_SUBST_ID'] );
			}
			$absName = NULL;
			if ($absParams['ABS_DEV_ID']>0) {
				$absName = obj_nice_name ( $sqlo, 'A_CHIP_READER', $absParams['ABS_DEV_ID'] );
			}
			$quanti = $absParams['QUANTITY'];
			if ($absParams['H_UNIT_ID']>0) {
				$quanti .= ' ['.obj_nice_name ( $sqlo, 'H_UNIT', $absParams['H_UNIT_ID'] ).']';
			}
			
			$headCols[]=$absSubstName;		
			$headCols[]=$absName;
			$headCols[]=$quanti;
			$headCols[]=$absParams['NOTES'];
			
		}
		reset ($this->step_array);
		
		$headerarr[] = $headCols;
		
		return $headerarr;
		
	}
	
	/**
	 * callback function for a single protocol row
	 * @return 
	 * @param object $sqlo
	 * @param object $params
	 */
	function singleRow(&$sqlo, $params) {
		
		$outarr = NULL;
		
		$objid = $params[0];
		$sqlsel = 'NAME, CONCRETE_PROTO_ID from CHIP_READER where CHIP_READER_ID='.$objid;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$dev_name = $sqlo->RowData[0];
		$CONCRETE_PROTO_ID = $sqlo->RowData[1];
		
		if ($this->protocolType=='archive') {
			$CONCRETE_PROTO_ID = 0;
			$CONCRETE_PROTO_ID = $this->protArchLib->getLastProto($sqlo, $objid);
		} 
		
		$this_abs_proto_id = 0;
		$this_exec_date = NULL;
		
		if ($CONCRETE_PROTO_ID) {
			$sqlsel = 'ABSTRACT_PROTO_ID, EXEC_DATE from CONCRETE_PROTO where CONCRETE_PROTO_ID='.$CONCRETE_PROTO_ID;
			$sqlo->Quesel($sqlsel);
			$sqlo->ReadRow();
			$this_abs_proto_id = $sqlo->RowData[0];
			$this_exec_date    = $sqlo->RowData[1];
		}
		$outarr[] = $objid;
		$outarr[] = $dev_name;
		$outarr[] = $CONCRETE_PROTO_ID;
		$outarr[] = $this_exec_date;
		$info=NULL;
		if ($this_abs_proto_id!=$this->absProtoID) {
			$info='other abstract protocol';
			$outarr[] = $info;
			return $outarr;
		} 
		$outarr[] = $info;
		
		reset ($this->step_array);
		foreach( $this->step_array as $dummy=>$stepnr) {
			$sqlsel = '* from CONCRETE_PROTO_STEP where '.
				' CONCRETE_PROTO_ID='.$CONCRETE_PROTO_ID." and STEP_NR=".$stepnr;
			$sqlo->Quesel($sqlsel);
			$sqlo->ReadArray();
			$stepParams = $sqlo->RowData;
			
			$substName = NULL;
			if ($stepParams['CONCRETE_SUBST_ID']>0) {
				$substName = obj_nice_name ( $sqlo, 'CONCRETE_SUBST', $stepParams['CONCRETE_SUBST_ID'] );
			}
			$devName = NULL;
			if ($stepParams['DEV_ID']>0) {
				$devName = obj_nice_name ( $sqlo, 'CHIP_READER', $stepParams['DEV_ID'] );
			}
			
			
			$outarr[]=$substName;
			$outarr[]=$devName;
			$outarr[]=$stepParams['QUANTITY'];
			$outarr[]=$stepParams['NOTES'];
			
		}
		reset ($this->step_array); 
		
		return $outarr;
	}
	
	function _getFirstProtocol(&$sqlo) {
		
		$sqlsel = 'x.CHIP_READER_ID from '.$this->sqlAfter;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$firstDevice = $sqlo->RowData[0];
		
		if (!$firstDevice) return;
		
		if ($this->protocolType=='archive') {
			$firstCProtoID = $this->protArchLib->getLastProto($sqlo, $firstDevice);
		} else {
			$sqlsel = 'CONCRETE_PROTO_ID from CHIP_READER where CHIP_READER_ID='.$firstDevice;
			$sqlo->Quesel($sqlsel);
			$sqlo->ReadRow();
			$firstCProtoID = $sqlo->RowData[0];
		}
		
		if (!$firstCProtoID) return;
		
		$sqlsel = 'ABSTRACT_PROTO_ID from CONCRETE_PROTO where CONCRETE_PROTO_ID='.$firstCProtoID;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$absProtoID = $sqlo->RowData[0];
		
		return $absProtoID;
	}
	
	// --------
	
	function form1(&$formobj) {
		
		$fieldx = array ( 
			"title" => "Protocol type", 
			"name"  => "protocolType",
			"object"=> "select",
			"val"   => $this->protocolType, 
			"inits" => array('normal'=>'[normal protocol]', 'archive'=>'active archive protocol'),
			"notes" => "which protocol?"
			 );
		$formobj->fieldOut( $fieldx );
		$formobj->close( TRUE );
	}
	
	function start(&$sqlo, &$sqlo2) {
		global $error;
		$FUNCNAME= $this->CLASSNAME.':start';
		
		$format='xls';
		
		$sqlsMain = 'CHIP_READER_ID from '.$this->sqlAfter;
		
		$absProtoID = $this->_getFirstProtocol($sqlo);
		if (!$absProtoID) {
			$error->set( $FUNCNAME, 1, 'first device has no protocol.' );
			return;
		}
		$this->absProtoID = $absProtoID;
		
		$this->protLib = new gProtoOrg( );
		$this->protLib->setProto( $sqlo, $absProtoID, 0);
		$this->step_array = &$this->protLib->step_array;
		
		$headerarr = $this->getHeaderArray($sqlo);
		
		
		$expoptions = array('selectCnt'=>$this->objCnt);
		$exportLib = new g_exportList_C($sqlo, $format, $this->modname, $sqlsMain, $expoptions);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'init export-class failed' );
			return;
		}
		
		$doForward = $exportLib->outputStart($sqlo, $sqlo2, $this, $headerarr);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 2, 'export failed' );
			return;
		}
		
		if ($doForward) {
			$exportLib->forwardPage();
		}
		
		
	}
	
	
	
}


/**
 * @author steffen
 *
 */
class o_CHIP_READER_proto_exp_XPL extends gPlugin {
	var $CLASSNAME='o_CHIP_READER_proto_exp_XPL';
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'CHIP_READER';
		$title		= 'Export device protocols';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_cnt']  = 1;
		
		$this->infoarr2 = array( 
			'accCheck' => array('tab'=>array('read'), 'obj'=>array() ),
			'listChkSelect' => 1,
			'progressBar'   => 1
			);
				
		
	}
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$this->objid   = $_REQUEST['id'];
		$go    = $_REQUEST['go'];
		$parx  = $_REQUEST['parx'];
		$sqlo  = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		$objCnt   = $this->listLib->getObjCnt();
		$sqlAfter = $this->listLib->getSqlAfter();
		
		
		$mainLib = new oCHIP_READER_proto_export($objCnt, $sqlAfter, $parx);
		
		if (!$go) {
			$initarr=NULL;
			$initarr["title"]       = "Select protocol mode";
			$initarr["submittitle"] = "Export";
			$hiddenarr=NULL;
			$formLib = $this->_html->formInit($initarr, $hiddenarr);
			$mainLib->form1($formLib);
			return;
		}
		
		$mainLib->start($sqlo, $sqlo2);
	}
	
}