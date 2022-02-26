<?php
/**
 * provide FPDF beans like a complete protocol
 * @package f.fpdf.beans.inc
 * @deprecated
 * @swreq   UREQ:0000212: g.report generator (pdf) 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

require_once ('f.workdir.inc');
require_once ('object.info.inc');
require_once ('f.objview.inc');
require_once ('o.DB_USER.subs.inc');
require_once ('o.SATTACH.mod.inc');
require_once ('o.CCT_ACCLOG.gui.inc');
require_once ('o.S_OBJLINK.subs.inc');

// require_once ( $_SESSION['s_sessVars']['AppLabLibDir'].'/project/107/g.ERP.info.inc');
// require_once ('gui/o.CONCRETE_PROTO.qcform2.inc');
require_once ('gui/f.fpdf.X1.inc');



/**
 * helper methods for reporting engine
 * @deprecated
 */
class gFPDF_help2 {
	
	function __construct() {
	    throw new Exception('DEPRECATED!');
		$this->objSimpleViewL  = new fObjViewC();
	}
	


	/**
	 * get simple object name presentation
	 */
	function getObjNiceName( &$sqlo, $table, $objid) {
		$viewopt    = array('format'=>'csv');
		$objectNice = $this->objSimpleViewL->bo_display( $sqlo, $table, $objid, $viewopt );
		return $objectNice;
	}
	

	/**
	 * get name of abstract subst by ABSTRACT_SUBST_ID
	 * @param $sqlo
	 * @param $csid
	 */
	function getAbsSubstName(&$sqlo, $asid) {
		$substFeats = glob_elemDataGet3( $sqlo, "ABSTRACT_SUBST", array("ABSTRACT_SUBST_ID"=>$asid), 
			array('NAME') ); 
			return $substFeats['NAME'];
	}
	
	/**
	 * get protocol info
	 * @param $sqlo
	 * @param $sqlo2
	 * @param $cpid
	 * @param $shOpt options for $protoLib
	 *   e.g. "protoType"= 'PREP', 'QC'
	 * @return STRUCT protoInfo_STRUCT
	 */
	function getProtoInfo( &$sqlo, $cpid, $shOpt) {
		$protoLib = new oCONCRETE_PROTO_qcform2();
		$a_proto_id=0;
		return $protoLib->showAll($sqlo, $cpid, $a_proto_id, $shOpt );
	}
}

/**
 * provide FPDF beans like a complete protocol
 * @package gFPDF_beans.inc
 * @swreq   UREQ:0000212: g.report generator (pdf) 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @deprecated
 */
class gFPDF_beans {
	
	var $pdf; // object based on phplib/ots/fpdf
	
	/**
	 * init class
	 * @param object $pdf
	 * @param array $infox
	 *   'pageWidth'
	 */
	function __construct(&$pdf, $infox) {
	    throw new Exception('DEPRECATED!');
		$this->pdf = &$pdf;
		$this->infox = $infox;
		
		$this->helpLib = new gFPDF_help2();
		
		$this->_initForm();
		$this->item2Cnt=0;
		
		$this->pageWidth = $infox['pageWidth'];
		
	}
	
	function _initForm() {
		$this->itemCnt=0;
	}
	
	function itemText($text) {
		$this->itemCnt++;
		$this->item2Cnt=0;
		$this->pdf->Ln();
		$this->pdf->z_fullLine('=== Item_'.($this->itemCnt).': '.$text.' ===', 'B');
		
	}
	
	// level 2 text
	function item2Text($text) {
		$this->item2Cnt++;
		$this->pdf->Ln();
		$this->pdf->z_fullLine('   ===== Item_'.($this->itemCnt).'_'.$this->item2Cnt.': '.$text.' =====', 'B');
		
	}
	
	function infoOut($key,$text) {
		$this->pdf->Ln();
		$pretext='INFO:';
		if ($key=='ERROR' ) $pretext='PROBLEM:';
		$this->pdf->z_fullLine( $pretext.' '.$text, 'B' );
		$this->pdf->Ln();
	}
	
	/**
	 * show audit trail
	 * @param  $sqlo
	 */
	function auditTrail(&$sqlo, &$sqlo2, $tablename, $id) {
		
		$accLogLib = new oAccLogGuiC();
		$accLogLib->setObject( $sqlo, $tablename, $id );
		
		$auditOpt=array('dateSlim'=>1);
			
		if ($accLogLib->actionsExist($sqlo) ) {
			$logarray = $accLogLib->getLogTable($sqlo, $sqlo2, $auditOpt);
			$tabopt   = array( 'header'=>$logarray['h'] );
			$widtharr = array(8,60,30, 27, 70, 10); // "#", "User", "Action", "Date", "Notes", 'QM'
			$this->pdf->z_tableOut($logarray['d'], $widtharr, $tabopt);
			
		} else {
			//
		}
		
	}
	
	/**
	 * show ONE protocol
	 * @param $sqlo
	 * @param cprotoLog_STRUCT $oneProtoArr
	 * 	 
	 *  'cp' : CONCRETE_PROTO_ID
	 *  'ap' : optional: ABSTRACT_PROTO_ID
	 * @param array $shOpt options for $protoLib
	 *   "protoType"= 'PREP', 'QC'
	 *   "no_SUC_col" : 0,1 : do not show a SUC/Device column
	 *   "Head_PRC_Name" : 0,1 : do show the PRC Name ?
	 *   "Head_PRA_Name" : -1, 0 ,[1] : do show the PRC Name ?
	 */
	function showProtocolOne( &$sqlo, &$sqlo2, $oneProtoArr, $shOpt ) {
		
		// define width of columns
		$stepColDefs = array(
		  'NAME'  =>array('w'=>60),
		  'cSUBST'=>array('w'=>60),
		  'cQUANT'=>array('w'=>20),
		  'UNIT'  =>array('w'=>15),
		  'cNOTES'=>array('w'=>50),
		  'DefMin'=>array('w'=>20),
		  'DefMax'=>array('w'=>20),
		  'Info'  =>array('w'=>17) 
		);
		
		if ($shOpt["no_SUC_col"]>0) {
			unset($stepColDefs['cSUBST']);
		}
		
		$apid       = $oneProtoArr['ap'];
		$c_proto_id = $oneProtoArr['cp']; // ID of CONCRETE_PROTO
		

		
		if (!$c_proto_id) {
			$objNiceName='?';
			if ($apid) $objNiceName = $this->helpLib->getObjNiceName( $sqlo, 'ABSTRACT_PROTO', $apid); // TBD:
			$this->infoOut('ERROR','Protocol missing. Planned Protocol: '.$objNiceName);
			return;
		}
		
		/*
		 * "cols" => coldef_STRUCT
		 */
		$shOpt['cols'] = array_keys($stepColDefs);
		
		$protoStruct = $this->helpLib->getProtoInfo($sqlo, $c_proto_id, $shOpt) ;
		
		// glob_printr( $protoStruct, "protoStruct info" );
		
		// analyse $protoStruct: 'protinfo', 'header', 'steps'
		// show structure
		// protocol header
		if (is_array($protoStruct['protinfo'])) {
			reset ($protoStruct['protinfo']);
			$notesarr=NULL;
			foreach( $protoStruct['protinfo'] as $key=>$valarr) {
				if ($shOpt["Head_PRA_Name"]<0 and $valarr[2]=='a_name') {
					continue; // do not show
				}
				if ($valarr[2]=='notes') {
					$notesarr[]  = array($valarr[0], $valarr[1]);
				} else {
					$keyValArr[] = array($valarr[0], $valarr[1]);
				}
			}
			$colsizeArr=array(40,$this->pageWidth-40); // full size
			$this->pdf->z_tableOut($keyValArr, $colsizeArr);
			
			if (is_array($notesarr)) {
				reset ($notesarr);
				foreach( $notesarr as $dummy=>$valarr) {
					$this->pdf->z_fullLine($valarr[0].':','B');
					$this->pdf->z_fullLine($valarr[1]);
					
				}
			}
			$this->pdf->z_fullLine('');
		}
		
		// protocol step table
		if (is_array($protoStruct['header'])) {
			
			$this->pdf->z_fullLine('Steps:','B');
			// define column widht
			$widtharr=array();
			foreach($stepColDefs as $col=>$valarr) {
				$widtharr[]=$valarr['w'];
			}
			
			$tabopt    = array( 'header'=>$protoStruct['header'] );
			// glob_printr( $protoStruct['steps'], "proto:keyValArr info" );
			$this->pdf->z_tableOut($protoStruct['steps'], $widtharr, $tabopt);
		}
		
		// show audit trail
		$this->pdf->z_fullLine("");
		$this->pdf->z_fullLine( "Audit-Trail");
		$this->auditTrail($sqlo, $sqlo2, 'CONCRETE_PROTO', $c_proto_id);
			
	}	
	
}