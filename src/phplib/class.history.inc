<?php

/**
 * manage $_SESSION['s_history']
 * GLOBALS:  ["g.histlist"]["somode"]
 * NEW: 
	- new sort concept
	- leave $_SESSION['s_history'] order, only unset() and push() allowed !!!
	- so the INDEX of will NOT change during a session !!!
	- changing show-order will create a new temp_array sortord[ordnum] = histid
		- create sortord[] on show
	- on save at logout: create a new INDEX, starting at 0 !!!
 * FORMAT: - used to save history of BO, can also handle SINGLE_PK_OBJECTS
  	$_SESSION['s_history'] = array ( 
  	"TAB" => "ID", 
  	"last"=> 0,1,    : is last one of BO-type
	"c" => number : touch counter
	"o" => visit-order-num :  1 - now, 2 - before, 3 - beforebefore ...
	"s" => showorder - num:  1 - first, ... N - now (so show it reverse)
	"li"=> TBD: TABLE (object is linked to table; support for object tracking)
	);
 * @package class.history.inc
 * @swreq UREQ:G_S_HISTORY
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/class.history.inc 59 2018-11-21 09:04:09Z $
 */
class historyc {
  
    function __construct () {
    	$this->SHOW_OBJ_NUM = 25;
    }
    
    /**
     * add new object to history
     * put new entry to end of stack
     * @param string
     * @param int
     * @param array &$indarr index array
     * @param int $maxshow last "s"
     * @param array $opt ["parent"]
     * @return int $hist_pos
     * @global array $_SESSION['s_history'] IN,OUT
     */
    function _addObj($tablename, $id, &$indarr, $maxshow, $opt) {
    	
    	
    	$hist_pos   = 0;		// new entry
    	$newentry = array( $tablename => $id, 'last' => '1', 'c'=>1, 'o'=>1, "s"=>$maxshow+1 );
    	if ( $opt["parent"]!="" ) {
    	   $newentry["li"] = $opt["parent"];
    	}
    	
    	if ( !sizeof($_SESSION['s_history']) ) {
    		$hist_pos = 0;
    		$_SESSION['s_history'][$hist_pos] = $newentry;
    		return ($hist_pos);
    	}
    	   
    	$lastpos   = end($indarr);
    	$hist_pos = $lastpos+1;
    	$_SESSION['s_history'][$hist_pos] = $newentry;
    	
    	return ($hist_pos);
    }
    
    function last_bo_get( $tablename ) {
    	
    
    	if ( !sizeof($_SESSION['s_history']) ) return;
    	reset ($_SESSION['s_history']);
    	$tmp_obj_id=0;
    
    	/*search last used object */
    	$indarr   = array_keys( $_SESSION['s_history'] );
    	
    	
    	foreach($indarr as $hi) {
    		$valarr = $_SESSION['s_history'][$hi];
    		$tab_key1 = key($valarr);
    		$value1   = current($valarr);
    		$last     = next($valarr);
    		
    		if ( ($tab_key1 == $tablename) and $last ) {
    			$tmp_obj_id = $value1;
    			break;
    		}
    	}
    	return ($tmp_obj_id);
    }
    
    /**
     * get all BOs of type
     * @param string $tablename
     * @return void|number|mixed
     */
    function bos_get_type( $tablename ) {
        
        $obj_list = array();
        
        if ( empty($_SESSION['s_history']) ) return;
        // reset ($_SESSION['s_history']);
       
        $indarr   = array_keys( $_SESSION['s_history'] );
     
        foreach($indarr as $hi) {
            
            $valarr = $_SESSION['s_history'][$hi];
            $tab_key1 = key($valarr);
            $value1   = current($valarr);
            // $last     = next($valarr);
            
            if ( ($tab_key1 == $tablename) ) {
                $obj_list[] = $value1;
            }
        }
        return ($obj_list);
    }
    
    function getObjects() {
    	// FUNCTION: get all objects from history
    	// NEW:
    	
    	
    	if ( !sizeof($_SESSION['s_history']) ) return;
    	
    	$retarr=NULL;
    	foreach($_SESSION['s_history'] as  $th1 ) {
    	
    	  $tab_key1 = key($th1 );
    	  $value1   = current($th1 );
    	  $retarr[] = array($tab_key1, $value1);
    	}
    	
    	return ($retarr);
    }
    
    function guessOrder() {
    	// fUNCTION: guess value for order_info ["o"]
    	// OUTPUT: $_SESSION['s_history']
    	
    	
    	if ( !sizeof($_SESSION['s_history']) ) return ;
    	reset ($_SESSION['s_history']);
    	$orderval = 1;
    	$s_historytmp = &$_SESSION['s_history'];
    	$histlen = sizeof($_SESSION['s_history']);
    	$showval = $histlen;
    	foreach($s_historytmp as  $pos=>$th ) {
    	  	$_SESSION['s_history'][$pos]["o"] = $orderval;
    		$_SESSION['s_history'][$pos]["s"] = $showval;
    		$orderval++;
    		$showval--;
    	}
    	
    }
    
    /**
     * - check history for new OBJECT entry 
    				- update the Visit-order-value
     * @param string $tablename
     * @param int $id
     * @param array $opt
     *    "parent" : table	
     * @return array entry-array of object
     */
    function historycheck(	$tablename, 	$id, $opt = NULL ) {
    
    	if (empty($id)) return;
    	
    	// echo "DEBUGXY: tab: ".$tablename." id:". $id." ";
    	
    	$hist_pos  = -1;
    	$cnt       = 0;
    	$orderinfo = NULL;
    	$oneorder  = "";
    	$ORDER_LIMIT= 6;
    	$last_order = -1;  // this object had last time $last_order_val
    	
    	if (sizeof($_SESSION['s_history'])) { 
    	
          $boThisTmp = $_SESSION['s_sessVars']['boThis'];
          if ($boThisTmp["t"] == $tablename AND $boThisTmp["id"]==$id) {
    		// fast exit, because the object was touched before, no increment of 'c'
    		return ($_SESSION['s_history'][$boThisTmp["pos"]]); 
          }
    	  
    	  $indarr   = array_keys( $_SESSION['s_history'] );
    	  
    	  $maxshow = 0;
    	  foreach($indarr as $ti) {
    	  
    		$tarr =  $_SESSION['s_history'][$ti];
            $tab_key = key($tarr);
            $tmp_id  = current($tarr);
            
    		if ($tarr["s"]> $maxshow ) $maxshow = $tarr["s"];
            if ($tab_key == $tablename) {
    			
                if ( $tarr['last'] ) {
                	$_SESSION['s_history'][ $ti]['last'] = ''; // remove last_touch_flag
                }
                if  ($tmp_id == $id)  { 
                	$hist_pos =  $ti; // flag, that it was found
    				$last_order = $tarr["o"];
                }
            }
            $cnt++;
    	  }
    	  
    	}
    	
    	$oneorder= $tarr["o"];    // get one order-value
    	if ($last_order=="") $last_order = -1; // may be was empty
    	
    	// recalculate order flag ?
    	if ( $oneorder=="" ) $this->guessOrder();
    	
    	if ( $last_order!=1 and sizeof($_SESSION['s_history']) ) {
    		// update visit-values
    	    foreach($indarr as $pos) {
    		
    			$lastordTmp = $_SESSION['s_history'][$pos]["o"];
    			$neworder = $lastordTmp;
    			if (($last_order<0) OR ($lastordTmp<$last_order)) $neworder = $lastordTmp + 1;
    			$_SESSION['s_history'][$pos]["o"] = $neworder;
    		}
    		
    	}
    	
    	if ( $hist_pos < 0 ) { 	// not found in history ?
    	   $hist_pos = $this->_addObj($tablename, $id, $indarr, $maxshow, $opt);
    	} else {
    		// already found in history
    		
        	$_SESSION['s_history'][$hist_pos]['last'] = '1';
            $_SESSION['s_history'][$hist_pos]['c']    = $_SESSION['s_history'][$hist_pos]['c'] + 1; // increment touch counter
    		$_SESSION['s_history'][$hist_pos]['o']    = 1; 
        }
    	
    	
    	if ($tablename == 'PROJ')  $_SESSION['s_sessVars']['boProjThis'] = $id;
    	$_SESSION['s_sessVars']['boThis'] = array("t"=>$tablename, "id"=>$id, "pos"=>$hist_pos );
    	
    	return ( $_SESSION['s_history'][$hist_pos] );
    }
    
    /** resort history by visit-order
     *		- order by "o" visit-order 
     *		- change "s" show-order
     *		- do not change "c" and "o"
     */
    function resort( 
    	$lastsort,  	// >0 did a sort before
        $opt = NULL 	// "MAX_NUM_SAVE"
    	) {         
        
    	
    	
        
        if ( !sizeof($_SESSION['s_history']) ) return;
        
        $MAX_NUM_SAVE = 30;
        $desta  = NULL;		  // new HISTORY structure
    	$indarr = array_keys($_SESSION['s_history']);
    	
    	// if ($lastsort)            $MAX_NUM_SAVE = $MAX_NUM_SAVE - $lastsort*5;
        if (!$MAX_NUM_SAVE)       $MAX_NUM_SAVE = 0;
        if ($opt["MAX_NUM_SAVE"]) $MAX_NUM_SAVE = $opt["MAX_NUM_SAVE"];
    	$lastsort ++;
    	
    	// contains array[POS] = visit-order-value
    	$visit_order = NULL;
    	$firstid = current($indarr);
    	$tmparr =  $_SESSION['s_history'][$firstid]; // get one value
    	if (!$tmparr["o"])  $this->guessOrder(); // guess it 
    	
    	// get visit-order and POS
    	foreach($indarr as $cnt) {
            $visit_order[$cnt] = $_SESSION['s_history'][$cnt]["o"];
        }
    	asort ($visit_order); // sort visit-value, remain index
    	
    	// delete OLD history-entries, reorder "s"
    	$newpos  = 0;
    	$nowshow = $MAX_NUM_SAVE;
    	foreach($visit_order as $hi=>$dummy) {
    		
    		if ($newpos>=$MAX_NUM_SAVE) {
    			unset( $_SESSION['s_history'][$hi] ) ; // unset old values
    		} else {
    			 $_SESSION['s_history'][$hi]["s"]= $nowshow;
    			 $nowshow--;
    		}
    		$newpos++;
        }
    	
    	return ($lastsort);
    	
    }
    
    /**
     * resort history : by OBJECT TYPE, change "s"
     *	1. get latest objects (have "lastbo" in separate structure)
     *	2. sort by object-type
     *	 - leave histID, just return the orderArray and UNSET() old entries
     *	 - collect the new histID, unset() the rest
    */
    function resort2( 
    	$lastsort,  	// >0 did a sort before
        $opt = NULL 	// "MAX_NUM_SAVE" (1...5)
    	) {        
        
        
        
        
        if ( !sizeof($_SESSION['s_history']) ) return;
        
        $MAX_NUM_SAVE = 15;
    	$MAX_NUM_SEC  = 7;
        $desta  = NULL;		  // new HISTORY structure
    	
        $tabCnt = NULL;		  // remember tables
    	$lastTable = $_SESSION['s_sessVars']["boThis"]["t"];
    	
    	$showindarr  = $this->showByClick();
    	$indarr = array_keys($_SESSION['s_history']);
    	
    	$tabcnt = NULL;
    	// get latest objects (have "lastbo" in separate structure)
    	
    	foreach($showindarr as $cnt=>$dummy) {
        
    		$tmparr = &$_SESSION['s_history'][$cnt];
            $tab_key = key($tmparr);
            $desta[$tab_key][] = $cnt;
    		$tabcnt[$tab_key]++;
    
        }
    	
    	
        $lastsort++;
    	
    	if ($lastsort) {
    		// $biggestnumber= max($tabcnt);
    		$MAX_NUM_SAVE = $MAX_NUM_SEC-$lastsort;
    		// if ($MAX_NUM_SAVE>$MAX_NUM_SEC) $MAX_NUM_SAVE=$MAX_NUM_SEC;
    	}
    	if ($opt["MAX_NUM_SAVE"]) $MAX_NUM_SAVE = $opt["MAX_NUM_SAVE"];
    	if ($MAX_NUM_SAVE<=0) $MAX_NUM_SAVE = 1;
    	
        $newpos    = 0;
    	$visit_order = NULL;
    	$showorder = sizeof($showindarr);
    	
    	foreach($desta as $tab_key => $relpos_arr) {  // for all found tabs
    	
    		if (sizeof($relpos_arr)) {
    			$newcnt_perType = 0;
    			
    			foreach($relpos_arr as $hi) {
    			// foreach( $relpos_arr as $dummy=>$hi)  {
    				if ($newcnt_perType<$MAX_NUM_SAVE) {
    					$visit_order[$hi] = 1;
    					$_SESSION['s_history'][$hi]["o"] = ($newpos+1); 	// set new visit order
    					$_SESSION['s_history'][$hi]["s"] = $showorder;
    					$showorder--;
    					$newpos++;
    					$newcnt_perType++;
    				} else {
    					unset( $_SESSION['s_history'][$hi] ) ;
    				}
    			}
    		}
    	}
    	
    	
        if ( $_SESSION['userGlob']["g.debugLevel"]>2 ) {
            echo "DEBUG2: historyc::resort(): <pre>";print_r($_SESSION['s_history']); echo"</pre>";
        }
    
        return ($lastsort);
    }
    
    /**
     * get $visit_order for showing history
     */
    function showByClick() {
    	
    	
    	if($_SESSION['s_history']==NULL) return;
    	
        $visit_order=NULL;
    	$indarr   = array_keys( $_SESSION['s_history'] );
    	foreach($indarr as $histind) {
    	// foreach( $indarr as $dummy=>$histind) { // for all found tabs
    		$visit_order[$histind] = $_SESSION['s_history'][$histind]["o"];
    	}
    	
    	asort ($visit_order); // sort visit-value, remain index
        return ( $visit_order );
    
    }
    
    /**
     * get $showarr by "s" showorder
     */
    function showByPush() {
    	
    	
    	if($_SESSION['s_history']==NULL) return;
    	
        $visit_order=NULL;
    	$indarr   = array_keys( $_SESSION['s_history'] );
    	foreach($indarr as $histind) { // for all found tabs
    	// foreach( $indarr as $dummy=>$histind) { 
    		$visit_order[$histind] = $_SESSION['s_history'][$histind]["s"];
    	}
    	
    	
    	arsort ($visit_order); // sort visit-value, remain index
    	
        return ( $visit_order );
    
    }
    
    function shorter() {
    	
    	
        $SHORT_STEP = 5;
    	
        if ( !sizeof($_SESSION['s_history']) ) return;
    	$lenhist = sizeof($_SESSION['s_history']);
    	$lennew  = $lenhist - $SHORT_STEP;
    	
    	if ( $lennew>$this->SHOW_OBJ_NUM ) $lennew = $this->SHOW_OBJ_NUM;
    	
    	$soopt = array("MAX_NUM_SAVE"=>$lennew);
    	$this->resort( 0, $soopt); 	// "MAX_NUM_SAVE"
    	
    }
    
    static function  getLastProj() {
        return $_SESSION['s_sessVars']['boProjThis'];
    }

} // END class
