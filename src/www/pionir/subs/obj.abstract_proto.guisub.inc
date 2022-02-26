<?php 
require_once('class.obj.search_usage.inc');
require_once ("f.objview.inc");

/**
 * extension for abstract protocol
 * @package obj.abstract_proto.guisub.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oABSTRACT_PROTO_guisub {
    
    function __construct($pra_id) {
        $this->objid=$pra_id;
    }
    
    function object_tracking($sqlo) {
        $useopt = array("showicon"=>1);
        
        $objSearch = new object_usage($sqlo, 'ABSTRACT_PROTO', $this->objid, 0, "", $useopt );
        //$objSearch->start($sqlo);
        $objSearch->getNumOneParent( $sqlo, 'AS_HAS_PR', 'ABSTRACT_PROTO_ID' );
        $used_tables = $objSearch->getUsedTabs();
        
        $num_used_by_sua=$used_tables['AS_HAS_PR'];
        if ($num_used_by_sua>0) {
            
            echo 'Used by: ';
            if ($num_used_by_sua>1) echo "<br>";
            $objLinkLib = new fObjViewC();
            
            $max_show=5;
            $suas=array();
            $sqlsel = "ABSTRACT_SUBST_ID from AS_HAS_PR where ABSTRACT_PROTO_ID=".$this->objid;
            $sqlo->Quesel($sqlsel);
            $i=0;
            while ( $sqlo->ReadRow() ) {
                
                if ($i>$max_show) {
                    break;
                }
                $sua_id = $sqlo->RowData[0];
                $suas[] = $sua_id;
                $i++;
            }
            
            foreach($suas as $sua_id) {
                $html_tmp = $objLinkLib->bo_display( $sqlo, 'ABSTRACT_SUBST', $sua_id );
                echo $html_tmp."<br>";
            }
            if ($i>$max_show) {
                echo "... <br>";
            }
            
            echo "<br>";
        }
        
        
    }
}