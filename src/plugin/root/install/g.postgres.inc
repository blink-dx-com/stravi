<?php

/**
 * install postgres model
 * CHANGES:
 *   - create views
 *   - add sequences
 *   - change DATA_Types
 * @package g.postgres.inc
 * @author  Steffen Kube (steffen.kube@gmx.de)
 */
class  g_postgres_XPL extends gPlugin {
    
    private $_infolevel = 0;


	function register() {
		
		$title		  = 'Init Postgres Model';

		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'tool';
		$this->infoarr['design']   = 'norm';
		$this->infoarr['locrow']   = array( array('rootsubs/rootFuncs.php', 'home') );

		$this->infoarr2 = array(
				'onlyAdmin'=>1
		);

	}

	/**
	 * - creates a form with one text field "Min-value"
	 * - creates a HTML-page parameter $_REQUEST['parx']['minval'] after SUBMIT
	 */
	function form1() {

		$initarr   = NULL;
		$initarr["title"]       = "Init!"; // title of form
		$initarr["submittitle"] = "Next &gt;"; // title of submit button
		$initarr["tabwidth"]    = "AUTO";   // table-width: AUTO

		$hiddenarr = NULL; // hidden form variables
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
		
		$fieldx = array (
		    "title" => "Infolevel",
		    "name"  => "infolevel",
		    "object"=> "text",
		    "val"   => $this->parx['infolevel'],
		   
		    "notes" => "0,1,2,"
		);
		$formobj->fieldOut( $fieldx );
		
		$formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	/**
	 * - creates a form with one text field "Min-value"
	 * - creates a HTML-page parameter $_REQUEST['parx']['minval'] after SUBMIT
	 */
	function form2() {
	    
	    $initarr   = NULL;
	    $initarr["title"]       = "Init!"; // title of form
	    $initarr["submittitle"] = "Upgrade"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    
	    $hiddenarr = NULL; // hidden form variables
	    $hiddenarr['parx[infolevel]'] = $this->parx['infolevel'];
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 1);
	    $formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	private function _alter_exe($sql_cmd) {
	    if ($this->_infolevel>0) {
	        echo 'SQL-INFO: <pre>'.$sql_cmd."</pre><br>\n";
	    }
	    if ($this->go==2)  {
	        $this->_sqlo_INT->Query($sql_cmd);
	    }
	}
	
	private function __one_sequ(&$sqlo, $table, $pkname) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $sqlsel = "max(".$pkname.") from ".$table;
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    $maxval = $sqlo->RowData[0];
	    
	    if ($maxval!=NULL and !is_numeric($maxval) ) {
	        echo 'INFO: Table '.$table.' has no numeric PK. IGNORED. <br>';
	        return;
	    }
	    
	    $next_val = $maxval + 1;
	    
	    echo '- T: '.$table."<br>";
	    $seq_name = strtolower($pkname).'_seq';
	    $sql_cmd = 'CREATE SEQUENCE '.$seq_name.' MINVALUE '.$next_val;
	    $this->_alter_exe($sql_cmd, $FUNCNAME);
	    
	    
	    $sql_cmd = "ALTER TABLE ".$table."  ALTER ".$pkname." SET DEFAULT nextval('".$seq_name."')";
	    $this->_alter_exe($sql_cmd, $FUNCNAME);
	    
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 1, 'Error on sequence setting.' );
	        return;
	    }
	}
	
	private function _ch_sequences($sqlo) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    
	    foreach($this->tables as $table) {
	        
	        if ($table=='CCT_ACCESS') {
	            continue;
	        }
	        
	        $pkname = '';
	        $sqlsel = "count(1) from CCT_COLUMN where TABLE_NAME=".$sqlo->addQuotes($table). ' and PRIMARY_KEY>0';
	        $sqlo->Quesel($sqlsel);
	        $sqlo->ReadRow();
	        $cnt = $sqlo->RowData[0];
	        
	        $sqlsel = "COLUMN_NAME from CCT_COLUMN where TABLE_NAME=".$sqlo->addQuotes($table). ' and PRIMARY_KEY=1';
	        $sqlo->Quesel($sqlsel);
	        $sqlo->ReadRow();
	        $pkname = $sqlo->RowData[0];
	        
	        if (!$pkname) {
	            echo 'INFO: no PK for table '.$table.'.';
	            continue;
	        }
	        
	        if ($cnt>1) continue; // no sequence needed
	        
	        $this->__one_sequ($sqlo, $table, $pkname);
	        if ($error->Got(READONLY))  {
	            $error->set( $FUNCNAME, 2, 'Error on sequence setting for table '.$table.'.' );
	            return;
	        }
	    }
	}
	
	/**
	 * data type of EXTRA_OBJ ...
	 * @param object $sqlo
	 */
	private function _ch_views($sqlo) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $insarr = array();
	    
	    $insarr[] = '
create or replace view sample_in_exp as
 select cps.concrete_subst_id sample_concrete_subst_id,
       cps.concrete_proto_id,
       ehp.exp_id
from abstract_proto_step aps,
     concrete_proto_step cps,
     exp_has_proto ehp
where aps.is_sample = 1
 and aps.abstract_proto_id = cps.abstract_proto_id
 and aps.step_nr = cps.step_nr
 and cps.concrete_subst_id is not NULL
 and cps.concrete_proto_id = ehp.concrete_proto_id
        ';
	    
	    
	}
	
	private function _ch_trigger($sqlo) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $insarr = array();
	    
	    //
	    // CCT_ACCLOG_aic TRIGGER
	    //
	    $insarr[] = '
        CREATE  OR REPLACE FUNCTION CCT_ACCLOG_aic_p () RETURNS trigger AS $CCT_ACCLOG_aic_p$ 
        DECLARE
            posvar integer;
        BEGIN
        	
        	if ( new.POS is NULL ) then
        		select max(POS) into posvar from CCT_ACCLOG 
        		where cct_access_id=new.cct_access_id;
        		if (posvar is NULL) then
        			new.POS := 1;
        		else 
        			new.POS := posvar + 1;
        		end if; 
        	end if; 
          RETURN NEW; 
        END;
        $CCT_ACCLOG_aic_p$  LANGUAGE PLPGSQL;
        ';
	    
	    $insarr[] = 
	    "CREATE TRIGGER CCT_ACCLOG_aic \n".
	    "before insert on CCT_ACCLOG\n".
	    "FOR EACH ROW\n".
	    "EXECUTE PROCEDURE CCT_ACCLOG_aic_p( );";
	    
	    //
	    // CCT_ACC_UP_AIC TRIGGER
	    //
	    $insarr[] = '
	    CREATE  OR REPLACE FUNCTION CCT_ACC_UP_AIC_p () RETURNS trigger AS $CCT_ACC_UP_AIC_p$
	    DECLARE
	       posvar integer;
	    BEGIN
	    
    	    if ( new.POS is NULL ) then
    	        select max(POS) into posvar from CCT_ACC_UP
        	    where cct_access_id=new.cct_access_id;
        	    if (posvar is NULL) then
        	       new.POS := 1;
    	        else
	              new.POS := posvar + 1;
	            end if;
	        end if;
	        RETURN NEW;
	    END;
	    $CCT_ACC_UP_AIC_p$  LANGUAGE PLPGSQL;
	    ';
	    
	    $insarr[] = '
        CREATE TRIGGER CCT_ACC_UP_AIC
        before insert on CCT_ACC_UP
        FOR EACH ROW
        EXECUTE PROCEDURE CCT_ACC_UP_AIC_p( );
        ';
	    
	    //
	    // CCT_ACCESS_UPI TRIGGER
	    //
	    
	    $insarr[] = '
        CREATE  OR REPLACE FUNCTION CCT_ACCESS_UPI_p () RETURNS trigger AS $CCT_ACCESS_UPI_p$ 
        DECLARE 
        	userid integer; 
        BEGIN 
        	if (old.TABLE_NAME=\'PROJ\') then 
        		return NEW; 
        	end if; 
        	  
        	userid := new.DB__DB_USER_ID; 
        	if ( new.DB__DB_USER_ID is NULL ) then 
        		userid := old.DB_USER_ID; 
        	end if; 
        		 
        	if ( new.MOD_DATE is not NULL ) then 
        		 
        		INSERT INTO CCT_ACC_UP ( CCT_ACCESS_ID, MODI_DATE, DB_USER_ID, UPINFO )  
        			VALUES (new.CCT_ACCESS_ID, new.MOD_DATE, userid, new.UPINFO ); 
        	end if; 
        	
        	return NEW;
        END;  
        $CCT_ACCESS_UPI_p$ LANGUAGE PLPGSQL;
        ';
	    
	    $insarr[] = '
        CREATE TRIGGER CCT_ACCESS_UPI
        AFTER UPDATE ON cct_access 
        FOR EACH ROW 
        EXECUTE PROCEDURE CCT_ACCESS_UPI_p( );
        ';
	    
	    //
	    // CCT_ACCLOG_T2 TRIGGER
	    //
	    $insarr[] = '
        CREATE  OR REPLACE FUNCTION CCT_ACCLOG_T2_p () RETURNS trigger AS $CCT_ACCLOG_T2_p$ 
        begin 
            IF new.H_ALOG_ACT_ID > 0  THEN 
                update cct_access set H_ALOG_ACT_ID = new.H_ALOG_ACT_ID 
                   where cct_access_id=new.cct_access_id; 
            END IF; 
        	RETURN NEW; 
        end;
        $CCT_ACCLOG_T2_p$  LANGUAGE PLPGSQL;
        ';
	    
	    $insarr[] = '
        CREATE TRIGGER CCT_ACCLOG_T2
        after insert 
        on CCT_ACCLOG 
        for each row 
        EXECUTE PROCEDURE CCT_ACCLOG_T2_p( );
        ';
	    
	    foreach($insarr as $sql_cmd) {
	        $this->_alter_exe($sql_cmd, $FUNCNAME);
	    }
	    
	}
	
	/**
	 * data type of EXTRA_OBJ ...
	 * @param object $sqlo
	 */
	private function _ch_data_types($sqlo) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $MAX_COL=19;
	    
	    for ($i=1; $i<=$MAX_COL; $i++) {
	        $colname = sprintf('n%02d', $i);
	        $sql_cmd="ALTER TABLE extra_obj ALTER COLUMN ".$colname." SET DATA TYPE double precision";
	        $this->_alter_exe($sql_cmd, $FUNCNAME);
	    }
	    
	    $sql_cmd="ALTER TABLE cct_column ALTER COLUMN POS SET DATA TYPE real";
	    $this->_alter_exe($sql_cmd, $FUNCNAME);
	    
	}
	
	function upgrade($sqlo) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $this->_ch_views($sqlo);
	    
	    $this->_ch_sequences($sqlo);
	    
	    $this->_ch_data_types($sqlo);
	    
	    $this->_ch_trigger($sqlo);
	}
	
	function _init(&$sqlo) {
	    // get all tables
	    
	    $this->tables = array();
	    // select * from information_schema.tables

	    
	    $sqlsel = "NAME from CCT_TAB_VIEW order by NAME";
	    $sqlo->Quesel($sqlsel);
	    while ( $sqlo->ReadRow() ) {
  
	        $this->tables[] = $sqlo->RowData[0];
	        
	    }
	    
	    
	    
	}

	function startMain() {
		global $error;

		$FUNCNAME='startMain';
		
		$go   = $_REQUEST['go'];
		$this->parx   = $_REQUEST['parx'];
		$sqlo = &$this->_sqlo;
		
		$this->_infolevel = $this->parx['infolevel'];
		$this->_sqlo_INT  = &$this->_plug_regSql2();
		
		$this->go = $go;
		
		$this->_init($sqlo);
		
		// glob_printr( $this->tables, $FUNCNAME.": tables" );
		
		
		if (!$go) {
			$this->form1();
			return;
		}
		
		if ($go==1) {
		    echo "Prepare.<br>";
		    $this->form2();
		   
		} else {
		    echo "Upgrade.<br>";
		}
		
		$this->upgrade($sqlo);
		
	}

}
