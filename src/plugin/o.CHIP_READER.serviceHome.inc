<?php
require_once ('subs/obj.rea_log.edform.inc');
require_once ('date_funcs.inc');
require_once ("gui/f.calendar.class.inc");
require_once ("gui/o.REA_LOG.guisub.inc");
require_once ("o.REA_LOG.servAna.inc");
require_once('f.rider.inc');

/**
 * device > service home
 * 
 * calendar:
 *  - BLUE:  service to to
 *  - GREEN: done
 * 
 * @package o.CHIP_READER.serviceHome.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq Task:0001458: g > Gerätewartung (Concept)
 * @global $_SESSION['s_formState']['o.CHIP_READER.serviceHome'] = array(
	'showtype'] = ['calendar'], 'overview'
	);
 * @param $_REQUEST['id'] id of device
 * 		$_REQUEST['showtype'] = ['calendar'], 'overview'
 * for calendar
		$_REQUEST['year'];
		$_REQUEST['month'];
		$_REQUEST['day'];
		$_REQUEST['view_type']
 */
class o_CHIP_READER_serviceHome_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'CHIP_READER';
		$title		= 'Service home';
		
		$calendarCss = Calendar::getCssFileName();
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		$this->infoarr['headIn']   = '<link href="'.$calendarCss.'" rel="stylesheet" type="text/css" />';
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array() )
				);	
	}
	
	/**
	 * show next open service-dates
	 * a) abstract_plan == NULL
	 * b) all distinct open plans
	 * @swreq UREQ:0001458:SUBREQ:008	Fuer ein Gerät Schnelluebersicht naechste offene Termine
	 */
	function showNextDates(&$sqlo) {
	
		$this->_html->line_out('');
		
		$refDateUnx = time() +  date_unixDay2Sec(1); // one day tolerance
	
		$devid      = $this->objid;
		$ACCEPT_DONE= 4;
		$refDateHuman = date_unix2datestr(  $refDateUnx,  3 );
		//$devinfo = NULL;
	
		$baseSqlsel   = "from REA_LOG x where" .
			" KEY='Service' and ACCEPT<". $ACCEPT_DONE . 
			" and x.CHIP_READER_ID=" . $devid;
		
		/**
		 * get next distinct entries, which are ACCEPT<4
		 */
		$sqlsel="distinct(DEVSIC_ID)" . $baseSqlsel; 
		$sqlo->Quesel($sqlsel);
		$devsicArr=NULL;
		while ( $sqlo->ReadRow() ) {
			$DEVSIC_ID   = $sqlo->RowData[0];
			$devsicArr[] = $DEVSIC_ID;
		}
		
		if (!is_array($devsicArr) ) {
			$this->_html->line_out('No planned services.');
			return;
		}
		
		// get next entries
		reset ($devsicArr);
		$nextServices=NULL;
		foreach( $devsicArr as $DEVSIC_ID) {
		
			// mind NULL-values for DEVSIC_ID
			if ($DEVSIC_ID==NULL) $devSicSql= 'is NULL';
			else $devSicSql= '='.$DEVSIC_ID;
			$sqlsel="x.* " . $baseSqlsel. " and x.DEVSIC_ID ".$devSicSql." order by x.XDATE"; 
			$sqlo->Quesel($sqlsel);
			// get only first entry
			$sqlo->ReadArray();
			$pos   = $sqlo->RowData['POS'];
			$name  = $sqlo->RowData['NAME'];
			$accept= $sqlo->RowData['ACCEPT'];
			$datex = substr($sqlo->RowData['XDATE'],0,10);
			
			$nextServices[]=array('ds'=>$DEVSIC_ID, 'p'=>$pos, 'n'=>$name, 'a'=>$accept, 'd'=>$datex );
		}
		
		
		//  visualize
		$headx = array ('#','Service name', 'Date', 'Substatus','Info');
		$headOpt = array ("title"=>'Next planned services');
		$this ->_html->_table3_init( $headx , $headOpt );
		
		foreach( $nextServices as $tmparr) {
			$infoStr = '';
			if ($refDateHuman > $tmparr['d']) {
				$infoStr = '<img src="images/i40.warnTime.gif">';
			}
		
			$dataArr = array(
				'<a href="obj.chip_reader.logEdit.php?id='.$devid.'&backPage=1&pos='.$tmparr['p'].'">==&gt;</a>',
				$tmparr['n'], 
				$tmparr['d'], 
				$tmparr['a'],
				$infoStr
				);
			$this ->_html->_table3_row ( $dataArr );
		}
		$this->_html->_table3_close ();
	}
	
	/**
	 * service calendar
	 * @param $sqlo
	 */
	function calendar(&$sqlo) {
		
		$now = time();
		$nowTimeArr = getdate  ( $now );
		
		$this->calFeats=NULL;
		$this->calFeats['view_type'] = 'monthly';
		
		$this->calFeats['view_type'] = $_REQUEST['view_type'];
		if ($_REQUEST['view_type']==NULL) $this->calFeats['view_type'] = 'monthly';
		$view_type = $this->calFeats['view_type'];
		
		
		$allowAnalysis=1;
		$eventCalArr = NULL;
		if ($view_type!='monthly' and $view_type!='yearly' and $view_type!='weekly') {
			$allowAnalysis=0;
		}
		
		if ($allowAnalysis) {
		    $param=array();
			$param['Y']   = $_REQUEST['year'];
			$param['M']   = $_REQUEST['month'];
			$param['D']   = $_REQUEST['day'];
			
			if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
				$this->_html->line_out('DEBUG_X: input_date_params: '.print_r($param,1));
			}
			
			if ($param['Y']==NULL) $param['Y'] = $nowTimeArr['year'];
			if ($param['M']==NULL) $param['M'] = $nowTimeArr['mon'];
			if ($param['D']==NULL) $param['D'] = $nowTimeArr['mday'];
			
			if ($view_type == 'monthly') {
				$param['D']='01';
			}
			
			if ($view_type == 'yearly') {
				$param['D']='01';
				$param['M']='01';
			}
			
			if ($view_type == 'weekly') {
				$startCalUnx = mktime  (0,0,0,$param['M'],$param['D'],$param['Y']);
				$startCalUnx = $startCalUnx - date_unixDay2Sec(7);
				$starArr = getdate($startCalUnx);
				$param['D']  = $starArr['mday'];
				$param['M']  = $starArr['mon'];
				$param['Y']  = $starArr['year'];
			}
			
			
			$startCalUnx = mktime  (0,0,0,$param['M'],$param['D'],$param['Y']);
			
			
			$dayDiff = 365; // yearly
			if ($view_type == 'monthly') {
				$dayDiff = 31;
			}
			if ($view_type == 'weekly') {
				$dayDiff = 14; // TBD: calculate start day better
			}
			$endCalUnx = $startCalUnx + date_unixDay2Sec($dayDiff);
			
			$dateUnxArr = array(
				$startCalUnx,
				$endCalUnx
			);
			
			$logAnaLib = new oREA_LOG_servAna();
			$logAnaLib->initDev($sqlo, $this->objid);
			$eventArr  = $logAnaLib->getSrvOfTimeRange($sqlo, $dateUnxArr);
			
			if (!empty($eventArr)) {
			
				$realogGuiLib = new oREA_LOG_guisub();
			
				// reformat
				reset ($eventArr);
				foreach( $eventArr as $pos=>$datex) {
					$newDate = substr($datex,0,4) . substr($datex,5,2) . substr($datex,8,2);
					
					$imgTmp = NULL;
					
					if ($view_type == 'monthly' or $view_type == 'weekly' ) {
						// store more content
						// get entry name
						$sqlsel = 'NAME, ACCEPT from REA_LOG where CHIP_READER_ID='.$this->objid.' and POS='.$pos;
						$sqlo->Quesel($sqlsel);
						$sqlo->ReadRow();
						$nameTmp = $sqlo->RowData[0];
						$accept  = $sqlo->RowData[1]; // 
						
						if ($accept!=NULL) {
							$imgurl = $realogGuiLib->getIconOfAccept($accept);
							if ($imgurl!=NULL) $imgTmp = '<img src="images/'.$imgurl.'">';
						}
						if (!is_array($eventCalArr[$newDate])) $eventCalArr[$newDate]=array();
						$eventCalArr[$newDate][] = array('urlpar'=>$pos, 'txt'=>$nameTmp, 'img'=>$imgTmp);
					} else {
						$eventCalArr[$newDate] = 1;
					}
				}
				reset ($eventArr); 
			}
		}
		
	    // create calendar object
	    $objCalendar = new Calendar('p.php?mod='.$this->_mod.'&id='.$this->objid);
	    ## *** set calendar width and height
	    $objCalendar->SetCalendarDimensions("800px", "500px");
	    ## *** set week day name length - "short" or "long"
	    $objCalendar->SetWeekDayNameLength("long");
	    ## *** set start day of week: from 1 (Sanday) to 7 (Saturday)
	    $objCalendar->SetWeekStartedDay("1");
	    ## *** set calendar caption
	    $objCalendar->SetCaption("Service calendar");
	
		// $eventCalArr=array('20110827'=>1, '20110825'=>1, '20110806'=>1);
	 	$objCalendar->setstoreEvent($eventCalArr);
	 	$objCalendar->setLinkBase('obj.chip_reader.logEdit.php?id='.$this->objid.'&backPage=1&pos=');
	    ## +---------------------------------------------------------------------------+
	    ## | 3. Draw Calendar:                                                         | 
	    ## +---------------------------------------------------------------------------+
	    
	    $objCalendar->Show();
	}
	

	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$this->objid   = $_REQUEST['id'];
		$sqlo = &$this->_sqlo;
		
		if ($_REQUEST['showtype']!=NULL) {
			$_SESSION['s_formState']['o.CHIP_READER.serviceHome']=array('showtype'=>$_REQUEST['showtype']);
		}
		
		$showtype = $_SESSION['s_formState']['o.CHIP_READER.serviceHome']['showtype'];
		if ($showtype==NULL) $showtype='calendar';
		
		$riderLib = new fRiderC();
		$xmodes=array(
			'overview'=> array('overview','p.php?mod='.$this->_mod.'&showtype=overview&id='.$this->objid),
			'calendar'=> array('calendar','p.php?mod='.$this->_mod.'&showtype=calendar&id='.$this->objid),
		);
		$riderLib->riderShow( $xmodes, $showtype, 'view:'); 
		
		if ($showtype=='overview') {
			$this->_html->line_out('');
			$this->_html->line_out('[<a href="p.php?mod=DEF/o.CHIP_READER.siccalc&id='.$this->objid.'">Berechne neue Service-Termine</a>]');
			$this->showNextDates($sqlo);
		} else {
		
			$this->_html->line_out('');
			$this->calendar($sqlo);
		}
	}
	
}