<?php
/**
 * - select special sort of columns
 * - define sort columns and the order
 * - first implementation: 0.1 - 20020904
 * @package glob.objtab.defsort.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param    $tablename
			$sorx[colname]    -- direction of sort ASC,DESC
		    $sorpos[colname]  -- position in sort-cond
 */


session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("subs/f.prefsgui.inc");
require_once ('func_form.inc');
require_once ("sql_query_dyn.inc");
require_once ("view.tmpl.inc");

class objTabDefSort {
	
var $sorpos;	// position of sort-column
	
function __construct($tablename) {
	
	$this->tablename = $tablename;
	$this->colNames  = columns_get_pos($tablename);
	$this->sortcritSQL = query_sort_org( $tablename );
	$this->viewSubLib  = new viewSubC($tablename);
}



function getSortCondCol($colName) {
	// RETURN: list($sortFlag, $sortPos)
	$sortCond = "";
	if ( $this->sortcritX[$colName]!="" ) {
		$sortCond = $this->sortcritX[$colName];
		$sortPos  = $this->sorpos[$colName];
	}
	return array( $sortCond, $sortPos );
}

function getColumnInfo(&$sql) {
	
	$colNames  = &$this->colNames;
	$tablename = $this->tablename;
	$selectCols = NULL;
	
	foreach( $colNames as $dummy=>$colName) {
		
		$showcol    = 1;  
		$colcomment = '';
        $colInfos   = colFeaturesGet($sql, $tablename, $colName, 0); 
        
        if ( $colInfos['VISIBLE']=="0"  ) {
            $showcol = 0; 
        }

        if ( $_SESSION['sec']['appuser'] == 'root' ) $showcol=1; /* all allowed */

        if ( !$showcol ) {
            continue; 
        }
		
		
		
		if ( (strstr($colName,"CCT_ACCESS_ID") != "") ) {
			$selectCols[] =  array( "code"=>"a.CREA_DATE",  "show"=>1,  "nice"=>"creation date", "notes"=>"" );
			$selectCols[] =  array( "code"=>"a.MOD_DATE",   "show"=>1,   "nice"=>"modified date", "notes"=>""  );
			$selectCols[] =  array( "code"=>"a.DB_USER_ID", "show"=>1, "nice"=>"user", 			"notes"=>""  );
			continue;
		}
			
		if ( (strstr($colName,"EXTRA_OBJ_ID") != "") ) {
			$selectCols[] =  array( "code"=>"c.NAME", "show"=>1,  "nice"=>"class", "notes"=>"extra class name" );
			continue;
		}
		
        $colNiceName = $colInfos['NICE_NAME'];
		$colcomment = column_remark2($tablename, $colName);
		
		$selectCols[] =  array( "code"=>"x.".$colName, "show"=>1, "nice"=>$colNiceName,  "notes"=>$colcomment );
		
	} 
	reset ($colNames);  
	  
	$this->selectCols = $selectCols;
}

function _analyseParams($sorx) {
	global $error;
	
	if (is_array($this->sorpos) ) {
		asort( $this->sorpos ); // sort by sort-position
	}
	if ( sizeof($sorx)==1 AND !sizeof($this->sorpos) ) {
		// ONE column set, but no position
		$colname = key( $sorx );
		$this->sorpos[$colname] = 1;
	}
	
	if ( !sizeof($this->sorpos) ) {
		$error->set("saveVars",1, "You must define the sort-position!");
		return;
	}
	
	$sortText = "";
	$tmpkomma = "";
	
	foreach( $this->sorpos as $colname=>$dummy) {
		
		$sortCond = $sorx[$colname];
		if ($sortCond=="ASC" OR $sortCond=="DESC") {
			
			if ( $lensorx>1 AND !$this->sorpos[$colname] )  {
				$error->set("saveVars",1, "When more than ONE column is used, you must define the sort-position!");
				return;
			}
			
			$sortTmpText = $colname." ".$sortCond;
			
			$sortText .= $tmpkomma . $sortTmpText;
			$tmpkomma = ",";
		}
	}
	reset($this->sorpos);
	
	return ($sortText);
}

function saveVars( &$sql, $sorx, $sorpos ) {
	// save sort condition 
			
	$this->sorpos = $sorpos;
	$lensorx  = sizeof($sorx);
	$sortText = "";
	
	if ( $lensorx>0 ) {
		$sortText = $this->_analyseParams($sorx);
	}
	
	query_set_sort($this->tablename, $sortText);
	
	$this->sortcritSQL = query_sort_org( $this->tablename );
}

function showForm(&$sql) {
	
	$tablename = $this->tablename;
	$colNames  = &$this->colNames;
	
	$this->sortcritX = NULL;
	$this->sorpos    = NULL;
	
	$sortArray = $this->viewSubLib->getSortMatrix( $this->sortcritSQL );
	
	if (sizeof($sortArray) ) {
		foreach( $sortArray as $colname=>$colarr) {
			$this->sortcritX[$colname] = $colarr["dir"];
			$this->sorpos[$colname]    = $colarr["pos"];
		}
		reset ( $sortArray );
	}
	
	$this->getColumnInfo($sql);
	echo '<form method="post" action="'.$_SERVER['PHP_SELF'].'">'."\n";
	echo '<input type="hidden" name="go" value="1">'."\n";
	echo '<input type="hidden" name="tablename" value="',$tablename,'">';
	echo "<table bgcolor=#EFEFFF>";
	?>
	<tr bgcolor=#CFCFCF>
	<th class=x1>Nice name</th>
	<th class=x1>Sort</th>
	<th class=x1>Order</th>
	<th class=x1>Notes</th>
	<th class=x1 align=left>Code name</th>
	</tr>
	<?
       
    foreach( $this->selectCols as $dummy=>$colarr) {
      
		echo "<tr>";
		echo "<td  align=right>".$colarr["nice"]."</td>";
		echo "<td>";
		
		list($sortFlag, $sortPos)  = $this->getSortCondCol( $colarr["code"] );
		
		$sortTmpArr = array("ASC"=>"ascending", "DESC"=>"descending");
		$selOption=array( "selecttext"=>"---");
		$seltext = formc::selectFget( "sorx[".$colarr["code"]."]", $sortTmpArr, $sortFlag, $selOption ); 	
		echo $seltext;
		echo "</td>";
		echo "<td>";
		$sortTmpArr = array( "1"=>"1", "2"=>"2","3"=>"3" );
		$selOption=array( "selecttext"=>"---");
		$seltext = formc::selectFget( "sorpos[".$colarr["code"]."]", $sortTmpArr, $sortPos, $selOption );
		echo $seltext;
		echo "</td>";
		echo "<td>".$colarr["notes"]."</td>";
		echo "<td>".$colarr["code"]."</td>";
        echo "</tr>";
        
    }
	
	echo "<tr bgcolor=#D0D0FF>";
	echo "<td  align=right></td>";
	echo "<td>";
	echo "<input type=submit value=\"Submit\">\n";
	echo "</td>";
	echo "<td colspan=3></td>";
	echo "</tr>";
		
     echo "</table>";
	 
	 
	 echo "</form>";
     reset ($this->selectCols);
}

}

// --------------------------------------------------- 
global $error, $varcol;

$tablename=$_REQUEST['tablename'];
$go=$_REQUEST['go'];
$sorx=$_REQUEST['sorx'];
$sorpos=$_REQUEST['sorpos'];

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
//$sql2  = logon2( );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

//$i_tableNiceName 	 = tablename_nice2($tablename);

$title       		 = "List view preferences: define sort-conditions";
$titleShort       	 = "Sort-conditions";
$infoarr			 = NULL;

$infoarr["title"] = $title;
$infoarr["title_sh"] = $titleShort;
$infoarr["form_type"]= "tool";

$infoarr["obj_name"] = $tablename;
$infoarr['help_url'] = 'g.listViewPrefs.html';
$infoarr["icon"]     = "images/ic24.userprefs.png";
$infoarr["locrow"]   = array(
			array("obj.db_user.settings.php", "Main preferences"),
			array("glob.objtab.pref.php?tablename=".$tablename, "List view preferences"),
			// array("", $titleShort )
			);
			

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

$prefsGuiLib = new fPrefsGuiC();		
$prefsGuiLib->showHeadTabs( 'LIST', $tablename );

echo "<ul>";



if ( $tablename=="" ) {
    htmlFoot("Error", "table name missing [<a href=\"obj.db_user.settings.php\">Back main preferences</a>]");
}
$mainLib = new objTabDefSort($tablename);

if ($go>0) {
	$mainLib->saveVars($sql, $sorx, $sorpos);
	if ($error->printAll()) echo "<br><br>\n";
} 

$tmpText = "<b>".$mainLib->sortcritSQL."</b>";
if ($mainLib->sortcritSQL=="")  $tmpText = "<font color=gray>None</font>";
echo "Current sort condition: ".$tmpText." &nbsp;&nbsp;&nbsp;\n";
if ( $mainLib->sortcritSQL !="" ) echo "[<a href=\"".$_SERVER['PHP_SELF']."?go=1\"><b>Clear all sort conditions</B></a>]\n";
echo " <br><br>\n";

$mainLib->showForm($sql);

htmlFoot();
