<?php
/**
 * create a new project
 * - edit project name, notes, extra_object_params
 * @namespace core::gui
 * @package obj.proj.edname.php
 * @swreq   UREQ:0000994: g > create/update project 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param 
 *  $proj_id        = project ID (if exists for update)
    [$action]       = ["update"], "create": mother_proj must be set
    [$mother_proj]  = mother ID, if $action = "create"; number or "NULL" (if root directory)
    [vals][NAME]    - name
    [vals][NOTES]    proj_notes
    [vals][TYPEX]    - only for root allowed
	[proj_notekeys]  KEYS for project
	[optGoChild]	0|1 : go to new project
    [sel_extra_class_id] comes from _select_class_html()
    [arguobj]            comes from _select_class_html()
    [xargu]
    [$go]
    
 *  @version $Header: trunk/src/www/pionir/obj.proj.edname.php 59 2018-11-21 09:04:09Z $
 */


// extract($_REQUEST); 
session_start(); 


require_once ('reqnormal.inc');
include_once('edit.sub.inc');
require_once('table_access.inc');
require_once('insert.inc');      
require_once('class.history.inc');
require_once('o.PROJ.subs.inc');
require_once('func_form.inc');
require_once('date_funcs.inc');
require_once ("javascript.inc");
require_once ("f.help.inc");
require_once('o.PROJ.modi.inc');
require_once ('f.update.inc');

class oPROJ_newProjGui {
	
	var $action;
	var $proj_id;
	var $mother_proj;
	var $ori_argu;
	var $argu_in;
	
function __construct(&$sqlo, $action, $proj_id, $go) {
    global $varcol;
    
    $varcol  = & Varcols::get();
    
	$this->action=$action;
	$this->proj_id=$proj_id;
	$this->go = $go;
	
	$this->mother_proj=$_REQUEST['mother_proj'];
	
	
	
	$this->sel_extra_class_id = $_REQUEST['sel_extra_class_id'];
	
	$this->argu_in = array();
	$this->argu_in['vals'] = array();
	$this->argu_in['vals'] = $_REQUEST['vals'];
	
	$this->argu_in['xargu']   = $_REQUEST['xargu'];
	$this->argu_in['arguobj'] = $_REQUEST['arguobj'];
	$this->argu_in['proj_notekeys'] = $_REQUEST['proj_notekeys'];
	
	$this->ori_argu = array();
	$this->ori_argu['vals']  = array();
    $this->ori_argu['proj_eo_id']     = 0;
    
    $this->fieldCondition = array(
    	'NAME' =>array('maxlength'=>200),
    	'NOTES'=>array('maxlength'=>4000)
    );
  
    if ($proj_id) {
        $sqlo->Quesel('* FROM proj WHERE proj_id = '.$proj_id);
        $sqlo->ReadArray(); 
        $this->ori_argu['vals']  = $sqlo->RowData; 
        $this->ori_argu['proj_eo_id']    = $sqlo->RowData['EXTRA_OBJ_ID'];
    }
    
    $this->varcol_classes = $varcol->get_class_nice_names('PROJ');
    
  
}

function get_ori_argu() {
	return $this->ori_argu;
}

function get_input_argu() {
	return array(
		'vals' => $this->argu_in['vals'],
		'proj_eo_id'=>$this->ori_argu['proj_eo_id']
	);
}

function _showRow ($name, $fieldtext, $with_hidden=0) {
	$this->_startRow($name);
	echo "</td><td>";
	echo $fieldtext;
	if ($with_hidden) {
	    echo '<input type="hidden" name="dummy" value="">',"\n"; // to keep FORM-ELEMENT-InputRemote structure intact 
	}
	echo "</td></tr>\n";
}

/**
 * 
string $text
 * @param int $showflag - REQUIRED 
 */
function _startRow($text, $showflag=NULL) {

		echo "<tr><td valign=top NOWRAP><B>";
		if ($showflag==1) 
			echo "".$text." <img src=\"images/redstar.gif\">";
		else 
			echo "<font color=gray>".$text."</font>";
		echo "</B>";
}

function _arrKeyValCpy($inarray) {
	$outarr = array();
	foreach( $inarray as $key=>$val) {
		$outarr[$val] = $val;
	}
	return $outarr;
}


function _KeysHas( $notes ) {
	// FUNCTION: extract from NOTES some keywords
	// FORMAT: - keyword-line starts with KEYS: than WHITE_SPACE separated KEYWORDS
	/*			text1 text2
				KEYS: synthese ramen oridant karbid
	*/
	$STARTWORD = "KEYS:";
	$STARTDIFF = strlen($STARTWORD);
	if ( ($pos = strpos($notes, $STARTWORD)) === FALSE ) return(0);
	return (1);
}

/* void  select_class_html(string tablename) */
function _select_class_html()
{

	$classes = $this->varcol_classes;
	
    echo '<select name="sel_extra_class_id" size=1>'."\n";
    echo '<option value="null">- normal folder -'."\n";

    foreach ($classes as $class_id => $class_name) {
        echo '<option value="'. $class_id.'">'.$class_name."\n";
    }
    echo '</select>';
}

function _newClassObjAct($proj_eo_id, $extra_class_id) {
	// FUNCTION: special new class
	global $varcol;
	
	$classname  = $varcol->class_id_to_name( $extra_class_id );
	if ($classname=="experiment") {
		// set default exp_date
		$nowdate = date_unix2datestr( time() , 3);
		$values  = array("exp_date"=>$nowdate);
		$varcol->update_by_name($proj_eo_id, $values);
	} 
}

/**
 * show attribute form
 * @param object $sql
 * @param object $sql2
 * @param array $argu_in
 * @return -
 */
function form1( &$sql, &$sql2, $argu_in) {
	global $varcol, $error;

	
	$action     =$this->action;
	$proj_id    =$this->proj_id;
	$mother_proj=$this->mother_proj;
	
    $proj_name  = $argu_in['vals']['NAME'];
    $proj_notes = $argu_in['vals']['NOTES'];
    $proj_eo_id = $argu_in['proj_eo_id']; 
    
    
	$tabBackColor = "#FFFFFF"; //"#6699FF";
	$TextAreaRows = 9;
	$elemCnt = 0; // count number of fields
	
	if ($action=="create") {
		 //$title = 'Create folder';
		 $saveText = "Create";
	} else {
		 $saveText = "Save";
	}
	
    echo "\n";
	echo '<form method="post" name="editform" action="'.$_SERVER['PHP_SELF'].'">'."\n";
	echo '<table cellpadding=1 cellspacing=0 border=0 bgcolor='.$tabBackColor.'><tr><td>'."\n";
    echo '<table cellpadding=3 cellspacing=0 border=0 bgcolor=#FFFFFF><tr bgcolor='.$tabBackColor.'>'."\n";
    echo '<td>&nbsp;</td>';
    echo '<td><input type="submit" value="'.$saveText.'" class="yButton">';
	if ($action != "create") {  
		echo '<input type="hidden" name="dummy" value="">',"\n"; // to keep FORM-ELEMENT-InputRemote structure intact 
	} else {
		echo "&nbsp;&nbsp;&nbsp;<input type=button name='dummy' class='yButton' ".
		   "value='Create and go there &gt;&gt;' onclick=\"document.editform.optGoChild.value=1; document.editform.submit();\">\n";
	}
	$elemCnt++;
	
	echo '</td></tr>'."\n";
	
	$this->_showRow ("","");
    $this->_startRow("Name",1);
    echo '<td><input name="vals[NAME]" value="'.$proj_name.
    	  '" size="72" maxlength="'. $this->fieldCondition['NAME']['maxlength'].'">'; // set focus after end of form!
	echo '<input type="hidden" name="dummy" value="">',"\n"; // to keep FORM-ELEMENT-InputRemote structure intact 
	$elemCnt++;
    echo '</td></tr>'."\n";
	
	$this->_showRow ("","");
	
	$hlpopt=array("object"=>"icon");
	$notesHelpText = fHelpC::link_show("single_sheet_edit.html#notesfield", "formatting help", $hlpopt);	
	$this->_startRow("Notes<br><img src=0.gif height=25 width=10>".$notesHelpText);
    echo '<td valign=top><textarea rows="'.$TextAreaRows.'" cols="72" name="vals[NOTES]">'.$proj_notes.'</textarea>';
	echo '<input type="hidden" name="dummy" value="">',"\n"; // to keep FORM-ELEMENT-InputRemote structure intact
	$elemCnt++;
	// NEW: do not show keys anymore, no need for this anymore ...
	echo '</td></tr>'."\n";
	
	if ( glob_isAdmin() ) {
    	
	    $this->_showRow ("","");
	    $this->_startRow("Type",0);
	    echo '<td><input name="vals[TYPEX]" value="'.$argu_in['vals']['TYPEX'].'" size="2" >'; 
	    echo '<input type="hidden" name="dummy" value="">',"\n"; // to keep FORM-ELEMENT-InputRemote structure intact
	    $elemCnt++;
	    echo '</td></tr>'."\n";
	} else {
	    $typex = $this->ori_argu['vals']['TYPEX'];
	    if ($typex==1) {
	        $this->_showRow ("Type","Category");
	    }
	}
	
	if (!$proj_eo_id and sizeof($this->varcol_classes) ) {
	    
		$this->_startRow("Class");
		echo "<td>";
		$this->_select_class_html();
		$elemCnt++;
		echo "</td></tr>\n";
	}
    
    
    if ($proj_eo_id) {
	
	  $this->_startRow("Class");
      echo '<td><table border=0>'."\n";
      $varcol_val = $varcol->select_by_name($proj_eo_id);
	  if ($error->printLast()) return;
      $extraobj_o = array( 'extra_obj_id'   => $proj_eo_id,
						   'extra_class_id' => $varcol_val['extra_class_id'],
						   'arguobj'        => $varcol_val['values']);

      $XFormLib = new fEditXobjForm('PROJ');
	  $XFormLib->showCols( $sql, $elemCnt, 1, $extraobj_o, 1, 1 );

	  echo '</table>'."\n";
      echo '</td></tr>'."\n";
	  
      echo '<input type="hidden" name="proj_eo_id" value="'.$proj_eo_id.'">'."\n";
    }
	
	echo '</table>'."\n";
	echo "</td></tr></table>\n";
	
	
	echo '<input type="hidden" name="proj_id" value="'.$proj_id.'">'."\n"; 
	echo '<input type="hidden" name="mother_proj" value="'.$mother_proj.'">'."\n";         
    echo '<input type="hidden" name="action" value="'.$action.'">'."\n";
    echo '<input type="hidden" name="go" value="1">'."\n";
	echo '<input type="hidden" name="optGoChild" value="">'."\n";
    echo '</form>'."\n";
    
    js_formFocus('vals[NAME]');
    
    if ($action == "create") {  
	    echo '<br /><br />'."\n";
	    htmlInfoBox( "Short info", "", "open", "CALM" );
	    ?>
	    the new folder inherits group-rights of the mother folder.<br> 
	    <?php 
		htmlInfoBox( "", "", "close" );
    }
}

/**
 * pre checks before update/insert action
 * OUT: 
 * - $this->mother_proj
 * @param object $sqlo
 * @throws array errors
 * @return -
 */
function preCheck(&$sqlo) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$action  = $this->action;
	$proj_id = $this->proj_id;
	
	$this->argu_in['vals']['NAME'] = trim($this->argu_in['vals']['NAME']);
	$proj_name = $this->argu_in['vals']['NAME'];
	
	
    if ( $proj_name=="" ) { 
    	$error->set( $FUNCNAME, 1, 'Please give a name' );
		return;
    }
    
    foreach ( $this->fieldCondition as $colname => $valarr ) {
   		if ($valarr['maxlength']>0) {
   			if ( strlen($this->argu_in['vals'][$colname]) > $valarr['maxlength'] ) {
   				$error->set( $FUNCNAME, 4, 'Column: "'.$colname.'": text too long; max length allowed: '.$valarr['maxlength'] );
				return;
   			}
   		}
    }

    if ( !glob_isAdmin() ) {
        unset( $this->argu_in['vals']['TYPEX']);  // not allowed ...
    }
    
    
    // filter chars, which are not allowed ...
    $pattern_raw   = "\\\\/:*";
    $pattern_human_arr = array('\\', '/',':','*'); // human readable string
	$pattern       = '@['.$pattern_raw.']@';
	$bad_found = preg_match($pattern, $proj_name);
	if ( $bad_found ) { 
	    $patt_arr2=array();
	    foreach( $pattern_human_arr as $charx) {
	        $patt_arr2[]='"'.$charx.'"';
	    }
	    $pattern_human_str= implode(", ", $patt_arr2);
    	$error->set( $FUNCNAME, 2, 'Bad name. Following characters are NOT allowed: '.htmlspecialchars($pattern_human_str) );
		return;
        
    }
    
    // check for same names in project
    $xtraCond = ""; 
    if ($action != "create" ) {
        $xtraCond = " AND PROJ_ID!=".$proj_id;
        // get mother-ID
        $sqls="PRO_PROJ_ID from PROJ where PROJ_ID=".$proj_id;
        $sqlo->Quesel($sqls);
        if ( $sqlo->ReadRow() ) { 
             $mother_projtmp = $sqlo->RowData[0]; 
             $this->mother_proj = $mother_projtmp;
             if ( !$mother_projtmp ) $this->mother_proj = "NULL"; 
        }
    }
    
    $sqls="PROJ_ID from PROJ where PRO_PROJ_ID=". $this->mother_proj . $xtraCond . 
    	  " AND name=".$sqlo->addQuotes($proj_name);
    $sqlo->Quesel($sqls);
    if ( $sqlo->ReadRow() ) {  
        $error->set( $FUNCNAME, 3, 'An other folder with this name already exists in mother-folder. Please give an other name.' );
		return;
    }
    
}


/**
 * create a new project
 * @param  $sqlo
 * @param  $mother_proj
 * @param  array $argus predefined argument values
 * @param  $proj_name
 * @param  $proj_notes
 */
function createProj(&$sqlo, $mother_proj, $argus) {
	
	$proj_lib = new oPROJ_modi();
	$proj_id = $proj_lib->makeProjWithMotherRights($sqlo, $mother_proj, $argus);
	if ( $proj_id<=0 ) {
		  echo "<font color=red><B>Error: </B></font>Creation of folder failed<br>\n";
	}
	return $proj_id;
}

/**
 * check permissions to update or insert a project
 * @param  $sqlo
 * @return $isok 0,1
 */
function checkPermission(&$sqlo) {
	$mother_proj= $this->mother_proj;
	$proj_id    = $this->proj_id;
  
	$t_rights = tableAccessCheck( $sqlo, 'PROJ' );
	if ( $t_rights['write'] != 1 ) {
		tableAccessMsg( "folder", 'write' );
		return 0;
	}  

  if ($this->action == "create") {   
    if ( !$mother_proj ) {  
        info_out('ERROR', 'Mother-folder-ID missing.');    
        return 0;
    }
    if ( $t_rights['insert'] != 1 ) {
	    tableAccessMsg( "folder", 'insert' );
	    return 0;
    }
    
    $o_rights = access_check($sqlo, 'PROJ', $mother_proj);
    if (!$o_rights['insert']) {
        info_out('ERROR', "No insert permission in mother-folder [ID:$mother_proj].");
        return 0;
    } 
    
    
  } else {
    if ( !$proj_id ) {  
        info_out('ERROR', 'Folder-ID missing.');    
        return 0;
    }
    
    
    $o_rights = access_check($sqlo, 'PROJ', $proj_id);
    if (!$o_rights['write']) {
        info_out('ERROR', "No write permission in folder [ID:$proj_id].");
        return 0;
    }
  }
  return 1;
} 

/**
 * create / update
 * @param $sql
 */
function createUpdate(&$sql) {
	global $varcol, $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$action     = $this->action;
	$argus      = $this->argu_in['vals'];
	
	$sel_extra_class_id = $this->sel_extra_class_id;
	$xargu         = $this->argu_in['xargu'];
	$arguobj       = $this->argu_in['arguobj'];
	$proj_notekeys = $this->argu_in['proj_notekeys'];
	$proj_eo_id    = $this->ori_argu['proj_eo_id'];
	
	
    $new_class  = false;
            
    if ( isset($sel_extra_class_id) ) {
    
      $new_class = true; // either new real class or new 'normal proj'
      if (is_numeric($sel_extra_class_id)) { // extra_class chosen, not 'normal proj'
		$proj_eo_id = $varcol->create_empty_extra_obj_by_id($sel_extra_class_id);
	    if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'error on EXTRA_OBJ.' );
			return;
		}
		$argus['EXTRA_OBJ_ID'] = $proj_eo_id;
		
		// extra_class_specials
		$this->_newClassObjAct($proj_eo_id, $sel_extra_class_id);
        if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 2, 'error on EXTRA_OBJ (2).' );
			return;
		}
      }
    } else { // class already chosen
    	if (!$xargu['CLASS']) // class removed
      		$argus['EXTRA_OBJ_ID'] = '';
      	else
      		$argus['EXTRA_OBJ_ID'] = $proj_eo_id;
    }


    if (!$new_class && $xargu['CLASS']) { // update existing class which was not deleted
      $varcol->update_by_name($proj_eo_id, $arguobj);
      if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 3, 'error on update of EXTRA_OBJ.' );
			return;
	  }
    }  
    
    if ( is_array($proj_notekeys) ) {
		
		$proj_notes = $this->argu_in['vals']['NOTES'];
		$hasKeys    = $this->_KeysHas( $proj_notes );
		if (!$hasKeys) $proj_notes .= "\nKEYS:";
		foreach( $proj_notekeys as $key=>$val) {
			// if KEYWORD does not exist in Notes, ADD it !
			if (strstr($proj_notes, $val) == NULL) {
				$proj_notes .= " " . $val;
			}
		}	
		$argus['NOTES'] = $proj_notes;
    }
      
    if ( $action == "create" ) {   
        
        
        
    	$this->proj_id = $this->createProj($sql, $this->mother_proj, $argus);
    	if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 4, 'error folder SQL-creation.' );
			return;
	  	}
    	
    } else {  
      
      $argus['PROJ_ID']      = $this->proj_id;
      $retVal = gObjUpdate::update_row($sql, 'PROJ', $argus);
    } 
    
    return array('proj_id'=>$this->proj_id, 'proj_eo_id'=>$proj_eo_id);

}

}

// ----------------------------------------------------------------------


$proj_id     = $_REQUEST['proj_id'];
$mother_proj = $_REQUEST['mother_proj'];
$action      = $_REQUEST['action'];
$go			 = $_REQUEST['go'];
$optGoChild  = $_REQUEST['optGoChild'];

if (!isset($go)) $go = 0;

$back_id = $proj_id;
if ($action == "create" ) $back_id = $mother_proj;

$title = 'Edit parameters';
if ($action=="create") {
	 $title = 'Create Folder';
}


$sql  = logon2( $_SERVER['PHP_SELF'] );
$sql2 = logon2( );
$error    = & ErrorHandler::get();
$varcol   = & Varcols::get();

$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr['help_url'] = 'o.PROJ.html';
$infoarr["obj_name"] = "PROJ";

if ($back_id!="NULL") { 
	$infoarr["obj_id"]   = $back_id;
	$infoarr["show_name"]= 1;
}
$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

$mainlib = new oPROJ_newProjGui($sql, $action, $proj_id, $go);

// need for $XFormLib->showCols()
// TBD: change this structure, create closer relation!
js_formSel(); 

echo "<ul>\n";


$isok = $mainlib->checkPermission($sql);
if (!$isok) $pagelib->htmlFoot();

$ori_argu = $mainlib->get_ori_argu();
  
// tests if create/update possible
if ( $go > 0 ) {
	$mainlib->preCheck($sql);
	if ($error->Got(READONLY))  {
		$error->printAllEasy();
		echo "<br>\n";
		$error->reset();
		
		$newArgu = $mainlib->get_input_argu();
		$mainlib->form1( $sql, $sql2, $newArgu);
    	$pagelib->htmlFoot();
	}
}
   

if ( !$go ) {  
  	$mainlib->form1( $sql, $sql2, $ori_argu);
    $pagelib->htmlFoot();
}
  
//
// update, create data
//
  
$result = $mainlib->createUpdate($sql);
$pagelib->chkErrStop();

$proj_id    = $result['proj_id'];
$proj_eo_id = $result['proj_eo_id'];
if (isset($proj_id)) {
    $hist_obj = new historyc();
    $hist_obj->historycheck( 'PROJ', $proj_id );
}
  
  $forwardToProjNavi = 1;
  
  if ($proj_eo_id)  $forwardToProjNavi = 0;  // no extra object  ?
  if ($action=="create" AND $optGoChild) {   // create and go to project ...
		$forwardToProjNavi = 1; 
  }
  
  if ($forwardToProjNavi)  {
  
		$next_id = $proj_id;
		if ($action == "create") { 
			$next_id = $mother_proj;
			if ($optGoChild) $next_id = $proj_id;
		}
		
		js__location_replace( "edit.tmpl.php?t=PROJ&id=".$next_id, 'go to folder' ); 
		exit;
  
  } else {
		
		js__location_replace( "obj.proj.edname.php?proj_id=".$proj_id, 'go back to folder' ); 
		exit;
  }



htmlFoot();
