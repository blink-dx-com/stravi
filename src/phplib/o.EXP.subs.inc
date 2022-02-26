<?php
require_once('o.EXP_TMPL.subs.inc');

/**
 * experiment analysis functions
 * @namespace core::obj::EXP
 * @package o.EXP.subs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/o.EXP.subs.inc 59 2018-11-21 09:04:09Z $
 */
class oEXP_subs {
    
    /**
     * 
     * @param object $sqlo
     * @param int $exp_id
     * @return int 0,1
     */
    static function is_multireactor(object $sqlo, int $exp_id) {
        
        $is_multi=0;
        
        $info = glob_elemDataGet3( $sqlo, 'EXP', array('EXP_ID'=> $exp_id), array('SOCKET_ID', 'EXP_TMPL_ID') );
        
        if ($info['SOCKET_ID']) {

            $abs_cart_id = glob_elementDataGet( $sqlo, 'SOCKET', 'SOCKET_ID', $info['SOCKET_ID'], 'ABS_SOCKET_ID');
            if(!$abs_cart_id) return -1; // unknown
            
            $H_SOCK_TYPE_ID = glob_elementDataGet( $sqlo, 'ABS_SOCKET', 'ABS_SOCKET_ID', $abs_cart_id, 'H_SOCK_TYPE_ID');
            if(!$H_SOCK_TYPE_ID) return -1; // unknown
            
            $type_key = glob_elementDataGet( $sqlo, 'H_SOCK_TYPE', 'H_SOCK_TYPE_ID', $H_SOCK_TYPE_ID, 'KEY');
            if($type_key==NULL) return -1; // unknown
        } else {
            if ($info['EXP_TMPL_ID']) {
                $exp_tmpl = new oEXP_TMPL_subs($info['EXP_TMPL_ID']);
                $type_key = $exp_tmpl->get_cart_SOCK_TYPE_KEY($sqlo);
            } else {
                return -1;
            }
        }
        
        if ($type_key=='07') {
            $is_multi=1;
        }
        
        return $is_multi;
    }
    
	/**
	 * get samples of experiment
	 * STATIC
	 * @param object $sqlo
	 * @param int $expid
	 * @return array sample_ids
	 */
	static function getSamples(&$sqlo, $expid) {
		$substarr = array();
		$sqls = "select SAMPLE_CONCRETE_SUBST_ID from SAMPLE_IN_EXP where exp_id=".$expid;
		$sqlo->query($sqls);
		while ($sqlo->ReadRow()) {
			$substid = $sqlo->RowData[0];
			$substarr[]=$substid;
		}
		return $substarr;
	}
	

	/**
	 * get sample names from sample array
	 * STATIC
	 * @param object $sqlo
	 * @param array $substarr array of IDs
	 * @return array of [ID] = name
	 */
	static function getSampleNamesOfArr(&$sqlo, $substarr) {
	    
		if (empty($substarr)) return array();
		
		$substNames=array();
		foreach( $substarr as $substid) {
			$substNames[$substid] = obj_nice_name ( $sqlo, 'CONCRETE_SUBST', $substid );
		}
		
		return $substNames;
	}
	
	static function getSamplesWithName(&$sqlo, $expid) {
	    $substarr    = self::getSamples($sqlo, $expid);
	    if (!sizeof($substarr)) return; 
	    $output = self::getSampleNamesOfArr($sqlo, $substarr);
	    return $output;
	}
	
	static function getSqlExpBySample($sampleid) {
		return 'select EXP_ID from SAMPLE_IN_EXP where SAMPLE_CONCRETE_SUBST_ID='.$sampleid;
	}
}