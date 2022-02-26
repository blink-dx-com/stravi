<?php
/**
 * - plugin for CONTAINER
 * - analyse selection of substances in container
 * - post selection of substances to ONE selected container
 * 
 * @package o.CONTAINER.anaSubst_li.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param int $_REQUEST['go']
 *   0 : show subst list: show aliquots
 *   1 : forward to container
 */
 
require_once ( "f.assocUpdate.inc");
require_once ("o.CONTAINER.subs.inc");
require_once ('func_form.inc');
require_once ("f.sql_query.inc");
require_once ("f.objview.inc");
require_once ( "javascript.inc" );

	
	


	



class o_CONTAINER_anaSubst_li_XPL extends gPlugin {
	var $CLASSNAME='o_CONTAINER_anaSubst_li_XPL';
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function info($text) {
		$this->_html->line_out('Info: '.$text);
	}
	
	function register() {
		$tablename	= 'CONCRETE_SUBST';
		$title		= 'analyse selected substances in container';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = 'analyse container';
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		if (!$_REQUEST['go']) {
			//$this->infoarr['obj_id']   = $_REQUEST['id'];
			//$this->infoarr['checkid']  = 1;
		}
		
	}
	
	/**
	 * are substances in one container ?
	 * @return number of containers
	 * @param object $sqlo
	 */ 
	function checkSubstInCont(&$sqlo, &$sqlo2)  {
		global $error;
		$FUNCNAME= 'checkSubstInCont';
		
		$forwardUrl= $this->forwardUrl;
		$this->firstCont=0;
		$this->sqlAfterNoOrd  = get_selection_as_sql( 'CONCRETE_SUBST' );
		
		$sqlsel ='distinct(CONTAINER_ID) from CONT_HAS_CSUBST where  CONCRETE_SUBST_ID in '.
			     '(select CONCRETE_SUBST_ID from '.$this->sqlAfterNoOrd.')';
		
		$sqlo2->Quesel($sqlsel);
		$cnt=0;
		$objLinkLib = new fObjViewC();
		
		while ($sqlo2->ReadRow() ) {
			
			$cont_id = $sqlo2->RowData[0];
			if (!$cnt) $this->firstCont = $cont_id;
				
			// get edit-link of BO + NAME + icon (object)
			
			$html1 = $objLinkLib->bo_display2( $sqlo, 'CONTAINER', $cont_id ); 
			echo '<a href="'.$forwardUrl.$cont_id.'">'.$html1.'</a></br>'."\n";
			$cnt++;
		}
		echo '<br />';
		
		if ($cnt!=1) {
			$errtxt='Error';
			if (!$cnt) {
				$errtxt='No substance is in a container.';
				$error->set( $FUNCNAME, 1, $errtxt );
				return;
			}
			//if ($cnt>1) {
			//	$errtxt='Substance are situated in more than ONE container.';
			//}
			
		}
		return ($cnt);
	}
	
	
	/**
	 * check, if substance are in one container
	 * @return container-id
	 * @param object $sqlo
	 */
	function action0(&$sqlo, &$sqlo2) {
		global $error;
		$FUNCNAME= $this->CLASSNAME.':action0';
		
		$cnt = $this->checkSubstInCont($sqlo, $sqlo2);
		if ($error->Got(READONLY))  {
			return;
		}
		
		if ($cnt==1) {
			// forward to first container
			js__location_replace( $this->forwardUrl.$this->firstCont , 'container' ); 
		}
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
		$this->infox = NULL;
		
		$tablename='CONCRETE_SUBST';
		$listLib = new gPlugListC($tablename);
		$listLib->initCheck($sqlo);
		if ($error->Got(READONLY))  {
			return;
		}
		
		$cnt = $listLib->getObjCnt();
		echo 'Selected Substances: <b>'.$cnt.'</b><br />';
		$this->forwardUrl = 'p.php?mod=DEF/o.CONTAINER.anaSubst2_li&id=';
		
		$this->sqlAfter = $listLib->getSqlAfter();
		
		if ($go<=1) {
			
			$this->action0($sqlo, $sqlo2);
			$this->pagelib->chkErrStop();
		}

		
	}
}
