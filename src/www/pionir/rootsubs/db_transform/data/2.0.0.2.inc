<?php
/*
 *  initial Blink changes
 *  @swreq 
 *  @version $Header:   2018-11-21 09:04:09Z $
 */
class gDB_tranData_2_0_0_2 extends gDB_transAbstract {

	
	function getBasicInfo() {
		$dataArray["info"] = "Exp improvement";
		$dataArray["reqs"] = array(
				"DbVersionNeed" => '2.0.0.1', 
				"DbVersionNew"  => "2.0.0.2 - 20190410 - 09:25");
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
				" EXT_ID VARCHAR(255) \n".
				")";
		$insarr[]="comment on column EXP.EXT_ID is  'external ID of experiment'";
		$meta[] = array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'EXT_ID', 'NICE_NAME'=>'external-ID', 'CCT_TABLE_NAME'=>'', 'APP_DATA_TYPE_ID'=>6, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>11.8, 'TABLE_NAME'=>'EXP', 'VISIBLE'=>2, 'EDITABLE'=>0) );

		// <table> ABSTRACT_PROTO
		$insarr[]="alter table ABSTRACT_PROTO add ( \n".
		  		" EXT_ID VARCHAR(255) \n".
		  		")";
		$insarr[]="comment on column ABSTRACT_PROTO.EXT_ID is  'external ID of assay'";
		$meta[] = array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'EXT_ID', 'NICE_NAME'=>'external-ID', 'CCT_TABLE_NAME'=>'', 'APP_DATA_TYPE_ID'=>6, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>6.5, 'TABLE_NAME'=>'ABSTRACT_PROTO', 'VISIBLE'=>2, 'EDITABLE'=>0) );
		
		
		// <table> SATTACH
		$insarr[]="alter table SATTACH add ( \n".
		  		" HASHX VARCHAR(255) \n".
		  		")";
		$insarr[]="comment on column SATTACH.HASHX is  'SHA1-Hash of document'";
		$meta[] = array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'HASHX', 'NICE_NAME'=>'Hash', 'CCT_TABLE_NAME'=>'', 'APP_DATA_TYPE_ID'=>6, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>20, 'TABLE_NAME'=>'SATTACH', 'VISIBLE'=>2, 'EDITABLE'=>0) );
		
		// <table> SUA_BOM
		$insarr[]="alter table SUA_BOM add ( \n".
		  		" DATAX VARCHAR(4000) \n".
		  		")";
		$insarr[]="comment on column SUA_BOM.datax is  'data field (JSON-format), e.g. for ASR'";
		$meta[] = array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'DATAX', 'NICE_NAME'=>'ext data', 'CCT_TABLE_NAME'=>'', 'APP_DATA_TYPE_ID'=>6, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>6, 'TABLE_NAME'=>'SUA_BOM', 'VISIBLE'=>2, 'EDITABLE'=>0) );
		
		
		// <table> CCT_NOTES
		$insarr[]="create table CCT_NOTES ( \n".
		  		" CCT_ACCESS_ID NUMBER not null,\n".
		  		" POS NUMBER not null,\n".
		  		" MOD_DATE DATE not null,\n".
		  		" NAME VARCHAR(255) not null,\n".
		  		" DB_USER_ID NUMBER not null,\n".
		  		" NOTES VARCHAR(4000),\n".
		  		" constraint PK_CCT_NOTES primary key (CCT_ACCESS_ID, POS) \n".
		  		")";
		$insarr[]="comment on table CCT_NOTES is 'note entry for an object'";
		$insarr[]="comment on column CCT_NOTES.CCT_ACCESS_ID is 'id of note entry'";
		$insarr[]="comment on column CCT_NOTES.POS is 'pos of entry'";
		$insarr[]="comment on column CCT_NOTES.MOD_DATE is 'entry date'";
		$insarr[]="comment on column CCT_NOTES.NAME is 'name of comment'";
		$insarr[]="comment on column CCT_NOTES.DB_USER_ID is 'editor'";
		$insarr[]="comment on column CCT_NOTES.NOTES is 'notes'";
		$insarr[]="/* indices */";
		$insarr[]="create index CCT_NOTES_AK2 on CCT_NOTES (NAME ASC)";
		$insarr[]="/* constraints */";
		$insarr[]="alter table CCT_NOTES \n".
		  		" add constraint FK_CCT_ACCESS2CCT_NOTES foreign key (CCT_ACCESS_ID) \n".
		  		" references CCT_ACCESS (CCT_ACCESS_ID)\n".
		  		" on delete cascade";
		$insarr[]="alter table CCT_NOTES \n".
		  		" add constraint FK_DB_USER2CCT_NOTES foreign key (DB_USER_ID) \n".
		  		" references DB_USER (DB_USER_ID)";
		
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_TABLE', 'vals'=>array('TABLE_NAME'=>'CCT_NOTES', 'NICE_NAME'=>'note entry', 'CCT_TABLE_NAME'=>'CCT_ACCESS', 'TABLE_TYPE'=>'SYS', 'IS_VIEW'=>0, 'INTERNAL'=>1, 'EXIM'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'CCT_ACCESS_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'CCT_ACCESS', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>1, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>1, 'TABLE_NAME'=>'CCT_NOTES', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'POS', 'NICE_NAME'=>'pos', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>2, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>2, 'TABLE_NAME'=>'CCT_NOTES', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'MOD_DATE', 'NICE_NAME'=>'entry date', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>12, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>3, 'TABLE_NAME'=>'CCT_NOTES', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'NAME', 'NICE_NAME'=>'name', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>1, 'NOT_NULL'=>1, 'POS'=>4, 'TABLE_NAME'=>'CCT_NOTES', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'DB_USER_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'DB_USER', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>5, 'TABLE_NAME'=>'CCT_NOTES', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'NOTES', 'NICE_NAME'=>'notes', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>3, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>6, 'TABLE_NAME'=>'CCT_NOTES', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		
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
