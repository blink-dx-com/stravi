<?php
/**
 * use of [protoQuant]
   simple protocol compare (show only quantities)
 * @package obj.exp.prot_comp2.php
 * @swreq UREQ:0001578: o.PROTO > ProtoQuant : Analyse protocol steps 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $parx
 * "doc_id" : doc_id
 * ["format"] = ["html"], "csv"
   @param $action
      'sel_config'
      'sel_apid'
 * @version $Header: trunk/src/www/pionir/obj.exp.prot_comp2.php 59 2018-11-21 09:04:09Z $
 */


session_start(); 

require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("down_up_load.inc");
require_once ("class.filex.inc");
require_once ('func_form.inc');
require_once ("javascript.inc" );
require_once ("sql_query_dyn.inc");
require_once ("f.visu_list.inc");

require_once ('impexp/protoquant/o.EXP.quant.inc');
require_once ('impexp/protoquant/o.PROTO.quant_gui.inc');

// --------------------------------------------------- 
global $error;
$parx   = $_REQUEST['parx'];
//$action = $_REQUEST['action'];


$error = & ErrorHandler::get();
$sqlo   = logon2( $_SERVER['PHP_SELF']); 
$sqlo2  = logon2( );
if ($error->printLast()) htmlFoot();


$tablename = "EXP";
$mainObj = new oProtoQuantC();
$guilib  = new oProtoQuantGuiC($tablename);

$suc_sess_conf = $_SESSION['s_formState']['o.EXP.pq'];
if (!$suc_sess_conf['docid']) {
    $conf_url = $guilib->get_config_url();
    js__location_replace($conf_url);
    exit;
}
$parx['docid'] = $suc_sess_conf['docid'];
$parx['table'] = 'EXP';


$pageinfo=array();
$pageinfo["title"] = "compare experiment protocols";
$pageinfo["info"]  = "Show protocol steps of selected experiments";
$mainObj->initPage( $sqlo, $tablename, 2, 0, $pageinfo, $parx );



if ( !$parx["docid"] ) {
    htmlFoot('ERROR','docid missing.');
}

$globset_tmp = $mainObj->get_globset();
debugOut('globset_tmp info:'.print_r($globset_tmp,1), 'MAIN', 1);

$exp_proto_lib = new oEXP_Quant();
$exp_proto_lib->init_quant_loop( $mainObj, $sqlo);

$mainObj->selectInfo($sqlo);

$mainObj->tabHeader2($sqlo);

if ($error->Got(READONLY))  {
    $error->printAll();
    exit;
}

$exp_proto_lib->doLoop( $sqlo, $sqlo2) ;

$mainObj->tabClose();
$format = $mainObj->parx["format"];

gHtmlMisc::func_hist("obj.exp.prot_comp2", 'ProtoQuant Exp', $_SERVER['PHP_SELF'] );

if ($format=="html" or $format=='csv') {
	htmlFoot("<hr>");
}


