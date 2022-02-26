<?php
/**
 * html output for access things (get table-admins, rightsInfo)
 * @namespace core::access
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */


class fAcessInfoC {
	
function __construct($tablename) {
	$this->tablename=$tablename;
}

// get table-admin-users as nice-text
function get_admins( &$sql ) {
			
	$tablename = $this->tablename;
	$outstr = "";
	$dbuser = NULL;
	$sqlsel = role_o_get_users($tablename, "admin");
	$sql->Quesel($sqlsel);
	$cnt=0;
	$MAX_SHOW=10;
	while($sql->ReadRow()) {
	  $cnt++;
	  if ( $cnt>$MAX_SHOW ) {
	  	$dbuser[] = '-1';
	  	break;
	  }
	  $dbuser[] = $sql->RowData[0];
	}
	
	if ( !sizeof($dbuser) ) return; 
	
	$outstr   = "(<b>";
	$tmpkomma = "";
	foreach( $dbuser as $dummy=>$userid) {
		if ($userid==-1) {
			$outstr .= $tmpkomma. '...';
		} else {
			$sql->query("select nick from db_user where db_user_id=".$userid);
			$sql->ReadRow();
			$outstr .= $tmpkomma. $sql->RowData[0];
		}
		$tmpkomma = ", ";
	}
	$outstr .= "</b>) ";
	return ($outstr); 
}

function rightsInfo(
	&$sql,
 	$opt=NULL // "isTabAdmin" 0|1
			  // "accCheckRed" 0|1
	) {
	// requires: func_head.inc
	
	
	echo "<small><font color=#707070><b>Insert</b> is the right to add/update/delete feature list elements of the object.<br>\n";
	echo "<b>Entail</b> is the right to set rights.<br>\n";
	if ( $opt["accCheckRed"]>0) 
		echo "<img src='images/but.checkred.gif'> &nbsp;&nbsp; the user has the object accession right, but not the role right.<br>\n";
	if ($_SESSION['globals']["security_write"]>0) 
		echo "<b>Security</b>: the database runs in 'ObjectHiProtectPolicy' (OHIPPO)".
		     " mode (<a href=\"".$_SESSION['s_sessVars']["loginURL"]."/help/robo/g.accWriteDataSecurity.html\" target=help>see help</a>).<br>\n";
	if ( $opt["isTabAdmin"]>0 AND !glob_isAdmin() ) 
		echo "<img src='images/i13_infog.gif'> &nbsp;&nbsp; You are 'table admin' - you can modify rights of all objects of this type.<br>\n";
	
	$tableAdminInfo =  $this->get_admins( $sql );
	if ($tableAdminInfo!="") {
		echo "<img src='images/i13_info.gif'> &nbsp;&nbsp; Table admins: ".
			"The users  ".$tableAdminInfo." have the right to add group rights.".
			"<br>\n";
	}
	echo "</font></small>";	
}

}
