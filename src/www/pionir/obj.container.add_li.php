<?php
/**
 * add selected substances to container
 * 
 * - special actions for aliquots
 * @namespace core::gui
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param  $parx["CONTAINER_ID"]
		   $parx["withAliquots"] : 0,1
		     
		   $go: 
		     0: prepare
		     1: show possible aliquot management
		     2: forward
 * @global 	$_SESSION['userGlob']["o.CONTAINER.fill"] options for aliquots  
 */ 
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("sql_query_dyn.inc");
require_once ("f.visu_list.inc");     // sub function LIST visualization
require_once ('glob.objtab.page.inc');
require_once ("o.CONTAINER.mod.inc");
require_once ("f.objview.inc");	

class oContAddSubList {

function __construct( &$listPageLib, $go, $parx ) {
	$this->parx=$parx;
	$this->go=$go;
	$this->sqlAfter  = $listPageLib->getSqlAfter();
	$this->sqlAfterNoOrd  = $listPageLib->getSqlAfterNoOrd();
	$dummy=NULL;
	$this->contModLib = new oContainerModC ($dummy);
	
	$this->userOpts = NULL;
	$varx = $_SESSION['userGlob']["o.CONTAINER.fill"];
	if ($varx!=NULL) $this->userOpts = unserialize($varx);
	
	
}

function saveOptions() {
	$this->userOpts['id'] =$this->parx["CONTAINER_ID"];
	$this->userOpts['useAli'] = $this->parx["withAliquots"];
	$_SESSION['userGlob']["o.CONTAINER.fill"] = serialize($this->userOpts);
	
}

function form1( &$sqlo ) {
	require_once ('func_form.inc');
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Select reservation mode";
	$initarr["submittitle"] = "Next &gt;&gt;";
	$initarr["tabwidth"]    = "AUTO";
 	$initarr["dblink"]		= 1;
	
	$hiddenarr = NULL;
	
	$lastContainer = $this->userOpts['id'];

	$formobj = new formc($initarr, $hiddenarr, 0);
	
	
	$inits = array( "table"=>"CONTAINER", "pos" =>0, "projlink"=> 1, 'noshDel' =>1 );
	if ($lastContainer) {
		$inits['getObjName'] = 1;
		$inits['sqlo'] = &$sqlo;
	}

	$fieldx = array ( 
		"title" => "Container", 
		"name"  => "CONTAINER_ID",
		"object"=> "dblink",
		"val"   => $lastContainer, 
		"inits" => $inits,
		"notes" => "the destination container"
		 );
	$formobj->fieldOut( $fieldx );
	$fieldx = array ( 
		"title" => "with Aliquots?", 
		"name"  => "withAliquots",
		"object"=> "checkbox",
		"val"   => $this->userOpts['useAli'], 
		"notes" => "brings you to a special Aliquot form"
		 );
	$formobj->fieldOut( $fieldx );
	

	$formobj->close( TRUE );
}

function form1_ali(&$sqlo) {
	require_once ('func_form.inc');
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Define aliquots per substance (abstract)";
	$initarr["submittitle"] = "Next &gt;&gt;";
	$initarr["tabwidth"]    = "AUTO";
 	$initarr["dblink"]		= 1;
	
	$hiddenarr = NULL;
	$hiddenarr['parx[CONTAINER_ID]']= $this->parx['CONTAINER_ID'];
	$hiddenarr['parx[withAliquots]']= 1;

	$formobj = new formc($initarr, $hiddenarr, 1);
	
	$sqlAfter = $this->sqlAfterNoOrd;
	
	$sqlsel = "distinct(x.ABSTRACT_SUBST_ID)  FROM ".$sqlAfter;
	$sqlo->Quesel($sqlsel);
	$MAX_SUBST=5; // max 5 different subst
	$cnt=0;
	$substArr=NULL;
	$storedSubstanceArray = $this->userOpts['subsAli'];
	
	while ( $sqlo->ReadRow() ) {
	    $aSubstID = $sqlo->RowData[0];
	    
	    if ($storedSubstanceArray[$aSubstID]>0) $useAliquotNum = $storedSubstanceArray[$aSubstID];
	    else $useAliquotNum=1;
	    
	    if ($cnt>$MAX_SUBST) {
	    	echo 'max '.$MAX_SUBST.' abstract substs allowed.<br>';
	    	break;
	    }
	    $cnt++;
	    $substArr[$aSubstID] = $useAliquotNum;
	}
	
	if (sizeof($substArr)) {
		foreach( $substArr as $aSubstID=>$aliNum) {
		
			$fieldx = array ( 
			"title" =>  obj_nice_name ( $sqlo, 'ABSTRACT_SUBST', $aSubstID ),
			"name"  => "subsAli[".$aSubstID."]",
			"namex" => TRUE, 
			"object"=> "text",
			"val"   => $aliNum, 
			"notes" => ""
			 );
			 $formobj->fieldOut( $fieldx );
		}
		reset ($substArr); 
	}
	
	
	
	$formobj->close( TRUE );
}

function form2( &$sqlo ) {
	require_once ('func_form.inc');
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Prepare reservation";
	$initarr["submittitle"] = "Finish";
	$initarr["tabwidth"]    = "AUTO";
 	$initarr["dblink"]		= 1;
	
	$hiddenarr = NULL;
	$hiddenarr['parx[CONTAINER_ID]']=$this->contid;

	$formobj = new formc($initarr, $hiddenarr, 1);
	$formobj->close( TRUE );
}

function _showSubst(&$sqlo, $substid) {
	// get edit-link of BO + NAME + icon (object)
	$opts=NULL;
	$htmltxt = fObjViewC::bo_display( $sqlo,  "CONCRETE_SUBST",  $substid, $opts );
	echo "substance: ".$htmltxt." ";
	
}

function _addOneSubst(&$sqlo, $substid, $startpos) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$parxOne = array( 
		"CONTAINER_ID"=> $this->parx["CONTAINER_ID"],
		"substid"     => $substid
	);
	
	$this->contModLib->initSubst($parxOne);
	
	$this->_showSubst($sqlo, $substid);
	
	$isOnAPos = $this->contModLib->getPosOfSubst($sqlo);
	if ($isOnAPos) {
		$error->set( $FUNCNAME, 1, "substance already on this container.");
		return;
	}
	
	$newpos = $this->contModLib->add_check($sqlo, $startpos);
	if ($error->Got(READONLY))  {
		return;
	}
	
	if ( $this->go==2 ) {
	    $ali_id=0;
	    $this->contModLib->addAliquot($sqlo, $newpos, $substid, $ali_id);
		if ($error->Got(READONLY))  {
			return;
		}
	}
	echo " pos: ".$this->contModLib->newpos." ";
	
	return $newpos;
}

function _getCntSubst( &$sqlo ) {
	$sqls = "select count(1) from CONT_HAS_CSUBST where CONTAINER_ID=".$this->parx["CONTAINER_ID"];
	$sqlo->query($sqls);
	$sqlo->ReadRow();
	$retid = $sqlo->RowData[0];
	return ($retid);
}

function info1(&$sqlo) {
	// check container and give info
	global $error;
	$FUNCNAME= "info1";
	
	$contid = $this->parx["CONTAINER_ID"];
	$this->contid=$contid;
	
	$cntsubst = $this->_getCntSubst($sqlo);
	// get container info
	$opts=NULL;
	$htmltxt = fObjViewC::bo_display( $sqlo,  "CONTAINER",  $contid, $opts );
	echo "destination container: ".$htmltxt." &nbsp;has substances: <b>".$cntsubst."</b><br><br>\n";
	
	$this->contModLib->setContainer($sqlo, $contid);
	$this->contModLib->modiCheck($sqlo);
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 2, "Container access problems." );
		return;
	}
	
	
}

/**
 * analyse abstract_substance => aliquot_number
 * @param $sqlo
 * @param $subsAliquotArray
 */
function _analyseAliquotNumbers(&$sqlo, $subsAliquotArray) {
	
	if (!sizeof($subsAliquotArray)) return;
	
	$MAX_STORED_SUBST  = 10;
	
	$startCnt = 0;
	$newSizeExpect = sizeof($this->userOpts['subsAli']) + sizeof($subsAliquotArray);
	if ($newSizeExpect>$MAX_STORED_SUBST) {
		
		htmlInfoBox( "Many aliquot preferences", "Forget old aliquot preferences.", "", "INFO" );
		$startCnt = $newSizeExpect-$MAX_STORED_SUBST;
		
		$oldAliquots=NULL;
		reset ($this->userOpts['subsAli']);
		$cnt=0;
		foreach( $this->userOpts['subsAli'] as $abstract_subst_id=>$aliquotNum) {
			if ($cnt>$startCnt) {
				$oldAliquots[$abstract_subst_id]=$aliquotNum;
			}
			$cnt++;
		}
		reset ($this->userOpts['subsAli']); 
		
	} else {
		$oldAliquots = $this->userOpts['subsAli'];
	}
	
	
	$newSubstanceArray = $oldAliquots;
	foreach( $subsAliquotArray as $abstract_subst_id=>$aliquotNum) {
		$newSubstanceArray[$abstract_subst_id]=$aliquotNum;
	}
	reset ($subsAliquotArray); 
	
	$this->userOpts['subsAli']= $newSubstanceArray;
	$this->saveOptions();
}


/**
 * send params to DEF/o.CONTAINER.addSubst_li.inc
 * @param $sqlo
 * @return -
 */
function sendToForm(&$sqlo, $absSubstanceAliquots) {
	global $error;
	$FUNCNAME= 'sendToForm';

	$MAX_SUBST_NUM=50;
	
	echo '... max supported Substance-number: '.$MAX_SUBST_NUM.'.<br>';
	
	$this->_analyseAliquotNumbers($sqlo, $absSubstanceAliquots);
	
	$sqlAfter = $this->sqlAfter;
	$contid   = $this->parx['CONTAINER_ID'];
	$primary_key = "CONCRETE_SUBST_ID";
	

	$sqlsel = "count(1) FROM ".$sqlAfter;
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$substcnt = $sqlo->RowData[0];
	if ($substcnt>$MAX_SUBST_NUM) {
		$error->set( $FUNCNAME, 1, 'Too many substances ('.$substcnt.'). Max '.$MAX_SUBST_NUM.' allowed.' );
		return;
	}
	
	$sqlsLoop = "SELECT x.".$primary_key." FROM ".$sqlAfter;
	$sqlo->query($sqlsLoop);
	
	$cnt=0;
	
	echo '<form style="display:inline;" method="post" '.
		 ' name="editform"  action="p.php?mod=DEF/o.CONTAINER.addSubst_li" >'."\n";
	echo '<input type=hidden name="id" value="'.$contid.'">'."\n";
	
	while ( $sqlo->ReadRow() ) {
		$substid = $sqlo->RowData[0];
		echo '<input type=hidden name="substarr['.$substid.']" value="1">'."\n";
		$cnt++;
	}
	echo "</form>";
	
	?>
	<script language="JavaScript">
		document.editform.submit();
	</script>
	<?php
	
	echo '... page will be forwarded. Please wait.<br>';
	
	return; 
	
}

function addnow(&$sqlo, &$sqlo2) {
	global $error;
	$FUNCNAME= "addnow";
	
	$dummy=NULL;
	
	$primary_key = "CONCRETE_SUBST_ID";
	$sqlAfter = $this->sqlAfter;
	$sqlsLoop = "SELECT x.".$primary_key." FROM ".$sqlAfter;
	$sqlo2->query($sqlsLoop);
	$cnt=0;
	$startpos=0;
	
	while ( $sqlo2->ReadRow() ) {
		echo ($cnt+1).". ";
		$substid = $sqlo2->RowData[0];
		$newpos = $this->_addOneSubst($sqlo, $substid, $startpos);
		if ($error->Got(READONLY))  {
			$errLast   = $error->getLast();
     		$error_txt = $errLast->text;
			echo "<font color=red>Error:</font> ".$error_txt;
			$error->reset();
		} else {
			echo "o.k.";
			$startpos = $newpos+1;
		}
		echo "<br>";
		$cnt++;
	}
}

}
// --------------------------------------------------- 
global $error, $varcol;
$FUNCNAME='MAIN';

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
$sqlo2 = logon2( );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$tablename			 = "CONCRETE_SUBST";

$go = $_REQUEST["go"];
$parx = &$_REQUEST["parx"];

$i_tableNiceName 	 = tablename_nice2($tablename);

$title       		 = "Add selected substances to a container";
$infoarr			 = NULL;
$infoarr["scriptID"] = "";
$infoarr["title"] = $title;
$infoarr["title_sh"] = 'Add to container';
$infoarr["form_type"]= "list";
$infoarr["sql_obj"]  = &$sqlo;
$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1;          // show number of objects

$listPageLib = new gObjTabPage($sqlo, $tablename );
$listPageLib->showHead($sqlo, $infoarr);
$listPageLib->initCheck($sqlo);
echo "<ul>";


$mainlib = new oContAddSubList( $listPageLib, $go, $parx);

if ( !glob_table_exists("CONTAINER") ) {
	htmlFoot("Error","table CONTAINER does not exist. Please ask your admin.");
}

if ( !$go ) {
	$mainlib->form1($sqlo);
	htmlFoot();
}

$contid = $parx["CONTAINER_ID"];
if ( !$contid ) {
	$error->set( $FUNCNAME, 1, "Please select a container." );
	return;
}

$mainlib->saveOptions();

if ($parx['withAliquots'] > 0) {
	if ($go==1) {
		$mainlib->form1_ali($sqlo);
	}
	if ($go==2) {
		$mainlib->sendToForm($sqlo, $_REQUEST['subsAli']);
	}
	$error->printAll();
	htmlFoot("<hr>");
}

$mainlib->info1($sqlo);
if ($error->Got(READONLY))  {
	$error->printAll();
	htmlFoot();
}

if ( $_REQUEST["go"]==1 ) {
	$mainlib->form2($sqlo);
	echo '<br>';
}

$mainlib->addnow($sqlo, $sqlo2);

$error->printAll();
htmlFoot("<hr>");
