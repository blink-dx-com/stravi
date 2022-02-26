<?php
/**
 * - substance QC things, like QC protocol check
 * - it contains code, which will be included in edit.tmpl.php, when the rider is set to "MODE"
 * @package    edit.tmpl.php
 * @subpackage obj.concrete_subst.xmode.qc.inc
 * @swreq UREQ:0002165: o.CONCRETE_SUBST > overview of QC-workflow for one substance 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
require_once ("visufuncs.inc");
require_once ("f.objview.inc");
require_once ('o.CCT_ACCLOG.subs.inc');
require_once ("o.DB_USER.subs.inc");
require_once ("gui/o.SUC_ProtoLogGui.inc");
require_once ('o.S_VARIO.subs.inc');
require_once ('f.prefvals.inc');
require_once 'lev1/o.CONCRETE_SUBST.qm_ana.inc';

/**
 * special status class
 * @author steffen
 *
 */
class ocSUC_qcStatusGui {
	function __construct(&$sqlo, $id, &$arguByKey, &$x_rights) {
		
		$this->tablename='CONCRETE_SUBST';
		$tablename = $this->tablename;
		$aSubstId = $arguByKey['ABSTRACT_SUBST_ID'];
		$this->substid   = $id;
		$this->arguByKey = &$arguByKey;
		$this->aSubstID  = $aSubstId;
		
		$this->lfcImgArr = array (
			1 => 'i20.lcf.p.gif',
			2 => 'i20.lcf.e.gif',
			3 => 'i20.lcf.f.gif'	
		);
		
		
		// normal status path
		// array (ACTION => array() )
		// 'act' => parameters for glob.obj.acclock.php : "action"= 'lock' or 'add'
		$this->normalPath=array(
			'produced'  =>array('act'=>'add'),
			'QC checked'=>array('act'=>'add'),
			'reviewed'  =>array('act'=>'add'), // TBD: should be lock ???	
		);
		
		// CERT_FLAG => planned final status
		$lfc2status = array(
			1=>'released',
			2=>'D_released',
			3=>'R_released'
		);
		
		$def_last_status=$lfc2status[$arguByKey['CERT_FLAG']];
		if ($def_last_status==NULL) $def_last_status='released';
		$this->normalPath[$def_last_status] = array('act'=>'lock');
		
		
		// 'finished'  =>array('act'=>'lock')
		
		$this->auditLogLib = new oAccLogC();
		$this->auditLogLib->setObject( $sqlo, $tablename, $this->substid );
				
		//future: $defStatus = array('released', 'R_released', 'finished', 'rejected', 'quarantine');
	
		$this->isAdmin = role_admin_check ( $sqlo, $tablename );
		
		$this->canChangeStatus=0;
		if ( $x_rights['entail']>0 or $this->isAdmin ) {
			$this->canChangeStatus=1;
		}
		
		// get subst features
		$this->suc_features = glob_elemDataGet3( $sqlo, 'CONCRETE_SUBST', 
			array('CONCRETE_SUBST_ID'=>$id), array('CCT_ACCESS_ID', 'CERT_FLAG') );
		
		$this->infox=NULL;
		$cctAccFeat = glob_elemDataGet3( $sqlo, 'CCT_ACCESS', 
			array('CCT_ACCESS_ID'=>$this->suc_features['CCT_ACCESS_ID'] ), array('WFLOW_ID') );
		$this->infox['wfl_id'] = $cctAccFeat['WFLOW_ID'];
		
		$this->message=NULL;
	}
	
	
	
	function _checkPrepProto(&$sqlo) {
		if ($this->arguByKey['CONCRETE_PROTO_ID']) return 1;
		else return 0;
	}
	
	function initSubst( &$sqlo ) {
		
	}
	
	function storeInfoBox($i0, $i1, $i2) {
		$this->message=NULL;
		$this->message[0]=$i0;
		$this->message[1]=$i1;
		$this->message[2]=$i2;
		
	}
	function _InfoBoxOut() {
		htmlInfoBox( $this->message[0], $this->message[1], '', $this->message[2] );
	}
	
	function _infoOut($key, $val, $addinfo=NULL) {
		echo '<tr><td><font color=gray>'.$key.'</font></td><td>'.$val."</td>";
		if ($addinfo!=NULL) {
			echo '<td>'.$addinfo.'</td>';
		}
		echo "</tr>\n";
	}
	
	function _showActionPath(&$sqlo, $curr_statusName) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $table='CONCRETE_SUBST';
		$substid = $this->substid;
		$substr	 = NULL;
		$komma	 = NULL;
		
		//$lastStatusFound=0;
		//if ($curr_statusName==NULL) $lastStatusFound=1; // it is the first status
		
		$userHelpLib = new DB_userC();

		$substr = '<table><tr valign=top>';
		
		$defaultLinkShow=1;
		if ($curr_statusName=='released' or $curr_statusName=='D_released') {
			$defaultLinkShow=0;
		}
		if (!$this->canChangeStatus) {
			$defaultLinkShow=0;
		}
		
		
		// check, if status action links are active
		// DEACTIVATE all actions before this current status
		// works also after REOPEN of a material
		$status_action_arr=array();
		$is_active=0;
		foreach ($this->normalPath as $statusLoop=>$subarray) {
		    $status_action_arr[$statusLoop]= $is_active;
		    if ($curr_statusName==$statusLoop) {
		        $is_active=1;
		    }
		}
		
		if (!$is_active) {
		    // reset all to 1
		    foreach ($status_action_arr as $statusLoop=>$dummy) {
		        $status_action_arr[$statusLoop] = 1;
		    }
		}
		
		foreach ($this->normalPath as $statusLoop=>$subarray) {
			
		    $statusLoopOut = '<span style="color:#808080">'.$statusLoop.'</span>'; // default
			$showLink   = $defaultLinkShow; // always on !
			$userName   = NULL;
			$showLink   = $status_action_arr[$statusLoop];
			
			$stateLoopId = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID($sqlo, $table, $statusLoop);
			$accLogEntry = $this->auditLogLib->getLastEntryOfAction($sqlo, $stateLoopId);
			
			
			if ($accLogEntry['DB_USER_ID']) {
				// get user name
				$DB_USER_ID = $accLogEntry['DB_USER_ID'];
				$userName   = $userHelpLib->getFullname($sqlo, $DB_USER_ID);
			} 
			
			if ($showLink) {
				$lockAction = $subarray['act'];
				$statusLoopOut = '<a href="glob.obj.acclock.php?t='.$this->tablename.'&id='.$substid.
						'&parx[statName]='.$statusLoop.'&action='.$lockAction.'">set '.$statusLoop.'</a>';
			}
			
			if ($curr_statusName==$statusLoop) {
				$statusLoopOut = '<b>'.$curr_statusName.'</b>';
				$lastStatusFound=2;
			} else {
				$lastStatusFound=0;
			}
			
			
			$substr  .= $komma . '<td>'.
				'<table class="xstPath"><tr><td>'.$statusLoopOut.'<br /><span style="color:#5050A0">'.
				$userName.'</span></td></tr></table></td>'."\n";
			$komma    ='<td><img src="res/img/chevron-right.svg"></td>';
			$substr  .= '</td>';
		}
		$substr  .= '</tr></table>';
		$this->_infoOut('Statuspath:', $substr);
	}
	
	function _possibleActions($curr_statusName) {
		$tmpStr = NULL;
		$tablename = 'CONCRETE_SUBST';
		$substid=$this->substid;
		
		if ($curr_statusName!='rejected' and $curr_statusName!='quarantine') { 
			// do NOT lock the substance !
			$tmpStr .= '[<a href="glob.obj.acclock.php?t='.$tablename.'&id='.$substid.
			'&parx[statName]=quarantine&action=add">Quarantine!</a>] ';
		}
		if ($curr_statusName!='rejected' and $curr_statusName!='finished') { 
		    $tmpStr .= '[<a href="glob.obj.acclock.php?t='.$tablename.'&id='.$substid.
			'&parx[statName]=finished&action=lock">Finish!</a>] ';
		}
		if ($curr_statusName!='rejected' and $curr_statusName!='finished') {	
			$tmpStr .= '[<a href="glob.obj.acclock.php?t='.$tablename.'&id='.$substid.
			'&parx[statName]=rejected&action=lock">Rejected!</a>]';
		}
		if ($curr_statusName!='rejected' and $curr_statusName!='R_released') {	
		    $tmpStr .= '[<a href="glob.obj.acclock.php?t='.$tablename.'&id='.$substid.
			'&parx[statName]=R_released&action=lock">R_released!</a>]';
		}
		$this->_infoOut('Special actions:', $tmpStr);
		
	}
	
	function _lfc_out(&$sqlo) {
		$lfc        = intval($this->suc_features['CERT_FLAG']);
		$tmpimg     = NULL;
		$statusname = '???';
		if ($lfc) {
			$statusname = fPrefValsGet::getNotesOfVal( $sqlo, 'CONCRETE_SUBST', 'CERT_FLAG', $lfc );
			$tmpimg = $this->lfcImgArr[$lfc];	
			if ($tmpimg!=NULL) $tmpimg = '<img src="images/'.$tmpimg.'"> ';
		} 
		$lfc_out = $tmpimg . $statusname;
		$this->_infoOut('Life cycle status:', $lfc_out);
	}
	
	function tableStart() {
		
		echo '<style type="text/css">'."\n";
	    echo '.xstPath  { *border-collapse: collapse; /* IE7 and lower */
    		border-spacing: 0; border-radius: 5px; background-color:#E0E0EF; padding:10px}'."\n";
	    echo '</style>'."\n";
		
		echo '<table border=0>'."\n";
	}
	
	function status_Info(&$sqlo, $x_rights) {
		$tablename = 'CONCRETE_SUBST';
		$substid = $this->substid;
		
		$curr_statusName = NULL;
		$CCT_ACCLOG_stat = $this->auditLogLib->getLastLog($sqlo);
		if ($CCT_ACCLOG_stat)
		    $curr_statusName = oH_ALOG_ACT_subs::statusID2Name( $sqlo, $CCT_ACCLOG_stat );
		
		$qm_ana_lib = new o_CONCRETE_SUBST_qm_ana_lev1();
		$qm_ana_lib->set_suc($sqlo, $substid);
		$releaseInfo = $qm_ana_lib->getUpfrontInfo($sqlo);

		// $hasSpecialPolicy = $this->auditLogLib->specialPolicy($sqlo);
		// $manirights 	  = $this->auditLogLib->hasManiRights( $sqlo );
		// $prepProtExists   = $this->_checkPrepProto($sqlo);
		
		$iconx = 'i13_ok.gif';
		if ($releaseInfo['ok']<0) $iconx = 'i13_err.gif';
		$statusName = $curr_statusName;
		if (!$statusName) $statusName = '---';
		
		/*
		$progStatus=0;
		if ($prepProtExists>0) $progStatus=2;
		if ($qcprotos!=NULL)   $progStatus=3;
		if ($curr_statusName=='released') {
			$progStatus=4;
		}
		if ($curr_statusName=='finished') {
			$progStatus=5;
		}
		*/
		
		// $this->_infoOut('Progress:', '<img src="f.imgbarState.php?type=SUC&id='.$progStatus.'">'.$tmptxt);
		
		
						
		$this->_showActionPath($sqlo, $curr_statusName);	
		$this->_infoOut('DHR-Subrecord:', '<a href="p.php?mod=LAB/o.CONCRETE_SUBST.qc2rep&id='.$this->substid.'&action=draft">'.
			'<img src="images/i20.pdf.gif" border=0> Preview</a>');
		// $this->_infoOut('&nbsp;', '');	

		
		$mess_arr = $qm_ana_lib->get_messages_nice($sqlo, $releaseInfo);
		if (!empty($mess_arr)) {
		    $this->storeInfoBox( $mess_arr['head'], $mess_arr['text'], $mess_arr['key'] );
		}
		
		$this->_possibleActions($curr_statusName);
		
		// $this->_infoOut('Audit status:', $statusName);
		$this->_lfc_out($sqlo);
		
		
		if ($this->message!=NULL) $this->_InfoBoxOut();
		
		
		
	}
	
	// @DEPRECATED
	function _anaWorklist(&$sqlo) {
		$substid  = $this->substid;
		$aSubstID = $this->aSubstID;
		$objLinkLib = new fObjViewC();
		
		$wllist    = NULL;
		$htmlNotes = NULL;
		
		$workObjects = oS_OBJLINK_subs::getChiObjByChiTab($sqlo, 'CONCRETE_SUBST', $substid, 'WORKLIST');
		
		// get possible worklist from VARIO
		$varioLib   = new oS_VARIO_sub('ABSTRACT_SUBST');
		$absWorklistID = $varioLib->getValByKey($sqlo, $aSubstID, 'SUC.WORKLIST.ABS_ID');
		if ($absWorklistID) {
			$htmlNotes  .= '[<a href="p.php?mod=DEF/o.CONCRETE_SUBST.wlcrea&id='.$substid.'&awlid='.$absWorklistID.'">Worklist anlegen</a>]';
		}
		
		
		$cntWorkLists = sizeof($workObjects);
		if (is_array($workObjects))	{
			$cntloop=0;
			$tmpbreak=NULL;
			foreach( $workObjects as $dummy=>$workObjId) {
				
				$wllist  .= $tmpbreak . $objLinkLib->bo_display( $sqlo, 'WORKLIST', $workObjId );
				$tmpbreak ='<br />';
				$cntloop++;
			}
			reset ($workObjects); 
	
		} 
		
		return array($htmlNotes, $wllist);
	
	}
	
	// @DEPRECATED
	function infoRest(&$sqlo) {
		require_once ("f.objview.inc");	
		// get edit-link of BO + NAME + icon (object)
		$objLinkLib = new fObjViewC();
		
		$worklistInfo = $this->_anaWorklist($sqlo);
		 
	
		$wflInfo = '<span class="yGgray">none</span>';
		if ($this->infox['wfl_id']) {
			$wflInfo = $objLinkLib->bo_display( $sqlo, 'MODULE', $this->infox['wfl_id'] );
		}
		$this->_infoOut('Workflow:', $wflInfo);
		$this->_infoOut('Data analysis:', ' [<a href="p.php?mod=DEF/g.QC_analysis2&t=CONCRETE_SUBST&id='.$this->substid.'">Start QC-data-analysis</a>]');
		$this->_infoOut('Worklists:', $worklistInfo[0], $worklistInfo[1]);
	}

	function tableClose() {
		echo '</table>'."\n";
		
		
	}
	
	
}


		
class oCONCRETE_SUBST_xmode_qc {
	
	function __construct( &$editLib ) {
	}		
	
	function _headLine($text,$addtext=NULL) {
		echo '<b><font color=gray>'.$text.'</font></b> '.$addtext.'<br><br>'."\n";
	}
	
	function xmode_start( &$sqlo, &$sqlo2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {
		echo '<ul>';
		require_once ('f.help.inc');
		$helpLib = new fHelpC();
		$helpopt = array('location'=>'pionir');
		$helpText = $helpLib->link_show("o.CONCRETE_SUBST.tab.qc.html", "help", $helpopt);
		$this->_headLine('QM: Audit-status management','['.$helpText.']');
		
		$xtype_ALL  = 0;
		$QcProtlib  = new oSUC_ProtoLogGui($sqlo, $id, $arguByKey, 0, $xtype_ALL);
		$protLibOpt = array('withType'=>1, 'withUser'=>1);
		$QcProtlib->showProtos($sqlo, $protLibOpt);
		
		

		$Qclib = new ocSUC_qcStatusGui($sqlo, $id, $arguByKey, $x_rights);
		
		$Qclib->tableStart();
		$Qclib->status_Info($sqlo, $x_rights);
		
		$Qclib->tableClose();
		
		/* analyse $_SESSION['s_formState']['edit.o.SUC'] = array(
		 *    'qc.shMore' : 0,1 : show more info on QC-reiter
		 * )
		 * 
		 */
		echo "<br />\n";
		$formInfo = $_SESSION['s_formState']['edit.o.SUC'];
		if (!$formInfo['qc.shMore']) {
			echo '<a class="yGgray" href="f.s_formState.set.php?key=edit.o.SUC&subkey=qc.shMore&val=1">[show more...]</a><br />';
		}
		if ($formInfo['qc.shMore']) {
			
			$Qclib->tableStart();
			$Qclib->_infoOut('', '<a class="yGgray" href="f.s_formState.set.php?key=edit.o.SUC&subkey=qc.shMore&val=0">[show less...]</a>');
			// $Qclib->infoRest($sqlo);
			$Qclib->tableClose();
		} 
		
		echo '</ul>';
		
	}
}
