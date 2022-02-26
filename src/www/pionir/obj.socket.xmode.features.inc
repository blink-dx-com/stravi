<?php
require_once ("f.objview.inc");

/**
 * extension for SOCKET
 * module: obj.socket.xmode.features.inc
 * @app_type_info will be overwritten by type:2021_abbott
 */
class obj_edit_ext extends gObj_edit_feat_ABS { 
    
    function show_mother_batch($sqlo) {
        
        $batch_id = glob_elementDataGet( $sqlo, 'BCBATCH_H_SOC', 'SOCKET_ID', $this->id,  'BCBATCH_ID');
        if (!$batch_id) return;
        
        $objLinkLib = new fObjViewC();
        $html_tmp = $objLinkLib->bo_display( $sqlo, 'BCBATCH', $batch_id );
        
        echo 'From Batch: ' .$html_tmp. '<br>'."\n";
    }
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
    	echo "<ul>";
    	
    	$this->id = $id;
    	$this->show_mother_batch($sqlo);
    	
    	$filename =  $_SESSION['s_sessVars']['AppLabLibDir'] . '/lablib/o.SOCKET.showcart.inc';
    	if (file_exists($filename)) {
    		require_once $filename;
    	
    		$show_lib = new oSOCKET_showcart($sqlo, $id);
    		
    		$sh_opt=array('suc.distinct.colors'=>1);
    		$show_lib->show($sqlo, $sh_opt);
    		echo "<br>\n";
    		echo "<br>\n";
    	}
    	
    	echo "</ul>\n";
    	return 0;
    }


}
