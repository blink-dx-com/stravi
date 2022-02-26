<?php
require_once(  basename(__FILE__)."/../../../_test_lib/test_utilities.inc");

// unittest for XMLRPC-method update_exp_chan_res
class update_exp_chan_res_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		$optInputParams = $this->params;
		$subtest  = $this->subtest;
		
		$exp_id	 =  $this->_getDefObjID( "EXP", "cytoRes" );
		$MAXNUM	 = 10;
		$MAXNUM2 = 20;
		$xdata 	 = array();
		$col_desc =array("CHANNEL_ID", "V01", "V03", "V04");
		
		switch  ( $subtest ) {

			case "errorData":
				$randGib = new test_utilities();
				$r = "";
				for ( $i=3; $i<$MAXNUM; $i++ ) {
					$r = $randGib->generateRandStr(rand($i, $MAXNUM));
					$xdata[]    = array( "ID"=>$i, "V01"=>$r, "V03"=>$r ); 
				}
				
				for ( $i=$MAXNUM; $i<$MAXNUM2; $i++ ) {
					$r = $randGib->generateRandStr(rand($i, $MAXNUM2));
					$xdata[]    = array( "ID"=>$i, "V01"=>$r, "V03"=>$r ); 
				}
				
				$params = array( $exp_id, $xdata );
				break;
				
			default:
				for ( $i=3; $i<$MAXNUM; $i++ ) {
					$xdata[]    = array( "ID"=>$i, "V01"=>$i*0.001, "V03"=>$i*0.001 ); 
				}
				
				for ( $i=$MAXNUM; $i<$MAXNUM2; $i++ ) {
					$xdata[]    = array( "ID"=>$i, "V01"=>$i*0.001, "V03"=>$i*0.001 ); 
				}
				
				$params = array( $exp_id, $xdata );
				break;
			
		}
			
		return $params;
	}
}
