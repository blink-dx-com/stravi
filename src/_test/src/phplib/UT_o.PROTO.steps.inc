<?php

class UT_o_PROTO_steps_inc extends gUnitTestSub {
	

	
	function dotest( &$sqlo, $options ) {
	
		$abs_proto_id = $this->_getDefObjID( "ABSTRACT_PROTO", "default" );
		$testLib = new gProtoOrg();
		
		$conc_proto_id=0;
		$testLib->setProto($sqlo, $abs_proto_id, $conc_proto_id	);
		$steps = $testLib->getStepArray();
		$this->_info( 'PRA-ID: '.$abs_proto_id.'; STEPS', print_r($steps,1) ); // just info for the test
		$EXPECT_STEP_NUM=6;
		
		$result=1;
		if ( sizeof($steps)!=$EXPECT_STEP_NUM ) $result=0;
		$this->_compareTestResult('TEST01 steps read', $EXPECT_STEP_NUM, sizeof($steps));
		$retval = 1;
		
		return ($retval);
	}

}
