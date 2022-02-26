<?php
require_once ("glob.obj.update.inc");
require_once 'o.CCT_ACCLOG.subs.inc';

class UT_object_subs_inc extends gUnitTestSub {
	
	
	private function _prepare_doc($sqlo, $doc_id) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$tablename='LINK';
		$UpdateLib = new globObjUpdate();
		$args = array('vals'=>array('MIME_TYPE'=>'test'));
		$UpdateLib->update_meta( $sqlo, $tablename, $doc_id, $args );
		
		$cct_access_id = glob_elementDataGet( $sqlo, $tablename, 'LINK_ID', $doc_id, 'CCT_ACCESS_ID');
		
		// remove all audit logs of object ...
		if ($cct_access_id) {
			$wherestr = 'CCT_ACCESS_ID='.$cct_access_id;
			$sqlo->Deletex('CCT_ACCLOG',$wherestr);
			
			$updatestr='H_ALOG_ACT_ID=NULL where CCT_ACCESS_ID='.$cct_access_id;
			$sqlo->Updatex('CCT_ACCESS', $updatestr);
		}
		
		// set audit status
		$auditLib = new oAccLogC();
		$auditLib->setObject( $sqlo, $tablename, $doc_id);
		$stateid  = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $tablename, 'reopen');
		if ($stateid) {
			$audit_argu = array(
				'action'=> $stateid,
				'notes' => 'audit test'
			);
			$auditLib->addLogObj($sqlo, $tablename, $doc_id, $audit_argu);
		}
		
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'Error on _prepare_doc DOC-ID:'.$doc_id );
			return;
		}
		
	}
	
	private function _temp_compare( &$info_compare, $title, $result) {
		$info_compare[] = $title.': '.$result;
		if (!$result) $this->error_sum++;
	}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;

	$ret=array();
	
	/** 
	 * TBD: define this test later, if table H_EXP_RAW_DESC is used
	 * 
	$table   ='EXP_RAW_RESULT';
	$H_EXP_RAW_DESC_NAME='iconoclust';
	$map_col ='V01';
	
	$desc_id = glob_elementDataGet( $sqlo, 'H_EXP_RAW_DESC', 'NAME', 
			$H_EXP_RAW_DESC_NAME, 'H_EXP_RAW_DESC_ID');
	
	if (!$desc_id) {
		
		$this->_saveTestResult('TEST01:_colFeatExpDesc', 0, 'Missing H_EXP_RAW_DESC, name='.$H_EXP_RAW_DESC_NAME);
	
	} else {
	
		_colFeatExpDesc ($sqlo, $table, $desc_id, $map_col, $ret);
		
		$this->_infoNow( '_colFeatExpDesc()', 'map_col: '.$map_col.'; answer'. print_r($ret,1) ); 
		
		$temp_result=1;
		$temp_notes=NULL;
		if ( $ret['NICE_NAME'] == NULL ) {
			$temp_notes = 'no new NICE_NAME for Column H_EXP_RAW_DESC_COL:V01';
			$temp_result=0;
		}
		$this->_saveTestResult('TEST01:_colFeatExpDesc', $temp_result, $temp_notes);
	}
	
	$ret = colFeaturesGet( $sqlo, $table, $map_col, $desc_id );
	$this->_infoNow( 'colFeaturesGet()', print_r($ret,1) ); 
	
	$temp_result=1;
	$temp_notes =NULL;
	if ( $ret == NULL ) {
		$temp_notes = 'colFeaturesGet failed.';
		$temp_result=0;
	}
	$this->_saveTestResult('TEST02:colFeaturesGet', $temp_result, $temp_notes);
	
	*/
	
	$NAME_val = $FUNCNAME.':glob_elementData';
	$NOTES_val= 'TEST_Notes';
	$tablename='LINK';
	$ins_params= array('vals'=>array('NAME'=>$NAME_val, 'NOTES'=>$NOTES_val) );
	$doc_id = $this->_createObj_checkExists($sqlo, $tablename, 'NAME', $NAME_val, $ins_params);
	
	
	$key='LINK_ID';
	$outputCol='NAME';
	$val_result = glob_elementDataGet( $sqlo, $tablename, $key, $doc_id, $outputCol);
	$this->_compareTestResult('TEST:glob_elementDataGet', $NAME_val, $val_result, $val_result);
	
	
	$outputCol='NAME';
	$val_result = glob_elementDataGet2( $sqlo, $tablename, array($key=>$doc_id), $outputCol);
	$this->_compareTestResult('glob_elementDataGet2', $NAME_val, $val_result, $val_result);
	
	$inp_array=array('NAME'=>$NAME_val, 'NOTES'=>$NOTES_val);
	$val_result = glob_elemDataGet3( $sqlo, $tablename, array($key=>$doc_id), array('NAME', 'NOTES'));
	
	$diff_result = array_diff($inp_array,$val_result);
	$this->_compareTestResult('glob_elemDataGet3:diff', array(), $diff_result, print_r($val_result,1));
	
	// prepare object
	$this->_prepare_doc($sqlo, $doc_id);
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, 'Error on _prepare_doc.' );
		return;
	}
	
	// [owner_id] => 1 [owner] => root [crea_date] => 2018-09-05 10:42:44 
	// [mod_date] => 2018-09-27 13:27:52 [modifier_id] => 1 
	// [wflow_id] => [modifier] => root [h_log_name] => reopen 
	
	$this->error_sum = 0;
	$info = access_data_get($sqlo, $tablename, 'LINK_ID', $doc_id);
	$info_compare = array();
	
	//$result = $info['h_log_name']=='reopen' ? 1 : 0;
	//$this->_temp_compare($info_compare, 'h_log_name', $result);
	
	$result = $info['owner_id']>0 ? 1 : 0;
	$this->_temp_compare($info_compare, 'owner_id', $result);
	$result = $info['modifier_id']>0  ? 1 : 0;
	$this->_temp_compare($info_compare, 'modifier_id', $result);
	$result = $info['modifier']!=NULL ? 1 : 0;
	$this->_temp_compare($info_compare, 'modifier', $result);
	$result = $info['owner']!=NULL  ? 1 : 0;
	$this->_temp_compare($info_compare, 'owner', $result);
	$result = $info['crea_date']!=NULL ? 1 : 0;
	$this->_temp_compare($info_compare, 'crea_date', $result);
	$result = $info['mod_date']!=NULL ? 1 : 0;
	$this->_temp_compare($info_compare, 'mod_date', $result);
	
	$this->_compareTestResult('access_data_get', 0, $this->error_sum, print_r($info_compare,1) );
	
	$this->_infoNow( 'access_data_get()', print_r($info,1) );
	
	
	// 
	// TEST: gObject_exists
	//
	$func_res = gObject_exists($sqlo, 'LINK', $doc_id);
	$this->_compareTestResult('gObject_exists:1', 1, $func_res, 'Search DOC-ID');
	$result = 0;
	$tmp_mess='expected error missing';
	try {
	    $func_res = gObject_exists($sqlo, 'LINK', 'bad ID');
	} catch (Exception $e) {
	    $tmp_mess = $e->getMessage();
	    $result = 1;
	}
	$this->_saveTestResult('gObject_exists:1:err', $result, $tmp_mess);
	
	
	$retval=1;
	return ($retval);
}

}
