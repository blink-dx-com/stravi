<?php
require_once('gui/glob.xfunc_abs.inc');
require_once 'o.MORDER.subs.inc';
require_once ("f.objview.inc");

/**
 * show optional attributes/features of module list view
 * @package obj.morder.xfunc.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class cMORDER  extends xfunc_ABS {
var $ident="MODULE";

function __construct( ) {
    parent::__construct();
    
    $this->mord_lib = new oMORDER_subs();
    
	$this->xcolarr[]=array ("prod.SUC"   => "produced MACs", "rem" => "produced MACs");
	$this->xcolarr[]=array ("sub_orders" => "Sub-Orders", "rem" => "has Sub-Orders");
	return 1;
}


//  get extra information for list view
function xColDataGet (&$sqlo, $colname, $idarr ) {
	$output = "";

	$this->mord_lib->set_obj($idarr[0]);
	
	if ( $colname == "prod.SUC" ) {
	    
	    $sucs = $this->mord_lib->get_prod_SUCs($sqlo);
	    if (!empty($sucs)) {
	        $komma='';
	        $sopt=array("noIcon"  => 1);
	        foreach($sucs as $suc) {
	            $html_tmp = fObjViewC::bo_display( $sqlo, 'CONCRETE_SUBST', $suc, $sopt );
	            $output .= $komma . $html_tmp;
	            $komma=', ';
	        }
	    }
	}
	if ( $colname == "sub_orders" ) {
	    
	    $orders = $this->mord_lib->get_sub_orders($sqlo);
	    
	    if (!empty($orders)) {
	        $komma='';
	        $sopt=array("noIcon"  => 1);
	        foreach($orders as $mord) {
	            $html_tmp = fObjViewC::bo_display( $sqlo, 'MORDER', $mord, $sopt );
	            $output .= $komma . $html_tmp;
	            $komma=', ';
	        }
	    }
	}
	
	
	return ( $output );
}


}

