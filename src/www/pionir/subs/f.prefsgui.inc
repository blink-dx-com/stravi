<?php

/**
 * sub functions for user-preferences GUI pages
 * @package f.prefsgui.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class fPrefsGuiC {
	
	// prefs_showHead
    function showHeadTabs ( $pagemode, $tablename ) {
    	// descr: show header for preferences
    	
    			
    	$modeurls = NULL;
    	$modeurls["LIST"]   = array ("List view prefs "    , 	'glob.objtab.pref.php?tablename='.$tablename );
    	$modeurls["SINGLE"] = array ("Single object prefs", 	'glob.obj.pref.php?tablename='.$tablename );
    	$modeurls["INFO"]   = array ("Object type info",   'glob.objtab.info.php?tablename='.$tablename );
    	$modeurls["OBJ"]    = array ("Advanced preferences info", 	  'glob.obj.pref2.php?tablename='.$tablename );
    	
    	if ( $tablename ) {
    		echo '<table cellpadding="5" cellspacing="0"><tr>'."\n";
    	
    		foreach( $modeurls as $tmpmode=>$tmparr) {
    			if ($pagemode == $tmpmode) {
    				echo '<td bgcolor="#336699"><font color="#ffffff"><B>&nbsp;'.$tmparr[0].'&nbsp;</B></td>'."\n";
    			} else {
    				echo '<td bgcolor="#DFDFDF">&nbsp;<a href="'.$tmparr[1].'">'.$tmparr[0].'</A>&nbsp;</td>'."\n";
    			}
    			echo '<td bgcolor="#FFFFFF" width=1></td>'."\n";
    		}
    		
    	
    		echo '</tr></table>',"\n";
    		echo "<br>\n";
    	}
    }

}

