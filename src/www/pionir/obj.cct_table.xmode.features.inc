<?php
require_once('javascript.inc');
require_once ("visufuncs.inc");
require_once 'f.prefvals.inc';

class obj_edit_ext extends gObj_edit_feat_ABS { 
    
    
    function _col_overview($sqlo, $id) {
        
       global   $_s_i_table;
       
       $tablename = $id;
       
       if (empty($_s_i_table[$tablename])) {
           echo "No columns defined.<br>";
           return;
       }
 
       $tabobj = new visufuncs();
       $headOpt = array( "title" => "Table-Columns" );
       $headx  = array ("Column-Code", "Nice name", 'Predefined Values');
       $tabobj->table_head($headx,   $headOpt);

       foreach($_s_i_table[$id] as $col=>$row) {
       
           $tmp_predef='';
           if ($col=='__table_data__') {
               continue;
           }
           
           $pre_list = fPrefValsGet::getListSort($sqlo, $tablename, $col);
           if( !empty($pre_list) ) {
               
               $tmp=array_keys($pre_list);
               $tmp_predef = implode(", ",$tmp);
           }
           
           $dataArr=array( $col, $row['NICE_NAME'], $tmp_predef);
           $tabobj->table_row ($dataArr);
       }
       
       $tabobj->table_close();
   }
    
   function rightWindow( &$sqlo, &$sqlo2) {
       
       $id = $this->id;
      global   $_s_i_table;
      
      echo '<ul>';
      echo '<li><a href="glob.cache.refresh.php?back='.$_SERVER['PHP_SELF'].js__get_param_to_url().'&amp;auto_back=1">Refresh global table data cache</a></li>';
      echo '<li><a href="view.tmpl.php?t='.$id.'">Go to the table list view <img src="images/but.list2.gif"></a></li>';
      echo '<br>';
      $this->_col_overview($sqlo, $id);
      
      echo '<br>';
      echo '<li>MetaCache structure of table:</li>';
      echo "<pre>";
      print_r  ( $_s_i_table[$id]['__table_data__'] );
      echo "</pre>";
      echo '</ul>';
 
      return 0;
    }

}

