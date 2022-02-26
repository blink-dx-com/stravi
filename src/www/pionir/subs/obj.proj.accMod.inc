<?php
require_once ("o.PROJ.tree.inc");

/**
 * project access modification module
 * $Header: trunk/src/www/pionir/obj.proj.accMod.inc 59 2018-11-21 09:04:09Z $
 * @package obj.proj.accMod.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class O_PROJ_accMod {
	
function setProj($projid, $name, $cct_access_id) {
	$this->prim_id=$projid;
	$this->namex =$name;
	$this->cct_access_id = $cct_access_id;
	$this->errcnt=0;
}

/**
 * do recursive PROJECT access settings
 */
function projectDo( &$sql, $proj_recursive_method, &$all_rights_arr ) {
	global $error;
	
	$prim_id = $this->prim_id;
	
	$mainRigProj = new gObjAccRigProj($sql);
	$mainRigProj->getGroups( $sql );
	$namex = $this->namex;
	
	$sql->query('SELECT extra_obj_id FROM proj WHERE proj_id = '.$prim_id);
	$sql->readRow();
	$proj_data     = array(
					'name'            => $namex,
					'reading_allowed' => 1,
					'cct_access_id'   => $this->cct_access_id,
					'extra_obj_id'    => $sql->RowData[0]
					);
	$proj_tree = &oPROJ_tree::tree_with_leafs_and_info2array($sql, $prim_id);
	
	if ($error->printLast()) htmlFoot();

	echo '<font color=gray>project action: &quot;'.$proj_recursive_method.'&quot; rights.</font><br /><br />';
	echo '<ul style="list-style-type: none;">';
	
	$mainRigProj->access_write_recursive($sql, $prim_id, $all_rights_arr, $proj_data, $proj_tree, $proj_recursive_method);
	$this->errcnt = $mainRigProj->getErrCnt();
}

function getErrCnt() {
	return ($this->errcnt);
}

}
