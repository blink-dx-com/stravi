<?php
require_once ('func_form.inc');
require_once ("lev1/o.CHIP_READER.c.qc_qcheck.inc");

/**
 * analyse REGULATED devices, provides testing for QC_DEVICE maintenance
 *    
 * @package o.CHIP_READER.REG_analyse.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param int $go (0,1)
 *   
 */
class o_CHIP_READER_REG_analyse_XPL extends gPlugin {
	
	
	function register() {
		$title		= 'Admin: analyse REGULATED devices maintenance';
		$tablename	= 'CHIP_READER';
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['objname'] = $tablename;
		
		
		$this->infoarr2 = array( 
			
		);
				
		
	}
	
	function form1() {
	    
	    $initarr   = NULL;
	    $initarr["action"]      = $_SERVER['PHP_SELF'];
	    $initarr["title"]       = "Send emails to users (allowed for Admin)";
	    $initarr["submittitle"] = "Submit";
	    $initarr["tabwidth"]    = "AUTO";
	    
	    $hiddenarr = NULL;
	    $hiddenarr['mod'] = $this->_mod;
	    
	    $formobj = new formc($initarr, $hiddenarr, 0);
	    
	    $formobj->close( TRUE );
	}


	function startMain() {
	    global $error;
		$sqlo = &$this->_sqlo;

		$go 		= $_REQUEST['go'];
		$isAdmin = glob_isAdmin();
		
		if ( !$isAdmin ) {
		    echo "INFO: Admin can send emails to users.<br>";
		}
		

		if (!$go and $isAdmin) {
		    $this->form1();
		    echo "<br>";
		}
		
		$mainlib = new o_CHIP_READER_qc_Mess($sqlo);
		$output = $mainlib->getAdminInfo( $sqlo );
		
		echo "OUTPUT: \n<pre>$output</pre>";
		
		
		if ($go>0 and $isAdmin) {
		    echo "<br>... send emails:<br>";
		    $mainlib->sendEmails( $sqlo ) ;
		}
		
	}
	
}
