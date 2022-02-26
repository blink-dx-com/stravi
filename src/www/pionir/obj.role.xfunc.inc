<?php
require_once('gui/glob.xfunc_abs.inc');

/**
 * show optional attributes/features of substance in project-navigator + list view
 * @package obj.role.xfunc.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 *  @version $Header: trunk/src/www/pionir/obj.role.xfunc.inc 59 2018-11-21 09:04:09Z $
 */
class cROLE  extends xfunc_ABS {
var $ident="ROLE";

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
		
		$sqlsel = 'count(1) from USER_ROLES where ROLE_ID='.$idarr[0];
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$output = $sqlo->RowData[0];
		
	}
	
	return ( $output );
}


}

