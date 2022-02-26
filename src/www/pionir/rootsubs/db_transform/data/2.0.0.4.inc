<?php
/*
 *  Blink changes
 */
class gDB_tranData_2_0_0_4 extends gDB_transAbstract {

	
	function getBasicInfo() {
	    $dataArray=array();
		$dataArray["info"] = "Proj improvement + CONTAINER news + AS_ASR, ABSTRACT_SUBST";
		$dataArray["reqs"] = array(
				"DbVersionNeed" => '2.0.0.3', 
				"DbVersionNew"  => "2.0.0.4 - 20210301 - 10:15");
		
		$this->dataArray = $dataArray;
		return $dataArray;
	}
	
	
	
	function getData( &$dataArray, $option=NULL ) {
		// $go = $option['go'];
		$dataArray = $this->dataArray;
				
		
		$insarr	= NULL;
		$meta	= NULL;
		$insarr[]="/* <TABLE> EXP_H_IMG : NEW 2021-02-01 */";
			
		$insarr[]="create table EXP_H_IMG ( \n".
		  		" EXP_ID BIGINT not null,\n".
		  		" POS INT not null,\n".
		  		" IMG_ID BIGINT,\n".
		  		" URL VARCHAR(2000),\n".
		  		" constraint PK_EXP_H_IMG primary key (EXP_ID, POS) \n".
		  		")";
		$insarr[]="comment on table EXP_H_IMG is E'experiment has image'";
		$insarr[]="comment on column EXP_H_IMG.EXP_ID is E'id of experiment has image'";
		$insarr[]="comment on column EXP_H_IMG.POS is E'pos of image'";
		$insarr[]="comment on column EXP_H_IMG.IMG_ID is E'id of image'";
		$insarr[]="comment on column EXP_H_IMG.URL is E'image-url'";
		$insarr[]="/* indices */";
		$insarr[]="/* constraints */";
		$insarr[]="alter table EXP_H_IMG \n".
		  		" add constraint FK_EXP2EXP_H_IMG foreign key (EXP_ID) \n".
		  		" references EXP (EXP_ID)\n".
		  		" on delete cascade";
		$insarr[]="alter table EXP_H_IMG \n".
		  		" add constraint FK_IMG2EXP_H_IMG foreign key (IMG_ID) \n".
		  		" references IMG (IMG_ID)";
		
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_TABLE', 'vals'=>array('TABLE_NAME'=>'EXP_H_IMG', 'NICE_NAME'=>'experiment has image', 'CCT_TABLE_NAME'=>'EXP', 'TABLE_TYPE'=>'BO_ASSOC', 'IS_VIEW'=>0, 'INTERNAL'=>0, 'EXIM'=>0) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'EXP_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'EXP', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>1, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>1, 'TABLE_NAME'=>'EXP_H_IMG', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'POS',    'NICE_NAME'=>'pos', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>15, 'PRIMARY_KEY'=>2, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>2, 'TABLE_NAME'=>'EXP_H_IMG', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'IMG_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'IMG', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>3, 'TABLE_NAME'=>'EXP_H_IMG', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'URL',    'NICE_NAME'=>'image-url', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>4, 'TABLE_NAME'=>'EXP_H_IMG', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		
		$insarr[]="/* <TABLE> AS_ASR_H_CSP : NEW 2021-02-04 */";
		
		$insarr[]="create table AS_ASR_H_CSP ( \n".
		  		" AS_ASR_ID BIGINT not null,\n".
		  		" BSID SMALLINT,\n".
		  		" DATAX VARCHAR(4000),\n".
		  		" LIB_REAG_ID BIGINT,\n".
		  		" BEAD_REAG_ID BIGINT,\n".
		  		" constraint PK_AS_ASR_H_CSP primary key (AS_ASR_ID, BSID) \n".
		  		")";
		$insarr[]="comment on table AS_ASR_H_CSP is 'ASR lib has concrete species'";
		$insarr[]="comment on column AS_ASR_H_CSP.AS_ASR_ID is 'id of ASR lib has species'";
		$insarr[]="comment on column AS_ASR_H_CSP.BSID is 'Species ID'";
		$insarr[]="comment on column AS_ASR_H_CSP.DATAX is 'JSON string of definition data'";
		$insarr[]="comment on column AS_ASR_H_CSP.LIB_REAG_ID is 'link to library reagent (concrete)'";
		$insarr[]="comment on column AS_ASR_H_CSP.BEAD_REAG_ID is 'link to bead reagent (concrete)'";
		$insarr[]="/* indices */";
		$insarr[]="create index AS_ASR_H_CSP_AK2 on AS_ASR_H_CSP (BSID ASC)";
		$insarr[]="/* constraints */";
		$insarr[]="alter table AS_ASR_H_CSP \n".
		  		" add constraint FK_AS_ASR2AS_ASR_H_CSP foreign key (AS_ASR_ID) \n".
		  		" references AS_ASR (AS_ASR_ID)\n".
		  		" on delete cascade";
		$insarr[]="alter table AS_ASR_H_CSP \n".
		  		" add constraint FK_CONCRETE_SUBST2AS_ASR_H_CSP foreign key (LIB_REAG_ID) \n".
		  		" references CONCRETE_SUBST (CONCRETE_SUBST_ID)";
		$insarr[]="alter table AS_ASR_H_CSP \n".
		  		" add constraint FK_CONCRETE_SUB2_2AS_ASR_H_CSP foreign key (BEAD_REAG_ID) \n".
		  		" references CONCRETE_SUBST (CONCRETE_SUBST_ID)";
		
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_TABLE', 'vals'=>array('TABLE_NAME'=>'AS_ASR_H_CSP', 'NICE_NAME'=>'ASR lib has concrete species', 'CCT_TABLE_NAME'=>'AS_ASR', 'TABLE_TYPE'=>'BO_ASSOC', 'IS_VIEW'=>0, 'INTERNAL'=>0, 'EXIM'=>0) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'AS_ASR_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'AS_ASR', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>1, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>1, 'TABLE_NAME'=>'AS_ASR_H_CSP', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'BSID', 'NICE_NAME'=>'Species ID', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>2, 'MOST_IMP_COL'=>1, 'NOT_NULL'=>0, 'POS'=>2, 'TABLE_NAME'=>'AS_ASR_H_CSP', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'DATAX', 'NICE_NAME'=>'channel data', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>6, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>3, 'TABLE_NAME'=>'AS_ASR_H_CSP', 'VISIBLE'=>1, 'EDITABLE'=>0) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'LIB_REAG_ID', 'NICE_NAME'=>'library reagent', 'CCT_TABLE_NAME'=>'CONCRETE_SUBST', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>4, 'TABLE_NAME'=>'AS_ASR_H_CSP', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'BEAD_REAG_ID', 'NICE_NAME'=>'bead reagent', 'CCT_TABLE_NAME'=>'CONCRETE_SUBST', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>5, 'TABLE_NAME'=>'AS_ASR_H_CSP', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		
		$insarr[]="/* <TABLE> EXP, EXP_TMPL :  2021-02-07 */";
		
		$insarr[]="ALTER TABLE EXP add AS_ASR_ID  BIGINT";
		$insarr[]="comment on column EXP.AS_ASR_ID is 'ID of AS_ASR'";
		$insarr[]="alter table EXP \n".
		  		" add constraint FK_AS_ASR2EXP foreign key (AS_ASR_ID) \n".
		  		" references AS_ASR (AS_ASR_ID)";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array(
		    'TABLE_NAME'=>'EXP',  'COLUMN_NAME'=>'AS_ASR_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'AS_ASR', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>15.5, 'VISIBLE'=>2, 'EDITABLE'=>1) );
		
		
		$insarr[]="ALTER TABLE EXP_TMPL add AS_ASR_ID  BIGINT";
		$insarr[]="comment on column EXP_TMPL.AS_ASR_ID is 'ID of AS_ASR'";
		$insarr[]="alter table EXP_TMPL \n".
		  		" add constraint FK_AS_ASR2EXP_TMPL foreign key (AS_ASR_ID) \n".
		  		" references AS_ASR (AS_ASR_ID)";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array(
		    'TABLE_NAME'=>'EXP_TMPL',  'COLUMN_NAME'=>'AS_ASR_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'AS_ASR', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>15.5, 'VISIBLE'=>1, 'EDITABLE'=>1) );

		// IMG_RESULT, IS_QC
		$insarr[]="ALTER TABLE EXP DROP COLUMN IF EXISTS IMG_RESULT";
		$insarr[]="ALTER TABLE EXP DROP COLUMN IF EXISTS IS_QC";
		$meta[]=array( 'act'=>'del', 'tab'=>'CCT_COLUMN', 'ids'=>array('TABLE_NAME'=>'EXP',  'COLUMN_NAME'=>'IMG_RESULT'));
		$meta[]=array( 'act'=>'del', 'tab'=>'CCT_COLUMN', 'ids'=>array('TABLE_NAME'=>'EXP',  'COLUMN_NAME'=>'IS_QC'));
		
		
		$insarr[]="/* <TABLE> H_PROTO_KIND, EXP:  2021-02-12 */";
		
		$insarr[]="ALTER TABLE H_PROTO_KIND add KEYX  VARCHAR(255)";
		$insarr[]="comment on column H_PROTO_KIND.KEYX is 'key for protocol kind'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array(
		    'TABLE_NAME'=>'H_PROTO_KIND',  'COLUMN_NAME'=>'KEYX', 'NICE_NAME'=>'key', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>2, 
		    'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>2.5, 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		$insarr[]="ALTER TABLE EXP add MO_EXP_ID  BIGINT";
		$insarr[]="ALTER TABLE EXP add MO_POS  INT";
		$insarr[]="comment on column EXP.MO_EXP_ID is 'id of mother experiment'";
		$insarr[]="comment on column EXP.MO_POS is 'position in mother experiment'";
		$insarr[]="alter table EXP \n".
		  		" add constraint FK_EXP2EXP foreign key (MO_EXP_ID) \n".
		  		" references EXP (EXP_ID)";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'EXP', 'COLUMN_NAME'=>'MO_EXP_ID', 'NICE_NAME'=>'Mother experiment', 'CCT_TABLE_NAME'=>'EXP', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>18, 'VISIBLE'=>2, 'EDITABLE'=>0) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'EXP', 'COLUMN_NAME'=>'MO_POS', 'NICE_NAME'=>'Mother exp pos', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>19, 'VISIBLE'=>2, 'EDITABLE'=>0) );
		
		$insarr[]="/* <TABLE> MORDER  2021-02-19 */";
		$insarr[]="ALTER TABLE MORDER add INFO_USER_ID  BIGINT";
		
		$insarr[]="comment on column MORDER.INFO_USER_ID is 'id of info user'";
		
		$insarr[]="alter table MORDER \n".
		  		" add constraint FK_DB_USER2MORDER foreign key (INFO_USER_ID) \n".
		  		" references DB_USER (DB_USER_ID)";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'MORDER', 'COLUMN_NAME'=>'INFO_USER_ID', 'NICE_NAME'=>'Info user', 'CCT_TABLE_NAME'=>'DB_USER', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>14, 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'MORDER', 'COLUMN_NAME'=>'CERT_FLAG',  'POS'=>16) );
		
		$insarr[]="/* <TABLE> EXP, EXP_TMPL:  2021-02-20 */";
		$insarr[]="ALTER TABLE EXP      add DEV_ABS_PROTO_ID  BIGINT";
		$insarr[]="ALTER TABLE EXP_TMPL add DEV_ABS_PROTO_ID  BIGINT";
		
		$insarr[]="comment on column EXP.DEV_ABS_PROTO_ID      is 'id of protocol (abstract) used for device script'";
		$insarr[]="comment on column EXP_TMPL.DEV_ABS_PROTO_ID is 'id of protocol (abstract) used for device script'";
		
		$insarr[]="alter table EXP \n".
		  		" add constraint FK_ABSTRACT_PROTO2EXP foreign key (DEV_ABS_PROTO_ID) \n".
		  		" references ABSTRACT_PROTO (ABSTRACT_PROTO_ID)";
		$insarr[]="alter table EXP_TMPL \n".
		  		" add constraint FK_ABSTRACT_PROTO2EXP_TMPL foreign key (DEV_ABS_PROTO_ID) \n".
		  		" references ABSTRACT_PROTO (ABSTRACT_PROTO_ID)";
		
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'EXP', 'COLUMN_NAME'=>'DEV_ABS_PROTO_ID', 'NICE_NAME'=>'device script protocol', 'CCT_TABLE_NAME'=>'ABSTRACT_PROTO', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>18,      'VISIBLE'=>2, 'EDITABLE'=>0) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'EXP_TMPL', 'COLUMN_NAME'=>'DEV_ABS_PROTO_ID', 'NICE_NAME'=>'device script protocol', 'CCT_TABLE_NAME'=>'ABSTRACT_PROTO', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>16, 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'EXP_TMPL', 'COLUMN_NAME'=>'NOTES',  'POS'=>100) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'EXP',      'COLUMN_NAME'=>'NOTES',  'POS'=>100) );
		
		
		$insarr[]="/* <TABLE> ABSTRACT_SUBST, CONCRETE_SUBST:  2021-02-27 */";
		$insarr[]="delete from CCT_COLUMN where TABLE_NAME='SAMPLE_IN_CONCRETE_SUBST'";
		$insarr[]="delete from CCT_TABLE  where TABLE_NAME='SAMPLE_IN_CONCRETE_SUBST'";
		$insarr[]="DROP view IF EXISTS  SAMPLE_IN_CONCRETE_SUBST";
		$insarr[]="ALTER TABLE CONCRETE_SUBST  drop column CONCRETE_PROTO_ID";
		$insarr[]="ALTER TABLE CONCRETE_SUBST  drop column QC_CPROTO_ID";
		$insarr[]="ALTER TABLE ABSTRACT_SUBST  drop column ABSTRACT_PROTO_ID";
		$insarr[]="ALTER TABLE ABSTRACT_SUBST  drop column QC_ABSPROTO_ID";
		
		$meta[]=array( 'act'=>'del', 'tab'=>'CCT_COLUMN', 'ids'=>array( 'TABLE_NAME'=>'CONCRETE_SUBST', 'COLUMN_NAME'=>'CONCRETE_PROTO_ID') );
		$meta[]=array( 'act'=>'del', 'tab'=>'CCT_COLUMN', 'ids'=>array( 'TABLE_NAME'=>'CONCRETE_SUBST', 'COLUMN_NAME'=>'QC_CPROTO_ID') );
		$meta[]=array( 'act'=>'del', 'tab'=>'CCT_COLUMN', 'ids'=>array( 'TABLE_NAME'=>'ABSTRACT_SUBST', 'COLUMN_NAME'=>'ABSTRACT_PROTO_ID') );
		$meta[]=array( 'act'=>'del', 'tab'=>'CCT_COLUMN', 'ids'=>array( 'TABLE_NAME'=>'ABSTRACT_SUBST', 'COLUMN_NAME'=>'QC_ABSPROTO_ID') );
		
		
		// 2021-03-01
		$insarr[]="ALTER TABLE ACCEPT_PROT add CODE_LOC VARCHAR(255)";
		$insarr[]="comment on column ACCEPT_PROT.CODE_LOC is 'Code location of validation code'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'ACCEPT_PROT', 'COLUMN_NAME'=>'CODE_LOC', 'NICE_NAME'=>'code location', 'CCT_TABLE_NAME'=>NULL, 
		    'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>5.7, 'VISIBLE'=>2, 'EDITABLE'=>0) );
		
		
		// -------------------
		$meta[] = array( "act"=>"upd", "tab"=>"GLOBALS", "vals"=>array("VALUE"=>$dataArray["reqs"]["DbVersionNew"]), "ids"=>array("NAME"=>"DBVersion" ) );
		
		
		$dataArray["sql"]  = &$insarr;
		$dataArray["meta"] = &$meta;
	}
	

}
