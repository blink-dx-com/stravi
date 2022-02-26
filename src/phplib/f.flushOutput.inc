<?php
/*MODULE:  f.flushOutput.inc
  DESCR:   manage flush of webserver output
  AUTHOR:  qbi
  INPUT:   
  VERSION: 0.1 - 20020904
*/

class fFlushOutputC {

function __construct( $period, $infochar ) {
	$this->showPeriod = $period;      // period of showing characters per alivePoint() , e.g. 100
	$this->infochar   = "*";    // flushed character
	$this->infoLineBreakPeriod  = 40;
	$this->showCharPerPoint 	= 0;
	$this->CharInLineCnt 	    = 0;
	$this->totalCnt   = 0;
	if ($infochar!="")  $this->infochar = $infochar;
}

function alivePoint()  {

	$this->showCharPerPoint++;
	if ( $this->showCharPerPoint >= $this->showPeriod ) {
        echo $this->infochar;
        while (@ob_end_flush()); // flush output
        $this->showCharPerPoint = 0;
		$this->CharInLineCnt++;
		if ($this->CharInLineCnt >= $this->infoLineBreakPeriod) {
			echo " <font color=gray>".($this->totalCnt+1)."</font><br>\n";
			$this->CharInLineCnt = 0;
		}
		
		
	}
	
	$this->totalCnt++;
}

}