<?php
require_once 'f.directorySub.inc';
require_once ( "javascript.inc" );

/**
 * widget manager
 * SEQUENCE:
 * 1. new $classname
 * 2. __set_input($in_arr)
 * 3. register()
 * 4. __check_input()
   5. check_input()
   OPTIONAL: get_extra_title()
   6. startMain()
 * 
 * @package f.widget_MGR.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class f_widget_ABS {
    
    /**
     * 
     * @var array
     *   'title'
     *   't' : TABLE : optional
     *   'in_arr_def' -- expected config input
     *     array of array
     *     'var' : variable name
     *     'title': string
     *     'notes': notes
     *     'type' : 'INT', 'STRING', 'LIST_OF_INT', 'LIST_OF_STRING'
     *     'req'  : TRUE, FALSE
     *     '_dict_': sub dict , if 'var_type'=='DICT'
     */
    protected $infoarr;
    protected $__in_arr;
    
    function __construct(&$sqlo, $widget_id) {
        $this->_sqlo = &$sqlo;
        $this->_sqlo2= NULL;
        $this->_mod  = $widget_id;
    }
    
    // set sqlo2 from extern
    function __set_sqlo2($sqlo2) {
        $this->_sqlo2 = &$sqlo2;
    }
    // register the second SQL-handle
    function &_plug_regSql2() {
        if ($this->_sqlo2==NULL) {
            $this->_sqlo2  = logon2(  );
        }
        return $this->_sqlo2;
    }
    
    function __getInfoarr() {
        return $this->infoarr;
    }
    function __set_input(&$in_arr) {
        $this->__in_arr = $in_arr;
    }
    
    // RECURSIVE method ...
    private static function _check_input_sub(&$input, &$definitions, $basevar) {
        
        if (!empty($definitions)) {
            
            foreach($definitions as $row) {
                
                $var=$row['var'];
                
                if (!empty($row['_dict_'])) {
                    self::_check_input_sub($input[$var], $row['_dict_'], $var);
                }
                if ($row['req']) {
                    if ($input[$var]===NULL or $input[$var]==='') {
                        $var_nice=$var;
                        if ($basevar) {
                            $var_nice=$basevar.':'.$var;
                        }
                        throw new Exception('Input-Parameter "'.$var_nice.'" not set.');
                    }
                }
            }
        }
    }
    
    
    function __check_input() { 
        $input = &$this->__in_arr;
        $definitions = &$this->infoarr['in_arr_def'];
        
        $basevar=NULL;
        self::_check_input_sub($input, $definitions, $basevar);
    }
    protected function _forward_page($url, $link_text) {
        js__location_replace($url, $link_text );
        exit;
    }
    
    function register() {}
    
    // get extra title, e.g. after analyzing the config
    //function get_extra_title() {}
    
    function check_input() {}
    function startMain() {}
       
}

class fWidgets_MGR {

    function __construct($widget_id) {
        $this->_wid_lib=NULL;
        $this->widget_id = $widget_id;
    }
    
    
    
    private function _load_widget( &$sqlo, &$in_arr ) {
        global $error;
        //$FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $widget_id =  $this->widget_id;
        
        $full_name= self::widget_full_file($widget_id);
        if (!file_exists($full_name)) {
            throw new Exception('Widget "'.$full_name.'" not found.');
            
        }
        require_once($full_name);
        
        $pureModPathArr = explode('/',$widget_id);
        $namePos   = sizeof($pureModPathArr)-1;
        $modName   = $pureModPathArr[$namePos];
        
        $lenWithoutInc = strlen($modName);
        $pureModName   = substr($modName,0,$lenWithoutInc); // without .inc
        $classname = $pureModName.'_XWI';
        $classname = str_replace('.','_',$classname);
        if (!class_exists($classname)) {
            throw new Exception('Widget "'.$widget_id.'": class:'.$classname.' not found.');
        }

        $this->_wid_lib = new $classname($sqlo, $widget_id);
        $this->_wid_lib->__set_input($in_arr);
        $this->_wid_lib->register();
    }
    
    function load($sqlo, &$in_arr) {
        $this->_load_widget( $sqlo, $in_arr );
    }
    
    function set_sqlo2(&$sqlo2) {
        $this->_wid_lib->__set_sqlo2($sqlo2);
    }
    
    function get_infoarr() {
        return $this->_wid_lib->__getInfoarr();
    }
    
    static function get_widget_dir() {
        return $_SESSION['s_sessVars']['AppRootDir'].'/widget';
    }
    
    static function widget_full_file($widget_id) {
        $BASELEN=4; // DEF/
        $basekeyX   = substr($widget_id,0,$BASELEN); // needed later
        $widget_key = substr($widget_id,$BASELEN);
        $short_file = $widget_key.'.inc';
        return self::get_widget_dir().'/'.$short_file;
    }
    
    
    
    /**
     * test the config input
     */
    function test_config() {
        $this->_wid_lib->__check_input();
        $this->_wid_lib->check_input();
    }
    
    /**
     * array
     *   'w' : widget name
     *   'wopt' : widget-specific options
     *
     * @param object $sqlo
     */
    function run($sqlo) {
        $this->_wid_lib->__check_input();
        $this->_wid_lib->check_input();
        $this->_wid_lib->startMain();
    }
    
}


class fWidgets_all_MGR {
    
    static function get_all_widget_files($sqlo) {
        $base_dir = fWidgets_MGR::get_widget_dir();
        $scanLib = new fDirextoryC();
        $filearr = $scanLib->scanDir($base_dir, '.inc');
        return $filearr;
    }
    
    /**
     *
     * @param object $sqlo
     * @return array
     *    'wid'
     *    'name'
     */
    static function get_all_widget_info($sqlo) {
        
        $LEN_EXT=4; // '.inc'
        
        $filearr = self::get_all_widget_files($sqlo);
        if(empty($filearr)) return array();
        
        $in_arr=array(); // empty
        
        $out=array();
        foreach($filearr as $short_file) {
            
            $widget_id = 'DEF/'.substr($short_file,0, -$LEN_EXT);
            
            $widget_lib = new fWidgets_MGR($widget_id);
            $widget_lib->load($sqlo, $in_arr);
            $infoarr = $widget_lib->get_infoarr();
            
            $title = $infoarr['title'];
            $out[] = array( 'wid'=>$widget_id, 'name'=>$title);
        }
        return $out;
    }
}



