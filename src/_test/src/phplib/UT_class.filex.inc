<?php
// test phplib/class.filex.inc


class UT_class_filex_inc extends gUnitTestSub {
	

function _searchOne(&$sqlo, $name) {
	
	$tmpcnt   = 0;
	$sqlAfter = "from EXP where NAME=". $sqlo->addQuotes($name);
	$sqls     = "select EXP_ID ".$sqlAfter;
    $sqlo->query($sqls);
    if ( $sqlo->ReadRow() ) {
    	// at least once found ...
        $tmpcnt  = 1;
        $objid   = $sqlo->RowData[0];
        $objname = $sqlo->RowData[1];
    
	    if ( $sqlo->ReadRow() )  {  // more than 1 found ?
	        $sqls   = "select count(*) ".$sqlAfter;
	        $sqlo->query($sqls);
	        $sqlo->ReadRow();
	        $tmpcnt = $sqlo->RowData[0];
	    }
    }
    return array($tmpcnt, $objid, $objname);
}

function _oneObjTest(&$testLib, &$sqlo, $inputObj) {
	$answer = $testLib->objGetByName($sqlo, 'EXP', 'NAME', $inputObj['name'] );
	$this->_infoNow( 'test:', $inputObj['name'].'; result:'. print_r($answer, 1) );
	return $answer;
}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
	$tablename="EXP";
	$objname=array();
	$objname[0]['name']='objGetByName.UT1'; // one
	$objname[1]['name']='objGetByName.UT2'; // many
	$objname[2]['name']='objGetByName.UT3'; // missing
	
	
	
	$answer[0] = $this->_searchOne($sqlo, $objname[0]['name']);
	$answer[1] = $this->_searchOne($sqlo, $objname[1]['name']);
	$answer[2] = $this->_searchOne($sqlo, $objname[2]['name']);
	
	if (!$answer[0][0]) {
		// create experiment
		$argu=array('NAME'=> $objname[0]['name']);
		$objid = insert_row($sqlo,'EXP', $argu);
		$objname[0]['id'] = $objid;
	} else {
		$objname[0]['id'] = $answer[0][1];
	}
	
	if ($answer[1][0]<2) {
	    
		// create experiments
	    $args = array( 
	        'vals'=>array('NAME'=> $objname[1]['name'] )
	    );
		
	    $objid  = $this->_createObj($tablename, $args);
	    $objid2 = $this->_createObj($tablename, $args);  // create second experiment
	    
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 1, 'Error on creation of UnitTest experiments.' );
	        return;
	    }
		
		$objname[1]['id'] = $objid;
	} else {
		$objname[1]['id'] = $answer[0][1];
	}
	
	if ($answer[2][0]>0) {
		$this->_info( 'Exp3: ', 'please change the name of EXP-ID: '.$answer[2][1].'');
		return (-1);
	}
	
	$this->_infoNow( 'Exp1-ID:', $objname[0]['id'].':'.$objname[0]['name']);
	
	$testLib = new fileC();
	
	$answer = $this->_oneObjTest($testLib, $sqlo, $objname[0]);
	$answer = $this->_oneObjTest($testLib, $sqlo, $objname[1]);
	$answer = $this->_oneObjTest($testLib, $sqlo, $objname[2]);
	$answer = $this->_oneObjTest($testLib, $sqlo, $objname[2]);
	$answer = $this->_oneObjTest($testLib, $sqlo, $objname[2]);
	
	$retval = 1;
	
	return ($retval);
}

}
