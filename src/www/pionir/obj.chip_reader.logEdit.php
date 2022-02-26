<?php
/**
 * UPDATE one log entry for one device
 * @package obj.chip_reader.logEdit.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq SREQ:0001488: o.REA_LOG > edit one existing entry 
 * @param array parx
 * @param 
 *    $go
 *    $id
 *    $pos
 *    $parx - REA_LOG params
 *    $rowx - parameter array for service entries
		array ( index => array(
		 t - title
	     v - value
	     n - notes
	     ) )
 *    $backPage go back to service-log page
 *    $viewmode = 'view', 'edit'
 *    $action:
 *       ['features']
 *       'exp.add'   : add experiments
 *       'exp.del'   : add experiments: extra @param expsel : [EXP_ID] = 0,1
 *       'att.add'   : add attachment, optional input s_arg: attachment arguments
 */



session_start(); 

require_once ('reqnormal.inc');
require_once ('subs/obj.rea_log.edform.inc');
require_once ('f.msgboxes.inc'); 
require_once ("date_funcs.inc");
require_once ("o.REA_LOG.new.inc");
require_once ('o.CHIP_READER.logs.inc');
require_once ('o.CHIP_READER.subs.inc');
require_once ('o.REA_LOG.servAna.inc');
require_once ("func_form.inc");

/**
 * add log entry to list of devices
 * @author steffen
 */
class oCHIP_READER_logEdit {
	
	var $oldParams; // old REA_LOG params of entry
	var $rowx; // FORM-data for REA_LOG:XDATA
	
    function __construct(&$sqlo, $id, $pos, $parx, $backPage, $rowx) {
    	// forget it ...$this->go  = $go;
    	$this->parx=$parx;
    	$this->id  = $id;
    	$this->pos = $pos;
    	$this->backPage= $backPage;
    	$this->rowx= $rowx;
    	
    	$sqlsel = '* from CHIP_READER where CHIP_READER_ID='.$id;
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadArray();
    	$this->devParams  = $sqlo->RowData;

    	$this->oldParams= self::get_old_params($sqlo, $id, $pos);

    }
    
    static function get_old_params($sqlo, $id, $pos) {
        
        if (!$pos) {
            $oldParams=array();
            return $oldParams;
        }
        
        $sqlsel = '* from REA_LOG where CHIP_READER_ID='.$id.' and POS='.$pos;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadArray();
        $oldParams  = $sqlo->RowData;
        return $oldParams;
    }
    
    /**
     * do initial checks
     * - global input: $this->oldParams
     * - generate $this->editAllow
     * @param $sqlo
     */
    function initChecks( &$sqlo) {
    	
    	$tablename	= 'CHIP_READER';
    	$id = $this->id;
    	$FINISHED = 4; // limit of substatus ACCEPT
    	
    	$this->editLockReason = NULL;
    	$this->editAllow      = 0;
    	
    	$reason    = NULL;
    	$editAllow = 1;
    	do {
    		// check write access on table CHIP_READER
    		$t_rights = tableAccessCheck( $sqlo, $tablename );
    		
    		if ( $t_rights["write"] != 1 ) {
    			$editAllow=0;
    			$reason='no role-right "write" on table';
    			break;
    		}
    		
    		$o_rights = access_check($sqlo, $tablename, $id);
    		if ( !$o_rights["write"]) {
    			$editAllow=0;
    			$reason='no group-right "write" on this device.';
    			break;
    		}
    		
    		$substatus = $this->oldParams['ACCEPT'];
    		if ($substatus>=$FINISHED) {
    		    
    		    if ( $t_rights["admin"] != 1 ) {
        			$editAllow=0;
        			$reason='Entry is locked: substatus >= finished. (a table-admin can edit)';
        			break;
    		    }
    		}
    		
    	} while (0);
    	
    	$this->editAllow = $editAllow;
    	$this->editLockReason = $reason;
    }
    
    function getEditAllow() {
    	return $this->editAllow;
    }
    
    /**
     * return: 0:only view, 1: edit mode
     */
    function getEditMode($editAllow) {
    	if (!$editAllow) return 0;
    	$modenow = $_SESSION['s_sessVars']['o.REA_LOG.editmode'];
    	if ($modenow!='edit') return 0;
    	return 1;
    }
    
    /**
     * show edit button + lock-reason
     * @param $edit_possible
     */
    function showEditModeButton($edit_possible) {
    	$modenow = $_SESSION['s_sessVars']['o.REA_LOG.editmode'];
    	
    	$htmlTmp = formc::editViewBut(
    		$modenow,		// ["view"], "edit"
    		$edit_possible, // 0|1
    		$_SERVER['PHP_SELF'].'?id='.$this->id.'&pos='.$this->pos,
    		"viewmode"
    		);
    	echo $htmlTmp;
    	
    	if (!$edit_possible) {
    		echo ' &nbsp;&nbsp;<img src="images/but.lock.in.gif"> <span style="color:gray">Edit not possible: '.$this->editLockReason.'</span>';
    	}
    	echo "<br>\n";
    }
    
    function viewEntry(&$sqlo) {
        
    	$EditFormLib = new oChip_readerLogC();
    	$EditFormLib->viewEntry($sqlo, $this->id, $this->pos );
    	
    	if (oCHIP_READER_LOG_attC::has_functionality()) {
        	if (oCHIP_READER_LOG_attC::count_SUB_entries($sqlo, $this->id, $this->pos)) {
        	    echo '<h3>Attachments:</h3>'."\n";
            	$att_lib = new oCHIP_READER_LOG_attC($this->id, $this->pos);
            	$att_lib->show_list($sqlo);
        	}
    	}
    	
    	echo "<br>";
    	$dev_exp_lib = new oCHIP_READER_LOG_expC($this->id, $this->pos);
    	if (!empty($dev_exp_lib->get_experiments($sqlo))) {
    	   $dev_exp_lib->exp_show($sqlo);
    	}
    }
    
    function form0(&$sqlo) {
    	
    	$EditFormLib = new oChip_readerLogC ();
    	$formopt  =array('action'=>'update', 'A_DEVID'=>$this->devParams['A_CHIP_READER_ID'], 'DEVID'=>$this->id);
    	$hiddenarr=array('backPage'=>$this->backPage);
    	$EditFormLib->edform( $sqlo, $_SERVER['PHP_SELF'].'?id='.$this->id.'&pos='.$this->pos.'&go=1', $this->parx, $hiddenarr, $formopt);
    	
    	if (oCHIP_READER_LOG_attC::has_functionality()) {
        	echo "<br><br>\n";
        	echo '<h3>Attachments:</h3>'."\n";
        	$att_lib = new oCHIP_READER_LOG_attC($this->id, $this->pos);
        	$att_lib->form1($sqlo);
        	$att_lib->show_list($sqlo);
    	}
    	
    	echo "<br>\n";
    	$dev_exp_lib=new oCHIP_READER_LOG_expC($this->id, $this->pos);
    	$dev_exp_lib->exp_up_form($sqlo);
    }
    
    /**
     * analyse, if qualification is needed
     @return NULL or array of arguments for requalification
     */
    function _analyseQuali(&$sqlo, $devsicid) {
    	$devid = $this->id;
    	$qualiNeed = glob_elementDataGet( $sqlo, 'DEVSIC', 'DEVSIC_ID', $devsicid, 'QUALIFLAG'); 
    	if (!$qualiNeed) return ;
    	
    	// alread needs qualification ?
    	$dev_lib = new oCHIP_READER_subs();
    	$dev_lib->set_dev($sqlo, $devid);
    	$answer  = $dev_lib->anaQualiState($sqlo);
    	if($answer['qualiok']==2) return;
    	
    	$helplib = new oREA_LOG_subs();
    	$key = $helplib->defKeys['qualineed'];
    	$argu = array(
    		'NAME'=>'nach Service',
    		"KEY" =>$key,
    		'XDATE'=>date_unix2datestr( time(), 1)
    	);
    	return $argu;
    }
    
    /**
     * check input params
     * - automatic set of XDATE2, if ACCEPT= 4 or 6 (finished)
     * @param $sqlo
     */
    function checkParams(&$sqlo) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;

    	if ($this->parx['ACCEPT']== oREA_LOG_subs::ACC_FINISHED or 
    	    $this->parx['ACCEPT']==oREA_LOG_subs::ACC_FINISHED_NIO) {
    		if ($this->parx['XDATE2']==NULL) {
    			// automatic set XDATE2 to current time
    			$this->parx['XDATE2'] = date_unix2datestr( time(), 1);
    		}
    	}
    	
    	$infoarrLib = new oREA_LOG_infoarrMod();
    	$this->parx['XDATA'] = $infoarrLib->formPar2XDATA($this->rowx);
    	
    	$this->entryModiLib  = new oREA_LOG_new();
    	$this->entryModiLib->setDevice($this->id);
    	$answer = $this->entryModiLib->paramCheck($sqlo, $this->parx, $this->pos);
    	if ($answer!=NULL) {
    		$error->set( $FUNCNAME, 1, $answer[1].' (Code:'.$answer[0].')' );
    		return;
    	}
    }
    
    /**
     * do the update
     * Input: $this->parx, $this->entryModiLib
     * @param $sqlo
     */
    function update_log(&$sqlo) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$serviceAnalib = new oREA_LOG_servAna();
    	$this->entryModiLib->updateEntry($sqlo, $this->pos, $this->parx);
    	echo 'update o.k. <br>';
    	
    	if ($this->parx['KEY']=='Service' and $this->parx['ACCEPT']==$serviceAnalib->AcceptDefs['finished']
    		and $this->parx['DEVSIC_ID']>0 ) {
    		$argu = $this->_analyseQuali($sqlo, $this->parx['DEVSIC_ID']);
    		if (is_array($argu)) {
    			$this->entryModiLib->addEntry( $sqlo, $argu);
    			if ( $error->Got(READONLY) ) {
    				$error->set( $FUNCNAME, 2, 'Problem after insert of requalification-entry.' );
    				return;
    			}
    			echo 'Requalification need added. <br>';
    		}
    	}
    }
    
    function add_exp($sqlo) {
        global $error;
        $dev_exp_lib=new oCHIP_READER_LOG_expC($this->id, $this->pos);
        $infoarr = $dev_exp_lib->add_exp_fr_clip($sqlo);
        if ($error->Got(READONLY))  return;
        
        if (!empty($infoarr['exp_exist']) ) {
            cMsgbox::showBox("warning", sizeof($infoarr['exp_exist'])." Experiments already exist on an log entry.."); 
            echo "<br>";
        }
        
        cMsgbox::showBox("ok", $infoarr['cnt']." Experiments added."); 
        
    }
    
    function del_exp($sqlo, $exp_ids_flag) {
        global $error;
        
        if (empty($exp_ids_flag)) {
            return;
        }
        
        $exp_arr=array();
        foreach($exp_ids_flag as $exp_id=>$flag) {
            if($flag) $exp_arr[]=$exp_id;
        }
        
        
        $dev_exp_lib=new oCHIP_READER_LOG_expC($this->id, $this->pos);
        $cnt = $dev_exp_lib->del_exp($sqlo, $exp_arr);
        if ($error->Got(READONLY))  return;
        
        cMsgbox::showBox("ok", $cnt." Experiments removed."); 
    }
    
    function add_att($sqlo, $parx, $file_info) {
        global $error;
        
        $att_lib = new oCHIP_READER_LOG_attC($this->id, $this->pos);
        
        $infoarr = $att_lib->add($sqlo, $parx, $file_info);
        if ($error->Got(READONLY))  return;
        
        if (!empty($infoarr['att_exist']) ) {
            cMsgbox::showBox("warning", sizeof($infoarr['exp_exist'])." Attachment already exist on this log entry.");
            echo "<br>";
        }
        
        cMsgbox::showBox("ok", $infoarr['cnt']." Attachment added.");
        
    }

}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();




$id 	= $_REQUEST['id'];
$pos	= $_REQUEST['pos'];
$parx	= $_REQUEST['parx'];
$go		= $_REQUEST['go'];
$backPage = $_REQUEST['backPage'];
$action  = $_REQUEST['action'];
if ($action==NULL) $action='features';


$tablename	= 'CHIP_READER';

$title		= 'Edit service log entry POS: '.$pos;

$infoarr			 = NULL;
$infoarr['scriptID'] = '';
$infoarr['title']    = $title;
$infoarr['form_type']= 'obj'; 
$infoarr['design']   = 'norm';
$infoarr['obj_name'] = $tablename;
$infoarr['obj_id']   = $_REQUEST['id'];
$infoarr['help_url']   = 'misc/o.rea_log';
$infoarr['help_base']  = 'wiki';
$infoarr['checkid']  = 1;

$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo, $infoarr);


if (!is_numeric($pos) ) {
	$pagelib->htmlFoot('ERROR','Pos missing');
}

if ( $_REQUEST['viewmode']!=NULL ) {
	$_SESSION['s_sessVars']['o.REA_LOG.editmode'] = $_REQUEST['viewmode']; // change editmode
}


$js_string = oChip_readerLogC::get_js();
echo '<script>'."\n";
echo $js_string;
echo '</script>'."\n";

$oldParams = oCHIP_READER_logEdit::get_old_params($sqlo, $id, $pos);

if ( (!$go and $action=='features') or $action!='features' ) {
    $parx = $oldParams; // this are the parameters for the main FORM
}

$mainLib = new oCHIP_READER_logEdit($sqlo, $id, $pos, $parx, $backPage, $_REQUEST['rowx']);

$mainLib->initChecks($sqlo);
$editAllow= $mainLib->getEditAllow();

$mainLib->showEditModeButton($editAllow);
$editMode = $mainLib->getEditMode($editAllow);

if (!$editMode) {
	$mainLib->viewEntry($sqlo);
	$pagelib->htmlFoot();
}

if ( $action == 'exp.add') {
    $mainLib->add_exp($sqlo);
    $error->printAll();
    echo "<br>";
    $action = 'features';
    $go=0;
}
if ( $action == 'exp.del') {
    $mainLib->del_exp($sqlo, $_REQUEST['expsel']);
    $error->printAll();
    echo "<br>";
    $action = 'features';
    $go=0;
}
if ( $action == 'att.add') {
    
    if (empty($_FILES['s_file'])) {
        cMsgbox::showBox("error", "Input: no uploaded file given.");
    } else {
        $mainLib->add_att($sqlo, $_REQUEST['s_arg'], $_FILES['s_file'] );
        $error->printAll();
    }
    echo "<br>";
    $action = 'features';
    $go=0;
}

if (!$go) {
	$mainLib->form0($sqlo);
	$pagelib->htmlFoot();
}



$mainLib->checkParams($sqlo);
if ($error->Got(READONLY))  {
	$error->printAllPrio();// do not store problem in error log
	echo "<br>";
	$mainLib->form0($sqlo);
	$pagelib->htmlFoot();
}

$mainLib->update_log($sqlo);
if ($error->Got(READONLY))  {
	$error->printAll();
	echo "<br>";
	$mainLib->form0($sqlo);
	$pagelib->htmlFoot();
	
}

$backurl='edit.tmpl.php?t=CHIP_READER&id='.$id.'&xmode=logs';
if ($backPage>0) {
	$backurl .= '&tabArgu[page]='.$backPage;
}
require_once ( "javascript.inc" );
js__location_replace($backurl, 'device' );  

$pagelib->htmlFoot();
