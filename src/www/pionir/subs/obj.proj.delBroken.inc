<?php
/*MODULE: obj.proj.delBroken.inc
  DESCR:  delete broken links from project
 		  - used in obj.proj.elementact.php
  AUTHOR: qbi
  VERSION: 0.1 - 20050601 	
*/
require_once ("func_formSp.inc");
require_once ("o.PROJ.subs.inc");
require_once ('o.PROJ.addelems.inc');
require_once ("o.PROJ.tree.inc");
require_once ("visufuncs.inc");

class projDelBrokenC {

var $infoLevel;
var $CLASSNAME;

function __construct() {
	$this->CLASSNAME = "projDelBrokenC";
	$this->tabstart  = 0;
	$this->projManiLib = new oProjAddElem($sql);
	// $this->t_rights  = $t_rights;
}

function _anaOneProj( &$sql, &$sql2, $projidX ) {
	// get all broken links of one project
	// RETURN: number of broken links
			
	global $error;
	$FUNCNAME= "_anaOneProj";	
		
	$cnt=0;
	// check access
	$o_rights = access_check($sql, "PROJ", $projidX);
	if ( !$o_rights["insert"] ) {
		$error->set( $FUNCNAME, 1, "No 'insert' access on project $projidX." );
		return;
	}

	$this->projManiLib->setProject( $sql, $projidX );
	$sqls = "select TABLE_NAME, PRIM_KEY from PROJ_HAS_ELEM where PROJ_ID=".$projidX." order by TABLE_NAME, PRIM_KEY ";
	$sql2->query($sqls);
	while ( $sql2->ReadRow() ) {
		$tablename = $sql2->RowData[0];		
		$objid     = $sql2->RowData[1];	
		
		$exists = gObject_exists ($sql, $tablename, $objid); 
		if ($exists) continue;
		
		$cnt++;
		
		if ($this->go==1) {
			$this->projManiLib->unlinkObj( $sql, $tablename, $objid );
		}
	}
	return($cnt);
}

function _doProjLoop(&$sql, &$sql2) {
	global $error;
	$FUNCNAME= "_doProjLoop";	
	
	$projArr = oPROJ_tree::tree2array( $sql, $this->proj_id );
	
	$masterProjName = obj_nice_name ( $sql, "PROJ", $this->proj_id );
	$tmparr = array( $this->proj_id=>$masterProjName );
	$projArr[0] = $tmparr;
	
	$tabobj = new visufuncs();
	$headOpt = array( "title" => "Analysis table",
	 		          "colopt" => array("0"=>"#EFBBEF")
					 );
	$headx  = array ("Project", "Broken links", "Info");
	$tabobj->table_head($headx,   $headOpt);
	
	$projcnt=0; 
	$brokenCnt = 0;    
	       
	
    foreach( $projArr as $projm=>$tmparr) {  
        foreach( $tmparr as $proj2=>$name) { 
			
            $show=0;
			$error_id  = 0;
			$info_txt  = "";
			$numBroken = $this->_anaOneProj( $sql, $sql2, $proj2 );
			if ($error->Got(READONLY))  {
				$errLast   = $error->getLast();
				$error_id  = $errLast->id;
				$info_txt  = "<font color=red>Error:</font>".$errLast->text;
				$error->reset();
			} else {
				$info_txt = "o.k.";
			}
			
			if ($error_id OR $numBroken) $show=1;
			
			if ($show) {
				$dataArr = array($name." [$proj2]", $numBroken, $info_txt );
				$tabobj->table_row ($dataArr);
			}
			
			if ($numBroken) {
				$projBroken++;
				$brokenCnt = $brokenCnt + $numBroken;
			}
			$projcnt++;
        }
    }  
    reset ($projArr);
	$tabobj->table_close();
	
	echo "<br>";
	$tabobj = new visufuncs();
	$dataArr= NULL;
	$dataArr[] = array( 'Observed projects:',$projcnt);
	$dataArr[] = array( 'Projects with broken links:','<B>'.$projBroken.'</B>');
	$dataArr[] = array( 'Broken links:','<B>'.$brokenCnt.'</B>');
	
	$headOpt = array( "title" => "Summary", "headNoShow" =>1);
	$headx   = array ("Key", "Val");
	$tabobj->table_out2($headx, $dataArr,  $headOpt);
}

function projgui( &$sql, &$sql2, $proj_id, $go, $delurl, $opt ) {
	
	$this->proj_id = $proj_id;
	$this->go=$go;
	$params = NULL;
	
	if ($go==0) {
		$formLib = new formSpecialc();
		$params["state"] = $this->go+1;
		$formLib->deleteForm( "Delete broken links from project", "Delete broken links?", $delurl, $params );
	}
	echo "<br>";
	$this->_doProjLoop($sql, $sql2);
}

}