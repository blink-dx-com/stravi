<?php
/* contains database-version-strings for convert.php
 * - STANDARD-versions start with a number or letter, but not with "i"
 * - LAB versions start with "i"
 * @package db_versions.inc
 * @swreq UREQ:
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/rootsubs/db_transform/db_versions.inc 59 2018-11-21 09:04:09Z $
 */
function db_versionsGet_BASIC() {
	
	/**
	 * - STANDARD-versions start with a number or letter, but not with "i"
     * - LAB versions start with "i"
	 */
	$versarr = array ( 
		"2.0.0.0",
		"2.0.0.1",
	    "2.0.0.2",
	    "2.0.0.3",
	    "2.0.0.4",
	    "2.0.0.5",
	    "2.0.0.6",
	    "2.0.0.7",
		"--- other ---",
		"i_views",
		'xTest',
		// 'iATJdocman'
		);

	return ($versarr);
}