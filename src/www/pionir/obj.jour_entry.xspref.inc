<?php
/**
 * contains function for single object preferences
 * $Header: trunk/src/www/pionir/obj.jour_entry.xspref.inc 59 2018-11-21 09:04:09Z $
 * @subpackage 
 * @package obj.jour_entry.xspref.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class o_JOUR_ENTRY_XSPREF  extends o_xspref_ABS {

    function singlePref( &$sqlo, $info) {
    	prefEmptLine();
    	
        $tmpopt = unserialize($_SESSION['userGlob']['o.EXP.labjourList']);
    
    	prefNameShow( "KEY-dictionary-doc" );
    	$vartmp = "KEY.docid";
    	$infotmp= $tmpopt[$vartmp];
       
        echo "<td><input type=text name=\"setti[".$vartmp."]\" value=\"".$infotmp."\" size=6> </td>";
    	echo "</td>";
    	prefClose ("ID of document, containing the KEY-dictionary.");
    }
    
    function prefSave( &$sql, $info, $setti ) {
        
    
        $tmpopt = unserialize($_SESSION['userGlob']['o.EXP.labjourList']);
    	
    	$formvar  = 'KEY.docid';
        $tmpopt[$flagname] = NULL;
        $tmp_val = $setti[$formvar] ;
        if ($tmp_val!=NULL) $tmpopt[$formvar] = $tmp_val;
        else unset($tmpopt[$formvar]);
    
    	$_SESSION['userGlob']['o.EXP.labjourList'] = serialize($tmpopt);
    }
}
