<?php
require_once 'o.DB_USER.subs.inc';
require_once 'insert.inc';
require_once 'date_funcs.inc';

/**
 * methods for CCT_NOTES
 * @author Steffen Kube
 * @package o.CCT_NOTES.sub.inc
 */
class oCCT_NOTES_sub {
    
    function __construct(&$sqlo, $tablename, $objid) {
        $this->tablename = $tablename;
        $this->objid = $objid;
        
        $pk_col = PrimNameGet2($tablename);
        // get cct_access
        $this->CCT_ACCESS_ID = glob_elementDataGet( $sqlo, $tablename, $pk_col, $objid, 'CCT_ACCESS_ID');
    }
    
    /**
     * add entry
     * @param object $sqlo
     * @param array $argu_in
     */
    function add(&$sqlo, $argu_in) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if ($argu_in['NAME']==NULL) {
            $error->set( $FUNCNAME, 1, 'Input missing: name' );
            return;
        }
        
        // get last POS
        $sqlsel = "max(POS) from CCT_NOTES where CCT_ACCESS_ID=".$this->CCT_ACCESS_ID;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $max_pos = $sqlo->RowData[0];
        if (!$max_pos) $max_pos = 0;
        $new_pos = $max_pos +  1;
        
        $argu = array();
        $argu['CCT_ACCESS_ID'] = $this->CCT_ACCESS_ID;
        $argu['POS']        = $new_pos;
        $argu['NAME']       = $argu_in['NAME'];
        $argu['MOD_DATE']   = date_unix2datestr( time(),1 );
		$argu['DB_USER_ID'] = $_SESSION['sec']['db_user_id'];
		$argu['NOTES']      = $argu_in['NOTES'];
        
        $dummy_id = insert_row_s($sqlo, 'CCT_NOTES', $argu );
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on insert of entry on POS:'.$new_pos );
            return;
        }
        
    }
    
    
    
   
}