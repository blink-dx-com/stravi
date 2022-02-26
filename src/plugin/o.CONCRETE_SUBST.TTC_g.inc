<?php
require_once ("f.objview.inc");
require_once dirname(__FILE__).'/../www/pionir/impexp/suc_ref/o.CONCRETE_SUBST.TTC.inc';
require_once dirname(__FILE__).'/../www/pionir/impexp/suc_ref/o.CONCRETE_SUBST.TTC_gui.inc';

/**
 * @package o.CONCRETE_SUBST.TTC_g.inc
 * @swreq REQ:41 BEADS > REF-Beads > Target-Type CHECKER
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @created 2021-06-14
 * @param int $id
 *
 */
class o_CONCRETE_SUBST_TTC_g_XPL extends gPlugin {
	
	function register() {
		$tablename	= 'CONCRETE_SUBST';
		$title		= 'Protocol-Ref-Checker';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title_sh'] = $title;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj';
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		$this->infoarr['locrow']   = array(  // defines the tool location-path
		    array('p.php?mod=DEF/o.CONCRETE_SUBST.TTC_home', o_LINK_TTC::TOOL_NAME), 
		); 

		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array('read', 'write') )
			);

	}
	
	function form1() {  
	    $initarr   = array();
	    $initarr["title"]       = "Update this Batch?";
	    $initarr["submittitle"] = "Update"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    $hiddenarr = NULL; // hidden form variables
	    $hiddenarr['id'] = $this->objid;
	    $formobj = $this->_html->formInit($initarr, $hiddenarr);
  
	    $formobj->close( TRUE );       
	}
	
	function _show_diff($sqlo, &$bead_gui_lib) {
	    $diff     = $bead_gui_lib->get_diff_arr();
	    $ref_bead = $bead_gui_lib->get_ref_doc();
	    $diff_lib = new o_CONCRETE_SUBST_TTC_gui($this->objid, $ref_bead);
	    if(empty($diff)) $diff=array();
	    $diff_lib->diff_details($sqlo, $diff);
	    echo "<br>\n";
	    if ( $_SESSION["userGlob"]["g.debugLevel"]>=1 ) {
	        echo "DIFF: RAW-structure:<pre>";
	        print_r($diff);
	        echo '</pre>';
	    }
	}

	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$tablename	= 'CONCRETE_SUBST';
		
		$sqlo = &$this->_sqlo;
		//$sqlo2= &$this->_plug_regSql2();

		//$li_options = $_REQUEST['parx']; // o_SUC_bead_ana3
		$go = $_REQUEST['go']; 
		$this->objid = $_REQUEST['id'];

		$bead_gui_lib = new o_CONCRETE_SUBST_TTC($sqlo);
		if ($error->Got(READONLY))  {
		    $error->set( $FUNCNAME, 1, 'init error.' );
		    return;
		}
		
		$bead_gui_lib->set_suc($sqlo, $this->objid);
		
// 		$target = $bead_gui_lib->get_target_type();
// 		if ($target==NULL) {
// 		    echo 'MAC has no defined TARGET-type.<br>';
// 		    return;
// 		}
// 		echo "Target-Type: ".$target."<br>";
		$answer = 0;
		$ref_bead = $bead_gui_lib->get_ref_doc();
		if ($ref_bead) {
		    
		    $version = $bead_gui_lib->get_curr_ref_version($sqlo);
		    echo 'Already has '.o_LINK_TTC::CONFIG_NAME_USER.': ';
		    $obj_link = fObjViewC::	bo_display( $sqlo, 'LINK', $ref_bead);
		    echo $obj_link." Version:".$version."<br>\n";	    
		    $answer = $bead_gui_lib->check1_suc($sqlo); 
		    
		    if ($answer!=1) {
		        $this->_show_diff($sqlo, $bead_gui_lib);
		    }
		    
		} else {
		    echo 'Has currently no linked '.o_LINK_TTC::CONFIG_NAME_USER.'.<br><br>';
		    $doc_arr = $bead_gui_lib->check0_get_configs($sqlo);
		    if (empty($doc_arr)) {
		        echo 'No Config found for this MAA.';
		        return;
		    }
		    foreach($doc_arr as $doc_id_loop) {  
		        $error->reset(); // reset error for new run ...
		        
		        $obj_link = fObjViewC::	bo_display( $sqlo, 'LINK', $doc_id_loop);
		        echo 'Analyse Config: '.$obj_link.'<ul>';
		        $answer = $bead_gui_lib->check0_this_doc($sqlo, $doc_id_loop);
		        if ($answer==1) {
		            echo '</ul>'."\n";
		            break;
		        } else {
		            $this->_show_diff($sqlo, $bead_gui_lib);
		        }
		        echo '</ul>'."\n";
		    }
		    
    		
		}
		echo "<br>";
		
		
		if ($answer==1) {
  
		    $this->_html->messageBox2('ok', 'This is a standard MAC.');
		    echo "<br>";
		    
		    if (!$go) {
		        $this->form1();
		        return;
		    }
		    
		    echo "start UPDATE ...<br>";
		    $bead_gui_lib->update_suc($sqlo);
		} else {
		    $error->printAllEasy();
		    echo "<br>";
		    
		}

		
// 		echo 'REF-Bead: ';
// 		$obj_link = fObjViewC::	bo_display( $sqlo, 'LINK', $ref_bead);
// 		echo $obj_link;
		echo "<br>";
	}
}
