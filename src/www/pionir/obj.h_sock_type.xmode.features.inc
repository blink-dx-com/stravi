<?php
/**
 * extension for H_SOCK_TYPE
 * @author  Steffen Kube (steffen.kube@blink-dx.com)
 */
require_once ("visufuncs.inc");
require_once ('o.ABS_SOCKET.subs.inc');
require_once ("f.objview.inc");	

require_once $_SESSION['s_sessVars']['AppLabLibDir'] . '/lablib/o.H_SOCK_TYPE.showcart.inc';

class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
	
    	echo "<ul>\n";

    	$show_lib = new oH_SOCK_TYPE_showcart($sqlo, $id);
    	$show_lib->show($sqlo);
    	echo "<br>\n";
    	echo "<br>\n";
    	
    
    	
    	echo "</ul>\n";
    }


}


