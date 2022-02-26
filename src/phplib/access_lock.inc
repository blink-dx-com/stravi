<?php
/**
 * help methods to lock/unlock single objects
 * @namespace core::obj::CCT_ACCESS
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
require_once ("javascript.inc");
// require_once ('func_form.inc');
require_once ("access_mani.inc");
require_once ('glob.obj.lockPolicy.inc');

/**
 * - lock/reopen an object    (remove all access rights)
 * - unlock  an object (add rights)
 * @package access_lock.inc
 * @swreq SREQ:0002125 g > access_lock (helper lib)
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 *
 */
class gObjaccLock {
	
	private $lockPolLibs; // libs defined in glob.obj.lockPolicy.inc
	private $infoLog=array(); // array of text per __construct
	
    function __construct() {
    	
    	$this->outLevel=0;
    	$this->resetInfo();
    	
    	/* 
    	 * manage children-columns, where the linked objects are also locked/reopen on action
    	 * call later "lock", "reopen" methods in glob.obj.lockPolicy.inc
    	 */
    	$this->lockChildrenDef= array(
    		'CONCRETE_SUBST'=> array('lock'=>1),  
    		'CHIP_READER'   => array('lock'=>1, 'reopen'=>1), 
    		'EXP'   		=> array('lock'=>1)
    		);
    }
    	
    function setOutLevel($outLevel) {
    	$this->outLevel=$outLevel;
    }
    
    // add text to infoarr
    private function setinfo($text) {
    	// check, if array is too big
    
    	if (sizeof($this->infoLog)<500)
    		$this->infoLog[] = "- ".$text;
    }
    
    // get info text
    public function getinfo() {
    	return $this->infoLog;
    }
    
    // reset info array to prevent big data
    public function resetInfo() {
    	$this->infoLog = array();
    }
    
    function _getCCT_ACCESS_ID( &$sql, $tablename, $id  ) {
    	$pkname = PrimNameGet2($tablename);
    	$sqls   = "select CCT_ACCESS_ID from ".$tablename." where ".$pkname."=".$id;
    	$sql->query($sqls, "_getCCT_ACCESS_ID");
    	$sql->ReadRow();
    	return ( $sql->RowData[0] );
    }
    
    /**
     * - lock one object
     * - on special objects: lock their children
     * TBD: circular reference possible !
     * @param object $sql
     * @param string $tablename
     * @param int $id
     * @return -
     */
    function accessLockOne( &$sqlo, $tablename, $id) {
    	
    	//$pkname = PrimNameGet2($tablename);
    	$cct_access_id = $this->_getCCT_ACCESS_ID( $sqlo, $tablename, $id  );
    	
    	// remove all manipulation rights
    	access_upDelMani($sqlo, $cct_access_id);
    	$this->setinfo("$tablename ID:$id locked.");
    	$this->lockChildren($sqlo, $tablename, $id);
    }
    
    /**
     * lock known children
     *  - WARNING: this calls recursive accessLockOne()
     * @param  $sqlo
     * @param  $tablename
     * @param  $id
     */
    function lockChildren(&$sqlo, $tablename, $objid) {
    	
    	if ( $this->lockChildrenDef[$tablename]['lock'] ) {
    		// lock children
    		if (!isset($this->lockPolLibs[$tablename])) {
    			$tmpClsName = 'o'.$tablename.'_lockPol';
    			$this->lockPolLibs[$tablename] = new $tmpClsName();
    		}
    		
    		$childrenArr = $this->lockPolLibs[$tablename]->getLockChildren($sqlo, $objid);
    		if (is_array($childrenArr)) {
    			reset ($childrenArr);
    			foreach( $childrenArr as $childSubarr) {
    				$childTable = $childSubarr['t'];
    				$childID    = $childSubarr['id'];
    				if ($childID) {
    					$this->accessLockOne( $sqlo, $childTable, $childID);
    				}
    			}
    		}
    	}
    }
    
    /**
     * reopen a child
     * - WARNING: this calls recursive reopen()
     * @param object $sqlo
     * @return -
     */
    function _reopenChilds(&$sqlo, $tablename, $objid, $newrights, $group_id) {
    	
    	// lock children
    	if (!isset($this->lockPolLibs[$tablename])) {
    		$tmpClsName = 'o'.$tablename.'_lockPol';
    		$this->lockPolLibs[$tablename] = new $tmpClsName();
    	}
    	
    	$childrenArr = $this->lockPolLibs[$tablename]->getOpenChildren($sqlo, $objid);
    	if (is_array($childrenArr)) {
    		reset ($childrenArr);
    		foreach( $childrenArr as $childSubarr) {
    			$childTable = $childSubarr['t'];
    			$childID    = $childSubarr['id'];
    			if ($childID) {
    				$this->reopen ( $sqlo, $childTable, $childID, $newrights, $group_id );
    			}
    		}
    	}
    	
    }
    
    /**
     * reopen an object
     * - special tables: reopens also selected children!
     * - this is a recursive method !!! using _reopenChild()
     * TBD: use this function also for glob.objtab.access_mod.php
     * @param object $sql
     * @param string $tablename
     * @param int $id
     * @param array $newrights
     * @param int $group_id ID of USER_GROUP
     * FUTURE: @param int $isChild 0,1 : if child: add the a CCT_ACCLOG status "reopen" automatically !!!
    	 *  $argu['CCT_ACCESS_ID']=$this->cct_access_id;
    		$argu['DB_USER_ID']	  =${cctuser_id};
    		$argu['ACT_DATE']	  =time();
    		$argu['H_ALOG_ACT_ID']=$reopenid;
    		$argu['NOTES']		  =$parx["notes"];
    		$insopt   = array("types"=>array('ACT_DATE'=>'TIME'));
    		$sqlAfter = insert_row_Get( $sql, "CCT_ACCLOG", $argu, $insopt );
    		$retval   = $sql->queryInsert("CCT_ACCLOG", $sqlAfter, 'CCT_ACCESS_ID'); 
     * @return -
     */
    function reopen ( &$sql, $tablename, $id, $newrights, $group_id ) { 
    	global $error;
    	$FUNCNAME= __CLASS__.":reopen";
    			
    	
    	$cct_access_id = $this->_getCCT_ACCESS_ID( $sql, $tablename, $id  );
    	access_write( $sql, $cct_access_id, $newrights, $group_id);
    	if ($error->Got(READONLY)) {
    		$error->set( $FUNCNAME, 4, "access_write failed." );
    		return;
    	}
    	$this->setinfo($tablename." ID:".$id." reopened.");
    	
    	if ( $this->lockChildrenDef[$tablename]['reopen'] ) {
    		$this->_reopenChilds($sql, $tablename, $id, $newrights, $group_id);
    	}
    }

}


