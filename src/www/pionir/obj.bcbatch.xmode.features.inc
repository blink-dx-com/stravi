<?php
/**
 * extension for BCBATCH
 * @package obj.bcbatch.xedit.php
 * @author  Steffen Kube
 */
require_once ("visufuncs.inc");
require_once ('o.ABS_SOCKET.subs.inc');
require_once ("f.objview.inc");	

class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        
        $id  = $this->id;
        $argu= &$this->argu;
        
    	echo "<ul>\n";
    	
    	$ABS_SOCKET_ID = $argu['ABS_SOCKET_ID'];
    	
    	$filename =  $_SESSION['s_sessVars']['AppLabLibDir'] . '/lablib/o.ABS_SOCKET.showcart.inc';
    	if ($ABS_SOCKET_ID and file_exists($filename)) {
    		require_once $filename;
    		
    		$show_lib = new oABS_SOCKET_showcart($sqlo, $ABS_SOCKET_ID	);
    		$show_lib->show($sqlo);
    		echo "<br>\n";
    		
    	}
    
    	echo "</ul>\n";
    }


}

