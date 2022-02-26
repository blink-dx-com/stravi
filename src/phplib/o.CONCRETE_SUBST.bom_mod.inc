<?php 
require_once ("f.objview.inc");	

/**
 * modify BOM of SUC
 * @author Steffen
 * @package o.CONCRETE_SUBST.bom_mod.inc
 *
 */
class oCONCRETE_SUBST_bom_mod {
    
    function __construct() {
        $this->assoclib = new  fAssocUpdate();
    }
    
    function set_mother($sqlo, $mother_suc_id){
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->mother_suc_id = $mother_suc_id;
        $this->assoclib->setObj( $sqlo, 'SUC_BOM', $this->mother_suc_id );
    }
    
    function get_last_pos($sqlo) {
        $sqlsel = 'max(POS) from SUC_BOM where CONCRETE_SUBST_ID='.$this->mother_suc_id;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $last_pos = $sqlo->RowData[0];
        return $last_pos;
    }
    
    /**
     * update/create
     * @param object $sqlo
     * @param int $upos
     * @param array $args
     *   'COMPO_SUC_ID'
     *   'APOS'
     */
    function update_pos(&$sqlo, int $upos, array $args) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$this->mother_suc_id) sys_error_my( 'Class not initialized.' );
        
        unset($args['CONCRETE_SUBST_ID']);
        unset($args['POS']);
        
        if ($upos<=0) {
            $error->set( $FUNCNAME, 1, 'No UPos given.' );
            return;
        }
        if ($args['APOS']<=0) {
            $error->set( $FUNCNAME, 5, 'No APos given.' );
            return;
        }
        if ($args['COMPO_SUC_ID']) {
            if ($this->mother_suc_id==$args['COMPO_SUC_ID'])  {
                $error->set( $FUNCNAME, 10, 'Parent and Component cannot be the same!' );
                return;
            }
        }
        
        $old_pos = glob_elementDataGet2($sqlo, 'SUC_BOM', array('CONCRETE_SUBST_ID'=>$this->mother_suc_id, 'POS'=>$upos), 'POS');
        
        $idarr=array('POS'=>$upos);
        if (!$args['COMPO_SUC_ID']) {
            if ($upos>0) {
                $this->assoclib->delOneRow($sqlo, $idarr);
                return;
            }
            return;
        }
        
        if ($old_pos) {
            $this->assoclib->update( $sqlo, $args, $idarr);
            return;
        }
        
        
        $args['POS']=$upos;
        $this->assoclib->insert($sqlo, $args);
        
    }
    
    /**
     * get order substances
     * @param $sqlo
     * @param $suc_id
     * @param int $upos UNIQUE POS
     * @param int $apos POS from SUA
     * @param $suc_id
     * @param $quant
     */
    function add_one_suc(&$sqlo, int $upos, int $apos, $suc_id, $quant) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$this->mother_suc_id) sys_error_my( 'Class not initialized.' );

        if ($upos<=0) {
            $error->set( $FUNCNAME, 1, 'No UPos given.' );
            return;
        }
        if ($apos<=0) {
            $error->set( $FUNCNAME, 5, 'No APos given.' );
            return;
        }
        
        $argu=array();
        $argu['POS']=$upos;
        $argu['APOS']=$apos;
        $argu['QUANTITY']= $quant;
        $argu['COMPO_SUC_ID']=$suc_id;

        $this->assoclib->insert($sqlo, $argu);
        
    }
}