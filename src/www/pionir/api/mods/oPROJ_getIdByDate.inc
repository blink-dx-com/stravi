<?php
/**
 * @namespace core::xmlrpc::meta
 */

/**
 
 * Get the Projekt-ID for a Experiment by Date under a given Root Project.
 * Will create the Project first if nessecary.
 * Convention for the Project is:
 * +--Root Project			(e.g.: RD_Experiments)
 *    |
 *    +--Year Project						//YYYY(e.g.: 2010)
 *       |
 *       +--Month Project   				//YYYY-MM(e.g.: 2010-06)
 *          |
 *          +--Day Project (+$identifier)	//YYYY-MM-DD_$identifier	(e.g.: 2010-06-25_Fetzel)
 * 
 * @author  Michael Brendel (michael.brendel@clondiag.com)
 * @param
 * <pre>
 * 	array ($f_param
 *   [0]	'$date'   		=> the experiment date
 *   						   supported formats: DD.MM.YYYY, DD/MM/YYYY, YYYY-MM-DD
 *   [1]	'$rootProjId'	=> the rootProjId
 *   						   optional, default:85003 (HIV-NAT Project -> RD_Experiments) 
 *   [2]	'$identifier'	=> the identifier that appends a day-folder
 *   						   optional, default:'Fetzel' for Fetzel experiments
 *	)
 * </pre> 
 * @return
 * <pre>
 * 	array (
 * 		[0]		ERRORCODE		=> 0 if no error occured
 *  	[1]		PROJ_ID			=> the Id of Project (Day-Folder for Fetzel Experiments)
 *  )
 * </pre>
 * @throws exception
 * @see    "usedby:external:ICONOCLUST"
 * @swreq UREQ: 0000908: p.98.experiment > Einheitliche Ablage in Tagesordnern
 */
require_once("object.info.inc");
require_once("globals.inc");
require_once("class.filex.inc");
require_once('f.wiid_roid.inc');
require_once("o.PROJ.subs.inc");
require_once("o.PROJ.modi.inc");

class oPROJ_getIdByDate_XRPC extends XRPC_BASE {
	//constructor
	function __construct() {
	}
	
	/**
	 * (non-PHPdoc)
	 * @see www/pionir/xmlrpc/xml_functions/XRPC_BASE#__checkParams()
	 */
	function __checkParams() {
		global $error;
		$FUNCNAME= '__checkParams';
		
		$sqlo = $this->_sqlo;
		
		$objectId = $this->_params[0];

		//1. Existenz-Checks
		
		//2. Access-Checks
		
		//3. Data-Checks
		
		$t_rights = tableAccessCheck( $sqlo, $tableName );
		if ( $t_rights["read"] != 1 ) {
			$answer = getRawTableAccMsg( $tableName, 'read' );
			$error->set( $FUNCNAME, 1, $answer );
			return -1;
		}
		
		if (!$objectId) {
			$error->set( $FUNCNAME, 2, 'id is not set.' );
			return -2;
		}
		return 1;
	}
	
	function &run() {
		global $error;
		$FUNCNAME= '&run';
		
		$objInfo = new objInfoC();
		$obj = new fileC();
		$sqlo = $this->_sqlo;
		$wiidLib = new fWiidRoidC();
		$wiid_num = 1;
		
		$date = $this->_params[0];
		$defRootProjID = '85003';
		$rootProj = $this->_params[1];
		if ($rootProj == null) {
			$rootProj = $defRootProjID;
		}
		$rootProj = $wiidLib->getObjID ($sqlo, 'PROJ', $wiid_num, $rootProj);
		if($rootProj == null) {
			$error->set( $FUNCNAME, 1, 'Root Project '.$this->_params[1].' not found.' );
			return -1;
		}
		
		//Access-check
		$o_rights = access_check($sqlo, 'PROJ', $rootProj );
		if ( !$o_rights["insert"]) {
			$error->set( $FUNCNAME, 1, "You do not have 'insert' permission on the Project ID:".$rootProj."!");
			return -2;
		}
		
		//determine a valid date using a simple regexp check
		if (preg_match('/^([0-2]*[0-9]|3[0-1])\.(0*[0-9]|1[0-2])\.([1-2][0-9]{3})$/', $date)) { //DD.MM.YYYY
			$gerDate = explode('.', $date);
			$date = $gerDate[2].'-'.$gerDate[1].'-'.$gerDate[0];
		}
		if (preg_match('/^([0-2]*[0-9]|3[0-1])\/(0*[0-9]|1[0-2])\/([1-2][0-9]{3})$/', $date)) { //DD/MM/YYYY
			$engDate = explode('/', $date);
			$date = $engDate[2].'-'.$engDate[1].'-'.$engDate[0];
		}
		if (!preg_match('/^([1-2][0-9]{3})-(0*[0-9]|1[0-2])-([0-2]*[0-9]|3[0-1])$/', $date)) { //YYYY-MM-DD
			$error->set( $FUNCNAME, 1, 'Unsupported date format: '.$date.'. Accepted date formats are: YYYY-MM-DD, DD.MM.YYYY or DD/MM/YYYY' );
			return -3;
		}
		
		$identifier = $this->_params[2];
		if ($identifier==null) $identifier = '';
		else $identifier = '_'.$identifier;
		
		/*
		 * determine project-name from the date (day-folder)
		 * check if exists or create new if applicable
		 * create: recursive from base project RD_Experiments(ID:85003)
		 * Project tree:
		 * +--YYYY
		 * +----YYYY-MM
		 * +------YYYY-MM-DD  
		*/
		$this->projMakeLib = new oPROJ_modi();
		$projSubLib = new cProjSubs();
		$tempDate = explode('-', $date);
	    $year  = $tempDate[0];
	    $month = $tempDate[1];
	    $day   = $tempDate[2];

	    $this->projMakeLib->setProj($sqlo, $rootProj);
	    $year_proj = $projSubLib->getProjByName($sqlo, $rootProj, $year);
	    if (!$year_proj) {
	    	$year_proj = $this->projMakeLib->makeProj($sqlo, $year);
	    }
	    $this->projMakeLib->setProj($sqlo, $year_proj);
	    $month_proj  = $projSubLib->getProjByName($sqlo, $year_proj  , $year. "-" .$month);
		if (!$month_proj) {
	    	$month_proj = $this->projMakeLib->makeProj($sqlo, $year. "-" .$month);
	    }
		$this->projMakeLib->setProj($sqlo, $month_proj);
	    $day_proj  = $projSubLib->getProjByName($sqlo, $month_proj  , $year. "-" .$month. "-" .$day .$identifier);
		if (!$day_proj) {
	    	$day_proj = $this->projMakeLib->makeProj($sqlo, $year. "-" .$month. "-" .$day .$identifier);
	    }
		
		return $day_proj;
	}
}