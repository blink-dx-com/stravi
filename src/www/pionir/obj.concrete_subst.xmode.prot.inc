<?php
require_once ('subs/obj.concrete_subst.xmode.sub.inc');
require_once 'o.MORDER.subs.inc';
require_once 'o.CCT_ACCLOG.subs.inc';
require_once ('o.H_ALOG_ACT.subs.inc');
require_once ('f.msgboxes.inc'); 
require_once ("f.objview.inc");
/**
 * - manage protocol steps
 * - it contains code, which will be included in edit.tmpl.php, when the rider is set to "MODE"
 * @package obj.concrete_subst.xmode.prot.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oCONCRETE_SUBST_xmode_prot {
	
    function __construct( &$editLib ) {
    	$this->editLib = &$editLib;
    }	
    
    private function _allow_by_MORDER($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $id=$this->objid;
        // analyse production workflow ...
        $morder_id = oMORDER_subs::SUC_has_order($sqlo, $id);
        if (!$morder_id) {
            return 1;
        }
        // check status
        
        $out=0;
        $accLogLib   = new oAccLogC();
        $accLogLib->setObject( $sqlo, 'MORDER', $morder_id );
        $now_status_id = $accLogLib->getLastLog($sqlo);
        
        $allow_names= array(
            oMORDER_subs::STATUS_PROD_START,
            oMORDER_subs::STATUS_PROD_CLOSED,
            oH_ALOG_ACT_subs::STATUS_reopen
        );
        $allow_ids = array();
        foreach($allow_names as $st_name) {
            $status_id_x =oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, 'MORDER', $st_name );
            $allow_ids[] = $status_id_x;
        }
        
        if ($now_status_id) {
            if ( array_search($now_status_id, $allow_ids) !== FALSE) {
                $out=1;
            }
        }
        
        if ($error->Got(READONLY))  {
            $error->printAll();
            $error->reset();
            return 0;
        }
        
        if (!$out) {
            echo "<br><br>\n";
            $tmp_text = "The MAC protocols <b>cannot</b> be modified.<br>Reason: ".tablename_nice2('MORDER').
            ': '.fObjViewC::bo_display( $sqlo, 'MORDER', $morder_id).
            ' must be on one of these states: '.implode(' or ',$allow_names).'!';
            if (glob_isAdmin()) {
                $out=1;
                $tmp_text .= '<br><b>ADMIN overrules !!!</b>';
            }
            cMsgbox::showBox("warning",  $tmp_text); 
            echo "<br>\n";
        }
        
        return $out;

    }
    
    function xmode_start( &$sqlo, &$sqlo2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {
    	
        
        if ($arguByKey['DEVIATION']!=NULL) {
            echo "<ul>\n";
            cMsgbox::showBox("warning", "<b>Deviations found:</b> ". htmlspecialchars($arguByKey['DEVIATION']));
            echo "</ul>\n";
        }
        
        $p_options=array();
        $p_options['qcEditAllow'] = 0;
        $p_options['userWants'] = $this->editLib->getEditMode();
        if ($p_options['userWants']=='edit' and $x_rights['insert']) $p_options['qcEditAllow'] = 1;
        $p_options['xtype_def']=1;
        
        $this->objid = $id;
        $allow_by_order = $this->_allow_by_MORDER($sqlo);
        if (!$allow_by_order) {
            $p_options['qcEditAllow'] = 0; // BAEMMMM!
        }
    	
    	$mainLib = new oSUC_xmode_prot_help();
    	$mainLib->show($sqlo, $sqlo2, $id, $arguByKey, $tabArgu, $p_options);
    	
    }

}