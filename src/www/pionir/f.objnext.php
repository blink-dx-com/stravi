<?php
/**
 * Jump to the next object in list
 * @package f.objnext.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $t ($tablename)
  		   $id
		   $dir : -1 prev
		   		   1 next
 * @version0 2002-09-04
 */
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once("sql_query_dyn.inc");

class fObjNextC {

function __construct( &$sql, $tablename, $id, $dir)  {
	$this->tablename=$tablename;
	$this->id =$id;
	$this->dirx=$dir;
	$this->cnt = 0;
	$this->pkName = PrimNameGet2($tablename);
}

function setPage($title, $infoarr) {
	$this->title = $title;
	$this->infoarr=$infoarr;
}

function showHead(&$sql) {
	$this->pagelib = new gHtmlHead();
	$this->pagelib->_PageHead ( $this->title,  $this->infoarr );
	
}

function showHtmlBody(&$sql) {
	$this->pagelib->_startBody( $sql, $this->infoarr );
	echo "<ul>";
	
	echo "Selected objects: <a href=\"view.tmpl.php?t=".$this->tablename."\"><b>".$this->cnt."</b> <img src=\"images/but.list2.gif\" border=0 title=\"list view\"></a><br>\n";
	echo "Selection-info: <b>".htmlSpecialchars($this->selectInfo)."</b><br>\n";
	echo "<br>";
}


function _checkSelection(&$sql) {
	global $error;
	$FUNCNAME= "_checkSelection";
	
	

	$this->selectInfo = query_get_info($this->tablename);
	if ($this->selectInfo=="") {
		$error->set( $FUNCNAME, 2, "No selection active." );
		return;
	}
	
	$sqlopt["order"] = 1;
	$this->sqlAfter  = get_selection_as_sql( $this->tablename, $sqlopt);
	$this->sqlAfterNoSort  = get_selection_as_sql( $this->tablename);
	
	$sqls = "SELECT count(1) FROM ".$this->sqlAfterNoSort;
	$sql->query($sqls);
	$sql->ReadRow();
	$this->cnt = $sql->RowData[0];
	
	if (!$this->cnt) {
		$error->set( $FUNCNAME, 3, "No objects in selection." );
		return;
	}
	
}

function _getNextObject( &$sql ) {
	global $error;
	$FUNCNAME= "_getNextObject";
	
	$objid = $this->id;
	$sqlsLoop = "SELECT x.".$this->pkName." FROM ".$this->sqlAfter;
	$sql->query($sqlsLoop);
	
	$lastID = NULL;
	$info   = NULL;
	$outID  = NULL;
	$found  = 0;
	
	while ( $sql->ReadRow() ) {
		$nowID = $sql->RowData[0];
		if ($nowID==$objid) {
			$outID = $objid;
			$found = 1;
			
			if ($this->dirx < 0) {
				$outID = $lastID;
				if ($outID==NULL) {
					$info = "First object reached in selection.";
				} 
				break;
			}
			
			if ($this->dirx>0) {
				if ( $sql->ReadRow() ) {
					$outID = $sql->RowData[0];
				} else {
					$outID = NULL;
					if ($outID==NULL) {
						$info = "Last object reached in selection.";
					} 
				}
				break;
			}
			
			break;
		}
		$lastID = $nowID;
	}
	
	if (!$found) {
		$info = "Object not found in current selection.";
	}
	
	return array($outID, $info);
}

function orgNext(&$sql) {
	global $error;
	$FUNCNAME= "orgNext";
	
	$object_is_bo = cct_access_has2($this->tablename);
	if (!$object_is_bo) {
		$error->set( $FUNCNAME, 1, "Only business objects supported by this function." );
		return;
	}
	
	
	if ( !gObject_exists($sql, $this->tablename, $this->id) ) {
		$error->set( $FUNCNAME, 1, "Object table: ".$this->tablename." ID: ".$this->id." does not exist." );
		return;
	}
	
	$this->_checkSelection($sql);
	if ($error->Got(READONLY))  {
		return;
	}
	
	list($nextID, $info) = $this->_getNextObject($sql);
	
	if ($nextID==NULL)  {
		$this->showHtmlBody($sql);
		htmlInfoBox( "No next object", $info, "", "INFO" );
		return;
	} 
	
	return ($nextID);
}

}

// --------------------------------------------------- 
global $error;
$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
if ($error->printLast()) htmlFoot();


$id = $_REQUEST["id"];
$tablename=$_REQUEST['t'];
$dir=$_REQUEST['dir'];

$title       		 = "Single-Object-Form : jump to NEXT object in selection";

$infoarr			 = NULL;
$infoarr["scriptID"] = "";
$infoarr["title"]    = $title;
$infoarr["title_sh"] = "jump to NEXT object";
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;

if ($tablename==NULL) {
	htmlFoot("ERROR", "No tablename given.");
}

$mainLib = new fObjNextC($sql, $tablename, $id, $dir);
$mainLib->setPage($title, $infoarr);
$mainLib->showHead($sql);

$nextID = $mainLib->orgNext($sql);

if ($error->Got(READONLY))  {
     $mainLib->showHtmlBody($sql);
     $error->printAllEasy();
	 htmlFoot();
}

if ( $nextID==NULL ) {
	htmlFoot();
}

$newurl = "edit.tmpl.php?t=".$tablename."&id=".$nextID;
?>
<script>
	location.replace("<?php echo $newurl?>");            
</script>
<? 

