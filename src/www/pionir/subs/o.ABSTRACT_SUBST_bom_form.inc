<?php
require_once ("func_form.inc");

/**
 * edit BOM of ABSTRACT_SUBST
 * @package o.ABSTRACT_SUBST_bom_form.inc
 * @author Steffen
 * @param array $bomcompo
 *   POS => array('sua'=>, 'q'=>)
 */
class oABSTRACT_SUBST_bom_form {
    
    function __construct($sua_id) {
        $this->objid = $sua_id;
    }
    
    /**
     *
     * @param object $sqlo
     * @param array $sua_compo_pos (compo_form_STRUCT)
     *  $POS => array('sua'=>, 'q'=> );
     * @param int $editAllow
     */
    function form1(&$sqlo, $sua_compo_pos, $editAllow) {
        
        $num_compos = sizeof($sua_compo_pos);
        
        if (!$editAllow and !$num_compos) {
            echo 'Material has no BOM.<br>'."\n";
            return;
        }
        
        $initarr   = NULL;
        $initarr["action"]      = 'p.php?mod=DEF/o.ABSTRACT_SUBST.bom';
        $initarr["title"]       = "BOM form";
        $initarr["dblink"]      = 1;
        $initarr["submittitle"] = "Submit";
        $initarr["tabwidth"]    = "700";
        $initarr["colspan"]     = 4;
        $initarr["colwidth"] = array('10%', '40%', '40%', '10%');
        
        $hiddenarr = NULL;
        $hiddenarr['id']=$this->objid;
        
        // dynamic number of positions ...
        $max_edit_positions = max(10, $num_compos+5);    
        
        
        $formobj = new formc($initarr, $hiddenarr, 0);
        
        $dblink_pos=0;
        
        $formobj->row_open();
        echo '<td>POS</td>';
        echo '<td>Component</td>';
        echo '<td>Quantity</td>';
        echo '<td>Unit</td>';
        
        $formobj->row_close();
        
        $pos=10;
        $posarr = array();
        if ($editAllow) {
            // insert also empty positions
            for ($i=1; $i<=$max_edit_positions; $i++) {
                $posarr[]=$pos;
                $pos = $pos + 10;
            }
        }
        
        foreach($sua_compo_pos as $pos =>$features) {
            $posarr[] = $pos;
        }
        
        $posarr = array_unique($posarr);
        sort($posarr);
        
        $onlyview=1;
        if ($editAllow) $onlyview=0;
        
        foreach($posarr as $pos) {
            
            $formobj->row_open();
            echo "<td>".$pos . '</td>';
            
           
            
            $compo_info = $sua_compo_pos[$pos];
            $m_unit     = $compo_info['unit.name'];
            
            // get SUA
            
            
            $fieldx = array (
                "title" => "SUA",
                'namex' => TRUE,
                "name"  => "bomcompo[".$pos."][sua]",
                "object"=> "dblink",
                "val"   => $compo_info['sua'],
                "inits" => array( 'table'=>'ABSTRACT_SUBST', 'getObjName'=>1, 'sqlo'=>&$sqlo, 'pos' =>$dblink_pos, 'projlink'=> 1),
                "notes" => "Plastik adapter",
                "view"=>$onlyview
            );
            
            echo '  <td>';
            $formobj->_valueField($fieldx);
            $dblink_pos++;
            echo '</td>'."\n";
            
            
            
            $fieldx = array (
                "title" => "Quant",
                'namex' => TRUE,
                "name"  => "bomcompo[".$pos."][q]",
                "object"=> "text",
                "val"   => $compo_info['q'],
                "notes" => "",
                "fsize" => 6,
                "view"=>$onlyview
            );
            
            echo '  <td>';
            $formobj->_valueField($fieldx);
            echo '</td>'."\n";
            
            echo '  <td>';
            echo $m_unit;
            echo '</td>'."\n";
            
            
            $formobj->row_close();
        }
        
        
        $close_opt=array();
        if ($editAllow) {
            $showSubmit=TRUE;
        } else {
            $showSubmit = FALSE;
            $close_opt["noRow"]=1;
        }
        $formobj->close( $showSubmit,$close_opt );
    }
}