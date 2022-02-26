<?php
/**
 * main admin tool collection
 * - contains tool-links for super users and admins
 * $Header: trunk/src/www/pionir/rootsubs/rootFuncs.php 59 2018-11-21 09:04:09Z $
 * @package rootFuncs.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
session_start(); 

require_once ('reqnormal.inc');
require_once ("f.textOut.inc");
require_once ("f.objview.inc");

class gRootFuncs {
	
	function __construct() {
		$this->linkout = new textOutC();
	}
	
	function showlinks($linkarr) {
		$this->linkout->linksOut($linkarr);
	}
	
}

$sqlo  = logon2( $_SERVER['PHP_SELF'] );
$title     = "Administration tools";
$infoarr			 = NULL;
$infoarr["scriptID"] = "rootFuncs.php";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "tool";
$infoarr["design"]= "slim";
$infoarr["help_url"] ="super_user_functions.html";
$infoarr["locrow"] = array( array("../home.php", "home") );

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);

$mainLib = new gRootFuncs();

if ( !$_SESSION['s_suflag'] && ($_SESSION['sec']['appuser']!="root") ) {
  echo "Sorry, you must be root or have su_flag.";
  return 0;
}

gHtmlMisc::func_hist("rootFuncs", $title, $_SERVER['PHP_SELF'] );
echo "<img src=\"../images/icon.system3.png\">";

echo '&nbsp;<br>';
echo '<table border=0><tr><td>&nbsp;&nbsp;&nbsp;&nbsp;</td><td valign=top>'."\n";

$linkarr = array(
	array('ty'=>'head', 'txt'=>'<img src="../res/img/info.svg" height=20> System info', "lay"=>"1"),
	array('ty'=>'lnk','href'=>'../glob.syscheck.php', 'txt'=>'<b>System check</b>', 
			'notes'=>'[<a href="../view.tmpl.php?t=GLOBALS">globals table</a>]'),
	array('ty'=>'lnk','href'=>'phpinfo.php', 'txt'=>'PHP Info'),
	array('ty'=>'lnk','href'=>'../glob.tabsel.php', 'txt'=>'<b>Select tables</b>'),
	array('ty'=>'lnk','href'=>'tableContents.php', 'txt'=>'<b>DB Tables:</b> Contents'),
	array('ty'=>'lnk','href'=>'globalsList.php', 'txt'=>'<b>PHP</b> System variables'),
	array('ty'=>'lnk','href'=>'test_caches.php', 'txt'=>'<b>Meta-data cache</b>: test &amp; refresh'),
	array('ty'=>'lnk','href'=>'f.benchmark.php', 'txt'=>'<b>Benchmark test</b>'),
	array('ty'=>'lnk','href'=>'obj.chip_reader.testQcDev.php', 'txt'=>'<b>Test QC_DEVICE maintenance</b>'),
	array('ty'=>'br'),
	array("ty"=>"headclose")
	);
$mainLib->showlinks($linkarr);



$linkarr = array(
	array('ty'=>'head','txt'=>'System and DB statistics',  "lay"=>"1"),
	
	
	array('ty'=>'lnk','href'=>'f.appLogAna.php?parx[mode]=apperr', 'txt'=>'Application ERROR log', 'li'=>'br',
		'icon'=>'../images/i13_info.gif'),
	array('ty'=>'lnk','href'=>'f.appLogAna.php?parx[mode]=phplog', 'txt'=>'PHP ERROR log', 'li'=>'br',
		'icon'=>'../images/i13_info.gif'),
	array('ty'=>'lnk','href'=>'f.appLogAna.php?parx[mode]=MODUL_LOG', 'txt'=>'MODUL_LOG', 'li'=>'br',
		'icon'=>'../images/i13_info.gif', 'notes'=>'[<a href="../p.php?mod=DEF/root/app.modulLog.compress">compress</a>]'),
	array('ty'=>'lnk', 'href'=>'bo.time_info.php', 'txt'=>'Data creation statistics', 'li'=>'br',
		'icon'=>'../images/i13_info.gif'),
	array('ty'=>'lnk','href'=>'f.userFuncsAnalyze.php', 'txt'=>'<b>Web</b> Server Log', 'li'=>'br',
		'icon'=>'../images/i13_info.gif'),
	array('ty'=>'lnk','href'=>'../p.php?mod=DEF/root/g.load.ana', 'txt'=>'Process Load Analysis', 'li'=>'br',
		'icon'=>'../images/i13_info.gif'),
	array('ty'=>'lnk','href'=>'../p.php?mod=DEF/root/g.advmod_log.ana', 'txt'=>'ADVMOD_LOG', 'li'=>'br',
		'icon'=>'../images/i13_info.gif'),
    array('ty'=>'lnk','href'=>'../p.php?mod=DEF/root/g.analyse_sqllog', 'txt'=>'SQL_GLOBLOG', 'li'=>'br',
		'icon'=>'../images/i13_info.gif'),
	array('ty'=>'lnk','href'=>'../p.php?mod=DEF/root/g.xmlrpc_log.ana', 'txt'=>'XML RPC LOG', 'li'=>'br',
		'icon'=>'../images/i13_info.gif'),

	// array('ty'=>'lnk','href'=>'f.SerialObjAna.php', 'txt'=>'Show Serialized objects'),
	// array('ty'=>'lnk','href'=>'../f.servload.php', 'txt'=>'Server load: time diagram'),
	array('ty'=>'lnk','href'=>'o.DB_USER.loggedin.php', 'txt'=>'<b>Users:</b> logged in',  
		'li'=>'br', 'icon'=>'../images/icon.DB_USER.gif'),
	array("ty"=>"headclose")
	);
$mainLib->showlinks($linkarr);


// next COLUMN
echo '</td><td>&nbsp;&nbsp;&nbsp;&nbsp;</td><td valign=top>'."\n";


$linkarr = array(
	array('ty'=>'head','txt'=>'<img src="../images/icon.DB_USER.gif"> User tables',  "lay"=>"1"),
	array('ty'=>'lnk', 'href'=>'../view.tmpl.php?t=DB_USER', 'txt'=>'users', 'li'=>'br', 'icon'=>'../images/icon.DB_USER.gif',
		'notes'=>'&nbsp;&nbsp;&nbsp;[<a href="../obj.db_user.su.php">login as other user</a>]'),
	array('ty'=>'lnk','href'=>'../view.tmpl.php?t=USER_GROUP', 'txt'=>'groups', 'li'=>'br','icon'=>'../images/icon.USER_GROUP.gif'),
	array('ty'=>'lnk','href'=>'../view.tmpl.php?t=ROLE', 'txt'=>'roles', 'li'=>'br','icon'=>'../images/icon.ROLE.gif'),
	array('ty'=>'lnk','href'=>'../view.tmpl.php?t=USER_RIGHT', 'txt'=>'user rights (single rights)', 
		'li'=>'br','icon'=>'../images/icon._empty.gif'),
    array('ty'=>'br'),
	array("ty"=>"headclose")	
	);
	
$mainLib->showlinks($linkarr);

// --- show links to special system tables

$linkarr = array();
$linkarr[] = array('ty'=>'head','txt'=>'<img src="../images/but.listshort.gif"> Other tables',  "lay"=>"1");

$viewopt = '../';
$objLinkLib = new fObjViewC($viewopt);

$sys_tables = array(
	'CCT_TABLE', 
	'CCT_COLUMN', 
	'EXTRA_CLASS', 
	'GLOBALS' ,
	'MODULE'
);


foreach($sys_tables as $onetable) {
	$iconUrl = $objLinkLib->_getIcon($onetable);
	$linkarr[] = array('ty'=>'lnk','href'=>'../view.tmpl.php?t='.$onetable, 'txt'=>tablename_nice2($onetable), 'li'=>'br','icon'=>$iconUrl);
}

$iconUrl = $objLinkLib->_getIcon('MODULE');
$url = '../view.tmpl.php?t=MODULE&condclean=1&searchCol=x.TYPE&searchtxt=2';
$linkarr[] = array('ty'=>'lnk','href'=>$url, 'txt'=>'-- workflows', 'li'=>'br','icon'=>$iconUrl);
$url = '../view.tmpl.php?t=MODULE&condclean=1&searchCol=x.TYPE&searchtxt=3';
$linkarr[] = array('ty'=>'lnk','href'=>$url, 'txt'=>'-- tools/plugins', 'li'=>'br','icon'=>$iconUrl);


$linkarr[] = array('ty'=>'br');
$linkarr[] = array("ty"=>"headclose");
$mainLib->showlinks($linkarr);



$linkarr = array();
$linkarr[] = array('ty'=>'head','txt'=>'<img src="../images/ic.config.gif"> System Tests',  "lay"=>"1");

$linkarr[] = array('ty'=>'lnk','href'=>'../glob.syscheck.php', 'txt'=>'System check', );
$linkarr[] = array('ty'=>'lnk','href'=>'test/test.appserver.php', 'txt'=>'Application server test',);

$tmparr = array('ty'=>'lnk','href'=>'', 'txt'=>'UnitTests', );
$tmpScripturl = '../../_tests/index.php';
if ( file_exists($tmpScripturl) ) {
	$tmparr['href'] = $tmpScripturl;
}
$linkarr[] = $tmparr;

$linkarr[] = array('ty'=>'br');
$linkarr[] = array("ty"=>"headclose");
$mainLib->showlinks($linkarr);

// -------------------

$linkarr = array();
$linkarr[] = array('ty'=>'head','txt'=>'<img src="../images/ic.paxml.png"> Paxml - data export/import',  "lay"=>"1");

$linkarr[] = array('ty'=>'lnk','href'=>'../impexp/partisanxml/export.select.php', 'txt'=>'Export', );
$linkarr[] = array('ty'=>'lnk','href'=>'../impexp/partisanxml/import.select.php', 'txt'=>'Import', );
$linkarr[] = array('ty'=>'lnk','href'=>'../impexp/partisanxml/info.php', 'txt'=>'Paxml DB info', 
	'notes'=>'[<a href="../help/robo/export_of_objects.html" target=help>help?</a>]');


$linkarr[] = array("ty"=>"headclose");
$mainLib->showlinks($linkarr);


// next COLUMN
echo '</td><td>&nbsp;&nbsp;&nbsp;&nbsp;</td><td valign=top>'."\n";

$linkarr = array();
$linkarr[] = array('ty'=>'head','txt'=>'<img src="../images/ic.config.gif"> DB Installation / Upgrades / Modification',  "lay"=>"1");

	$linkarr[] = array('ty'=>'head','txt'=>'System initialization (in SQL)',  "lay"=>"1");
	$linkarr[] = array('ty'=>'lnk','href'=>'init/index.php', 'txt'=>'Create/Export INITIAL system data', );
	$linkarr[] = array('ty'=>'br');
	$linkarr[] = array("ty"=>"headclose");
	
	$linkarr[] = array('ty'=>'head','txt'=>'DB Transformations / Upgrades',  "lay"=>"1");
	
	$linkarr[] = array('ty'=>'lnk','href'=>'db_transform/index.php', 'txt'=>'<b>Upgrade HOME</b>', );
	$linkarr[] = array('ty'=>'lnk','href'=>'db_transform/convert.php', 'txt'=>'[DB_Haeckelianer] Upgrade DB-version', );
	$linkarr[] = array('ty'=>'lnk','href'=>'o.user_group.secwri.php', 'txt'=>'DB-mods for "security_write" mode', );
	$linkarr[] = array('ty'=>'lnk','href'=>'db_clean/index.php', 'txt'=>'CleanHouse', );
	
	//OLD: $linkarr[] = array('ty'=>'lnk','href'=>'../p.php?mod=DEF/root/temp.USER_PREF.convert', 'txt'=>'Convert USER_PREF cols', );
	$linkarr[] = array('ty'=>'br');
	$linkarr[] = array("ty"=>"headclose");
	
	$linkarr[] = array('ty'=>'head','txt'=>'DB dumps, import/export [DbDump]',  "lay"=>"1");
	$linkarr[] = array('ty'=>'lnk','href'=>'db_imp/index.php', 'txt'=>'Import dump', );
	
	$linkarr[] = array("ty"=>"headclose");
	
	
$linkarr[] = array("ty"=>"headclose");
$mainLib->showlinks($linkarr);


$linkarr=array();
$linkarr[] = array('ty'=>'head','txt'=>'<img src="../images/i20.expmisc.gif"> Misc',  "lay"=>"1");
$linkarr[] = array('ty'=>'lnk','href'=>'../show.tmpl.php', 'txt'=>'SQL-Hardcore', );
$linkarr[] = array('ty'=>'lnk','href'=>'test/index.php', 'txt'=>'Script-Function-Tests', );

$linkarr[] = array("ty"=>"headclose");
$mainLib->showlinks($linkarr);

// close TABLE
echo '</td></tr></table>'."\n";


$pagelib->htmlFoot();


