<?php
/**
 * o.ABSTRACT_SUBST > QM > plugin [MAA_Spec_Release] old name:SUA_Spec
 * 
 * @package o.ABSTRACT_SUBST.spec.inc 
 * @swreq SREQ:0002557: o.ABSTRACT_SUBST > QM > plugin [MAA_Spec_Release] 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   long $id ID of ABSTRACT_SUBST
 * @param   string $act
 *   released,D_released,R_released
 **/
require_once ("o.AS_HAS_PR.subs.inc");
require_once ("o.ACCEPT_PROT.subs.inc");
require_once ("f.objview.inc");	
require_once ('o.CCT_ACCLOG.subs.inc');
require_once ('o.CCT_ACCLOG.gui.inc');
require_once ('access_lock.inc');
require_once ('f.signature.sub.inc');
require_once 'o.S_OBJLINK.subs.inc';

/**
 * worker class
 * @author steffen
 *
 */
class o_SUA_spec_data {
	
	private $children_objs; // array of (table,id)
	private $children_data; // array of objects
	
	function initSUA(&$sqlo, $sua_id) {
		$this->sua_id   = $sua_id;
		$this->accProto = new oACCEPT_PROT_subs ($sqlo);
	}
	
	/**
	 * get major data
	 * @param $sqlo
	 */
	function collectData(&$sqlo) {
		
		$sua_id = $this->sua_id;
		$this->children_data = NULL;
		
		$this->sua_data = NULL;
		$this->ap_array = NULL;
		$this->accProtos = NULL;
		
		$sqls = "select * from ABSTRACT_SUBST where ABSTRACT_SUBST_ID=".$sua_id;	
		$sqlo->query($sqls);
		$sqlo->ReadArray();
		$this->sua_data = $sqlo->RowData;
		
		// analyse ERP-Children
		$childrenLib = new oS_OBJLINK_subs();
		$this->children_objs = $childrenLib->getChiObjByKey($sqlo, 'ABSTRACT_SUBST', $sua_id, 'ERPchild'); 
		$this->collect_children_data($sqlo);
		
		$AS_HAS_PR_lib   = new oAS_HAS_PR_subs();
		$AS_HAS_PR_lib->setObj($sua_id);
		$this->ap_array = $AS_HAS_PR_lib->getProtoLog($sqlo, 0);
		
		if (is_array($this->ap_array)) {
			// for abstract protocols: get acceptance protos
			//  array('st'=>$step_id, 'ap'=>$a_proto_id, 'or'=>$XORDER, 'ty'=>$type)
			
		    foreach($this->ap_array as $key=> $valarr) {
			
				$apid = $valarr['ap'];
				$accProtosLoop = $this->accProto->getAcceptByAbstract($sqlo, $apid);
				
				if (is_array($accProtosLoop)) $this->accProtos[$apid] = $accProtosLoop;
			}
		}
	}
	
	/**
	 * collect data of children
	 * @param object $sqlo
	 */
	private function collect_children_data(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->children_data = NULL;
		if (!sizeof( $this->children_objs )) return;
		
		foreach($this->children_objs as $index => $objarr) {
			$chi_table=$objarr[0];
			
			if ($chi_table=='ABSTRACT_SUBST') {
				$error->set( $FUNCNAME, 1, 'object-type ABSTRACT_SUBST not allowed as "Object link child".' );
				return;
			}
		}
		
	}

	// get SUA-object ID
	function getData_OBJ() {
		return $this->sua_id;
	}
	
	function getData_SUA() {
		return $this->sua_data;
	}
	
	function getData_APROTO() {
		return $this->ap_array;
	}
	
	function getData_ACCEPT() {
		return $this->accProtos;
	}
	
	function getData_children() {
		return $this->children_objs;
	}
	
}

/**
 * modify status of objects
 * @author steffen
 *
 */
class o_SUA_spec_modStats {
	
	function o_SUA_spec_modStats(&$sqlo, $statusName) {
		$this->accLogLib  = new oAccLogC();
		$this->lockObj    = new gObjaccLock();
		$this->objLinkLib = new fObjViewC();
		
		$table='ABSTRACT_SUBST';
		
		$this->auditNames = array();
		$this->auditNames['released']    = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID($sqlo, $table, 'released');
		$this->auditNames['D_released']  = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID($sqlo, $table, 'D_released');
		$this->auditNames['R_released']  = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID($sqlo, $table, 'R_released');
		$this->auditNames['review_start']= oH_ALOG_ACT_subs::getH_ALOG_ACT_ID($sqlo, $table, 'review_start');
		
		$this->_setStatus($statusName);
		
		$this->answerArr = array(
			0=>'no action',
			1=>'change status',
			2=>'not possible'
		);
	}
	
	/**
	 * 
	 * Enter description here ...
	 * @param o_SUA_spec_data $dataLib
	 */
	function setObjects( &$dataLib ) {
		$this->dataLib = &$dataLib;
	}
	
	function _setStatus($statusName) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->statusName=$statusName;
		$this->statusID  =$this->auditNames[$statusName];
		
		if (!$this->statusID) {
			$error->set( $FUNCNAME, 1, 'internal error; missing status '.$this->statusName.' in the database!' );
			return;
		}
	}
	
	/**
	 * analyse status and set status
	 * @param $sqlo
	 * @param $t
	 * @param $objid
	 * @return array('act'=> 
	 * 	0 : no action needed
	 *  1 : change it
	 *  2 : not possible !
	 *  )
	 */
	function _setStatusOneObj(&$sqlo, $t, $objid) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		
		$newStatusName = $this->statusName;
		$this->accLogLib->setObject($sqlo, $t, $objid);
		$status_now = $this->accLogLib->getLastLogName($sqlo);
		
		//$doAction=0;
		
		if ($status_now==$newStatusName) {
			return array('act'=>0);
		}
		
		if ($status_now=='released') {
			// already released
			return array('act'=>0);
		}
		
		if ($status_now=='finished') {
			// status change impossible !
			return array('act'=>2, 'info'=>'is finished');
		}
		
		/*
		if ($this->statusName=='released' and $status_now!='review_start') {
			// not possible !
			return array('act'=>2, 'info'=>'old status must be "review_start"');
		}
		*/
		
		
		$this->lockObj->accessLockOne( $sqlo, $t, $objid);
		$auditParam = array("action"=>$this->statusID); // "action", "notes"
		$this->accLogLib->addLogObj( $sqlo, $t, $objid, $auditParam );
		
		return array('act'=>1);
	}
	
	function oneObj(&$sqlo, $t, $objid) {
		
		$answer = $this->_setStatusOneObj($sqlo, $t, $objid);
		$answer2 = array( 'info'=>$this->answerArr[$answer['act']] );
		if ($answer['info']!=NULL) $answer2['info'] .= ': '.$answer['info'];
		return $answer2;
	}
	
	
}

/**
 * visualization
 * @author steffen
 *
 */
class o_SUA_spec_visu {
	
	function __construct() {
		// get edit-link of BO + NAME + icon (object)
		$this->objLinkLib = new fObjViewC();
		$this->accLogLib  = new oAccLogC();
		
		$this->statusColors = array(
			'released'     =>'#D0FFD0',
			'review_start' =>'#FFFFD0'
		);
		$this->withAct=0;
	}
	
	function startTable($withAct) {
		$this->withAct = $withAct;
		echo "<table cellpadding=1 cellspacing=1 border=0>\n";
		echo '<tr backcolor="#EFEFFF"><th>Object</th><th>Status</th>';
		if ($withAct) echo '<th>Action</th>';
		echo '</tr>'."\n";
	}
	
	/**
	 * one object
	 * @param $sqlo
	 * @param $table
	 * @param $objid
	 * @param $level
	 * @param $action
	 */
	function visuObject(&$sqlo, $table, $objid, $level, $action=NULL) {
		
		$htmlTmp = $table.', ID:'.$objid.' ???';
		switch ($table) {
			case 'ABSTRACT_SUBST':
				$opts= array("showID"=>1);
				$htmlTmp = $this->objLinkLib->bo_display( $sqlo, $table, $objid, $opts ); 
				break;
			default:
				$opts= array("showID"=>1);
				$htmlTmp = $this->objLinkLib->bo_display( $sqlo, $table, $objid, $opts ); 
				break;
		}
		
		$this->accLogLib->setObject($sqlo, $table, $objid);
		$status_Name = $this->accLogLib->getLastLogName($sqlo);
		if ($status_Name==NULL) {
			$status_Name='---';
		}
		
		$color_s = $this->statusColors[$status_Name];
		if ($color_s!=NULL) {
			$status_Name = '<span style="background-color:'.$color_s.'">'.$status_Name.'</span>';
		}
		
		$level_html = '<img src="0.gif" width="'.(20*$level).'" height=1>'; // spacer
		echo "<tr><td>".$level_html . $htmlTmp. "</td><td>".$status_Name."</td>";
		if ($this->withAct) echo "<td>".$action."</td>";
		echo "</tr>\n";
	}
	
	function closeTable() {
		echo "</table>\n";
	}
}

/**
 * main class
 * @author steffen
 *
 */
class o_ABSTRACT_SUBST_spec_XPL extends gPlugin {
	
	
	function register() {
		$tablename	= 'ABSTRACT_SUBST';
		$title		  = 'MAA_Spec_Release';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = 'MAA Specification Release Tool';
		$this->infoarr['title_sh'] = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		$this->infoarr['inforow']  = 'For Review/Release: need role-rights: g.QM_expert oder g.QM_MAA_spec';
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array() )
				);
		
	}
	
	function initObject(&$sqlo) {
		// get data
		$this->dataLib = new o_SUA_spec_data();
		$this->dataLib->initSUA($sqlo, $this->objid);
		$this->dataLib->collectData($sqlo);
	}
	
	/**
	 * show the tree
	 * @param $sqlo
	 * @param int $withAct : 0,1
	 */
	function showTree(&$sqlo, $withAct=0) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		// show tree
		
		$visuLib = new o_SUA_spec_visu();
		$visuLib->startTable($withAct);
		
		$level=0;
		$info=array();
		
		if ($withAct) {
			$info = $this->changeStats->oneObj($sqlo, 'ABSTRACT_SUBST', $this->objid);
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 1, 'error on object' );
				return;
			}
			
			
		}
		$visuLib->visuObject($sqlo, 'ABSTRACT_SUBST', $this->objid, $level, $info['info']);
		
		$level = 1;
		
		$proto_arr  = $this->dataLib->getData_APROTO();
		$accept_arr = $this->dataLib->getData_ACCEPT();
		
		if ( sizeof($proto_arr) ) {
			
		    foreach($proto_arr as $valarr) {
			
				
				$apid = $valarr['ap'];
				$info = array();
				if ($withAct) {
					$info = $this->changeStats->oneObj($sqlo, 'ABSTRACT_PROTO', $apid);
					if ($error->Got(READONLY))  {
						$error->set( $FUNCNAME, 1, 'error on object' );
						return;
					}
				}
				$visuLib->visuObject($sqlo, 'ABSTRACT_PROTO', $apid, $level, $info['info']);
				
				if (is_array($accept_arr[$apid])) {
					
					$accept_tmp = $accept_arr[$apid];
				
					foreach($accept_tmp as $accept_id) {
					
						if ($withAct) {
							$info = array( 'info'=>'Acceptance Protocol muss per Hand "released" werden');
						}
						$visuLib->visuObject($sqlo, 'ACCEPT_PROT', $accept_id, $level+1, $info['info'] );
					
					}
				}
			}
		}
		
		
		$children_arr  = $this->dataLib->getData_children();
		$level = 1;
		if (sizeof($children_arr)) {
		    
			foreach($children_arr as $objarr) {
				
				$chi_table=$objarr[0];
				$chi_id   =$objarr[1];
				$info = array();
				if ($withAct) {
					$info = $this->changeStats->oneObj($sqlo, $chi_table, $chi_id);
				}
				$visuLib->visuObject($sqlo, $chi_table, $chi_id, $level, $info['info'] );
				
			}
		}
		
		
		$visuLib->closeTable();
	}
	
	function showActions(&$sqlo, $action) {
		
		$html_tmp='Actions: ';
		$actUrlBase='p.php?mod='.$this->_mod.'&id='.$this->objid.'&act=';
		
		$revStart  = '[<a href="'.$actUrlBase.'review_start">review_start</a>]';
		$release   = '[<a href="'.$actUrlBase.'released">released</a>]';
		$release  .=' [<a href="'.$actUrlBase.'D_released">D_released</a>]';
		$release  .=' [<a href="'.$actUrlBase.'R_released">R_released</a>]';
		
		if ($action=='special') {
			$html_tmp .= $release; // $revStart.' '. 
		} else {
			$html_tmp = '[<a href="'.$actUrlBase.'">view only</a>]';
		}
		
		$this->_html->line_out($html_tmp);
		$this->_html->line_out('');
	}
	
	/**
	 * ask for submit
	 * @param $sql
	 */
	function formAsk(&$sqlo, $act) {
	    $table='ABSTRACT_SUBST';
		
		
// 		$status_id = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID($sqlo, $table, $act);
// 		$params = array('action'=>$status_id);
		
		$accLogGuigLib = new oAccLogGuiC('ABSTRACT_SUBST');
		
		$initarr   = NULL;
		$initarr["action"]      = 'p.php?mod='.$this->_mod;
		$initarr["title"]       = "Prepare process";
		$initarr["submittitle"] = "Next &gt;&gt;";
		$initarr["tabwidth"]    = "AUTO";
	
		$hiddenarr = NULL;
		$hiddenarr["act"] = $act;
		$hiddenarr["id"]  = $this->objid;
		
		$formobj = new formc($initarr, $hiddenarr, 0);
		
		$actionReq = 1;
		$fields   = array();
		$fields[] = array(
			"title" => "Status", 
			"object"=> "info2",
			"val"   => $act, 
			"notes" => "the new status");
		$fields[] = $accLogGuigLib->form_getRowByType('signpw');
		foreach($fields as $fieldx) {
		
			$formobj->fieldOut( $fieldx );
		}
		reset ($fields); 
	
		$formobj->close( TRUE );
	}
	
	/**
	 * check initial parameters
	 * - throws errors
	 */
	function paramCheck(&$sqlo, &$parx) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		
		// @swreq UREQ:0000704:add audit status;SUBREQ: 11.200(a)(1)(i)
		$fSignLib = new fSignatureSub();
		
		if ($parx['signpw']==NULL) {
			$error->set( $FUNCNAME, 4, 'Need a password.' );
		 	return;
		}
		$pwOK = $fSignLib->checkPW($sqlo, $parx['signpw']); // password o.k. ?
		if ($pwOK<=0) {
			$error->set( $FUNCNAME, 5, 'Wrong password.' );
		 	return;
		}
		
	}
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$this->objid   = $_REQUEST['id'];
		$go            = $_REQUEST['go'];
		$sqlo = &$this->_sqlo;
		
		$withAct = 0;
		$act  = $_REQUEST['act'];
		$this->initObject($sqlo);
		
		if ($act!=NULL) {
			
			$this->showActions($sqlo, 'tree');
			// check role
			
			$role_right_name = "g.QM_MAA_spec";
			$role_right      = role_check_f($sqlo, $role_right_name);
			if ($role_right!="execute") {
				$this->htmlFoot('ERROR','Du darfst keinen Review/Release-Prozess starten. '.
					'Du brauchst role-right: '.$role_right_name);
			}
			
			
			
			$withAct = 1;
			$this->changeStats = new o_SUA_spec_modStats($sqlo, $act);
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 2, 'Error on init.' );
				return;
			}
			
			if ( !$go ) {
				$this->formAsk($sqlo, $act);
				$this->htmlFoot();
			}
			
			$parx  = $_REQUEST['parx'];
			$this->paramCheck($sqlo, $parx);
			
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 1, 'Param check failed.' );
				return;
			}
			
		} else {
			$this->showActions($sqlo, 'special');
		}
		
		$this->showTree($sqlo, $withAct);
		
		
	}
	
}
