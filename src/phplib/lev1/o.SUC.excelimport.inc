<?php
require_once 'insertx.inc';
require_once 'o.PROTO.upsteps.inc';
require_once 'o.CS_HAS_PR.subs.inc';
require_once 'o.AS_HAS_PR.subs.inc';
require_once 'f.assocUpdate.inc';
require_once 'o.S_VARIO.subs.inc';

/**
 * manage import of excel sheet cell data into a protocol of a SUC
 * @package o.SUC.excelimport.inc
 * @swreq UREQ:12963 Material Monitoring Concept 2018
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/lev1/o.SUC.excelimport.inc 59 2018-11-21 09:04:09Z $
 */
class oSUC_excelimport {
	
	private $_initialized = 0;
	
	/**
	 * update protcol steps
	 * @param unknown $sqlo
	 * @param unknown $cpid
	 * @param int $pra_id
	 * @param array $data_dict
	 *   [step][type] = value
	 *    supported types:
	 *       q: QUANTITY
	 *       n: NOTES
	 */
	private function update_proto($sqlo, $cpid, $pra_id, $data_dict) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		$error->infolog_set($FUNCNAME,'CPID:'.$cpid.' update protocol with data ');
	
		
		$stepArr = array();
	
		foreach($data_dict as $step => $valarr) {
			
			$oneStepArr=array();
			if (array_key_exists('q', $valarr) ) {
				$quant = trim($valarr['q']);
				if ($quant!==NULL and $quant!=="") {
					if (!is_numeric($quant)) {
						
						$error->set( $FUNCNAME, 1, 'Value:"'.$quant.'" (Step:"'.$step.'") is not a number.' );
						return;
					}
				}
				$oneStepArr['QUANTITY'] = $quant;
			}
			if (array_key_exists('n', $valarr) ) {
				$strval = trim($valarr['n']);
				$oneStepArr['NOTES'] = $strval;
			}
			
			$stepArr[$step] = $oneStepArr;
			
		}
	
		$protoUpdLib = new oConcProtoUpSteps();
		$protoUpdLib->setProto($cpid);
		$protoUpdLib->protoUpdate( $sqlo, $pra_id, $stepArr );
	
	}
	
	/**
	 * create protocol and steps
	 * @param unknown $sqlo
	 * @param unknown $pra_id
	 * @param array $data_dict
	 *   KEY => VAL (number)
	 */
	private function create_proto($sqlo, $pra_id, $data_dict) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;

		$insertlib = new insertC();
		$feature_array=array();
		$feature_array['ABSTRACT_PROTO_ID'] = $pra_id;
		$args= array( "vals"=>$feature_array );
		$cpid = $insertlib->new_meta($sqlo, 'CONCRETE_PROTO', $args);
	
		if (!$cpid) {
			$error->set( $FUNCNAME, 2, 'No protocol created.' );
			return;
		}
	
		$error->infolog_set($FUNCNAME,'new CPID');
	
	
		return $cpid;
	}
	
	/**
	 * init SUC for EXCEL import
	 * @param object $objid
	 * @param string $pra_IdentKey - identificator of the PROTOCOL_ABSTRACT: vario: key="ATPL.excelimport", VAL= e.g. "BTRTaq"
	 * @param string $doc_path
	 * @param array   $excel_config - configuration of the import data
	 *   'sheets'=> array of array ( 
	 *       'sheet'=>name, 
	 *       'cellmap'=> array of array(
	 *     	 	'row'=>number, 'col'=>col-letter, 'type'=>(q,n,s,d), 'st'=>protocol_step_number)
	 *       )
	 *    )
	 */
	function set_data($sqlo, $objid, $pra_IdentKey, $doc_path, $excel_config) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->pra_IdentKey = $pra_IdentKey;
		$this->pra_id   = 0;
		$this->objid    = $objid;
		$this->doc_path = $doc_path;
		$this->excel_config = $excel_config;
		
		if ($this->doc_path==NULL)  {
			$error->set( $FUNCNAME, 1, 'no doc_path given.' );
			return;
		}
		
		$this->_initialized = 1;
		
	}
	
	
	/**
	 * get needed ABSTRACT_PROTO_ID from SUA
	 * - ABSTRACT_PROTO_ID is identified by a VARIO key
	 * OUTPUT: $this->pra_id 
	 * @param object $sqlo
	 */
	function get_protocol_abstract(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		if (!$this->_initialized) {
			sys_error_my( 'class not inizialized.' );
		}
		
		$this->pra_id = 0;
		
		if ($this->pra_IdentKey==NULL) {
			$error->set( $FUNCNAME, 1, 'no pra_IdentKey given.' );
			return;
		}
		
		//
		// now search for the ABSTRACT_PROTO with VARIO:ATPL.excelimport = $this->pra_IdentKey
		//
		$sua_id = glob_elementDataGet( $sqlo, 'CONCRETE_SUBST', 'CONCRETE_SUBST_ID', $this->objid, 'ABSTRACT_SUBST_ID');
		
		$variolib = new oS_VARIO_sub('ABSTRACT_PROTO');
		$vario_KEY= 'ATPL.excelimport';
		
		$found_pra_id = 0;
		
		$AS_HAS_PR_lib   = new oAS_HAS_PR_subs();
		$AS_HAS_PR_lib->setObj($sua_id);
		$a_proto_log = $AS_HAS_PR_lib->getProtoLog($sqlo);
		
		
		
		foreach($a_proto_log as $row) {
			
			$pra_id = $row['ap'];
			
			// get vario val
			if ($pra_id) $vario_val = $variolib->getValByKey( $sqlo, $pra_id, $vario_KEY );
			else $vario_val = NULL;
			
			if ($vario_val == $this->pra_IdentKey) {
				$found_pra_id = $pra_id;
				break;
			}
		}
		
		if (!$found_pra_id) {
			$error->set( $FUNCNAME, 2, 'on SUA (ID:'.$sua_id.') no protocol (abstract) '.
				' found with VARIO:'.$vario_KEY.'='.$this->pra_IdentKey );
			return;
		}
		
		$this->pra_id = $found_pra_id;
	}
	
	/**
	 * get data from excel
	 * @param unknown $sqlo
	 * @return array $data_dict
	 */
	function get_excel_data(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		if (!$this->_initialized) {
			sys_error_my( 'class not inizialized.' );
		}
		
		$doc_path     = $this->doc_path;
		$excel_config = $this->excel_config;
		
		if (!array_key_exists('sheets', $excel_config) ) {
			$error->set( $FUNCNAME, 1, 'excel_config: no sheets key.' );
			return;
		}
		
		$phpexcel_include = 'ots/PHPExcel/PHPExcel.php';
		if (!stream_resolve_include_path($phpexcel_include)) {
			$error->set( $FUNCNAME, 2, 'Missing PHPExcel library. Please inform the admin.' );
			return;
		}
		require_once 'ots/PHPExcel/PHPExcel.php';
		
		
		try {
			$inputFileType= 'Excel2007';
			//  Create a new Reader of the type defined in $inputFileType
			$objReader = PHPExcel_IOFactory::createReader($inputFileType);
			
			//  Advise the Reader of which WorkSheets we want to load
			//$objReader->setLoadSheetsOnly($sheetname);
			
			$objPHPExcel = $objReader->load($doc_path);
			
			$cnt = $objPHPExcel->getSheetCount();
	
				
			foreach( $excel_config['sheets'] as $onesheet) {
			
				if (!array_key_exists('sheet', $onesheet) ) {
					$error->set( $FUNCNAME, 3, 'excel_config: sheet has no sheet-name.' );
					return;
				}
				$sheetname= $onesheet['sheet'];
			
				$objPHPExcel->setActiveSheetIndexByName($sheetname);
				//$objReader->setSheetIndex(1);
				$sheetData = $objPHPExcel->getActiveSheet()->toArray(null,true,true,true);
					
				/**
				 * array ( [STEP_ID][TYPE] = dataval )
				*/
				$data_dict = array();
				foreach($onesheet['cellmap'] as $cellinfo) {
					$data_dict[ $cellinfo['st'] ][ $cellinfo['type'] ] = $sheetData[$cellinfo['row']][$cellinfo['col']];
			
				}
				// write dict-data to protocol
				// get next free proto-pos on SUC
			}
		}  catch(Exception $e) {
			$error->set( $FUNCNAME, 2, 'Attachment:'.$doc_path.'; Error loading file:'.$e->getMessage() );
			return;
		}
		
		return $data_dict;
	}
	
	/**
	 * start the import
	 * @param unknown $sqlo
	 
	 */
	function do_import($sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		if (!$this->_initialized) {
			sys_error_my( 'class not inizialized.' );
		}
		
		$this->get_protocol_abstract($sqlo);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'error on getting protocol (abstract).' );
			return;
		}
		debugOut('PRA_ID:'.$this->pra_id , $FUNCNAME, 1);
		
		$data_dict = $this->get_excel_data($sqlo);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 2, 'error on EXCEL analysis.' );
			return;
		}
		debugOut('Excel-mapping: num of data-vals:'.sizeof($data_dict) , $FUNCNAME, 1);
		
		$pra_id = $this->pra_id;
				
		$CS_HAS_PR_lib   = new oCS_HAS_PR_subs();
		$CS_HAS_PR_lib->setObj($this->objid);
		$proto_log = $CS_HAS_PR_lib->getProtoLog($sqlo);
		$prc_exists = 0;
		$last_order = 0;
		$cpid = 0;
			
		if (sizeof($proto_log)) {
			foreach($proto_log as $row) {
				$prc_loop = $row['cp'];
				if ($row['or']< 9998) $last_order    = $row['or'];
				if ($prc_loop) $pra_loop = glob_elementDataGet( $sqlo, 'CONCRETE_PROTO',
						'CONCRETE_PROTO_ID', $prc_loop, 'ABSTRACT_PROTO_ID');
					
				if ($pra_id==$pra_loop) {
					$prc_exists = 1;
					$cpid = $prc_loop;
					break;
				}
			}
		}
			
		if (!$prc_exists) {
	
			// create protocol
			$cpid = $this->create_proto($sqlo, $pra_id, $data_dict);
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 3, 'No data-protcol created.' );
				return;
			}
	
			$this->update_proto($sqlo, $cpid, $pra_id, $data_dict);
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 4, 'Error on Protocol-Update (CPR-ID:'.$cpid.').' );
				return;
			}
	
			$xorder = $last_order+10;
			$params = array(
					'STEP_NO'=> $xorder,
					'CONCRETE_PROTO_ID'=>$cpid,
					'TYPE'    => 1 // QC
			);
				
			$error->infolog_set($FUNCNAME,'CPID:'.$cpid, 'xorder:'.$xorder);
	
			$assoclib = new  fAssocUpdate();
			$assoclib->setObj( $sqlo, 'CS_HAS_PR', $this->objid );
			$assoclib->insert( $sqlo, $params );
			$assoclib->close($sqlo);
	
		} else {
	
			$this->update_proto($sqlo, $cpid, $pra_id, $data_dict);
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 5, 'Error on Data-protcol '.$cpid.' update.' );
				return;
			}
	
		}
		
		debugOut('Protocol-Update: PRC-ID:'.$cpid , $FUNCNAME, 1);
				
		
	}
}
