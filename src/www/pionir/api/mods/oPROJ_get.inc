<?php
require_once('o.PROJ.subs.inc');

/**
 * get elements of project, ordered by NAME
 * 
 * 
 * - the child-projects are marked as table-type: 
 *       "PROJ_ORI", 
 *       "PROJ"  : linked projects 
 *       "PROJ_MOTHER" : mother project
 * ```     
 * EXAMPLE:
 *    get all EXP_TMPL from project 443423 
 * 		IN:  array('id'=>443423, 'tables'=>array('EXP_TMPL') )
 * 		OUT: array( 
 * 			   'EXP_TMPL'=>array( 
 * 				  0=>array('id'=>433, 'name'=>'expt1'), 
 * 				  1=>array('id'=>435, 'name'=>'expt2') 
 * 				  )
 * 				)
 * 
 * OUTPUT: array( 
 *   'objects'=> array( TABLE=> array('id'=>ID, 'name'=>NAME)  )
 *   'mother' => array('id'=>ID, 'name'=>NAME)  
 *   'path'   => array( array('id'=>ID, 'name'=>NAME) )
 *   )
 * ```
 * @global array $this->f_param  <pre>
 *	'id'  => the ID of PROJ	
 *	'table'   => OPTIONAL: tablename, get only objects of this type
 *  'path'    => [0],1 : get the paths as array
 *  'objects' => [0],1 : get objects of project
 * @throws exception
 * @author Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0000938 g.XML-RPC > get list of all objects in project (PROJ)
 * @package WEBAPI\oPROJ_get
 * @module oPROJ_get.inc
 */
class oPROJ_get_XRPC extends XRPC_BASE {

	function _oneTypeAnalyse(&$sqlo, $tablename) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
		$projid  = $this->projid;
		
		if ($tablename=='PROJ_ORI') {
		    // get sub-projects
		    $objarr = $this->getSubProjects($sqlo, $projid);
		    return $objarr;
		}
		
		$pkname  = PrimNameGet2($tablename);
		if ($pkname==NULL) {
		    $error->set( $FUNCNAME, 1, 'table "'.$tablename.'" has no defined PK.' );
		    return;
		}
		
		$nameCol = importantNameGet2($tablename);
		$sqlAfter= $this->projLib->getTableSQLInSort( $projid, $tablename );
		$sqlsel  = "x.".$pkname.", x.".$nameCol." from ".$sqlAfter;
		$sqlo->Quesel($sqlsel);
		$objarr=NULL;
		while ( $sqlo->ReadRow() ) {
		    $tmpid   = $sqlo->RowData[0];
		    $tmpname = $sqlo->RowData[1];

		    $objarr[] = array('id'=>$tmpid, 'name'=>$tmpname);
		}
		return $objarr;
	}
	
	private function getSubProjects($sqlo, $proj_id) {
	    $projIds = $this->projLib->getSubProjects($sqlo, $proj_id);
	    if (sizeof($projIds)>0) { // tables exist
	        reset ($projIds);
	        $oneObjArr=NULL;
	        foreach( $projIds as $subProjID) {
	            $tmpname = obj_nice_name ( $sqlo, 'PROJ', $subProjID );
	            $oneObjArr[]=array('id'=>$subProjID, 'name'=>$tmpname);
	        }
	        reset ($projIds);
	        return $oneObjArr;
	    }
	}
	
	
	/**
	 
	 */
	function &run() {
		global $error;
		
		$sqlo = $this->_sqlo;
		$proj_id    = $this->_params['id'];
		$tablenames = $this->_params['tables'];
		
		
		$this->__objectCheck($sqlo, 'PROJ', $proj_id);
		if ( $error->got(READONLY) ) {
			return -1;
		}	
		
		$this->projLib = new cProjSubs();
		$this->projid = $proj_id;
		$objarr = NULL;
		
		if ($this->_params['objects']>0) {

    		if (is_array($tablenames)) {
    			// only one specific tabel-type
    		    foreach($tablenames as $tablename) {
    		        $oneObjArr = $this->_oneTypeAnalyse($sqlo, $tablename);
    		        $objarr[$tablename] = $oneObjArr;
    		    }
    			
    			
    		} else {
    			// get all tables, all objects
    			
    			// get sub-projects
    		    $objarr['PROJ_ORI'] = $this->getSubProjects($sqlo, $proj_id);

    			
    			$tables = $this->projLib->tablesInProject( $sqlo, $proj_id ); 
    			if (sizeof($tables)>0) { // tables exist
    				reset ($tables);
    				foreach( $tables as $oneTable) {
    					$oneObjArr = $this->_oneTypeAnalyse($sqlo, $oneTable);
    					$objarr[$oneTable] = $oneObjArr;
    				}
    				reset ($tables); 
    			}
    		}
		}
		
		$parent_proj_id = $this->projLib->getParentProj($sqlo, $proj_id );
		$motherObjArr=array();
		if ($parent_proj_id) {
		    $tmpname = obj_nice_name ( $sqlo, 'PROJ', $parent_proj_id );
		    $motherObjArr=array('id'=>$parent_proj_id, 'name'=>$tmpname);
		}
		
		$patharr=array();
		if ($this->_params['path']) {
		    $patharr = $this->projLib->getPathArr( $sqlo, $proj_id );
		}
		
    	
		return array('objects'=>$objarr, 'mother'=>$motherObjArr, 'path'=>$patharr);
	}
}
