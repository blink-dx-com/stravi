<?php
require_once ("f.objview.inc");
require_once ("visufuncs.inc");
require_once dirname(__FILE__).'/../www/pionir/impexp/suc_ref/o.CONCRETE_SUBST.TTC.inc';
require_once dirname(__FILE__).'/../www/pionir/impexp/suc_ref/o.LINK.TTC.inc';
// require_once dirname(__FILE__).'/../www/pionir/impexp/suc_ref/o.CONCRETE_SUBST.TTC_gui.inc';

/**
 * @package o.CONCRETE_SUBST.TTC_home.inc
 * @swreq REQ:41 BEADS > REF-Beads > Target-Type CHECKER
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @created 2021-06-14
 * @param string $action
 *   'forward.SUCs' -- need parx['doc_id']  -- forward to used MACs
 *
 */
class o_CONCRETE_SUBST_TTC_home_XPL extends gPlugin {
	
	function register() {
		//$tablename	= 'CONCRETE_SUBST';
	    $title		= o_LINK_TTC::TOOL_NAME.' : the MAC-Standard-Configuration-Checker - Home';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title_sh']    = o_LINK_TTC::TOOL_NAME.' - home';
		$this->infoarr['title']    = $title;
		$this->infoarr['ureq.info'] = 'REQ:41 BEADS > REF-Beads > Target-Type CHECKER';
		$this->infoarr['form_type']= 'tool';
		$this->infoarr['design']   = 'slim';
		//$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['locrow']   = array(  // defines the tool location-path
		    array('home.php', 'home'), 
		); 


	}
	
	function show_some_docs($sqlo, $sqlo2) {
	    global $varcol;
	    $sql_where_arr = $varcol->sql_get_fromWhereAdd($this->link_class_id);
	    
	    $MAX_SHOW=30;
	    $sqlsel = "x.LINK_ID from LINK x ".$sql_where_arr['j'].' where '.$sql_where_arr['w'].' order by x.NAME';
	    $sqlo2->Quesel($sqlsel);

	   
	    $tabobj  = new visufuncs();
	    $headOpt = array( "title" => "List of ALL Config docs" );
	    $headx   = array ("#", "Edit", "Configuration - name", 'Target MAA', 'number of linked MACs');
	    
	    $tabobj->table_head($headx,   $headOpt);
        $cnt=0;
        while ( $sqlo2->ReadRow() ) {
	        
	        if ($cnt>$MAX_SHOW) {
	            $dataArr=array('', '... more ...', '');
	            $tabobj->table_row ($dataArr);
	            break;
	        }
	        
	        $tmpid = $sqlo2->RowData[0];
	        
	        $config_lib = new o_LINK_TTC($sqlo, $tmpid);
	        $sua_id = $config_lib->get_SUA();

	        $doc_link = fObjViewC::	bo_display( $sqlo, 'LINK', $tmpid);
	        $doc_name = obj_nice_name ( $sqlo, 'LINK', $tmpid); 
	        $doc_link = '<a href="p.php?mod=DEF/o.LINK.SUC_REF_config&id='.$tmpid.
	           '"><img src="res/img/settings.svg" height="18" title="Show/edit"></a>';
	        
	        $sel_sucs_sql = o_LINK_TTC::get_linked_SUC_SQL($sqlo, $tmpid);
	        $sql_complete='count(1) from CONCRETE_SUBST where CONCRETE_SUBST_ID in (select '.$sel_sucs_sql.')';
	        
	        $sqlo->Quesel($sql_complete);
	        $sqlo->ReadRow();
	        $suc_count = $sqlo->RowData[0];
	        
	        $sua_link  = fObjViewC::bo_display( $sqlo, 'ABSTRACT_SUBST', $sua_id);
	        
	        
	        $dataArr=array($cnt+1, $doc_link, $doc_name, $sua_link, $suc_count);
	        $tabobj->table_row ($dataArr);
	        $cnt++;
	    }
	    
	    $tabobj->table_close();
	}

	function startMain() {
	    global $error, $varcol;
		$FUNCNAME='startMain';
		$tablename	= 'CONCRETE_SUBST';
		
		?>
        <div class="yHeaderWithIMG" style="background-image: url(images/layerimg.tschimtarga.jpg);">Tschimtarga - 5489m</div>
        <ul>
        
        <a href="p.php?mod=DEF/o.LINK.SUC_REF_config&action=doc_create"><img src="images/ic.plus.png"> create new configuration</a><br>
        
        <?php
		
		
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();

		//$li_options = $_REQUEST['parx']; // o_SUC_bead_ana3
		$go = $_REQUEST['go']; 
		
		$this->link_class_id = $varcol->class_name_to_id( 'LINK', o_LINK_TTC::DOC_CLASS_REF );
		$this->show_some_docs($sqlo, $sqlo2);
		
		gHtmlMisc::func_hist($this->_mod, $this->infoarr['title_sh'], $this->_plug_get_URL());
		//
		//echo $obj_link;
		echo "</ul>";
	}
}
