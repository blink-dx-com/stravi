<?php
require_once 'o.CHIP_READER.compo.inc';
require_once ("f.objview.inc");
/**
 * - analyse this device: where used as sub module
 * @module obj.chip_reader.xmode.submod.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oCHIP_READER_xmode_submod {
		

    function xmode_start( &$sqlo, &$sqlo2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {
        
        $tablename  = 'CHIP_READER';
        $dev_nice   = tablename_nice2($tablename);
        echo '<ul>'."\n";
        echo 'Analyze, where this is used as component of other '.$dev_nice.'s:<br><br>'."\n";
    	
        echo '<ul>'."\n";
    	
        $dev_lib = new oCHIP_READER_compo($id);
        $dev_arr = $dev_lib->used_by_devs($sqlo);
        if(empty($dev_arr)) {
            echo '<span class="yGgray">Nothing found.</span><br>';
            return;
        }
        
        $add_url = '&xmode=features';
        foreach($dev_arr as $dev_id_loop) {
            $url = fObjViewC::bo_ID_URL($tablename, $dev_id_loop);
            $txt = fObjViewC::bo_nice($sqlo, $tablename, $dev_id_loop);
            $ico = fObjViewC::getIcon($tablename);
            
            $obj_link = '<a href="'.$url.$add_url.'"><img src="'.$ico.'"> '.$txt.'</a>';
            echo $obj_link."<br>\n";
        }
        echo '</ul>'."\n";

    	echo '</ul>'."\n";
    }

}