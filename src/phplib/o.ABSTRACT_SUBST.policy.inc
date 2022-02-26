<?php
require_once ("o.AS_HAS_PR.subs.inc");
/**
 * special release policy for ABSTRACT_SUBST
 * called by e.g. www/lab/pol.objRelease.inc
 * @package o.ABSTRACT_SUBST.policy.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   SREQ:0003363: f.object.release.policy > special for ABSTRACT_SUBST 
 */
class oSUA_policy {
	
    /**
     * 
     * @param object $sqlo
     * @param object $polOption
     * * @param array $polOption
 	 *   'mo.CERT_FLAG' : [1],2,3,4 planned Life Cycle Flag of mother object
     */
    function __construct( &$sqlo, $objid, $polOption=NULL ) {
		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
		    $this->_debug = $_SESSION["userGlob"]["g.debugLevel"];
		}
		$this->objid = $objid;
		$this->polOption=$polOption;
		
		$this->t_short = globTabGetSHORT('ABSTRACT_SUBST');
	}
	
	/**
	 * get reopen info
	 * @param $sqlo
	 
	 
	 * @return array ( 'txt'=>$qc_infoText, 'err'=>$qc_problem )
	 * 	'txt' => string
	 *  'err' <= 0 o.k.
	 *  	  >  0 error
	 */
	function pol_reopen(&$sqlo, $objid) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$objid = $this->objid;
		$pol_problem = 0;
		$outtext     = NULL;
		
		if (!glob_column_exists('ABSTRACT_SUBST', 'VERSFLAG')) {
			return array ( 'txt'=>$outtext, 'err'=>$pol_problem );
		}
		
		$sua_feats = glob_elemDataGet3( $sqlo, 'ABSTRACT_SUBST', array('ABSTRACT_SUBST_ID'=>$objid),
			 array('VERSFLAG') );
			 
		if ($sua_feats['VERSFLAG']!=2) return array ( 'txt'=>$outtext, 'err'=>$pol_problem );
		
		// check usage
		$sqlsel = 'count(1) from CONCRETE_SUBST where ABSTRACT_SUBST_ID='.$objid;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$cnt = $sqlo->RowData[0];
		
		if ($cnt) {
		    $suc_short = globTabGetSHORT('CONCERET_SUBST');
		    $pol_problem=1;
		    $outtext='Ich kann kein "reopen" durchf&uuml;hren bei einer '.$this->t_short.'-version,
			 die schon bei einer '.$suc_short.' benutzt wurde. Sie darf nicht manipuliert werden!';
		    return array ( 'txt'=>$outtext, 'err'=>$pol_problem );
		}

		return array ( 'txt'=>$outtext, 'err'=>$pol_problem );
	}
	
	// return array($qc_text, $problem_id)
	private function _check_protos($sqlo) {
	    $out = array('', 0);
	    $AS_HAS_PR_lib   = new oAS_HAS_PR_subs();
	    $AS_HAS_PR_lib->setObj($this->objid);
	    $ap_array = $AS_HAS_PR_lib->getProtoLog($sqlo, 0);
	    if(!empty($ap_array)) {
	        $accLogSub = new oAccLogC();
	        
	        $err_pra_cnt=0;
	        foreach($ap_array as $loop_row) {
	            $apid = $loop_row['ap'];
	            $accLogSub->setObject( $sqlo, 'ABSTRACT_PROTO', $apid );
	            
	            if (!$accLogSub->currentIsReleased($sqlo)) {
	                $err_pra_cnt++;
	            }
	        }
	        if ($err_pra_cnt) {
	            $out = array($err_pra_cnt.' '. tablename_nice2('ABSTRACT_PROTO') .'s are not released.', 1);
	        }
	    }
	    
	    return $out;
	}
	
	/**
	 * 
	 * @param object $sqlo
	 * @return array $test_names
	 *    'Params'
	 *    'Compo_check' : check sub components (e.g. Audit status of PRAs)
	 *    'CERT_flag'   : check CERT_FLAG
	 * @return array
	 *    'txt' : STRING
	 *    'err' : INT
	 */
	private function _check_matrix(&$sqlo, $CERT_FLAG, $test_names ) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
	        debugOut('Tests: '.implode(', ',$test_names), $FUNCNAME);
	    }
	    
	    $qc_problem  = 0;
	    
	    $errPre      = 'Err: ';
	    $qc_infoArr=array();
	    $sua_feats = glob_ObjDataGet( $sqlo, 'ABSTRACT_SUBST', $this->objid, array('*') );
	    
	    if (in_array('CERT_flag', $test_names)) {
	        
	        $sua_CERT_FLAG=$sua_feats['CERT_FLAG'];

	        if ($CERT_FLAG==1) {
	            if ($sua_CERT_FLAG!=1) {
	                $qc_infoArr[] = $errPre . ' "life cycle status" must be 1 for normal release.';
	                $qc_problem = 4;
	            }
	        }
	        if ($CERT_FLAG==2) {
	            if ($sua_CERT_FLAG!=1 and $sua_CERT_FLAG!=2) {
	                $qc_infoArr[] = $errPre . ' "life cycle status" must be (1 or 2) for D_released.';
	                $qc_problem = 4;
	            }
	        }
	        if ($CERT_FLAG==3) {
	            // no special CERT check
	        }
	    }

	    if (in_array('SUA.params', $test_names)) {
 
	        foreach($sua_feats as $key=>$val) {
	            if (!$val) {
	                $qc_infoArr[] = $errPre . 'value missing for "'. columnname_nice2('ABSTRACT_SUBST', $key) .'"';
	                $qc_problem  = 1;
	            }
	        }
	    }
	    
	    if (in_array('Compo_check', $test_names)) {
	        list($qc_text, $problem_id) = $this->_check_protos($sqlo);
	        if ($problem_id>0) {
	            $qc_infoArr[] = $errPre . 'Sub-Components: '.$qc_text;
	            $qc_problem   = 10;
	        }
	    }
	    
	    $qc_infoText = "";
	    if ($qc_infoArr!=NULL) {
	        $qc_infoText = implode('<br />', $qc_infoArr);
	    }
	    return array( 'txt'=>$qc_infoText, 'err'=>$qc_problem );
	}
	
	function policy_released( &$sqlo) {
	    if (!$this->objid) throw new Exception('Class not initialized.');
	    $test_names= array(
	        'Params',
	    );
	    return $this->_check_matrix($sqlo, 1, $test_names);
	}
	
	function policy_D_released( &$sqlo) {
	    if (!$this->objid) throw new Exception('Class not initialized.');
	    $test_names= array(
	        'Params',
	        'Compo_check',
	        'CERT_flag'
	    );
	    return $this->_check_matrix($sqlo, 2, $test_names);
	}
	
	function policy_R_released( &$sqlo) {
	    if (!$this->objid) throw new Exception('Class not initialized.');
	    $test_names= array(
	        'Params',
	        'Compo_check',
	        'CERT_flag'
	    );
	    return $this->_check_matrix($sqlo, 3, $test_names);
	}
}