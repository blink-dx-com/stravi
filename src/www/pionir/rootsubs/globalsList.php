<?php
/*MODULE:  globalsList.php
  DESCR:   show internal PHP-variables
  AUTHOR:  qbi
  INPUT:   -
  VERSION: 0.1 - 20020904
*/

extract($_REQUEST); 
session_start(); 

require_once ("db_access.inc");
require_once ("globals.inc");
require_once ("func_head.inc");
require_once ("visufuncs.inc");

class gRootGlobalsListC {

function showResultData( 
	$title, 
	&$tmparr // can be array of 2 or 3 elements
	 ) {
    echo "&nbsp;&nbsp;&nbsp;<B>$title</B>";
	
	if (!sizeof($tmparr) ) {
		echo " <font color=gray>No data</font>.<br>";
		return;
	}
	
	echo "<ul>\n";
	
    echo "<table cellspacing=1 cellpadding=1 bgcolor=#E0E0E0>";  
    $color1 = "#EFEFEF";   
    $color2 = "#EFEFFF";    
    $color  = $color1;
	
	
	reset($tmparr);
    foreach( $tmparr as $key=>$valx) {
        
        if ($color == $color1)  $color = $color2;
        else $color = $color1;       
        echo "<tr bgcolor=\"".$color."\">";
		
		echo "<td>".$key."</td>";
		if ( is_array($valx) ) {
		
			echo "<td>";
			reset($valx);
			$tmpstr = glob_array2String( $valx );
			echo $tmpstr;
			echo "</td>";
			
		} else 	echo "<td>".htmlspecialchars($valx)."</td>";
		echo "</tr>\n";
    }
	reset($tmparr);
    echo "</table></ul>\n";
}

function s_tabSearchCondShow () {
	
	
	echo "&nbsp;&nbsp;&nbsp;<B>s_tabSearchCond</B> (current table conditions)<UL>";
	echo "<table cellpadding=1 cellspacing=1 border=0 bgcolor=#B0B0B0>";
	echo "<tr bgcolor=#D0D0D0><td><b>table</b></td><td><b>from_clause</b></td><td><b>where_clause</b></td>".
		 "<td><b>extra_where</b></td><td><b>class name</b></td>".
		 "<td><b>user info</b></td> <td> mother ID </td><td>xSQL</td><td>Arch</td> </tr>\n";
	$color1 = "#EFEFEF";   
	$color2 = "#EFEFFF";
			
	foreach(  $_SESSION['s_tabSearchCond'] as $th0=>$th1) { 
	
		$tablen= $th0;
		$tab = $th1["f"];
		$id0 = htmlspecialchars($th1["w"]);
		$id1 = $th1["x"];
		$id2 = $th1["c"];
		$info= $th1["info"];
		$mothid = $th1["mothid"];
		$xSql= $th1["y"];
		$arch= $th1["arch"];
		if ($color == $color1)  $color = $color2;
		else $color = $color1;
	
		echo "<tr bgcolor=\"".$color."\"><td>$tablen</td><td>$tab</td><td>$id0</td><td>$id1</td><td>$id2</td>".
			 "<td> $info </td><td>$mothid</td><td>$xSql</td><td>$arch</td></tr>\n";
	}
	

	
	echo "</table>";
	echo "</UL>";
}

function historyShow() {
	
	
	echo "<p>s_history<UL>";
	$i=0;
	$inftab=NULL;
	foreach( $_SESSION['s_history'] as $hi=>$th) {
		
		$tab  = key($th);
		$id   = current($th);
		$isbo = $th['last'];
		$count   = $th['c'];
		$touchord= $th['o'];
		$inftab[] = array($hi, $tab, $id, $isbo, $count, $touchord,  $th['s']);
		// echo "$i : $tab : $id bo?:$isbo c:$count<br>";
		$i++;
	}
	reset($_SESSION['s_history']);
	$header = array("Hist<br>index", "Tablename", "ID", "last", "touch<br>cnt", "touch<br>order", "show<br>order"); 
	visufuncs::table_out( 
			$header,  // can be NULL
			$inftab  // array of values or just the value
			
		);
		
	echo "</UL>";
}


}

// ********************************************************

global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] ); // give the URL-link for the first db-login
if ($error->printLast()) htmlFoot();

$title = "List of system variables";

$infoarr=NULL;
$infoarr["scriptID"] = "globalsList.php";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "tool"; // "tool", "list"
$infoarr["locrow"]   = array(array("rootFuncs.php", "Administration"));
$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sql, $infoarr);


$globInfoLib = new gRootGlobalsListC();

echo "<br>";

// SECURE 
$bad_keys=array("passwd");
$tmparr  =array();
foreach($_SESSION['sec'] as $key=>$val) {
    if (in_array($key,$bad_keys)) {
        continue;
    }
    $tmparr[$key] = $val;
}
$tmparr["SessionID"]=session_id();


$title= "SECURE (sec)";
$globInfoLib->showResultData( $title, $tmparr);


$title= "GLOBALS";
$globInfoLib->showResultData( $title, $_SESSION['globals'] );

$globInfoLib->s_tabSearchCondShow();

$title="s_sessVars";
$globInfoLib->showResultData($title,$_SESSION['s_sessVars']);

$title= "s_formState";
$globInfoLib->showResultData( $title, $_SESSION['s_formState'] );

$title= "s_clipboard";
$globInfoLib->showResultData( $title, $_SESSION['s_clipboard'] );

$title="s_product";
$globInfoLib->showResultData( $title, $_SESSION['s_product'] );
echo "</UL>"; 


$globInfoLib->historyShow();

$title= "s_historyL : History of table-lists";
$globInfoLib->showResultData( $title, $_SESSION['s_historyL'] );


$title= "s_funcclip : Function clipboard";
$globInfoLib->showResultData( $title, $_SESSION['s_funcclip'] );

$title= "s_objCache";
$globInfoLib->showResultData( $title, $_SESSION['s_objCache'] );

$title= "s_tree";
$globInfoLib->showResultData( $title, $_SESSION['s_tree'] );

$sql = logon2( $_SERVER['PHP_SELF'] );
$db_type_tmp= $sql->_ident();
echo "Interface DB_TYPE:".$db_type_tmp."<br>";

//$title="Browser info";
//$tmpBrowser = get_browser();
//$globInfoLib->showResultData( $title, $tmpBrowser );

htmlFoot("<hr>");

