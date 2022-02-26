<?php
require_once ("visufuncs.inc");
require_once ("f.objview.inc");
require_once 'o.CCT_TABLE.info.inc';
require_once ('object.info.inc');

/**
 * USER objects
 * @package o.DB_USER.obj_my.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $user_id INT or 'ME'
 * @param   $parx 
 *   'my_last_cnt_max' : up to 200
 */
class o_DB_USER_obj_my_XPL extends gPlugin {
	
	function register() {
		
	    $title_sh		= 'my objects statistics'; 
	    $title = 'statistics: objects created by you';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh']    = $title_sh;
		$this->infoarr['form_type']= 'tool'; 
		$this->infoarr['locrow']   = array(  
		    array('home.php', 'home'),
		); 
		
	}
	
	private function my_latest_objects($sqlo, $parx) {
	    
	    $MAX_SHOW_DEF=20;
	    $MAX_SHOW_MAX=200;
	    
	    $MAX_SHOW = $MAX_SHOW_DEF;
	    if ($parx['my_last_cnt_max']>0) {
	        $MAX_SHOW = $parx['my_last_cnt_max'];
	    }
	    if($MAX_SHOW>$MAX_SHOW_MAX) $MAX_SHOW=$MAX_SHOW_MAX;
	    
	    $user_id= $this->user_id;
	    
	    $hide_tables = array('CONCRETE_PROTO');
	    $obj_cache=array();
	    
	    $sqlsel = 'a.CCT_ACCESS_ID, a.TABLE_NAME FROM cct_access a '.
	   	    ' WHERE a.DB_USER_ID='.$user_id. ' order by a.CREA_DATE DESC';
	    $sqlo->Quesel($sqlsel);
	    $i=0;
	    while ($sqlo->ReadRow() ) {
	        
	        if($i>$MAX_SHOW) break;
	        $acc_id = $sqlo->RowData[0];
	        $table  = $sqlo->RowData[1];
	        if (in_array($table, $hide_tables)) {
	            continue;
	        }
	        
	        $obj_cache[]=array('a'=>$acc_id, 't'=>$table);
	        $i++;
	    }

	    $tabobj = new visufuncs();
	    $headOpt= array( "title" => "My LAST ".$MAX_SHOW." Objects" );
	    $headx  = array ("#", "Object", "Type", "Creation date");
	    $tabobj->table_head($headx,   $headOpt);
	    
	    foreach($obj_cache as $i=> $acc_row) {
	        
	        $table = $acc_row['t'];
	        $outputCol= PrimNameGet2($table);
	        $obj_id   = glob_elementDataGet( $sqlo, $table, 'CCT_ACCESS_ID', $acc_row['a'], $outputCol);
	        $obj_link='???';
	        $crea_date='???';
	        
	        if ($obj_id) {
	           $obj_link = fObjViewC::bo_display( $sqlo, $table, $obj_id);

	           $objInfoLib = new objInfoC();
	           $features = $objInfoLib->getParamsRaw ( $sqlo, $table, $obj_id);
	           
	           $crea_date = $features['access']['CREA_DATE'];
	           
	        }
	        
	        $dataArr = array($i+1, $obj_link, tablename_nice2($table), $crea_date);
	        $tabobj->table_row ($dataArr);
	    }
	    $dataArr = array('', '<a href="'.$this->_plug_get_URL().'&parx[my_last_cnt_max]=200">... more</a>', '', '');
	    $tabobj->table_row ($dataArr);
	    
	    $tabobj->table_close();
	    
	}
	
	private function _one_table($sqlo, $table) {
	    $user_id= $this->user_id;
	    
	    $sqlsel = 'COUNT(1) FROM cct_access a '.
	   	    ' WHERE a.DB_USER_ID='.$user_id. ' AND a.TABLE_NAME='.$sqlo->addQuotes($table);
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    $countx = $sqlo->RowData[0]; 
	    
	    $icon = '<img src="'.fObjViewC::getIcon($table).'">';
	    
	    $out = array($icon, tablename_nice2($table), $countx);
	    return $out;
	}

	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$sqlo = &$this->_sqlo;
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
		
		$this->user_id = $_SESSION['sec']['db_user_id'];
		
		?>
		<div class="container" style="display: flex; ">
		<div style="width: width: 30%; flex-grow: 1;">
		<?php
		
		$tabobj = new visufuncs();
		$headOpt= array( "title" => "My Objects: counting" );
		$headx  = array ("", "Table", "No of Objects");
		$tabobj->table_head($headx,   $headOpt);

		$cnt_all=0;
		$table_list = oCCT_TABLE_info::getTablesNiceByType($sqlo, 'BO');
		foreach($table_list as $table=> $tab_nice) {
		    $dataArr = $this->_one_table($sqlo, $table);
		    
		    $cnt_loop = $dataArr[2];
		    
		    if ($cnt_loop>0) {
		      $tabobj->table_row ($dataArr);
		    }
		    
		    $cnt_all = $cnt_all + $cnt_loop;
		}
		
		$dataArr=array('','<SUM of all', '<b>'.$cnt_all.'</b>');
		$tabobj->table_row ($dataArr);
		
		$tabobj->table_close();
		
		?>
		</div>
		<div style="flex-grow: 3;">
		<?php
		
		$this->my_latest_objects($sqlo,$parx);
		
		?>
		</div>
		</div>
		<?php
	}
}
