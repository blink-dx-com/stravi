<?php
require_once('gui/glob.xfunc_abs.inc');

/**
 * 
 * @author skube
 * @package obj.contact.xfunc.inc
 */
class cCONTACT extends xfunc_ABS {
var $ident="CONTACT";

function __construct( ) {
    parent::__construct();
	$this->xcolarr[]=array ("show_logo" => "show logo");
	return 1;
}

function xColDataGet ( &$sql, $colname, $idarr ) /* get extra information */
{
	require_once("f.data_dir.inc");
	$output="";
	
	if ( $colname == "show_logo" ) {
		$id = $idarr[0];
		$pfilename = datadirC::datadir_filename( "CONTACT", $id ,"jpg" );
		if ( file_exists($pfilename) ) {
			$output="<img src=\"glob.obj.img_show.php?tablename=CONTACT&primid=".$id."&extension=jpg\" height=50>";
		}
	}
	return ( $output );
}


}

