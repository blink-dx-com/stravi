<?php
################################################################################
##              -= YOU MAY NOT REMOVE OR CHANGE THIS NOTICE =-                 #
## --------------------------------------------------------------------------- #
##  ApPHP Calendar version 1.0.2 (28.08.2009)                                  #
##  Developed by:  ApPhp <info@apphp.com>                                      #
##				   further developed: St. Kube, Jena
##					       
##  License:       GNU GPL v.2                                                 #
##  Site:          http://www.apphp.com/php-calendar/                          #
##  Copyright:     ApPHP Calendar (c) 2009. All rights reserved.               #
##                                                                             #
################################################################################


/**
 * Calendar visualization
 * @package f.acalendar.class.inc
 * @swreq Task:0001458:004 service planning : show calendar
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * _GET variables:
 *  'action' : 'display', none
 *  'year'
	'month'
	'day'
	'view_type':
		daily NOT yet supported
		weekly
		monthly
		yearly
 */
class Calendar{
	
	
	// PUBLIC
	// --------
	// __construct()
	// __destruct()
	// Show()
	// SetCalendarDimensions
	// SetCaption
	// SetWeekStartedDay
	// SetWeekDayNameLength
	// 
	// STATIC
	// ----------
	// Version
	// 
	// PRIVATE
	// --------
	// SetDefaultParameters
	// GetCurrentParameters
	// DrawJsFunctions
	// DrawYear
	// DrawMonth	
	// DrawMonthSmall
	// DrawWeek
	// DrawDay
	// DrawTypesChanger
	// DrawDateJumper
	// DrawTodayJumper
	// --------
	// isYear
	// isMonth
	// isDay
	// ConvertToDecimal

	//--- PUBLIC DATA MEMBERS --------------------------------------------------
	var $error;
	
	//--- PROTECTED DATA MEMBERS -----------------------------------------------
	var $weekDayNameLength;
	
	//--- PRIVATE DATA MEMBERS -------------------------------------------------
	
	
	/**  
		array (YYYYMMDD => array( array('urlpar'=>urlparam, 'txt'=>text) ) ) for monthly
		  OR 
		array (YYYYMMDD => 0,1 )  for yearly
	*/
	var  $storeEvent; 
	
	var  $arrWeekDays;
	var  $arrMonths;
	var  $arrViewTypes;
	var  $defaultView;
	var  $defaultAction;
	
	var  $arrParameters;
	var  $arrToday;
	var  $prevYear;
	var  $nextYear;
	var  $prevMonth;
	var  $nextMonth;
	
	var  $isDrawNavigation;
	
	var  $crLt;	
	var  $caption;		
	var  $calWidth;		
	var  $calHeight;
	var  $cellHeight;

	var $version = "1.0.2";
	
		
	/**
	 * @param $postBackUrl URL + init-params
	 */
	function __construct($postBackUrl)
	{
		
		$this->_debug = $_SESSION["userGlob"]["g.debugLevel"];
		$this->defaultView   = "monthly";
		$this->defaultAction = "view";
		$this->linkBase      = NULL;
		
		$this->postBackUrl = $postBackUrl;
		
		// possible values 1,2,....7
		$this->weekStartedDay = 2;
		
		$this->weekDayNameLength = "short"; // short|long
		
		$this->arrWeekDays = array();
		$this->arrWeekDays[0] = array("short"=>"Sun", "long"=>"Sunday");
		$this->arrWeekDays[1] = array("short"=>"Mon", "long"=>"Monday");
		$this->arrWeekDays[2] = array("short"=>"Tue", "long"=>"Tuesday");
		$this->arrWeekDays[3] = array("short"=>"Wed", "long"=>"Wednesday");
		$this->arrWeekDays[4] = array("short"=>"Thu", "long"=>"Thursday");
		$this->arrWeekDays[5] = array("short"=>"Fri", "long"=>"Friday");
		$this->arrWeekDays[6] = array("short"=>"Sat", "long"=>"Saturday");
		
		$this->arrMonths = array();
		$this->arrMonths["1"] = "January";
		$this->arrMonths["2"] = "February";
		$this->arrMonths["3"] = "March";
		$this->arrMonths["4"] = "April";
		$this->arrMonths["5"] = "May";
		$this->arrMonths["6"] = "June";
		$this->arrMonths["7"] = "July";
		$this->arrMonths["8"] = "August";
		$this->arrMonths["9"] = "September";
		$this->arrMonths["10"] = "October";
		$this->arrMonths["11"] = "November";
		$this->arrMonths["12"] = "December";
		
		$this->arrViewTypes = array();
		// $this->arrViewTypes["daily"]   = "Daily";
		$this->arrViewTypes["weekly"]  = "Weekly";
		$this->arrViewTypes["monthly"] = "Monthly";
		$this->arrViewTypes["yearly"]  = "Yearly";
		
		$this->arrParameters = array();
		$this->SetDefaultParameters();

		$this->arrToday  = array();
		$this->prevYear  = array();
		$this->nextYear  = array();
		$this->prevMonth = array();
		$this->nextMonth = array();
		
		$this->isDrawNavigation = true;
		
		$this->crLt = "\n";
		$this->caption = "";
		$this->calWidth = "800px";
		$this->calHeight = "470px";
		$this->celHeight = number_format(((int)$this->calHeight)/6, "0")."px";
	}
	
	static function getCssFileName() {
		return ('res/css/f.calendar.style.css');
	}
	
	//--------------------------------------------------------------------------
    // CLASS DESTRUCTOR
	//--------------------------------------------------------------------------
    function __destruct()
	{
		// echo 'this object has been destroyed';
    }

	
	//==========================================================================
    // PUBLIC DATA FUNCTIONS
	//==========================================================================			
	/**
	 *	Show Calendar 
	 *
	*/	
	function Show()
	{
		$this->GetCurrentParameters();
		$this->DrawJsFunctions();
		
		echo "<div id='calendar' style='width:".$this->calWidth.";'>".$this->crLt;		
		
		// draw calendar header
		echo "<table id='calendar_header'>".$this->crLt;
		echo "<tr>";
		echo "<th class='caption_left'>".$this->DrawTodayJumper(false)."</th>";
		echo "<th class='caption'>".$this->caption."</th>";
		echo "<th class='types_changer'>".$this->DrawTypesChanger(false)."</th>";
		echo "</tr>".$this->crLt;
		echo "</table>";

		
		switch($this->arrParameters["view_type"])
		{			
			case "daily":
				$this->DrawDay();
				break;
			case "weekly":
				$this->DrawWeek();
				break;
			case "yearly":
				$this->DrawYear();
				break;			
			default:
			case "monthly":				
				$this->DrawMonth();
				break;
		}
		
		echo "</div>".$this->crLt;
        echo $this->crLt."<!-- This script was generated by ApPHP Calendar v.". $this->version .
        	" -->".$this->crLt;
		
	}
	
	function setStoreEvent ($storeEvent) {
		$this->storeEvent=$storeEvent;
		if ($this->_debug>0) {
			echo "DEBUG: ". __CLASS__.":setStoreEvent: <pre>";
			print_r($storeEvent);
			echo "</pre><br />\n";
		}
	}
	
	function setLinkBase ($linkBase) {
		$this->linkBase=$linkBase;
	}
	
	
	/**
	 *	Set calendar dimensions
	 *  	@param $width
	 *  	@param $height
	*/	
	function SetCalendarDimensions($width = "", $height = "")
	{
		$this->calWidth = ($width != "") ? $width : "800px";
		$this->calHeight = ($height != "") ? $height : "470px";
		$this->celHeight = number_format(((int)$this->calHeight)/6, "0")."px";
	}

	/**
	 *	Check if parameters is 4-digit year
	 *  	@param $year - string to be checked if it's 4-digit year
	*/	
	function SetCaption($caption_text = "")
	{
		$this->caption = $caption_text;	
	}
	
	/**
	 *	Set week started day
	 *  	@param $started_day - started day of week 1...7
	*/	
	function SetWeekStartedDay($started_day = "1")
	{
		if(is_numeric($started_day) && (int)$started_day >= 1 && (int)$started_day <= 7){
			$this->setWeekStartedDay = (int)$started_day;				
		}
	}

	/**
	 *	Set week day name length 
	 *  	@param $length_name - "short"|"long"
	*/	
	function SetWeekDayNameLength($length_name = "short")
	{
		if(strtolower($length_name) == "long"){
			$this->weekDayNameLength = "long";
		}
	}
	
	//==========================================================================
    // STATIC
	//==========================================================================		
	/**
	 *	Return current version
	*/	
	function Version()
	{
		return $this->version;
	}
	
	
	
	//==========================================================================
    // PRIVATE DATA FUNCTIONS
	//==========================================================================		
	/**
	 *	Set default parameters
	 *
	*/	
	function SetDefaultParameters()
	{
		$this->arrParameters["year"]  = date("Y");
		$this->arrParameters["month"] = date("m");
		$this->arrParameters["month_full_name"] = date("F");
		$this->arrParameters["day"]   = date("d");
		$this->arrParameters["view_type"] = $this->defaultView;
		$this->arrParameters["action"] = "display";
		$this->arrToday = getdate();

		// get PostBackUrl
		$this->arrParameters["current_file"] = $this->postBackUrl;
		// $parts = explode('/', $this->arrParameters["current_file"]);
		// $this->arrParameters["current_file"] = $parts[count($parts) - 1];		
	}

	/**
	 *	Get current parameters - read them from URL
	 *
	*/	
	function GetCurrentParameters()
	{
		
		$year 		= (isset($_GET['year']) && $this->isYear($_GET['year'])) ? $this->remove_bad_chars($_GET['year']) : date("Y");
		$month 		= (isset($_GET['month']) && $this->isMonth($_GET['month'])) ? $this->remove_bad_chars($_GET['month']) : date("m");
		$day 		= (isset($_GET['day']) && $this->isDay($_GET['day'])) ? $this->remove_bad_chars($_GET['day']) : date("d");
		$view_type 	= (isset($_GET['view_type']) && array_key_exists($_GET['view_type'], $this->arrViewTypes)) ? $this->remove_bad_chars($_GET['view_type']) : "monthly";
	
		$cur_date = getdate(mktime(0,0,0,$month,$day,$year));
		
		///echo "<br>3--";
		///print_r($cur_date);
		
		$this->arrParameters["year"]  = $cur_date['year'];
		$this->arrParameters["month"] = $this->ConvertToDecimal($cur_date['mon']);
		$this->arrParameters["month_full_name"] = $cur_date['month'];
		$this->arrParameters["day"]   = $day;
		$this->arrParameters["view_type"] = $view_type;
		$this->arrParameters["action"] = "display";
		$this->arrToday = getdate();

		$this->prevYear = getdate(mktime(0,0,0,$this->arrParameters['month'],$this->arrParameters["day"],$this->arrParameters['year']-1));
		$this->nextYear = getdate(mktime(0,0,0,$this->arrParameters['month'],$this->arrParameters["day"],$this->arrParameters['year']+1));

		$this->prevMonth = getdate(mktime(0,0,0,$this->arrParameters['month']-1,$this->arrParameters["day"],$this->arrParameters['year']));
		$this->nextMonth = getdate(mktime(0,0,0,$this->arrParameters['month']+1,$this->arrParameters["day"],$this->arrParameters['year']));
	}

	/**
	 *	Draw javascript functions
	 *
	*/	
	function DrawJsFunctions()
	{
		echo "<script type='text/javascript'>";
		echo "
			function JumpToDate(){
				var jump_day   = (document.getElementById('jump_day')) ? document.getElementById('jump_day').value : '';
				var jump_month = (document.getElementById('jump_month')) ? document.getElementById('jump_month').value : '';
				var jump_year  = (document.getElementById('jump_year')) ? document.getElementById('jump_year').value : '';
				var view_type  = (document.getElementById('view_type')) ? document.getElementById('view_type').value : '';
				
				__doPostBack('view', view_type, jump_year, jump_month, jump_day);
			}
		
			function __doPostBack(action, view_type, year, month, day)
			{			
				var action    = (action != null) ? action : 'view';
				var view_type = (view_type != null) ? view_type : 'monthly';
				var year      = (year != null) ? year : '".$this->arrToday["year"]."';
				var month     = (month != null) ? month : '".$this->ConvertToDecimal($this->arrToday["mon"])."';
				var day       = (day != null) ? day : '".$this->arrToday["mday"]."';
			
				document.location.href = '".$this->arrParameters["current_file"]."&action='+action+'&view_type='+view_type+'&year='+year+'&month='+month+'&day='+day;		
			}
		";
		echo "</script>";
		
	}

	/**
	 *	Draw yearly calendar
	 *
	*/	
	function DrawYear()
	{
		$this->celHeight = "20px";
		echo "<table class='year_container'>".$this->crLt;
		echo "<tr>".$this->crLt;
			echo "<th colspan='3'>";
				echo "<table class='table_navbar'>".$this->crLt;
				echo "<tr>";
				echo "<th class='tr_navbar_left' valign='middle'>
					  ".$this->DrawDateJumper(false, false, false)."
					  </th>".$this->crLt;
				echo "<th class='tr_navbar'></th>".$this->crLt;
				echo "<th class='tr_navbar_right'>				
					  <a href=\"javascript:__doPostBack('view', 'yearly', '".$this->prevYear['year']."', '".$this->arrParameters['month']."', '".$this->arrParameters['day']."')\">".$this->prevYear['year']."</a> |
					  <a href=\"javascript:__doPostBack('view', 'yearly', '".$this->nextYear['year']."', '".$this->arrParameters['month']."', '".$this->arrParameters['day']."')\">".$this->nextYear['year']."</a>
					  </th>".$this->crLt;
				echo "</tr>".$this->crLt;
				echo "</table>".$this->crLt;
			echo "</td>".$this->crLt;
		echo "</tr>".$this->crLt;

		echo "<tr>";
		for($i = 1; $i <= 12; $i++){
			echo "<td align='center' valign='top'>";
			echo "<a href=\"javascript:__doPostBack('view', 'monthly', '".$this->arrParameters['year']."', '".$this->ConvertToDecimal($i)."', '".$this->arrParameters['day']."')\"><b>".$this->arrMonths["$i"]."</b></a>";
			$this->DrawMonthSmall($this->arrParameters['year'], $this->ConvertToDecimal($i));
			echo "</td>";
			if(($i != 1) && ($i % 3 == 0)) echo "</tr><tr>";
		}
		echo "</tr>";
		echo "<tr><td nowrap height='5px'></td></tr>";
		echo "</table>";
	}
	
	
	/**
		show event for MONTHLY view
		@param $targetDate YYYYMMDD
	*/
	function getEventWeek($y,$m,$d) {
		
		$targetDate = $y . str_pad( $m, 2, "0", STR_PAD_LEFT ) . str_pad( $d, 2, "0", STR_PAD_LEFT );
		
		if ( is_array($this->storeEvent[$targetDate]) ) {
			$info=NULL;
			
			$tmpOneDay = $this->storeEvent[$targetDate];
			reset ($tmpOneDay);
			$tmpKomma=NULL;
			foreach( $tmpOneDay as $dummy=>$oneEntryArr) {
				$info .= $tmpKomma;
				if ($oneEntryArr['img']!=NULL) {
					$info .= $oneEntryArr['img'];
				}
				if (strlen($oneEntryArr['txt'])>60) {
					$showTxt = substr($oneEntryArr['txt'],0,60).'..';
				} else $showTxt = $oneEntryArr['txt'];
				
				if ($this->linkBase!=NULL) {
					
					$info .= '<a href="'.$this->linkBase.$oneEntryArr['urlpar'].'" title="'.$oneEntryArr['txt'].'">'.$showTxt.'</a>';
				} else {
					$info .= $showTxt;
				}
				$tmpKomma='<br />';
			}
			reset ($tmpOneDay);
			
		}
		return $info;
	}
	
	/**
		show event for MONTHLY view
	*/
	function getEventHit($class, $actday) {
	    
	    $MAX_TXT_LEN=20; // 8
	    
		$actday = str_pad( $actday, 2, "0", STR_PAD_LEFT );
		$targetDate = $this->arrParameters['year'].$this->arrParameters['month'].$actday;
		if ( is_array($this->storeEvent[$targetDate]) ) {
			$info="<br>";
			
			$tmpOneDay = $this->storeEvent[$targetDate];
			reset ($tmpOneDay);
			$tmpKomma=NULL;
			foreach( $tmpOneDay as $dummy=>$oneEntryArr) {
				$info .= $tmpKomma;
				if ($oneEntryArr['img']!=NULL) {
					$info .= $oneEntryArr['img'];
				}
				if (strlen($oneEntryArr['txt'])>$MAX_TXT_LEN-1) {
				    $showTxt = substr($oneEntryArr['txt'],0,$MAX_TXT_LEN).'..';
				} else $showTxt = $oneEntryArr['txt'];
				
				if ($this->linkBase!=NULL) {
					
					$info .= '<a href="'.$this->linkBase.$oneEntryArr['urlpar'].'" title="'.$oneEntryArr['txt'].'">'.$showTxt.'</a>';
				} else {
					$info .= $showTxt;
				}
				$tmpKomma='<br />';
			}
			reset ($tmpOneDay);
			
			$class = " class='td_eveday'";
		}
		return array($class,$info);
	}
	
	function getEventHitSmall($class, $actday, $month, $year) {
		$actday = str_pad( $actday, 2, "0", STR_PAD_LEFT );
		$targetDate = $year.$month.$actday;
		if ($this->storeEvent[$targetDate]>0) {
			$info="!!!";
			$class = " class='td_small_eveday'";
		}
		return array($class,$info);
	}

	/**
	 *	Draw monthly calendar
	 *
	*/	
	function DrawMonth()
	{
	
		$startTimeUnx = mktime(0,0,0,$this->arrParameters['month'],1,$this->arrParameters['year']);
		$endDayUnx = mktime(0,0,0,$this->arrParameters['month']+1,0,$this->arrParameters['year']);
		
		// today, first day and last day in month
		$firstDay = getdate($startTimeUnx);
		$lastDay  = getdate($endDayUnx);
		
		if ( $this->_debug>0 ) {
			debugOut('startTimeUnx: '.date('Y-m-d',$startTimeUnx), 'DrawWeek');
			debugOut('endDay: '.date('Y-m-d',$endDayUnx), 'DrawWeek');
		}
		
		///print_r($firstDay);
		
		// Create a table with the necessary header informations
		echo "<table class='month'>".$this->crLt;
		echo "<tr>";
			echo "<th colspan='8'>";
				echo "<table class='table_navbar'>".$this->crLt;
				echo "<tr>";
				echo "<th class='tr_navbar_left'>
					  ".$this->DrawDateJumper(false)."	
					  </th>".$this->crLt;
				echo "<th class='tr_navbar'>";
				echo " <a href=\"javascript:__doPostBack('view', 'monthly', '".$this->prevMonth['year']."', '".$this->ConvertToDecimal($this->prevMonth['mon'])."', '".$this->arrParameters['day']."')\">&laquo;&laquo;</a> ";
				echo $this->arrParameters['month_full_name']." - ".$this->arrParameters['year'];
				echo " <a href=\"javascript:__doPostBack('view', 'monthly', '".$this->nextMonth['year']."', '".$this->ConvertToDecimal($this->nextMonth['mon'])."', '".$this->arrParameters['day']."')\">&raquo;&raquo;</a> ";
				echo "</th>".$this->crLt;
				echo "<th class='tr_navbar_right'>				
					  <a href=\"javascript:__doPostBack('view', 'monthly', '".$this->prevYear['year']."', '".$this->arrParameters['month']."', '".$this->arrParameters['day']."')\">".$this->prevYear['year']."</a> |
					  <a href=\"javascript:__doPostBack('view', 'monthly', '".$this->nextYear['year']."', '".$this->arrParameters['month']."', '".$this->arrParameters['day']."')\">".$this->nextYear['year']."</a>
					  </th>".$this->crLt;
				echo "</tr>".$this->crLt;
				echo "</table>".$this->crLt;
			echo "</th>".$this->crLt;
		echo "</tr>".$this->crLt;
		echo "<tr class='tr_days'>";
		
		echo "<td widht=30></td>";  // empty KW-cell
			for($i = $this->weekStartedDay-1; $i < $this->weekStartedDay+6; $i++){
				echo "<td class='th'>".$this->arrWeekDays[($i % 7)][$this->weekDayNameLength]."</td>";		
			}
		echo "</tr>".$this->crLt;
		
		// Display the first calendar row with correct positioning
		if ($firstDay['wday'] == 0) $firstDay['wday'] = 7;
		$max_empty_days = $firstDay['wday']-($this->weekStartedDay-1);		
		if($max_empty_days < 7){
			echo "<tr class='tr' style='height:".$this->celHeight.";'>".$this->crLt;		
			echo "<td width=30><a href=\"javascript:__doPostBack('view', 'weekly', '".$this->arrParameters['year']."', '".$this->arrParameters['month']."', '01')\">week</a></td>".$this->crLt;

			for($i = 1; $i <= $max_empty_days; $i++){
				echo "<td class='td_empty'>&nbsp;</td>".$this->crLt;
			}			
			$actday = 0;
			for($i = $max_empty_days+1; $i <= 7; $i++){
				$actday++;
				if (($actday == $this->arrToday['mday']) && ($this->arrToday['mon'] == $this->arrParameters["month"])) {
					$class = " class='td_actday'";			
				} else if ($actday == $this->arrParameters['day']){				
					$class = " class='td_selday'";				
				} else {
					$class = " class='td'";
				} 
				
				list($class,$hitinfo) = $this->getEventHit($class,$actday);
				echo "<td$class>$actday".$hitinfo."</td>".$this->crLt;
			}
			echo "</tr>".$this->crLt;
		}
		
		//Get how many complete weeks are in the actual month
		$fullWeeks = floor(($lastDay['mday']-$actday)/7);
		
		for ($i=0;$i<$fullWeeks;$i++){
			echo "<tr class='tr' style='height:".$this->celHeight.";'>".$this->crLt;
			echo "<td width=30><a href=\"javascript:__doPostBack('view', 'weekly', '".$this->arrParameters['year']."', '".$this->arrParameters['month'].
				"', '".$this->ConvertToDecimal($actday+1)."')\">week</a></td>".$this->crLt;

			for ($j=0;$j<7;$j++){
				$actday++;
				if (($actday == $this->arrToday['mday']) && ($this->arrToday['mon'] == $this->arrParameters["month"])) {
					$class = " class='td_actday'";
				} else if ($actday == $this->arrParameters['day']){				
					$class = " class='td_selday'";				
				} else {
					$class = " class='td'";
				}
				
				list($class,$hitinfo) = $this->getEventHit($class,$actday);
				echo "<td$class>$actday".$hitinfo."</td>".$this->crLt;
			}
			echo "</tr>".$this->crLt;
		}
		
		//Now display the rest of the month
		if ($actday < $lastDay['mday']){
			echo "<tr class='tr' style='height:".$this->celHeight.";'>".$this->crLt;
			echo "<td width=30><a href=\"javascript:__doPostBack('view', 'weekly', '".$this->arrParameters['year']."', '".
				$this->arrParameters['month']."', '".$this->ConvertToDecimal($actday+1)."')\">week</a></td>".$this->crLt;
			for ($i=0; $i<7;$i++){
				$actday++;
				if (($actday == $this->arrToday['mday']) && ($this->arrToday['mon'] == $this->arrParameters["month"])) {
					$class = " class='td_actday'";
				} else {
					$class = " class='td'";
				}		
				list($class,$hitinfo) = $this->getEventHit($class,$actday);

				if ($actday <= $lastDay['mday']){
					echo "<td$class>$actday".$hitinfo."</td>".$this->crLt;
				} else {
					echo "<td class='td_empty'>&nbsp;</td>".$this->crLt;
				}
			}					
			echo "</tr>".$this->crLt;
		}		
		echo "</table>".$this->crLt;
		
	}


	/**
	 *	Draw small monthly calendar
	 *
	*/	
	function DrawMonthSmall($year = "", $month = "")
	{
		if($month == "") $month = $this->arrParameters['month'];
		if($year == "") $year = $this->arrParameters['year'];
		$week_rows = 0;
		
		// today, first day and last day in month
		$firstDay = getdate(mktime(0,0,0,$month,1,$year));
		$lastDay  = getdate(mktime(0,0,0,$month+1,0,$year));
		
		///print_r($firstDay);
		
		// create a table with the necessary header informations
		echo "<table class='month_small'>".$this->crLt;
		echo "<tr class='tr_small_days'>";
			for($i = $this->weekStartedDay-1; $i < $this->weekStartedDay+6; $i++){
				echo "<td class='th_small'>".$this->arrWeekDays[($i % 7)]["short"]."</td>";		
			}
		echo "</tr>".$this->crLt;
		
		// display the first calendar row with correct positioning
		if ($firstDay['wday'] == 0) $firstDay['wday'] = 7;
		$max_empty_days = $firstDay['wday']-($this->weekStartedDay-1);		
		if($max_empty_days < 7){
			echo "<tr class='tr_small' style='height:".$this->celHeight.";'>".$this->crLt;			
			for($i = 1; $i <= $max_empty_days; $i++){
				echo "<td class='td_small_empty'>&nbsp;</td>".$this->crLt;
			}			
			$actday = 0;
			for($i = $max_empty_days+1; $i <= 7; $i++){
				$actday++;
				if (($actday == $this->arrToday['mday']) && ($this->arrToday['mon'] == $month) && ($this->arrToday['year'] == $year)) {
					$class = " class='td_small_actday'";			
				} else if ($actday == $this->arrParameters['day'] && ($this->arrParameters['mon'] == $month)){			
					$class = " class='td_small_selday'";				
				} else {
					$class = " class='td_small'";
				}
				list($class,$hitinfo) = $this->getEventHitSmall($class,$actday,$month, $year);
				echo "<td$class>$actday</td>".$this->crLt;
			}
			echo "</tr>".$this->crLt;
			$week_rows++;
		}
		
		// get how many complete weeks are in the actual month
		$fullWeeks = floor(($lastDay['mday']-$actday)/7);
		
		for ($i=0;$i<$fullWeeks;$i++){
			echo "<tr class='tr_small' style='height:".$this->celHeight.";'>".$this->crLt;
			for ($j=0;$j<7;$j++){
				$actday++;
				if (($actday == $this->arrToday['mday']) && ($this->arrToday['mon'] == $month) && ($this->arrToday['year'] == $year)) {
					$class = " class='td_small_actday'";
				} else if ($actday == $this->arrParameters['day'] && ($this->arrParameters['mon'] == $month)){				
					$class = " class='td_small_selday'";				
				} else {
					$class = " class='td_small'";
				}
				list($class,$hitinfo) = $this->getEventHitSmall($class,$actday,$month, $year);
				echo "<td$class>$actday</td>".$this->crLt;
			}
			echo "</tr>".$this->crLt;
			$week_rows++;			
		}
		
		// now display the rest of the month
		if ($actday < $lastDay['mday']){
			echo "<tr class='tr_small' style='height:".$this->celHeight.";'>".$this->crLt;			
			for ($i=0; $i<7;$i++){
				$actday++;
				if (($actday == $this->arrToday['mday']) && ($this->arrToday['mon'] == $month) && ($this->arrToday['year'] == $year)) {
					$class = " class='td_small_actday'";
				}  else if ($actday == $this->arrParameters['day'] && ($this->arrParameters['mon'] == $month)){				
					$class = " class='td_small_selday'";				
				} else {
					$class = " class='td_small'";
				}	
				list($class,$hitinfo) = $this->getEventHitSmall($class,$actday,$month, $year);
				if ($actday <= $lastDay['mday']){
					echo "<td$class>$actday</td>".$this->crLt;
				} else {
					echo "<td class='td_small_empty'>&nbsp;</td>".$this->crLt;
				}
			}					
			echo "</tr>".$this->crLt;
			$week_rows++;
		}
		
		// complete last line
		if($week_rows < 5){
			echo "<tr class='tr_small' style='height:".$this->celHeight.";'>".$this->crLt;			
			for ($i=0; $i<7;$i++){
				echo "<td class='td_small_empty'>&nbsp;</td>".$this->crLt;
			}					
			echo "</tr>".$this->crLt;
			$week_rows++;			
		}
		
		echo "</table>".$this->crLt;
		
	}
	

	/**
	 *	Draw weekly calendar
	 *
	*/	
	function DrawWeek()
	{
		
		$tmpDay   = $this->arrParameters['day'];
		$unxNow   = mktime(0,0,0,$this->arrParameters['month'],$tmpDay,$this->arrParameters['year']);
		$nowArr   = getdate($unxNow);
		$nowMonth = $nowArr ['mon'];
		$kwString = date('W',$unxNow);
		
		$tmpDayNUM = date('w', $unxNow); // for PHP5.x use: date('N'); 1-Mo
		if (!$tmpDayNUM) $tmpDayNUM=7;   // sunday
		$daysDiff  = $tmpDayNUM - 1;     // calc diff days
		$startTimeUnx = $unxNow - $daysDiff*3600*24;
		$endDayUnx    = $startTimeUnx + $this->_day2Unx(6);
		
		// today, first day and last day in month
		// $firstDay = getdate(mktime(0,0,0,$this->arrParameters['month'],1,$this->arrParameters['year']));
		$firstDay = getdate($startTimeUnx);
		$lastDay  = getdate($endDayUnx);
		
		$prevWeek = getdate($startTimeUnx - $this->_day2Unx(7.1));  // small amount more than one day
		$nextWeek = getdate($endDayUnx    + $this->_day2Unx(1.1)); // small amount more than one day
		
		if ( $this->_debug>0 ) {
			debugOut('unxNow: '.date('Y-m-d',$unxNow).' dayDiffToMonday: '.$daysDiff, 'DrawWeek');
			debugOut('startTimeUnx: '.date('Y-m-d',$startTimeUnx), 'DrawWeek');
			debugOut('endDay: '.date('Y-m-d',$endDayUnx), 'DrawWeek');
			debugOut('nextWeek: '.print_r($nextWeek,1), 'DrawWeek');
		}
		
		
		// Create a table with the necessary header informations
		echo "<table class='month'>".$this->crLt;
		echo "<tr>";
		echo "<th class='tr_navbar_left' colspan='2'>
			  ".$this->DrawDateJumper(false)."	
			  </th>".$this->crLt;
			
		echo "<th class='tr_navbar' colspan='3'>";
		echo " <a href=\"javascript:__doPostBack('view', 'weekly', '".$prevWeek['year']."', '".$this->ConvertToDecimal($prevWeek['mon']).
			"', '".$this->ConvertToDecimal($prevWeek['mday'])."')\">&laquo;&laquo;</a> ";
		echo $this->arrParameters['month_full_name']." - ".$this->arrParameters['year'].' &nbsp;&nbsp;&nbsp; KW '.$kwString;
		echo " <a href=\"javascript:__doPostBack('view', 'weekly', '".$nextWeek['year']."', '".$this->ConvertToDecimal($nextWeek['mon']).
			"', '".$this->ConvertToDecimal($nextWeek['mday'])."')\">&raquo;&raquo;</a> ";
		echo "</th>".$this->crLt;
			  
		// echo "<th class='tr_navbar' colspan='3'>".$this->arrParameters['month_full_name']." - ".$this->arrParameters['year']."</th>".$this->crLt;
		echo "<th class='tr_navbar_right' colspan='2'>				
			  <a href=\"javascript:__doPostBack('view', 'monthly', '".$this->prevYear['year']."')\">".$this->prevYear['year']."</a> |
			  <a href=\"javascript:__doPostBack('view', 'monthly', '".$this->nextYear['year']."')\">".$this->nextYear['year']."</a>
			  </th>".$this->crLt;
		echo "</tr>".$this->crLt;
		
		/*
		echo "<tr class='tr_days'>";
			for($i = $this->weekStartedDay-1; $i < $this->weekStartedDay+6; $i++){
				echo "<td class='th'>".$this->arrWeekDays[($i % 7)][$this->weekDayNameLength]."</td>";		
			}
		echo "</tr>".$this->crLt;
		*/
		
		
		// Display the first calendar row with correct positioning
		
		
		$dayName_i = $this->weekStartedDay-1;
		
		$actday = $firstDay["mday"];
		
		$timeLoop = $startTimeUnx;
		
		for($i = 0; $i < 7; $i++){
			$timeLoopArr = getdate($timeLoop);
			$actday  = $timeLoopArr["mday"];
			$loopMon = $timeLoopArr["mon"];
			echo "<tr class='tr'>".$this->crLt;
			
			
			if (($actday == $this->arrToday['mday']) && ($this->arrToday['mon'] == $this->arrParameters["month"])) {
				$class = " class='td_actday'";
			} else {
				$class = " class='td'";
			}
			if ($loopMon!=$nowMonth) $class = " class='td_empty'";
			
			$hitinfo = $this->getEventWeek($timeLoopArr["year"], $loopMon, $actday);
			
			echo "<td$class>".$this->arrWeekDays[($dayName_i % 7)][$this->weekDayNameLength]."<br>".$actday."</td>";
			echo "<td$class colspan=6>".$hitinfo."</td>".$this->crLt;
			$timeLoop = $timeLoop + 3600*24; // next day
			$dayName_i++;
			echo "</tr>".$this->crLt;
		}
		
		
		echo "</tr>".$this->crLt;
		echo "</table>".$this->crLt;
		
	}



	/**
	 *	Draw daily calendar
	 *
	*/	
	function DrawDay()
	{
		echo "<br /><font color='#a60000'>This type of calendar view is not available in free version</font>";	        
	}

	/**
	 *	Draw calendar types changer
	 *  	@param $draw - draw or return
	*/	
	function DrawTypesChanger($draw = true)
	{
		$result = "<select class='form_select' name='view_type' id='view_type' ".
			"onchange=\"document.location.href='".$this->arrParameters["current_file"]."&action=view&view_type='+this.value\">";
		reset ($this->arrViewTypes);
		foreach( $this->arrViewTypes as $key=>$val) {
			$result .= "<option value='".$key."' ".(($this->arrParameters['view_type'] == $key) ? "selected='selected'" : "").">".$val."</option>";
		}
		reset ($this->arrViewTypes);
		$result .= "</select>";
		
		if($draw){
			echo $result;
		}else{
			return $result;
		}
	}

	/**
	 *	Draw today jumper
	 *  	@param $draw - draw or return
	*/	
	function DrawTodayJumper($draw = true)
	{
		$result = "<input class='form_button' type='button' value='Today' onclick=\"javascript:__doPostBack('".$this->defaultAction."', '".$this->defaultView."', '".$this->arrToday["year"]."', '".$this->ConvertToDecimal($this->arrToday["mon"])."', '".$this->arrToday["mday"]."')\" />";
	
		if($draw){
			echo $result;
		}else{
			return $result;
		}
	}
	
	/**
	 *	Draw date jumper
	 *  	@param $draw - draw or return
	*/	
	function DrawDateJumper($draw = true, $draw_day = true, $draw_month = true, $draw_year = true)
	{
		$result = "<form name='frmCalendarJumper' class='class_form'>";

		// draw days ddl
		if($draw_day){
			$result = "<select class='form_select' name='jump_day' id='jump_day'>";
			for($i=1; $i <= 31; $i++){
				$i_converted = $this->ConvertToDecimal($i);
				$result .= "<option value='".$this->ConvertToDecimal($i)."' ".(($this->arrParameters["day"] == $i_converted) ? "selected='selected'" : "").">".$i_converted."</option>";
			}
			$result .= "</select> ";			
		}else{
			$result .= "<input type='hidden' name='jump_day' id='jump_day' value='".$this->arrToday["mday"]."' />";			
		}

		// draw months ddl
		if($draw_month){			
			$result .= "<select class='form_select' name='jump_month' id='jump_month'>";
			for($i=1; $i <= 12; $i++){
				$i_converted = $this->ConvertToDecimal($i);
				$result .= "<option value='".$this->ConvertToDecimal($i)."' ".(($this->arrParameters["month"] == $i_converted) ? "selected='selected'" : "").">".$this->arrMonths[$i]."</option>";
			}
			$result .= "</select> ";			
		}else{
			$result .= "<input type='hidden' name='jump_month' id='jump_month' value='".$this->ConvertToDecimal($this->arrToday["mon"])."' />";			
		}

		// draw years ddl
		if($draw_year){			
			$result .= "<select class='form_select' name='jump_year' id='jump_year'>";
			for($i=$this->arrParameters["year"]-10; $i <= $this->arrParameters["year"]+10; $i++){
				$result .= "<option value='".$i."' ".(($this->arrParameters["year"] == $i) ? "selected='selected'" : "").">".$i."</option>";
			}
			$result .= "</select> ";
		}else{
			$result .= "<input type='hidden' name='jump_year' id='jump_year' value='".$this->arrToday["year"]."' />";			
		}
		
		$result .= "<input class='form_button' type='button' value='Go' onclick='JumpToDate()' />";
		$result .= "</form>";
		
		if($draw){
			echo $result;
		}else{
			return $result;
		}
	}

	////////////////////////////////////////////////////////////////////////////
	// Auxilary
	////////////////////////////////////////////////////////////////////////////
	/**
	 *	Check if parameters is 4-digit year
	 *  	@param $year - string to be checked if it's 4-digit year
	*/	
	function isYear($year = "")
	{
		if(!strlen($year) == 4 || !is_numeric($year)) return false;
		for($i = 0; $i < 4; $i++){
			if(!(isset($year[$i]) && $year[$i] >= 0 && $year[$i] <= 9)){
				return false;	
			}
		}
		return true;
	}

	/**
	 *	Check if parameters is month
	 *  	@param $month - string to be checked if it's 2-digit month
	*/	
	function isMonth($month = "")
	{
		if(!strlen($month) == 2 || !is_numeric($month)) return false;
		for($i = 0; $i < 2; $i++){
			if(!(isset($month[$i]) && $month[$i] >= 0 && $month[$i] <= 9)){
				return false;	
			}
		}
		return true;
	}

	/**
	 *	Check if parameters is day
	 *  	@param $day - string to be checked if it's 2-digit day
	*/	
	function isDay($day = "")
	{
		if(!strlen($day) == 2 || !is_numeric($day)) return false;
		for($i = 0; $i < 2; $i++){
			if(!(isset($day[$i]) && $day[$i] >= 0 && $day[$i] <= 9)){
				return false;	
			}
		}
		return true;
	}

	/**
	 *	Convert to decimal number with leading zero
	 *  	@param $number
	*/	
	function ConvertToDecimal($number)
	{
		return (($number < 10) ? "0" : "").$number;
	}

   	/**
	 *	Remove bad chars from input
	 *	  	@param $str_words - input
	 **/
	function remove_bad_chars($str_words)
	{
		$found = false;
		$bad_string = array("select", "drop", ";", "--", "insert","delete", "xp_", "%20union%20", "/*", "*/union/*", "+union+", "load_file", "outfile", "document.cookie", "onmouse", "<script", "<iframe", "<applet", "<meta", "<style", "<form", "<img", "<body", "<link", "_GLOBALS", "_REQUEST", "_GET", "_POST", "include_path", "prefix", "http://", "https://", "ftp://", "smb://", "onmouseover=", "onmouseout=");
		for ($i = 0; $i < count($bad_string); $i++){
			$str_words = str_replace($bad_string[$i], "", $str_words);
		}
		return $str_words;            
	}
	function _day2Unx($days) {
		return 3600*24*$days;
	}
}
?>