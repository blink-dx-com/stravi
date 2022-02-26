<?php
/**
 * - object creation wizard for:
 *   - BOs
 *	 - single PK objects (only root allowed)
 *
 * - if object is a BO and no $proj_id is given:
 *   - for the first time in the session: show the project selector !
 *   - not needed if $table_name_conn is given
 * - CALL examples:
 *    WIZ?tablename=XXX&go=5" -- if you want to start the wizardWITHOUT type-seletion-form 
 * SESSION-VARS:
 * - if $_SESSION['userGlob']["o.TABLENAME.fCreaOpt"]['trigger'] = MXID : add a trigger after object creation
 * - $_SESSION['s_sessVars']['o.'.$tablename.'.create.mode'] : $selecter
 * 
 * @package glob.obj.crea_wiz.php
 * @swreq UREQ:0000972 glob.obj.crea_wiz.php > Object creation wizard (MOTHER-REQ) 
 *   001: for ABSTRACT_SUBST: check, if name of substance already exists
 * @link  file://CCT_QM_doc/89_1002_SDS_code.pdf#pck:glob.obj.crea_wiz.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param 
	[$proj_id]		- project-ID
	[$tablename]	- table name
	$obj_name		- NAME of the object
	$deep_copy      - deep copy array e.g. array["SPOT_RESULT"]=1
	$objAssCrea		- tablenames of real copy objects
	$objAssFrgn		- tablenames of foreign-assoc-tables
	$paste_new		- [0]: nothing special 
					-  1 : paste clipboard object as new object, ask for name
					- do not show other alternatives
	$selecter       - select type of creation
	     ["empty"]
	     "obj_tmpl" - from template object ( e.g. EXP_TMPL)
	     "obj"      - from object ( e.g. EXP)
	  				- if go==0 and $selecter!="" : show only the selected ROW !!!
	  				- store in $_SESSION['s_sessVars']['o.'.$tablename.'.create.mode'] 
	$obj_blueprint_id - if $selecter=="obj"  => this is the SOURCE object
	$obj_tmpl_id	- if ($selecter=="obj_tmpl") => this is the template object
	$optspecial		- array of special option e.g.:
							for DB_USER ["showpasswd"]=1
	$gopt - general options, can contain other form-specific variables					
	$gopt['form']	- ['norm'], 'all'	: used for LINK
	$gopt['bottomInfo'] - [''], 'more'  : bottom info text + MultiCreator
					
	$newparams		- additional TABLE column attributes for object 
	       e.g. ("EMAIL"=>'test@dummy.com", "NOTES"=>"hallo")
	$newpx		    - extra parameters
	   'worflow_id' : ID of workflow: overrules all other workflow settings (see objCreaWiz_act::_post_crea_actions)
	   'upload.need' : -1 : no need of upload (e.g. for LINK)
	$go 			- do it 
	  0   : first form: define $selecter
      5   : $selecter is already defined ... e.g. for creation of SUC: give name
            if you want to start the crea-wizard with this state: call " WIZ?tablename=XXX&go=5"
        INPUT:  $selecter
      7   : [OPTIONAL] further input
      10   : create
    
	$action			- string of actions 
	   ['normal']
	   'projSelect'   : select a target project
	   DEPRECATED: 'alturl_reset' : reset $_SESSION['userGlob']["o.".$tablename.".fCreaOpt"]['altUrl']
	  
	 # connection object: the produced object will not connect to the project
	  $table_name_conn    - table_name of parent object
	  $conn_params	      - extra params for connect table
	  $id_conn	          - ID of parent object
      
     # Upload [optional]
	   $_FILES['userfile']
	  
 * @global $_SESSION['userGlob']["o.TABLENAME.fCreaOpt"] = serialized( array('trigger'=>MODULE_ID) )
 * @global $_SESSION['globals']["o.EXP.fCreaOpt"] => if set to 1 : show extra wizards
 * @global $_SESSION['s_formState']['glob.obj.crea_wiz'] = array
 * 		'projIsSel' => 0,1 : project has been selected once in the session
 * 		'projLastSelect' => PROJ_ID : last project, which was selected
 * 		
 *
 */ 
session_start(); 


require_once ('reqnormal.inc');
require_once('insert.inc');
require_once('insertx.inc');
require_once('edit.sub.inc');
require_once('db_x_obj.inc');
require_once('object.subs.inc');
require_once('object.info.inc');
require_once('varcols.inc');
require_once('class.history.inc');
require_once('javascript.inc');
require_once('o.DB_USER.subs2.inc');
require_once('f.clipboard.inc');
require_once('o.PROJ.addelems.inc');
require_once('f.rider.inc');
require_once('o.PROJ.paths.inc');
require_once ('f.msgboxes.inc');

require_once('subs/glob.obj.crea_wiz_act.inc');
require_once('subs/glob.obj.crea_wizGuiSub.inc');

// ****************************** start of page ******************************
$FUNCNAME='MAIN';
$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );

$title = 'Object creation wizard';

$infoarr			 = NULL;
$infoarr["scriptID"] = "glob.obj.crea_wiz";

$tablename = $_REQUEST['tablename'];
$proj_id   = $_REQUEST['proj_id'];
$id_conn   = $_REQUEST['id_conn'];
$paste_new = $_REQUEST['paste_new'];
$selecter  = $_REQUEST['selecter']; 
$newparams = $_REQUEST['newparams']; 

$proj_tab_nice = tablename_nice2('PROJ');


$infoarr["form_type"]= "list";
$infoarr['obj_name'] = $tablename;
$infoarr["title"]    = $title; // ' for <img src="'.$icon.'"> '.tablename_nice2($tablename);
$infoarr["title_sh"] = $title;
$infoarr['help_url'] = 'f.obj_crea_wiz.html'; 

if ( $proj_id ) {
    
  // $objmore = "<img src=\"$icon\"> ".tablename_nice2($tablename);
  $infoarr["locrow"] = array( array('edit.tmpl.php?tablename=PROJ&id='.$proj_id, $proj_tab_nice) );
  $infoarr["locrow.strict"] = 1;
  
  // put the INPUT-PROJ to the object history ...
  $hist_obj = new historyc();
  $hist_obj->historycheck('PROJ', $proj_id);
  
} 

$pagelib = new gHtmlHead();
$pagelib->_PageHead ( $infoarr["title"],  $infoarr );

js__linkto();


$create_now    = 0;    // DEFAULT: at CREATE_EMPTY: the object will not be created instantly
$attach_proj   = 1;	   // attach object on project?


$table_name_conn = $_REQUEST['table_name_conn'];
$go              = $_REQUEST['go'];
if (!isset($go)) $go = 0;
if (!isset($_REQUEST['conn_params'])) 	$conn_params = array();
else $conn_params = $_REQUEST['conn_params'];
$obj_name  = trim($_REQUEST['obj_name']);  	// no default object name
$deep_copy = $_REQUEST['deep_copy'];    // default: no deep copy
$action    = $_REQUEST['action'];
$gopt      = $_REQUEST['gopt'] ;
$obj_blueprint_id = $_REQUEST['obj_blueprint_id'];
$obj_tmpl_id      = $_REQUEST['obj_tmpl_id']; 
$objAssCrea = $_REQUEST['objAssCrea'] ;
$objAssFrgn = $_REQUEST['objAssFrgn'] ;
$optspecial = $_REQUEST['optspecial'] ;
$newpx = $_REQUEST['newpx'] ;


$mainScrForm  = new objCreaWizGuiSubs($sql, $tablename, $gopt, $go);
$main_act_lib = new objCreaWiz_act($tablename, $gopt);

$mainScrForm->setObjects( $obj_blueprint_id, $obj_tmpl_id, $obj_name );
// $mainScrForm->debug_show('newparams');

if ( $action == 'alturl_reset' ) {
	if ($tablename=="" ) {
		htmlFoot('ERROR', 'Tablename missing.');
	}
	$main_act_lib->set_wizardOps('altUrl', NULL);
	$main_act_lib->_saveUserGlobCreaOpt();
	
	echo "<br>Alternative Wizard Reset ...<br>\n";
	js__location_replace( $_SERVER['PHP_SELF'].'?tablename='.$tablename, "Back to standard creation wizard" );
	return;
}

// $specialAction = 0;
// if ( $selecter!="" OR $obj_blueprint_id OR $go OR $paste_new!="") $specialAction=1; // flags which FORCE the default wizard !

$userWiz = $mainScrForm->getAltUrl();
if (  is_array($userWiz) ) {
	// forward to an alternative page !!!
	if (file_exists($userWiz['script'])) {
		if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
		    debugOut('Alternative URL was configured.', $FUNCNAME, 1);
			// show reset button on debug 
			//echo '[<a href="'.$_SERVER['PHP_SELF'].'?tablename='.$tablename.'&action=alturl_reset">Reset Alternaticve Creation Wizard!</a>]<br>'."\n";
		}
		js__location_replace( $userWiz['script']."?".$userWiz['params'].'&proj_id='.$proj_id, "<b>... special creation page</b>" );
		return;
	} else {
		htmlErrorBox("Error", "Special wizard '$userWiz' not found"  );
	}

}


$sql2  = logon2( $_SERVER['PHP_SELF'] );
$sql3  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();

// if ($_SESSION['userGlob']["g.debugLevel"]>1)  {
// 	$stop_anyway = 1;
// 	$stop_reason .= "debug-level, ";
// }

$forwardArr = array(
	'PROJ'      => array( 'url'=>'obj.proj.elementact.php?tablename=PROJ&actio_elem=new', 'proj_param'=>'proj_id'),
    'JOUR_ENTRY'=> array( 'url'=>'obj.jour_entry.ed1.php?1', 'proj_param'=>'parx[PROJ_ID]'),
	// 'RESX'		=> array( 'url'=>'obj.resx.xnew.php?parx[PROJ_ID]='),
	'I_DOCNUM'	=> array( 'url'=>'p.php?mod=LAB/o.I_DOCNUM.create'),
    'PUR'	=> array( 'url'=>'p.php?mod=DEF/o.PUR.create')
	);
	
if ( $tablename!="" AND isset($forwardArr[$tablename]) ) {
	$tmparr = $forwardArr[$tablename];
	$tmpUrl = $tmparr['url'];
	if ($tmparr['proj_param'] and $proj_id) {
	    $tmpUrl .='&'.$tmparr['proj_param'].'='.$proj_id;
	}
	$x_params = htmlZeugC::get_requests_for_URL();
	if ($x_params) {
	    $tmpUrl .= $x_params;
	}
	js__location_replace($tmpUrl, 'forward page');
	htmlFoot();
}

// javascript before 
$pagelib->_startBody( $sql, $infoarr );

if ($tablename=="" ) {
	htmlFoot('ERROR', 'Tablename missing.');
}

$table_nicename = tablename_nice2($tablename);
if (!$table_nicename or ($table_nicename == $tablename)) {
  htmlFoot('ERROR', 'Table &quot;'.$tablename.'&quot; not defined in the system.');
}

$t_rights = tableAccessCheck($sql, $tablename);
if (!$t_rights['insert']) {
  tableAccessMsg($table_nicename, 'insert');
  htmlFoot();
}



if ( !empty($paste_new) ) {  // object paste
    $clip_obj_id_arr = clipboardC::obj_get ( $tablename );
    $obj_blueprint_id = $clip_obj_id_arr[0];
    if ( !$obj_blueprint_id ) {
        echo "<B>ERROR:</B> Clipboard empty.<br>\n";
        return (0);
    }    
}

if ( ($table_name_conn!='') && $id_conn) { /* attach object to parent object */
	$create_now = 1; /* if an object will be connected, create object now */
	$attach_proj= 0; /* the produced object will not connect to the project*/
}

$bois           = cct_access_has2($tablename);
$objectNumPk    = countPrimaryKeys($tablename);
// $retval         = 0; // default: everything okay

$selecter_new = $mainScrForm->orgSelecter($selecter);

echo "<ul>\n";

if (!$bois) {
	// before 2010-11 only root could do this, now it is all controlled by $t_rights (role-rights)
	$proj_id     = 0; // no project allowed
	$attach_proj = 0;
	if ($objectNumPk > 1) {
		htmlFoot('ERROR', 'This wizard is only working for objects (no ASSOC elements; number of keys: '.$objectNumPk.').');
	}
}

$mainScrForm->initParams($sql);
$error->printAll();



$showProjSelection=0;
if (empty($proj_id) and $attach_proj) {
	
	$proj_id = !empty($_SESSION['s_sessVars']['boProjThis']) ? $_SESSION['s_sessVars']['boProjThis'] : oDB_USER_sub2::userHomeProjGet($sql);
	$showProjSelection=1;
	$action='projSelect';
	
	$proj_last = $main_act_lib->getSessVal('projLastSelect'); // get last selected project
	if ($proj_last) $proj_id = $proj_last;
	
	/* FUTURE: if tool is called: 
	  - for the first in session and 
	  - for this $tablename
	  - without project-id
	  ==> show selector
	
	if ( !$main_act_lib->sessStore['projIsSel'][$tablename] ) $action='projSelect';
	$main_act_lib->setSessStore( array('projIsSel',$tablename) ,1);
	*/
}

$mainScrForm->show_progress($go);

debugOut('(311) GO:'.$go, 'MAIN', 2);

if ($action=='projSelect' and $attach_proj) {
   
    $mainScrForm->selProject($sql, $proj_id, $go);
	htmlFoot("<hr>");
}

if ($proj_id) {
	$main_act_lib->setSessStore( array('projIsSel',$tablename) ,1);
	$main_act_lib->setSessStore( array('projLastSelect') ,$proj_id);
	$ok = $mainScrForm->projectCheck($sql, $proj_id, $go);
	if (!$ok) htmlFoot("<hr>");
} 


//OLD: SUA name check ...
if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
    debugOut('(331) _REQUEST:'.print_r($_REQUEST,1), 'MAIN', 2);
}

debugOut('(330) go:'.$go, 'MAIN', 2);

if ( !$go ) {

    /*** INITIAL GUI ******************************************************/
    /**********************************************************************/
	$mainScrForm->setProject($proj_id);
	$mainScrForm->initObjDepend($paste_new, $selecter_new);

	
	if ($proj_id) { // $proj_name
		$mainScrForm->showProjPath($sql, $proj_id, $showProjSelection);    		
	}

	$mainScrForm->FormBody( $sql, $sql2, $obj_blueprint_id, $paste_new);
	
	
	if ( $table_name_conn!="" ) {
		?>
		<input type=hidden name=id_conn value="<?echo $id_conn?>">  
		<input type=hidden name=table_name_conn value="<?echo $table_name_conn?>"> 
		<? 
	}
	if (!empty($conn_params)) { 
	    foreach($conn_params as $th0=>$th1) {
		    echo "<input type=hidden name=conn_params[".$th0."] value=\"".$th1."\">\n";  
		}
	}    
	
	$mainScrForm->form00_finish();
    	
    // text after form 	
    if ( $gopt['bottomInfo']== 'more' ) {
    	echo "<br>";
    	if ($bois) {
    	    
    	    echo "<br>";
    	    $mainScrForm->showObjExt( $sql );
    	    
    		echo "<br>";
    		$mainScrForm->showGroupRights( $sql );
    
    		echo "<br>\n";
    		$mainScrForm->showWizardCollect( $sql, $obj_blueprint_id, $proj_id  );
    
    		echo "<br>\n";
    		$mainScrForm->showTriggerOpt( $sql );
    	}	
    } else {
    	$mainScrForm->showSlimFoot($sql);
    }
}

if ( $go>0 ) {
    $mainScrForm->set_editmode();
    $mainScrForm->setProject( $proj_id );
}

if ( $go == 5 ) {
    
    // intermediate form e.g. SUA: material type ...
    
    if ( $mainScrForm->has_form05() ) {
        $mainScrForm->initObjDepend( $paste_new, $selecter_new );
        $mainScrForm->FormBody_go0_5( $sql, $sql2 ); 
    } else {
        $go=10; // increment $go ...
    }
}

if ( $go == 7 ) {
    
    // e.g. for detailed parameters for CONCRETE_SUBST
    
    $mainScrForm->form07_paramcheck($sql);
    
    if ( $mainScrForm->has_feature('has_form07') ) {
        $mainScrForm->Form_go7( $sql, $sql2 );
    } else {
        $go=10;
    }
}

if ( $go==10 ) {
    
    if (($tablename == 'IMG') || ($tablename == 'LINK')) {
        
        // $userfile_size = $_FILES['userfile']['size'];
        $userfile      = $_FILES['userfile']['tmp_name'];
        $userfile_name = $_FILES['userfile']['name'];
        $userfile_type = $_FILES['userfile']['type'];
        
        if ($mainScrForm->get_gopt_form()=='norm' and $newpx['upload.need']>=0 ) { // normal form, not the expert one
            if ($userfile==NULL) {
                $error->set( 'main', 1, 'User input: Please give an upload file.' );
                $error->printAll();
                echo "<br>\n";
                $mainScrForm->form_go_back(0);
                htmlFoot();
            }
        }
        
        
        $main_act_lib->setUploadVars($userfile, $userfile_name, $userfile_type);
    }
    if ( ($table_name_conn!="") AND $id_conn) { /* attach object to parent object */
        $main_act_lib->setConnVars( $table_name_conn, $conn_params, $id_conn );
    }
    
    if ($mainScrForm->has_feature('has_param_trafo') ) {
        debugOut('(422) do_param_trafo', 'MAIN', 2);
        $newparams = $mainScrForm->do_param_trafo($newparams);
    }
    
    $create_now = $mainScrForm->create_now_flag($create_now);
        
    
    $has_name_flag = $mainScrForm->has_name_flag;
    
    try {
    
        $main_act_lib->go1 (
            $sql, $sql2, $sql3,
            $create_now,
            $deep_copy,    //
            $newparams,    //
            $newpx,
            $has_name_flag,
            $obj_name,     
            $obj_tmpl_id,
            $obj_blueprint_id,
            $objAssCrea,
            $objAssFrgn,
            $optspecial,
            $proj_id,
            $selecter_new,
            $mainScrForm->KEY_EXISTOBJ_ID
            );
    
    } catch (Exception $e) {
        htmlFootExc($e);
    }
    
    if ($error->Got(READONLY))  {
        $error->printAll();
        echo "<br>\n";
        $mainScrForm->form_go_back(0);
    }
    
    $stop_array = $main_act_lib->get_stop_array();
    if ( $stop_array['anyway'] )  {
        htmlFoot('<hr>');
    }
    

} 

if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
    $info_arr = $mainScrForm->get_info_arr();
    debugOut('Tool-Infos: '.implode("<br>",$info_arr), $FUNCNAME, 1);
}



htmlFoot();
