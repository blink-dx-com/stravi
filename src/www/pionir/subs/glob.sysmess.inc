<?php
/**
 * system message class
 * $Header: trunk/src/www/pionir/subs/glob.sysmess.inc 59 2018-11-21 09:04:09Z $
 * @package    glob.syscheck.inc
 * @subpackage glob.sysmess.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
 
 
class sysMessC {

var $debug; /*
		0 - short info
		1 - more info, number of files in paths
		2 - more info, file size (takes time...)
			*/

function __construct($debug) {
	$this->debug = $debug;
	$this->tableHeaderIsOut = 0;
	$this->helpLib = new fHelpC();
	
	$this->_helpArr= array(
	    'app.advmod',
		"DbLogin.LDAP",
		"DbLogin.method",
  		"xObjModiLog",
		"security_write",
		"initTriggerID"
		);
}

function _helpExists($varname) {
	return ( in_array($varname, $this->_helpArr) );
}

/**
 *  - show one information row
 *  - if $this->debug <= 0 AND $error_flag=-2 : do not show the row
 * 
 * @param $test_key // keyword for info
 * @param $texter   // short info
 * @param $message  // short OK/ERROR message
 * @param $varname  // GLOBALS variable name, can be NULL
 * @param $value    // value of variable (can be LONG text !!!)
 * @param $error_flag <pre>
 * 0  : o.k.
 * 1  : error
 * -1  : warning
 * -2  : not set (gray)
 * </pre>
 * @param $notes
 * @param $optx <pre>
 *   "noglob" [0] | 1 is not a global variable 
 *   "headline" = 
 *       [0] 
		  1  show only $texter
		  2  white placeholder
 *   "infIcon"  = "INFO" : show special icons in col: "Info"
 *   "addValTxt"  - add text to value (also HTML)
 * </pre>
 * @return 
 */
function messageout( $test_key, $texter, $message, 	$varname,  $value,  $error_flag, $notes=NULL,$optx =NULL ) {
     global $bgcolor;
	 
	 if ($error_flag=="")  $error_flag = 0;
	 $colInfoIcon = "";
	 if ($optx["infIcon"]=="INFO")  $colInfoIcon = " <img src=\"images/i13_warning.gif\" title=\"important!\">";
	 
	 
	 // if no ERROR or WARNING, hide the row
	 if ( ($this->debug<=0) AND ( ($error_flag==-2) OR ($error_flag == 0) ) ) return; // hide the row
	 
	 if ( !$this->tableHeaderIsOut ) $this->_messageTabHead();
	 
	 $mess_color="green";
     if ( $error_flag==1 )  $mess_color="red";
     if ( $error_flag==-1 ) $mess_color="#808000";
	 if ( $error_flag==-2 ) $mess_color="#808080";	// e.g. not set
	 
	 $bgcolor    = empty($bgcolor) ? ' bgcolor="#DDDDff"' : '';
	 $bgcolorout = empty($bgcolor) ? ' bgcolor="#EFEFEF"' : $bgcolor;
	 
	 if ($optx["headline"]) {
	 	$tabcol_num=2;
		$tmpbgColor="#EFEF80";
		if ($this->debug) $tabcol_num = $tabcol_num + 4;
		if ($optx["headline"]==2) $tmpbgColor="#FFFFFF";
		
	 	echo '<tr bgcolor='.$tmpbgColor.'><td colspan='.$tabcol_num.'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<font color=gray><b>'.$texter.'</b></font></td></tr>'; 
		return;
	 }
	 
     echo '<tr'.$bgcolorout.'><td> <b>'.$texter.'</b>'; 
	 echo '<td align="center"><b><span style="color:'.$mess_color.';"> '.$message;
	 echo $colInfoIcon;
	 echo ' </span></b></td>';
	 
     echo '</td>';
     if ( $this->debug ) {
     	echo "<td>";
		if ( $varname!="") {
			if ( $optx["noglob"]!=1 ) {
				echo '<a href="rootsubs/obj.globals.editelem.php?name='.$varname.'">'.$varname.'</a>';
		} else {
				echo $varname;
			}
		}
		
		$valOut = $value;
		if ($optx["noglob"]<=0) { // if "noglob" : show real value
			$maxlen_val=50;
			if (strlen($value)>$maxlen_val ) {
				$valOut2 = '<span title="'.htmlspecialchars($value, ENT_QUOTES, 'UTF-8').'">'.substr( $value,0,$maxlen_val )."...</span>" ;
			}
			else $valOut2=htmlspecialchars($value);
			$valOut = $valOut2;
		}
		if ($optx["addValTxt"]!=NULL) $valOut .=$optx["addValTxt"];
		echo '</td><td>'.$valOut;  
        echo "</td>";
     
     }
     if ($this->debug) {
		
		 if ( $this->_helpExists($test_key) ) {
		 	$helpurl = $this->helpLib->link_show("g.System_check.html#".$test_key, "help");
		 }
		 echo "<td>".$helpurl."</td>";
		 echo "<td>".$notes."&nbsp;</td>";
	 }
     echo "</tr>\n";
     echo "\n";

}

function _messageTabHead() {
	$this->tableHeaderIsOut = 1;
	echo '<table cellspacing="1" border="0" cellpadding="2" frame="void">'."\n";
	echo "<tr>";
	echo "<td class=\"xshead\">Test</td>";
	echo "<td class=\"xshead\">Message</td>";
	if ($this->debug)  {
		echo "<td class=\"xshead\">DB-key</td>";
		echo "<td class=\"xshead\">Value</td>";
		echo "<td class=\"xshead\">Help</td>";
		echo "<td class=\"xshead\">Notes</td>";
	}
	
	echo "</tr>\n";
}

function tableClose() {
	if ( !$this->tableHeaderIsOut ) return;
	
	echo "</table>\n<br><br>";
}
 
}
