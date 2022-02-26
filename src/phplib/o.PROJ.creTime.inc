<?php
require_once ("insert.inc");

/**
 * create a project structure by time-stamp
 * @author Steffen Kube (steffen@blink-dx.com)
 * @package o.PROJ.creTime.inc
 */
class oProjCreTime {

    /**
     * 
     * @param object $sql
     * @param int $baseID : ID of base-project
     * @param array $options "access" => array(group => access-rights)  : set specials additional access-rights
     */
    function __construct( &$sqlo,    $baseID,   	$options   	) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
    	$this->baseID = $baseID;
    	$this->opts   = $options;
    	
    	if (!gObject_exists ($sqlo, 'PROJ', $baseID) ) {
    	    $error->set( $FUNCNAME, 1, "Base-Project-ID: ".$this->baseID.' not found.' );
    	    return;
    	}
    }
    
    /**
     * 
     * @param object $sql
     * @param int $dateUnx  date in unix-fomat
     * @param string $name  optinal name of project
     * @param string $action 
     *  ["get"]  - only check
    	"create" - try to create
     * @return array : array of project-IDs
     *   ('day.id'=>$d_id, 'month.id'=>$m_id, 'year.id'=>$y_id )
     */
    function getProjID( &$sql, $dateUnx,  $name, $action='get' ) {
    	
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $d_id = 0;
        $m_id = 0;
        $datex=array();
    	$datex["y"] = date ("Y", $dateUnx);
    	$datex["m"] = date ("m", $dateUnx);
    	$datex["d"] = date ("d", $dateUnx);
    	
    	$y_id = $this->_getProjByName( $sql, $this->baseID, $datex["y"], $action);
    	if ( $y_id ) $m_id = $this->_getProjByName( $sql, $y_id, $datex["m"], $action);
    	if ( $m_id ) $d_id = $this->_getProjByName( $sql, $m_id, $datex["d"], $action);
    	
    	if ($error->Got(READONLY))  {
    		$error->set( $FUNCNAME, 1, "Error occurred during project creation. Base-Project-ID: ".$this->baseID );
    		return;
    	}
    	return array('day.id'=>$d_id, 'month.id'=>$m_id, 'year.id'=>$y_id );
    }
    
    /**
     * create project: YYYY/YYYY-MM/YYYY-MM-DD USERKEY
    		   USERKEY - first letter of user-name
     * @param object $sql
     * @param int $dateUnx date in unix-fomat
     * @param string $name optinal name of project
     * @param string $action  
     *     "" - only check
    	   "create" - try to create
     * @return int project id
     */
    function getProjAlgo2( &$sql, $dateUnx,   $name, $action ) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$numOfNickLetters = 3;
    	$datex=array();
    	$datex["y"] = date ("Y", $dateUnx);
    	$datex["m"] = date ("m", $dateUnx);
    	$datex["d"] = date ("d", $dateUnx);
    	
    	
    	$userkey = substr($_SESSION['sec']['appuser'],0,$numOfNickLetters);
    	
    	$y_id = $this->_getProjByName( $sql, $this->baseID, $datex["y"], $action);
    	if ( $y_id ) $m_id = $this->_getProjByName( $sql, $y_id, $datex["y"]."_".$datex["m"], $action);
    	if ( $m_id ) $d_id = $this->_getProjByName( $sql, $m_id, $datex["y"]."-".$datex["m"]."-".$datex["d"]." ".$userkey, $action);
    	
    	if ($error->Got(READONLY))  {
    		$error->set( $FUNCNAME, 1, "Error occurred during project creation. Base-Project-ID: ".$this->baseID );
    		return;
    	}
    	return ($d_id);
    }
    
    
    
    /**
     * get/create one project in project = $motherID
     * @param object $sql
     * @param int $motherID
     * @param string $newName  optional name of project
     * @param string $action 
     *  ["get"] - only check
    	"create" - try to create
     * @return int $newID
     */
    function _getProjByName( 	&$sql, 	$motherID,	$newName, $action 		) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	    
    	
    	$sqls= "select proj_id from proj where pro_proj_id=".$motherID. " AND NAME='".$newName."'";
        $sql->query($sqls);
        if ( $sql->ReadRow() ) {
            $newID = $sql->RowData[0];
    		return ($newID);
        }
    	
    	if ( $action != "create" ) return 0;
    		
    	$o_rights = access_check( $sql, "PROJ", $motherID ); 
    	if ($o_rights["insert"] != 1) {
    		$error->set($FUNCNAME, 1, 'no "insert" right on project [ID:'.$motherID.']!');
    		return;
    	} 
    	
    	$insOpt = NULL;
    	if ( $this->opts["access"]!="" ) {
    		// add access
    		$insOpt["rights"] = $this->opts["access"];	
    	}
    	$argu=array();
    	$argu["NAME"] 		 = $newName;
    	$argu["PRO_PROJ_ID"] = $motherID;
    	$newID = insert_row_role( $sql, "PROJ", $argu, $insOpt ); // with role check !!!
    	
    	return ($newID);
    		
    }

}
