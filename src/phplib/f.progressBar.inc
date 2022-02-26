<?php
/**
 * show progress bar
 * see examples in 
 * $Header: trunk/src/phplib/f.progressBar.inc 59 2018-11-21 09:04:09Z $
 * @package f.progressBar.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @usage:
 * 
  $flushLib = new fProgressBar( 20000 );
  <html>
  <head>
	<style type="text/css">
	 echo $flushLib->getCss() ;
	</style>
	<script type="text/javascript">
	echo  $flushLib->getJS(); 
	</script>
    <title>SQL Hardcore</title>
  </head>
  
  $prgopt['objname']='rows';
  $prgopt['maxnum']= 1000;
  $flushLib->shoPgroBar($prgopt);
  { 
    $flushLib->alivePoint($cnt);
  }
  $flushLib->alivePoint($cnt,1); // finish
  
  ---------------
  OR pure ASCII
  ---------------
  $flushLib = new fProgressBar( 20000 );
  $prgopt['objname']='rows';
  $prgopt['maxnum']= 1000;
  $prgopt['mode']  = 'char';
  shoPgroBar( $prgopt ) 
  ...
    $flushLib->alivePointEasy($cnt);
  ...
  $flushLib->alivePointEasy($cnt,1); // finish
 */
 
class fProgressBar {
	
	var $startTimeUnx=0; // start time of progress
	var $vpart_mem;   /* array of velocity-parts
		array[] = array( last velocity )
	*/
	var $vpart_index;      // index of $vpart_mem
	var $vpart_indMax=10; // 10 entries
	var $vpart_tpo;   // time per object
	var $maxnum;
	
    /**
     * @param array $options : 
     *   'periodSec'
     *   'imgmaxlen' : only valid for static progressbar
     */
    function __construct( $maxnum=NULL, $options=NULL ) {
    	$this->maxnum = 100;
    	$this->charPerCnt=50;
    	if ($maxnum>0) $this->maxnum = $maxnum;
    	
    	$this->base_opts = $options;
    	
    	$this->periodSec = 3;
    	if ($options['periodSec']>0)  $this->periodSec = $options['periodSec'];
    	$this->totalCnt = 0;
    	$this->lastTimeUnx = 0;
    	$this->vpart_index=0;
    	$this->vpart_tpo   = 0.0;
    	$this->vpart_lastCnt=0;
    }
    
    /**
     * do best calculation of rest time estimation
     * - manage vpart_mem (velocityPart-memory)
     * @return $timeRestSec
     */
    private function _calc_timeRest($nowtime, $timediff, $num) {
        $indNow = $this->vpart_index;      // index of $vpart_mem
        // $vpart_indMax=
        
        if ($this->vpart_tpo>0.0) {
            $timePerObj  = $this->vpart_tpo; // the best value
        } else {
            $timePerObj  = ($nowtime - $this->startTimeUnx) / $num;  // default
        }
        
        if ( $indNow==$this->vpart_indMax ) {
            // only after X loops
            $velsum=0;
            for ( $i=0; $i<$this->vpart_indMax; $i++) {
                $velsum += $this->vpart_mem[$i];
            }
            // echo "DEBUG: _calc_timeRest".print_r($this->vpart_mem,1)." vesum:$velsum<br>";
            $timePerObj = $velsum / $this->vpart_indMax;
            $this->vpart_tpo  = $timePerObj;
            $this->vpart_index = 0; // reset index!!!
        }
        
        $lastCnt = $this->vpart_lastCnt;
        $numDiff = $num - $lastCnt;
        if ($numDiff>0) { // remind Division by Zero !
            $this->vpart_mem[$this->vpart_index] = $timediff / $numDiff;
            $this->vpart_lastCnt=$num;
            $this->vpart_index++;
        }
        
        $restCnt = $this->maxnum - $num;
        $timeRestSec = round($restCnt * $timePerObj);
        
        return $timeRestSec;
    }
    	
    function getCss($abspath=0) {
    	
    	
    	$addurl=NULL;
    	if ($abspath) {
    		$addurl = $_SESSION['s_sessVars']["loginURL"].'/';
    	}
    	return ('
    .progressBar{
    	width:402px;
    	height:14px;
    	background:url('.$addurl.'images/ic.progBar0.gif) 0 0; 
    	position:relative;
    }
    .progressBar span{
    	position:absolute;
    	display:block;
    	width:1px;
    	height:12px;
    	background:url('.$addurl.'images/ic.progBarC.gif) 0 0;
    	top:1px;
    	left:1px;
    	overflow:hidden;
    }
    
    .progressBar em{
    	position:absolute;
    	display:block;
    	width:400px;
    	height:12px;
    	background:url('.$addurl.'images/ic.progBar3.gif) 0 0;
    	top:1px;
    	left:1px;
    	overflow:hidden;
    }
    ');
    	
    }
    
    function getJS() {
    
    	return ( '
    	var counter = 0;
    	var prgAutoObj = null;
    	
    	function prgBarSet(num, info, timeRest){
    		var tagBase  = "prgBar";
    		var imgmaxlen= 400;
    		var fact = 0.01 * imgmaxlen;
    		
    		if (num>100) num=100;
    		imglen = num * fact;
    		if (num>=0) {
    			document.getElementById(tagBase).style.width  = imglen + "px";
    			spaceLen = imgmaxlen - imglen;
    			if (num>=100) { // prevent gray rest on 100%
    				spaceLen = 0;
    			}
    			spaceLeft = imglen + 1;
    			document.getElementById(tagBase+"Em").style.left  = spaceLeft + "px";
    			document.getElementById(tagBase+"Em").style.width = spaceLen + "px";
    			
    		}
    		timeStr = null;
    		if (timeRest!=null) {
    			timeStr=timeRest;
    		}
    		
    		document.getElementById(tagBase+"Txt").innerHTML= num + "%";
    		document.getElementById(tagBase+"Txt2").innerHTML= info;
    		document.getElementById(tagBase+"Txt3").innerHTML= timeStr;
    		counter=num;
    		if (num >= 100)
    		{
    			counter=0;
    		}
    	}
    			
    	
    	function prgAutoProgress() {
    		prgBarSet(counter, null, null);
    		counter = counter + 1;
    	}
    	
    	function prgActivateAuto(active, delay) {
    		if (active==1) {
    			prgAutoObj = setInterval("prgAutoProgress()", delay*1000);
    		} else  {
    			if (prgAutoObj) {
    				clearInterval(prgAutoObj);
    			}
    		}
    	}
    		
    	');
    			
    }
    
    function getCssStatic() {
        return '
.prg_meter { 
	height: 15px;  /* Can be anything */
	position: relative;
	background: #555;
	border-radius: 18px;
	padding: 5px;
	box-shadow: inset 0 -1px 1px rgba(255,255,255,0.3);
}

.prg_meter > span {
      display: block;
      height: 100%;
      border-top-right-radius: 4px;
      border-bottom-right-radius: 4px;
      border-top-left-radius: 10px;
      border-bottom-left-radius: 10px;
      background-color: rgb(43,194,83);
      background-image: linear-gradient(
        center bottom,
        rgb(43,194,83) 37%,
        rgb(84,240,84) 69%
      );
      box-shadow: 
        inset 0 2px 4px  rgba(255,255,255,0.3),
        inset 0 -2px 3px rgba(0,0,0,0.4);
      position: relative;
      overflow: hidden;
    }
';
    }
    
    
    
    /**
     * - start progress
     * @param array  $option 
     *   'maxnum' e.g. 1000
     *   'mode' = ['bar'], 'char' -- type of progress
     *   'charPerCnt' chars per count (for 'mode' = 'char')
     *   'objname' : e.g. "rows"
     */
    function shoPgroBar( $option=NULL ) {
    	$tagBase = 'prgBar';
    	$objname = '&nbsp;';
    	if ($option['graphName']!=NULL) {
    		// ???
    	}
    	
    	$this->saveopt = $option;
    	if ($option['maxnum']>0) $this->maxnum = $option['maxnum'];
    	if ($option['charPerCnt']>0) $this->charPerCnt=$option['charPerCnt'];
    	
    	$this->startTimeUnx = time();
    	
    	ob_implicit_flush(TRUE); // switch BUFFERING OFF !!!
    	if ( $option['objname'] != '' ) $objname = 	'&nbsp;'.$option['objname'].':&nbsp;';
    	$this->charPerLineCnt=0;
    
    	
    	if ( $option['mode'] == 'char' ) {
    		return;
    		// no further actions ...
    	} 
    	
    ?>
    <table border=0 cellpadding=0 cellspacing=0><tr>
    <td NOWRAP><?echo $objname?>
    <span id="<?echo $tagBase?>Txt2"></span>
    &nbsp;&nbsp;remaining time:&nbsp;
    <span id="<?echo $tagBase?>Txt3"></span>
    </td>
    </tr><tr>
    <td width=405>
    <p class="progressBar">
    		<span id="<?echo $tagBase?>"></span><em id="<?echo $tagBase?>Em"></span>
    </p>
    </td>
    <td NOWRAP>&nbsp;<span id="<?echo $tagBase?>Txt"></span>&nbsp;</td>
    </tr></table>
    <?
    }
    
    /**
     * get static bar string
     * - needs getCssStatic()
     * @param int $num
     * @param array $option
     *   'width' : in pixels
     */
    function getBarStatic( $num,  $option=array()) {
        
        $this->saveopt = $option;
        if ($option['maxnum']>0) $this->maxnum = $option['maxnum'];
        
        if (!$this->maxnum) $this->maxnum=1;
        $percent = floor($num * 100 / $this->maxnum);
        if ($percent>100) $percent=100;
        $html_add='';
        if ($this->base_opts['imgmaxlen']) {
            $html_add=' style="width:'.$this->base_opts['imgmaxlen'].'px;"';
        }
        
        $out = '<div class="prg_meter"'.$html_add.'><span style="width: '.$percent.'%"></span></div>';
        return $out;
    }
    
    /**
     * get static bar with DIGITS
     * - needs getCssStatic()
     * @param array $digits
     *    array('c'=>color)
     * @param array $option
     *   'imgmaxlen' : in pixels
     *   'img_height' : in pixels
     */
    static function getBarStaticDigits( $digits,  $option=array()) {

        $bg_color = '#555555';
        $border_sum = 2;
        $num_elems = sizeof($digits);
        
        if (!$option['img_height']) {
            $option['img_height']=20;
        }
        if (!$option['imgmaxlen']) {
            $option['imgmaxlen']=300;
        }
        
        $imgmaxlen=$option['imgmaxlen'];

        if (empty($digits)) {
            $svg='<svg xmlns="http://www.w3.org/2000/svg" width="'.$imgmaxlen.'" height="'.$option['img_height'].'" viewBox="0 0 '.$imgmaxlen
            .' '.$option['img_height'].'" >'."\n";
            $svg.='<rect x="0" y="0" rx="2" ry="2" width="'.$imgmaxlen.'" height="'.$option['img_height'].'" style="fill:'.$bg_color.';" />'."\n";
        } else {
            
            $digits_all_len = $imgmaxlen  - $border_sum;
            $r_diff = intval($digits_all_len/$num_elems);
            
            // correct imglen ...
            $imgmaxlen = $r_diff*$num_elems + $border_sum;
            
            
            $svg='<svg xmlns="http://www.w3.org/2000/svg" width="'.$imgmaxlen.'" height="'.$option['img_height'].'" viewBox="0 0 '.$imgmaxlen
            .' '.$option['img_height'].'" >'."\n";
            $svg.='<rect x="0" y="0" rx="2" ry="2" width="'.$imgmaxlen.'" height="'.$option['img_height'].'" style="fill:'.$bg_color.';" />'."\n";
        
            

            $x_digit_space=2;
            $x_off=1;
            $y_off=1;
            $ry = $option['img_height'] - $border_sum;
            foreach($digits as $i=>$row) { 
                if ($row['c']) {
                    $x = intval($x_off + $i*$r_diff);
                    $svg.='<rect x="'.$x.'" y="'.$y_off.'" rx="2" ry="2" width="'.($r_diff-$x_digit_space).'" height="'.$ry.'" style="fill:'.$row['c'].';opacity:0.9" />'."\n";
                }
            }
        }

        $svg.='</svg>';
        
        return $svg;
    }
    
    /**
     * reuse the progressbar , but with other limit 'maxnum'
     * - reset the progress bar
     * @param array $option
     */
    function setNewLimits($option) {
    	if ($option['maxnum']>0)     $this->maxnum = $option['maxnum'];
    	if ($option['charPerCnt']>0) $this->charPerCnt=$option['charPerCnt'];
    	$this->charPerLineCnt=0;
    	$this->totalCnt    = 0;
    	$this->lastTimeUnx = 0;
    	$this->vpart_index = 0;
    	$this->vpart_tpo   = 0.0;
    	$this->vpart_lastCnt=0;
    }
    
    
    
    function alivePoint($num, $force=0)  {
    	$lasttime = $this->lastTimeUnx;
    	$nowtime  = time();
    	$timediff = $nowtime - $lasttime;
    	
    	if ( ($timediff > $this->periodSec) OR $force) {
    
    		if ($this->saveopt['mode']=='char') {
    			$MAX_PER_LINE = 50;
    			echo '*';
    			if ($this->charPerLineCnt>$MAX_PER_LINE) {
    				echo "<br />\n";
    				$this->charPerLineCnt=0;
    			}
    			$this->charPerLineCnt++;
    			
    		} else {
    			$timeRestStr = NULL;
    	        $percent = intval($num / $this->maxnum * 100.0);
    	        // $restCnt = $this->maxnum - $num;
    	        
    	        if ( $num>0 ) { // analyse elapsed time
    		        // $timePerObj  = ($nowtime - $this->startTimeUnx) / $num;
    		        // $timeRestSec = round($restCnt * $timePerObj);
    		        $timeRestSec = $this->_calc_timeRest($nowtime, $timediff, $num);
    		        if ($timeRestSec>60) {
    		        	$timeRestStr = round($timeRestSec / 60) .' min';
    		        } else $timeRestStr = $timeRestSec .' sec';
    		        
    	        }
    			echo '<script language="JavaScript"> prgBarSet('.$percent.', '.$num.', "'.$timeRestStr.'"); </script>'."\n";
    			
    		}
    		
    		$this->lastTimeUnx = time();
    		while (@ob_end_flush());
    	}
    	
    	$this->totalCnt++;
    }
    
    /**
     * start alive points automatic; increment $num every; $inc_time seconds
     * @param int $active; 1: start, 2: stop
     * @param float $inc_time increment every $inc_time seconds
     */
    function alivePointAuto($active, $inc_time=1.0) {
    	if ($active==1) {
    	?>
    		<script language="JavaScript">prgActivateAuto(1, <?php echo $inc_time;?>);   </script>
    	<?php
    	} else {
    		// stop auto point
    	?>
    		<script language="JavaScript">prgActivateAuto(2, 0); </script>
    		<?php
    	}
    }
    
    
    /**
     * - show pure ASCII-char progress (a PLUS-sign)
     * - ecah start represents a time-slice ! 
     * @param $num
     * @param $force
     */
    function alivePointEasy($num, $force=0)  {
    	$lasttime = $this->lastTimeUnx;
    	$nowtime  = time();
    	$timediff = $nowtime - $lasttime;
    	$MAX_PER_LINE = 50;
    	
    	if ( ($timediff > $this->periodSec) OR $force) {
            echo '*';
    		
    		if ($this->charPerLineCnt>$MAX_PER_LINE) {
    			echo "<br />\n";
    			$this->charPerLineCnt=0;
    		}
    		$this->charPerLineCnt++;
    		
    		$this->lastTimeUnx = time();
    		while (@ob_end_flush());
    	}
    	
    	
    	$this->totalCnt++;
    }
	
}
