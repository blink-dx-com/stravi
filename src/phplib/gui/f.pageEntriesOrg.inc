<?php
/**
 * - manage showing many entries per page, wind forward, back
 * @namespace core::gui::page
 * @package f.pageEntriesOrg.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/gui/f.pageEntriesOrg.inc 59 2018-11-21 09:04:09Z $
 * @example:
 * 
    $pageNavLib = new f_pageEntriesGui();
    $showinf= array( 'startShow'=> $_REQUEST['page'], 'foundEntries'=>$auditCnt );
    $parx=NULL;
    $info = $pageNavLib->setParams( $showinf, $parx );
    $cnt = 0;
	$moreExists = 0;
	$startShow = $info['start'];
	$endShow   = $info['end'];
	
	while ( $sql->ReadArray() ) {
		$showit = 0;
		
		$thisData = $sql->RowData;
		if ( $cnt >= $startShow ) $showit=1;
		if ( $cnt > $endShow )   {
			$moreExists = 1;
			break;
		}
		
		if ($showit) {
			$this->oneEntryOut($sql2, $thisData);
		}
		$cnt++;
	}
	
	$pageNavLib->setLastShownEntry($cnt-1);
	$desturl='showinfo.php?page=';
	$pageNavLib->showEntryNav ( $desturl, $pageNow );
	
 */
class f_pageEntriesGui {

	/* main info array
	 * "startShow"     : starting page number, start with 1, can be 0
	 * "entryStartCnt" : starting count (default:0)
	 * "lastshow"	   : last shown cnt; e.g. 19
	 * "foundEntries"  : number of table entries
	*/
	var $showinf; 
	
	var $parx; /* 
		user settings
		"entryPerPage"
	*/
	
/**
 * set parameters
 * @param array $showinf 
 * 	 'startShow' : page-number; fallback: 0 will be 1
 * @param array $parx
 * @return array(start entry, end entry cnt) : counts starting at 0
 */
function setParams( $showinf, $parx ) {
	
	if ( $parx['entryPerPage']<=0 ) $parx['entryPerPage'] = 20; 
	
	if ( $showinf['startShow']<=0 ) $showinf['startShow'] = 1;
	$viewPage = $showinf['startShow'];
	if (!$viewPage) $viewPage=0;
	$startEntryCnt = ($viewPage-1)  * $parx['entryPerPage'];
	$endEntryCnt   = $startEntryCnt + $parx['entryPerPage'] - 1;
	$showinf["lastshow"] = $endEntryCnt;
	
	$showinf["entryStartCnt"] = $startEntryCnt;
	
	$this->showinf = $showinf;
	$this->parx    = $parx; 
	
	
	return array('start'=>$startEntryCnt, 'end'=>$endEntryCnt);
}

function setLastShownEntry($cnt) {
	$this->showinf["lastshow"]=$cnt;
}
	
/**
 * show navigation at end of an entry-list
 * @param $desturl : destination URL, must contain '?' or '&', the tool appends the page-number
 * @param array $options
 *  'addtext' : extra text at the end
 * @return -
 */
function showEntryNav ( $desturl, $options=NULL  ) {

	$pageNow = $this->showinf['startShow'];
	echo '&nbsp;&nbsp;&nbsp;<span style="color:#808080">Showing: <b>'.($this->showinf["entryStartCnt"]+1)."</B>...<b>".($this->showinf["lastshow"]+1).
		"</B> of <b>".$this->showinf["foundEntries"].'</b> entries';
	echo '&nbsp;&nbsp;&nbsp;&nbsp;Page: </span>';
	
	$pageCnt         = 1;
	$firstPoints     = 0;
	$pagesShowAround = 5;
	
	if ($pageNow<=5) $pagesShowAround = 10 - $pageNow + 1;
	
	$pagemax = ceil ($this->showinf["foundEntries"] / $this->parx["entryPerPage"]);
	
	if ( $pagemax>1 AND $pageNow>1) echo  "&nbsp;<a href=\"".$desturl.($pageNow-1)."\">&lt;&lt;prev</a>&nbsp;&nbsp;";
	
	while ($pageCnt <= $pagemax) {
	
		$pagePrintFlag=0;
		$pageOut = "<a href=\"".$desturl.$pageCnt."\">".$pageCnt."</a>";
		// echo "DEBBB: " . abs($pageCnt-$pageNow).":$pagesShowAround ";
		if ( $pageCnt==$pageNow ) $pageOut = "<b>" . $pageOut . "</b>";
		
		if ( (abs($pageCnt-$pageNow)<$pagesShowAround) OR ($pageCnt==1)  ) {
				echo $pageOut . " ";
				$pagePrintFlag=1;
		}
		
		if ( ($pageCnt<$pageNow) AND !$pagePrintFlag AND !$firstPoints) {
			echo " ... ";
			$firstPoints=1;
		}
		
		if ( ($pageCnt>$pageNow) AND !$pagePrintFlag ) {
			echo " ... ";
			break;
		}
		$pageCnt++;
	}
	if ( $pagemax>1 AND $pageNow<$pagemax) echo  "&nbsp;&nbsp;<a href=\"".$desturl.($pageNow+1)."\">next &gt;&gt;</a> ";
	
	if ($options['addtext']!=NULL) {
		echo ' '.$options['addtext'];
	}
	
	echo '<br />'."\n";
	
}

}
