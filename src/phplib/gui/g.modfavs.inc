<?php
require_once ('o.USER_PREF2.subs.inc');
require_once ('gui/f.plugin_mng.inc');
require_once 'o.MODULE.subs.inc';
/**
 * manage module/plugin favorites
 * OLD: $_SESSION['userGlob']['modfavs']
 * NEW: table:USER_PREF2
 *   LIST of favs: KEY='fav.plugin.list'
 *   ONE favorite: KEY='fav.plugin'
 * 
 * ------
 * JSON-string of ONE favorite in table:USER_PREF2 : 
 *   array( -- plug_fav_ONE_STRUCT --
 *     'mxid'=>,
 *     'title'=>... 
 *     't' => [OPTIONAL] if TABLE related: the table name
 *     'conf'=>array(
 *        KEY=>VAL  ... will be passed to PLUGIN via ??? $_REQUEST ?
 *     ) 
 *     )
 *     
 *
 * @package g.modfavs.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @var array in_arr_def_STRUCT -- expected config-structure input of the plugin
 *      saved in PLUGIN infoarr['user.fav.conf']
     *     array of array 
     *     'var' : variable name
     *     'title': string
     *     'notes': notes
     *     'type' : 'INT', 'STRING', 'LIST_OF_INT', 'LIST_OF_STRING'
     *     'req'  : TRUE, FALSE
     *     '_dict_': sub dict , if 'var_type'=='DICT'
 */

/**
 * ONE FAV
 *
 */
class g_modfavs_one {
    
    const FAV_ONE_KEY='fav.plugin';
    private $data; // plug_fav_ONE_STRUCT
    
    function __construct($sqlo, $fav_id) {
        $this->fav_id=$fav_id;
        $this->data = oUSER_PREF2_subs::my_get_by_id($sqlo, $fav_id);
    }
    function get_data() {
        return $this->data;
    }
    function get_fav_config() {
        return $this->data['conf'];
    }
    
    // get configured URL parameters
    function get_url_params() {
        if(empty($this->data['conf'])) return;
        
        $p_arr=array();
        foreach($this->data['conf'] as $key=>$val) {    
            $p_arr[]='&'.$key.'='.urlencode($val);
        }
        $p=implode('',$p_arr);
        return $p;
    }
    
    // update data ...
    function update_title($title) {
        $title=trim($title);
        if ($title==NULL) throw new Exception('Input-Error: TITLE is empty.');
        $this->data['title']=$title;
    }
    
    function update_conf($sqlo, $conf_arr) {
        $this->data['conf']=$conf_arr;
        oUSER_PREF2_subs::my_update_by_id($sqlo, $this->fav_id, $this->data);
    }
    static function create($sqlo, $args) {
        $fav_id = oUSER_PREF2_subs::my_insert_entry($sqlo, self::FAV_ONE_KEY, $args);
        return $fav_id;
    }
    
    /**
     * get the FORM vars of the plugin
     * @param object $sqlo
     */
    function get_plugin_config($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;

        $data=$this->get_data();
        $mxid=$data['mxid'];
        $mod_info = g_modfavs::get_mod_info($sqlo, $mxid);
        $mod = $mod_info['mod'];
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on module.' );
            return;
        }
        $plugin_config = g_modfavs::get_plugin_config($sqlo, $mod);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on Plugin-analysis.' );
            return;
        }
        return $plugin_config;
    }
}


class g_modfavs {
    
    const table='USER_PREF2';
    const MX_TYPE_PLUGIN=3;
    const FAV_LIST_KEY='fav.plugin.list';
    static $fav_arr; // list of favs ...
    
    static function get_my_favs($sqlo) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!glob_table_exists(self::table)) return array();

        self::$fav_arr = oUSER_PREF2_subs::my_get_key_val($sqlo, self::FAV_LIST_KEY);

        if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
            debugOut('fav_arr:'.print_r(self::$fav_arr,1), $FUNCNAME);
        }

        return self::$fav_arr;
    }
    
    static function save_internal($sqlo) {
        oUSER_PREF2_subs::my_update_key_val($sqlo, self::FAV_LIST_KEY, self::$fav_arr);
    }
    

    static function mx_is_valid($sqlo, $mxid) {
        $mx_features = glob_ObjDataGet( $sqlo, 'MODULE', $mxid, array('TYPE', 'TITLE', 'LOCATION'));
        if ( $mx_features['TYPE']!= self::MX_TYPE_PLUGIN ) {
            return -1;
        }
        if ($mx_features['TITLE']==NULL) {
            return -2;
        }
        if ($mx_features['LOCATION']==NULL) {
            return -3;
        }
        
        return 1;
    }
    

    /**
     * 
     * @param object $sqlo
     * @param int $mxid
     * @return array
     *   $new_fav_id int
     *   $create_flag int
            1 : added
        *   2 : already in
     */
    static function add_to_favs(object $sqlo, int $mxid) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;

        $out=0;
        $fav_arr = self::get_my_favs($sqlo);
        
        $mxid=intval($mxid);
        if (self::mx_is_valid($sqlo, $mxid)<1) {
            $error->set( $FUNCNAME, 1, 'Module is not valid for a favorite.' );
            return;
        }
        
        
        // revalidate all favorites ...
        $fav_arr_new=array();
        $found=0;
        foreach($fav_arr as $fav_id) {
            
            $one_fav_lib = new g_modfavs_one($sqlo, $fav_id);
            $row = $one_fav_lib->get_data();
            
            $mxid_loop=$row['mxid'];
            if (self::mx_is_valid($sqlo, $mxid_loop)>0) {
                $fav_arr_new[] = $fav_id;
            }
            if ($mxid_loop==$mxid) {
                $found=1;
                $new_fav_id = $fav_id;
            }
        }
        
        if ($found) {
            $out=2;
            
        } else {
            if (self::mx_is_valid($sqlo, $mxid)>0) {
                
                $mod_info = self::get_mod_info($sqlo, $mxid);
                $args     = array('mxid'=>$mxid,'title'=>$mod_info['title']);
                if ($mod_info['t'])  $args['t']=$mod_info['t'];
                $new_fav_id = g_modfavs_one::create($sqlo, $args);
                $fav_arr_new[] = $new_fav_id;
                $out=1;
            }
        }
        self::$fav_arr = $fav_arr_new;
        self::save_internal($sqlo);
        
        return array($new_fav_id, $out);
    }
    
    static function remove_my_many($sqlo, array $fav_arr_del) {
        
        $fav_arr = self::get_my_favs($sqlo);
        
        $fav_arr_new=array();
        foreach($fav_arr as $fav_id_loop) {
            if (!in_array($fav_id_loop, $fav_arr_del)) {
                $fav_arr_new[]=$fav_id_loop;
            }
        }
        self::$fav_arr = $fav_arr_new;
        
        self::save_internal($sqlo);
    }
    
    static function remove_all($sqlo) {
        self::$fav_arr = array();
        self::save_internal($sqlo);
    }
    
    /**
     * 
     * @param object $sqlo
     * @param int $mxid
     * @return array 
     *  'name'
     *  'title'
        'url'
        't'
     */
    static function get_mod_info(object $sqlo, int $mxid) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $answer = array();
        
        $mxid=intval($mxid);
        $sqlsel = 'NAME, TITLE, LOCATION, TYPE from MODULE where TYPE=3 and MXID='.$mxid ;
        $sqlo->Quesel($sqlsel);
        if ( $sqlo->ReadArray() ) {
            $data = $sqlo->RowData;
            $location = $data['LOCATION'];
        } else {
            $error->set( $FUNCNAME, 1, 'Plugin '.$mxid.' not found.' );
            return;
        }
        
        
        $keyx='PLUGIN:';
        if (substr($location,0, strlen($keyx))!=$keyx) {
            $error->set( $FUNCNAME, 2, 'Plugin '.$mxid.' has no correct location!' );
            return;
        }
        
        // get more 
        $module_lib = new oMODULE_one($mxid);
        $meta_val_table   = $module_lib->get_meta_by_key($sqlo, 'TABLE_NAME');
        
        $this_mod = substr($location, strlen($keyx));
        
        $answer['name']  = $data['NAME'];
        $answer['title'] = $data['TITLE'];
        $answer['url']   = 'p.php?mod='.$this_mod;
        $answer['plugin.type']   = $keyx;
        $answer['mod']   = $this_mod;
        if ($meta_val_table) {
            $answer['t']   = $meta_val_table;
        }
        
        return $answer;
    }
    
    /**
     * load plugin and read some REGISTER vars
     * can throw errors ..
     * @param object $sqlo
     * @param string $mod
     * @return array
     *   'config'=> array() : in_arr_def_STRUCT
     *   'error_txt'=> string
     */
    static function get_plugin_config($sqlo, $mod) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        $plugin_lib = new f_Plugin_MngPage();
        $classname  = $plugin_lib->includeMod($sqlo, $mod);
        $plugLib    = new $classname($sqlo);
        $plugLib->register();
        $infoarr    = &$plugLib->infoarr;
        
        $config = $infoarr['user.fav.conf'];
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
            debugOut('(104) in_arr_def'.print_r($in_arr_def,1), $FUNCNAME, 0);
        }
        
        return $config;
    }
}
 