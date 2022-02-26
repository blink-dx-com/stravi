<?php
/**
 * show diagram of time distribution
 * - used by glob.objtab.timedia.php
 * @package glob.objtab.timedia.inc
 * @namespace core::gui::objtab
 * @swreq UREQ:xxxxxxxxxxxx
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/gui/glob.objtab.timedia.inc 59 2018-11-21 09:04:09Z $
 */


require_once ('date_funcs.inc');
require_once ("f.sql_query.inc");	

class gObjtabTimedia {
	
var $timex; // time in unix-stamp:
			// [0] start time
			// [1] end time
var $parx; /*
  		 $parx["duration"] 
				1  : day
				7  : week 
				28 : month

		 $parx["inobj"]  Show extra object count for 
		 					... for "CONCRETE_PROTO > "W_WAFER_STEP"
		 $parx["assoc"] : show count of associated table
		 */

function __construct($tablename, $parx) {
	$this->tablename = $tablename;
	
	$this->MAX_POINTS  = 500;
	$this->accumulator = 'count(1)'; // the data, which will be analysed
	$this->DATE_COL    = "a.crea_date";
	if ( $tablename == "CCT_ACC_UP" OR  $tablename == "CCT_ACC_UP_VIEW" ) {
		 $this->DATE_COL = "MODI_DATE";
	}
	
	$this->parx = $parx;
} 

function setQuery($sqlAfter) {
	$this->sqlAfter = $sqlAfter;
}
function setDateCol($datecol) {
	$this->DATE_COL = $datecol;
}
function setAccumulator($accumulator) {
	$this->accumulator = $accumulator;
}

function setMaxPoint($max) {
	$this->MAX_POINTS  = $max;
}

function _info($key, $text) {
	echo "<font color=gray>INFO:</font> ".$key .': '.$text."<br>\n";
}

 
function img_get_width ( $tmp_obj_cnt, $max_val ) {

    return (floor($tmp_obj_cnt * $this->MAX_POINTS / $max_val));
}

function _firstOfNextMonth( $unixtag ) {
	
	$datearr = getdate  ( $unixtag );
	$datearr["mday"] = 1; // first day
	$datearr["mon"] =  $datearr["mon"] + 1;		
	if ( $datearr["mon"] > 12 ) {
		$datearr["mon"] = 1;
		$datearr["year"]++;
	}
	
	$retval = mktime  (0, 0, 0, $datearr["mon"], $datearr["mday"], $datearr["year"] );
	return ($retval);
}

function _dayInSecs($days) {
	return ( $days * (60*60*24) );
}

function prepare(&$sql) {
	$parx = $this->parx;
	$tablename=$this->tablename;
	
	$DATE_COL = $this->DATE_COL;

	$sqls = "SELECT min(".$sql->Sql2DateString($DATE_COL, 1).") cmdate FROM ".$this->sqlAfter;
	$sql->query($sqls);
	$sql->ReadRow();
	$times[0] = $sql->RowData[0];
	
	$sqls = "SELECT max(".$sql->Sql2DateString($DATE_COL, 1).") cmdate FROM ".$this->sqlAfter;
	$sql->query($sqls);
	$sql->ReadRow();
	$times[1] = $sql->RowData[0];
	
	echo "time: ".$times[0]." ... ".$times[1]."<br>";
	
	$timex[0] = date_dtsql2unix($sql, $times[0]);
	$timex[1] = date_dtsql2unix($sql, $times[1]) +  $this->_dayInSecs(1);
	
	if ($parx["duration"] == 28) {
		$datearr = getdate  ($timex[0]);
		$datearr["mday"] = 1; // first day
		$timex[0] = mktime  (0, 0, 0, $datearr["mon"], $datearr["mday"], $datearr["year"] );
	}
	if ($parx["duration"] == 7) {
		$datearr = getdate  ($timex[0]);
		$wday = $datearr["wday"];
		if ($wday==0) $wday = 7;
		if ($wday>1) {  // not a monday, make it monday
			$daydiff = $wday - 1;
			$seconddiff = $this->_dayInSecs($daydiff);
			$timex[0]   = $timex[0] - $seconddiff;
		}
	}
	
	$timex_diff = $timex[1]-$timex[0];
	
	$times_diff = $timex_diff / $this->_dayInSecs(1);
	echo "Difference: ".(int)$times_diff." days<br>\n";
	
	if ( $parx["inobj"]>0 ) {
		$this->_info ("Show extra object count for", "<b>Array-batch</b>");
		$this->setMaxPoint(200);
	}
	
	if ( $parx["assoc"]!=NULL ) {
		
		$this->pkname      = PrimNameGet2($tablename);
		$this->pkAssocName = PrimNameGet2($parx["assoc"]);
		$this->_info ("Show associated table", $parx["assoc"] );
		$this->setMaxPoint(300);
	}
	
	echo "<br>\n";
	
	$this->times_diff=$times_diff;
	$this->timex = $timex;
}

/**
 * get extra assoc-count 
 */
function _getXobj( &$sql, $sqlpre) {
	$sqls = "select count(1) from W_WAFER_STEP where CONCRETE_PROTO_ID in (".
			" select x.CONCRETE_PROTO_ID from ".$sqlpre.")";
	$sql->query($sqls);
	$sql->ReadRow();
	$tmpnum = $sql->RowData[0];
	return ($tmpnum);
}

/**
 * get count of assoc-table
 */
function _getAssocCount( &$sqlo, $sqlpre) {

	$assocTable = $this->parx['assoc'];
	$sqlsel = "count(1) from ".$assocTable." where ".$this->pkAssocName." in (".
				" select x.". $this->pkname ." from ".$sqlpre.
			")";
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$tmpnum = $sqlo->RowData[0];
	return ($tmpnum);
}

function _getData ( &$sql, &$sql2 ) {
	
	$timex 	= $this->timex;
	$parx 	= $this->parx;
	$DATE_COL = $this->DATE_COL;
	$SHOWMAX = 1000;
	
	$cnt	    =	0;
	$timex_now 	= $timex[0];
	$max_val 	= 0;
	$xobj_max_val = 0;
	$xobjcnt 	= NULL;
	$this->memit		= NULL; /* [0] - unix time-stamp */
	
	$CONST_dayInSeconds = $this->_dayInSecs(1);
	
	// number of shown time lanes
	$numUnits = ( $timex[1] -  $timex[0] ) / ($parx["duration"] * $CONST_dayInSeconds);
	
	
	$prgopt['objname']='rows';
	$prgopt['maxnum'] = $numUnits;
	$this->flushLib->shoPgroBar($prgopt);

	
	
	while ( $timex_now <= $timex[1] ) {
		
		if ( $cnt > $SHOWMAX ) {
			htmlInfoBox( "Too many time intervals", "More than max ".$SHOWMAX." time-intervals. Analysis stopped.", "", "WARN" );
			echo "<br>";
			break;
		}
		
		$timex_nowmax = $timex_now + $parx["duration"] * $CONST_dayInSeconds;
		if ($parx["duration"] == 28) {
			$timex_nowmax = $this->_firstOfNextMonth( $timex_now );
		}
		
		$timeLs[0] = date_unix2sql($timex_now);	   // first day
		$timeLs[1] = date_unix2sql($timex_nowmax); // before THIS day
		
		$sqlPre   = $this->sqlAfter. " AND ($DATE_COL>='".$timeLs[0]."' AND $DATE_COL<'".$timeLs[1]."')";
		$sqlsLoop = "SELECT ".$this->accumulator." FROM ".$sqlPre;
		$sql->query($sqlsLoop);
		$sql->ReadRow();
		$tmpnum = $sql->RowData[0];
		
		if ( $parx["inobj"]>0 ) {
			$xobjcnt = $this->_getXobj( $sql2, $sqlPre);
		}
		if ( $parx["assoc"]!=NULL ) {
			$xobjcnt = $this->_getAssocCount( $sql2, $sqlPre);
			if ($xobjcnt>$xobj_max_val) $xobj_max_val = $xobjcnt;
		}
		
		$this->memit[] = array( $timex_now, $tmpnum, $xobjcnt );
		
		if ($tmpnum>$max_val) $max_val = $tmpnum;
		
		
		$timex_now=$timex_nowmax;
		
		$this->flushLib->alivePoint($cnt);
		$cnt++;
	}
	$this->flushLib->alivePoint($cnt,1); // finish
	
	$this->maxval = $max_val;
	$this->XobjMaxVal = $xobj_max_val;
}

function _oneRow($datarr, $bgcolor) {
	
	if ($bgcolor!=NULL) $bgcolorTag = 'bgcolor='.$bgcolor;
	else $bgcolorTag = '';
	echo "<tr ".$bgcolorTag.">";

	foreach( $datarr as $dummy=>$val) {
		echo '<td>'.$val.'</td>';
	}
	reset($datarr);
	echo "</tr>\n";
}

function _showGraphs( &$sql) {	
	$parx 	= $this->parx;
	$this_showXobj = 0;
	
	echo "<table cellpadding=0 cellspacing=0 border=0 class=\"xdataTab\" >";
	echo "<tr bgcolor=#D0D0FF>";
	echo "<th>Info</th>";
	echo "<th>&nbsp;</th>"; // space
	echo "<th>Date</th>";
	echo "<th>Cnt&nbsp;</th>";
	echo "<th>Graph</th>\n";
	if ( $parx["inobj"]>0 or $parx["assoc"]>NULL ) {
		echo "<th  bgcolor=#C0FFFF>extra<br>object</th>";
		echo "<th  bgcolor=#C0FFFF>extra Graph</th>";
		$this_showXobj = 1;
	}
	echo "</tr>\n";
	
	
	$lastmonth = 0;
	$lastyear  = 0;
	
	$sumObj = 0;
	$sumXobj= 0;
	
	foreach( $this->memit as $dummy=>$infarr) {
		
		$outstr="";
		$XobjOutstr=NULL;
		
		$timeUnx = $infarr[0];
		$loopnum = $infarr[1];
		$xobjcnt = $infarr[2];
		
		$tmpnum = $loopnum;
		if ($loopnum>0) {
			$img_width = $this->img_get_width ( $loopnum, $this->maxval );
			$outstr = '<img src="images/point.gif" height="5" width="'.$img_width.'">';
		} else {
			$tmpnum = "";
		}
		
		if ($xobjcnt>0) {
			$img_width = $this->img_get_width ( $xobjcnt, $this->XobjMaxVal );
			$XobjOutstr = '<img src="images/point.gif" height="5" width="'.$img_width.'">';
		} else {
			$xobjcnt = "";
		}
		
		
		
		$timecolorTag = "";
		$timecolor    = "";
		$timecolorInfo= "";
		
		$timeOut = date("Y-m-d", $timeUnx);
		$datearr = getdate  ($timeUnx);
		
		
		if ( ($parx["duration"]==7) OR ($parx["duration"]==1) ) { // week or day
			if ( $datearr["mon"] != $lastmonth ) {
				$timecolor = "#C0C0F0";
				$timecolorInfo = $datearr["mon"];
			}
			$lastmonth = $datearr["mon"];
		}
		
		
		if ( $datearr["year"] != $lastyear ) {
			$timecolor = "#E080E0";
			$timecolorInfo = $datearr["year"];
		}
		$lastyear = $datearr["year"];
		
		
		// $timex[0] = mktime  (0, 0, 0, $datearr["mon"], $datearr["mday"], $datearr["year"] );
		
		$bgcolorTag   = "";
		if ($timecolor!="") {
			$timecolorTag = " bgcolor=\"".$timecolor."\"";
			$bgcolorTag   = " bgcolor=\"#EFEFEF\"";
		}
		
		echo "<tr ".$bgcolorTag.">";
		echo "<td ".$timecolorTag.">$timecolorInfo</td>";
		echo "<td>&nbsp;</td>";
		echo "<td><font color=gray>".$timeOut."&nbsp;&nbsp;</font></td>".
			"<td align=right>".$tmpnum."&nbsp;</td>";
		echo	"<td>".$outstr."</td>";
		
		if ( $this_showXobj ) {
			echo "<td>".$xobjcnt."</td>";
			echo "<td>".$XobjOutstr."</td>";
			$sumXobj = $sumXobj + $xobjcnt;
		}
		
		echo "</tr>\n";
		
		$sumObj = $sumObj + $loopnum;
	}
	reset ($this->memit);
	
	
	$datarr=array('-','','<b>Summary</b>',$sumObj,'');
	if ($this_showXobj) {
		$datarr[] = $sumXobj;
		$datarr[] = '';
		
	} 
	$this->_oneRow($datarr, '#D0D0FF');
	
	echo "</table>\n";
}

function doloop ( &$sql, &$sql2 ) {
	$this->_getData($sql, $sql2);
	$this->_showGraphs($sql);
	
}

function setProgBarLib(&$flushLib) {
	$this->flushLib=&$flushLib;
}

function formshow(&$sqlo) {
	require_once ('func_form.inc');
	
	$parx = $this->parx;
	$tablename = $this->tablename;
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF']."?tablename=".$tablename;
	$initarr["title"]       = "Options";
	$initarr["submittitle"] = "Submit";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["tablename"]     = $tablename;

	$formobj = new formc($initarr, $hiddenarr, 0);

	$tmparr = array("1"=>"day", "7"=>"week", "28"=>"month");
	
	$fieldx = array ( 
			"title" => "Scale", 
   			"name"  => "duration",
			"object" => "select",
			"val"   => $parx["duration"], 
   			"inits" => $tmparr,
			"notes" => "scale unit" );
	$formobj->fieldOut( $fieldx );
	
	
	if ($tablename=="CONCRETE_PROTO") {
		$fieldx = array ( 
			"title" => "Related batches", 
   			"name"  => "inobj",
			"object"=> "checkbox",
			"val"   => $parx["duration"], 
   			"notes" => "Show also number of related array-batches" );
		$formobj->fieldOut( $fieldx );
	} 
	
	$assocarr2 = get_assoc_tables2( $sqlo, $tablename );
	reset ($assocarr2);
	$assocarr=NULL;
	foreach( $assocarr2 as $dummy=>$tmparr) {
		$assocarr[$tmparr[0]] = $tmparr[1];
	}
	reset ($assocarr2); 
	
	if ( sizeof($assocarr) ) {
		$fieldx = array ( 
				"title" => "assoc tables", 
	   			"name"  => "assoc",
				"object"=> "select",
				"val"   => $parx["assoc"], 
				"inits" => $assocarr,
	   			"notes" => "Show also number associated elements" );
		$formobj->fieldOut( $fieldx );
	}
	$formobj->close( TRUE );

}

}
