<?php

require_once('gui/o.SATTACH.imgshow.inc');
require_once('gui/glob.xfunc_abs.inc');
require_once 'o.CONTAINER.subs.inc';

/**
 * 
 * @author  Steffen Kube 
 * @package obj.concrete_subst.xfunc.inc
 *
 */
class cCONCRETE_SUBST extends xfunc_ABS {
var $ident="CONCRETE_SUBST";

function __construct( ) {
    
    parent::__construct();
    
    $this->xcolarr[]=array ("StorageQ"  => "Storage Quantity", "rem" => "Quantity in storage");
    $this->xcolarr[]=array ("StorageN"  => "Storage Names", "rem" => "Storage names");
	$this->xcolarr[]=array ("Thumbnail"  => "Thumbnail", "rem" => "show thumbnail of object");
	
}

/**
 * show following attributes in project
 * @param object $sql
 * @param int $prim_id
 * @param array $options
 * @return string
 */
function inProjShowAtt(&$sql, $prim_id, $options=NULL ) {

	$tmp_optinfo = $this->get_abs_object_name($sql, $prim_id);
	
    if ( $options["batch"]>0 ) { 
        $sqlsel = "a.name, c.BATCH from CONCRETE_SUBST c, ABSTRACT_SUBST a where
	        c.CONCRETE_SUBST_ID=" .$prim_id. " AND c.ABSTRACT_SUBST_ID=a.ABSTRACT_SUBST_ID";
        $sql->Quesel($sqlsel);
        $sql->ReadRow();
        
        $tmp_BATCH = $sql->RowData[1];
		$tmp_optinfo .= "&nbsp; <font color=gray>BA:</font>".$tmp_BATCH;     
    }
	
	return $tmp_optinfo;


}

function xColDataGet (&$sql, $colname, $idarr ) /* get extra information */
{
	$output = "";

	if ( $colname == "Thumbnail" ) {
	    $attachLib = new oSATTACH_imgshow();
	    $relid = $attachLib->getThumbAttach( $sql, 'CONCRETE_SUBST', $idarr[0]);
	    if ($relid) {
	        $htmlTmp = $attachLib->getImgTag($relid);
	        $output  = $htmlTmp;
	        
	    }
	}
	
	if ( $colname == "StorageQ" ) {
	    $output = oCONTAINER_many::get_one_SUC_QUANTITY( $sql, $idarr[0] );
	    if (!$output) {
	        $output  = '';  
	    }
	}
	if ( $colname == "StorageN" ) {
	    $store_ids = oCONTAINER_many::get_one_SUC_stores( $sql, $idarr[0] );
	    if (!empty($store_ids)) {
	        $komma='';
	        foreach($store_ids as $stor_id) {
	            $output  .= $komma. obj_nice_name ( $sql, 'CONTAINER', $stor_id );
	            $komma=', ';
	        }
	    }
	}
	
	return ( $output );
}

}
