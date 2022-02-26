<?php
/**
 * extension for MORDER
 * @package obj.morder.xedit.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header:  2018-11-21 09:04:09Z $
 */
require_once ("visufuncs.inc");
require_once ("f.objview.inc");	
require_once 'o.S_OBJLINK.subs.inc';
require_once 'o.CCT_ACCLOG.subs.inc';

require_once 'o.MORDER.network.inc';
require_once 'gui/o.MORDER.gui.inc';


class oMORDER_network_gui {
    
    function __construct($mord_id) {
        $this->mord_id=$mord_id;
        $this->help_lib = new oMORDER_network($mord_id);
        $this->sub_ords = NULL;
    }
    
    // get orders
    function sub_orders_exist($sqlo) {
        $this->sub_ords = $this->help_lib->get_orders($sqlo);
        $output=0;
        if (!empty($this->sub_ords)) $output=1;
        return $output;
    }
    
    /**
     * show all SUB orders
     * @param object $sqlo
     */
    function show_subs($sqlo) {
        
        if ($this->sub_ords===NULL) {
            $this->sub_ords = $this->help_lib->get_orders($sqlo);
        }
        
        
        if (empty($this->sub_ords)) return;
        
       
        $morder_gui_lib = new oMORDER_gui_table();
        $morder_gui_lib->init_progress_bar();
        
        $tabobj         = new visufuncs();
        
        $headOpt = array( "title" => "Sub Orders" );
        $headx   = array ("Name", "MAA", "Status Graph", "Status", "MACs");
        $tabobj->table_head($headx,   $headOpt);
        

        foreach( $this->sub_ords as $sub_ord_id) {
            
            // $htmltmp = $objLinkLib->bo_display($sqlo, 'MORDER', $sub_ord_id);
            
            $s_feats = $morder_gui_lib->get_features($sqlo, $sub_ord_id);
            
            $dataArr = array( 
                '+ '.$s_feats['MORDERHtml'],
                $s_feats['SUA_html'],
                $s_feats['order_status.graph'],
                $s_feats['order_status'],
                $s_feats['SUCHtml']
                );
            
            $tabobj->table_row ($dataArr);
        }
        
        $tabobj->table_close();
    }
}

class o_morder_xedit {
    
    function __construct($id) {
        $this->order_id = $id;
    }
    
    function showBatches(&$sqlo) {
        
        $id = $this->order_id;
        
        $this->accLogLib = new oAccLogC();
        
        $objlink_lib = new oS_OBJLINK_subs();
        $children = $objlink_lib->getChiObjByChiTab($sqlo, 'MORDER', $id, 'CONCRETE_SUBST');
     
        
        if ( empty($children)) {
            return;
        }
            
        // get edit-link of BO + NAME + icon (object)
        $this->objLinkLib = new fObjViewC();
        $tabobj  = new visufuncs();
        $headOpt = array( "title" => ' <img src="images/icon.CONCRETE_SUBST.gif"> Produced MACs' );
        $headx   = array ("pos", "MAC", 'Quantity', 'Status');
        $tabobj->table_head($headx,   $headOpt);
       
        $objLinkLib = new fObjViewC();
        
        $pos=1;
        foreach($children as $suc_id) {

            $html_batch = $objLinkLib->bo_display( $sqlo, 'CONCRETE_SUBST', $suc_id );
            $dataArr= array($pos, $html_batch, '', '');
            
           
            $tabobj->table_row ($dataArr);
            $pos++;
        }
        
        $tabobj->table_close();

    }
    
    function show_sub_orders($sqlo) {
        
        $gui_help = new oMORDER_network_gui($this->order_id);
        
        if ($gui_help->sub_orders_exist($sqlo) ) echo "<br>\n";
        
        $gui_help->show_subs($sqlo);
    }
    
}

class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;

        $o_rights = &$this->o_rights;
        $viewmode = $this->viewmode;
	
        $tablename='MORDER';
        $editAllow = $o_rights["write"];
        if ($viewmode!= "edit" ) $editAllow = 0;
        
    	echo "<ul>\n";
    	
    	if ($this->argu['DEVIATION']!=NULL) {
    	    echo "<ul>\n";
    	    cMsgbox::showBox("warning", "<b>Deviations found:</b> ". htmlspecialchars($this->argu['DEVIATION']));
    	    echo "</ul>\n";
    	}
    	
    	if ($editAllow) {
    	    
    	    $mord_features = glob_elemDataGet3( $sqlo, $tablename, array('MORDER_ID'=>$id), array('CCT_ACCESS_ID') );
    	    $CCT_ACCESS  = $mord_features['CCT_ACCESS_ID'];
    	    $accLogLib   = new oAccLogC();
    	    $accLogLib->setObject( $sqlo, $tablename, $id, $CCT_ACCESS );
    	    $status_name = $accLogLib->getLastLogName($sqlo);
    	    
    	    if ($status_name==NULL or $status_name=='started') {
    	    
        	    $nextstatus='beauftragt';
        	    $extratext .= '<a href="glob.obj.acclock.php?t='.$tablename.'&id='.$id.
        	    '&parx[statName]='.$nextstatus.'"><img src="images/arrow.but.gif"> Send Order</a> (beauftragen)';
    	    
        	    echo $extratext."<br><br>";
    	    }
    	}
    	
    	$helplib = new o_morder_xedit($id);
    	$helplib->showBatches($sqlo);
    	
    	$helplib->show_sub_orders($sqlo);
    	
    	echo "</ul>\n";
    }

}
