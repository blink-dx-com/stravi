<?php
require_once 'insert.inc';
require_once ('f.update.inc');
require_once 'o.CCT_TABLE.info.inc';
/*
 *  Blink changes
 */
class gDB_tranData_2_0_0_6 extends gDB_transAbstract {

	
	function getBasicInfo() {
	    $dataArray=array();
		$dataArray["info"] = "new: PUR. ABSTRACT_PROTO_STEP, USER_PREF2";
		$dataArray["reqs"] = array(
				"DbVersionNeed" => '2.0.0.5', 
				"DbVersionNew"  => "2.0.0.6 - 20211029 - 10:12");
		
		$this->dataArray = $dataArray;
		return $dataArray;
	}

	function getData( &$dataArray, $option=NULL ) {
		// $go = $option['go'];
		$dataArray = $this->dataArray;
				
		
		$insarr	= NULL;
		$meta	= NULL;
		
		$insarr[]="/* <TABLE> PUR  : NEW 2021-10-26 */";
		
		$insarr[]="create table PUR ( \n".
		  		" PUR_ID SERIAL,\n".
		  		" NAME VARCHAR(255),\n".
		  		" ABSTRACT_SUBST_ID BIGINT not null,\n".
		  		" CONTACT_ID BIGINT,\n".
		  		" PROD_NO VARCHAR(255),\n".
		  		" CONT_QUANT DOUBLE PRECISION,\n".
		  		" QUANTITY DOUBLE PRECISION,\n".
		  		" H_UNIT_ID BIGINT not null,\n".
		  		" REQ_DATE TIMESTAMP,\n".
		  		" EST_DEL_DATE TIMESTAMP,\n".
		  		" DELIVER_DATE TIMESTAMP,\n".
		  		" CCT_ACCESS_ID BIGINT not null,\n".
		  		" EXTRA_OBJ_ID BIGINT,\n".
		  		" NOTES VARCHAR(4000),\n".
		  		" constraint PK_PUR primary key (PUR_ID) \n".
		  		")";
		$insarr[]="comment on table PUR is E'Purchase requesition (Bestellwunsch)'";
		$insarr[]="comment on column PUR.PUR_ID is E'id of Bestellwunsch'";
		$insarr[]="comment on column PUR.NAME is E'name of the Bestellwunsch'";
		$insarr[]="comment on column PUR.ABSTRACT_SUBST_ID is E'the material ttemplate'";
		$insarr[]="comment on column PUR.CONTACT_ID is E'supplier contact'";
		$insarr[]="comment on column PUR.PROD_NO is E'product number of supplier'";
		$insarr[]="comment on column PUR.QUANTITY   is E'number of containers'";
		$insarr[]="comment on column PUR.CONT_QUANT is E'container size'";
		$insarr[]="comment on column PUR.H_UNIT_ID is E''";
		$insarr[]="comment on column PUR.REQ_DATE is E''";
		$insarr[]="comment on column PUR.EST_DEL_DATE is E''";
		$insarr[]="comment on column PUR.DELIVER_DATE is E'date of delivery'";
		$insarr[]="comment on column PUR.CCT_ACCESS_ID is E'id of the access rights'";
		$insarr[]="comment on column PUR.EXTRA_OBJ_ID is E'id of the extra obj'";
		$insarr[]="comment on column PUR.NOTES is E'notes'";
		$insarr[]="/* indices */";
		$insarr[]="create index PUR_AK2 on PUR (NAME ASC)";
		$insarr[]="/* constraints */";
		$insarr[]="alter table PUR \n".
		  		" add constraint FK_ABSTRACT_SUBST2PUR foreign key (ABSTRACT_SUBST_ID) \n".
		  		" references ABSTRACT_SUBST (ABSTRACT_SUBST_ID)";
		$insarr[]="alter table PUR \n".
		  		" add constraint FK_CONTACT2PUR foreign key (CONTACT_ID) \n".
		  		" references CONTACT (CONTACT_ID)";
		$insarr[]="alter table PUR \n".
		  		" add constraint FK_H_UNIT2PUR foreign key (H_UNIT_ID) \n".
		  		" references H_UNIT (H_UNIT_ID)";
		$insarr[]="alter table PUR \n".
		  		" add constraint FK_CCT_ACCESS2PUR foreign key (CCT_ACCESS_ID) \n".
		  		" references CCT_ACCESS (CCT_ACCESS_ID)\n".
		  		" on delete cascade";
		$insarr[]="alter table PUR \n".
		  		" add constraint FK_EXTRA_OBJ2PUR foreign key (EXTRA_OBJ_ID) \n".
		  		" references EXTRA_OBJ (EXTRA_OBJ_ID)";
		$insarr[]="/* trigger: delete extra_obj */";
		$insarr[]="CREATE FUNCTION trigger_PUR_deo() RETURNS trigger\n".
		  		" LANGUAGE plpgsql\n".
		  		" AS \$\$\n".
		  		" BEGIN\n".
		  		" delete from extra_obj where extra_obj_id = OLD.extra_obj_id;\n".
		  		" RETURN OLD;\n".
		  		" end\n".
		  		" \$\$;\n".
		  		" ";
		$insarr[]="CREATE TRIGGER PUR_deo AFTER DELETE ON PUR FOR EACH ROW WHEN ((old.extra_obj_id > 0)) EXECUTE PROCEDURE trigger_PUR_deo();";
		
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_TABLE', 'vals'=>array('TABLE_NAME'=>'PUR', 'NICE_NAME'=>'Bestellwunsch', 'CCT_TABLE_NAME'=>NULL, 'TABLE_TYPE'=>'BO', 'IS_VIEW'=>0, 'INTERNAL'=>0, 'EXIM'=>0) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'PUR_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>1, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>1, 'TABLE_NAME'=>'PUR', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'NAME', 'NICE_NAME'=>'name', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>1, 'NOT_NULL'=>0, 'POS'=>2, 'TABLE_NAME'=>'PUR', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'ABSTRACT_SUBST_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'ABSTRACT_SUBST', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>3, 'TABLE_NAME'=>'PUR', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'CONTACT_ID', 'NICE_NAME'=>'supplier', 'CCT_TABLE_NAME'=>'CONTACT', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>4, 'TABLE_NAME'=>'PUR', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'PROD_NO', 'NICE_NAME'=>'external product number', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>5, 'TABLE_NAME'=>'PUR', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'QUANTITY',   'NICE_NAME'=>'number of containers', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>5, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>6, 'TABLE_NAME'=>'PUR', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'CONT_QUANT', 'NICE_NAME'=>'container quantity', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>5, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0,   'NOT_NULL'=>1, 'POS'=>6.5, 'TABLE_NAME'=>'PUR', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'H_UNIT_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'H_UNIT', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>7, 'TABLE_NAME'=>'PUR', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'REQ_DATE', 'NICE_NAME'=>'request date', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>12, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>8, 'TABLE_NAME'=>'PUR', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'EST_DEL_DATE', 'NICE_NAME'=>'estimated delivery date', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>12, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>8.2, 'TABLE_NAME'=>'PUR', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'DELIVER_DATE', 'NICE_NAME'=>'delivery date', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>12, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>8.4, 'TABLE_NAME'=>'PUR', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'CCT_ACCESS_ID', 'NICE_NAME'=>'cct access id', 'CCT_TABLE_NAME'=>'CCT_ACCESS', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>9, 'TABLE_NAME'=>'PUR', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'EXTRA_OBJ_ID', 'NICE_NAME'=>'extra obj id', 'CCT_TABLE_NAME'=>'EXTRA_OBJ', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>10, 'TABLE_NAME'=>'PUR', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'NOTES', 'NICE_NAME'=>'notes', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>3, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>11, 'TABLE_NAME'=>'PUR', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_TABLE_DYN', 'vals'=>array('TABLE_NAME'=>'PUR', 'KEY'=>'SHORT', 'VALUE'=>'PRQ') );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_TABLE_DYN', 'vals'=>array('TABLE_NAME'=>'PUR', 'KEY'=>'COLOR', 'VALUE'=>'#D0D0D0') );
		
		
		$insarr[]="/* <TABLE> ABSTRACT_PROTO_STEP  : NEW 2021-10-27 */";
		$insarr[]="ALTER TABLE ABSTRACT_PROTO_STEP add DATAX VARCHAR(4000)";
		$insarr[]="comment on column ABSTRACT_PROTO_STEP.DATAX is 'extra JSON data'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'ABSTRACT_PROTO_STEP', 'COLUMN_NAME'=>'DATAX', 'NICE_NAME'=>'JSON data', 'CCT_TABLE_NAME'=>NULL,
		    'APP_DATA_TYPE_ID'=>19, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>11.5, 'VISIBLE'=>0, 'EDITABLE'=>0) );
		
		$insarr[]="/* <TABLE> USER_PREF2  : NEW 2021-10-29 */";
		$insarr[]="create table USER_PREF2 ( \n".
		  		" USER_PREF2_ID SERIAL,\n".
		  		" DB_USER_ID BIGINT not null,\n".
		  		" KEY VARCHAR(255) not null,\n".
		  		" VALUE VARCHAR(4000),\n".
		  		" constraint PK_USER_PREF2 primary key (USER_PREF2_ID) \n".
		  		")";
		$insarr[]="comment on table USER_PREF2 is E'user config preferences'";
		$insarr[]="comment on column USER_PREF2.USER_PREF2_ID is E'id of user config preferences'";
		$insarr[]="comment on column USER_PREF2.DB_USER_ID is E'id of user'";
		$insarr[]="comment on column USER_PREF2.KEY is E'name of the user config preferences'";
		$insarr[]="comment on column USER_PREF2.VALUE is E'JSON content'";
		$insarr[]="/* indices */";
		$insarr[]="create index USER_PREF2_AK2 on USER_PREF2 (KEY ASC)";
		$insarr[]="/* constraints */";
		$insarr[]="alter table USER_PREF2 \n".
		  		" add constraint FK_DB_USER2USER_PREF2 foreign key (DB_USER_ID) \n".
		  		" references DB_USER (DB_USER_ID)".
		  		" on delete cascade";
		
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_TABLE', 'vals'=>array('TABLE_NAME'=>'USER_PREF2', 'NICE_NAME'=>'user config preferences', 'CCT_TABLE_NAME'=>NULL, 'TABLE_TYPE'=>'SYS', 'IS_VIEW'=>0, 'INTERNAL'=>1, 'EXIM'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'USER_PREF2_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>1, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>1, 'TABLE_NAME'=>'USER_PREF2', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'DB_USER_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'DB_USER', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>2, 'TABLE_NAME'=>'USER_PREF2', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'KEY', 'NICE_NAME'=>'key', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>1, 'NOT_NULL'=>1, 'POS'=>3, 'TABLE_NAME'=>'USER_PREF2', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'VALUE', 'NICE_NAME'=>'value', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>19, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>4, 'TABLE_NAME'=>'USER_PREF2', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		
		$insarr[]="/* <TABLE> ABSTRACT_SUBST  : NEW 2021-10-31 */";
		$insarr[]="ALTER TABLE ABSTRACT_SUBST add CONT_QUANT DOUBLE PRECISION";
		$insarr[]="comment on column ABSTRACT_SUBST.CONT_QUANT is 'pack size'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'ABSTRACT_SUBST', 'COLUMN_NAME'=>'CONT_QUANT', 'NICE_NAME'=>'pack size', 'CCT_TABLE_NAME'=>NULL,
		    'APP_DATA_TYPE_ID'=>5, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>25, 'VISIBLE'=>2, 'EDITABLE'=>1) );
		
		
		$meta[] = array( "act"=>"upd", "tab"=>"GLOBALS", "vals"=>array("VALUE"=>$dataArray["reqs"]["DbVersionNew"]), "ids"=>array("NAME"=>"DBVersion" ) );
		
		
		$dataArray["sql"]  = &$insarr;
		$dataArray["meta"] = &$meta;
	}
	
	

}
