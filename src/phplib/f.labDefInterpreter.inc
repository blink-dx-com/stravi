<?php
/**
 * interpreter for LAB definitions
 * @package f.labDefInterpreter.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @example
 * 
 * 
function set_defs( ) {
	$defs = NULL;
	$defs['QC.ABSTRACT_PROTO.CytoQCparams'] = 12499;
	$defs['QC.ABSTRACT_PROTO.ReleaseBead']  =  array('t'=>'ABSTRACT_PROTO', 'id'=>12787);
	
	$defs['PROD.USER_GROUP.manage']  = array('t'=>'USER_GROUP', 'name'=>'129.Manage'); 	   // alt: HIV_Cyto_KProd Manage
	$defs['PROD.USER_GROUP.normal']  = array('t'=>'USER_GROUP', 'name'=>'129.Assembling'); // alt: HIV_Cyto_KProd
	$defs['PROD.group.AssembManage'] = array('t'=>'USER_GROUP', 'name'=>'129.Assembling.Manage'); // alt: HIV_Cyto_KProd AssembManage
	$defs['PROD.PROJ.01_assemb']    = array('t'=>'PROJ', 'id'=>94861, 'name'=>'project der Gruppe Assembling' ) ;
	$defs['PROD.PROJ.01_assemb.Schichtbuch']   = array('t'=>'PROJ', 'name'=>'Schichtbuch' ) ;  
	$defs['PROD.PROJ.01_assemb.abs_substances']= array('t'=>'PROJ', 'id'=>91221 ) ;  // Vorlagen fuer Management-Themenpark
	$defs['PROD.PROJ.03_lab']   	= array('t'=>'PROJ', 'id'=>91245 ) ; // labor-home 
	$defs['PROD.PROJ.03_lab.abs_substances']   = array('t'=>'PROJ', 'id'=>91264 ) ;  // blueprints for labor ( baerbel)
	$defs['PROD.PROJ.conc_substances']  = array('t'=>'PROJ', 'id'=>72479 );   // main substance project
	
	$this->defs = $defs;
}

   $labdefArr = getLabDefs(); // get specific definition-array
   $labDeflib = new f_labDefInter();
   $labDeflib->setLabDefs($labdefArr);
   $objid     = $labDeflib->getObjID2($sqlo, 'PROD.PROJ.conc_substances' );
 */
 
require_once ('f.wiid_roid.inc');
require_once ("class.filex.inc");

class f_labDefInter {

    var $defs; /** var $defs definitions for a LAB, array()
    	KEY => array('t'=>'EXP', 'id'=>12787, 'wiid'=>1 ) -- define an experiment by ID
    	KEY => array('t'=>'USER_GROUP',  'name'=>'HIV_Cyto_KProd Manage') -- define a group by name
    	KEY => array('key'=>val) -- simple key-value-pair
     */
    
    protected $wiid_num;
    
    function __construct() {
    	$this->wiidLib = new fWiidRoidC();
    	$this->setLabDefs();
    }
    
    /**
     * OVERWRITE this method!
     * set $this->defs = ...
     */
    function setLabDefs() {
        $this->wiid_num=0;
        $this->defs = array();
    }
    
    
    function getDefVal( $key1, $key2=NULL, $key3=NULL, $key4=NULL ) {
        if ($key1!="" AND $key2!="" AND $key3!="") {
    		return ($this->defs[$key1][$key2][$key3]); 
    	}
    	if ($key1!="" AND $key2!="") {
    		return ($this->defs[$key1][$key2]); 
    	}
    	if ($key1!="") {
    		return ($this->defs[$key1]); 
    	}
    }
    
    /**
     * get translated object-ID
     * @param $sqlo
     * @param long $wiid_num
     * @param string $key1 master tag
     * @param string $key2 must be a table
     * @param string $key3 id-tag
     * @return  long ID
     */
    function getObjID(&$sqlo, $wiid_num, $key, $table ) {
    	global $error;
    	$FUNCNAME= 'getObjID';
    	$oriObjID = $this->getDefVal( $key );
    	if (!$oriObjID) {
    		$error->set( $FUNCNAME, 1, 'no value found for key:'. $key );
    		return;
    	}
    	$newid    = $this->wiidLib->getObjID ($sqlo, $table, $wiid_num, $oriObjID);
    	if (!$newid) {
    		$error->set( $FUNCNAME, 2, 'object with roid:'.$wiid_num.':'.$oriObjID.' not found ' );
    		return;
    	}
    	return ($newid);
    }
    
    /**
     * get translated object-ID, all definitions are stored in $this->defs('id', 't', 'wiid')
     * @param $sqlo
     * @param string $key master tag
     * @return long ID
     */
    function getObjID2(&$sqlo, $key ) {
    	global $error;
    	$FUNCNAME= 'getObjID2';
    	
    	$objarr = $this->defs[$key];
    	if ($objarr==NULL) {
    		$error->set( $FUNCNAME, 1, 'no definition found for key:'. $key );
    		return;
    	}
    	
    	$oriObjID = $objarr['id'];
    	if (!$oriObjID) {
    		$error->set( $FUNCNAME, 2, 'no ID found for key:'. $key );
    		return;
    	}
    	if (!$objarr['wiid']) $wiid_num = $this->wiid_num;
    	if ($objarr['t']==NULL) {
    		$error->set( $FUNCNAME, 3, 'table missing for key:'. $key );
    		return;
    	}
    	
    	// analyse object type
    	if (!cct_access_has2($objarr['t']) ) {
    		// object ID can not be translated
    		if ( !gObject_exists ($sqlo, $objarr['t'], $oriObjID) ) {
    			$error->set( $FUNCNAME,5, 'object t:'.$objarr['t'].' id:'.$wiid_num.':'.$oriObjID.' not found.' );
    			return;
    		}
    		return ($oriObjID);
    	}
    	
    	$newid    = $this->wiidLib->getObjID ($sqlo, $objarr['t'], $wiid_num, $oriObjID);
    	if (!$newid) {
    		$error->set( $FUNCNAME,4, 'object t:'.$objarr['t'].' roid:'.$wiid_num.':'.$oriObjID.' not found ' );
    		return;
    	}
    	return ($newid);
    }
    
    /**
     * get object-ID by NAME; need e.g. array('t'=>'USER_GROUP', 'name'=>'HIV_Cyto Prod');
     * @param $sqlo
     * @param string $key 
     * @return ID of object
     */
    function getObjIDbyName(&$sqlo, $key ) {
    	global $error;
    	$FUNCNAME= __CLASS__.':getObjIDbyName';
    	
    	$objArr  = $this->getDefVal( $key );
    	$table   = $objArr['t'];
    	$name    = $objArr['name'];
    	if ($table==NULL) {
    		$error->set( $FUNCNAME,1, 'table missing in object definition.' );
    		return;
    	}
    	if ($table=='EXTRA_CLASS') {
    	    $objid = glob_elementDataGet( $sqlo, $table, 'NAME', $name, 'EXTRA_CLASS_ID');
    	} else {
    	    $objid = glob_getObjIdByName($sqlo, $table, $name);
    	}
    	if (!$objid) {
    		$error->set( $FUNCNAME,2, 'object t:'.$objArr['t'].' name:'.$objArr['name'].' not found ' );
    		return;
    	}
    	return ($objid);
    }
    
    /**
     * get object-ID by NAME; check for unique object name
     * @param $sqlo
     * @param string $table tablename
     * @param string $name name of object
     * @return ID
     */
    function getObjIDbyName2(&$sqlo, $table, $name ) {
    	global $error;
    	$FUNCNAME= 'getObjIDbyName2';
    	
    	$nameColumn = importantNameGet2($table);
    	list($cnt, $objid, $objname) = fileC::objGetByName( $sqlo, $table, $nameColumn, $name);
    	if ($cnt!=1) {
    		$error->set( $FUNCNAME,1, 'no unique object found in "'.$table.'" with name "'.$name.'"' );
    		return;
    	}
    	return $objid ;
    }
    
    /**
     * get Project-ID by case-sensitive NAME; need e.g. array('t'=>'PROJ', 'name'=>'2012-03-12', 'ancestor'=>'098.HIV-NAT');
     * tries to find the right corresponding project by checking the projectpath beneath the given ancestor
     * name of ancestor should be unique
     * return an projID only if one project is found within the given ancestor
     *
     * @param $sqlo
     * @param string $key
     * @return ID of project
     */
    function getProjIDbyName(&$sqlo, $key ) {
    	global $error;
    	require_once 'o.PROJ.subs.inc';
    	$FUNCNAME= __CLASS__.':getProjIDbyName3';
    
    	$objArr  	= $this->getDefVal( $key );
    	$table  	= $objArr['t'];
    	$name   	= $objArr['name'];
    	$ancestor   = $objArr['ancestor'];
    	$proSubLib	= new cProjSubs();
    
    	//check if table exists
    	if ($table!=='PROJ') {
    		$error->set( $FUNCNAME, 1, 'table "PROJ" missing in object definition for key: '.$key );
    		return;
    	}
    
    	//check if ancestor is unique
    	$nameColumn = 'NAME';
    	list($ancnt, $ancid, $ancname) = fileC::objGetByName( $sqlo, $table, $nameColumn, $ancestor);
    	if ($ancnt!=1) {
    		$error->set( $FUNCNAME, 2, 'no unique ancestor project found in "'.$table.'" with name "'.$ancestor.'"');
    		return;
    	}
    	
    	$pPid = $proSubLib->getParentProj($sqlo, $ancid); //parentid des ancestors
    	#echo var_dump($name, fileC::objGetByName( $sqlo, $table, $nameColumn, $name));
    	
    	//get possible proj-ids
    	list($cnt, $objid, $objname) = fileC::objGetByName( $sqlo, $table, $nameColumn, $name);
    	if ($cnt>1) {
    		// try to get the right project by its ancestor
    		// echo var_dump($ancid, $proSubLib->getPathArr( $sqlo, $objid, $ancid));
    		$projects = array(); //all projects with the same name
    		$sqlsel = " PROJ_ID from PROJ where NAME = '".$name."'";
    		$sqlo->Quesel($sqlsel);
    		while($sqlo->ReadRow()) {
    			$projects[] = $sqlo->RowData[0];
    		}
    		$patharr = NULL;
    		$supProj = array();
    		$found = 0;
    		foreach ($projects as $proj) {
    			$patharr = $proSubLib->getPathArr( $sqlo, $proj, $pPid);
    			if ($ancid === $patharr[sizeof($patharr)-1][0]) $found++;
    			if ($found>1) {
    				$error->set( $FUNCNAME, 3, 'multiple projects with name "'.$name.'" found in superproject "'.$ancestor.'"');
    				return;
    			}
    		}
    		return ($proj);
    	}
    	if ($cnt==1) return ($objid);
    	return;
    }

}
