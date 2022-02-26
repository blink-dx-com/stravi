<?php
/**
 * it contains code, which will be included in edit.tmpl.php, 
 *  - param "xmode"=qc
 * 
 * @package obj.concrete_proto.xmode.qc.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0001750: o.CONCRETE_PROTO > gui > show QC-Problems of protocol 
 */
require_once ("gui/o.PROTO.stepout1.inc");
require_once ('f.msgboxes.inc'); 
require_once ("o.ACCEPT_PROT.subs.inc");
require_once 'gui/o.CONCRETE_PROTO.edit.inc';
require_once 'o.CS_HAS_PR.subs.inc';
require_once ("f.objview.inc");	

class oCONCRETE_PROTO_accGui {
	
    function init(&$sqlo, $c_proto_id)  {
    	$this->c_proto_id = $c_proto_id;
    	
    	$sqlsel = '* from CONCRETE_PROTO where CONCRETE_PROTO_ID='.$c_proto_id;
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadArray();
    	$this->attribs = $sqlo->RowData;
    	
    	if (!$this->attribs['ACCEPT_PROT_ID']) {
    		echo 'No acceptance protocol set.';
    		return -1;
    	}
    	$sqlsel = 'ABSTRACT_PROTO_ID, VERSION_DATE from ACCEPT_PROT where ACCEPT_PROT_ID='.$this->attribs['ACCEPT_PROT_ID'];
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadRow();
    	$abs_accept_proto_id = $sqlo->RowData[0];
    	$accept_versDate     = $sqlo->RowData[1];
    	
    	$ABS_PROTO_ID = $this->attribs['ABSTRACT_PROTO_ID'];
    	
    	if ($ABS_PROTO_ID != $abs_accept_proto_id ) {
    		cMsgbox::showBox("error", "abstract protocol IDs of concrete protocol and acceptance protocol are different."); 
    		return -1;
    	}
    	
    	
    	// get edit-link of BO + NAME + icon (object)
    	$objLinkLib = new fObjViewC();
    	echo tablename_nice2('ACCEPT_PROT') . ': ';
    	if ($this->attribs['ACCEPT_PROT_ID']) {
    		$htmlTmp = $objLinkLib->bo_display( $sqlo, 'ACCEPT_PROT', $this->attribs['ACCEPT_PROT_ID'] );
    		echo $htmlTmp. ' (version-date: '.$accept_versDate.')';
    	}
    	echo "<br />\n";
    	
    	$protoOrgLib = new gProtoOrg();
    	$protoOrgLib->setProto($sqlo, $ABS_PROTO_ID, 0);
    		
    	$this->stepArr = $protoOrgLib->getStepArray();
    	
    	$step_count = sizeof( $this->stepArr ) ;
    
    	if (!$step_count) {
    		echo 'Abstract protocol has no steps.';
    		return -2;
    	}
    	
    	return 1;
    }
    
    function showSteps(&$sqlo, &$sqlo2) {
        
    	$etxraCols=array('Min_val', 'Max_val', 'Notes', 'QC-Problem');
    	
    	$a_proto_id      = $this->attribs['ABSTRACT_PROTO_ID'];
    	$accept_proto_id = $this->attribs['ACCEPT_PROT_ID'];
    	$c_proto_id      = $this->c_proto_id;
    	     
    	?>
        <style type="text/css">
        td.xAccProt  { background-color:#333399; font-size:80%; color: #FFFFFF; } 
        </style>
    	<?php
    	
    	$showOpts=NULL;
    	$proto_step_c = new  protostep( $sqlo, $sqlo2, $a_proto_id, 0, 0, $c_proto_id, $showOpts );
    	
    	$headOpt = array('xColsCssCls'=>'xAccProt');
    	$proto_step_c->table_init($etxraCols, $headOpt);
    	
    	$step_count = sizeof( $this->stepArr) ;	
    	$i=0; 
    	
    	$cp_accept_lib = new oACCEPT_PROT_prc($sqlo, $c_proto_id);
    	$accProtLib    = new oACCEPT_PROT_subs($sqlo);
    	if ($accept_proto_id) {
    		$accProtLib->setAccProtoID($accept_proto_id);
    	}
    	
    	while ( $i< $step_count ) {
    		
    		$step_nr   = $this->stepArr[$i];
    		$qcProblem = NULL;
    		
    		$ok = $proto_step_c->outStepStart( $step_nr, $sqlo);
    		
    		$answer=array();
    		//$used_str = '';
    		$used_flag = 0;
    		
    		if ($accept_proto_id) {
    			$sqlsel= " *  from ACCEPT_PROT_STEP where ACCEPT_PROT_ID=" .
    				$accept_proto_id." AND STEP_NR=".$step_nr ;
    		    $sqlo->Quesel($sqlsel);
    			$sqlo->ReadArray();
    			$acceptFeat = $sqlo->RowData;

    			$answer = $cp_accept_lib->getLimitInfoOfStep($sqlo, $step_nr);
    			
    			if ($answer['ok']<=0) {
    				$qcProblem = 'Limits: '.$answer['errtxt'];
    			}
    			
    			$used_flag = $answer['used'];
    			//$used_str  = $answer['used']>0 ? 'x' : '';
    		}
    		
    		
    		// show limits only, if $used_flag is set 
    		
    		echo '<td>';
    		if ($used_flag) echo  $acceptFeat['MIN_VAL'];
    		echo '</td>';
    		echo '<td>';
    		if ($used_flag) echo  $acceptFeat['MAX_VAL'];
    		echo '</td>';
    		echo '<td>';
    		if ($used_flag) echo  htmlspecialchars($acceptFeat['NOTES']);
    		echo '</td>';
    		
    		echo '<td>';
    		if ($qcProblem!=NULL) {
    			echo '<font color=red>Error:</font> '.$qcProblem;
    		}
    		echo '</td>';
    		
    		$proto_step_c->outStepEnd($ok);
    		$i++;
    	}
    	$proto_step_c->table_end();
    	    
    	
    }

}


//   which describes the possible MODEs for table TABLENAME 
class oCONCRETE_PROTO_xmode_qc {

    // PLEASE use this function; still optional 
    function xmode_start( &$sqlo, &$sqlo2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {
    	global $error;
    	
    	$protStepLib = new oCONCRETE_PROTO_accGui();
    	echo '<ul>'."\n";
    	echo '<b>Shows protocols steps and acceptance parameters.</b><br />'."\n";
    	$answer = $protStepLib->init($sqlo, $id);
    	echo '</ul>'."\n";
    

    	if ($answer<0) return;
    	$protStepLib->showSteps($sqlo, $sqlo2);
    	
    	
    	$suc_id = oCS_HAS_PR_subs::get_SUC_by_PRC($sqlo, $id);
    	$mo_CERT_FLAG = glob_elementDataGet( $sqlo, 'CONCRETE_SUBST', 'CONCRETE_SUBST_ID', $suc_id, 'CERT_FLAG'); 
    	
    	oConcProtoEdform::policyAnalysis($sqlo, $id, $mo_CERT_FLAG);
    	
    	$error->printAll();
    }

}
