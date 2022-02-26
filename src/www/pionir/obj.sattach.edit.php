<?php
/**
 * insert, edit, view one attachment
 * - support Attachment-Analysis-Plugin: to analyse the attachment after upload
 *   - Config: ABS_OBJECT:VARIO: attachment.plugin = {ATTACH_KEY} = NAME_OF_PLUGIN (e.g. EVAgreen)
 *   - Attachment-Analysis-Plugin is located in www/lab/obj/{TABLE}
 *   - example: lab/obj/ABSTRACT_SUBST/ATPL_EVAgreen.inc
 * @package obj.sattach.edit.php
 * @swreq  SREQ:0002033: o.SATTACH > visualisieren, editieren eines Attachments 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param 
 * 	$tx      ( mother tablename ) 
  	$id      (e.g. ARRAY_LAYOUT_ID)
	$rel_id  (if $action!='insert')
	$action = 
		["edit"]   : on $go=0: show edit-form
				   : on $go=1: update
		 "view"    : view attachment info
		 "insert"  : insert new attachment
		 "sessvar" : set editmode
	$parx   : parameters
	$go
	$opt   : options
	  'forward' : 'obj' -- auto forward back to object
	$_FILES['userfile'] - uploaded file
 * @version $Header: trunk/src/www/pionir/obj.sattach.edit.php 59 2018-11-21 09:04:09Z $
 */
// extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("visufuncs.inc");
require_once ('f.msgboxes.inc');
require_once ('func_form.inc');
require_once ('insert.inc');
require_once ('date_funcs.inc');
require_once ("javascript.inc" );
require_once ('glob.obj.conabs.inc');

require_once ("o.SATTACH.subs.inc");
require_once ("o.SATTACH.mod.inc");
require_once ('o.CCT_TABLE_DYN.subs.inc');
require_once ('lev1/o.SATTACH.upload.inc');
require_once ('gui/o.SATTACH.ed.inc');
/**
 * get predefined keys for attachment
 * @author steffen
 */
class attach_DefKeys {
	function __construct() {
		$this->helpConCAbs = new gConcAbsC();
	}
	/**
	 * get predefined keys for this object
	 * - look at the abstract object, if keys are defined there
	 * @swreq SREQ:0002033:006.a: Anzeige Vorauswahl von KEYs
	 * @param  object $sqlo
	 * @param  string $table tablename of object
	 * @param  int $objid ID of object
	 * @return array of keys
	 */
	function getKeys(&$sqlo, $table, $objid) {

		$keys = array(); // must be an array
		
		// get keys from CCT_TABLE_DYN
		$tableDynLib  = new oCCT_TABLE_DYN_subs();
		$glob_key_str = $tableDynLib->getValByKey( $sqlo, $table, 'g.SATTACH.keys');
		if ($glob_key_str!=NULL) {
			$keys = explode(',',$glob_key_str);
			if (!is_array($keys)) $keys=array();
		} 

		do {
			
			$tmplTable = $this->helpConCAbs->getTemplate($table);
			if ($tmplTable==NULL) break;
			
			$pkName = $table.'_ID';
			$fkName = $tmplTable.'_ID';
			$fkColFeatures = colFeaturesGet2($table, $fkName );
		
			if ($fkColFeatures==NULL) break; // column not defined
		
			// get abstract objid
			$absObjID = glob_elementDataGet( $sqlo, $table, $pkName, $objid, $fkName);
			if (!$absObjID) break; // no abstract object
			
			// get dynamic value 'attachment.keys'
			$varioLib = new oS_VARIO_sub($tmplTable);
			$key='attachment.keys';
			$valTmp = $varioLib->getValByKey( $sqlo, $absObjID, $key );
			if (trim($valTmp)==NULL)  break; // no keys defined on abstract object
			
			$keys_extra = explode(',',$valTmp);
			if (!is_array($keys_extra)) $keys_extra=array();
			
			$keys = array_merge($keys, $keys_extra); // merge with keys from CCT_TABLE_DYN:g.SATTACH.keys
			
		} while (0);
		
		
		return $keys;
	}
}



/**
 * GUI to manage one attachment
 * @author steffen
 */
class oSattachEdGui {
	
	var $modenow; // 'edit', 'view'
	var $iniArgu; // initial argus of attachment

	
    function __construct(	&$sql, $tablename, $id, $rel_id, $action, $parx, $go, $tool_opt) {
    	
    	$this->tablename=$tablename;
    	$this->objid  = $id;
    	$this->rel_id = $rel_id;
    	$this->action = $action;
    	$this->go	  = $go;
    	$this->editAllow = 1;
    	$this->parx   = $parx;
    	$this->tool_opt = $tool_opt;
    	$this->iniArgu= NULL;
    	
    	
    	$this->modenow = $_SESSION['s sessVars']['o.'.$tablename.'.editmode'];
    	$this->satObj  = new cSattachSubs();
    	
    	$this->infox=NULL;
    	$this->infox['IsProtected'] = 0; // if 1: protect special fields
    	
    	// new columns exists ? ARCHIVE, XDATE, DB_USER_ID
    	$newColsExist = glob_column_exists('SATTACH', 'ARCHIVE');
    	$this->infox['newColsExist'] = $newColsExist;
    	
    	// 
    	//   check   R I G H T S
    	//
    	$t_rights = tableAccessCheck( $sql, $tablename );
    	if ( $t_rights["read"] != 1 ) {
    		$answer = getRawTableAccMsg( $tablename, 'read' );
    		htmlFoot('ERROR', $answer);
    	}
    	if ( !$t_rights["write"] ) {
    		$this->editAllow = 0;
    	}
    	
    	
    	$o_rights = access_check($sql, $tablename, $id);
    	if ( !$o_rights["read"] ) htmlFoot("ERROR", "no read permissions on object!");
    	if ( !$o_rights["write"])  $this->editAllow = 0;
    	
    	$attachKeyLib = new attach_DefKeys();
    	$keyarr      = $attachKeyLib->getKeys($sql, $tablename, $id);
    	$keySelField = $this->_getSelect($keyarr);
    	
    	$fsize = 60;
    	
    	$this->fieldx = array();
    	$this->fieldx['NAME'] = array ( "title" => "Name", "name"  => "NAME",
    			"object" => "text", "req"   => 1,
    			"val"   => $parx["NAME"], 'fsize'=>$fsize,
    			"notes" => "Name of attachment" );
    	
    			
    	$this->fieldx['MIME_TYPE'] = array ( "title" => "Mime-type", "name"  => "MIME_TYPE",
    			"object" => "text",
    			"val"   => $parx["MIME_TYPE"],'fsize'=>$fsize,
    			"notes" => "Mime-type" );
    	
    	
    	$this->fieldx['KEY'] = array ( "title" => "Key", "name"  => "KEY",
    			"object" => "text",
    			"val"   => $parx["KEY"], 'fsize'=>$fsize,
    			"notes" => $keySelField .
    			' <img src="images/help.but.gif" title="keys can be predefined in the related abstract object; vario-column: attachment.keys">');
    	    			
    	$this->fieldx['NO_EXPORT'] = array ( "title" => "No export", "name"  => "NO_EXPORT",
    			"object" => "checkbox",
    			"val"   => $parx["NO_EXPORT"], 'fsize'=>$fsize,
    			"notes" => "Export of attachment is not allowed" );
    	
    	if ($this->infox['newColsExist'] ) {
    		$this->fieldx['ARCHIVE'] = array ( 
    			"title" => "Protect", 
    			"name"  => "ARCHIVE",
    			"object" => "checkbox",
    			"val"   => $parx["ARCHIVE"], 
    			"colspan"=> 2,
    			"notes" => ' if "Protect" is set, the attachment is proteced against manipulation' );
    	}
    		
    	$this->fieldx['NOTES'] = array ( "title" => "Notes", "name"  => "NOTES",
    			"object" => "textarea",
    			"val"   => $parx["NOTES"],
    			"inits"=> array("rows"=>20, "cols"=>70),
    			"colspan"=>"2"  
    			);
    	
    	$this->fieldx['UPLOAD']  = array (
    	    "title" => "Upload new file",
    	    "name"  => "userfile",
    	    "namex" => TRUE,
    	    "object" => "file",
    	    "fsize" => 40
    	    
    	); 
    }
    
    /**
     * init structure
     * - set $this->iniArgu, $this->infox
     * @param  $sqlo
     */
    function initForm(&$sqlo) {
    	$tablename	= $this->tablename;
    	$id 		= $this->objid;
    	$rel_id 	= $this->rel_id;
    	
    	$argu=NULL;
    	
    	if ($rel_id) {
    		$sqls = "* from SATTACH where table_name='".$tablename."' AND obj_id=".$id." AND rel_id=".$rel_id;
    		$sqlo->Quesel($sqls);
    		if ( !$sqlo->ReadArray() ) {
    			htmlFoot("Error", "Attachment ($tablename, $id, $rel_id) not found!");
    		}
    		$argu = $sqlo->RowData;
    		$this->iniArgu = $argu;
    		
    		$this->infox['IsProtected'] = 0; // if 1: protect fields
    		if ($this->iniArgu['ARCHIVE']>0) {
    			$this->infox['IsProtected']=1;
    		}
    	}
    	return $argu;
    }
    
    
    function _initSelectTable($formname, $destVarName)  {
    	
    	?>
    	<script language="JavaScript">
    	<!--
    		function takeVal(x)
    		{
    			if (x=="") return;
    			document.<?php echo $formname.".elements['".$destVarName."']" ?>.value=x;
    		}
    	//-->
    	</script>
    	<?php
    }
    
    
    	
    	
    function _getSelect($keyarr) {
    	
    	if (!sizeof($keyarr)) return;
    	
    	$var = '<select  name="optionsx" size=1 onChange="takeVal(this.form.optionsx.options[this.form.optionsx.options.selectedIndex].value)">'.
    		'<option value="">--- predefined keys ---</option>';
    	reset ($keyarr);
    	foreach( $keyarr as $dummy=>$key) {
    		$var .='<option value="'.$key.'">'.$key.'</option>';
    	}
    	reset ($keyarr); 
    	$var .= 	'</select>';
    	return $var;
    
    
    }
    
    /**
     * show insert form
     */
    function form_insert( &$sql ) {

    	$tablename=$this->tablename;
    	$id = $this->objid;
    	
    	$this->_initSelectTable('editform', 'parx[KEY]');
    			
    	$initarr= NULL;
    	$initarr["action"]      = $_SERVER['PHP_SELF'];
    	
    	$initarr["submittitle"] = "Upload";
    	$initarr["tabwidth"]    = "AUTO";
    	$initarr["ENCTYPE"]     = "multipart/form-data";		// for uploading files
    	$initarr["tabnowrap"]	= 1;
    	
    	$hiddenarr = NULL;
    	$hiddenarr["tx"] 	= $tablename; 
    	$hiddenarr["id"] 	= $id;
    	$hiddenarr["action"] 	= 'insert';
    	if (!empty($this->tool_opt)) {
    	    $hiddenarr["opt[forward]"] = $this->tool_opt['forward'];
    	}
    	
    	
    	$formobj = new formc($initarr, $hiddenarr, 0);
    	
    	// recommended to use name "userfile"
    	$fieldx=NULL;
       
    	$field_UPLOAD = $this->fieldx['UPLOAD'];
    	$field_UPLOAD["req"] = 1;
        $fieldx[] = $field_UPLOAD;
        
        $fieldx[] = $this->fieldx['KEY'];
        $fieldx[] = $this->fieldx['ARCHIVE'];
        
        $field_NOTES = $this->fieldx['NOTES'];
        $field_NOTES["inits"]["rows"] = 2;
        $field_NOTES["optional"] = 1;
        
        $fieldx[] = $field_NOTES;
        
        $formobj->fieldArrOut( $fieldx );
    	
    	$formobj->close( TRUE );
    	
    	
    }
    
    function _niceKeyVal($key,$val) {
    	echo ' <span style="color:gray">'.$key.':</span>'.$val;
    }
    
    function _showFormHead(&$sqlo) {
    	$tablename = $this->tablename;
    	$id		   = $this->objid;
    	$rel_id    = $this->rel_id;
    	
    	// echo "&nbsp;<a href=\"obj.sattach.view.php?t=$tablename&id=$id\"><img src=\"images/but.list2.gif\" border=0> list of other attachments</a> ";
    	
    	
    	$urlSess = $_SERVER['PHP_SELF'] .'?tx='.$tablename.'&id='.$id.'&rel_id='.$this->rel_id.
    		'&action=sessvar';
    	$editViewButStr = formc::editViewBut(
    		$this->modenow,	  // ["view"], "edit"
    		$this->editAllow, // 0|1
    		$urlSess,  		  // go to ... e.g. "waferfunc.php?id=92939"
    		'parx[editmode]'		  // e.g. "viewmode"
    		);
    	echo $editViewButStr;
    	
    	$uploaderName='---';
    	$XDATE='---';
    	$XESIG_ID='---';
    	if ($this->iniArgu['DB_USER_ID']) {
    		$uploaderName = obj_nice_name ( $sqlo, 'DB_USER', $this->iniArgu['DB_USER_ID'] );
    	}
    	if ($this->iniArgu['XDATE']!=NULL) {
    		$XDATE=$this->iniArgu['XDATE'];
    	}
    	if ($this->iniArgu['XESIG_ID']!=NULL) {
    		$XESIG_ID = $this->iniArgu['XESIG_ID'];
    	}
    	
    	echo "\n".'&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
    	$this->_niceKeyVal('rel_id',$rel_id);
    	$this->_niceKeyVal('upload-date',$XDATE);
    	$this->_niceKeyVal('upload-user',$uploaderName);
    	$this->_niceKeyVal('eSign',$XESIG_ID);
    	echo "<br>";
    }
    
    /**
     * show normal edit/view form of ONE attachment
     * @param  $sql
     * @param  $parx
     */
    function formshow( &$sql, $parx) {
    	
    	global $error;
    	$tablename = $this->tablename; // $parx["TABLE_NAME"];
    	$id		   = $this->objid;
    	$rel_id    = $this->rel_id;
    	
    	$this->_initSelectTable('editform', 'parx[KEY]');
    	
    	$this->_showFormHead($sql);
    	
    	
    	
    	// manage the action-key
    	$action = $this->action;
    	if (!$this->editAllow) $action = "view";
    	
    	$fieldx = $this->fieldx;
    	
    	$initarr= NULL;
    	$initarr["action"]      = $_SERVER['PHP_SELF'];
    	
    	$initarr["submittitle"] = "Update";
    	$initarr["tabwidth"]    = "AUTO";
    	$initarr["tabnowrap"]	= 1;
    	$initarr["ENCTYPE"]     = "multipart/form-data";		// for uploading files
    	
    	$hiddenarr = NULL;
    	$hiddenarr["tx"] 	= $tablename; //$parx["TABLE_NAME"]; 
    	$hiddenarr["id"] 	= $id; 		 // $parx["OBJ_ID"];
    	$hiddenarr["rel_id"]= $rel_id;   // $parx["REL_ID"];
    	$hiddenarr["action"]= 'edit';
    	$tmpallow = TRUE;
    	$optclos  = NULL;
    	
    	if ($action=="view") {
    		$tmpallow = TRUE;
    		$optclos["noRow"] = 1;
    	}
    	
    	// protect these fields, if ARCHIVE=1
    	$protectedKeys = array('NAME','ARCHIVE','MIME_TYPE', 'KEY', 'UPLOAD');
    	if ( glob_isAdmin() ) {
    		// allow edit of 'ARCHIVE' flag ???
    		if (($key = array_search('ARCHIVE', $protectedKeys)) !== false) {
    			unset($protectedKeys[$key]);
    		}
    	}
    	
    	$formobj = new formc($initarr, $hiddenarr, 0);
    				
    	foreach( $fieldx as $key=>$fieldNow) {
    		
    		$loopAction = $action;
    		if ($this->infox['IsProtected'] and in_array($key, $protectedKeys) ) {
    			$loopAction='view';
    		} 
    		
    		if ($loopAction=='edit') {
    			$fieldNow["val"]    = $parx[$key];
    		}
    		if ($loopAction=="view") {
    			$fieldNow["req"]    = NULL;
    			$fieldNow["object"] = "info2";
    			$fieldNow["inits"]  = $parx[$key];
    			unset ($fieldNow["val"]);
    			unset ($fieldNow["name"]);
    			
    			if ($key=='KEY') {
    				$fieldNow["notes"] = NULL; // remove special select-box
    			}
    		}
    		
    		$formobj->fieldOut( $fieldNow );
    	}
    	
    	
    	
    	$formobj->close( $tmpallow, $optclos );
    	
    	$rel_id   = $this->rel_id;
    	$filename = $this->satObj->getDocumentPath($tablename, $id, $rel_id);
    	if (file_exists( $filename )) {
    		$filesizex = filesize( $filename );
    		echo "<b>Attachment-Download</B>";
    		echo " &nbsp;<font color=gray>(for download: on icon, click right mouse button: <I>\"save link as\")</I></font>";
    		echo "<br><br>";
    		echo "<a href=\"obj.sattach.down.php?t=$tablename&id=$id&rel_id=$rel_id\"><img src=\"images/ic.docdown.big.gif\" TITLE=\"Click right mouse button\" border=0></a>";
    		
    		echo " &nbsp;&nbsp;&nbsp;$filesizex bytes<br><br>";
    	}
    }
    
    
    
    /**
     * show INSERT GUI
     */
    function gui_insert(&$sql) {
    	
    	$tablename	= $this->tablename;
    	$id 		= $this->objid;
    	$parx 		= $this->parx;
    	
    	echo "<B>Add attachment ...</B><br>";
    
    	$this->form_insert($sql);
    	
    	$thumbTables=array('A_CHIP_READER', 'CHIP_READER', 'ABS_CONTAINER', 'CONTAINER', 'ABSTRACT_SUBST', 'CONCRETE_SUBST');
    	
    	$show_help = 0;
    	if ( in_array($tablename,$thumbTables) ) {
    		$show_help = 1;
    	}
    	
    	if ($show_help) {
    		echo "<br><br>";
    		htmlInfoBox( "Short help", "", "open", "HELP" );
    		echo "<ul>\n";
    		
    		if ( in_array($tablename,$thumbTables)) {
    			echo '<li>To upload a thumbnail-image of the object, tag it with key="ObjThumbnail"</li>';
    		}
    		echo "</ul>\n";
    		htmlInfoBox( "", "", "close" );
    	}
    	
    	
    }




}

// -----------------------------------------------------------------------

global $error, $varcol;

$error = & ErrorHandler::get();
$sql   =  logon2( $_SERVER['PHP_SELF'] );
// $sql2  =  logon2(  );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$tx 	= $_REQUEST['tx'];
$id 	= $_REQUEST['id'];
$rel_id = $_REQUEST['rel_id'];
$parx   = $_REQUEST['parx'];
$action = $_REQUEST['action'];
$go     = $_REQUEST['go'];
$argu_opt = $_REQUEST['opt'];

$tablename  = $tx;
// $attachtab  = "SATTACH";
$objTabNice = tablename_nice2($tablename);

$title      = $objTabNice.':'.$id. ":Attachment rel_id:".$rel_id;
if ($action==NULL) {
	$action = $_SESSION['s sessVars']['o.'.$tablename.'.editmode']; // fallback 1
	if ($action==NULL) $action ='view'; // fallback 2
}

$infoarr=array();
$infoarr["title"] = $title;
$infoarr["title_sh"] = "one Attachment";
$infoarr["form_type"]= "obj";
$infoarr["icon"]     = "images/icon.SATTACH.gif";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;
$infoarr["show_name"]= 1;
$infoarr['help_url'] = 'o.SATTACH.html';
$infoarr["locrow"]   = array( array( "obj.sattach.view.php?t=".$tablename."&id=".$id, "attachment list") );

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

if ($tablename=="" OR $id=="") {
	htmlFoot("Error", "tablename or obj-id missing");
} 
if ( ($action=="" or $action=="edit" or $action=="view") and !$rel_id) {
	htmlFoot("Error", "rel_id missing. (action: ".$action.")");
} 

if ($action=="sessvar" and $parx['editmode']!=NULL) {
	$_SESSION['s sessVars']['o.'.$tablename.'.editmode'] = $parx['editmode'];
	$action = $parx['editmode']; // edit or view
	$go=0;
}

echo "<ul>";

if ($go>0) {
	$doStop=0;
	$modLib = new oSATTACH_modWork($sql, $tablename, $id, $go);
	if ($error->Got(READONLY))  {
		$pagelib->chkErrStop(); // severe error
		return;
	}
	
	
	$answer = $modLib->manageActions($sql, $_FILES['userfile'], $parx, $rel_id, $action);
	if ($error->Got(READONLY))  {
		$error->printAll();
		/*
		$errLast   = $error->getLast();
		$error_txt = $errLast->text;
		$error_id  = $errLast->id;
		*/
		$error->reset();
		$doStop=1;
		
	} else {
	    if ($argu_opt['forward']=='obj') {
	        $newurl = "edit.tmpl.php?t=$tablename&id=$id";
	        js__location_replace($newurl, 'object' ); 
	        htmlFoot('<hr>');
	    }
	}
	
	if ( $modLib->get_doStop()>0) {
		$doStop  = 1;
	}
	
	if ( $answer['do'] =='showform' ) {
		$doStop  = 1;
		$go      = 0;
		$mainLib = new oSattachEdGui($sql, $tablename, $id, $rel_id, $action, $parx, $go, $argu_opt);
		$mainLib->initForm($sql);
		
		if ($action=="insert") {
			$mainLib->gui_insert($sql);
			
			if (cSattachSubs::count_attach($sql, $tablename, $id)) {
    			echo "<br>";
    			$attach_lib = new o_SATTACH_ed_sub();
    			$attach_lib->show_form($sql, $tablename, $id, 0);
			}
			htmlFoot();
		}
		
		$argu    = $answer['argu'];
		$mainLib->formshow($sql, $argu);
	}
	
	if ($doStop) {
		htmlFoot('<hr>');
	}
	
	// back to list view
	$newurl = "obj.sattach.view.php?t=$tablename&id=$id";
	js__location_replace($newurl, 'attachment list' ); 
	htmlFoot('<hr>');
	
}

$mainLib = new oSattachEdGui($sql, $tablename, $id, $rel_id, $action, $parx, $go, $argu_opt);
$argu    = $mainLib->initForm($sql);

if ($action=="insert") {
	$mainLib->gui_insert($sql);
	
	
	if (cSattachSubs::count_attach($sql, $tablename, $id)) {
	    
	    echo "<br>";
	    $attach_lib = new o_SATTACH_ed_sub();
	    $attach_lib->show_form($sql, $tablename, $id, 0);
	}
	htmlFoot('<hr>');
}

$mainLib->formshow($sql, $argu);


htmlFoot('<hr>');
