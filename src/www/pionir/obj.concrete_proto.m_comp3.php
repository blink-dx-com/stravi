<?php
/**
 * compare protocols, export steps HORIZONTAL
 * - called by obj.concrete_proto.m_comp2e.php
 * 
 * @package obj.concrete_proto.m_comp3.php
 * @swreq UREQ:0000671: o.CONCRETE_PROTO > protocol compare >  HORIZONTAL
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $step[]               : array of STEP_NR in concrete_proto_step
								: array[STEP_NR] = 1
		$tablename
		$parx["a_proto_id"]   : a_proto_id
		$parx["format"]       : csv, html
		$parx["ref_exp"]      : ID of ref-object
		$projid
 */
 
session_start(); 

require_once ('reqnormal.inc');
require_once ("gui/o.PROTO.stepout1.inc");
require_once ('subs/obj.concrete_proto.m_comp2e.su1.inc');
require_once ('glob.objtab.page.inc');
require_once ("visufuncs.inc");
require_once ("down_up_load.inc");
 
class gCsvProtoHelp {
	
	function init($format) {
		$this->csvKeys = array('quant'=>"QUANT", 'SUBST_NAME'=>"SUBST", 'DEV_NAME'=>'DEV', 'notes'=>"NOTES");
		
		$this->tabobj = new visufuncs();
		$headOpt = array( 
				"title" => "Protocol-steps",
		 		"format" => $format,
				"headNoShow"=>1
						  );
		$headx=array();
		$this->tabobj->table_head($headx,   $headOpt);
		$this->outarr = NULL;
		
	}
	
	function lineStart($objid, $objname, $status) {
		$this->outarr = array($objid ,$objname,$status);
	}
	
	function oneSteparr($stepnr, $datx) {
		reset($this->csvKeys);
		
		foreach( $this->csvKeys as $key=>$dummy) {
			$this->outarr[] = $datx[$key];
			
		}
		reset($this->csvKeys);
	}
	
	function lineEnd() {
		$this->tabobj->table_row ($this->outarr);
	}
	
	function table_close() {
		$this->tabobj->table_close();
	}
}
 
class oCPROTO_comp3 {
	function __construct(&$sqlo, $proj_id, $tablename, $parx, $step, $sqlAfterOrder) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		$this->parx = $parx;
		$this->step = $step;
		$this->tablename = $tablename;
		
		$this->helpLib = new oProtoCompLib($proj_id, $tablename, $parx, $step);
		$this->helpLib->InitialChecks();
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'error on init protocol lib.' );
			return;
		}
		
		$this->oProtoOrgLib = new gProtoOrg();
		$this->csvLib  = new gCsvProtoHelp();
		
		if ($this->parx["format"]=="csv") { 
			$filename = "ProtocolCompare3.xls";
			set_mime_type("application/vnd.ms-excel", $filename);
		}
		
		ob_end_flush ();
		
		$this->csvLib->init($this->parx['format']);
		
		$go=0;
		$this->sel=array();
		$num = $this->helpLib->getSel( $sqlo, $this->sel, $sqlAfterOrder, $go );
	}
	
	function init(&$sqlo) {
		$this->first_a_proto=NULL;
		
		$first_obj = key($this->sel);
		reset($this->sel);

		$inf =	$this->helpLib->obj_one_proto_get( $sqlo, $first_obj  );  
		$this->first_a_proto = $inf['apid'];

	}

	function _showCsvHead(&$sqlo) {
		$this->absDatArr = NULL;
		$csvKeys = $this->csvLib->csvKeys;
		$step    = &$this->step;
		
		$this->csvLib->lineStart('# abstract:'.$this->first_a_proto, '', '');	
		
		foreach( $step as $step_nr=>$dummy) {
			$sqlsel = '* from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID='.
				$this->first_a_proto. ' and STEP_NR='.$step_nr;
			$sqlo->Quesel($sqlsel);
			$sqlo->ReadArray();
			$aarr = $sqlo->RowData;
			$outarr=NULL;
			$outarr['quant']=$aarr['NAME'];
			
			$this->absDatArr[$step_nr]['QUANTITY']=$aarr['QUANTITY'];
			
			$this->csvLib->oneSteparr($step_nr,$outarr);
		}
		reset ($step);	
		$this->csvLib->lineEnd();
		
		$this->csvLib->lineStart('OBJ_ID', 'OBJ_NAME', 'Info');
		foreach( $step as $step_nr=>$dummy) {
			$outarr['quant']	= $step_nr.':'.'QUANT';
			$outarr['SUBST_NAME']=$step_nr.':'.'SUBST';
			$outarr['DEV_NAME']  =$step_nr.':'.'DEV';
			$outarr['notes']	= $step_nr.':'.'NOTES';
			$this->csvLib->oneSteparr($step_nr,$outarr);
		}
		reset ($step);	
		$this->csvLib->lineEnd();
		
	}
	
	function showSteps(&$sqlo) {

		$a_arr = &$this->a_arr;
		
		$sel  = &$this->sel;
		$step = $this->step;
		
		$first_a_proto = $this->first_a_proto;
		
		$this->_showCsvHead($sqlo);
			
		
	   
		$total_proto_steps = 0; 
	    $numSteps = sizeof($step);
		$a_arr = &$this->absDatArr;
		
	 	reset($sel);
		foreach( $sel as $obj_id=>$dummy) {
			
			
			$inf_loop = $this->helpLib->obj_one_proto_get( $sqlo, $obj_id, $first_a_proto );    
			$obj_name   = $inf_loop['name'];
			$c_proto_id = $inf_loop['cpid'];
			$a_proto_tmp= $inf_loop['apid'];
			
			$protoStepCnt = 0;
			$proto_ok = 1;
			$status='';
			if ($c_proto_id) {  
		        if ($a_proto_tmp!=$first_a_proto) { 
		            // abstract protos different
		            $status="different protocol: $a_proto_tmp";
		            $proto_ok = 0;
		        }
		    } else {        
		        $status="no protocol";
		        $proto_ok = 0;
		    }
			
			$this->csvLib->lineStart($obj_id, $obj_name, $status);
			
			if ($proto_ok) {
				
				foreach( $step as $proto_step_nr=>$dummy) {
					
					$c_arr = $this->oProtoOrgLib->cproto_step_info( $sqlo, $c_proto_id, $proto_step_nr );
					
					$datx=NULL;
					if ( $c_arr["NOT_DONE"]==1) $datx["inact"] = 1;
					if ( is_array($c_arr) )  $datx["exist"] = 1;
					
					// $datx["stepnr"] = $a_arr[$proto_step_nr]["STEP_NR"];
					$datx["notes"]  = $c_arr["NOTES"];
					
					$datx["quant"] = $c_arr["QUANTITY"];
					if ($c_arr["QUANTITY"]=="") {
						$datx["quant"] = $a_arr[$proto_step_nr]["QUANTITY"];
					}
					$datx["SUBST_NAME"] = $c_arr["SUBST_NAME"];
					$datx["DEV_NAME"]    = $c_arr["DEV_NAME"];
					
					$this->csvLib->oneSteparr($proto_step_nr, $datx);
					$protoStepCnt++; 
	
				}
				reset ($step);	
			}
			$this->csvLib->lineEnd();	
		
		}
		reset($sel);
		$this->csvLib->table_close();
	}
}



global $error;
$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
// $sqlo2  = logon2( );
if ($error->printLast()) htmlFoot();

$step=$_REQUEST['step'];
$tablename=$_REQUEST['tablename'];
$parx=$_REQUEST['parx'];
$proj_id=$_REQUEST['proj_id'];

$title       = "horizontal compare, selected steps";
$infoarr=NULL;
$infoarr["obj_cnt"]  = 1;          // show number of objects
$infoarr["title"]  = $title;

if ($parx["format"]==NULL) {
	$parx["format"]='html';
}

$headopt = array( "obj_cnt"=>1, "noHead"=>1 );
if ($parx["format"]=='html') {
	$headopt["noHead"]=0;
}
$mainObj = new gObjTabPage($sqlo, $tablename );
$mainObj->showHead($sqlo, $infoarr, $headopt);
$mainObj->initCheck($sqlo);

$sqlAfterOrder = $mainObj->getSqlAfter();

$mainLib = new oCPROTO_comp3($sqlo, $proj_id, $tablename, $parx, $step, $sqlAfterOrder);
if ($error->Got(READONLY))  {
	$error->printAll();
	htmlFoot();
}

$mainLib->init($sqlo);
if ($parx["format"]=='html') echo '<pre>';
$mainLib->showSteps($sqlo);
if ($parx["format"]=='html') echo '</pre>';

if ($parx["format"]=='html') {
	$mainObj->htmlFoot();
}