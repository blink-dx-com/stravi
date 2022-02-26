<?php
require_once ('g_delete.inc');
require_once ("glob.obj.assocObj.inc");

/**
 * meta delete one object:
 * - delete also ASSOC objects
 * - delete also project links
 * $Header: trunk/src/phplib/g_deleteMeta.inc 59 2018-11-21 09:04:09Z $
 * @package g_deleteMeta.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)

 */
class fObjDelMeta {
	
	/**
	 * 
	 * @var array $parx
	 *   "optdeep" : deep delete ?
	 *   "projLinksIgnore" : 0,1
	 */
	private $parx;
	
	/**
	 * 
	 * @var $delxopt
	 *   "deep"
	 *   "projLinksIgnore"
	 */
	private $delxopt;

/**
 * 
 */
function __construct(&$sql, $tablename) {
	$this->dellib = new fObjDelC();
	$this->tablename = $tablename;
	$this->is_BO 	   = cct_access_has2($tablename);
	$this->infox = NULL;
	$this->debug  = isset($_SESSION['userGlob']['g.debugLevel']) ? $_SESSION['userGlob']['g.debugLevel'] : 0;
	
	if ($this->is_BO) {
		$this->assocLib = new objAssocObj();
		$this->assocLib->initTable($sql, $tablename);
		if ( $this->assocLib->typeHasAssocs() > 0 ) {
			$this->infox["tabHasAssocs"] = 1;
		}
	}
	
	$this->delxopt		 = NULL;
	
	
}

function getAssocInfo(){
	return ( $this->infox["tabHasAssocs"] );
}
function getAssocsNice() {
	return $this->assocLib->getAssocsNice() ;
}

function setParams($forceflag, $parx) {
	$this->forceflag= $forceflag;
	$this->parx = $parx;
	
	$this->delxopt["info"]= 0;
	
	if ( $parx["optdeep"]>0 ) {
		$this->delxopt["deep"]=1;
	}
	if ( $parx["projLinksIgnore"]>0 ) {
		$this->delxopt["projLinksIgnore"]=1;
	}
}

function setAssocDo($do) {
	$this->infox["hasAssocDoDeep"] = $do;
}

/**
 * delete associated objects now
 * INPUT:  $this->forceflag
 * @param unknown $sql
 * @return void|number
 */
function _delAsocObj( &$sql ) {

	global $error;
	$FUNCNAME = "_delAsocObj";
	
	$objPool = &$this->assocLib->getObjPool();
	$objDef  = &$this->assocLib->getObjDef();
	
	$errcnt = 0;
	$objcnt = 0;
	$saveErr = NULL;
	$saveObj = 0;
	
	$dellib2 = new fObjDelC();
	
	foreach( $objPool as $assocTab=>$colarr) {
	
		$colsDef = $objDef[$assocTab]["cols"];
		
		foreach( $colarr as $colname=>$valarr) {
		
			$ForObjTab = $colsDef[$colname];
			if ($ForObjTab==NULL)  {
				$error->set( $FUNCNAME, 2, "foreign-object-table missing for ".$assocTab.":".$colname );
				return;
			} 
			
			$lastObj = 0;
			foreach( $valarr as $dummy=>$objid) {
			
				$retval = 0;
				
				if ( $this->forceflag==2 ) {
					if ( $objid!="" ) {
						// only try to delete once in this loop ...
						if ($lastObj != $objid) $retval  = $dellib2->obj_delete ($sql, $ForObjTab, $objid);
						$lastObj = $objid;
					}
				}
				
				if ($retval<0 or $error->Got(READONLY)) {
					
					if (!$errcnt) {
						$errLast   = $error->getLast();
						$error_txt = $errLast->text;
						$error_id  = $errLast->id;
	
						$saveErr = $error_txt.':'.$error_id.':'.$retval;
						$saveObj = $ForObjTab.":".$objid;
					}
					
					$error->reset(); // reset error !!!
					
					$errcnt++;
				}
				else $objcnt++;
				//
			}
			reset ($valarr); 
		}
		reset ($colarr); 
	}
	reset ($objPool); 
	
	if ($errcnt>0) {
		$error->set( $FUNCNAME, 1, "$errcnt errors during subObject-delete. (First error: obj:".
				$saveObj." err:".$saveErr.")" );
	}
	
	return ($objcnt);
}

/**
 * delete ONE BO
 * @global $this->forceflag
 * @global $this->debug
 * $this->infox
 * $this->assocLib
 * $this->debug
 * @return $infox ['error', 'ok', 'assocDelCnt', 'assocErr']
 */
function doOneObj(&$sql2, $main_id ) {
	global $error;
	$FUNCNAME='doOneObj';
	
	$tablename = $this->tablename;
	$isOK = 0;
	$infox = NULL;
	
	// get assoc Objects
	if ($this->infox["hasAssocDoDeep"] ) {
		$this->assocLib->initObj($main_id);
		$this->assocLib->getForeignObs($sql2);
	}			
		
	if ( $this->debug <= 3 AND $this->forceflag>1 ) {
		$retval = $this->dellib->obj_delete ($sql2, $tablename, $main_id, NULL, NULL, $this->delxopt);
	} else {
		$retval = 0;
		$isOK = 1;
	}
	
	if ($retval<0) {
		return ($infox);
	} else {
		$isOK = 1;
		// glob_printr( $this->assocLib->objPool, "objPool info" );
		if ( $this->infox["hasAssocDoDeep"] AND $this->assocLib->hasObjects() ) {
		
			$infox["assocDelCnt"] = $this->_delAsocObj( $sql2 );
			
			if ($error->Got(READONLY))  {
				$isOK = 0;
				//$errLast   = $error->getLast();
				//$error_txt = $errLast->text. ' ID: '.$err->id;
				$infox["assocErr"] = 'Assoc error detected.';
				// do not $error->reset();
			}
		}
	
		
	}
	
	$infox["ok"] = $isOK;
	return ($infox);
}

function AssGetObjCnt() {
	return $this->assocLib->getObjCntNice();
}

}

// ----------------------------