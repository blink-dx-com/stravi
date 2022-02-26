<?php

require_once ("visufuncs.inc");
require_once ("f.objview.inc");
require_once ("gui/o.EXP_ProtoLogGui.inc");
require_once ("gui/o.CONCRETE_PROTO.form2.inc");
require_once ("gui/o.CONCRETE_PROTO.edit.inc");
require_once ('f.help.inc');

/**
 * class to handle protocol editor for EXP
 * @package o.EXP.protos_show.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oEXP_xmode_prot_help {
	
	
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
	private $exp_id;
	
	function __construct(int $id) {
	    $this->tablename='EXP';
	    $this->exp_id  = $id;
	    $this->EditUrl = "edit.tmpl.php?t=".$this->tablename."&id=".$id."&tabArgu[PRC]=";
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
    public function getProtAccess(&$sqlo, $cpid) {
    	$writeacc = -1;
    	$o_rights = access_check($sqlo, 'CONCRETE_PROTO', $cpid);
    	if ( $o_rights["insert"]>0 ) $writeacc = 1;
    	return $writeacc;
    }
    
    /**
     * 
     * @param string $userWants   : 'edit','view'
     * @param int $mo_edit_allowed
     * @param int $cp_writeacc
     * @param int $c_proto_id
     * @return string
     */
    public function showEditButton(string $userWants, int $mo_edit_allowed, int $cp_writeacc, int $c_proto_id) {
        
        //$arg_list = func_get_args();
        //echo "DDDX:showEditButton: ".print_r($arg_list,1)."<br>";
        
    	if ($userWants=='edit') {
    		if ($mo_edit_allowed and $cp_writeacc>0) {
    			$oneProtoEditLogo='<a href="' .$this->EditUrl . $c_proto_id. '">'.
    				'<img src="images/but.edit.ac.gif" border=0 title="edit"></a>';
    		} else {
    			$oneProtoEditLogo='<img src="images/but.edit.no.gif" title="no access">';
    		}
    	}
    	
    	if ($mo_edit_allowed and $cp_writeacc<1) {
    		// if write-access on SUC and NO write-access on Proto ...
    		$oneProtoEditLogo .= ' <a href="p.php?mod=DEF/o.SUC.cp_reopen&id='. $this->exp_id . '&cpid=' . $c_proto_id. '">'.
    			'<img src="images/but.edit.unlock.gif" border=0 title="reopen"></a>';
    	}
    	
    	return $oneProtoEditLogo;
    }
    
    function _showProtForm2(&$sqlo, $c_proto_id, $qcEditAllow, $info_title, 
    	$qm_check_arr, $userWants, $prEditIsActive) {
    	
    	$tablename 	     = $this->tablename;
    	$oneProtoEditLogo= NULL;
    	$loopEditAllow   = 0; // default
    	
    	
    	if (!$prEditIsActive) {
    		// EDIT is NOT active
    		$cp_writeacc = $this->getProtAccess($sqlo, $c_proto_id);
    		$oneProtoEditLogo = $this->showEditButton($userWants, $qcEditAllow, $cp_writeacc, $c_proto_id);
    	} else {
    		// EDIT is active
    		$loopEditAllow   = $qcEditAllow;
    	}
    	
    	$edFormLib = new oCONCRETE_PROTO_form2();
    	
    	//$loop_shAbsNotes = 1;
    	$optRows         = NULL;
    	
    	$edFormLib->setProtoID($c_proto_id, $this->exp_id);
    	$optarr  = array(
    		'headAdd'     => '&nbsp;'.$oneProtoEditLogo
    	);
    	$editProtoAllow = $edFormLib->showIntro( $sqlo,	$loopEditAllow, $info_title, $optRows, $optarr );
    	
    	$devopt = array(
    		"backurl"   => "edit.tmpl.php?t=".$tablename."&id=".$this->exp_id.'&tabArgu[PRC]='.$c_proto_id,
    		
    	);
    	$dummy=NULL;
    	$edFormLib->showform( $sqlo, $dummy, $editProtoAllow, $devopt );
    	
    }
    
    /**
     * show the PROTO-Editor
     * @param $sqlo
     * @param $c_proto_id
     * @param $qcEditAllow
     * @param $info_title
    
     * @param $userWants  : substance-edit-mode:  'edit', 'view'
     * @paramint  $prEditIsActive 0,1: protocol edit-mode is active ?
     */
    private function _showProtEditor(&$sqlo, $c_proto_id, $qcEditAllow, $info_title, 
    	$userWants, $prEditIsActive) {
    	
    	$tablename 	     = $this->tablename;
    	$oneProtoEditLogo= NULL;
    	$loopEditAllow   = 0; // default
    	
    	
    	if (!$prEditIsActive) {
    		// EDIT is NOT active
    		$cp_writeacc = $this->getProtAccess($sqlo, $c_proto_id);
    		$oneProtoEditLogo = $this->showEditButton($userWants, $qcEditAllow, $cp_writeacc, $c_proto_id);
    	} else {
    		// EDIT is active
    		$loopEditAllow   = $qcEditAllow;
    	}
    	
    	$edFormLib = new oConcProtoEdform($sqlo);
    	
    	$loop_shAbsNotes = 1;
    	$optRows         = NULL;
    	
    	$edFormLib->setProtoID($sqlo, $c_proto_id);
    	//$optRows = array(array('QC status',$qcStatus));
    	$optarr  = array(
    		'headAdd'     => '&nbsp;'.$oneProtoEditLogo,
    		'sh:AP:NOTES' => $loop_shAbsNotes,
    		'sh_info_head'=> $this->sh_options['sh_info_head']
    	);
    	$editProtoAllow = $edFormLib->showIntro( $sqlo,  $loopEditAllow, $info_title, $optRows, $optarr );
    
    	$devopt = array(
    		"backurl"    => "edit.tmpl.php?t=".$tablename."&id=".$this->exp_id.'&tabArgu[PRC]='.$c_proto_id,
    		"showRequ"   => 0,
    		);
    	$dummy=NULL;
    	$edFormLib->showform( $sqlo, $editProtoAllow, $devopt );
    	
    	if ($this->_debug>0) {
    		debugOut("prEditIsActive:$prEditIsActive userWants:$userWants qcEditAllow:$qcEditAllow p_writeacc:$cp_writeacc",__FUNCTION__);
    	}
    }
    
    /**
     * show help - text
     * @param array $qm_check_arr
     */
    function _helpRow($qm_check_arr) {
    	//$helpLib     = new fHelpC();
    	$tempHelpTxt = '['.fHelpC::link_show("o.EXP.html", "help").']';
    	echo '<span class="yGgray">Info:</span> '.$tempHelpTxt;
    	
    }
    
    /**
     * set options
     * @param string $key
     * @param variant $val
     */
    public function set_option($key, $val) {
    	$this->sh_options[$key] = $val;
    }
    	
    /**
     * 
     * Enter description here ...
     * @param $sqlo
     * @param $sqlo2
     * @param $id   : EXP_ID
     * @param array $arguByKey
     * @param array $tabArgu
     * 	'PRC' => ID of active  protocol
     * @param int $qcEditAllow
     * @param string $userWants : substance-edit-mode:  'edit', 'view'
    
     */
    function show(&$sqlo, &$sqlo2, &$arguByKey, &$tabArgu, $qcEditAllow, $userWants) {
    
    	$tablename   = 'EXP';
    	$id = $this->exp_id;
    	
    	
    	$this->_debug  = $_SESSION["userGlob"]["g.debugLevel"];
    	
    	
    	// default: Preparation
    	$info_title      = 'Protocol';
    	//$nextStatusName  = 'closed';

    	
    	$QcProtlib      = new oEXP_ProtoLogGui($sqlo, $id, $arguByKey, $qcEditAllow);
    	// $numOfProtocols = $QcProtlib->getNumProts();
    	$cprotoLog      = $QcProtlib->getCprotoLog();
    	
    	// start actions
    	
    	//echo '<ul>'."\n";
    	

    	//
    	// else: more than one protocol: show a more complex view
    	//
    	if ($tabArgu['PRC']) {
    		
    	    echo '<a href="edit.tmpl.php?t='.$tablename.'&id='.$id.'"><img src="images/but.back.png"> Protocol overview</a> &gt; <b>Edit one protocol</b>'."<br /><br />\n";
    		$c_proto_id      = $tabArgu['PRC'];
    		if ($tabArgu['PRC']) $prEditIsActive=1;
    		if ($this->sh_options['form_type']=='form2') {
    			$this->_showProtForm2($sqlo, $c_proto_id, $qcEditAllow, $info_title,  $userWants, $prEditIsActive);
    		} else {
    			$this->_showProtEditor($sqlo, $c_proto_id, $qcEditAllow, $info_title, $userWants, 1);
    		}
    		return;
    	}
    	

    	$QcProtlib->showProtos($sqlo);
    
    	//echo '</ul>'."\n";
    	
    	if ( !sizeof($cprotoLog) ) {
    	    echo '<span class="yGgrayBold">No Protocols.</span><br>'."\n";
    	    return;
    	}
    
    
    	// show protocol details
    	echo '<br><span class="yGgrayBold">Protocol details ...</span><br><br>'."\n";
    	
    	reset ($cprotoLog);
    	foreach( $cprotoLog as $dummy=>$oneProtoArr) {
    		
    		$c_proto_id   = $oneProtoArr['cp'];
    		
    		if ($c_proto_id) {
    			$prEditIsActive=0;
    			if($tabArgu['PRC']) $prEditIsActive=1;
    			if ($this->sh_options['form_type']=='form2') {
    				$this->_showProtForm2($sqlo,  $c_proto_id, $qcEditAllow, $info_title, $userWants, $prEditIsActive);
    			} else {
    				$this->_showProtEditor($sqlo, $c_proto_id, $qcEditAllow, $info_title, $userWants, $prEditIsActive);
    			}
    		}
    		
    
    	}
    		
    
    }

}
