<?PHP

/**
 * easy graph for html tables
 * @package f.htmlGraph2.inc
 * @swreq   UREQ:GLOBAL
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/gui/f.htmlGraph2.inc 59 2018-11-21 09:04:09Z $
 */
class fHtmlGraph2C {
	
	/**
	 * 
	 * @param $maxval
	 * @param $MAX_POINTS
	 * @param $options:
	 * 		'colorName' : CSS color name
	 * 			blue : blue 
	 * 			t    : transparent
	 * 		'stripHeight' int
	 * 		'imgUrlPre' : leading image-url path (with SEPARATOR)
	 */
	function __construct($maxval, $MAX_POINTS, $options=NULL) {
		$this->MAX_POINTS  = $MAX_POINTS;
		$this->maxVal      = $maxval;
		$this->colorName   = 'blue';
		$this->stripHeight = 5;
		$this->imgUrlPre   = NULL;
		
		if ($options['stripHeight']>0)   $this->stripHeight = $options['stripHeight'];
		if ($options['colorName']!=NULL) $this->colorName = $options['colorName'];
		if ($options['imgUrlPre']!=NULL) $this->imgUrlPre = $options['imgUrlPre'];
		
	}
	function _img_get_width ( $obj_cnt ) {
		if ($this->maxVal<=0) return 0;
		$pixcnt = floor($obj_cnt * $this->MAX_POINTS / $this->maxVal );
		if (!$pixcnt and $obj_cnt>0) $pixcnt = 1; // show a minimum stripe, if a count is set
		return $pixcnt;
	}
	
	/**
	 * get image tag
	 * @param float $value
	 * @param string $color
	 * @param boolean $withFillImg ; 1: show a gray filling graph for $this->maxVal 
	 * @param int $cutmax : 0,1 - cut image length, if max val reached?
	 * @return string $imageTag
	 */
	function getImgHtml($value, $color=NULL, $withFillImg=0, $cutmax=0) {
		
		$colorarr = array (
			'violett'=>'DarkMagenta',
			'blue_li'=>'#B0B0FF',
			't'=>'#FFFFFF'
		);
		
		if ($color==NULL) $color = $this->colorName;
		if ($colorarr[$color]!=NULL) $color = $colorarr[$color];
		
		if ($color=='violett') $color='DarkMagenta';
		
		if ($cutmax) {
			// cut value!
			if ($value>$this->maxVal) $value = $this->maxVal;
		}
		
		$img_width_Full = $this->_img_get_width ( $this->maxVal );
		$img_width1 = $this->_img_get_width ( $value );
		//$graphHtml = '<img src="'.$this->imgUrlPre.'images/point_'.$color.'.gif" height="'.$this->stripHeight.'" width="'.$img_width.'">';
		$graphHtml = '<div style="background-color:'.$color.'; height:'.$this->stripHeight.'px; width:'.$img_width1.'px;"></div>';
		
		if ($withFillImg) {
			$graphHtml = '<div style="background-color:'.$color.'; height:'.$this->stripHeight.'px; width:'.$img_width_Full.'px;">';
			$graphHtml .= '<div style="float: left; width:'.$img_width1.'px; height:'.$this->stripHeight.'px;"></div>';
			//$img_width2 = $this->_img_get_width ( $this->maxVal - $value );
			$graphHtml .= '<div style="margin-left: '.$img_width1.'px; background-color:#D0D0D0; height:'.$this->stripHeight.'px;"></div>';
			//$graphHtml .= '<div style="background-color:#D0D0D0; height:'.$this->stripHeight.'px; width:'.$img_width2.'px;"></div>';
			//$graphHtml .= '<img src="'.$this->imgUrlPre.'images/point_gray.gif" height="'.$this->stripHeight.'" width="'.$img_width.'">';
			$graphHtml .='</div>'."\n";
		}
		
		return $graphHtml;
	}
	
	/**
	 * get image tag
	 * @param $box_array
	 *  'val'=>float  $value
	 *  'color'=>string $color
	
	
	 * @return string $imageTag
	 */
	function many_boxes( $box_array ) {
	    
	   
	    $img_width_Full = $this->_img_get_width ( $this->maxVal );
	    
	    $graphHtml = '<div style="display:inline-block; height:'.$this->stripHeight.'px; width:'.$img_width_Full.'px;">'."\n";
	    foreach($box_array as $row) {
	        
	        $tmp_width = $this->_img_get_width ( $row['val'] );
	       
	        $element = '  <div style="display:block;width:'.$tmp_width.'px;float:left; background:'.$row['color'].';">&nbsp;</div>'."\n";
	        $graphHtml .= $element;
	    }

	    $graphHtml .= '</div>'."\n";
	    
	    return $graphHtml;
	}
}
