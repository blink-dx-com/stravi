<?php 
/**
 * define objects for UNITTESTS
 * used by _test/misc/unittest_onetest.inc
 * @author Steffen Kube
 * @package test_config.inc
 *
 */
class unittest_config {
	
	/**
	 * predefined features of UnitTest objects
	 * @var $useObjectFeatures
	 */
	private $useObjectFeatures;
	
	function __construct() {

	    /**
	     * 
	     * @var unittest_config $useObjexts
	     *   TABLE => array(
	     *      'synonym' => array ( 
	     *            'args'=> array('NOTES'=> 'hallo' )
	     *            '_post_actions' => 0,1 : post actions exist 
	     *       )
	     *   )
	     */
	    $this->useObjexts = array(
	        
	        "A_CHIP_READER" => array(
	            "default" => array(  ),
	        ),
	        
	        
	        "ABS_WORKLIST" => array(
	            "default" => array(  ),
	        ),

	        
	        "ABSTRACT_PROTO" => array(
	            "default" => array( 'EXPECT_STEP_NUM'=>6 ), 
	            "QC"=>array(),
	            "assay-file"=> array( ), 
	            'Assaying'=> array( ), 
	            'Experiment protocol'=>array( ),
	            'Exp_with_samples' => array()
	        ),
	        "ABS_SOCKET" => array(
	            "default" => array( ),
	        ),
	        "ABSTRACT_SUBST" => array(
	            "default" => array(  ), 
	            "with_Class" => array( ), 
	            //TBD: "class_ASR" => array(),
	            "qcsteps" => array(  ), 
	        ),
	        
	        "CHIP_READER" => array(
	            "default" => array(  ),
	        ),
	        
	        "CONCRETE_PROTO" => array(
	            "default" => array(  ),
	            "assocUpdate" => array(  ),
	            
	        ),
	        
	        "CONCRETE_SUBST" => array(
	            "default" => array(),
	            "UT1"=>array(),
	            "UT2"=>array(),
	            "proto"=>array()
	        ),
	        
	        "CONTACT" => array(
	            "default" => array(),
	        ),
	        
	        
	        "DB_USER" => array(
	            "default"  => array(),
	            "real_user"=> array('NICK'=>'skube'),
	            "test"     => array('PASS_WORD'=>'1234AbCd'),
	        ),
	        
	        "EXP" => array(
	            "default" => array(),
	            'toolbox' => array()
	        ),
	        "EXP_TMPL" => array(
	            "default" => array(),
	            'toolbox' => array(),
	            'toolbox_customer' => array()
	        ),
	        
	        "IMG" => array(
	            "default" => array( '_post_actions'=>1 ),
	        ),
	        
	        "LINK" => array(
	            "default" => array(),
	        ),
	        
	        
	        "PROJ" => array(
	            "default" => array(  ),
	        ),
	        
	        "ROLE"=> array(
	            "login_norm" => array( ),
	        ),
			
	        "USER_GROUP" => array(
	            "default" => array(  ),
	        ),
	        
	        
	        
	
		);
		
		$this->useObjectFeatures = array(
			'DB_USER'   =>array( "default" => array('NAME'=>'UnitTest_01')   ),
			'USER_GROUP'=>array( "default" => array('NAME'=>'UnitTest_grp01') )
		);
	}
	
	
	
	function get_objects() {
		return $this->useObjexts ;
	} 
	
	function get_object_features() {
		return $this->useObjectFeatures;
	}
}
