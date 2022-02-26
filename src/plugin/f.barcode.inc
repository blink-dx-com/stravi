<?php
require_once ('barcode.code128.inc');

/**
 * barcode
 *    
 * @package f.barcode.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 * @param  $c     urlencoded barcode string
 * @param  $sub = A,B,[C]
 * @param  $y  height of image in pixels

 *   
 */
class f_barcode_XPL extends gPlugin {
	
	function register() {
		$title		= 'Barcode';
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'tool'; 
		
		$this->infoarr2 = array(
		  'noHtmlHead'=>1

		);
	}
	
	
	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		

		$barcode = urldecode($_REQUEST['c']);
		$subtype   = $_REQUEST['sub'];
		$IMG_Y_LEN = $_REQUEST['y'];
		
		if (!strlen($barcode)) {
		    htmlFoot('ERROR', 'No data');
		}
		
		$baroption  = array();
		$charExpect = 0;
		if ( $IMG_Y_LEN<5 or $IMG_Y_LEN>500 ) $IMG_Y_LEN  = 50;
		if ($subtype=='B') {
		    $baroption['subtype'] = 'B';
		    $charExpect = strlen($barcode)*2;
		}
		
		$params = array("charExpect"=>$charExpect, "IMG_Y_LEN"=> $IMG_Y_LEN );
		$barObj = new barcodeCode128($params);
		
		$barObj->buildImage($barcode, $baroption);
		
		if ( $error->printAll()) {
		    return;
		}
		
		if ( $_SESSION["userGlob"]["g.debugLevel"]<=0 ) {
		    $barObj->htmlOut();
		}
		
		
	}
	
}