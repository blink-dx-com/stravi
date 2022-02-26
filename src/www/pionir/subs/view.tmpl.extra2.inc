<?php
/**
 * extra GUI methods for view.tmpl.php and obj.proj.selector.php
 * @package view.tmpl
 * @module  view.tmpl.extra2.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class view_tmpl_extra2 {
    
    function __construct ( viewSubC $viSubObj ) {
        $this->viSubObj = &$viSubObj;
    }
    
    
    /**
     * - show objects from history  $_SESSION['s_history']
     * @global $this->cctgoba
     */
    function history_select( &$sqlo, $tablename ) {
        
        
        $cnt=0;
        if ( !sizeof($_SESSION['s_history'])) {
            return;
        }
        
        $colsel = importantNameGet2($tablename);
        if ( $this->viSubObj->access_id_has ) $colsel .= ', CCT_ACCESS_ID';
        $sqlsel = $colsel.' from '.$tablename.' where '.$this->viSubObj->pk_name.'=';
        
        $tableIntro =
        "<table cellpadding=1 cellspacing=1 border=0 >\n" .
        '<tr bgcolor=#DDDDDD><td>' .
        '&nbsp;<font color=gray><b>... from the object history:</b></font>'."\n" .
        '<tr  bgcolor=#EFEFEF><td>';
        
        $rowOut = 0;
        
        foreach( $_SESSION['s_history'] as $th1 ) {
            $tab_key=key($th1);
            $tmp_id =current($th1);
            
            if ( $tab_key==$tablename ) {
                $sqlo->Quesel( $sqlsel.$sqlo->addQuotes($tmp_id) );
                $sqlo->ReadRow();
                $tmp_nameo = $sqlo->RowData[0];
                $acc_id    = $sqlo->RowData[1];
                
                if (!$rowOut) echo $tableIntro;
                $this->viSubObj->_selShowOne( $sqlo, $tmp_nameo, $tmp_id, $acc_id);
                $rowOut++;
                echo " <B>".$tmp_nameo."</B> [ID:".$tmp_id."]<br>";
            }
            $cnt++;
        }
        reset ( $_SESSION['s_history'] );
        if ($rowOut) echo "</td></tr></table><br />\n";
        
    }
}