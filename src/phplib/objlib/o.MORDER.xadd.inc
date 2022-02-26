<?php
require_once('glob.obj.create.ABS.inc');
require_once 'gStatusTriggerAbs.inc';
require_once 'o.CHIP_READER.prodline.inc';
require_once 'o.MORDER.subs.inc';

/**
 * object specific modification class
 * @package oMORDER.xadd.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oMORDER_xmodC  extends gObjCreateC_ABS  {
    
    /**
     * RULE:
     * try to get a workflow ...
     * A) get workflow_id from ABSTRACT_SUBST_ID
     * B) get workflow_id from PROD_LINE_ID
     * {@inheritDoc}
     * @see gObjCreateC_ABS::post_crea()
     */
    function post_crea( &$sqlo, &$sqlo2, $objid,  $blueObjId, $option  = array()	) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
 
        $this->_add_log("manage workflow ID ...");
        
        $mord_feats = glob_ObjDataGet( $sqlo, 'MORDER', $objid, array('PROD_LINE_ID', 'ABSTRACT_SUBST_ID'));
        $prod_line_id = $mord_feats['PROD_LINE_ID'];
        $sua_id = $mord_feats['ABSTRACT_SUBST_ID'];
        $workflow_id = 0;
        
        do {
           
            $por_lib = new oMORDER_subs($objid);
            $workflow_id = $por_lib->get_POR_wfl_fr_SUA($sqlo);
            if ($workflow_id) {
                debugOut("get Workflow:$workflow_id from SUA:".$sua_id, $FUNCNAME, 2);
                break;
            }
            if ($prod_line_id) {  
                $prodline_lib = new o_CHIP_READER_prodline($sqlo, $prod_line_id);
                $workflow_id = $prodline_lib->get_def_MORDER_wfl($sqlo);
                if ($workflow_id) debugOut("get Workflow:$workflow_id Production-line-ID:$prod_line_id", $FUNCNAME, 2);
            }
            
        } while (0);
        
        if ($workflow_id) {
            $this->_add_log("use workflow ID: $workflow_id");
            $trigger_lib = new gStatusTriggerDef();
            $trigger_lib->setTrigger2Obj($sqlo, $workflow_id, 'MORDER', $objid);
        }
        
    }
}
