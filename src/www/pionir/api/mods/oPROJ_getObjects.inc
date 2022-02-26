<?php
/**
 *  @package oPROJ_getObjects.inc
 */
require_once('o.PROJ.subs.inc');

/**
 * get elements of project, ordered by NAME
 * @author Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0000938 g.XML-RPC > get list of all objects in project (PROJ)
 */
class oPROJ_getObjects_XRPC extends XRPC_BASE {
	
	
	/**
	 * type analysis
	 * @param unknown $sqlo
	 * @param unknown $tablename
	 * @return void|array[]|unknown[]|unknown[]
	 */
	function _oneTypeAnalyse(&$sqlo, $tablename) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
		$projid  = $this->projid;
		
		if ($tablename=='PROJ_ORI') {
		    // get sub-projects
		    $objarr = $this->getSubProjects($sqlo, $this->projid);
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
	    
	    if (!empty($projIds)) { // tables exist 
	        $oneObjArr=array();
	        foreach( $projIds as $dummy=>$subProjID) {
	            $tmpname = obj_nice_name ( $sqlo, 'PROJ', $subProjID );
	            $oneObjArr[]=array('id'=>$subProjID, 'name'=>$tmpname);
	        }
	        return $oneObjArr;
	    }
	}
	
	
	/**
	 * 
	 * - get elements of project now
	 * - the child-projects are marked as table-type: 
	 *       "PROJ_ORI", 
	 *       "PROJ"  : linked projects 
	 *       "PROJ_MOTHER" : mother project
	 * <pre>
	 * example: Input-fomat 
	 *    get all EXP_TMPL from project 443423 
	 * 		IN:  array('id'=>443423, 'tables'=>array('EXP_TMPL') )
	 * 		OUT: array( 
	 * 			   'EXP_TMPL'=>array( 
	 * 				  0=>array('id'=>433, 'name'=>'expt1'), 
	 * 				  1=>array('id'=>435, 'name'=>'expt2') 
	 * 				  )
	 * 				)
	 *  </pre>
	 * @global array $this->f_param  <pre>
	 *	'id'  => the ID, if "0" => use root project
	 *	'tables'  => OPTIONAL: array of tablenames, get only objects of this type
	 *  'getpath' => 0,1 : get the path as list
	 * 	</pre>
	 * @return array(  <pre> 
	 *   'objects'=> array( TABLE=> array('id'=>ID, 'name'=>NAME)  )
	 *   'mother' => array('id'=>ID, 'name'=>NAME)  
	 *   'path'=> array( array('id'=>ID, 'name'=>NAME) )
	 *   )
	 * </pre>
     * @throws exception
	 */
	function &run() {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$sqlo = $this->_sqlo;
		$proj_id   = $this->_params['id'];
		$tablenames = $this->_params['tables'];
		$tablename = 'PROJ';
		
		$motherObjArr=array();
		$patharr   = array();
		$objarr    = NULL;
		
		$this->projLib = new cProjSubs();
		$this->projid  = $proj_id;
		
		if ($proj_id===0) {
		    
		    $t_rights = tableAccessCheck( $sqlo, $tablename );
		    if ( $t_rights['read'] != 1 ) {
		        $answer = getTableAccessMsg( $tablename, 'read' );
		        $error->set( $FUNCNAME, 2, $answer );
		        return;
		    }
		    
		    // get sub-projects
		    $objarr['PROJ_ORI'] = $this->getSubProjects($sqlo, $proj_id);
		    
		} else {
		
    		$this->__objectCheck($sqlo, 'PROJ', $proj_id);
    		if ( $error->got(READONLY) ) {
    			return -1;
    		}	
		
		
    		if ( !is_array($tablenames) or in_array('PROJ_ORI', $tablenames) ) {
    		    // get sub-projects
    		    $objarr['PROJ_ORI'] = $this->getSubProjects($sqlo, $proj_id);
    		}
    		
    
    		if (is_array($tablenames)) {
    			// only one specific tabel-type
    		    foreach($tablenames as $table_loop) {
    		        if ($table_loop=='PROJ_ORI') {
    		            continue; // already collected ...
    		        }
    		        $oneObjArr = $this->_oneTypeAnalyse($sqlo, $table_loop);
    		        $objarr[$table_loop] = $oneObjArr;
    		    }
    			
    			
    		} else {
    			// get all tables, all objects

    			$tables = $this->projLib->tablesInProject( $sqlo, $proj_id ); 
    			if (sizeof($tables)>0) { // tables exist
    				reset ($tables);
    				foreach( $tables as $dummy=>$oneTable) {
    					$oneObjArr = $this->_oneTypeAnalyse($sqlo, $oneTable);
    					$objarr[$oneTable] = $oneObjArr;
    				}
    				reset ($tables); 
    			}
    		}

    		$parent_proj_id = $this->projLib->getParentProj($sqlo, $proj_id );
    		
    		if ($parent_proj_id) {
    		    $tmpname = obj_nice_name ( $sqlo, 'PROJ', $parent_proj_id );
    		    $motherObjArr=array('id'=>$parent_proj_id, 'name'=>$tmpname);
    		}
		
		
    		
    		if ($this->_params['getpath']) {
    		    $patharr = $this->projLib->getPathArr( $sqlo, $proj_id );
    		}
    		
		}
    	
		return array('objects'=>$objarr, 'mother'=>$motherObjArr, 'path'=>$patharr);
	}
}
