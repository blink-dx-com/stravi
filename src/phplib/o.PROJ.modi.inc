<?php
/**
 * methods to modify a PROJECT in the meaning of SUB-PROJECTS, NOT elements of project
 * - add, delete, ...
 * $Header: trunk/src/phplib/o.PROJ.modi.inc 59 2018-11-21 09:04:09Z $
 * @package oPROJ_modi
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
require_once ("insertx.inc");
require_once 'glob.obj.touch.inc';

class oPROJ_modi {
	var $selectMotherSQL; // SQL sub-selection 
	var $CLASSNAME='oPROJ_modi';
	
	function __construct() {
		$this->isInit = 0;
	}
	
	/**
	 * set project-ID
	 * - check access
	 * @param $projid (can be NULL!)
	 */
	function setProj(&$sqlo, $projId ) {
		global $error;
		$FUNCNAME= $this->CLASSNAME.':setProj';
		$this->projId = $projId;
		
		if ( $this->projId==NULL ) $this->selectMotherSQL = 'is NULL';
		else $this->selectMotherSQL = '='.$this->projId;
		
		$right='insert';
		
		if ($this->projId!=NULL) {
			$o_rights = access_check($sqlo, 'PROJ', $projId);
			if ( !$o_rights[$right]) {
				$projNice = tablename_nice2('PROJ');
				$error->set( $FUNCNAME, 1, 'You do not have '.$right.
					' permission on '.$projNice.' ID:'. $projId.' !' );
				return;
			}
		} else {
			if ( !glob_isAdmin() ) {
				$projNice = tablename_nice2('PROJ');
				$error->set( $FUNCNAME, 1, 'You do not have '.$right.
					' permission on '.$projNice.' ID: NULL !' );
				return;
			}
		}
		
		$this->isInit = 1;
	}
	
	/**
	 * search, if project with NAME=$name exists in MOTHER-project $this->projId
	 * @throws exception, if more than on project found
	 * @param $name
	 * @return int $PROJ_ID
	 */
	function getProjByName(&$sqlo, $name) {
		global $error;
		$FUNCNAME= $this->CLASSNAME.':getProjByName';
		
		$sqlsel = "PROJ_ID from PROJ where PRO_PROJ_ID ".$this->selectMotherSQL. " and NAME=".$sqlo->addQuotes($name);
		$sqlo->Quesel($sqlsel);
		if ( $sqlo->ReadRow() ) {
		    $PROJ_ID = $sqlo->RowData[0];
		} else {
			return;
		}
		if ( $sqlo->ReadRow() ) {
			$error->set( $FUNCNAME, 2, 'more than one project found in ID:'.$this->projId.' with name="'.$name.'"' );
		}
		return ($PROJ_ID);
	}
	
	/**
	 * create sub-project
	 * @param $name
	 * @return int $PROJ_ID
	 */
	function makeProj(&$sqlo, $name) {
		global $error;
		$FUNCNAME= $this->CLASSNAME.':makeProj';
		
		$ExistID = $this->getProjByName($sqlo, $name);
		if ($ExistID>0) {
			$error->set( $FUNCNAME, 1, 'Project with this name already exists here.' );
			return;
		}
		
		$args = array( "vals"=>array(
			'PRO_PROJ_ID'=>$this->projId, 
			'NAME'=>$name
			) );
		$insertLib = new insertC();
		$newId = $insertLib->new_meta($sqlo,'PROJ',$args); 
		
		return ($newId);
	}
	

	/**
	 * - create project with inherited userrights of mother-project
	 * - touch mother project
	 * @param $mother_proj - id of the mother project or "NULL"
	 * @param $argus - argument array : NAME, NOTES, ...
	 * @return int $PROJ_ID
	 */
	function makeProjWithMotherRights( &$sqlo, $mother_proj, $argus ) {
		$mother_insertval = $mother_proj;
		if ($mother_proj=="NULL") {
		    
		    $mother_insertval = "";
		} else {
		    
		    // analyse mother
		    $sqlsel = '* from PROJ where PROJ_ID='.$mother_proj;
		    $sqlo->Quesel($sqlsel);
		    $sqlo->ReadArray();
		    $mo_argu = $sqlo->RowData;
		    if ($mo_argu['TYPEX']==1) {
		        // inherit TYPEX flag to child (category) ...
		        $argus['TYPEX'] = 1;
		    }
		}
	
		// analyse mother rights, use mother rights for object rights
		$rightArr=NULL;
		if ($mother_insertval>0) {
			$CCT_ACCESS_ID = glob_elementDataGet( $sqlo, 'PROJ', 'PROJ_ID', $mother_proj, 'CCT_ACCESS_ID');
			$rightArr = access_getAllRightsIns( $sqlo, $CCT_ACCESS_ID );
		}
		$options = array('rights'=>$rightArr);
	
		$argus['PRO_PROJ_ID']= $mother_insertval;
		$proj_id = insert_row($sqlo, 'PROJ', $argus, $options);
		
		if ($proj_id and $mother_insertval>0) {
			// touch mother project
			$actarr = array('d'=>array('x'=>array('ass'=>array('t'=>'PROJ'))), 'a'=>array('key'=>'new') );
			$touchRet = globObjTouch::touch( $sqlo, 'PROJ', $mother_insertval, $actarr );
		}

		return $proj_id;
	}
	
	
}