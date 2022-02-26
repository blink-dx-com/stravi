<?php
require_once('gui/glob.xfunc_abs.inc');

/**
 * show optional attributes/features of module list view
 * @package obj.module.xfunc.inc
 * @swreq   UREQ:0000959 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class cMODULE  extends xfunc_ABS {
var $ident="MODULE";

function __construct( ) {
    parent::__construct();
	$this->xcolarr[]=array ("objectTable" => "object-table", "rem" => "workflow for object");
	return 1;
}


//  get extra information for list view
function xColDataGet (&$sql, $colname, $idarr )
{
	$output = "";

	if ( $colname === "objectTable" ) {
		$sqlsel="VALUE from MOD_META where MXID=".$idarr[0]." and KEY='TABLE_NAME'" ;
		$sql->Quesel($sqlsel);
		$sql->ReadRow();
		$output = $sql->RowData[0];
	}
	
	
	return ( $output );
}


}

