<?
require_once('gui/glob.xfunc_abs.inc');


/**
 * @package obj.link.xfunc.inc
 * @author  Steffen Kube
 *
 */
class cLINK  extends xfunc_ABS {
 var $ident="LINK";
 var $xcolarr;

function __construct( ) {
    parent::__construct();
	$this->xcolarr[]=array ("SIZE" => "size of document", "rem" =>"in bytes");
}

function xColDataGet ( &$sql, $colname, $idarr ) /* get extra information */
{
	

	if ( $colname == "SIZE" ) {
		$filex = $_SESSION['globals']["data_path"] . "/o.LINK.".$idarr[0];
		if ( !file_exists($filex) ) return;
		$output  = filesize($filex);
	}
	return ( $output );
}


}

