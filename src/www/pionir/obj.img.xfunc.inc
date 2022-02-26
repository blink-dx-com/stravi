<?
require_once('gui/glob.xfunc_abs.inc');

/** 
 * @package obj.img.xfunc.inc
 * @author  Steffen Kube
 */
class cIMG extends xfunc_ABS {
 var $ident="IMG";
 var $xcolarr;

function __construct( ) {
    parent::__construct();
	$this->xcolarr[]=array ("SIZE" => "size of image", "rem" =>"in bytes");
}

function xColDataGet ( &$sql, $colname, $idarr ) /* get extra information */
{
	

	if ( $colname == "SIZE" ) {
		$imgname = $_SESSION['globals']["img_path"]. "/Org_" . $idarr[0] . ".tif";
		if ( !file_exists($imgname) ) return;
		$output  = filesize($imgname);
	}
	return ( $output );
}


}

