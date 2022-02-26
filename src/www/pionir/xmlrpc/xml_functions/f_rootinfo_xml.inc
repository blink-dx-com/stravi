<?php
/**
 * @namespace core::xmlrpc
 */


/**
 * info for user root
 *
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   array $f_param <pre>
 * 	 [0] sess_id
 * 	 [1] $funcname  : "lastlogin", 
 * 	</pre>
 * @return array   <pre>
 *  array(user_id, nick, login_last, logout_last)
 * </pre>
 * @throws exception 
 * @see     "usedby:external:ICONOCLUST"
 * 
 */
function f_rootinfo_xml(
	$f_name, 
	$f_param
	)
{


    if (sizeof($f_param) < 2) return array(1, "Incorrect number of parameters.");
    
	$my_s_id  = $f_param[0];
	$funcname = $f_param[1];
    $retans = "";
	$retval = 0;
	$retarr = NULL;
	
    // session start
    $xmlHelpLib = new xmlrpcHelper(); 
    $answer = $xmlHelpLib->startSession($my_s_id, 'f_rootinfo'); 
    if ($answer[0]>0) return $answer;    

	
	$sql = $xmlHelpLib->getSqlHandle();
	
	if ($_SESSION['sec']['appuser']!="root")  return array(2, "Only root can call these functions");
	
	switch ($funcname) {
		case "lastlogin":
			// !!! do not change the order of fields
			$sqls = "select db_user_id, nick, ".$sql->Sql2DateString("login_last",1).", ".$sql->Sql2DateString("logout_last",1).
				    " from db_user where login_last is not NULL order by login_last DESC ";
			$sql->query($sqls);
			$sql->ReadRow();
			$retarr = $sql->RowData;
			break;
		default: return array(3, "Unknown subfunction #$funcname#");
		
	}
	return array($retval, $retarr);
}
