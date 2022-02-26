<?php 
require_once ("o.AS_HAS_PR.subs.inc");
require_once ("o.CS_HAS_PR.subs.inc");
require_once 'o.H_ALOG_ACT.subs.inc';
require_once ('f.help.inc');
require_once ('func_form.inc');
require_once ('gui/f.box_modal.inc');

/**
 * class, supporting a code-bean to show protocol-log of CONCRETE_SUBSTANCE
 * @package gui\o.SUC_ProtoLogGui.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   UREQ:0001352: o.CONCRETE_SUBST > GUI: modify/Create QC-protocol 
 *
 */
class oSUC_ProtoLogGui {
	
	/**
	 * @var int $xtype type of protocol:
	 * 	0: all
	 *  1: Prep
	 *  2: QC
	 */
	var $xtype;
	var $mergearr; // ORDER => array('st', 'cp', 'ap', 'or', 'ty')
	
	var $numProtos; // number of expected+performed protocols
	
	const statusIconArr = array(
	    'empty'=>array('i13_gray.gif',  'not done'),
	    'prog' =>array('i13_inwork.gif','in progress'),
	    'tech' =>array('i13_blue.gif',  'tech ready'),
	    'ok'   =>array('i13_ok.gif'  ,  'o.k.'),
	    'err'  =>array('i13_err.gif' ,  'failed'),
	);
	
	private $_apid_cache;
	
	/**
	 * 
	 * @var array $c_protos
	 *   ORDER => array('st', 'cp', 'ap', 'or', 'ty')
	 */
	private $c_protos;
	
	/**
	 * init class
	 * @param $sqlo
	 * @param $id
	 * @param array $arguByKey
	 * @param int $qcEditAllow 0,1
	 * @param $xtype 
	 * 	0: all
	 *  1: Prep
	 *  2: QC
	 */
	function __construct(&$sqlo, $id, &$arguByKey, $qcEditAllow, $xtype) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
		//$tablename='CONCRETE_SUBST';
	
		// next possible substance audit status change
		$this->nextActionArr = array(
			1=> 'produced',
			2=> 'QC checked'
		);
		
		$this->_apid_cache = array();
		
		$aSubstId = $arguByKey['ABSTRACT_SUBST_ID'];
		$this->substid   = $id;
		$this->EditAllow = $qcEditAllow;
		$this->arguByKey = &$arguByKey;
		$this->aSubstID  = $aSubstId;
		$this->xtype 	 = $xtype;
		
		$this->cs_PR_lib = new oCS_HAS_PR_subs();
		$this->cs_PR_lib->setObj($this->substid);
		
		$this->nextAction = NULL;
		if ($this->xtype) $this->nextAction = $this->nextActionArr[$this->xtype];
		
		
		
		// $this->as_PR_lib  = new oAS_HAS_PR_subs();
		$this->TypeDefArr = $this->cs_PR_lib->getTypeDef();
		
		// $this->a_protos = $this->aSubstQcProtLib->getQcProtoId($sqlo, $aSubstId );
		$this->c_protos = $this->cs_PR_lib->getProtoLog($sqlo, $this->xtype); // QC!
		
		
		// OLD: $this->_OldCProtoShow($sqlo);
		$this->AS_HAS_PR_lib = new oAS_HAS_PR_subs($this->aSubstID);
		$this->a_proto_arr   = $this->AS_HAS_PR_lib->getProtoLog($sqlo, $this->xtype);
		// merge abstract with concrete
		$this->mergearr     = $this->cs_PR_lib->mergeProtoArr($this->c_protos, $this->a_proto_arr);
		$this->numProtos    = sizeof($this->mergearr);
		
		$this->substFeat = glob_elemDataGet3( $sqlo, 'CONCRETE_SUBST', array('CONCRETE_SUBST_ID'=>$id), 
			array('CERT_FLAG') );
		$this->subst_CERT_FLAG = $this->substFeat['CERT_FLAG'];
		if (!$this->subst_CERT_FLAG) $this->subst_CERT_FLAG = 1; // default
	}
	
	// @return array(icon, info text)
	static function status_name2icon_info($status_tmp) {
	    $index = 'prog';
	    
	    if ($status_tmp=='released' or $status_tmp=='R_released') $index = 'ok';
	    if ($status_tmp=='tech_ready')
	        $index = 'tech';
	        if ($status_tmp=='rejected' or $status_tmp=='quarantine' ) $index = 'err';
	        
	    return self::statusIconArr[$index];
	}
	
	static function status_name2icon($status_tmp) {
	    $icon_info = self::status_name2icon_info($status_tmp);    
	    $status_icon = '<img src="images/'.$icon_info[0].'">';
        return $status_icon;
	}
	
	// return number of protocols (merged planned+performed
	function getNumProts() {
		return $this->numProtos;
	}
	
	/**
	 * @return array cprotoLog_STRUCT 
	 */
	function getCprotoLog() {
		return $this->c_protos;
	}
	
	function _infoLine($key, $val) {
		echo '<font color=gray>'.$key.':</font> '.$val.'<br />'."\n";
			
	}
	
	
	
	/**
	 * manage edit-actions
	 * @param $status_tmp
	 * @param $c_proto_id
	 */
	function _mngProtoAct(&$sqlo, $status_tmp, $c_proto_id) {
		

		// only for CERT_FLAG=1: allow "release"
		if ($status_tmp!='released' and $status_tmp!='quarantine') { 
		
			//$addParam = '&parx[mo.CERT_FLAG]='.$this->subst_CERT_FLAG;
			//$nextAction = "edit.tmpl.php?t=CONCRETE_SUBST&id=".$this->substid;
			
			if ( $this->numProtos==1 ) {
				
				// check, if next planned status already reached ...
				$nextStatusID = $this->_SUC_auditInfoDef[$this->nextAction];
				$accLogLib= new oAccLogC();
				$accLogLib->setObject($sqlo, 'CONCRETE_SUBST', $this->substid);
				$hasStatusEntry = $accLogLib->getLastEntryOfAction($sqlo, $nextStatusID);
				
				if ( empty($hasStatusEntry) ) {
					// @swreq   UREQ:0001352:007: Freigabe "released": wenn nur ein Protokoll geplant ...
					// substance has only ONE protocol and next status is NOT already set
// 					$nextAction='glob.obj.acclock.php?t=CONCRETE_SUBST&id='.$this->substid.'&action=add'.
// 						'&parx[statName]='.$this->nextAction.'&go=1'; // TBD: test this !!!!
				}
			}
			
			// OLD:
			// $backurl  = urlencode($nextAction);
			// $base_url = 'glob.obj.acclock.php?t=CONCRETE_PROTO&id='.$c_proto_id. $addParam;
			if ($status_tmp!='tech_ready') {
// 			   $auditUrl_tech = $base_url.'&action=add'.
// 			  			'&parx[action]='.$this->_auditInfoDef['tech_ready']. '&backurl='.$backurl;
			    $actCell  .= '<a href="#" onClick="xSubmit('.$c_proto_id.',\'tech_ready\')">'."TechReady</a> | ";
			}
// 			$auditUrl_release = $base_url.'&action=lock'.
// 			 	'&parx[action]='.$this->_auditInfoDef['released']. '&backurl='.$backurl ;
			$actCell  .= '<a href="#" onClick="xSubmit('.$c_proto_id. ',\'released\')">Release</a>'; 
            
			
			// currently not needed:
			// $backUrlRej = urlencode("edit.tmpl.php?t=CONCRETE_SUBST&id=".$this->substid);
			// $auditUrlRej= $base_url.'&action=lock'.
			// 			'&parx[action]='.$this->_auditInfoDef['quarantine'].'&backurl='.$backUrlRej;
			// $actCell  .= '&nbsp;Quarantine <a href="' .$auditUrlRej. '" title="set quarantine"></a>';
		}
	   /*
	   } else {
			
			if ($status_tmp!='R_released' and $status_tmp!='rejected') { 
				$nextAction = "edit.tmpl.php?t=CONCRETE_SUBST&id=".$this->substid;
				
				$backurl  = urlencode($nextAction);
				$auditUrl = 'glob.obj.acclock.php?t=CONCRETE_PROTO&id='.$c_proto_id.'&action=lock'.
					'&parx[action]='.$this->_auditInfoDef['R_released'].'&backurl='.$backurl;
				
				$backUrlRej = urlencode("edit.tmpl.php?t=CONCRETE_SUBST&id=".$this->substid);
				$auditUrlRej= 'glob.obj.acclock.php?t=CONCRETE_PROTO&id='.$c_proto_id.'&action=lock'.
					'&parx[action]='.$this->_auditInfoDef['rejected'].'&backurl='.$backUrlRej;
					
				$actCell  .= '<a href="' .$auditUrl. '">'."RD_release</a> |";
				$actCell  .= '&nbsp;Reject <a href="' .$auditUrlRej. '"><img src="images/i13.fail.gif"></a>';
			}
		}
		*/
		
		return '<span class="yGgray">Create!</span> | '.$actCell;
	}
	
	function getQcStatusNice() {
		return $this->qcStatusNice;
	}
	
	/**
	 * build a HTML hidden form box for fats Audit-Status changes
	 * - the FORM calls script glob.obj.acclock.php for audit-actions ...
	 */
	private function _audit_box() {

	    $suc_id    = $this->substid;
        $title_base= 'Change Audit-Status of protocol to';
        $box_id    ='xAuditModal1';
        $hbox_lib  = new F_box_modal($box_id);
        $js_string = $hbox_lib->get_click_js();
	        
	        ?>
	    <script>
    	function xSubmit(prc_id, action) {

    		form_action='';
    		if (action=='tech_ready') {
        		form_action='add';
    		}
    		if (action=='released') {
        		form_action='lock';
    		}

    		title_compo = document.getElementById('<?php echo $box_id;?>_title');
    		title_compo.innerHTML = "<?php echo $title_base?>" + " " +action;

    		document.prc_audit.elements['action'].value=form_action;
			document.prc_audit.id.value=prc_id;
			document.prc_audit.elements['parx[statName]'].value=action;
			<?php echo $js_string;?>
    	}
		</script>
		<?php

	    $hbox_lib->draw_open($title_base);

	    $hiddenarr=array();
	    $hiddenarr['t']   = 'CONCRETE_PROTO'; 
	    $hiddenarr['id']= NULL;	
	    $hiddenarr['parx[statName]']= NULL;	
	    $hiddenarr['action']= 'add';
	    $hiddenarr['backurl'] = urlencode('edit.tmpl.php?t=CONCRETE_SUBST&id='.$suc_id);
	    $hiddenarr["parx[mo.CERT_FLAG]"] = $this->subst_CERT_FLAG;
	    
	    $initarr   = array();
	    $initarr["action"] = 'glob.obj.acclock.php';
	    $initarr["title"]  = "Change Audit-Status of protocol";
	    $initarr["submittitle"] = "";
	    $initarr["no_html"]  = 1;
	    $initarr["formname"] = 'prc_audit';

	    
	    $formobj = new formc( $initarr, $hiddenarr, 0 );
	    $formobj->start_form();
	    
	    echo '<table>'."\n";
	    
	    $fieldx = array ("title" => "Notes", "name"  => "notes",
	        "val"    =>'',
	        "object" => "textarea" 
	    );
	    $formobj->fieldOut( $fieldx );
	    
	    //TBD: add optional signature-password field

	    $fieldx = array ("title" => "Save", "object" => "submit" );
	    $formobj->fieldOut( $fieldx );

	    echo '</table>'."\n";
	    $formobj->close_form();

	    $hbox_lib->draw_close();

	}
	
	
	/**
	 * show one row
	 * @param $sqlo
	 * @param $tabobj
	 * @param $valarr
	 */
	function _oneShortRow(&$sqlo, &$tabobj, $valarr) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
		// glob_printr( $valarr, "DEBUG_SDF: _oneShortRow()" );
		$xtype = $this->xtype;
		
		$a_proto_id = $valarr['ap'];
		$c_proto_id = $valarr['cp'];
		$XORDER     = $valarr['or'];
		$type_loop  = $valarr['ty'];
		$a_stepid   = $valarr['st'];
		
	
		
		$status_tmp  = NULL;
		$a_name		 = "";
		$infoProtHtml= NULL;
		$actCell  	 = NULL;
		$status_icon = NULL;
		$userNice	 = NULL;
		
		$cpid_crea_url = 'obj.concrete_subst.proto.php?id='.$this->substid.'&action=new&parx[type]='.$xtype;
		
		if ( $c_proto_id ) {
		    $tablename='CONCRETE_PROTO';
			$sqls="ap.ABSTRACT_PROTO_ID, ap.NAME, cp.CCT_ACCESS_ID, cp.EXEC_DATE ".
				" from CONCRETE_PROTO cp join ABSTRACT_PROTO ap on cp.ABSTRACT_PROTO_ID=ap.ABSTRACT_PROTO_ID".
				" where cp.CONCRETE_PROTO_ID=".$c_proto_id;
			$sqlo->Quesel($sqls);
			$sqlo->ReadRow();
			// $a_tmpid = $sqlo->RowData[0];
			$a_name        = $sqlo->RowData[1];
			$CCT_ACCESS_ID = $sqlo->RowData[2];
			$EXEC_DATE     = $sqlo->RowData[3];
			
			$this->accLogLib->setObject( $sqlo, $tablename, $c_proto_id, $CCT_ACCESS_ID );
			
			$statusid   = $this->accLogLib->getLastLog($sqlo);
			if ($statusid) {
			    $status_tmp = oH_ALOG_ACT_subs::statusID2Name( $sqlo, $statusid );
				if ($this->shOptions['withUser']) {
					// only do on option "withUser" to save DB-exec-time
					$auditInfo  = $this->accLogLib->getLastEntryOfAction( $sqlo, $statusid );
					if ($auditInfo['DB_USER_ID']) {
						$userFeats  = glob_elemDataGet3( $sqlo, "DB_USER", array("DB_USER_ID"=>$auditInfo["DB_USER_ID"]), 
							array('FULL_NAME') );
						$userNice = $userFeats['FULL_NAME'];
					}
				}
			}
			$crea_userNice=NULL;
			if ($this->shOptions['withCreator']) {
			    // only do on option "withUser" to save DB-exec-time
			    $crea_user_id = glob_elementDataGet( $sqlo, 'CCT_ACCESS', 'CCT_ACCESS_ID', $CCT_ACCESS_ID, 'DB_USER_ID');
			    if ($crea_user_id) {
			        $userFeats  = glob_elemDataGet3( $sqlo, "DB_USER", array("DB_USER_ID"=>$crea_user_id),
			            array('FULL_NAME') );
			        $crea_userNice = $userFeats['FULL_NAME'];
			    }
			}
			
			if ($this->EditAllow) {
				$actCell = $this->_mngProtoAct($sqlo, $status_tmp, $c_proto_id);	
			}
			
			$status_icon = self::status_name2icon($status_tmp);

			// show only name of protocol, but not the link, because many users click by accident on the link (!)
			// OLD code: with link and ID
			//    "<a href=\"edit.tmpl.php?t=CONCRETE_PROTO&id=".$c_proto_id."\">".
			//    " [ID:".$c_proto_id."]</a>";
			$infoProtHtml = "<img src=\"images/icon.CONCRETE_PROTO.gif\" border=0> ".$a_name;
			
			if ($XORDER>10000) {
				$actCell='deactivated!';
			}
			
			$del_allow_status = oH_ALOG_ACT_subs::STATUS_reopen;
			if ($this->EditAllow and (!$status_tmp or $del_allow_status) ) {
			    $actCell  .= ' | <a href="obj.concrete_subst.proto.php?id='.$this->substid.
			      '&action=delete&parx[CONCRETE_PROTO_ID]='.$c_proto_id.'">'."Delete</a>";   
			}
			// FUTURE: allow multiple protocol creation of same type (ABSTRACT_PROTO_ID)
			//if ( $this->EditAllow and ($this->_apid_cache[$a_proto_id]==$XORDER)  ) {
			      // show this at the last SUB_STEP of the same ABSTRACT_PROTO
			//    $new_step  = $XORDER + 1;
			//    $actCell  .= ' | <a href="'.$cpid_crea_url.'&parx[ABSTRACT_PROTO_ID]='.$a_proto_id.'&parx[STEP_ID]='.$new_step.'">'."Create New!</a>";    
			// }
				
		} else {
		
			if ( $a_proto_id ) {
				$sqls="NAME from ABSTRACT_PROTO where ABSTRACT_PROTO_ID=".$a_proto_id;
				$sqlo->Quesel($sqls);
				$sqlo->ReadRow();
				$a_name = $sqlo->RowData[0];
				
				$infoProtHtml = "<img src=\"images/icon.ABSTRACT_PROTO.gif\" border=0> ".$a_name;
				$status_icon  = '<img src="images/'.self::statusIconArr['empty'][0].'">';
				
				if ($this->EditAllow) {
				    $actCell  = '<a href="'.$cpid_crea_url.'&parx[STEP_ID]='.$a_stepid.'">'."Create!</a>";
				}
			}
		}
		
		$rowopt = NULL;

		$loopData= array(
			$XORDER,
			$infoProtHtml,
			$status_icon,
			$status_tmp,
			$actCell
			);
		if ($this->shOptions['withType']) {
			$typeNice = $this->TypeDefArr[$type_loop];
			$loopData[] = $typeNice;
		}
		if ($this->shOptions['withCreator'])  {
		    $loopData[] = $crea_userNice;
		}
		if ($this->shOptions['withExecDate'])  {
		    $loopData[] = substr($EXEC_DATE,0,16);
		}
		
		if ($this->shOptions['withUser'])  {
			$loopData[] = $userNice;
		}
		if ($XORDER>10000) {
			$rowopt['bgcolor']='#FFC0C0'; // deactivated (!)
		}
		
		$tabobj->table_row( $loopData, $rowopt );
	}
	
	/**
	 * show OVERVIEW of planned/performed Protocols of one substance
	 * @param array $options
	 * 	'withType'=> 0,1 show column type
	 *  'withUser'=> 0,1 show user of audit-action
	 *  'withCreator' => 0,1 show user of creation-action
	 *  'withExecDate'=> 0,1 show EXEC date of protocol
	 */
	function showProtos(&$sqlo, $options=NULL) {
		global $error;
		$FUNCNAME= 'showProtos';

		$mergearr = $this->mergearr;
		$this->shOptions = $options;
		$this->accLogLib = new oAccLogC();
		
		// set some audit info
		$table= 'CONCRETE_SUBST';
		$akeys= array('released', 'rejected', 'R_released', 'produced', 'QC checked', 'quarantine','tech_ready');
		$this->_SUC_auditInfoDef=array();
		foreach($akeys as $key) {
		    $this->_SUC_auditInfoDef[$key]   = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $table, $key );
		}
		
		// set some audit info
		$table= 'CONCRETE_PROTO';
		$akeys= array('released', 'rejected', 'R_released', 'quarantine','tech_ready');
		$this->_auditInfoDef=array();
		foreach($akeys as $key) {
		    $this->_auditInfoDef[$key]   = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $table, $key );
		}
		
		$this->_audit_box();
		

		if (sizeof($mergearr)) {
		    
		    // analyse STEP_IDs
		    // APID => last STEP_NR
		    $this->_apid_cache = array();
		    foreach($mergearr as $valarr) {
		        $apid    = $valarr['ap'];
		        $step_nr = $valarr['or'];
		        $this->_apid_cache[$apid] = $step_nr;  // last STEP_ID of same APID
		    } 
		    
		   
		    $he_option= array( "object"=> "icon" );
	        $help_url='o.CONCRETE_SUBST.tab_proto.html';
	        if ($this->xtype==2) $help_url='o.CONCRETE_SUBST.qcprot.html';
	        $tempHelpTxt = fHelpC::link_show($help_url, "help", $he_option);
	    
			
			$tabobj  = new visufuncs();
			$headOpt = array( "title" => "Protocol log ".$tempHelpTxt, 
			    'title_notes'=>' -- see also material preferences <img src="res/img/settings.svg" height=13>.');
			$headx   = array ("No", "Protocol", "#", "Status", "Action");
			if ($this->shOptions['withType']) 
				$headx[]='Type';
			if ($this->shOptions['withCreator'])
				$headx[]='Creator';
			if ($this->shOptions['withExecDate'])
			    $headx[]='ExecutionDate';
			if ($this->shOptions['withUser']) 
				$headx[]='User (Audit-Status)';
			
			$tabobj->table_head($headx,   $headOpt);
			
			
		
			foreach($mergearr as $key => $valarr) {
				$this->_oneShortRow($sqlo, $tabobj, $valarr);
				$cnt++;
			} 
			$tabobj->table_close();
		}
		

	}
}
