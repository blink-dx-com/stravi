<?php
// unittest for XMLRPC-method exp_create_macro
// @version $Header: trunk/src/www/_tests/www/test/xmlrpc/exp_create_macro.inc 59 2018-11-21 09:04:09Z $

require_once ("insertx.inc");

class exp_create_macro_TST extends BASE_TSTxrpc {
	
	private function checkDevices(&$sqlo, $input) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		/**
		 * $oneDevice = array(
		 * 	['vals']['NAME']
		 * 
		 *     OPTIONAL:
		 *  "xobj" => array ( 
				'extra_class_id' => , 
				"values"  => array(attrib_id=>val) 
				"optname" => 1 
		 *  )
		 */
		
		$creaLib = new insertC();
		
		foreach($input as $oneDevice) {
			
			if ($oneDevice['vals']['NAME']==NULL) {
				$error->set( $FUNCNAME, 1, 'input error on init. NAME missing' );
				return;
			}
			$devid = glob_elementDataGet( $sqlo,  'CHIP_READER', 'NAME', $oneDevice['vals']['NAME'], 'CHIP_READER_ID');
			
			if ($devid) {
				continue; // exists
			}
			
		
			$metaargu = array(
				"vals"  => $oneDevice['vals'],
				"xobj"  => $oneDevice['xobj'],
			);
			$newid = $creaLib->new_meta($sqlo, 'CHIP_READER', $metaargu);
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 2, 'error on test init. DEV:'.$oneDevice['NAME'] );
				return;
			}
		}
	}
	
	function getXmlrpc_params() {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$optInputParams = $this->params;
		$subtest  	    = $this->subtest;
		
		$sqlo = &$this->sqlo;
		
		$varcol = & Varcols::get();
		$qcDeviceClsID	= $varcol->class_name_to_id(  'CHIP_READER', 'qc_device' );
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 2, 'error on test init. Class missing' );
			return;
		}
		
		/**
		 * DEVICES:
		 * PIMA-X-000018 : Produktions-PIMA; CCT_PID=129
		 * PIMA-X-000019 : F+E-PIMA; CCT_PID=99
		 * 
		 */
		$tmpNotes = 'UnitTest Device for '.__CLASS__;
		$deviceArr = array(
			array('vals'=>array('NAME'=>'PIMA-X-000018', 'NOTES'=>$tmpNotes), 'xobj'=>array(
				'extra_class_id'=>$qcDeviceClsID, 'optname' => 1, 'values'=>array('CCTprojID'=>129)) 
				),
			array('vals'=>array('NAME'=>'PIMA-X-000019', 'NOTES'=>$tmpNotes), 'xobj'=>array(
				'extra_class_id'=>$qcDeviceClsID, 'optname' => 1, 'values'=>array('CCTprojID'=>99)) 
				),
		);
		
		$this->checkDevices($sqlo, $deviceArr);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 2, 'error on test init.' );
			return;
		}

		$exp_date= date('Y-m-d H:i:s'); // dynamic date ! e.g.: "2007-11-15 09:17:35"
		$mainParams = array(
			'macro'		=> 'iroq3cyto',
			"exp_name" 	=> "hallo12", 
			"exp_notes" => "notes to test",    
			"cart_name" => "0929400097",  
			"device"	=> "PIMA-X-000019",  // R&D PIMA
			"exp_date"	=> $exp_date,  
			"exp_tmpl_name" =>  "Cyto_Iroq3"
			);
			
		if ($optInputParams=='NEW_EXP_NAME') {
			// create a new EXP-name
			$mainParams["exp_name"]='xmlrpc_test '.rand(1,100000000);
		}
			
		switch  ( $subtest ) {
			case "bad-cartname":
				$mainParams["cart_name"]='bad-cart-name-123';
				break;
			case "129.PIMA.labor":
				// doeas NOT create an ICONO protocol !!!
				$mainParams["device"]='PIMA-X-000018';
				break;
			case "99.PIMA":
				$mainParams["device"]='PIMA-X-000019';
				break;
				
			case "HIVNAT":
				switch ($optInputParams) {
					case 't001': // project error - not exists
						$mainParams["macro"]         = 'hivnat1';
						$mainParams["exp_name"]      = 'HIVNAT_011_st1';
						$mainParams["exp_tmpl_name"] = 'HIV_quant_10_zeitreihe';
						$mainParams["cart_name"]     ='0402700014';
						$mainParams["device"] ='';
						$mainParams["proj_id"]= 'array()'; // defined ERROR !!!!
						break;
					case 't002': // o.k. on robur-test
						$mainParams["macro"]         = 'hivnat1';
						$mainParams["exp_name"]      = 'HIVNAT_011_st2';
						$mainParams["exp_tmpl_name"] = 'HIV_quant_10_zeitreihe';
						$mainParams["cart_name"]     ='0402700014';
						$mainParams["device"]='';
						$mainParams["proj_id"]=85231 ;
						break;
					default:
						$mainParams["macro"]='hivnat1';
						$mainParams["exp_name"] = 'HIVNAT_011';
						$mainParams["exp_tmpl_name"] = 'HIV_quant_10_zeitreihe';
						$mainParams["cart_name"]='0402700014';
						$mainParams["device"]='';
						$mainParams["proj_id"]='88706';
						break;
				}
				break;
			case "129.PIMA.assemb":
				$mainParams["device"]='PIMA-D-001145';
				$mainParams["exp_name"] = "hallo.129.PIMA.assemb";
				break;
				
			default:
				break;
		}
		
		$params = array ( $mainParams );
		return $params;
	}
}
