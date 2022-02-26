<?php
/**
 * - protocol specials
 * - it contains code, which will be included in edit.tmpl.php, when the rider is set to "MODE"
 * $Header: trunk/src/www/pionir/obj.exp.xmode.protocols.inc 59 2018-11-21 09:04:09Z $
 * @package    edit.tmpl.php
 * @subpackage obj.exp.xmode.protocols.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
require_once("o.EXP.subs.inc");
require_once 'o.EXP.mr.inc';
require_once 'gui/o.EXP.protos_show.inc';

class oEXP_xmode_protocols {

    function __construct( &$editLib ) {
        $this->editLib = &$editLib;
    }
    

    private function show_samples($sqlo) {
    
        $id = $this->exp_id;
        // Dealing with 'IS_SAMPLE'-flag:
        $samples = oEXP_subs::getSamplesWithName($sqlo, $id);
        echo '<span class="yGgrayBold">Samples of Experiment:</span> &nbsp;';
        
        if (empty($samples)) echo ' <span style="color:gray;">no samples defined.</span>'."\n";
        else {
            if (count($samples) >1) echo "<br>";
            $komma='';
            foreach($samples as $sample_id => $sample_name) {
                
                if (empty($sample_name)) $sample_name = "sample without name";
                echo $komma. "<img src=\"images/icon.SAMPLE.gif\"> ".
                    "<a href=\"edit.tmpl.php?t=CONCRETE_SUBST&id=".$sample_id."\">$sample_name</a>";
                $komma='<br>'."\n";
            }
        }
        echo "<br><br>\n";
        
    }
    
    /*
     * protocol extras
     * @param object $sql
     * @param object $sql2
     * @param object $id   
     * @param object $arguByKey
     * @param object $x_rights
     * @param object $extraobj_o
     * @param object $tabArgu
     * @param object $editAllow
     */
    function xmode_start( &$sqlo, &$sqlo2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {
    	
    	echo "<UL>";
    	$this->exp_id = $id;
 
    	$mo_edit_allow = 0;
    	$userWants = $this->editLib->getEditMode();
    	if ($userWants=='edit' and $x_rights['insert']) $mo_edit_allow = 1;
    	
    	$is_multireactor = 0;
    	
    	if($arguByKey['EXP_TMPL_ID']) {
        	$exp_mr_lib = new oEXP_mr($id);
        	$is_multireactor = $exp_mr_lib->is_multireactor($sqlo);
    	}
    	if ($is_multireactor) {
    	    
    	    require_once($_SESSION['s_sessVars']['AppLabLibDir'].'/lablib/gui/o.EXP.plate.inc');
    	    
    	    $mochi_type = oEXP_plate_static_GUI::get_mochi_type($sqlo, $id);

    	    if ($mochi_type==oEXP_plate_static_GUI::MOCHI_TYPE_MO) {
               
    	        echo 'This is a MASTER-experiment. Please look at the "Sub experiments" tab.<br>';
    	        echo "</UL>\n";
    	        return;
    	        
    	    } else {

    	        $exp_gui_lib = new oEXP_plate_CHI_GUI($sqlo, $id, $userWants, $mo_edit_allow);
    	        $exp_gui_lib->get_css();
    	        $in_blocks=array();
    	        $exp_gui_lib->show($sqlo, $in_blocks);
    	    }
    	    
    	} else {
    	    
    	    $this->show_samples($sqlo);
    	    $mainLib = new oEXP_xmode_prot_help($id);
        	$mainLib->show($sqlo, $sqlo2, $arguByKey, $tabArgu, $mo_edit_allow, $userWants, 1);
    	}
    }

}

