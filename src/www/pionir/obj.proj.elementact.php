<?php
/**
* - add/link/copy BO to/from a project
* - Projects, which were copied/cutted from a project-tree are named "PROJ_ORI" !
* @package obj.proj.elementact.php
* @author  Steffen Kube (steffen@blink-dx.com)
* @param  
*        $go : 0,1
*        $proj_id    -- "NULL" is allowed 
  	     $actio_elem 
	 	   "copy"     - copy objects to clipboard, need $sel[]
		   "copyplus" - copy objects additional to clipboard
	 	   "new"
		   "minusclip"
		   "link_paste" - paste as link
		   "paste_new"
		   "select"  - select checked objects to list
           "cut"     - unlink object from project, need $sel[]
		   "del" 	 - delete link or object
		   "delBroken" - delete broken links from $proj_id
		   "tocut"   - transform a clipboard-copy to a project-CUT action
	    [tablename]  (tablename of BO )
		[$actio_extra_param]
	    [sel[] ]    (selected objects ["TABLENAME,ID"]
	 del:
		[doflag]  "delproj", "deldb"
        [state]  0: prepare, 1: doit now
		[opt]  options:
        	   "projcutobj"
               "objdeepdel"  see obj.proj.delgui.inc
               "stopforward"
			   "selall"      select all object-links
			   "infoLevel"   0, [1], 2 
*/

// extract($_REQUEST); 
session_start(); 



require_once('db_access.inc');
require_once('globals.inc');
require_once('o.PROJ.addelems.inc');
require_once('access_check.inc');
require_once('table_access.inc');
require_once('func_head.inc');	
require_once("javascript.inc" );
require_once("f.objview.inc");	
require_once("f.clipboard.inc");
require_once("subs/obj.proj.manage.inc");
require_once 'glob.obj.touch.inc';
require_once 'glob.obj.touch.inc';

class oProjElemAct {
    
    var $proj_id;
    var $stopforward;
    var $errstack;
	
    function __construct($proj_id) {
    	$this->proj_id = $proj_id;
    	$this->stopforward = 0;
    	$this->errstack = NULL;
    }
    
    function err_out ($text, $fronttext=NULL ) {
        if ( $fronttext == "") $fronttext="Error";
        echo "<font color=red><B>".$fronttext.": </B></font><font color=#808080>".$text."</font>";
    }
    
    function xinfo_out ( $key, $text ) {
     	$color="gray";
        if ( $key == "ERROR")     $color="red";
    	if ( $key == "INFODENY" ) $color="red";
        $tmpstr = "<font color=".$color."><B>".$key.": </B></font> <font color=#808080>".$text."</font>";
    	return ($tmpstr);
    }
    
    function xinfo_show($infokey, $text){
    	$infokeys = array(
    		"INFO" => array("col"=>"gray", "txt"=>"Info" ),
    		"ERROR"=> array("col"=>"red", "txt"=>"Error" ),
    		"WARN" => array("col"=>"#606000", "txt"=>"Warning" ),
    		);
    	$thisInfx = &$infokeys[$infokey];
        echo "<font color=".$thisInfx["col"]."><b>".$thisInfx["txt"].":</b> </font>".$text."<br>\n";
    }
    
    function _userError($funcname, $text) {
    	$this->errstack[] = array("f"=>$funcname, "t"=>$text);
    }
    
    function _sub_paste( &$sql, $clipActCutProj ) {
    	// RETURN: $this->objfound
    	//		   $error object
    	global $error;
    	$FUNCNAME = "_sub_paste";
    	
    	$proj_id = $this->proj_id;
    	
    	$projMangLib  = new oProjManageC($proj_id);
    	
    	$proj_cut_Lib = NULL;
    	if ($clipActCutProj) {
    	    $proj_cut_Lib  = new oProjAddElem($sql, $clipActCutProj);
        	if ($error->Got(READONLY))  {
        	    $error->set( $FUNCNAME, 1, "Problems on Init PROJ-ID:".$clipActCutProj );
        	    return;
        	}
    	}
    	
    	$projAddLib  = new oProjAddElem($sql, $proj_id);
    	if ($error->Got(READONLY))  {
    		$error->set( $FUNCNAME, 1, "Problems on addElement-init" );
    		return;
    	}
    	$this->objfound = 0;
    	
    	foreach( $_SESSION['s_clipboard'] as $tmparr) {
    
    		$tmp_tablename = $tmparr["tab"];
    		$id0 = $tmparr["ida"];
    		
    		$alias_table = $tmp_tablename;
    		if  ( $tmp_tablename == "PROJ_ORI" )$alias_table = "PROJ";
    		
    		if ( !cct_access_has2($alias_table)  ) {
    			$error->set( $FUNCNAME, 2, "table '$alias_table' not allowed.".
    				" Only business objects can be stored in projects." );
    			return;
    		}
    		
    		if ($_SESSION['userGlob']["g.debugLevel"]>1 ) {
    			echo " - $tmp_tablename : $id0<br>\n";
    		}
    		
    		if ( $clipActCutProj ) { /* only after CUT operation */
			
    			if  ( $tmp_tablename == "PROJ_ORI" ) {	
    				$projMangLib->proj_paste( $sql, $id0 ); // check, if user tries to paste PRO_ID to same PRO_PROJ_ID !
    			} else {
    			    $proj_cut_Lib->unlinkObj($sql, $tmp_tablename, $id0);
    				$projAddLib->addObj($sql, $tmp_tablename, $id0);
    				
    			}
    		} else {
    			if  ( $tmp_tablename == "PROJ_ORI" ) $tmp_tablename = "PROJ";
    			$projAddLib->addObj($sql, $tmp_tablename, $id0);
    		}
    	
    		if ($error->Got(READONLY))  {
    			$error->set( $FUNCNAME, 4, "Error on oject $tmp_tablename ID: $id0!" );
    			return;
    		}
    		$this->objfound++;
    	}
    }
    
    // --------------------------------------
    
    function f_copy( &$sel, $actio_extra_param ) {
    	// copy elements
    	// RETURN: $this->userError
    	
    	global $error;
    	
    	$cliplib = new clipboardC();
    	$cliplib->reset();
    	
    	if ( $actio_extra_param != "plus ")
    		$cliplib->resetx();
    		
    	$cntelem = 0;	
    	if ( sizeof ($sel) ) {
    		foreach( $sel as $tmp_table=>$idarr) {
    			$loopcnt = sizeof($idarr);
    			$cliplib->obj_put($tmp_table, $idarr, 1);
    			$cntelem = $cntelem + $loopcnt;
    		}
    	} else {
    		$error->set( "f_copy", 1, 'No object selected!');
    		return;
    	}
    	$this->xinfo_show('INFO', "$cntelem object(s) copied to clipboard.");
    
    }
    
    /**
     *  bring selected objects to LIST-view selection
     * @param array $sel
     */
    function f_select(&$sqlo, &$sel) {
    	//
    	global $error;
    	require_once('sql_query_dyn.inc');
          
    	if ( empty($sel) ) {
    		$error->set( "f_copy", 1, 'No object selected!');
    		return;
    	} 
    	
    	//$tmpor    = "";
    	$sqlQuery = "";
    	$cntelem=0;
    	
    	// select only one TABLE-type
    	$tmp_table= key($sel);
    	$tmpIdArr = current( $sel );
    	
    	
    	if ($tmp_table=="PROJ_ORI") $tmp_table="PROJ";
    	
    	$pkname  = PrimNameGet2($tmp_table);
    	$idkeys  = array_keys($tmpIdArr);
    	$sqlseq  = implode(",", $idkeys);
    	$cntelem = sizeof($tmpIdArr);
    	$this->xinfo_show('INFO', "$cntelem object(s) selected in list.\n");
    	
    	$sqlQuery = "x.".$pkname." in (".$sqlseq.")";
    	selectGetEasy( $sqlo, $tmp_table, $sqlQuery);
    	
    	$newurl = "view.tmpl.php?t=".$tmp_table;
    	js__location_replace( $newurl, "list view");
    	exit;
    }
    
    function f_tocut() {
    	// bring projects in clipboard to a possible CUT action
    
    	$this->stopforward = 1;
    	$clipLib = new clipboardC();
    	$clipdata = $clipLib->getClipBoard();
    	
    	reset ($clipdata);
    	$cntelem = sizeof($clipdata);
    	$this->xinfo_show( "INFO", "$cntelem objects selected to be cut from THIS project after 'paste'.\n");
    	if (!$cntelem) {
    		htmlErrorBox("Error", "No elements in clipboard" );
    		return;
    	}
    	
    	// analyse clipboard
    	$infoNoPROJ  = 0;
    	$infoHasPROJ = 0;
    			
    	foreach( $clipdata as $dummy=>$valarr) {
    		$tab = $valarr["tab"];
    		if ($tab!="PROJ") {
    			$infoNoPROJ = 1;
    		}
    		if ($tab=="PROJ") {
    			$infoHasPROJ = 1;
    		} 
    	}
    	reset ($clipdata); 
    	
    	if ($infoHasPROJ AND $infoNoPROJ) {
    		htmlErrorBox("Error", "If you want to cut real PROJECTS, ".
    				"please do NOT have other object-types in the clipboard!" );
    		return;
    	}
    	
    	if ($infoHasPROJ) {
    		// transform PROJ => PROJ_ORI
    		$clipdataCopy = $clipdata; // make a copy
    		$clipLib->resetx(); // clear
    		
    		$cnt=0;
    		foreach( $clipdataCopy as $dummy=>$valarr) {
    			$tablename = $valarr["tab"];
    			$objid	   = $valarr["ida"];
    			if ($tablename=="PROJ")  $tablename="PROJ_ORI";
    			$clipLib->obj_addone ( $tablename, $objid );
    			$cnt++;
    		}
    		reset ($clipdataCopy); 		
    		$this->xinfo_show( "INFO", "$cnt project-entries converted for CUT action\n");
    	}
    	
    }
       
    /**
     *  paste BO from clip board 
    				 - on CUT: test access of CUT-project
     * @param object $sql
     */
    function f_paste(&$sql) {
    	global  $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$proj_id = $this->proj_id;
    	$clipActCutProj = $_SESSION['s_sessVars']["clipActCutProj"];
    	
    	if ( $clipActCutProj>0 ) {
    		$this->xinfo_show("INFO", "Mother-project '".
    				fObjViewC::bo_display( $sql, "PROJ", $clipActCutProj )  .
    				"' for CUT-action is activated");
    	
    		$o_rights = access_check( $sql, 'PROJ', $clipActCutProj );
    		if ( !$o_rights["insert"] ) {
    			$error->set( $FUNCNAME, 1, "no perimission to CUT objects from project ID:$clipActCutProj!".
    				" You need 'insert'-access on this project.");
    			return;
    		}  			
    	}
    			
    	if (!sizeof($_SESSION['s_clipboard']) ) {
    		$error->set( $FUNCNAME, 5, "No object in clipboard!" );
    		return;
    	}
    	
    	$this->_sub_paste( $sql, $clipActCutProj );
    	
    	if ( $error->Got(READONLY) )   return;
    	
    	$this->xinfo_show("INFO", "<b>".$this->objfound."</b> objects pasted.\n");
    	
    
    	if ($error->Got(READONLY))  {
    		$error->set( $FUNCNAME, 6, "error on object link insert!");
    		return;
    	}
    		
    }
    
    function f_minus_clip(&$sql, $go) {
    	global   $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$proj_id = $this->proj_id;
    	$numclip =  sizeof($_SESSION['s_clipboard']);
    		
    	if (!$numclip) {
    		$error->set( $FUNCNAME, 1, "No objects in clipboard!");
    		return;
    	}
    	
    	if (!$go) {
    		echo "[<a href=\"".$_SERVER['PHP_SELF']."?proj_id=".$proj_id."&actio_elem=minusclip&go=1\">".
    			"<B>Remove $numclip object-links from project now!</B></a>]<br><br>\n";
    	}
    	
    	if ($go) {
    	    $proj_cut_Lib  = new oProjAddElem($sql, $proj_id);
    	    if ($error->Got(READONLY))  {
    	        $error->set( $FUNCNAME, 1, "Problems on Init PROJ-ID:".$proj_id );
    	        return;
    	    }
    	}
    	
    	$found	= 0;
    	$removed= 0;
    	foreach( $_SESSION['s_clipboard'] as $th0=>$th1) {
    
    		$tmp_tablename=current($th1);
    		$id0 = next($th1);
    		
    		if ($_SESSION['userGlob']["g.debugLevel"]>1 ) {
    			echo " - t:$tmp_tablename id:$id0 <br>\n";
    		}
    		
    		if  ( $tmp_tablename == "PROJ_ORI" ) {
    			$this->xinfo_show("WARN", "real project [ID:$id0] found in clipboard,".
    				" please use DELETE for real projects.");
    			$this->stopforward = 1;
    		} else {
    			if ($go) { 
    				$proj_cut_Lib->unlinkObj($sql, $tmp_tablename, $id0);
    				if ($error->Got(READONLY))  {
    				    $error->set( $FUNCNAME, 1, 'Error on PROJ-ID:'.$proj_id );
    				    return;
    				}
    			}	
    			$removed++;
    		}
    		$found++;
    	}
    	
    	if ($go) $this->xinfo_show( "INFO", "Removed objects: ". $removed);
    	else {
    		$this->xinfo_show( "INFO", "Analysed objects: ". $removed);
    		$this->stopforward = 1;
    	}
    	
    }
    
    function f_pastenew() {
    	/* paste BO from clip board as new */
    	global   $error;
    	$FUNCNAME = "f_pastenew";
    	
    	$proj_id = $this->proj_id;
    	$_SESSION['s_sessVars']['boProjSel'] = $proj_id;
    	
    	if (count($_SESSION['s_clipboard']) != 1) {
    		$error->set( $FUNCNAME, 1, 'You must copy exactly ONE object to the clipboard!' );
    		return;
    	} 
    	
    	$th1          = current( $_SESSION['s_clipboard'] );
    	$insertelem   = 1;
    	$tmp_tablename= current($th1);
    	$id0          = next($th1);
    	
    	if ($tmp_tablename === "PROJ_ORI") {
    		// use other link for paste only *** hier ***
    		echo '[<a href="obj.proj.duplicate.php?recursive=yes&proj_id='.$id0.'">Create new project(s) recursivly</a>] (does NOT create new objects, only copies the links!) <br><br>';
    		echo '[<a href="obj.proj.duplicate.php?recursive=no&proj_id='.$id0.'">Create only this project</a>] (without objects) ';
    		$this->stopforward = 1;
    		return;
    	}
    	
    	if ( !cct_access_has2($tmp_tablename) ) {
    		$error->set( $FUNCNAME, 2, 'You must choose a business object.' );
    		return;
    	}
    	
    	$newurl = "glob.obj.crea_wiz.php?tablename=".$tmp_tablename."&paste_new=1";
    	js__location_replace( $newurl, "create new object");
    	exit;	
    	
    	
    }
    
    function f_cut( &$sel ) {
    	global  $error;
    	$FUNCNAME = "f_cut";
    	
    	$cntelem=0;
    	if ( !sizeof ($sel) ) {
    		$error->set( $FUNCNAME, 1, "No objects in clipboard!");
    		return;
    	}
    	
    	if ( sizeof($_SESSION['s_clipboard']) ) reset ($_SESSION['s_clipboard']);
    	$_SESSION['s_clipboard'] = array();
    	
    	foreach( $sel as $th0=>$th1) { 
    		$tmp_table=$th0;
    		foreach( $th1 as $tmp_id=>$xth1) {
    			$_SESSION['s_clipboard'][]=array( "tab"=>$tmp_table, "ida"=>$tmp_id, "idb"=>"", "idc"=>"" );	 
    			$cntelem++;	
    		}
    	}
    	$this->xinfo_show( "INFO", "$cntelem objects saved in clipboard (cut from project is prepared).");
    	
    }
    
    function f_new($tablename) {
    	global  $error;
    	$FUNCNAME = "f_new";
    	
    	$proj_id = $this->proj_id;
    	$_SESSION['s_sessVars']["boProjSel"] = $proj_id;
    	
    	if ( $tablename != "PROJ") { 
    		$newurl = "edit.insert.php?tablename=".$tablename;
    		js__location_replace( $newurl, "new object" );
    		exit;
    	} else {
    		$newurl = "obj.proj.edname.php?mother_proj=".$proj_id."&action=create";
    		js__location_replace( $newurl, "new project" );
    		exit;
    	}
    }

}


// -----------------------------------------------------------
$infarr=array();
$infarr["copy"]       = "Copy selected objects to clipboard";
$infarr["copyplus"]   = "Additional copy";
$infarr["cut"]        = "Cut selected objects from project to clipboard";
$infarr["minusclip"]  = "Remove objects-in-clipboard from project";
$infarr["del"]        = "Unlink or delete selected objects";
$infarr["delBroken"]  = "Delete broken links from project-tree";
$infarr["link_paste"] = "Paste objects from clipboard to project";
$infarr["new"]        = "Create new object";
$infarr["paste_new"]  = "Paste object from clipboard as new";
$infarr["select"]     = "Put selected objects (of one type) to the list view";
$infarr["tocut"]      = "Transform a clipboard-copy action to a CUT action";


$retval 	 = 0;
$stopforward = 0;
$stopreason  = "";
$error  = & ErrorHandler::get();
$sql    = logon2( $_SERVER['PHP_SELF'] );
$sql2   = logon2( $_SERVER['PHP_SELF'] );

$go=$_REQUEST['go'];
$proj_id=$_REQUEST['proj_id'];
$opt=$_REQUEST['opt'];
$actio_elem=$_REQUEST['actio_elem'];
$tablename=$_REQUEST['tablename'];
$doflag=$_REQUEST['doflag'];
$state=$_REQUEST['state'];
$sel=$_REQUEST['sel'];

$proj_tab_nice_fcap = tablename_nice_fcap('PROJ');
$proj_tab_nice = tablename_nice2('PROJ');


$proj_use=$proj_id;
if ( $proj_id=="NULL" ) {
	$proj_use=NULL;
}

$title  = $proj_tab_nice_fcap.": object actions (copy, paste, cut, del)";
$infoarr = NULL; 
$infoarr["title"]    = $title;
$infoarr["title_sh"] = $proj_tab_nice_fcap." action";
$infoarr["scriptID"] = "";
$infoarr["form_type"]= "obj";

$infoarr["obj_name"] = "PROJ";
$infoarr["obj_id"]   = $proj_use;

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

if (empty($proj_id)) {
  $pagelib->htmlFoot('INFO', 'Please give PROJ_ID!');
}

$mainlib = new oProjElemAct($proj_id);

if ( $opt["objdeepdel"] ) {
   $stopforward = 1;
}

/*
 * 
if ($proj_id!="NULL") { 
	$proj_name = obj_nice_name( $sql, "PROJ", $proj_id);
} else {
	$proj_name = "/root";
}
echo "<img src=0.gif height=5 width=1><UL>\n";
echo "<B><a href=\"edit.tmpl.php?t=PROJ&id=".$proj_id."\">".
	"&lt;&lt; Back</a></B>&nbsp;&nbsp; <img src=\"images/icon.PROJ.gif\" border=0>".
	" project <b>".$proj_name."</b><br>\n";
*/

$tmpinfo = $infarr[$actio_elem]; 
if ($tmpinfo=="") {
    htmlFoot("Error", "Action: ".$actio_elem." unknown");
} 
echo "<font color=gray size=+1>Action: <B>".$tmpinfo."</B></font>\n";
echo "<br><ul>\n";
 

if (empty($actio_elem)) {
  info_out("ERROR", "action parameters missing!");
  return -1;
} 

if (empty($doflag)) $doflag = "";
if (empty($sel)) $sel = array();


$o_rights = access_check($sql, 'PROJ', $proj_id);     
$t_rights = tableAccessCheck( $sql, "PROJ");

if ( $actio_elem == "copyplus" ) {
	$actio_elem = "copy";
	$actio_extra_param = "plus";
} else {
	$actio_extra_param = "";
}

// clear CUT-info
if ($actio_elem!="link_paste") $_SESSION['s_sessVars']["clipActCutProj"]="";

$action_found = 1;
switch  ( $actio_elem ) { /* actions without permission */
	case "copy":
        $mainlib->f_copy( $sel, $actio_extra_param );
		if ($error->Got(READONLY))  {
			$error->printAllEasy();
			return;
		}
		break;
        
   case "select":
        // select only one TABLE-type
        $mainlib->f_select($sql, $sel);
		if ($error->Got(READONLY))  {
			$error->printAllEasy();
			return;
		}
        break;
		
	default:
		$action_found=0;
		break;
}

if ( !$action_found ) {

  if ( !$t_rights["write"] ) {
      $mainlib->xinfo_show("ERROR", "no role right \"WRITE\" to manipulate a ".$proj_tab_nice."! Stopped.");
	htmlFoot();
  }
  
  if ( !$o_rights["insert"] ) {
      $mainlib->xinfo_show("ERROR", "no INSERT perimission to this ".$proj_tab_nice."!");
	 htmlFoot();
  }
  
  switch ( $actio_elem ) {
	case "new" :
	  	$mainlib->f_new($tablename);
		break;
		
	case "minusclip":
		// Remove objects-in-clipboard from project
		$mainlib->f_minus_clip($sql, $go);
		if ($error->Got(READONLY))  {
			$error->printAllEasy();
			return;
		}
		break;
		
	case "link_paste":
		$mainlib->f_paste($sql);
		$_SESSION['s_sessVars']["clipActCutProj"] = NULL; // remove CUT-info !!!
		if ($error->Got(READONLY))  {
			$error->printAllEasy();
			return;
		}
		break;
        
	case "paste_new":
		$mainlib->f_pastenew();
		if ($error->Got(READONLY))  {
			$error->printAllEasy();
			return;
		}
		break;
		
	case "cut":
	    $mainlib->f_cut( $sel );
		if ($error->Got(READONLY))  {
			$error->printAllEasy();
			return;
		}
		break;

	case "tocut":
		$mainlib->f_tocut();
		break;

	case "del":
        require_once("subs/obj.proj.delgui.inc");
		$delProjObj = new projDelGuiC( );
        $retval = $delProjObj->projdelgui($sql, $proj_id, $sel, $state, $doflag, $opt);
	    if ( $retval>0 ) htmlFoot(); // stop script!
        break;
		
	case "delBroken":
        require_once("subs/obj.proj.delBroken.inc");
		$delProjBrokObj = new projDelBrokenC( );
		$delurl = "obj.proj.elementact.php?proj_id=".$proj_id."&actio_elem=delBroken";
        $retval = $delProjBrokObj->projgui($sql, $sql2, $proj_id, $state, $delurl, $opt);
	    $stopforward = 1;
		if ( $retval>0 ) htmlFoot(); // stop script!
        break;
		
	 default:
	   echo "ERROR: define an action!<br>";
	   break;	
	 }
   
}

echo "<br>";
// echo "<hr width=300 align=left>\n";

//  - save Proj-ID of last CUT-action, the next "paste" will delete
//     the last cutted elements in project


if ($actio_elem =="cut" or $actio_elem =="tocut") $_SESSION['s_sessVars']["clipActCutProj"]=$proj_id;	

$newurl = "edit.tmpl.php?t=PROJ&id=".$proj_id;
if ( $retval<0 ) {
    $stopreason= "due to previous error ...";
    $stopforward = 2;
}  

if ( $mainlib->stopforward>0 )  $stopforward =  $mainlib->stopforward;
if ( $opt["stopforward"] )      $stopforward = 1;

if ($stopforward) {
	if ($stopreason!=NULL) echo "Stop-reason: ".$stopreason."<br />\n";
	$pagelib->htmlFoot();
}


js__location_replace( $newurl, "back to ".$proj_tab_nice);

$pagelib->htmlFoot();
