<?php
/**
 * drop tables
 *    
 * @package g.table.drop.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:14601 g.admin > drop tables tool
 * @param int $go
 * @param array $parx
 *   'table'   : name of table to drop
 *   'confirm' : int: 0 or 1
 *   'shsql' : 0 
 *             1  -- pure SQL, no debug info
 *             2  -- with debug info
 * @version $Header: trunk/src/plugin/root/g.table.drop.inc 59 2018-11-21 09:04:09Z $
 */
class g_table_drop_XPL extends gPlugin {
	
	
	
	function register() {
		
		$title		  = 'Admin: drop tables';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'tool'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['inforow']   = 'This tool drops a database table and related children tables.';
		$this -> infoarr['locrow'] = array (
				array ('home.php', 'home') ,
				array ('rootsubs/rootFuncs.php', 'Administration' )
		);
		
		
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
		$initarr["title"]       = "Select tables to drop"; // title of form
		$initarr["submittitle"] = "Next &gt;&gt;"; // title of submit button
		$initarr["tabwidth"]    = "AUTO";   // table-width: AUTO

		$hiddenarr = NULL; // hidden form variables

		// init form, uses class formc
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);

		$fieldx = array ( // form-field definition
			"title"   => "Tables", 
			"name"  => "tables",
			"object"=> "textarea",
			"val"    => '', 
			"notes" => "e.g. H_STATEX EXP"
			 );
		$formobj->fieldOut( $fieldx ); // output the form-field

		$formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	/**
	 * - creates a form with one text field "Min-value"
	 * - creates a HTML-page parameter $_REQUEST['parx']['minval'] after SUBMIT
	 */
	function form2() {
	
		$initarr   = NULL;
		$initarr["title"]       = "Drop tables now!"; // title of form
		$initarr["submittitle"] = "Drop!"; // title of submit button
		$initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	
		$hiddenarr = NULL; // hidden form variables
		$hiddenarr["parx[tables]"]     = $this->parx['tables']; 
	
		// init form, uses class formc
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 1);
		
		$fieldx = array ( // form-field definition
				"title"   => "Confirm",
				"name"  => "confirm",
				"object"=> "checkbox",
				"val"    => NULL,
				"notes" => "Confirm the drop! If not checked: just show the SQL commands"
		);
		$formobj->fieldOut( $fieldx ); // output the form-field
		
		$fieldx = array ( // form-field definition
		    "title"   => "Show SQL",
		    "name"  => "shsql",
		    "object"=> "select",
		    "inits" => array(0=>'no', 1=>'pure SQL', 2=>'with debug info'),
		    "val"    => NULL,
		    "notes" => "Show the SQL commands"
		);
		$formobj->fieldOut( $fieldx ); // output the form-field
	
		$formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	function set_table($tablename) {
		$this->tablename = $tablename;
	}
	
	static function table_exists_sql(&$sqlo, $table) {
		$sqlo->Quesel( 'NAME FROM CCT_TAB_VIEW where UPPER(NAME) like '. $sqlo->addQuotes($table) );
		if ( !$sqlo->ReadRow() ) {
			return 0;
		} else {
			return 1;
		}
		
	}
	
	private function info_show($text, $no_LF=0) {
	    
	    if ($this->parx['shsql']==1) return; // do not show the debug info ...
	    
	    $br='<br>\n';
	    if ($no_LF) $br='';
	    echo $text . $br;
	}
	
	
	private function _print_SQL($text) {
	    echo htmlspecialchars($text) ."; <br>";
	}
	/**
	 * covered function
	 */
	private function _delete_f(&$sqlo, $table, $sql_string) {
	    
	    if ($this->parx['shsql'])  {
	        $txt_where =  '';
	        if ($sql_string!='')  $txt_where =  ' where '.$sql_string;
	        $this->_print_SQL('delete from '.$table . $txt_where);
	    }
	    if ($this->parx['confirm'])  $sqlo->Deletex($table, $sql_string);
	}
	
	/**
	 * covered function
	 */
	private function _alter_f(&$sqlo, $sql_string) {
	    
	    if ($this->parx['shsql'])  $this->_print_SQL( $sql_string );
	    if ($this->parx['confirm']) $sqlo->query( $sql_string );
	}
	
	private function analyse_table($sqlo) {
		$table = $this->tablename;
		$exists = $this->table_exists_sql($sqlo, $table);
		if (!$exists) {
			return array('exists'=>0, 'info'=>'Table not exists.');
		}
		
		// get children tables
		$child_tables=array();
		
		$pk_name = $table.'_ID';
		$sqlo->Quesel( 'TABLE_NAME, COLUMN_NAME	FROM CCT_COLUMN where  CCT_TABLE_NAME='. $sqlo->addQuotes($table).
				' and TABLE_NAME not like '.  $sqlo->addQuotes($table) );
		while ( $sqlo->ReadRow() ) {
			$child_table = $sqlo->RowData[0];
			$child_tables[] = array('t'=>$child_table, 'colname'=>$sqlo->RowData[1] );
		}
		
		foreach($child_tables as $index => $tablerow) {
		    
		    $colname = $tablerow['colname'];
		    $table   = $tablerow['t'];
		
		    $sqlo->Quesel( 'PRIMARY_KEY FROM CCT_COLUMN where  COLUMN_NAME='. $sqlo->addQuotes($colname).
		        ' and TABLE_NAME='.  $sqlo->addQuotes($table) );
		    $sqlo->ReadRow();
            $PK_FLAG = $sqlo->RowData[0];
            $type=NULL;
            if ($PK_FLAG>0) {
                $type='ASSOC';
            } else {
                $type='COLUMN';
            }
            $child_tables[$index]['tabtype']=$type;
		    
		}
		
		return array('exists'=>1, 'child_tables'=>$child_tables);
	}
	
	private function _drop_table_atomic(&$sqlo, $table, $childkey='') {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    // new syslog entry
	    $argu=array(
	        'NAME'=>'drop table '.$table,
	        'LOG_DATE'=> date('Y-m-d'),
	        'KEY'=>'DROP TABLE'
	    );
	    $sqlo->InsertSysArr( 'G_SYSLOG', $argu );
	    
	    if ($childkey=='child') $this->info_show( ' &nbsp;&nbsp;&nbsp;', 1);
	    $this->info_show('... drop table: '.$table);
	    
	    if (table_is_view($table)) {
	        $this->_alter_f( $sqlo,  'drop view '.$table );
	    } else {
	        $this->_alter_f( $sqlo,  'drop table '.$table );
	    }
	    
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 1, 'error on Drop of table '.$table );
	        return;
	    }
	}
	
	private function _drop_functions($sqlo, $table) {
	    // EXTRA_OBJ triggers
	    $deo_func='trigger_'.$table.'_deo';
	    $this->_alter_f( $sqlo,  'DROP FUNCTION IF EXISTS '.$deo_func.' CASCADE');
	}
	
	private function _delete_CCT_TABLE(&$sqlo, $table) {
	    // delete data
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $this->info_show( ' ..... CCT_TABLE : remove entries for table '.$table);
	    
	    $sqlo->Quesel( 'KIND FROM CCT_TAB_VIEW where UPPER(NAME)='.  $sqlo->addQuotes($table) );
	    $sqlo->ReadRow();
	    $ttype = $sqlo->RowData[0];
	    
	    $isBo = 0;
	    
	    $sqlo->Quesel( 'COLUMN_NAME FROM CCT_COL_VIEW where  UPPER(COLUMN_NAME)='. $sqlo->addQuotes('CCT_ACCESS_ID').
	        ' and UPPER(TABLE_NAME) ='.  $sqlo->addQuotes($table) );
	    if ( $sqlo->ReadRow() ) {
	        $isBo = 1;
	    }
	    
	    $isview=0;
	    if ($ttype==2) $isview=1;
	    
	    if (!$isview) {
	        
	        
	        
	        $this->info_show( '... delete old data ('.$isBo.')' );
	        if ( $isBo ) {
	           $this->_delete_f($sqlo, 'CCT_ACCESS',   'CCT_ACCESS_ID in (select CCT_ACCESS_ID from '.$table .')');
	           $this->_delete_f($sqlo, 'PROJ_HAS_ELEM', 'TABLE_NAME='.$sqlo->AddQuotes($table) );
	        } else {
	           $this->_delete_f($sqlo, $table, '1=1');
	        }
	        $this->_delete_f($sqlo, 'S_VARIO', 'TABLE_NAME='.$sqlo->AddQuotes($table) );
	        
	        if ($error->Got(READONLY))  {
	            $error->set( $FUNCNAME, 1, 'delete old data failed: '.$table );
	            return;
	        }
	    } 

	    // - delete CCT_COLUMN
	    $this->_delete_f($sqlo, 'CCT_COLUMN', 'TABLE_NAME='.$sqlo->AddQuotes($table) );
	    $this->_delete_f($sqlo, 'CCT_TABLE', 'TABLE_NAME=' .$sqlo->AddQuotes($table) );
	}
	
	/**
	 * drop TABLE and its children
	 * @param array $table_ana_result
	 */
	private function drop_table( &$sqlo, $table_ana_result ) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$table = $this->tablename;
		$children = $table_ana_result['child_tables'];
		
		// for all children ...
		if ( sizeof($children) ) {
		    foreach($children as $row) {

		        $child_table=$row['t'];
		        $this->info_show( ' --- child: '.$child_table);
		        
		        // - delete CCT_TABLE
		        $isview=0;
		        if ($row['type']==2) $isview=1;
		        
		        if ( $row['tabtype']=='COLUMN') {
		            // drop column
		            
		            $this->info_show( ' ----- drop column '.$row['colname']);
		            $this->_delete_f(  $sqlo, 'CCT_COLUMN', 'TABLE_NAME='.$sqlo->AddQuotes($child_table).' and COLUMN_NAME='.$sqlo->AddQuotes($row['colname']) );
		            
		            $this->_alter_f($sqlo, 'alter table '.$child_table .' drop column '. $row['colname'] );
		            
		            if ($error->Got(READONLY))  {
		                $error->set( $FUNCNAME, 1, 'error on Drop-Column '.$child_table.':'. $row['colname'] );
		                return;
		            }
		            
		        } else {
		        
    		        $this->_delete_CCT_TABLE($sqlo, $child_table, $isview);
    		        if ($error->Got(READONLY))  {
    		            $error->set( $FUNCNAME, 1, 'error on delete of CCT_TABLE-entry for child-table '.$child_table );
    		            return;
    		        }

		            $this->_drop_table_atomic($sqlo, $child_table, 'child');
		            if ($error->Got(READONLY))  {
		                $error->set( $FUNCNAME, 1, 'error on Drop of table '.$child_table );
		                return;
		            }
		        }
		        
		    }
		}
		
		
		$this->_drop_functions($sqlo, $table);
		
		// - delete CCT_TABLE
		$this->_delete_CCT_TABLE($sqlo, $table);
		
		if ($error->Got(READONLY))  {
		    $error->set( $FUNCNAME, 1, 'error on delete of CCT_TABLE-entry for '.$table );
		    return;
		}
		
		// ---- now start to drop ---
		
		// new syslog entry
		$argu=array(
				'NAME'=>'drop table '.$table,
				'LOG_DATE'=> date('Y-m-d'),
				'KEY'=>'DROP TABLE'
		);
		$sqlo->InsertSysArr( 'G_SYSLOG', $argu );
		$this->info_show( '... write log to G_SYSLOG');
		
		
		
		// main table
		$this->_drop_table_atomic($sqlo, $table);
		if ($error->Got(READONLY))  {
		    $error->set( $FUNCNAME, 1, 'error on Drop of table '.$table );
			return;
		}
	}
	
	function dropOne_main(&$sqlo, $tablename, $go) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $parx = $this->parx;
	    echo 'Table: <b>'.$tablename."</b>"."<br>";
	    
	    
	    $this->set_table($tablename);
	    $table_ana_result = $this->analyse_table($sqlo);
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 1, 'Error on initial analysis.' );
	        return;
	    }
	    
	    echo "DDD:<pre>";
	    print_r($table_ana_result);
	    echo '</pre>';
	    
	    echo 'Child Tables: ';
	    if ( is_array($table_ana_result['child_tables']) ) {
	        echo '<br>';
	        $tmpstr=NULL;
	        
	        $children = $table_ana_result['child_tables']; 
	        foreach($children as $childrow) {
	            $child= $childrow['t'];
	            $tmpstr .= '<br> - <b>'.$child .'</b>';
	        }
	        
	    } else $tmpstr = 'none';
	    echo $tmpstr. " <br><br>\n";
	    
	    $exists = $table_ana_result['exists'];
	    if (!$exists) {
	        $this->htmlFoot('INFO', 'Table "'.$tablename.'" not exists in system.');
	    }
	    
	   
	    
	    if ($go<=1) {

	        return;
	    }
	    
	    if ($go==2) {
	        
            // 	        if (!$parx['confirm']) {
            // 	            $this->htmlFoot('INFO', 'Please confirm the checkbox to drop.');
            // 	        }
	        
	        $this->_html->line_out('');
	        $this->_html->line_out('Start drop analysis ...');
	        
	        
	        $table_ana_result = $this->drop_table($sqlo, $table_ana_result);
	        if ($error->Got(READONLY))  {
	            $error->set( $FUNCNAME, 2, 'Error on drop.' );
	            return;
	        }
	        
	        echo "Ready.<br>";
	    }
	}

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$go    = $_REQUEST['go'];
		$this->parx  = $_REQUEST['parx'];
		$sqlo  = &$this->_sqlo;
		
		$tablenames = trim($this->parx['tables']);
		
		$this->_html->line_out('[<a href="'.$this->_html->_plug_get_URL().'">Restart</a>]');
		$this->_html->line_out('');
		
		if (!$go or $tablenames==NULL) {
			$this->form1();
			return;
		}
		
		if ($go==2 ) {
		    $do_shsql= '<span style="color:gray;">No SQL shown.</span>';
		    $do_drop= '<span style="color:gray;">Not dropped.</span>';
		    if ($this->parx['shsql'])   $do_shsql= 'show SQL';
		    if ($this->parx['confirm'])  $do_drop= '<span style="color:red;">perform DROP actions!</span>';
		    
		    $this->_html->line_out('Show SQL: '.$do_shsql);
		    $this->_html->line_out('Drop actions: '.$do_drop);
		    $this->_html->line_out('');
		}
		
		$tablearr = explode("\n",$tablenames);
		foreach($tablearr as $tablename ) {
		    
		    $tablename = trim($tablename);
		    
		    $this->dropOne_main($sqlo, $tablename, $go);
		    if ($error->Got(READONLY))  {
		        $error->set( $FUNCNAME, 1, 'Error on table: '.$tablename );
		        return;
		    }
		}
		
		if ($go<=1) {
		  $this->form2($this->parx);
		  return;
		}
	}
	
}