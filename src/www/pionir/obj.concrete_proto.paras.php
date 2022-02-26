<?php
/**
 * insert/update concrete protocol step parameters
 * @package obj.concrete_proto.paras.php
 * @swreq  SREQ:0001016: g > protocol editor modul; level 2 
 * @author Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/obj.concrete_proto.paras.php 59 2018-11-21 09:04:09Z $
 * @param   
  	  int $conc_proto_id : CONCRETE_PROTO_ID
  	  int $mo_suc_id     : [OPTIONAL] mother SUC-ID
  	  
  	  ---- follwing arrays use the STEP_NO as index ---
	  $quanti[]
	  $newnote[]
	  $concsubst[] CONCRETE_SUBST_ID
	  $devids[]	   CHIP_READER
	  $not_done[]
	  
	  $_FILES[att] [step_no]   -- files to upload as attachment
	  $fileinfo[]  -- attachment info: array of ('attkey': attachment key )
	  ----- end of step arrays ---
	  
	  $backurl (encoded URL)
	  $xargu[]	
	     'EXEC_DATE', 'NOTES' -- feature of concrete_proto; will be updated
	     'VARIO.DATAURL' TBD: ...
*/
session_start(); 


require_once('reqnormal.inc'); 
require_once('javascript.inc' );
require_once('o.PROTO.subs.inc');
require_once('glob.obj.update.inc');
require_once 'date_funcs.inc';
require_once 'lev1/o.SATTACH.upload.inc';

class oCONCRETE_PROTO_paras {
	
	var $cp_feats; // array of features
	
	function __construct() {
		
		$this->errcnt=0;
		$this->debug = $_SESSION["userGlob"]["g.debugLevel"];
		
		
		$this->_debug = 0;
		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
		    $this->_debug = $_SESSION["userGlob"]["g.debugLevel"];
		}
	}
	
	function setProtocol(&$sqlo, $conc_proto_id) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->errcnt=0;
		$this->protoid = $conc_proto_id;
		
		
		$sqls= "ABSTRACT_PROTO_ID, CCT_ACCESS_ID from CONCRETE_PROTO where CONCRETE_PROTO_ID=".$conc_proto_id;
		$sqlo->Quesel($sqls);
		$sqlo->ReadArray();
		
		$this->cp_feats = $sqlo->RowData;
		
		$a_proto_id	   = $this->cp_feats['ABSTRACT_PROTO_ID'];
		if (!$a_proto_id) {
			$error->set( $FUNCNAME, 1, 'no abstract protocol in concrete protocol defined !' );
			return;
		} 
		
		$o_rights = access_check($sqlo, 'CONCRETE_PROTO', $conc_proto_id);
		if ( !$o_rights['insert'] ) {
		    $error->set( $FUNCNAME, 2, 'no insert-access to protocol');
		}
		
	}

	/**
	 * update CONCRETE_PROTO features
	 * @param array $xargu  : 'EXEC_DATE', 'NOTES', 'VARIO.DATAURL'
	 */
	function updateFeatures( &$sqlo, $xargu ) {
		global $error;
		$FUNCNAME= 'updateFeatures';
		
		// norml FEATURES
		$allowArr = array('NOTES', 'EXEC_DATE');
		$objid = $this->protoid;
		
		$argu  = NULL;
		foreach( $xargu as $key=>$val) { 
			if ( in_array( $key, $allowArr ) ) {
				$argu[$key] = $val;
			}
		}
		$args     = array( 'vals'=>$argu );
		
		// VARIO
		if (array_key_exists('VARIO.DATAURL', $xargu)) {
		    $args['vario']=array('DATAURL'=>$xargu['VARIO.DATAURL']);
		}
		
		
		$ObjUpLib = new globObjUpdate();
		$ObjUpLib->update_meta( $sqlo, 'CONCRETE_PROTO', $objid, $args );
		if ($error->Got(READONLY))  {
	    	$error->set( $FUNCNAME, 1, 'update features failed.' );
	    	return;
		}
		if ($this->debug>0) echo "... some protocol attributes updated.<br>";
	}
	
	function flagProblem($stepnr, $problem) {
		echo '<font color=red><b>Error:</b></font> Step-Nr: <b>'.$stepnr. '</b> '.$problem.'<br />'."\n";
		$this->errcnt++;
	}
	
	/**
	 * update the steps
	 * - check $quanti[] for numeric
	 * - check $concsubst[] for numeric, if exists
	 * @param  $sqlo
	 * @param  $sqlo2
	 * @param  $a_proto_id
	 * @global $_REQUEST[protocol params]
	 */
	function updateSteps(&$sqlo, &$sqlo2) {
		global $error;
		$FUNCNAME= 'updateSteps';
		
		$conc_proto_id = $this->protoid;
		$a_proto_id    = $this->cp_feats['ABSTRACT_PROTO_ID'];
		
		// read all steps from ABSTRACT_PROTO
		$sqls= "select STEP_NR from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$a_proto_id. " ORDER by STEP_NR";
		$sqlo->query($sqls);
		
		while ( $sqlo->ReadRow() ) {
			$steparr[] = $sqlo->RowData[0];
		}    
		
		$protoManiLib = new oCprotoMani();
		$protoManiLib->setCprotoID($sqlo,  $conc_proto_id, $a_proto_id);
		
		
		$this->updateCnt=0;
		foreach( $steparr as $dummy=>$step_nr) {
			
		    $new=array();
		    $new["quantity"] = $_REQUEST['quanti']   [$step_nr];
		    $new["notes"]    = $_REQUEST['newnote']  [$step_nr];
		    $new["subst"]    = $_REQUEST['concsubst'][$step_nr];
			$new["dev"]      = $_REQUEST['devids']   [$step_nr];
		    $new["not_done"] = $_REQUEST['not_done'] [$step_nr];
		    
		    $new["quantity"] = trim($new["quantity"]);
		    
		    do {
			    if ($new["quantity"]!="") {
			    	if (!is_numeric($new["quantity"])) {
			    		$this->flagProblem($step_nr, 'Value for QUANTITY ('.$new["quantity"].
			    			') is not a valid number! (may be used a komma instead of point?)');
			    		break;
			    	}
			    }
			    
			    // check each substance if exists in DB
		    	if ($new["subst"]!=NULL) {
		    		if (!is_numeric($new["subst"])) {
			    		$this->flagProblem($step_nr, 'Value for Substance-ID ('.$new["subst"].
			    			') is not a valid number!');
			    		break;
			    	}
		    		$new["subst"] = trim($new["subst"]);
		    		if ( !gObject_exists ($sqlo, 'CONCRETE_SUBST', $new["subst"]) ) {
			    		$this->flagProblem($step_nr, 'Substance SUC-ID:'.$new["subst"].' does not exist in DB');
			    		break;
		    		}
			    }
			    
			    $info = $protoManiLib->oneStepSave( $sqlo, $new, $step_nr );
				if ( ($info!="") and ($this->_debug>1) ) echo 'INFO: Step-Nr:'.$step_nr.' '. $info ."<br>\n";
				$this->updateCnt++;
		    } while (0);
		}  
	
		$protoManiLib->close($sqlo);
		
		if ($this->errcnt) {
			$error->set( $FUNCNAME, 1, 'User errors occurred.', 1 );
		}
		
		echo "<br>\n";
		
	}
	
	/**
	 * upload files to mother SUC
	 * @param object $sqlo
	 * @param int $mo_suc_id - mother SUC_ID of protocol
	 * @param array $files_arr - the upload $_FILE structure
	 * @param array $fileinfo
	 */
	function uploadFiles($sqlo, $mo_suc_id, $files_arr, $fileinfo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		//$conc_proto_id = $this->protoid;
		//$a_proto_id    = $this->cp_feats['ABSTRACT_PROTO_ID'];
		
		$step_arr = $files_arr['name'];
		$tmperr = array();
		foreach($step_arr as $step_nr => $file_nice ) {
			
			$attkey  = $fileinfo[$step_nr]['attkey'];
			$tmpfile = $files_arr['tmp_name'][$step_nr];
			
			if ($attkey==NULL) {
				$tmperr[]='Step '.$step_nr.': no fileinfo[attkey] given.';
				continue;
			}
			
			if ($files_arr['tmp_name'][$step_nr]==NULL) {
				// no file uploaded
				continue;
			}
			
			// upload
			$one_file_info = array(
				'tmp_name' => $files_arr['tmp_name'][$step_nr],
				'name' => $files_arr['name'][$step_nr],
				'size'=> $files_arr['size'][$step_nr],
				'type'=> $files_arr['type'][$step_nr],
			);
			
			if (!$one_file_info['size']) {
				$tmperr[]='Step '.$step_nr.': attachment file is empty.';
				continue;
			}
			
			$rel_id = 0;
			$action = 'insert';
			$go = 1;
			$parx = array('KEY'=>$attkey, 'ARCHIVE'=>1); // save as PROTECTED attachment
			
			$modLib = new oSATTACH_modWork($sqlo, 'CONCRETE_SUBST', $mo_suc_id, $go);
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 1, 'severe attach-init error on SUC-ID:'.$mo_suc_id );
				return;
			}
			
			$answer = $modLib->manageActions($sqlo, $one_file_info, $parx, $rel_id, $action);
			if ($error->Got(READONLY))  {
				$error_txt = $error->getAllAsText();
				$error->reset();
				$tmperr[]='Step '.$step_nr.': error on file upload: '.$error_txt;
			}
		}
		
		if (sizeof($tmperr)) {
			$error->set( $FUNCNAME, 2, 'errors occurred: '. implode("; ",$tmperr) );
			return;
		}
		
	}

}
 		
// ---------------------------------------------------------------


$sqlo   = logon2();	
$sqlo2  = logon2();
$error = & ErrorHandler::get();

$conc_proto_id = $_REQUEST['conc_proto_id'];
$backurl  	= $_REQUEST['backurl'];
$xargu 		= $_REQUEST['xargu'];
$fileinfo   = $_REQUEST['fileinfo'];

$title = 'insert/update concrete protocol steps of a concrete protocol';
if ($conc_proto_id) {
	$title .= " [<a href=\"edit.tmpl.php?t=CONCRETE_PROTO&id=".$conc_proto_id."\">ID: ".
		$conc_proto_id."]</a>";
}  

$infoarr = NULL;
$infoarr['title'] = $title;
$pagelib = new gHtmlHead();
$pagelib->startPageLight( $sqlo, $infoarr);
echo '<ul>' ."\n";

if (!$conc_proto_id) {
	$pagelib->htmlFoot('ERROR', 'no concrete protocol given !');
}   

$MainLib = new oCONCRETE_PROTO_paras();

$MainLib->setProtocol($sqlo, $conc_proto_id);
if ($error->Got(READONLY))  {
	$error->printAllPrio();
	$pagelib->htmlFoot();
}



$stopForward = 0;
 
if ( !empty($xargu) ) {
	echo '... Update features ....<br>';
	$MainLib->updateFeatures( $sqlo, $xargu );
	if ($error->Got(READONLY))  {
		$error->printAll();
		$error->reset();
		$stopForward = 1; // error occured !
	}
}

$MainLib->updateSteps($sqlo, $sqlo2);
echo '... '.$MainLib->updateCnt. ' steps updated.<br>';
if ( $error->got() ) {
	$error->printAll();
	$stopForward = 1;
}


if ( !empty($_FILES['att']) and $_REQUEST['mo_suc_id'] ) {
	echo '... Upload files ....<br>';
	$MainLib->uploadFiles($sqlo, $_REQUEST['mo_suc_id'], $_FILES['att'], $fileinfo);
	if ( $error->got() ) {
		$error->printAll();
		$stopForward = 1;
	}
}


echo "<br />\n";
if ($backurl=="") $nexturl = "edit.tmpl.php?t=CONCRETE_PROTO&id=".$conc_proto_id."#goodies";
else $nexturl = urldecode($backurl);
if ( $stopForward ) $stopopt =  array($stopForward, 'error occured');
else $stopopt = NULL;

js__location_replace( $nexturl, "back to protocol $conc_proto_id", $stopopt);

echo '</ul>'."\n";
$pagelib->htmlFoot();

