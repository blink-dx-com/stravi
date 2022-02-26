<?php
require_once("f.modulLog.inc");
/**
 * global XMLRPC helper library
 * @package xml_rpc_func.inc
 * @swreq UREQ:xxxxxxxxxxxx
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * $Header: trunk/src/www/pionir/xmlrpc/xml_rpc_func.inc 59 2018-11-21 09:04:09Z $
 */

/**
 * xmlrpc helper class
 * @author steffen
 */
class xmlrpcHelper {
	
	/**
	 * call to start a session
	 * @param string $sessid
	 * @param string $methodname
	 * @return array
	 *    errval : 0 : no problem
	 *    errtext
	 */
	public function startSession($sess_id, $methodname) {
		
		if( $sess_id==NULL ) return array(2,"Invalid session ID !");
		
		session_id($sess_id);
extract($_REQUEST); 
session_start(); 

		
		if( $_SESSION['sec']['appuser']==NULL ) return array(2,"Invalid session ID !"); //invalid session id
		
		$modLogLib = new fModulLogC();
		$sqldummy  = NULL;
		$modLogLib->logModul($sqldummy, 'XMLRPC:'.$methodname, 1); // do NOT logging on file
		
		return array(0);
	}
	
	function &getSqlHandle() {
		
		return logon2( $_SERVER['PHP_SELF'] );
	}


	/**
	 * Run a query on database and return the results
	 * Return  array with the results
	 * @param object $sql
	 * @param string $query_str
	 * @return multitype:number unknown
	 */
	function run_query( &$sql, $query_str ){
	
	   $p=$sql->query($query_str);
	
	   $row_cc   =0;
	   $my_test  =0;
	   $my_retval=0;
	
	   while($sql->ReadRow()) {
	    foreach ($sql->RowData as $mk => $mv)
	     if(($mv==="")) $sql->RowData[$mk]="";
	     $row[$row_cc++]=$sql->RowData;
	     $my_test=1;
	   };
	
	   return array($my_retval,$row);
	}
}
