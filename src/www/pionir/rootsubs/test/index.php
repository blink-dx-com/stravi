 <?php
/*MODULE: index.php  ( in test/ )
  DESCR:  test functions
*/

session_start(); 


require_once ('db_access.inc');
require_once ('globals.inc');
require_once ("func_head.inc");     

$sqlo  = logon2( $_SERVER['PHP_SELF'], '../../' ); 
 

$title = 'Test scripts / designs';

$infoarr			 = NULL;
$infoarr["scriptID"] = "";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "tool"; 
$infoarr["locrow"] = array( array("../rootFuncs.php", "Administration") );

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr); 

if ( !$_SESSION['s_suflag'] && ($_SESSION['sec']['appuser']!="root") ) {
     htmlFoot("Error", "Sorry, you must be root or have su_flag.");
}   

?>
<ul> 
<li><a href="../../p.php?mod=DEF/root/test/formtest2">Form test</a> (old)</li> 
<li><a href="../../p.php?mod=DEF/root/test/formtest&action=multiselect">Other form tests</a></li> 
<li><a href="test.naviElements.php">Navigation elements</a> (buttons, Icons, Info-Boxes, ...)</li>
<li><a href="test.diagram.php">Diagrams 1</a> (OLD: Line, Scatter plot, Bragraph)</li>
<li><a href="../../p.php?mod=DEF/root/test/chart_test">Diagrams 2</a> (Chart.js)</li> 
<li><a href="../../p.php?mod=DEF/root/g.html_utils">HTML utilities</a> </li>
<li><a href="../../p.php?mod=DEF/zzz_example">PLUGIN design test</a> </li>
<li><a href="test.appserver.php">Application server test</a> (also without login)</li>
<li><a href="test.fpdf.php">FPDF-Report-Generator</a></li>
<li><a href="../../p.php?mod=LAB/x_test_bootstrap">HTML Bootstrap test</a></li> 
</ul>
<?

htmlFoot("<hr>");
