<?php


class UT_glob_obj_touch_inc extends gUnitTestSub {

    
    private function _get_last_info($sqlo, $tablename, $objid) {
        $cca_id = getAccessId  ( $sqlo, $tablename, $objid);
        $data = glob_elemDataGet3( $sqlo, 'CCT_ACCESS', array('CCT_ACCESS_ID'=>$cca_id), array('*') );
        return $data;
    }
    
    function dotest( &$sqlo, $options ) {
    
    	$PRA_ID = $this->_getDefObjID( "ABSTRACT_PROTO", "default" );
    	$table = 'ABSTRACT_PROTO';
    	
    	$this->_infoNow("OBJECT", 'T:'.$table. ' ID:'.$PRA_ID );
    	
    	$test='TEST01';
    
    	$actarr = array('d'=>array('x'=>array('mod'=>NULL)  ), 'a'=>array('key'=>'mod') );
    	$answer = globObjTouch::touch($sqlo, $table, $PRA_ID, $actarr);
    	
    	$this->_infoNow($test, 'INPUT: '.print_r($actarr,1) );
    	
    	$data_out = $this->_get_last_info($sqlo, $table, $PRA_ID);
    	$info_str    = $data_out['UPINFO'];
    	$info_struct = json_decode($info_str,TRUE);
    	$len=0;
    	if (is_array($info_struct)) {
    	   $len = sizeof($info_struct);
    	}
    	$this->_infoNow($test, 'OUTPUT: LEN: '. $len .' DATA: '.print_r($info_struct,1) );
    	
    	
    	$actarr = array('d'=>array('x'=>array('mod'=>NULL, 'ass'=>array('t'=>'ABSTRACT_PROTO_STEP'))  ), 'a'=>array('key'=>'new'), 'x'=>array("warning"=>array('text'=>'warn-text 1')) );
    	$answer = globObjTouch::touch($sqlo, $table, $PRA_ID, $actarr);
    	$this->_infoNow($test, 'INPUT: '.print_r($actarr,1) );
    	$data_out = $this->_get_last_info($sqlo, $table, $PRA_ID);
    	$info_str    = $data_out['UPINFO'];
    	$info_struct = json_decode($info_str,TRUE);
    	$len=0;
    	if (is_array($info_struct)) {
    	    $len = sizeof($info_struct);
    	}
    	$this->_infoNow($test, 'OUTPUT: LEN: '. $len .' DATA: '.print_r($info_struct,1) );
    	
    	$result=1;
    	$this->_saveTestResult($test, $result); // save test result ...
      
    	
    	return 1;
    }

}
