<?php
/**
 * - Edit/View one object or element
 * - can be called, even if user is not logged in, 
 *   - tries to forward to the login page see: $_SESSION['sec']['appuser']==""
 * - OPTIONAL: redirect to other script: via $_SESSION['globals']['app.o.'.$tablename.'.edit.tmpl.php']
 * - table: PROJ => redirect to obj.proj.xsubst.inc
 * 
 * @package edit.tmpl.php
 * @swreq UREQ:0000059: g > edit.tmpl.php : HOME of single object form 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param 
 *      string $tablename OR $t ( UPPER CASE ) 
        $id        (value of first primary key)
        
     --- optional ---:
	  
        $argu   : arguments ready for insert/update
				: should NOT contain the PK
				: info: CCT_ACCESS_ID will not be passed at the edit form, even for root !!!
        $xargu  : extra arguments despite this TABLE, 
           [CLASS]=extra_class_id
           [__ftx__] : array of feature table extended variables (FUTURE)
        $arguobj: arguments for EXTRA OBJECT    		  
        $xmode       : extended view mode by $_SESSION['s_sessVars']["o.TABLENAME.tabmode"]; 
					   e.g. EXP->protocols $_SESSION['s_sessVars']["o.EXP.tabmode"]="protocols"
        $primas      : OPTIONAL, otherwise taken from $pk_arr: ( key name (except PK1), index: [1]==PK2, [2]==PK3
        $primasid[]  : values of primary-keys 2 and 3, start with 1:second-PK
        $remotepos   : this edit window was called by OPENER
                       and sends back the PRIMARY KEY to valueInputRemote
                       variable must be saved in the form to inherit it to the INSERT PAGE (input hidden name=remotepos ...)
        $action      : { "update", TBD }, for insert use edit.insert.php
        $editmode    : "view" | "edit" 
        $tabArgu     : arguments from a obj.*.xmode.*.inc-script-form
		$dbid		 : if user is not logged in, you can give also the DB-service-name
		$moObj		 : mother object 't', 'id' (analysed in edit.tmpl.inc)
		
		$dbid   : alternativ dbid
		
	FUTURE: $info:       : can contain info text (e.g. name) to give easier understanding of the link
	FUTURE: $roid, $wiid : look for WIID, ROID instead of $id, TBD: not yet implemented
	DEPRECATED: $sessVar     : [key] can update a session-var $_SESSION["s_sessVars"]  direct
	DEPRICATED: $argu_xtra   : DEPRICATED: !!! preselected data for insert-form
	DEPRICATED: $getIdHist   : 0,1 : get ID from history
	DEPRECATED: $idname      : first primary key
	
 * @global $_SESSION['userGlob']    : "g.sof.opt" view options 
        
 * @var extraobj_o_STRUC $extraobj_o =  array( 
			"extra_obj_id"  => $extra_obj_id, 
			"extra_class_id"=> $extra_class_id, 
			"arguobj"       => $arguobj // by name !!!
			'CLASS'         => ID or NULL : if key is given, the update procedure is working
			)
 * @var $editAllow: product = 'viewmode' * $o_right['write']
 * @todo check $colNames_ori
 * @todo change $argu_load: NEW $arguByCol -- argument by colname
 			 $showFormLib->form_show()
 * @version $Header: trunk/src/www/pionir/edit.tmpl.php 59 2018-11-21 09:04:09Z $			
 */

//extract($_REQUEST); 
session_start(); 


require_once('reqnormal.inc');
require_once('exception.inc');
require_once ('f.msgboxes.inc');

function this_error($text) {
	$htmlPageLib = new gHtmlHead();
	$htmlPageLib->_PageHead ('Single object');
	
	echo "<ul><br><br>";
	htmlInfoBox( "Single object sheet Error", $text, "", "ERROR" );
	echo "<BR><br><a href=\"javascript:history.back()\">Back</a>";
	$htmlPageLib->htmlFoot();
	exit;
}

// handle time-out
function this_sessout($tablename, $id, $xmode, $dbid, &$argu) {
	$opturl = "";
    if (isset($dbid)) $opturl .="&dbid=".$dbid;
	$objurl     = "edit.tmpl.php?t=".$tablename."&id=".$id."&xmode=".$xmode;
	$urlencode  = urlencode( $objurl );
	$forwardUrl = 'index.php?forwardUrl='.$urlencode.$opturl;
	$doForward  = 1;
	if ( !empty($argu) ) { // user tried to update: show his parameters ...
		$doForward = 0;
		$pagelib = new gHtmlHead();
		$pagelib->PageHeadLight ( 'object single form: '.$tablename.' ID: '.$id );
		
		echo '<ul><br><br><b>WARNING:</b> Your session has timed out.<br> '.
			 'May be you can save the submitted form-parameters in the clipboard ...';
		echo '<pre>';
		
		echo htmlspecialchars(print_r($argu, TRUE));
		echo '</pre><br>'."\n";
	}
	echo '<br>... not logged in. automatic forward to <a href="'.$forwardUrl.'">login page</a> ...<br>';
    
	if ( !$doForward ) exit;
	
	echo '<script language="JavaScript">' . "\n";
    echo '    location.href="'. $forwardUrl .'";' . "\n" ;
    echo '</script>' . "\n";
    
    exit;  

}


// START of page-code 
$FUNCNAME='edit.tmpl.php';
$tablename = $_REQUEST['tablename']; // DEPRECATED
$t = $_REQUEST['t'];
$id= $_REQUEST['id'];

$argu   	=$_REQUEST['argu'];
$xargu  	=$_REQUEST['xargu'];
$arguobj	=$_REQUEST['arguobj'];
$xmode      =$_REQUEST['xmode'];
$primas     =$_REQUEST['primas'];
$primasid	=$_REQUEST['primasid'];
$remotepos  =$_REQUEST['remotepos'];
$action     =$_REQUEST['action'];
$editmode   =$_REQUEST['editmode'];
$tabArgu    =$_REQUEST['tabArgu'];
$dbid	=$_REQUEST['dbid'];
$moObj	=$_REQUEST['moObj'];
// $getIdHist   	=$_REQUEST['getIdHist'];
// $sessVar     	=$_REQUEST['sessVar'];
// $idname      	=$_REQUEST['idname'];


if (isset($t)) $tablename = $t; 

if ( $_SESSION['sec']['appuser']=="" ) {
	this_sessout($tablename, $id, $xmode, $dbid, $argu);
}


// test, if dbid is set
if ($dbid!="" AND $_SESSION["s_sessVars"]["g.db_index"]!=$dbid) {
	this_error("Session to an other database is active!<br>You are logged in on database-index: '".
		$_SESSION["s_sessVars"]["g.db_index"].
		"'.<br>You want to have access to '".$dbid."'.<br>Please logout first!");
}

// INFO: still no <html> tag on output ...
$sess_var_tmp = "o.".$tablename.".tabmode";
if ( isset($xmode) ) {
    $_SESSION['s_sessVars'][$sess_var_tmp]=$xmode;
}
$tabmode = $_SESSION['s_sessVars'][$sess_var_tmp];
//echo "DDDDD: $tabmode<br>\n";
if (substr($tabmode,0,1)=='0') {
    $meta_urls=array(
        '0meta'=>'glob.obj.feat.php',
        '0perm'=>'glob.obj.access.php',
        '0modlog'=>'obj.cct_acc_up.showobj.php'
    );
    $newurl=$meta_urls[$tabmode].'?t='.$t.'&id='.$id;
    ?>
	<script>
		location.replace("<?php echo $newurl?>");            
	</script>
	<? 
	exit;
}

if ($tablename=="PROJ") {
	include ("obj.proj.xsubst.inc");
	return 0;
}
if (is_array($_SESSION['globals']['app.o.'.$tablename.'.edit.tmpl.php'])) {
	// include alternative code for the page, except user root
	if ( $_SESSION['sec']['appuser']!="root" ) {
		$forwardArr = $_SESSION['globals']['app.o.'.$tablename.'.edit.tmpl.php'];
		$_REQUEST[$forwardArr['REQUEST_key']] = $forwardArr['REQUEST_param'];
		$p_session_started = 1; // session already started, analysed in the included module ...
		include $forwardArr['filename'];
		return 0;
	}
}

if ( $_SESSION['globals']["o.".$tablename.".editScript"]!="" ) {
	include ($_SESSION['globals']["o.".$tablename.".editScript"]); // start an other page
	return 0;
} 

if (!isset($action))  $action = '';
if (!isset($id)) 	  $id = '';
if (!isset($arguobj)) $arguobj = array();
if (!isset($xargu))   $xargu   = array();
if (!isset($argu)) 	  $argu    = array();
$primasid = $_REQUEST['primasid'];


require_once('object.subs.inc');
require_once('db_x_obj.inc');
require_once('edit.sub.inc');
require_once('access_check.inc');


require_once('table_access.inc');
require_once('f.clipboard.inc');
require_once('varcols.inc');
require_once('javascript.inc');
require_once('validate.inc');
require_once('edit.edit.inc');
require_once("subs/toolbar.inc");
require_once("sql_query_dyn.inc");
require_once("edit.tmpl.inc");


$o_rights = array(); // access rights from function access_check() => gives only access info for BO
$x_rights = array(); // resulting access rights table_check(), access_check() and ASSOC rules
                     // so for ASSOC elements sets "insert", "update", delete"
// $infox   = NULL;     // script globals
$error   = & ErrorHandler::get();
$formopt = unserialize($_SESSION['userGlob']["g.sof.opt"]);
$sql  = logon2( $_SERVER['PHP_SELF'] );
$sql2 = logon2( $_SERVER['PHP_SELF'] );


if ($tablename=='') {
	this_error("You must provide a table name!", 0);
	exit;
}
$nicename = tablename_nice2($tablename);


// $moObj will be analyzed inside 
$editLib = new fEditLibC( $tablename, $id, $primasid );
$editLib->init($sql);

if ($nicename=='') {
	this_error("table <B>$tablename</B> is not described by the system.".
		" Please ask the administrator!");
	exit;
}


if (!empty($editmode)) {
	$_SESSION['s_sessVars']['o.'.$tablename.'.editmode'] = $editmode;
}
	
$editmode   = $editLib->getEditMode();



$pk_arr = $editLib->getPkArr();
$has_single_pk = $editLib->has_single_pk;
// $idname 	  = PrimNameGet2($tablename);
$object_is_bo = $editLib->object_is_bo;
$varcol       = & Varcols::get();

$answer = $editLib->checkID_ofObj($sql);
if ($answer[0]<1) {
	this_error('table: "'.$nicename.': " '.$answer[1]);
	exit;
}


$t_rights = tableAccessCheck($sql, $tablename);
$o_rights = access_check($sql, $tablename, $id);

$rightAnswer = $editLib->rightMaskMng($t_rights, $o_rights);

if ( !$rightAnswer['ok'] ) {
	this_error( $rightAnswer['err']);
	exit;
}
$x_rights = $rightAnswer['x'];

if ( is_array($primas) ) $pk_sel = $primas;
else {
    $pk_sel = $pk_arr;
    $primas = $pk_arr;
}

$sqls_main    = $editLib->makeQueryStr( $sql, $pk_sel, $primasid  );
$arguByCol    = $editLib->init_obj_data( $sql );

$editLib->setInfoxName($sql);
$editLib->showHead(); // output <head> and <body> tags

if ( $formopt["formslim"] != 1 ) {
	$menuopt=array("menushow"=> 1);
	$menuRight = $editLib->objHeadLib->getMenuRight($menuopt);
	require_once ("edit.menu.inc");
	$tablename_l= strtolower($tablename);
	edit_menu( $object_is_bo,$has_single_pk,$tablename ,$primasid ,$id ,$o_rights,$tablename_l,$menuRight );
}
/***************************************************************************************************/

$editAllow=0; // default NOT allow edit


$colNames_ori = $editLib->getColNames();

if ( in_array('EXTRA_OBJ_ID', $colNames_ori)  AND ($tablename!='EXTRA_OBJ')) {
	$extra_obj_col_exists = 1;
}


$results	   = 0;
$CCT_ACCESS_ID = 0;
$extraobj_o    = array();
$results       = sizeof( $arguByCol ); 

if ( !$arguByCol ) {
    $results=0;
	$editLib->errorstop("No dataset found for ID(s) [".$editLib->select_str."]");       
}

if ( $object_is_bo ) { // CCT_ACCESS exists
    $CCT_ACCESS_ID = $arguByCol['CCT_ACCESS_ID'];
    $access_data   = $editLib->get_access_data();
}

if ($extra_obj_col_exists) {
    $extraobj_o = fVarcolMeta::get_args_by_id( $sql, $arguByCol['EXTRA_OBJ_ID'] );
}     

if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
    debugOut('tablename: '.$tablename.' colNames_orid:'.print_r($colNames_ori,1), $FUNCNAME, 2);
    debugOut('ARGUS_old:'.print_r($arguByCol,1), $FUNCNAME, 2);
    debugOut('EXTRA_OBJ_old:'.print_r($extraobj_o,1), $FUNCNAME, 2);
    debugOut('other: extra_obj_col_exists: '.$extra_obj_col_exists, $FUNCNAME, 2);
}

$editLib->historycheck( );

echo "<a name=top></a>\n";
//TBD:slim if ( $formopt["formslim"] != 1 )  echo "<img height=20 width=1><br>"; /* for the menu */

$editAllow = $x_rights['write'];

if ( $_SESSION['sec']['appuser'] == 'root' ) $editAllow=1;
if ( $editmode != 'edit' ) {
  $editAllow     = 0;
  $access_reason = 'viewmode'; // used later?
}

/*** UPDATE element ??? ***/
$argu_sizeof = sizeof ( $argu );
$updateError = NULL;
$updateWarning = NULL;


if ( $argu_sizeof ) {  
	// start update-process ...
	$editLib->infox["modeUpdate"] = 1;
	
	// TBD: should a primary key (assoc element) be an argument ???
	if (!empty($pk_arr[1]))  $primasid[1] = $argu[$pk_arr[1]];
	if (!empty($pk_arr[2]))  $primasid[2] = $argu[$pk_arr[2]];
	
	if ( $_SESSION["userGlob"]["g.debugLevel"]>1 )  glob_printr( $argu, "array info" );
	
	// overwrite original class data, except extra_obj_id
	// @var extraobj_o_STRUC $extraobj_o_params
	$extraobj_o_params = NULL; // no extra attribs exist in this view (e.g. in View-Tab "2")
	if ( array_key_exists('CLASS', $xargu) ) { // only, if the FORM gives this parameter !
		$extraobj_o_params['extra_obj_id']  =$extraobj_o['extra_obj_id'];
		$extraobj_o_params['extra_class_id']=$extraobj_o['extra_class_id'];
		$extraobj_o_params['arguobj']		=$arguobj;
		$extraobj_o_params['CLASS']  		=$xargu['CLASS']; // new class
	}
	$editFormLib = new fFormEditC();
	$editFormLib->setObject($tablename, $id);
	$editFormLib->formUpdate( $sql, $sql2, $colNames_ori, $argu, $extraobj_o_params); // can produce warnings and errors
					
	if ($error->Got(CCT_ERROR_READONLY)) {
		$updateError = $error->getAll();	
		// set back  arguments for form	
		$arguByCol   = $argu; 		
		$extraobj_o  = $extraobj_o_params;
		// show form again
	} else {
	    
	    if ($error->Got(CCT_WARNING_READONLY)) {
	        $updateWarning = $error->getAll();
	    }
	    
		$action = ""; // reset info
		$arguByCol_new = $editLib->init_obj_data( $sql );
		$access_data   = $editLib->get_access_data();  // get new access date, e.g. modifier
	
	           
	    //  now prepare edit form
	    if ( !$arguByCol_new ) {
	        $results = 0;
	    } else {      
	        $results   = sizeof($arguByCol_new);
	        $arguByCol = &$arguByCol_new;
	        $extraobj_o= NULL;
			if ($extra_obj_col_exists) {
			    $extraobj_o = fVarcolMeta::get_args_by_id( $sql, $arguByCol['EXTRA_OBJ_ID'] ); 
	        }
	    }
		$editLib->setInfoxName($sql); // set name for page header
	}
}

htmlShowHistory(); // do this before the include of $lab_special_file
$xmodes = $editLib->getXmodes($sql);

// *** <TOOLBAR> *****************************


$infox2      = array('slim'=>NULL);
$info_tab    = ''; 
$editLib->toolbarPrep( $sql, $x_rights, $access_data, $formopt["formslim"], $info_tab, $infox2) ;

if ( $formopt["formslim"] ) {
    $editLib->objHeadSlim($infox2, $xmodes);
} else {
    $editLib->objHeadFull($sql, $CCT_ACCESS_ID, $infox2, $xmodes, $info_tab, $x_rights, $o_rights);
    
	
}

if ($editLib->infox["modeUpdate"]) {
	
	if ( $updateError ) {
		echo "<ul>";
		htmlErrorClassBox("Update error", 'ERROR', $updateError);
		echo "</ul>";
	} else {
	    
	    if ( $updateWarning ) {
	        echo "<ul>";
	        htmlErrorClassBox("Update warning", 'WARN', $updateWarning);
	    }
	    
		echo "<font color=green><b>&nbsp;&nbsp;&nbsp;... updated ...</b></font><br>\n";
	}
}
// *** </TOOLBAR> **************************** 

if ($_SESSION['userGlob']["g.debugLevel"]>1) {
    echo "<B>DEBUG</B> query-string:$sqls_main <BR>";
}
       
if ( !$results )   {
    echo "<br>No dataset found.";
    return 0;
}

list($filename_inc, $xmodeClass) = $editLib->manageXmode($xmodes); 
debugOut("(455) filename_inc:$filename_inc, xmodeClass:$xmodeClass", $FUNCNAME,3);
if ( $filename_inc!=NULL and is_readable($filename_inc) ) {  
	// load a TAB-specific script 
	require($filename_inc);
	// call class oTABLE_xmode_MODE
	$XmodeLib = new $xmodeClass( $editLib );
	
	try {
	    $XmodeLib->xmode_start($sql, $sql2, $id, $arguByCol, $x_rights, $extraobj_o, $tabArgu, $editAllow, $editmode);	
	} catch (Exception $e)  {
	    
	    cMsgbox::showBox("error",$e->getMessage());
	    $stack_nice = MyExcept::stack_nice($e);
	    $error->set( basename(__FILE__), 1,  $e->getMessage() . '| Stack:'.$stack_nice);
	    $error->logError();
	    
	}
	$editLib->pageEnd(); // and exit ...
}     


$tmpHasAdvCols = $editLib->hasAdvCols($sql);



echo "<img src=\"0.gif\" width=1 height=5><br>\n"; // spacer for data-table

$colMode = $editLib->getColMode();
if ($colMode=='vario') {
	$editLib->manageDyn($sql, $editAllow, $tmpHasAdvCols);
	$editLib->pageEnd();
}
if ($colMode=='att') {
    $editLib->manageAttach($sql, $editAllow, $tmpHasAdvCols);
    $editLib->pageEnd();
}

if ($has_single_pk and $tmpHasAdvCols) {
	// filter attributes for special view
	list($colNamesForm, $arguByColForm) = $editLib->adjustColMode($colNames_ori, $arguByCol);
	
} else {
	$colNamesForm  = &$colNames_ori;
	$arguByColForm = &$arguByCol;
}

if ( $editAllow ) { // show edit-form ?
	// @swreq UREQ:0001332: g > edit.tmpl.php > no object usage on edit 
	if ( $object_is_bo and $_SESSION['globals']['app.edit.usedByObject']!=-1 ) {
		require_once("glob.obj.usagegui.inc");
		$objShowUseLib = new gObjUsageGui();
		$objShowUseLib->show_usage($sql, $sql2, $tablename, $id, $CCT_ACCESS_ID);
	}
	$tmpurlstr = fObjFormSub::urlGetPrimkeys( $id, $primas, $primasid );
	echo '<form name="editform" style="display:inline;" method="post" action="edit.tmpl.php?t=',$tablename,'&',$tmpurlstr,'">';
	echo "\n";
	$editFormLib = new fFormEditC();
	$editFormLib->setObject($tablename, $id);
	$editFormLib->init($sql);
	
	$colDefOpt = $editFormLib->x_cols_extra($sql, $arguByCol);
	
	
	$edformOpt= array( ); //OLD:  "H_EXP_RAW"=>$H_EXP_RAW_DESC_ID
	if ($colMode=='NORM') $edformOpt["assocShow"] = 1;
	
	$headopt  = NULL;
	if ($has_single_pk) {
		$headopt["colModeSplit"] = 1;
		$headopt['HasAdvCols'] = $tmpHasAdvCols;
	}
	
	//if ($colMode=='adv') 
	//	$editLib->_infoout('This form shows advanced attributes of this object.');
	$editFormLib->formHead($headopt);
	$editFormLib->formBody( "update", $sql, $colNamesForm, 
			$pk_arr, $arguByColForm, $extraobj_o, $editAllow, $edformOpt, $colDefOpt );
		
	if (isset($remotepos))
		echo '<input type="hidden" name="remotepos" value="'.$remotepos.'">'; 
	
	$editFormLib->close();

} else {         /* only SHOW object */   

	require_once ("edit.show.inc");
	$showFormLib = new fFormShowC();
	$showFormLib->setObject($tablename, $id);
	
	$colDefOpt = $showFormLib->x_cols_extra($sql, $arguByCol);
	
	//$formopt["H_EXP_RAW"]=$H_EXP_RAW_DESC_ID;
	if ($colMode=='NORM') $formopt["assocShow"] = 1;
	if ($has_single_pk) {
		$formopt["colModeSplit"] = 1;
		$formopt['HasAdvCols']   = $tmpHasAdvCols;
	}
	// if ($colMode=='adv') 
	// 	$editLib->_infoout('This form shows advanced attributes of this object.');
	$showFormLib->form_show( $sql, $sql2, $arguByColForm,  $colNamesForm,  $extraobj_o, $formopt, $colDefOpt );

} 
echo "<img src=0.gif height=7 width=1><br>\n"; // a spacer

if ($colMode=='adv') {
	$editLib->pageEnd();
}

/*** END OF OBJECT INDEPENDENT TABLE ***/      

if (class_exists('obj_edit_ext') )  {
    
	//  START OF OBJECT INDEPENDENT PAGE PART
	echo "<a name=\"goodies\"></a>";
	$obj_part_lib = new obj_edit_ext($id, $arguByCol, $o_rights, $editmode, $colNames_ori, $extraobj_o);
	try {
	    $obj_part_lib->rightWindow($sql, $sql2);
	} catch (Exception $e)  {
	    cMsgbox::showBox("error",$e->getMessage());
	    $stack_nice = MyExcept::stack_nice($e);
	    $error->set( basename(__FILE__), 2,  $e->getMessage() . '| Stack:'.$stack_nice);
	    $error->logError();
	}
	
	
	
}

        
$editLib->pageEnd();
