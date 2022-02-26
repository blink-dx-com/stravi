<?php
/**
 * create a container-graphics
 * $Header: trunk/src/phplib/gui/o.CONTAINER.img.inc 59 2018-11-21 09:04:09Z $
 * @package o.CONTAINER.img.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

require_once('f.grImgSub2.inc');
require_once ("down_up_load.inc");
require_once 'o.ABS_CONTAINER.subs.inc';

/**
 * - create container image
 * - one BOX: 10 pix (BOX_x_px)
 * |S |R |B| R| S|
 */
class oCONTAINER_img {
    
	function __construct() {
		
		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
		    $this->_debug = $_SESSION["userGlob"]["g.debugLevel"];
		}
		
		$this->abs_cont_lib = new oABS_CONTAINER_subs();
	}
	
	/**
	 * init image for ABSTRACT container
	 * @param $sqlo
	 * @param array $option <pre>
	 * 	   'limits'=> array('S'=>5, 'R'=>4, 'B'=>16 )
	 *     'coord_type' = 'BOX
	 * </pre>
	 * @return NULL
	 */
	function initImage_Abs( &$sqlo, $abs_cont_id, $option=NULL ) {
	    $this->initImage_Conc( $sqlo, 0, $abs_cont_id, $option );
	}
	
	/**
	 * init image for CONCRETE container
	 * @param $sqlo
	 * @param $cont_id : if 0 : init the abstract container
	 * @param array $option <pre>
	 * 	   'limits'=> array('S'=>5, 'R'=>4, 'B'=>16 )
	 *     'coord_type' = 'BOX
	 * </pre>
	 * @return NULL
	 */
	function initImage_Conc( &$sqlo, $cont_id, $abs_cont_id, $option=NULL ) {
		
	    $this->abs_cont_id = $abs_cont_id;
	    if ($abs_cont_id) $this->abs_cont_lib->setContainer($sqlo, $abs_cont_id);
	    
		$limits = $option['limits'];
		$this->coord_type = $option['coord_type'];
		
		$this->limits = $limits;
		$this->inits=array();
		$this->inits['BOX_x_px'] = 30; // OLD:10
		$this->inits['BOX_y_px'] = 30; // OLD:10
		
		if ( ($limits['S']*$limits['R']*$limits['B']) > 400) {
		    $this->inits['BOX_x_px'] = 10; 
		    $this->inits['BOX_y_px'] = 10; 
		}
		
		$flist = array();
		for ($i=1;$i<=$limits['B']+1;$i++) {
			$f = $limits['B']/$i;
			if (is_int($f)) $flist[] = $f;
		}
		sort($flist);
		$box=array();
		$box[0] = $flist[floor(sizeof($flist)/2)];
		if ($box[0]) {
		    $box[1] = $limits['B']/$box[0];
		} else {
		    $box[1] = 1;
		}
		
		
		$this->inits['BOX_x_num'] = $box[0]; // number of boxes in X
		$this->inits['BOX_y_num'] = $box[1];
		
		// calc borders
		
		//$C_shelfLines = 2; // 2 pixels
		$CON_px = 2;       // border pixel num
		$this->inits['B_xlen'] = $this->inits['BOX_x_px']+$CON_px;
		$this->inits['B_ylen'] = $this->inits['BOX_y_px']+$CON_px;
		$this->inits['R_xlen'] = $this->inits['B_xlen']*$this->inits['BOX_x_num']+$CON_px;
		$this->inits['R_ylen'] = $this->inits['B_ylen']*$this->inits['BOX_y_num']+$CON_px;
		$this->inits['S_xlen'] = $this->inits['R_xlen']*$limits['R'] + $CON_px;
		$this->inits['S_ylen'] = $this->inits['R_ylen']*1 + $CON_px;
		
		$this->inits['CONT_xlen_px'] = $this->inits['S_xlen'] + $CON_px;
		$this->inits['CONT_ylen_px'] = $this->inits['S_ylen'] * $limits['S'] + $CON_px;
		
		$this->inits['CONT_xof_px'] = 20;
		$this->inits['CONT_yof_px'] = 20;
		
		$this->inits['IMG_legOf_x'] = 20;
		$this->inits['IMG_legOf_y'] = 5;
		$this->inits['IMG_legHi_y'] = 50;
		$legend_yLen = $this->inits['IMG_legOf_y'] + $this->inits['IMG_legHi_y'];
		
		$bord_x = 10;
		
		$this->inits['img_x_size'] = $this->inits['CONT_xof_px'] + $this->inits['CONT_xlen_px'] + $bord_x;
		$this->inits['img_y_size'] = $this->inits['CONT_yof_px'] + $this->inits['CONT_ylen_px'] + $legend_yLen ;
		$this->inits['CONT_yBof_px'] = $this->inits['img_y_size']-$this->inits['CONT_yof_px'];
		
		$this->ImgLib = new fgrImgSub2C();
		$optarr=NULL;
		$GR = array(
			'img_x_size'=>$this->inits['img_x_size'],
			'img_y_size'=>$this->inits['img_y_size'],
			'GRCO'=> array(
			    'green4'=>array(49,183, 62), // dark green
			    'green3'=>array(68, 226,89), // middle green
			    'green2'=>array(100,239,95), // light 2 green
				'green1'=>array(180,255,100), // light green
				'gray' =>array(200,200,200),
				'gr1' =>array(100,100,100),
				'gr2' =>array(201,201,201),
				'S'=>array(0,0  ,100),
				'R'=>array(100,100,200),
				'B'=>array(200,200,250),
				'C'=>array(100,100,100),
				)
			);
		$this->ImgLib->init ( $GR, $optarr );
		$this->imgH = $this->ImgLib->getImgHandle();
		$this->GRCO = $this->ImgLib->getGRCO();
		
		if ($this->_debug>1) glob_printr( $this->inits,'inits' );
	}
	
	/**
	 * get lower left coord of s,r,b
	 * @return 
	 * @param object $s
	 * @param object $r
	 * @param object $b
	 */
	private function _getXY($s, $r, $b){
		
		$CON_px = 1;
		$inits = &$this->inits;
		$bposY = intval(($b-1) / $inits['BOX_x_num']);
		$apx=array();
		$apx[0] = 5*$CON_px + ($r-1)*$inits['R_xlen'] + fmod($b-1, $inits['BOX_x_num'])*$inits['B_xlen'];
		$apx[1] = 4*$CON_px + ($s-1)*$inits['S_ylen'] + 0*$inits['R_ylen'] + $bposY*$inits['B_ylen'];
		$apx[3] = $apx[1] + $inits['B_ylen'] - $CON_px*3;
		
		$apAbs=array();
		$apAbs[0] = $inits['CONT_xof_px'] + $apx[0];
		$apAbs[1] = $inits['CONT_yBof_px']- $apx[3];
		
		return $apAbs;
	}
	
	/**
	 * 
	 * @return 
	 * @param object $apx relative coordiates
	 * @param object $key 'S', 'R', ...
	 */
	private function _drawBox($apx, $key) {
		imagerectangle($this->imgH, 
			$this->inits['CONT_xof_px']+$apx[0], 
			$this->inits['CONT_yBof_px']-$apx[1], 
			$this->inits['CONT_xof_px']+$apx[2],
			$this->inits['CONT_yBof_px']-$apx[3], 
			$this->GRCO[$key]);
	}
	
	/**
	 * draw TEXT in box
	 * @param int $s
	 * @param int $r
	 * @param int $b
	 * @param string $key
	 * @param string $text
	 */
	function _textBySRB( $s, $r, $b, $key, $text) {
		$font  = 2;
		$color = $this->GRCO['black'];
		$xy = $this->_getXY($s, $r, $b);
		
		if ($key=='S') {
			$x = $xy[0] - 25;
			$y = $xy[1];
			
		}
		if ($key=='R') {
			$x = $xy[0];
			$y = $xy[1] + 10 + $this->inits['BOX_y_px']; //old:20
			
		}
		if ($key=='B') { // Box
		    $font  = 2;
		    $x = $xy[0] + 1;
		    $y = $xy[1];
		}
		imagestring($this->imgH,  $font, $x,$y, $text, $color);
	}
	
	function _drawText($apx, $key, $text, $cnt) {
		
		$font  = 1;
		$color = $this->GRCO['black'];
		
		$x = $this->inits['CONT_xof_px'] + $apx[0];
		$y = $this->inits['CONT_yBof_px']- $apx[1]; 
		
		imagestring($this->imgH,  $font, $x, $y,
			$text, $color);
	}
	
	private function _shadows() {
		$CON_px = 1;
		$inits  = &$this->inits;
		
		$apx=array();
		$apx[0]= $CON_px;
		$apx[1]= 0;
		$apx[2]= $apx[0] + $inits['CONT_xlen_px'] - $CON_px;
		$apx[3]= $apx[1] + $inits['CONT_ylen_px'] - $CON_px;
			
		$this->_drawBox($apx, 'C');
		
		
		/* shadows : VERTICAL */
		$shi=0;
		while ( $shi<5 ) {
			$apx[0]= $inits['CONT_xof_px'] + $this->inits['CONT_xlen_px'] + $shi + 1;
			$apx[1]= $inits['CONT_yBof_px'] - $shi - 1;
			$apx[2]= $apx[0];
			$apx[3]= $apx[1] - $this->inits['CONT_ylen_px'] + 1;
			
			ImageLine($this->imgH, $apx[0], $apx[1], $apx[2], $apx[3], $this->GRCO['gr1'] );
			$shi++;
		}
		/* shadows */
		$shi=0;
		while ( $shi<5 ) {
			$apx[0]= $inits['CONT_xof_px'] + $shi + 1;
			$apx[1]= $inits['CONT_yBof_px'] - ($this->inits['CONT_ylen_px'] + $shi);
			$apx[2]= $apx[0] + $this->inits['CONT_xlen_px'];
			$apx[3]= $apx[1];
			
			ImageLine($this->imgH, $apx[0], $apx[1], $apx[2], $apx[3], $this->GRCO['gr2'] );
			$shi++;
		}

	}
	
	function _getBoxCoord($s, $r, $b) {
	    
	    $CON_px = 1;
	    $inits = &$this->inits;
	    $bposY = intval(($b-1) / $inits['BOX_x_num']);
	    $apx=NULL;
	    $apx[0] = 5*$CON_px + ($r-1)*$inits['R_xlen'] + fmod($b-1, $inits['BOX_x_num'])*$inits['B_xlen'];
	    $apx[1] = 4*$CON_px + ($s-1)*$inits['S_ylen'] + 0*$inits['R_ylen'] + $bposY*$inits['B_ylen'];
	    $apx[2] = $apx[0] + $this->inits['B_xlen'] - $CON_px*3;
	    $apx[3] = $apx[1] + $this->inits['B_ylen'] - $CON_px*3;
	    
	    return ($apx);
	}
	
	private function _get_max_arr() {
	    $limits = $this->limits;
	    
	    $max=NULL;
	    $max[0]=$limits['S'];
	    $max[1]=$limits['R'];
	    $max[2]=$limits['B'];
	    return $max;
	}
	
	/**
	 * if coord_type=BOX : drwa the alias names
	 */
	public function draw_box_names($sqlo) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    if ($this->coord_type!='BOX') return;
	    
	    $cnt=array();
	    $max=$this->_get_max_arr();

	    $i=0;
	    $cnt[$i] = 1;
	    
	    while ( $cnt[0] <= $max[0] ) {  // SHELF

	        $i=1;
	        $cnt[$i] = 1;
	        
	        while ( $cnt[1] <= $max[1] ) { // RACK
	            
	           
	            $alias = $this->abs_cont_lib->get_alias_by_SRB($sqlo, $cnt[0], $cnt[1], 1);   
	            
	            debugOut('(306) COORDs:'.print_r($cnt,1)." alias: $alias", $FUNCNAME, 1);
	            // echo "DDDD: ".print_r($cnt,1)." aluias: $alias<br>";
	            $this->_textBySRB($cnt[0], $cnt[1], 1, 'B', $alias) ;
	            
	            $cnt[$i] = $cnt[$i] + 1;
	        }
	        
	        $i=0;
	        $cnt[$i] = $cnt[$i] + 1;
	    }
	}
	
	function drawContainer() {
		$limits = $this->limits;
		//$inits  = &$this->inits;
		$cnt=array();
	
		$pos=1;
		$i=0;
		
		$max=NULL;
		$max[0]=$limits['S'];
		$max[1]=$limits['R'];
		$max[2]=$limits['B'];
		
		$cnt[$i] = 1;
		$CON_px = 1; // one pixel border
		
		$this->_shadows();
		
		$ti=1;
		while ( $ti <= $limits['R'] ) {
			$this->_textBySRB(1, $ti, 1, 'R', 'R'.$ti ) ;
			$ti++;
		}
		
		$save=array();
		
		while ( $cnt[$i] <= $max[$i] ) {  // SHELF
		    $apx=array();
			$apx[0]= $CON_px + $CON_px;
			$apx[1]= $CON_px + ($cnt[$i]-1) * $this->inits['S_ylen'];
			$apx[2]= $apx[0] + $this->inits['S_xlen'] - 1;
			$apx[3]= $apx[1] + $this->inits['S_ylen'] - 1;
			
			$this->_drawBox($apx, 'S');
			$this->_textBySRB($cnt[$i], 1, 1, 'S', 'S'.$cnt[$i] ) ;
			
			$save[$i] = $cnt[$i];
			$i=1;
			$cnt[$i] = 1;
			
		
			while ( $cnt[$i] <= $max[$i] ) { // RACK
			
			    $apr=array();
				$apr[0]= $apx[0] + ($cnt[$i]-1) * $this->inits['R_xlen'] + $CON_px;
				$apr[1]= $apx[1] + 1;
				$apr[2]= $apr[0] + $this->inits['R_xlen'] - 1;
				$apr[3]= $apr[1] + $this->inits['R_ylen'] - 1;
				$this->_drawBox($apr, 'R');
				// $this->_drawText($apr, 'R', $cnt[$i] ) ;
				
				$save[$i] = $cnt[$i];
				$i=2;
				$cnt[$i] = 1;
				while ( $cnt[$i] <= $max[$i] ) { //  BOX
					$bCnt  = $cnt[$i]-1;
					$bposY = intval($bCnt / $this->inits['BOX_x_num']);
					$bposX = fmod($bCnt, $this->inits['BOX_x_num']);
					
					$apb=array();
					$apb[0]= $apr[0] + ($bposX * $this->inits['B_xlen']) + $CON_px;
					$apb[1]= $apr[1] + ($bposY * $this->inits['B_ylen']) + $CON_px;
					$apb[2]= $apb[0] + $this->inits['B_xlen'] - 1;
					$apb[3]= $apb[1] + $this->inits['B_ylen'] - 1;
					$this->_drawBox( $apb, 'B');
					// $this->_drawText($apb, 'B', $cnt[$i] ) ;
					// echo 'DEBUG:'. 'B:'. $cnt[$i].'-'.$bposX.':'.$bposY."\n"  ;
				
					$cnt[$i] = $cnt[$i] + 1;
				}
				$i = 1;
				$cnt[$i] = $save[$i];
				$cnt[$i] = $cnt[$i] + 1;
			}
			
			$i = 0;
			$cnt[$i] = $save[$i];
			$cnt[$i] = $cnt[$i] + 1;
		}
		 
		
		$this->entries = $pos-1;
	}
	
	
	
	/**
	 * fill one box with content-color
	 * @param string $boxFillKey : 'none', ['fill1'], 'fill2', 'reserve'
	 * @return 
	 */
	function oneBoxFill($s, $r, $b, $boxFillKey ) {
		
		
		switch ($boxFillKey) {
		    
		    case 'fill1':
		        $color  = $this->GRCO['green1'];
		        break;
		    case 'fill3':
		        $color  = $this->GRCO['green3'];
		        break;
		    case 'fill2':
		        $color  = $this->GRCO['green2'];
		        break;
		    case 'fill4':
		        $color  = $this->GRCO['green4'];
		        break;
			case 'half':
				$color  = $this->GRCO['green2'];
				break;
			case 'reserve':
				$color  = $this->GRCO['gray'];
				break;
			case 'none':
				// NO DRAW !!!
				return;
			default:
				$color  = $this->GRCO['green1'];
		}
		
		$inits = &$this->inits;
		$apx= $this->_getBoxCoord($s, $r, $b);
		
		imageFilledRectangle( $this->imgH, 
			$inits['CONT_xof_px'] +$apx[0], 
			$inits['CONT_yBof_px']-$apx[3], 
			$inits['CONT_xof_px'] +$apx[2],
			$inits['CONT_yBof_px']-$apx[1], 
			$color );
	}
	 
	// draw a red frame around a box
	function oneBoxFrame($s, $r, $b) {
		
		$color = $this->GRCO['red'];
		$inits = &$this->inits;
		$apx   = $this->_getBoxCoord($s, $r, $b);
		
		imageRectangle( $this->imgH, 
			$inits['CONT_xof_px'] +$apx[0], 
			$inits['CONT_yBof_px']-$apx[3], 
			$inits['CONT_xof_px'] +$apx[2],
			$inits['CONT_yBof_px']-$apx[1], 
			$color );
	}
	
	/**
	 * draw title line
	 * @return 
	 * @param object $text
	 * @param object $line[optional] 0,1,2
	 * @param object $colorKey[optional] 'red', 'black'
	 */
	function titleText($text, $line=0, $colorKey="") {
		$fontHeight=12;
		$font  = 3;
		if ($colorKey==NULL) $color = $this->GRCO['black'];
		else $color = $this->GRCO[$colorKey];
		
		$x = 10;
		$y = 5 + $line*$fontHeight; // $this->inits['img_y_size']*0.5;
		
		imagestring($this->imgH,  $font, $x,$y,$text, $color);
	}
	
	function errorImage( $textArr, $err_type=NULL ) {
		$this->ImgLib = new fgrImgSub2C();
		$this->ImgLib->errorImage( $textArr, $err_type );
	}
	
	function save($filename) {
		$this->ImgLib->imgOut($filename);
	}
	
	function streamOut($filename) {
		set_mime_type("image/png", $filename);
		readfile($filename);
	}
}
