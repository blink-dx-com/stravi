<?php
/**
 * layer.my config
 *
 * @package g.layer.my_config.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $go 
 * @param $parx
 * @param $action
 */
require_once ('func_form.inc');
require_once ('gui/f.widget_MGR.inc');
require_once ('gui/f.layer_my.inc');
require_once  ( $_SESSION['s_sessVars']['loginPATH'].'/subs/layer.my.subs.inc');


class g_layer_my_config_XPL extends gPlugin {
    
    function register() {
        $title		= 'my world > Configuration';
        
        $this->infoarr			 = NULL;
        $this->infoarr['title']    = $title;
        $this->infoarr['form_type']= 'tool';
        $this->infoarr['design']   = 'norm';
        $this->infoarr['locrow']   = array(array('home.php','Home'));
    }

    function startMain() {
        global $error;
        $FUNCNAME='startMain';
        
        $action = $_REQUEST['action'];
        $go     = $_REQUEST['go'];
        $parx   = $_REQUEST['parx'];
        $sqlo   = &$this->_sqlo;
        $sqlo2= &$this->_plug_regSql2();
        
        $main_lib = new g_theme_my_cls();
        $main_lib->set_config_mode(1);
        $main_lib->show($sqlo, $sqlo2);
    }
    
}
