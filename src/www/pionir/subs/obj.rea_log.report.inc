<?php
require_once ("o.CHIP_READER.logs.inc"); 
require_once  'o.DEVSIC.subs.inc';
require_once  'o.REA_LOG.devsic_spec.inc';
require_once ('gui/o.REA_LOG.guisub.inc');

/**
 * service log report functions
 * @package obj.rea_log.report.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0000894: o.CHIP_READER > Verwaltung geplanter Services/Stillstaende
 */
class oCHIP_READER_logSh {
    
    private $devsic_cache=array();
    
    /**
     * 
     * @var array 
     *   'vmode' : full, smlim
     */
    private $options;
	
    function __construct($options=array()) {
        $this->options=$options;
    	$this->colorblue ="#CCDDEE";
    	$this->fullcolspan = 4;
    	
    	$this->view_mode = $options['vmode'];
    	if ($this->view_mode=='')  $this->view_mode='full'; // 'slim'
    	if ($this->view_mode=='slim') $this->fullcolspan = 6;
    	
    	$this->pageNow = 1;
    	$this->cssKeys = array('Repaired'=>'doc-txRepaired', 'Problem'=>'doc-txProblem', 'STOP'=>'doc-txStop', 'Working'=>'doc-txWork');
    }
    function set_col_num($col_num) {
        $this->fullcolspan = $col_num;
    }
    
    function setPageNow($pageNow) {
    	$this->pageNow = $pageNow;
    }
    
    function getCss() {
    	
    	$logSubLib = new oREA_LOG_subs();
    	$keyColors = $logSubLib->getActColors();
    	
    	$css ="<style type=\"text/css\">\n".
    		  ".doc-lx1 { \n".
    		"  margin-left:9px; margin-right:9px;\n".
    		"} \n".
    		".doc-tx1 { font-weight:bold; color: #AABBFF}\n".
    		".doc-txProblem  { color: ".$keyColors['Problem']." }\n".
    		".doc-txRepaired { color: ".$keyColors['Repaired']." }\n".
    		".doc-txStop { font-weight:bold; color: ".$keyColors['STOP']." }\n".
    		".doc-txWork { font-weight:bold; color: ".$keyColors['Working']." }\n".
    		".doc-tx2 { font-size:0.8em; white-space: nowrap; }\n".
    		".doc-tx3 { font-size:0.8em; }\n".
    		"</style>\n";
    	return ($css);
    }
    
    /**
     * table width: 800px
     * @param string $headtext
     */
    function tableStart( $headtext=NULL ) {
        

        if ($this->view_mode=='slim') {
            
            echo '&nbsp; <span style="font-weight:bold; font-size:1.4em;">LOGS</span> &nbsp; '.$headtext."<br>";
            echo '<table cellpadding=1 cellspacing=1 border=0  width=1000 class="doc-lx1">'."\n";
            $cols=array( 
                'NAME',
                'KEY',
                'XDATE',
                'DB_USER_ID',
                'NOTES'
            );
            echo '<tr style="background-color: #E0E0EF; ">'."\n";
            foreach($cols as $xcol) {
                $nice = columnname_nice2('REA_LOG', $xcol);
                if ($xcol=='DB_USER_ID')  $nice='user';
                echo '<th style="padding:10px;">'.$nice.'</th>'."\n";
            }
            echo "</tr>\n";
            
        } else {
        	echo "<table cellpadding=1 cellspacing=1 border=0 bgcolor=".$this->colorblue." width=800 class=\"doc-lx1\"><tr><td bgcolor=#FFFFFF>";
        	echo "<table cellpadding=1 cellspacing=1 border=0 width=100%>\n";
        	echo '<tr><td colspan='.$this->fullcolspan .'><b><span style="color:'.$this->colorblue.';">LOGS</span></B> &nbsp;&nbsp;&nbsp;'.
        			 $headtext."</td></tr>\n";
        }
    }
    
    function tableEnd() {
        if ($this->view_mode=='slim') {
            echo "</table>\n";
        } else {
        	echo "</table>\n";
        	echo "</td></tr></table>\n";
        }
    }
    
    function line_full($text) {
        echo '<tr><td colspan='.$this->fullcolspan .'>'.$text.'</td></tr>'."\n";
    }
    
    private function _getfullUser($sqlo, $user_id) {
    	if ($user_id==NULL) return '???';
    	$fullname = glob_elementDataGet( $sqlo, 'DB_USER', 'DB_USER_ID', $user_id, 'FULL_NAME');
    	if ($fullname==NULL) $fullname=$user_id;
    	return $fullname;
    }
    
    /**
     * - show one log row
     * - support key coloring
     * @param $RowData
     * @param $option 'withDevice' : 0,1
     */
    function oneRowFull( &$sqlo, &$RowData, $option=NULL ) {
    	
    	$fullcolspan = $this->fullcolspan;
    	$id	   = $RowData["CHIP_READER_ID"];
    	$pos   = $RowData["POS"]; 
    	$name  = $RowData["NAME"]; 
    	$datex = $RowData["XDATE"]; 
    	$datex2 =$RowData["XDATE2"];
    	$user_id = $RowData["DB_USER_ID"];
    	$key   = $RowData["KEY"];
    	$accept= $RowData["ACCEPT"];
    	$DEVSIC_ID= $RowData["DEVSIC_ID"];
    	$att_cnt = cSattachSubs::count_SUB_entries($sqlo, 'REA_LOG', $id, $pos);
    	
    	$devsic_type='';
    	if ($DEVSIC_ID) {
    	    if (!$this->devsic_cache[$DEVSIC_ID]) {
    	        $this->devsic_cache[$DEVSIC_ID] = oDEVSIC_subs::features($sqlo, $DEVSIC_ID);
    	    }
    	    $devsic_type = $this->devsic_cache[$DEVSIC_ID]['TYPE'];
    	}
    	
    	$trStyle=NULL;
    	$plannedTxt=NULL;
    	
    	// 0000894: o.CHIP_READER > Verwaltung geplanter Services/Stillst�nde 
    	switch ($accept) {
    	    case oREA_LOG_subs::ACC_PLANNED:
    			// planned
    			$plannedTxt='<span style="font-weight:bold;">planned</span> ';
    			//$trStyle=' style="background-color: #FFD0C0;"';
    			break;
    	    case oREA_LOG_subs::ACC_ACCEPTED:
    			// acc
    			$plannedTxt='<span style="font-weight:bold;">accepted</span> ';
    			break;
    	    case oREA_LOG_subs::ACC_STARTED:
    			// planned
    			$plannedTxt='<span style="font-weight:bold;">started</span> ';
    			$trStyle=' style="background-color: #FFFFDF;"';
    			break;
    	    case oREA_LOG_subs::ACC_FINISHED:
    			// planned
    			$plannedTxt='<span style="color:gray;">finished</span> ';
    			break;
    	    case oREA_LOG_subs::ACC_FINISHED_NIO:
    	        // planned
    	        $plannedTxt='<span style="color:#FFA0A0;">finished NIO</span> ';
    	        break;
    	}
    	echo "<tr><td colspan=".$fullcolspan."><hr size=1 noshade color=".$this->colorblue."></td></tr>\n";
    	echo "<tr valign=top ".$trStyle.">";
    	echo "<td width=70%>"; // Subject column
    	if ( $option['withDevice'] > 0) {
    		echo "<a href=\"edit.tmpl.php?t=CHIP_READER&id=$id\"><img src=\"images/icon.CHIP_READER.gif\" border=0> ".
    		  $RowData["readerName"]."</a> [ID:".$id."]</b><br />\n";
    	}
    	
    	// edit.tmpl.php?t=REA_LOG&id=$id&primasid[1]=$pos\
    	$editUrl = 'obj.chip_reader.logEdit.php?id='.$id.'&pos='.$pos;
    	if ($this->pageNow>0) {
    		$editUrl .= '&backPage='.$this->pageNow;
    	}
    	
    	echo '<a href="'.$editUrl.'">'.$name.'</a></b>';
    	if ($att_cnt) {
    	    echo ' &nbsp; <img src="res/img/paperclip.svg" height=15 title="has attachments">';
    	}
    	echo "</td>";
    	
    	if ($this->cssKeys[$key]!=NULL) {
    		$cssClass=$this->cssKeys[$key];
    	} else $cssClass='doc-tx1';
    	
    	$userx_fullname = $this->_getfullUser($sqlo, $user_id);
    	$datex_short  = substr($datex,0,16);
    	$datex2_short = substr($datex2,0,16);
    	
    	echo "<td>".$plannedTxt."<span class=".$cssClass.">".$key;
    	if ($devsic_type!=NULL) {
    	    echo ' &gt; '. $devsic_type;
    	}
        echo '</span></td>'.
    	  '<td class="doc-tx2">'.$datex_short.'<br>'.$datex2_short.'</td><td class="doc-tx3">'.$userx_fullname."</td>";
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
    	if ( $RowData["XDATA"]!=NULL ) { 
    	    
    	    $infoarr_lib = new oREA_LOG_infoarrGui(); // TBD: build cache ...
    	    $infoarr_lib->devsic_init($sqlo, $DEVSIC_ID);
    	    echo "<tr><td colspan=".$fullcolspan.">";
    	    $infoarr_lib->view_slim($sqlo, $RowData["XDATA"]);
    	    echo "</td></tr>\n";
    	}
    }
    
    /**
     * - show one log row
     * - support key coloring
     *  'NAME',
        'KEY',
        'XDATE',
        
        'DB_USER_ID',
        'NOTES'
     * @param $RowData
     
     */
    function oneRowSlim( &$sqlo, &$RowData, $option=NULL ) {
        
        $fullcolspan = $this->fullcolspan;
        $id	   = $RowData["CHIP_READER_ID"];
        $pos   = $RowData["POS"];
        $name  = $RowData["NAME"];
        $datex = $RowData["XDATE"];
        $datex2 =$RowData["XDATE2"];
        $user_id = $RowData["DB_USER_ID"];
        $key   = $RowData["KEY"];
        $accept= $RowData["ACCEPT"];
        $DEVSIC_ID= $RowData["DEVSIC_ID"];
        
        $devsic_type='';
        if ($DEVSIC_ID) {
            if (!$this->devsic_cache[$DEVSIC_ID]) {
                $this->devsic_cache[$DEVSIC_ID] = oDEVSIC_subs::features($sqlo, $DEVSIC_ID);
            }
            $devsic_type = $this->devsic_cache[$DEVSIC_ID]['TYPE'];
        }
        
        $trStyle=NULL;
        $plannedTxt=NULL;
        
       
        switch ($accept) {
            case oREA_LOG_subs::ACC_PLANNED:
                // planned
                $plannedTxt='<span style="font-weight:bold;">planned</span> ';
                //$trStyle=' style="background-color: #FFD0C0;"';
                break;
            case oREA_LOG_subs::ACC_ACCEPTED:
                // acc
                $plannedTxt='<span style="font-weight:bold;">accepted</span> ';
                break;
            case oREA_LOG_subs::ACC_STARTED:
                // planned
                $plannedTxt='<span style="font-weight:bold;">started</span> ';
                $trStyle=' style="background-color: #FFFFDF;"';
                break;
            case oREA_LOG_subs::ACC_FINISHED:
                // planned
                $plannedTxt='<span style="color:gray;">finished</span> ';
                break;
            case oREA_LOG_subs::ACC_FINISHED_NIO:
                // planned
                $plannedTxt='<span style="color:#FFA0A0;">finished NIO</span> ';
                break;
        }
       
       
        echo "<tr valign=top>";
        echo "<td>"; // Subject column
        
//         if ( $option['withDevice'] > 0) {
//             echo "<a href=\"edit.tmpl.php?t=CHIP_READER&id=$id\"><img src=\"images/icon.CHIP_READER.gif\" border=0> ".
//                 $RowData["readerName"]."</a> [ID:".$id."]</b><br />\n";
//         }
        
        // edit.tmpl.php?t=REA_LOG&id=$id&primasid[1]=$pos\
        $editUrl = 'obj.chip_reader.logEdit.php?id='.$id.'&pos='.$pos;
        if ($this->pageNow>0) {
            $editUrl .= '&backPage='.$this->pageNow;
        }
        
        echo '<a href="'.$editUrl.'">'.$name.'</a></b>';
        if ($this->cssKeys[$key]!=NULL) {
            $cssClass=$this->cssKeys[$key];
        } else $cssClass='doc-tx1';
        
        $userx_fullname = $this->_getfullUser($sqlo, $user_id);
        $datex_short  = substr($datex,0,16);
        $datex2_short = substr($datex2,0,16);
        
        echo "</td><td>".$plannedTxt."<span class=".$cssClass.">".$key;
        if ($devsic_type!=NULL) {
            echo ' &gt; '. $devsic_type;
        }
        echo '</span></td>';
        echo '<td class="doc-tx2">'.$datex_short.', '.$datex2_short.'</td>';
        echo '<td class="doc-tx3">'.$userx_fullname."</td>";
        echo '<td>';
        if ( $RowData["NOTES"]!="" ) {
            
            $SHOW_LEN_AMX=120;
            if ( strlen($RowData["NOTES"])>$SHOW_LEN_AMX)  {
                $tmpstr = substr($RowData["NOTES"],$SHOW_LEN_AMX)." ...";
            } else {
                $tmpstr = $RowData["NOTES"];
            }
            $tmpstr = htmlspecialchars($tmpstr);
            $tmpstr = str_replace("\n", "<br>\n", $tmpstr);
            echo $tmpstr;
        }
        echo "</td>";
        echo "</tr>\n";
    }
    
    function oneRow( &$sqlo, &$RowData, $option=NULL ) {
        if ($this->view_mode=='slim') {
            $this->oneRowSlim($sqlo, $RowData, $option);
        } else {
            $this->oneRowFull($sqlo, $RowData);
        }
    }

}
