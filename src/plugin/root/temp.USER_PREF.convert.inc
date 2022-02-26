<?php
require_once 'o.CCT_TABLE.info.inc';
require_once 'o.USER_PREF.manage.inc';

/**
 * Convert USER_PREF colview
 *    
 * @package temp.USER_PREF.convert.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   UREQ:0001088: o.CCT_ACC_UP > AutoCleaner 
 * @param   $_REQUEST['go'] 0,1
 * @version $Header: trunk/src/plugin/root/o.CCT_ACC_UP.AutoClean.inc 59 2018-11-21 09:04:09Z $
 *   
 */
class temp_USER_PREF_convert_XPL extends gPlugin {
	
	
	
	function register() {
		$tablename	= 'USER_PREF';
		$title		= 'Convert USER_PREF colview';
		
		$this->infoarr			 = NULL;
		
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;

		
		$this->infoarr2 = array( 
			'onlyAdmin'   =>1,
			
		);
				
		
	}
	
	function form0() {
	    $initarr=array();
		$initarr["title"]       = "Convert USER_PREF colview";
		$initarr["submittitle"] = "Now!";
		$initarr["tabwidth"]    = "AUTO";
		$hiddenarr = NULL;
		$formobj = $this->_html->formInit($initarr, $hiddenarr);
		$formobj->close( TRUE );
	}
	
	function _one_user(&$sqlo, $user_id) {
	
	    $colview_old_pat = "o.%.viewcol";
	    $new_patt="o.%.viewcols";
	    
	    $user_nice = obj_nice_name ( $sqlo, 'DB_USER',$user_id ); 
	    echo "- User: ".$user_id." user_nice: $user_nice <br>";
	    
	    $sqlsel = "var_name, value FROM user_pref WHERE db_user_id = ".$user_id." and var_name like ".$sqlo->addQuotes($colview_old_pat) ."  ORDER BY var_name";
	    $sqlo->Quesel($sqlsel);
	  
	    
	    $new_tables = array();
	    $cnt=0;
	    while ($sqlo->ReadRow()) {
	        
	        $key = $sqlo->RowData[0];
	        $val = $sqlo->RowData[1];
	        
	        // echo "DDD: $key,$val <br>";
	        
	        $key_arr=explode('.',$key);
	        $table = $key_arr[1];
	        $col   = $key_arr[2];
	        
	        if ($table and $col) {
    	        if (glob_table_exists($table)) {
    	            
    	            if (!is_array($new_tables[$table])) {
    	                $new_tables[$table]=array();
    	            }
    	            $new_tables[$table][$col] = $val;
    	        }
	        }
	        
	        $cnt++;
	    }
	    
	    $tabs = array_keys($new_tables);
	    echo " &nbsp;&nbsp;- COL-Prefs: ".$cnt." tables_with_viewcol: ".implode(", ",$tabs)."<br>";
	    
	    $sqlsel = "var_name, value FROM user_pref WHERE db_user_id = ".$user_id." and var_name like ".$sqlo->addQuotes($new_patt) ."  ORDER BY var_name";
	    $sqlo->Quesel($sqlsel);
	    while ($sqlo->ReadRow()) {
	       $exist_key = $sqlo->RowData[0];
	       $exist_val = $sqlo->RowData[1];
	       echo " &nbsp;&nbsp;- exists: $exist_key: $exist_val <br>";
	    } 
	    
	   
	    
	    foreach($new_tables as $table => $row) {
	        
	        if (empty($row)) continue;

	        
	        $new_key =  'o.'.$table.'.viewcols';
	        $val = serialize($row);
	        
	        echo " &nbsp;&nbsp;- Update: ".$table.": ".print_r($row,1)."<br>";
	        
	        oUSER_PREF_manage::entry_update_user($sqlo, $user_id, $new_key, $val);
	    }

	    // delete all old PREFS in DB!
	    // $sqlo->Deletex('USER_PREF', "DB_USER_ID=".$user_id." and VAR_NAME like ".$sqlo->addQuotes($colview_old_pat) );
	    
	}

	function startMain() {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$go   = $_REQUEST['go'];
		$sqlo = &$this->_sqlo;

		
		if (!$go)  {
			$this->form0();
			return;
		}
		
		//$tables = oCCT_TABLE_info::getTables($sqlo);
		
		$users = array();
		$sqlsel = 'distinct(db_user_id) FROM user_pref ORDER BY db_user_id';
		$sqlo->Quesel($sqlsel);
		while ($sqlo->ReadRow()) {
		    $user_id = $sqlo->RowData[0];
		    $users[]=$user_id;
		}
		
		foreach($users as $user_id) {
		    $this->_one_user($sqlo, $user_id);
		    if ($error->Got(READONLY))  {
		        $error->set( $FUNCNAME, 1, 'error on user:'. $user_id);
		        return;
		    }
		    
		   
		}
		

		
	}
	
}