<?php
require_once ("edit.show.inc");
require_once ("f.objview.inc");
require_once 'db_x_obj.inc';

/**
 * - show SUA
 * @package obj.concrete_subst.xmode.abs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oCONCRETE_SUBST_xmode_abs {
	
    function __construct( &$editLib ) {
    	$this->editLib = &$editLib;
    }	
    
    private function _columns(&$argus) {
        
        $tablename = $this->abs_tablename;
        $colNamesNew  = array();
        $argu_new     = array();

        $colNames_ori = columns_get_pos($tablename);

        foreach( $colNames_ori as $colName ) {
            // foreach( $colNames_ori as $dummy=>$colName) {
            $colFeatures = colFeaturesGet2($tablename, $colName );
            $useCol=0;
            if ($colFeatures['VISIBLE']>0) $useCol=1;
            
            
            if ($argus[$colName]==='' or $argus[$colName]===NULL) {
                $useCol = 0;
            }
            
            
            if ($useCol) {
                $colNamesNew[]=$colName;
                $argu_new[$colName] = $argus[$colName];
            }
            
        }
        
        
        return array($colNamesNew, $argu_new);
    }
    
    function xmode_start( &$sqlo, &$sqlo2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {
    	
        $abs_id = $arguByKey['ABSTRACT_SUBST_ID'];
        $this->abs_tablename='ABSTRACT_SUBST';
        $abs_tablename = $this->abs_tablename;
       
        $objLinkLib = new fObjViewC();
        $html_tmp = $objLinkLib->bo_display( $sqlo, $abs_tablename, $abs_id );
        echo '<br> &nbsp;&nbsp; Main Features of <b>template</b> '.$html_tmp.'<br><br>'."\n";
        
        
        $showFormLib = new fFormShowC();
        $showFormLib->setObject($abs_tablename, $abs_id);
        
        $colDefOpt = $showFormLib->x_cols_extra($sqlo, $arguByKey);
        
        $argus = glob_elemDataGet3( $sqlo, $abs_tablename, array('ABSTRACT_SUBST_ID'=>$abs_id), array('*') );
        
        $extraobj_o_abs = fVarcolMeta::get_args_by_id( $sqlo, $argus['EXTRA_OBJ_ID'] );
        list($colNamesForm, $argus) = $this->_columns($argus);

        $formopt=array();
        //$formopt["colModeSplit"] = 1;
        //$formopt['HasAdvCols']   = 1;
        
        
        
        // if ($colMode=='adv')
        // 	$editLib->_infoout('This form shows advanced attributes of this object.');
        $showFormLib->form_show( $sqlo, $sqlo2, $argus,  $colNamesForm,  $extraobj_o_abs, $formopt, $colDefOpt );
    	
    }

}