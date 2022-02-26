<?php
require_once 'export/o.EXP.raw_save_att.inc';
require_once ('f.workdir.inc');

/**
 * plugin to archive one RAW-result set
 *    
 * @package o.EXP.raw_save_att_one.inc
 * @swreq UREQ:4720 EXP_RAW_SPOT_RESULT > extract results as xlsxml-Attachment,
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $_REQUEST['id'] id of EXP
 *   
 */
class o_EXP_raw_save_att_one_XPL extends gPlugin {
	
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
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array() )
				);
				
		
	}
	

	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$this->objid   = $_REQUEST['id'];
		$sqlo = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		
		$workdirObj = new workDir();
		$workdir   = $workdirObj->getWorkDir ( "o.EXP.raw_save_att_one" );
		if ($error->Got(READONLY))  {
			return;
		}
		
		$mainlib = new oEXP_raw_save_att($workdir);
		$mainlib->initExp($sqlo, $this->objid);
		if ($error->Got(READONLY))  {
			return;
		}
		
		$filename = $mainlib->createFile($sqlo, $sqlo2);
		if ($error->Got(READONLY))  {
			return;
		}
		
		$downtext = $mainlib->getDownloadText();
		
		echo $downtext;
		
		/*
		$mainlib->saveAttachment($sqlo);
		if ($error->Got(READONLY))  {
			return;
		}
		*/
		
	}
	
}
