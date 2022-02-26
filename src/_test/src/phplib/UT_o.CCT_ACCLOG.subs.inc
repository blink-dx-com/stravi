<?php
require_once 'o.CONCRETE_SUBST.subs.inc';
require_once 'o.ABSTRACT_PROTO.stepMod.inc';
require_once 'o.ABSTRACT_PROTO.stepx.inc';
require_once 'o.CONCRETE_SUBST.proto.inc';
require_once 'o.PROTO.upsteps.inc';
require_once 'o.AS_HAS_PR.mod.inc';

class UT_o_CCT_ACCLOG_subs_inc extends gUnitTestSub {
	
    private function _set_obj(&$auditLib, $testname, $expect_info) {

        $now_info = $auditLib->getInfoAll();
        $fail='';
        foreach($expect_info as $key=>$val) {
            if ($now_info[$key]!==$val) {
                $fail = $key.': expect:'.$val. ', NOW:'.$now_info[$key];
            }
        }
        if ($fail=='') {
            $result=1;
        } else {
            $result=0;
        }
        $this->_saveTestResult($testname, $result, 'DIFF: '.$fail );
    }
    
    function _test_Release($sqlo, $sua_id) {

        $suc_tablename= 'CONCRETE_SUBST';
        $SUC_NAME= __CLASS__.':sub1';
        $ins_params = array(
            'vals'=>array(
                'NAME'=>$SUC_NAME,
                'ABSTRACT_SUBST_ID'=>$sua_id,
                'EXPIRY_DATE'=>'9999-12-01',
                'CERT_FLAG'=> oConcSubst_subC::CERT_FLAG_prod
            )
        );
        $suc_id_sub1 = $this->_createObj_checkExists($sqlo, $suc_tablename, 'NAME', $SUC_NAME, $ins_params);
        
        $SUC_NAME= __CLASS__.':sub2';
        $ins_params = array(
            'vals'=>array(
                'NAME'=>$SUC_NAME,
                'ABSTRACT_SUBST_ID'=>$sua_id,
                'EXPIRY_DATE'=>'9999-12-01',
                'CERT_FLAG'=> oConcSubst_subC::CERT_FLAG_prod
            )
        );
        $suc_id_sub2 = $this->_createObj_checkExists($sqlo, $suc_tablename, 'NAME', $SUC_NAME, $ins_params);
        
        $sua_tablename= 'ABSTRACT_SUBST';
        $SUA_NAME= __CLASS__.':with_proto';
        $ins_params = array(
            'vals'=>array(
              
                'NAME'=>$SUA_NAME,
                'NOTES'=>'with protocol'
            )
        );
        
        $sua_id_main = $this->_createObj_checkExists($sqlo, $sua_tablename, 'NAME', $SUA_NAME, $ins_params);
        $pra_tablename= 'ABSTRACT_PROTO';
        $PRA_NAME= __CLASS__.':simple_proto';
        $ins_params = array(
            'vals'=>array(
                'NAME'=>$PRA_NAME,
                'NOTES'=>'with protocol'
            )
        );
       
        $pra_id = $this->_createObj_checkExists($sqlo, $pra_tablename, 'NAME', $PRA_NAME, $ins_params);
        
        $pra_sub_lib = new oABSTRACT_PROTO_stepx($pra_id);
        if ( !$pra_sub_lib->step_cnt($sqlo) )  {

            $pra_lib = new oAbsProtoStepMod();
            $pra_lib->init($sqlo, $pra_id);
            $steparr = array(
                1=>array('NAME'=>'Compo 1', 'ABSTRACT_SUBST_ID'=>$sua_id, 'SUB_STEP_NR'=>1),
                2=>array('NAME'=>'Compo 2', 'ABSTRACT_SUBST_ID'=>$sua_id, 'SUB_STEP_NR'=>1)
            ); 
            $pra_lib->insert_steps($sqlo, $steparr);
            $pra_lib->close($sqlo);
        }
        
        $sua_p_params= array(
            'STEP_ID'=>10, 
            'ABSTRACT_PROTO_ID'=>$pra_id, 
            'XORDER'=>10,
            'TYPE'=>1
        );
        
        $sua_ana_lib = new oAS_HAS_PR_subs($sua_id_main);
        if (empty($sua_ana_lib->getProtoLog($sqlo)) ) {
            $sua_prot_lib = new oAS_HAS_PR_mod($sqlo, $sua_id_main);
            $sua_prot_lib->add_protocol($sqlo, $sua_p_params);
        }
        
        $SUC_NAME= __CLASS__.':proto_main';
        $ins_params = array(
            'vals'=>array(
                'NAME'=>$SUC_NAME,
                'ABSTRACT_SUBST_ID'=>$sua_id_main,
                'EXPIRY_DATE'=>'9999-12-01',
                'CERT_FLAG'=> oConcSubst_subC::CERT_FLAG_prod
            )
        );
        $suc_id_main = $this->_createObj_checkExists($sqlo, $suc_tablename, 'NAME', $SUC_NAME, $ins_params);
        
        $prc_lib  = new oCONCRETE_SUBST_proto($suc_id_main);
        
        if (empty($prc_lib->get_c_protos($sqlo))) {
            $p_feat   = array('vals'=>array());
            $prc_id_main = $prc_lib->create_proto($sqlo, $p_feat);
            
            // add steps
            $prc_step_lib = new oConcProtoUpSteps();
            $prc_step_lib->setProto($prc_id_main);
            
            $stepArr=array(
                1=> array("CONCRETE_SUBST_ID"=>$suc_id_sub1),
                2=> array("CONCRETE_SUBST_ID"=>$suc_id_sub2),
            );
            $prc_step_lib->protoUpdate($sqlo, $pra_id, $stepArr);
        }
        
    }
    
    // return: 0 : not passed, 1: passed
    function dotest( &$sqlo, $options ) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$tablename= 'ABSTRACT_SUBST';
    	$sua_id = $this-> _getExistObjID($tablename, 'default' );
    	if (!$sua_id) {
    		$this->_stop('sua_id not found.');
    	}
    	
    	$suc_tablename= 'CONCRETE_SUBST';
    	$SUC_NAME= __CLASS__.':1';
    	$ins_params = array(
    	    'vals'=>array(
    	        'ABSTRACT_SUBST_ID'=>$sua_id,
    	        'EXPIRY_DATE'=>'9999-12-01',
    	        'CERT_FLAG'=> oConcSubst_subC::CERT_FLAG_research
    	    )
    	);
    	
    	$suc_id = $this->_createObj_checkExists($sqlo, $suc_tablename, 'NAME', $SUC_NAME, $ins_params);
    	
    	
    	$this->_infoNow( 'analysed SUC ID', $suc_id );
    	$cct_access_id = glob_elementDataGet( $sqlo, $suc_tablename, 'CONCRETE_SUBST_ID', $suc_id, 'CCT_ACCESS_ID');
    	/*
    	$module_id     = glob_elementDataGet( $sqlo, 'CCT_ACCESS', 'CCT_ACCESS_ID', $cct_access_id, 'WFLOW_ID');
    	if ( !$module_id ) {
        	$error->set( $FUNCNAME, 1, 'device has no workflow.' );
        	return 0;
        }
        $this->_infoNow('Workflow-ID',$module_id);
        */
    	
    	$auditLib = new oAccLogC();
    	
    	$released_stateid  = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $suc_tablename, 'R_released'); 
    	$produced_stateid  = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $suc_tablename, 'produced'); 
    	
    	// $suc_tablename tests
    	$expect_info=array(
    	    'cct_access_id'=>$cct_access_id,
    	    'id'=>$suc_id,
    	    'tablename'=>$suc_tablename
    	);
    
        $auditLib->setObject( $sqlo, $suc_tablename, $suc_id, NULL);
        $this->_set_obj($auditLib, 'TEST01', $expect_info);
        
        $auditLib->setObject( $sqlo, $suc_tablename, NULL, $cct_access_id);
        $this->_set_obj($auditLib, 'TEST02', $expect_info);
        
        $auditLib->setObject( $sqlo, NULL, NULL, $cct_access_id);
        $this->_set_obj($auditLib, 'TEST03', $expect_info);
        
        $testname = 'TEST04';
        try {
            $auditLib->setObject( $sqlo, NULL, $suc_id, NULL);
            $this->_saveTestResult($testname, 0, 'Error expected.' );
        } catch (Exception $e) {
            $mess = $e->getMessage();
            $this->_saveTestResult($testname, 1, 'Error expected:'. $mess );
        }
        
        if (!$released_stateid or !$produced_stateid) {
        	$error->set( $FUNCNAME, 1, 'AUDIT_STATUS released or produced not found in system!' );
        	return 0;
        }
        
        $testname = 'TEST05';
        
        $auditLib->setObject( $sqlo, $suc_tablename, NULL, $cct_access_id);
        
        $H_ALOG_ACT_ID = $auditLib->getLastLog($sqlo);
        
        if ($H_ALOG_ACT_ID==$released_stateid) {
        	// set to "produced"
        	$logArgu = array('action'=>$produced_stateid);
        	$auditLib->addLogSub( $sqlo, $logArgu );
        	$H_ALOG_ACT_ID = $auditLib->getLastLog($sqlo);
        	if ($H_ALOG_ACT_ID!=$produced_stateid) {
        		$error->set( $FUNCNAME, 2, 'Status "produced" was not set.' );
        		return 0;
        	}
        	if ($error->Got(READONLY))  {
        	    $error->set( $FUNCNAME, 1, 'Error on '.$testname );
        	    return;
        	}
        }
        
        /*
         * set action "action" : action-id
           set "notes"  : can be NULL
           set "qm_val"
     	*			
     	*/
       
         
        $logArgu = array('action'=>$released_stateid, 'notes'=>'TEST', 'qm_val'=>0.5);
        $this->_infoNow('test addLogSub(): params:', print_r($logArgu,1) );
        $auditLib->addLogSub( $sqlo, $logArgu );
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 3, 'Error on '.$testname );
            return;
        }
        
        // check vals
        $sqlsel = "H_ALOG_ACT_ID, NOTES, QM_VAL from CCT_ACCLOG where CCT_ACCESS_ID=".$cct_access_id. 
    			" and H_ALOG_ACT_ID= ".$released_stateid." order by POS DESC";
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadArray();
    	$infoArr = $sqlo->RowData;	
    	$this->_infoNow('Last Audit-entry', print_r($infoArr,1) );
        
    	$info_keys = array('H_ALOG_ACT_ID', 'NOTES', 'QM_VAL');
    	$compare_input = array('H_ALOG_ACT_ID'=>$logArgu['action'], 'NOTES'=>$logArgu['notes'] );  // without QM_value
    	$diff_arr = $this->_compareArrayVals($compare_input, $infoArr);
    	
    	if (is_array($diff_arr)) $result=0;
    	else $result=1;
    	
    	$this->_saveTestResult($testname, $result, 'DIFF: '.print_r($diff_arr,1) );
    	
      
    	$this->_test_Release($sqlo, $sua_id);
    
    	$retval=1;
    	return ($retval);
    }

}
