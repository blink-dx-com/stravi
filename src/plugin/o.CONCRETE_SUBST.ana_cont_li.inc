<?php
/**
 
 * - analyse selection of substances 
 * 
 * @package o.CONCRETE_SUBST.ana_cont_li.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
 
require_once ( "f.assocUpdate.inc");
require_once ("o.CONTAINER.subs.inc");
require_once ("o.CONTAINER.mod.inc");
require_once ('func_form.inc');
require_once ("f.sql_query.inc");
require_once ("f.visu_list.inc");
require_once ("f.objview.inc");



class o_CONCRETE_SUBST_ana_cont_li_XPL extends gPlugin {
	
	
	
	function info($text) {
		$this->_html->line_out('Info: '.$text);
	}
	
	function register() {
		$tablename	= 'CONCRETE_SUBST';
		$title		= 'storage analysis';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = 'analyse storage quantities';
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr["obj_cnt"]  = 1;
		
		
		if ($_REQUEST['go']>0) {
			$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read') )
				);
		}
		
		
	}
	
	
	
	/**
	 * get aliquots of one substance in container
	 * @return array($aliqArr,$maxStoreReached)
	 * 		$aliqArr [pos] = array('aliqno'=>, 'q'=>quantity, 'cont', 'alias'=>'')
	 */
	function _getAliqots(&$sqlo, $substid) {
		
		$MAX_ALIQ_SHOW=40;
		
		$maxStoreReached=0;
		// search aliquots; not many (less 20) exist
		$sqlsel ='POS, ALIQUOT_NO, QUANTITY, CONTAINER_ID from CONT_HAS_CSUBST where '.
			' CONCRETE_SUBST_ID='.$substid. ' order by ALIQUOT_NO';
		$sqlo->Quesel($sqlsel);

		$cnt=0;
		$aliqArr=array();
		while ( $sqlo->ReadRow() ) {
		    
			if ($cnt>$MAX_ALIQ_SHOW) {
				$maxStoreReached=1;
				break;
			}
			$pos    = $sqlo->RowData[0];
			$ali_no = $sqlo->RowData[1];
			$quantity =  $sqlo->RowData[2];
			$aliqArr[] = array( 'aliqno'=>$ali_no, 'q'=>$quantity, 'cont'=>$sqlo->RowData[3], 'pos'=>$pos );
			$cnt=$cnt+1;
		}
		
		$cont_lib = new oCONTAINER_SubsC();
		
		
		
		// POST analysis of container
		// get aliases ...
		$old_cont=0;
		for ($i=0; $i < sizeof($aliqArr); $i++) {
		    
		    $row = $aliqArr[$i];
		    $cont_id_loop = $row['cont'];
		    if (!$cont_id_loop) continue;
		    
		    if ($old_cont!=$cont_id_loop) {
		        $cont_lib->setContainer($sqlo, $cont_id_loop);
		    }
		    
		    
		    if ($cont_lib->has_coordinates($sqlo)) {
		        $alias = $cont_lib->getPosAlias($sqlo, $row['pos']);
		        $aliqArr[$i]['alias'] = $alias;
		       
		    }
		    
		    $old_cont = $cont_id_loop;
		}
		
		
		return array($aliqArr,$maxStoreReached);
	}
	
	
	
	function _tableOpen($showForm, $nextGo) {
		
		
		
		$headOpt = array( "title" => "Storage entries");
		$headx   = array ( "Substance", "Storage", "Aliquot", "Quantity", "Pos", "Pos-Alias", 'Info' );
	
		$this->tabobj3 = new visufuncs();
		$this->tabobj3->table_head($headx, $headOpt);
	}
	function _tableClose($showForm, $buttonText) {
		$this->tabobj3->table_close();
		
		
	}
	
	/**
	 * show aliquot array of one substance
	 * @return -
	 */
	function _showAliqData($sqlo, $substid, $aliq_data, $maxStoreReached) {

		$nice = $this->objLinkLib->bo_display ( $sqlo, 'CONCRETE_SUBST', $substid );
		
		if (empty($aliq_data)) {
		    $dataArr = array($nice, '<span style="color:gray;">not in store</span>', "...","...", "...","","");
		    $this->tabobj3->table_row ($dataArr);
		    return;
		}
		
		$cnt=0;
		foreach( $aliq_data as $pos_info) {
			
		    $ali_id   = $pos_info['aliqno'];
		    $quantity = $pos_info['q'];
		    $cont_id  = $pos_info['cont'];
			$alias    = $pos_info['alias'];
			$info  = '';
			
			$cont_out = obj_nice_name ( $sqlo, 'CONTAINER', $cont_id );
			

			$substOut= NULL;
			$rowopt  = NULL;
			if (!$cnt) {	
				$substOut = $nice;
				$rowopt=array("bgcolor" => "#EEDD00");
			}
			
			$dataArr = array( $substOut, $cont_out, $ali_id, $quantity, $pos_info['pos'], $alias, $info );
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
	function show_list(&$sqlo, &$sqlo2, $sqlAfter) {
		
		$go = $this->go;
		$showForm=0;
		
		$this->objLinkLib = new fObjViewC();
		
		$this->_tableOpen($showForm, 2);

		
		$sqlsel ='CONCRETE_SUBST_ID from '.$sqlAfter;
		$sqlo2->Quesel($sqlsel);
		
		while ( $sqlo2->ReadRow() ) {
			
			$substid = $sqlo2->RowData[0];
			list($aliq_data, $maxStoreReached) = $this->_getAliqots($sqlo, $substid);
			$this->_showAliqData($sqlo, $substid, $aliq_data, $maxStoreReached);	
		
		}
		$this->_tableClose($showForm,'Next &gt;&gt;');

		
	}


	/**
	 * main
	 */
	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$this->go    = $_REQUEST['go'];
		
		
		$sqlo  = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		$go = $this->go;
		$this->infox = array();
	
		$sqlAfter = $this->listLib->getSqlAfter();
		
		echo '[<a href="p.php?mod=DEF/o.CONTAINER.anaSubst_li">Take out selected MACs</a>]<br><br>';

		
		if ($go<=1) {
		    $this->show_list($sqlo, $sqlo2, $sqlAfter);		
		}

		$this->pagelib->chkErrStop();
		
		
	}
}
