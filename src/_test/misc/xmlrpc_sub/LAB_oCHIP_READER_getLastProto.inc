<?php
require_once ("insertx.inc");
require_once("object.info.inc");
require_once("class.filex.inc");

class LAB_oCHIP_READER_getLastProto_TST extends BASE_TSTxrpc {
	
	function getSubTestKeys() {
		return array('error1');
	}
	
	function getXmlrpc_params() {
		
		$sqlo = &$this->sqlo;
		$tablename = 'CHIP_READER';
		$subtest = $this->subtest;
		$params  = $this->params;
		
		$paramFunc = array ('Fetzel 2.02');
		
		$objarr = array(
				'XXX XUNIT 1',
				'XXX XUNIT 2',
				'XXX XUNIT 3',
				);
		
		$insertLib = new insertC();
		
		foreach($objarr as $onename) {
			$objid  = glob_elementDataGet( $sqlo, 'CHIP_READER', 'NAME', $onename, 'CHIP_READER_ID');
			
			if (!$objid) {
				$args = array('vals'=>array('NAME'=>$onename));
				$newobj =  $insertLib->new_meta($sqlo,$tablename,$args);
			}
		}	

		$doubleName = 'XXX XUNIT 3';
		$obj  = new fileC();
		list( $objcnt, $objid, $objname ) = $obj->objGetByName(
				$sqlo,
				$tablename,
				importantNameGet2($tablename),
				$doubleName);
		if ($objcnt<2) {
			// create two devices ...
			$args = array('vals'=>array('NAME'=>$doubleName));
			$newobj =  $insertLib->new_meta($sqlo,$tablename,$args);
		}
			
		
		
		switch  ( $subtest ) {
			case "error1":
				$paramFunc = array ('Fetzel NOT exists');
				break;
			case "error2":
				$paramFunc = array ('XUNIT 3','XXX'); // two devices exist
				break;
			case "found1":
				$paramFunc = array ('XUNIT 1','XXX');
				break;
			case "found2":
				$paramFunc = array ('XUNIT 2','XXX');
				break;
			default:
				break;
		}
		
		
		return $paramFunc;
	}
}
