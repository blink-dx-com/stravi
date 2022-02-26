<?php
require_once ('object.info.inc');

/**
 * helper methods for BCBATCH
 * @package o.BCBATCH.subs.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 *
 */
class oBCBATCH_subs{
    
    function __construct($bcbatch_id=NULL) {
        if ($bcbatch_id) {
            $this->bcbatch_id = $bcbatch_id;
        }
        
    }
    
    function get_mother_SUC(&$sqlo) {
        
        if (!$this->bcbatch_id) {
            sys_error_my( 'Class not initialized.' );
        }
        
        $suc_id = glob_elementDataGet( $sqlo, 'BCBATCH', 'BCBATCH_ID', $this->bcbatch_id, 'MO_SUC_ID');
        return $suc_id;
    }
    
    /**
     * transform barcode "0012-033442" to (0012,033442)
     * @param string $barcode
     * @return array(batch,pos)
     */
    private function _barcode2BC($barcode) {
        $output = explode('-', $barcode);
        if (sizeof($output)<=1) $output = array('', $barcode);
        return $output;
    }
    
    function get_carts_range($sqlo) {
        /**
         * get Cartridge Code range
         * @return array  
         * 'code0' =>$code0,  with leading zeros
         * 'code1' =>$code1,  with leading zeros
         * 'batch' => string of batch (to analyse the leading zeros, e.g. 00023
         */
        if (!$this->bcbatch_id) {
            sys_error_my( 'Class not initialized.' );
        }
        
        $sqladd = 'from SOCKET where SOCKET_ID in (select SOCKET_ID from BCBATCH_H_SOC where BCBATCH_ID='.$this->bcbatch_id .')';
        
        $sqlsel = 'min(BARCODE) '. $sqladd;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $code0 = $sqlo->RowData[0];
 
        $sqlsel = 'max(BARCODE) '. $sqladd;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $code1 = $sqlo->RowData[0];
        
        return array( 'code0'=>$code0, 'code1'=>$code1 );
    }
    
    /**
     * get ParamsRaw
     * - and features of ABS_SOCKET
     * @param object $sqlo
     * @return array
     *   'vals':
     *     'NAME'
     *     'PROD_DATE'
     *     'EXPIRY_DATE'
     *   'misc'
     *      'ABS_SOCKET'
     *          'C_ASSAY_ID' - customer Assay-ID
     */
    function get_params($sqlo) {
        
   
        $objInfoLib = new objInfoC();
        $params     = $objInfoLib->getParamsRaw ( $sqlo, 'BCBATCH',  $this->bcbatch_id);
        
        
        
        if ($params['vals']['MO_SUC_ID']) {
            // get data from related SUC
            $SUC_param = glob_elemDataGet3( $sqlo, 'CONCRETE_SUBST', array('CONCRETE_SUBST_ID'=>$params['vals']['MO_SUC_ID'] ),
                array( 'SOME_DATE', 'EXPIRY_DATE' ) );
            $params['vals']['PROD_DATE']   = $SUC_param['SOME_DATE'];
            $params['vals']['EXPIRY_DATE'] = $SUC_param['EXPIRY_DATE'];
        }
        
        if ($params['vals']['ABS_SOCKET_ID']) {
            $ABS_SOCKET_param = glob_elemDataGet3( $sqlo, 'ABS_SOCKET', array('ABS_SOCKET_ID'=>$params['vals']['ABS_SOCKET_ID']), 
                array('C_ASSAY_ID') );
            $params['misc']['ABS_SOCKET'] = $ABS_SOCKET_param;
        }
        return $params;
    }
    
}