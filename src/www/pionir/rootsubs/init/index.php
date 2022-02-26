<?php
/**
 * index of system init/setup functions
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */


session_start(); 

require_once ("reqnormal.inc");
require_once 'o.MODULE.subs.inc';
require_once("f.textOut.inc");

class This_sys_init {
    
    static function get_role_rights($sqlo) {
        $sqlo->Quesel("count(1) from USER_RIGHT"); 
        $sqlo->ReadRow();
        $cnt = $sqlo->RowData[0];
        return $cnt;
    }
    static function get_module_cnt($sqlo) {
        $sqlo->Quesel("count(1) from MODULE where TYPE=".oMODULE_one::TYPE_PLUGIN );
        $sqlo->ReadRow();
        $cnt = $sqlo->RowData[0];
        return $cnt;
    }
    static function get_class_cnt($sqlo) {
        $sqlo->Quesel("count(1) from EXTRA_CLASS" );
        $sqlo->ReadRow();
        $cnt = $sqlo->RowData[0];
        return $cnt;
    }
    
    
}


global $error;
$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );

$back_url = "../rootFuncs.php";
$back_txt = 'Administration';

$sqloDummy = NULL;
$title = "System data init - home";
$infoarr			 = NULL;
$infoarr["scriptID"] = "rootsubsInitIndex";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "tool";
$infoarr["locrow"]   = array( array($back_url, $back_txt) );

$pagelib = new gHtmlHead();
$pagelib->startPage($sqloDummy, $infoarr);

if ( !glob_isAdmin() ) {
	htmlFoot("Info", "Sorry, you must be admin.");
}

$role_right_cnt = This_sys_init::get_role_rights($sqlo);
$reg_module_cnt = This_sys_init::get_module_cnt($sqlo);
$class_cnt = This_sys_init::get_class_cnt($sqlo);

?>
<ul>
<B>Main functions</B><br>
<UL>

<LI><a href="rights_insert.php"><img src="../../images/icon.ROLE.gif" border=0> Create missing role rights</a> (<?php echo $role_right_cnt;?>)</LI>
<LI><a href="../o.MODULE.register_li.php"><img src="../../images/icon.ROLE.gif" border=0> Register modules</a> (<?php echo $reg_module_cnt;?>)</LI>
<LI><a href="../../p.php?mod=DEF/root/install/g.install.CLASSES"><img src="../../images/icon.ROLE.gif" border=0> 
Register basic classes</a> (<?php echo $class_cnt;?>)</LI>
<LI><a href="../../p.php?mod=DEF/root/install/g.install.TABLE_OBJs"> Export/Create basic objects</a> (JSON-config)</LI>
<LI><a href="../../p.php?mod=DEF/root/install/o.CHIP_READER.upd_loc"><img src="../../images/icon.CHIP_READER.gif" border=0> 
Update related containers</a></LI>
<br />
<LI><a href="cct_table_ins.php"><img src="../../images/icon.ROLE.gif" border=0> Auto create CCT_TABLE</a></LI>

</ul>
<?php

$linkarr = array();
$linkarr[] = array('ty'=>'head','txt'=>'Other exports',  "lay"=>"1");
$linkarr[] = array('ty'=>'lnk','href'=>'../../p.php?mod=DEF/root/install/g.install.export.UT_data', 'txt'=>'Export Initial UT objects', );
$linkarr[] = array('ty'=>'lnk','href'=>'i_meta_export.php', 'txt'=>'Export SYSTEM table content', );
$linkarr[] = array("ty"=>"headclose");

$linkarr[] = array("ty"=>"headclose");

$linkout = new textOutC();
$linkout->showlinks($linkarr);
		

htmlFoot();

