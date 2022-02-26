<?php 
/**
 * extend for Crea_WIZ
 * @package o.MORDER.crea_wiz.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
require_once 'lev1/glob.obj.crea_wiz_objabs.inc';
require_once('edit.edit.inc');
require_once 'javascript.inc';
require_once 'func_form.inc';
require_once 'o.CHIP_READER.prodline.inc';

class oMORDER_objCreaWiz_G extends objCreaWiz_OBJ_gui {
    
    function __construct() { 
        $this->_internals['submit.text']     = ''; 
    }
    
    function has_form05() {
        $value = 1;
        return $value;
    }
    
    function next_go_val() {
        return 7;
    }
    
    function create_now_flag($create_now) {
        $create_now=1;
        return $create_now;
    }
    
  
    /**
     * 
     * {@inheritDoc}
     * @see objCreaWiz_OBJ_gui::show_form_0_5()
     * @param object  $htmlXObj htmlZeugC
     */
    function show_form05( &$sqlo, &$htmlXObj) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $htmlXObj->form_row_raw('', 'Name', $this->obj_name, '');
        $tablename='MORDER';
        
        if ($this->selecter=='empty') {
 
            $jsFormLib = new gJS_edit();
            
            $pre_sel_sua=$this->newparams['ABSTRACT_SUBST_ID'];
            $obj_name='';
            if ($pre_sel_sua) $obj_name = obj_nice_name($sqlo, 'ABSTRACT_SUBST', $pre_sel_sua);
            $x = $jsFormLib->getAll('ABSTRACT_SUBST', 'newparams[ABSTRACT_SUBST_ID]', $pre_sel_sua, $obj_name,  1);
            $htmlXObj->form_row_raw('', columnname_nice2($tablename,'ABSTRACT_SUBST_ID' ) , $x, '');
            
            
            // get all prod_lines
            $all_devs = o_CHIP_READER_prodline::get_all_prodlines($sqlo);
            if (empty($all_devs)) {
                $error->set( $FUNCNAME, 1, 'No productionlines defined in the system (VARIO:is.prodline). Please ask your Admin!' );
                return;
            }
            
            $all_devs_nice = array();
            foreach($all_devs as $dev_id_loop) {
                $all_devs_nice[$dev_id_loop] = obj_nice_name($sqlo, 'CHIP_READER', $dev_id_loop);
            }
            $x = formc::selectFget('newparams[PROD_LINE_ID]', $all_devs_nice, $this->newparams['PROD_LINE_ID']);
            $htmlXObj->form_row_raw('', columnname_nice2($tablename,'PROD_LINE_ID' ), $x, 'registered production lines');
            
            $this->_internals['submit.text']='Next'; // otherwise create
           
        } else {
            $this->_internals['submit.text']='Create'; 
        }
    }

}

class oMORDER_objCreaWiz extends objCreaWiz_OBJ {
    
    function preCheck(&$sqlo, $obj_name, &$newparams, $upload) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $tablename='MORDER';
        $req_fields=array(
            'ABSTRACT_SUBST_ID',
            'PROD_LINE_ID' 
        );
        if ($this->selecter=='empty') {

            $defvals=array();
            if (!$newparams['ABSTRACT_SUBST_ID']) {
                $error->set( $FUNCNAME, 1, columnname_nice2($tablename,'ABSTRACT_SUBST_ID' ) .' must be set.');
                return;
            }
            if (!$newparams['PROD_LINE_ID']) {
                $error->set( $FUNCNAME, 2, columnname_nice2($tablename,'PROD_LINE_ID' ) .' must be set.');
                return;
            }
            $prodline_lib = new o_CHIP_READER_prodline($sqlo, $newparams['PROD_LINE_ID']);
            $dev_workflow_id = $prodline_lib->get_def_MORDER_wfl($sqlo);
            if (!$dev_workflow_id)  {
                $error->set( $FUNCNAME, 3, columnname_nice2($tablename,'PROD_LINE_ID' ) .' ID:'.$newparams['PROD_LINE_ID'].' needs workflow-settings.');
                return;
            }
            
            foreach($req_fields as $key) {
                $defvals[$key] = $newparams[$key];
            }
        }
        
        return array('obj_name'=>$obj_name, 'defvals'=>$defvals, 'create_now'=>1 );
    }
}


