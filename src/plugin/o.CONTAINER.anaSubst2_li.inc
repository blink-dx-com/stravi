<?php
/**
 * - plugin for CONTAINER
 * - analyse selection of substances in ONE container
 * - remove sustances from container
 * 
 * @package o.CONTAINER.anaSubst_li.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param int $_REQUEST['go'] <pre>
 *   0 : show subst list: show aliquots
 *   1 : show place
 *   2 : show selected aliquots
 *   3 : remove selected aliquots
 *  </pre>
 * @param int $_REQUEST['id'] ID of container
 * @param array $_REQUEST['posx'] array[POS] = 1 <pre>
 		array of POS in container for remove
		</pre>
 */
 
require_once ( "f.assocUpdate.inc");
require_once ("o.CONTAINER.subs.inc");
require_once ("o.CONTAINER.mod.inc");
require_once ('func_form.inc');
require_once ("f.sql_query.inc");
require_once ("f.visu_list.inc");
require_once ("f.objview.inc");

class o_CONTAINER_anaSub2 {
function __construct(&$sqlo, $cont_id) {
	$this->cont_id = $cont_id;
	
	$sqlOrigin  = get_selection_as_sql( 'CONCRETE_SUBST' );
	$sqlAfterNoOrd = $sqlOrigin . ' and x.CONCRETE_SUBST_ID in '.
		'(select CONCRETE_SUBST_ID from CONT_HAS_CSUBST where CONTAINER_ID='.$cont_id.')';
	$this->sqlAfterNoOrd  = $sqlAfterNoOrd;
	$this->sqlAfter = $sqlAfterNoOrd . ' order by x.NAME';
	
	$this->objcnt = $this->_elemCount( $sqlo );
}

function _elemCount( &$sqlo ) {
	
	$sqlsel = 'count(1) from '.$this->sqlAfterNoOrd;
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	return ( $sqlo->RowData[0] );
}
	
}	


class o_CONTAINER_anaSubst2_li_XPL extends gPlugin {
	var $CLASSNAME='o_CONTAINER_anaSubst_li_XPL';
	
	
	
	function info($text) {
		$this->_html->line_out('Info: '.$text);
	}
	
	function register() {
		$tablename	= 'CONTAINER';
		$title		= 'analyse selected substances in container';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = 'analyse substances';
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		if ($_REQUEST['go']>0) {
			$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('write'), 'obj'=>array('insert') )
				);
		}
		
		
	}
	
	function form1(&$sqlo) {
		
		$initarr   = NULL;
		$initarr["title"]       = "Remove Aliquots";
		$initarr["submittitle"] = "Next &gt;&gt;";
		$initarr["tabwidth"]    = "AUTO";
		
		$hiddenarr = NULL;
		$hiddenarr['id']=$this->objid;
		
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
		
		$formobj->close( TRUE );
		
	}
	
	/**
	 * get aliquots of one substance in container
	 * @return array($aliqArr,$maxStoreReached)
	 * 		$aliqArr [pos] = array('aliqno'=>, 'q'=>quantity)
	 */
	function _getAliqots(&$sqlo, $substid) {
		$cont_id=$this->cont_id;
		$MAX_ALIQ_SHOW=20;
		
		$maxStoreReached=0;
		// search aliquots; not many (less 20) exist
		$sqlsel ='POS, ALIQUOT_NO, QUANTITY from CONT_HAS_CSUBST where CONTAINER_ID='.$cont_id.
			' and CONCRETE_SUBST_ID='.$substid. ' order by ALIQUOT_NO';
		$sqlo->Quesel($sqlsel);

		$cnt=0;
		$aliqArr=NULL;
		while ( $sqlo->ReadRow() ) {
			if ($cnt>$MAX_ALIQ_SHOW) {
				$maxStoreReached=1;
				break;
			}
			$pos    = $sqlo->RowData[0];
			$ali_no = $sqlo->RowData[1];
			$quantity =  $sqlo->RowData[2];
			$aliqArr[$pos]=array('aliqno'=>$ali_no, 'q'=>$quantity);
			$cnt=$cnt+1;
		}
		return array($aliqArr,$maxStoreReached);
	}
	
	/**
	 * get aliquots of one substance in container
	 * @return array($aliqArr,$maxStoreReached)
	 * 		$substArr [substid][pos] = array('aliqno'=>, 'q'=>quantity)
	 */
	function _getSubstByPos(&$sqlo, &$posarr) {
		$cont_id = $this->cont_id;
		// $maxStoreReached=0;
		$substArr = NULL;
		$cnt=0;
		
		
		foreach( $posarr as $pos=>$val) {
			$sqlsel ='CONCRETE_SUBST_ID, ALIQUOT_NO, QUANTITY from CONT_HAS_CSUBST '.
				' where CONTAINER_ID='.$cont_id.
				' and POS='.$pos;
			$sqlo->Quesel($sqlsel);
			$sqlo->ReadRow();
			$subst  = $sqlo->RowData[0];
			$ali_no = $sqlo->RowData[1];
			$quant  = $sqlo->RowData[2];
			
			if ($subst) {
			    $substArr[$subst][$pos] = array('aliqno'=>$ali_no, 'q'=> $quant);
			} else {
				echo 'WARN: no substance on POS:'.$pos.'<br />';
			}
			$cnt=$cnt+1;

		}
		 
		
		return $substArr;
	}
	
	function _tableOpen($showForm, $nextGo) {
		$cont_id=$this->cont_id;
		
		if ($showForm) {
			echo '<form style="display:inline;" method="post" '.
				 ' name="editform"  action="p.php?mod='.$this->_mod.'">'."\n";
			echo "<input type=hidden name='go' value='".$nextGo."'>\n";
			echo "<input type=hidden name='id' value='".$cont_id."'>\n";
		}
		$this->boxes=NULL;
		$this->lastBox='0,0,0';
		$headOpt = array( "title" => "Container entries");
		$headx   = array ( "Substance", "Aliquot", "Quantity", "Pos", "Pos-Alias", 'Info' );
	
		$this->tabobj3 = new visufuncs();
		$this->tabobj3->table_head($headx, $headOpt);
	}
	function _tableClose($showForm, $buttonText) {
		$this->tabobj3->table_close();
		
		if ($showForm) {
			echo '<input type=submit value="'.$buttonText.'" class="yButton">'."\n";
			echo "</form><br />\n";
		}
	}
	
	/**
	 * show aliquot array of one substance
	 * @return -
	 */
	function _showAliqArr(&$sqlo, $substid, &$aliqArr, $showForm, 
		$maxStoreReached, $hiddenBoxes=0) {
		
		$objLinkLib = new fObjViewC();
		$nice = $objLinkLib->bo_display ( $sqlo, 'CONCRETE_SUBST', $substid );
		$cnt=0;
		foreach( $aliqArr as $contpos=>$pos_info) {
			
		    $ali_id = $pos_info['aliqno'];
		    $quantity = $pos_info['q'];
			$info  = '';
			$loopSubstFirst=0;
			
			$posFeats   = $this->contAnaLib->getPosFeats($sqlo, $contpos) ;
			$alias  = $posFeats['ALIAS'];
			$nowBox = $posFeats['SHELF'].','.$posFeats['RACK'].','.$posFeats['BOX'];
			if ($nowBox!=$this->lastBox) {
				
				if ( sizeof($this->boxes) ) {
					$info  = '<B>New Box!</b>';
				}
				$this->boxes[] = $nowBox;
				$this->lastBox = $nowBox;
			}
			
			$substOut= NULL;
			$rowopt  = NULL;
			if (!$cnt) {
				$loopSubstFirst=1;
				$substOut=$nice;
				$rowopt=array("bgcolor" => "#EEDD00");
			}
			$ali_out = $ali_id;
			if ($showForm) {
				if ($hiddenBoxes) {
					$ali_out = '<input type=hidden '.
						'name="posx['.$contpos.']" value=1> '.$ali_id ;
				} else {
					$checked ='';
					if ($loopSubstFirst) $checked =' checked';
					$ali_out = '<input type=checkbox '.
						'name="posx['.$contpos.']" value=1'.$checked.'> '.$ali_id ;
				}
			}
			$dataArr = array($substOut, $ali_out, $quantity, $contpos, $alias, $info);
			$this->tabobj3->table_row ($dataArr, $rowopt);
			$cnt++;
		}
		 
		if ($maxStoreReached) {
		    $dataArr = array("", "...", "...","...", "...","more aliquots");
			$this->tabobj3->table_row ($dataArr, $rowopt);
		}
	
	}
	
	/**
	 * - analyse selected substances
	 * - show them in a list
	 * @return 
	 * @param object $sqlo
	 */
	function _showSubstAliList(&$sqlo, &$sqlo2) {
		$cont_id=$this->cont_id;
		$go = $this->go;
	
		if ($go==1) {
			$showForm=1;
			
		}
		$this->_tableOpen($showForm, 2);
		
		$komma=0;
		
		
		// sorted by SUBSTANCE LIST-VIEW-order !
		$sqlAfter=$this->contSubLib->sqlAfter;
		
		$sqlsel ='CONCRETE_SUBST_ID from '.$sqlAfter;
		$sqlo2->Quesel($sqlsel);
		
		while ( $sqlo2->ReadRow() ) {
			
			$substid = $sqlo2->RowData[0];
			list($aliqArr,$maxStoreReached) = $this->_getAliqots($sqlo, $substid);
	
			if (empty($aliqArr)) {
				
			    $dataArr = array($substid, "", "", "", "","Not in container");
				$this->tabobj3->table_row ($dataArr);
				
			} else {
				
				$this->_showAliqArr($sqlo, $substid, $aliqArr, $showForm, $maxStoreReached);
			}
		
		}
		$this->_tableClose($showForm,'Next &gt;&gt;');
		
		
		
		
	}

	/**
	 * check, if substance are in one container
	 * @return container-id
	 * @param object $sqlo
	 */
	function action0(&$sqlo, &$sqlo2) {
		global $error;
		$FUNCNAME= $this->CLASSNAME.':action0';
		
		
		$this->_showSubstAliList($sqlo, $sqlo2);
	}
	
	/**
	 * check, if substance are in one container
	 * @return container-id
	 * @param object $sqlo
	 */
	function action2(&$sqlo, &$sqlo2) {
		global $error;
		$FUNCNAME= $this->CLASSNAME.':action2';
		
		$showForm=1;
		if ($this->go==3) $showForm=0;
		
		$substArr = &$this->substArr;
		
		$this->_tableOpen($showForm, $this->go+1);
		foreach( $substArr as $substid=>$aliqArr)
		{
			$this->_showAliqArr($sqlo, $substid, $aliqArr, $showForm, 0, 1);
		
		}
		
		
		$this->_tableClose($showForm, 'Take from NOW!');
	}
	
	/**
	 * do TAKE OUT !
	 * @return 
	 * @param object $sqlo
	 * @param object $sqlo2

	 */
	function action3(&$sqlo, &$sqlo2) {
		global $error;
		$FUNCNAME='action3';
		
		$substArr = &$this->substArr;
		$cont_id  = $this->cont_id;
		
		$modLib   = new oContainerModC (NULL);
		$modLib->setContainer($sqlo, $cont_id);
		$modLib->activateModi($sqlo);
		
		if (!sizeof($substArr)) {
			$error->set( $FUNCNAME, 2, 'no substances selected.' );
			return;
		}
		
		$posDef   = array();
		$cnt=0;
		foreach( $substArr as $substid=>$aliqArr)
		{
		    foreach( $aliqArr as $contpos=>$pos_info) {
				// remove substance-info
				
				$posDef['POS']     =$contpos;
				$posDef['substid'] =$substid;
				$modLib->initSubst($posDef); 
				$modLib->del($sqlo);
				if ($error->Got(READONLY))  {
					$error->set( $FUNCNAME, 1, 'take out failed on POS:'.$contpos );
					return;
				}
				$cnt++;
			}
			
		}
		
		
		echo '<br><b>'.$cnt.'</b> substance (aliquots) removed from container.<br />';
	}
	
	function introInfo() {
		
		if ($this->go) {
			$goArray = array(
				0=>'Analyse container', 
				1=>'Check aliquots',
				2=>'Show selected aliquots',
				3=>'Take them from container');
			$this->_html->GoInfo($goArray);
		} else {
			$initarr   = NULL;
			$initarr["title"]       = "Take from container: Prepare";
			$initarr["submittitle"] = "Next &gt;&gt;";
			$initarr["tabwidth"]    = "AUTO";
			$hiddenarr = NULL;
			$hiddenarr['id']=$this->cont_id;
			$formobj =  &$this->_html->formInit($initarr, $hiddenarr);
			$formobj->close( TRUE );
			$this->_html->line_out('');
		}
		
		$objLinkLib = new fObjViewC();
		$htmlOut = $objLinkLib->tableViewLink('CONCRETE_SUBST');
		echo 'Selected: <b>'.$this->contSubLib->objcnt.'</b> '.$htmlOut.'<br /><br />';
	}
	
	
	/**
	 * main
	 */
	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$this->go    = $_REQUEST['go'];
		$this->cont_id    = $_REQUEST['id'];
		
		$cont_id = $this->cont_id;
		$sqlo  = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		$go = $this->go;
		$this->infox = NULL;
		
		$tablename='CONCRETE_SUBST';
		$listAnaLib = new visu_listC();
		$copt = array ("doCount"=>1 ); // prevent double SQL counting
		list ($stopFlag, $stopReason, $objcnt)= $listAnaLib->checkSelection( $sqlo, $tablename, $copt );
		if ( $stopFlag<0 ) {
			$tablenice = tablename_nice2($tablename);
			$this->htmlFoot('ERROR', $stopReason." Please select elements of '".$tablenice."'!");
		}

		$this->contSubLib = new o_CONTAINER_anaSub2($sqlo, $cont_id);
		$this->contAnaLib = new oCONTAINER_SubsC();
		$this->contAnaLib->setContainer($sqlo, $cont_id) ;
		
		$this->introInfo();
		
		if ($go<=1) {
			$this->action0($sqlo, $sqlo2);		
		}
		
		if ($go>=2) {
			$this->substArr = $this->_getSubstByPos($sqlo, $_REQUEST['posx']);
			if (empty($this->substArr)) {
				$error->set( $FUNCNAME, 1, 'No substances for positions found.' );
				return;
			}
		}
		if ($go==2) {
			if (!sizeof($_REQUEST['posx'])) {
				$this->htmlFoot('ERROR','No substances selected.');
			}
			$this->action2($sqlo, $sqlo2);
			
		}
		if ($go==3) {
			if (!sizeof($_REQUEST['posx'])) {
				$this->htmlFoot('ERROR','No substances selected.');
			}
			$this->action3($sqlo, $sqlo2);
			
		}
		$this->pagelib->chkErrStop();
		
		if ($go<3) {
		    
			$redBoxStr=NULL;
			if (sizeof($this->boxes)) {
				foreach( $this->boxes as $pos=>$oneBox) {
					$redBoxStr .= '&redbox['.$pos.']=' . $oneBox;
				}
			}
			
			
			if ($this->contAnaLib->has_coordinates($sqlo)) {
			    echo '<br><img src="obj.container.img.php?id=' .$this->cont_id . $redBoxStr.'">';
			}
		}
	}
}
