<?php
require_once('f.sql_query.inc');
require_once ('impexp/protoquant/o.CONCRETE_SUBST.quant.inc');

/**
 * show graphics of protoquant
 * @package o.CONCRETE_SUBST.protquant.img.inc
 * @swreq UREQ:2361 o.CONCRETE_SUBST > Protoquant > usedProto
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/plugin/o.CONCRETE_SUBST.protquant.img.inc 59 2018-11-21 09:04:09Z $
 * @param $parx
 * @param int $docid
 */
class o_CONCRETE_SUBST_protquant_img_XPL extends gPlugin {
	
	function register() {
		$tablename	= 'CONCRETE_SUBST';
		$title		= 'MaterialMonitoring: show graph';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array() ),
				'listChkSelect' =>1
				);
		$this->infoarr2['noHtmlHead'] =1;
		
	}
	
	
	
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$docid = $_REQUEST['docid'];
		$parx  = $_REQUEST['parx'];
		$sqlo  = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();

		
		$parx['format'] = 'img';
		$pageinfo=array();
		$pageinfo["title"] = "Analyse Protocol of substances";
		$pageinfo["info"]  = "Used to analyse e.g attached QC-Protocol-parameters";
		
		$parx['docid'] = $docid;
		
		$subst_proto_lib = new oCONCRETE_SUBST_Quant();
		$subst_proto_lib->init_quant_loop( $sqlo);
		
		$pquant_obj = new oProtoQuantC();
		$pquant_obj->initPage( $sqlo, "CONCRETE_SUBST", 2, NULL, $pageinfo, $parx );
		
		$globset_tmp = $pquant_obj->get_globset();
		debugOut('globset_tmp info:'.print_r($globset_tmp,1), 'MAIN', 1);
		
		
		$pquant_obj->selectInfo($sqlo);
		$pquant_obj->tabHeader2($sqlo);
		
		if ($error->Got(READONLY))  {
		    $allErrTxt = $error->getAllAsText();
		    $pquant_obj->print_error($allErrTxt);
		    exit;
		}
		
		$subst_proto_lib->doLoop( $pquant_obj, $sqlo, $sqlo2) ;
		
		$pquant_obj->tabClose();
		
		
		
		
// 		$mainObj = new oConcSubstProtQuantS();
// 		$mainObj->initPure( $sqlo, "CONCRETE_SUBST", 2, NULL, $pageinfo, $parx );
// 		$mainObj->initHeadSubst();
// 		$mainObj->initLib($sqlo, $id);
		
		
// 		if ( !$mainObj->parx["aprotoid"] ) {
// 			echo "No Protocol-id set.<br>\n";
// 			return;
// 		}
		
// 		// get THE abstract_protocol
// 		$mainObj->setAprotoID($sqlo, $mainObj->parx["aprotoid"]);
// 		$mainObj->selectInfo($sqlo);
// 		$mainObj->tabHeader2($sqlo);
		
// 		$mainObj->doLoop($sqlo, $sqlo2);
// 		$mainObj->tabClose();
		
	}
	
}
