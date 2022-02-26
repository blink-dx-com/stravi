<?php
/**
 * @package oEXP_get.inc
 */
require_once 'object.info.inc';
require_once 'o.EXP.subs.inc';
require_once 'o.PROJ.subs.inc';

/**
 *
 * get experiment features:
 * - main features
 * - protcol details
 * @author  Steffen Kube
 * @param
 * array $f_param:
 *  <pre>
 * 	array (
 * 	 	['id'] exp_id
 *      ['types'] => array( -- output types
 *        ['exp.feat'] => 0,[1] : [DEFAULT]
 *         'proto.all' => 0,1   : protocol features
 *        )
 *  )
 * 	</pre>
 * @return array
 * 	<pre>
 * 	 'exp.feat' => array (
 *        COLNAME => array('val'=>$val, 'valnice'=>, 'colnice'=> )
 *        ... extra columns ...
 *        'sample_id'=>array()
 *       )
 *   'meta':
 *      'folders' : array('id'=>, 'name'=>) array of folders, where experiment is in
 *      
 *   'protos'=>array( 
 *      array('name'=>, 'steps'=>array(
 *         'name'  => NAME of step, 
 *         'substn'=> name of SUC
 *         'quant'=> quantity,
 *         'note'=>  notes 
 *         'inact'=> inactive?, 
 *         'devn'=>  device name 
 *       ) )
 *     )
 * 	</pre>
 *
 * @swreq NONE
 */
class oEXP_get_XRPC extends XRPC_BASE {
	
	//constructor
	function __construct() {
	}
	
	function __checkParams() {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$sqlo = $this->_sqlo;
		
		$tablename='EXP';
		$objectId = $this->_params['id'];
		
		$this->__objectCheck($sqlo, $tablename, $objectId);
		if ( $error->got(READONLY) ) {
			$error->set( $FUNCNAME, 1, "No object with ID: ".$objectId." found in table ".$tablename.".");
			return;
		}	
		return 1;
	}
	
	function &run() {
	    
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;

		$sqlo   = $this->_sqlo;
	  	$exp_id = $this->_params['id'];
	  	$in_types = $this->_params['types'];
	  	
	  	
	  	$tablename='EXP';
	  	$objInfo = new objInfoC();
	  	$exp_feats = $objInfo->getParamsRaw($sqlo, $tablename, $exp_id);
	  	
	  	$exp_nice  = $objInfo->paramRaw2nice($sqlo, $tablename, $exp_feats);
	  	
	  	$dataout=array();
	  	
	  	$obj_nice_data = $exp_nice['vals'];
	  	$exp_feat_out = array();
	  	foreach( $exp_feats['vals'] as $col => $val ) {
	  	    if (is_array($obj_nice_data[$col])) {
	  	        $exp_feat_out[$col] = array('val'=>$val, 'valnice'=>$obj_nice_data[$col]['valnice'], 
	  	            'colnice'=>$obj_nice_data[$col]['colnice']);
	  	    }
	  	    
	  	}
	  	
	  	// SAMPLE of EXP
	  	$samples = oEXP_subs::getSamples($sqlo, $exp_id);
	  	if (is_array($samples)) {
	  	   
	  	    $sample_id = current($samples);
	  	    if($sample_id) {
	  	        $sample_nice = obj_nice_name ( $sqlo, 'CONCRETE_SUBST', $sample_id );
	  	    }
	  	    $col='sample_id';
	  	    $exp_feat_out[$col] = array('val'=>$sample_id, 'valnice'=>$sample_nice, 'colnice'=>'sample');
	  	}
	  	
	  	$dataout['exp.feat'] = $exp_feat_out;
	  	
	  	// 'meta' > 'folders'
	  	$projs = cProjSubs::getProjectsByObject($sqlo, $tablename, $exp_id );
	  	if (is_array($projs)) {
	  	    
	  	    $projs_out = array();
	  	    foreach($projs as $proj_id) {
	  	        $proj_nice = obj_nice_name ( $sqlo, 'PROJ', $proj_id );
	  	        $projs_out[]=array('id'=>$proj_id, 'name'=>$proj_nice);
	  	    }
	  	    $dataout['meta']['folders'] = $projs_out;
	  	}
	  	
	  	
	  	// PROTOCOLS
	  	if ( array_key_exists('proto.all',$in_types) ) {
	  	    
	  	    
	  	    require_once('o.PROTO.steps.inc');
	  	    require_once 'o.EXP.proto.inc';
	  	    
	  	    $exp_protos = new oEXPprotoC();
	  	    $exp_protos = $exp_protos->getCProtos ($sqlo, $exp_id);
	  	    $protos_info_arr = array();
	  	    
	  	    foreach($exp_protos as $conc_proto_id) {
	  	        
	  	        $step_data=array();
	  	        $abstr_proto_id = glob_elementDataGet( $sqlo, 'CONCRETE_PROTO', 'CONCRETE_PROTO_ID', $conc_proto_id, 'ABSTRACT_PROTO_ID');
	  	        $abstr_name =     glob_elementDataGet( $sqlo, 'ABSTRACT_PROTO', 'ABSTRACT_PROTO_ID', $abstr_proto_id, 'NAME');
	  	    
    	  	    $protolib = new gProtoOrg();
    	  	    $protolib->setProto(	$sqlo, $abstr_proto_id, $conc_proto_id);
    	  	    $steps = $protolib->getStepArray();
    	  	    foreach($steps as $step_nr) {
    	  	        $onestep = $protolib->cStepSumGet( $sqlo, $step_nr, $conc_proto_id );
    	  	        $step_data[]=$onestep;
    	  	    }
    	  	    $protos_info_arr[] =  array('name'=>$abstr_name, 'steps'=>$step_data );
	  	    }
	  	    $dataout['protos'] =  &$protos_info_arr;
	  	}

	  	
		return $dataout;
	}
}