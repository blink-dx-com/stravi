<?php 
/**
 * Extended array functions
 * @package f.array_extended.inc
 * @swreq UREQ:
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/f.array_extended.inc 59 2018-11-21 09:04:09Z $
 */
class array_extended {
    
    static function arrayRecursiveDiff_oneway(&$aArray1, &$aArray2) {
        
        $aReturn = array();
        
        foreach ($aArray1 as $mKey => $mValue) {
            if (array_key_exists($mKey, $aArray2)) {
                if (is_array($mValue)) {
                    $aRecursiveDiff = self::arrayRecursiveDiff($mValue, $aArray2[$mKey]);
                    if (count($aRecursiveDiff)) { $aReturn[$mKey] = $aRecursiveDiff; }
                } else {
                    if ($mValue != $aArray2[$mKey]) {
                        $aReturn[$mKey] = $mValue;
                    }
                }
            } else {
                $aReturn[$mKey] = $mValue;
            }
        }
        
        return $aReturn;
    }
	/**
	 * array_diff recursive
	 * @param array $aArray1
	 * @param array $aArray2
	 * @return multitype:unknown - difference of arrays
	 */
	static function arrayRecursiveDiff(&$aArray1, &$aArray2) {
	    
	    $aReturn = self::arrayRecursiveDiff_oneway($aArray1, $aArray2);
	    if (empty($aReturn)) {
	        $aReturn = self::arrayRecursiveDiff_oneway($aArray2, $aArray1);
	    }
		
		return $aReturn;
	}
	
}