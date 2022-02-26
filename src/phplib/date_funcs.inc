<?php
require_once('ots/adodb-time.inc.php');
/**
 * functions for date/time; mainly formatting human-date to UNIX-stamps
 * @namespace core::misc
 * @package date_funcs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 * @version  $Header: trunk/src/phplib/date_funcs.inc 59 2018-11-21 09:04:09Z $
 */


/**
 * convert "DD-MM-RR HH24:MI:SS" to unix stamp
 * have this here, because may be the format will change
 * @param object $sql
 * @param string $sql_time
 * @return long $unix_stamp
 */
function date_dtsql2unix( &$sql, $sql_time ) {
  $unix_stamp = strtotime ( $sql_time, 0);
  return ($unix_stamp);
}

/**
 * convert "DD-MM-YYYY" to unix stamp
 * @param string $timestr
 * @return long
 */
function date_dt2sql2unix( $timestr ) {
  $day   = substr($timestr,0,2);
  $month = substr($timestr,3,2);
  $year  = substr($timestr,6,4);
  $unix_stamp = mktime ( 0,0, 0, $month, $day, $year );
  return ($unix_stamp);
}

/**
 * convert human-string to unix stamp
 * @param string $timestr
 * @param int $format 
 *   5 : "YYYY-MM-DD" or "YYYY-MM-DD HH:MM"
 *   6 : "YYYY-MM-DD HH:MM:SS" (with seconds)
 * @return long
 */
function date_str2unix( $timestr, $format ) {
	
	$hour   = 0;
	$minute = 0;
	$second = 0;
	
	if ( $format==5 or $format==6 ) { 
		$day   = substr($timestr,8,2);
		$month = substr($timestr,5,2);
		$year  = substr($timestr,0,4);
		if (substr($timestr,11,2)!=NULL) {
			$hour=intval(substr($timestr,11,2));
			if ($hour>23) $hour=0;
		}
		 if (substr($timestr,14,2)!=NULL) {
			$minute=intval(substr($timestr,14,2));
			if ($minute>59) $minute=0;
		}
	}
	
	if ( $format==6 ) { 
	    $second = intval(substr($timestr,17,2));
	}
	
	$unix_stamp = mktime ( $hour, $minute, $second, $month, $day, $year );
	// new PHP version supports year>2018 again
	// $unix_stamp = adodb_mktime($hour, $minute, $second, $month, $day, $year); // high precision lib; supports date>2038
	return ($unix_stamp);
}

// format special input format to SQL-format
function date_string2sql($datex) { // DD-MM-YYYY
	$unxstamp = mktime ( 0,0,0, substr($datex,3,2),  substr($datex,0,2), substr($datex,6,4) );
	$sqldate  = date_unix2sql( $unxstamp );
	return ($sqldate);
}

// unix-timestamp to SQL-format
function date_unix2sql($unxstamp) {
	$datePattern = 'd-M-Y';
	if ( $_SESSION['globals']['db.dateFormat']=='YYYY-MM-DD') {
		$datePattern = 'Y-m-d';
	}
	$sqldate  = date ($datePattern, $unxstamp);
	return ($sqldate);
}

/**
 * transform UNIX to string
 * @param int $unxstamp
 * @param int $format
 *  0: Y-m-d
 *  1: Y-m-d H:i:s
 *  2: Y-m-d H:i
 *  3: YYYY-MM-DD
 *  4: Y-m-dTH:i:s
 *  5: d-m-Y
 * @return string
 */
function date_unix2datestr( $unxstamp,$format=0 ) { 
	$datestr  = date ("Y-m-d", $unxstamp); // DD-MM-YYYY
	if ($format==1)  $datestr  = date ("Y-m-d H:i:s", $unxstamp);
	if ($format==2)  $datestr  = date ("Y-m-d H:i", $unxstamp);
	if ($format==3)  $datestr  = date ("Y-m-d", $unxstamp);
	if ($format==4)  $datestr  = date ("Y-m-d\TH:i:s", $unxstamp);
	if ($format==5)  $datestr  = date ("d-m-Y", $unxstamp);
	return ($datestr);
}

// DD-MM-YYYY => YYYY-MM-DD
function date_DDMMYYYY2YYYYmmDD( $DDMMYYYY ) {
	return ( substr($DDMMYYYY,6,4). "-" . substr($DDMMYYYY,3,2) . "-" . substr($DDMMYYYY,0,2) );
}

/**
 * transform DATE-string to other DATE-string format
 * @param string $dateIn
 * @param string $transType
 * 	  'Ymd2ISO'
 * 	  'ISO2Ymd'
 *    'Y-m-d2ymd'
 */
function date_transXtoY( $dateIn, $transType ) {
	$outDate=$dateIn; // default
	switch ($transType) {
		case 'Ymd2ISO':
			// 'YYYY-MM-DD HH:MM' => 'YYYY-MM-DDTHH:MM'
			$outDate = substr($dateIn,0,10).'T'.substr($dateIn,11);
			break;
		case 'ISO2Ymd':
			// 'YYYY-MM-DDTHH:MM' => 'YYYY-MM-DD HH:MM'
			$outDate = str_replace("T"," ",$dateIn);
		case 'Y-m-d2ymd':
		    // 'YYYY-MM-DD' => 'YYMMDD'
		    $outDate = substr($dateIn,2,2) . substr($dateIn,5,2) . substr($dateIn,8,2);
	}
	return $outDate;
}

/**
 * YYYYMMDD to unix
 * @param $datex  :  YYYY-MM-DD 12:45
				     0    5  8  11 14
 * @return -
 * @param unixstamp h m s , m d y 
 *		   0 : error
 */
function date_YYYYMMDD2unix( $datex
	) {
	
	$year = substr($datex,0,4);
	$month= substr($datex,5,2);
	if ( $year<1970 ) return (0);
	if ( $month<1 OR $month>12 ) return (0);
	
	$unxstamp = mktime (  substr($datex,11,2),substr($datex,14,2), 0, $month,  substr($datex,8,2), $year );
	return ($unxstamp);
}

/**
 * transform EXCEL-format to YYYY-MM-DD
 * @param $dateString
 * @return date as YYYY-MM-DD
 */
function date_Excel2YMD( 
	$dateString
	) {
	global $error;

	if ( $dateString=="" ) return;
	$valcheckErr = NULL;
	if ( !preg_match( '/^[0-9]{2}.([0-9]{2}).([0-9]{4})$/', $dateString ) ) {
		$error->set( "_formatDate",1, $dateString."  needs format: 'DD.MM.YYYY'.");
		return;
	}
	$dateString = str_replace(".", "-", $dateString );
	$dateString = date_DDMMYYYY2YYYYMMDD( $dateString );
	return ( $dateString );
}

/*
 * convert days to seconds
 */ 
function date_unixDay2Sec($days) {
	$seconds = 3600 * 24 * $days;
	return $seconds;
}

/*
 * convert days to seconds
 */ 
function date_unixHour2Sec($hours) {
	$seconds = 3600 * $hours;
	return $seconds;
}

// cut time-part for a complete date-string (YYYY-MM-DD HH:MM)
function date_StrCutTime($datex) {
	$DATE_PURE_LEN = 10;
	if (strlen($datex)<=0) return;
	
	$puredate = substr($datex,0,$DATE_PURE_LEN);
	return $puredate;
}

/**
 * transform unix time stamp to:
 *   'dayEnd'   : END of the day
 *   'dayStart' : START of the day
 * @param  $unix
 * @return long $newTime
 */
function datex_shift($unix, $key) {
	$dar = getdate ($unix);
	
	switch ($key) {
		case 'dayStart':
			$newTime = mktime(0,0,0,$dar['mon'], $dar['mday'], $dar['year']);
			break;
		case 'dayEnd':
			$newTime = mktime(23,59,59,$dar['mon'], $dar['mday'], $dar['year']);
			break;
		default:
			throw new Exception('datex_shift() key:'.$key.' unknown');
	}
	return $newTime;
}

/**
 * validate date format of $date
 * @param string $date
 * @param string $format
 * @return boolean
 */
function date_validate($date, $format = 'Y-m-d H:i:s') {
	
	$d = DateTime::createFromFormat($format, $date);
	return $d && $d->format($format) == $date;
	
}

