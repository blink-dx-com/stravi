<?php
/**
 * special forms
 * @package  func_formSp.inc
 * 
 * @author  Steffen Kube (steffen@blink-dx.com)

 */
class formSpecialc {
	
    /**
     * 
     * @param string $title
     * @param string $asktext
     * @param string $delurl
     * @param array $params //$delurl-parameters
     */
    function deleteForm($title, 	$asktext,	$delurl, 	$params ) {
    
    	$iopt=array();
    	$iopt["icon"] = "ic.del.gif";
    	htmlInfoBox( $title, "", "open", "INFO", $iopt );
    	echo '<div style=" text-align: center;">';
    	echo $asktext."\n";
    	
    	echo "<form name=\"delform\" action=\"".$delurl."\" method=post>\n";
    	
    	foreach( $params as $key=>$val) {
    		echo '<input type="hidden" name="'.$key.'" value="'.$val.'">'."\n";
    	}
    	
    	
    	echo '<br><input type=submit value="YES" class="yButton">'."\n";
    
    	echo "</form>\n";
    	echo '</div>';
    	htmlInfoBox( "", "", "close" );
    
    }

}