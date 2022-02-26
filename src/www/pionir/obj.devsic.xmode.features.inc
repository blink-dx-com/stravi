<?php
/**
 * DEVSIC extension, edit the Parameter-Definition-FORM
 * calls plugin DEF/o.DEVSIC.infoarr
 * - infoarr: 
	 * 	t   =>�title�, doppelpunkt verboten (!)
	 *  type=>�checkbox�, �select�, �text�, 
	 *  �n� => notes
 *  speichern in REA_LOG als:
 *  [FORM]
 *  title:VAL:notes
 *  [/FORM]
 * @package obj.devsic.xedit.php
 * @swreq SRD-002: Provide an Equipment Service Concept (OLD:UREQ:0001458)
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

require_once ("gui/o.DEVSIC.gui.inc");

class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
        
        $argu     = &$this->argu;
        $o_rights = &$this->o_rights;
        $viewmode = $this->viewmode;
        
        $helplib = new oDEVSIC_INFOARR_gui($id);
    	echo "<ul>\n";
    	
    	$checkarr = $helplib->get_info_struct($sqlo);
    	if ($viewmode=="edit" and $o_rights['write']>0) {
    		$helplib->editfrom($checkarr);
    	} else {
    		$helplib->viewfrom($checkarr);
    	}
    	echo "</ul>\n";
    }
}
