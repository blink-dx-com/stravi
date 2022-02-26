<?php
/**
 * class for $_SESSION['s_historyL'] (list history)
 * @namespace core::misc
 * @package f.s_historyL.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @global  $_SESSION['s_historyL'] = array(table)=>array('cnt')
 */
class historyList {
  
  function __construct () {
  }
  
  // reset array
  function reset() {
  	
  	$_SESSION['s_historyL']=array();
  }


  /**
   * check history for new OBJECT list entry
   * @param $cnt current selected objects
   * @return 0 : table was in history before
             1 : table is new or has updated info
   */
  function historycheck($tablename, $objcnt=NULL) {  
	
    
        
	if (empty($tablename)) return;
	
	$infoarr = array('cnt'=>$objcnt);
	
	$cnt      = 0;
	$tabFound = -1; // position of first entry of table in list (-1:no; >=0 position found)
	$tabIsNew = 0;
    
	if (sizeof($_SESSION['s_historyL'])) {
	    
	
    	foreach( $_SESSION['s_historyL'] as $tab_key=>$dummy ) {
            if ($tab_key == $tablename) {   
                $tabFound = $cnt;
                break;
            }
            $cnt++;
    	}
	
	
          if ( $tabFound < 0 ) { 	// not found
                $_SESSION['s_historyL'][$tablename] = $infoarr;
                $tabIsNew = 1;
          } else {
          	// new objcnt is different to old objcnt ?
          	if  ($_SESSION['s_historyL'][$tablename]['cnt']!=$objcnt) {
          		$_SESSION['s_historyL'][$tablename] = $infoarr;
                $tabIsNew = 1;
          	}
          }
    } else {
            $_SESSION['s_historyL'][$tablename] = $infoarr;    
            $tabIsNew = 1;
    }
    
    $_SESSION['s_sessVars']["boTable"] = $tablename;
    
    return ( $tabIsNew );
   }
}
