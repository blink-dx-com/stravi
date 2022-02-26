<?php
/**
 * - manage the right help file
   - can be in the pionir of lab-directory
 * @package f.help.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $f : help-file ( based on help/robo )
  		  [$base] = ["auto"], "pionir", "lab"
  		  [type]  = 
  		  	["norm"],  normal help page
  		  	'vario'  : special help for vario attribute, 
  		  	           need $t (table) and $f contains the vario-code-name
  		  [t]     = table; e.g. for vario
 * @version0 2002-09-04
 */
session_start();
require_once ('reqnormal.inc');
require_once ("f.help.inc");

function _htmlHead(&$sqlo) {
	$title       		 = "Help file manager";
	$infoarr			 = NULL;
	$infoarr["scriptID"] = "f.help.php";
	$infoarr["title"]    = $title;
	$infoarr["form_type"]= "tool"; // "tool", "list"

	$pagelib = new gHtmlHead();
	$pagelib->startPage($sqlo, $infoarr);

	echo "<ul>";
}

function _htmlErr(&$sqlo, $errtext) {
	_htmlHead($sqlo);
	htmlFoot("Error", $errtext);
	exit;
}
 

$sqlo  = logon2( $_SERVER['PHP_SELF'] );

$f 		= $_REQUEST['f'];
$base   = $_REQUEST['base'];
$type   = $_REQUEST['type'];
$t      = $_REQUEST['t'];

if ($f=="") {
	_htmlErr($sqlo, "No help-file given.");
} 

$infox=NULL;

if ($type=='vario') {
	if ($t=="") {
		_htmlErr($sqlo, "Table as param missing.");
	} 
	$varioName = $f;
	_htmlHead($sqlo);
	
	require_once 'help/o.S_VARIO.vars.php';
	
	$labFile = $_SESSION['s_sessVars']['AppLabLibDir'].'/help/o.S_VARIO.vars.php';
	if (file_exists($labFile)) {
		require_once $labFile;
	}
	
	echo "</table>";
	echo "</body></html>";
	
	// jump to HTML-tag $t:$varioName
	// example: "CONCRETE_SUBST:expiryDate"
	?>
	<script language="JavaScript">
		location = "#<?php echo $t.':'.$varioName?>";            
	</script>
	<? 	
	exit;
	
} else {
	$helpLib = new fHelpC();
	$baseflag = "auto"; //  // ["auto"], "lab", "pionir"
	if ($base!="")  $baseflag = $base;
	$urlpath = $helpLib->htmlCheckHelp( $f, $baseflag );
	
	if ($urlpath=="") { 
		_htmlErr($sqlo, "Help-file '".$f."' not found on system!");
	}
}

echo "<html>\n";

if ( $_SESSION['userGlob']["g.debugLevel"]>0 ) {
	echo "... automatic page forward stopped due to debug-level.<br>";
	echo "[<a href=\"".$urlpath."\">next page &gt;&gt;</a>]<br>";
	exit;
}	
 
?>
<script language="JavaScript">
	location.replace("<?php echo $urlpath?>");            
</script>
