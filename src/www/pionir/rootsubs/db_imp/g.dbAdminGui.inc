<?php
/**
 * - provide admin GUI functions
 * 
 * $Header: trunk/src/www/pionir/rootsubs/db_imp/g.dbAdminGui.inc 59 2018-11-21 09:04:09Z $
 * @package g.dbAdminGui
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ('func_form.inc');
require_once ("visufuncs.inc");
require_once ('g.dbImpAct.inc');
require_once ('g.dbAdminSub.inc');
require_once ('g.confInstFileSub.inc');

// pure actions
class dbAdminGui {
	
function __construct() {
	$this->fieldx = NULL;
	$this->fieldx['newuser'] = array ( 
		"title" => "new user", 
		"name"  => "newuser",
		"object"=> "text",
		"val"   => "", 
		"notes" => "new DB_USER"
		 );
	$this->fieldx['newpw'] = array ( 
		"title" => "new password", 
		"name"  => "newpw",
		"object"=> "text",
		"val"   => "", 
		"notes" => "new password"
		 );
	$this->fieldx['dumpfile'] = array ( 
		"title" => "dump file", 
		"name"  => "dumpfile",
		"object"=> "text",
		"val"   => "", 
		"fsize" => "80",
		"notes" => "e.g. /data/oradata/magasin/test.dmp "
		 );

	$this->fieldx['dumpfileTabs'] = array ( 
		"title" => "selected tables", 
		"name"  => "dumpfileTabs",
		"object"=> "text",
		"val"   => "", 
		"fsize" => "80",
		'optional'=>1,
		"notes" => "only selected tables; WARNING: you have to create CCT_TAB_VIEW and CCT_COLVIEW, becuase VIEWS are not created,".
				" when selected tables are imported! <br> e.g. APP_DATA_TYPE, CCT_COLUMN, ..."
		 );
	
	$this->fieldx['fromuser'] = array ( 
		"title" => "from user", 
		"name"  => "fromuser",
		"object"=> "text",
		"val"   => "", 
		"notes" => "dump-file comes from user (e.g. dynal) "
		 );
     
     $this->fieldx['magasin_ser'] = array ( 
		"title" => "magasin_serial", 
		"name"  => "magasin_ser",
		"object"=> "text",
		"val"   => "", 
		"notes" => "e.g. 3 (get from http://partilog)"
		 );
}

function &getFormArr() {
	return ($this->fieldx);
}

}