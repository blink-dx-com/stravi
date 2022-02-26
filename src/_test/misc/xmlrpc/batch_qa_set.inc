<?php
// unittest for XMLRPC-method batch_qa_set

class batch_qa_set_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$batch_id	 =  $this->_getDefObjID( "W_WAFER", "default" );
		$valarr=NULL;
		$MAXPOS=700;
		$pos=1;
		while ($pos <= $MAXPOS) {
			$valarr[]=array($pos, 0.5);
			$pos++;
		}
		$params = array( $batch_id, $valarr );
		return $params;
	}
}
