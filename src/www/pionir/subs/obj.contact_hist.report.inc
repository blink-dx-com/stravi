<?php
require_once 'o.DB_USER.subs.inc';
/**
 * service log report functions
 * @package obj.contact_hist.report.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0000894: o.CHIP_READER > Verwaltung geplanter Services/Stillstaende 
 */

// require_once ("o.CHIP_READER.logs.inc"); 
 
class oCONTACT_HIST_logSh {
	
function __construct() {
	$this->colorblue ="#CCDDEE";
	$this->fullcolspan = 3;
	
	$this->pageNow = 1;
	$this->cssKeys = array('Repaired'=>'doc-txRepaired', 'Problem'=>'doc-txProblem', 'STOP'=>'doc-txStop', 'Working'=>'doc-txWork');
}

function setPageNow($pageNow) {
	$this->pageNow = $pageNow;
}

function getCss() {
	
	
	$css ="<style type=\"text/css\">\n".
		  ".doc-lx1 { \n".
		"  margin-left:9px; margin-right:9px;\n".
		"} \n".
		".doc-tx1 { font-weight:bold; color: #AABBFF}\n".
		".doc-tx2 { white-space: nowrap; }\n".
		".doc-hr  {
		height:1px; border:none; color:#CCDDEE; background-color:#CCDDEE;
		 }".
		"</style>\n";
	return ($css);
}

function tableStart( $headtext=NULL ) {
	echo "<table cellpadding=1 cellspacing=1 border=0 bgcolor=".$this->colorblue." width=800 class=\"doc-lx1\"><tr><td bgcolor=#FFFFFF>";
	echo "<table cellpadding=1 cellspacing=1 border=0 width=100%>\n";
	echo '<tr><td colspan='.$this->fullcolspan.'><b><span style="color:'.$this->colorblue.'">LOGS</span></B> &nbsp;&nbsp;&nbsp;'.
			 $headtext."</td></tr>\n";
}

function tableEnd() {
	echo "</table>\n";
	echo "</td></tr></table>\n";
}

/**
 * - show one log row
 * - support key coloring
 * @param $RowData
 * @param array $option 
 * 	  'withContact' : 0,1
 * 	  'editallow': [0], 1 : show EDIT button for the entry ?
 * @return unknown_type
 */
function oneRow( &$sqlo, &$RowData, $option=NULL ) {
	
	$fullcolspan = $this->fullcolspan;
	$id	   = $RowData["CONTACT_ID"];
	$pos   = $RowData["POS"]; 
	$name  = $RowData["SUBJECT"]; 
	$datex = $RowData["EVENT_DATE"]; 
	$userid = $RowData["DB_USER_ID"];
	
	
	$trStyle=NULL;

	
	$userx = $userid;
	if ($userid) {
		$userx= DB_userC::getFullname( $sqlo, $userid );
	}
	
	
	echo '<tr><td colspan='.$fullcolspan.'><hr class="doc-hr"></td></tr>'."\n";
	echo "<tr valign=top ".$trStyle.">";
	echo "<td width=60%><b>"; // SUBJECT
	
	
	// edit.tmpl.php?t=REA_LOG&id=$id&primasid[1]=$pos\
	$editUrl = 'edit.tmpl.php?t=CONTACT&id='.$id.'&tabArgu[POS]='.$pos;
	if ($this->pageNow>0) {
		$editUrl .= '&backPage='.$this->pageNow;
	}
	
	echo $name.'</b>';
	echo '</td>';
	echo '<td class="doc-tx2">';
	if ($option['editallow']) echo '<a href="'.$editUrl.'"><img src="images/but.edit.gif"></a> ';
	echo $datex.'</td><td>'.$userx.'</td>'; // DATE + USER
	
	echo "</tr>\n";
	if ( $RowData["NOTES"]!="" ) { 
		echo "<tr><td colspan=".$fullcolspan.">";
		$tmpstr = htmlspecialchars($RowData["NOTES"]);
		$tmpstr = str_replace("\n", "<br>\n", $tmpstr);
		//if ( strlen($RowData[4])>120)
		//	$tmpstr = substr($RowData[4],0,120)."...";
		//else  $tmpstr = $RowData[4];
		//echo htmlspecialchars($tmpstr);
		echo $tmpstr;
		echo "</td></tr>\n";
	} 
}

}
