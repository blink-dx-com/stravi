<?php
require_once ('javascript.inc');
require_once ('func_form.inc');
require_once ("f.objview.inc");
require_once ("visufuncs.inc");
require_once dirname(__FILE__).'/o.LINK.TTC.inc';


/**
 * GUI to manage a Material-ReferenceConfig
 * @package o.LINK.TTC.GUI.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class o_LINK_SUC_REF_config_SHOW {
    
    private $edit_flag=0;
    private $obj_id;
    private $action;
    private $parx;
    
    function __construct($sqlo, $obj_id, $_mod, $action, $parx) {
        $this->obj_id=$obj_id;
        $this->_mod=$_mod;
        $this->action=$action;
        $this->parx=$parx;
        
        $this->doc_lib = new o_LINK_TTC($sqlo, $this->obj_id);
    }
    
    function set_edit_flag(int $flag) {
        $this->edit_flag=$flag;
    }
    function set_edit_version($edit_vers) {
        $this->doc_lib->set_edit_version($edit_vers);
    }
    
    private function _plug_get_URL() {
        return 'p.php?mod='.$this->_mod;
    }
    
    
    private function _one_step_PRE($sqlo, &$row, &$x_one_step, $edit_flag=0) {
        
        $step       = $row['STEP_NR'];
        $step_name  = $row['NAME'];
        $sua_id_ORI = $row['ABSTRACT_SUBST_ID'];
        $css_bold='';
        
        $obj_link_USE=NULL;
        $sua_id_USE=$x_one_step['sua'];
        
        if ($sua_id_USE) {
            $obj_link_USE = fObjViewC::	bo_display( $sqlo, 'ABSTRACT_SUBST', $sua_id_USE);
            $checked='checked';
            $css_bold='yGgrayBold';
        }

        $obj_link_ORI = fObjViewC::	bo_display( $sqlo, 'ABSTRACT_SUBST', $sua_id_ORI);
        echo '<li> <span class="'. $css_bold. '">'.$step_name.'</span> ';
        //if ($edit_flag) echo '<input type=checkbox name="parx[step]['.$step.']" value="1" '.$checked.'> '."\n";
 
        if ($this->edit_flag) echo ' ORI:'.$obj_link_ORI;
        echo ' | USE:'.$obj_link_USE;
        
    }
    private static function _one_step_CLOSE() {
        echo '</li>'."\n";
    }
    
    /**
     * SHOW/EDIT
     * @param object $sqlo
     * @param array $pra_steps
     * @param array $x_data -- VERS_PROTO_PRA_STEPS_STRUCT
     * @param array $pra_features
     * @param int $edit_flag
     */
    function show_ONE_protocol(object $sqlo, $pra_steps, $x_data, $pra_features, int $edit_flag) {
        
        $x_data_steps=$x_data['steps'];
        
        if ($edit_flag) {
            $initarr=array();
            
            js_formAll();
            
            $hiddenarr = NULL;
            $hiddenarr['id'] = $this->obj_id;
            $hiddenarr['parx[sua]'] = $pra_features['sua'];
            $hiddenarr['parx[pno]'] = $pra_features['pno'];
            $hiddenarr['action'] = 'pra_edit';
            
            $f = new formPure($initarr, $hiddenarr, 0);
            $f->start();
        }
        
        if ($edit_flag) {
            
            $this->jsFormLib = new gJS_edit();
            
            foreach($pra_steps as $row) {
                $is_active=0;
                $step   = $row['STEP_NR'];
                $sua_ORI= $row['ABSTRACT_SUBST_ID'];
                if(!$sua_ORI) {
                    continue;
                }
                
                $sua_id_NAME='';
                $sua_id_USE='';
                $x_one_step = o_LINK_TTC::get_data_of_STEP_STRUCT($x_data_steps,$step);
                
                if (!empty($x_one_step)) {
                    if ($x_one_step['sua']) {
                        $is_active=1;
                        $sua_id_USE=$x_one_step['sua'];
                        $sua_id_NAME=obj_nice_name ( $sqlo, 'ABSTRACT_SUBST', $x_one_step['sua'] );
                    }
                }
                
                $row['active']=$is_active;
                $this->_one_step_PRE($sqlo, $row, $x_one_step, $edit_flag);
                $fieldopt=array();
                
                echo $this->jsFormLib->getAll( 'ABSTRACT_SUBST', 'parx[sua_sel]['.$step.']', $sua_id_USE,
                    $sua_id_NAME, $step, $fieldopt );
                
                $this->_one_step_CLOSE();
            }
        } else {
            
            $mo_sua = $pra_features['sua'];
            $mo_pno = $pra_features['pno'];
            
            foreach($pra_steps as $row) {
                $step=$row['STEP_NR'];
                $x_one_step = o_LINK_TTC::get_data_of_STEP_STRUCT($x_data_steps,$step);
                
                if (!empty($x_one_step)) {
                    $this->_one_step_PRE($sqlo, $row, $x_one_step);
                    $loop_sua = $x_one_step['sua'];
                    
                    if ($this->edit_flag and $loop_sua) {
                        
                        $url = $this->_plug_get_URL().'&id='.$this->obj_id.'&parx[sua]='.$mo_sua.'&parx[pno]='.$mo_pno.'&parx[st]='.$step;
                        
                        if (empty($x_one_step['sua_link'])) {
                            $action_link ='[<a href="'.$url.'&action=sua_pra_act">Activate Protocols</a>] &nbsp;';
                            $exists_text='';
                        } else {
                            $is_active=1;
                            $exists_text='<b>Proctocol activated</b>';
                            $action_link ='[<a href="'.$url.'&action=sua_pra_deact">Deactivate Protocols</a>]';
                        }
                        
                        echo ' &nbsp; <span class="'. $exists_text. '"><span>&nbsp;'. $action_link;
                    }
                    $this->_one_step_CLOSE();
                }
            }
        }
        
        if ($edit_flag) {
            echo '<input type=submit value="Submit" class="yButton">'."\n";
            $f->close();
        }
    }
    
    function show_one_SUA_full($sqlo, &$data, $sua_id) {
        $sua_struct = o_SUA_TTC_proto::get_ref_params($sqlo, $sua_id);
        
        //$f = new formPure($initarr, $hiddenarr, 0);
        //$f->start();
        $sualink = fObjViewC::	bo_display( $sqlo, 'ABSTRACT_SUBST', $sua_id);
        echo $sualink.'<ul>';
        
        foreach($sua_struct as $row) {
            
            $pra_id = $row['ap'];
            $no = $row['or'];
            $url=$this->_plug_get_URL().'&id='.$this->obj_id.'&parx[sua]='.$sua_id.'&parx[pno]='.$no;
            
            $is_active=0;
            $css_bold='';
            
            $x_data = $data[$sua_id][$no];
            $action_link='[<a href="'.$url.'&action=pra_sel">Select</a>]';
            if (empty($x_data)) {
                if (!$this->edit_flag) {
                    continue; // do not show
                }
            } else {
                $is_active=1;
                $css_bold='yGbold';
                $action_link ='[<a href="'.$url.'&action=pra_edit">EDIT</a>] &nbsp;';
                $action_link .='[<a href="'.$url.'&action=pra_deact">Deactivate</a>]';
            }
            if (!$this->edit_flag) {
                $action_link=NULL;
            }
            
            $obj_link = fObjViewC::	bo_display( $sqlo, 'ABSTRACT_PROTO', $pra_id);
            echo $no.' <span class="'. $css_bold. '">'.$obj_link. '</span> &nbsp;'. $action_link. "<br>\n";
            
            if ($is_active) {
                $this_proto_edit=0;
                if ($this->action=='pra_edit' and $sua_id==$this->parx['sua'] and $no==$this->parx['pno']) {
                    $this_proto_edit=1;
                }
                echo "<ul>";
                $pra_features=array('sua'=>$sua_id, 'pno'=>$no);
                
                $this->show_ONE_protocol($sqlo, $row['steps'], $x_data, $pra_features, $this_proto_edit );
                echo "</ul>";
            }
        }
        echo '</ul>'."\n";
        echo "<br>\n";
    }
    
    /**
     * show the CURRENT version structure
     * @param object $sqlo
     */
    function view_structure($sqlo) {
        
        $version = $this->doc_lib->current_version();
        
        echo "<b>Current Configuration:</b> &nbsp;Version: <b>".$version."</b><br><br>\n";
 
        
        $data    = &$this->doc_lib->get_data_of_version($version);
        
        if (empty($data)) {
            $sua_id = $this->doc_lib->get_SUA();
            $this->show_one_SUA_full($sqlo, $data, $sua_id);
        } else {
            foreach($data as $sua_id => $row) {
                $this->show_one_SUA_full($sqlo, $data, $sua_id);
            }
        }
    }
    
    
    function view_versions($sqlo) {
        
        $versions = $this->doc_lib->get_versions_short();
        
        if (empty($versions)) {
            echo 'No data available.<br>';
        }
       
        $tabobj = new visufuncs();
        $headOpt = array( "title" => "Versions" );
        $headx  = array ("#", "Version", "Released?");
        $tabobj->table_head($headx,   $headOpt);
        
        foreach($versions as $row) {
            
            $version_loop = $row['version'];
            $baseurl = $this->_plug_get_URL().'&id='.$this->obj_id;
            
            $act_url = $baseurl.'&action=set_act_version&parx[version]='.$version_loop;
            $action = '<a href="'.$act_url.'">EDIT</a> ';
            if ($this->edit_flag) {
                $url = $baseurl.'&action=set_valid_version&parx[version]='.$version_loop;
                $action .=' | <a href="'.$url.'">Release</a>';
            }
            $dataArr=array($action, $row['version'], $row['valid']);
            $tabobj->table_row ($dataArr);
        }
        $tabobj->table_close();
    }
}
