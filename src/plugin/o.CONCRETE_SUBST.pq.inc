<?php
require_once ($_SESSION['s_sessVars']['loginPATH'].'/impexp/protoquant/o.CONCRETE_SUBST.quant.inc');
require_once ($_SESSION['s_sessVars']['loginPATH'].'/impexp/protoquant/o.PROTO.quant_gui.inc');

/**
 * [Protoquant] : analyse protocols of selected substances
 * USN:
 *   01: if no docid give:
 *    - get $user_globals_qu from $_SESSION['userGlob']["o.proto.Quant_sel"]
 *    - if $user_globals_qu['doc.sel'] = AUTO :
 *         get docid from .profile/Protoquant/ document with feature: MAA-ID = this MAA-ID
 *           use cache: $_SESSION['s_formState']['o.PROTO.quant']['SUA_arr']
 *      else: ('useid')
 *         docid = $user_globals_qu['docid']
 *         
 *      $_SESSION['s_formState']['o.CONCRETE_SUBST.pq'] = array('docid'=>)  
 * GLOBAL: 
 *    $_SESSION['userGlob']["o.proto.Quant_sel"]
 *    $_SESSION['s_formState']['o.PROTO.quant'] = array( -- features during session
 *        'SUA_arr'=> array( SUA-ID => DOC_ID ) - matching SUA-ID versus DOC-ID, must be updated, when new config-doc was created ...
 *    )
 * 
 * @package PQ_CONCRETE_SUBST
 * @module  o.CONCRETE_SUBST.pq.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @swreq UREQ:0002361: o.CONCRETE_SUBST > Protoquant > usedProto 
 * @param   $parx
     "format" = ["html"], "csv", "img", "img2"
	 "docid" : doc_id, which stores analysis parametzers
	@param  $action
	  sel_config
 */
class o_CONCRETE_SUBST_pq_XPL extends gPlugin {
	
	
	function register() {
		$tablename	= 'CONCRETE_SUBST';
		$title		= '[ProtoQuant]';
		
		$this->infoarr			   = array();
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['design']   = 'slim';
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read')  )
		);
	}
	

	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$tablename	= 'CONCRETE_SUBST';
		
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		// $go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
		
		$pquant_obj = new oProtoQuantC();
		$guilib     = new oProtoQuantGuiC($tablename);
		
		
		$suc_sess_conf = $_SESSION['s_formState']['o.CONCRETE_SUBST.pq'];
		if (!$suc_sess_conf['docid']) {
		    $conf_url = $guilib->get_config_url();
		    $this->_html->pageForward($conf_url, 'forward');
		    return;
		}
		$parx['docid'] = $suc_sess_conf['docid'];
		
		
		
		$pageinfo=array();
		$pageinfo["title"] = "Analyse Protocols of ".tablename_nice2($tablename).'s' ;
		$pageinfo["info"]  = NULL;  // OLD: Used to analyse e.g attached QC-Protocol-parameters
		$pquant_obj->initPure( $sqlo, $tablename, 2, 0, $pageinfo, $parx);
		$pquant_obj->head_table($sqlo);
		
		$globset_tmp = $pquant_obj->get_globset();
		debugOut('globset_tmp info:'.print_r($globset_tmp,1), 'MAIN', 1);
		
		
		$subst_proto_lib = new oCONCRETE_SUBST_Quant();
		$subst_proto_lib->init_quant_loop( $pquant_obj, $sqlo);
		
		
		$pquant_obj->selectInfo($sqlo);
		
		$pquant_obj->tabHeader2($sqlo);
		
		if ($error->Got(READONLY))  {
		    return;
		}

		$subst_proto_lib->doLoop( $sqlo, $sqlo2) ;
		
		$pquant_obj->tabClose();
		
		
		gHtmlMisc::func_hist( $this->_mod, 'ProtoQuant - MAC', $this->_plug_get_URL() );
	}
}
