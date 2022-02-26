<?php
require_once 'insert.inc';
require_once ('f.update.inc');
require_once 'o.CCT_TABLE.info.inc';
/*
 *  Blink changes
 */
class gDB_tranData_2_0_0_5 extends gDB_transAbstract {

	
	function getBasicInfo() {
	    $dataArray=array();
		$dataArray["info"] = "SOCKET, REA_LOG upgrades";
		$dataArray["reqs"] = array(
				"DbVersionNeed" => '2.0.0.4', 
				"DbVersionNew"  => "2.0.0.5 - 20210927 - 15:15");
		$dataArray['extconvert'] = array('transform'=>1 );
		$this->dataArray = $dataArray;
		return $dataArray;
	}

	function getData( &$dataArray, $option=NULL ) {
		// $go = $option['go'];
		$dataArray = $this->dataArray;
				
		
		$insarr	= NULL;
		$meta	= NULL;
		
		$insarr[]="/* <TABLE> CART_H_SUC  : NEW 2021-03-10 */";
		$insarr[]="ALTER TABLE CART_H_SUC add GRP SMALLINT";
		$insarr[]="comment on column CART_H_SUC.GRP is 'Well Group ID'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'CART_H_SUC', 'COLUMN_NAME'=>'GRP', 'NICE_NAME'=>'well group ID', 'CCT_TABLE_NAME'=>NULL, 
		    'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>6, 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		$insarr[]="/* <TABLE> JOUR_ENTRY  : NEW 2021-03-29 */";
		$insarr[]="ALTER TABLE JOUR_ENTRY add IMPACT SMALLINT";
		$insarr[]="comment on column JOUR_ENTRY.IMPACT is 'Level of impact: 1 (low).. 10 (high)'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'JOUR_ENTRY', 'COLUMN_NAME'=>'IMPACT', 'NICE_NAME'=>'Impact-level', 'CCT_TABLE_NAME'=>NULL,
		    'APP_DATA_TYPE_ID'=>15, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>8.5, 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		
		$insarr[]="/* <TABLE> REA_LOG  : NEW 2021-04-08 */";
		$insarr[]="ALTER TABLE REA_LOG add XDATA VARCHAR(4000)";
		$insarr[]="ALTER TABLE REA_LOG add MOD_DATE timestamp ";
		$insarr[]="comment on column REA_LOG.XDATA is 'JSON data'";
		$insarr[]="comment on column REA_LOG.MOD_DATE is 'modification date'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'REA_LOG', 'COLUMN_NAME'=>'XDATA', 'NICE_NAME'=>'JSON data', 'CCT_TABLE_NAME'=>NULL,
		    'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>8, 'VISIBLE'=>1, 'EDITABLE'=>0) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'REA_LOG', 'COLUMN_NAME'=>'MOD_DATE', 'NICE_NAME'=>'mod date', 'CCT_TABLE_NAME'=>NULL,
		    'APP_DATA_TYPE_ID'=>12, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>9, 'VISIBLE'=>1, 'EDITABLE'=>0) );
		
		
		$insarr[]="/* <TABLE> CHIP_READER: CHANGE : responsible contact => db_user 2021-04-13 */";
		
		// drop
		$insarr[]="delete from CCT_COLUMN where table_name='CHIP_READER' and COLUMN_NAME='RES_CONTACT_ID'";
		$insarr[]="delete from CCT_COLUMN where table_name='CHIP_READER' and COLUMN_NAME='RES_CONT2_ID'";
		$insarr[]="ALTER TABLE CHIP_READER drop column IF EXISTS RES_CONTACT_ID";
		$insarr[]="ALTER TABLE CHIP_READER drop column IF EXISTS RES_CONT2_ID";
		
		// create new
		$insarr[]="ALTER TABLE CHIP_READER add RES_USER_ID    BIGINT";
		$insarr[]="ALTER TABLE CHIP_READER add RES_GROUP_ID    BIGINT";
		$insarr[]="comment on column CHIP_READER.RES_USER_ID is E'id of responsible user'";
		$insarr[]="comment on column CHIP_READER.RES_GROUP_ID is E'id of responsible group'";
		$insarr[]="alter table CHIP_READER \n".
		  		" add constraint FK_DB_USER2CHIP_READER foreign key (RES_USER_ID) \n".
		  		" references DB_USER (DB_USER_ID)";
		$insarr[]="alter table CHIP_READER \n".
		  		" add constraint FK_USER_GROUP2CHIP_READER foreign key (RES_GROUP_ID) \n".
		  		" references USER_GROUP (USER_GROUP_ID)";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'RES_USER_ID', 'NICE_NAME'=>'responsible user', 'CCT_TABLE_NAME'=>'DB_USER', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 
		    'MOST_IMP_COL'=>0, 'EXIM'=>1,  'NOT_NULL'=>0, 'POS'=>7.5, 'TABLE_NAME'=>'CHIP_READER', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'RES_GROUP_ID', 'NICE_NAME'=>'responsible group', 
		    'CCT_TABLE_NAME'=>'USER_GROUP', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'EXIM'=>1, 'NOT_NULL'=>0, 'POS'=>7.6, 'TABLE_NAME'=>'CHIP_READER', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		
		$insarr[]="/* <TABLE> LINK: new: DATAX 2021-04-24 */";
		$insarr[]="ALTER TABLE LINK add DATAX VARCHAR(32000)";
		$insarr[]="comment on column LINK.DATAX is 'JSON data'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'LINK', 'COLUMN_NAME'=>'DATAX', 'NICE_NAME'=>'JSON data', 'CCT_TABLE_NAME'=>NULL,
		    'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>9, 'VISIBLE'=>1, 'EDITABLE'=>0) );
		
		$insarr[]="/* <TABLE> CONT_LOG: new: DATAX 2021-05-03 */";
		$insarr[]="ALTER TABLE CONT_LOG add DATAX VARCHAR(255)";
		$insarr[]="comment on column CONT_LOG.DATAX is 'JSON data'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'CONT_LOG', 'COLUMN_NAME'=>'DATAX', 'NICE_NAME'=>'JSON data', 'CCT_TABLE_NAME'=>NULL,
		    'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>7, 'VISIBLE'=>1, 'EDITABLE'=>0) );
		
		$insarr[]="/* <TABLE> CCT_COLUMN : NEW column 2021-06-02 */";
		$insarr[]="ALTER TABLE CCT_COLUMN add DATAX VARCHAR(4000)";
		$insarr[]="comment on column CCT_COLUMN.DATAX is 'JSON data'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'CCT_COLUMN', 'COLUMN_NAME'=>'DATAX', 'NICE_NAME'=>'JSON data', 'CCT_TABLE_NAME'=>NULL,
		    'APP_DATA_TYPE_ID'=>19, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>15.5, 'VISIBLE'=>1, 'EDITABLE'=>0) );
		
		$insarr[]="/* <TABLE> CONTAINER : NEW column 2021-07-12 */";
		$insarr[]="ALTER TABLE CONTAINER add LOCATION VARCHAR(255)";
		$insarr[]="comment on column CONTAINER.LOCATION is 'location'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'CONTAINER', 'COLUMN_NAME'=>'LOCATION', 'NICE_NAME'=>'location', 'CCT_TABLE_NAME'=>NULL,
		    'APP_DATA_TYPE_ID'=>2, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>5.7, 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		
		$insarr[]="/* <TABLE> AS_HAS_PR : NEW column 2021-07-15 */";
		$insarr[]="ALTER TABLE AS_HAS_PR add DATAX VARCHAR(4000)";
		$insarr[]="comment on column AS_HAS_PR.DATAX is 'JSON data'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'AS_HAS_PR', 'COLUMN_NAME'=>'DATAX', 'NICE_NAME'=>'JSON data', 'CCT_TABLE_NAME'=>NULL,
		    'APP_DATA_TYPE_ID'=>19, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>8, 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		
		$insarr[]="/* <TABLE> ABSTRACT_SUBST : NEW column 2021-08-11 */";
		$insarr[]="ALTER TABLE ABSTRACT_SUBST add PROORI_CON_ID BIGINT";
		$insarr[]="comment on column ABSTRACT_SUBST.PROORI_CON_ID is 'id of original producer contact'";
		$insarr[]="comment on column ABSTRACT_SUBST.PROD_CONTACT_ID is 'id of supplier contact'";
		
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'ABSTRACT_SUBST', 'COLUMN_NAME'=>'PROORI_CON_ID', 'NICE_NAME'=>'original producer', 'CCT_TABLE_NAME'=>'CONTACT',
		    'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>9, 'VISIBLE'=>2, 'EDITABLE'=>1) );
		
		
		$insarr[]="/* <TABLE> DB_USER : NEW column CCT_ACCESS_ID 2021-09-06 */";
		$insarr[]="ALTER TABLE DB_USER add CCT_ACCESS_ID BIGINT";
		$insarr[]="comment on column DB_USER.CCT_ACCESS_ID is 'id of the access rights'";
		$insarr[]="alter table DB_USER \n".
		  		" add constraint FK_CCT_ACCESS2DB_USER foreign key (CCT_ACCESS_ID) \n".
		  		" references CCT_ACCESS (CCT_ACCESS_ID)\n".
		  		" on delete cascade";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'CCT_ACCESS_ID', 'NICE_NAME'=>'cct access id', 'CCT_TABLE_NAME'=>'CCT_ACCESS', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>12.5, 'TABLE_NAME'=>'DB_USER', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		
		$insarr[]="/* <TABLE> SUC_BOM: NEW column APOS, ABSTRACT_SUBST:CERT_FLAG 2021-09-24 */";
		$insarr[]="ALTER TABLE SUC_BOM add APOS INT";
		$insarr[]="comment on column SUC_BOM.APOS is 'POS of SUA_BOM (not unique here)'";
		$insarr[]="ALTER TABLE ABSTRACT_SUBST add CERT_FLAG INT";
		$insarr[]="comment on column ABSTRACT_SUBST.CERT_FLAG is 'life cycle status [1,2,3]'";
		
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'APOS', 'NICE_NAME'=>'SUA_BOM pos', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>5, 'TABLE_NAME'=>'SUC_BOM', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'CERT_FLAG', 'NICE_NAME'=>'life cycle status', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>15, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>22, 'TABLE_NAME'=>'ABSTRACT_SUBST', 'VISIBLE'=>2, 'EDITABLE'=>1) );
		
		// -------------------

		$meta[] = array( "act"=>"upd", "tab"=>"GLOBALS", "vals"=>array("VALUE"=>$dataArray["reqs"]["DbVersionNew"]), "ids"=>array("NAME"=>"DBVersion" ) );
		
		
		$dataArray["sql"]  = &$insarr;
		$dataArray["meta"] = &$meta;
	}
	
	function transform($sqlo, $sqlo2, $part, $go) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    if ($go<1) return;
	    

	    if ( oCCT_TABLE_info::column_exists($sqlo, 'DB_USER', 'CCT_ACCESS_ID') ) {

    	    $this->infoLine('Start transform DB_USER: add CCT_ACCESS.');
    	    
    	    $tablename='DB_USER';
    	    $sqlsel = "DB_USER_ID from DB_USER where CCT_ACCESS_ID is NULL order by DB_USER_ID";
    	    $sqlo2->Quesel($sqlsel);
    	    $cnt=0;
    	    while ( $sqlo2->ReadRow() ) {
    	        $user_id = $sqlo2->RowData[0];
    	        $access_id = insert_access( $sqlo, $tablename);
    	        $argu=array('CCT_ACCESS_ID'=>$access_id);
    	        $idarr = array('DB_USER_ID'=>$user_id);
    	        gObjUpdate::update_row_s($sqlo, $tablename, $argu, $idarr);
    	        if ($error->Got(READONLY))  {
    	            $error->set( $FUNCNAME, 1, 'Error on DB_USER_ID:'. $user_id);
    	            return;
    	        }
    	        $cnt++;
    	    }
    	    $this->infoLine('Transform END: touched DB_USERs: '.$cnt);
	    }
	    
	    if ( oCCT_TABLE_info::column_exists($sqlo, 'SUC_BOM', 'APOS') ) {

	        // UPDATE make APOS == POS for old data
	        $this->infoLine('Start transform SUC_BOM::APOS.');
	        $tablename='SUC_BOM';
	        $sqlsel = "CONCRETE_SUBST_ID, POS, COMPO_SUC_ID from SUC_BOM where APOS is NULL order by CONCRETE_SUBST_ID, POS";
	        $sqlo2->Quesel($sqlsel);
	        $cnt=0;
	        while ( $sqlo2->ReadArray() ) {
	            
	            $idarr = $sqlo2->RowData;
	            $argu=array('APOS'=>$idarr['POS']);
	            gObjUpdate::update_row_s($sqlo, $tablename, $argu, $idarr);
	            if ($error->Got(READONLY))  {
	                $error->set( $FUNCNAME, 1, 'Error on :'. print_r($idarr,1));
	                return;
	            }
	            $cnt++;
	        }
	        $this->infoLine('Transform END: SUC_BOM::APOS Elements updated:'.$cnt);
	    }
	}
	

}
