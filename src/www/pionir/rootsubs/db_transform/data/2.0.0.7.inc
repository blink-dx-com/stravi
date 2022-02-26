<?php
require_once 'insert.inc';
require_once ('f.update.inc');
require_once 'o.CCT_TABLE.info.inc';
/*
 *  Blink changes
 */
class gDB_tranData_2_0_0_7 extends gDB_transAbstract {

	
	function getBasicInfo() {
	    $dataArray=array();
		$dataArray["info"] = "mod: SATTACH";
		$dataArray["reqs"] = array(
				"DbVersionNeed" => '2.0.0.6', 
				"DbVersionNew"  => "2.0.0.7 - 20211220 - 09:00");
		
		$this->dataArray = $dataArray;
		return $dataArray;
	}

	function getData( &$dataArray, $option=NULL ) {
		// $go = $option['go'];
		$dataArray = $this->dataArray;
				
		
		$insarr	= NULL;
		$meta	= NULL;
		
		$insarr[]="/* <TABLE> SATTACH: MOD 2021-10-26 */";
		
		$insarr[]="ALTER TABLE SATTACH add SUB_POS_STR VARCHAR(255)";
		$insarr[]="comment on column SATTACH.SUB_POS_STR is 
            'optional POS string for e.g REA_LOG: (X,att_no), where: X: REA_LOG.POS, att_no: index of attachement on POS'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'SATTACH', 'COLUMN_NAME'=>'SUB_POS_STR', 'NICE_NAME'=>'sub pos info', 'CCT_TABLE_NAME'=>NULL,
		    'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>21, 'VISIBLE'=>2, 'EDITABLE'=>0) );
		
		$insarr[]="ALTER TABLE CONCRETE_SUBST add DEVIATION VARCHAR(4000)";
		$insarr[]="comment on column CONCRETE_SUBST.DEVIATION is
            'Deviation or problem of this material batch (caused by production, transport or other)'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'CONCRETE_SUBST', 'COLUMN_NAME'=>'DEVIATION', 'NICE_NAME'=>'deviation', 'CCT_TABLE_NAME'=>NULL,
		    'APP_DATA_TYPE_ID'=>3, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>25, 'VISIBLE'=>2, 'EDITABLE'=>1) );
		
		$insarr[]="ALTER TABLE MORDER add DEVIATION VARCHAR(4000)";
		$insarr[]="comment on column MORDER.DEVIATION is
            'Deviation or problem during production'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'MORDER', 'COLUMN_NAME'=>'DEVIATION', 'NICE_NAME'=>'deviation', 'CCT_TABLE_NAME'=>NULL,
		    'APP_DATA_TYPE_ID'=>3, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>20, 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		
		$meta[] = array( "act"=>"upd", "tab"=>"GLOBALS", "vals"=>array("VALUE"=>$dataArray["reqs"]["DbVersionNew"]), "ids"=>array("NAME"=>"DBVersion" ) );
		
		
		$dataArray["sql"]  = &$insarr;
		$dataArray["meta"] = &$meta;
	}
	
	

}
