<?php
require_once ("visufuncs.inc");
require_once ('o.ABS_SOCKET.subs.inc');
require_once ("f.objview.inc");	

/**
 * extension for ABS_SOCKET
 * @swreq UREQ:xxxxxxxxxxxx
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @app_type_info contaions code for type:2021_abbott
 */
class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        
        $id = $this->id;
        $o_rights = $this->o_rights;
        
    	echo "<ul>\n";

    	if ( $_SESSION['s_product']["type"]=='2021_abbott') {
    	    
    	    require_once ('o.ABS_SOCKET.NAT.subs.inc');
    	    echo "<ul>\n";
    	    $absCartLib = new oABS_SOCKET_NAT_subs();
    	    $objLinkLib = new fObjViewC();
    	    $absCartLib->init($sqlo, $id);
    	    $sua_id = $absCartLib->get_SUA();
    	    $infox=NULL;
    	    if ($sua_id) {
    	        $infox['expiryDays'] = $absCartLib->getExpiryDays($sqlo);
    	    }
    	    $bom_id = $absCartLib->get_BOM($sqlo);
    	    $sua_id = $absCartLib->get_SUA();
    	    
    	    $tabobj = new visufuncs();
    	    $dataArr= NULL;

    	    if ($sua_id) {
    	        $sua_html = $objLinkLib->bo_display( $sqlo, 'ABSTRACT_SUBST', $sua_id );
    	    } else {
    	        $sua_html = '<img src="images/i13_err.gif" title="missing"> MISSING!';
    	    }
    	    $dataArr[] = array( 'Verbundene SUA:', $sua_html, "via ObjectLink");
    	    
    	    $tmp_exp = $infox['expiryDays'];
    	    if (!$tmp_exp) {
    	        $tmp_exp = '<img src="images/i13_err.gif" title="missing"> MISSING!';
    	    }
    	    $dataArr[] = array( 'ExpiryDays:',$tmp_exp, "via vario:ExpiryDays");
    	    
    	    $xnotes=  "via meta:Object link children:Materialstammsatz";
    	    if ($bom_id) {
    	        
    	        $bom_html  = $objLinkLib->bo_display( $sqlo, 'ABSTRACT_PROTO', $bom_id );
    	        $dataArr[] = array( 'BOM:',$bom_html,  $xnotes);
    	    } else {
    	        $dataArr[] = array( 'BOM:','???',$xnotes);
    	    }
    	    $headOpt = array( "title" => "Weitere Infos", "headNoShow" =>1);
    	    $headx   = array ("Key", "Val", "Notes");
    	    $tabobj->table_out2($headx, $dataArr,  $headOpt);
    	    
    	} else {
    	    
    	    $filename =  $_SESSION['s_sessVars']['AppLabLibDir'] . '/lablib/o.ABS_SOCKET.showcart.inc';
    	    if (file_exists($filename)) {
    	        require_once $filename;
    	        
    	        $show_lib = new oABS_SOCKET_showcart($sqlo, $id);
    	        $show_lib->show($sqlo);
    	        echo "<br>\n";
    	        echo "<br>\n";
    	    }

        	$absCartLib = new oABS_SOCKET_subs();
        	$absCartLib->init($sqlo, $id);
        	$sua_id = $absCartLib->get_SUA();
        	$objLinkLib = new fObjViewC();
        	
        	$infox=NULL;
        	if ($sua_id) {
        		$infox['expiryDays'] = $absCartLib->getExpiryDays($sqlo);
        	}
    
        	// $bom_id = $absCartLib->get_BOM($sqlo);
        	$sua_id = $absCartLib->get_SUA();
        	
        	$tabobj = new visufuncs();
        	$dataArr= NULL;
        	
        	$sua_action = '';
        	if ($sua_id) {
        		$sua_html = $objLinkLib->bo_display( $sqlo, 'ABSTRACT_SUBST', $sua_id );
        	} else {
        		$sua_html   = '<img src="images/i13_err.gif" title="missing"> MISSING!';
        		if ($o_rights['write']) {
        		    $sua_action = '[<a href="p.php?mod=LAB/o.ABS_SOCKET.edit&id='.$id.'&action=addsua">Create Link-SUA</a>]';
        		}
        	}
        	$dataArr[] = array( 'Verbundene SUA:', $sua_html, "via ObjectLink ".$sua_action);
        	
        	$tmp_exp = $infox['expiryDays'];
        	if (!$tmp_exp) {
        		$tmp_exp = '<img src="images/i13_err.gif" title="missing"> MISSING!';
        	}
        	$dataArr[] = array( 'ExpiryDays:',$tmp_exp, "via vario:ExpiryDays");
        	
        	/*
        	$xnotes=  "via meta:Object link children:Materialstammsatz";
        	if ($bom_id) {
        		
        		$bom_html  = $objLinkLib->bo_display( $sqlo, 'ABSTRACT_PROTO', $bom_id );
        		$dataArr[] = array( 'BOM:',$bom_html,  $xnotes);
        	} else {
        		$dataArr[] = array( 'BOM:','???',$xnotes);
        	}
        	*/
        	$headOpt = array( "title" => "Weitere Infos", "headNoShow" =>1);
        	$headx   = array ("Key", "Val", "Notes");
        	$tabobj->table_out2($headx, $dataArr,  $headOpt);
    	}
    	
    	echo "</ul>\n";
    }


}


