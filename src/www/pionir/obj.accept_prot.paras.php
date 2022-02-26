<?php
/**
 * insert/update ACCEPT_PROT_STEP parameters
 * @package obj.accept_prot.paras.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq SREQ:0001754: o.ACCEPT_PROT > manage edit/show of acceptance steps
 * @param <pre>
 *    $id (ACCEPT_PROT)
	  $qmin[]
	  $qmax[]
	  $qnotes[]
	  $backurl (encoded URL)
	  
 * </pre>
 */ 
session_start(); 


require_once('reqnormal.inc'); 
require_once('javascript.inc' );
require_once('o.ACCEPT_PROT.mod.inc');
require_once('o.ABSTRACT_PROTO.stepx.inc');
require_once('glob.obj.update.inc');

class oACCEPT_PROT_paras {
	
function __construct(&$sqlo, $protoid) {
	global $error;
	$FUNCNAME= 'oACCEPT_PROT_paras';

	$this->protoid = $protoid;
	$this->errcnt=0;
	
	$sqls= "select ABSTRACT_PROTO_ID from ACCEPT_PROT where ACCEPT_PROT_ID=".$protoid;
	$sqlo->query($sqls);
	$sqlo->ReadRow();
	$a_proto_id	   = $sqlo->RowData[0];
	if (!$a_proto_id) {
		$error->set( $FUNCNAME, 1, 'no abstract protocol in concrete protocol defined!');
		return;
	} 
	$this->a_proto_id=$a_proto_id;
}

function flagProblem($stepnr, $problem) {
	echo '<font color=red><b>Error:</b></font> Step-Nr:<b>'.$stepnr. '</b> '.$problem.'<br />'."\n";
	$this->errcnt++;
}

function updateSteps(&$sqlo, &$sqlo2) {
	global $error;
	$FUNCNAME= 'updateSteps';
	
	$a_proto_id=$this->a_proto_id;
	$accept_proto_id = $this->protoid;
	
	
	// read all steps from ABSTRACT_PROTO
	$absProtLib = new oABSTRACT_PROTO_stepx();
	$absProtLib->setObj($a_proto_id);
	$steparr = $absProtLib->getStepArray($sqlo);
	
	
	$protoManiLib = new oACCEPT_PROT_mod();
	$protoManiLib->setCprotoID($sqlo,  $accept_proto_id);
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, 'error on init.' );
		return;
	}
	
	$this_debug = 0;
	if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
	    $this_debug = $_SESSION["userGlob"]["g.debugLevel"];
	}
	
	$this->updateCnt=0;
	
	
	
	foreach( $steparr as $dummy=>$tmparr) {
		
		$step_nr = $tmparr[0];
	    $new = array();
	    $new["MIN_VAL"] = $_REQUEST['qmin']   [$step_nr];
	    $new["MAX_VAL"] = $_REQUEST['qmax']   [$step_nr];
	    $new["NOTES"]   = $_REQUEST['qnotes'] [$step_nr];
	   
	    do {
	    	$new["MIN_VAL"] = trim($new["MIN_VAL"]);
	    	$new["MAX_VAL"] = trim($new["MAX_VAL"]);
	    	$new["NOTES"] = trim($new["NOTES"]);
	    	
		    if ( $new["MIN_VAL"]!=NULL and !is_numeric($new["MIN_VAL"])) {
	    		$this->flagProblem($step_nr, 'Value of of MIN_VAL ('.$new["MIN_VAL"].
	    			') is not a valid number! (may be used a komma instead of point?)');
	    		break;
		    	
		    }
	    	if ( $new["MAX_VAL"]!=NULL and !is_numeric($new["MAX_VAL"])) {
	    		$this->flagProblem($step_nr, 'Value of of MAX_VAL ('.$new["MAX_VAL"].
	    			') is not a valid number! (may be used a komma instead of point?)');
	    		break;
		    }
		    
		    if ( is_numeric($new["MIN_VAL"]) and is_numeric($new["MAX_VAL"]) and 
		    	$new["MIN_VAL"]>$new["MAX_VAL"] ) {
		    	$this->flagProblem($step_nr, 'MIN_VAL bigger than MAX_VAL (MIN: '.$new["MIN_VAL"].
	    			', MAX:'.$new["MAX_VAL"].')');
	    		break;
		    }
		    
		    
		    $info = $protoManiLib->oneStepSave( $sqlo, $new, $step_nr );
		    if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 1, 'error on step '.$step_nr );
				return;
			}
			if ( ($info!="") and ($this_debug>1) ) echo 'INFO: Step-Nr:'.$step_nr.' '. $info ."<br>\n";
			$this->updateCnt++;
	    } while (0);
	    
	}  
	
	if ($this->errcnt) {
		$error->set( $FUNCNAME, 1, 'User errors occurred.', 1 );
	}
	
	echo "<br>\n";
	
}

}
 		
// ---------------------------------------------------------------


$sqlo   = logon2();	
$sqlo2  = logon2();
$error = & ErrorHandler::get();
if ($error->printLast()) htmlFoot();

$id = $_REQUEST['id'];
$backurl = $_REQUEST['backurl'];

$tablename='ACCEPT_PROT';
$title = 'insert/update steps of a acceptance protocol';

$infoarr = NULL;
$infoarr['title'] = $title;
$infoarr['form_type']= 'obj'; // 'tool', 'list'
$infoarr['design']   = 'norm';
$infoarr['obj_name'] = $tablename;
$infoarr['obj_id']   = $id;
$infoarr['checkid']  = 1;

$pagelib = new gHtmlHead();
$pagelib->startPage( $sqlo, $infoarr);

$accCheckArr = array('tab'=>array('write'), 'obj'=>array('insert') );
$pagelib->do_objAccChk($sqlo, $accCheckArr);

echo '<ul>' ."\n";

if (!$id) {
	$pagelib->htmlFoot('ERROR', 'no acceptance protocol given !');
}   

$MainLib = new oACCEPT_PROT_paras($sqlo, $id);
$pagelib->chkErrStop();

$stopForward = 0;


$MainLib->updateSteps($sqlo, $sqlo2);

if ( $error->got() ) {
	$error->printAllPrio(9);
	$stopForward = 1;
}

echo '<br>... '.$MainLib->updateCnt. ' steps updated.<br>';


if ($backurl=="") $nexturl = "edit.tmpl.php?t=".$tablename."&id=".$id."#goodies";
else $nexturl = urldecode($backurl);
if ( $stopForward ) $stopopt =  array($stopForward, 'error occured');
else $stopopt = NULL;
js__location_replace( $nexturl, "back to protocol $id", $stopopt);

htmlFoot();

