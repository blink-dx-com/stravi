<?php
require_once("obj.abstract_array.export_array.inc");

/**
 * export SPOT_RESULT or EXP_RAW_RESULT of a SINGLE experiment
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @package o.EXP.result.exp.inc
 * @swreq UREQ:0001154: o.EXP > FeatureExport of array experiment
 * @swreq UREQ:0001177: o.EXP > AllFeatureExport > Error, when Exp has no array
 * @var
   	  $EXP_ID: experiment ID
	  $colname    : Column name in result table (matrix for output types ??? )
	  $exp_res_type: '' (SPOT_RESULT) | 'EXP_RAW_RESULT'
	  INACTIV: $order_flag : '' (default SPOT_ID), 'ABSTRACT_SUBST_ID'
	  $application: '' (save to file), 'excel', 'cluster', 'display'
	  $extra_class_id: 0 : nothing, >0 extra_class_id
	  $witharr[]   : options for substance feature output witharr: Undefined variable.

	  	'subst_with_notes'= 0;
		'spot_names'	= 0;
		'class_info'	= 0;
	  	'subst_grouped'	= 0|1
	
  TBD: ob_start("mb_output_handler");
  		function 	mb_output_handler() {
		}
  @version $Header: trunk/src/phplib/o.EXP.result.exp.inc 59 2018-11-21 09:04:09Z $
 */
class oExpOneExportC {

var $exp_res_type;
var $channel_num;
var $colname;
var $pingout;

function __construct() {
	$this->spotResLib = new gSpotResC();
	$this->pingout    = 0; 
}

public function set_pingout($flag) {
	$this->pingout = $flag;
}

function get_spot_results( &$DB_Handle1, $exp_res_type) {

	$SQL_query = $this->typex[$exp_res_type]["query1"] . $this->EXP_ID . 
		$this->typex[$exp_res_type]["query2"]; /* count spot results */
	$DB_Handle1->query($SQL_query);
	$DB_Handle1->ReadRow();
	$this->typex[$exp_res_type]["spot_res_num"]  = $DB_Handle1->RowData[0];
}

function indyGetResNum(&$DB_Handle1, $EXP_ID) {

	$this->EXP_ID = $EXP_ID;
	
	$this->defineQuery();
	$this->get_spot_results($DB_Handle1, "SPOT_RESULT");
	$this->get_spot_results($DB_Handle1, "EXP_RAW_RESULT");
	
	$resarr=NULL;
	$resarr["res_num_norm"]  = $this->typex["SPOT_RESULT"]["spot_res_num"];
	$resarr["res_num_raw"]   = $this->typex["EXP_RAW_RESULT"]["spot_res_num"];
	
	return ( $resarr );
}

function defineQuery() {

	$this->typex["SPOT_RESULT"]=array(
		"query1"=>'select count(*) from spot_result where EXP_ID=',
		"query2"=>''
	);
	
	$this->typex["EXP_RAW_RESULT"]=array(
			"query1"=>'select count(*) from EXP_RAW_RESULT where EXP_ID=',
			"query2"=>' AND CHANNEL_ID=1'
	);
}

function getImageInfo( &$sql ) {
	global $varcol;
	
	$sqlsImg = "SELECT e.img_id, i.name, i.extra_obj_id FROM img_of_exp e, img i WHERE exp_id =".$this->EXP_ID
		." and e.img_id=i.img_id ORDER BY i_order";
	$sql->query($sqlsImg);
	$sql->ReadRow();
	$image["id"]   = $sql->RowData[0];
	$image["name"] = $sql->RowData[1];
	$image["objid"]= $sql->RowData[2];
	
	if ($image["objid"]) {
		$values_tmp = $varcol->select_by_name  ( $image["objid"] );
		$image["xvals"] = $values_tmp['values'];
		$image["class"] = $varcol->class_id_to_name( $values_tmp['extra_class_id'] );
	} 
	
	return ($image);
}

// prepare export
// $_SESSION['userGlob']['f.spot_result.table']='normalized', 'raw'
function prepare ( &$DB_Handle, &$DB_Handle1, 
	$EXP_ID, 
	$colname, 
	$exp_res_type	// "", 'SPOT_RESULT', 'EXP_RAW_RESULT'
	) {
	
	global $error;
	
	if ( !isset($EXP_ID) ) {
		echo 'ERROR: no experiment selected!<br>';
		return ;
	}
	
	$this->EXP_ID = $EXP_ID;
	
	// TBD !!!: add more $colname
	if ( !$colname ) $colname='V03'; 
	$this->resColName = $colname;
	$this->typex      = NULL;
	
	if ( $exp_res_type=="") {
		$exp_res_type='SPOT_RESULT';
		if ($_SESSION['userGlob']['f.spot_result.table']=='raw' ) $exp_res_type='EXP_RAW_RESULT';
	}
	
	$this->exp_res_type = $exp_res_type;
		
	$exp_type_nice = 'NORM';
	if ( $exp_res_type=='EXP_RAW_RESULT' ) {
		$exp_type_nice = 'RAW';
	}
	
	//if ( $exp_res_type == "EXP_RAW_RESULT" ) {
	$this->channel_num = $this->spotResLib->get_channel_num ( $DB_Handle, $EXP_ID );
	//}
	
	$this->defineQuery();
	$this->get_spot_results($DB_Handle1, $exp_res_type);
	$spot_res_num   = $this->typex[$exp_res_type]["spot_res_num"];
	
	$this->imageinfo = $this->getImageInfo( $DB_Handle );
	$abstractARR_ID = NULL;
	
	$SQL_query = 'SELECT concrete_array_id, h_exp_raw_desc_id, socket_id2 FROM exp WHERE exp_id = '.$EXP_ID;
	$DB_Handle->query($SQL_query);
	$DB_Handle->ReadRow();
	$CHIP_ID   	   = $DB_Handle->RowData[0];
	$H_EXP_DESC_ID = $DB_Handle->RowData[1];
	$SOCKET_ID 	   = $DB_Handle->RowData[2];
	
	if ( $CHIP_ID ) {
		$SQL_query = "select abstract_array_id from concrete_array where concrete_array_id=$CHIP_ID";
		$DB_Handle->query($SQL_query);
		$DB_Handle->ReadRow();
		$abstractARR_ID = $DB_Handle->RowData[0];	
	}
	
	elseif ( $SOCKET_ID ) {
 		$SQL_query = "select abstract_array_id from ABS_SOCKET where ABS_SOCKET_ID in (".
					" select ABS_SOCKET_ID from SOCKET where SOCKET_ID=".$SOCKET_ID.")";
		$DB_Handle->query($SQL_query);
		$DB_Handle->ReadRow();
	  	$abstractARR_ID = $DB_Handle->RowData[0];
  	}
	
	if ( !$abstractARR_ID ) {
		echo 'ERROR: No abstract array found for experiment [EXP_ID:'.$EXP_ID.'].<br>';
		return ;
	}
	/* 
	 else {
		echo 'ERROR: experiment contains no array.<br>';
		return ;
	}
	*/
  	
	$col_names_tmp1['SPOT_ID'] = 'spot_id';
	$col_names_tmp2 = $this->spotResLib->get_column_names(  $DB_Handle, $exp_res_type, $EXP_ID );
	if ( $exp_res_type == "EXP_RAW_RESULT" ) {
		$col_names_tmp2 = array( $this->resColName, $this->spotResLib->get_raw_nice( $DB_Handle, $EXP_ID, $this->resColName ) );
	}
	
	
	$EXP_NAME    	 = obj_nice_name( $DB_Handle, 'EXP', $EXP_ID);
	$H_EXP_DESC_name = obj_nice_name( $DB_Handle, 'H_EXP_RAW_DESC', $H_EXP_DESC_ID);
	
	$resarr = NULL;
	
	$resarr["col_names_tmp1"] = $col_names_tmp1;
	$resarr["col_names_tmp2"] = $col_names_tmp2;
	$resarr["abstractARR_ID"] = $abstractARR_ID;
	$resarr["EXP_NAME"] 	 = $EXP_NAME;
	$resarr["H_EXP_DESC_ID"] = $H_EXP_DESC_ID;
	$resarr["spot_res_num"]  = $spot_res_num;
	$resarr["H_EXP_DESC_name"]  = $H_EXP_DESC_name;
	$resarr["exp_type_nice"]  = $exp_type_nice;
	
	
	return ($resarr);
}


/**
 * print result table
 * Enter description here ...
 * @param unknown_type $DB_Handle
 * @param unknown_type $DB_Handle1
 * @param unknown_type $EXP_ID
 * @param unknown_type $extra_class_id
 * @param unknown_type $infoarr
 *   ["col_names_tmp2"]
	 ["application"]
	 "setHttpHead" [0] | 1 - must be set for DIRECT downloads
 * @param unknown_type $witharr
 * @param array $options
 *   'workDir'  - use this workdir
 *   'filename' - use this file name
 */
function resout( 
	&$DB_Handle, 
	&$DB_Handle1, 
	$EXP_ID, 
	$extra_class_id,
	&$infoarr,
	&$witharr,
	$options
	) {	

	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$this->downloadText = NULL;
	
	$col_names_tmp1 = $infoarr["col_names_tmp1"];
	$col_names_tmp2 = $infoarr["col_names_tmp2"];
	$abstractARR_ID = $infoarr["abstractARR_ID"];
	$EXP_NAME 		= $infoarr["EXP_NAME"] ;
	$application    = $infoarr["application"] ;
	$layout_arr = NULL;
	$h_poa_jobs = NULL;
	$attribs = NULL;
	
	$exportObj  = new oExpExportC();
	if ($this->pingout!=0) $exportObj->set_pingout($this->pingout);
	
	$col_names = $col_names_tmp2;                                                                  
	// all SPOTs need SPOT_ID, GROUPed results do not need SPOT_ID
	if ( !$witharr['subst_grouped'] ) $col_names = array_merge ($col_names_tmp1, $col_names_tmp2);
	
	$this_EXP_ID = 0;
	$filename = 'result.'.$EXP_NAME.'.txt';
	
	
	if ( $infoarr["setHttpHead"] ) {
		if(strcmp($application,'excel')==0) {
			$header_content = 'application/vnd.ms-excel';
			$filename = 'result.'.$EXP_NAME.'.xls';
		} elseif (strcmp($application,'csv')==0) {
			$header_content = 'text/plain';
			$filename = 'result.'.$EXP_NAME.'.txt';
		}    
		
	}
		
	$expOptions = array(
		'workDir' => $options[ 'workDir'],
	    'filename'=> $options[ 'filename'] 
	);
	$exportObj->startExport('o.EXP.oneout', $application, $filename, $expOptions);
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, 'startExport failed for EXP-ID:'.$EXP_ID );
		return;
	}
	##
	## Print table header
	##
		
	$witharr['extra_class_id'] 	= $extra_class_id;
	
	$witharr['varcols']		= 1;	
	$witharr['subst_id']	= 1;
	$witharr['source_pos']	= 0;
	$witharr['spot_coord']	= 0;	
	$witharr['h_poa_job']	= 0;
	$witharr['h_poa_job_id']= 0;
	
	/* selected by checkboxes*/
	#$witharr['alias_names']	 = 0;
	#$witharr['subst_with_notes']= 0;
	#$witharr['spot_names']		 = 0;
	#$witharr['class_info']		 = 0;
	
	$exp_arr['col_type'] 	= $col_names;
	$exp_arr['exp_res_type']= $this->exp_res_type;
	$exp_arr['exp_info']	= $EXP_NAME." ID:$EXP_ID";
	
	// $exportObj->setDefExpParams( array("absarrid" => $abstractARR_ID) ); // TBD:
	
	$retval = $exportObj->export_substs_header( $DB_Handle, $DB_Handle1, $abstractARR_ID, $witharr, $layout_arr, $h_poa_jobs, $attribs, $exp_arr );
	if ( $retval < 0 ){
		echo "WARNING: error occured at writing of HEADER ...\n";
		if ($error->printLast()) {
			echo "... continue ...\n";
		}     
	}
	
	$texttmp = "name: ".$this->imageinfo["name"];
	$exportObj->headerLineOut("image",$texttmp);
	if ($this->imageinfo["class"]!="") {
		$texttmp  = $this->imageinfo["class"];
		$texttmp .= " params: ".glob_array2String( $this->imageinfo["xvals"], 1 );
		$exportObj->headerLineOut("image-class",$texttmp);
	}
	
	
	if ( $this->exp_res_type == "EXP_RAW_RESULT" ) {
		$this->rawResultsOut( $exportObj, $DB_Handle, $infoarr, $witharr);
	} else {
		//output COLUMN NAMES !!!
		$extraCols=NULL;
		while ( list( $exp_col_type, $exp_col_nice_name) = each($col_names)) {
			$extraCols[] = $exp_col_nice_name;
		}
		reset ($col_names);
		if ($witharr['subst_grouped']) $extraCols[] = "No of spots";
		
		$exportObj->outSubstCols( $witharr, $attribs, $extraCols);
		
		## Print table body (one line for each spot):
			
		$sel[0]=$EXP_ID;
		$exportObj->export_substs( $DB_Handle, $DB_Handle1, $DB_Handle1, $abstractARR_ID, $witharr, $layout_arr, $h_poa_jobs, $attribs, $sel, $exp_arr );
	}
	$exportObj->finishExport();
	$this->downloadText = $exportObj->getDownloadText();
}

function showDownload()  {
	echo $this->downloadText;
}

/**
 * export EXP_RAW_RESULTs of one experiments
 */
function rawResultsOut( &$exportObj, &$sql, $infoarr, $witharr) {

	$id = $this->EXP_ID;
	
	// each spot !!!
	$abstractARR_ID = $infoarr["abstractARR_ID"];
	
	$resultarr = NULL;
	$spotarr   = NULL;
	
	$exportObj->_commentLine("Running number in the header-column identifies the CHANNEL_ID");
	$exportObj->_commentLine("exported result-data-column: ".
		$this->spotResLib->get_raw_nice( $sql, $id, $this->resColName ) );
	
		// print channel-header
	
	$tmparr=NULL;
	$tmparr[] = "SPOT_ID";
	$tmparr[] = "SUBST_NAME";
	
	$channel_id = 1;
	while ( $channel_id <= $this->channel_num ) {
			$tmparr[] = $channel_id;
			$channel_id++;
	}
	$exportObj->dataRowOut($tmparr);
	
	if ($witharr['subst_grouped']) {
		 $this->rawResultsMeanOut( $exportObj, $sql, $infoarr, $witharr);
		 return;
	}
	
	//
	// get SPOT_IDs and subst names
	//
	$sqls = "select a.SPOT_ID, s.name from A_PROBE_ON_ARRAY a, ABSTRACT_SUBST s where a.abstract_array_id=".$abstractARR_ID. 
			" AND a.ABSTRACT_SUBST_ID=s.ABSTRACT_SUBST_ID order by a.spot_id";
	$sql->query($sqls);
	$sql->query($sqls);
	while ( $sql->ReadRow() ) {
		$spotid     = $sql->RowData[0];
		$subst_name = $sql->RowData[1];
		$spotarr[$spotid] = $subst_name;
	}

	// get result values
	while ( list($spotid, $substname) = each($spotarr) ) {
		$tmparr=NULL;
		$tmparr[]= $spotid;
		$tmparr[]= $substname;
	
		$add_query = "EXP_RAW_RESULT e WHERE e.EXP_ID = ".$id. " AND e.SPOT_ID=".$spotid;
		$sql->query( "SELECT e.channel_id, e.".$this->resColName." FROM ".$add_query.
			" order by e.channel_id" );
		
		$resPerSpot = NULL;
		
		while ($sql->ReadRow()) {
	
			$channel_id = $sql->RowData[0];
			$value 		= $sql->RowData[1];
			$resPerSpot[$channel_id] = $value;
	
		}
		$channel_id = 1;
		while ( $channel_id <= $this->channel_num ) {
			$tmparr[]= $resPerSpot[$channel_id];
			$channel_id++;
		}
		$exportObj->dataRowOut($tmparr);
		
	}

}

/**
 * export MEAN RAW_RESULTs, mean over same probes
 */
function rawResultsMeanOut( &$exportObj, &$sql, $infoarr, $witharr) {

	// mean results ...
	
	$abstractARR_ID = $infoarr["abstractARR_ID"];
	$EXP_ID = $this->EXP_ID;
	
	//
	// get SPOT_IDs and subst names
	//
	$sqls = "select s.ABSTRACT_SUBST_ID, s.name from A_PROBE_ON_ARRAY a, ABSTRACT_SUBST s where a.abstract_array_id=".$abstractARR_ID. 
			" AND a.ABSTRACT_SUBST_ID=s.ABSTRACT_SUBST_ID order by s.NAME";
	$sql->query($sqls);
	$sql->query($sqls);
	while ( $sql->ReadRow() ) {
		$subst_id   = $sql->RowData[0];
		$subst_name = $sql->RowData[1];
		$spotarr[$subst_id] = $subst_name;
	}

	// get result values
	while ( list($subst_id, $substname) = each($spotarr) ) {
	
		$tmparr=NULL;
		$tmparr[]= $spotid;
		$tmparr[]= $substname;
	
		$channel_id_old=0;
		$add_query = "EXP_RAW_SPOT_RESULT e WHERE e.EXP_ID = ".$EXP_ID. " AND e.ABSTRACT_SUBST_ID=".$subst_id;
		$sql->query( "SELECT e.channel_id, e.".$this->resColName." FROM ".$add_query.
			" order by e.channel_id" );
		
		$resPerSpot = NULL;
		$substsum   = 0;
		$spotcnt    = 0;
		
		while ($sql->ReadRow()) {
			
			$channel_id = $sql->RowData[0];
			$value 		= $sql->RowData[1];
			
			if ($channel_id_old != $channel_id) {
				 
				if ($spotcnt>0) {
					$meanval = $substsum / $spotcnt;
					$resPerSpot[$channel_id_old] = $meanval;
				}
				$substsum   = 0;
				$spotcnt    = 0;
			}	
				
			$substsum += $value;
			$spotcnt++;
			
			$channel_id_old = $channel_id;
	
		}
		
		// finish ...
		if ($spotcnt>0) {
			$meanval = $substsum / $spotcnt;
			$resPerSpot[$channel_id] = $meanval;
		}
		
		$channel_id = 1;
		while ( $channel_id <= $this->channel_num ) {
			$tmparr[]= $resPerSpot[$channel_id];
			$channel_id++;
		}
		$exportObj->dataRowOut($tmparr);
		
	}

}

}