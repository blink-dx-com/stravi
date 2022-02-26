<?php
/**
 * show meta features of object
 * @package glob.obj.feat.php
 * @swreq UREQ:0000980: g > glob.obj.feat.php : show META features of object 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param  $t tablename
 * @param  $id id of object
 * @version $Header: trunk/src/www/pionir/glob.obj.feat.php 59 2018-11-21 09:04:09Z $
 */

// extract($_REQUEST); 
session_start(); 


// require_once ("reqnormal.inc"); // includes all normal *.inc files
// 
// require_once ('edit.sub.inc');
// require_once ( 'glob.obj.access.head.inc');

require_once ("visufuncs.inc");
require_once ('subs/glob.obj.propShow.inc');
require_once ("f.objview.inc");	
require_once('db_x_obj.inc');
require_once 'subs/glob.obj.superhead.inc';

class gObjFeatShow {	
		
    function __construct(&$sqlo, $tablename, $id) {
    	$this->tablename = $tablename;
    	$this->id = $id;
    	$this->isbo      = cct_access_has2($tablename);
    	$this->prim_name = PrimNameGet2($tablename);
    	$this->nameCol   = importantNameGet2($tablename);
    	
    	$this->t_rights = tableAccessCheck( $sqlo, $tablename );
    	$this->o_rights = access_check($sqlo, $tablename, $id);
    	
    	
    }
    
    function _headOut($text) {
    	echo "<font color=gray>- ".$text.":</font> \n";
    }
    function _sectionClose() {
    	echo "<br />\n";
    }
    
    function _showAttach(&$sqlo) {
    	
    	$tablename = $this->tablename;
    	$attachtab="SATTACH";
    	$id=$this->id;
    	
    	$this->_headOut('Attachments');
    	
    	$sqlo->query("SELECT REL_ID FROM ".$attachtab." WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$id);
    	if (!$sqlo->ReadRow()) {
    		echo " none<br>\n";
    		return;
    	}
    	echo "<br>\n";
    	
    	$tabobj = new visufuncs();
    	$headOpt = array( "title"=>"Attachments", "borderColor"=>"#E0E0E0" );
    	$headx   = array("", "ID", "Name", "Key");
    	$tabobj->table_head($headx,   $headOpt);
    	$sqlo->query("SELECT REL_ID, NAME, KEY FROM ".$attachtab." WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$id. " ORDER BY REL_ID");
    	while ($sqlo->ReadRow() ) {
    	
    		$relid = $sqlo->RowData[0];
    		$tname = $sqlo->RowData[1];
    		$key = $sqlo->RowData[2];
    		$tmpAttName = htmlspecialchars($tname);
    		$dataArr = array(
    			"<a href=\"obj.sattach.edit.php?tx=$tablename&id=$id&rel_id=$relid\"><img src=\"images/arrow.but.gif\" border=0></a>", 
    			$relid, 
    			"&nbsp;<b>".$tmpAttName."</b>&nbsp;",
       			$key
    			);
    		$tabobj->table_row ($dataArr);
    	}
    	$tabobj->table_close();
    }
    
    /**
     * show children of object
     * @param  $sqlo
     */
    function _show_objlinkParents(&$sqlo, &$sqlo2) {
    	
    	$MAXSHOW=20; // max shown elements
    	
    	 
    	if (!table_exists2("S_OBJLINK")) return; // for older Partisan-DB-versions
    	
    	
    	
    	$headTxt = 'Object link parents';
    	$this->_headOut($headTxt);
    	$tablename = $this->tablename;
    	$useTab    = "S_OBJLINK";
    	$id = $this->id;
    	$objLinkLib = new fObjViewC();
    	
    	$sqlsel = "MO_TABLE, MO_ID, KEY from S_OBJLINK where ".
    		' CH_ID='.$id. ' and CH_TABLE= '.$sqlo->addquotes($tablename).' order by POS';
    	$sqlo->Quesel($sqlsel);
    	$cnt=0;
    	while ($sqlo->ReadRow()) {
    		if ($cnt==0) echo "<ul>\n";
    		if ($cnt > $MAXSHOW) {
    			echo "... more ...<br>\n";
    			break;
    		}
    		$childTable = $sqlo->RowData[0];
    		$childID    = $sqlo->RowData[1];
    		$key        = $sqlo->RowData[2];
    		$html1 = $objLinkLib->bo_display( $sqlo2, $childTable, $childID );
    		echo $html1." (".$key.")<br />\n";
    		$cnt++;
    	}
    	if ($cnt) echo "</ul>\n";
    	$this->_sectionClose();
    }
    
    /**
     * show children of object
     * @param  $sqlo
     */
    function _show_objlinkChild(&$sqlo, &$sqlo2) {
    	$MAXSHOW=20; // max shown elements
    	$allowChildAdd = 0;
    	if (!table_exists2("S_OBJLINK")) return; // for older Partisan-DB-versions
    	
    	// check insert-group-right
    	if ( $this->o_rights["insert"]) $allowChildAdd=1;
    	
    	$headTxt = 'Object link children';
    	if ( $allowChildAdd ) $headTxt .= ' [<a href="p.php?mod=DEF/o.S_OBJLINK.addChild'.
    		'&t='. $this->tablename .'&id=' .$this->id. '&parx[action]=add">Add child</a>]';
    	$this->_headOut($headTxt);
    	$tablename = $this->tablename;
    	$useTab    = "S_OBJLINK";
    	$id = $this->id;
    	$objLinkLib = new fObjViewC();
    	
    	$sqlsel = "CH_TABLE, CH_ID, KEY, POS from S_OBJLINK where ".
    		' MO_ID='.$id. ' and MO_TABLE= '.$sqlo->addquotes($tablename).' order by POS';
    	$sqlo->Quesel($sqlsel);
    	$cnt=0;
    	while ($sqlo->ReadRow()) {
    		if ($cnt==0) echo "<ul>\n";
    		if ($cnt > $MAXSHOW) {
    			echo "... more ...<br>\n";
    			break;
    		}
    		$childTable = $sqlo->RowData[0];
    		$childID    = $sqlo->RowData[1];
    		$key        = $sqlo->RowData[2];
    		$pos        = $sqlo->RowData[3];
    		$html1 = $objLinkLib->bo_display( $sqlo2, $childTable, $childID );
    		echo $html1;
    		if ($key!=NULL) echo ' <span style="color:gray">Key:</span> '.$key;
    		if ( $allowChildAdd ) {
    			echo ' &nbsp;&nbsp;&nbsp; [<a href="p.php?mod=DEF/o.S_OBJLINK.addChild'.
    				'&t='.$this->tablename.'&id='.$this->id.'&parx[pos]='.$pos.'&parx[action]=del">Remove</a>]';
    		}
    		echo "<br />\n";
    		$cnt++;
    	}
    	if ($cnt) echo "</ul>\n";
    	$this->_sectionClose();
    }
    
    function _shVarios(&$sqlo) {
    	$tablename=$this->tablename;
    	$id = $this->id;
    	
    	$this->_headOut('Object vario values');
    	echo ' [<a href="glob.obj.S_VARIO.php?t='.$this->tablename.'&id='.$this->id.'">View/Edit</a>]';
    	
    	// OLD: $varioLib = new globObj_S_VARIO_ed();
    	
    }
    
    function _shExtraObj2(&$sqlo, &$sqlo2) {
    	global $varcol;
    	
    	$tablename=$this->tablename;
    	if ( !glob_column_exists($tablename, 'EXTRA_OBJ2_ID') ) return;
    	
    	$this->_headOut('Second Extra Class');
    	echo '(still no EDIT-support) ';
    	
    	$sqlsel = 'EXTRA_OBJ2_ID from '.$tablename.' where '.$this->prim_name.'='.$sqlo->addQuotes($this->id);
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadRow();
    	$EXTRA_OBJ2_ID = $sqlo->RowData[0];
    	
    	if (!$EXTRA_OBJ2_ID) {
    		echo 'none<br>'."\n";
    		return;
    	}
    	
    	echo '<br />'."\n";
    	
    	$extraobj_o = fVarcolMeta::get_args_by_id( $sqlo, $EXTRA_OBJ2_ID );
    	
    	$viewcnt=0;
    	$editAllow=0;
    	$editRemarks=1;
    	$selexists=0;
    	
    	echo '<table>'."\n";
    	
    	$XFormLib = new fEditXobjForm( $tablename );
    	$viewcnt  = $XFormLib->showCols( $sqlo, $viewcnt, $editAllow, $extraobj_o, $editRemarks, $selexists );
    		
    	echo '</table>'."\n";
    	echo '<br />'."\n";
    }
    
    function doit(&$sqlo, &$sqlo2) {
    
    	$propLib = new gObjPropShowC();
    	$propLib->initObj( $sqlo, $this->tablename, $this->id );
    	$propLib->objFeaturesShow( $sqlo );
    
    	echo "<br>\n";
    	
    	if ($this->isbo) {
    		// $this->_shVarios($sqlo);
    		$this->_show_objlinkParents($sqlo, $sqlo2);
    		$this->_show_objlinkChild($sqlo, $sqlo2);
    		if ($this->tablename=='ABSTRACT_SUBST') $this->_shExtraObj2($sqlo, $sqlo2);
    		$this->_showAttach($sqlo);	
    	}
    }

}
// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
$sqlo2 = logon2( );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$t = $_REQUEST['t'];
$id= $_REQUEST['id'];

$tablename			 = $t;
// $i_tableNiceName 	 = tablename_nice2($tablename);

// $title      = "Meta features of this object";
// $infoarr = NULL; 
// $infoarr["title"]      = $title;
// $infoarr["title_sh"] = 'Meta features';
// $infoarr["scriptID"] = "";
// $infoarr["form_type"]= "obj";
// $infoarr["obj_name"] = $tablename;
// $infoarr["obj_id"]   = $id;
// $infoarr['help_url'] = 'glob.obj.feat.html';
// $infoarr['checkid']  = 1;

// $pagelib = new gHtmlHead();
// $pagelib->startPage($sqlo, $infoarr);
// $accHeadLib = new gObjAccessHead( 'meta', $tablename, $id );
// $accHeadLib->showNavTab();

$gui_lib = new glob_obj_superhead($tablename, $id);
$gui_lib->page_open($sqlo, '0meta');

$mainlib = new gObjFeatShow($sqlo, $tablename, $id);
//echo "&nbsp;<font color=gray>Info: This tool shows meta features of an object, which are not reflected in the EDIT/VIEW-form or in the ACCESS-form.</font>";

echo "<ul>\n";
echo "<b>Meta features</b>";
echo " &nbsp;&nbsp;&nbsp; ".fHelpC::link_show('glob.obj.feat.html', "help", array("object" =>"icon"));
echo "<br>\n";

if ( !cct_access_has2($tablename) ) {
	htmlFoot("Info", "This function supports only business objects.");
}

$mainlib->doit($sqlo, $sqlo2) ;

$gui_lib->page_close($sqlo);
