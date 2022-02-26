<?php
/**
 * sub functions for project: special SQL queries
 * - getPathArr()
 * - objectInProject()
 * @namespace core::obj::PROJ
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @package o.PROJ.subs.inc
 * @version $Header: trunk/src/phplib/o.PROJ.subs.inc 59 2018-11-21 09:04:09Z $
 */
class cProjSubs {

var $stopLevelReached;
	
/**
 * get project path as array  (ID, NAME)
 * @param object $sql
 * @param int $projid
 * @param number $stopProjID  ID of project to stop getting the parent-project
 * @param number $stopLevel : number of levels where to stop scanning ( 2 : stop )
				              - if stopped: save flag in $this->stopLevelReached
 * @return array
 */
function getPathArr( &$sql, 	$projid, $stopProjID=0,	$stopLevel =0 	) {
	
	$MAX_LEVEL		= 10;
	$depth_cnt	    = 0;
	$current_projid = $projid;
	$pather         = array();
	$stopLevelGot   = 0;
	if ( !$stopLevel ) $stopLevel = $MAX_LEVEL;
	$this->stopLevelReached = 0;
	
	while ( $current_projid && ($depth_cnt < $MAX_LEVEL) ) {
	
		if ($stopProjID AND ($stopProjID==$current_projid) ) break;
		
		$sqls= "select pro_proj_id, name, proj_id from proj  where proj_id=" . $current_projid ;
		$sql->query($sqls);
		if ( $sql->ReadRow() ) {
			if ( $depth_cnt >= ($stopLevel-1) ) {
				$stopLevelGot = 1;
				break;
			}
			$current_projid = $sql->RowData[0];
			$master_name    = $sql->RowData[1];
			$pather[$depth_cnt][0]=$sql->RowData[2];
			$pather[$depth_cnt][1]=$master_name;
		} else {
			break;
		}
		$depth_cnt++;
	}
	
	if ($stopLevelGot) $this->stopLevelReached = $depth_cnt;
	
	return ($pather);
}

/**
 * get all sub-projects of project, order by NAME
 * @param  $sqlo
 * @param  int $proj_id : if "0" => convert the id to NULL
 * @return array of IDs
 */
static function getSubProjects(&$sqlo, $proj_id) {
    
    $sql_cond = '='.$proj_id;
    if (!$proj_id) {
        $sql_cond = ' is NULL'; // the root ...
    }

    $sqlsel="PROJ_ID FROM proj WHERE PRO_PROJ_ID ".$sql_cond." ORDER BY name";
	$sqlo->Quesel($sqlsel);

	$projarr=NULL;
	while ( $sqlo->ReadRow() ) {
		$projarr[] = $sqlo->RowData[0];
	}
	return $projarr;
}

static function getNotesKeys ( &$sql ) {
	// FUNCTION: get from GLOBALS["o.PROJ.notesKeys"] all possible keywords for notes
	// RETURN: array of keywords
	$keyarray = NULL;
	$sqls= "select value from H_VAL_INIT where TABLE_NAME='PROJ' AND COLUMN_NAME='NOTES' order by VALUE";
	$sql->query("$sqls");
	while ($sql->ReadRow() ) {
		$keyarray[] = $sql->RowData[0];
	}
	return ($keyarray);
}

/**
 * get object links, outside $projid
 * @param $sqlo
 * @param $projid : can be NULL
 * @param $table
 * @param $objid
 * @return string
 */
static function getObjLinkExcept( &$sqlo, $projid, $table, $objid ) {
	$sqlAfter  = " from PROJ_HAS_ELEM where ".
		" TABLE_NAME='".$table."' and PRIM_KEY=".$sqlo->addQuotes($objid) ;
	if ($projid>0) $sqlAfter .= " and PROJ_ID != ". $projid;
	return ($sqlAfter);
}

static function getTableSQL ( $proj_id, $table ) {
	$sqlAfterSort  = "select PRIM_KEY from PROJ_HAS_ELEM where PROJ_ID=".$proj_id." AND TABLE_NAME='".$table."'";
	return ($sqlAfterSort);
}

static function getTable_ELEMORD_SQL ( $proj_id, $table ) {
    $sqlAfterSort  = "PROJ_HAS_ELEM where PROJ_ID=".$proj_id." AND TABLE_NAME='".$table."' order by ELEM_ORDER, PRIM_KEY";
    return ($sqlAfterSort);
}

static function getTableSQLInSort( $proj_id, $table ) {
	$pkname = PrimNameGet2($table);
	if ($pkname==NULL) {
		sys_error_my( 'table '.$table.' is not defined or has no PK.' );
	}
	$nameCol = importantNameGet2($table);
	if ($nameCol==NULL) {
		sys_error_my( 'table '.$table.' has no importantName.' );
	}
	$sqlafter = self::getTableSQL ( $proj_id, $table );
	$sqlafter = $table." x where x.".$pkname." IN (".$sqlafter.") order by x.".$nameCol;
	return ($sqlafter);
}

/**
 * get number of object of type $table in project $proj_id
 * @param object $sql
 * @param int $proj_id
 * @param string $table
 * @return int (number of objects)
 */
static function getTableCnt ( &$sql, $proj_id, $table ) {
	$sqls = "select count (1) from PROJ_HAS_ELEM where PROJ_ID=".$proj_id." AND TABLE_NAME='".$table."'";
	$sql->query($sqls);
	$sql->ReadRow();
	$cnt = $sql->RowData[0];
	return ($cnt);
}

/**
 * get ALL object of type $table in project $proj_id
 * @param object $sql
 * @param int $proj_id
 * @param string $table
 * @return int (number of objects)
 */
static function get_OBJECTS_all ( &$sqlo, $proj_id, $table ) {
    
    $obj_ids=array();
    $sqls = self::getTableSQL($proj_id, $table);
    $sqlo->query($sqls);
    while ( $sqlo->ReadRow() ) {
        $obj_ids[] = $sqlo->RowData[0];
    }
   
    return $obj_ids;
}

/**
 * - search in project with $motherID for sub-project with NAME=$subProjName
 * - get ID of sub-project
 * @param object $sqlo
 * @param int $motherID 'NULL' or ID
 * @param string $subProjName name of sub project
 * @return int subProjID (ID : if exists, 0 : not exists)
 */
static function getProjByName(&$sqlo, $motherID, $subProjName) {
	if ($subProjName==NULL) return;
	$motherID_sql=$motherID;
	if ($motherID==NULL) return;
	if ($motherID=='NULL') $motherID_sql=' is NULL';
	elseif (!is_numeric($motherID)) return;
	
	$sqlsel= "proj_id from proj where pro_proj_id=".$motherID. " AND NAME='".$subProjName."'";
	$sqlo->Quesel($sqlsel);
    if ( $sqlo->ReadRow() ) {
        $newID = $sqlo->RowData[0];
		return ($newID);
    } else return 0;
}

// check, if object is in project $proj_id
static function objectInProject(&$sqlo, $proj_id, $table, $objid ) {
	$sqlsel = "PRIM_KEY from PROJ_HAS_ELEM where PROJ_ID=".$proj_id.
		" AND TABLE_NAME='".$table."' and PRIM_KEY=".$objid;
	$sqlo->Quesel($sqlsel);
	if ($sqlo->ReadRow()) {
		return 1;
	} else return 0;
}

// check, if object is in project $proj_id
static function objectByNameInProject(&$sqlo, $proj_id, $table, $objName ) {
	$nameCol = importantNameGet2($table);
	$pkCol   = PrimNameGet2($table);
	$sqlsel = "PRIM_KEY from PROJ_HAS_ELEM where PROJ_ID=".$proj_id.
		" AND TABLE_NAME='".$table."' and PRIM_KEY in (".
		"   select ".$pkCol." from ".$table.' where '.$nameCol.'='.$sqlo->addQuotes($objName).
		")";
	$sqlo->Quesel($sqlsel);
	if ($sqlo->ReadRow()) {
		return 1;
	} else return 0;
}

// count, in how many projects this object is
static function cntProjectsByObj(&$sqlo, $table, $objid ) {
	$sqlsel = "count(1) from PROJ_HAS_ELEM where TABLE_NAME='".$table."' and PRIM_KEY=".$objid;
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$cnt = $sqlo->RowData[0];
	return $cnt;
}

// get all distinct tables in project
static function tablesInProject(&$sqlo, $proj_id ) {
	$tables=NULL;
	$sqlsel = "distinct(TABLE_NAME) from PROJ_HAS_ELEM where PROJ_ID=".$proj_id." order by TABLE_NAME";
	$sqlo->Quesel($sqlsel);
	$sqlo->Quesel($sqlsel);
	while ( $sqlo->ReadRow() ) {
	    $tables[] = $sqlo->RowData[0];
	}
	return $tables;
}

// get one PROJECT_IDs where object is located, this answer is amgigous, if object is linked to many projects
static function getProjByObject(&$sqlo, $table, $objid ) {
	$sqlsel = "PROJ_ID from PROJ_HAS_ELEM where TABLE_NAME='".$table."' and PRIM_KEY=".$objid;
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$projid = $sqlo->RowData[0];
	return $projid;
}

// 
/**
 * get all PROJECT_IDs where object is located
 * unsorted result
 * @param object $sqlo
 * @param string $table
 * @param int $objid
 * @return array of PROJ_ID
 */
static function getProjectsByObject(&$sqlo, $table, $objid ) {
	$sqlsel = "PROJ_ID from PROJ_HAS_ELEM where TABLE_NAME='".$table."' and PRIM_KEY=".$objid;
	$sqlo->Quesel($sqlsel);
	$proj_arr=array();
	while($sqlo->ReadRow()) {
		$projid = $sqlo->RowData[0];
		$proj_arr[] = $projid;
	}
	return $proj_arr;
}

// get parent project
static function getParentProj(&$sqlo, $objid ) {
	$sqlsel = "PRO_PROJ_ID from PROJ where PROJ_ID =".$objid;
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$projid = $sqlo->RowData[0];
	return $projid;
}

}