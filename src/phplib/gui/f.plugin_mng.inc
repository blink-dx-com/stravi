<?php
/**
 * manage the plugin, load the plugin, check the plugin-class
 * @package f.plugin_mng.inc
 * @swreq 
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class f_Plugin_MngPage { 
    
    /**
     * features of plugin module
     * @var array $oMODULE_feats
     *   'user.fav.allow' => 0,1
     *   'MXID' => ID of module
     *   'LOCKED' => 0,1
     */
    private $oMODULE_feats;
    private $module_file;
    private $mod;
    
    
    /**
     * check ROLE access to plugin
     * @param object $sqlo
     * @param string $mod
     * @return int
     *   0 : deny
     *   >0: allow
     */
    private function _check_role(&$sqlo, $mod) {
        
        $answer = 0;
        $rolename='PLUGIN:'.$mod;
        
        // check, if exists
        if (!role_plugin_right_exists($sqlo, $rolename)) return 1;
        
        // check, if user has the right
        $answer = role_check_p( $sqlo, $rolename);
        if ($answer=='execute') return 2;
        
        return 0;
    }
    /**
     * check the plugin-module
     * 
     * @example
     * $classname = $mng_lib->includeMod($sqlo, $mod);
     * $plugLib = new $classname($sqlo);
       $plugLib->register();
     * 
     * @param $mod
     * @return string $classname
     */
    function includeMod(&$sqlo, $mod) {
        global $error;
        $FUNCNAME = __CLASS__.':includeMod';
        
        $this->oMODULE_feats = array();
        $this->mod=$mod;
        
        $basekeys = array(
            'DEF'=> '../../plugin',
            'LAB'=> '../' . $_SESSION['globals']["lab_path"].'/plugin',
            'TST'=> '../../_test/misc/plugin',
        );
        $BASELEN  = 3; // 3 letters !!!
        
        if ($mod==NULL) {
            $error->set( $FUNCNAME, 1, 'no Plugin name given.' );
            return;
        }
        if (strstr($mod,'..')!=NULL) {
            $error->set( $FUNCNAME, 2, 'Plugin-path invalid.' );
            return;
        }
        $basekeyX = substr($mod,0,$BASELEN);
        if ( $basekeys[$basekeyX]==NULL ) {
            $error->set( $FUNCNAME, 3, 'No support for Base:'.$basekeyX.'.' );
            return;
        }

        $pureModPath = substr($mod,$BASELEN+1); // without '/'
        $BASE_DIR    = $basekeys[$basekeyX];
        $relFile     = $BASE_DIR.'/'.$pureModPath.'.inc';
        $this->module_file = $relFile;
        
        if (!file_exists($relFile)) {
            $error->set( $FUNCNAME, 4, 'Plugin "'.$relFile.'" not found.' );
            return;
        }
        require_once($relFile);
        
        $pureModPathArr = explode('/',$pureModPath);
        $namePos   = sizeof($pureModPathArr)-1;
        $pureModName    = $pureModPathArr[$namePos];
        $classname = $pureModName .'_XPL';
        
        // replace "." and "-" in filename
        $classname = str_replace('.','_',$classname);
        $classname = str_replace('-','_',$classname);
        if (!class_exists($classname)) {
            $error->set( $FUNCNAME, 5, 'Plugin "'.$relFile.'": class:'.$classname.' not found.' );
            return;
        }
        
        return ($classname);
    }
    
    // check permissions
    function check_permission($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $mod = $this->mod;
        
        // check, if registered in MODULE-table
        // TBD: what, if multiple MXIDs occure ==> analyse new flag MAIN_MOD_FLAG=1
        $this->oMODULE_feats = glob_elemDataGet3( $sqlo, 'MODULE',
            array('LOCATION'=>'PLUGIN:'.$mod, 'TYPE'=>3), array('MXID', 'LOCKED') );
        if ($this->oMODULE_feats['MXID']) {
            // module found, check if LOCKED. only root can perform the action ...
            if ($this->oMODULE_feats['LOCKED']>0 and $_SESSION['sec']['appuser']!='root') {
                $error->set( $FUNCNAME, 6, 'Plugin "'.$mod.'" is LOCKED by system-configuration. Ask the admin to UNLOCK.' );
                return;
            }
            
            // allow user faviorite selection?
            $tmpVarioLib = new oS_VARIO_sub('MODULE');
            $tmp_meta = $tmpVarioLib->getValByKey($sqlo, $this->oMODULE_feats['MXID'], 'user.fav.allow');
            $this->oMODULE_feats['user.fav.allow'] = $tmp_meta;
            
        }
        
        $access_ok = $this->_check_role($sqlo, $mod);
        if ($access_ok<=0 and $_SESSION['sec']['appuser']!='root') {
            $error->set( $FUNCNAME, 7, 'Plugin "'.$mod.'": you have no permission to use this plugin.' .
                ' Ask the admin to get the role-right for this plugin.' );
            return;
        }
    }
    
    function get_module_file_name() {
        return $this->module_file;
    }
    
    /**
     * get features of this plugin, stored in MODULE
     */
    function get_MODULE_features() {
        return $this->oMODULE_feats;
    }
    
    
    
    // general error, before the plugin was initialized ...
    function pageInitError(&$sqlo) {
        global $error;
        $infoarr			 = NULL;
        $infoarr['scriptID'] = '';
        $infoarr['title']    = 'Plugin manager';
        $infoarr['form_type']= 'tool';
        $infoarr['design']   = 'norm';
        $infoarr['locrow']   = array( array('home.php', 'home') );
        $infoarr["modLockChecked"] = 1;  // do not check MODULE:LOCKED here !
        $pagelib = new gHtmlHead();
        $pagelib->startPage($sqlo, $infoarr);
        
        $error->printAll();
        
        $pagelib->htmlFoot();
        
    }
    
    /*
     * get the $_GET parameters to produces a valid forward-URL
     * - this is needed, if the session was killed,
     * - the forward-URL is automatically send to index.php (the login-page) to recall this plugin
     */
    static function Get_Params() {
        $input = &$_GET;
        if (!is_array($input)) return;
        
        $url = '?dummy=1';
        reset($input);
        foreach( $input as $key=>$val) {
            if(is_array($val)) {
                foreach( $val as $akey=>$aval) {
                    $url .= '&'.$key.'['.$akey.']='.$aval;
                }
            } else {
                $url .= '&'.$key.'='.$val;
            }
        }
        
        
        return $url;
    }
}
