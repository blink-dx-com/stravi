<?php
/**
 * - plugin for CONTAINER
 * - manage reservation of selected substances to container
 *  - input: Substances + Aliquot-number
 *    substarr[id]=aliqu_num
 *  - search next free container-place
 *  - show place
 *  - start to fill
 * 
 * @package o.CONTAINER.addSubst_li.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param long $_REQUEST['id'] container
 * @param array $_REQUEST['substarr'] of [id] = aliquot_num
 	needed on go= 0,1
 * @param array $_REQUEST['substmap'] of 
 *  [id][aliquot_id] = pos
 	- needed on go= 2
 * @param int $_REQUEST['go']
 *   0 : show subst list: show aliquots
 *   1 : show place
 *   2 : save place
 * @global 	$_SESSION['userGlob']["o.CONTAINER.fill"] options for aliquots 
 */
 
require_once ( "f.assocUpdate.inc");
require_once ("o.CONTAINER.mod.inc");
require_once ("o.CONTAINER.subs.inc");
require_once ('func_form.inc');

class o_CONTAINER_addSub {
	var $CLASSNAME='o_CONTAINER_addSub';
	
	function __construct(&$sqlo, $contid) {
		$this->contid=$contid;
		
		$parx=NULL;
		$this->contModiLib = new oContainerModC ($parx);
		$this->contModiLib->setContainer($sqlo, $contid);
		
		$this->contAnaLib = new oCONTAINER_SubsC();
		$this->contAnaLib->setContainer($sqlo, $contid) ;
		
		$this->storePointer = $this->contAnaLib->getCurrPointer();
	}
	
	/**
	 * get position list for each aliquot
	 * @return $posarr[$substid][$aliqCnt]=$pos;
	 * @param object $sqlo
	 */
	function getPosList(&$sqlo) {
		$posarr   = NULL;
		$startpos = $this->storePointer; // startpos to search
		
		reset ($this->substarr);
		foreach( $this->substarr as $substid=>$aliqoNum) {
			$aliqCnt=1;
			$oneSubstArr=NULL;
			while ($aliqCnt<=$aliqoNum) {
				$pos = $this->contModiLib->getFreePosAndAbs($sqlo,$startpos);
				$oneSubstArr[$aliqCnt]=$pos;
				$aliqCnt++;
				$startpos = $pos+1;
			}
			$posarr[$substid] = $oneSubstArr;
		}
		reset ($this->substarr); 
		return ($posarr);
	}
	
	function getAlias(&$sqlo, $pos) {
		return $this->contAnaLib->getPosAlias($sqlo, $pos);
	}
	
	function getPosFeats(&$sqlo, $pos) {
		return $this->contAnaLib->getPosFeats($sqlo, $pos);
	}
	
	
	/**
	 * add aliquots ($substmap) to container
	 * @param unknown_type $sqlo
	 * @param unknown_type $substmap
	 * @return unknown_type
	 */
	function addAliquots( $sqlo, &$substmap ) {
		global $error;
		$FUNCNAME= $this->CLASSNAME.':addAliquots';
		
		$this->currContPos = 0;
		$this->savecnt=0;
		$this->contModiLib->activateModi($sqlo);
	
		foreach( $substmap as $substid=>$aliqArr) {
			
			foreach( $aliqArr as $ali_id=>$contpos) {
				
				$alias      = $this->contAnaLib->getPosAlias($sqlo, $contpos) ;
				$subsAliTxt = 'substance SUC-ID:'.$substid.',aliquot:'.$ali_id;
				
				$pos = $this->contAnaLib->aliquotInCont($sqlo, $substid, $ali_id);
				if ($pos>0) {
					$error->set( $FUNCNAME, 1, $subsAliTxt.' is already in container.');
					return;
				}
				
				$this->currContPos = $contpos;
				$this->contModiLib->addAliquot( $sqlo, $contpos, $substid, $ali_id );
				if ($error->Got(READONLY))  {
					$error->set( $FUNCNAME, 1, 'error on '.$subsAliTxt );
					return;
				}
				$this->savecnt++;
			}
			reset ($aliqArr); 
		
		}
		reset ($substmap);
		
		return $this->savecnt;
	}
	
	/**
	 * check if substances are in container
	 * @return 
	 * @param object $sqlo
	 */
	function checkSubtInCont(&$sqlo, &$substarr) {
		global $error;
		$FUNCNAME= $this->CLASSNAME.':checkInContainer';
		
		$this->substarr = &$substarr;
		reset ($substarr);
		foreach( $substarr as $substid=>$dummy) {
			
			// $nice = obj_nice_name ( $sqlo, 'CONCRETE_SUBST', $substid ); 
			$pos = $this->contAnaLib->substInCont($sqlo, $substid);
			if ($pos>0) {
				$error->set( $FUNCNAME, 1, 'substance SUC-ID:'.$substid. ' is already in container.');
				return;
			}
		}
		reset ($substarr); 
	}
}


class o_CONTAINER_addSubst_li_XPL extends gPlugin {
	var $CLASSNAME='o_CONTAINER_addSubst_li_XPL';
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function info($text) {
		$this->_html->line_out('Info: '.$text);
	}
	
	function register() {
		$tablename	= 'CONTAINER';
		$title		= 'add selected substances to container';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('write'), 'obj'=>array('insert') )
				);
	}
	
	function mainInit() {
		$this->userOpts = NULL;
		$varx = $_SESSION['userGlob']["o.CONTAINER.fill"];
		if ($varx!=NULL) $this->userOpts = unserialize($varx);
	}
	
	function checkInContainer(&$sqlo, &$substarr) {
		$this->contSubLib->checkSubtInCont($sqlo, $substarr);
		$this->substarr = &$substarr;
	}
	
	function form0(&$sqlo) {
		
		$initarr   = NULL;
		$initarr["title"]       = "Define Aliquots per substance";
		$initarr["submittitle"] = "Next &gt;&gt;";
		$initarr["tabwidth"]    = "AUTO";
		
		$hiddenarr = NULL;
		$hiddenarr['id']=$this->objid;
		
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
		
		if (sizeof($this->userOpts['subsAli'])) {
			$absSubstAliquots = &$this->userOpts['subsAli'];
		} else $absSubstAliquots = array();
		
		reset ($this->substarr);
		foreach( $this->substarr as $substid=>$aliqunum) {
			
			$sqlsel = 'NAME, ABSTRACT_SUBST_ID from CONCRETE_SUBST where CONCRETE_SUBST_ID='.$substid;
			$sqlo->Quesel($sqlsel);
			$sqlo->ReadRow();
			$niceName   = $sqlo->RowData[0];
			$absSubstId = $sqlo->RowData[1];
			
			// predefined number of aliquots ?
			if ($absSubstAliquots[$absSubstId]>0) {
				$aliquotNum = $absSubstAliquots[$absSubstId];
			} else $aliquotNum = 1;
			
			$fieldx = array ( 
				"title" => $niceName.' ['.$substid.']', 
				'namex' => TRUE ,
				"name"  => 'substarr['.$substid.']',
				"object"=> "text",
				"val"   => $aliquotNum, 
				"notes" => "aliquots",
				"fsize" => 3    
				 );
			$formobj->fieldOut( $fieldx );
		}
		reset ($this->substarr); 
		$formobj->close( TRUE );
		
	}
	
	function form1(&$formarr) {
		$initarr   = NULL;
		$initarr["title"]       = "Send reservation";
		$initarr["submittitle"] = "Finish";
		$initarr["tabwidth"]    = "AUTO";
		
		$hiddenarr = NULL;
		$hiddenarr['id']=$this->objid;
		
		
		foreach( $formarr as $key=>$val) {
			$hiddenarr[$key] = $val;
		}
		reset ($formarr); 
		
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 1);
		$formobj->close( TRUE );
	}
	
	function _showSubstAliList(&$sqlo, &$poslist) {
		$headOpt = array( "title" => "Container entries");
		$headx   = array ( "Substance", "Aliquot", "Pos", "Pos-Alias", 'Info' );
	
		$tabobj3 = new visufuncs();
		$tabobj3->table_head($headx, $headOpt);
		
		$komma=0;
		$lastBox='0,0,0';
		$formarr=NULL;
		$boxes=NULL;
		
		foreach( $poslist as $substid=>$aliqArr) {
			
			if (!sizeof($aliqArr)) continue;
			
			$nice = obj_nice_name ( $sqlo, 'CONCRETE_SUBST', $substid );
			
			$cnt=0;
			foreach( $aliqArr as $ali_id=>$contpos) {
				
				$info=NULL;
				$formarr['substmap['.$substid.']['.$ali_id.']'] = $contpos;
				$posFeats   = $this->contSubLib->getPosFeats($sqlo, $contpos) ;
				$alias  = $posFeats['ALIAS'];
				$nowBox = $posFeats['SHELF'].','.$posFeats['RACK'].','.$posFeats['BOX'];
				if ($nowBox!=$lastBox) {
					
					if ( sizeof($boxes) ) {
						$alias = '<b>'.$alias.'</b>';
						$info  = '<B>New Box!</b>';
					}
					$boxes[]=$nowBox;
					$lastBox = $nowBox;
				}
				
				$substOut= NULL;
				$rowopt  = NULL;
				if (!$cnt) {
					$substOut=$nice;
					$rowopt=array("bgcolor" => "#EEDD00");
				}
				$dataArr = array($substOut, $ali_id, $contpos, $alias,$info);
				$tabobj3->table_row ($dataArr, $rowopt);
				$cnt++;
			}
			reset ($aliqArr); 
		
		}
		reset ($poslist);
		$tabobj3->table_close();
		
		return array($formarr,$boxes);
	}
	
	function action1(&$sqlo) {
		global $error;
		$FUNCNAME= 'action1';

		$poslist = $this->contSubLib->getPosList($sqlo);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'no valid container entries found.' );
			return;
		}
		
		list($formarr,$boxes) = $this->_showSubstAliList($sqlo, $poslist);
		
		echo '<br>';
		$aliNum = sizeof($formarr);
		$boxNum = sizeof($boxes);
		$this->info($aliNum.' Aliquots prepared.');
		$this->info($boxNum.' different box(es).');
		echo '<br>';
		
		$this->form1($formarr);
		
		echo '<br />';
		
		$redBoxStr=NULL;
		if (sizeof($boxes)) {
			foreach( $boxes as $pos=>$oneBox) {
				$redBoxStr .= '&redbox['.$pos.']=' . $oneBox;
			}
			reset ($boxes); 
		}
		echo '<img src="obj.container.img.php?id='.$this->objid , '&opt[refresh]=1'. $redBoxStr.'">';
	}
	
	
	
	function actionSave(&$sqlo, &$substmap) {
		$this->contSubLib->addAliquots( $sqlo, $substmap );
		
		$this->info('... '.$this->contSubLib->savecnt.' Aliquots reservated.');
	}
	
	
	
	/**
	 * - check: protocol already on archive ???
	 * - lock the protocol (only read)
	 */
	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$this->go    = $_REQUEST['go'];
		$this->objid = $_REQUEST['id'];
		$sqlo = &$this->_sqlo;
		$go = $this->go;
		$this->substarr = NULL;
		
		$goArray=array(
			0=>'Prepare aliquot number per substance', 
			1=>'Prepare reservation',
			2=>'Do reservation'
			);
			
		$this->_html->GoInfo($goArray);
		
		$this->contSubLib = new o_CONTAINER_addSub($sqlo, $_REQUEST['id']);
		
		$this->mainInit();
		
		if ($go<=1) {
			if (!sizeof($_REQUEST['substarr']) ) {
				$this->htmlFoot('ERROR', 'substance-list missing');
			}
			$this->checkInContainer($sqlo, $_REQUEST['substarr']);
			$this->pagelib->chkErrStop();
		}

		
		if (!$go) {
			$this->form0($sqlo);
			return;
		}
		if ($go==1) {
			$this->action1($sqlo);
			return;
		}
		if ($go==2) {
			if (!sizeof($_REQUEST['substmap'])) {
				$this->htmlFoot('ERROR', 'substance-reservation-list missing');
			}
			
			$this->actionSave($sqlo, $_REQUEST['substmap']);
			if ($error->Got(READONLY)) return;
			$this->_html->line_out('<b>Ready</b>');
			return;
		}
	}
}
