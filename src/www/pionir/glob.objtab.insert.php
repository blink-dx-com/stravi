<?php
/**
 * [BulkCreate] Create a list of objects (as copy from clipboard)
 * @package glob.objtab.insert.php
 * @swreq UREQ:942 g > BulkCreate > HOME
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $tablename
  		  $go : 0, : params
  		  		1  : prep
  		  		2  : create
		  [$assoc_cpy] array[TABLE] = 1
		   $deep_cpy[ASSOC-table] = 0,1
		   $proj_id : ID of destination project
		   $parx['addName'] : add 'addName' to original name
 */
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("object.subs.inc");
require_once ("insertx.inc");
require_once ('class.history.inc');
require_once ('glob.obj.copyobj2.inc');
require_once ("glob.obj.update.inc");
require_once ('func_form.inc');
require_once ("visufuncs.inc");
require_once ("o.PROJ.addelems.inc");
	
class gObjTabInsSubOne {
	
	/**
	 * init class
	 * @param $doCrea 0 : prepare
	 * 				  1 : do create !
	 */
	function __construct($tablename, $proj_id, $doCrea) {
		$this->tablename = $tablename;
		$this->proj_id   = $proj_id;
		$this->creaFuncObj = new objCreaWiz($tablename);
		$this->UpdateLib = new globObjUpdate();
		$this->doCrea = $doCrea;
		$this->nameCol = importantNameGet2($tablename);
	}
	
	/**
	 * 
	 * @return 
	 * @param object $creaOpt
	 * @param object $assoc_cpy
	 * @param object $parx : 'addName'
	 */
	function setParams(&$sqlo, $creaOpt, $assoc_cpy, $parx) {
		$this->creaOpt=$creaOpt;
		$this->assoc_cpy=$assoc_cpy;
		$this->parx = $parx;
		$this->projAddLib = new oProjAddElem($sqlo, $this->proj_id);
	}
	
	function _getNewName( &$sqlo, $srcid ) {
		global $error;
		$FUNCNAME= '_getNewName';
		$tablename = $this->tablename;
		$nameCol   = $this->nameCol;
		
		$pkName  = PrimNameGet2($tablename);
		$oldName = glob_elementDataGet( $sqlo, $tablename, $pkName, $srcid, $nameCol);
		$newname = $oldName . $this->parx['addName'];
		
		// check, if exists in project
		$answer = $this->projAddLib->getObjByName( $sqlo, $tablename, $newname );
		if ($answer[0]>0) {
			$error->set( $FUNCNAME, 1, 'experiment with name "'.$newname.'" already in project. ' );
			return;
		}
		
		return ($newname);
	}
	
	function oneExp( &$sql, &$sql2, &$sql3, $obj_blueprint_id) {
		global $error;
		$FUNCNAME= 'oneExp';
		
		$tablename = $this->tablename;
		$newname = $this->_getNewName( $sql, $obj_blueprint_id );
		
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, '' );
			return;
		}
		if ( $this->doCrea<=0 ) return; // just prepare ...
		
		$dummy=array();
		
		$obj_new = $this->creaFuncObj->objCreate( 
				$sql, $sql2, $sql3, 
				$obj_blueprint_id, $dummy, $this->assoc_cpy, $this->creaOpt );
				
		if (!$obj_new or $error->Got(READONLY)) return;
		
		if ($this->parx['addName']!=NULL) {
			// add suffix to name
			$argu    = NULL;
			$argu[$this->nameCol] = $newname;
			$args = array('vals'=>$argu);
			$this->UpdateLib->update_meta( $sql, $tablename, $obj_new, $args );
		}
		
		return ($obj_new);
	}
	function getInfo() {
		return ( $this->creaFuncObj->getInfo() );
	}
}

class gObjtabInsert {
	/**
	 * user-quota
	 *   'MAXNUM' : INT : SUMMARY max number of objects for this user; if <0 : NO LIMIT
 	 *   'EXCEPT_USERS' : nick name of except users
 	 *   'MAX_NUM_INIT' : initial max number of objects per day
	 *   'OBJREST' : number of object still allowed to create
	 *   'CREATED' : number of objects created by user
	 * @var $quotaArr
	 */
	var $quotaArr;

function __construct(&$sqlo, $tablename, $proj_id, $parx, $go, $assoc_cpy, $deep_cpy) {
	$this->tablename = $tablename;
	$this->go = $go;
	$this->proj_id = $proj_id;
	$this->parx = $parx;
	
	$this->assoc_cpy=$assoc_cpy;
	$this->deep_cpy =$deep_cpy;
	$this->tablenice = tablename_nice2($tablename);
	
	$this->quotaArr = $this->getQuota($sqlo);
	
	$this->quotaArr['OBJREST'] = -1;
	
	if ($this->quotaArr['MAXNUM']>0) {
		
		$this->quotaArr['OBJREST'] = 0; // init the value
		// get current number of objects of user
		$numObjects = $this->getNumOfObjectsUser($sqlo);
		$this->quotaArr['CREATED'] = $numObjects;
		
		$rest = $this->quotaArr['MAXNUM'] - $numObjects;
		if ($rest<=0) $rest=0;
		$this->quotaArr['OBJREST'] = $rest;
	}
	
	if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
	    glob_printr( $this->quotaArr, 'this->quotaArr' );
	}
}

/**
 * get quota info
 * @param  $sqlo
 * @return array $quotaArr
 *   
 */
private function getQuota(&$sqlo) {
	
	$quotaInfo = trim(glob_elementDataGet( $sqlo, 'GLOBALS', 'NAME', 'app.glob.objtab.insert', 'VALUE')); 
	if ($quotaInfo==NULL) return array( 'MAXNUM'=> -1, 'INFO'=>'no quotas set' );
	
	$dataAsArray = explode(';', $quotaInfo);
	
	
	// no data ?
	if ( !is_array($dataAsArray) ) {
		 return array( 'MAXNUM'=> -1, 'INFO'=>'setting contains no data'  );
	}
	
	// the QUOTA-initialization is active
	$quotaArr = array('MAXNUM'=>0); // set to create == DENY
	
	// $keyVal = 'KEY:VAL'
	foreach($dataAsArray as $keyValString) {
		
		$KeyValArr = explode(':',$keyValString);
		switch ($KeyValArr[0]) {
			case 'MAXNUM_PER_DAY':
				$val = intval($KeyValArr[1]);
				$quotaArr['MAX_NUM_INIT'] = $val;
				break;
			case 'EXCEPT_USERS':
				$userArr = explode(',',$KeyValArr[1]);
				$quotaArr['EXCEPT_USERS'] = $userArr;
				break;
		}
		
		if ($quotaArr['MAX_NUM_INIT']>0) $quotaArr['MAXNUM'] = $quotaArr['MAX_NUM_INIT'];
		
		if ( is_array($quotaArr['EXCEPT_USERS']) ) {
			if ( in_array($_SESSION['sec']['appuser'], $quotaArr['EXCEPT_USERS']) ) {
				$quotaArr['MAXNUM'] = -1; // no quota for user
			}
		}
	}
	
	return $quotaArr;
}

/**
 * get number of objects of type $this->tablename of USER $_SESSION['sec']['db_user_id'] in last 24 hours 
 * @param $sqlo
 */
private function getNumOfObjectsUser(&$sqlo) {
	$sqlsel = 'count(1) from CCT_ACCESS where DB_USER_ID='.$_SESSION['sec']['db_user_id'].
		' and TABLE_NAME='.$sqlo->addQuotes($this->tablename). ' and CREA_DATE>=(CURRENT_DATE -1)';
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$numObjects = $sqlo->RowData[0];
	
	return $numObjects;
}

function GoInfo($go, $coltxt=NULL) {
	
	$goArray   = array( "0"=>"Give parameters", 1=>"Prepare", 2=>"Create" );
	$extratext = '[<a href="'.$_SERVER['PHP_SELF'].'?tablename='.$this->tablename.'">Start again</a>]';
	
	$formPageLib = new FormPageC();
	$formPageLib->init( $goArray, $extratext );
	$formPageLib->goInfo( $go ); 

}

function initTests( &$sql ) {
	
	global $error;
	$FUNCNAME= "initTests";
	
	$tablename = $this->tablename;
	$tablenice = tablename_nice2($tablename);
	
	$numObjectsSelect = sizeof($_SESSION['s_clipboard']);
	
	if ( sizeof($_SESSION['s_clipboard'])<=0 ) {
		$error->set( $FUNCNAME, 1, "Please copy objects of type '".$tablenice."' to clipboard");
		return;
	}
	$pkArr = primary_keys_get2($tablename);
	if ( sizeof($pkArr)>1 ) {
		$error->set( $FUNCNAME, 2, "Function only allowed for Business objects");
		return;
	}
	// 
	//   check   R I G H T S
	//
	$t_rights = tableAccessCheck( $sql, $tablename );
	if ( $t_rights["insert"] != 1 ) {
		$answer = getTableAccessMsg( $tablename, "insert" );
		$error->set( $FUNCNAME, 4, $answer );
		return;
	}
	
	// check quota
	// @swreq UREQ:942:010: BulkCreate-Quota: optional limit number of objects per user/day to a max number: 
	if ($this->quotaArr['OBJREST']>=0 and $numObjectsSelect>$this->quotaArr['OBJREST']) {
		$error->set( $FUNCNAME, 5, 'your BulkCreate-Quota exceeded. Please ask the Admin to increase your Quota.' );
		return;
	}
	
	if ( $this->go > 0 ) {
		$proj_id = $this->proj_id;
		if ( !$proj_id ) {
			$error->set( $FUNCNAME, 3, "please select a destination project first");
			return;
		}
		
		$o_rights = access_check($sql, "PROJ", $proj_id);
		if ( !$o_rights["insert"]) {
			$error->set( $FUNCNAME, 4,  "You do not have insert permission on the destination project!");
			return;
		}
	}

}

function getProjObjs( &$sql ) {
	// FUNCTION: get number of objects in project
	$FUNCNAME="getProjObjs";
	$proj_id = $this->proj_id;
	$tablename = $this->tablename;
	$sqls = "select count(PRIM_KEY) from PROJ_HAS_ELEM where PROJ_ID=".$proj_id." AND TABLE_NAME='".$tablename."'";
	$sql->query($sqls, $FUNCNAME);
	$sql->ReadRow();
	$retval = $sql->RowData[0];
	return ($retval);
}

function formshow(&$sqlo) {

	
	
	$tablename = $this->tablename;
	$assocArr  = $this->assocTabs;
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Select parameters for insert";
	$initarr["submittitle"] = "Next &gt;";
	$initarr["tabwidth"]    = "AUTO";
	$initarr["tabnowrap"]   = "1";
	$initarr["dblink"]  	= 1;
	
	$hiddenarr = NULL;
	$hiddenarr["tablename"]     = $tablename;
	$formobj = new formc($initarr, $hiddenarr, 0);
	
	if (!$this->proj_id) {
		$hist_obj  = new historyc();
		if ( !$proj_id ) $proj_id = $hist_obj->last_bo_get( "PROJ" );
	} else $proj_id = $this->proj_id;
	if ($proj_id) $objname= obj_nice_name ( $sqlo, 'PROJ', $proj_id ); 
	else $objname='---';
	
	$fieldx = array ( 
		"title" => "Destination project", 
		"name"  => "proj_id",
		"object"=> "dblink",
		"namex" => TRUE,
		"val"   => $proj_id, 
		"inits" => array(
					"table"=>"PROJ",
					"objname"=>$objname,
					"pos" =>  0,
					"projlink"=> 1 
					),
		"notes" => "destination"
		 );
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ( 
		"title" => "Name Addition", 
		"name"  => "addName",
		"object"=> "text",
		"val"   => ' COPY', 
		"notes" => "add a suffix"
		 );
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ( "object"=> "hr");
	$formobj->fieldOut( $fieldx );
	
	if (sizeof($assocArr)) {
		foreach( $assocArr as $tmparr) {
			$key = current($tmparr);
			next($tmparr);
			$val = current($tmparr);
			$optnotes=NULL;
			if ($tablename=="EXP" AND $key=="EXP_HAS_PROTO") {
				$optnotes = " | deep-copy <input type=checkbox name=\"deep_cpy[".$key."]=1\">";
			}
			
			$fieldx = array ( "title" => $val, "name"  => "assoc_cpy[".$key."]",
					"object" => "checkbox",
					"val"   => 1, 
					"namex" => 1,
					"notes" => "copy associated elements".$optnotes );
			$formobj->fieldOut( $fieldx );
		}
	}
	$formobj->close( TRUE );

}

function form1(&$sqlo) {

	
	
	$tablename = $this->tablename;
	$assocArr  = $this->assocTabs;
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Create";
	$initarr["submittitle"] = "Create";
	$initarr["tabwidth"]    = "AUTO";
	$initarr["tabnowrap"]   = "1";
	
	$hiddenarr = NULL;
	$hiddenarr['tablename']   = $tablename;
	$hiddenarr['proj_id']     = $this->proj_id;
	
	if ( sizeof($this->assoc_cpy)) {
		foreach( $this->assoc_cpy as $key=>$val) {
			$hiddenarr['assoc_cpy['.$key.']'] = $val;
		}
		reset ($this->assoc_cpy); 
	}
	
	if (sizeof($this->deep_cpy)) {
		foreach( $this->assoc_cpy as $key=>$val) {
			$hiddenarr['deep_cpy['.$key.']'] = $val;
		}
		reset ($this->deep_cpy); 
	}

	$formobj = new formc($initarr, $hiddenarr, 1);
	$formobj->addHiddenParx($this->parx);
	$formobj->close( TRUE );
	
		
}

function _info($key, $text) {
	$dataArr = array('<font color=gray>'.$key.'</font>', $text);
	$this->infoobj->table_row ($dataArr);
}

function preInfo( &$sql ) {
	$proj_id    = $this->proj_id;
	$tablename  = $this->tablename;
	$numClipObj = sizeof( $_SESSION["s_clipboard"] );
	
	$this->infoobj = new visufuncs();
	$headOpt = array( "title" => "Parameter summary", "headNoShow" =>1 );
	$headx  = array ("Key", "Value");
	$this->infoobj->table_head($headx,   $headOpt);
	
	$this->_info('Creates objects','<font size=+1><b>'.$numClipObj.'</b></font>');
	
	if ($this->go>0) {
		$projname = obj_nice_name($sql, "PROJ",  $proj_id);
		$this->_info( "Destination project <img src=\"images/icon.PROJ.gif\">",
			 "<b>".$projname."</b> [<a href=\"edit.tmpl.php?t=PROJ&id=".$proj_id."\">ID:$proj_id</a>]\n");
		
		if ($this->parx['addName']!=NULL) {
			$this->_info(  "Add suffix", "'<b>".$this->parx['addName']."</B>'");
		}
		
	}
	$this->infoobj->table_close();
	echo "<br>\n";
	
	if ( $this->go>0 ) {
		$numexp = $this->getProjObjs($sql);
		if ($numexp>0) {
			htmlInfoBox( "Warning", "Already <b>$numexp</b> objects (type ".$this->tablenice.") in project.", "", "INFO" );
			echo "<br>";
		}
	}
}

function showHelp() {
	echo "<br>";
	htmlInfoBox( "Short Help", "", "open", "INFO" );
	
	echo "This tool creates copies of objects (selected by the clipboard).<br>";
	echo "The function stops, if one single error occurs (e.g. duplicate names not allowed).<br>";
	echo "<br>\n";
	echo "The number of objects per USER per DAY of this OBJECT-TYPE can be limited by the Admin!<br>";
	echo 'KEY:app.glob.objtab.insert : <br>';#
	echo '- MAXNUM_PER_DAY:{INT} max number of objects per user/day <br>';
    echo '- EXCEPT_USERS:{usernick1},{usernick2}<br>';
    echo 'your QUOTA: ';
    if ($this->quotaArr['MAXNUM']>=0 ) echo "<b>".$this->quotaArr['MAXNUM']."</b> objects per day";
    else echo 'none';
    echo "<br>";
    if ($this->quotaArr['OBJREST']>=0) echo "you can still create: <b>".$this->quotaArr['OBJREST']."</b> objects this day";
    echo "<br>";
    echo "you already created: <b>".$this->quotaArr['CREATED']."</b> objects in the last 24 hours";
	echo "<br>";
	
	htmlInfoBox( "", "", "close", "INFO" );
}

function initData( &$sql ) {
	$tablename = $this->tablename;
	$this->assocTabs = get_assoc_tables2($sql, $tablename);
}



function doloop( &$sql,  &$sql2,  &$sql3 ) {
	/* do the object creation
	*/
	
	global $error;
	$FUNCNAME= "doloop";
	
	$tablename = $this->tablename;
	$proj_id   = $this->proj_id;
	$assoc_cpy = $this->assoc_cpy;
	$deep_cpy  = $this->deep_cpy;
	$go = $this->go;
	
	$creaOpt   = NULL;
	$creaOpt["info"]    = 1;
	$creaOpt["proj_id"] = $proj_id;
	if ( $deep_cpy != NULL ) {
		$creaOpt["objAssCrea"]  = $deep_cpy;
	}
	
	$creaOneLib = new gObjTabInsSubOne($tablename, $proj_id, $this->go-1);
	$creaOneLib->setParams($sql, $creaOpt, $assoc_cpy, $this->parx);
	
	$obj_name = "";
	
	
	reset ($_SESSION['s_clipboard']);	
	$this->analysed	   = 0;
	$errorcnt  = 0;
	$dummy = NULL;
	
	foreach( $_SESSION['s_clipboard'] as $th0=>$th1) { 
	
		$tmp_tablename = current($th1);
	
		$id0=next($th1);
		$id1=next($th1);
		$id2=next($th1);
		if ( $tmp_tablename!=$tablename ) continue;
		
		$obj_blueprint_id = $id0;
		
		$tmpName = obj_nice_name($sql, $tablename, $obj_blueprint_id );
		
		echo "<img src=\"images/icon.".$tablename.".gif\"> ".
			  ($this->analysed+1).". Origin: ID:$obj_blueprint_id <b>$tmpName</b> ";
		
		$obj_new = $creaOneLib->oneExp( $sql, $sql2, $sql3, $obj_blueprint_id);
		
		$xinfo = $creaOneLib->getInfo();
		if ($xinfo!=NULL) echo " Info: <br>".$xinfo;
		
		if ( $go>1 ) {
			echo " &nbsp;&nbsp;&nbsp;&nbsp;NewID: <b>$obj_new</b>";
		}
		echo "<br>";
		
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, "Error occurred." );
			return;
		}
		
		$this->analysed++; 
		
	}
	
	echo "<hr>";
	
	
}

}


// ---------------------------

global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( );
$sql2  = logon2( );
$sql3  = logon2( );
if ($error->printLast()) htmlFoot();
$varcol    = & Varcols::get();

$tablename=$_REQUEST['tablename'];
$go=$_REQUEST['go'];
$assoc_cpy=$_REQUEST['assoc_cpy'];
$deep_cpy=$_REQUEST['deep_cpy'];
$proj_id =$_REQUEST['proj_id '];
$parx=$_REQUEST['parx'];



$numClipObj = sizeof($_SESSION['s_clipboard']);


$title       = "BulkCreate: create real objects as copies from clipboard";
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["title_sh"] = 'BulkCreate';
$infoarr["form_type"]= "list";
$infoarr['obj_more'] = ' number of copied objects: <b>'.$numClipObj.'</b>';
$infoarr["obj_name"] = $tablename;
// $infoarr["obj_cnt"]= 1;


$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

echo "<ul>";

if ($tablename=="") {
	htmlFoot("Error","No table given");
} 
$mainlib = new gObjtabInsert($sql, $tablename, $proj_id, $parx, $go, $assoc_cpy, $deep_cpy);
$mainlib->GoInfo($go);
$mainlib->initTests( $sql );
if ($error->Got(READONLY))  {
	$tablenice = tablename_nice2($tablename);
	$error->set( 'main', 1, 'UserProblem on BulkCreate for table '.$tablenice );
	$error->printAll();
	$mainlib->showHelp();
	htmlFoot();
}

$mainlib->preInfo( $sql );
$mainlib->initData( $sql );

if ( !$go ) {
	$mainlib->formshow($sql) ;
	$mainlib->showHelp();

	htmlFoot();
}




if ( $go==1 ) $mainlib->form1($sql);

$mainlib->doloop( $sql, $sql2, $sql3 );
echo "<br><b>".$mainlib->analysed."</b> objects analysed.<br>";
if ( $error->printAll() ) {
	htmlFoot();
}

htmlFoot("<hr>");


