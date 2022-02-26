<?php
/**
 * test for edit.tmpl objlink
 * $Header: trunk/src/www/_tests/www/test/edit.tmpl.objlink.php 59 2018-11-21 09:04:09Z $
 * @package edit.tmpl.objlink
 * @author  qbi
 * @version 1.0
 */

extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
class clEditTmplTest {

function form1() {
	echo '<form name=editform ACTION="'.$_SERVER['PHP_SELF'].'" METHOD=POST>'."\n";
	
	$jsFormLib = new gJS_edit();
	$fopt = array("urlfull" =>1);
	
	echo "test column1: ";
	$this_colval = 5;
	$button_value = "--test1--";
	$viewcnt = 0;
	$answer = $jsFormLib->getAll('EXP_TMPL', 'exp_tmpl_id', $this_colval, $button_value,  $viewcnt, $fopt);
	echo $answer;
	echo "<br>";
	
	echo "test column2: ";
	$this_colval = 2;
	$button_value = "--test2--";
	$viewcnt = 1;
	$answer = $jsFormLib->getAll('SOCKET', 'socket_id', $this_colval, $button_value,  $viewcnt, $fopt);
	echo $answer;
	echo "<br>";
	
	echo "test col select: ";
	$this_colval = 2;
	$button_value = "--test2--";
	$viewcnt = 2;
	$colName ="selcheck";

	$selectVals = array(array(1,"test"), array(2,"test2"), array(4,"test3") );
	$butid = $jsFormLib->getID($viewcnt);
	echo '<select size=0 name=argu[' .$colName. '] id="'.$butid.'"> '."\n";
	$found=0;
	foreach( $selectVals as $dummy=>$valis) {
		 echo "<option value=\"".$valis[0]."\" ";
		 if ( $valis[0] == $this_colval) {
			 echo " selected";
			 $found=1;
		 }	
		 echo "> ". $valis[1] ."\n";
	}
	if ( !$found ) {
		 echo "<option selected";	
		 echo "> ". $this_colval ."\n";
	}
	echo "</select>\n";

	echo "</td><td>";
	$jsFormLib->shSelectAlter($viewcnt);

	
	echo "<br>";
	
	
	echo "<input type=hidden name='go' value='1'>\n";
	
	echo "<input type=submit value=\"Submit\">\n"; 
	echo "</form>";
}


}

global $error;

$error = &ErrorHandler::get();

$sqlo  = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
if ($error->printLast()) htmlFoot();

$go   = $_REQUEST["go"];
$parx = $_REQUEST["parx"];

$title       		 = "edit.tmpl objlink test";
$infoarr			 = NULL;
$infoarr["title"]    = $title;
$infoarr["scriptID"] = "edit.tmpl.objlink";
$infoarr["form_type"]= "tool";
$infoarr["locrow"]   =  array( array("./index.php", "root") );
//$infoarr['help_url'] = "o.EXAMPLE.htm";

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);
js_formAll();
js_formSel();
echo "<br>";

if ( $go ) {
	echo "params: <br>";
	print_r($_REQUEST);
}

$mainlib = new clEditTmplTest();
$mainlib->form1();