<?php
require_once 'o.USER_PREF.manage.inc';

/**
 * my lyer manager : config
 * @package f.layer_my.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class fWidgets_my_show {
    
    /**
     *
     * @var array of
     *   'wids'=> array of
     *    array()
     *      'wind' => int internal widget index
     *      'wid' => string widget name
     *      'conf'=> array()
     */
    private $_conf;
    
    function __construct() {
        $this->_conf = array('wids'=>array());
        if ($_SESSION["userGlob"]['layer.my']) {
            $this->_conf = unserialize($_SESSION["userGlob"]['layer.my']);
        }
    }
    
    function get_my_widgets() {
        return $this->_conf['wids'];
    }
    
    function get_wids_index($wind) { 
        $index=-1;
        $wid_index = array_keys($this->_conf['wids']);
        foreach($wid_index as $i) {
            if ($this->_conf['wids'][$i]['wind']==$wind) {
                $index = $i;
                break;
            }  
        }
        return $index;
    }
    function get_max_wind() {
        $max_wind=0;
        $wid_index = array_keys($this->_conf['wids']);
        foreach($wid_index as $i) {
            if ($this->_conf['wids'][$i]['wind']>$max_wind) {
                $max_wind = $this->_conf['wids'][$i]['wind'];
            }
        }
        return $max_wind;
    }
    
    function get_wid_conf($wind) {
        $ind = $this->get_wids_index($wind);
        if ($ind<0) {
            throw new Exception('Widget-index "'.$wind.'" not found in your config');
        }
        $wid_conf=$this->_conf['wids'][$ind]['conf'];
        return $wid_conf;
    }
    
    function get_wid($wind) {
        $ind = $this->get_wids_index($wind);
        if ($ind<0) {
            throw new Exception('Widget-index "'.$wind.'" not found in your config');
        }
        return $this->_conf['wids'][$ind]['wid'];
    }
    
    function get_all_of_wid($wind) {
        $ind = $this->get_wids_index($wind);
        if ($ind<0) {
            throw new Exception('Widget-index "'.$wind.'" not found in your config');
        }
        return $this->_conf['wids'][$ind];
    }
    
    function add($widget_id) {
        $wind_new = $this->get_max_wind()+1;
        $this->_conf['wids'][] = array(
            'wid' => $widget_id,
            'conf'=> array(),
            'wind'=> $wind_new
        );
        return $wind_new;
    }
    
    function delete($wind) {
        $ind = $this->get_wids_index($wind);
        if ($ind>=0) {
            unset($this->_conf['wids'][$ind]);
        }
    }
    
    function widget_set_conf($wind, $config_arr) {
        $ind = $this->get_wids_index($wind);
        if ($ind<0) {
            throw new Exception('Widget-index "'.$wind.'" not found in your config');
        }
        $this->_conf['wids'][$ind]['conf']=$config_arr;
        
    }
    
    /**
     * 
     * @param array $wind_arr list of wind
     * @throws Exception
     */
    function resort_all(array $wind_arr) {
        
        $new_config = array();
        foreach ($wind_arr as $wind) {
            $ind = $this->get_wids_index($wind);
            if ($ind<0) {
                throw new Exception('Widget-index "'.$wind.'" not found in your config');
            }
            $new_config[] = $this->_conf['wids'][$ind];
        }
        $this->_conf['wids'] = $new_config; 
    }
    
    function save($sqlo) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
            debugOut('_conf: '.print_r($this->_conf,1), $FUNCNAME);
        }
        
        $value = serialize($this->_conf);
        oUSER_PREF_manage::entry_update_on_change($sqlo, 'layer.my', $value);
    }
}
