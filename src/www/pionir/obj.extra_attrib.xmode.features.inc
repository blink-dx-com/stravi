<?
require_once('javascript.inc');

class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
    	echo "<ul>";
    	echo '<LI><a href="glob.cache.refresh.php?back='.
    		  $_SERVER['PHP_SELF'].js__get_param_to_url().'&auto_back=1">Refresh global class cache</a></LI>';
    	
    	echo "<LI><a href=\"obj.extra_attrib.import_pref.php?id=".$id."\">Import preferred values</a></LI>\n";
    	
    	echo "</ul>";
    	return 0;
    }

}
