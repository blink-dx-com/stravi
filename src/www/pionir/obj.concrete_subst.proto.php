<?php
/**
 * [suc.creaProto] : 
 *  A) create  a Protocol for CONCRETE_SUBST
 *  B) delete  a protocol
 *  
 * - check if Protocol already exists
 * - create a Protocol
 * - automatic forward to: DEFAULT: edit.tmpl.php or $forward_url
 * - check function-role-right: o.CONCRETE_SUBST.acclog
 * - changes for DB-version: 1.0.4.9 
 * @swreq UREQ:0001351: o.CONCRETE_SUBST > GUI: create QC-protocol 
 * @package obj.concrete_subst.proto.php OLD:obj.concrete_subst.qcProt.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param int $go  
 * 		0 : normal
 *      1 : the form showAcceptanceForm() was sent; expect $parx['ACCEPT_PROT_ID']
 * @param int $id    ID of CONCRETE_SUBST
 * @param $action
 *   new    - new protocol
 *   delete - delete protocol: need $parx['CONCRETE_PROTO_ID'] 
 *   
 * @param $parx['ACCEPT_PROT_ID'] selected id of ACCEPT_PROT
 * @param $parx['STEP_ID'] 'final' or Step-ID of ABSTRACT_SUBST: AS_HAS_PR
 * @param $parx['ABSTRACT_PROTO_ID'] [OPTIONAL]: if given: in combination with  STEP_ID: a protocol can be created multiple times ...
 * @param $parx['type'] : 1:Prep, 2:QC 
 * @param $forward_url : [OPTIONAL] urlencoded URL to forward to
 * 
 */

session_start(); 

require_once ('func_form.inc');
require_once ('reqnormal.inc');
require_once ("insertx.inc");
require_once ("date_funcs.inc");
require_once ('o.PROTO.subs.inc');
require_once ("o.ABSTRACT_SUBST.qcprot.inc");
require_once ('o.ABSTRACT_SUBST.defs.inc');
require_once ("o.ACCEPT_PROT.subs.inc");
require_once ("javascript.inc" );
require_once ("glob.obj.update.inc");
require_once ("glob.obj.proto_act.inc");
require_once ("o.AS_HAS_PR.subs.inc");
require_once ("o.CS_HAS_PR.subs.inc");
require_once 'g_delete.inc';


/**
 * helper class to create a new CONCRETE protocol and attach it to SUBSTANCE (concrete)
 * @author steffen
 *
 */
class ocSUC_qcProtoCreaG {
	var $CLASSNAME='ocSUC_qcProtoCreaG';
	var $infox; /*
		'STEP_NR' sample : step-NR
		'accProtoArr'    : array of possible ACCEPT_PROT_ID (only [released])
		'acc_num'		 : number of ALL ACCEPT_PROT_ID on QC_ABS_PROTO_ID
		'ACCEPT_PROT_ID' : the used ACCEPT_PROT_ID
		'QC_ABS_PROTO_ID': QC protocol abstract
	*/
	
	
	function __construct(&$sqlo, $substid, $parx, $go) {
		global $error;
		$FUNCNAME = $this->CLASSNAME.':__init__';
		
		$this->MIN_FINAL_ORDER = 9998; // order limit, defines the XORDER of the final protocols
		
		$this->substid = $substid;
		$this->parx    = $parx;
		$this->go      = $go;
		$this->dummy_sample_ID=NULL;
		
		$this->dummy_sample='QC_SAMPLE';
		$sqlsel = 'CONCRETE_SUBST_ID from CONCRETE_SUBST where NAME='.$sqlo->addQuotes($this->dummy_sample);
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$dummy_sample_ID = $sqlo->RowData[0];
		if (!$dummy_sample_ID) {
			$error->set( $FUNCNAME, 1, 'Need a substance with name "'.$this->dummy_sample.'".' );
			return;
		} 
		$this->dummy_sample_ID=$dummy_sample_ID;
		
		$sqlsel = '* from CONCRETE_SUBST where CONCRETE_SUBST_ID='.$this->substid;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadArray();
		$this->feats = $sqlo->RowData;
		
		$this->_newtable_exists = glob_table_exists('CS_HAS_PR');
		
		// check databse version
		if ($parx['STEP_ID']<$this->MIN_FINAL_ORDER and !$this->_newtable_exists ) {
			$error->set( $FUNCNAME, 1, 'Need a new DB-version to support this STEP_ID.' );
			return;
		}
		
		$this->cs_PR_lib = new oCS_HAS_PR_subs();
		$this->cs_PR_lib->setObj($this->substid);
		$this->AS_HAS_PR_lib = new oAS_HAS_PR_subs($this->feats['ABSTRACT_SUBST_ID']);
		
	}
	
	// abstract classes
	function checkBeforeCreaSpecial(&$sqlo) {}
	function createSpecial(&$sqlo, $protoID) {}
	
	// -------------------
	
	/**
	 * check,  if exists
	 * @param  $sqlo
	 * @return array 
	 *   ('errflag'=>-1,1, 'info'=>string, 'cpid'=>, 'apid'=> id of abstract, 'ord'=>order id)
	 */
	protected function _getAbsProtocol(&$sqlo, $step_id_want) {
		
		$a_proto_arr  = $this->AS_HAS_PR_lib->getProtoLog($sqlo);
		
		// search, if exists
		$found=0;
		reset ($a_proto_arr);
		foreach( $a_proto_arr as $valarr) {
		    
			if ($valarr['st']==$step_id_want) {
				$found=1;
				break;
			}
			if ($step_id_want>=$this->MIN_FINAL_ORDER) {
				if ($valarr['or']==$step_id_want) {
					$found=1;
					break;
				}
			}
		}
		
		if (!$found) {
			return array('errflag'=>-1, 'info'=>'STEP_ID '.$step_id_want.' not found in substance (abstract) log');
		}
		
		$apid    = $valarr['ap'];
		$aporder = $valarr['or'];
		return array('errflag'=>1, 'info'=>'', 'apid'=>$apid, 'or'=>$aporder, 'ty'=>$valarr['ty']);
	}
	
	/**
	 * check abstract protocol steps by ($ABSTRACT_PROTO_ID, $STEP_ID)
	 * @param object $sqlo
	 * @param int $ABSTRACT_PROTO_ID
	 * @param int $STEP_ID
	 * @return array number[]|string[]|unknown[]
	 */
	protected function _check_step_apid($sqlo, $ABSTRACT_PROTO_ID, $STEP_ID) {
	    $qtype = 1;
	    return array('errflag'=>1, 'info'=>'', 'apid'=>$ABSTRACT_PROTO_ID, 'or'=>$STEP_ID, 'ty'=>$qtype);
	}
	
	/**
	 * check, if protocol already exists
	 * @param $sqlo
	 * @param $orderWant
	 * @return array('errflag'=>, 'info'=>string)
	 */
	function _checkConProtocol( &$sqlo, $orderWant ) {
		
		$c_protos = $this->cs_PR_lib->getProtoLog($sqlo);
		
		if (!sizeof($c_protos)) {
			return array('errflag'=>1, 'info'=>''); // no protocols exist
		}
		
		// search, if exists
		$found=0;
		reset ($c_protos);
		foreach( $c_protos as $valarr) {
			if ($valarr['or']==$orderWant) {
				$found=1;
				break;
			}
		}
		
		if (!$found) {
			return array('errflag'=>1, 'info'=>'');
		}
		
		return array('errflag'=>-1, 'info'=>'protocol (concrete) already exists on pos:'.$orderWant );
	}
	
	/**
	 * check parameters
	 * @param $sqlo
	 * @return string $warnString
	 *   = 'selectAccept' : select an acceptance protocol
	 * @throws
	 *   5: 'input-param STEP_ID missing.'
	 */
	function checkParams( &$sqlo ) {
		global $error;
		$FUNCNAME = $this->CLASSNAME.':checkParams';
		// $aProtoID = $this->parx['aprotoID'];
		
		$parx = $this->parx;
		
		if ($parx['STEP_ID']==NULL) {
			$error->set( $FUNCNAME, 5, 'input-param STEP_ID missing.' );
			return;
		}
		
		$feats = $this->feats;
		
		$this->infox['ABSTRACT_SUBST_ID'] = $feats['ABSTRACT_SUBST_ID'];
		$this->infox['CERT_FLAG']         = $feats['CERT_FLAG'];
		
		$absSubstID = $this->infox['ABSTRACT_SUBST_ID'];
		
		if ($parx['ABSTRACT_PROTO_ID']) {
		    $abs_answer = $this->_check_step_apid($sqlo, $parx['ABSTRACT_PROTO_ID'], $parx['STEP_ID']);
		} else {
		    $abs_answer = $this->_getAbsProtocol($sqlo, $this->parx['STEP_ID']);
		}
		if ($abs_answer['errflag']<=0) {
			$error->set( $FUNCNAME, 2, 'substance (abstract) ['.$absSubstID.']: '.$abs_answer['info'] );
			return;
		}
		
		$absQcProtoID = $abs_answer['apid'];
		$this->infox['QC_ORDER']        = $abs_answer['or'];
		$this->infox['QC_TYPE']			= $abs_answer['ty'];
		$this->infox['QC_ABS_PROTO_ID'] = $absQcProtoID;
		
		// concrete substance_check
		$con_answer = $this->_checkConProtocol($sqlo, $this->infox['QC_ORDER'] );
		if ($con_answer['errflag']<=0) {
			$error->set( $FUNCNAME, 3, $con_answer['info'] );
			return;
		}
		
		$this->checkBeforeCreaSpecial($sqlo);
		if ($error->Got(READONLY))  {
			return;
		}
		
		// get acceptance protocols from ABSTRACT_PROT (QC)
		$acceptLib = new oACCEPT_PROT_subs($sqlo);
		$accOpt    = array( 'onlyReleased'=>1 );
		$this->infox['accProtoArr'] = $acceptLib->getAcceptByAbstract($sqlo, $absQcProtoID, $accOpt);
		$this->infox['acc_num']     = $acceptLib->getNumAcceptByAbs($sqlo, $absQcProtoID);
		
		if (!$this->go) {
			if (is_array($this->infox['accProtoArr'])) {
				if (sizeof($this->infox['accProtoArr'])==1) {
					// take that one
					$currentAccID = current ($this->infox['accProtoArr']);
					$this->infox['ACCEPT_PROT_ID'] = $currentAccID;
				} else {
					echo "WARN: mehrere Acceptances: w&auml;hle aus!";
					return 'selectAccept';
				}
			} else {
				// @swreq UREQ:0001351:005b: wenn mehrere acceptance protocol existieren, aber keines ist [released] => zeige FEHLER
				if ($this->infox['acc_num']>0) {
					$error->set( $FUNCNAME, 6, 'Acceptance Protocols existieren fuer das abstrakte Protokoll, aber keines ist [released]! Bitte den Eigner des abstrakten Protokolls fragen!' );
					return;
				}
			}
		} else {
			// the form showAcceptanceForm() was performed ...
			// @swreq UREQ:0001724:03: "life cycle flag" => 1 or 0: acceptance protocol MUSS ausgewaehlt werden ???
			if ($this->parx['ACCEPT_PROT_ID']) {
				// o.k., one was selected
				$this->infox['ACCEPT_PROT_ID'] = $this->parx['ACCEPT_PROT_ID'];
			} else {
				// no acceptance selected
				if ( is_array($this->infox['accProtoArr']) and ($this->infox['CERT_FLAG']==1 or !$this->infox['CERT_FLAG']) ) {
					echo "WARN: du musst ein Acceptance-Protocol auswaehlen!";
					return 'selectAccept';
				}
				// else: o.k. allowed
			}
		}
		return;
	}
	
	/**
	 * show form
	 * 
	 * @param $sqlo
	 */
	function showAcceptanceForm(&$sqlo) {
		$accProtoArr = $this->infox['accProtoArr'];
		
		reset ($accProtoArr);
		$selarr=NULL;
		foreach( $accProtoArr as $dummy=>$acpid) {
			$tmpname = obj_nice_name ( $sqlo, 'ACCEPT_PROT', $acpid );
			$selarr[$acpid]=$tmpname.' [ID:'.$acpid.']';
		}
		
		require_once ('func_form.inc');
	
		$initarr   = NULL;
		$initarr["action"]      = $_SERVER['PHP_SELF'];
		$initarr["title"]       = "Select acceptance protocol and Create Protocol";
		$initarr["submittitle"] = "Select";
		$initarr["tabwidth"]    = "AUTO";
	
		$hiddenarr = NULL;
		$hiddenarr["id"]            = $this->substid;
		$hiddenarr["action"]        = 'new';
		$hiddenarr["parx[type]"]    = $this->parx['type'];
		$hiddenarr["parx[STEP_ID]"] = $this->parx['STEP_ID'];
	
		$formobj = new formc($initarr, $hiddenarr, 0);
		
		if ($this->infox['CERT_FLAG']==1 or !$this->infox['CERT_FLAG']) {
			$isRequired = 1;
		} else $isRequired = 0;
	
		$fieldx = array ( 
			"title" => "acceptance protocol", 
			"name"  => "ACCEPT_PROT_ID",
			"object"=> "select",
			"val"   => '', 
			"inits" => $selarr,
			"notes" => "select one acceptance protocol",
			"req"      => $isRequired
			 );
			 
			 
		$formobj->fieldOut( $fieldx );
	
		$formobj->close( TRUE );
		
	}
	
	/**
	 * add protocol to substance
	 * @param $sqlo
	 * @param $protoID
	 * @param $xorder
	 */
	function attachProto(&$sqlo, $protoID, $xorder, $typex) {
		$protoCreaLib = new obj_proto_act();
		$substid      = $this->substid;
		$protoCreaLib->attachProto($sqlo, $substid, $protoID, $xorder, $typex);
	}
	
	/**
	 * create protocol
	 * - put DUMMY_SAMPLE to the sample-step
	 * - add protocol to substance
	 * @param $sqlo
	 * @return -
	 */
	function _create1(&$sqlo) {
		global $error;
		$FUNCNAME = $this->CLASSNAME.':create';
		
		$aProtoID = $this->infox['QC_ABS_PROTO_ID'];
		$substid  = $this->substid;
		$error_SUM= NULL;
		
		$InsertLib = new insertC(); 
		$argu=NULL;
		$argu['ABSTRACT_PROTO_ID'] = $aProtoID;
		$argu['EXEC_DATE']         = date_unix2datestr(time(),1);
		
		if ($this->infox['ACCEPT_PROT_ID']>0) {
			$argu['ACCEPT_PROT_ID'] = $this->infox['ACCEPT_PROT_ID'];
		}
		$protoID = $InsertLib->new_meta($sqlo, 'CONCRETE_PROTO', array( "vals"=>$argu ) );
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'error on creation of protocol' );
			return;
		}
		echo '- protocol ID:'.$protoID.' created.<br />';
		
		return $protoID;
	}
	
	
	
	function _attach(&$sqlo, $protoID) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->attachProto($sqlo, $protoID, $this->infox['QC_ORDER'], $this->infox['QC_TYPE']);
		if ($error->Got(READONLY)) {
			$error->set( $FUNCNAME, 4, 'protcol attachment failed.' );
			return;
		}
	}
	
	/**
	 * create protocol
	 * - put DUMMY_SAMPLE to the sample-step
	 * - add protocol to substance
	 * @param $sqlo
	 * @return -
	 */
	function create(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->newProtoID = 0;
		
		$protoID = $this->_create1($sqlo);
		if ($error->Got(READONLY)) {
			return;
		}
		
		$this->newProtoID = $protoID;
		
		$this->createSpecial($sqlo, $protoID); // specials for QC or PREP
		if ($error->Got(READONLY)) {
			return;
		}
		$this->_attach($sqlo, $protoID);
	}
	
	function getNewProtoID() {
		return $this->newProtoID;
	}
}

// ---------------------------
/**
 * specials for QC-protocol
 * @author steffen
 *
 */
class oSUC_QC_Rea_help extends ocSUC_qcProtoCreaG {
	
	function checkBeforeCreaSpecial(&$sqlo) {
	    
	    // OLD: need no sample anymore
// 		global $error;
// 		$FUNCNAME= __CLASS__.':'.__FUNCTION__;	
// 		$absQcProtoID = $this->infox['QC_ABS_PROTO_ID'];
// 		$absSubstID = $this->infox['ABSTRACT_SUBST_ID'];	


//		$this->infox['STEP_NR'] = $stepnr;
	}
	
	
	// do NOT add the substance to protocol, because this causes a circle reference in the DB
	// SUBST X => QC_PROTO > SUBST X
	// add substance to protocol
	function createSpecial(&$sqlo, $protoID) {
	    
	    // OLD: sample insert ...
// 		global $error;
// 		$FUNCNAME= __CLASS__.':'.__FUNCTION__;

		
        
        // 		$aProtoID = $this->infox['QC_ABS_PROTO_ID'];
        // 		$ProtoManiLib = new oCprotoMani();
        // 		$ProtoManiLib->setCprotoID($sqlo, $protoID, $aProtoID );
        		
        // 		$new = array( 'subst'=>$this->dummy_sample_ID, 'notes'=>'DO NOT change '.$this->dummy_sample.'!' );
        // 		$ProtoManiLib->oneStepSave( $sqlo, $new, $this->infox['STEP_NR'] );
        // 		if ($error->Got(READONLY)) {
        // 			$error->set( $FUNCNAME, 3, 'error on oneStepSave' );
        // 			return;
        // 		}
	}
}

/**
 * helper class to create a new CONCRETE protocol and attach it to SUBSTANCE (concrete)
 * @author steffen
 *
 */
class ocSUC_ProtoModG {
    function __construct($suc_id, $parx) {
        $this->suc_id = $suc_id;
        $this->parx = $parx;
    }
    
    function ask_form($sqlo) {
        
        $cpid = $this->parx['CONCRETE_PROTO_ID'];
        
        $pra_id = glob_elementDataGet( $sqlo, 'CONCRETE_PROTO', 'CONCRETE_PROTO_ID',  $cpid, 'ABSTRACT_PROTO_ID') ;
        $nicename = obj_nice_name ( $sqlo, 'ABSTRACT_PROTO', $pra_id );
      
        
        $initarr   = NULL;
        $initarr["action"]      = $_SERVER['PHP_SELF'];
        $initarr["title"]       = "Do you want to delete this protocol?";
        $initarr["submittitle"] = "Delete!";
        $initarr["tabwidth"]    = "600";
        $initarr["tabBgColor"]  = '#FF9BA2';
        
        $hiddenarr = NULL;
        $hiddenarr["id"]            = $this->suc_id;
        $hiddenarr["action"]        = 'delete';
        $hiddenarr["parx[CONCRETE_PROTO_ID]"] = $cpid;
        
        $formobj = new formc($initarr, $hiddenarr, 0);
        
        $fieldx = array ( // form-field definition
            "title"   => "Protocol",
            "object"  => "info2",
            "colspan" => 2,
            "val"     => $nicename. ' [PRC-ID:'.$cpid.']',
           
        );
        $formobj->fieldOut( $fieldx ); // output the form-field

        $formobj->close( TRUE );
        
    }

    function delete($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $cp_tablename='CONCRETE_PROTO';
        
        echo '... start delete.'."<br>\n";
        $cpid = $this->parx['CONCRETE_PROTO_ID'];
        
        $this->cs_PR_lib = new oCS_HAS_PR_subs();
        $this->cs_PR_lib->setObj($this->suc_id);
        
        $log = $this->cs_PR_lib->getProtoLog($sqlo);
        if (empty($log)) {
            $error->set( $FUNCNAME, 1, 'CPID '.$cpid.' not in protocol log.' );
            return;
        }
        $found=0;
        foreach($log as $row) {
            if ($row['cp']==$cpid) $found=1;
        }
        if (!$found) {
            $error->set( $FUNCNAME, 2, 'CPID '.$cpid.' not in protocol log.' );
            return;
        }
        
        $this->cs_PR_lib->unlink_protocol($sqlo, $cpid);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 3, 'Error on protocol unlink' );
            return;
        }
        
        // check table-access-rights for user
        $t_rights = tableAccessCheck($sqlo, $cp_tablename);
        if ( $t_rights["delete"] != 1 ) {
            echo '... protocol unlinked, but not deleted due to your role rights.'."<br>\n";
            return;
        }
        
        // delete protocol
        $delete_lib = new fObjDelC();
        $delete_lib->obj_delete($sqlo, $cp_tablename, $cpid);
        
        echo '... ok, deleted.'."<br>";
    }
    
}


/**
 * specials for PREP-protocol
 * @author steffen
 *
 */
class oSUC_PREP_Rea_help extends ocSUC_qcProtoCreaG {
	
}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$id 		= $_REQUEST['id'];
$parx 		= $_REQUEST['parx'];
$go 		= $_REQUEST['go'];
$forward_url = $_REQUEST['forward_url'];

$tablename	= 'CONCRETE_SUBST';
$title		= '[suc.modProto] - Create/Delete a protocol for a '.tablename_nice2($tablename) ;

$infoarr			 = NULL;
$infoarr['scriptID'] = '';
$infoarr['title']    = $title;
$infoarr['title_sh']    = '[suc.modProto]';
$infoarr['form_type']= 'obj';
$infoarr['design']   = 'norm';
$infoarr['obj_name'] = $tablename;
$infoarr['obj_id']   = $_REQUEST['id'];
$infoarr['checkid']  = 1;

$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo, $infoarr);

// check object-rights !!!
$accCheckArr = array('tab'=>array('write'), 'obj'=>array('insert') );
$pagelib->do_objAccChk($sqlo, $accCheckArr);
$pagelib->chkErrStop();

$creatable = 'CONCRETE_PROTO';
$t_rights = tableAccessCheck( $sqlo, $creatable );
if ( $t_rights["insert"] != 1 ) {
	$answer = getTableAccessMsg( $creatable, 'insert' );
	htmlFoot('ERROR', $answer);
}

if ($_REQUEST['action']=='delete') {
    
    // delete one protocol
    
    $suc_help_lib = new ocSUC_ProtoModG($id, $parx);
    if (!$parx['CONCRETE_PROTO_ID']) {
        $pagelib->htmlFoot('Input-Error: protocol-id missing.');
    }
    if (!$go) {
        $suc_help_lib->ask_form($sqlo);
        $pagelib->chkErrStop();
       
    } else {
        $suc_help_lib->delete($sqlo);
        $pagelib->chkErrStop();
        $newurl='edit.tmpl.php?t=CONCRETE_SUBST&id='.$id;
        js__location_replace($newurl, '... object' ); 
        
    }
    $pagelib->htmlFoot();
}

$type=$parx['type'];

if ($type!=1 and $type!=2) {
	htmlFoot('ERROR', 'Input-param "type" missing.');
}

if ($type==oABSTRACT_SUBST_DEFS::PR_TYPE_PREP) {
	$MainLib = new oSUC_PREP_Rea_help($sqlo, $id, $parx, $go);
}
if ($type==oABSTRACT_SUBST_DEFS::PR_TYPE_QC) {
	$MainLib = new oSUC_QC_Rea_help($sqlo, $id, $parx, $go);
}
$pagelib->chkErrStop();

$infoString = $MainLib->checkParams( $sqlo ) ;
$pagelib->chkErrStop();

if ($infoString=='selectAccept') {
	$MainLib->showAcceptanceForm($sqlo);
	$pagelib->htmlFoot();
}

$MainLib->create($sqlo);
$pagelib->chkErrStop();

$newProtoID = $MainLib->getNewProtoID();

$newurl='edit.tmpl.php?t=CONCRETE_SUBST&id='.$id.'&tabArgu[PRC]='.$newProtoID;
if ($forward_url!=NULL) {
    $newurl= urldecode($forward_url);
}
js__location_replace($newurl, '... object' ); 

$pagelib->htmlFoot();
