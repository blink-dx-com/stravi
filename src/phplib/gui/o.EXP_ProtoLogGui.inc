<?php 
require_once 'o.EXP.proto.inc';
require_once 'o.EXP_TMPL.proto.inc';

/**
 * class, supporting a code-bean to show protocol-log of EXP
 * @package gui/o.EXP_ProtoLogGui.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 *
 */
class oEXP_ProtoLogGui {
	
    var $mergearr; // exp_protoLog_STRUCT
	
	var $numProtos; // number of expected+performed protocols
	
	private $_apid_cache;
	
	/**
	 * 
	 * @var array $c_protos exp_protoLog_STRUCT
	 */
	private $c_protos;
	
	/**
	 * init class
	 * @param $sqlo
	 * @param $id
	 * @param $arguByKey
	 * @param int $qcEditAllow 0,1
	 */
	function __construct(&$sqlo, $id, &$arguByKey, $qcEditAllow) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
	    $this->tablename='EXP';
		
		$this->statusIconArr = array(
			'empty'=>array('i13_gray.gif','not done'),
			'prog'=>array('i13_inwork.gif','in work'),
			'ok'  =>array('i13_ok.gif','o.k.'),
			'err' =>array('i13_err.gif','failed'),
			);
			
		// next possible substance audit status change
		$this->nextActionArr = array(
			1=> 'produced',
			2=> 'QC checked'
		);
		
		$this->_apid_cache = array();
		
		
		$this->exp_id      = $id;
		$this->EditAllow   = $qcEditAllow;
		$this->arguByKey   = &$arguByKey;
		$this->TMPL_OBJ_id = $arguByKey['EXP_TMPL_ID']; // OLD: $aSubstId 
	
		
		$this->exp_PR_lib = new oEXPprotoC();
		
		
		$this->nextAction = NULL;

		$this->c_protos = $this->exp_PR_lib->get_c_protos_details ($sqlo, $this->exp_id);
		
		$a_proto_arr_tmp=array();
		if ($this->TMPL_OBJ_id) {
    		$exp_tmpl_proto_lib = new oEXP_TMPL_protoC($this->TMPL_OBJ_id);
    		$a_proto_arr_tmp  = $exp_tmpl_proto_lib->abs_protos_details($sqlo);
		}
		
		// merge abstract with concrete
		$this->a_proto_arr = array();
		if (sizeof($a_proto_arr_tmp)) {
		    foreach($a_proto_arr_tmp as $row) {
		        $this->a_proto_arr[] = array('st'=>$row['step'], 'ap'=>$row['pra']);
		    }
		}
		
		$this->mergearr     = $this->exp_PR_lib->mergeProtoArr($this->c_protos, $this->a_proto_arr);
		$this->numProtos    = sizeof($this->mergearr);
		
	
	}
	
	// return number of protocols (merged planned+performed
	function getNumProts() {
		return $this->numProtos;
	}
	
	/**
	 * @return array $c_protos : exp_protoLog_STRUCT 
	 */
	function getCprotoLog() {
		return $this->c_protos;
	}
	
	function _infoLine($key, $val) {
		echo '<font color=gray>'.$key.':</font> '.$val.'<br />'."\n";
			
	}
	
	
	
	
	function getQcStatusNice() {
		return $this->qcStatusNice;
	}
	
	
	/**
	 * show one row
	 * TBD: show protocol kind !
	 * @param $sqlo
	 * @param $tabobj
	 * @param $valarr
	 * @return array
	 */
	function _oneShortRow(&$sqlo, &$tabobj, $valarr) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;

		$a_proto_id = $valarr['ap'];
		$c_proto_id = $valarr['cp'];
		$kind_name  = $valarr['ki']; 
		$a_stepid   = $valarr['st'];
		$XORDER     = $valarr['st'];
		$tablename='CONCRETE_PROTO';
		$a_name		 = "";
		$infoProtHtml= NULL;
		$actCell  	 = NULL;
	
		$userNice	 = NULL;
		
		// OLD: 'obj.concrete_subst.qcProt.php?id='.$this->exp_id; 
		$cpid_crea_url = 'obj.exp.proto_act.php?id='.$this->exp_id.'&xaction=new'; 
		
		if ( $c_proto_id ) {
			$sqls="ap.ABSTRACT_PROTO_ID, ap.NAME, cp.CCT_ACCESS_ID ".
				" from CONCRETE_PROTO cp join ABSTRACT_PROTO ap on cp.ABSTRACT_PROTO_ID=ap.ABSTRACT_PROTO_ID".
				" where cp.CONCRETE_PROTO_ID=".$c_proto_id;
			$sqlo->Quesel($sqls);
			$sqlo->ReadRow();
			$a_tmpid = $sqlo->RowData[0];
			$a_name  = $sqlo->RowData[1];
			$CCT_ACCESS_ID  = $sqlo->RowData[2];
			
			$this->accLogLib->setObject( $sqlo, $tablename, $c_proto_id, $CCT_ACCESS_ID );
			$statusid   = $this->accLogLib->getLastLog($sqlo);
			if ($statusid) {
			    //$status_tmp = oH_ALOG_ACT_subs::statusID2Name( $sqlo, $statusid );
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
			

			// show only name of protocol, but not the link, because many users click by accident on the link (!)
			// OLD code: with link and ID
			//    "<a href=\"edit.tmpl.php?t=CONCRETE_PROTO&id=".$c_proto_id."\">".
			//    " [ID:".$c_proto_id."]</a>";
			$infoProtHtml = "<img src=\"images/icon.CONCRETE_PROTO.gif\" border=0> ".$a_name;
			
			if ($XORDER>10000) {
				$actCell='deactivated!';
			}

				
		} else {
		
			if ( $a_proto_id ) {
				$sqls="NAME from ABSTRACT_PROTO where ABSTRACT_PROTO_ID=".$a_proto_id;
				$sqlo->Quesel($sqls);
				$sqlo->ReadRow();
				$a_name = $sqlo->RowData[0];
				
				$infoProtHtml = "<img src=\"images/icon.ABSTRACT_PROTO.gif\" border=0> ".$a_name;
			
				
				if ($this->EditAllow) {
				    $actCell  = '<a href="'.$cpid_crea_url.'&step_no='.$a_stepid.'&abstract_proto_id='.$a_proto_id.'">'."Create!</a>";
				}
			}
		}
		
		$rowopt = NULL;

		$loopData= array(
			$XORDER,
		    $kind_name,
			$infoProtHtml,
			$actCell
			);
		
		if ($this->shOptions['withUser'])  {
			$loopData[] = $userNice;
		}
		if ($XORDER>10000) {
			$rowopt['bgcolor']='#FFC0C0'; // deactivated (!)
		}
		
		$tabobj->table_row( $loopData, $rowopt );
	}
	
	/**
	 * show planned/performed Protocols of one substance
	 * @param array $options
	 *  'withUser'=> 0,1 show user of audit-action
	 */
	function showProtos(&$sqlo, $options=NULL) {
		global $error;
		$FUNCNAME= 'showProtos';

		$mergearr = $this->mergearr;
		$this->shOptions = $options;
		$this->accLogLib = new oAccLogC();
		
		// set some audit info
		$table='EXP';
		$this->_auditInfoDef=array();
		$this->_auditInfoDef['released']   = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $table, 'released' );
		$this->_auditInfoDef['rejected']   = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $table,'rejected' );
		$this->_auditInfoDef['R_released'] = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $table,'R_released' );
		$this->_auditInfoDef['quarantine'] = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $table,'quarantine' );
		

		if (sizeof($mergearr)) {
		    
		    // analyse STEP_IDs
		    // APID => last STEP_NR
		    $this->_apid_cache = array();
		    foreach($mergearr as $key => $valarr) {
		        $apid    = $valarr['ap'];
		        $step_nr = $valarr['st'];
		        $this->_apid_cache[$apid] = $step_nr;  // last STEP_ID of same APID
		    } 
	    
			
			$tabobj  = new visufuncs();
			$headOpt = array( "title" => "Protocol log");
			$headx   = array ("No", "Kind", "Protocol", "Action");
			
			if ($this->shOptions['withUser']) 
				$headx[]='User (Status)';
			$tabobj->table_head($headx,   $headOpt);
			
			
		
			foreach($mergearr as $key => $valarr) {
				$this->_oneShortRow($sqlo, $tabobj, $valarr);
				$cnt++;
			} 
			$tabobj->table_close();
		}
		

	}
}
