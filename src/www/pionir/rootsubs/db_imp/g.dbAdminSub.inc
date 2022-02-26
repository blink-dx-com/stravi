<?php
/**
 * - provide functions for database dump import
 * - DB-user generation
 * 
 * $Header: trunk/src/www/pionir/rootsubs/db_imp/g.dbAdminSub.inc 59 2018-11-21 09:04:09Z $
 * @package g.dbAdminSub
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

// pure actions
class dbAdminSub {
	
function installCheckLogin() {
	if ( $_SESSION["install_admin"] == NULL ) return 0;
	return 1;
}

function checkLogin() {
	if ( $_SESSION["db_admin"] == NULL ) return 0;
	return 1;
}

}