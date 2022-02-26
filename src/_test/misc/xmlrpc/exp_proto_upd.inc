<?php
// unittest for XMLRPC-method exp_proto_upd
// @version $Header: trunk/src/www/_tests/www/test/xmlrpc/exp_proto_upd.inc 59 2018-11-21 09:04:09Z $

class exp_proto_upd_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$optInputParams = $this->params;
		$subtest        = $this->subtest;

		// DEFAULT test values
		if ( $optInputParams==NULL) {
			$exp_id	  = $this->_getDefObjID("EXP", "exp_update_script" );
			$proto_id = $this->_getDefObjID("ABSTRACT_PROTO", "assay");
			$subst_id = $this->_getDefObjID("CONCRETE_SUBST", "default");
			$dev_id   =	$this->_getDefObjID("CHIP_READER", "keyence");
			$devName  =	"CCT-AP reader";
			
			$feat = NULL;
			$steps = array(
						array("st"=>1, "s"=>$subst_id, "d"=>$dev_id, "q"=>0.0, "n"=>"quant with 0.0"),
						array("st"=>2, "dn"=>$devName, "n"=>"Device name, quantity with space characters", "q"=>" 345.56  "),
						array("st"=>3, "i"=>1, "s"=>$subst_id, "q"=>3.56945), 
						array("st"=>24, "s"=>"0",    'n'=>'no subst'), 
						array("st"=>25, 'q'=>NULL,   'n'=>'QUANT=NULL'),
						array("st"=>26, 'q'=>"NULL", 'n'=>'QUANT="NULL"'), // since 2013-11-22
					    array("st"=>27, 'q'=>0,      'n'=>'QUANT=0'),
					 );
		}
		
		if ( $optInputParams=='HIV_NAT') {
			$exp_id	  = 340601;
			$proto_id = 12764;
			$subst_id = $this->_getDefObjID("CONCRETE_SUBST", "default");
			$dev_id   =	$this->_getDefObjID("CHIP_READER", "keyence");
			$devName  =	"HIV-NAT Fetzel 2.02";
			
			$feat = NULL;
			$steps = array(
				array("n"=>"NA","st"=>"14"),
				array("q"=>72.0,"st"=>"45"),
				array("q"=>30.0,"st"=>"19"),
				array("n"=>"NA","st"=>"49"),
				array("n"=>"NA","st"=>"48"),
				array("n"=>"NA","st"=>"52"),
				array("q"=>56.0,"st"=>"66"),
				array("n"=>"NA","st"=>"60"),
				array("q"=>5.0,"st"=>"32"),
				array("q"=>95.0,"st"=>"15"),
				array("n"=>"NA","st"=>"76"),
				array("n"=>"NA","st"=>"59"),
				array("q"=>27.0,"st"=>"75"),
				array("n"=>"NA","st"=>"53"),
				array("n"=>"NA","st"=>"38"),
				array("n"=>"NA","st"=>"54"),
				array("n"=>"no","st"=>"71"),
				array("q"=>600.0,"st"=>"65"),
				array("q"=>3.0,"n"=>"Fetzel2.02","st"=>"29"),
				array("n"=>"HIV-NATExperimentProtocolRT-PCR002","st"=>"70"),
				array("n"=>"\\2010\\2010-03-18\\generated\\blood_sample_1-CartridgeID_sdfhsfd\\log\\","st"=>"72"),
				array("q"=>0.0,"st"=>"11"),
				array("n"=>"NA","st"=>"37"),
				array("q"=>50.0,"st"=>"62"),
				array("n"=>"NA","st"=>"51"),
				array("n"=>"NA","st"=>"55"),
				array("q"=>65.0,"st"=>"42"),
				array("n"=>"NA","st"=>"41"),
				array("n"=>"NA","st"=>"40"),
				array("n"=>"NA","st"=>"50"),
				array("q"=>2.2999999999999998,"st"=>"13"),
				array("q"=>95.0,"st"=>"63"),
				array("q"=>250.0,"st"=>"67"),
				array("q"=>0.0,"st"=>"30"),
				array("q"=>31.0,"st"=>"17"),
				array("n"=>"yes","st"=>"74"), 	  
					 );
		}
		
		switch  ( $subtest ) {
			//non-numeric Substance-ID
			case "err1": 
				$testinfo['nice'] = 'non-numeric Substance-ID';
				$testinfo['expectErr'] = 5;
				$steps[0]['s'] = 'wrongSubstID1';
				break;
			
			//non-existing StepNr (all previous valid update or insert operations will be done...no rollback)
			case "err2":
				$testinfo['nice'] = 'non-existing StepNr';
				$steps[3]['st'] = 4;
				break;
				
			case "err3":
				$testinfo['nice'] = 'dn and n not allowed';
				$steps[1]['d']  = 321;
				$steps[1]['dn'] = 'testdevice';
				break;
				
			case "err4":
				$testinfo['nice'] = 'wrong device';
				$steps[0]['d']  = 99999999999999;
				break;
			case "err5":
				$testinfo['nice'] = 'wrong number';
				$steps[0]['q']  = '1.#ok';
				$steps[1]['q']  = '1,456'; // no english number format !
				break;	
			case "err6": 
				$testinfo['nice'] = 'non-numeric Substance-ID: 00000';
				$testinfo['expectErr'] = 5;
				$steps[0]['s'] = '00000';
				break;
			default:
		}
		
		$params = array( $exp_id, $proto_id, $feat, $steps );
		
		$this->testinfo = $testinfo;
			
		return $params;
	}
}
