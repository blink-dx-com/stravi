<?php
 require_once ("insertx.inc");
 require_once ("object.info.inc");
 require_once 'o.SATTACH.mod.inc';
 require_once 'o.SATTACH.subs.inc';
 
 /**
 * simple object copy with assoc elements
 * $Header: trunk/src/phplib/glob.obj.copyobj1.inc 59 2018-11-21 09:04:09Z $
 * @package glob.obj.copyobj1.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
 class gObjCopyS {
 
 	public $creaInfo;
 	
 	/**
 	 * copy all object attachments
 	 * @param array $cpopt
 	 *   'attIgnorKey'=>array(of keys) ignore attachments with keys
 	 */
 	private function copyAttachments(&$sqlo, $tablename, $srcid, $destid, $cpopt=NULL) {
 		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
 		$cacheRelIDs= array();
 		
 		$sqlo->Quesel("REL_ID, KEY FROM SATTACH WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$srcid. " ORDER BY REL_ID");
		while ($sqlo->ReadArray() ) {
			$relid   = $sqlo->RowData['REL_ID'];
			$keyLoop = $sqlo->RowData['KEY'];
			
			if ( is_array($cpopt['attIgnorKey']) and $keyLoop!=NULL) {
				if (in_array($keyLoop, $cpopt['attIgnorKey'])) {
					continue; // ignore attachment
				}
			}
			
			$cacheRelIDs[] = $relid;
		}
		
		if (!sizeof($cacheRelIDs)) {
			return;
		}
		
		$this->creaInfo[] =  "... copy attachments";
		
		$attachSubLib = new cSattachSubs();
		$attachModLib = new oSattachMod();
		$attachModLib->setObj($tablename, $destid);
		
		reset ($cacheRelIDs);
		foreach( $cacheRelIDs as $dummy=>$relid) {
			
			$sqlo->Quesel("* FROM SATTACH WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$srcid. " and REL_ID=".$relid);
			$sqlo->ReadArray();
			$feats = $sqlo->RowData;

			
			unset($feats['REL_ID']);
			unset($feats['OBJ_ID']);
			
			$newRelID   = $attachModLib->insertAtt( $sqlo, $feats );
			
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 1, '' );
				return;
			}
			
			if ($newRelID) {
				// get file and copy ...
				$srcfile    = $attachSubLib->getDocumentPath($tablename, $srcid, $relid);
				if (file_exists( $srcfile )) {
					$attachModLib->copyFile($newRelID, $srcfile );
				}
			}
		}
		
 	}
 	
 	/**
     * copy one object with:
     * - features
     * - assoc elements
     * - vario
     * - attachments
     * @param $option
     *   'attach' => [0],1 - with attachments ?
     *   'attIgnorKey' => array() - attachment option
     * @return int ID of new object
 	 */
 	function copyOne( &$sqlo, &$sqlo2, $table, $srcid, $option=NULL ) {
 		$FUNCNAME="copyOne";
 		global $error;
 		
 		$this->creaInfo = NULL;
 		
 		$rawopt = array("vario"=>1, "xObjParById"=>1);
 		$obj_oriparam = objInfoC::getParamsRaw($sqlo, $table, $srcid, $rawopt);
 		
 		// unset PK-VALUE, because this will be a new value
 		$prim_name = PrimNameGet2($table);
 		unset( $obj_oriparam['vals'][$prim_name] );
 		
 		$obj_oriparam['acc'] = array('ORI_ID' => $srcid);  // save the original object id in the copy-object
	 	$this->creaInfo[] =  "... creating new object in table ".$table;
		
	 	$new_id = insertC::new_meta( $sqlo, $table, $obj_oriparam );
		if (!$new_id) {
			$error->set($FUNCNAME, 1, 'object in table '.$table.' not created!');
			return;
		}
		
		// get assoc tables
		$assocs = get_assoc_tables($table);
		if (sizeof($assocs)) {
			$assoc_arr=NULL;
			reset ($assocs);
			foreach( $assocs as $tabAssoc=>$dummy) {
				$assoc_arr[$tabAssoc]=1;
			}
			reset ($assocs); 
			$AssocLib = new fCopyAssoc();
			$AssocLib->new_assocFrgn($sqlo, $sqlo2, $table, $srcid, $new_id, $assoc_arr );
		 	if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 2, 'Error occurred during copy of assoc elements.' );
				return ($new_id);
			}
		}
		
		if ($option['attach']) {
			$this->copyAttachments($sqlo, $table, $srcid, $new_id, $option);
		}
		
		return ($new_id);
 	}
 	
 	
 
 }
 