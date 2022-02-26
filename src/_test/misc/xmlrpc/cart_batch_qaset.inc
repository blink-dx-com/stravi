<?php
// unittest for XMLRPC-method cart_batch_qaset

class cart_batch_qaset_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$subtest  = $this->subtest;
		$xinfo  = array('mod'=>'BEA','err'=>'LAROB');
		$params = array( "0913900470", array("QUALITY"=>0.33, "BEAD_CNT"=>28), $xinfo );
		switch  ( $subtest ) {
			case "err1":
				$params[0]='11100007'; // cart not on batch
				break;
			default:
				break;
		}
		return $params;
	}
}
