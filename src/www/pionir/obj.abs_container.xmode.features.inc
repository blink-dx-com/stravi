<?php
require_once('gui/o.SATTACH.imgshow.inc');

class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;

    	$tablename='ABS_CONTAINER';
    	$sqlsel = 'count(1) from ABS_CONT_ENTRY where ABS_CONTAINER_ID='.$id;
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadRow();
    	$entryCnt = $sqlo->RowData[0];
    
    	echo '<ul>'."\n";
    
    	echo "<table cellpadding=1 cellspacing=1 border=0>\n<tr valign=top>";
    	echo "<td>";
    	
    	if ($entryCnt) {
    	    echo '<font color=gray>Container-Content-Graphics:</font><ul>';
    	    echo '<img src="obj.abs_container.img.php?id='.$id.'">';
    	}
    	echo "</td>";
    	echo '<td>&nbsp;&nbsp;&nbsp;</td>';
    	echo "<td>";
    	
    	// show thumbnail
    	$attachLib = new oSATTACH_imgshow();
    	$relid = $attachLib->getThumbAttach( $sqlo, $tablename, $id);
    	if ($relid) {
    		$htmlTmp = $attachLib->getImgTag($relid);
    		echo '<span style="color:gray">Thumbnail:</span><ul>'.$htmlTmp.'</ul>'."\n";
    	}
    	
    	
    	echo "</td></tr>";
    	echo "</table>";
    	
    	echo '</ul>'."\n";
    	
    }

}

