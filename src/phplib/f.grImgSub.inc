<?php
/**
 * f.grImgSub.inc : collection of image color functions
 *
 * @author  Steffen Kube
 * @version 1.0
 * @package image
 * @swreq SREQ:0001412: p.089 -> Result View Timedia
 * added another color map for better differences
 */



class fImageCreaC {

/* DO NOT use RED (255,0,0) because it is used for errors and references
 * array of easily distinguishable colors for {@link assign_colors}
 */

var $def_colors = array(
	array(100,0,  100), // 1 dark violett  ???
	array(0,  255,0  ), // 2 green
	array(0,  0,  255), // 3 blue
	array(128,0,  0  ), // 4 dark red
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

/* better differnce and darker colormap for timeseries */
var $def_colors_ts = array(
	array(127, 0, 0),
	array(0, 255, 127),
	array(255, 127, 0),
	array(127, 0, 255),
	array(0, 127, 0),
	array(255, 0 ,127),
	array(0, 127, 255),
	array(0, 0, 127),	//1 down
	array(127, 255, 0),	//1 up
	array(127, 0, 127),
	array(0, 127, 127),
	array(127, 127, 0),
	array(127, 127, 127),
	array(0, 0, 0)		//14
);

function __construct( 
	&$imgHandle // can be NULL
	 ) {
	$this->imgHandle = &$imgHandle;
}

function setImgPointer(&$imgHandle) {
	$this->imgHandle = &$imgHandle;

}

/**
 * assign easily distinguishable colors to the keys of an array
 *
 * only assignes up to {@link IMAGE_CREATION__MAX_DIST_COLORS}
 *
 * @param object instance of subclass of {@link CDBAbstract}
 * @param object handle of an image-object (return value of ImageCreate)
 * @param array  values are taken as keys for return
 * @return array assigning values of input_array to return value of ImageColorAllocate
 * @global array array of distinguishable colors
 * old: image_creation__assign_colors
 */
function &assign_colors( 
	$colposarr,		// array[CNT] = POS;
	$key=NULL
	) 
{
	$MAX_COLORS = 100;
    $cnt    = 0;
	$cntall = 0;
	if ($key=="ts") { //timeseries colors
		$colorArr = $this->def_colors_ts;
		$cntDefcols = count($colorArr);
	}
	else {
		$colorArr = $this->def_colors;
		$cntDefcols = count($colorArr);
	}
    $res = array(); // result array
    reset($colposarr);
	
    foreach( $colposarr as $dummy=>$val) {
        // break, if too many entries in $array	
        $res[$val] = imagecolorallocate( $this->imgHandle,
                                        $colorArr[$cnt][0],
                                        $colorArr[$cnt][1],
                                        $colorArr[$cnt][2]);
		$cnt++;
		if ($cnt>=$cntDefcols) $cnt=0;
		$cntall++;
		        
		if ($cntall>$MAX_COLORS) break; 
	}
	reset ($colposarr);
	return $res;
}


}