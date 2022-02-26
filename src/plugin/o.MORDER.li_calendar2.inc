<?php
require_once ('date_funcs.inc');
require_once ("visufuncs.inc");

require_once 'o.MORDER.subs.inc';
require_once ('object.info.inc');
require_once 'o.H_ALOG_ACT.subs.inc';
require_once ("gui/o.MORDER.guisub.inc");
require_once('o.MORDER.network.inc');
require_once 'f.sql_sel2.inc';


/**
 * LIST of MORDER > show calendar as GANTT chart
 * 
 * calendar:
 *  - BLUE:  service to to
 *  - GREEN: done
 * 
 * @package o.MORDER.li_calendar2.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param array parx
 *    'showType' : ['calendar'], 'list'
 *    'filt' : set MORDER Filter
 *       array('col'=>, 'val'=>, 'con' )
 *         e.g. 'col':'PROD_LINE_ID'
 *        
 *       
 * view_type  :
 *        monthly
 *        yearly
 *        weekly
 * year
   month
   day
 *        

 */
class o_MORDER_li_calendar2_XPL extends gPlugin {
    
    /**
     * 
     * @var array $timeStamps2 MOR_timestamp_STRUCT (see o.MORDER.subs.inc:MOR_timestamp_STRUCT)
     */
    private $timeStamps2;
    private $eventCalArr;
    private $dateUnxArr; // array(start, end) of shown period
    private $xgid; // group ID

	
	function register() {
		$tablename	= 'MORDER';
		$title		= 'Gantt Chart';
		

		$this->infoarr			   = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_cnt']  = 1;
		$this->infoarr['jsFile']   = array("res/gantt/jsgantt.js");
		$this->infoarr['cssFile']  = array("res/gantt/jsgantt.css?d=1");
		
		

		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array() )
				);	
	}
	
	function getTypeLink() {
		return $_SERVER['PHP_SELF'].'?mod='.$this->_mod.'&view_type='.$this->view_type.
			'&year='.$this->param['Y'].'&month='.$this->param['M'].'&day='.$this->param['D'].'&parx[showType]=';
	}
	
	
	
	function anaInit(&$sqlo) {
	    
	    $this->xgid=1;
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
		
		$param=array();
		$param['Y']   = $_REQUEST['year'];
		$param['M']   = $_REQUEST['month'];
		$param['D']   = $_REQUEST['day'];
		
		
		
		$this->allowAnalysis=1;
		$this->eventCalArr = array();
		if ($view_type!='monthly' and $view_type!='yearly' and $view_type!='weekly') {
			$this->allowAnalysis=0;
		}
		
		if ($this->allowAnalysis) {
		    
		    
			
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
			
			$this->calFeats['year']       = $param['Y'];
			$this->calFeats['month']      = $param['M'];
			$this->calFeats['month_name'] = $param['M'];
			

			$this->timeStamps2  = oMORDER_subs::getOfRangeMany($sqlo, $dateUnxArr, $deviceSubSql);

			$this->status_id2name = oH_ALOG_ACT_subs::getActionEntries($sqlo, 'MORDER');
		}	
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
	
	/**
	 * show $this->timeStamps2 entries
	 * @param object $sqlo
	 */
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
		$headx   = array ("#", "POR-ID", $this->table_nice, "Status", "Start-Date *", "End-Date", "Request-Date", "Plan start", "Plan End");
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
			    $oneEntryLoop['END_DATE_PLAN'],
				);
			
			$tabobj->table_row ($dataArr);
			$cnt++;
		}
		 
		
		$tabobj->table_close();
	}
	
	private function _lnk($mord_id) {
	    // the HTML attributes MUST be enclsed in single QUOTES due to GANNTT JSON
	    $tmp_link= "<a href='edit.tmpl.php?t=MORDER&id=".$mord_id."'>&gt;</a>"; 
	    return $tmp_link;
	}
	
	/**
	 * 
	 * @param object $sqlo
	 * @param int $mord_id
	 * @return array
	 */
	private function get_group($sqlo, $mord_id) {
	    
	    
	    $mord_features = objInfoC::getParamsRaw ( $sqlo, 'MORDER', $mord_id);
	    $event_name = $mord_features['vals']['NAME'];
	    
	    
	    $xgid = $this->xgid++;
	    
	    $event_arr = array(
	        'id'=>$mord_id,
	        'link'=>'',
	        'txt'=> $this->_lnk($mord_id).' '.$event_name,
	        'parent'=>0,
	        'complete' => 0,
	        'css' => 'ggroupblack',
	        'gid' => 0,
	        'grpFlag'=>1,
	        'suc_ids'=>'',
	        'xgid'=> $xgid  // make negative ID
	    );
	    
	    return $event_arr;
	}
	
	private function _create_grp($sqlo,$mid) {
    	$sub_event = $this->get_group($sqlo, $mid);
    	$grp_id    = $sub_event['xgid'];
    	$this->eventCalArr[$grp_id] = $sub_event;
    	
    	return $grp_id;
	}
	
	/**
	 * create $this->eventCalArr
	 * @param string $start_date
	 * @param string $end_date
	 * @param array $one_event_arr
	 * @param int $days_diff
	 * @return array
	 *    START_DATE_PLAN
	 *    END_DATE_PLAN
	 *    depend
	 */
	function _get_event($sqlo, $ori_event_arr) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $date_now_HUM = date_unix2datestr( time(), 3);
	    
	    $mord_id = $ori_event_arr['MORDER_ID'];
	    $mord_features = objInfoC::getParamsRaw ( $sqlo, 'MORDER', $mord_id);
	    $event_name = $mord_features['vals']['NAME'];
	    
	    // store more content
	    // $imgTmp  = $this->_get_icon_of_state($mord_features['access']['H_ALOG_ACT_ID']);
	    
	    //$tmp_link= '<a href="edit.tmpl.php?t=MORDER&id='.$mord_id.">LINK</a>";

	    $event_arr = array(
	        'id'=>$mord_id,
	        'link'=>'', 
	        'txt'=>$this->_lnk($mord_id).' '.$event_name, 
            'parent'=>0 ,
	        'gid'=>0,
	        'grpFlag'=>0,
	        'suc_ids'=>NULL
	    );
	    
	    $net_lib = new oMORDER_network($mord_id);
	    $parent_id  = $net_lib->get_parent($sqlo);
	    if ($parent_id) {
	        $event_arr['parent']=$parent_id;
	    }
	    
	    $children  = $net_lib->get_orders($sqlo);
	    if (!empty($children)) {
	        $event_arr['children'] = $children;
	        $event_arr['depend']=implode(',',$children);
	    }

	    $ord_lib = new oMORDER_subs($mord_id);
	    $suc_ids = $ord_lib->get_prod_SUCs($sqlo);
	    if (!empty($suc_ids)) {
	        $event_arr['suc_ids'] = 'MACs: '.implode(', ',$suc_ids);
	    }
	    
	    $start_date = $ori_event_arr['START_DATE'];
	    $end_date   = $ori_event_arr['END_DATE'];
	    
	    $event_arr['START_DATE']=$start_date;
	    $event_arr['END_DATE']  =$end_date;
	    
	    if ($ori_event_arr['START_DATE_PLAN']) {
	       $event_arr['START_DATE_PLAN']=$ori_event_arr['START_DATE_PLAN'];
	    } else {
	       $event_arr['START_DATE_PLAN'] = $start_date;
	    }
	    $event_arr['END_DATE_PLAN']  =$ori_event_arr['END_DATE_PLAN'];

	    
	    $event_arr['complete'] = 0;
	    $event_arr['css'] = 'gtaskblue';
	    
	    //
	    
	    if ($ori_event_arr['REQU_DATE']!=NULL) {
	        $event_arr['END_DATE_PLAN'] = $ori_event_arr['REQU_DATE'];
	    }
	    
	    if ($ori_event_arr['START_DATE']!=NULL) {
	        
	        if ( $ori_event_arr['END_DATE']!=NULL) {
	            $event_arr['complete'] = 100;
	            $event_arr['css'] = 'gtaskgreen';
	        } else {
	            // repair END_DATE
	            $event_arr['END_DATE'] = '';
	            $event_arr['complete'] = 50;
	            if ( $event_arr['END_DATE_PLAN'] == NULL) {
	                $event_arr['END_DATE_PLAN'] = $date_now_HUM;
	            }
	            
	            if ($event_arr['END_DATE']==NULL and $event_arr['END_DATE_PLAN']!=NULL) {
	                // if no REAL end and END_DATE_PLAN exists => set END for the calendar view
	                $event_arr['END_DATE'] = $event_arr['END_DATE_PLAN'];
	            }
	        }
	        
	    } else {
	        $event_arr['css'] = 'gtaskred';
	    }
	    
	    // print('-Event: '.print_r($event_arr,1))."<br>";
	    return $event_arr;
	    

	}
	
	/**
	 * service calendar
	 * @param $sqlo
	 */
	function calendar(&$sqlo) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
		$view_type= $this->view_type;
		
		if (!$this->allowAnalysis) {
		    echo 'INFO: analysis not allowed (351)<br>';
		    return;
		}
			
		if (!sizeof($this->timeStamps2)) {
		    echo '<br>'."\n";
		    echo 'INFO: no data to analyse (414)<br>';
		    return;
		}
 
		
		foreach($this->timeStamps2 as $oneEntryLoop) {
			
		    // echo "- ".print_r($oneEntryLoop,1)."<br>";
		    $mid = $oneEntryLoop['MORDER_ID'];
		    
// 		    if ($oneEntryLoop['START_DATE']=='' and $oneEntryLoop['REQU_DATE']=='') {
// 		        continue;
// 		    }

			if ($view_type == 'monthly' or $view_type == 'weekly' ) {
			    
			    if ($this->eventCalArr[$mid]) continue;  // already in ...
			    
			    $one_event = $this->_get_event($sqlo, $oneEntryLoop);
			    
			    $parent_id = $one_event['parent'];
			    if ( $parent_id and !$this->eventCalArr[$parent_id] ) {
			        // get data of parent ...
			        $parent_ord_lib = new oMORDER_subs($parent_id);
			        $raw_entry = $parent_ord_lib->get_all_features($sqlo);
			        $parent_event = $this->_get_event($sqlo, $raw_entry);
			        
			        $grp_id = $this->_create_grp($sqlo, $parent_id);
			        $parent_event['gid'] = $grp_id;
			        $one_event['gid']    = $grp_id;
			        
			        $this->eventCalArr[$parent_event['id']] = $parent_event;
			    }
			    
			    if ( !empty($one_event['children']) ) {
			        // get children ...
			        
			        $grp_id = $this->_create_grp($sqlo,$mid);
			        $one_event['gid'] = $grp_id;
			        
			        foreach($one_event['children'] as $loop_id) {
			            
			            if ($this->eventCalArr[$loop_id]) {
			                
			                $this->eventCalArr[$loop_id]['gid'] = $grp_id;
			                
			            } else {
			            
    			            $parent_ord_lib = new oMORDER_subs($loop_id);
        			        $raw_entry = $parent_ord_lib->get_all_features($sqlo);
        			        $sub_event = $this->_get_event($sqlo, $raw_entry);
        			        $sub_event['gid'] = $grp_id;
        			        $this->eventCalArr[$loop_id] = $sub_event;
			            }
			        }
			    }
			    
			    $this->eventCalArr[$one_event['id']] = $one_event;
			    
			} else {
			    
				//TBD: ...$this->eventCalArr[$event_date] = 1; // for yearly 
			}
		}

		
		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
		    debugOut('ANALYSED Events: ', $FUNCNAME, 1);
		    foreach($this->eventCalArr as $index => $row ) {
		        echo ' - ID:'.$index.' :'.print_r($row,1)."<br>";
		    }
		}
		
		// ---------------------------------------------------------------------------------
		
	    //TBD: JS, CSS
	    ?>
		<div style="position:relative" class="gantt" id="GanttChartDIV"></div>
		<script type="text/javascript">

		var g = new JSGantt.GanttChart(document.getElementById('GanttChartDIV'), 'day');

		// hide some columns
		g.setShowDur(0);
		g.setShowRes(1);
		g.setShowComp(0);
		g.setShowStartDate(0);
		g.setShowEndDate(0);
		
		<?php 
		
		foreach($this->eventCalArr as $index => $row ) {
		    
		    /**
		     *  pBarText: "ex. bar text",
		     */
    		echo 'g.AddTaskItemObject({
    		    pID: '.($index).',
    		    pName: "'. $row['txt'].'",
    		    pStart: "'.$row['START_DATE'].'",
    		    pEnd: "'.  $row['END_DATE'].  '",
    		    pPlanStart: "'.$row['START_DATE_PLAN'].'",
    		    pPlanEnd: "'.  $row['END_DATE_PLAN'].  '",
    		    pClass: "'.$row['css'].'",
    		    pLink: "'. $row['link'].'",
    		    pMile: 0,
    		    pRes: "'   .$row['suc_ids'].'",
    		    pComp: '.   $row['complete'].',
    		    pGroup: ' . $row['grpFlag'].',
    		    pParent: '. $row['gid'].',
    		    pOpen: 1,
    		    pDepend: "'.$row['depend'].'",
    		    pCaption: "",
    		    pCost: 0,
    		    pNotes: "",
    		    category: "X",
    		    sector: "Production"
    		}); 
            ';
		}
    		?>

    	g.Draw();
    		
		</script>
		<?php
		
	}
	
	private function _nav_link($p, $text) {
	    echo ' <a href="'.$this->_plug_get_URL().'&view_type='.$this->calFeats['view_type'].
	       '&year='.$p['Y'].'&month='.$p['M'].'&day='.$p['D'].'">'.$text.'</a> ';
	}
	
	function navigate() {

	    if ($this->calFeats['view_type']=='monthly') {
	        
	        $year  = $this->calFeats['year'];
	        $mo    = $this->calFeats['month'] - 1;
	        if ($mo<1) {
	            $mo=12;
	            $year=$year-1;
	            
	        }
	        $prev_d = array('Y'=>$year, 'M'=>$mo,'D'=>1);
	        
	        $year  = $this->calFeats['year'];
	        $mo    = $this->calFeats['month'] + 1;
	        if ($mo>12) {
	            $mo=1;
	            $year=$year+1;
	            
	        }
	        $next_d = array('Y'=>$year, 'M'=>$mo,'D'=>1);
	    }
	    $this->_nav_link($prev_d, '&laquo;&laquo;');
	    
	    echo $this->calFeats['month']." - ".$this->calFeats['year'] ;
	    	   
	    $this->_nav_link($next_d, '&raquo;&raquo;');
	   
	}
	


	function startMain() {
		global $error;
		// $FUNCNAME='startMain';
		
		// $this->objid  = $_REQUEST['id'];
		$this->parx   = $_REQUEST['parx'];
		$sqlo = &$this->_sqlo;
		
		?>
		<style type="text/css">
		.gtaskname div, .gtaskname span {
            font-size: 11px;
        }
		</style>
		<?php
		
		
		if ($this->parx['showType']==NULL) $this->parx['showType']='calendar';
		
		if ( is_array($this->parx['filt']) ) {
		   
		    $query_lib = new fSql_sel2('MORDER');
		    $query_lib->cleanCond();
		    $query_lib->set_filter($sqlo, $this->parx['filt']);
		    if ($error->Got(READONLY))  {
		        return;
		    }
		    $query_lib->queryRelase();
		    
		    // reload page 
		    $this->_html->pageForward($this->_plug_get_URL().'&parx[showType]='.$this->parx['showType'], 'reload');
		    return;
		}
		$sqlAfterNoOrd = $this->listLib->getSqlAfterNoOrder();
		$this->sqlAfterNoOrd = $sqlAfterNoOrd;
		
		gHtmlMisc::func_hist( $this->_mod, 'POR Gantt', $this->_plug_get_URL() );
		
		$ObjCnt=$this->listLib->getObjCnt();
		if ($ObjCnt>500) {
			$this->htmlFoot('ERROR', 'Too many objects selected. Max 500 allowed.');
		}
		
		$this->anaInit($sqlo);
		
		$this->navigate();
		echo " &nbsp; ";
		
		if ($this->parx['showType']=='list') {
			$this->_html->line_out(' [<a href="'.$this->getTypeLink() .'calendar">Show as calendar</a>]');
			$this->showList($sqlo);
		} else {
			$this->_html->line_out(' [<a href="'.$this->getTypeLink() .'list">Show as List</a>]');
			$this->calendar($sqlo);
		}
	}
	
}
