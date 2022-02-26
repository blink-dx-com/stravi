<?php
require_once "globals.inc";
require_once "role.inc";
/**
 * Contains functions to analyse table role rights
 * @package table_access.inc
 * @swreq   SREQ:0001071: g > table role rights : HOME 
 * @swreq   UREQ:0000507: o.ROLE > introduce TABLE_ROLES for ASSOC-tables (FDA) 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 */


/**
 * - Checks accession rights for a BO table. The rights are defined by ROLES.
 * - it does NOW reflect the "admin"-right from the ROLE-entries
 * - if table is a VIEW => deny all modification-rights
 * - if table is INTERNAL => deny all (except root)
 * - check, if table has an OBJECT-right:
 *     - use this mask directly
 * - else: check for mother
 * 
 * - mother-child inheritance:
 *       RIGHT MOTHER => CHILD
         READ   0/1	0/1
         WRITE  0/1	0/1
         INSERT 0/1	-- see WRITE
         DELETE 0/1	-- see WRITE
        
 * @param $tablename : any table type !
 * @return array( "read"=>, "write"=>, "delete"=>, "insert"=>, "admin"=> )  rights for the table
 */
function tableAccessCheck(&$sql, $tablename ) {
	global  $_s_i_table;
	
	$table_name_c = '';
	$mothertable  = '';
	
	$t_rights=array();
	$t_rights["insert"]=0; /* insert an element */
	$t_rights["delete"]=0; /* delete an element */
	$t_rights["write"] =0; /* update elements */
	$t_rights["read"]  =0; /* read elements */
	$t_rights["admin"] =0; /* admin elements */
	
	if ( $_SESSION['sec']['appuser'] == 'root' ) {
		$t_rights["insert"]=1; 
		$t_rights["delete"]=1; 
		$t_rights["write"] =1; 
		$t_rights["read"]  =1; 
		$t_rights["admin"] =1; 
		return ( $t_rights );
	}
	
	// check internal flag
	$internFlag = $_s_i_table[$tablename]["__table_data__"]["INTERNAL"];
	if ($internFlag>0)  {
		return ( $t_rights ); // ALL rights denied !!!
	}
	
	$sqlsel = "1 from USER_RIGHT where NAME='o.".$tablename."'" ;
	$sql->Quesel($sqlsel);
	if ( $sql->ReadRow() ) {
		// table is role-right controlled: overrules everything
		// can also be used for BO_ASSOC tables
		// array_merge necessary because does only return rights != 0
		$t_rights = array_merge($t_rights, role_check($sql, "o.".$tablename));
		return ( $t_rights ); 
	}
	
	$access_id_has = cct_access_has2($tablename);
	
	// is an BO ?
	if ( $access_id_has  > 0 ) {
		$table_name_c=$tablename;
	} else {
		$mothertable = mothertable_get2($tablename);	
		if ( $mothertable != "" ) {
			$access_id_has = cct_access_has2($mothertable);
			if ($access_id_has) $table_name_c = $mothertable;			
		}
	}
	
	if ( $table_name_c != "" ) { /* is related to BO */
	    $t_rights = array_merge($t_rights, role_check($sql, "o.".$table_name_c)); //array_merge necessary because does only return rights != 0
	} else {
		$t_rights["read"]=1; // only read allowed
	}
	
	if ($mothertable != "") {
		$t_rights["insert"]=0;
		$t_rights["delete"]=0;
		if ($t_rights["write"]) { // ASSOC elements inherit INSERT and DELETE from Mother[write]
			$t_rights["insert"]=1;
			$t_rights["delete"]=1;
		}
	}	
	
	if ( table_is_view($tablename) ) {
		$t_rights["insert"]=0; 
		$t_rights["delete"]=0; 
		$t_rights["write"] =0;
		$t_rights["admin"] =0;
	}

	
	return ( $t_rights );
}

function getRawTableAccMsg( $tablename, $right ) {
	$nicename = tablename_nice2($tablename);
	return( getTableAccessMsg($nicename, $right) );
}

function getTableAccessMsg( $nicename, $right ) {
	return( "You have no permission to '$right' at table <B>".$nicename."</B>.<br>\n".
	 "Reason: You have no role right '$right' for this object.<br>\n".
	 "Please contact the administrator to get role rights." );
}

function tableAccessMsg( $nicename, $right ) {
	echo "&nbsp;<center>";
	
	echo "<font color=red><B>INFO</B></font><P>\n";
	echo getTableAccessMsg( $nicename, $right );
	echo "<BR>";
	
	echo "</center>";
}
