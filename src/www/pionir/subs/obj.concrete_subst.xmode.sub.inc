<?php

require_once ("visufuncs.inc");
require_once ("f.objview.inc");
require_once ("gui/o.SUC_ProtoLogGui.inc");

require_once ("o.ABSTRACT_SUBST.qcprot.inc");
require_once ("o.CONCRETE_SUBST.qcProt.inc");
require_once ("gui/o.CONCRETE_PROTO.edit.inc");

require_once ('f.help.inc');
// require_once ('obj.concrete_subst.statusAna.inc'); // located also in dir "sub" !

/**
 * class to handle protocol editor (QC and PREP)
 * @package obj.concrete_subst.xmode.sub.inc
 * @swreq SREQ:0001352: o.CONCRETE_SUBST > GUI: modify/Create QC-protocol 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/subs/obj.concrete_subst.xmode.sub.inc 59 2018-11-21 09:04:09Z $
 */
class oSUC_xmode_prot_help {
	
	var $XTYPE_PREP=1;
	var $XTYPE_QC=2;
	/**
	 * 
	 * @var array $sh_options
	 *   'sh_info_head' : [1] : show it, -1: do not show it
	 *   'form_type' : 
	 *   		[default], normal form 
	 *   		'form2'    form2 type
	 *   'sh_help_row'  : [1] : show it, -1: do not show it
	 */
	private $sh_options = array();
	private $sucid;
	
	function __construct() {
	    $this->_SUC_prefs = unserialize($_SESSION['userGlob']['o.CONCRETE_SUBST.xspref']);
	}
	
    function warningsOut(&$qcProblem) {
    	if ( $qcProblem!="" ) {
    		echo "<ul>";
    		htmlInfoBox( "QM Warning", "", "open", "WARN" );
    		echo $qcProblem;
    		htmlInfoBox( "", "", "close" );
    		echo "</ul>\n";
    	}
    }
    
    /**
     * get access-flag of concrete_proto $cpid
     * @param $sqlo
     * @param $cpid
     * @return $writeacc
     * 		1  : o.k.
     * 		-1 : no access
     */
    function _getProtAccess(&$sqlo, $cpid) {
    	$writeacc = -1;
    	
    	$o_rights = access_check($sqlo, 'CONCRETE_PROTO', $cpid);
    	if ( $o_rights["insert"]>0 ) $writeacc = 1;
    	
    	return $writeacc;
    }
    
    function _showEditButton($userWants, $qcEditAllow, $cp_writeacc, $c_proto_id) {
    	if ($userWants=='edit') {
    		if ($qcEditAllow and $cp_writeacc>0) {
    			$oneProtoEditLogo='<a href="' .$this->EditUrl . $c_proto_id. '">'.
    				'<img src="images/but.edit.ac.gif" border=0 title="edit"></a>';
    		} else {
    			$oneProtoEditLogo='<img src="images/but.edit.no.gif" title="no access">';
    		}
    	}
    	
    	if ($qcEditAllow and $cp_writeacc<1) {
    		// if write-access on SUC and NO write-access on Proto ...
    		$oneProtoEditLogo .= ' <a href="p.php?mod=DEF/o.SUC.cp_reopen&id='. $this->sucid . '&cpid=' . $c_proto_id. '">'.
    			'<img src="images/but.edit.unlock.gif" border=0 title="reopen"></a>';
    	}
    	
    	return $oneProtoEditLogo;
    }
    
    function _showProtForm2(&$sqlo, $substid, $c_proto_id, $qcEditAllow, $info_title, 
    	$qm_check_arr, $userWants, $prEditIsActive) {
        
    	require_once ("gui/o.CONCRETE_PROTO.form2.inc");
    	
    	$tablename 	     = 'CONCRETE_SUBST';
    	$oneProtoEditLogo= NULL;
    	$loopEditAllow   = 0; // default
    	
    	
    	if (!$prEditIsActive) {
    		// EDIT is NOT active
    		$cp_writeacc = $this->_getProtAccess($sqlo, $c_proto_id);
    		$oneProtoEditLogo = $this->_showEditButton($userWants, $qcEditAllow, $cp_writeacc, $c_proto_id);
    	} else {
    		// EDIT is active
    		$loopEditAllow   = $qcEditAllow;
    	}
    	
    	$edFormLib = new oCONCRETE_PROTO_form2();
    	
    	$loop_shAbsNotes = 1;
    	$optRows         = NULL;
    	
    	$edFormLib->setProtoID($c_proto_id, $substid);
    	$optarr  = array(
    		'headAdd'     => '&nbsp;'.$oneProtoEditLogo
    	);
    	$editProtoAllow = $edFormLib->showIntro( $sqlo,	$loopEditAllow, $info_title, $optRows, $optarr );
    	
    	$devopt = array(
    		"backurl"   => "edit.tmpl.php?t=".$tablename."&id=".$substid.'&tabArgu[PRC]='.$c_proto_id,
    		
    	);
    	$edFormLib->showform( $sqlo, $sqlo2, $editProtoAllow, $devopt );
    	
    }
    
    function _showProtForm3(&$sqlo, $substid, $c_proto_id, $qcEditAllow, $info_title,
        $qm_check_arr, $userWants, $prEditIsActive) {
            
            require_once ("gui/o.CONCRETE_PROTO.form3.inc");
            
            $tablename 	     = 'CONCRETE_SUBST';
            $oneProtoEditLogo= NULL;
            $loopEditAllow   = 0; // default
            
            if (!$prEditIsActive) {
                // EDIT is NOT active
                $cp_writeacc = $this->_getProtAccess($sqlo, $c_proto_id);
                $oneProtoEditLogo = $this->_showEditButton($userWants, $qcEditAllow, $cp_writeacc, $c_proto_id);
            } else {
                // EDIT is active
                $loopEditAllow   = $qcEditAllow;
            }
            
            $backurl = "edit.tmpl.php?t=".$tablename."&id=".$substid.'&tabArgu[PRC]='.$c_proto_id;
            
            $edFormLib = new oCONCRETE_PROTO_form3();
            //$loop_shAbsNotes = 1;
            $optRows         = NULL;
            $edFormLib->setProtoID($sqlo, $c_proto_id, $substid);
            $optarr  = array(
                'headAdd'   => '&nbsp;'.$oneProtoEditLogo,
                "backurl"   => $backurl, 
            );
            $editProtoAllow = $edFormLib->showIntro( $sqlo,	$loopEditAllow, $info_title, $optRows, $optarr );
            $devopt = array(
                "backurl"   => $backurl,  
            );
            $edFormLib->showform( $sqlo, $editProtoAllow, $devopt );
            
    }
    
    /**
     * show the PROTO-Editor
     * @param $sqlo
     * @param $substid
     * @param $c_proto_id
     * @param $qcEditAllow
     * @param $info_title
     * @param array $qm_check_arr
     * @param $userWants  : substance-edit-mode:  'edit', 'view'
     * @param int $prEditIsActive 0,1: protocol edit-mode is active ?
     */
    private function _showProtEditor(&$sqlo, $substid, $c_proto_id, $qcEditAllow, $info_title, 
    	$qm_check_arr, $userWants, $prEditIsActive) {
    	
    	$tablename 	     = 'CONCRETE_SUBST';
    	$oneProtoEditLogo= NULL;
    	$loopEditAllow   = 0; // default
    	
    	
    	if (!$prEditIsActive) {
    		// EDIT is NOT active
    		$cp_writeacc = $this->_getProtAccess($sqlo, $c_proto_id);
    		$oneProtoEditLogo = $this->_showEditButton($userWants, $qcEditAllow, $cp_writeacc, $c_proto_id);
    	} else {
    		// EDIT is active
    		$loopEditAllow   = $qcEditAllow;
    	}
    	
    	$edFormLib = new oConcProtoEdform($sqlo);
    	$backurl = "edit.tmpl.php?t=".$tablename."&id=".$substid.'&tabArgu[PRC]='.$c_proto_id;
    	
    	$loop_shAbsNotes = 1;
    	$optRows         = NULL;
    	
    	$edFormLib->setProtoID($sqlo, $c_proto_id);
    	$edFormLib->set_mo_obj('CONCRETE_SUBST',$substid);
    	
    	//$optRows = array(array('QC status',$qcStatus));
    	$optarr  = array(
    		'headAdd'     => '&nbsp;'.$oneProtoEditLogo,
    		'sh:AP:NOTES' => $loop_shAbsNotes,
    		'sh_info_head'=> $this->sh_options['sh_info_head'],
    	    "backurl"     => $backurl
    	);
    	$editProtoAllow = $edFormLib->showIntro( $sqlo,
    		$loopEditAllow, $info_title, $optRows, $optarr );
    	
    	$devopt = array(
    	    "backurl"    => $backurl,
    	    "showRequ"   => $qm_check_arr['showRequ'],
    	    "qm_check"   => $qm_check_arr['qm'],
    		"nonqm_check"=> $qm_check_arr['nonqm'],
    	    "plan_check" => $qm_check_arr['plan_check'],
    		"mo.CERT_FLAG"=> $this->mo_CERT_FLAG
    		);
    	
    	$edFormLib->showform( $sqlo, $editProtoAllow, $devopt );
    	
    	if ($this->_debug>0) {
    		debugOut("prEditIsActive:$prEditIsActive userWants:$userWants qcEditAllow:$qcEditAllow p_writeacc:$cp_writeacc",__FUNCTION__);
    	}
    }
    
    /**
     * show help - text
     * @deprecated
     * @param array $qm_check_arr
     */
    private function _helpRow($qm_check_arr, $xtype_def) {
    	// $helpLib     = new fHelpC();
    	
        $help_url='o.CONCRETE_SUBST.tab_proto.html';
        if ($xtype_def==2) $help_url='o.CONCRETE_SUBST.qcprot.html';
        
        $tempHelpTxt = '['.fHelpC::link_show($help_url, "help").']';
    	$do_qm_check = $qm_check_arr['qm'];
    	
    	echo '<div id="xOptionsForm" style="display:none; background-color:#E0E0FF; padding-bottom:10px;">'."\n";
    
    	echo "<br>\n".'&nbsp;&nbsp;<span class="yGgray">Info:</span> '.$tempHelpTxt;
    	echo '&nbsp;&nbsp;&nbsp;<span class="yGgrayBold">QM-warnings:</span> ';
    	if ($do_qm_check) {
    		echo '<b>Shown</b> | <a href="preferences.php?var=o.CONCRETE_PROTO.qm_check&val=0">Hide</a>';
    	} else {
    		echo '<a href="preferences.php?var=o.CONCRETE_PROTO.qm_check&val=1">Show</a> | <b>Hidden</b>';
    	}
    	
    	echo '&nbsp;&nbsp;&nbsp;&nbsp;<span class="yGgrayBold">Non-QM-warnings:</span> ';
    	if ($qm_check_arr['nonqm']) {
    		echo '<b>Shown</b> | <a href="preferences.php?var=o.CONCRETE_PROTO.nonqm_check&val=0">Hide</a>';
    	} else {
    		echo '<a href="preferences.php?var=o.CONCRETE_PROTO.nonqm_check&val=1">Show</a> | <b>Hidden</b>';
    	}
    	echo '&nbsp;&nbsp;&nbsp;&nbsp;<span class="yGgrayBold">Planned parameters check:</span> ';
    	if ($qm_check_arr['plan_check']) {
    	    echo '<b>Shown</b> | <a href="preferences.php?var=o.CONCRETE_SUBST.plan_check&val=0">Hide</a>';
    	} else {
    	    echo '<a href="preferences.php?var=o.CONCRETE_SUBST.plan_check&val=1">Show</a> | <b>Hidden</b>';
    	}
    	echo '</div>'."\n";
    	// if ($statusInfoHtml!=NULL) echo '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'.$statusInfoHtml;
    }
    
    /**
     * set options
     * @param string $key
     * @param string $val
     */
    public function set_option($key, $val) {
    	$this->sh_options[$key] = $val;
    }
    
    private function _ana_one_PRC($sqlo, $cpid) {
        $info=array();
        $sqlo->Quesel('a.LAYOUT from CONCRETE_PROTO p join ABSTRACT_PROTO a on p.ABSTRACT_PROTO_ID=a.ABSTRACT_PROTO_ID where p.CONCRETE_PROTO_ID='.$cpid);
        $sqlo->ReadRow();
        $layout = $sqlo->RowData[0];
        $info['form_type']='default';
        if ($layout!=NULL) {
            $info['form_type']='form3';
        }
        return $info;
    }
    	
    /**
     * 
     * Enter description here ...
     * @param $sqlo
     * @param $sqlo2
     * @param $id   : CONCRETE_SUBST_ID
     * @param array $arguByKey
     * @param array $tabArgu
     * 	'PRC' => ID of active  protocol
     * @param array $options
     *  qcEditAllow : int
     *  userWants : string : substance-edit-mode:  'edit', 'view'
     *  xtype : int     : 
     *     (1) XTYPE_PREP
     *     (2) XTYPE_QC
     */
    function show(&$sqlo, &$sqlo2, $id, &$arguByKey, &$tabArgu, $options ) {
    
        ?>
        <script>
        function xShowOptions()  {
            id="xOptionsForm";
            if (document.getElementById(id).style.display == 'none')   {
                document.getElementById(id).style.display = '';
            }
        }
        </script>
        <?php
        
    	$tablename   = 'CONCRETE_SUBST';
    	$this->sucid = $id;
    	
    	$qcEditAllow=$options['qcEditAllow'];
    	$userWants  =$options['userWants'];
    	$xtype_def  =$options['xtype_def'];
    	
    	
    	$this->EditUrl = "edit.tmpl.php?t=".$tablename."&id=".$id."&tabArgu[PRC]=";
    	$this->_debug  = $_SESSION["userGlob"]["g.debugLevel"];
    	$this->mo_CERT_FLAG = $arguByKey['CERT_FLAG'];
    	
    	// default: Preparation
    	$info_title      = 'Preparation protocol';
    	//$nextStatusName  = 'produced';
    	
    	if ($xtype_def==2) {
    		$info_title    ='QC protocol';
    		//$nextStatusName='QC checked';
    	}
    	$qm_check_arr=array(); // do QM-checks ? : array( 'qm'=>0,1, 'nonqm'=>0,1 )
    	if ($_SESSION['userGlob']['o.CONCRETE_PROTO.qm_check']>0) {
    		$qm_check_arr['qm'] = 1;
    	}
    	if ($_SESSION['userGlob']['o.CONCRETE_PROTO.nonqm_check']>0) {
    		$qm_check_arr['nonqm'] = 1;
    	}
    	if ($_SESSION['userGlob']['o.CONCRETE_SUBST.plan_check']>0) {
    	    $qm_check_arr['plan_check'] = 1;
    	}
    	if ($this->_SUC_prefs['showRequ']>0) $qm_check_arr['showRequ'] = 1;
    	    
    	$xtype  = $xtype_def;
    	$QcProtlib = new oSUC_ProtoLogGui($sqlo, $id, $arguByKey, $qcEditAllow, $xtype);
    	$numOfProtocols = $QcProtlib->getNumProts();
    	$cprotoLog = $QcProtlib->getCprotoLog();
    	
    	// start actions
    	
    	echo '<ul>'."\n";
    	$suc_tab_nice = tablename_nice2($tablename) ;
    	if (!$numOfProtocols) {
    	    echo '<span style="color:gray;">'.$suc_tab_nice.' has no planned protocols.</span><br>';
    	}
    	
    	/**
    	 * - if only ONE protocol planned: show Proto-OVERVIEW + EDITFORM
    	 * - if more than ONE protocol:
    	 *   - show Proto-OVERVIEW or single-proto-editor (!)
    	 */
    	if ($numOfProtocols==1) {
    		
    	    $sh_opt = array('withCreator'=>1);
    	    $QcProtlib->showProtos($sqlo, $sh_opt);
    	    //echo '<a href="javascript:xShowOptions()" class="yGgray"><img src="res/img/settings.svg"> Settings</a>'."\n";
    	    // if ( $this->sh_options['sh_help_row']>=0 )
    	    //$this->_helpRow($qm_check_arr, $xtype_def);
    	    
    		echo '</ul>'."\n";
    		$c_proto_id=0;
    		if (sizeof($cprotoLog)) {
    			$oneEntry = current($cprotoLog);
    			$c_proto_id = $oneEntry['cp'];
    		}
    		// $form_type = $this->sh_options['form_type'];
    		
    		if ($c_proto_id) {
    		    
    		    $prc_info = $this->_ana_one_PRC($sqlo, $c_proto_id);
    		    
    			$prEditIsActive=0;
    			if($tabArgu['PRC']) $prEditIsActive=1;
    			switch ($prc_info['form_type']) {
    			    case 'form2':
    			         $this->_showProtForm2($sqlo, $id, $c_proto_id, $qcEditAllow, $info_title, $qm_check_arr, $userWants, $prEditIsActive);
    			         break;
    			    case 'form3':
    				    $this->_showProtForm3($sqlo, $id, $c_proto_id, $qcEditAllow, $info_title, $qm_check_arr, $userWants, $prEditIsActive);
    				    break;
    			    default:
    				    $this->_showProtEditor($sqlo, $id, $c_proto_id, $qcEditAllow, $info_title, $qm_check_arr, $userWants, $prEditIsActive);
    			}
    		}
    		
    		
    		return;
    	}
    	
    	//
    	// else: more than one protocol: show a more complex view
    	//
    	if ($tabArgu['PRC']) {
    		
    		echo '<a href="edit.tmpl.php?t=CONCRETE_SUBST&id='.$id.'"><img src="images/but.back.png"> Protocol overview</a> &gt; <b>Edit one protocol</b>'."<br /><br />\n";
    		$c_proto_id      = $tabArgu['PRC'];
    		if ($tabArgu['PRC']) $prEditIsActive=1;
    		
    		$prc_info = $this->_ana_one_PRC($sqlo, $c_proto_id);
    		
    		switch ($prc_info['form_type']) {
    		    case 'form2':
    			     $this->_showProtForm2($sqlo, $id, $c_proto_id, $qcEditAllow, $info_title, $qm_check_arr, $userWants, $prEditIsActive);
    			     break;
    		    default:
    			     $this->_showProtEditor($sqlo, $id, $c_proto_id, $qcEditAllow, $info_title, $qm_check_arr, $userWants, 1);
    		}
    		return;
    	}
    	
    	// show OVERVIEW of protocols
    	$sh_opt = array( 'withCreator'=>1, 'withExecDate'=>1 );
    	$QcProtlib->showProtos($sqlo, $sh_opt);
    	
    	//echo '<a href="javascript:xShowOptions()" class="yGgray"><img src="res/img/settings.svg"> Settings</a>'."\n";
    	//$this->_helpRow($qm_check_arr, $xtype_def);
    	
    
    	echo '</ul>'."\n";
    
    	
    	if ( !sizeof($cprotoLog) ) {
    		return;
    	}
    	
    
    	/**
    	 * show EACH protocol in DETAIL
    	 */
    	foreach( $cprotoLog as $oneProtoArr) {
    		
    		$c_proto_id   = $oneProtoArr['cp'];
    		
    		if ($c_proto_id) {
    		    $prc_info = $this->_ana_one_PRC($sqlo, $c_proto_id);
    			$prEditIsActive=0;
    			if($tabArgu['PRC']) $prEditIsActive=1;
    			switch ($prc_info['form_type']) {
    			     case 'form2':
    				    $this->_showProtForm2($sqlo, $id, $c_proto_id, $qcEditAllow, $info_title, $qm_check_arr, $userWants, $prEditIsActive);
    				    break;
    			     case 'form3':
    			         $this->_showProtForm3($sqlo, $id, $c_proto_id, $qcEditAllow, $info_title, $qm_check_arr, $userWants, $prEditIsActive);
    			         break;
    			     default:
    				    $this->_showProtEditor($sqlo, $id, $c_proto_id, $qcEditAllow, $info_title, $qm_check_arr, $userWants, $prEditIsActive);
    			}
    		}
    	}
    	
    	
    		
    
    }



}
