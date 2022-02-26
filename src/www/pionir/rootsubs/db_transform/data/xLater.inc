<?php 
/**
 * for future
 */

$insarr[]="/* <TABLE> DEV_BOOK 2020-11-02 */";

$insarr[]="create table DEV_BOOK ( \n".
    " DEV_ID BIGINT not null,\n".
    " POS BIGINT not null,\n".
    " DATE1 TIMESTAMP not null,\n".
    " DATE2 TIMESTAMP not null,\n".
    " DB_USER_ID BIGINT not null,\n".
    " NAME VARCHAR(255),\n".
    " XDATA VARCHAR(4000),\n".
    " constraint PK_DEV_BOOK primary key (DEV_ID, POS) \n".
    ")";
$insarr[]="comment on table DEV_BOOK is E'equipment booking log'";
$insarr[]="comment on column DEV_BOOK.DEV_ID is E'id of equipment booking log'";
$insarr[]="comment on column DEV_BOOK.POS is E'pos'";
$insarr[]="comment on column DEV_BOOK.DATE1 is E'start date'";
$insarr[]="comment on column DEV_BOOK.DATE2 is E'end date'";
$insarr[]="comment on column DEV_BOOK.DB_USER_ID is E'id of user'";
$insarr[]="comment on column DEV_BOOK.NAME is E'name of entry'";
$insarr[]="comment on column DEV_BOOK.XDATA is E'JSON data'";
$insarr[]="/* indices */";
$insarr[]="/* constraints */";
$insarr[]="alter table DEV_BOOK \n".
    " add constraint FK_CHIP_READER2DEV_BOOK foreign key (DEV_ID) \n".
    " references CHIP_READER (CHIP_READER_ID)\n".
    " on delete cascade";
$insarr[]="alter table DEV_BOOK \n".
    " add constraint FK_DB_USER2DEV_BOOK foreign key (DB_USER_ID) \n".
    " references DB_USER (DB_USER_ID)";

$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_TABLE', 'vals'=>array('TABLE_NAME'=>'DEV_BOOK', 'NICE_NAME'=>'equipment booking log', 'CCT_TABLE_NAME'=>'CHIP_READER', 'TABLE_TYPE'=>'BO_ASSOC', 'IS_VIEW'=>0, 'INTERNAL'=>0, 'EXIM'=>1) );
$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'DEV_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'CHIP_READER', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>1, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>1, 'TABLE_NAME'=>'DEV_BOOK', 'VISIBLE'=>1, 'EDITABLE'=>1) );
$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'POS', 'NICE_NAME'=>'pos', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>2, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>2, 'TABLE_NAME'=>'DEV_BOOK', 'VISIBLE'=>1, 'EDITABLE'=>1) );
$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'DATE1', 'NICE_NAME'=>'start date', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>12, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>3, 'TABLE_NAME'=>'DEV_BOOK', 'VISIBLE'=>1, 'EDITABLE'=>1) );
$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'DATE2', 'NICE_NAME'=>'end date', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>12, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>4, 'TABLE_NAME'=>'DEV_BOOK', 'VISIBLE'=>1, 'EDITABLE'=>1) );
$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'DB_USER_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'DB_USER', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>5, 'TABLE_NAME'=>'DEV_BOOK', 'VISIBLE'=>1, 'EDITABLE'=>1) );
$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'NAME', 'NICE_NAME'=>'name of entry', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>6, 'TABLE_NAME'=>'DEV_BOOK', 'VISIBLE'=>1, 'EDITABLE'=>1) );
$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'XDATA', 'NICE_NAME'=>'xdata', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>3, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>7, 'TABLE_NAME'=>'DEV_BOOK', 'VISIBLE'=>1, 'EDITABLE'=>1) );

