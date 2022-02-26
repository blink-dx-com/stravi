<?php
require_once ('o.PROTO.def.inc');

/**
 *
 * Manage one OBJ_STRUCT
 * @module o.PROTO.quant_sub_struct.inc
 */
class fQuant_sub_struct {
    
    private $can_edit; // 0,1
    private $docid;
    private $so_id; // index of OBJ_STRUCT, start with 1
    private $OBJ_STRUCT; // the SUB-structure
    
    /**
     * mo_apid
     * mo_stepnr
     * @param array $idarr
     */
    function __construct($sqlo, $so_id, $docid) {
        
        $this->docid = $docid;
        $this->so_id = $so_id;
        $this->quantLib = new oProtoQuantC();
        $this->quantLib->set_docid($sqlo, $docid);
        
        $this->can_edit = $this->quantLib->has_write_access($sqlo);
        
        $this->OBJ_STRUCT = $this->quantLib->get_sub_struct($so_id);
        $tablename = $this->OBJ_STRUCT['table'];
        
        $this->obj_sepcial_lib = new fQuant_obj_def_extra($tablename);
    }
    
    private function _show_OBJ_STRUCT($sqlo) {
        echo "<br>_show_OBJ_STRUCT: SO_ID:".$this->so_id." <pre>".print_r($this->OBJ_STRUCT,1)."</pre><br>";
    }
    private function get_row_of_COL($colname) {
        $col_flag_vals = &$this->OBJ_STRUCT['cols'];
        foreach($col_flag_vals as $ind => $colrow) {
            if ( $colrow['name']==$colname ) {
                $found=1;
                break;
            } 
        }
        if (!$found) return;
        return $colrow;
    }
    
    function change_maa_check($sqlo, &$parx) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $parx['abs_id'] = trim($parx['abs_id']);
        if (!$parx['abs_id']) {
            $error->set( $FUNCNAME, 1, 'Input parameter ABS-ID missing');
        }
        if (!is_numeric($parx['abs_id'])) {
            $error->set( $FUNCNAME, 2,  'Input must be a number.');
        }
        
        $tablename = $this->OBJ_STRUCT['table'];
        $abs_conLib = new gConcAbsC();
        $abs_table  = $abs_conLib->getTemplate( $tablename );
        
        if (!gObject_exists ($sqlo, $abs_table, $parx['abs_id'] ) ) {
            $error->set( $FUNCNAME, 3,  'Object '.$abs_table.':'.$parx['abs_id'].' not found.');
        }
    }
    
    /**
     * change the material of sub-struct
     * @param object $sqlo
     * @param int $sua_id
     */
    function change_maa($sqlo, $abs_id) {
        $key = $this->so_id;
        $this->quantLib->sub_struct_ch_obj($sqlo, $key, $abs_id);
    }
    
    function change_maa_form($sqlo) {
        
        $tablename = $this->OBJ_STRUCT['table'];
        $abs_conLib = new gConcAbsC();
        $abs_table  = $abs_conLib->getTemplate( $tablename );
        
        $initarr   = NULL;
        $initarr["action"]      = $_SERVER['PHP_SELF'];
        $initarr["title"]       = "Select a new template object";
        $initarr["submittitle"] = "Save";
        $initarr["tabwidth"]    = "AUTO";
        $initarr["tabnowrap"]   = 1;
        $initarr["dblink"]      = 1;
        $hiddenarr = NULL;
        $hiddenarr['action'] = 'sub_struct_cm';
        $hiddenarr["so_id"]   = $this->so_id;
        
        
        
        $formobj = new formc($initarr, $hiddenarr, 0);
        
        
        $fieldx = array (
            "title" => "ID of ".tablename_nice2($abs_table) ,
            "name"  => "abs_id",
            "object"=> "text",
            "val"   => '',
            "notes" => "set alternative object" );
        $formobj->fieldOut( $fieldx );
        $formobj->close( TRUE );
    }
    
    private function _get_cols($sqlo) {
        $tablename = $this->OBJ_STRUCT['table'];
        
        $col_arr  = array();
        $fea_cols = columns_get_pos( $tablename );
        foreach($fea_cols as $loop_col) {
            
            $col_feat = colFeaturesGet   ( $sqlo,  $tablename, $loop_col);
            if ($col_feat['VISIBLE']<1) {
                continue;
            }
            if ($loop_col=='CCT_ACCESS_ID' or $loop_col=='EXTRA_OBJ_ID') {
                continue;
            }
            
            $col_arr[$loop_col] = $col_feat;
        }
        
        return $col_arr;
    }
    
    private function _gui_proto_select($sqlo, $tablename, $apid_arr) {
        $table='CONCRETE_PROTO';
        
        if (!$this->can_edit) {
            return;
        }
        
        echo "Protocol-Selector: <br><br>";
        
        if (empty($apid_arr)) {
            echo "No protocol found.<br>";
            return;
        }
        
        $selarr=array();
        foreach($apid_arr as $loop_apid) {
            $sqls = "select NAME from ABSTRACT_PROTO where ABSTRACT_PROTO_ID=".$loop_apid;
            $sqlo->query($sqls);
            $sqlo->ReadRow();
            $aname = $sqlo->RowData[0];
            $selarr[$loop_apid]=$aname;
        }
        
        $fkt='??';
        
        switch ($tablename) {
            case 'EXP':
                $fkt='EXP_HAS_PROTO';
                break;
            case 'CONCRETE_SUBST':
                $fkt='CS_HAS_PR';
                break;
        }
        
        foreach($selarr as $loop_apid =>$aname ) {
            
            $found_so_id = $this->quantLib->sub_struct_sea( $table, $this->so_id, array('ty'=>'ass', 'col'=>'CONCRETE_PROTO_ID', 't'=>$fkt, 'abs_id'=>$loop_apid ) );
            
            echo '- '.$aname.' [ID:'.$loop_apid.'] ';
            
            if ($found_so_id) {
                $sub_action ='[<a href="'.$_SERVER['PHP_SELF'].'?action=sub_struct&so_id='.$found_so_id.'">Define Protocol</a>]';
            } else {
                $sub_action ='[<a href="'.$_SERVER['PHP_SELF'].'?action=sel_apid&go=1&so_id='.$this->so_id.
                '&parx[aprotoid]='.$loop_apid.'&parx[fkt]='.$fkt.'">Define new sub object</a>]';
            }
            
            echo $sub_action.'<br>';
            
        }
    }
    
    private function _show_PROTO_steps($sqlo, $sqlo2) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $x=array();
        
        $step_lib = new oAbsProtoStepDet($sqlo, $this->docid, $x, 'sub_struct');
        $step_lib->set_sub_struct($this->so_id, $this->OBJ_STRUCT);
        
        $apid = $this->OBJ_STRUCT['src']['abs_id'];
        
        if ($apid) {
            $tablename_ap	= "ABSTRACT_PROTO";
            $apname = obj_nice_name  ( $sqlo, $tablename_ap, $apid);
            echo "Selected Protocol (abstract): <a href=\"edit.tmpl.php?t=".$tablename_ap."&id=".$apid."\">".$apname."</a>";
            echo "<br><br>\n";
        } else {
            echo "ERROR: Need an APID.<br>";
            return;
        }
        
        
        $fo_opt=array(  );
        $step_lib->form2( $sqlo, $sqlo2, $fo_opt );
        
        
        $error->printAll();
        
        
    }
    
    private function _checkbox($varname, $val) {
        
        if ($this->can_edit) {
            if ($val) $checked='checked';
            else $checked='';
            $out = '<input type=checkbox name="'.$varname.'" value="1"  '.$checked.'>';
        } else {
            if ($val) $checked='x';
            else $checked='';
            $out = $checked;
        }
        return $out;
    }
    
    private function _select_box($varname, $sel_array, $val) {
        if ($this->can_edit) {
            $out = formc::selectFget($varname, $sel_array, $val);
        } else {
            if ($val) 
                $out = $val;
        }
        return $out;
    }
    
    /**
     *
     * @param string $foreign_table
     * @param string $column
     * @param array $options
     * @return string
     */
    private function _get_action_str($foreign_table, $column, $options=array() ) {
        
        if (!$this->can_edit) {
            return;
        }
        
        $sub_action='';
        if (!$foreign_table) return '';
        
        // $so_id exists ???
        $found_so_id = $this->quantLib->sub_struct_sea($foreign_table, $this->so_id, array('ty'=>'vals', 'col'=>$column ) );
        if ($found_so_id) {
            $sub_action ='[<a href="'.$_SERVER['PHP_SELF'].'?action=sub_struct&so_id='.$found_so_id.'">Define sub object</a>]';
        } else {
            
            $parx_in=array();
            $parx_in['mo_id'] = $this->so_id;
            $parx_in['ty']  = 'vals';
            $parx_in['t']   = $foreign_table;
            $parx_in['col'] = $column;
            
            
            if ($options['ty'])  $parx_in['ty'] = $options['ty'];
            if ($options['fkt']) $parx_in['fkt'] = $options['fkt'];
            if ($options['ty']=='objtrc') {
                // use this column
                $parx_in['col'] =  $options['fk_col'];
            }
            if ($options['col_nice']) $parx_in['col_nice'] = $options['col_nice'];
            
            $parx_arr = array();
            foreach($parx_in as $key=>$val) {
                $parx_arr[] = 'parx['.$key.']='.$val;
            }
            $parx_str=implode('&', $parx_arr);
            
            $sub_action ='[<a href="'.$_SERVER['PHP_SELF'].'?action=sub_struct_new&'. $parx_str .'">Define new sub object</a>]';
        }
        return $sub_action;
    }
    
    /**
     * 
     * @param object $formobj
     * @param array $row
     * @param array $r_options
     *  'bgcolor'
     */
    private function t_row($formobj, $row, $r_options=array() ) {
        $formobj->row_open();
        
        $first_col_opt='';
        if ($r_options['bgcolor']) $first_col_opt=' style="background-color:'.$r_options['bgcolor'].'"';
        echo '<td'.$first_col_opt.'>'.$row[0].'</td>';
        echo '<td>'.$row[1].'</td>';
        echo '<td>'.$row[2].'</td>';
        echo '<td>'.$row[3].'</td>';
        
        $formobj->row_close();
    }
    
    private function _one_column_out($loop_col, $col_feat, $formobj) {
        
        $col_flag_vals = &$this->OBJ_STRUCT['cols'];
        
        $sub_action = $this->_get_action_str($col_feat['CCT_TABLE_NAME'], $loop_col);
        
        $val = 0;
        $val1= 0;
        $val2= 0;
        foreach($col_flag_vals as $ind => $colrow) {
            if ( $colrow['name']==$loop_col and !$colrow['nice'] )  $val1=1;
            if ( $colrow['name']==$loop_col and  $colrow['nice'] )  $val2=1;
        }
        
        $loop_notes = ''; // TBD
        
        $fieldx = array (
            //'namex' => TRUE,
            //'name'  => 'x[cols]['.$loop_col.']',
            "title" => $col_feat['NICE_NAME'],
            //"object"=> "checkbox",
            //"notes" => $loop_notes .' '. $sub_action,
            //"val"   => $val,
        );
        
        $check2='';
        $check1 = $this->_checkbox('x[cols]['.$loop_col.']', $val1);
        if ($col_feat['CCT_TABLE_NAME']) {
            $check2 = 'Nice: '.$this->_checkbox('x[cols]['.$loop_col.'.nice]', $val2);
        }
        
        $tmp_arr=array(
            $fieldx["title"],
            $check1,
            $check2,
            $sub_action
        );
        $this->t_row($formobj, $tmp_arr);
 
    }
 
    private function _x_class_manage($formobj, &$xobj_classes) {
        global $varcol;
        $varcol = & Varcols::get();
        
        $tablename = $this->OBJ_STRUCT['table'];
        $class_id=0;
        $class_name='';
       
        
        $class_name_arr=array();
        foreach($xobj_classes as $cls_id_loop) {
            $cls_name = $varcol->class_id_to_name($cls_id_loop);
            $cls_nice = $varcol->class_id_to_nice_name($cls_id_loop);
            $class_name_arr[$cls_name] = $cls_nice;
        }
        
        
        $loop_col='cx.class_name';
        $cx_row = $this->get_row_of_COL($loop_col);
        if (!empty($cx_row)) {
            $class_name = $cx_row['CX_name'];
            if ($class_name) {
                $class_id   = $varcol->class_name_to_id($tablename, $class_name);
            }
        }
        
 
        $check1 = $this->_select_box('x[cols]['.$loop_col.']', $class_name_arr, $class_name);
        
        $tmp_arr=array(
            'CLASS',
            $check1,
            '',
            ''
        );
        
        $this->t_row($formobj, $tmp_arr);
        
        if ($class_id) {
            $attrib_names = $varcol->get_attrib_names($class_id);
            if (!empty($attrib_names) ) {
                
                $r_options=array('bgcolor'=>'#D0D000');
                foreach($attrib_names as $attr_id=>$attr_name) {
                    
                    $loop_col='ca.'.$attr_name;
                    $val1 = 0;
                    if (!empty($this->get_row_of_COL($loop_col)) ) $val1 = 1;
                    $check1 = $this->_checkbox('x[cols]['.$loop_col.']', $val1);
                    
                    $tmp_arr=array(
                        '&nbsp;&nbsp;'.$attr_name,
                        $check1,
                        '',
                        ''
                    );
                    $this->t_row($formobj, $tmp_arr, $r_options);
                }
            }
        }
    }
    
    /**
     * show Objects columns
     * @param object $sqlo
     * @param object $sqlo2
     */
    function show_struct($sqlo, $sqlo2) {
        
        global $error, $varcol;
        
        
        $src_data  = $this->OBJ_STRUCT['src'];
        $tablename = $this->OBJ_STRUCT['table'];
        
        
        $obj_html_tmp='';
        
        
        echo '<table cellpadding=5><tr valign=top><td>'."\n";
        
        echo 'Objects columns: <span style="font-weight:bold; font-size:1.2em;}">'.  tablename_nice2($tablename) . "</span><br>\n";
        if ($src_data['abs_id']) {
            $obj_html_tmp = fQuant_helper::get_abs_obj_html( $sqlo, $tablename,  $src_data['abs_id']);
            echo 'Template: '.$obj_html_tmp.' [<a href="'.$_SERVER['PHP_SELF'].'?action=sub_struct_cm&so_id='.$this->so_id.'">Change</a>]';
        } else {
            echo "<br>";
        }
        echo "<br>\n";
        
        
        $initarr   = NULL;
        $initarr["action"]      = $_SERVER['PHP_SELF'];
        $initarr["title"]       = "Objects columns";
        $initarr["submittitle"] = "Save";
        $initarr["tabwidth"]    = "600";
        $initarr["colwidth"]    = array(30,20,20,30);
        $initarr["tabnowrap"]   = 1;
        $initarr["colspan"]     = 4;
        $hiddenarr = NULL;
        $hiddenarr['action'] = 'sub_struct';
        $hiddenarr["so_id"]   = $this->so_id;
        
        
        $formobj = new formc($initarr, $hiddenarr, 0);
        
        if ($src_data['ty']=='objtrc2') {
            
            $formobj->row_open();
            if (!$src_data["depend_id"]) {
                $style_tmp=' style="background-color:yellow;"';
            } else $style_tmp='';
            
            echo '<td><b>ID of Protocol template ID</b></td>';
            if ($this->can_edit) {
                echo '<td colspan=2 ><input '.$style_tmp.' type="text" size=7 name="x[depend_id]" value="'.$src_data["depend_id"].'" required></td>';
            } else {
                echo '<td colspan=2 >'.$src_data["depend_id"].'</td>';
            }
            echo '<td>Needed for object tracking</td>';
            $formobj->row_close();
            
        }
        
        
        // columns
        $fea_cols      = $this->_get_cols($sqlo);
        $col_flag_vals = &$this->OBJ_STRUCT['cols'];
        
        foreach($fea_cols as $loop_col => $col_feat) {
            
            if ($col_feat['VISIBLE']<1) {
                continue;
            }
            if ($loop_col=='CCT_ACCESS_ID' or $loop_col=='EXTRA_OBJ_ID') {
                continue;
            }
            
            $this->_one_column_out($loop_col, $col_feat, $formobj);
        }
        
        $varcol = & Varcols::get();
        $xobj_classes = $varcol->all_classes_of_t($tablename);
        if (!empty($xobj_classes)) { 
            $this->_x_class_manage($formobj, $xobj_classes);
 
        }
        
        //
        // object-dependend
        //
        $extra_cols = $this->obj_sepcial_lib->get_columns();
        if ( !empty($extra_cols) ) {
            
            foreach($extra_cols as $row) {
                
                
                $loop_col   = $row['name'];
                $x_name     = 'x.'.$row['name'];
                $dest_table = '';
                if ($row['fkt']) {
                    $dest_table    = $row['t'];
                }
                
                $ac_options = $row;
                if ($row['dty']) {
                    // show destination action#
                    $options=array();
                    $options['ty'] = $row['dty'];
                    $sub_action = $this->_get_action_str($dest_table, $loop_col, $ac_options);
                }
                
                $check1='';
                $check2='';
                
                if ($row['sh_col']<0) {
                    // do not show checkboxes ...
                } else {
                    
                    $val1 =0;
                    $val2 =0;
                    foreach($col_flag_vals as $ind => $colrow) {
                        
                        if ( $colrow['name']==$x_name and !$colrow['nice'] )  $val1=1;
                        if ( $colrow['name']==$x_name and  $colrow['nice'] )  $val2=1;
                    }
                    $check1 = $this->_checkbox('x[cols]['.$x_name.']', $val1);
                    if ($row['fkt'] and $row['nice']>0) {
                        $check2 = 'Nice: '.$this->_checkbox('x[cols]['.$x_name.'.nice]', $val2);
                    }
                }
                
                if (array_key_exists('input_abs_id', $row) ) {
                    
                    $val3='';
                    $xi_name = 'x.'.$row['name'].'.input';
                    foreach($col_flag_vals as $ind => $colrow) {
                        if ( $colrow['name']==$xi_name  )  $val3=$colrow['input_abs_id'];
                    }
                    
                    
                    if ($this->can_edit) $sub_act2 = '<input type=text name="x[cols]['.$x_name.'.input]" value="'.$val3.'"  size=7> ';
                    else $sub_act2 = $val3;
                    $sub_action = 'PRA-ID: '.$sub_act2. ' ' . $sub_action;
                }
                
                $loop_notes = $row['notes'];
                
                $formobj->row_open();
                echo '<td>'.$row['col_nice'].'</td>';
                echo '<td>'.$check1.'</td>';
                echo '<td>'.$check2.'</td>';
                echo '<td>'.$sub_action.' '.$loop_notes.'</td>';
                $formobj->row_close();
            }
        }
        
        
        $submit_allow=FALSE;
        if ($this->can_edit) $submit_allow=TRUE;
        
        $formobj->close( $submit_allow );
        
        echo '</td><td>'."\n";
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
            $this->_show_OBJ_STRUCT($sqlo);
        }
        
        if ($tablename=='EXP') {
            
            $help_lib = new fQuant_sel_ap($sqlo,  $tablename, $this->so_id,  $this->docid);
            $apid_arr = $help_lib->get_abs_protos();
            $this->_gui_proto_select($sqlo, $tablename, $apid_arr);
        }
        
        if ($tablename=='CONCRETE_SUBST') {
            
            // $abs_id = $this->OBJ_STRUCT['src']['abs_id'];
            $help_lib = new fQuant_sel_ap($sqlo,  $tablename, $this->so_id,  $this->docid);
            $apid_arr = $help_lib->get_abs_protos(); // get_pra_by_abs($sqlo, $abs_id);
            $error->printAll();
            $this->_gui_proto_select($sqlo, $tablename, $apid_arr);
        }
        
        if ($tablename=='CONCRETE_PROTO') {
            $this->_show_PROTO_steps($sqlo, $sqlo2); 
            
//             $help_lib = new fQuant_sel_ap($sqlo,  $tablename, $this->so_id,  $this->docid);
//             $apid_arr = $help_lib->get_abs_protos(); // get_pra_by_abs($sqlo, $abs_id);
//             $error->printAll();
//             $this->_gui_proto_select($sqlo, $tablename, $apid_arr);
        }

        echo '</tr></table>'."\n";
        
        
    }
    
    /**
     * save output columns
     * @param object $sql
     * @param array $x
     */
    function save_output1($sqlo, $x ) {
        global $error, $varcol;
        $varcol = & Varcols::get();
        
        $tablename = $this->OBJ_STRUCT['table'];
        $cols_flag_in = $x['cols'];
        if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
            echo "(593) DEBUG:save_output1 cols_flag_in:".print_r($cols_flag_in,1)."<br>";
        }

        // columns
        $fea_cols = $this->_get_cols($sqlo);
        $cols=array();
        
        foreach($fea_cols as $loop_col => $col_feat) {
            
            if ($cols_flag_in[$loop_col]>0) {
                $cols[]= array('name'=>$loop_col);
            }
            if ($cols_flag_in[$loop_col.'.nice']>0) {
                $col_def = array('name'=>$loop_col, 'nice'=>1);
                $cols[]= $col_def;
            }
        }
        
        // EXTRA_OBJ data
        $col_key='cx.class_name';
        if ($cols_flag_in[$col_key]) {
            $class_name = $cols_flag_in[$col_key];
            $class_id   = $varcol->class_name_to_id($tablename, $class_name);
            if ($class_id) {
                $col_def = array(
                    'name'    =>$col_key,
                    'col_nice'=>'CLASS',
                    'CX_name' =>$class_name
                );
                $cols[]= $col_def;
            }
            
            $attrib_names = $varcol->get_attrib_names($class_id);
            if (!empty($attrib_names) ) {
                foreach($attrib_names as $attr_id=>$attr_name) {
                    
                    $loop_col='ca.'.$attr_name;
                    if ($cols_flag_in[$loop_col]) {
                        $col_def = array(
                            'name'    =>$loop_col,
                            'col_nice'=>$attr_name,
                        );
                        $cols[]= $col_def;
                    }
                    
                }
            }
        }
        
        $extra_cols = $this->obj_sepcial_lib->get_columns();
        if (!empty($extra_cols)) {
            
            foreach($extra_cols as $row) {
                
                $loop_col = 'x.'.$row['name'];
                
                if ( $cols_flag_in[$loop_col]>0 or $cols_flag_in[$loop_col.'.nice'] ) {
                    
                    $col_def = array('name'=>$loop_col);
                    if ($row['col_nice'])  $col_def['col_nice'] = $row['col_nice'];
                    if ($row['ty'])  $col_def['ty']  = $row['ty'];
                    if ($row['fkt']) $col_def['fkt'] = $row['fkt'];
                    if ($row['t'])   $col_def['t']   = $row['t'];
                    if ($row['pk1']) $col_def['pk1'] = $row['pk1'];
                    if ($row['pk2']) $col_def['pk2'] = $row['pk2'];
                    if ($row['pk2_val']) $col_def['pk2_val'] = $row['pk2_val'];
                    if ($row['fk_col']) $col_def['col'] =  $row['fk_col'];  // fk_col => col
                    $cols[] = $col_def;
                    
                }
                if ($cols_flag_in[$loop_col.'.nice']>0) {
                    $col_def['nice']=1;
                    $cols[] = $col_def;
                    
                }
                
                
            }
        }
        
        if ($x['depend_id']) {
            $this->OBJ_STRUCT['src']['depend_id'] = $x['depend_id'];
        }
        
        $this->OBJ_STRUCT['cols'] = $cols;
        if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
            $this->_show_OBJ_STRUCT($sqlo);
        }
        $this->quantLib->set_sub_struct( $this->so_id, $this->OBJ_STRUCT);
        $this->save_globset($sqlo);
    }
    
    function save_globset($sqlo) {
        $this->quantLib->save_globset($sqlo);
    }
}
