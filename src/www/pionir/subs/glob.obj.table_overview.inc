<?php 

require_once ("toolbar.inc");

/**
 * show details of aBO-tables
 * @package glob.obj.table_overview.inc
 * @swreq UREQ:xxxxxxxxxxxx
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @version $Header:  Exp $
 */
class glob_obj_table_overview {
    
    private function showDetOne( &$sql, $tablename ) {
        
        
        
        $info1	= ""; // "&nbsp;";
        $tolopt = NULL;
        
        $url='view.tmpl.php?t='.$tablename;
        // $table_go   = '<a href="='.$url.'"><img src="images/arrow.but.gif"></a>';
        echo '<tr class="xclickable-row" data-href="'.$url.'"><td>'."\n";
        
        $objHeadLib = new fObjToolbar($tablename, 0, "list");
        $objHeadLib->toolbar_show($sql, $info1, $tolopt );
        echo "<td width=80% valign=top>";
        $tabNotes = table_remark2($tablename);
        echo $tabNotes;
        
        echo "   </td>\n";
        
        $objHeadLib->toolbar_end();
        
        echo '</td></tr>'."\n";
    }
    
    function showDet( &$sqlo, &$sqlo2 ) {
        
        ?>
        <style>
        .xclickable-row:hover { filter:brightness(80%); }
        </style>
        <?php
        
        echo '<table>'."\n";
        
        $retVal = $sqlo2->Quesel("table_name, TABLE_TYPE FROM cct_table where TABLE_TYPE='BO' and IS_ACTIVE=1 ORDER BY nice_name");
        while ( $sqlo2->ReadRow() ) {
            $tablename = $sqlo2->RowData[0];
            $this->showDetOne( $sqlo, $tablename );
           
        }
        echo '</table>'."\n";
        
        ?>
        <script>
        jQuery(document).ready(function($) {
            $(".xclickable-row").click(function() {
                window.location = $(this).data("href");
            });
        });
        </script>
        <?php
    }
}