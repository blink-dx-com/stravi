<?
/**
 * special functions for list view of object type
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @package obj.a_chip_reader.xfunc.inc 
 */
require_once('gui/glob.xfunc_abs.inc');
require_once('gui/o.SATTACH.imgshow.inc');

class cA_CHIP_READER extends xfunc_ABS  {
  var $ident="A_CHIP_READER";
  var $xcolarr;

  function __construct( ) {
      parent::__construct();
      
  
  	$this->xcolarr[]=array ("Thumbnail"  => "Thumbnail", "rem" => "show thumbnail of object");

  }
  
  

  
  function xColDataGet ( &$sqlo,$colname,$idarr ) {
  	global $error;
  	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
  	
  	$output = NULL;

  	
  	if ( $colname == "Thumbnail" ) {
  	    $attachLib = new oSATTACH_imgshow();
  	    $relid = $attachLib->getThumbAttach( $sqlo, 'A_CHIP_READER', $idarr[0]);
  	    if ($relid) {
  	        $htmlTmp = $attachLib->getImgTag($relid);
  	        $output  = $htmlTmp;
  	    }
  	}

	return ( $output );
  
  }
}
