<?php 
require_once ("visufuncs.inc");
require_once ("f.objview.inc");	
require_once 'o.CONCRETE_SUBST.bom.inc';
require_once 'o.ABSTRACT_SUBST.bom.inc';

/**
 * show BOM of SUC
 * @package o.CONCRETE_SUBST.bom_gui.inc
 * @author Steffen
 *
 */
class oCONCRETE_SUBST_bom_gui {

    function __construct($sqlo, $suc_id) {
        $this->suc_id = $suc_id;
        $this->suc_features = glob_elemDataGet3( $sqlo, 'CONCRETE_SUBST', array('CONCRETE_SUBST_ID'=>$suc_id), array('ABSTRACT_SUBST_ID') );
    }
    
    function get_show_col_header() {
        $headx   = array ( 'POS','Component (planned)', 'Component (used)', 'Quantity', 'Expiry_date');
        return $headx;
    }
    
    
    /**
     * get concrete substances by POS
     * @param $sqlo
     * @param $sqlo2
     * 
     */
    function _getSubsts(&$sqlo) {
        
       $suc_id = $this->suc_id;
       $suc_bom_lib = new oCONCRETE_SUBST_bom($suc_id);
       $cSubstArr = $suc_bom_lib->get_compos_by_pos($sqlo);
       return $cSubstArr;
    }
    
    /**
     * 
     * @param object $sqlo
     * @return array of array(COMPO_SUA_ID, POS, QUANTITY)
     */
    function _get_abs_substs(&$sqlo) {
        
        $sua_id = $this->suc_features['ABSTRACT_SUBST_ID'];
        $sua_bom_lib = new oABSTRACT_SUBST_bom($sua_id);
        $sua_compos  = $sua_bom_lib->get_components($sqlo);
        return $sua_compos;
    }
    
    /**
     * 
     * @param object $sqlo
     * @return array
     *   'POS'
     *   'APOS'
     *   'SUC'
     *   'SUC.SUA'
     *   'SUA.plan'
     *   'QUANTITY'
     *   'EXPIRY_DATE'
     *   'H_UNIT.name' : for quantity
     */
    function get_BOM_info($sqlo) {
        $suc_id = $this->suc_id;
        
        $suc_bom_lib = new oCONCRETE_SUBST_bom($suc_id);
        
        $aSubstArr = $this->_get_abs_substs($sqlo);
        $cSubstArr = $this->_getSubsts($sqlo);
        
        $posarr = $suc_bom_lib->merge_conc_and_abs($cSubstArr, $aSubstArr);
        
        $out=array();
        foreach($posarr as $loop_row) {

            $sua_id_plan = $loop_row['a']['COMPO_SUA_ID'];
            $csubst      = $loop_row['c']['COMPO_SUC_ID'];
            $sua_meas_unit='';
            
            if ($sua_id_plan)
                if ($csubst) {
                    
                    $cSubstAttArr = glob_elemDataGet3( $sqlo, 'CONCRETE_SUBST', array('CONCRETE_SUBST_ID'=>$csubst), 
                        array('ABSTRACT_SUBST_ID', 'EXPIRY_DATE', 'QUANTITY') );
                    $sua_is = $cSubstAttArr['ABSTRACT_SUBST_ID'];
                    $sua_is_features = glob_elemDataGet3( $sqlo, 'ABSTRACT_SUBST', array('ABSTRACT_SUBST_ID'=>$csubst),
                        array('H_UNIT_ID') );
                    if ($sua_is_features['H_UNIT_ID']) {
                        $sua_meas_unit = obj_nice_name ( $sqlo, 'H_UNIT', $sua_is_features['H_UNIT_ID'] );
                    }
                    
                } else {
                    $cSubstAttArr=array();
                }
                
                $loop_out = array( 
                    'POS'=>  $loop_row['POS'],
                    'APOS'=> $loop_row['APOS'],
                    'QUANTITY'=>$loop_row['c']['QUANTITY'],
                    'EXPIRY_DATE'=>$cSubstAttArr['EXPIRY_DATE'] ,
                    'SUC'=>$csubst,
                    'SUC.SUA' =>$cSubstAttArr['ABSTRACT_SUBST_ID'],
                    'SUA.plan'=>$sua_id_plan,
                    'H_UNIT.name'=>$sua_meas_unit
                );
                $out[] = $loop_out;
        }
        return $out;
    }
    
    function show_BOM(&$sqlo) {
        $suc_id = $this->suc_id;
        
        $BOM_info = $this->get_BOM_info($sqlo);
        
        if ( empty($BOM_info) ) {
            echo '&nbsp;&nbsp;&nbsp;<span class=yGgray>No Components in the template and real material found.</span><br>'."\n";
            return;
        }
        
        $cond = 'x.CONCRETE_SUBST_ID in (select COMPO_SUC_ID from SUC_BOM where CONCRETE_SUBST_ID='.$suc_id.')';
        $condParm = '&condclean=1&tableSCond='.urlencode($cond);
        echo "<a href=\"view.tmpl.php?t=CONCRETE_SUBST&".$condParm."\" >".
            "<img src=\"images/but.list2.gif\" border=0> Components in list view</a><br>";
 
        
        $this->tabobj = new visufuncs();
        $this->objLinkLib = new fObjViewC();

        
        $headOpt = array( "title" => '<img src="images/icon.CONCRETE_SUBST.gif"> BOM: '.
            ' abstrakte/konkrete Materialien;' );
        $headx   = $this->get_show_col_header();
        
        $this->tabobj = new visufuncs();
        $this->tabobj->table_head($headx,   $headOpt);
        
        $objLinkLib = new fObjViewC();

        foreach($BOM_info as $loop_row) {

            $pos_show   = $loop_row['APOS']; // .' ('.$pos.')';
            $html_sua='';
            $html_suc = '';
            
            $csubst   = $loop_row['SUC'];
            if ($loop_row['SUA.plan']) {
                $html_sua = $objLinkLib->bo_display( $sqlo, 'ABSTRACT_SUBST', $loop_row['SUA.plan'] );
            }
            if ($csubst) {
                $html_suc = $objLinkLib->bo_display( $sqlo, 'CONCRETE_SUBST', $csubst);
            }

            $outarr = array( $pos_show, $html_sua, $html_suc, $loop_row['QUANTITY'],  $loop_row['EXPIRY_DATE'] );
            $this->tabobj->table_row ($outarr);
        }
        
        $this->tabobj->table_close();  
    }
    
    private function _one_form_row_RAW( $formobj, $sqlo, $a_row, $suc_row, $apos, $upos, $in_options=array() ) {

        $row_opt=array();
        if ($in_options['hidden_row']) {
            $row_opt=array(
                "rowid"=>'xformrow'.$this->ipos,
                'style'=>'DISPLAY: none'
            );
        }
        
        $formobj->row_open($row_opt);
        echo '<input type=hidden name="bomcompo['.$this->ipos.'][pos]" value="'.$upos.'">';
        echo '<input type=hidden name="bomcompo['.$this->ipos.'][apos]" value="'.$apos.'">';
        echo "<td>".$apos; //  ' ('.$upos.')</td>';

        
        $sua_link = fObjViewC::bo_display( $sqlo, 'ABSTRACT_SUBST', $a_row['sua'] );
        echo "<td>".$sua_link . '</td>';
        $fieldx = array (
            "title" => "Material",
            'namex' => TRUE,
            "name"  => "bomcompo[".$this->ipos."][suc]",
            "object"=> "dblink",
            "val"   => $suc_row['COMPO_SUC_ID'],
            "inits" => array( 'table'=>'CONCRETE_SUBST', 'getObjName'=>1, 'sqlo'=>&$sqlo, 'pos' =>$this->dblink_pos, 'projlink'=> 1),
            
        );
        echo '  <td>';
        
        $formobj->_valueField($fieldx);
        if ($in_options['showAddField']) {
            echo " <a href=\"javascript:xShowField('xformrow".($this->ipos+1)."')\" title=\"Add additional MAC\">".
                '<img src="res/img/plus-square.svg">'.
                "</a>";
        }
        echo '</td>'."\n";
        echo '  <td>';
       
        echo '<input type=text name="bomcompo['.$this->ipos.'][q]" value="'.$suc_row['QUANTITY'].'" size=7>';
        echo '</td>'."\n";
        $formobj->row_close();
        
        $this->ipos++;
        $this->dblink_pos++;

    }
    
  
    
    /**
     * edit form for BOM
     * @param object $sqlo
     */
    function form_BOM(&$sqlo) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        $suc_id = $this->suc_id;

        
        $aSubstArr = $this->_get_abs_substs($sqlo);
        $cSubstArr = $this->_getSubsts($sqlo);
        
        if (!sizeof($aSubstArr)) {
            echo '&nbsp;&nbsp;&nbsp;<span class=yGgray>No Components in the template found.</span><br>'."\n";
            return;
        }
        
        ?>
	<script>  
 
    function xShowField(id)  
    {  
         if (document.getElementById(id).style.display == 'none')  { 
              document.getElementById(id).style.display = '';  
         }  
    }  
    </script>  
    <?php
        
        debugOut('cSubstArr: '.print_r($cSubstArr,1), $FUNCNAME, 2);
        
        $initarr   = NULL;
        $initarr["action"]      = 'p.php?mod=DEF/o.CONCRETE_SUBST.bom';
        $initarr["title"]       = "Edit BOM";
        $initarr["dblink"]      = 1;
        $initarr["submittitle"] = "Submit";
        $initarr["tabwidth"]    = "900";
        $initarr["colspan"]     = 3;
        $initarr["colwidth"] = array('20%', '30%', '40%', '10%');
        
        $hiddenarr = NULL;
        $hiddenarr['id']  =  $suc_id;
        
        
        
        $formobj = new formc($initarr, $hiddenarr, 0);
        
        $this->dblink_pos=0;
        $this->ipos=0;
        
        $formobj->row_open();
        echo '<td>POS</td>';
        echo '<td>Component (planned)</td>';
        echo '<td>Component (used)</td>';
        echo '<td>Quantity</td>';
        // echo '<td>Quantity</td>';
        // echo '<td>Key</td>';
        
        $formobj->row_close();
        
        
        $posarr = array();
        $sua_compo_pos = array();
        
        // COMPO_SUA_ID, POS, QUANTITY
        foreach ($aSubstArr as $row) {
            $pos = $row['POS'];
            $posarr[]=$pos;
            
            $sua_compo_pos[$pos] = array('apos'=>$pos, 'sua'=>$row['COMPO_SUA_ID']);
        }
        
//         $suc_compo_pos = array();
//         foreach($cSubstArr as $loop_row) {
//             $pos    = $loop_row['POS'];
//             $csubst = $loop_row['COMPO_SUC_ID'];
//             $apos   = $loop_row['APOS'];
//             $suc_compo_pos[$pos] = array('suc'=>$csubst);

//         }

        foreach($sua_compo_pos as $apos=>$a_row) {
            // search SUC_ROW

            $loop_sucs=array();
            foreach($cSubstArr as $suc_row_tmp) {
                if ($apos==$suc_row_tmp['APOS']) {
                    $loop_sucs[]=$suc_row_tmp;
                }
            }

            if (empty($loop_sucs)) {
                $suc_row=array();
                $upos=-1;
                $this->_one_form_row_RAW( $formobj, $sqlo, $a_row, $suc_row, $apos, $upos);
                
            } else {
                $num_sucs = sizeof($loop_sucs);
                
                foreach($loop_sucs as $i=>$suc_row_loop) {
                    $ropt=array();
                    if ($num_sucs==($i+1)) {
                        $ropt=array('showAddField'=>1);
                    }
                    
                    $upos = $suc_row_loop['POS'];
                    $this->_one_form_row_RAW( $formobj, $sqlo, $a_row, $suc_row_loop, $apos, $upos, $ropt);
                    
                    if ($num_sucs==($i+1)) {
                        $suc_row_dummy=array();
                        $ropt=array('hidden_row'=>1);
                        $this->_one_form_row_RAW( $formobj, $sqlo, $a_row, $suc_row_dummy, $apos, -1, $ropt);
                    }
                }
            }
        }
        
        
        $close_opt=array();
        $showSubmit=TRUE;
        $formobj->close( $showSubmit,$close_opt );
        
        
    }
}