<?php
/*
 *  initial Blink changes
 *  @swreq 
 *  @version $Header:   2018-11-21 09:04:09Z $
 */
class gDB_tranData_2_0_0_0 extends gDB_transAbstract {

	
	function getBasicInfo() {
		$dataArray["info"] = "initial Blink changes";
		$dataArray["reqs"] = array(
				"DbVersionNeed" => NULL, // no predecesor needed here, but MINIMUM should be 1.0.2.1
				"DbVersionNew"  => "2.0.0.0 - 20181212 - 09:20");
		// $dataArray['extconvert'] = array( 'parts'=> 2, 'transform'=>1 ) ;
		$this->dataArray = $dataArray;
		return $dataArray;
	}
	
	
	
	function getData( &$dataArray, $option=NULL ) {
		$go = $option['go'];
		$dataArray = $this->dataArray;
				/**
         * MODEL changes:
         * 
		 */
		
		$insarr	= NULL;
		$meta	= NULL;

		
		// <TABLE> CCT_TABLE
		/*
		 * CCT_TABLE: [OK]
         *   COLUMNS:
         *     IS_ACTIVE: INT: (DEFAULT: 1) is table active? 1: yes, -1: no
         *     NOTES: VARCHAR: notes of table: initial inherit from SQL comment
		 */
		$insarr[]="alter table CCT_TABLE add ( \n".
		   " IS_ACTIVE NUMBER DEFAULT 1,\n".
		   " NOTES VARCHAR(4000)\n".
		   ")";
		
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'IS_ACTIVE', 'NICE_NAME'=>'active', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>15, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>8, 'TABLE_NAME'=>'CCT_TABLE', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'NOTES', 'NICE_NAME'=>'notes', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>3, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>15, 'TABLE_NAME'=>'CCT_TABLE', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		

		/*
		*  <TABLE> CCT_COLUMN: 
		*   COLUMNS:
		*     NOTES: notes of column
		*/
		$insarr[]="alter table CCT_COLUMN add ( \n".
				" NOTES VARCHAR(4000)\n".
				")";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'NOTES', 'NICE_NAME'=>'notes', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>3, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>16, 'TABLE_NAME'=>'CCT_COLUMN', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		
		
			
		// <TABLE> EXP
		/*
		 * EXP: 
         * 	COLUMNS:
         *    [DROP] CONCRETE_ARRAY_ID
         * 	  [DROP] SOCKET_ID2
         *    
         * 	  IMG_ID: Link to image object
     	 * 	  [nicename] SOCKET_ID: NICE: ID of cartridge 
		 */
		$insarr[]="alter table EXP drop COLUMN CONCRETE_ARRAY_ID";
		$insarr[]="alter table EXP drop COLUMN SOCKET_ID2";
		$insarr[]="alter table EXP add ( \n".
				" IMG_ID NUMBER \n".
				")";
		
		$insarr[]="alter table EXP \n".
				" add constraint FK_IMG2EXP foreign key (IMG_ID) \n".
				" references IMG (IMG_ID)";
		
		// EXP
		$meta[] = array( "act"=>"del", "tab"=>"CCT_COLUMN", "ids"=>array("TABLE_NAME"=>"EXP", 'COLUMN_NAME'=>'CONCRETE_ARRAY_ID' ) );
		$meta[] = array( "act"=>"del", "tab"=>"CCT_COLUMN", "ids"=>array("TABLE_NAME"=>"EXP", 'COLUMN_NAME'=>'SOCKET_ID2' ) );
		$meta[] = array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'IMG_ID', 'NICE_NAME'=>'id of image', 'CCT_TABLE_NAME'=>'IMG', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>11.5, 'TABLE_NAME'=>'EXP', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[] = array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'EXP', 'COLUMN_NAME'=>'SOCKET_ID', 'NICE_NAME'=>'ID of cartridge') );
		
		
		
			
		/**
		 * 
         * <TABLE> IMG_OF_EXP: DROP table
         * TBD: remove from CCT_TABLE ?
         * 
		 */
		$insarr[]="delete from CCT_COLUMN where TABLE_NAME='IMG_OF_EXP'";
		$insarr[]="delete from CCT_TABLE  where TABLE_NAME='IMG_OF_EXP'";
		$insarr[]="drop table IMG_OF_EXP";
		
		
		
		/**
		 *  <TABLE> SOCKET: 
         *    - nice name: cartridge
         *    - delete columns: QUALITY, PROD_NOTES
         *    - TBD: new table SOCKET_H_SUC (cartridge has SUCs) ???
		 */
		$insarr[]="alter table SOCKET drop (QUALITY, PROD_NOTES)";
		$insarr[]="comment on table SOCKET is 'physical cartridge'";
		$insarr[]="comment on column SOCKET.ABS_SOCKET_ID is  'id of cartridge (abstract)'";
		
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_TABLE', 'vals'=>array('TABLE_NAME'=>'SOCKET', 'NICE_NAME'=>'cartridge') );
		
		
		
		
		/**
         *  <TABLE> ABS_SOCKET:
         *    - nice name: cartridge (abstract)
         *    - delete columns:  ABSTRACT_ARRAY_ID
         *    - [nice] column: H_SOCK_TYPE_ID : link to cartidge type: defines geometry
         *    - TBD: column: REACT_CH_ID : link to reactor chamber => ABSTRACT_SUBST_ID
         *    - TBD: column: SAMPLE_MOD_ID : link to sample module ID ? link to reactor chamber => ABSTRACT_SUBST_ID
         *    - new ASSOC: ABS_CART_H_SUA
		 */
			
		
		$insarr[]="alter table ABS_SOCKET drop COLUMN  ABSTRACT_ARRAY_ID";
		$insarr[]="comment on column ABS_SOCKET.H_SOCK_TYPE_ID is 'id of cartridge type'";
		$insarr[]="comment on column ABS_SOCKET.ABS_SOCKET_ID is  'id of cartridge (abstract)'";
		$insarr[]="comment on column ABS_SOCKET.NAME is  'name'";
		
		$meta[] = array( "act"=>"del", "tab"=>"CCT_COLUMN", "ids"=>array("TABLE_NAME"=>"ABS_SOCKET", 'COLUMN_NAME'=>'ABSTRACT_ARRAY_ID' ) );
		$meta[] = array( 'act'=>'insupd', 'tab'=>'CCT_TABLE',  'vals'=>array('TABLE_NAME'=>'ABS_SOCKET', 'NICE_NAME'=>'cartridge (abstract)') );
		$meta[] = array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('TABLE_NAME'=>'ABS_SOCKET','COLUMN_NAME'=>'H_SOCK_TYPE_ID', 'NICE_NAME'=>'id of cartidge type') );
		
		
		
			
		/**
		 *    
         *  <TABLE> ABS_CART_H_SUA: (new table) NICE:  
         *  - nice name: cartridge (abstract) has reagents
         *  COLUMNS:
         *    - ABS_SOCKET_ID: link to mother    (PK1)
         *    - POS: INT : position in cartridge (PK2)
         *    - ABSTRACT_SUBST_ID: INT : ID of substance (abstract)
         *    - KEY: varchar : a key
         *    - NOTES : varchar : notes
		 */
		$insarr[]="create table ABS_CART_H_SUA ( \n".
				" ABS_SOCKET_ID NUMBER not null,\n".
				" STEP NUMBER not null,\n".
				" POS  NUMBER not null,\n".
				" ABSTRACT_SUBST_ID NUMBER not null,\n".
				" KEY VARCHAR(255),\n".
				" NOTES VARCHAR(4000),\n".
				" constraint PK_ABS_CART_H_SUA primary key (ABS_SOCKET_ID, STEP) \n".
				")";
		$insarr[]="comment on table ABS_CART_H_SUA is 'cartridge (abstract) has reagents'";
		$insarr[]="comment on column ABS_CART_H_SUA.ABS_SOCKET_ID is 'id of cart (abstract) has reagents'";
		$insarr[]="comment on column ABS_CART_H_SUA.STEP is 'relative identifier of component'";
		$insarr[]="comment on column ABS_CART_H_SUA.POS  is 'position of component in cart'";
		$insarr[]="comment on column ABS_CART_H_SUA.ABSTRACT_SUBST_ID is 'link to the reagent'";
		$insarr[]="comment on column ABS_CART_H_SUA.KEY is 'optional key'";
		$insarr[]="comment on column ABS_CART_H_SUA.NOTES is 'notes'";
		$insarr[]="/* indices */";
		$insarr[]="/* constraints */";
		$insarr[]="alter table ABS_CART_H_SUA \n".
				" add constraint FK_ABS_SOCKET2ABS_CART_H_SUA foreign key (ABS_SOCKET_ID) \n".
				" references ABS_SOCKET (ABS_SOCKET_ID)\n".
				" on delete cascade";
		$insarr[]="alter table ABS_CART_H_SUA \n".
				" add constraint FK_ABSTRACT_SUB2ABS_CART_H_SUA foreign key (ABSTRACT_SUBST_ID) \n".
				" references ABSTRACT_SUBST (ABSTRACT_SUBST_ID)";
		
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_TABLE', 'vals'=>array('TABLE_NAME'=>'ABS_CART_H_SUA', 'NICE_NAME'=>'cart (abstract) has reagents', 'CCT_TABLE_NAME'=>'ABS_SOCKET', 'TABLE_TYPE'=>'BO_ASSOC', 'IS_VIEW'=>0, 'INTERNAL'=>0, 'EXIM'=>0) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'ABS_SOCKET_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'ABS_SOCKET', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>1, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>1, 'TABLE_NAME'=>'ABS_CART_H_SUA', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'STEP', 'NICE_NAME'=>'step', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>15, 'PRIMARY_KEY'=>2, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>2, 'TABLE_NAME'=>'ABS_CART_H_SUA', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'POS', 'NICE_NAME'=>'pos of reagent', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>15, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>2.5, 'TABLE_NAME'=>'ABS_CART_H_SUA', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'ABSTRACT_SUBST_ID', 'NICE_NAME'=>'id of substance (abstract)', 'CCT_TABLE_NAME'=>'ABSTRACT_SUBST', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>3, 'TABLE_NAME'=>'ABS_CART_H_SUA', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'KEY', 'NICE_NAME'=>'key', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>6, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>4, 'TABLE_NAME'=>'ABS_CART_H_SUA', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'NOTES', 'NICE_NAME'=>'notes', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>3, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>5, 'TABLE_NAME'=>'ABS_CART_H_SUA', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		
		/**
		 *
		 *  <TABLE> CART_H_SUC: (new table) NICE:
		 *  - nice name: cartridge has reagents
		 *  COLUMNS:
		 *    - SOCKET_ID: link to mother    (PK1)
		 *    - STEP: INT : STEP in cartridge (PK2)
		 - POS: INT : STEP in cartridge
		 *    - CONCRETE_SUBST_ID: INT : ID of substance (abstract)
		 *    - KEY
		 */
		$insarr[]="create table CART_H_SUC ( \n".
				" SOCKET_ID NUMBER not null,\n".
				" STEP NUMBER not null,\n".
				" POS  NUMBER not null,\n".
				" CONCRETE_SUBST_ID NUMBER not null,\n".
				" KEY VARCHAR(255),\n".
				" constraint PK_CART_H_SUC primary key (SOCKET_ID, STEP) \n".
				")";
		$insarr[]="comment on table CART_H_SUC is 'cartridge (abstract) has reagents'";
		$insarr[]="comment on column CART_H_SUC.SOCKET_ID is 'id of cart (abstract) has reagents'";
		$insarr[]="comment on column CART_H_SUC.STEP is 'relative identifier of component'";
		$insarr[]="comment on column CART_H_SUC.POS  is 'position of component in cart'";
		$insarr[]="comment on column CART_H_SUC.CONCRETE_SUBST_ID is 'link to the reagent'";
		$insarr[]="comment on column CART_H_SUC.KEY is 'optional key'";
		
		$insarr[]="/* indices */";
		$insarr[]="/* constraints */";
		$insarr[]="alter table CART_H_SUC \n".
				" add constraint FK_SOCKET2CART_H_SUC foreign key (SOCKET_ID) \n".
				" references SOCKET (SOCKET_ID)\n".
				" on delete cascade";
		$insarr[]="alter table CART_H_SUC \n".
				" add constraint FK_CONCRETE_SUB2CART_H_SUC foreign key (CONCRETE_SUBST_ID) \n".
				" references CONCRETE_SUBST (CONCRETE_SUBST_ID)";
		
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_TABLE', 'vals'=>array('TABLE_NAME'=>'CART_H_SUC', 'NICE_NAME'=>'cart has reagents', 'CCT_TABLE_NAME'=>'SOCKET', 'TABLE_TYPE'=>'BO_ASSOC', 'IS_VIEW'=>0, 'INTERNAL'=>0, 'EXIM'=>0) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'SOCKET_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'SOCKET', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>1, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>1, 'TABLE_NAME'=>'CART_H_SUC', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'STEP', 'NICE_NAME'=>'step', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>15, 'PRIMARY_KEY'=>2, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>2, 'TABLE_NAME'=>'CART_H_SUC', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'POS', 'NICE_NAME'=>'pos of reagent', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>15, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>2.5, 'TABLE_NAME'=>'CART_H_SUC', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'CONCRETE_SUBST_ID', 'NICE_NAME'=>'id of substance (concrete)', 'CCT_TABLE_NAME'=>'CONCRETE_SUBST', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>3, 'TABLE_NAME'=>'CART_H_SUC', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'KEY', 'NICE_NAME'=>'key', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>6, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>4, 'TABLE_NAME'=>'CART_H_SUC', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		
		
		
		/**
		 * <TABLE> SUA_BOM: substance (abstract) BOM
         *  - columns:
         *    ABSTRACT_SUBST_ID : int (PK1) - mother SUA
         *    POS : POS in BOM  : int (PK2) - POS in BOM
         *    COMPO_SUA_ID : component      - component SUA
         *    QUANTITY : float - quantity of component
		 */
		$insarr[]="create table SUA_BOM ( \n".
				" ABSTRACT_SUBST_ID NUMBER not null,\n".
				" POS NUMBER not null,\n".
				" COMPO_SUA_ID NUMBER not null,\n".
				" QUANTITY NUMBER,\n".
				" KEY VARCHAR(4000),\n".
				" constraint PK_SUA_BOM primary key (ABSTRACT_SUBST_ID, POS) \n".
				")";
		$insarr[]="comment on table SUA_BOM is 'Bill of material'";
		$insarr[]="comment on column SUA_BOM.ABSTRACT_SUBST_ID is 'id of BOM of substance (abstract)'";
		$insarr[]="comment on column SUA_BOM.POS is 'position of component'";
		$insarr[]="comment on column SUA_BOM.COMPO_SUA_ID is 'link to component'";
		$insarr[]="comment on column SUA_BOM.QUANTITY is 'quantity of component'";
		$insarr[]="comment on column SUA_BOM.KEY is 'optional key'";
		$insarr[]="/* indices */";
		$insarr[]="create index SUA_BOM_AK2 on SUA_BOM (POS ASC)";
		$insarr[]="/* constraints */";
		$insarr[]="alter table SUA_BOM \n".
				" add constraint FK_ABSTRACT_SUB12SUA_BOM foreign key (ABSTRACT_SUBST_ID) \n".
				" references ABSTRACT_SUBST (ABSTRACT_SUBST_ID)\n".
				" on delete cascade";
		$insarr[]="alter table SUA_BOM \n".
				" add constraint FK_ABSTRACT_SUB22SUA_BOM foreign key (COMPO_SUA_ID) \n".
				" references ABSTRACT_SUBST (ABSTRACT_SUBST_ID)\n".
				" on delete cascade";
		
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_TABLE', 'vals'=>array('TABLE_NAME'=>'SUA_BOM', 'NICE_NAME'=>'BOM of substance (abstract)', 'CCT_TABLE_NAME'=>'ABSTRACT_SUBST', 'TABLE_TYPE'=>'BO_ASSOC', 'IS_VIEW'=>0, 'INTERNAL'=>0, 'EXIM'=>0) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'ABSTRACT_SUBST_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'ABSTRACT_SUBST', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>1, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>1, 'TABLE_NAME'=>'SUA_BOM', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'POS', 'NICE_NAME'=>'position', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>15, 'PRIMARY_KEY'=>2, 'MOST_IMP_COL'=>1, 'NOT_NULL'=>1, 'POS'=>2, 'TABLE_NAME'=>'SUA_BOM', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'COMPO_SUA_ID', 'NICE_NAME'=>'component SUA', 'CCT_TABLE_NAME'=>'ABSTRACT_SUBST', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>3, 'TABLE_NAME'=>'SUA_BOM', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'QUANTITY', 'NICE_NAME'=>'quantity', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>5, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>4, 'TABLE_NAME'=>'SUA_BOM', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'KEY', 'NICE_NAME'=>'key', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>6, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>5, 'TABLE_NAME'=>'SUA_BOM', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		
		
			
		/**
		 * <table> REA_LOG
		 * TBD: change code in Partisan ...
		 * COLUMN: 
		 *  [drop] USERX
		 *  [add] DB_USER_ID
		 */
		$insarr[]="alter table REA_LOG drop COLUMN USERX";
		$insarr[]="alter table REA_LOG add ( \n".
				" DB_USER_ID NUMBER \n".
				")";
		$insarr[]="comment on column REA_LOG.DB_USER_ID is 'id of user'";
		
		$insarr[]="alter table REA_LOG \n".
				" add constraint FK_DB_USER2REA_LOG foreign key (DB_USER_ID) \n".
				" references DB_USER (DB_USER_ID)";
		
		$meta[]=array( 'act'=>'del', 'tab'=>'CCT_COLUMN', "ids"=>array('COLUMN_NAME'=>'USERX','TABLE_NAME'=>'REA_LOG') );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'DB_USER_ID', 'NICE_NAME'=>'id of user', 'CCT_TABLE_NAME'=>'DB_USER', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>5, 'TABLE_NAME'=>'REA_LOG', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		// --------------------------
		
		
		
		/**
		 * <table> SUC_BOM
		 * - allow many SUCs for one SUA-BOM POS !
		 * 
		 */
		$insarr[]="create table SUC_BOM ( \n".
		  		" CONCRETE_SUBST_ID NUMBER not null,\n".
		  		" POS NUMBER not null,\n".
		  		" COMPO_SUC_ID NUMBER not null,\n".
		  		" QUANTITY NUMBER,\n".
		  		" constraint PK_SUC_BOM primary key (CONCRETE_SUBST_ID, POS, COMPO_SUC_ID) \n".
		  		")";
		$insarr[]="comment on table SUC_BOM is 'BOM of substance (concrete)'";
		$insarr[]="comment on column SUC_BOM.CONCRETE_SUBST_ID is 'id of BOM of substance (concrete) '";
		$insarr[]="comment on column SUC_BOM.POS is 'position in SUA BOM'";
		$insarr[]="comment on column SUC_BOM.COMPO_SUC_ID is 'id of substance (concrete)'";
		$insarr[]="comment on column SUC_BOM.QUANTITY is 'quantity of component'";
		$insarr[]="/* indices */";
		$insarr[]="/* constraints */";
		$insarr[]="alter table SUC_BOM \n".
		  		" add constraint FK_CONCRETE_SUBST2SUC_BOM foreign key (CONCRETE_SUBST_ID) \n".
		  		" references CONCRETE_SUBST (CONCRETE_SUBST_ID)\n".
		  		" on delete cascade";
		$insarr[]="alter table SUC_BOM \n".
		  		" add constraint FK_CONCRETE_SUBS22SUC_BOM foreign key (COMPO_SUC_ID ) \n".
		  		" references CONCRETE_SUBST (CONCRETE_SUBST_ID )\n".
		  		" on delete cascade";
		
	
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_TABLE', 'vals'=>array('TABLE_NAME'=>'SUC_BOM', 'NICE_NAME'=>'BOM of substance (concrete) ', 'CCT_TABLE_NAME'=>'CONCRETE_SUBST', 'TABLE_TYPE'=>'BO_ASSOC', 'IS_VIEW'=>0, 'INTERNAL'=>0, 'EXIM'=>0) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'CONCRETE_SUBST_ID', 'NICE_NAME'=>NULL, 'CCT_TABLE_NAME'=>'CONCRETE_SUBST', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>1, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>1, 'TABLE_NAME'=>'SUC_BOM', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'POS', 'NICE_NAME'=>'POS in SUA BOM', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>15, 'PRIMARY_KEY'=>2, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>3, 'TABLE_NAME'=>'SUC_BOM', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'COMPO_SUC_ID', 'NICE_NAME'=>'component', 'CCT_TABLE_NAME'=>'CONCRETE_SUBST', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>3, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>1, 'POS'=>4, 'TABLE_NAME'=>'SUC_BOM', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'QUANTITY', 'NICE_NAME'=>'quantity', 'CCT_TABLE_NAME'=>NULL, 'APP_DATA_TYPE_ID'=>5, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>5, 'TABLE_NAME'=>'SUC_BOM', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		/**
		 * <TABLE> ABSTRACT_SUBST
		 * - allow many SUCs for one SUA-BOM POS !
		 *
		 */
		$insarr[] = 'alter table ABSTRACT_SUBST add (EXPIRY_DAYS NUMBER)';	
		$insarr[]="comment on column ABSTRACT_SUBST.EXPIRY_DAYS is 'planned Expiry Days of substance (concrete)'";
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'EXPIRY_DAYS', 'NICE_NAME'=>'ExpiryDays', 'CCT_TABLE_NAME'=>NULL, 
		    'APP_DATA_TYPE_ID'=>5, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>11, 'TABLE_NAME'=>'ABSTRACT_SUBST', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		$insarr[] = 'alter table ABSTRACT_SUBST drop (EXTRA_OBJ2_ID)';
		$meta[]=array( 'act'=>'del', 'tab'=>'CCT_COLUMN', 'ids'=>array('COLUMN_NAME'=>'EXTRA_OBJ2_ID', 'TABLE_NAME'=>'ABSTRACT_SUBST') );

		
		
		/**
		 * <TABLE> EXP_TMPL
		 * - allow many SUCs for one SUA-BOM POS !
		 *
		 */
		$insarr[] = 'alter table EXP_TMPL drop (ABSTRACT_ARRAY_ID, H_STATE_ID)';
		$insarr[] = "alter table EXP_TMPL add ( \n".
		  		"   ABS_SOCKET_ID  NUMBER \n".
		  		")";
		$insarr[]="comment on column EXP_TMPL.ABS_SOCKET_ID is 'link to cartidge (abstract)'";
		$insarr[]="alter table EXP_TMPL \n".
		  		" add constraint FK_ABS_SOCKET2EXP_TMPL foreign key (ABS_SOCKET_ID) \n".
		  		" references ABS_SOCKET (ABS_SOCKET_ID)";
		
		$meta[]=array( 'act'=>'del', 'tab'=>'CCT_COLUMN', 'ids'=>array('COLUMN_NAME'=>'ABSTRACT_ARRAY_ID', 'TABLE_NAME'=>'EXP_TMPL') );
		$meta[]=array( 'act'=>'del', 'tab'=>'CCT_COLUMN', 'ids'=>array('COLUMN_NAME'=>'H_STATE_ID', 'TABLE_NAME'=>'EXP_TMPL') );
		$meta[]=array( 'act'=>'insupd', 'tab'=>'CCT_COLUMN', 'vals'=>array('COLUMN_NAME'=>'ABS_SOCKET_ID', 'NICE_NAME'=>'cartidge (abstract)', 'CCT_TABLE_NAME'=>'ABS_SOCKET', 'APP_DATA_TYPE_ID'=>1, 'PRIMARY_KEY'=>0, 'MOST_IMP_COL'=>0, 'NOT_NULL'=>0, 'POS'=>4, 'TABLE_NAME'=>'EXP_TMPL', 'VISIBLE'=>1, 'EDITABLE'=>1) );
		
		// conly update CCT_ACC_UP, when a AUDIT_LOG was started ..
		$insarr[] = "create or replace trigger CCT_ACCESS_UPI2 \n".
		  		" AFTER UPDATE of MOD_DATE\n".
		  		" on CCT_ACCESS \n".
		  		" for each row \n".
		  		"   DECLARE
                    	userid number;
                    BEGIN
                    	if (:old.TABLE_NAME='PROJ') then
                    		return;
                    	end if;
		  		    
                    	userid := :new.DB__DB_USER_ID;
                    	if ( :new.DB__DB_USER_ID is NULL ) then
                    		userid := :old.DB_USER_ID;
                    	end if;
		  		    
                    	if ( :new.MOD_DATE is not NULL AND :old.H_ALOG_ACT_ID > 0 ) then
		  		    
                    		INSERT INTO CCT_ACC_UP ( CCT_ACCESS_ID, POS, MODI_DATE, DB_USER_ID, UPINFO )
                    			VALUES (:new.CCT_ACCESS_ID, NULL, :new.MOD_DATE, userid, :new.UPINFO );
                    	end if;
                   END;
                ";
		$insarr[] = "alter trigger CCT_ACCESS_UPI2 compile";
		
		
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
