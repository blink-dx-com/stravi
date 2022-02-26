<?php
/**
 * show DB table overview
 * @package glob.tabsel.php
 * @swreq UREQ:xxxxxxxxxxxx
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $parx["action"] = "normal", "detail"
		  $parx["type"]   = ["ALL"], "BO"
		  $parx["order"] = ["CODE"], "NICE"
 * @version $Header:  Exp $
 */

session_start(); 


require_once ('db_access.inc');
require_once ('globals.inc');
require_once ('func_head.inc');
require_once ('access_check.inc');
require_once ('table_access.inc'); 
require_once ("subs/toolbar.inc");
require_once 'subs/glob.obj.table_overview.inc';
require_once ("f.rider.inc");

class showTablesC {

    function __construct($parx) {
    	if ( $parx["order"]=="") $parx["order"]="CODE";
    	$this->parx = $parx;
    }
    
    function link1($table) {
    	$icon      = file_exists("images/icon.".$table.".gif") ? 
    					"images//icon.".$table.".gif":
    					"0.gif";
    	echo "<tr>".
    	"<td align=right>".tablename_nice2($table)."</td>".
    	"<td>&nbsp;<img src=\"".$icon."\">&nbsp;</td>".
    	"<td><a href=\"view.tmpl.php?t=".$table."\">".$table."</a></td>".
    	"</tr>\n";
    }
    
    function link2($table) {
    	$icon      = file_exists("images/icon.".$table.".gif") ? 
    					"images//icon.".$table.".gif":
    					"0.gif";
    	echo "<tr>".
    	"<td align=right>".$table."</td>".
    	"<td>&nbsp;<img src=\"".$icon."\">&nbsp;</td>".
    	"<td><a href=\"view.tmpl.php?t=".$table."\">".tablename_nice2($table)."</a></td>".
    	"</tr>\n";
    }
    
    function showNorm(&$sql) {
    
    	$colst1 = "";
    	$colst2 = "";
    	
    	$ordcol = "TABLE_NAME";
    	$colst1 = " font-weight:bold;";
    	if ( $this->parx["order"]=="NICE") {
    		$ordcol = "NICE_NAME";
    		$colst2 = " font-weight:bold;";
    	}
    	
    	echo "<br>";
    	echo "<table cellpadding=0 cellspacing=0 border=0>\n";
    	echo "<tr><td  valign=top>\n";
    	
    	echo "<table cellpadding=0 cellspacing=0 border=0>\n";
    	echo "<tr style=\"text-align:center; background-color:#EFEFFF;\">".
    		"<td><font style=\"".$colst2."\"><a href=\"".$_SERVER['PHP_SELF']."?parx[order]=NICE\">NiceName</a></td>".
    		"<td>&nbsp;</td>".
    		"<td><font style=\"".$colst1."\"><a href=\"".$_SERVER['PHP_SELF']."?parx[order]=CODE\">CodeName</a></td>".
    		
    		"<tr>\n";
    	$retVal = $sql->query('SELECT table_name FROM cct_table ORDER BY '.$ordcol);
    	while ( $sql->ReadRow() ) {
    		$tabname = $sql->RowData[0]; 
    		$this->link1($tabname);
    	}
    	
    	echo "</table>\n";
    	echo "</td>";
    	
    	 $showzwei = 0;
    	if ( $showzwei ) {
    		echo "<td>&nbsp;</td><td valign=top>\n";
    		echo "<table cellpadding=0 cellspacing=0 border=0>\n";
    		echo "<tr><td colspan=3 style=\"text-align:center; background-color:#EFEFFF;\"><b>NICE name ordered</b></td><tr>\n";
    		$retVal = $sql->query('SELECT table_name FROM cct_table ORDER BY nice_name');
    		while ( $sql->ReadRow() ) {
    			$tabname = $sql->RowData[0]; 
    			$this->link2($tabname);
    		}
    		echo "</table>\n";
    		echo "</td>";
    	}
    	echo "</tr>\n";
    	echo "</table>\n";
    }
    
    

}


$parx=$_REQUEST['parx'];
$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot(); 
$varcol = & Varcols::get(); 


$title       = 'Select a table';

#$infoarr['help_url'] = 'o.EXAMPLE.htm';

$infoarr["title"] = $title;
$infoarr["form_type"]= "tool";

$infoarr["locrow"]= array( array('home.php', 'home' ) );

if ($parx["action"]=="") $parx["action"] = "normal";

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
gHtmlMisc::func_hist("glob.tabsel", $title, $_SERVER['PHP_SELF'] );

echo '<ul>'."\n";

$xmodes=array(
	"normal"=>array("overview", $PHPSELF."?parx[action]=normal"),
	"detail"=>array("details", $PHPSELF."?parx[action]=detail"),
	);
$riderObj = new fRiderC();
$riderObj->riderShow( $xmodes, $parx["action"], "Select Mode");
echo "<br>";


$mainLib = new showTablesC($parx);

if ($parx["action"]=="normal") {
	$mainLib->showNorm($sql);
} else {
    
    $helpLib = new glob_obj_table_overview();
    $helpLib->showDet($sql, $sql2);
    
}

echo '</ul>'."\n";

htmlFoot("<hr>");

 