<?php
/**
 * - extras for chip_reader log
 * - it contains code, which will be included in edit.tmpl.php, when the rider is set to "MODE"
 * $Header: trunk/src/www/pionir/obj.chip_reader.xmode.logs.inc 59 2018-11-21 09:04:09Z $
 * @package    edit.tmpl.php
 * @subpackage obj.chip_reader.xmode.logs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

require_once ("f.objview.inc");
require_once 'o.CHIP_READER.subs.inc';
require_once ('subs/obj.rea_log.edform.inc');
require_once ('subs/obj.rea_log.report.inc');
require_once ('subs/obj.chip_reader.xmode.logs.sub.inc');
require_once ('o.CHIP_READER.logs.inc');
require_once 'o.A_CHIP_READER.subs.inc';
require_once 'o.DEVSIC.subs.inc';
require_once ('gui/f.pageEntriesOrg.inc');
require_once ("gui/o.CHIP_READER.statusView.inc");

/**
 * show REA_LOG
 */
class oCHIP_READER_xmode_logs {
	
    function __construct( &$editLib ) {
    	$this->editLib = &$editLib;
    }
    
    function anaDevQuali(&$sqlo, $id) {

    	$qualiDef=array(
    		0=>array('txt'=>'N/A',  'icon'=>'' ),
    		1=>array('txt'=>'o.k.', 'icon'=>'i13_ok.gif' ),
    		2=>array('txt'=>'need qualification!', 'icon'=>'i13_err.gif'),
    	);
    	
    	$answer  = $this->dev_lib->anaQualiState($sqlo, 1);
    	if (!$answer['qualiok']) {
    	    return; 
    	}
    	
    	$defSubArr = $qualiDef[$answer['qualiok']];
    	$iconImg = ' <img src="images/'.$defSubArr['icon'].'">';
    	$date_short = substr($answer['qdate'],0,10);
    	echo '<span style="color:gray;"> &nbsp;|&nbsp;Qualification status: </span>'.$defSubArr['txt'].$iconImg.
            ' <span style="color:gray;">last qualification:</span> '.$date_short;
    }
    
    function anaDevService($sqlo) {
        $answer  = $this->dev_lib->anaServiceStatus($sqlo);
        if (!$answer['active']) return;
        
        echo '&nbsp;<span style="color:gray;">|&nbsp;Service:</span> active&nbsp;';
    }
    
    /**
     * show OK-Status
     * @param object $sqlo
     * @param int $id
     */
    function anaOkStatus(&$sqlo, $id) {
    
    	$helplib = new oCHIP_READER_statusView();
    	$answer = $helplib->_getLastStatus($sqlo, $id);
    	$this->serv_log_status = $answer;
    	$text = $answer['OK.key'];
    	echo '<span style="color:gray;">OK-Status: </span>'.$text.' &nbsp;&nbsp;';
    }
    
    static function _show_NAV($links, $select_key, $url) {
        $out='';
        foreach($links as $key => $row) {
            
            $style='';
            if ($key!=$select_key) {
                $style=' style="filter: opacity(50%);"';
            }
            $row_rider = array(
                '<img src="images/'. $row['icon'].'" height=20 vspace=2 '.$style.' title="'.$row['txt'].'"> ',
                $url.'&'.$row['param']);
            $out .= '<a href="'.$row_rider[1].'">'.$row_rider[0].'</a>'."\n";
        }
        return $out;
    }
    
    function newAnaForm1( &$sqlo, &$EditLib, $parx, $A_DEVID) {
       
        if ($parx['XDATE']==NULL) $parx['XDATE'] = date_unix2datestr( time(), 1 );
        
        if ($parx['KEY']=='Qualifizierung') {
            // check template
            $devsic_type = oDEVSIC_subs::TYPE_Q;
            $devsic_list = oA_CHIP_READER_subs::getDevSicList_byType($sqlo, $A_DEVID, $devsic_type);
            
            if (empty($devsic_list)) {
                htmlErrorBox( 'Error', 'Qualification service plan missong', 
                    'Please define a '.tablename_nice2('DEVSIC'). ' with type "'.$devsic_type.'".'   );
                echo "<br>";
                return;
            }
            $one_dev_sic = current($devsic_list);
                
            $parx['KEY']='Service';
            $parx['DEVSIC_ID']=$one_dev_sic;
            $parx['NAME']='Qualifizierung';
            
            // forwqard as service
            
        }
        
        if ($parx['KEY']=='Service') {
            // forward to advanced FORM-script
            $parx_encode = http_build_query( array('parx'=>$parx) );
            
            $newurl = "obj.rea_log.new.php?id=".$this->dev_id.'&action=prep&'.$parx_encode;
                js__location_replace($newurl, "forward to Service module ..." );
                return;
        }
        
        $formopt= array('A_DEVID'=>$A_DEVID);
        $dummy  = NULL;
        $EditLib->edform( $sqlo, 'obj.rea_log.new.php?id='.$this->dev_id, $parx, $dummy, $formopt );
    }
    
    
    /**
     * 
     * @param object $sql
     * @param object $sql2
     * @param int $id
     * @param array $arguByKey
     * @param array $x_rights
     * @param array $extraobj_o
     * @param array $tabArgu
     * 	'page' show page number
     *  'timeZone' : ['past'], 'future'
     * @param int $editAllow
     * @return -
     * @global s_formState['o.CHIP_READER.logs'] = array('timeZone'=>...)
     */
    function xmode_start( &$sql, &$sql2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {
    	/**
    	 * OLD:
    	 * '&nbsp;[<a href="view.tmpl.php?t=REA_LOG&searchMothId='.$id.'&condclean=1">Advanced list</a>] '.
    	 *	'[<a href="../lab/obj.chip_reader.logDia.php?id='.$id.'&parx[mode]=ID">as time diagram</a>]   '.
    	 */
    	
    	$table='CHIP_READER';
    	$tmp_editallowed=0;
    	if ( $_SESSION['s_sessVars']["o.CHIP_READER.editmode"] == "edit" ) {
    		if ( $x_rights["insert"] ) {
    			$tmp_editallowed=1;
    		}
    	}
    	$this->dev_id = $id;
    	$A_DEVID = $arguByKey['A_CHIP_READER_ID'];
    	if ($tabArgu['vmode']) {
    	    $_SESSION['s_formState']['o.CHIP_READER.logs']['vmode'] = $tabArgu['vmode'];
    	}
    	$view_mode = $_SESSION['s_formState']['o.CHIP_READER.logs']['vmode'];
    	if ($view_mode=='') $view_mode='full'; // 'slim'
    	
    	$log_opts= array('vmode'=>$view_mode);
    	
    	$EditLib       = new oChip_readerLogC ();
    	$ReportLib     = new oCHIP_READER_logSh($log_opts);
    	$this->dev_lib = new oCHIP_READER_subs();
    	$this->dev_lib->set_dev($sql, $id);
    	
    	
    	
    	echo $ReportLib->getCss();
    	
    	echo "<ul>";
    	$this->anaOkStatus($sql, $id);
    	$this->anaDevQuali($sql, $id);
    	$this->anaDevService($sql, $id);
    	echo "<br>";
    	
    		
    	if ($tmp_editallowed) { 
    		
    		if ($tabArgu['KEY']==NULL) {
    			echo '<table><tr><td>'."\n";
    			$parx=array(); 
    			$EditLib->edformNew( $sql, 'obj.rea_log.new.php?id='.$id, $parx, NULL, $id );
    			echo '</td><td>'."\n";
    			
    			// TBD: future: show: STOP > service start > service end > qualification start > qualification end > Working
    			//$helplib = new oCHIP_READER_xmode_logs_sub($id);
    			//$curr_ok_status = $this->serv_log_status['OK.key.raw'];
    			//$helplib->_showActionPath($sql, $curr_ok_status);
    			
    			echo '</td></tr></table>'."\n";
    			
    		} else {
    		    $this->newAnaForm1($sql, $EditLib, $tabArgu, $A_DEVID);
    		    
    			
    		}
    	} 
    	
    	if ($tabArgu['timeZone'] !=NULL) {
    		$_SESSION['s_formState']['o.CHIP_READER.logs']['timeZone']=$tabArgu['timeZone'];
    	}
    	$timeZone = $_SESSION['s_formState']['o.CHIP_READER.logs']['timeZone'];
    	
    	$timeNowSql = $sql->Timestamp2Sql ( time() ); 
    	$historySqlAdd = " and XDATE<=".$timeNowSql;
    	$futureSqlAdd  = " and XDATE>"  .$timeNowSql;
    	
    	$sqlsel="count(1) from REA_LOG where CHIP_READER_ID=".$id.$historySqlAdd;
    	$sql->Quesel($sqlsel);
    	$sql->ReadRow();
    	$foundEntries = $sql->RowData[0];
    	
    	$sqlsel="count(1) from REA_LOG where CHIP_READER_ID=".$id.$futureSqlAdd;
    	$sql->Quesel($sqlsel);
    	$sql->ReadRow();
    	$entriesFuture = $sql->RowData[0];
    	
    	
    	$cnt=0;
    	$entryPerPage=10;
    	if ($view_mode=='slim') $entryPerPage=50;
    	
    	$pageNavLib = new f_pageEntriesGui();
    	$showinf = array(
    		"startShow"   => $tabArgu['page'],
    	    "foundEntries"=> $foundEntries
    		);
    	$parx = array('entryPerPage'=>$entryPerPage);
        $info = $pageNavLib->setParams( $showinf, $parx );
    	
        $timeSetUrl = $_SERVER['PHP_SELF'].'?t=CHIP_READER&id='.$id.'&tabArgu[timeZone]=';
        if ($timeZone=='future' and $entriesFuture) {
        	$useSqlAdd = $futureSqlAdd;
        	
        	$futureStyle = ' color:white; font-weight:bold; background-color:#3366cc; padding:5px;'; // #D0D0FF;
        	$textPast = '><a href="'.$timeSetUrl.'past">'.$foundEntries.' entries</a>';
        	$textFuture = $entriesFuture.' entries';
        } else {
        	$useSqlAdd = $historySqlAdd;
        	$futureStyle = '';
        	$tmpStyle = $entriesFuture ? ' color:white; background-color:#3366cc;' : ''; // #D0D0FF;
        	$textPast = ' style="font-weight:bold;'.$tmpStyle.' padding:5px;">'.$foundEntries.' entries';
        	$textFuture= '<a href="'.$timeSetUrl.'future">'.$entriesFuture.' entries</a>';
        }
        
        // only events in history ! 
        $sqlsel="* from REA_LOG where CHIP_READER_ID=".$id. $useSqlAdd. " order by XDATE desc";
    	$sql2->Quesel($sqlsel);
    	echo "</ul>";
    	
    	
    	
    	$entryStart = $info['start'];
    	$endShow    = $info['end'];
    	$pageNow    = $pageNavLib->showinf["startShow"];
    	
    	$headtext = '<span style="color:gray;"><span'.$textPast.' in past</span> ';
    	if ($entriesFuture) {
    		$headtext .= ' &nbsp;<span style="color:gray;'.$futureStyle.'"> '.$textFuture.
    			' in future </span>&nbsp; '; 
    	}
    	$headtext .=
    		'</span>&nbsp;&nbsp;&nbsp;'.
    		'[<a href="p.php?mod=DEF/o.CHIP_READER.serviceHome&id='.$id.'">Service-Home</a>] ';
    	if ( gGlobal::productIsIntern() ) $headtext .= ' [<a href="f.help.php?f=o.REA_LOG.html&base=lab" target="_help">help</a>]';
    	$links=array(
    	    'list'=> array('param'=>'tabArgu[vmode]=slim', 'txt'=>'list view', 'icon'=>'ic.list.png'),
    	    'grid'=> array('param'=>'tabArgu[vmode]=full', 'txt'=>'grid view', 'icon'=>'ic.grid.png'),
    	);
    	
    	$headtext .= ' &nbsp;&nbsp;  '.self::_show_NAV($links, $view_mode, 'edit.tmpl.php?t=CHIP_READER&id='.$id );
    	
    	if ($view_mode=='slim') echo "<br>\n";
    	
    	$ReportLib->setPageNow($pageNow);
    	$ReportLib->tableStart($headtext);
    	
    	while ($sql2->ReadArray()) {
    		
    		$showit = 0;

    		if ( $cnt >= $entryStart ) $showit=1;
    		if ( $cnt > $endShow )   {
    			$moreExists = 1;
    			break;
    		}
    		
    		if ($showit) {
    			$ReportLib->oneRow( $sql, $sql2->RowData );
    		}
    		$cnt++;
    	}
    	
    	if ( !$cnt ) {
    	    $ReportLib->line_full('<br><center><b>No entries!</b></center><br><br>');
    	}
    	
    	$ReportLib->tableEnd();
    	
    	if ($cnt) {
    		$pageNavLib->setLastShownEntry($cnt-1);
    		$desturl= $_SERVER['PHP_SELF'].'?t='.$table.'&id='.$id.'&tabArgu[page]=';
    		echo "<br />\n";
    		$pageNavLib->showEntryNav ( $desturl );
    	}
    }

}
