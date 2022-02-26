<?php

require_once ('f.prefvals.inc');
require_once ('func_form.inc');
require_once ("visufuncs.inc");
require_once ("f.objview.inc");	
require_once 'f.clipboard.inc';
require_once ("f.text_html.inc");


require_once ('o.REA_LOG.subs.inc');
require_once ("o.REA_LOG.devsic_spec.inc");
require_once ('gui/o.REA_LOG.guisub.inc');
require_once ('o.S_VARIO.subs.inc');
require_once ('o.A_CHIP_READER.subs.inc');
require_once 'o.CHIP_READER.logs.inc';
require_once 'o.SATTACH.subs.inc';
require_once 'lev1/o.SATTACH.upload.inc';
require_once 'gui/o.SATTACH.visu.inc';

require_once 'o.S_OBJLINK.subs.inc';
require_once 'o.S_OBJLINK.mod.inc';



class oCHIP_READER_LogStatic {
    
    static function getKeyVals(&$sqlo) {
        
        $prefValLib = new fPrefValsGet();	// TBD: test the new func
        $prefs = $prefValLib->getValInitList( $sqlo, "REA_LOG", "KEY" );
        $prefarr = NULL;
        foreach( $prefs as $valarr) {
            $prefarr[$valarr[0]] = $valarr[1];
        }
        
        return $prefarr;
    }
    
    /**
     * get selection for DEVSIC
     * - if ACCEPT > 1; user can not modify this value !!!
     * @param $sqlo
     * @param int $oldDevSicID  initial DEVSIC_ID
     * @param int $adevid A_CHIP_READER_ID
     */
    static function getServicePref(&$sqlo, $oldDevSicID, $adevid, $acceptflag) {
        
        if (!glob_table_exists('DEVSIC')) return;
        if (!$adevid) return;
        
        if ($oldDevSicID>0) $oldDevSicName = glob_elementDataGet( $sqlo, 'DEVSIC', 'DEVSIC_ID', $oldDevSicID, 'NAME');
        else $oldDevSicName = NULL;
        
        $MAX_EDIT_VAL=1;
        if ( ($acceptflag>$MAX_EDIT_VAL) and $oldDevSicID>0) {
            // user is not allowed to change this value, if planning phase is > 1
            $seltext  = '<input type=hidden name="parx[DEVSIC_ID]" value="'.$oldDevSicID.'">';
            $seltext .= htmlSpecialchars($oldDevSicName);
            return $seltext;
        }
        
        $prefarr=array();
        if ($oldDevSicID>0) {
            $prefarr[$oldDevSicID]=$oldDevSicName; // prefill select-array
        }
        $sqlsel = "x.DEVSIC_ID, dp.NAME from ADEV_HAS_SIC x join DEVSIC dp on x.DEVSIC_ID=dp.DEVSIC_ID ".
            " where A_CHIP_READER_ID=".$adevid. ' order by DEVSIC_ID';
        $sqlo->Quesel($sqlsel);
        while ( $sqlo->ReadRow() ) {
            $tmpid = $sqlo->RowData[0];
            $name  = $sqlo->RowData[1];
            $prefarr[$tmpid]=$name;
        }
        
        $seltext = formc::selectFget( "parx[DEVSIC_ID]", $prefarr, $oldDevSicID);
        
        return ($seltext);
    }
}

/**
 * show extra FORM-part of the DEVSIC -- service plan
 * @author skube
 *
 */
class oDEVSIC_form {
    
    function __construct($DEVSIC_ID) {
        $this->DEVSIC_ID=$DEVSIC_ID;
    }
    
    private function _attachment_exists($sqlo, $DEVSIC_ID) {
        $attachlib = new cSattachSubs();
        $doc_arr = $attachlib->getRelArrByKey($sqlo, 'DEVSIC', $DEVSIC_ID, NULL);
        
        if (!empty($doc_arr)) {
            return 1;
        } else {
            return 0;
        }
    }
    
    private function _attachment_show($sqlo, $DEVSIC_ID) {
        $attachlib2 = new oSATTACH_visu();
        $attachlib2->set_object('DEVSIC', $DEVSIC_ID);
        $html_str = $attachlib2->show_attachments($sqlo);
        
        echo $html_str;
    }
    
    function show_form(&$sqlo) { // showDevSicForm
        
        $DEVSIC_ID = $this->DEVSIC_ID;
        $sqlsel = '* from DEVSIC where DEVSIC_ID='.$DEVSIC_ID;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadArray();
        $params = $sqlo->RowData;
        
        if ($params['TYPE']!='IPC') {
            
            // show details for 'Wartung', 'Verifizierung', NOT for IPC
            
            $objLinkLib = new fObjViewC();
            $bo_option = array("format"  => "csv");
            $htmlTmp    = $objLinkLib->bo_display( $sqlo, 'DEVSIC', $DEVSIC_ID, $bo_option );
            echo '<b>Device service plan:</b> &nbsp;&nbsp;'. $htmlTmp."<br />\n";
            
            $showCols = array('TYPE', 'PERIOD', 'INEXT', 'NOTES');
            
            $dataArr   = NULL;
            
            foreach( $showCols as $colLoop) {
                
                $colval = $params[$colLoop];
                $colFeatures = colFeaturesGet( $sqlo, 'DEVSIC', $colLoop );
                
                if ($colLoop=='NOTES') continue; // handle later ...
                
                $usetext = $colval;
                
                $niceName = $colFeatures['NICE_NAME'];
                $notes    = $colFeatures['COMMENTS'];
                $dataArr[] = array( '<b>'.$niceName.'</b>', $usetext, $notes);
            }
            
            $tabobj = new visufuncs();
            $headOpt = array ( "title" => 'features of plan', "headNoShow" =>1);
            $headx   = array ("Key", "Val", "Notes");
            $tabobj->table_out2($headx, $dataArr,  $headOpt);
        }
        
        // if attachments exist, show them
        $attach_exists = $this->_attachment_exists($sqlo, $DEVSIC_ID);
        if ($attach_exists) {
            echo "<br />\n";
            echo '<table><tr valign=top><td><b>Attachments:</b></td>';
            echo '<td>';
            $this->_attachment_show($sqlo, $DEVSIC_ID);
            echo '</td></tr></table>'."\n";
        }
        
        echo "<br />\n<b>Notes of plan:</b>\n";
        $html_txt = htmlspecialchars ($params['NOTES']);
        if ($html_txt==NULL) {
            echo '<span class="yGgray">none</a>';
        } else {
            echo "<pre>";
            f_text_html::notes_out($sqlo, $html_txt);
            echo "</pre>\n";
        }
        
    }
}


/**
 * manage edit form for REA_LOG
 * @package obj.rea_log.edform.inc
 * @swreq UREQ:0000921 (o.CHIP_READER > REA_LOG : service log abbilden/bearbeiten) 
 * @swreq UREQ: FS-ID:FS-QA04-g o.DEV > save data for IPC in "service log"
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 * @version $Header: trunk/src/www/pionir/subs/obj.rea_log.edform.inc 59 2018-11-21 09:04:09Z $
 */
class oChip_readerLogC {
    
	private $A_DEVID;
	private $EDIT_fields_def= array( // defined by KEY
	    oREA_LOG_subs::KEY_Working => array(
	        'XDATE2'=>-1,
	    ),
	    oREA_LOG_subs::KEY_STOP => array(
	        'XDATE2'=>-1,
	    )
	);
	private $parx;
	
	
    function __construct () {
        $this->ACCEPT_pref = oREA_LOG_subs::ACCEPT_pref;
        
    }
 
    /**
     * get javascript
     */
    static function get_js() {
        return '
        function x_set_time(varname) {
            /* ISO: 2021-12-20T08:08:20.687Z */
            const  date_new = new Date()
            minutes_TZ = date_new.getTimezoneOffset(); 
            date_new.setMinutes(date_new.getMinutes() - minutes_TZ);
            let date_str = date_new.toISOString();
            date_str = date_str.replace("T", " ");
            date_str = date_str.substring(0,19);
            document.editform.elements[varname].value = date_str;
        }
        ';
    }
    
    function _reaPrefs(&$sqlo, $preselected=NULL) {
        $this->KEY_prefArr = oCHIP_READER_LogStatic::getKeyVals($sqlo);
    	
    	$prefarr = $this->KEY_prefArr;
    	$seltext = formc::selectFget( "parx[KEY]", $prefarr, $preselected); 
    
    	return ($seltext);
    }
    
    
    
    
    
    /**
     * one row
     * @param string $text
     * @param string $field
     * @param array $params 'req' : 0,1
     * @return void
     */
    function _formLine($text, $field, $params=NULL) {
    	
    	$keyOpt = NULL;
    	if ($params['req']>0) {
    		$keyOpt = ' <img src="images/redstar.gif">';
    	
    	}
    	echo "<tr valign=top>";
    	echo "<td align=right><b>$text</b>".$keyOpt."&nbsp;</td>";
    	echo "<td>$field";
    	echo "</td></tr>\n";
    }
    
    function _formLineOpen($text) {
    	echo "<tr valign=top>";
    	echo "<td align=right><b>$text</b>&nbsp;</td>";
    }
    
    function _formLineClose() {
    	echo "</tr>\n";
    }
    
    function _showRow($params) {
    	echo "<tr valign=top>";
    	echo "<td align=right><b>".$params[0]."</b>&nbsp;</td>";
    	echo "<td>".$params[1];
    	echo "</td></tr>\n";
    }
    function show_infoarrOp($key) {
    	echo "<tr valign=top>";
    	echo "<td align=right><b>".$key."</b>&nbsp;</td>";
    }
    
    /**
     * show edit-form for service-plan actions
     * @param string $xdata contains: JSON string
     */
    function infoarr_edit(&$sqlo, $xdata, $devsic_id) {
    	
    	$infoarrLib = new oREA_LOG_infoarrGui();
    	$infoarrLib->devsic_init($sqlo, $devsic_id);
    	$this->_formLineOpen('plan actions');
    	echo "<td>";
    	$paramStruct = $infoarrLib->structInit($xdata);
    	$infoarrLib->editform($paramStruct);
    	echo "</td>";
    	$this->_formLineClose();
    }
    
    /**
     * show table of service-plan actions
     * @param $formString contains: '[FORM] [/FORM]'
     */
    function show_infoarr(&$sqlo, $xdata_str, $devsic_id) {
    	
    	$infoarrLib = new oREA_LOG_infoarrGui();
    	$infoarrLib->devsic_init($sqlo, $devsic_id);
    	
    	$paramStruct = $infoarrLib->structInit($xdata_str);
    	$infoarrLib->viewtable($sqlo, $paramStruct);
    	
    }
    
    function _addHidden( $hiddenarr ) {
    	foreach( $hiddenarr as $key=>$val) {
    		echo '<input type=hidden name="'.$key.'" value="'.$val.'">'."\n"; 
    	}
    	reset ($hiddenarr); 
    }
    
    /**
     * @return array($notes, $infoarrStr)
     */
    // private function _splitNotesInfoStr($notes) {
    // 	if ( ($formpos = strpos($notes,'[FORM]')) !==FALSE ) {
    // 		$useNotesEnd   = ($formpos>0) ? $formpos-1 : $formpos ; // remove the last NEWLINE
    // 		$tmp_NOTES     = substr($notes, 0, $useNotesEnd);
    // 		$tmpInfoArrStr = substr($notes, $formpos);
    // 	} else {
    // 		$tmp_NOTES = $notes;
    // 		$tmpInfoArrStr = NULL;
    // 	}
    // 	return array($tmp_NOTES,$tmpInfoArrStr);
    // }
    
    function _infoBoxOpen($title) {
    	//$topt = array("color"=> "#E0EFFF", "fcolor"=>"#404080" );
        $topt=array();
    	htmlInfoBox( "&nbsp;".$title, "", "open", "CALM",  $topt );
    }
    function _infoBoxClose() {
    	htmlInfoBox("" , "", "close" );
    }
    
    
    
    
    
    private function _getfullUser($sqlo, $user_id) {
        if ($user_id==NULL) return '???';
    	$fullname = glob_elementDataGet( $sqlo, 'DB_USER', 'DB_USER_ID', $user_id, 'FULL_NAME');
    	if ($fullname==NULL) $fullname = 'ID:'.$user_id;
    	return $fullname;
    }
    
    /**
     * handle NAME = 'Problem'
     * @param string $value
     */
    private function _NAME_problem(&$sqlo, $value) {
    	if (!$this->A_DEVID) {
    		sys_error_my(__FUNCTION__.': A_DEVID fehlt.');
    	}
    	
    	// get all keys, SEMICOLON separated
    	$variolib = new oS_VARIO_sub('A_CHIP_READER');
    	$value_tmp    = $variolib->getValByKey($sqlo, $this->A_DEVID, 'SLOG.Problem.keys');
    	if ($value_tmp==NULL) return 0;
    	
    	$valarr = explode( ';',$value_tmp );
    	sort($valarr);
    	$newarr=array();
    	foreach($valarr as $oneval) {
    		$tmp=trim($oneval);
    		$newarr[$tmp] = $tmp;
    	}
    	
    	return formc::selectFget( "parx[NAME]", $newarr, $value);
    }
    
    private function _addNotes($textareaHeight, $val_in) {
        $this->_formLine( "notes", '<textarea  name="parx[NOTES]" cols=60 rows='.$textareaHeight.'>'.$val_in.'</textarea>');
    }
    
    private function _addSubStatus($val_in) {
        $prefarr = $this->ACCEPT_pref;
        $seltext = formc::selectFget( "parx[ACCEPT]", $prefarr, $val_in);
        $this->_formLine( '<span style="color:gray;">substatus</span>', $seltext);
    }
    
    /**
     * show EDIT form
     * @param  $sql
     * @param  $desturl
     * @param  $parx : INPUT params
     * @param  $hiddenarr : FORM hidden vars
     * @param  $formopt
     *   'action' : 
    		['insert']
    		'update' : $parx['KEY'] not allowed to change
    	 'allowKEY': 0,1 needed if action==update for obj.chip_reader.lognew_li.php
     *   'DEVID' : ID of device
     *   'A_DEVID' : ID of abstract device
     * @return void
     */
    function edform( &$sql, $desturl, $parx=NULL, $hiddenarr=NULL, $formopt=NULL ) {
    	
    	
        $p_key = $parx['KEY'];
    	$this->A_DEVID = $formopt['A_DEVID'];
    	$EDIT_defs = $this->EDIT_fields_def[$p_key];
    	if (!is_array($EDIT_defs)) $EDIT_defs=array();
    	
    	echo "<form style=\"display:inline;\" method=\"post\" ".
    	"name=\"editform\"  action=\"".$desturl."\" >\n";
    	
    	if ($formopt['action'] == NULL ) $action='insert';
    	else  $action = $formopt['action'];
    	
    	$KEY_text = NULL;
    	if ($formopt['allowKEY']) {
    	    $this->KEY_prefArr = oCHIP_READER_LogStatic::getKeyVals($sql);
    		$prefarr = $this->KEY_prefArr;
    		$KEY_text= formc::selectFget( "tabArgu[KEY]", $prefarr, $parx['KEY']);
    	} else {
    		$KEY_text = '<input type=hidden name="parx[KEY]" value="'.$parx['KEY'].'">'.
    			$parx['KEY'].
    			"\n";
    	}
    	
    	if ($action=='insert') {
    		$title = 'Add new log entry';
    		$submitBut = 'Add entry';
    		$textareaHeight = 5;
    
    		if ($p_key=='Working') {
    		    if ($parx['NAME']==NULL) {
    		        $parx['NAME']='Working';
    		    }
    		}
    	} else {
    		$title = 'Update log entry';
    		$submitBut = 'Update';
    		$textareaHeight = 10;
    		
    	}
    	
    	$this->_infoBoxOpen($title);
    	echo "<table cellpadding=1 cellspacing=0 border=0>";
    	$this->_formLine("",  '<input type=submit value="'.$submitBut.'" class="yButton">') ;
    	
    	$rowparam=array('req'=>1);
    	
    	//$UNIX_MIN_ONE = 60;
    	//$timeNowHuman = date_unix2datestr(time() , 2);
    	
    	$dateText = 
    		'<input type=text name="parx[XDATE]" size=18 value="'.$parx['XDATE'].'" required>'.
    		"<img src=\"images/but.timenow.gif\" onclick=\"x_set_time('parx[XDATE]')\" border=0 title=\"start date: now!\"> ";
    	
    	if ($EDIT_defs['XDATE2']>=0) {
        	$dateText .= '&nbsp;&nbsp;<b>end time</b> <input type=text name="parx[XDATE2]" size=18 value="'.$parx['XDATE2'].'">'.
        		"<img src=\"images/but.timenow.gif\" onclick=\"x_set_time('parx[XDATE2]')\" border=0 title=\"end date: now!\"> "; // ' [YYYY-MM-DD HH:MM]
    	}
    	
    	
    	$user_fullname = $this->_getfullUser($sql, $parx['DB_USER_ID']);
    	
    	$this->_formLine( "key",$KEY_text . '&nbsp;&nbsp;&nbsp;&nbsp;<B>Last editor:</B> '.$user_fullname, $rowparam);
    	$this->_formLine( "start time",$dateText, $rowparam);
    	if ($p_key=='Problem') {
    		$html_tmp = $this->_NAME_problem($sql, $parx['NAME']);
    		if ($html_tmp==NULL) $html_tmp = '<input type=text name="parx[NAME]" size=50 value="'.$parx['NAME'].'"> ';
    			$this->_formLine( "short description",$html_tmp, $rowparam);
    	} else {
    		$this->_formLine( "short description",'<input type=text name="parx[NAME]" size=50 value="'.$parx['NAME'].'" required> ', $rowparam);
    	}
    
    	switch ($p_key) {
    		case 'Service':
 
    		    $this->_addNotes($textareaHeight, $parx['NOTES']);
    		    // $this->_formLine( "notes", '<textarea  name="parx[NOTES]" cols=60 rows='.$textareaHeight.'>'.$parx['NOTES'].'</textarea>');
    			
    		    $this->infoarr_edit( $sql, $parx['XDATA'] , $parx['DEVSIC_ID'] );
    			
    			$seltext = oCHIP_READER_LogStatic::getServicePref($sql, $parx['DEVSIC_ID'], $formopt['A_DEVID'], $parx['ACCEPT']);
    			$this->_formLine( 'service plan', $seltext);
    		
    			$sel_opt = $rowparam;
    			$prefarr = $this->ACCEPT_pref;
    			$seltext = formc::selectFget( "parx[ACCEPT]", $prefarr, $parx['ACCEPT'], $sel_opt); 
    			$this->_formLine( '<span style="color:gray;">substatus</span>', $seltext.' (used for planned services)', $rowparam);
    			break;
    		case 'Quali_erwartet':
    		    $this->_addNotes($textareaHeight, $parx['NOTES']);
    		    break;
    		case 'STOP':
    		    $this->_addNotes($textareaHeight, $parx['NOTES']);
    		    break;
    		case 'Working':
    		    $this->_addNotes($textareaHeight, $parx['NOTES']);
    		    break;
    		default:
    		    $this->_addNotes($textareaHeight, $parx['NOTES']);
    		    $this->_addSubStatus($parx['ACCEPT']);	
    	}
    	
    	echo "</table>\n";
    	$this->_infoBoxClose();
    	
    	
    	if ($hiddenarr!=NULL) {
    		$this->_addHidden( $hiddenarr );
    	}
    	
    	echo "</form>";
    	
    	if ($parx['DEVSIC_ID']>0) {
    		echo "<br>\n";
    		$devsic_form = new oDEVSIC_form($parx['DEVSIC_ID']);
    		$devsic_form->show_form($sql);
    	}
    }
    
    /**
     * show SHORT edit form to create new entry
     * @param  $sql
     * @param  $desturl
     * @param  $parx
     * @param  $formopt
     *   'action' : 
    	['insert']
    	'update' : $parx['KEY'] not allowed to change
     *  
     * @return void
     */
    function edformNew( &$sql, $desturl, $parx=NULL, $hiddenarr=NULL, $devid ) {
    	
    	
    	// p.php?mod=DEF/o.CHIP_READER.log_guinew&id=".$devid
    	echo "<form style=\"display:inline;\" method=\"post\" "."name=\"editform\"  
    		action=\"edit.tmpl.php?t=CHIP_READER&id=".$devid."\" >\n";
    	
    	//$action='insert';
    	$KEY_text=NULL;
    	
    	$title = 'Add new log entry';
    	//$submitBut = 'Add entry';
    	//$textareaHeight = 5;
    
    	$this->KEY_prefArr = oCHIP_READER_LogStatic::getKeyVals($sql);
    	$prefarr = $this->KEY_prefArr;
    	$selopt= array("selbody"=>"onChange=\"this.form.submit();\"");
    	$KEY_text   = formc::selectFget( "tabArgu[KEY]", $prefarr, '', $selopt);
    	// echo "<select name=t size=1 onChange=\"thisGo(this.form.KEY.options[this.form.KEY.options.selectedIndex].value)\">\n";
    	
    	
    	$topt = array("color"=> "#E0EFFF", "fcolor"=>"#404080" );
    	htmlInfoBox( "&nbsp;".$title, "", "open", "INFO",  $topt );
    			   
    	echo "<table cellpadding=1 cellspacing=0 border=0 bgcolor=#E0EFFF>";
    	
    	$this->_formLine( "key",$KEY_text);
    	
    	echo "</table>\n";
    	htmlInfoBox("" , "", "close" );
    	
    	if ($hiddenarr!=NULL) {
    		$this->_addHidden( $hiddenarr );
    	}
    	
    	echo "</form>";
    }
    
    /**
     * show only the log entry
     */
    function viewEntry(&$sqlo, $devid, $pos) {
    	$sqlsel = '* from REA_LOG where CHIP_READER_ID='.$devid.' and POS='.$pos;
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadArray();
    	$params = $sqlo->RowData;
    	
    	// $tabobj = new visufuncs();
    	$objLinkLib = new fObjViewC();
    	
    	$tmp_NOTES1=$params['NOTES'];
    	
    	$NOTES_tmp = htmlspecialchars($tmp_NOTES1);
    	$NOTES_tmp = str_replace("\n", "<br>\n", $NOTES_tmp);
    	
    	$this->_infoBoxOpen('Show one service log entry');
    	echo "<table cellpadding=1 cellspacing=0 border=0>";
    	
    	$user_fullname = $this->_getfullUser($sqlo, $params['DB_USER_ID']);
    
    	$this->_showRow ( array( 'key',$params['KEY'] . '&nbsp;&nbsp;&nbsp;&nbsp;<B>Last editor:</B> '.$user_fullname) );
    	$this->_showRow ( array( 'short description',$params['NAME'] ) );
    	$this->_showRow ( array( 'start time', $params['XDATE']. 
    		         ' &nbsp;&nbsp;&nbsp; <b>end time:</b> '.$params['XDATE2'] ) );
    	$this->_showRow ( array( 'notes', $NOTES_tmp ) );
    // 	if ($params['XDATA']) {
    // 	    show_infoarr($sqlo, $params['XDATA'], $params['DEVSIC_ID']);
    // 	    $this->_showRow ( array( 'Actions', $xdata_slim ) );
    // 	}
    	
    	switch ($params['KEY']) {
    		case 'Service':
    	
    			$this->show_infoarrOp( 'service plan');
    			echo "<td>";
    			$this->show_infoarr( $sqlo, $params['XDATA'], $params['DEVSIC_ID'] );
    			echo "</td></tr>";
    			
    			$this->_showRow ( array( 'substatus', $this->ACCEPT_pref[$params['ACCEPT']] ) );
    			if ($params['DEVSIC_ID']) {
    				$tmpObjLink = $objLinkLib->bo_display( $sqlo, 'DEVSIC', $params['DEVSIC_ID'] );
    			} else $tmpObjLink ==NULL;
    			$this->_showRow ( array( 'service plan', $tmpObjLink ) );
    			break;
    			
    		
    		default:
    			$this->_showRow ( array( 'substatus', $this->ACCEPT_pref[$params['ACCEPT']] ) );
    			break;
    	}
    	
    	echo "</table>\n";
    	$this->_infoBoxClose();
    	
    	
    	if ($params['DEVSIC_ID']>0) {
    		echo "<br>\n";
    		$devsic_form = new oDEVSIC_form($params['DEVSIC_ID']);
    		$devsic_form->show_form($sqlo);
    		
    	}
    }

}


/**
 * manage experiments
 * @author skube
 *
 */
class oCHIP_READER_LOG_expC {
    
    const MASTER_KEY='realog';
    
    function __construct($dev_id, $log_pos) {
        $this->dev_id = $dev_id;
        $this->log_pos= $log_pos;
        $this->_S_KEY= self::MASTER_KEY . '.'.$this->log_pos;
    }
    
    
    function get_experiments($sqlo) {
        $obj_s_lib = new oS_OBJLINK_subs();
        $exp_ids = $obj_s_lib->getChiObjByTabOrKey($sqlo, 'CHIP_READER',  $this->dev_id, 'EXP', $this->_S_KEY);
        return $exp_ids;
    }
    
    /**
     * add experiments from clipboard
     * @param object $sqlo
     */
    function add_exp_fr_clip($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $EXP_MAX = 100;

        $clip_lib = new clipboardC();
        $exp_ids = $clip_lib->getAllObjects('EXP');
        
        if (empty($exp_ids)) {
            $error->set( $FUNCNAME, 1, 'There are no experiments in the clipboard.' );
            return;
        }
        
        if (sizeof($exp_ids)>$EXP_MAX) {
            $error->set( $FUNCNAME, 1, 'Too many experiments in the clipboard. Max '.$EXP_MAX.' allowed.' );
            return;
        }
        
        $obj_s_lib = new oS_OBJLINK_subs();
        $sobj_mod_lib = new oS_OBJLINK_mod();
        $sobj_mod_lib->setObj($sqlo, 'CHIP_READER',  $this->dev_id);
        
        $exists=array();
        $cnt=0;
        foreach($exp_ids as $exp_id) {
            
            //TBD: warning if exist
            if ($obj_s_lib->chiObjLinked($sqlo, 'CHIP_READER', $this->dev_id, 'EXP', $exp_id)) {
                $exists[]=$exp_id;
                continue;
            }
            
            $sobj_mod_lib->addLink($sqlo, 'EXP', $exp_id, $this->_S_KEY);
            if ($error->Got(READONLY))  {
                $error->set( $FUNCNAME, 1, 'Error on EXP:'.$exp_id );
                return;
            }
            $cnt++;
        }
        
        return array('cnt'=>$cnt,'exp_exist'=>$exists);
    }
    
    /**
     * add experiments from clipboard
     * @param object $sqlo
     */
    function del_exp($sqlo, $exp_ids) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;

        
        $sobj_mod_lib = new oS_OBJLINK_mod();
        $sobj_mod_lib->setObj($sqlo, 'CHIP_READER',  $this->dev_id);
        
        $cnt=0;
        foreach($exp_ids as $exp_id) {

            $sobj_mod_lib->removeLinkByObjID($sqlo, 'EXP', $exp_id);
            if ($error->Got(READONLY))  {
                $error->set( $FUNCNAME, 1, 'Error on EXP:'.$exp_id );
                return;
            }
            $cnt++;
        }
        
        return $cnt;
    }
    
    function exp_up_form($sqlo) {
        
        $main_url='obj.chip_reader.logEdit.php';
        
        echo '<b>Related experiments:</b> ';
        echo ' [<a href="'.$main_url.'?id='.$this->dev_id .'&pos='.$this->log_pos.'&action=exp.add">Add from clipboard</a>]';
        echo "<br><br>";
        
        $exp_ids = $this->get_experiments($sqlo);
        if (empty($exp_ids)) return;
        
      
        $objLinkLib = new fObjViewC();
        
        
        echo '<form style="display:inline;" method="post" '.
            ' name="editform"  action="'.$main_url.'" >'."\n";
        echo '<input type=hidden name="go" value="0">'."\n";
        echo '<input type=hidden name="action" value="exp.del">'."\n";
        echo '<input type=hidden name="id" value="'.$this->dev_id.'">'."\n";
        echo '<input type=hidden name="pos" value="'.$this->log_pos.'">'."\n";
        echo '<input type=submit value="Remove">'."\n"; 
       
        echo "<br>\n"; 

        foreach($exp_ids as $exp_id) {
            $html_tmp = $objLinkLib->bo_display( $sqlo, 'EXP', $exp_id );
            echo '<input type=checkbox name="expsel['.$exp_id.']" value="1"> '.$html_tmp."<br>";
        }
        echo '</form>'."\n";
    }
    
    function exp_show($sqlo) {
        
        echo '<b>Related experiments:</b>'."<br>";
        echo "<br>";
        
        $exp_ids = $this->get_experiments($sqlo);
        if (empty($exp_ids)) return;

        $objLinkLib = new fObjViewC();
        
        foreach($exp_ids as $exp_id) {
            $html_tmp = $objLinkLib->bo_display( $sqlo, 'EXP', $exp_id );
            echo $html_tmp."<br>";
        }
    }
}

/**
 * manage ATTACHMENTs
 *
 */
class oCHIP_READER_LOG_attC {
    
    const MAIN_URL='obj.chip_reader.logEdit.php';
    const TABLE='REA_LOG';
    
    private $att_entries;

    function __construct($dev_id, $log_pos) {
        $this->dev_id = intval($dev_id);
        $this->log_pos= intval($log_pos);
    }
    
    static function has_functionality() {
        return glob_column_exists('SATTACH', 'SUB_POS_STR');
    }
    
    static function count_SUB_entries(object $sqlo, int $id, int $log_pos) {
        return cSattachSubs::count_SUB_entries($sqlo, self::TABLE, $id, $log_pos);
    }
    
    function get_attachments($sqlo) {
        $this->att_entries = cSattachSubs::get_all_entries_of_SUB($sqlo, self::TABLE, $this->dev_id, $this->log_pos);
    }
    
    function form1($sqlo) {

        $initarr   = NULL;
        $initarr["action"]      = $_SERVER['PHP_SELF'];
        $initarr["title"]       = "Upload file";
        $initarr["ENCTYPE"]  = "multipart/form-data";
        $initarr["formname"] = 'attform';
        
        $hiddenarr = NULL;
        $hiddenarr["id"]  = $this->dev_id ;
        $hiddenarr["pos"] = $this->log_pos ;
        $hiddenarr["action"]     = 'att.add';
        
        $formobj = new formPure($initarr, $hiddenarr, 0);
        $formobj->start();
        
        echo 'Upload a file: ';
        echo '<input type="file" name="s_file"> '."\n";
        echo '<input type=submit value="Upload" class="yButSmall">'."\n";
        
        $formobj->close();
    }
    
    function show_list($sqlo) {
        
        $cnt = cSattachSubs::count_SUB_entries($sqlo, self::TABLE, $this->dev_id, $this->log_pos);
        
        if (!$cnt) {
            return;
        }
        
        $attachlib2 = new oSATTACH_visu();
        $attachlib2->set_object(self::TABLE, $this->dev_id);
        $visu_opt= array(
            'filter' => array(
               array('col'=>'SUB_POS_STR', 'con'=>'LIKE', 'val'=> $this->log_pos.',%' )
               ),
            'img.thumbnail' => 1
        );
        $html_str = $attachlib2->show_attachments($sqlo, $visu_opt);
        $html_str .= '<br>'."\n";
        
        echo $html_str;
    }
    
    /**
     * 
     * @param object $sqlo
     * @param array $parx
     * @param array $file_info
     */
    function add($sqlo, $parx, $file_info) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $last_index = cSattachSubs::get_last_p1_of_SUB($sqlo, self::TABLE, $this->dev_id, $this->log_pos);
        $new_index = $last_index + 1;
        $parx['SUB_POS_STR'] = $this->log_pos.','.$new_index;
        
        $modLib = new oSATTACH_modWork($sqlo, self::TABLE, $this->dev_id, 1);
        if ($error->Got(READONLY))  {
            return;
        }
        
        $file_short = $file_info['name'];
        $name_exists = $modLib->checkNameExistsSubPos($sqlo, $this->log_pos, $file_short);
        if ($name_exists) {
            $error->set( $FUNCNAME, 1, 'File with Name "'.$file_short.'" already exists on this Log-Entry.' );
            return;
        }

        $ins_opt = array( 'name_check'=>-1 );
        $answer = $modLib->insert_att($sqlo, $file_info, $parx, $ins_opt);
        if ($error->Got(READONLY))  {
            return; 
        }
        return $answer;
    }
}