<?php
/**
 * graphical sub functions for glob.obj.crea_wiz.php
 * - prepare the input form
 * - can be extended by LAB.'/objlib/o.{TABLENAME}.crea_wiz.inc'
 * @package glob.obj.crea_wizGuiSub.inc
 * @swreq UREQ:0000972 glob.obj.crea_wiz.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/subs/glob.obj.crea_wizGuiSub.inc 59 2018-11-21 09:04:09Z $
 */
require_once("o.DB_USER.subs.inc");
require_once("o.EXP.crea.inc");
require_once ('func_form.inc');
require_once ("visufuncs.inc");
require_once 'f.prefvals.inc';
require_once 'f.namePolicy.inc';
require_once ('gui/glob.creaWizTrigger.inc');
require_once ('glob.obj.crea_wiz_html.inc');
require_once 'class.filex.inc';

class objCreaWizGuiSubs2 {
    
    public function __construct($tablename, $obj_blueprint_id, $obj_tmpl_id) {
        $this->tablename=$tablename;
        $this->obj_blueprint_id= $obj_blueprint_id;
        $this->obj_tmpl_id=$obj_tmpl_id;
        
        $this->objConAbsLib  = new gConcAbsC();
        $this->tmpl_tabname  = $this->objConAbsLib->getTemplate($tablename);
        
    }
    
    function set_selecter($selecter) {
        $this->saved_sel = $selecter;
    }
    
    function _getLastObjFrObj( &$sql, $objid ) {
        // get LAST object, derived from the OBJ_TEMPLATE of this OBJ
        $tablename = $this->tablename;
        $last_name = NULL;
        $pkname    = PrimNameGet2($tablename);
        $pknameTmpl= PrimNameGet2($this->tmpl_tabname);
        
        // get OBJ_TEMPLATE
        $sqls = "select ".$pknameTmpl." from ".$tablename." where ".$pkname."=".$objid;
        $sql->query($sqls);
        $sql->ReadRow();
        $tmpl_id = $sql->RowData[0];
        if (!$tmpl_id) return;
        
        $last_name = objCreaWizGuiSubs_static::getObjsFromTmpl( $sql, $tablename, $tmpl_id);
        
        return ($last_name);
    }
    
    /**
     * FUNCTION: get last object
       		1. if "obj_tmpl" selected from template
        	2. which was created by user and is younger than one day
     * @param object $sql
     * @return string last name
     */
    function getLastObj( &$sql ) {
        
        $tablename = $this->tablename;
        $mostimp_ColName = importantNameGet2($tablename);
        
        if (!cct_access_has2($tablename)) {
            // only support for business objects
            return;
        }
        
        $now = time();
        $today = $now - 3600 * 24; // 24 hours
        $todayHuman = date("Y-m-d", $today);
        $pkName = PrimNameGet2($tablename);
        
        if ( $this->tmpl_tabname!="" AND $this->saved_sel=="obj_tmpl" AND $this->obj_tmpl_id ) {
            $last_name = objCreaWizGuiSubs_static::getObjsFromTmpl( $sql, $tablename, $this->obj_tmpl_id );
            if ($last_name!="") return ($last_name);
        }
        
        if ( $this->tmpl_tabname!="" AND $this->saved_sel=="obj" AND $this->obj_blueprint_id ) {
            $last_name = $this->_getLastObjFrObj( $sql, $this->obj_blueprint_id );
            if ($last_name!="") return ($last_name);
        }
        
        $sqls= "select ".$mostimp_ColName." from ".$tablename." e, CCT_ACCESS a ".
            " where e.CCT_ACCESS_ID=a.CCT_ACCESS_ID and ".
            "a.DB_USER_ID=".$_SESSION['sec']['db_user_id']." AND a.crea_date>=".$sql->DateString2Sql ($todayHuman, 1).
            "order by ".$pkName." desc";
        $sql->query($sqls);
        if ( $sql->ReadRow() ) {
            $last_name = $sql->RowData[0];
        }
        return ($last_name);
    }
}

/**
 * static methods, leightweight related to the class objCreaWizGuiSubs
 * @author steffen
 *
 */
class objCreaWizGuiSubs_static {
	
	public function __construct() {
		$this->htmlXObj = new htmlZeugC('');
	}
	/**
	 * show group rights
	 */
	public function showGroupRights( &$sql ) {
		
		
		$groupRights     = userActiveGroupsGet( $sql, $_SESSION['sec']['db_user_id']);
		$this->htmlXObj->showPostHead ("The new object gives access rights to groups",
			"&nbsp;&nbsp;[<a href=\"obj.db_user.manage_users_groups.php\" target=_new>Change settings</a>]");
	
		
		if (!empty($groupRights)) {
			echo "<img src=\"images/icon.USER_GROUP.gif\"> <font color=gray>";
			$tmpkomma="";
		
			foreach( $groupRights as $tmp_groupid=>$dummy) {
				$sql->query("select NAME from USER_GROUP where USER_GROUP_ID=".$tmp_groupid);
				$sql->ReadRow();
				echo $tmpkomma . "".$sql->RowData[0].""; 
				$tmpkomma=", ";
			}
		
			echo "</font>\n";
			// htmlInfoBox( "", "", "close");
		} else {
			echo "<font color=gray>[<a href=\"obj.db_user.manage_users_groups.php\" target=_new><font color=gray>Give access to selected groups</font></a>]</font><br>";
		}
	}
	
	
	
	/**
	 * 
	 * @param object $sqlo
	 * @param string $tablename
	 * @param string $in_name
	 * @return array
	 *   'cnt'=>
	 *   'f_objid' => first object ID
	 */
	public static function check_name(&$sqlo, $tablename, $in_name) {
	    
	    $result=array();
	    $sea_lib = new fileC();
	
	    $searchCol = 'NAME';
	    $sea_opt=array("caseinsense" => 1); // "like" 
	    $answer = $sea_lib->objGetByName( $sqlo, $tablename, $searchCol, $in_name, $sea_opt);
	    
	    $result['cnt']     = $answer[0];
	    $result['f_objid'] = $answer[1];
	    $result['f_name']  = $answer[2];
	    
	    return $result;
	}
	
	public static function getObjsFromTmpl( &$sql, $tablename, $id) {
	    // get last object from template
	    
	    $tablename = $tablename;
	    $last_name = NULL;
	    $objConAbsLib  = new gConcAbsC();
	    $mostimp_ColName = importantNameGet2($tablename);
	    
	    $sqlAfter = $objConAbsLib->getConcFrAbsSql($tablename, $id);
	    $idcol    = PrimNameGet2($tablename);
	    $sqls     = "select x.".$idcol.", x.".$mostimp_ColName.
	    " from ".$sqlAfter." order by x.".$idcol." DESC";
	    $sql->query($sqls);
	    if ( $sql->ReadRow() ) {
	        $last_name = $sql->RowData[1];
	    }
	    return ($last_name);
	}
}

/**
 * graphical sub functions for glob.obj.crea_wiz.php
 * - prepare the input form
 * @swreq UREQ:0000972 glob.obj.crea_wiz.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class objCreaWizGuiSubs {
    
    var $tablename;
	var $infox;     /* script important variables
		'obj_name'
		*/
	var $saved_sel; // type of selection: ["empty"], "obj_tmpl" , "obj"
	var $gopt;      // GRAPHIC options: 'form' -- type of view 'norm', 'full'
	var $wizardOpts;/* wizard options:
		'altUrl' => array('script'=>xxx, 'params'=> yyyy) - alternativ Wizard URL
		            example: 'altUrl' => array('script'=>'p.php', 'params'=> '&mod=LAB/p.129.subst_crea&parx[projid]')
		'trigger' - ID of trigger
		'defvals' - array of predefined values
			array(key=>val)
		*/
	var $formRowShow; /* array(
	    'name'  =>   array('show'=>0,1)
	 	'empty' =>   array('show'=>0,1, 'selShow' => 0,1)
	 	"obj_tmpl"=> ...
	 	"obj"=>		 ...
	*/
	var $useTriggerID; // optional ID of MODULE for a trigger
	private $obj_blueprint_id;
	private $obj_tmpl_id;
	
	private $headtext;
	private $infoarr;
	private $obj_extend_lib=NULL; // extension lib
	private $go; // initial $go
	private $obj_helper_lib;
	private $newparams; //  array e.g. ("EMAIL"=>'test@dummy.com", "NOTES"=>"hallo")
	
/**
 * 
 * @param object $sql
 * @param string $tablename
 * @param array $gopt
 *    ['form'] = ['norm'], used for LINK
				  'all'	
 */
function __construct(&$sql, $tablename, $gopt=NULL, $go=0 ) {
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    
	$this->tablename=$tablename;
	$this->go = $go;
	$this->infox= NULL;
	$this->newparams = $_REQUEST['newparams'];
	
	if ( $gopt['form']==NULL ) $gopt['form']= 'norm';
	$this->gopt = $gopt;
	$this->wizardOpts = NULL;
	$this->obj_helper_lib = NULL;
	
	$this->htmlXObj = new htmlZeugC($tablename);
	$this->hist_obj = new historyc();
	$this->wizHelpLib = new objCreaWizGuiSubs_static();
	
	$this->htmlXObj->javaScriptX();
	
	$this->html_tab_cnt = 0; /* for MODAL FUNCTION */
	$this->bois  = cct_access_has2($tablename);
	$this->KEY_EXISTOBJ_ID = "o.".$tablename.".create.obj_id";  // keyword of session variable
	$this->COLSPAN = 4; 
	
	$this->mostimp_ColName = importantNameGet2($tablename);
	$pk_arr = primary_keys_get2($tablename);
	$this->has_name_flag = 1;
	
	$mostimpcol_features = $this->mostimp_ColName ? colFeaturesGet($sql, $tablename, $this->mostimp_ColName) : array();
	if ( sizeof($pk_arr)>=2 or !$this->mostimp_ColName )  { 
		// if no IMPORTANT name given OR 
				
		$this->has_name_flag = 0; 
	}

	$this->mostimpcol_nice = 'name';
	if (!empty($mostimpcol_features['NICE_NAME'])) $this->mostimpcol_nice = $mostimpcol_features['NICE_NAME'];
	
	$this->objConAbsLib  = new gConcAbsC();
	$this->tmpl_tabname  = $this->objConAbsLib->getTemplate($tablename);
	
	if ($_SESSION['userGlob']["o.".$tablename.".fCreaOpt"]!=NULL) {
		$this->wizardOpts = unserialize($_SESSION['userGlob']["o.".$tablename.".fCreaOpt"]);
	}
	
	$table_nicename = tablename_nice2($tablename);
	$this->headtext = "Create a new &quot;".$table_nicename."&quot;";
	$this->infoarr = array();
	
	// search for extension
	$this->obj_extend_lib=NULL;
	$helper_file = $_SESSION['s_sessVars']['AppLabLibDir'].'/objlib/o.'.$tablename.'.crea_wiz.inc';
	if (!file_exists($helper_file)) {
	    $helper_file = dirname(__FILE__).'/../objlib/o.'.$tablename.'.crea_wiz.inc';
	}
	debugOut('(285) helper_file:'.$helper_file, $FUNCNAME, 1);
	if (file_exists($helper_file)) {
	    debugOut('(285) has obj_extend_lib!', $FUNCNAME, 1);
	    require_once $helper_file;
	    $clsName = 'o'.$tablename.'_objCreaWiz_G';
	    
	    if (class_exists($clsName)) {
	       $this->obj_extend_lib = new $clsName();
	       
	    }
	}
}


public function has_feature($key) {
    if ($this->obj_extend_lib==NULL) return 0;
    return $this->obj_extend_lib->has_feature($key);
}
public function has_form05() {
    if ($this->obj_extend_lib==NULL) return 0;
    return $this->obj_extend_lib->has_form05();
}

public function do_param_trafo(&$newparams) {
    if ($this->obj_extend_lib==NULL) return 0;
    return $this->obj_extend_lib->do_param_trafo($newparams);
}
public function create_now_flag($create_now) {
    if ($this->obj_extend_lib==NULL) return $create_now;
    return $this->obj_extend_lib->create_now_flag($create_now);
}
function form07_paramcheck( $sqlo ) {  
    if ($this->obj_extend_lib==NULL) return;
    return $this->obj_extend_lib->form07_paramcheck($sqlo);
}

function get_gopt_form() {
    return $this->gopt['form'];
}

/**
 * add info to a cache
 * @param string $text
 */
private function _add_info($text) {
    $this->infoarr[]=$text;
}

public function get_info_arr() {
    
    if ($this->obj_extend_lib) {
        $tmp_info = $this->obj_extend_lib->get_info();
        $tmp_info = array_merge($this->infoarr, $tmp_info);
        return $tmp_info;
    } else {
        return $this->infoarr;
    }
}

function initParams(&$sqlo) {
	$this->useTriggerID = glob_creaWizTrigger::getTriggerOfTable($sqlo, $this->tablename, $this->wizardOpts['trigger'] ); 
}

function getTrigger(&$sqlo) {
	return $this->useTriggerID;
}

// get alternative wizard URL
function getAltUrl() {
    if ($this->obj_extend_lib==NULL) return;
    return $this->obj_extend_lib->get_alternate_wiz();
}

public function showObjExt($sqlo) {
    if ($this->obj_extend_lib) {
        $this->htmlXObj->showPostHead ('Object Specials');
        $this->obj_extend_lib->sh_options($sqlo);
    }
}

public function showGroupRights(&$sqlo) {
	$this->wizHelpLib->showGroupRights( $sqlo );
}

function setObjects( $obj_blueprint_id, $obj_tmpl_id, $obj_name=NULL ) {
	
	$this->obj_blueprint_id = $obj_blueprint_id;
	$this->infox['obj_name']  = $obj_name;
	
	// $obj_tmpl_id
	if ($obj_tmpl_id==NULL) {
		$obj_tmpl_id = $_SESSION['s_sessVars']["o.".$this->tablename.".create.obj_tmpl_id"];
		if ( !$obj_tmpl_id ) $obj_tmpl_id = $this->hist_obj->last_bo_get( $this->tmpl_tabname );
	}
	$this->obj_tmpl_id = $obj_tmpl_id;
	
	$this->obj_helper_lib = new objCreaWizGuiSubs2( $this->tablename, $obj_blueprint_id, $obj_tmpl_id);
	
	if ($this->obj_extend_lib!=NULL) {
	    $this->obj_extend_lib->set_obj_selections( $this->obj_helper_lib, $obj_blueprint_id, $obj_tmpl_id, $obj_name, $this->gopt, $this->go, $this->wizardOpts);
	    $this->obj_extend_lib->set_newparams($this->newparams);   
	    $this->obj_extend_lib->init();
	}
}

public function orgSelecter($selecter) {
	// OUTPUT: $this->saved_sel 
	
	$tablename = $this->tablename;

	if ( $selecter!="" ) $_SESSION['s_sessVars']['o.'.$tablename.'.create.mode'] = $selecter;
	
	if ($tablename=="CONCRETE_PROTO") {
		if ($_SESSION['s_sessVars']["o.".$tablename.".create.mode"] !="obj") $_SESSION['s_sessVars']["o.".$tablename.".create.mode"] = "obj_tmpl";
	}
	if (($tablename == 'IMG') || ($tablename == 'LINK')) {
	    if ($this->gopt['form']=='norm') {
	        $_SESSION['s_sessVars']['o.'.$tablename.'.create.mode']='empty';
	    }
	}
	
	$this->saved_sel = $_SESSION['s_sessVars']['o.'.$tablename.'.create.mode'];
	if ($this->saved_sel=="") $this->saved_sel="empty"; // default
	
	if ($this->obj_extend_lib!=NULL) {
	    $this->obj_extend_lib->set_selecter($this->saved_sel);
	}
	$this->obj_helper_lib->set_selecter($this->saved_sel);
				
	return ($this->saved_sel);
}

function show_progress($go) {
    $goArray   = array( "0"=>"Initial input", 5=>"Next Parameters", 7=>"Next Parameters", 10=>'Create' );
    // $extratext = '[<a href="'.$_SERVER['PHP_SELF'].'">Start again</a>]';
    $extratext ='';
    $formPageLib = new FormPageC();
    $formPageLib->init( $goArray, $extratext );
    $formPageLib->goInfo( $go ); 
}



function showWizardCollect( &$sql, $obj_blueprint_id, $proj_id ) {
	$tablename=$this->tablename;
	$defaultKey='default';
	$riderArr=array(
		"default"=>array("CreationWizard", ""), 
		);
		
	$riderArr["MultiCreator"] = array("MultiCreator", "glob.obj.creaMulti.php?t=".$tablename.
			"&id=".$obj_blueprint_id."&projid=".$proj_id);	
		
	$optWiz = glob_elementDataGet( $sql, "GLOBALS", "NAME", "o.".$tablename.".fCreaOpt", "VALUE");
	if ($optWiz==1) {
		// file name, e.g. obj.exp.fCreaOpt_arr.inc
		$requ_file =  $_SESSION['s_sessVars']['AppLabLibDir'].'/obj.'.strtolower($tablename).'.fCreaOpt_arr.inc';
		if (file_exists($requ_file)) {
		    $riderArr_opt=array();
		    include($requ_file); // gives $riderArr_opt
			if (is_array($riderArr_opt)) $riderArr = array_merge($riderArr, $riderArr_opt);
		}
	}

	
	$this->htmlXObj->showPostHead ('Wizards collection');
	foreach( $riderArr as $key=>$valarr) {
		if ($key==$defaultKey) $outstr= '<b>'.$valarr[0].'</B>';
		else $outstr='<a href="'.$valarr[1].'&from=crea_wiz">'.$valarr[0].'</a>';
		echo '['.$outstr.']&nbsp;&nbsp;';
	}
	reset ($riderArr); 
	
}

function showTriggerOpt( &$sqlo ) {
	$notes='[<a href="p.php?mod=DEF/g.crea_wiz.prefs&t='.$this->tablename.'">Set workflow</a>] &nbsp; ';
	$trigger= $this->getTrigger($sqlo);
	if ($trigger>0) {
		$objLinkLib = new fObjViewC();
		$htmlTmp = $objLinkLib->bo_display( $sqlo, 'MODULE', $trigger);
		$notes .= '<b>Active:</b> '.$htmlTmp;
	} else {
		$notes .= '<span style="color:gray">No active workflow</span>';
	}
	
	
	$this->htmlXObj->showPostHead ('Workflow', $notes);
}

// show foot after the main form, show option link and trigger
function showSlimFoot(&$sqlo) {
	$paramsGet = $_REQUEST;
	$paramsGet['gopt']['bottomInfo']='more';
	$params = js__get_param_to_urlArray($paramsGet);
	echo '[<a href="'.$_SERVER['PHP_SELF'].'?dummy'.$params.'"  class="yGgray">options</a>]'."\n";
	
	echo '<span style="color:gray">';
	$trigger= $this->getTrigger($sqlo);
	if ($trigger>0) {
		echo ' Workflow is active. ';
	}
	echo '<span>'."\n";
}

/**
 * object dependend actions, settings
 * @param int $paste_new
 * @param string $selecter
 */
function initObjDepend($paste_new, $selecter) {
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    debugOut("(500) selecter:$selecter paste_new:$paste_new", $FUNCNAME, 2);
	 
	$this->formRadios = 1;
	$tablename = $this->tablename;
	
	$this->formRowShow["name"]     = array('show'=>0);
	if ( $this->has_name_flag ) {
	   $this->formRowShow["name"]     = array('show'=>1);
	}
	$this->formRowShow["empty"]    = array('show'=>1);
	$this->formRowShow["obj_tmpl"] = array('show'=>1, 'selShow' => 1);
	$this->formRowShow["obj"] 	   = array('show'=>1, 'selShow' => 1);
	
	if ( $paste_new ) {
		$this->formRowShow["obj_tmpl"] = array('show'=>0);
		$this->formRowShow["empty"]    = array('show'=>0);
	}
	
	if ($selecter=="obj_tmpl" and $this->obj_blueprint_id) {
		$this->formRowShow["empty"]    = array('show'=>0);
		$this->formRowShow["obj_tmpl"] = array('show'=>1, 'selShow' => 0);
		$this->formRowShow["obj"] 	   = array('show'=>0);
		$this->formRadios = 0; // no other selection !
	}
	
	if ($tablename=="CHIP_READER") {
	    $this->formRowShow["empty"] = array('show'=>0);
	}
	if ($tablename=="CONCRETE_PROTO") {
	    $this->formRowShow["empty"] = array('show'=>0);
	}
	if ($tablename=="CONCRETE_SUBST") {
	    $this->formRowShow["name"]  = array('show'=>0);
		$this->formRowShow["empty"] = array('show'=>0);
	}
	if ($tablename=="SOCKET") {
	    $this->formRowShow["name"]  = array('show'=>0);
	    $this->formRowShow["empty"] = array('show'=>0);
	}
	if ($tablename=="PUR") {
	    $this->formRowShow["name"]  = array('show'=>0);
	    $this->formRowShow["empty"] = array('show'=>0);
	}
	
	
	if ($tablename=="DB_USER") {
		// do not show select-radio-buttons.
		// do not allow an empty DB_USER.
		// must select an existing or abstract
		$this->formRadios = 0;
		$this->saved_sel  = "obj";
		$this->formRowShow["empty"] = array('show'=>0);
	}
	
}

function initForm() {

}

function setProject($projid) {
	$this->projid = $projid;
	if ($projid) {
	    $this->headtext .= " and add it to folder";
	    if ($this->obj_extend_lib) $this->obj_extend_lib->setProject($projid);
	} 
	
}

function htmlTabCntAdd($summand) {
	$this->html_tab_cnt = $this->html_tab_cnt + $summand;
}







/**
 * show raw table row
 * @param string $pre
 * @param string $title
 * @param string $value
 * @param string $notes
 */
function form_row_raw($pre, $title, $value, $notes=NULL) {
    $this->htmlXObj->form_row_raw($pre, $title, $value, $notes);
}


/**
 * manage the first columns of a selection-row
 * @param int $rowpos 1,2,3
 * @param string $creatype "empty", "obj_tmpl", "obj"
 * @param string $text
 */
function rowColFirst( $rowpos,   $creatype, $text ) {
	$this->rowpos = $rowpos;
	$this->htmlXObj->form_row_ColFirst( $rowpos,  $creatype,  $text, $this->formRadios, $this->saved_sel  );
}

function rowColFirst_end() {
	// echo "<hr noshade size=1>";
	// <img src=\"0.gif\" height=1 width=1>
	// echo "<tr><td colspan=4><hr noshade size=1></td>";
	// echo "</tr>\n";
	
	/*
	$rowpos = $this->rowpos;
	echo "<tr bgcolor=".$this->htmlXObj->rowarr["bgcolor"][$rowpos]."><td  colspan=".$this->COLSPAN.">";
	echo "<img src=\"0.gif\" height=3 width=1></td></tr>\n";
	*/
}




function formRow_First() {
	// FUNCTION: show row for creation from EMPTY object
	
	$tablename = $this->tablename;
	$table_nicename = tablename_nice2($tablename);
	$this->rowColFirst( 1, "empty", "empty ".$table_nicename );
	
	echo "&nbsp;".
		"	<!-- for modal-dialog purposes -->".
		"	<input type=hidden name=dummy>".
		"	<input type=hidden name=dummy>".
		"</td><td>".
		"	<I>all values are NULL</I>".
		"</td>".
		"</tr>";
	$this->rowColFirst_end();

	$this->htmlTabCntAdd(1);
		
}

function formRow_Tmpl( &$sql, $tmpl_tabname ) {
	// FUNCTION: show row for creation from OBJECT_TEMPLATE
	
	
	
	$tablename = $this->tablename;
	$tmpl_tabname_nice  =  tablename_nice2($tmpl_tabname);
	
	$this->rowColFirst( 2, "obj_tmpl", "from ".$tmpl_tabname_nice );
	
	$obj_tmpl_id  = $this->obj_tmpl_id;
	$obj_tmp_name = NULL;
	if ($obj_tmpl_id) $obj_tmp_name = obj_nice_name($sql, $tmpl_tabname, $obj_tmpl_id);

    if ( $this->formRowShow["obj_tmpl"]["selShow"] ) {
		
		
		echo $this->htmlXObj->db_obj_link( $tmpl_tabname, 'obj_tmpl_id', $obj_tmpl_id, $obj_tmp_name,  $this->html_tab_cnt, $fopt );
		
		?>
		</td><td>
			<I>select an <?echo $tmpl_tabname_nice?></I>
		</td>
		</tr>
		<? 
	} else {
		echo '<input type=hidden name="obj_tmpl_id" value="'.$obj_tmpl_id.'">'."\n";
		echo htmlspecialchars($obj_tmp_name).' [ID:'.$obj_tmpl_id.']';
		?>
		</td><td></td></tr>
		<? 
	}
	$this->rowColFirst_end();
	$this->htmlTabCntAdd(1);
}

function formRow_Obj(&$sql, &$sql2, $obj_blueprint_id, $paste_new) {
	// FUNCTION: show row for creation from existing OBJECT
	
	
	$tablename 	    = $this->tablename;
	$table_nicename = tablename_nice2($tablename);
	
	if ( !$paste_new ) {
	
		$hist_obj       = new historyc();
		
		$this->rowColFirst( 3, "obj", "from existing ".$table_nicename );
			
		if (empty($obj_blueprint_id)) $obj_blueprint_id = isset($_SESSION['s_sessVars'][$this->KEY_EXISTOBJ_ID]) ? $_SESSION['s_sessVars'][$this->KEY_EXISTOBJ_ID] : NULL;
		if (empty($obj_blueprint_id)) $obj_blueprint_id = $hist_obj->last_bo_get( $tablename );
	
		$obj_tmp_name = NULL;
		if ($obj_blueprint_id!=NULL) {
			$obj_tmp_name = obj_nice_name($sql, $tablename, $obj_blueprint_id);
		}

		echo $this->htmlXObj->db_obj_link( $tablename, 'obj_blueprint_id', $obj_blueprint_id, $obj_tmp_name, $this->html_tab_cnt);

	
		echo "<br>\n";
		$objnotes = "select an existing ".$table_nicename;  
	
	} else {
	 
        echo "<tr valign=top><td>&nbsp;</td><td><font color=#606000><B>from clipboard<B></font></td><td>\n";
        echo "<input type=hidden name=obj_blueprint_id value=\"".$obj_blueprint_id."\">\n";  
        echo "<input type=hidden name=selecter value=\"obj\">\n";
        $objnotes = "take ".$table_nicename." from clipboard";
		
    }
	 
	$this->objShowSpecial( $sql, $tablename ); 	
	if ($tablename!="DB_USER") $this->htmlXObj->showAssocBoxes($sql);
		
	echo "</td>";
	$this-> _objCopyNotes($objnotes);
	echo "</tr>\n";
	$this->rowColFirst_end();
	$this->htmlTabCntAdd(1);
}

function _objCopyNotes($objnotes) {
	
	$tablename 	    = $this->tablename;
	echo "<td valign=top><I>";
	echo $objnotes;
	$infoPre = "<br><font color=green>INFO:</font> ";
	if ($tablename=="CONCRETE_SUBST") {
			echo $infoPre."Creates also a copy of the protocol.<br>";
	}
	if ($tablename=="CHIP_READER") {
			echo $infoPre."Creates also a copy of the protocol.<br>";
	}
	if ($tablename=="DB_USER") {
			echo $infoPre."typical settings (roles, groups, ...)<br>\n".
			"[advanced: <a href=\"edit.insert.php?tablename=DB_USER\">empty user form</a>]\n";
	}
	 
	echo "<br></I></td>";
}

function objFormsel($selarr) {
	echo "<font color=gray>create real copies of linked objects:</font><br>";
	foreach( $selarr as $key=>$val) {
		echo "<input type=checkbox name=\"objAssCrea[$key]\" value=\"1\">".$val."<br>\n";
	}
}
function objFormselFrgn($selarr) {
	echo "<font color=gray>copy other associated elements of:</font><br>";
	foreach( $selarr as $key=>$val) {
		echo "<input type=checkbox name=\"objAssFrgn[$key]\" value=\"1\">".$val."<br>\n";
	}
}

function objShowSpecial( &$sql, $tablename ) {
	// FUNCTION: specials for creating from EXISTING object 
	
	$selarr  = NULL;
	$frgnArr = NULL;
	switch ($tablename) {
		case "EXP":
			$selarr["EXP_HAS_PROTO"] = tablename_nice2("EXP_HAS_PROTO");
			break;
		//case "DB_USER":
		//	$frgnArr["DB_USER_IN_GROUP"] = tablename_nice2("DB_USER_IN_GROUP");
		//	$frgnArr["USER_ROLES"]       = tablename_nice2("USER_ROLES");
		//	break;
	}
	
	if (!empty($selarr))  $this->objFormsel($selarr);
	if (!empty($frgnArr)) $this->objFormselFrgn($frgnArr); 
	
	if ($tablename == "DB_USER") {
		echo "<input type=checkbox name=\"optspecial[showpasswd]\" value=\"1\" > show generated password<br>\n";
	} 
	
}

/**
 * check project access rights
 */
function projectCheck( &$sqlo, $proj_id, $go) {
	$ok=0;
	// check rights
	$projAcc = access_check($sqlo, 'PROJ', $proj_id);
    if ( !$projAcc['insert'] ) {
		echo "<ul>";
		$proj_name = obj_nice_name ( $sqlo, 'PROJ', $proj_id );
		htmlErrorBox('ERROR', "No 'insert' permissions on destination folder <img src=\"images/icon.PROJ.gif\"> ".
			"'<a href=\"edit.tmpl.php?t=PROJ&id=".$proj_id."\">".$proj_name."</a>'.".
		    "<br>Please give 'insert' permission to the folder or choose an other folder!");
		echo "<br>";
		if ( !$go ) $this->htmlXObj->selProject($sqlo, $proj_id, $go);
		
	} else {
		$ok=1;
	}
	return ($ok);
}


function selProject($sqlo, $proj_id, $current_go) {
    $this->htmlXObj->selProject($sqlo, $proj_id, $current_go);
}

function showProjPath(&$sql, $proj_id, $showProjSelection) {
    $projPathObj = new oPROJpathC();
    $desturl = "edit.tmpl.php?t=PROJ&id=";
    $rettext = $projPathObj->showPathSlim( $sql, $proj_id, $desturl, 0 );
    echo "<img src=\"images/icon.PROJ.gif\" border=0 hspace=3></a><font color=gray><B>Destination folder:</B></font> ".$rettext;
    if ($showProjSelection) {
        $extra = '&nbsp;&nbsp;[<a href="'.$_SERVER['PHP_SELF'].'?action=projSelect&tablename='.$this->tablename.'">change</a>]';
        echo $extra;
    }
    echo "<br><br>\n"; // extra space for design reasons
    
}

// FUNCTION: show the main form-ROWS
function formRows(&$sql, &$sql2, $obj_blueprint_id, $paste_new) {
    
	$tablename = $this->tablename;
	
	//
	// 1. EMPTY
	//    
	
	if ( $this->formRowShow["empty"]["show"] ) {
		$this->formRow_First();
	} 
	
	//
	// 2.  FROM TEMPLATE
	//
	if ( $this->formRowShow["obj_tmpl"]["show"]  ) {
		$tmpl_tabname = $this->tmpl_tabname;
		if ( $tmpl_tabname ) {
			$this->formRow_Tmpl($sql, $tmpl_tabname);
		} 
	}
	//
	// 3.  FROM EXISTING OBJ
	//
	if ( $this->formRowShow["obj"]["show"]  ) {
		$this->formRow_Obj($sql, $sql2, $obj_blueprint_id, $paste_new);
    }
		
	// after all ...
	
	if ( ($tablename=="IMG") || ($tablename=="LINK") ) { 
	    $uplopt = array('sh_upload_flag'=>1);
	   
	    $this->htmlXObj->formUploadFile($tablename, $this->projid, $uplopt);
	} 
}

function FormBodyNorm( &$sql, &$sql2, $obj_blueprint_id, $paste_new ) {
	
    if ( $this->formRowShow["name"]['show']  ) {
        
        // LAT OBJECT
       $use_obj_name  = $this->obj_helper_lib->getLastObj( $sql );
       $edit_flag = 1;
       

       // OVERRULE NAME
       if ($this->infox['obj_name']) $use_obj_name = $this->infox['obj_name']; // overrule ...
       
       if ($this->obj_extend_lib!=NULL) {
           // AUTOMATIC NAME overrules all
           $policy_result = $this->obj_extend_lib->get_new_obj_name($sql);
           if ($policy_result['name']!=NULL) $use_obj_name = $policy_result['name'];
           if ($policy_result['policy_has']) {
               $edit_flag = -1;
               $this->_add_info('has name_policy');
           }
       }
       
       $this->htmlXObj->form_row_Name($sql, $use_obj_name, $edit_flag);
       // $this->formRow_Name($sql, $use_obj_name);
	   $this->htmlTabCntAdd(1);
    }
    
    
    $tab_cnt=0;
	if ($this->obj_extend_lib) {
	    $tab_cnt = $this->obj_extend_lib->form0_extra_rows($sql, $this->htmlXObj);
	}
	
    $this->htmlTabCntAdd($tab_cnt);
	$this->htmlXObj->htmlRowEmpty("hr");
    $this->formRows($sql, $sql2, $obj_blueprint_id, $paste_new);
	
}

// function FormBody_LINK( &$sql, &$sql2, $obj_blueprint_id, $paste_new, $headtext ) {
//     $this->formRows($sql, $sql2, $obj_blueprint_id, $paste_new);
// }

/**
 * the first form (select object type $selecter)
 * @param object $sql
 * @param object $sql2
 * @param int $obj_blueprint_id
 * @param int $paste_new
 */
function FormBody( &$sql, &$sql2, $obj_blueprint_id, $paste_new ) {
    
    $tablename = $this->tablename;
    $proj_id   = $this->projid;
    $advancedView = $this->gopt['form'];

    $submit_opt = array();
	$go_next    = 5; // OLD:10
	
	if ($this->obj_extend_lib!=NULL) {
	   $submit_opt['submit.text'] = $this->obj_extend_lib->has_feature('submit.text');
	}
	
	
	$this->htmlXObj->form_start($proj_id, $tablename, $go_next, $this->headtext);
	
	$advView= NULL;
	
	do {
		if ($tablename=='LINK') {
			$advView=array(
				'url'=>$_SERVER['PHP_SELF'].'?tablename='.$tablename.'&proj_id='.$proj_id.'&gopt[form]=',
				'textarr'=>array( 'norm'=>'normal', 'all'=>'advanced' ),
				'val'=> $advancedView
				);
			if ($advancedView=='norm') {
				$uplopt = array('noHR'=>1);
				
				$this->htmlXObj->formUploadFile($tablename, $proj_id, $uplopt);
				break;
			}
		}
		$this->FormBodyNorm( $sql, $sql2, $obj_blueprint_id, $paste_new );
	} while (0);
	
	$this->htmlXObj->htmlRowSubmit( $advView, $submit_opt );
}

/**
 * finish form
 * @param array $opt
 * 'all_req_params' : 0,1
 */
function form00_finish( ) {
    $this->htmlXObj->add_hidden_params( array('newparams'=>$this->newparams, 'newpx'=>$_REQUEST['newpx'] ) );
    $this->htmlXObj->form_finish();
}

/**
 * - the "create from" type is already selected
 * - needs $this->obj_extend_lib
 */
function FormBody_go0_5(&$sqlo, &$sqlo2) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    $tablename = $this->tablename;
    $proj_id   = $this->projid;
    
    $next_go = $this->obj_extend_lib->next_go_val(); // Standard: 10
    $this->htmlXObj->form_start($proj_id, $tablename, $next_go, $this->headtext);

    $this->obj_extend_lib->show_form05($sqlo, $this->htmlXObj);
    
    $submit_opt = array();

    $error_cache = array();
    if ($error->Got(READONLY))  { 
        $error_cache_r = $error->getAll();
        // copy the  array, because reset() deletes the data
        $error_cache_obj   = new ArrayObject($error_cache_r) ;
        $error_cache = $error_cache_obj->getArrayCopy();
        $error->reset();
        $submit_opt['submit.allow'] = -1; // deny submit

    }

    // if no newparam given show optional params here ...
    
    if ($this->obj_extend_lib) {
        $tab_cnt = $this->obj_extend_lib->form0_extra_rows($sqlo, $this->htmlXObj);
    }
    $this->htmlTabCntAdd($tab_cnt);
    
    
    $advView=NULL;
    
    $submit_opt['submit.text'] = $this->obj_extend_lib->has_feature('submit.text');
    $submit_opt['back.button'] = 1;
    $submit_opt['back.go']     = 0;
    $this->htmlXObj->htmlRowSubmit( $advView, $submit_opt );
    
    $fopt=array(
        'all_req_params'=>1, 
        'params_except_newparams'=>1 
    );
    $this->htmlXObj->form_finish($fopt); 
    
    if(!empty($error_cache)) {
        echo "<br><br>";
        $error->setAll($error_cache);
        $error->printAll();
    }
    
}

function Form_go7( $sqlo, $sqlo2 ) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    $tablename = $this->tablename;
    $next_go   = 10; // Standard: 10
    $this->htmlXObj->form_start_no_table($this->projid, $tablename, $next_go);
    
    $this->obj_extend_lib->show_form07($sqlo, $this->htmlXObj);
    if ($error->Got(READONLY))  {
        $error->printAllEasy();
        $error->reset();
    } else {
        $fopt=array('all_req_params'=>1);
        $this->htmlXObj->form_finish_pure($fopt);
    }
}

/**
 * after an error, go back to go=0
 * @param int $go
 */
function form_go_back($go) {
    $tablename = $this->tablename;
    $this->htmlXObj->form_start($this->projid, $tablename, $go, 'Error options');
    
    $subopt=array('submit.text'=>'Go Back');
    $advView=array();
    $this->htmlXObj->htmlRowSubmit($advView, $subopt );
    
    $finopt=array('all_req_params'=>1);
    $this->htmlXObj->form_finish( $finopt ); 
}
    

function set_infox($key, $val) {
    $this->infox[$key] = $val;
}

// if object creation wizard is used: set EDITMODE=EDIT for the object type
function set_editmode() {
    $_SESSION["s_sessVars"]["o.".$this->tablename.".editmode"]='edit';
}

function debug_show($key) {
    switch ($key) {
        case 'newparams':
            $val = $this->newparams;
            if (is_array($val)) $val=print_r($val,1);
            break;
        case 'ext.newparams':
            $val = $this->obj_extend_lib->get_param('newparams');
            if (is_array($val)) $val=print_r($val,1);
            break;
        default:
            $val='ERROR:KEY unknown!';
    }
    echo "DEBUG:debug_show: $key:$val<br>";
}

}
