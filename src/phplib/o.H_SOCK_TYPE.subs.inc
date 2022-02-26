<?php
//require_once('o.S_VARIO.subs.inc');
//require_once('o.S_VARIO.mod.inc');
require_once ("glob.obj.update.inc");

/**
 * helper methods for H_SOCK_TYPE:Blink specific
 * @package o.H_SOCK_TYPE.subs.inc
 * @swreq UREQ:
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @version $Header:  59 2018-11-21 09:04:09Z $
 * 
 * H_SOCK_TYPE_fea_STRUCT:
 *  'NAME' : name of socket type; e.g. '04'
 *  'CART_MAX_POS'= 15;
 *  'xcnt' : e.g. 12;
    'ycnt' : e.g. 8;
	'ADAPTER.has' = 0,1
	'REAG.pos.enable' = array()
	'chambers'= array(
	0=> array( 'vol_m': max basis volume [microl] )
	1=>
	...
	) 
 */
class oH_SOCK_TYPE_subs {
    
    static function get_type($sqlo, $objid) {
        $features= glob_elemDataGet3( $sqlo, 'H_SOCK_TYPE', array('H_SOCK_TYPE_ID'=>$objid), array('NAME', 'KEY') );
        $type=$features['KEY'];
        if ($type==NULL) $type=$features['NAME'];
        return $type;
    }
    
    
    /**
     * CONSTANT definitions ...
     * @param object $sqlo
     * @param string $cart_type : 01, 02
     * @return array
     *   'CART_MAX_POS'
     *   'ADAPTER.has' : 0,1
     */
    private function features_by_type($cart_type) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        
        $result=array();
        
        switch ($cart_type) {
            case '01':
                // sonne
                $result['center.pos.flag']= 1;
                $result['CART_MAX_POS']= 10;
                $result['ADAPTER.has'] = 1;
                break;
                
            case '02':
                // compact
                $result['center.pos.flag']= 1;
                $result['CART_MAX_POS']=9;
                break;
            case '03':
                // chamber
                $result['center.pos.flag']= 0;
                $result['CART_MAX_POS']=1;
                break;
            case '04':
                // COMPACT 2019-10
                $result['center.pos.flag']= 1;
                $result['CART_MAX_POS']= 15;
                $result['ADAPTER.has'] = 0;
                $result['REAG.pos.enable'] = array(
                    0=>0,
                    1=>0,
                    2=>1,
                    3=>1,
                    4=>1,
                    5=>0, // waste
                    6=>1,
                    7=>1,
                    8=>1,
                    9=>1,
                    10=>1,
                    11=>1,
                    12=>1,
                    13=>1,
                    14=>1,
                    15=>1,
                );
                break;
            case '05':
                // COMPACT P3 2020-11
                $result['center.pos.flag']= 1;
                $result['CART_MAX_POS']= 13;
                $result['ADAPTER.has'] = 0;
                $result['REAG.pos.enable'] = array(
                    0=>0,
                    1=>0, // Lyseheizung
                    2=>1,
                    3=>0, // waste
                    4=>1,
                    5=>1,
                    6=>1,
                    7=>1,
                    8=>1,
                    9=>1,
                    10=>1,
                    11=>1,
                    12=>1,
                    13=>1,
                    
                );
                break;
            case '07':
                // MTP
                $result['CART_MAX_POS']= 96;
                $result['ADAPTER.has'] = 0;
                
                break;
            case '08':
                // VARIO
                $result['ADAPTER.has'] = 0;
                break;
        }
        
        
        
        return $result;
    }
    
    /**
     * get features
     * @param object $sqlo
     * @param int $objid
     * @return array H_SOCK_TYPE_fea_STRUCT
     */
    public function features_by_id($sqlo, $objid) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        #$vario_lib = new oS_VARIO_sub('H_SOCK_TYPE');
        #$data_json = $vario_lib->getValByKey($sqlo, $objid, 'defs'); 
        
        $other_features= glob_elemDataGet3( $sqlo, 'H_SOCK_TYPE', array('H_SOCK_TYPE_ID'=>$objid), array('NAME', 'JSON_DATA', 'KEY') );
        
        $type=$other_features['KEY'];
        if ($type==NULL) $type=$other_features['NAME'];
        
        $data = $this->features_by_type($type);
        
        $data_json = $other_features['JSON_DATA'];
        if ($data_json==NULL) {
            $data_t = array();
        } else {
            $data_t = json_decode($data_json, True);
            $data   = array_merge($data, $data_t);
        }
        
        debugOut("DATA:".print_r($data,1), $FUNCNAME, 3);

        $data['NAME']= $other_features['NAME'];
        
        return $data;
    }
    
    /**
     * save features in database
     * @param object $sqlo
     * @param int $objid
     * @param array $data H_SOCK_TYPE_fea_STRUCT
     * @return Null
     */
    function save_features($sqlo, $objid, $data) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        // check data
        if (!is_array($data)) {
            $error->set( $FUNCNAME, 1, 'Bad data format: no data.' );
            return;
        }
        if (empty($data['chambers'])) {
            $error->set( $FUNCNAME, 2, 'Bad data format: no chambers.' );
            return;
        }

        $data_json = json_encode($data);
        
        //$vario_mod_lib = new oS_VARIO_mod();
        //$vario_mod_lib->setObject( $sqlo, 'H_SOCK_TYPE', $objid );
        //$vario_mod_lib->updateKeyVal($sqlo, 'defs', $data_json);
        $args=array( 'vals'=>
            array('JSON_DATA'=>$data_json)
        );
        
        $UpdateLib = new globObjUpdate();
        $UpdateLib->update_meta( $sqlo, 'H_SOCK_TYPE', $objid, $args );

    }
	
	
	
	
	
	
}