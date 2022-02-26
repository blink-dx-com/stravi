<?
require_once ('reqnormal.inc');

session_start(); 
$error = & ErrorHandler::get();
$pagelib = new gHtmlHead();
$pagelib->startPageNoHtml();

if ( !$_SESSION['s_suflag'] && ( $_SESSION['sec']['appuser']!="root" ) ) {
	echo "Sorry, you must be root or have su_flag.";
	return 0;
}

phpinfo();
?>
