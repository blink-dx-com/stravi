<?php
require_once ('subs/obj.concrete_subst.xmode.sub.inc');
require_once 'o.CONCRETE_SUBST.proto.inc';
require_once ("visufuncs.inc");

/**
 * - manage QC protocol steps
 * - it contains code, which will be included in edit.tmpl.php, when the rider is set to "MODE"
 * - included by: edit.tmpl.php
 * @package obj.concrete_subst.xmode.qcprot.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0001352: o.CONCRETE_SUBST > GUI: modify/Create QC-protocol 
 */
class oCONCRETE_SUBST_xmode_qcprot {
	
    function __construct( &$editLib ) {
    		$this->editLib = &$editLib;
    }	
    
    /**
     * QC-specials
     * @param $sqlo
     * @param $sqlo2
     * @param $id
     * @param $arguByKey
     * @param $x_rights
     * @param $extraobj_o
     * @param $tabArgu
     * 	 'PRC' => ID of CONCRETE_PROTO: if given, edit this protocol
     * @param $editAllow
     */
    function xmode_start( &$sqlo, &$sqlo2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {

        if ($arguByKey['DEVIATION']!=NULL) {
            echo "<ul>\n";
            cMsgbox::showBox("warning", "<b>Deviations found:</b> ". htmlspecialchars($arguByKey['DEVIATION']));
            echo "</ul>\n";
        }
        
    	$p_options=array();
    	$p_options['qcEditAllow'] = 0;
    	$p_options['userWants'] = $this->editLib->getEditMode();
    	if ($p_options['userWants']=='edit' and $x_rights['insert']) $p_options['qcEditAllow'] = 1;
    	$p_options['xtype_def']=2;
    	
    	$mainLib = new oSUC_xmode_prot_help();
    	$mainLib->show($sqlo, $sqlo2, $id, $arguByKey, $tabArgu, $p_options);
    	
    	if ( $_SESSION['userGlob']['o.CONCRETE_SUBST.sh.proto_sample_use'] ) {
    	    
    	    $sublib = new oCONCRETE_SUBST_proto_use($id);
    	    $prc_ids = $sublib->get_all_protos_where_sample($sqlo);
 
    	    if (!empty($prc_ids)) {
    	       
    	        echo "<ul>\n";
    	        $tabobj = new visufuncs();
    	        $headOpt = array( "title" => "Protocols, where this MAC is used as SAMPLE" );
    	        $headx  = array ("#", "PRC-ID", 'PRA');
    	        $tabobj->table_head($headx,   $headOpt);
    	        
    	        $i=0;
    	        foreach($prc_ids as $prc_id) {
    	            $pra_id = glob_elementDataGet( $sqlo, 'CONCRETE_PROTO', 'CONCRETE_PROTO_ID', $prc_id, 'ABSTRACT_PROTO_ID'); 
    	            $tmp_name = obj_nice_name($sqlo, 'ABSTRACT_PROTO', $pra_id);
    	            $dataArr=array(($i+1), fObjViewC::bo_IDLink('CONCRETE_PROTO', $prc_id), $tmp_name);
    	            $tabobj->table_row ($dataArr);
    	            $i++;
    	        }
    	        $tabobj->table_close();
    	        echo "</ul>\n";
    	    }
    	}
    	
    }

}