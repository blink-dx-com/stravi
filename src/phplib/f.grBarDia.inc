<?php
/*MODULE:  f.grBarDia.inc
  DESCR:   GD-lib image features for Bargraphs
  AUTHOR:  qbi
  VERSION: 0.1 - 20071017
  Keywords: ObjectArea - segment of ONE object: contains x bars
			QuantiBar  - one bar

*/

class fgrBarDiaC {

var $CLASSNAME="fgrBarDiaC";
var $GR;
var $GRCO;
var $COLN; // current color
var $imgHandle;
var $xColors;   // variable for extra-colors
var $quant_inf; /* description of quantities : array[QUNAT_ID] = array(
						"name"=>Name, 
						"ty"=>s,q    -- s : string, [q] : number
						)
*/
var $obj_inf;   // object-array :    array[OBJ_POS] = array( 0=> name, 1=> other param, ... 
var $quantVals; // quantity-values : array[OBJ_POS] = array(quanti-pos => value)
var $quantMiMa; // start-end-quantity-values : array[OBJ_POS] = array( array(start,end), ...)
				// can only be used for quanti-pos = 0
var $infoxz;	// "obj_max_len", "value_min", "value_max"
		
/**
 * 
 * @param array $quant_inf  description of quantities 
 * @param array $obj_inf
 * @param array $quantVals
 * @param array $infoxz  data min/max info
 * @param array $titles  array ( "title", "x", "y" )
 * @param array $optarr
 *   "scmax", "scmin", 
	"sub_col"   => LOCAL_BACKGRD, 1-BACK, 
	"fontalias" => 3 - very small, show one PIXEL per probe, show every 3rd probe-name
				   1 - small, 
				   [2] - big
	"shProbMax" => "", number
	"debug"
	"noLegend"  => 1 : no legend
	"barColor"  => "mono" : monochrome
 */
function  init ( 
	&$quant_inf,
    &$obj_inf,
	&$quantVals,
	&$infoxz, 	
	&$titles, 	
	$optarr      
	) {
	global $error;
	$FUNCNAME = $this->CLASSNAME.":init";
					 
	$this->quant_inf = &$quant_inf;
	$this->obj_inf   = &$obj_inf;
	$this->quantVals = &$quantVals;
	$this->optarr    = &$optarr;
	
	if ( $_SESSION["userGlob"]["g.debugLevel"]>1) {
	    debugOut('quant_inf:'.print_r($quant_inf,1), $FUNCNAME, 1);
	    debugOut('obj_inf:'.print_r($this->obj_inf ,1), $FUNCNAME, 1);

	}
	
	$this->quantCnt  = sizeof($this->quant_inf);
	$this->infoxz    = NULL;		
	if ( $optarr["scmax"] == "" OR $optarr["scmin"] == "") {
		$this->_getQuantExtreme();
		
	}

	$obj_cnt  = sizeof($this->obj_inf);
	if (!$obj_cnt) {
		$error->set( $FUNCNAME, 1, "No object-info given." );
		return;
	}
	$this->_getObjExtreme();
	
	$this->tmp_debug_lev = $optarr["debug"];
	$this->errorIntern   = NULL;
	$this->quantMaxNum   = 30;  // max number of quantities
			
	if ( $this->tmp_debug_lev<=1 ) {
    	Header("Content-type: image/png");
	} else {
		echo "DEBUG active!<br>\n";
	}
	
    // image data
    // <- "BAR_WIDTH" -><- "X_UnitWidht" ->
	// "X_UnitWidht" - distance between substances
	$init_QuantStrings = $this->_analyseQuant_inf();
	$GR_originY		   = 280;
    $GR_X_title_off    = 17;
    $GR_X_UnitText_yoff_ini = 4 + $this->infoxz["obj_max_len"] * 7;
    if ( $GR_X_UnitText_yoff_ini>240 ) $GR_X_UnitText_yoff_ini=240;
    
    $GR_Val_max = sprintf ("%.1f", $this->infoxz["value_max"]);
	if ($GR_Val_max<5) $GR_Val_max = $GR_Val_max + 0.1;
    if (  $optarr["scmax"] != "" ) $GR_Val_max = $optarr["scmax"];
    if ( ($optarr["scmax"] == "" ) AND ($GR_Val_max == 0.0) ) $GR_Val_max = 1.0;  // fallback
    
	$GR_Val_min = floor($this->infoxz["value_min"] * 10) * 0.1;
    if ( $optarr["scmin"] != "" ) $GR_Val_min = $optarr["scmin"];
    // fallback mechanism: ensure, that max > min !!!
    if ( $GR_Val_max <= $GR_Val_min)  $GR_Val_max =  $GR_Val_min + 1.0;  // this denies also division by ZERO in line: LINE_GR["yfact"]
	if ( $optarr["originY"]>0 ) $GR_originY = $optarr["originY"];
	
	$this->GR = array(); 		 // graphic options
	
	$this->GR["Y_axlog10"]  = floor( log10( abs($GR_Val_max)) ); // log10 : start with 0
	$this->GR["FONT_Alias"] = 2; // 2 - normal, 1- small
	$this->GR["FONT_SIZE"]  = 2; // 2
	$this->GR["X_UnitWidht"]= 11; 	// Pixel width of one Unit
	$this->GR["X_right_border"] = 20;
	
	if ( $optarr["fontalias"]>0 ) $this->GR["FONT_Alias"] = $optarr["fontalias"];
	
	$init_BAR_WIDTH   = 5;
	if ( $this->GR["FONT_Alias"]==1 ) { // small font
		$init_BAR_WIDTH = 3;
		$this->GR["FONT_SIZE"]  = 1;
	}
	if ( $this->GR["FONT_Alias"]==3 ) { // extrem small font
		$init_BAR_WIDTH = 1;
		$this->GR["FONT_SIZE"]  = 1;
	}
    if ( $init_QuantStrings ) $init_BAR_WIDTH   = 9;
	
    $this->GR["BAR_WIDTH"]  = $init_BAR_WIDTH;  // bar graph width
	$this->GR["X_UnitWidht"]= ($init_BAR_WIDTH)*$this->quantCnt+2;
	if ($this->GR["X_UnitWidht"]<9)  $this->GR["X_UnitWidht"]=9;
	
   
	if ( $this->quantCnt > 1 AND !$optarr["noLegend"] ) { // more than one quantity ?
		$this->GR["X_right_border"] = 200; // extra legend-text for experiment names
		$this->GR["legend_yoffs"]   = 40;
	}
    $this->GR["X_UnitLimit"] = $obj_cnt; // max number of X index units
                                               // $distinct_subst contains substances with GROUP 
    $this->GR["X_UnitText_yoff"]   = $GR_X_UnitText_yoff_ini;    // offset for unit text
	
    $extra_unit    = 0;
	$UnitLimitTemp = 256;
	if ($optarr["shProbMax"]>5) { // take from options
		$UnitLimitTemp = $optarr["shProbMax"];
	}
	
    if ( $this->GR["X_UnitLimit"] > $UnitLimitTemp ) {
        $this->GR["X_UnitLimit"] = $UnitLimitTemp;
        $extra_unit = 1; // space unit for extra info 
    } 
    $x_diff = ($this->GR["X_UnitLimit"]+$extra_unit) * $this->GR["X_UnitWidht"];
    $this->GR["originX"]    = 50 + $this->GR["Y_axlog10"]*7; // origin of coordinate system in x-direction
    $this->GR["img_x_size"] = $x_diff+$this->GR["originX"] + $this->GR["X_right_border"];   // size of image x-direction
    if ( $this->GR["img_x_size"]<300 ) $this->GR["img_x_size"]=300;
    $this->GR["X_max"]= $x_diff+$this->GR["originX"];
    
    $this->GR["originY"]    = $GR_originY;  // origin of coordinate system in y-direction (origin in upper left in gd-image!)
    $this->GR["img_y_size"] = $this->GR["originY"] + $this->GR["X_UnitText_yoff"] + $GR_X_title_off;  // size of image y-direction                  
      
    $this->GR["header_ydiff"]= 40;
    $this->GR["Y_max"]       = $this->GR["header_ydiff"];          
    $this->GR["X_title_yoff"]= $this->GR["img_y_size"] - $GR_X_title_off;      // y_start of Y-axis 
    $this->GR["Val_max"]     = $GR_Val_max;          // max shown value
    $this->GR["Val_min"]     = $GR_Val_min;          // min shown value
    $this->GR["yfact"]       = ($this->GR["originY"]-$this->GR["Y_max"]) / ($this->GR["Val_max"]-$this->GR["Val_min"]); //LINE_GR["yfact"]: calculate Pix factor
        
    if ($this->tmp_debug_lev > 2) {
		echo "DEBUG: Object-info: cnt:".$obj_cnt." <br>\n"; 
        echo "DEBUG: Real-limits \$infoxz:  min_val:".$this->infoxz["value_min"]."  max_val:".$this->infoxz["value_max"].
			 " obj_max_len:".$this->infoxz["obj_max_len"]." <br>\n";
        echo "DEBUG: Shown-limits: min_val:$GR_Val_min  max_val:$GR_Val_max <br>\n";
		glob_printr($this->GR, "graphic-params");
    }    

   // Create Image
  $this->imgHandle = imagecreate($this->GR["img_x_size"], $this->GR["img_y_size"]);
  if (!$this->imgHandle) die('GD error: Image creation error');

  /* Image Color allocation*/
  $this->GRCO["white"] =  imagecolorallocate($this->imgHandle, 255,255,255);
  $this->GRCO["black"] =  imagecolorallocate($this->imgHandle, 0,  0,  0  );
  $this->GRCO["red"]   =  imagecolorallocate($this->imgHandle, 255,0,  0  ); 
  $this->GRCO["orange"]=  imagecolorallocate($this->imgHandle, 250, 180, 180  );
  $this->GRCO["blue"]  =  imagecolorallocate($this->imgHandle, 180, 180, 255  );
  $this->GRCO["bluedark"]  =  imagecolorallocate($this->imgHandle, 0, 0, 255  );
  $this->GRCO["gray"]  =  imagecolorallocate($this->imgHandle, 128,128, 128);
  $this->GRCO["gray2"] =  imagecolorallocate($this->imgHandle, 200,200, 200);
  
  $this->getQuantiColors();
  $this->graph_titles($titles);
  $this->graph_axis( ); 
}

function _analyseQuant_inf() {
	// analyse quantitiy types: number or string
	$quantHasStrings=0;
	foreach( $this->quant_inf as $quant_pos=>$valarr) { // for each object do
		if ( $valarr["ty"]=="s" ) $quantHasStrings = 1;
	}
	
	
}

function _getObjExtreme() {
	// FUNCTION: get object name length
	// OUTPUT:  $this->infoxz["obj_max_len"];
	
	$maxall = 0;
	foreach( $this->obj_inf as $key=>$val) {	
		$namelen = strlen($val[0]); // analyse first element
		$maxall  = max($namelen, $maxall); 
	}
	reset($this->obj_inf);
	$this->infoxz["obj_max_len"] = $maxall;
}

function _getQuantExtreme() {
	// FUNCTION: get quantity extrema
	// OUTPUT:  $infoxz["value_min"];
	//			$infoxz["value_max"];
	$cnt=0;
	$maxQuant = sizeof($this->quant_inf);
	
	if ( sizeof($this->quantVals) ) {
		foreach( $this->quantVals as $key=>$val) {
					
			// remove all strings from $val
			for ($i=0; $i<$maxQuant; $i++) {
				if ( $this->quant_inf[$i]["ty"]=="s" or !is_numeric($val[$i]) )  $val[$i]="";
			}
			
			
			$minnow = min($val);
			$maxnow = max($val);
			if (!$cnt) {
				$minall = $minnow;
				$maxall = $maxall;
			}
			$minall = min($minnow, $minall);
			$maxall = max($maxnow, $maxall); 
			$cnt++;
			
		}
		reset ($this->quantVals); 
	}
	
	$this->infoxz["value_min"] = $minall;
	$this->infoxz["value_max"] = $maxall;

	if ( $this->infoxz["value_max"] ==$this->infoxz["value_min"] ) {
		if ( $this->infoxz["value_max"]>0 ) $this->infoxz["value_min"]=0.0;
		else $this->infoxz["value_max"]=0.0;
	}
}

function getQuantiColors() {
	// get colors of quantities in one ObjectArea
			
	global $error;
	
	if ( $this->optarr["barColor"]  == "mono" ) {
		// monocrome
		$pos=0;
    	while ( $pos < $this->quantCnt ) {
			$this->xColors[$pos] = $this->GRCO["bluedark"];
			$pos++;
		}
		return;
	}
	
	if ( $this->quantCnt > $this->quantMaxNum ) {
		$error->set("getExpColors()", 1, "Too many quantity-entries. Max ".$this->quantMaxNum);
		return;
	}
	
	// array(0,  150,0  ),   // 2 green
			
	$colorsdef = array(
		array(0,  0, 255  ),    // blue
		array(150,  150,150  ), // gray
		array(200,  0,  200),   // 3 violett
		array(230,200,  0  ),   // 4 yellow
		array(0,  255,255), // 5
		array(0  ,0  ,0 ),
		array(128,128,0  ),
		array(240, 240, 0),
		array(255,145,145), // 9
		array(0  ,128,0  ), // 10
		array(255,0  ,255), // 11
		array(0  ,0  ,150), // 12
		array(0, 80,80),
		array(128, 0,128),
		array(0, 155,155),
		array(200,255,108), 
		array(170, 0, 170),
		array(128,128,128),  // 18
		array(0,  255,0  ),   // 2 green
		array(200,  0,  200), // 3 violett
		array(230,200,  0  ),   // 4 yellow
		array(0,  255,255), // 5
		array(0  ,0  ,0 ),
		array(128,128,0  ),
		array(240, 240, 0),
		array(255,145,145), // 9
		array(0  ,128,0  ), // 10
		array(255,0  ,255), // 11
		array(0  ,0  ,150), // 12
		array(0, 80,80),
		array(128, 0,128),
		array(0, 155,155),
		array(200,255,108), 
		array(170, 0, 170),
		array(128,128,128)  // 18
	);
	
	$pos=0;
    while ( $pos < $this->quantCnt ) {
        $this->xColors[$pos] = imagecolorallocate(
					$this->imgHandle, $colorsdef[$pos][0], $colorsdef[$pos][1], $colorsdef[$pos][2] );
		$pos++;
	}
	
}
	
function graph_titles($titles   // [x] [y] [title] "subtitle"
           ) {       
       
    $thisFontSize = 3;
    $letterSize = 7;
    $color = $this->GRCO["black"];
	
	$pos["subtitle"] = array(5,18);  
     
    if ( $titles["title"] != "" ) {
        imageString( $this->imgHandle, $thisFontSize, 5, 3, $titles["title"], $color);
    }
	
    if ( $titles["x"] != "" ) {
        $xpos = $this->GR["originX"] + ($this->GR["X_max"]-$this->GR["originX"])*0.5 - 0.5* $letterSize*strlen($titles["x"]);
        if ($xpos<$this->GR["originX"]) $xpos=$this->GR["originX"];
        imageString( $this->imgHandle, $thisFontSize, $xpos, $this->GR["img_y_size"]-15, $titles["x"], $color); 
    } 
    if ( $titles["y"] != "" ) {
        imageStringUp( $this->imgHandle, $thisFontSize, 3, $this->GR["originY"]-3, $titles["y"], $color); 
    } 
	
	if ( $this->errorIntern != "" ) {
        imageString( $this->imgHandle, 2, $pos["subtitle"][0], $pos["subtitle"][1] , $this->errorIntern, $this->GRCO["red"]); 
		$pos["subtitle"][1] = $pos["subtitle"][1] + 12;
    }
	
    if ( $titles["subtitle"] != "" ) {
        imageString( $this->imgHandle, 2, $pos["subtitle"][0], $pos["subtitle"][1] , $titles["subtitle"], $color); 
    }
	
	
} 

function _yaxisTick($x0str, $ival) {
	$x0   =$this->GR["originX"]-3;
    $x1   =$x0+6;
	$y0   = $this->GR["originY"] - ($ival-$this->GR["Val_min"])*$this->GR["yfact"];
	imageLine ( $this->imgHandle, $x0, $y0, $x1, $y0,  $this->GRCO["gray"]);
	imageString( $this->imgHandle, $this->GR["FONT_SIZE"], $x0str, $y0 , sprintf("%.1f", $ival), $this->GRCO["gray"]);
}

function graph_axis( $axis_data = NULL // ["x"], ["y"] [0]=min [1]=max
     ) {
	
   // get exponent
   $logval   = $this->GR["Y_axlog10"];
   $tickbase = pow(10, $logval);	   
   $ival     =    $tickbase*0;
   $x0str=$this->GR["originX"]-22 - $logval*7;
   
   
    // X- axis
   $x0=$this->GR["originX"];
   $x1=$this->GR["X_max"];
   $y0=$this->GR["originY"];
   $y1=$this->GR["originY"];
   imageLine ( $this->imgHandle, $x0, $y0, $x1, $y1,  $this->GRCO["gray"]);
   
   // === Y-axis ===
   $x0=$this->GR["originX"];
   $x1=$x0;
   $y0=$this->GR["originY"];
   $y1=$this->GR["Y_max"];
   imageLine ( $this->imgHandle, $x0, $y0, $x1, $y1,  $this->GRCO["gray"]);
   
   // Y-axis extrema 
	/*  
   $xpos=$x0str;
   $ypos=$this->GR["originY"];
   imageString( $this->imgHandle, $this->GR["FONT_SIZE"], $xpos, $ypos , $this->GR["Val_min"], $this->GRCO["gray"]);
   
   $ypos=$this->GR["Y_max"];
   imageString( $this->imgHandle, $this->GR["FONT_SIZE"], $xpos, $ypos , $this->GR["Val_max"], $this->GRCO["gray"]);
   */
	
   //
   // show Y-axis ticks
   //
   

   $cnt  = 0;
   while ( $cnt <= 10 ) { 
	   
	    $ival   = $tickbase * $cnt;
		$cnt++;
		if ($ival>$this->GR["Val_max"]) break;
		if ($ival < $this->GR["Val_min"]) continue;
		$this->_yaxisTick($x0str, $ival);
		
		
   }
   
   
   // X-axis ticks
   end($this->obj_inf);
   list($lastpos, ) = each($this->obj_inf);
   $obj_pos  = 0;
   reset($this->obj_inf);
   $y0 = $this->GR["originY"];
   $y1 = $y0 + 5;
   while ( $obj_pos <= $lastpos ) { 
	   
		$x_pix   = $this->GR["originX"] + $obj_pos * $this->GR["X_UnitWidht"];	
		imageLine ( $this->imgHandle, $x_pix, $y0, $x_pix, $y1,  $this->GRCO["gray"]);	
		$obj_pos++;
		
   }
   
   
   // 0.0
   $y0=$this->GR["originY"];
   $y1=$y0; 
   imageLine ( $this->imgHandle, $x0, $y0, $x1, $y1,  $this->GRCO["gray"]);
   // Max
   $y0=$this->GR["Y_max"];
   $y1=$y0; 
   imageLine ( $this->imgHandle, $x0, $y0, $x1, $y1,  $this->GRCO["gray"]);
  
} 

function out_quant_txt( $obj_pos, $quant_id, $text ) {
	// print quantity-text
	$y_pix   = $this->GR["originY"] - 3;
    $x_pix   = $this->GR["originX"] + $obj_pos * $this->GR["X_UnitWidht"] + $quant_id*$this->GR["BAR_WIDTH"];
	$color   = $this->GRCO["black"];
	
	imageStringUp($this->imgHandle, $this->GR["FONT_SIZE"], $x_pix, $y_pix, $text, $color);
}

function out_xax_text( $obj_pos, $text ) {
	$y_pix   = $this->GR["originY"] + $this->GR["X_UnitText_yoff"];
    $x_pix   = $this->GR["originX"] + $obj_pos * $this->GR["X_UnitWidht"] + 1;
	$color   = $this->GRCO["black"];
	
	imageStringUp($this->imgHandle, $this->GR["FONT_SIZE"], $x_pix, $y_pix, $text, $color);
}

function out_value( $obj_pos, $quant_id, $value ) {
    
	$col_now = $this->xColors[$quant_id];
    $color   = $this->GRCO["black"];
    $y_pix   = $this->GR["originY"] + $this->GR["X_UnitText_yoff"];
    $x_pix   = $this->GR["originX"] + $obj_pos * $this->GR["X_UnitWidht"] + $quant_id*$this->GR["BAR_WIDTH"]; 
    $x0=$x_pix + 1;
    $x1=$x0+$this->GR["BAR_WIDTH"]-1;
    $y1=$this->GR["originY"];
    $y0=$this->GR["originY"] - ($value - $this->GR["Val_min"])*$this->GR["yfact"];
     
    imageFilledRectangle ( $this->imgHandle, $x0, $y0, $x1, $y1, $col_now );
    
}

/**
 * output a value box (start,end)
 * @param array $qvals [0]: start, [1]:end
 * @param string $colorOpt alternative color name
 */
function out_valBox( $quant_id, $qvals, $colorOpt=NULL ) {
	$obj_pos  = 0;
	$col_now = $this->xColors[$quant_id];
    $color   = $this->GRCO["black"];
    if ($colorOpt!=NULL) $col_now = $this->GRCO[$colorOpt];
    $y_pix   = $this->GR["originY"] + $this->GR["X_UnitText_yoff"];
    $x_pix   = $this->GR["originX"] + $obj_pos * $this->GR["X_UnitWidht"] + $quant_id*$this->GR["BAR_WIDTH"]; 
    $x0=$x_pix + 1;
    $x1=$x0+$this->GR["BAR_WIDTH"]-1;
    $y1=$this->GR["originY"] - ($qvals[0] - $this->GR["Val_min"])*$this->GR["yfact"];
    $y0=$this->GR["originY"] - ($qvals[1] - $this->GR["Val_min"])*$this->GR["yfact"];
     
    imageFilledRectangle ( $this->imgHandle, $x0, $y0, $x1, $y1, $col_now );
}

function out_badsubst( $obj_pos, $quant_id ) {

    $color = $this->GRCO["black"];
    $y_pix = $this->GR["originY"] + $this->GR["X_UnitText_yoff"];
    $x_pix = $this->GR["originX"] + $obj_pos * $this->GR["X_UnitWidht"] + $quant_id*$this->GR["BAR_WIDTH"];
    $x0 = $x_pix + 1;
	$y0 = $this->GR["originY"]-5;
    $x1 = $x0+$this->GR["BAR_WIDTH"];
    $y1 = $this->GR["originY"]+5;
    $col_now = $this->GRCO["red"];  
    imageline( $this->imgHandle, $x0, $y0, $x1, $y1,  $col_now );
	$ytmp = $y1;
	$y1   = $y0;
	$y0   = $ytmp;
	imageline    ( $this->imgHandle, $x0, $y0, $x1, $y1,  $col_now );
}

function draw2( 
	$obj_pos 	// pos of object
	) {
   // FUNCTION: draw one object-area
   // INPUT: $this->quant_inf

  
   $obj_arr = &$this->quantVals[$obj_pos];
   if (!sizeof($obj_arr)) return;
   
   foreach( $obj_arr as $quant_id=>$qval) {
		
		if ( $this->tmp_debug_lev > 3 )echo "   DEBUG:obj:$obj_pos  quantid:$quant_id :: $qval <br>\n"; 
		do {
			if ( is_numeric($qval) ) {
				$this->out_value( $obj_pos, $quant_id, $qval );	
				break;
			}
			if ($qval=="") {
				$this->out_badsubst( $obj_pos, $quant_id );
				break;
			}
			// is a string !!!
			$this->out_quant_txt( $obj_pos, $quant_id, $qval );
			
		} while (0);
   }
   reset($obj_arr);
   
   $obj_name = $this->obj_inf[$obj_pos][0];
   $this->out_xax_text( $obj_pos, $obj_name );
}

/**
 * draw one object-area with (Y_start, Y_end) tuples
 * @param $obj_pos 	: pos of object
 * @global $this->quantMiMa
 */
function draw3( $obj_pos, &$quantMiMa ) {
   
   $obj_arr = &$quantMiMa[$obj_pos];
   if (!sizeof($obj_arr)) return;
   
   foreach( $obj_arr as $obj_pos=>$qvalsOfPos) {
		
   		if ( sizeof($qvalsOfPos) ) {
   			if ( $this->tmp_debug_lev > 3 )echo "  DEBUG:obj:$obj_pos :: ".print_r($qvalsOfPos,1)." <br>\n"; 
   			reset ($qvalsOfPos);
			foreach( $qvalsOfPos as $dummy=>$valMiMa) {
				$this->out_valBox( $obj_pos, $valMiMa );	
			}
			reset ($qvalsOfPos); 
   			
   		}
   }
   reset($obj_arr);
   
   $obj_name = $this->obj_inf[$obj_pos][0];
   $this->out_xax_text( $obj_pos, $obj_name );
}

function _legendName(
	$name,
	$pos,    // 0..n
	$colorx
	) {
	// FUNCTION: print one legend-name with matching color
			
	$x_pix = $this->GR["X_max"] + 3;
	$y_pix = $this->GR["legend_yoffs"] + $pos*12;
	$color = $this->GRCO["black"];
	
	$x0 = $x_pix;
	$y0 = $y_pix;
	$x1 = $x0 + 8;
	$y1 = $y0 + 3;
	
	imageFilledRectangle ( $this->imgHandle, $x0, $y0, $x1, $y1, $colorx );
	imageString( $this->imgHandle, $this->GR["FONT_SIZE"], $x_pix+12, $y_pix-5, $name, $color);
}

function drawQuantLegend() {
	// FUNCTION: print full Quantity legend
			
	foreach( $this->quant_inf as $quant_pos=>$valarr) { // for each object do
		$quantname = $valarr["name"];
		$this->_legendName($quantname, $quant_pos, $this->xColors[$quant_pos] );
	}
	
}

function show() {
  if ( $this->tmp_debug_lev<=1 ) {
    	ImagePNG    ($this->imgHandle);
  }
  ImageDestroy($this->imgHandle);
}

function errorImage( $err_text, $err_type=NULL ) {

  if ($this->imgHandle) ImageDestroy($this->imgHandle);
  else Header("Content-type: image/png");
  
  $ydiff = 14;
    // calc number of lines of the text field
  $txtarr    = explode ("\n", $err_text);
  $txtLines  = sizeof($txtarr);
  $img_y_pix = $ydiff * $txtLines + 6;
  $img_x_pix = 600;
  $imgHandle     = imagecreate($img_x_pix, $img_y_pix);
  $GRCO["red"]   = imagecolorallocate($imgHandle, 255,0,0);   
  $GRCO["gray"]  = imagecolorallocate($imgHandle, 128,128,128);
  $white = imagecolorallocate($imgHandle, 255,255,255);
  imageFill   ($imgHandle, 0, 0, $white);
  
  if ( $err_type == "INFO" ) { 
    $txtcolor = $GRCO["gray"];
    $err_pre = "INFO: ";
  } else {    
    $txtcolor = $GRCO["red"];  
    $err_pre = "ERROR: ";
  } 
  imageRectangle ( $imgHandle, 0, 0, $img_x_pix-1, $img_y_pix-1,  $GRCO["gray"] );
  // write out text lines to image
  $ytmp = 2;
  $i=0;
  foreach( $txtarr as $dummy=>$txttmp) {
    if (!$i) $txttmp = $err_pre . $txttmp;    // PRE text like "INFO: "
    imageString ($imgHandle, 3, 10, $ytmp, $txttmp, $txtcolor);
    $ytmp = $ytmp + $ydiff;
    $i++;
  }
  ImagePNG($imgHandle);
  ImageDestroy($imgHandle);
}

function drawAll(
	$opt=NULL // "nolegend" : 0,1
	) {
	// FUNCTION: draw everything
			
	if ($opt["nolegend"]<=0) $this->drawQuantLegend(); 
	
	foreach( $this->obj_inf as $obj_pos=>$valarr) { // for each object do
		$this->draw2($obj_pos);
	}
	reset ($this->obj_inf);
	
	$this->show();
	
}

function drawOneObject() {
	// FUNCTION:- draw bargraph with ONE color
	//			- no legend
	
	foreach( $this->obj_inf as $obj_pos=>$valarr) { // for each object do
		
		$this->draw2($obj_pos);
	}
	reset ($this->obj_inf);
	
	$this->show();
	
}

function infoOut(
	$text, 
	$color=NULL // "red", "gray"
	) {
	// FUNCTION: show error text as graphics
	$imgH = imagecreate( 500, 40 );
	if ( !$imgH )  {
		echo "ERROR: $text\n";
	}
	// $imNow = &$this->imH;
	
	$colorx = array(100,100,100);
	if ($color=="red") $colorx = array(255,0,0);
	
	$white  = ImageColorAllocate($imgH, 255,255,255); 
	$dark   = ImageColorAllocate($imgH, $colorx[0],$colorx[1],$colorx[2]);
	imagestring( $imgH, 3, 4, 15, "$text", $dark);
	Imagepng    ( $imgH );
	ImageDestroy( $imgH );	
	$imgH = NULL;
}

}