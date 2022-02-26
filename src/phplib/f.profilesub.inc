<?php
require_once ("o.proj.profile.inc");
require_once ('access_check.inc');
require_once ("class.history.inc");


/**
 * - sub function for user profile, 
   - save objects from history in profile dir
   - use $_SESSION['userGlob']["g.lastobj"] to save the history
  		= array( table => array( IDs ), ... )
   CHANGES: - from 09.01.2007 on, last objects will not saved in a project anymore, 
  			because it confuses the application all the time (now save it in the prefs)
 * @package f.profilesub.inc
 * @swreq UREQ:G_S_HISTORY
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/f.profilesub.inc 59 2018-11-21 09:04:09Z $
 */
class profileSubC{

	var $MAX_NUM_SAVE = 30; // max saved objects

function lastObjectsSave( &$sql ) {
	/* FUNCTION:- save objects from $_SESSION['s_history'] in profile dir
				- save the visit-order-flag too
       FORMAT: 
		h[0] -> 1.
		h[1] -> 2.
		h[2] -> 3.
		h[3]
		h[4]		
	*/
	
	
	
	if ( !sizeof($_SESSION['s_history']) ) return (-1);
	
	$histobj = new historyc();
	
	$optsort = array("MAX_NUM_SAVE"=>5);
	if ( count($_SESSION['s_history'])>25 ) $optsort["MAX_NUM_SAVE"] = 3; // so many objects, save less objects per type
	
	$histobj->resort2(0,$optsort); // do a resort by TYPE and shorten
	$sortidarr = $histobj->showByPush();

	$tempTabIds=NULL;
	$oldtabkey = "";
	
	$cntmax = count($_SESSION['s_history'])-1;
	if ($cntmax > $this->MAX_NUM_SAVE) $cntmax = $this->MAX_NUM_SAVE-1;
	
	// write history backwards to prefs
	$cnt = 0;
	$sumarr = NULL;
	foreach($sortidarr as $hi=>$dummy) {
	
		if ($cnt>$cntmax) {
			break;
		}
		$tmparr  = $_SESSION['s_history'][$hi];
		
		if (!sizeof($tmparr)) continue;

		$tab_key = key    ($tmparr);
		$value   = current($tmparr);
		
		if (!cct_access_has2($tab_key)) continue;  // no BO
		if ($tab_key=="CONCRETE_PROTO") continue;
		
		if (!is_array($sumarr[$tab_key]) ) $sumarr[$tab_key]=NULL;
		$sumarr[$tab_key][] = $value;

		$cnt++;
	}

	$_SESSION['userGlob']["g.lastobj"] = serialize($sumarr);
}

function getLastObject( &$sql ) {
    // FUNCTION:- get objects from $_SESSION['userGlob']["g.lastobj"]
	//			- get also last visit-order
	
	
	
	$hist_obj = new historyc();  
	
	// echo "TEST3:".$_SESSION["userGlob"]["g.debugLevel"]."|".$_SESSION['userGlob']["g.debugLevel"]."<br>";
	if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
		echo "DEBUG: profileSubC::getLastObject()<br>\n";
	}
	
	$sumSerial = $_SESSION['userGlob']["g.lastobj"];
	if ($sumSerial=="") return (-3);
	$sumarr = unserialize($sumSerial); 
	
	if (!sizeof($sumarr)) return (-4);
	
	// collect tables and IDs
    $cnt=0;
	$sortarr = NULL;
	foreach($sumarr as $tmptab=>$tabarr) {
		if ( !sizeof($tabarr) ) continue;
		foreach($tabarr as $tmpid) {
			if ($cnt > $this->MAX_NUM_SAVE) break;
			$sortarr[] = array($tmptab, $tmpid);
			$cnt++;
		}
		if ($cnt > $this->MAX_NUM_SAVE) break;
    }
	

	// go backwards
	end($sortarr);
	while ( $tmparr = current($sortarr) ) {
		$hist_obj->historycheck($tmparr[0], $tmparr[1]);
		if ( prev($sortarr) == FALSE ) break;
	}
	reset($sortarr);
	
	if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {		
		if (is_array($_SESSION['s_history'])) $info=sizeof($_SESSION['s_history']);
		echo "DEBUG: profileSubC::getLastObject() SIZE:".$info." <br>\n";
	}
	
    $hist_obj->resort(0);
    
}


}
