<?php
require_once ("f.objview.inc");
require_once 'o.ABSTRACT_PROTO.stepx.inc';
require_once 'o.PROTO.steps.inc';
require_once ("gui/f.colors.inc");
require_once ('func_form.inc');

/**
 * show protocol compare table
 * @package o.CONCRETE_PROTO.li_show1.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @example
 * 
 *  if (sizeof($all_proto_arr)>1) {
        $tool_opts['many_PRA_flag'] = 1;
    }
    
    $show_objects = array('mo_arr'=>&$suc_inp_arr,  'mo_table'=>'CONCRETE_SUBST');
    
    $prot_edit_lib = new o_CONCRETE_PROTO_li_show1($sqlo, $show_objects, $tool_opts);
    $prot_edit_lib->open_table($sqlo);
    foreach($all_proto_arr as $one_cp_arr) {
        $prot_edit_lib->show_one_pra($sqlo, $one_cp_arr);
    }
    $prot_edit_lib->table_close();
 */
class o_CONCRETE_PROTO_li_show1 {
    
    private $edit_flag;
    private $pra_step_use; // array of [] = array('st'=>, 'ty'=>)
    private $pra_step_all; // array of STEP DB-column=>val
    
    private $mo_obj_arr;
    private $proto_arr;
    
    /**
     * @var array $show_opt
     *   'sh.suc.name' : 0,1
     *   'sh.step_nr' : 0,1
     */
    private $show_opt;
    
  
        
        /**
         *
         * @param object $sqlo
         * @param int $id
         * @param $show_objects
         *   'mo2_arr' array    mother of mother
         *   'mo_arr'    array  mother objects
         *   'prc_arr'   array
         *   
         *   'mo_table'    string
         *   'mo2_table' string
         *   
         * @param array $tool_opts
         *    'step_no' needed for MR-tools
         *    'form_url' $form_url
         *    'sh.step_nr' : 0,1
         *    'many_PRA_flag' : 0,1
         */
        function __construct(object $sqlo, array &$show_objects, array $tool_opts ) {
            global $error;
            $FUNCNAME= __CLASS__.':'.__FUNCTION__;

            $this->mo2_table = NULL;
            $this->mo2_obj_arr = NULL;
            if (!empty($show_objects['mo2_arr'])) {
                $this->mo2_obj_arr = &$show_objects['mo2_arr'];
                $this->mo2_table   = $show_objects['mo2_table'];
            }
            
            $this->mo_obj_arr = &$show_objects['mo_arr'];

            $this->mo_table   = $show_objects['mo_table'];
            
            $this->infox=array();
            $this->_debug   = $_SESSION["userGlob"]["g.debugLevel"];
            $this->show_opt = $tool_opts;
            
            
            $this->form_url = $tool_opts['form_url'];
   
            $this->pra_cnt=0;
            
           
        }
        
        
        private function one_proto($sqlo, $exp_id_l, $cpid, $cnt) {
            
            // $pra_id       = $this->infox['pra_id'];
            $pra_step_all = &$this->pra_step_all;
            
            $c_data=array();
            $i=0;
            foreach($pra_step_all as $row) {
                
                $p_step_nr=$row['STEP_NR'];
                //$pra_sua  =$row['ABSTRACT_SUBST_ID'];
                $c_step = gProtoOrg::cStep_pure($sqlo, $cpid, $p_step_nr);
                
                $c_data_slim=array();
                foreach($c_step as $key=>$val) {
                    if ($val==='' or $val===NULL) {
                        //no
                    } else {
                        $c_data_slim[$key]=$val;
                    }
                }
                
                $c_data[$p_step_nr]= $c_data_slim;
                
                $i++;
            }
            
            return $c_data;
            
        }
        
        
        
    private function _tab_head($sqlo) {
            
        //  echo '<th colspan=2 style="background-color: #E0E0FF;">'.$row['NAME'].'</th>';
            
            ?>
        <style>
        .xMainTable table, .xMainTable th, .xMainTable td {
          border: 1px solid #E0E0E0;
        }
        .xMainTable {
          border-collapse: collapse;
        }
        .xMainTable th, .xMainTable td {
          padding: 4px;
        }
        
        </style>
        <?php
        
        
        $sh_opt = $this->show_opt;
        
        $this->table_lead_cols = 4;
        if ($sh_opt['sh.step_nr']) $this->table_lead_cols++;

        
        echo '<table class="xMainTable">'."\n";
        
        if ( $this->_debug>0) {
            
            echo "<tr>\n";
           
            echo '<th>DEBUG:</th>';
            if ($sh_opt['sh.step_nr']) 
                echo '<th></th>';  // step_nr
            
            echo '<th></th>';  // flags
            echo '<th></th>';
            echo '<th></th>';
            
            
            foreach($this->mo_obj_arr as $cnt=>$exp_id_l) {
               
                $cpid_l   = $this->proto_arr[$cnt];
                
                
                echo '<th>'.$exp_id_l.','.$cpid_l.'</th>';
            }
            echo "</tr>\n";
        }
        
        
        if ($this->mo2_table!=NULL) {
            
            // show SUPER mothers
            echo "<tr>\n";
            if ($sh_opt['sh.step_nr'])
                echo '<th>step_nr</th>';
                
            echo '<th></th>';
            echo '<th></th>';
            echo '<th></th>';
            echo '<th>Mothers =&gt;</th>';
            
            $obj_g_lib = new fObjViewC();
            foreach($this->mo2_obj_arr as $cnt=>$obj_id_l) { 
                
                $obj_name = obj_nice_name ( $sqlo, $this->mo2_table , $obj_id_l );
                $obj_link = $obj_g_lib->bo_IDLink($this->mo2_table, $obj_id_l, $obj_name);
                echo '<th style="background-color: #A9D3BD;">'.$obj_link.'</th>';
            }
            echo "</tr>\n";
        }
        
        echo "<tr>\n";
        if ($sh_opt['sh.step_nr'])
            echo '<th>step_nr</th>'; 
        
        echo '<th>index</th>'; 
        echo '<th>flags</th>';
        echo '<th>step-name</th>';
        echo '<th>Unit</th>';
        
        $obj_g_lib = new fObjViewC();
        foreach($this->mo_obj_arr as $cnt=>$exp_id_l) {
            
            $exp_name = obj_nice_name ( $sqlo, $this->mo_table , $exp_id_l );
            $obj_link = $obj_g_lib->bo_IDLink($this->mo_table, $exp_id_l, $exp_name);
            echo '<th style="background-color: #A9D3BD;">'.$obj_link.'</th>';
        }

        echo "</tr>\n";
        
    }
    
    
    
    private static function  _get_differ_index(&$val_diff_array, $val_ori) {
        
        $c_ind = array_search($val_ori, $val_diff_array);
        if ($c_ind===FALSE) {
            $val_diff_array[] = $val_ori;
            $c_ind = array_search($val_ori, $val_diff_array);
        }

        if (!$c_ind) {
            // first number has NO special color !
            return;
        }
        
        $diff_color = fColors::DIFF_COLORS[$c_ind];
        if($diff_color==NULL) {
            // end of colors
            $diff_color='#E0CC44'; // last color
        }
        $txt_color  = fColors::get_fg_of_bg($diff_color);
        
        return array('i'=>$c_ind, 'bg'=>$diff_color, 'fg'=>$txt_color);
    }
    
    
    
    private function _init_protocols($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        
        $pra_arr=array();
        foreach($this->proto_arr as $cpid_l) {
            if ($cpid_l) {
                $pra_id = glob_elementDataGet( $sqlo, 'CONCRETE_PROTO', 'CONCRETE_PROTO_ID', $cpid_l, 'ABSTRACT_PROTO_ID');
                $pra_arr[$pra_id]=1;
            }
            
        }
        
        
        if (empty($pra_arr)) {
            $error->set( $FUNCNAME, 1, 'No protocol found for this step.' );
            return;
        }
        
        if (sizeof($pra_arr)>1) {
            $error->set( $FUNCNAME, 2, 'Different '.tablename_nice2('ABSTRACT_PROTO').'s found for these experiments. Only one type allowed.' );
            return;
        }
        
        $pra_id = key($pra_arr);
        $this->infox['pra_id'] = $pra_id;
        
        $pra_lib = new oABSTRACT_PROTO_stepx($pra_id);
        $pra_steps = $pra_lib->get_steps($sqlo);
        $pra_lib->build_index($sqlo);
        
        $this->pra_step_all=array();
        $this->pra_step_use=array();
        
        // $mand_keys = oABSTRACT_PROTO_stepx::MANDATORY_KEYS;
        $mand_trans_arr = oABSTRACT_PROTO_stepx::MANDA_KEYS2CONC;
        
        foreach($pra_steps as $step_nr) {
            
            $one_st_inf = $pra_lib->step_info($sqlo, $step_nr, 1);
            $this->pra_step_all[$step_nr]=$one_st_inf;
            
            
            if ($one_st_inf['MANDATORY_VALUES']!=NULL) {
                $mvals = trim($one_st_inf['MANDATORY_VALUES']);
                $mvals_arr = str_split($mvals);
                foreach($mvals_arr as $mkey) {
                    $conc_col = $mand_trans_arr[$mkey];
                    $this->pra_step_use[]=array('st'=>$step_nr, 'mk'=>$mkey, 'ty'=>$conc_col );
                }
            } else {
                $this->pra_step_use[]=array('st'=>$step_nr ); // show only the name
            }
            
            
        }
    }
   
    /**
     * 
     * @param array $parx
     * @param int $step_no
     * @param array $hiddenarr
     *    't'  = tablename
     *    "id" = is sometimes required input parameter
     */
    public static function form_settings($parx, $step_no, array $hidden_extra ) {
        
        $initarr   = array();
        $initarr["title"]       = "Tool settings";
        $initarr["submittitle"] = "Submit";
        $initarr["tabwidth"]    = "AUTO";
        
        $hiddenarr = NULL;
        $initarr["action"]    = $_SERVER['PHP_SELF'];
        
        $hiddenarr["step_no"] = $step_no;
        $hiddenarr["action"]  = 'settings';
        foreach($hidden_extra as $key=>$val) {
            $hiddenarr[$key]  = $val;
        }
        $formobj = new formc($initarr, $hiddenarr, 0);
        
        $fieldx = array (
            "title"   => "Show Step-Nr column",
            "name"  => 'sh.step_nr',
            "object"=> "checkbox",
            "val"    => $parx['sh.step_nr'],
        );
        $formobj->fieldOut( $fieldx ); // output the form-field
        
        $fieldx = array ( // form-field definition
        "title"   => "Show names of the ".tablename_nice2('CONCRETE_SUBST').'s',
        "name"  => 'sh.suc.name',
        "object"=> "checkbox",
        "val"    => $parx['sh.suc.name'],
        );
        $formobj->fieldOut( $fieldx ); // output the form-field
        
        $formobj->close( TRUE ); // close form, sshow submit-button
    }
    public static function settings_save($parx) {
        $_SESSION["s_formState"]['o.ALL.mr_prot_P'] = $parx;
    }
    public static function settings_get() {
        return $_SESSION["s_formState"]['o.ALL.mr_prot_P'];
    }
    
    
    function set_edit_flag($edit_flag) {
        $this->edit_flag=$edit_flag;
    }
    
    function open_table($sqlo) {
        $pra_id = $this->infox['pra_id'] ;
        $html_tmp = fObjViewC::bo_display( $sqlo, 'ABSTRACT_PROTO', $pra_id );
        
        echo '<a href="'.$this->form_url.'&action=settings" title="tool settings"><img src="res/img/settings.svg" height=25></a> &nbsp;&nbsp;&nbsp;';
        echo tablename_nice2('ABSTRACT_PROTO').': '. $html_tmp."<br><br>";
        
        if ($this->edit_flag) {
            echo "INFO: edit currently not supported.<br>";
        }
        
        if ($this->edit_flag) {
            
            $initarr   = array();
            $initarr["action"]      = $this->form_url;
            $hiddenarr = array();
            
            $hiddenarr["action"]  = 'update';
            $this->formobj = new formPure($initarr, $hiddenarr, 0);
            $this->formobj->start();
            
            $this->jsFormLib = new gJS_edit();
            $this->suc_run_id = 0;
        }
        
        
        $this->_tab_head($sqlo);
        
    }
    
    function show_one_pra($sqlo, &$prc_arr) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        $this->proto_arr  = &$prc_arr;
        $this->_init_protocols($sqlo);

        $sh_opt = $this->show_opt;

        // build CACHE
        $proto_c_cache=array();
        
        foreach($this->mo_obj_arr as $cnt => $exp_id_l) {

            $cpid_l   = $this->proto_arr[$cnt];
            
            if (empty($proto_c_cache[$cpid_l])) {
                $c_data = $this->one_proto($sqlo, $exp_id_l, $cpid_l, $cnt);
                $proto_c_cache[$cpid_l] = $c_data; 
            }
        }
        
        // glob_printr( $proto_c_cache, $FUNCNAME.":array proto_c_cache", 1 );
        
        
        if ($this->show_opt['many_PRA_flag']) {
            
            echo "<tr>\n";
            
            $pra_id = $this->infox['pra_id'];
            $pra_name = obj_nice_name ( $sqlo, 'ABSTRACT_PROTO', $pra_id );
            echo '<td style="background-color:#808080; color:#FFFFFF;" colspan='.$this->table_lead_cols.'>Protocol: '.$pra_name.'</td>';
            
            $data_row=array();
            foreach($this->mo_obj_arr as $exp_id_l) {
                $data_row[]='';
            }
            
            foreach($data_row as $i => $tmp) {
                echo '<td style="background-color:#808080; color:#FFFFFF;">'.$tmp.'</td>';
            }
            echo "</tr>\n";
        }
        
        
        foreach($this->pra_step_use as $row) {

            $step=$row['st'];
            $pra_step_info = $this->pra_step_all[$step];
            
            $legend_cells=array();
            if ($sh_opt['sh.step_nr'])
                $legend_cells[]=$step;
            
            $legend_cells[]=$pra_step_info['index'];
            $legend_cells[]=$row['mk'];
            $legend_cells[]=$pra_step_info['NAME'];
            $legend_cells[]=$pra_step_info['H_UNIT_NAME'];
            
            $data_col = $row['ty'];
            
            
            
            $val_diff_array=array();
            $all_colors = array();
            $data_row=array();
            $cnt=0;
            
            foreach($this->mo_obj_arr as $exp_id_l) {
                
               
                $cpid_l   = $this->proto_arr[$cnt];
                $c_data   = &$proto_c_cache[$cpid_l];
                $color_info=NULL;

                
                if ($data_col!=NULL) {
                    
                    $val_ori = $c_data[$step][$data_col];
                    
                    if ($val_ori===NULL or $val_ori==='') {
                    } else {
                        $color_info = self::_get_differ_index($val_diff_array, $val_ori);
                    }
                    $all_colors[$cnt] = $color_info;
                   
                    
                    $val = $val_ori;
                    if ($data_col=='CONCRETE_SUBST_ID' and $val_ori) {
                        if ($sh_opt['sh.suc.name']) {
                            $val = obj_nice_name ( $sqlo, 'CONCRETE_SUBST', $val_ori ). ' ['.$val_ori.']';
                        } else {
                            $val = $val_ori;
                        }
                    }
                } else {
                    $val=''; // empty
                }
                $data_row[]=$val;

                $cnt++;
            }
            
            echo "<tr>\n";
            
            foreach( $legend_cells as $tmp) {
                echo '<td>'.$tmp.'</td>';
            }
            
            foreach($data_row as $i => $tmp) {
                if(!empty($all_colors[$i])) {
                    echo '<td style="background-color:'.$all_colors[$i]['bg'].'; color:'.$all_colors[$i]['fg'].';"">'.$tmp.'</td>';
                } else {
                    echo '<td>'.$tmp.'</td>';
                }
            }
            echo "</tr>\n";
            
        }
        
        $this->pra_cnt++;


    }
    
    public function table_close() {
        echo "</table>\n";
        
        if ($this->edit_flag) {
            echo "<input class='yButton' type=submit value=\"Save\">\n";
            $this->formobj->close();
        }
    }
    
    //TBD:
    //     private function row_EDIT($sqlo) {
    
    
    //         $suc_name='';
    
    //         if($suc_id) {
    //             $suc_name = obj_nice_name ( $sqlo, 'CONCRETE_SUBST', $suc_id );
    //         }
    
    //         if ($this->edit_flag) {
    
    //             if ($has_MAC_col) {
    //                 $suc_select_html='';
    //                 if ($pra_sua or $suc_id) {
    //                     $table='CONCRETE_SUBST';
    //                     $suc_select_html = $this->jsFormLib->getAll( $table, 'cs['.$exp_id_l.']['.$p_step_nr.']', $suc_id, $suc_name, $this->suc_run_id );
    //                     $this->suc_run_id++;
    //                 }
    //                 echo '<td>'.$suc_select_html.'</td>';
    //             }
    //             echo '<td><input type="text" name="q['.$exp_id_l.']['.$p_step_nr.']" value="'.$quant.'" size=7></td>';
    //         } else {
    //             if ($has_MAC_col) {
    //                 echo '<td>'.$suc_name.'</td>';
    //             }
    //             echo '<td>'.$quant.'</td>';
    //         }
    
    //     }
}

 