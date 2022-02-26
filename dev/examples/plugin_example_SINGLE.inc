<?php
/**
 * example of test module
 * @package 
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   string INPUT 
 */
class example_XPL extends gPlugin {
	
	
	function register() {
		$tablename	= 'CHIP_READER';
		$title		= 'activate device-protocol + archive';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
        
        $this->infoarr['version']     ="2018-04-94";
        $this->infoarr['version.info']='';
        $this->infoarr['ureq.info']   = "REQ-0003 Design of production xyz";
        $this->infoarr['inforow'] = '';
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('write'), 'obj'=>array('read') )
			);
	}
	

	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
	}
}
