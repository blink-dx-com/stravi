<?php

/**
 * useful methods for REA_LOG
 * @package o.REA_LOG.subs.inc
 * @swreq UREQ:0000921 (o.CHIP_READER > REA_LOG : service log abbilden/bearbeiten) 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 * @version $Header: trunk/src/phplib/o.REA_LOG.subs.inc 59 2018-11-21 09:04:09Z $
 */
class oREA_LOG_subs2 {
	

    
    /**
     * Quali_erwartet
     * Qualifizierung
     * @var array
     */
    static $actionColors = array(
        'Repaired'=>'#008000',
        'Problem'=>'#FF3333',
        'STOP'   =>'#FF0000',
        'Working'=>'#008000',
        'unknown'=>'#808080'
        
    );
    
    
    // transform key => ACCEPT value or array(values)
    static function ACCEPT_key2id($key) {
        $val=NULL;
        if ($key=='START')  $val=3;
        if ($key=='END')    $val=array(4,5,6);
        if ($key=='FINISH') $val=4;
        return $val;
    }
	
	/** OLD:
	 * encode string to escape the COLON ":"
	 * @param string $input
	 * @return mixed
	 
	private function _decode_form_value($input) {
		$temp_str = str_replace('&#58;', ':',$input);
		return $temp_str;
	}
	*/
	
	/**
	 * @swreq UREQ:11339 FS-ID:FS-QA04-g o.DEV > save data for IPC in "service log"
	 * - transform NOTES of one REA_LOG to key/val array inside the [FORM] ... [/FORM] construct
	 * - the return data transforms the escpace-sequence "&#58;" into COLONs again
	 * FORMAT:
	 *  - column-delimiter: COLON :
	 *  - escape char for COLON is: &#58;
	 * @return array of ( KEY => array('KEY'=>,'STATUS'=>, 'NOTES'=>, 'custome_keys'=>) )
	 * @example of data 
	 * --- INPUT:
	 * 
		DTM-Nr:6:ok
		Pfad:Z&#58;\Produktion\Alle\2017\035_QC\071_IQC\01_Standard:
		UDTMean:34.67:Test-Bemerkung&#58;aha
		
		
		--- OUTPUT:
		Array
		(
	    [DTM-Nr] => Array
	        (
	            [KEY] => DTM-Nr
	            [STATUS] => 6
	            [NOTES] => ok
	        )
	
	    [Pfad] => Array
	        (
	            [STATUS] => Z:\Produktion\Alle\2017\035_QC\071_IQC\01_Standard
	            [NOTES] => 
	        )
	
	
	    [UDTMean] => Array
	        (
	            [STATUS] => 34.67
	            [NOTES] => Test-Bemerkung:aha
	        )

		)
	*/
	public static function get_XDATA_keyvals($xdata) {
		
	    $nowStrArr = json_decode($xdata, TRUE);
		if (empty($nowStrArr)) $nowStrArr=array();
		
		$now_infoarr = NULL; // array[KEY] = array('STATUS'=>,'NOTES'=>);

		foreach( $nowStrArr as $row) {
		   $key   = $row['KEY'];
		   $now_infoarr[$key]=$row;
		}
		return $now_infoarr;
	}
}
