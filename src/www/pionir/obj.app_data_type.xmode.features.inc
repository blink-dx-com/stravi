<?php
require_once('javascript.inc');

class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
       echo '<P>';
       echo '<p><a href="glob.cache.refresh.php?back='.$_SERVER['PHP_SELF'].js__get_param_to_url().'&amp;auto_back=1">Refresh global table data cache</a>';
    }

}
