<?php
/*
 *  Blink changes
 */
class gDB_tranData_2_0_0_3 extends gDB_transAbstract {

	
	function getBasicInfo() {
	    $dataArray=array();
		$dataArray["info"] = "Proj improvement + CONTAINER news + AS_ASR, ABSTRACT_SUBST";
		$dataArray["reqs"] = array(
				"DbVersionNeed" => '2.0.0.2', 
				"DbVersionNew"  => "2.0.0.3 - 20210130 - 11:15");
		// $dataArray['extconvert'] = array( 'parts'=> 2, 'transform'=>1 ) ;
		$this->dataArray = $dataArray;
		return $dataArray;
	}
	
	
	
	function getData( &$dataArray, $option=NULL ) {
		$go = $option['go'];
		$dataArray = $this->dataArray;
				
		
		$insarr	= NULL;
		$meta	= NULL;

			
		// <TABLE> PROJ
		$insarr[]="alter table PROJ add ( \n".
				" TYPEX NUMBER \n".
				")";
		$insarr[]="comment on column PROJ.TYPEX is  'special type of project'";
		$meta[] = array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'TYPEX', 'NICE_NAME'=>'type', 'CCT_TABLE_NAME'=>'', 'APP_DATA_TYPE_ID'=>8, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>7, 'TABLE_NAME'=>'PROJ', 'VISIBLE'=>2, 'EDITABLE'=>0) );

		// <TABLE> CONT_HAS_ASUBST
		
		$insarr[]="create table CONT_HAS_ASUBST ( \n".
		  		" CONTAINER_ID NUMBER not null,\n".
		  		" ABSTRACT_SUBST_ID NUMBER not null,\n".
		  		" POS NUMBER,\n".
		  		" KEY VARCHAR(255),\n".
		  		" constraint PK_CONT_HAS_ASUBST primary key (CONTAINER_ID, ABSTRACT_SUBST_ID) \n".
		  		")";
		$insarr[]="comment on table CONT_HAS_ASUBST is 'storage has material templates'";
		$insarr[]="comment on column CONT_HAS_ASUBST.CONTAINER_ID is 'id of storage has MAA'";
		$insarr[]="comment on column CONT_HAS_ASUBST.ABSTRACT_SUBST_ID is 'id of material (template)'";
		$insarr[]="comment on column CONT_HAS_ASUBST.POS is 'optional position in storage'";
		$insarr[]="comment on column CONT_HAS_ASUBST.KEY is 'optional key'";
		$insarr[]="/* indices */";
		$insarr[]="create index CONT_HAS_ASUBST_AK2 on CONT_HAS_ASUBST (ABSTRACT_SUBST_ID ASC)";
		$insarr[]="/* constraints */";
		$insarr[]="alter table CONT_HAS_ASUBST \n".
		  		" add constraint FK_CONTAINER2CONT_HAS_ASUBST foreign key (CONTAINER_ID) \n".
		  		" references CONTAINER (CONTAINER_ID)\n".
		  		" on delete cascade";
		$insarr[]="alter table CONT_HAS_ASUBST \n".
		  		" add constraint FK_ABSTRACT_SU2CONT_HAS_ASUBST foreign key (ABSTRACT_SUBST_ID) \n".
		  		" references ABSTRACT_SUBST (ABSTRACT_SUBST_ID)";
		

		
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_TABLE', 'vals'=>array('TABLE_NAME'=>'CONT_HAS_ASUBST', 'NICE_NAME'=>'storage has MAA', 'CCT_TABLE_NAME'=>'CONTAINER', 'TABLE_TYPE'=>'BO_ASSOC', 'IS_VIEW'=>0, 'INTERNAL'=>0, 'EXIM'=>0) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'CONTAINER_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'CONTAINER', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>1, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>1, 'TABLE_NAME'=>'CONT_HAS_ASUBST', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'ABSTRACT_SUBST_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'ABSTRACT_SUBST', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>2, 'MOST_IMP_COL'=>1, 'NOT_NULL'=>1, 'POS'=>2, 'TABLE_NAME'=>'CONT_HAS_ASUBST', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'POS', 'NICE_NAME'=>'position', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>15, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>3, 'TABLE_NAME'=>'CONT_HAS_ASUBST', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'KEY', 'NICE_NAME'=>'key', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>4, 'TABLE_NAME'=>'CONT_HAS_ASUBST', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		// <TABLE> AS_ASR ---
		
		$insarr[]="create table AS_ASR ( \n".
		  		" AS_ASR_ID NUMBER not null,\n".
		  		" NAME VARCHAR(255),\n".
		  		" CCT_ACCESS_ID NUMBER not null,\n".
		  		" EXTRA_OBJ_ID NUMBER,\n".
		  		" DATAX VARCHAR(4000),\n".
		  		" NOTES VARCHAR(4000),\n".
		  		" constraint PK_AS_ASR primary key (AS_ASR_ID) \n".
		  		")";
		$insarr[]="comment on table AS_ASR is 'describes the ASR Bead lib'";
		$insarr[]="comment on column AS_ASR.AS_ASR_ID is 'id of ASR lib'";
		$insarr[]="comment on column AS_ASR.NAME is 'name of the ASR lib'";
		$insarr[]="comment on column AS_ASR.CCT_ACCESS_ID is 'id of the access rights'";
		$insarr[]="comment on column AS_ASR.EXTRA_OBJ_ID is 'id of the extra obj'";
		$insarr[]="comment on column AS_ASR.DATAX is 'internal data'";
		$insarr[]="comment on column AS_ASR.NOTES is 'notes'";
		$insarr[]="/* indices */";
		$insarr[]="create index AS_ASR_AK2 on AS_ASR (NAME ASC)";
		$insarr[]="/* constraints */";
		$insarr[]="alter table AS_ASR \n".
		  		" add constraint FK_CCT_ACCESS2AS_ASR foreign key (CCT_ACCESS_ID) \n".
		  		" references CCT_ACCESS (CCT_ACCESS_ID)\n".
		  		" on delete cascade";
		$insarr[]="alter table AS_ASR \n".
		  		" add constraint FK_EXTRA_OBJ2AS_ASR foreign key (EXTRA_OBJ_ID) \n".
		  		" references EXTRA_OBJ (EXTRA_OBJ_ID)";
		$insarr[]="/* trigger: primary-key */";
		$insarr[]="create sequence AS_ASR_SEQ start with 1 nocache";
		$insarr[]="create or replace trigger AS_ASR_AIC \n".
		  		" before insert\n".
		  		" on AS_ASR\n".
		  		" for each row\n".
		  		" \n".
		  		" begin\n".
		  		" IF :new.AS_ASR_ID is null or :new.AS_ASR_ID > 0 THEN\n".
		  		" select AS_ASR_SEQ.nextval into :new.AS_ASR_ID from dual;\n".
		  		" END IF;\n".
		  		" end;";
		$insarr[]="alter trigger AS_ASR_AIC compile";
		$insarr[]="/* trigger: delete extra_obj */";
		$insarr[]="create or replace trigger AS_ASR_DEO \n".
		  		" after delete \n".
		  		" on AS_ASR \n".
		  		" for each row \n".
		  		" when (old.extra_obj_id > 0) \n".
		  		" begin \n".
		  		" delete from extra_obj where extra_obj_id = :old.extra_obj_id; \n".
		  		" end;";
		$insarr[]="alter trigger AS_ASR_DEO compile";
		
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_TABLE', 'vals'=>array('TABLE_NAME'=>'AS_ASR', 'NICE_NAME'=>'ASR lib', 'CCT_TABLE_NAME'=>NULL, 'TABLE_TYPE'=>'BO', 'IS_VIEW'=>0, 'INTERNAL'=>0, 'EXIM'=>0) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'AS_ASR_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>1, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>1, 'TABLE_NAME'=>'AS_ASR', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'NAME', 'NICE_NAME'=>'name', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>1, 'NOT_NULL'=>0, 'POS'=>2, 'TABLE_NAME'=>'AS_ASR', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'CCT_ACCESS_ID', 'NICE_NAME'=>'cct access id', 'CCT_TABLE_NAME'=>'CCT_ACCESS', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>3, 'TABLE_NAME'=>'AS_ASR', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'EXTRA_OBJ_ID', 'NICE_NAME'=>'extra obj id', 'CCT_TABLE_NAME'=>'EXTRA_OBJ', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>4, 'TABLE_NAME'=>'AS_ASR', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'DATAX', 'NICE_NAME'=>'internal data', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>6, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>5, 'TABLE_NAME'=>'AS_ASR', 'VISIBLE'=>1, 'EDITABLE'=>0) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'NOTES', 'NICE_NAME'=>'notes', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>3, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>15, 'TABLE_NAME'=>'AS_ASR', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		//
		// // <TABLE> AS_ASR_H_SP --- 
		//
		$insarr[]="create table AS_ASR_H_SP ( \n".
		  		" AS_ASR_ID NUMBER not null,\n".
		  		" BSID NUMBER,\n".
		  		" DATAX VARCHAR(4000),\n".
		  		" LIB_REAG_ID NUMBER,\n".
		  		" BEAD_REAG_ID NUMBER,\n".
		  		" constraint PK_AS_ASR_H_SP primary key (AS_ASR_ID, BSID) \n".
		  		")";
		$insarr[]="comment on table AS_ASR_H_SP is 'ASR lib has species'";
		$insarr[]="comment on column AS_ASR_H_SP.AS_ASR_ID is 'id of ASR lib has species'";
		$insarr[]="comment on column AS_ASR_H_SP.BSID is 'Species ID'";
		$insarr[]="comment on column AS_ASR_H_SP.DATAX is 'JSON string of channel data '";
		$insarr[]="comment on column AS_ASR_H_SP.LIB_REAG_ID is 'link to library reagent'";
		$insarr[]="comment on column AS_ASR_H_SP.BEAD_REAG_ID is 'link to bead reagent '";
		$insarr[]="/* indices */";
		$insarr[]="create index AS_ASR_H_SP_AK2 on AS_ASR_H_SP (BSID ASC)";
		$insarr[]="/* constraints */";
		$insarr[]="alter table AS_ASR_H_SP \n".
		  		" add constraint FK_AS_ASR2AS_ASR_H_SP foreign key (AS_ASR_ID) \n".
		  		" references AS_ASR (AS_ASR_ID)\n".
		  		" on delete cascade";
		$insarr[]="alter table AS_ASR_H_SP \n".
		  		" add constraint FK_ABSTRACT_SUBST2AS_ASR_H_SP foreign key (LIB_REAG_ID) \n".
		  		" references ABSTRACT_SUBST (ABSTRACT_SUBST_ID)";
		$insarr[]="alter table AS_ASR_H_SP \n".
		  		" add constraint FK_ABSTRACT_SUB2_2AS_ASR_H_SP foreign key (BEAD_REAG_ID) \n".
		  		" references ABSTRACT_SUBST (ABSTRACT_SUBST_ID)";
		
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_TABLE', 'vals'=>array('TABLE_NAME'=>'AS_ASR_H_SP', 'NICE_NAME'=>'ASR lib has species', 'CCT_TABLE_NAME'=>'AS_ASR', 'TABLE_TYPE'=>'BO_ASSOC', 'IS_VIEW'=>0, 'INTERNAL'=>0, 'EXIM'=>0) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'AS_ASR_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'AS_ASR', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>1, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>1, 'TABLE_NAME'=>'AS_ASR_H_SP', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'BSID', 'NICE_NAME'=>'Species ID', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>2, 'MOST_IMP_COL'=>1, 'NOT_NULL'=>0, 'POS'=>2, 'TABLE_NAME'=>'AS_ASR_H_SP', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'DATAX', 'NICE_NAME'=>'channel data', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>6, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>3, 'TABLE_NAME'=>'AS_ASR_H_SP', 'VISIBLE'=>1, 'EDITABLE'=>0) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'LIB_REAG_ID', 'NICE_NAME'=>'library reagent', 'CCT_TABLE_NAME'=>'ABSTRACT_SUBST', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>4, 'TABLE_NAME'=>'AS_ASR_H_SP', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'BEAD_REAG_ID', 'NICE_NAME'=>'bead reagent', 'CCT_TABLE_NAME'=>'ABSTRACT_SUBST', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>5, 'TABLE_NAME'=>'AS_ASR_H_SP', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		//
		// // <TABLE> EXP_SRES ---
		//
		$insarr[]="/* TABLE EXP_SRES */";
		$insarr[]="create table EXP_SRES ( \n".
		  		" EXP_ID NUMBER not null,\n".
		  		" POS NUMBER not null,\n".
		  		" CONCRETE_SUBST_ID NUMBER not null,\n".
		  		" STATUS NUMBER,\n".
		  		" ERRCODE VARCHAR(255),\n".
		  		" N01 NUMBER,\n".
		  		" N02 NUMBER,\n".
		  		" N03 NUMBER,\n".
		  		" N04 NUMBER,\n".
		  		" N05 NUMBER,\n".
		  		" N06 NUMBER,\n".
		  		" N07 NUMBER,\n".
		  		" N08 NUMBER,\n".
		  		" N09 NUMBER,\n".
		  		" N10 NUMBER,\n".
		  		" constraint PK_EXP_SRES primary key (EXP_ID, POS) \n".
		  		")";
		$insarr[]="comment on table EXP_SRES is 'experiment sample result'";
		$insarr[]="comment on column EXP_SRES.EXP_ID is 'id of experiment sample result'";
		$insarr[]="comment on column EXP_SRES.POS is 'sample pos'";
		$insarr[]="comment on column EXP_SRES.CONCRETE_SUBST_ID is 'link to the sample'";
		$insarr[]="comment on column EXP_SRES.STATUS is 'status of sample result'";
		$insarr[]="comment on column EXP_SRES.ERRCODE is 'error code'";
		$insarr[]="comment on column EXP_SRES.N01 is 'data value'";
		$insarr[]="comment on column EXP_SRES.N02 is 'data value'";
		$insarr[]="comment on column EXP_SRES.N03 is 'data value'";
		$insarr[]="comment on column EXP_SRES.N04 is 'data value'";
		$insarr[]="comment on column EXP_SRES.N05 is 'data value'";
		$insarr[]="comment on column EXP_SRES.N06 is 'data value'";
		$insarr[]="comment on column EXP_SRES.N07 is 'data value'";
		$insarr[]="comment on column EXP_SRES.N08 is 'data value'";
		$insarr[]="comment on column EXP_SRES.N09 is 'data value'";
		$insarr[]="comment on column EXP_SRES.N10 is 'data value'";
		$insarr[]="/* indices */";
		$insarr[]="create index EXP_SRES_AK2 on EXP_SRES (CONCRETE_SUBST_ID ASC)";
		$insarr[]="/* constraints */";
		$insarr[]="alter table EXP_SRES \n".
		  		" add constraint FK_EXP2EXP_SRES foreign key (EXP_ID) \n".
		  		" references EXP (EXP_ID)\n".
		  		" on delete cascade";
		$insarr[]="alter table EXP_SRES \n".
		  		" add constraint FK_CONCRETE_SUBST2EXP_SRES foreign key (CONCRETE_SUBST_ID) \n".
		  		" references CONCRETE_SUBST (CONCRETE_SUBST_ID)";
		
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_TABLE', 'vals'=>array('TABLE_NAME'=>'EXP_SRES', 'NICE_NAME'=>'experiment sample result', 'CCT_TABLE_NAME'=>'EXP', 'TABLE_TYPE'=>'BO_ASSOC', 'IS_VIEW'=>0, 'INTERNAL'=>0, 'EXIM'=>0) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'EXP_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'EXP', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>1, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>1, 'TABLE_NAME'=>'EXP_SRES', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'POS', 'NICE_NAME'=>'sample pos', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>15, 'PRIMARY_KEY'=>2, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>2, 'TABLE_NAME'=>'EXP_SRES', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'CONCRETE_SUBST_ID', 'NICE_NAME'=>'sample', 'CCT_TABLE_NAME'=>'CONCRETE_SUBST', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>1, 'NOT_NULL'=>1, 'POS'=>3, 'TABLE_NAME'=>'EXP_SRES', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'STATUS', 'NICE_NAME'=>'status', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>15, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>4, 'TABLE_NAME'=>'EXP_SRES', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'ERRCODE', 'NICE_NAME'=>'error code', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>5, 'TABLE_NAME'=>'EXP_SRES', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'N01', 'NICE_NAME'=>'data value 01', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>5, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>6, 'TABLE_NAME'=>'EXP_SRES', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'N01', 'NICE_NAME'=>'data value 02', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>5, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>6, 'TABLE_NAME'=>'EXP_SRES', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'N01', 'NICE_NAME'=>'data value 03', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>5, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>6, 'TABLE_NAME'=>'EXP_SRES', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'N01', 'NICE_NAME'=>'data value 04', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>5, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>6, 'TABLE_NAME'=>'EXP_SRES', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'N01', 'NICE_NAME'=>'data value 05', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>5, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>6, 'TABLE_NAME'=>'EXP_SRES', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'N01', 'NICE_NAME'=>'data value 06', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>5, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>6, 'TABLE_NAME'=>'EXP_SRES', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'N01', 'NICE_NAME'=>'data value 07', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>5, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>6, 'TABLE_NAME'=>'EXP_SRES', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'N01', 'NICE_NAME'=>'data value 08', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>5, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>6, 'TABLE_NAME'=>'EXP_SRES', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'N01', 'NICE_NAME'=>'data value 09', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>5, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>6, 'TABLE_NAME'=>'EXP_SRES', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'N01', 'NICE_NAME'=>'data value 10', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>5, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>6, 'TABLE_NAME'=>'EXP_SRES', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		
		// <TABLE> ABSTRACT_SUBST 
		$insarr[]="alter table ABSTRACT_SUBST add ( \n".
		  		" SYNONYMX VARCHAR(255),\n".
		  		" MSDS NUMBER,\n".
		  		" CAS VARCHAR(255)\n".
		  		")";
		
		$insarr[]="comment on column ABSTRACT_SUBST.SYNONYMX is 'alternative name'";
		$insarr[]="comment on column ABSTRACT_SUBST.MSDS is 'Material has a Safety data sheet'";
		$insarr[]="comment on column ABSTRACT_SUBST.CAS is 'CAS Registry Number: Wikipedia: CAS_Registry_Number'";
		
		
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'SYNONYMX', 'NICE_NAME'=>'synonym', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>2.5, 'TABLE_NAME'=>'ABSTRACT_SUBST', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'MSDS', 'NICE_NAME'=>'MSDS flag', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>15, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>12, 'TABLE_NAME'=>'ABSTRACT_SUBST',  'VISIBLE'=>2, 'EDITABLE'=>0) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'CAS', 'NICE_NAME'=>'CAS number', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0,  'MOST_IMP_COL'=>0,  'NOT_NULL'=>0,  'POS'=>16, 'TABLE_NAME'=>'ABSTRACT_SUBST', 'VISIBLE'=>2, 'EDITABLE'=>1) );
		
		
		// <TABLE> ABS_CART_H_SUA
		$insarr[]="alter table ABS_CART_H_SUA add ( \n".
		  		" VOLUME NUMBER \n".
		  		")";	
		$insarr[]="comment on column ABS_CART_H_SUA.VOLUME is 'reagent volume [ul]'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'VOLUME', 'NICE_NAME'=>'volume', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>5, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>4.5, 'TABLE_NAME'=>'ABS_CART_H_SUA',  'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		// <TABLE> CART_H_SUC
		$insarr[]="alter table CART_H_SUC add ( \n".
		  		" VOLUME NUMBER \n".
		  		")";
		$insarr[]="comment on column CART_H_SUC.VOLUME is 'reagent volume [ul]'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'VOLUME', 'NICE_NAME'=>'volume', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>5, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>5, 'TABLE_NAME'=>'CART_H_SUC',  'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		//
		//
		// POSTGRES starting here ...
		//
		//
		
		// <TABLE> ABS_SOCKET 2020-01-15
		$insarr[]="alter table ABS_SOCKET add  C_ASSAY_ID VARCHAR(255)";
		$insarr[]="alter table ABS_SOCKET add  C_ASSAY_NAME VARCHAR(255)";
		  	
		$insarr[]="comment on column ABS_SOCKET.C_ASSAY_ID is 'Customer Assay-ID'";
		$insarr[]="comment on column ABS_SOCKET.C_ASSAY_NAME is 'Customer Assay Name'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'C_ASSAY_ID', 'NICE_NAME'=>'C-Assay-ID', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0,  'MOST_IMP_COL'=>0,  'NOT_NULL'=>0,  'POS'=>5.1, 'TABLE_NAME'=>'ABS_SOCKET', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'C_ASSAY_NAME', 'NICE_NAME'=>'C-Assay-Name', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0,  'MOST_IMP_COL'=>0,  'NOT_NULL'=>0,  'POS'=>5.2, 'TABLE_NAME'=>'ABS_SOCKET', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		// ...
		// <TABLE> H_SOCK_TYPE 2020-03-10
		$insarr[]="alter table H_SOCK_TYPE add JSON_DATA VARCHAR(4000)";
		$insarr[]="comment on column H_SOCK_TYPE.JSON_DATA is 'JSON definition data'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'JSON_DATA', 'NICE_NAME'=>'JSON-data', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0,  'MOST_IMP_COL'=>0,  'NOT_NULL'=>0,  'POS'=>4, 'TABLE_NAME'=>'H_SOCK_TYPE', 'VISIBLE'=>1, 'EDITABLE'=>0) );
		
		
		$insarr[]="/* <TABLE> CONT_LOG 2020-06-10 */";
		$insarr[]="create table CONT_LOG ( \n".
		  		" CONTAINER_ID BIGINT not null,\n".
		  		" POS BIGINT not null,\n".
		  		" MOD_DATE timestamp  not null,\n".
		  		" CONCRETE_SUBST_ID BIGINT not null,\n".
		  		" QUANTITY real,\n".
		  		" DB_USER_ID BIGINT,\n".
		  		" KEY VARCHAR(255),\n".
		  		" constraint PK_CONT_LOG primary key (CONTAINER_ID, POS) \n".
		  		")";
		$insarr[]="comment on table CONT_LOG is 'storage time log'";
		$insarr[]="comment on column CONT_LOG.CONTAINER_ID is 'id of storage'";
		$insarr[]="comment on column CONT_LOG.CONCRETE_SUBST_ID is 'id of material'";
		$insarr[]="comment on column CONT_LOG.DB_USER_ID is 'id of user'";
		$insarr[]="comment on column CONT_LOG.QUANTITY is 'quantity diff'";
		$insarr[]="comment on column CONT_LOG.POS is 'log position'";
		$insarr[]="comment on column CONT_LOG.KEY is 'action-key: add, del, none'";
		
		$insarr[]="/* indices */";
		$insarr[]="create index CONT_LOG_HAS_CSUBST_AK2 on CONT_LOG (CONCRETE_SUBST_ID ASC)";
		$insarr[]="create index CONT_LOG_HAS_POS_AK3 on CONT_LOG (CONCRETE_SUBST_ID ASC)";
		
		$insarr[]="/* constraints */";
		$insarr[]="/* INFO: NO hard FK-constraint for CONCRETE_SUBST: otherwise delete of SUC not possible ... */";
		
		$insarr[]="alter table CONT_LOG \n".
		  		" add constraint FK_CONTAINER2CONT_LOG foreign key (CONTAINER_ID) \n".
		  		" references CONTAINER (CONTAINER_ID)\n".
		  		" on delete cascade";
		$insarr[]="alter table CONT_LOG \n".
		  		" add constraint FK_DB_USER2CONT_LOG foreign key (DB_USER_ID) \n".
		  		" references DB_USER (DB_USER_ID)";
		
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_TABLE', 'vals'=>array('TABLE_NAME'=>'CONT_LOG', 'NICE_NAME'=>'storage log', 'CCT_TABLE_NAME'=>'CONTAINER', 'TABLE_TYPE'=>'BO_ASSOC', 'IS_VIEW'=>0, 'INTERNAL'=>0, 'EXIM'=>0) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'CONTAINER_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'CONTAINER', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>1, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>1, 'TABLE_NAME'=>'CONT_LOG', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'POS',          'NICE_NAME'=>'position', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>15, 'PRIMARY_KEY'=>2, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>2, 'TABLE_NAME'=>'CONT_LOG', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'MOD_DATE', 'NICE_NAME'=>'log date', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>12, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>3, 'TABLE_NAME'=>'CONT_LOG', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'CONCRETE_SUBST_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'CONCRETE_SUBST', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>1, 'NOT_NULL'=>1, 'POS'=>4, 'TABLE_NAME'=>'CONT_LOG', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'QUANTITY', 'NICE_NAME'=>'quantity diff', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>5, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>4.5, 'TABLE_NAME'=>'CONT_LOG', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'DB_USER_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'DB_USER', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>5, 'TABLE_NAME'=>'CONT_LOG', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'KEY', 'NICE_NAME'=>'key', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>6, 'TABLE_NAME'=>'CONT_LOG', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		
		// 
		$insarr[]="/* <TABLE> ABSTRACT_PROTO 2020-06-15 */";
		$insarr[]="alter table ABSTRACT_PROTO add  FORMULA VARCHAR(32000)";
		$insarr[]="comment on column ABSTRACT_PROTO.FORMULA is 'JONS string of formulas'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'FORMULA', 'NICE_NAME'=>'Formula', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>3, 'PRIMARY_KEY'=>0,  'MOST_IMP_COL'=>0,  'NOT_NULL'=>0,  'POS'=>6.7, 'TABLE_NAME'=>'ABSTRACT_PROTO', 'VISIBLE'=>0, 'EDITABLE'=>0) );
		
		//
		$insarr[]="/* <TABLE> ABSTRACT_PROTO 2020-06-22 */";
		$insarr[]="alter table ABSTRACT_PROTO add  LAYOUT VARCHAR(32000)";
		$insarr[]="comment on column ABSTRACT_PROTO.LAYOUT is 'JONS string of layout'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'LAYOUT', 'NICE_NAME'=>'Layout', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>3, 'PRIMARY_KEY'=>0,  'MOST_IMP_COL'=>0,  'NOT_NULL'=>0,  'POS'=>6.8, 'TABLE_NAME'=>'ABSTRACT_PROTO', 'VISIBLE'=>0, 'EDITABLE'=>0) );
		
		// 2020-08-19 : add constraints
		$insarr[]="/* <TABLE> CHIP_READER 2020-08-19 */";
		$insarr[]="ALTER TABLE CHIP_READER  alter column A_CHIP_READER_ID set NOT NULL";
		$insarr[]="ALTER TABLE CHIP_READER add constraint UQ_CHIP_READER2NAME unique (A_CHIP_READER_ID,NAME)";
		$meta[]=array( 'act'=>'upd', 'tab'=>'CCT_COLUMN', 'ids'=>array('COLUMN_NAME'=>'A_CHIP_READER_ID', 'TABLE_NAME'=>'CHIP_READER'), 'vals'=>array('NOT_NULL'=>1) );
		
	
		// 2020-09-03 : add CONCENTRATION
		$insarr[]="/* <TABLE> ABSTRACT_SUBST, CONCRETE_SUBST 2020-09-03 */";
		$insarr[]="ALTER TABLE ABSTRACT_SUBST add CONC_UNIT_ID BIGINT";
		$insarr[]="comment on column ABSTRACT_SUBST.CONC_UNIT_ID is 'concentration meas. unit'";
		$insarr[]="alter table ABSTRACT_SUBST \n".
		  		" add constraint FK_H_UNIT2ABSTRACT_SUBST_C foreign key (CONC_UNIT_ID) \n".
		  		" references H_UNIT (H_UNIT_ID)";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array( 'TABLE_NAME'=>'ABSTRACT_SUBST',  'COLUMN_NAME'=>'CONC_UNIT_ID', 'NICE_NAME'=>'concentration meas. unit', 'CCT_TABLE_NAME'=>'H_UNIT', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>6.7, 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		
		$insarr[]="ALTER TABLE CONCRETE_SUBST add CONC_QUANT numeric";
		$insarr[]="comment on column CONCRETE_SUBST.CONC_QUANT is 'chem. concentration'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array( 'TABLE_NAME'=>'CONCRETE_SUBST',  'COLUMN_NAME'=>'CONC_QUANT', 'NICE_NAME'=>'chem. concentration', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>5, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>12, 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		// 2020-09-03 : add CONCENTRATION
		$insarr[]="/* <TABLE> CCT_ACCESS, CCT_ACC_UP 2020-09-24 */";
		$insarr[]="alter table CCT_ACCESS alter column UPINFO type VARCHAR(4000)";
		
		// ... and REBUILD cct_acc_up_view
		$insarr[]="drop VIEW CCT_ACC_UP_VIEW;";
		$insarr[]="alter table CCT_ACC_UP alter column UPINFO type VARCHAR(4000)";
		$insarr[]="CREATE VIEW CCT_ACC_UP_VIEW as SELECT cc.cct_access_id,".
		  "cc.table_name, \n".
		  "cu.pos, \n".
		  "cu.modi_date, \n".
		  "cu.db_user_id, \n".
		  "cu.upinfo \n".
		  "FROM cct_access cc, \n".
		  "cct_acc_up cu \n".
		  "WHERE cc.cct_access_id = cu.cct_access_id;";
		
		// 2020-12-11 : CHIP_READER : status_mx - status matrix
		/**
		 * ok-status:    o or s  (ok, stop)
		 * quali-status: q or n  (q:qualified or n:needed)
		 * service status: x or y (x:service started or y:service done)
		 */
		$insarr[]="/* <TABLE> CHIP_READER : status_mx - status matrix 2020-12-11 */";
		$insarr[]="ALTER TABLE CHIP_READER add STATUS_MX  VARCHAR(20)";
		$insarr[]="comment on column CHIP_READER.STATUS_MX is 'status matrix'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array( 
		    'TABLE_NAME'=>'CHIP_READER',  'COLUMN_NAME'=>'STATUS_MX', 'NICE_NAME'=>'status matrix', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>8.7, 'VISIBLE'=>1, 'EDITABLE'=>0) );
		
		
		// 2020-12-16 : CHIP_READER : IS_REGUL is regulated : 0 or 1
		
		$insarr[]="/* <TABLE> CHIP_READER : IS_REGUL 2020-12-11 */";
		$insarr[]="ALTER TABLE CHIP_READER add IS_REGUL  SMALLINT";
		$insarr[]="comment on column CHIP_READER.IS_REGUL is 'equipment is regulated (e.g. due to FDA regulations)? 0,1'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array(
		    'TABLE_NAME'=>'CHIP_READER',  'COLUMN_NAME'=>'IS_REGUL', 'NICE_NAME'=>'is regulated', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>7, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>11, 'VISIBLE'=>2, 'EDITABLE'=>1) );
		
		$meta[]=array( 'act'=>'upd', 'tab'=>'CCT_COLUMN', 'ids'=>array( 'TABLE_NAME'=>'CHIP_READER',  'COLUMN_NAME'=>'NOTES'), 
		    'vals'=>array( 'POS'=>20) );
		
		
		$insarr[]="/* <TABLE> H_SOCK_TYPE :  2021-01-29 */";
		$insarr[]="ALTER TABLE H_SOCK_TYPE add KEY  VARCHAR(255)";
		$insarr[]="comment on column H_SOCK_TYPE.KEY is 'simple KEY'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array(
		    'TABLE_NAME'=>'H_SOCK_TYPE',  'COLUMN_NAME'=>'KEY', 'NICE_NAME'=>'key', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>2.5, 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		
		$insarr[]="/* <TABLE> H_ALOG_ACT :  2021-01-30 */";
		$insarr[]="ALTER TABLE H_ALOG_ACT add DATAX  VARCHAR(4000)";
		$insarr[]="comment on column H_ALOG_ACT.DATAX is 'JSON data'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array(
		    'TABLE_NAME'=>'H_ALOG_ACT',  'COLUMN_NAME'=>'DATAX', 'NICE_NAME'=>'datax', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>6, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>4, 'VISIBLE'=>0, 'EDITABLE'=>0) );
		
		
		// -------------------
		$meta[] = array( "act"=>"upd", "tab"=>"GLOBALS", "vals"=>array("VALUE"=>$dataArray["reqs"]["DbVersionNew"]), "ids"=>array("NAME"=>"DBVersion" ) );
		
		
		$dataArray["sql"]  = &$insarr;
		$dataArray["meta"] = &$meta;
	}
	

}
