<?php
require_once ('glob.obj.conabs.inc');
require_once ('o.CCT_ACCLOG.subs.inc');
require_once ('o.PROJ.subs.inc');
require_once ('o.CCT_ACCLOG.subs.inc');
require_once ("f.objview.inc");
require_once ("visufuncs.inc");

/**
 * WIDGET: show list of objects
 * @package o.CONCRETE_SUBST.list1.inc
  * @swreq 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oCONCRETE_SUBST_list1 {
    
    private $srcProjID;
	
    
    private function _getLastSubst(&$sqlo,  $substid) {
        
        $suc_info=array();
        
        $sqlsel = 'max(x.CONCRETE_SUBST_ID) from CONCRETE_SUBST x '.
            ' where ABSTRACT_SUBST_ID='.$substid;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $CONCRETE_SUBST_ID = $sqlo->RowData[0];
        
        if ($CONCRETE_SUBST_ID) {
            $suc_info['suc_id'] = $CONCRETE_SUBST_ID;
            $acc_data = access_data_get  ( $sqlo, 'CONCRETE_SUBST', 'CONCRETE_SUBST_ID', $CONCRETE_SUBST_ID);
            $suc_info = array_merge($suc_info,$acc_data);
        }
        
        
        return $suc_info;
    }
    
    private function _get_suc_num(&$sqlo, $sua_id) {
        $sqlsel = 'count(x.CONCRETE_SUBST_ID) from CONCRETE_SUBST x where ABSTRACT_SUBST_ID='.$sua_id;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $cnt = $sqlo->RowData[0];
        return $cnt;
    }
    
    private function _getProjLink($sqlo, $projid) {
        $projstr = '(in folder '. $this->objLinkLib->bo_display( $sqlo, 'PROJ', $projid ) .')';
        return $projstr;
    }
    
    
    /**
     * show substance table; selected abstract substances in PROJECT $srcProjID
     * @param int $srcProjID source project ID
     * @param $title
     * @param array $li_options
     *   'SUC.crea.proj' : ID of destination project for batch creation
     *   'order.PROD_LINE_ID' : ID of CHIP_READER for order ...
     */
    function showTable(&$sqlo, $srcProjID, $title, $li_options) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->srcProjID = $srcProjID;
        
        $accLogLib = new oAccLogC();
        $this->releaseID = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, 'CONCRETE_SUBST', "released" );
        
        $this->objLinkLib = new fObjViewC();
        
        $destProjID = 0;
        if ($li_options['SUC.crea.proj']) {
            $destProjID = $li_options['SUC.crea.proj'];
        }
        
        $projSubLib = new cProjSubs();
        
        $sqlsAfter  = $projSubLib->getTableSQLInSort ( $this->srcProjID, 'ABSTRACT_SUBST' );
        
        $sqlsel = 'ABSTRACT_SUBST_ID from '.$sqlsAfter;
        $sqlo->Quesel($sqlsel);
        $substarr=NULL;
        while ($sqlo->ReadRow()) {
            $retid = $sqlo->RowData[0];
            $substarr[]=$retid;
        }
        
        if ($substarr==NULL) {
            echo 'No MAAs found in folder '.$this->srcProjID.'<br>';
            return;
        }
        
        $tabobj = new visufuncs();
        $headOpt = array( "title" => '<img src="images/icon.CONCRETE_SUBST.gif"> '.$title.' '.
            $this->_getProjLink($sqlo, $this->srcProjID) );
        
        $headx  = array (
            'Liste MAR', 
            "MAA", 
            "Neue Bestellung", 
            "Batch<br>anlegen",  
            "Letzte angelegte MAR", 
            "Creation Date"
        );
        $tabobj->table_head($headx,   $headOpt);
        
        $crea_url='glob.obj.crea_wiz.php?proj_id='.$destProjID.'&tablename=CONCRETE_SUBST&go=5';
        $morder_url='p.php?mod=LAB/o.MORDER.suc&parx[PROD_LINE_ID]='.$li_options['order.PROD_LINE_ID'].'&parx[ABSTRACT_SUBST_ID]=';
        
        foreach( $substarr as $dummy=>$sua_id) {
            
            $last_SUC_info = $this->_getLastSubst($sqlo,  $sua_id);
           
            $last_SUC_html='';
            $SUC_crea_date_html='';
            if ($last_SUC_info['suc_id'] ) {
                $last_SUC_html = $this->objLinkLib->bo_display( $sqlo, 'CONCRETE_SUBST', $last_SUC_info['suc_id'] );
                $SUC_crea_date_html = substr($last_SUC_info['crea_date'],0,10);
            }
            
            $suc_num = $this->_get_suc_num($sqlo, $sua_id);
            $listHtml= '<a href="view.tmpl.php?t=CONCRETE_SUBST&condclean=1&tableSCond='.
                urlencode('x.ABSTRACT_SUBST_ID='.$sua_id). 
                '"><img src="images/but.list2.gif" border=0> '.$suc_num .
                '</a>';
            
                $dataArr = array(
                    $listHtml,
                    $this->objLinkLib->bo_display( $sqlo, 'ABSTRACT_SUBST', $sua_id ),
                    '[<a href="' . $morder_url . $sua_id . '">Neue Bestellung</a>]', 
                    '[<a href="' . $crea_url  .'&obj_tmpl_id=' . $sua_id . '">Neue Batch</a>]',
                    $last_SUC_html,
                    $SUC_crea_date_html
                    
                );
                $tabobj->table_row ($dataArr);
        }
        reset ($substarr);
        $tabobj->table_close();
        echo "<br />\n";
        
    }
    
  
}