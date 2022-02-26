<?
/*MODULE: tableContents.php
  DESCR: Counts the contents of all tables
  AUTHOR: mac, qbi
  INPUT: $go
  		 $parx["sort"]
         $parx["type"]  ["all"], "BO", ....
		 $parx["tabTemp"] "" | 1 : temporary tables
  DB_MODIFIED: none
*/
extract($_REQUEST); 
session_start(); 


require_once ('db_access.inc');
require_once ('globals.inc');
require_once ('func_head.inc');
require_once ('access_check.inc');
require_once ('table_access.inc');
require_once ('func_form.inc');  

class fTableContentGui {

function __construct( &$parx ) {
	 $this->parx = $parx;
	 // temporary tables
	$this->tabsraw = array( 
		"T_EXPORT"	   =>array("c"=>"used for paxml-export/inport"),
	 	"W_WAFER_STEP2"=>array("c"=>"used by burlean-tool (jens)"), 
		"USER_TYPES"   =>array("c"=>"used by BulkInsert()") );
}

function form1( &$sql ) {
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Show content"; 
	$initarr["submittitle"] = "Show"; 
	$initarr["tabwidth"]    = "AUTO"; 
	
	
	$hiddenarr = NULL;
	$formobj = new formc($initarr, $hiddenarr, 0); 
	
	$sqls = "select distinct(TABLE_TYPE) from CCT_TABLE order by TABLE_TYPE";
	$sql->query($sqls);
	$typearr=NULL;
	$typearr["all"] = "--- all ---"; 
	while ( $sql->ReadRow() ) {
		$typearr[$sql->RowData[0]]    = $sql->RowData[0];                    
	}
	
	$fieldx = array ("title" => "Show type", "name"  => "type", "inits" => $typearr, 
						"val"   => $this->parx["type"], "notes" => "show type of table", "object" => "select" );
	$formobj->fieldOut( $fieldx );
	
	$tmpinit = array("name" => "name", "object_type" => "object_type");
	$fieldx = array ("title" => "Sort by", "name"  => "sort", "inits" => $tmpinit, 
						"val"   => $this->parx["sort"], "notes" => "sort by", "object" => "select" );        
	$formobj->fieldOut( $fieldx );
		
	$formobj->close( TRUE );  
}

function _colOut( $valarr ) {

	$tab = $valarr[0];
	$type= $valarr[1];
	$thiscnt = $valarr[2];
	$info = $valarr[3];
	
	echo '<tr bgcolor=#FFFFFF><td><a href="../view.tmpl.php?t=',$tab,'">',strtolower($tab),'</a></td>';
	echo "<td>".$type."</td>";
	echo '<td>',(($thiscnt != 0) ? $thiscnt : ''),'</td>';
	echo '<td>',$info,'</td>';
	echo '<td>',$valarr[4],'</td>';
	echo "</tr>\n";
}

function tabOut( &$sql, &$sql2 ) {

	$parx = $this->parx;

	echo "<table cellpadding=1 cellspacing=1 border=0 bgcolor=#B0B0B0><tr><td>\n";
	echo "<table cellpadding=2 cellspacing=2 border=0 bgcolor=#EFEFEF>".
		"<tr><th>table</th><th>type</th><th>datasets</th><th>info</th><th>comment</th></tr>";
	
	/* reset ($_s_i_table);
	foreach( $_s_i_table as $tab=>$dummy) {
	if (table_is_view($tab)) continue;
	if ($tab == tablename_nice2($tab)) continue; 
	*/
	$where_sql = "";  
	$order_sql = "TABLE_NAME ASC";
	if ($parx["sort"] == "object_type") $order_sql = "TABLE_TYPE ASC, ".$order_sql;   
	
	if ( $parx["type"]!="" AND $parx["type"]!="all" ) {
		$where_sql = "where TABLE_TYPE='".$parx["type"]."'";
	}
	
	$cntSum = 0;
	
	if (!$parx["tabTemp"]) {
	
		$sqls = "select TABLE_NAME, TABLE_TYPE, IS_VIEW from CCT_TABLE ".$where_sql." order by ". $order_sql;
		$sql2->query($sqls);
		while ( $sql2->ReadRow() ) {
			$tab    = $sql2->RowData[0];
			$type   = $sql2->RowData[1]; 
			$isview = $sql2->RowData[2];
			if ($isview) continue;
			
			$sql->query('SELECT COUNT(1) FROM '.$tab);
			$sql->ReadRow();
			$thiscnt = $sql->RowData[0];
			echo '<tr bgcolor=#FFFFFF><td><a href="../view.tmpl.php?t=',$tab,'">',strtolower($tab),'</a></td>';
			echo "<td>".$type."</td>";
			echo '<td>',(($thiscnt != 0) ? $thiscnt : ''),'</td>';
			
			echo "</tr>\n";
			$cntSum += $thiscnt;
		}
		
	} else {
		// SHOW RAW tables
		foreach( $this->tabsraw as $xtab=>$infarr) {
			$tabinfo = "";
			$thiscnt = 0;
			do {
				$type = "intern";
				$sqls = "select NAME from CCT_TAB_VIEW where NAME='$xtab'";
				$sql2->query($sqls);
				if ( $sql2->ReadRow() ) {
					$tabinfo = "";
					$sqls = "select count(1) from $xtab";
					$sql2->query($sqls);
					$sql2->ReadRow();
					$thiscnt = $sql2->RowData[0];
					break;
				} 
				
				// try anyway
				$type = "SYSTEM-table";
				$sqls = "select count(1) from $xtab";
				$sql2->query($sqls);
				$sql2->ReadRow();
				$thiscnt = $sql2->RowData[0];
					
				
			} while (0);
			
			$sqls = "SELECT COMMENTS FROM ALL_TAB_COMMENTS where  TABLE_NAME = '".$xtab."'";
			$sql2->query($sqls);
			$sql2->ReadRow();
			$comment = $sql2->RowData[0];
			
			if ( $infarr["c"]!="" ) $comment .= "; ".$infarr["c"];
			
			$valarr = array( $xtab, $type, $thiscnt, $tabinfo, $comment  );
			$this->_colOut( $valarr );
			$cntSum += $thiscnt;
		}
		reset ($this->tabsraw);
	}
	
	echo '<tr bgcolor=#d0d0FF><td>SUM</td>';
	echo "<td></td>";
	echo '<td><b>'.$cntSum.'</b></td>';
	echo "</tr>\n";
	
	echo '</table>';
	echo '</td></tr></table>';
}

}

// ---------------------------------------------

$error = & ErrorHandler::get();
$sql   = & logon2( $_SERVER['PHP_SELF'] );
$sql2  = & logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();


$title = 'Count table contents';


$infoarr["title"] = $title;
$infoarr["form_type"]= "tool";

$infoarr["locrow"]= array( array('rootFuncs.php', 'root' ) );

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>\n"; 

if ($_SESSION['sec']['appuser'] != 'root' AND !$_SESSION['s_suflag'] ) htmlfoot('Sorry', 'You must be root or super-user');  

if ($parx["type"]=="") $parx["type"]="all";

$mainLib = new fTableContentGui($parx);



$mainLib->form1( $sql );


echo "[<a href=\"".$_SERVER['PHP_SELF']."?parx[tabTemp]=1&go=1\">Show Temporary tables</a>]<br><br>";
    
if (!$go) htmlfoot();

$mainLib->tabOut( $sql, $sql2 );

htmlFoot();