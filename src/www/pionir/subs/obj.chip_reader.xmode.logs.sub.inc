<?php
require_once('gui/o.CHIP_READER.statusView.inc');
require_once 'o.CHIP_READER.logs.inc';

/**
 * used later 
 * sub functions for obj.chip_reader.xmode.logs.inc
 * $Header: trunk/src/www/pionir/subs/obj.chip_reader.xmode.logs.sub.inc 59 2018-11-21 09:04:09Z $
 * @package    obj.chip_reader.xmode.logs.sub.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oCHIP_READER_xmode_logs_sub { 
	
	function __construct($objid) {
		$this->objid = $objid;
		$this->logsub_lib = new oREA_LOG_subs();
	}
	
	/**
	 * search all logs with DATE> STOP-Date
	 * letzter status: Service : _X_X_
	 * letzter Status: Qualifikation: _X_X
	 * @param object $sqlo
	 * @return array of INDEX => array(
	 *   'POS'=> $answer['POS'], 
	 *   'KEY'=>'Service', 
	 *   'a'  => 0,1 - is active ? 
	 *  );
	 *  array( 1,1,1,0,0)
	 */
	private function getLastLog_ID(&$sqlo, $mindate) {
		$DEF_STARTED = 3;
		$DEF_FINISHED= 4;
		$accept=3;
		$retarr=array();
		$retarr[1] = array('a'=>1); // identify, that STOP exists
		
		$answer = $this->logsub_lib->getLastKey_SUBLAST($sqlo, $this->objid , 'Service', $mindate); // start
		if ($answer['ACCEPT']==$DEF_STARTED) {
			$retarr[2] = array('a'=>1);
			$retarr[3] = array('a'=>0, 'POS'=>$answer['POS']);
			
		}
		if ($answer['ACCEPT']==$DEF_FINISHED) {
			$retarr[2] = array('a'=>1);
			$retarr[3] = array('a'=>1);
		}
		
		$answer = $this->logsub_lib->getLastKey_SUBLAST($sqlo, $this->objid , 'Qualifizierung', $mindate); // start
		if ($answer['ACCEPT']==$DEF_STARTED) {
			$retarr[4] = array('a'=>1);
			$retarr[5] = array('a'=>0, 'POS'=>$answer['POS']);
			
			
		}
		if ($answer['ACCEPT']==$DEF_FINISHED) {
			$retarr[4] = array('a'=>1);
			$retarr[5] = array('a'=>1);
		}
		
		// analyse active status
		

		return $retarr;
	}
	
	/**
	 * show "service log" path
	 * - show STOP > service start > service end > qualification start > qualification end > Working
	 * @param object $sqlo
	 * @param string $curr_ok_status : Working or STOP or unknown
	 */
	function _showActionPath(&$sqlo, $curr_ok_status) {
		
		$objid = $this->objid;
		/**
		 * @var $defpath_arr
		 *   'key' => name of service log key
		 *   'start'=> 1,2 
		 *      0 : need only start date, no extra text
		 *      1 : need start date, extra text: "start"
		 *      2 : need end   date, extra text: "finsih"
		 *   'showActLink'=> 0,1 : show active link anyway ?
		 */
		$defpath_arr=array(
				1=> array('key'=>'STOP',    'start'=>0), 
				2=> array('key'=>'Service', 'start'=>1, 'subtxt'=>'start'),
				3=> array('key'=>'Service', 'start'=>2, 'subtxt'=>'finish'),
				//4=> array('key'=>'Qualifizierung', 'start'=>1, 'subtxt'=>'start'),
				//5=> array('key'=>'Qualifizierung', 'start'=>2, 'subtxt'=>'finish'),
				4=> array('key'=>'Working',  'start'=>0, 'showActLink'=>1),
		);
		
		/**
		 * analyse: 
		 * - is $curr_ok_status=='Working' ? show blank path
		 * - is $curr_ok_status=='Stop'    ? analyse path
		 *   - what is the last status of "Quali" or "Service"; substatus != planned ?
		 * 
		 */
		$statusViewLib = new oCHIP_READER_statusView();
		
		//debugOut("curr_ok_status:$curr_ok_status", __FUNCTION__ );
		
		/**
		 * array of INDEX => array('a'=>1, 'POS'=>POS of service log);
		 */
		$log_status_arr = array();
		
		if ($curr_ok_status=='STOP') {
			$answerArrST    = $statusViewLib->_getLastKey($sqlo, $this->objid, 'STOP');
			$mindate        = $answerArrST[1];
			if ($mindate!=NULL) $log_status_arr = $this->getLastLog_ID($sqlo, $mindate);
			if ($log_status_arr[3]['a']) $log_status_arr[6]['showActLink']=1;
		}
		
		if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
			glob_printr( $log_status_arr, "DEBUG: array info: log_status_arr" );
		}
		
		$substr	 = NULL;
		$komma	 = NULL;
	
		$statusTableArr=array();
		
		
	
		$main_action_str='edit.tmpl.php?t=CHIP_READER&id='.$this->objid.'&tabArgu[KEY]=';
		$substr = '<table><tr valign=top>';
	
		$defaultLinkShow=1;
		/*
		if (!$this->canChangeStatus) {
			$defaultLinkShow=0;
		}
		*/
	
		$last_active=0;
		if (!sizeof($log_status_arr)) {
			$last_active=1; // device is working => show STOP-Link
		}
		
		foreach ($defpath_arr as $index => $subarray) {
				
			$this_key        = $subarray['key'];
			$statusLoopOut   = $this_key.' '.$subarray['subtxt'];
			$active_flag_arr = $log_status_arr[$index];
			
			if ($active_flag_arr['a']>0) {
				$last_active=1;
				$statusLoopOut = '<b>'.$statusLoopOut.'</b>'; // show bold text
			} else {
				if ( ($last_active>0 or $active_flag_arr['showActLink']) and $defaultLinkShow) {
					switch ($subarray['start']) {
						case 1:  
							$action_str=$main_action_str.$this_key.'&tabArgu[NAME]='.$this_key.'&tabArgu[ACCEPT]=3';
							
							break;
						case 2:
							if ($active_flag_arr['POS']>0) {
								$action_str='obj.chip_reader.logEdit.php?id='.$objid.'&pos='.$active_flag_arr['POS'].'&backPage=1';
							}
							
							break;
						default: $action_str=$main_action_str.$this_key.'&tabArgu[NAME]='.$this_key;
					}
					
					
					$statusLoopOut = '<a href="'.$action_str.'">'.$statusLoopOut.'</a>';
				}
				$last_active=0;
			}
				
		
				
			$substr  .= $komma . '<td>'.
					'<table class="xstPath"><tr><td>'.$statusLoopOut.'<br /><span style="color:#5050A0">'.
					$userName.'</span></td></tr></table></td>'."\n";
			$komma    ='<td>&gt;</td>';
			$substr  .= '</td>';
		}
		$substr  .= '</tr></table>'."\n";
		echo '<span class="yGgray">Statuspath:</span>', $substr;
	}
}