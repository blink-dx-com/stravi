<?php
/**
 * the plugin manager page
 * - following CODE-bases are supported:
 *   - the DEF: BASE: [APP]/plugin
 *   - the LAB: BASE: [APP]/www/[LAB]/plugin
 * - the plugin class must be defined like this: <pre>
 * 	 class moduleName_XPL extends gPlugin {
 *   }
 * </pre>
 * @package p.php
 * @swreq   SREQ:0001100: g > provide a Plugin Manager 
 * @see 1002_SDS_code ; chapter plugin
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param string $mod module-path e.g. DEF/o.CHIP_READER.protAddArch
 * @param array  $parx [RESERVED] normal params
 * @param int    $go   [RESERVED] 0,1,2, progress state
 * @param $p_session_started [0],1 - if called from edit.tmpl.php
 */
 
if (isset($_REQUEST['p_session_started'])) {
	// this is needed, if this script is called from inside an other script
	// example: can be included in edit.tmpl.php
} else {
    session_start(); 
}
require_once ('reqnormal.inc');
require_once ('gui/f.plugin.inc');
require_once ('gui/f.plugin_mng.inc');
require_once 'o.S_VARIO.subs.inc';






global $error, $varcol;
$FUNCNAME='p.php';
$error = & ErrorHandler::get();

$paramsUrl = f_Plugin_MngPage::Get_Params();
$sqlo  = logon2( $_SERVER['PHP_SELF'] . $paramsUrl );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$orglib = new f_Plugin_MngPage();
$classname = $orglib->includeMod($sqlo, $_REQUEST['mod']);
if ($error->Got(READONLY))  {
	$error->set( $FUNCNAME, 1, 'Error on Plugin: "'.$_REQUEST['mod'].'"' );
	$orglib->pageInitError($sqlo);
}
$orglib->check_permission($sqlo);
if ($error->Got(READONLY))  {
    $error->set( $FUNCNAME, 1, 'Permission-Error on Plugin: "'.$_REQUEST['mod'].'"' );
    $orglib->pageInitError($sqlo);
}

$plugLib = new $classname($sqlo);
$plugLib->register();
$mod_features = $orglib->get_MODULE_features();
if (isset($mod_features['LOCKED']) and $mod_features['LOCKED']>0) {
	$plugLib->infoarr["row2_extTxt"] .= '<img src="images/but.lock.in.gif" title="module is locked for users by table MODULE.">';
}
$varcont = unserialize($_SESSION["userGlob"]["g.plugin"]);
// get version of module
if ($varcont['shMod']>0 and $plugLib->infoarr["version"]==NULL ) {
	$mod_file_name = $orglib->get_module_file_name();
	$mod_version   = gHtmlHead::get_version($mod_file_name);
	$plugLib->infoarr["version"] = $mod_version;
}

$plugLib->_set_mod_features($mod_features);
$plugLib->infoarr["modLockChecked"] = 1; // already checked
$plugLib->startHead();
if ($error->Got(READONLY))  {
	$error->printAll();
	$plugLib->htmlFoot();
}

try {
    
    $plugLib->startMain();
    
    if ($error->Got(READONLY))  {
        
        $err_stack = &$error->getAllNoReset();	
        htmlErrorClassBox("Error", 'ERROR', $err_stack);
        $error->save_ErrLogAuto();
        $error->reset();
    	
    }
    
} catch (Exception $e) {
    
    $errmess = $e->getMessage( );
    $trace_string = $e->getTraceAsString();
    $trace_string = str_replace("\n", "<br>", $trace_string);
    $plugLib->htmlFoot('ERROR', $errmess.'<br><br>'.$trace_string);
    
}

$plugLib->htmlFoot();
 
 