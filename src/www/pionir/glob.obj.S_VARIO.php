<?php
/**
 * manage S_VARIO tags of ONE object
 * - now with ADVMOD
 * @package glob.obj.S_VARIO.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param string $t tablename
 * @param int $id   ID of object
 * @param parx
 *    editmode = 'view', 'edit'
 *   newkey : string
 *   newval : string 
 * @param array $keyval
 */
session_start(); 

require_once ('reqnormal.inc');
require_once ('gui/o.S_VARIO.ed.inc');
require_once ('o.S_VARIO.mod.inc');
require_once ("glob.obj.update.inc");
require_once ( "javascript.inc" );

class oS_VARIO_modgui {
	
	
	function __construct(&$sqlo, $tablename, $id) {
		$this->tablename=$tablename;
		$this->id=$id;
		$this->editPossible= 0;
		$this->editAllow   = 0; // form edit flag
		
		$o_rights = access_check($sqlo, $tablename, $id);
		$right='insert';
		if ( $o_rights[$right]>0) {
			$this->editPossible=1;
		}
		
	}

	
	/**
	 * update values
	 * @param $sqlo
	 * @param $parx
	 * @param $keyval
	 * @return -
	 */
	function update(&$sqlo, $parx, &$keyval) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$UpdateLib = new globObjUpdate();
		
// 		$modLib = new oS_VARIO_mod();
// 		$modLib->setObject( $sqlo, $this->tablename, $this->id );
// 		if ($error->Got(READONLY))  {
// 			return;
// 		}
		
		if ($parx['newkey']!=NULL and $parx['newval']!=NULL) {
		    $keyval[$parx['newkey']]=$parx['newval'];
			// $modLib->updateKeyVal($sqlo, $parx['newkey'], $parx['newval']);
		}
		
		if (!empty($keyval)) {
		    $args=array(
		        'vario'=>$keyval
		    );
		    $UpdateLib->update_meta($sqlo, $this->tablename, $this->id, $args);
		    if ($error->Got(READONLY))  {
		        return;
		    }
// 			foreach( $keyval as $key=>$val) {
// 				$modLib->updateKeyVal($sqlo, $key, $val);
// 			}
			
		}
		echo '... updated.<br />'."\n";
	}
}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$id 		= $_REQUEST['id'];
$tablename	= $_REQUEST['t'];
$go	= $_REQUEST['go'];
$parx	= $_REQUEST['parx'];
$keyval= $_REQUEST['keyval'];

$title		= 'Manage VARIO values of object';

$infoarr			 = NULL;
$infoarr['scriptID'] = '';
$infoarr['title']    = $title;
$infoarr['title_sh'] = 'VARIO values';
$infoarr['form_type']= 'obj'; 
$infoarr['help_url'] = 'o.S_VARIO.html';
$infoarr['obj_name'] = $tablename;
$infoarr['obj_id']   = $id;
$infoarr['checkid']  = 1;



$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo, $infoarr);

$accCheckArr = array('tab'=>array('write'), 'obj'=>array('write') );
$pagelib->do_objAccChk($sqlo, $accCheckArr);
$pagelib->chkErrStop();

echo ' [<a href="view.tmpl.php?t=S_VARIO_DESC&searchMothId='.$tablename.'">Defined Keys for this table</a>]<br>'."\n";
echo '<ul>'."\n";
$mainLib = new oS_VARIO_modgui($sqlo, $tablename, $id);

if ($go) {
	$mainLib->update($sqlo, $parx, $keyval);
	$pagelib->chkErrStop();
}

echo '</ul>'."\n";

$url='edit.tmpl.php?t='.$tablename.'&id='.$id;
js__location_replace($url, 'back to object' );

$pagelib->htmlFoot();
