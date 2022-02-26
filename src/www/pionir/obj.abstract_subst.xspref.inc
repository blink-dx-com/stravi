<?php
/**
 * contains function for single object preferences
 * 
 * @package obj.abstract_subst.xspref.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class o_ABSTRACT_SUBST_XSPREF  extends o_xspref_ABS {
    
    const PREF_VAR_NAME='o.ABSTRACT_SUBST.xspref';

    function singlePref( &$sqlo, $info) {
    	prefEmptLine();
    	
    	$tmpopt = unserialize($_SESSION['userGlob'][self::PREF_VAR_NAME]);
    
    	$this->prefNameShow( "show related journal entries" );
    	$vartmp = 'JOUR_ENTRY.sh';
    	$infotmp= $tmpopt[$vartmp];
    	
    	$checked_tmp="";
    	if ($infotmp) $checked_tmp="checked";
    	echo "<td><input type=checkbox name=\"setti[".$vartmp."]\" value=\"1\" ".$checked_tmp."> </td>";
    	echo "</td>";
    	$this->prefClose ("Automtically shows related ".tablename_nice2('JOUR_ENTRY') );
    }
    
    function prefSave( &$sql, $info, $setti ) {
        
    
        $tmpopt = unserialize($_SESSION['userGlob'][self::PREF_VAR_NAME]);
    	
        $formvar = 'JOUR_ENTRY.sh';
        $tmpopt[$formvar] = NULL;
        $tmp_val = $setti[$formvar] ;
        if ($tmp_val!=NULL) $tmpopt[$formvar] = $tmp_val;
        else unset($tmpopt[$formvar]);
    
        $_SESSION['userGlob'][self::PREF_VAR_NAME] = serialize($tmpopt);
    }
}
