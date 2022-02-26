<?php
/**
 * get BO creation statistics 
 * @package bo.time_info.php
 * @swreq UREQ:xxxxxxxxxxxx
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param    
 * $go : 0,1
   $parx["days"] : number of days
   $parx["showtable"] : show statistics as table
   $parx["showCompact"]
   $parx[showgraphDetail
   $parx[opt_show] = "", "week"
   $parx[tablename] : OPTIONAL
   $parx["bo_action"]  "creation", modification
 * @version $Header: trunk/src/www/pionir/rootsubs/bo.time_info.php 59 2018-11-21 09:04:09Z $
 */



session_start(); 


require_once ('reqnormal.inc');
require_once('javascript.inc');

class gBoTimeInfo {
	
	var $day_cnt; /* array of counts */
	
	
    function __construct( $parx, $go ) {
    	global $_s_i_table;
    	
    	$this->parx = $parx;
    	$this->DATE_FORMAT    = 5;
    	$this->PHP_DATE_FORMAT= 'Y-m-d';
    	
    	
    	$this->tablename = $this->parx["tablename"];
    	$this->ob_arr   = NULL;  
    	reset($_s_i_table);
    	foreach( $_s_i_table as $tmp_table=>$dummy) {
    		if ( cct_access_has2($tmp_table) and !table_is_view($tmp_table) )
    			$this->ob_arr[$tmp_table] = tablename_nice2($tmp_table);
    	}
    	
    	asort($this->ob_arr);
    	reset ($this->ob_arr);
    	
    	$this->time_now       = mktime(0,0,0, date('m'), date('d'), date('Y'));
    	$this->time_today      = $this->time_now;
    	
    	if ($parx["bo_action"] == 'modification') {
    	    $this->DATE_COL    = 'MOD_DATE';
    	    $this->action_text = 'modification';
    	} else {
    	    $this->DATE_COL    = 'CREA_DATE';
    	    $this->action_text = 'creation';
    	}
    }
    
    function form1() {
    	echo '<blockquote>';
    	echo '<form name="test2" ACTION="',$_SERVER['PHP_SELF'],'" METHOD="POST">';
    	echo '<input type="hidden" name="go" value="1">';
    	echo "<table border=0 cellspacing=1 cellpadding=0><tr><td>"; // light blue bgcolor
    	echo '<table><tr><td>Number of days:</td>';
    	echo '<td><input type="text" name="parx[days]" value="14"></td></tr>';
    
    	echo '<tr><td>Display </td>';
    	echo '<td>'.
    		 '<input type="radio" name="parx[bo_action]" value="creation" checked> creation date &nbsp;&nbsp;&nbsp;';
    	echo '<input type="radio" name="parx[bo_action]" value="modification"> modification date</td></tr>';
    
    	echo '<tr><td>Show data in Table</td>';
    	echo '<td><input type="checkbox" name="parx[showtable]" value="1" checked> <I>(show elements per object type)</I></td></tr>';
    	
    	echo '<tr><td>Show detailed table graph</td>';
    	echo '<td><input type="checkbox" name="parx[showgraphDetail]" value="1" checked> <I>(each object type shown as colored bar graph)</I></td></tr>';
    
    	echo '<tr><td>Compact view</td>';
    	echo '<td><input type="checkbox" name="parx[showCompact]" value="1"></td></tr>'; 
        
        echo '<tr><td>MEAN week values</td>';
    	echo '<td><input type="checkbox" name="parx[opt_show]" value="week"></td></tr>';
    	
    	echo '<tr><td>only specific object type (optional)</td>';
    	echo '<td><select name="parx[tablename]">';
    	echo '<option value=""> --- select objects ---</option>';
    
    	foreach( $this->ob_arr as $tmp_table=>$tmp_table_nice)
    		echo '<option value="',$tmp_table,'">',$tmp_table_nice,'</option>';
    	
    
    	echo '</select><P></td></tr>';
    		
    	echo '<tr><td></td><td><input type="submit" class="yButton" value="Create statistics"></td></tr></table>';
    	echo "</td></tr></table>\n";
    	echo "</form>";
    	
    }
    
    function simple_info_get( &$sql ) {
    	global $error;
    	
    	$parx      = $this->parx;
    	$tablename = $this->tablename;
    	
    	
    	echo '<P>',ucfirst($this->action_text),' statistics of <b>';	
    	if ($tablename) { 
    		echo tablename_nice2($tablename);
    	} else {
    		echo 'all business objects';
    	}
    	echo ':</B><br>';
    	
    	
    	$time_start     = mktime(0,0,0, date('m'), date('d') - $parx["days"] + 1, date('Y'));
    	
    	echo 'Displaying objects, which have ',$this->action_text,' date after ',
    		date('Y-m-d', $time_start),'.<br>';
      
    	$sqls = 'SELECT '.$sql->Sql2DateString($this->DATE_COL, $this->DATE_FORMAT).' cmdate, COUNT(1) num'.
            ' FROM cct_access WHERE '.$this->DATE_COL.' >= '.$sql->Timestamp2Sql($time_start);
      
    	if ($tablename) {
    		$this->whereClauseXtra    = ' AND table_name = '.$sql->addQuotes($tablename);
    		$this->whereClauseXtraUrl = urlencode($this->whereClauseXtra);	
    		$sqls              .= $this->whereClauseXtra;	
    	} else {
    		$this->whereClauseXtra    = '';
    		$this->whereClauseXtraUrl = '';
    	}
    	
    	$this->sqls =  $sqls . ' GROUP BY '.$sql->Sql2DateString($this->DATE_COL, $this->DATE_FORMAT);
    	
    	$this->day_cnt  = array();
    	
    	$sql->query($this->sqls);
    	if ($error->printLast()) htmlFoot();
    	
    	while ( $sql->ReadArray() ) {
    	    $this->day_cnt[] = $sql->RowData;
    	}
    	
    	if ( empty($this->day_cnt) ) {
    	    
    	    
    		echo 'No object found which meets conditions.<br>';
    		
    		$sqls = 'SELECT '.$sql->Sql2DateString('MAX('.$this->DATE_COL.')', $this->DATE_FORMAT).' FROM cct_access';
    		if ($this->whereClauseXtra != '') $sqls .= ' WHERE 1=1 '.$this->whereClauseXtra;
    		$sql->query($sqls);	
    		if ($sql->ReadRow()) {
    			if ($tablename)
    				echo 'Newest ',$this->action_text,' date of '.tablename_nice2($tablename).' is ',$sql->RowData[0],'.';
    			else
    				echo 'Newest ',$this->action_text,' date of a business object is ',$sql->RowData[0],'.';
    		}
    		htmlFoot();
    	}
    }
    
    function simple_info_show() {
        
        $parx = $this->parx;
    	
        $max_val = 0;
        if (is_numeric($this->day_cnt['NUM'])) {
            $max_val   = max($this->day_cnt['NUM']);
        }
    	
    	if ( $parx["opt_show"]=="week" ) { 
    		$max_val         = $max_val * 3;  //TBD: this value could be changed
    	}
    	
    	$this->max_val = $max_val;
    	$this->MAX_POINTS      = 800;
    	
    	// $time_tomorrow   = mktime(0,0,0, date('m', $this->time_today), date('d', $this->time_today) + 1, date('Y', $this->time_today));
    	
    	if ($parx["showCompact"]) {
    		echo "<B>Compact show mode (no weekends)</B><br>";
    		echo '<img src="../images/0.gif" height="2" width="5">';
    		echo '<img src="../images/point.gif" height="2" width="',$this->MAX_POINTS,'"> max = ',$max_val,'<P>';
    	}  
    	if ($parx["opt_show"]) { 
    		echo "<B>Show MEAN week values</B><br>\n";  
    	}
    }
    
    /**
     * show bar-graph by date
     */
    function showGraph( &$sql )  {
    	
    	$parx       = $this->parx;
    	$time_today = $this->time_today;
    	
    	$was_weekend = 0; // remember that time
    	$week_cnt    = 0;
    	$graph_normal_height=15;
    	$time_tomorrow=0;  //TBD: is this correct ?
    	
    	for ($i = $parx["days"]; $i--; $i > 0) {
    		$outstr = "";
    		$time_today_user = strtoupper(date($this->PHP_DATE_FORMAT, $time_today));
    		$today_weekday   = date('w', $time_today);
    		$is_weekend=0;
    		
    		if (($today_weekday == 0) or ($today_weekday == 6)) $is_weekend=1;
    		$txt_color       = ($is_weekend) ? ' color="#DDDDDD"' : ''; // other color for weekend-days
    		$keys_in_daylist=array();
    		if (!empty($this->day_cnt['CMDATE'])) {
    		    $keys_in_daylist = array_keys($this->day_cnt['CMDATE'], $time_today_user);
    		}
    		$tmp_obj_cnt     = isset($keys_in_daylist[0]) ? $this->day_cnt['NUM'][$keys_in_daylist[0]] : 0; // number of created/modified objects
    		
    		if ($tmp_obj_cnt) {
    			$img_width       = $this->img_get_width ( $tmp_obj_cnt );
    			$date_search_str = urlencode('x.'.$this->DATE_COL.' >= '.$sql->Timestamp2Sql($time_today).' AND x.'.$this->DATE_COL.' < '.$sql->Timestamp2Sql($time_tomorrow));
    			$outstr = $outstr . "<a href='javascript:openwin(\"CCT_ACCESS\", \"\", \"&condclean=1&tableSCond=".$date_search_str.$this->whereClauseXtraUrl."\", \"\")'>"; // mac, leave it this way!!
    		} else {
    			$img_width = 0;
    		}
    		$show_now=1;   
    		if ($is_weekend)  $was_weekend = 1;
    		
    		if ($parx["showCompact"]) {
    			
    			if ($is_weekend) {// on weekend
    				$show_now = 0;
    			} else {
    				if ($was_weekend) {
    					$outstr = $outstr . '<img src="../images/point_gray.gif" height="2" width="5" border="0">';
    				} else {
    					$outstr = $outstr . '<img src="../images/0.gif" height="2" width="5" border="0">';
    				}
    			}
    			if ($tmp_obj_cnt)
    				$outstr = $outstr . '</a><img src="../images/point.gif" height="2" width="'.$img_width.'">';
    				
    		} else { // not compact
    			$outstr = $outstr . '<font size=-1'.$txt_color.'>'.$time_today_user.'</font></a>';
    			if ( $parx["opt_show"]=="week" ) {
    				if ( !$is_weekend && $was_weekend ) {
    					if ($week_cnt) {
    						$img_width       = $this->img_get_width ( $week_cnt );
    						$outstr = $outstr . ' <img src="../images/point.gif" height="'.$graph_normal_height.'" width="'.$img_width.'">';
    						$outstr = $outstr . ' <font size="-1">'.$week_cnt.'</font>';
    					}
    				} else $show_now = 0;
    			} else {
    				if ($tmp_obj_cnt) {
    					$outstr = $outstr . ' <img src="../images/point.gif" height="'.$graph_normal_height.'" width="'.$img_width.'">';
    					$outstr = $outstr . ' <font size="-1">'.$tmp_obj_cnt.'</font>';
    				}
    			}
    		}
    		if ($show_now) echo $outstr . '<br>';
    		else {
    		}
    		// now switch to the day before
    		$time_tomorrow = $time_today;
    		$time_today = mktime(0,0,0, date('m', $time_today), date('d', $time_today) - 1, date('Y', $time_today));
    		
    		if ( $was_weekend && !$is_weekend ) $week_cnt = 0;  // reset counter
    		if ( !$is_weekend ) $was_weekend = 0; // switch off 
    		
    		$week_cnt = $week_cnt + $tmp_obj_cnt;
    	}
    	echo 'last date: ',$time_today_user,'<P>';
    }
    
    /**
     * do detail analysis: many tables, stats for each table
     * @global result: $this->time_details
     * @param object $sql
     * @param object $sql2
     */
    function detail_analysis(&$sql, &$sql2) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	/**
    	 * @var $time_details: array of DATE => array(TABLE=>cnt)
    	 */
    	$this->time_details = array();
    	
    	$parx = $this->parx;
    	$time_today      = $this->time_now;
    	$time_tomorrow   = mktime(0,0,0, date('m', $time_today), date('d', $time_today) + 1, date('Y', $time_today));
    	$daysnow = $parx["days"];
    	
    	echo ' ... analysis progress: ';
    	while ( $daysnow > 0) {
    		
    		$onedate_arr=array();
    	
    		$daysnow--;
    		$time_today_user = date($this->PHP_DATE_FORMAT, $time_today);
    		$today_weekday   = date('w', $time_today);
    		$obj_arr_res     = array();
    
    	
    		$sql2->query('SELECT table_name, COUNT(1) FROM cct_access'.
    				' WHERE '.$this->DATE_COL. ' >= '.$sql->timestamp2sql($time_today).
    				' AND '.$this->DATE_COL.' < '.$sql->timestamp2sql($time_tomorrow).
    				' GROUP BY table_name');
    		if ($error->Got(READONLY))  {
    			$error->set( $FUNCNAME, 1, 'error during day '. $time_today_user);
    			return;
    		}
    		while ($sql2->ReadRow()) {
    			$obj_arr_res[$sql2->RowData[0]] = $sql2->RowData[1];
    		}
    		
    		$this->time_details[$time_today_user] = $obj_arr_res;
    	
    		// now switch to the day before
    		$time_tomorrow = $time_today;
    		$time_today    = mktime(0,0,0, date('m', $time_today), date('d', $time_today) - 1, date('Y', $time_today));
    		
    		echo '.';
    	}
    	echo ' done.<br>';
    	
    }
    
    /**
     * overview database-TABLE versus TIME
     * @global INPUT: $this->time_details
     */
    function showTable( &$sql, &$sql2) {
    	global $error;
    	$parx = $this->parx;
    	
    	echo '<P><B>',ucfirst($this->action_text),' statistics for business objects seperated by type</B><P>';
    	
    	if (!is_array($this->time_details) or !sizeof($this->time_details)) {
    		htmlErrorBox('ERROR', 'No input data!');
    		return;
    	}
    	
    	echo '<table cellspacing="1" border="0" bgcolor="#ffffff">';
    	
    	echo '<tr valign="bottom" bgcolor="#EEEEEE">';
    	echo '<th>day</th>';
    	$bgcolor = 'FFFFFF';
    
    	
    	foreach( $this->ob_arr as $tmp_table=>$dummy) {
    		echo '<td><font size="-1">';
    		$textnice = tablename_nice2($tmp_table);
    		echo "<img src=\"../f.imgtxt.php?f=14&t=".$textnice."\">";
    		echo '</font></td>';
    	}
    	
    	echo '</tr>';
    	
    	echo '<tr bgcolor="#EEEEEE" >';
    	echo '<td></td>';
    
    	foreach( $this->ob_arr as $tmp_table=>$dummy) {
    		echo '<td>';
    		$icon = htmlObjIcon($tmp_table, 1);
    		echo "<img src=\"".$icon."\">";
    		echo '</td>';
    	}
    	reset ($this->ob_arr);
    	echo "</tr>\n";
    
    	$time_today      = $this->time_now;
    	// $time_tomorrow   = mktime(0,0,0, date('m', $time_today), date('d', $time_today) + 1, date('Y', $time_today));
    	$daysnow = $parx["days"];
    	
    	
    	while ( $daysnow > 0) {
    		
    		$daysnow--;
    		$time_today_user = date($this->PHP_DATE_FORMAT, $time_today);
    		$today_weekday   = date('w', $time_today);
    		$txt_color       = (($today_weekday == 0) or ($today_weekday == 6)) ? ' color="#cccccc"' : 'color="#000000"'; // other color for weekend-days
    		$obj_arr_res     = array();
    		$bgcolor         = 'FFFFFF';
    
    
    		echo '<tr align="middle">';
    		echo '<td nowrap><font ',$txt_color,'>',$time_today_user,'</font></td>';
    
    		$obj_arr_res = $this->time_details[$time_today_user];
    		
    		
    		foreach( $this->ob_arr as $tmp_table=>$dummy) {
    			$res     = empty($obj_arr_res[$tmp_table]) ? '&nbsp;' : $obj_arr_res[$tmp_table];
    			$bgcolor = (($bgcolor == 'CCCCCC') ? 'FFFFFF' : 'CCCCCC');
    			echo '<td bgcolor="#',$bgcolor,'">',$res,'</td>';
    		}
    		
    		
    		echo '</tr>';
    		// now switch to the day before
    		// $time_tomorrow = $time_today;
    		$time_today    = mktime(0,0,0, date('m', $time_today), date('d', $time_today) - 1, date('Y', $time_today));
    	}
    	echo '</table>';
    }
    
    private function _graphDetailOneDay($time_today_user) {
    	$outstr = "";
    	
    	$is_weekend = $this->_is_weekend($time_today_user);
    	
    	
    	if ($is_weekend)  $was_weekend = 1;
    	$txt_color       = ($is_weekend) ? ' color:#DDDDDD;' : ''; // other color for weekend-days
    	
    	$obj_arr_res = $this->time_details[$time_today_user];
    	
    	$outstr = $outstr . '<span style="font-size:0.8em;'.$txt_color.'">'.$time_today_user.'</span> ';
    	
    	foreach( $this->ob_arr as $tmp_table=>$dummy ) {
    		
    		$tmp_obj_cnt = $obj_arr_res[$tmp_table];
    		if ($tmp_obj_cnt) {
    			
    			$img_width = $this->img_get_width($tmp_obj_cnt);
    			$objcolor = globTabMetaByKey( $tmp_table, 'COLOR');
    			if ( $objcolor=="" ) $objcolor="#E0E0E0";
    			 
    			$outstr = $outstr . '<div style="display:inline-block; background-color:'.$objcolor.
    				'; width:'.$img_width.'px; height:10px;" title="'.$tmp_table.'; cnt:'.$tmp_obj_cnt.'"></div>';
    		}
    	}
    	
    	echo $outstr . "<br>\n";
    	
    	
    }
    
    private function _is_weekend($time_today_user) {
    	
    	$time_today = strtotime($time_today_user);
    	$today_weekday   = date('w', $time_today);
    	$is_weekend=0;
    	
    	if (($today_weekday == 0) or ($today_weekday == 6)) $is_weekend=1;
    	
    	return $is_weekend;
    }
    
    function showDetailGraph( &$sql) {
    	$parx       = $this->parx;
    	$time_today = $this->time_today;
    	
    	//$was_weekend = 0; // remember that time
    	//$week_cnt    = 0;
    	
    	for ($i = $parx["days"]; $i--; $i > 0) {
    		
    		$time_today_user = strtoupper(date($this->PHP_DATE_FORMAT, $time_today));
    		//$today_weekday   = date('w', $time_today);
    		//$is_weekend=0;
    	
    		//if (($today_weekday == 0) or ($today_weekday == 6)) $is_weekend=1;
    		
    		
    		$this->_graphDetailOneDay($time_today_user);
    	
    
    		// now switch to the day before
    		// $time_tomorrow = $time_today;
    		$time_today = mktime(0,0,0, date('m', $time_today), date('d', $time_today) - 1, date('Y', $time_today));
    	
    		// if ( $was_weekend && !$is_weekend ) $week_cnt = 0;  // reset counter
    		//if ( !$is_weekend ) $was_weekend = 0; // switch off
    	
    		//$week_cnt = $week_cnt + $tmp_obj_cnt;
    	}
    	echo 'last date: ',$time_today_user,'<P>';
    }
    
    function img_get_width ( $tmp_obj_cnt ) {
        if ($this->max_val) $out = (floor($tmp_obj_cnt * $this->MAX_POINTS / $this->max_val));
        else $out=0;
        return $out;
    }
	
}
   


// ------------------------------------

$title         = 'Business objects creation/modification statistics';
$obj_max_count = 4000;
$go = $_REQUEST['go'];
$parx = $_REQUEST['parx'];

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();

$infoarr			 = NULL;
$infoarr["scriptID"] = "bo.time_info.php";
$infoarr["title"]    = $title;
$infoarr["title_sh"]    = "BO statistics";
$infoarr["form_type"]= "tool";
$infoarr['help_url'] = 'creation_statistics.html';
$infoarr["locrow"] = array( array("rootFuncs.php", "Administration") );

$pagelib = new gHtmlHead();
$pagelib->_PageHead ( $infoarr["title"],  $infoarr );
js__openwin();
$pagelib->_startBody( $sql, $infoarr );



$tablename   = $parx["tablename"];

$mainlib = new gBoTimeInfo($parx, $go);


echo "[<a href=\"".$_SERVER['PHP_SELF']."\">Refine</a>] Created or modified business objects of the last <i>n</i> days.<br>\n";

if (empty($go)) {
	
	$mainlib->form1();
	htmlFoot('</blockquote><hr>');
}

if (empty($parx["bo_action"]))   $parx["bo_action"]   = 'creation';
if (empty($parx["days"]))        $parx["days"]        = 60;
if (empty($tablename))   $tablename   = NULL;
if (empty($parx["showtable"]))   $parx["showtable"]   = 0;
if (empty($parx["showCompact"])) $parx["showCompact"] = 0;


$mainlib->simple_info_get( $sql );
//$mainlib->simple_info_show( $sql);
//$mainlib->showGraph($sql);

	
if ( !$tablename and ($parx["showtable"] or $parx["showgraphDetail"])) { // show table of all objects only when no single table is select and user ckecked show-table
	$mainlib->detail_analysis($sql, $sql2);
	if ($parx["showtable"]) $mainlib->showTable( $sql, $sql2);
	if ($parx["showgraphDetail"]) {
		$mainlib->showDetailGraph( $sql);
	}
}
echo '<P>done.<P>';
htmlFoot();

