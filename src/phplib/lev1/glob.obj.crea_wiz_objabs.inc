<?php
/**
 * ABSTRACT class, used in glob.obj.crea_wiz_act.inc
 * @author Steffen Kube
 * @package glob.obj.crea_wiz_objabs.inc
 */

/**
 * class for object creation actions, NON-GUI
 * @author skube
 *
 */
class objCreaWiz_OBJ {
    
    protected $selecter;
    protected $obj_tmpl_id;
    protected $obj_blueprint_id;
    protected $gopt=array();
    
    function __construct($selecter, $obj_tmpl_id, $obj_blueprint_id) {
        $this->selecter    = $selecter;
        $this->obj_tmpl_id = $obj_tmpl_id;
        $this->obj_blueprint_id = $obj_blueprint_id;
    }
    function set_gopt($gopt) {
        $this->gopt=$gopt;
    }
    
    
    /**
     * - the function can modify $newparams
     * @param object $sqlo
     * @param string $obj_name
     * @param array  &$newparams IN/OUT
     * @param string $upload
     * @return array
     *   'defvals' => KEY => VAL : $this->wizardOpts['defvals']
     */
    function preCheck(&$sqlo, $obj_name, &$newparams, $upload) {
        
        return array('obj_name'=>$obj_name, 'create_now'=>0, 'defvals'=>array() );
    }
}

/**
 * ABSTRACT Class for GUI methods; used by class objCreaWizGuiSubs
 *
 */
class objCreaWiz_OBJ_gui {
    
    protected $projid=0;
    protected $selecter='';
    protected $obj_blueprint_id;
    protected $obj_name;
    protected $gopt; // option array
    protected $submit_opt=array();
    protected $wizardOpts;
    protected $obj_helper_lib;
    
    /**
     * alternative creation wiz URL
     * array('script'=>xxx, 'params'=> yyyy) - alternativ Wizard URL
		      example: 'altUrl' => array('script'=>'p.php', 'params'=> '&mod=LAB/p.129.subst_crea&parx[projid]')
     */
    protected $alternate_wiz=NULL; 
    
    /**
     * internals
     * @var array $_internals
     *   'submit.text'
     *   'next.go' : int
     */
    protected $_internals=array();
    protected $infoarr=array();
    protected $newparams;
    
    function __construct() {
    }
    
    function setProject($projid) {
        $this->projid= $projid;
    }
    
    function set_selecter($selecter) {
        $this->selecter=$selecter;
    }
    function set_obj_selections( &$obj_helper_lib, $obj_blueprint_id, $obj_tmpl_id, $obj_name, $gopt, $go, $wizardOpts) {
        
        $this->obj_helper_lib  = &$obj_helper_lib;
        $this->obj_blueprint_id= $obj_blueprint_id;
        $this->obj_tmpl_id     = $obj_tmpl_id;
        $this->obj_name =$obj_name;
        $this->gopt=$gopt;
        $this->go = $go;
        $this->wizardOpts= $wizardOpts;
    }
    function set_newparams($newparams) {
        $this->newparams = $newparams; 
    }
    
    function has_feature($key) {
        return $this->_internals[$key];
    }
    function _add_info($text) {
        $this->infoarr[]=$text;
    }
    function get_info() {
        return $this->infoarr;
    }
    function get_alternate_wiz() {
        return $this->alternate_wiz;
    }
    function get_param($key) {
        switch ($key) {
            case 'newparams':
                return $this->newparams;
            default:
                return "KEY:UNKNOWN";
                
        }
    }
    
    //
    // CAN be OVERWRITEN
    //
    
    // called after set_obj_selections()
    function init() {}
    
    function next_go_val() {
        return 10;
    }
    
    function has_form05() {
        return 0;
    }
    
    function has_param_trafo() {
        return 0;
    }
    function create_now_flag($create_now) {
        return $create_now;
    }
    
    // return array('name'=>string, 'policy_has'=>0,1)
    function get_new_obj_name($sqlo) {}
    
    /**
     * 
     * @param object $sqlo
     * @param object $htmlXObj
     * @return int $go new go value
     */
    function show_form05( &$sqlo, &$htmlXObj) {  }
    
    // extra row in form0
    function form0_extra_rows(&$sqlo, &$htmlXObj) {}
    
    function form07_paramcheck( &$sqlo ) {  }
    function show_form07( &$sqlo, &$htmlXObj) {  }
    
    // return new_param
    function do_param_trafo(&$newparams) {
        return $newparams;
    }
    
    function sh_options($sqlo) {}
}