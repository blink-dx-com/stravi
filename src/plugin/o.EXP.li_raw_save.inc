<?php
require_once 'export/o.EXP.raw_save_att.inc';
require_once ('f.workdir.inc');

/**
 * plugin to archive RAW-result sets for a LIST of EXP
 *    
 * @package o.EXP.li_raw_save.inc
 * @swreq UREQ:4720 EXP_RAW_SPOT_RESULT > extract results as xlsxml-Attachment,
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $_REQUEST['go'] 
 *   0 : prepare
 *   1 : go
 *   
 */
class o_EXP_li_raw_save_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'EXP';
		$title		= 'archive the EXP_RAW_RESULT set as attachment';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = 'archive EXP_RAW_RESULT';
		//$this->infoarr['help_url'] = 'p129/xxxx'; // link im Wiki: 
		//$this->infoarr['help_base']= 'wiki';  // use wiki
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_cnt']  = 1;
	
		
		$this->infoarr2 = array( 
				'accCheck'    => array('tab'=>array('read', 'write'), 'obj'=>array() ),
				'progressBar' => 1,
				);
				
		
	}
	
	/**
	 * get number of raw results
	 */
	private function getExpInfo(&$sqlo, $exp_id) {
		$sqlsel = 'count(1) from EXP_RAW_RESULT where EXP_ID='.$exp_id;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$count = $sqlo->RowData[0];
		
		return $count;
	}
	
	private function manageOneExp(&$sqlo, &$sqlo2, $exp_id) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->rawExportLib->initExp($sqlo, $exp_id);
		if ($error->Got(READONLY))  {
			return;
		}
		
		$filename = $this->rawExportLib->createFile($sqlo, $sqlo2);
		if ($error->Got(READONLY))  {
			return;
		}
		
		$downtext = $this->rawExportLib->getDownloadText();
		
		
		$this->rawExportLib->saveAttachment($sqlo);
		if ($error->Got(READONLY))  {
			return;
		}
	}
	

	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		
		$sqlo = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		$sqlo3 = &$this->_plug_regSql3();
		$go  = $_REQUEST['go'];
		
		
		$workdirObj = new workDir();
		$workdir   = $workdirObj->getWorkDir ( "o.EXP.li_raw_save" );
		if ($error->Got(READONLY))  {
			return;
		}
		
		$this->rawExportLib = new oEXP_raw_save_att($workdir);
		
		$sqlAfter = $this->listLib->getSqlAfter();
		
		
		$objCnt = $this->listLib->getObjCnt();
		$prgopt = array ('maxnum'=> $objCnt );
		$this->_html->progressInit ( $prgopt );
		
		$sqlsel = "x.EXP_ID, x.NAME from " . $sqlAfter ;
		$sqlo3->Quesel( $sqlsel );
		
		$loopcnt = 0;
		while ( $sqlo3 -> ReadRow () ) {
			$exp_id = $sqlo3-> RowData [0];
			$name  = $sqlo3-> RowData [1];
			echo 'ID: '. $objid .' name : '. $name ;
			
			$rawCount = $this->getExpInfo($sqlo, $exp_id);
			
			echo " raw-cnt: ".$rawCount;
			
			if ( $rawCount>0 ) {
				$this->manageOneExp($sqlo, $sqlo2, $exp_id);
				if ($error->Got(READONLY))  {
					$error->printAll();
					$error->reset();
				}
			}
			echo "<br>\n";
			$loopcnt++;
			
			$this->_html->progressAlive ( $loopcnt );
		}
		
		$this->_html->progressAlive($loopcnt, 1) ;
		
		
		
	}
	
}
