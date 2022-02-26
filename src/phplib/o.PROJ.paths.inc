<?php
require_once("o.PROJ.subs.inc");

/**
 * - project path sub functions
  		 - show interactive project paths
 * @package o.PROJ.paths.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oPROJpathC {

function __construct(
	$optx = NULL // "relPioPath" e.g. "../pionir/"
	) {
	$this->query_condition = 'PRO_PROJ_ID is NULL';
	$this->xRelPath   = $optx["relPioPath"];
	$this->projSubLib = new cProjSubs();
}

/**
 * show the path, direct echos
 * @param object $sql
 * @param object $id
 * @param string $desturl : e.g. : "edit.tmpl.php?id="
 * @param array $optx
 */
function showPath(
	&$sql, 
	$id, 
	$desturl,	
	$optx= NULL	
	) {
	
	
	if ( $id!='NULL' ) {
		$this->query_condition = "PRO_PROJ_ID = $id";
		$sqls= 'select name, pro_proj_id, notes, extra_obj_id, cct_access_id from proj  where proj_id='. $id ;
		$sql->query("$sqls");
		if ( $sql->ReadRow() ) {
			$proj_name 	  = $sql->RowData[0];
			$master_proj_id = $sql->RowData[1];
			$master_proj_master = $master_proj_id;
			$proj_notes 	  = $sql->RowData[2];
			$proj_eobjid    = $sql->RowData[3];
			$cct_access_id  = $sql->RowData[4];
		} else {
			echo "Folder navigator: <font color=red><B>Error:</B></font> Folder with ID=$id not found.";
			return;
		}
	}
	
	if (!$master_proj_master ) $linkto_proj = 'NULL';
	else $linkto_proj = $master_proj_master;
	
	$pather    = $this->projSubLib->getPathArr( $sql, $master_proj_id );
	$depth_cnt = sizeof($pather);

	$ROOT_IMG  = "db:";
	echo "<a href=\"".$desturl.$linkto_proj."\" ><img src=\"".$this->xRelPath."images/but.projup.gif\" border=0 TITLE=\"up\"></a>";
    echo ' <font color=gray><B>Folder:</B></font> ';

    if ($id!='NULL') {
        echo "<a href=\"".$desturl."NULL\"><font color=#0000A0>$ROOT_IMG</font></a>/";
    }
	
    $tmps = 1;
    $cnt  = $depth_cnt-1;
    while ( $cnt >= 0) {	
        $master_proj_id= $pather[$cnt][0];
        $master_name=    $pather[$cnt][1];
        echo "<a href=\"".$desturl.$master_proj_id."\" >". $master_name . "</a>/";
        $cnt--;
        $tmps++;
    }

    echo "<B><a href=\"".$desturl.$id."\" ><font color=#000000>". $proj_name . "</font></a></B>\n";
	
}

/**
 * get pure path text
 * @param $sql
 * @param string $id : 'NULL':root or INTEGER for PROJ_ID
 * @param $desturl     e.g. : "edit.tmpl.php?id="
 * @param int $stopProjID  ID of project to stop getting the parent-project
 * @param int $stopLevel   number of levels where to stop scanning ( 2 : stop )
 * @param array $options
 *   'ROOT_TXT' : optional root-text 
 * @return array(basetext, lastProjectName)
 */
private function _getPathTextSub(&$sql, $id, $stopProjID=0, $stopLevel=0, $options=array()  ) {
	global $error;
	$FUNCNAME= '_getPathTextSub';
	
	$resultstr = "";
	if ( $id!='NULL' ) {
		$this->query_condition = "PRO_PROJ_ID = $id";
		$sqls= 'select name, pro_proj_id from proj  where proj_id='. $id ;
		$sql->query("$sqls");
		if ( $sql->ReadRow() ) {
			$proj_name 	    = $sql->RowData[0];
			$master_proj_id = $sql->RowData[1];
		} else {
			$error->set($FUNCNAME, 1, "Project with ID=$id not found.");
			return;
		}
	}
	
	
	$pather    = $this->projSubLib->getPathArr( $sql, $master_proj_id, $stopProjID, $stopLevel );
	$depth_cnt = sizeof($pather);

	$ROOT_TXT  = "db:/";
    $resultstr = "";
    if ($options['ROOT_TXT']!=NULL) {
        $resultstr = $options['ROOT_TXT'];
    } else {

        if ($id!='NULL') {
            $resultstr = $ROOT_TXT;
        }
    	
    	if ($stopProjID) {
    		$resultstr = "./";
    	}
    	if ( $this->projSubLib->stopLevelReached>0 ) {
    		$resultstr = ".../";
    	}
    }
    
    $tmps = 1;
    $cnt  = $depth_cnt-1;
    while ( $cnt >= 0) {	
        $master_proj_id= $pather[$cnt][0];
        $master_name=    $pather[$cnt][1];
        $resultstr .= $master_name . "/";
        $cnt--;
        $tmps++;
    }
    
    return array($resultstr, $proj_name);
}

/**
 * show slim path
 * @param $sql
 * @param $id
 * @param $desturl    // e.g. : "edit.tmpl.php?id="
 * @param $stopProjID // ID of project to stop getting the parent-project
 * @param $stopLevel  // number of levels where to stop scanning ( 2 : stop )
 * @return string
 */
function showPathSlim(	&$sql, 	$id, $desturl,	$stopProjID=0, 	$stopLevel=0, $options=array()  ) {

    list($resultstr,$proj_name)  = $this->_getPathTextSub($sql, $id, $stopProjID, $stopLevel, $options);
    $resultstr .= "<a href=\"".$desturl.$id."\" ><font color=#000000>". $proj_name . "</font></a>\n";
	
	return ($resultstr);
}

/**
 * get pure path text
 * @param $sql
 * @param $id
 * @param $stopProjID // ID of project to stop getting the parent-project
 * @param $stopLevel  // number of levels where to stop scanning ( 2 : stop )
 * @return string project path
 */
function getPathText(&$sql, $id, $stopProjID=0, $stopLevel=0  ) {
	list($resultstr,$proj_name)  = $this->_getPathTextSub($sql, $id, $stopProjID, $stopLevel);
    $resultstr .= $proj_name;
	
	return ($resultstr);
}

function showSubProjs(&$sql, $id, $desturl) {

    if ( $id=='NULL' ) {
        $query_condition = "PRO_PROJ_ID is NULL";
    } else {
        $query_condition = "PRO_PROJ_ID = ".$id;
    }
	$sql->query("SELECT proj_id, name, extra_obj_id, notes FROM proj WHERE ".$query_condition." ORDER BY name");
	$projcnt = 0;
	$icon = $this->xRelPath . "images/icon.PROJ.gif";
	
	while ( $sql->ReadRow() ) {
	
		$sub_projid = $sql->RowData[0];
		$name       = $sql->RowData[1];
		//$eobj_id    = $sql->RowData[2];
		//$notes      = $sql->RowData[3];
		
		echo "<a href=\"".$desturl.$sub_projid."\"><img src=\"".$icon."\" border=0> $name</a>";
		echo "<br>\n";
		
		$projcnt++;
	}
}

/**
 * search an object in the projects
  - ignore "bookmarks" projects (!!!)
 * @param object $sql
 * @param int $obj_id
 * @param string $table
 * @param array  $options
 *   'non_cat' = 0,1 : search only for NON-category projects 
 * @return array 
 *   PROJ_ID, 
 *   exists_flag: 
 *     0,
 *     1,  - one exists
 *     2   - more than 1
 */
function searchProject ( &$sql, $obj_id, $table, $options=array() ) {
	
	$exists = 0;
	$where_add = " AND (p.NAME not like 'bookmarks')";
	$sqlsel = "p.PROJ_ID from PROJ p where p.PROJ_ID in (select PROJ_ID from PROJ_HAS_ELEM where TABLE_NAME='".$table."' AND PRIM_KEY=".$obj_id.')' . $where_add;
	
	if ($options['non_cat']>0) {
	    // ignore CATEGORY projects ...
	    $sqlsel = "p.PROJ_ID from PROJ_HAS_ELEM pe join PROJ p on p.PROJ_ID=pe.PROJ_ID ".
	   	    " where pe.TABLE_NAME='".$table."' AND pe.PRIM_KEY=".$obj_id . ' and (p.TYPEX is NULL or p.TYPEX!=1)' . $where_add;
	}
	
	$sql->Quesel($sqlsel);
	if ( $sql->ReadRow() ) {
		$proj_id = $sql->RowData[0];
		$exists    = 1;
		if ($sql->ReadRow()) $exists = 2;
	}
	
	
	return array($proj_id,$exists);
}

/**
 * search all CATEGORY projects of object
 
 * @param object $sqlo
 * @param int    $obj_id
 * @param string $table
 * @return array of proj_ids
 */
function search_cat ( &$sqlo, $obj_id, $table, $options=array() ) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    $max_cats = 30;
    $cnt = 0;
    $proj_ids = array();
    
    $sqlsel = "p.PROJ_ID from PROJ_HAS_ELEM pe join PROJ p on p.PROJ_ID=pe.PROJ_ID ".
        " where pe.TABLE_NAME='".$table."' AND pe.PRIM_KEY=".$obj_id . ' and p.TYPEX=1 order by p.PROJ_ID';
    $sqlo->Quesel($sqlsel);
    while ( $sqlo->ReadRow() ) {
        if ($cnt>$max_cats) {
            $error->set( $FUNCNAME, 1, 'too many categories',1 );
            break;
        }
        $proj_ids[] = $sqlo->RowData[0];
        $cnt++;
    }
    return $proj_ids;
}

}