<?php
/*MODULE:  f.grImgHelp.inc
  DESCR:   some image functions
  AUTHOR:  qbi
  VERSION: 0.1 - 20020904
*/

class fGrImgHelp {

function open( $imageIn // PNG-image
	) {
	$this->img_size = getimagesize( $imageIn );
	$this->imgGd = imagecreatefrompng($imageIn);
}

function _getAlpha() {
	if (!$this->alpha) {
		$this->alpha = imagecolorallocate($this->imgGd, 255,255,255);
	}
	return ($this->alpha);
}

function _drawEdge( 
	$edgeFlag, // 0,1,2,3
	&$imgCircle,
	&$circ,
	$imgSize
 ) {
	// draw one edge
	
	$imgPosArr = array( 
		array(0,0), 
		array($imgSize [0]-$circ["radius"],0), 
		array($imgSize [0]-$circ["radius"],	$imgSize[1]-$circ["radius"]),
		array(0,$imgSize[1]-$circ["radius"]), 
		);
		
	$imgCircArr = array( 
		array(0,0), 
		array($circ["radius"],0), 
		array($circ["radius"],$circ["radius"]),
		array(0,$circ["radius"]), 
		);	
		
	$xStart = $imgCircArr[$edgeFlag][0];
	$yStart = $imgCircArr[$edgeFlag][1];
	
	$xDestOff  = $imgPosArr[$edgeFlag][0];
	$yDestOff  = $imgPosArr[$edgeFlag][1];
	
	$ypos  = $yStart;
	$ydest = $yDestOff;
	
	for ( $y=0; $y<$circ["radius"]; $y++ ) {
	
		$xpos  = $xStart;
		$xdest = $xDestOff;
		
		for ( $x=0; $x<$circ["radius"]; $x++ ) {
			$rgb = ImageColorAt( $imgCircle, $xpos, $ypos);
			
			// imagestring ( $this->imgGd, 2, 10+$x*10, 30+$y*7, $rgb, $circ["alpha"] );
			if ($rgb==0) {
				imagesetpixel ( $this->imgGd, $xdest, $ydest, $circ["alpha"] );
			}
			$xpos++;
			$xdest++;
		}
		$ypos++;
		$ydest++;
	}
}

function roundEdges( 
	$edgeOpt=NULL // "radius" => number (10)
				  // "oneEdge" => only one edge: 0,1,2,3
	) {

	// read image
	// make radius
	
	$alpha = $this->_getAlpha();
	imagecolortransparent ($this->imgGd, $alpha);
	
	$radius = 10;
	if ( $edgeOpt["radius"]>0 ) $radius=$edgeOpt["radius"];
	$diameter = $radius*2;
	$start  = array($radius, $radius);  /*
	
		+---
	    |
	    |        ----> 0
	*/
	$imgCircle = @imagecreate($diameter, $diameter);
	$circ["white"]     = imagecolorallocate($imgCircle, 255,255,255);
	$circ["black"]     = imagecolorallocate($imgCircle, 0,0,0);
	$circ["radius"] = $radius;
	$circ["alpha"]  = $alpha;
	
	imagefilledrectangle ($imgCircle,0, 0, $diameter-1, $diameter-1, $circ["white"]  );
	imagefilledellipse( $imgCircle, $radius, $radius, $diameter, $diameter, $circ["black"] );
	// imagefilledellipse($this->imgGd, $radius, $radius, $diameter, $diameter, $this->circ["black"] );

	// Imagepng( $imgCircle, "/home/steffen/html/Temp.tiber/circle.png" ); // save image to cache
	if ( $edgeOpt["oneEdge"]!= "" ) {
		$this->_drawEdge( $edgeOpt["oneEdge"], $imgCircle, $circ, $this->img_size);
	} else { 
		// all edges
		for ($i=0; $i<4; $i++) {
			$this->_drawEdge( $i, $imgCircle, $circ, $this->img_size);
		}
	}
	/*
	imagefilledarc ( $this->imgGd, $start[0], $start[1], $radius*2, $radius*2, 180, 270, $alpha, IMG_ARC_PIE );
	imagefilledarc ( $this->imgGd, $img_size[0]-$start[0], $start[1], $radius*2, $radius*2, 270, 0, $alpha, IMG_ARC_PIE );
	imagefilledarc ( $this->imgGd, $img_size[0]-$start[0],  $img_size[1]-$start[1], $radius*2, $radius*2, 0, 90, $alpha, IMG_ARC_PIE );
	imagefilledarc ( $this->imgGd, $start[0], $img_size[1]-$start[1], $radius*2, $radius*2, 90, 180, $alpha, IMG_ARC_PIE );
	*/
	
}

function Stripes() {
	// make image stripes right
	
	$xOffs  = 40;
	$yArea  = $this->img_size[1];
	$y2		= $yArea*0.5;
	$xStart = $this->img_size[0]-$xOffs;
	$alpha  = $this->_getAlpha();
	$ystep  = 2;
	$radius = $y2 + 10;

	
	for ( $y=0; $y<$yArea; $y=$y+$ystep ) {

		$ypos = $y;
		$xpos = $x;

		$x0 = $xStart;
		$y0 = $ypos;
		$x1 = $this->img_size[0];
		$y1 = $ypos;
		
		$yrel = abs($ypos-$y2);
		$xRel = sqrt($radius*$radius - $yrel*$yrel);
		$x0   = $xStart - $xOffs + $xRel;
		
		imageline ( $this->imgGd, $x0, $y0, $x1, $y1, $alpha );
		/*
		if ($y==1 AND $x<8) imagestring ( $this->imgGd, 2, 5+$y*10, 5+$x*7, "o:".$rgb." N:".$colorx." c:".
			$newcol["red"].":". $newcol["green"].":". $newcol["blue"]
			, $white );
		*/
	}
	
}

function darkDown() {
	// make image dark
	$yArea = 50;
	$alpha = $this->_getAlpha();
	$colStep= -10;
	$ypos = $this->img_size[1]-$yArea;
	$white     = imagecolorallocate( $this->imgGd, 255,255,255);
	$colDif =0;
	
	for ( $y=0; $y<$yArea; $y++ ) {

		$xpos = 0;
		for ( $x=0; $x< $this->img_size[0]; $x++ ) {
		
			$rgb = ImageColorAt( $this->imgGd, $xpos, $ypos );
			
			if ($rgb!=$alpha) {
			
				$dst_rgb = imagecolorsforindex( $this->imgGd, $rgb );
				$newcol=NULL;
				foreach( $dst_rgb as $key=>$val) {
					$newcol[$key] = $dst_rgb[$key]-$colDif;
					if ($newcol[$key]<0)  $newcol[$key]=0;
					if ($newcol[$key]>255)  $newcol[$key]=255;
				} 
				reset ($dst_rgb);
				
				$colorx = imagecolorexact  (  $this->imgGd , $newcol["red"], $newcol["green"], $newcol["blue"] );
				if ($colorx<0) {
					$colorx =  imagecolorallocate( $this->imgGd, $newcol["red"], $newcol["green"], $newcol["blue"] );
				}
				
				imagesetpixel ( $this->imgGd, $xpos, $ypos, $colorx );
				/*
				if ($y==1 AND $x<8) imagestring ( $this->imgGd, 2, 5+$y*10, 5+$x*7, "o:".$rgb." N:".$colorx." c:".
					$newcol["red"].":". $newcol["green"].":". $newcol["blue"]
					, $white );
				*/
			}
			$xpos++;
			
		}
		$ypos++;
		$colDif = $colDif + $colStep;
	}
	
}

function imageOut($imageOut) {
	Imagepng( $this->imgGd, $imageOut ); // save image to cache
	ImageDestroy($this->imgGd);
}

}
