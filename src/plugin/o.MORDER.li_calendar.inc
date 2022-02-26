<?php
require_once ('subs/obj.rea_log.edform.inc');
require_once ('date_funcs.inc');
require_once ("visufuncs.inc");
require_once ("gui/f.calendar.class.inc");
require_once 'o.MORDER.subs.inc';
require_once ('object.info.inc');
require_once 'o.H_ALOG_ACT.subs.inc';
require_once ("gui/o.MORDER.guisub.inc");
require_once 'f.sql_sel2.inc';

/**
 * LIST of MORDER > show calendar
 * 
 * calendar:
 *  - BLUE:  service to to
 *  - GREEN: done
 * 
 * @package o.MORDER.li_calendar.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param parx
 *    ['showType'] : ['calendar'], 'list'
 *    'filt' : table filter [OPTIONAL]
 * $_REQUEST['year'];
   $_REQUEST['month'];
   $_REQUEST['day'];

 */
class o_MORDER_li_calendar_XPL extends gPlugin {
    
    private $timeStamps2;
    private $eventCalArr;
    
    const VMODE_START_ONLY=1;
    const VMODE_ALL   =2; // start and end

	
	function register() {
		$tablename	= 'MORDER';
		$title		= 'Calendar';
		
		$calendarCss = Calendar::getCssFileName();
		
		$this->infoarr			   = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_cnt']  = 1;
		$this->infoarr['headIn']   = '<link href="'.$calendarCss.'" rel="stylesheet" type="text/css" />';
		// $this->infoarr['help_url'] = 'o.REA_LOG.html';
		

		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array() )
				);	
	}
	
	function getTypeLink() {
		return $_SERVER['PHP_SELF'].'?mod='.$this->_mod.'&view_type='.$this->view_type.
			'&year='.$this->param['Y'].'&month='.$this->param['M'].'&day='.$this->param['D'].'&parx[showType]=';
	}
	
	function anaInit(&$sqlo) {
	    
	    $this->table_nice = tablename_nice2('MORDER');
	    
		$deviceSubSql = 'select x.MORDER_ID from '.$this->sqlAfterNoOrd;
		
		$now = time();
		$nowTimeArr = getdate  ( $now );
		
		$this->calFeats=NULL;
		$this->calFeats['view_type'] = 'monthly';
		
		$this->calFeats['view_type'] = $_REQUEST['view_type'];
		if ($_REQUEST['view_type']==NULL) $this->calFeats['view_type'] = 'monthly';
		$view_type = $this->calFeats['view_type'];
		
		$this->view_type=$view_type;
		
		
		$this->allowAnalysis=1;
		$this->eventCalArr = NULL;
		if ($view_type!='monthly' and $view_type!='yearly' and $view_type!='weekly') {
			$this->allowAnalysis=0;
		}
		
		if ($this->allowAnalysis) {
		    
		    $param=array();
			$param['Y']   = $_REQUEST['year'];
			$param['M']   = $_REQUEST['month'];
			$param['D']   = $_REQUEST['day'];
			
			if ($param['Y']==NULL) $param['Y'] = $nowTimeArr['year'];
			if ($param['M']==NULL) $param['M'] = $nowTimeArr['mon'];
			if ($param['D']==NULL) $param['D'] = $nowTimeArr['mday'];
			
			$dayDiff = 365; // yearly
			
			if ($view_type == 'monthly') {
				$param['D']='01';
				$dayDiff = 31;
			}
			
			if ($view_type == 'yearly') {
				$param['D']='01';
				$param['M']='01';
			}
			
			if ($view_type == 'weekly') {
				$startCalUnx  = mktime  (0,0,0,$param['M'],$param['D'],$param['Y']);
				$dayOfWeek    = date('w',$startCalUnx);
				if (!$dayOfWeek)  $dayOfWeek=7; //sunday
				$diffDaysToMo = $dayOfWeek - 1;
				
				$startCalUnx = $startCalUnx - date_unixDay2Sec($diffDaysToMo);
				$starArr = getdate($startCalUnx);
				
				$param['D']  = $starArr['mday'];
				$param['M']  = $starArr['mon'];
				$param['Y']  = $starArr['year'];
				
				$dayDiff = 7; 
			}
			
			$param['D'] = str_pad( $param['D'], 2, "0", STR_PAD_LEFT );
			$param['M'] = str_pad( $param['M'], 2, "0", STR_PAD_LEFT );
			$param['Y'] = str_pad( $param['Y'], 2, "0", STR_PAD_LEFT );
			
			
			$startCalUnx = mktime  (0,0,0,$param['M'],$param['D'],$param['Y']);
			$endCalUnx   = $startCalUnx + date_unixDay2Sec($dayDiff);
			
			$dateUnxArr = array(
				$startCalUnx,
				$endCalUnx
			);
			
			$this->dateUnxArr = $dateUnxArr;
			$this->param = $param;

			$this->timeStamps2  = oMORDER_subs::getOfRangeMany($sqlo, $dateUnxArr, $deviceSubSql);
			
			$this->status_id2name = oH_ALOG_ACT_subs::getActionEntries($sqlo, 'MORDER');
		}	
	}
	
	private function _get_date_str($datex) {
	    $newDate = substr($datex,0,4) . substr($datex,5,2) . substr($datex,8,2);
	    return $newDate;
	}
	
	private function _other_day($date1, $date2) {
	    $date1_D = substr($date1,0,10);
	    $date2_D = substr($date2,0,10);
	    if ($date2_D!=$date1_D) {
	        
	        $date1_U = date_str2unix($date1_D, 5);
	        $date2_U = date_str2unix($date2_D, 5);
	        
	        $diff_sec  = $date2_U-$date1_U;
	        $days_diff = ceil($diff_sec / (3600*24));
	    } else {
	        $days_diff = 0;
	    }
	    return $days_diff;
	}
	
	private function _get_icon_of_state($status_id) {
	    
	    $imgTmp  = NULL;
	    if ($status_id!=NULL) {
	        $status_name = $this->status_id2name[$status_id];
	        $imgfile    = oMORDER_guisub::iconOfState($status_name);
	        if ($imgfile!=NULL) $imgTmp = '<img src="images/'.$imgfile.'" style="padding-right:3px;">';
	    }
	    return $imgTmp;
	}
	
	function showList(&$sqlo) {
		
		$view_type= $this->view_type;
		$dateUnxArr = $this->dateUnxArr;
		
		
		
		echo "Datum: " . date('Y-m-d', $dateUnxArr[0])." ... ".date('Y-m-d', $dateUnxArr[1]). " <br>";
		
		if (!$this->allowAnalysis) return;
		if (!sizeof($this->timeStamps2)) return;
		if ($view_type != 'monthly' and $view_type != 'weekly' ) {
			echo "Nur unterstützt für Monat oder Woche.<br>";
			return;
		}
		
		$objInfoLib = new objInfoC();
		
		$tabobj  = new visufuncs();
		$headOpt = array( "title" => "Event Liste; " );
		$headx   = array ("#", "POR-ID", $this->table_nice, "Status", "Start-Date *", "End-Date", "Request-Date", "Planned Start");
		$tabobj->table_head($headx,   $headOpt);

		
		$cnt=0;
		foreach($this->timeStamps2 as $oneEntryLoop) {
		
		    $mord_id   = $oneEntryLoop['MORDER_ID'];

		    $mord_features = $objInfoLib->getParamsRaw ( $sqlo, 'MORDER', $mord_id);
		    $mord_name = $mord_features['vals']['NAME'];
		    
		    $imgTmp = $this->_get_icon_of_state($mord_features['access']['H_ALOG_ACT_ID']);
			
		    
			$dataArr = array(
				$cnt+1,
				$mord_id,
			    '<a href="edit.tmpl.php?t=MORDER&id='.$mord_id.'">'.$mord_name.'</a>', 
				$imgTmp,
			    $oneEntryLoop['START_DATE'],
			    $oneEntryLoop['END_DATE'],
			    $oneEntryLoop['REQU_DATE'],
			    $oneEntryLoop['START_DATE_PLAN'],
				);
			
			$tabobj->table_row ($dataArr);
			$cnt++;
		}
		 
		
		$tabobj->table_close();
	}
	
	/**
	 * add events to the $this->eventCalArr
	 * @param string $date1
	 * @param string $date2
	 * @param array $one_event_arr
	 */
	private function add_events($date1, $date2, $one_event_arr ) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    
	    if ($date2==NULL) $date2=$date1; // fallback: date2 is empty
	    // start with next day
	    $date1_D = substr($date1,0,10);
	    $date2_D = substr($date2,0,10);
	    $date1_U = date_str2unix($date1_D, 5);
	    $date2_U = date_str2unix($date2_D, 5);

	    
	    
	    $diff_sec  = $date2_U-$date1_U;
	    $days_diff_exact = $diff_sec / (3600*24);
	    $days_diff = ceil($days_diff_exact);
	    if (!$days_diff) $days_diff=1;
	    
	    $date1_corr = substr($date1,0,10);
	    $now_date   = $date1_corr;
	    $extra_sec= 3600 +10; // extra seconds for the "SCHALTTAG" in November ...
	    
	    debugOut("(256) ($date1_D, $date2_D) days_diff:$days_diff days_diff_exact:$days_diff_exact", $FUNCNAME, 2);
	    
	    // loop through following days ...
	    
	    for ($i=0; $i<$days_diff; $i++) {
	        
	        $date_now_U1 = date_str2unix($now_date, 5);
	        $date_now_U = $date_now_U1 + date_unixDay2Sec(1) + $extra_sec; // next day

	       
	        $now_date  = date_unix2datestr( $date_now_U,3 );
	        $now_date  = substr($now_date,0,10);
	        
	        $event_date = $this->_get_date_str($now_date);
	        
	        // debugOut("(251) i:$i date_now_U1:$date_now_U1 date_now_U:$date_now_U now_date:$now_date event_date:$event_date", $FUNCNAME, 2);
	        debugOut("  - (272) now_date:$now_date one_event_arr:".print_r($one_event_arr,1), $FUNCNAME, 2);
	        
	        if (!is_array($this->eventCalArr[$event_date])) $this->eventCalArr[$event_date]=array();
	        $this->eventCalArr[$event_date][] = $one_event_arr;
	        
	    }
	    
	    
	}
	
	function _add_ev($start_date, $end_date, $one_event_arr, $days_diff) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $event_date = $this->_get_date_str($start_date);
	    
	    if (!is_array($this->eventCalArr[$event_date])) $this->eventCalArr[$event_date]=array();
	    $this->eventCalArr[$event_date][] = $one_event_arr;
	    
	    if ($days_diff and $this->visu_mode==self::VMODE_ALL) {
	        // produce only many entries on MODE VMODE_ALL
	        debugOut("(301) one_event_arr:  ($start_date, $end_date):".print_r($one_event_arr,1), $FUNCNAME, 2);
	        $this->add_events($start_date, $end_date, $one_event_arr);
	    }
	}
	
	private function _simple_ev_point( &$mord_features, $start_date, $icon_shot, $add_status=0) {
	    
	    $view_type= $this->view_type;
	    $mord_id = $mord_features['vals']['MORDER_ID'];
	    $mord_Name = $mord_features['vals']['NAME'];
	    $end_date   = NULL;
	    $event_date = $this->_get_date_str($start_date);
	    
	    if ($view_type == 'monthly' or $view_type == 'weekly' ) {
	        
	        $imgTmp   = '<img src="images/'.$icon_shot.'" style="padding-right:3px;">';
	        if ($add_status) $imgTmp  .= $this->_get_icon_of_state($mord_features['access']['H_ALOG_ACT_ID']); // add status
	        
	        $one_event_arr = array('urlpar'=>$mord_id, 'txt'=>$mord_Name, 'img'=>$imgTmp );
	        $this->_add_ev($start_date, $end_date, $one_event_arr, 0);
	        
	    } else {
	        $event_date = $this->_get_date_str($start_date);
	        $this->eventCalArr[$event_date] = 1; // for yearly
	    }
	}
	
	/**
	 * service calendar
	 * input: $this->timeStamps2
	 * @param $sqlo
	 */
	function calendar(&$sqlo) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
		$view_type= $this->view_type;
		
		if ($this->allowAnalysis) {
			
			if (sizeof($this->timeStamps2)) {
			
			    $objInfoLib = new objInfoC();
				
				foreach($this->timeStamps2 as $oneEntryLoop) {
					
				    // echo "- ".print_r($oneEntryLoop,1)."<br>";
				    
				    $days_diff=0;
				    // $mord_type='prod';
				    $event_date=NULL;
				    
				    $mord_id = $oneEntryLoop['MORDER_ID'];
				    $mord_features = $objInfoLib->getParamsRaw ( $sqlo, 'MORDER', $mord_id);
				    $devName = $mord_features['vals']['NAME'];
				    
				    if ($oneEntryLoop['REQU_DATE']!=NULL) {
				        // REQUEST_DATE
				        $start_date = $oneEntryLoop['REQU_DATE'];
				        $this->_simple_ev_point($mord_features, $start_date, 'i13_morder_U.png', 1);
				    }
				    
				    // is this a real planned event ?
				    if ($oneEntryLoop['START_DATE_PLAN']!='' and $oneEntryLoop['START_DATE']=='') {
				        $this->_simple_ev_point($mord_features, $oneEntryLoop['START_DATE_PLAN'], 'i13_morder_L.png', 1);
				        continue;
				    }
				    
				    if ($oneEntryLoop['START_DATE']=='') {
				        continue;
				    }
				    
			        $start_date = $oneEntryLoop['START_DATE'];
			        $end_date   = $oneEntryLoop['END_DATE'];
			        if ($end_date!=NULL) {
			            $days_diff = $this->_other_day($start_date, $end_date);
			        }
     
				    
					if ($view_type == 'monthly' or $view_type == 'weekly' ) {

						// store more content
					    $imgTmp  = $this->_get_icon_of_state($mord_features['access']['H_ALOG_ACT_ID']);

						$one_event_arr = array('urlpar'=>$mord_id, 'txt'=>$devName, 'img'=>$imgTmp );
						$this->_add_ev($start_date, $end_date, $one_event_arr, $days_diff);
							
						
						
					} else {
					    $event_date = $this->_get_date_str($start_date);
						$this->eventCalArr[$event_date] = 1; // for yearly 
					}
				}

			}
		}
		
		debugOut('Events: '.print_r($this->eventCalArr,1), $FUNCNAME, 2);
		
	    // create calendar object
	    $objCalendar = new Calendar('p.php?mod='.$this->_mod);
	    ## *** set calendar width and height
	    $objCalendar->SetCalendarDimensions("1000px", "500px");
	    ## *** set week day name length - "short" or "long"
	    $objCalendar->SetWeekDayNameLength("long");
	    ## *** set start day of week: from 1 (Sanday) to 7 (Saturday)
	    $objCalendar->SetWeekStartedDay("1");
	    ## *** set calendar caption
	    $objCalendar->SetCaption(tablename_nice2('MORDER')." calendar");
	
		// $this->eventCalArr=array('20110827'=>1, '20110825'=>1, '20110806'=>1);
	 	$objCalendar->setstoreEvent($this->eventCalArr);
	 	$objCalendar->setLinkBase('edit.tmpl.php?t=MORDER&id=');
	    ## +---------------------------------------------------------------------------+
	    ## | 3. Draw Calendar:                                                         | 
	    ## +---------------------------------------------------------------------------+
	    
	    $objCalendar->Show();
	    
	    echo "<br>Icons: ";
	    $states = oMORDER_guisub::get_states_info();
	    $states['User Requested']='i13_morder_U.png';
	    
	    foreach($states as $state=>$imgfile) {
	        echo '&nbsp;&nbsp;&nbsp;&nbsp;<img src="images/'.$imgfile.'" style="padding-right:3px;"> '.$state;
	    }
	    
	    $imgfile='i13_morder_L.png';
	    $state='Planned';
	    echo '&nbsp;&nbsp;&nbsp;&nbsp;<img src="images/'.$imgfile.'" style="padding-right:3px;"> '.$state;
	    echo "<br>";
	}
	

	function startMain() {
		global $error;
		// $FUNCNAME='startMain';
		
		// $this->objid  = $_REQUEST['id'];
		$this->parx   = $_REQUEST['parx'];
		$sqlo = &$this->_sqlo;
		
		$this->visu_mode=self::VMODE_START_ONLY;
		
		if ( is_array($this->parx['filt']) ) {
		    
		    $query_lib = new fSql_sel2('MORDER');
		    $query_lib->cleanCond();
		    $query_lib->set_filter($sqlo, $this->parx['filt']);
		    if ($error->Got(READONLY))  {
		        return;
		    }
		    $query_lib->queryRelase();
		    
		    // reload page
		    $this->_html->pageForward( $this->_plug_get_URL().'&parx[showType]='.$this->parx['showType'], 'reload');
		    return;
		}
		
		
		if ($this->parx['showType']==NULL) $this->parx['showType']='calendar';
		
		$sqlAfterNoOrd = $this->listLib->getSqlAfterNoOrder();
		$this->sqlAfterNoOrd = $sqlAfterNoOrd;
		
		gHtmlMisc::func_hist( $this->_mod, 'POR Calendar', $this->_plug_get_URL() );
		
		$ObjCnt=$this->listLib->getObjCnt();
		if ($ObjCnt>500) {
			$this->htmlFoot('ERROR', 'Too many objects selected. Max 500 allowed.');
		}
		
		$this->anaInit($sqlo);
		
		if ($this->parx['showType']=='list') {
			$this->_html->line_out(' [<a href="'.$this->getTypeLink() .'calendar">Show as calendar</a>]');
			$this->showList($sqlo);
		} else {
			$this->_html->line_out(' [<a href="'.$this->getTypeLink() .'list">Show as List</a>]');
			$this->calendar($sqlo);
		}
	}
	
}
