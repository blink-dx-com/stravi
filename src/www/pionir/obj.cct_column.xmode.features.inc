<?php
require_once ('func_form.inc');
require_once('javascript.inc');

class obj_edit_ext extends gObj_edit_feat_ABS {
    
    var $advanced_keys=array(
       array ( 
            "title" => "field.type",
            "name"  => "field.type",
            "object"=> "select",
            "inits" =>array(
                'datalist'=>'Datalist',
                'select'  =>'Select Box',
                'text'    =>'Text'
            ),
            "notes" => ""
        )
    );
    
    function json_form($json_str) {
 
        $datax_arr=array();
        if ($json_str!=NULL) {
            $datax_arr = json_decode($json_str, TRUE);
        }
        
        if ($this->viewmode!='edit') {
            
            if(empty($datax_arr)) {
                echo " No Advanced data.<br>";
                return;
            }
            
            echo "<ul>\n";
            
            foreach($this->advanced_keys as $row) {
                $fieldx  = $row;
                $varname = $row['name'];
                $val = $datax_arr[$varname];
                echo '- '.$varname.': '.$val.'<br>';
            }
            echo "</ul>\n";
            return;
        }
        
        echo "<ul>\n";
        
        $argu= &$this->argu;
        $tmp_table = $argu['TABLE_NAME'];
        $tmp_col   = $argu['COLUMN_NAME'];
        
        $initarr   = NULL;
        $initarr["action"]      = 'p.php?mod=DEF/o.CCT_COLUMN.adv';
        $initarr["title"]       = "Edit advanced parameters";
        $initarr["submittitle"] = "Submit";
        $initarr["tabwidth"]    = "AUTO";
        $initarr["formname"]    = "editform2";
        
        
        $hiddenarr = NULL;
        $hiddenarr["id"]    = $tmp_table;
        $hiddenarr["id2"]   = $tmp_col;
        //$hiddenarr["primas[1]"]    = 'COLUMN_NAME';
        //$hiddenarr["primasid[1]"]  = $tmp_col;
        
        $formobj = new formc($initarr, $hiddenarr, 0);

        foreach($this->advanced_keys as $row) {
            $fieldx = $row;
            $varname = $row['name'];
            $fieldx['val'] = $datax_arr[$varname]; 
            $formobj->fieldOut( $fieldx );
        }
        $formobj->close( TRUE );
        echo "</ul>\n";
    }
    
    function rightWindow( &$sqlo, &$sqlo2) {
        
        $id = $this->id;
        $argu= &$this->argu;
        
        $tmp_table = $argu['TABLE_NAME'];
        $tmp_col   = $argu['COLUMN_NAME'];
        
        $sqlo->query("select count(1) from H_VAL_INIT where TABLE_NAME='".$tmp_table."' AND COLUMN_NAME='".$tmp_col."'");
        $sqlo->ReadRow();
        $numprefs = $sqlo->RowData[0];
        $urlcond = urlencode("TABLE_NAME='".$tmp_table."' AND COLUMN_NAME='".$tmp_col."'");
         
        echo '<ul>';
        echo '[<a href="glob.cache.refresh.php?back='.$_SERVER['PHP_SELF'].js__get_param_to_url().'&amp;auto_back=1">Refresh global table data cache</a>]<br>';
        
        $this->_sub_title('Predefined values');
        echo " <a href=\"view.tmpl.php?t=H_VAL_INIT&condclean=1&tableSCond=".$urlcond."\"><b>".$numprefs."</B></a>";
        if ($_SESSION['sec']['appuser']=="root") {
        	echo '&nbsp;&nbsp;&nbsp;[<a href="obj.cct_column.pref_import.php?tablename='.$tmp_table.'&column='.$tmp_col.'">Import predefined values</a>]';
        }
        echo "<br>";
        
        $this->_sub_title('Advanced Attributes');
        $this->json_form($argu['DATAX']);
        
        echo "</ul>\n";
        	
    }
}
