<?

require_once('get_cache.inc');
require_once('javascript.inc');
require_once('func_head.inc');
require_once ("visufuncs.inc");

class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
        
        $argu  = &$this->argu;
        $classname  = $argu["NAME"];
        $xtablename = $argu["TABLE_NAME"];
        
        
        echo '<ul>';
        $sqlo->query('SELECT COUNT(*) FROM extra_attrib WHERE extra_class_id = '.$id);
        $sqlo->ReadRow();
        $attrib_cnt = $sqlo->RowData[0];
        	
        echo '<a href="view.tmpl.php?t=EXTRA_ATTRIB&tableSCond=EXTRA_CLASS_ID='.$id.'&condclean=1">';
        echo '<img src="images/but.listshort.gif" border="0" TITLE="list"> <b>Attribute list</b></A> ('.
        $attrib_cnt.' attributes defined)';
        
        if ( $attrib_cnt ) {
            echo "<ul style=\"padding-top:8px; padding-left:20px;\">";
            $tabobj = new visufuncs();
            $headOpt = array( "title" => "Attributes", "headNoShow" =>1 );
            $headx  = array ("Key", "Value", "Nice", "Notes");
            $tabobj->table_head($headx,   $headOpt);
            
            
            $sqls = "select EXTRA_ATTRIB_ID, NAME, NICE_NAME,NOTES from EXTRA_ATTRIB where extra_class_id=".$id. " order by POS";
            $sqlo->query($sqls);
            $cnt=1;
            while ( $sqlo->ReadRow() ) {
            	$tmpid   = $sqlo->RowData[0];
            	$tmpname = $sqlo->RowData[1];
            	$tmp_nice = $sqlo->RowData[2];
            	$tmpNotes = $sqlo->RowData[3];
            	$dataArr = array(
            	    $cnt,
            	    "<a href=\"edit.tmpl.php?t=EXTRA_ATTRIB&id=".$tmpid."\">".$tmpname."</a>", 
            	    $tmp_nice,
            		$tmpNotes
            	    
            	);
            	$tabobj->table_row ($dataArr);
            	$cnt++;
            }
            $tabobj->table_close();
            echo "</ul>\n";
        }
        
        echo '<br>';
        echo '<a href="view.tmpl.php?t='.$xtablename.'&searchClass='.$classname.'&condclean=1">';
        echo '<img src="images/but.listshort.gif" border="0" TITLE="list"> All elements of class in table '.$xtablename.'</A>';
        
        echo '<br>';
        echo '<a href="glob.cache.refresh.php?back='.$_SERVER['PHP_SELF'].js__get_param_to_url().'&amp;auto_back=1">';
        echo '<img src="images/but.img_refresh.gif" border=0> Refresh global class cache</a><br>';
        echo "\n"; 
        
        $helpurl = htmlCheckHelp( "o.".$xtablename.".class.".$classname.".html" );  
        if ($helpurl!="") {
            echo "<a href=\"javascript:open_info('".$helpurl."')\"> ";
            echo "<img src=\"images/but.help.gif\" border=\"0\" TITLE=\"help\"> Help for this class</a><br>\n";
        }

        echo "</ul>\n";
        return 0;
    }

}

