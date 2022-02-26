<?php
require_once ("f.grBarDia.inc");

/**
 * build a proto quant image based on class fgrBarDiaC
 * @author Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @package o.PROTO.quant.img.inc
 * @swreq UREQ:0001578: o.PROTO > ProtoQuant : Analyse protocol steps
 * @version $Header: trunk/src/phplib/o.PROTO.quant.img.inc 59 2018-11-21 09:04:09Z $
 */
class oProtoQuant_imgC {
	
	private $imgErr;
	private $imgQuant; // array of the data
	
	function __construct() {
		$this->imgErr = NULL;
		$this->imgQuant=array();
		$this->imgObj  =array();
		
		$this->imgx = array();
		$this->imgx["quantDetMax"] = 15;  // max number of step details
		$this->imgx["objMaxNum"]   = 100; // max number of objects
	}
	
	function add_datarow($dataarr, $nicename) {
		
		if ( sizeof($this->imgObj) > $this->imgx["objMaxNum"] ) {
			$this->imgErr = "Too many objects (max ".$this->imgx["objMaxNum"].")";
			return;
		}
		
		$imgQuant_in = array_slice($dataarr, 1);  // get the quantities, ignore first column ...
		
		
		$this->imgQuant[] = $imgQuant_in;
		$this->imgObj[]   = array("0"=> $nicename ); // get the name
	}
	
	/**
	 * 
	 *  
	 * @param array $axis_info
	 *  POS => array(
	 *     "NAME"=>
	 *     "TYPE"=>
	 * @param string $title
	 */
	function img_init($axis_info, $title) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->title = $title;
		if ( $_SESSION["userGlob"]["g.debugLevel"]>1) {
    		debugOut('axis_info:'.print_r($axis_info,1), $FUNCNAME, 1);
    		debugOut('imgObj:'.print_r($this->imgObj ,1), $FUNCNAME, 1);
    		debugOut('imgQuant:'.print_r($this->imgQuant ,1), $FUNCNAME, 1);
		}
		// get quantitiy-infos
		
		
		do {
			if ( $this->imgErr!="" ) {
				$error->set("_imgShow", 1, $this->imgErr );
				break;
			}
		
			if ( sizeof($this->imgQuant) > $this->imgx["quantDetMax"] ) {
				$error->set("_imgShow", 2, "Too many quantity details for graphics. (max ".$this->imgx["quantDetMax"].")" );
				break;
			}
		
			$quant_inf=array();
			foreach( $axis_info as $valarr) {
				$quant_inf[] = array("name"=> $valarr["NAME"], "ty"=>$valarr["TYPE"] );
			}
			
		
			
			$subtitle = "X";
		
			$grOpt  = NULL;
			$titles = array("title"=>$title, "subtitle"=>$subtitle, "x"=>"objects", "y"=>"quant-value");
			$infoxz = NULL;
			$optarr = array( "debug"=>$_SESSION["userGlob"]["g.debugLevel"] );
			$this->diaObj = new fgrBarDiaC();
			$this->diaObj->init (
					$quant_inf,
					$this->imgObj,
					$this->imgQuant,
					$infoxz, 	// data min/max info
					$titles, 	// array ( "main", "x", "y" )
					$optarr
			);
		
		} while (0);
		
		
		
	}
	function show_img() {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    if ($error->Got(READONLY))  {
	        $errLast   = $error->getLast();
	        $error_txt = $errLast->text;
	        
	        debugOut('ERROR: '.$error_txt, $FUNCNAME, 1);
	        
	        $this->diaObj = new fgrBarDiaC();
	        $this->diaObj->errorImage ("diagram-builder: ".$error_txt);
	        return;
	    }
	    
		$this->diaObj->drawAll();
	}
	
	function error_image($err_text) {
	    $this->diaObj = new fgrBarDiaC();
	    $this->diaObj->errorImage($err_text);
	}
}
