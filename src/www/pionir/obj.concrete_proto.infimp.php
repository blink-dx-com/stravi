<?php
/**
 * [ExportProtoFormat] Export format of 'protocol step file
 * - get typical columns for a single protocol
		  - used by obj.exp.imp_sample.php
 * @package obj.concrete_proto.infimp.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $id         (CONCRETE_PROTO_ID)
		  $tablename  OPTIONAL ( table-name of related object )
							"EXP", "CONCRETE_PROTO", "W_WAFER", "CONCRETE_SUBST"
		  $parx['usesteps'] = ['set'] -- only with values in protocol
		  		   
 * @version0 2002-09-04
 */
session_start(); 

require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("class.history.inc");
require_once ("o.PROTO.steps.inc");
require_once ("down_up_load.inc");
require_once ("visufuncs.inc");

class oConcrete_protoInfImp {

function __construct($objid, $tablename, &$infoarr, $parx) {
	$this->objid = $objid;
	$this->infoarr = $infoarr;
	$this->tablename = $tablename;
	$this->parx = $parx;
}

function htmlhead(&$sql) {
	
	$pagelib = new gHtmlHead();
	$pagelib->startPage($sql, $this->infoarr);
	
	echo "<ul>";
	echo "<i>This script exports the header for a file, which fits for the tool <a href=\"obj.exp.imp_sample.php\">ProtoImporter</a>.</i><br><br>";
	
	if ($this->aproto!= NULL) {
		echo 'Selected protocol: '.$this->aproto['name']." ID:".$this->aproto['id'].'<br>';
	} 

}

function tabRow( $rowarray, $isComment=0 ) {
	if ($isComment) $rowarray[0] = '# ' . $rowarray[0];
	$this->tabobj->table_row ( $rowarray );
}

function getData( &$sql ) {
	global $error;
	$FUNCNAME= 'getData';
	
	$tablename = $this->tablename;
	$id = $this->objid;

	$useAllSteps = 0;
	if ($this->parx['usesteps'] == 'all') {
		$useAllSteps = 1;
	}
	
	$this->aproto = NULL;
	$sqls = "select a.abstract_proto_id, a.name ".
		"from CONCRETE_PROTO c, abstract_proto a where c.CONCRETE_PROTO_ID=".$id.
		" AND a.abstract_proto_id=c.abstract_proto_id";
	$sql->query($sqls);
	$sql->ReadRow();
	$a_protoid = $sql->RowData[0];
	$a_proto_name = $sql->RowData[1];
	
	if (!$a_protoid) {
		$error->set( $FUNCNAME, 1,  "No abstract protocol found.");
		return;
	}
	$this->aproto = array('id'=>$a_protoid, 'name'=>$a_proto_name );
	
	$protoObj = new gProtoOrg();
	$protoObj->setProto( $sql, $a_protoid, 0 );
	
	$step_arrayX = $protoObj->getStepArray();
	$step_count  = sizeof( $step_arrayX ) ;	
	$i=0;
	
	$stDataArr   = NULL; 
	$keyTransArr = $protoObj->getCSumCols();
	$tmpTab      = ""; 
	$hasdata     = 0;

	$nameLine  = NULL; // starts with a comment char!
	$keyline   = NULL;
	$dataLine  = NULL;
	
	$pkMother   = PrimNameGet2($tablename);
	$pkMotherNice = columnname_nice2($tablename, $pkMother);
	$nameLine[]  = $pkMotherNice;
	$keyline[]   = $pkMother;
	$dataLine[]  = 'XXX';
	
	while ( $i < $step_count ) {
		$step_nr = $step_arrayX[$i];
		
		$sqls= "select name from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$a_protoid. " AND STEP_NR=" . $step_nr;
		$sql->query($sqls);
		$sql->ReadRow();
		$stepname = $sql->RowData[0];
		
		$colArr   = array_keys($keyTransArr);	
		$colNames = implode( ",", $colArr);
		
		// all steps which are found in CONCRETE_PROTO_STEP
		$sqls= "select ".$colNames." from CONCRETE_PROTO_STEP ".
				" where CONCRETE_PROTO_ID=".$id. " AND STEP_NR=" . $step_nr;
		$sql->query($sqls);
		$sql->ReadArray();
		
		reset ($keyTransArr);	
		foreach( $keyTransArr as $key=>$val) {
			$showCol = 0;
			$valtemp = $sql->RowData[$key];
			if ($key=="NOT_DONE" AND $valtemp==0) continue;
			
			if ( $valtemp!="" ) $showCol = 1;
			if ( $useAllSteps ) $showCol = 1;
				
			if ($showCol) {
				$nameLine[] = $stepname;
				$keyline[] = $step_nr . ":" . $keyTransArr[$key];
				
				$tmpval = trim($valtemp,"\t");
				if ( strlen($tmpval)>20 ) $tmpval = substr($tmpval,0,20) ."..."; 
				$dataLine[] =  $tmpval;
				
				$stepname = ""; // only first entry gets a name
				$hasdata++;
			} 
		}	
		reset ($keyTransArr);
			 
		
		
		$i++;
	}
	
	if ($hasdata) {
	
		$csvLib = new fDownloadC();
		$csvLib->pageCsv ( $this->infoarr );
		
	
		$this->tabobj = new visufuncs();
		$headOpt = array( "title" => "Protocol import format", 'format'=>'csv', "headNoShow"=>1 );
		$headx  = array ();
		$this->tabobj->table_head($headx,   $headOpt);
		$this->tabRow( array( "protocol (abstract): $a_proto_name ID:".$this->aproto['id']), 1 );
		$this->tabRow( $nameLine, 1 );
		$this->tabRow( $keyline );
		$this->tabRow( $dataLine );
	
	}
	
	return ($hasdata);

}

}


// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( );

if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$id 		= $_REQUEST['id'];
$parx 		= $_REQUEST['parx'];
$tablename 	= $_REQUEST['tablename'];

if ( !$id and $tablename!=NULL ) {
	$hist_obj = new historyc();
	$id  = $hist_obj->last_bo_get('CONCRETE_PROTO');
}

$title   = "[ExportProtoFormat] Export format of 'protocol step file'";
$infoarr = NULL;

$infoarr["title"]    = $title;
$infoarr["title_sh"] = 'ExportProtoFormat';
$infoarr["form_type"]= "list";
$infoarr['obj_name'] = $tablename;
$infoarr['mime']     = 'application/vnd.ms-excel';
$infoarr['filename'] = 'protoColFormat.csv';
// $infoarr['inforow']  = '';
$infoarr['locrow']   = array(  array('obj.exp.imp_sample.php?tablename='.$tablename, 'ProtoImporter') );

$mainlib = new oConcrete_protoInfImp($id, $tablename, $infoarr, $parx);

if ( !$id ) {
	$mainlib->htmlhead($sql);
	htmlFoot("INFO", "No protocol (concrete) found in the history.<br> Please touch a protocol, which contain typical data of your data for import.");
}

if ( $tablename==NULL ) {
	$mainlib->htmlhead($sql);
	htmlFoot("INFO", "No mother table given.");
}

$hasdata = $mainlib->getData( $sql );


if (!$hasdata) {
	$mainlib->htmlhead($sql);
	htmlFoot("INFO", "The selected protocol contains no step data.");
}

