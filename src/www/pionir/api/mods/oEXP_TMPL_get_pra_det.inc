<?php
/**
 * get protocol steps
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @package oEXP_TMPL_get_pra_det.inc
 * 
 * @var array toolbox_step_STRUCT
 * array(
 *    array ( KEY=VAL ) KEY: DB_toolbox_keys::key_translator
 * )
 */
require_once ('object.info.inc');
require_once ('o.EXP_TMPL.proto.inc');
require_once 'o.ABSTRACT_PROTO.stepx.inc';
require_once 'o.CONCRETE_PROTO_api.inc';



/**
 * get protocol steps
 */
class oEXP_TMPL_get_pra_det_XRPC extends XRPC_BASE {


	function __checkParams() {
		global $error;
		$FUNCNAME= '__checkParams';
		
		$sqlo = $this->_sqlo;
	
		$obj_id  	  = $this->_params['id'];
		$this->tablename = 'EXP_TMPL';

		$this->__objectCheck($sqlo, $this->tablename, $obj_id);
		if ( $error->got(READONLY) ) {
		    return -10;
		}
		
		if ($this->_params['kind']==NULL) {
		    $error->set( $FUNCNAME, 1, 'Input: kind missing' );
		    return -11;
		}
	
		return 1;
	}
	

	/**
	 * LOAD an abstract proto + attachment file
	 * - test: access, allowed columns
	 
	 * @param   array $f_param <pre>
	 *   'id':
	 *   'kind' : string
	 * 	</pre>
	 * @return <pre> array(
	 *     'feats' => array('ID'=>, 'NAME=>'')
	 *     'steps' => toolbox_step_STRUCT
	 *   )
	 * </pre>
	 * @throws exception
	 */
	function &run() {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
		$sqlo = $this->_sqlo;
		$obj_id    = $this->_params['id'];
		$kind_name = $this->_params['kind'];
		
		$exp_lib = new oEXP_TMPL_protoC($obj_id);
		$protos = $exp_lib->abs_protos_details($sqlo);
		
		if (empty($protos)) {
		    $error->set( $FUNCNAME, 1, 'EXP_TMPL:'.$obj_id.' has no protocols.' );
		    return;
		}
		foreach($protos as $row) {
		    if ($row['kind_name']==$kind_name) {
		        $found=1;
		        break;
		    }
		}
		if (!$found) {
		    $error->set( $FUNCNAME, 2, 'No protocol of kind "'.$kind_name.'" found on EXP_TMPL.' );
		    return;
		}
		
		$pra_id = $row['pra'];
		
		$objInfoLib = new objInfoC();
		$pra_args   = $objInfoLib->getParamsRaw ( $sqlo, 'ABSTRACT_PROTO', $pra_id);
		
		$output=array();
		$output['fea']= array(
		    'ID'=>$pra_id,
		    'NAME'=>$pra_args['vals']['NAME']  
		);
		$output['steps']= array();
		
		$pra_step_lib = new oABSTRACT_PROTO_stepx($pra_id);
		$steps_raw    = $pra_step_lib->getStepArray($sqlo);
		if (empty($steps_raw)) {
		    return $output;
		}
		
		$use_keys = array(
		    'STEP_NR',
		    'NAME',
		    'QUANTITY',
		    'ABSTRACT_SUBST_NAME',
		    'ABS_DEV_NAME',
		    'H_UNIT_NAME',
		    'NOTES',
		    
		);
		
		$steps = array();
		foreach($steps_raw as $st_row) {
		    
		    $step_nr  = $st_row[0];
		    $step_det = $pra_step_lib->step_info($sqlo, $step_nr, 1);
		    $row_use=array();
		    
		    $out_keys = oCONCRETE_PROTO_toolbox::get_abs_DB2key();
		    
		    foreach($use_keys as $key) {
		        if ($step_det[$key]!=='' and $step_det[$key]!==NULL)  {
		            
		            $key_out = $out_keys[$key];
		            $row_use[$key_out] = $step_det[$key];
		        }
		    }
		    $steps[]=$row_use;
		    
		}
		$output['steps'] = $steps;
		

		return $output;

	}
}