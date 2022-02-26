<?php
/**
 * abstract worklist entry  extension 
 * used by edit.tmpl.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0000932 db.WORKLIST: Messauftrag organisieren
 */
class obj_edit_ext extends gObj_edit_feat_ABS {
    function rightWindow( &$sqlo, &$sqlo2) {

    	echo '<ul>'."\n";
    	htmlInfoBox( "Short help", "", "open", "HELP" );
    	?>
    	<b>Field Options</b>
    	<ul>
    	<li>title=string : title of column</li>
    	<li>input=[ID],NAME : if type=OBJ, user input is NAME or ID; on NAME: a pure text-box is shown per object</li>
    	<li>selectdata=string|string|string  : creates a select box; example: selectdata=Fehler1|Fehler2|Weiterer Fehler3 </li>
    	</ul>
    	<?php
    	htmlInfoBox( "", "", "close" );
    	echo "</ul>\n";
    	
    }

}