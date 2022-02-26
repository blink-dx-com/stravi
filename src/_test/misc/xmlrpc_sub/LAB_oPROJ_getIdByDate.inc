<?php
/**
 *  [0]  	'$date'   		=> the experiment date
 *   						   supported formats: DD.MM.YYYY, DD/MM/YYYY, YYYY-MM-DD
 *   [1]	'$rootProjId'	=> the rootProjId
 *   						   optional, default:85003 (HIV-NAT Project -> RD_Experiments) 
 *   [2]	'$identifier'	=> OPTIONAL: the identifier that appends a day-folder
 *   						   optional, default:'Fetzel' for Fetzel experiments
 *   [3]	'$rootkey'	
 * 
 * @author steffen
 *
 */

class LAB_oPROJ_getIdByDate_TST extends BASE_TSTxrpc {
	
	function getXmlrpc_params() {
		global $error;
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		switch ($subtest){
			
			case "001": // nur DATE => error: no root project
				$datex =  "2013-05-02";
				$paramFunc = array($datex);
				break;
			case "002": // DATE, root-proj => o.k.
				$datex    =  "2013-05-02";
				$rootProj = 85232; // on tiberia2/frosch
				$paramFunc = array($datex, $rootProj);
				break;
			
			case "003": // DATE, root-proj, $identifier, rootkey => o.k.
				$datex      = "2013-05-02";
				$rootProj   = 0; // on tiberia2/frosch
				$identifier = 'steffen_test2';
				$rootkey    = 'NAT/03';
				$paramFunc  = array($datex, $rootProj, $identifier, $rootkey);
				break;
			case "004": // DATE, root-proj, $identifier, rootkey => o.k.
				$datex    =  "2013-05-02";
				$rootProj = 0; // on tiberia2/frosch
				$identifier = 'steffen_testTB';
				$rootkey    = 'NAT/01';
				$paramFunc  = array($datex, $rootProj, $identifier, $rootkey);
				break;
			case "005": // DATE, root-proj, '', rootkey => error : BAD rootkey
				$datex    =  "2013-05-02";
				$rootProj = 0; // on tiberia2/frosch
				$identifier = 'steffen_testTB';
				$rootkey    = 'NAT/XXXXX';
				$paramFunc  = array($datex, $rootProj, $identifier, $rootkey);
				break;
			case "006": // DATE, root-proj, $identifier, rootkey => o.k.
				$datex      =  "2013-05-02";
				$rootProj   = 0; //
				$identifier = 'steffen_testTB2';
				$rootkey    = 'NAT/01';
				$paramFunc  = array($datex, $rootProj, $identifier, $rootkey);
				break;
			default:
				$error->set($FUNCNAME, 1, 'benoetige sub_test als "Parameter"');
				return;
				break;
		}
		
		return $paramFunc;
	}
}
