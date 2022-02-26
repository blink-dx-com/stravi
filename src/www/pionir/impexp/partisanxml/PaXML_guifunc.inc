 <?php
/*MODULE: PaXML_guifunc.inc
  DESCR:  help funcs for Paxml
  AUTHOR: qbi
  VERSION: 0.1 - 20050111
*/

require_once("access_check.inc");
require_once("role.inc");

class paxmlHelpC {
	function exportCheckRole( &$sql, $printout=1) {
		$allow = 0;
		$role_right_name = "PaXml_export";
		$role_right      = role_check_f($sql, $role_right_name);
		if ($role_right!="execute" ) {
			echo "<br>";
			htmlErrorBox("Info", "Export denied!", "Sorry, you must have role right '$role_right_name'. to use this tool."  );
			echo "<br>";
		} else {
			$allow = 1;
		}
		
		return ($allow);
	}
}