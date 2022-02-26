<?php
require_once('gui/glob.xfunc_abs.inc');
require_once('gui/o.SATTACH.imgshow.inc');

/**
 * show optional attributes/features of substance in project-navigator + list view
 * @package obj.abstract_subst.xfunc.inc
 * @swreq   UREQ:0001395 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class cABSTRACT_SUBST extends xfunc_ABS {
var $ident="ABSTRACT_SUBST";

function __construct( ) {
    parent::__construct();
    $this->xcolarr[]=array ("Thumbnail"  => "Thumbnail", "rem" => "show thumbnail of object");
	return 1;
}



//  get extra information for list view
function xColDataGet (&$sqlo, $colname, $idarr )
{
	$output = "";

	if ( $colname == "Thumbnail" ) {
	    $attachLib = new oSATTACH_imgshow();
	    $relid = $attachLib->getThumbAttach( $sqlo, 'ABSTRACT_SUBST', $idarr[0]);
	    if ($relid) {
	        $htmlTmp = $attachLib->getImgTag($relid);
	        $output  = $htmlTmp;  
	    }
	}
	
	return ( $output );
}


}

