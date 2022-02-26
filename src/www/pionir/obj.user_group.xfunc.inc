<?php
require_once('gui/glob.xfunc_abs.inc');

/**
 * show optional attributes/features of substance in project-navigator + list view
 * @package obj.user_group.xfunc.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/obj.user_group.xfunc.inc 59 2018-11-21 09:04:09Z $
 */
class cUSER_GROUP extends xfunc_ABS  {
var $ident="USER_GROUP";

function __construct( ) {
    parent::__construct();
	$this->xcolarr[]=array ("cnt(users)" => "User count", "rem" => "User count");
	return 1;
}


//  get extra information for list view
function xColDataGet (&$sqlo, $colname, $idarr )
{
	$output = "";

	if ( $colname === "cnt(users)" ) {
		
		$sqlsel = 'count(1) from 	DB_USER_IN_GROUP where USER_GROUP_ID='.$idarr[0];
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$output = $sqlo->RowData[0];
		
	}
	
	return ( $output );
}


}

