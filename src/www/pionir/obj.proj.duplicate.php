<?php
/**
 * duplicate a given project with or without contents (business-objects (other than proj) get only linked, not copied!)
 * 
 * set LIMIT_OF_PROJECT: 100
 * set LIMIT_OF_ELEMENTS: 100
 * $Header: trunk/src/www/pionir/obj.proj.duplicate.php 59 2018-11-21 09:04:09Z $
 * @package obj.proj.duplicate.php
 * @swreq UREQ:0003653 o.PROJ > duplicate project structure 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param 
 * 	 $recursive ... [yes/no] duplicate with or without contents
     $proj_id   ... id of project to duplicate
     $go ... action flag
       0 : prepare
       1 : do inserts
 */
session_start(); 


require_once ('reqnormal.inc');
require_once ('subs/obj.proj.manage.inc');
require_once ("o.PROJ.tree.inc");
require_once ('insert.inc');
require_once ("f.objview.inc");
require_once ('o.DB_USER.subs.inc');
require_once 'func_form.inc';

class oProjDuplC {
	
	private $LIMIT_OF_PROJECT  = 100;  // max LIMIT_OF_PROJECT projects allowed
	private $LIMIT_OF_ELEMENTS = 100; // max LIMIT_OF_ELEMENTS elements allowed
	
	/**
	 * 
	 * @param int $go
	 * 	 0 : prepare
	 *   1 : do insert
	 */
	function __construct($proj_id, $recursive, $go) {
		$this->go = $go;
		$this->proj_id=$proj_id;
		$this->recursive=$recursive;
		
		$this->projMangLib = new oProjManageC();
	}
		
	
	function pasteAsNew( &$sql, $new_mother_proj_id, $proj_id) {
		
		$this->projMangLib->setProjId($new_mother_proj_id);
	    $new_proj_id = $this->projMangLib->pj_pasteAsNew( $sql, $proj_id );
		return ($new_proj_id);
	}
	
	private function showProj($new_proj_id, $proj_name) {
		
		if ($this->go>=0) {
		
			echo '<li><a href="edit.tmpl.php?t=PROJ&id='.$new_proj_id.'" target="cctinfo">';
			echo '<img src="images/icon.PROJ.gif" border="0"> '.$proj_name.'</a> -- ';
		} else {
			echo '.';
		}
	}
	
	private function showOK() {
		if ($this->go) {
			echo '<font color="#00ff00">okay</font>';
		} else {
			echo '<font color="#909090">... preparation</font>';
		}
		echo '<ul style="list-style-type: none;">';
	}
	
	private function showSubObjsClose() {
		if ($this->go>=0) {
			echo '</ul></li>',"\n";
		}
	}
	
	private function showClose() {
		if ($this->go>=0) {
			
		} else {
			echo "<br>\n";
		}
	}
	
	private function paste_as_new_recursive( 
		&$sql,    
		$proj_id,       
		$proj_name,    
		$new_mother_proj_id,    
	 	&$proj_tree	//  array proj_tree
		) {
	#        warning! recursive function
	# descr: duplicates tree structure recursivly through a project
	# input: proj_id   ... id of the current project
	#        proj_name ... name of current top-project
	#        new_mother_proj_id ... id of the project where to insert the subtree
	#        proj_tree ... array containing whole project tree
	# errors:
	#        db_access 2 error in query
	
	     $error     = & ErrorHandler::get();
	     $FUNCNAME= "paste_as_new_recursive";
	     $got_error = 0;
	
	     // duplicate project
		 $this->projMangLib->setProjId($new_mother_proj_id);
		 
		 if ($this->go) {
	     	$new_proj_id = $this->projMangLib->pj_pasteAsNew( $sql, $proj_id );
		 }
		 
	     $this->showProj($new_proj_id, $proj_name);
	     if ( $error->got(CCT_ERROR_READONLY) ) {
	         $error->set($FUNCNAME, 1, "Error in pasteAsNew($proj_id)");
	         return;
	     }
	
	     foreach( $proj_tree[$proj_id]['elements'] as $tab_name=>$tab_prim_ids) {
	         foreach( $tab_prim_ids as $dummy=>$tab_prim_id) {
	         	
	         	if ($this->go) {
		             $argu = array();
		             $argu['PROJ_ID'] = $new_proj_id;
		             $argu['TABLE_NAME'] = $tab_name;
		             $argu['PRIM_KEY'] = $tab_prim_id;
		             $argu['IS_LINK'] = 1;
		             $ret = insert_row($sql, 'PROJ_HAS_ELEM', $argu);
		             if ($ret < 0) {
						 // only necessary because insert_row does not support error-handler
		                 $error->set($FUNCNAME, 2, 'insert error: of table:'.$tab_name.' ID:'.$tab_prim_id);
		                 return;
		             }
	         	}
	         }
	     }
	
	     $this->showOK();
	  
	     
	     foreach( $proj_tree[$proj_id]["projects"] as $proj_id_sub=>$proj_name_sub)  { // show subprojects
	         $this->paste_as_new_recursive( $sql, $proj_id_sub, $proj_name_sub, $new_proj_id, $proj_tree);
		 	 if ( $error->got(CCT_ERROR_READONLY) ) { 
				$error->set($FUNCNAME, 3, "Error from paste_as_new_recursive(name:$proj_name_sub) ");
				return;
			 }
		 } 
		 
		 $this->showSubObjsClose();
		 
		 
	     return $new_proj_id;
	}
	
	/**
	 * check limits of copied elements
	 * @param  $sqlo
	 * @param  $sizeProjArr
	 * @param  $numElem
	 */
	private function checkLimits(&$sqlo, $sizeProjArr, $numElem) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		// get vario-attributes of user
		
		$userLib = new DB_userC();
		$userLib->setUserID($_SESSION['sec']['db_user_id']);
		
		$MAX_PROJ = $userLib->getVarioVal($sqlo, 'obj.proj.duplicate.MAX_PROJ', $this->LIMIT_OF_PROJECT);
		$MAX_ELEM = $userLib->getVarioVal($sqlo, 'obj.proj.duplicate.MAX_ELEM', $this->LIMIT_OF_ELEMENTS);
		
		echo '... Your Limits: LIMIT_OF_PROJECT: '.$MAX_PROJ.'; LIMIT_OF_ELEMENTS: '.$MAX_ELEM."<br />\n";
		
		
		if ($sizeProjArr>$MAX_PROJ) {
			$error->set( $FUNCNAME, 2, 'Too many sub-projects selected ('.$sizeProjArr.'), max '.$MAX_PROJ.' allowed.'.
					' Admin can help: set vario-element in DB_USER key=obj.proj.duplicate.MAX_PROJ' );
			return;
		}
		
		
		if ($numElem>$MAX_ELEM) {
			$error->set( $FUNCNAME, 2, 'Too many elements in sub-projects ('.$numElem.'), max '.$MAX_ELEM.' elements allowed.'.
						' Admin can help: set vario-element in DB_USER key=obj.proj.duplicate.MAX_ELEM' );
			return;
		}
	}
	
	function paste_rec_start( &$sql, $proj_id, $proj_name, $new_mother_proj_id, $proj_tree ) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		// analyse tree
		if (!sizeof($proj_tree)) {
			$error->set( $FUNCNAME, 1, 'no projects to copy ...' );
			return;
		}
		
		
		$numElem=0;
		reset ($proj_tree);
		foreach( $proj_tree as $projid=>$valarr) {
			$elemArray = $valarr['elements'];
			$numElem = $numElem + sizeof($elemArray);
		}
		$sizeProjArr = sizeof($proj_tree);
		
		$this->checkLimits($sql, $sizeProjArr, $numElem);
		if ($error->Got(READONLY))  {
			return;
		}
		
		
		echo '... Statistics: Number of projects to be copied: '.$sizeProjArr.'; Number of element-links to be copied: '.$numElem."<br>\n";
		
		echo '<ul style="list-style-type: none;">';
		$this->paste_as_new_recursive( $sql, $proj_id, $proj_name, $new_mother_proj_id, $proj_tree );
		
		$this->showClose();
		echo '</ul>';
		
	}
	
	
	function form1() {
		require_once ('func_form.inc');
	
		$initarr   = NULL;
		$initarr["action"]      = $_SERVER['PHP_SELF'];
		$initarr["title"]       = "Copy now";
		$initarr["submittitle"] = "Copy!";
		$initarr["tabwidth"]    = "AUTO";
	
		$hiddenarr = NULL;
		$hiddenarr["proj_id"]     = $this->proj_id;
		$hiddenarr["recursive"]   = $this->recursive;
	
		$formobj = new formc($initarr, $hiddenarr, 0);
	
		$formobj->close( TRUE );
	}

}

$error = & ErrorHandler::get();
$sql   = logon2();

$css = '
a:hover  { text-decoration: underline; color: #0000ff; }
a:active { text-decoration: none;      color: #ff0000; }
a        { text-decoration: none;      color: #000000; }
';

$proj_id   = $_REQUEST['proj_id'];
$recursive = $_REQUEST['recursive'];
$go        = $_REQUEST['go'];

$title = 'Duplicate Project';


$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";

$infoarr["obj_name"] = "PROJ";
$infoarr["obj_id"] = "$proj_id";
$infoarr['css'] = $css;

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>";
if (empty($recursive)) htmlFoot('ALERT', 'Missing recursion-info.');
 if (!(($recursive === "yes") || ($recursive === "no"))) htmlFoot('ALERT', 'wrong datatype of recursion-info.');
 if (empty($proj_id)) htmlFoot('ALERT', 'Missing project_id.');
 if (empty($_SESSION['s_sessVars']['boProjSel'])) htmlFoot('ERROR', 'No project for inserting into selected.');

 $new_mother_proj_id = $_SESSION['s_sessVars']['boProjSel'];
 
 if (!$new_mother_proj_id) htmlFoot('ERROR', 'No Source-project in clipboard.');
 
 $o_rights           = access_check($sql, 'PROJ', $new_mother_proj_id);
 
 $objLinkLib = new fObjViewC();
 $htmlx = $objLinkLib->bo_display( $sql, 'PROJ', $new_mother_proj_id );
 
 echo 'Destination-project: '.$htmlx.'<br />';

 if (!$o_rights['write']) htmlFoot('ERROR', 'You are not allowed to write in to the selected project.');

 $mainlib = new oProjDuplC($proj_id, $recursive, $go);
 
 if ($recursive === "no") { // duplicate without contents
     $new_proj_id = $mainlib->pasteAsNew($sql, $new_mother_proj_id, $proj_id);
     if ($error->printLast()) htmlFoot();
     echo '<script language="JavaScript"><!-- '."\n";
     echo 'location.href="obj.proj.edname.php?proj_id='.$new_proj_id.'"; //--></script>';
     echo '<p><center><a href="obj.proj.edname.php?proj_id='.$new_proj_id.'">done</a></center>';
     htmlFoot();  
 }
 if ($recursive === "yes") { // duplicate with contents
 	
 	$goArray = array(
 			0=>'Prepare Copy',
 			1=>'Do Copy'
 		);
 	
 	echo "<br>\n";
 	$formLib = new FormPageC();
 	$formLib->init($goArray);
 	$formLib->goInfo($go);
 	
     $sql->query('SELECT name FROM proj WHERE proj_id = '.$proj_id);
     if ($error->printLast()) htmlFoot();
     if ($sql->ReadRow())
         $proj_name = $sql->RowData[0];
     else
         htmlFoot('ERROR', 'project with id = "'.$proj_id.'" does not exist.');

     
     info_out('ATTENTION', 'Only the project-structure gets duplicated. The business objects in the projects get only linked to the new projects!');
     
     
     $proj_tree = &oPROJ_tree::tree_with_leafs2array($sql, $proj_id);
     if ($error->printLast()) htmlFoot();

     $sql->SetAutoCommit(false);
     
     $mainlib->paste_rec_start( $sql, $proj_id, $proj_name, $new_mother_proj_id, $proj_tree );
     if ($error->printAll()) {
         $sql->rollback();	
         htmlFoot();
     } else {
         $sql->commit();
         $sql->SetAutoCommit(true);
     }
     
     
     
     if (!$go) {
     	$mainlib->form1();
     }
     

}

htmlFoot("<hr>");
