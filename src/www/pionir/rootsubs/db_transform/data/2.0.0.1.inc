<?php
/*
 *  initial Blink changes
 *  @swreq 
 *  @version $Header:   2018-11-21 09:04:09Z $
 */
class gDB_tranData_2_0_0_1 extends gDB_transAbstract {

	
	function getBasicInfo() {
		$dataArray["info"] = "Exp improvement";
		$dataArray["reqs"] = array(
				"DbVersionNeed" => '2.0.0.0', 
				"DbVersionNew"  => "2.0.0.1 - 20190125 - 09:25");
		// $dataArray['extconvert'] = array( 'parts'=> 2, 'transform'=>1 ) ;
		$this->dataArray = $dataArray;
		return $dataArray;
	}
	
	
	
	function getData( &$dataArray, $option=NULL ) {
		$go = $option['go'];
		$dataArray = $this->dataArray;
				
		
		$insarr	= NULL;
		$meta	= NULL;


		
			
		// <TABLE> EXP
		
		$insarr[]="alter table EXP add ( \n".
				" DATAURL VARCHAR(255), \n".
				" DEV_ID  NUMBER \n".
				")";
		
		$insarr[]="comment on column EXP.DATAURL is  'URL of external data directory'";
		$insarr[]="comment on column EXP.DEV_ID is  'id of analysis equipment'";
		
		$insarr[]="alter table EXP \n".
				" add constraint FK_CHIP_READER2EXP foreign key (DEV_ID) \n".
				" references CHIP_READER (CHIP_READER_ID)";
		
		// EXP
		$meta[] = array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'DATAURL', 'NICE_NAME'=>'data-url', 'CCT_TABLE_NAME'=>'', 'APP_DATA_TYPE_ID'=>6, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>11.6, 'TABLE_NAME'=>'EXP', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[] = array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'DEV_ID', 'NICE_NAME'=>'', 'CCT_TABLE_NAME'=>'CHIP_READER', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>11.7, 'TABLE_NAME'=>'EXP', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		

		$meta[] = array( "act"=>"upd", "tab"=>"GLOBALS", "vals"=>array("VALUE"=>$dataArray["reqs"]["DbVersionNew"]), "ids"=>array("NAME"=>"DBVersion" ) );
		
		
		$dataArray["sql"]  = &$insarr;
		$dataArray["meta"] = &$meta;
	}
	
	/**
	 * do data transform ...
	 * @param int $go 0,1
	 */
	function transform(&$sqlo, &$sqlo2, $partno, $go) {
		global $error;
		$FUNCNAME= $this->CLASSNAME.':transform';
	
		
	}
}
