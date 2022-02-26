<?php
/*MODULE: f.visu_list.inc 
  DESCR:  function for visualization of list-scripts
  AUTHOR: qbi
  VERSION: 0.1 - 20040904
*/
require_once("sql_query_dyn.inc");

class visu_listC {

var $maxNumSelected;

function __construct() {
	$this->maxNumSelected = 1000; // default
}

function selTable( &$sql, $formurl, 
	$option=NULL //  
				 // "type" == ""    => only BO
				 //		   == "all" => all tables
				 // "showPure" => show also pure name
	) {
	
	$optwhere = "where TABLE_TYPE='BO'";
	if ( $option["type"]=="all") $optwhere = "";
	echo "<form method=\"post\"  name=\"editform\"  action=\"".$formurl."\" >\n"; 
	echo '<table border="0" bgcolor=#EFEFEF>'."\n";
	echo '<tr>'."\n";
	echo '<td align="right">'."\n";
	echo '<select name="tablename">'."\n";
	echo '<option value="---">--- select table ---'."</option>\n";
	
	$retVal = $sql->query('SELECT table_name, nice_name FROM cct_table '.$optwhere.' ORDER BY nice_name');
	while ( $sql->ReadRow() ) {
		  $tabname  = $sql->RowData[0];
		  $nicename = $sql->RowData[1]; 
		  if ($nicename=="")  $nicename = "[$tabname]";
		  echo '<option value="'.$tabname.'">'.$nicename."</option>\n";
	}
	
	echo "</select>\n";
	echo "<input type=submit value=\"Submit\">\n"; 
	
	echo '</td></tr></table>'."\n";
	echo '</form>'."\n";
}

/**
 *  check, if elements where selected
				- set a default max number of selected elements
	INPUT: $_SESSION['s_formState']["f.visu_list"] = array( $_SERVER['PHP_SELF'] => 0,1 )
 * @param object $sql
 * @param string $tablename
 * @param array $options
 * "checkNumMax" :  0 : no check 
					>0 : a specific number to check
					-1 : for DEFAULT number
				"elemNum" :		give cached NUMBER of elements (otherwise use "doCount"-flag)
				"doCount"		[0] | 1 : do an SQL-count (otherwise get from "elemNum")
	@return array  array( $flag, $text, $cnt)
			$flag >= 0 : ok
 */
function checkSelection( object &$sql, string$tablename, $options) {

	
	$obj_cnt = $options["elemNum"];
	$retarr  = array(0,"",$obj_cnt );
	
	do {
		// 1. check for "info"
		$tmp_info   = $_SESSION['s_tabSearchCond'][$tablename]["info"];
		if ($tmp_info=="") {
			$retarr = array( -1, "No elements selected (selection info is empty).");
			break;
		}
		
		if ($options["doCount"]>0) { // do a real count
			$sqlAfter  = get_selection_as_sql( $tablename );
			$sql->query("SELECT count(1) FROM ".$sqlAfter);
			$sql->ReadRow();
			$obj_cnt   = $sql->RowData[0];
			$retarr[2] = $obj_cnt; // update
		}
		
		// 2. check for real count
		if ($obj_cnt <= 0) {
			$retarr = array( -2, "No elements selected.");
			break;
		}
		
		if ( $options["checkNumMax"]>0 ) {
			if ($obj_cnt > $options["checkNumMax"]) {
				if ( !$_SESSION['s_formState']["f.visu_list"][$_SERVER['PHP_SELF']] ) {
					$addtext = "Option: <a href=\"".$_SESSION['s_sessVars']["loginURL"]."/f.s_formState.set.php?key=f.visu_list&subkey=".$_SERVER['PHP_SELF']."&val=1\">".
							"<b>==&gt; Ignore</b></a> element-maximum during this session.<br>";
					$retarr = array( -3, "Too many elements (".$obj_cnt.") selected. Expected maximum: ".$options["checkNumMax"].".<br>".
								   $addtext);
					break;
				}
				
				
			}
		} 
		
	} while (0);
	
	return ($retarr);
}

function exitByTest( 
	&$sqlo, 
	$tablename, 
	$exopt 	    // optional "elemNum"
	) {
	// FUNCTION: - do checkSelection() test
	//			 - EXIT, if test failed
	// check TABLE selection
			
	$copt = array ("elemNum" => $exopt["elemNum"] ); // prevent double SQL counting
	list ($stopFlag, $stopReason)= $this->checkSelection( $sqlo, $tablename, $copt );
	if ( $stopFlag<0 ) {
		$tablenice = tablename_nice2($tablename);
		htmlFoot("Attention", $stopReason." Please select elements of '".$tablenice."'!");
	}
}

}
