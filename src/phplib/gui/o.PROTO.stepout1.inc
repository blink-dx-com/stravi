<?php
require_once ( "o.PROTO.steps.inc" );
require_once ( "javascript.inc" );

/**
 * class protocol editor class; level 1 
 * OLD module-name: class.protocol.inc
 * FORM-PARAMS: index of each array is is STEP_NO
 *    $quanti[]
	  $newnote[]
	  $concsubst[] CONCRETE_SUBST_ID
	  $devids[]	   CHIP_READER
	  $not_done[]
 * IMPORTANT: in EDITMODE: you need all js-code from js_formAll() !
 * @package o.PROTO.stepout1.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq SREQ:0001017: g > protocol editor class; level 1 
 * @unittest exists as plugin
 */
class protostep {			
    
    // show ABSTRACT or CONCRETE MODE
    const CA_MODE_CONC='C';
    const CA_MODE_ABS ='A';
	
var $abstr_proto_id;
var $conc_proto_id ;
var $ca_mode;        // CA_MODE: ABSTRACT or CONCRETE MODE
var $step_array;
var $editAllow;		 // 0,1 protocol-view is in edit/viewmode ?
var $editConc;       // editAllow for concrete_proto ?
var $main_step_cnt;  // counter for main steps
var $sub_step_cnt;   // counter for sub steps
var $checkboxes;     // 0|1
var $checkboxes_ext; // 0|1 
var $abstract_show;  // 0|1 show abstract steps in concrete proto ???
var $absSubstCol_sh; // 0|1 show column of abstract substance ?
var $tmp_c_color;
var $step_out_cnt;   /* count steps printed on screen by outstep */
var $options;		 // input-options

var $deact_notshow;  // do not show deactivated steps
var $deactSteps_cnt; // count deactivated steps
var $sub_step_col_cnt;
var $linexHead;      // special header
var $opt_out_type;   // ["display"]
                     // ["printer"] => page width minimal output
var $opt_out_nowrap; // "" or "NOWRAP"
var $opt_showReq;	 // [0],1 show required fields ???
var $opt_qc;	     // DEPRECATED: special QC-options ???
var $showGoCol;      // show go column ?
var $showStepLnk;	 // [0]|1 show Step_nr Link ???
var $colarr;         // array of colors
var $ederr;			 // count missing mandatory fields; get value with  get_ederr()
					 // inactive steps: ignore this step, no $ederr
var $colspan;		 // current colspan
var $fieldSZnote;	 // current field size of notes
var $opt_show_SUA_name; // 0,1 if USC exists, show also show SUA-name ?




/**
 * construct class
 * @param object $sql, 
 * @param int $dummy -- not used any more ..
 * @param $abstr_proto_id : ABSTRACT_PROTO_ID
 * @param $editAllow  : allow edit-mode? 0,1
 * @param $checkboxes : 0|1 : show checkboxes for single step actions like copy
 * @param int $conc_proto_id : can be 0 
 * @param array $options  
 * 	"abstrStepsShow" : 0,1
 *  "ca_mode"  : OPTIONAL: which protocol mode: 'C', 'A'
 *      use this, if no  $conc_proto_id is given, but you want the CONC-protocol-view  
 *  "out_type"      ["display"] | "printer" => optimzed output
 *  "showRequ"      0,1 : set $this->opt_showReq
 *  "noStepScan"    [0] | 1 do not scan the protocol_steps !!!
 *    ** step_array[] is empty than **
 *  "showStepLnk" = 0|1 show Step_nr Link ???
 *  "linexHead"   = do not intend extra spaces for sub steps
 *  "showIntStepNo"  0, {1] : show INTERNAL step-numbers ?
 *  "showStepNo"  [1], 0 : do not show step-numbers
 *  "qc"          [0], 1  DEPRECATED: QC-specials; overrules $_SESSION['userGlob']["g.appQC"]
 *  "absSubstIdShow" 0,1 show ID of abstract_subst, only if ABSTRACT_PROTO
 *  "cs.inp"     [string] select the input mechanism to input a concrete substance, 
 *     - [dblink]  : use classic dblink-button 
 *     - text      : give text-field to input the substance
 *  "header.show" : [1], -1 show table header ?
 */
function __construct( &$sql,  $dummy, $abstr_proto_id, $editAllow,	$checkboxes,	$conc_proto_id, $options = array() ) {
    
	
	$optShow = unserialize($_SESSION['userGlob']["o.CONCRETE_PROTO.show"]);
    $this->ederr = 0;
	$this->colarr[0]="#F0F0B0";
	$this->colarr[1]="#F8F8C0";
	$this->colarr[2]="#FFFFD0";
	$this->sub_step_col_cnt=0;
	$this->showStepNo    = 1;
	$this->showIntStepNo = 0;

	$this->abstr_proto_id = $abstr_proto_id;
	$this->conc_proto_id  = $conc_proto_id;
	$this->editAllow      = $editAllow;
    $this->editConc       = 0;
	$this->tmp_c_color    = "#F0F080"; /* background for concrete values*/
	$this->main_step_cnt  = 0;
	$this->sub_step_cnt   = 0;
	$this->sub_step2_cnt  = 0;
	$this->checkboxes     = $checkboxes;
	$this->fieldSZnote	  = 22;
	$this->opt_showReq	  = 0;
	$this->opt_show_SUA_name = 1;
	
	if (!is_array($options)) $options=array();
	
	$this->ca_mode = self::CA_MODE_ABS;
	if ($this->conc_proto_id) $this->ca_mode = self::CA_MODE_CONC;
	if ($options["ca_mode"]!=NULL) $this->ca_mode = $options["ca_mode"];
	
	$this->options = $options;
	
	if ($options["showStepNo"]!="") $this->showStepNo = $options["showStepNo"];
	if ($options["showIntStepNo"]!="") $this->showIntStepNo = $options["showIntStepNo"];
	if ($options["showStepLnk"])   $this->showStepLnk = $options["showStepLnk"];
	if ($options["linexHead"]!="") $this->linexHead   = $options["linexHead"];
// 	if ($options["qc"]!="") 	   $this->opt_qc      = $options["qc"];
// 	else {
// 		$this->opt_qc = $_SESSION['userGlob']["g.appQC"];
// 	}
    
    if ( !empty($options["out_type"]) ) 
	    $this->opt_out_type = $options["out_type"];
    else $this->opt_out_type = "display";
    
    $this->opt_out_nowrap = 'class="nowrap"';
    if ($this->opt_out_type == "printer")
         $this->opt_out_nowrap = ""; // width minimal output
    
    if ( !empty($options["checkboxes"]) ) 
	    $this->checkboxes= $options["checkboxes"];
    
    // checkbox variable name contains also abstract_proto_id 
    // useful, if this function is called for different protocols in one FORM
    if ( !empty($options["checkboxes_ext"]) ) $this->checkboxes_ext=1; 
 	
    $this->abstract_show = 0; /* 0|1 */
	$this->absSubstCol_sh= 0;

    if ( $options["showGoCol"] ) {
        $this->showGoCol = 1;
    } 
    
    if ( $optShow["inactStepsNotShow"] && !$this->editAllow) {
        $this->deact_notshow = 1;
    }
    
	
	if ( $options["showRequ"]>0 ) $this->opt_showReq = 1;
	if ( $optShow["shNoteSz"]>0 ) $this->fieldSZnote = $optShow["shNoteSz"];
	if ($this->fieldSZnote>200)   $this->fieldSZnote = 200; // maximum!
	
	
	if ( $this->ca_mode == self::CA_MODE_ABS ) {
		$this->abstract_show  = 1;
		$this->absSubstCol_sh = 1;
	}
	if ( $editAllow ) {
	    if ($this->ca_mode == self::CA_MODE_CONC) {
			$this->editConc=1;
			$this->absSubstCol_sh = 0; // !!! DO NOT SHOW absract_subst COLUMN
		}
        $this->abstract_show = 1;
	}
	$this->jsFormLib = new gJS_edit();
	
    if ( isset($options["abstrStepsShow"]) ) {
		$this->abstract_show  = $options["abstrStepsShow"];
		$this->absSubstCol_sh = $options["abstrStepsShow"]; 
	}  

    $this->deactSteps_cnt=0;

	$this->protLib = new gProtoOrg( );
	$this->protLib->setProto( $sql, $abstr_proto_id, $conc_proto_id, $options);
	$this->step_array = &$this->protLib->step_array;
    
	
}

// PRIVATE ....
// $table : ['CONCRETE_SUBST'], 'CHIP_READER'
function _js_fields_out( $step_nr, $cobj_id, $ca_objname, $modalButId, $tmp_cond, $table=NULL) {
	// - for modal-dialog purposes 
	// - the JS-function openproj()  comes from edit.tmpl.php 
	
	// $tmplen="";
	// if ( strlen($ca_objname)>5 ) $tmplen = " style=\"width:".(strlen($ca_objname)*0.6)."em;\"";
	if ($table=='') { 
		$table   = 'CONCRETE_SUBST';
	} 
	
	if ($table=='CHIP_READER') {
		$varname = 'devids['.$step_nr.']';
		$butID  = 'xd'.$modalButId;
	} else {
		$varname = 'concsubst['.$step_nr.']';
		$butID  = 'x'.$modalButId;
	}
	
	$name_out = $ca_objname; // if no object: contains string of SPACES for nice input-button
	if ($cobj_id) {
		$name_out = $cobj_id.": ".$ca_objname;
	}
	
	$this->jsFormLib->setTable($table, $butID);
	$olopt= array('butxtra'=> 'class=tbut');
	$out  = $this->jsFormLib->getObjLnk( $varname, $cobj_id, $name_out, $olopt);
	echo $out;
	echo '</td><td class="nowrap">';
	$osopt = NULL;
	$out   =  $this->jsFormLib->getObjSelector( $osopt );
	$butIdTmp = $this->jsFormLib->butid;
	echo "<a href=\"javascript:openwin('".$table."','".$butIdTmp."','".$tmp_cond."','')\">";
	echo "<IMG src=\"images/select_abs.svg\" TITLE=\"select by abstract\"></a> ";
	echo $out;
	
}
 
/**
 * show CONCRETE_SUBST column in EDIT-modus
 * 
 * @param array $parm 
 *  ["A_SUBST_ID"]
	["A_SUBST_name"]
	["mand_val"]
 * @param array $cPars
 *  ['cs']
    ['canam']
 * @param  $conc_color
 * @param  $step_nr_par
 */
function _edConcSubst( &$parm, &$cPars, $conc_color, $step_nr_par) {
	
	$hadButtons = 0;
	if ( $parm["A_SUBST_ID"] OR $cPars['cs'] ) {
		$this_col = "";
		if ($cPars['cs']!="") $this_col = " bgcolor=".$conc_color;	
		echo '<td ".$this_col." class="nowrap">';
		if ( !$this->absSubstCol_sh ) echo "<img src=\"images/ic.ABSTRACT_SUBST.gr.gif\" title=\"planned\"> ".
					$parm["A_SUBST_name"]."<br>"; // show abstract subst
					
		if ($this->opt_showReq AND strstr($parm["mand_val"],"s")) {
			echo "<img src=\"images/but.ed13.gif\"> ";
		} 
		
		$modalButId = $this->step_out_cnt; 
		if ($this->options['cs.inp']=='text') {
			echo 'ID: <input type="text" name="concsubst['.$step_nr_par.']" value="'.$cPars['cs'].'" size=10>';
			echo '</td><td class="nowrap">&nbsp;';
		} else {
			// only concrete_subst selection, which is parent from $parm["A_SUBST_ID"]
			$tmp_cond = "&condclean=1&searchAlias=absID&searchtxt=".$parm["A_SUBST_ID"];
			$this->_js_fields_out($step_nr_par, $cPars['cs'], $cPars['canam'], $modalButId, $tmp_cond);
		}
		echo "</td>";
		$hadButtons = 1;
	} 
	
	if ( $parm["ABS_DEV_ID"] OR $cPars['dev'] ) { 
		
		if ($hadButtons) {
			$this->_RowExtendEnd();
			$this->_RowExtend($parm);
		}
		
		$this_col = "";
		if ($cPars['dev']!="") $this_col = " bgcolor=".$conc_color;	
		echo '<td '.$this_col.' class="nowrap">';
		if ( !$this->absSubstCol_sh ) echo "<img src=\"images/icon.A_CHIP_READER.gif\" title=\"planned\"> ".
					$parm["ABS_DEV_name"]."<br>"; // show abstract
					
		if ($this->opt_showReq AND strstr($parm["mand_val"],"d")) {
			echo "<img src=\"images/but.ed13.gif\"> ";
		} 
		
		$modalButId = $this->step_out_cnt; 
		// only concrete_subst selection, which is parent from $parm["A_SUBST_ID"]
		$tmp_cond = "&condclean=1&searchAlias=absID&searchtxt=".$parm["ABS_DEV_ID"];
		$this->_js_fields_out($step_nr_par, $cPars['dev'], $cPars['devna'], $modalButId, $tmp_cond, 'CHIP_READER');
		echo "</td>";
		$hadButtons = 1;
	} 
	
	
	if ( !$hadButtons ) {
		$this_col = "";
		// to have a INPUT-field matrix at   the HTML document 
		echo "<td>&nbsp;</td>";  // <input type=hidden name=dummy><input type=hidden name=dummy>
		echo "<td>&nbsp;</td>";
	}
}

// show column: concrete_subst, device
function _shConcSubst( $parm, $cPars, $conc_color, $step_nr_par) {
	// 
	$conc_subst_show = NULL;
	$this_col = NULL;
	$tmpswarn = "";

	if ( $cPars['cs'] == "" ) {
		if ($this->opt_showReq AND strstr($parm["mand_val"],"s") and !$cPars['dea']) {
			$tmpswarn =  "<img src=\"images/but.ederr13.gif\"> ";
			$this->ederr++;
		}
		$conc_subst_show = $parm["A_SUBST_name"]; // if ( !$cPars['dea'] ) 
	} else {
		$this_col = "bgcolor=".$conc_color;
		$conc_subst_show = "<B><a href=\"javascript:goSubst( '" .$cPars['cs']. "' )\">".
			$cPars['canam']."</a></B>";
		if ($this->opt_show_SUA_name>0) {
    		$conc_subst_show .= " [ID:".$cPars['cs']."]";
    		$conc_subst_show .= "<br>MAA: ".$cPars['anam'];
		}
	}

	// device
	$dev_show = NULL;
	if ( $cPars['dev'] == "" ) {
		
		if ($this->opt_showReq AND strstr($parm["mand_val"],"d") and !$cPars['dea']) {
			$tmpswarn =  "<img src=\"images/but.ederr13.gif\"> ";
			$this->ederr++;
		}
		if ( $parm['ABS_DEV_name']!=NULL ) $dev_show = 'Dev: '.$parm['ABS_DEV_name']; // !$cPars['dea'] and 
	} else {
		$this_col = "bgcolor=".$conc_color;
		$dev_show = "Dev: <B><a href=\"javascript:goDev( '" .$cPars['dev']. "' )\">".
			$cPars['devna']."</a></B>";
		if ($this->opt_show_SUA_name>0) {
			$dev_show .= " [ID:".$cPars['dev']."]";
			$dev_show .= "<br>abs: ".$cPars['devAnam'];
		}
	}
	if ($conc_subst_show!=NULL and $dev_show!=NULL) $tmpbr = '<br>';
	else $tmpbr = NULL;
	echo "<td " .$this_col. ">".$tmpswarn;
	echo $conc_subst_show . $tmpbr . $dev_show ."&nbsp;</td>";
}

function _getAbsDevCol(&$parm) {
	if ($parm['ABS_DEV_name']!='') $outtmp =  '<b>Dev:</b> '.$parm['ABS_DEV_name'];
	else $outtmp = '';
	echo "<td>".$outtmp."</td>";
}

// show step-row extension
function _RowExtend( &$parm ) {
    echo "<tr>"; // ".$this->_bgcolor_show."
    if ($this->showStepNo)  echo "<td>&nbsp;</td>";
    if ($this->showIntStepNo)  echo "<td>&nbsp;</td>";
	echo "<td>&nbsp;</td>";
	echo "<td>&nbsp;</td>";
	if ($this->showGoCol)  echo "<td>&nbsp;</td>";
	if ( $this->absSubstCol_sh) {
		$this->_getAbsDevCol($parm);
	}
}
function _RowExtendEnd() {
	$maxCol=5;
	$i=0;
	while ( $i<$maxCol ) {
		echo "<td>&nbsp;</td>";
		$i++;
	}
	echo "</tr>\n";
}

/**
 * handle the start of a protocol-step-ROW
 * @param $bgcolor : '', 'main', 'inactive'
 * @param array $li_opt
 *    'check_ext' : string
 */
function line_show_tmp($step_nr, $sub_step, $bgcolor, $li_opt=array()) {
	
	$bgcolor_show ="";
	
	if ( $this->linexHead>0 ) {
	    
		$bgcolor_show=" bgcolor=#E0E0E0";
		if ($bgcolor== "inactive") $bgcolor_show=" bgcolor=#FFE0E0";
		echo "<tr>"; // ".$bgcolor_show."
		echo "<td>&nbsp;";
		
	} else {
		if ($bgcolor== "main")     $bgcolor_show=" bgcolor=#E0E0E0";
		if ($bgcolor== "inactive") $bgcolor_show=" bgcolor=#FFE0E0";
		
		echo "<tr>"; 
		
		
		# build step_nr NICE; see also code gProtoOrg::stepnr_nice_add() 
		$tmp_spaces='';

		if ($sub_step>0) { // is it a SUB step ?
		    
			if ($bgcolor!="inactive") {
				$colid = $this->sub_step_col_cnt;
				$bgcolor_show = " bgcolor=".$this->colarr[$colid];
				$this->sub_step_col_cnt++;
				if ($this->sub_step_col_cnt>2)  $this->sub_step_col_cnt=0;
			}

			$tmp_spaces = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
			
			
			if ($sub_step>1) {
			    $tmp_spaces .= "&nbsp;&nbsp;&nbsp;&nbsp;"; 
			    $this->sub_step2_cnt++;
			    $tmp_step = $this->main_step_cnt .'.'. $this->sub_step_cnt.'.'.$this->sub_step2_cnt;
			} else {
			    $this->sub_step2_cnt=0;
    			$this->sub_step_cnt++;
    			$tmp_step = $this->main_step_cnt .'.'. $this->sub_step_cnt;
			}
			
		} else {

		    $this->main_step_cnt++;
			$tmp_step = $this->main_step_cnt;
			
			
			$this->sub_step_cnt =0;
			$this->sub_step2_cnt=0;
			$this->sub_step_col_cnt=0;
			
		}

		if ($this->showIntStepNo) {
		    echo '<td>'.$step_nr."</td>";
		}
		if ($this->showStepNo) {
		    echo '<td class="nowrap">';
		    if ( $this->checkboxes ) {
		        $check_ext="";
		        if ( $li_opt['check_ext'] ) $check_ext=$li_opt['check_ext'];  // add ABSTRACT_PROT_ID
		        echo "<input type=checkbox name=\"step".$check_ext."[".$step_nr."]\" value=0> ";
		    }	
		    if ($this->showStepLnk) echo "<a href=\"javascript:a_gostep('".$this->abstr_proto_id."','".$step_nr."')\" >".$tmp_step.".</a> ";
		    else echo $tmp_step.".&nbsp;";
		    echo "</td>";
		}
		echo "<td ".$this->opt_out_nowrap.">&nbsp;";
		echo $tmp_spaces;
		
	}
	$this->_bgcolor_show = $bgcolor_show;
}  

/**
 * show a text row
 * @param string $text
 */
private function line_show_extra($text) {
    echo "<tr>"; // $bgcolor_show
    echo '<td colspan="'.$this->colspan.'">'.$text.'</td>';
}

/**
 * init protocol step table-head
 */
function table_init($extracols=NULL, $shOpt=NULL) {
	//$bgcolor= "#999933";
	
	$tx_con_color = ''; // '<span style="color:'.$this->tmp_c_color.';">';

    // WARNING: do not change the order!
    // mind the submit buttons for concrete_protocol: edit !!!
    
	echo '<table class="yProtoTable" >';

	
	$tmpcols = NULL;
	if ($this->showIntStepNo) $tmpcols[] =   "STEP_NR";
	if ($this->showStepNo) $tmpcols[] =   "#";
	$tmpcols[] 	=   "Description\n";
	if ($this->editConc)        $tmpcols[] =   "<img src=\"images/but9.del.gif\" TITLE=\"make step inactive\">"; // TBD: inactive
	if ( $this->showGoCol     ) $tmpcols[] =   $tx_con_color."<IMG src=\"images/but.gocp13.gif\" border=0>";
	if ( $this->absSubstCol_sh) $tmpcols[] =   "Material<br>planned\n";
	if ( $this->ca_mode==self::CA_MODE_CONC ) $tmpcols[] =   $tx_con_color."Material<br>NOW";
	if ( $this->editConc      ) $tmpcols[] =   "&nbsp;";  // select field
	if ( $this->abstract_show ) $tmpcols[] =   "Quantity<br>planned\n";
	if ( $this->ca_mode==self::CA_MODE_CONC )  $tmpcols[] =   $tx_con_color."Quantity<br>NOW";
	$tmpcols[] =   "Unit\n";
	if ( $this->ca_mode==self::CA_MODE_CONC )  $tmpcols[] =   $tx_con_color."Notes<br>NOW";
	
	if ( $this->abstract_show ) $tmpcols[] =   "Notes&nbsp;planned";
	if ( $this->editAllow ) {
	    if ($this->ca_mode==self::CA_MODE_ABS ) $tmpcols[] ="Edit";
	}
	
	$this->colspan = 0;
	$col_str_arr=array();

	foreach( $tmpcols as $val) {
	    $col_str_arr[]= "<th>".$val."</th>";
		$this->colspan++;
	}

	if ($extracols!=NULL and is_array($extracols) ) {
		$classTag=NULL;
		if ($shOpt['xColsCssCls']!=NULL) {
			$classTag=' class="'.$shOpt['xColsCssCls'].'"';
		}
		foreach( $extracols as $val) {
		    $col_str_arr[]= '<td'.$classTag.'>'.$val.'</td>';
			$this->colspan++;
		}
	}

	
    if ($this->options["header.show"]>=0) {
        echo '<tr align=center>'."\n";
        foreach($col_str_arr as $row) {
            echo $row."\n";
        }
        echo "</tr>\n";
  
	}
	$this->step_out_cnt=0;	
	
}


function table_end(  ) {
	echo "</table>\n";
	if ( $this->opt_showReq AND $this->ederr) echo "<img src=\"images/but.ederr13.gif\"> <font color=red>Missing mandatory fields: <B>".$this->ederr."</B></font><br>"; 
    if (  $this->deact_notshow && $this->deactSteps_cnt) {
        echo "<I>Number of hidden inactive steps: ".$this->deactSteps_cnt."</I><br>";
    }
	
}
 



function _editButtons( &$sql, $proto_id, $step_nr, $sub_step ) {
	
	echo '<td class="nowrap>'; // control column
	echo "<a href=\"javascript:delstep(".$proto_id.",".$step_nr.")\" >";
	echo "<img src=\"images/but9.del.gif\" TITLE=\"Delete step\" border=0></a> &nbsp;";

	/* search for last SUB-STEP-number */
	$sqls="select max(SUB_STEP_NR) from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$proto_id. 
			" AND MAIN_STEP_NR=".$step_nr;
	$sql->query("$sqls");
	$sql->ReadRow();
	$maxsubstep = $sql->RowData[0];
	if ( $maxsubstep<=0  ) $maxsubstep=0;
	//$main_step_new=$step_nr;
	
	echo " <a href=\"javascript:newstep(".$proto_id.",".$step_nr.", 'NEW_PARAM')\" >";
	echo "<img src=\"images/but9.new.gif\" TITLE=\"New step\" border=0></a>";
	if ( $sub_step>0 ) { /* it is a SUB step */
		echo "  <a href=\"javascript:movestep(".$proto_id.",".$step_nr.",'LEFT')\" > <img
		src=\"images/but9.left.gif\" TITLE=\"main\" border=0></a>";
	} else {
		echo "  <img src=\"images/0.gif\" width=9 height=9>";
	}

	echo "  <a href=\"javascript:movestep(".$proto_id.",".$step_nr.",'UP')\" ><img
	src=\"images/but9.up.gif\" TITLE=\"up\" border=0></a>";
	echo "  <a href=\"javascript:movestep(".$proto_id.",".$step_nr.",'DOWN')\" ><img
	src=\"images/but9.down.gif\" TITLE=\"down\" border=0></a>";
	if ( $sub_step<=0 ) { /* it is a MAIN step */
		echo "  <a href=\"javascript:movestep(".$proto_id.",".$step_nr.",'RIGHT')\" > <img
		src=\"images/but9.right.gif\" TITLE=\"sub\" border=0></a>";
	} 

	echo "</td>";
}
   

// PUBLIC function for class protostep_cmp
function conc_proto_params ( &$sql, $step_nr, $c_proto_id, $conc_color  ) {
	return ( $this->protLib->conc_proto_params ( $sql, $step_nr, $c_proto_id, $conc_color) );
}

// PUBLIC
function &getStepArray() {
	return ($this->protLib->step_array);
}


/**
 * start output of a single step of ONE protocol (without row end </tr> )
 * @param $opt 
 *    "xtratxt" put text to first column
      "step_nr_par" : alternative STRING for <input> fields (used by obj.concrete_proto.m_comp2e.php)
      "cPars" : array: alternative cPars, only used, if $this->conc_proto_id  not set
 * @return  int 
 *   0 : o.k.
 * 	 -1: not shown   
 */
function outStepStart( $step_nr, &$sql, $opt = NULL ) {
 
	$tempenc = $_SESSION['globals']['Db.encoding'];
	if ($tempenc==NULL) $tempenc='UTF-8';
	
   	$conc_color  = $this->tmp_c_color;
    $editAllow   = $this->editAllow;
    $proto_id    = $this->abstr_proto_id;
    $step_nr_par = $step_nr;
	$parm		 = NULL; /* parameters:
					"A_SUBST_ID"
					"A_SUBST_name"	
					"H_UNIT_ID"	
						 */

    if ($opt["step_nr_par"]) $step_nr_par = $opt["step_nr_par"];

    $sqls= "select STEP_NR, NAME, ABSTRACT_SUBST_ID, QUANTITY, H_UNIT_ID, ".
		   " MAIN_STEP_NR, SUB_STEP_NR, NOTES, IS_SAMPLE, MANDATORY_VALUES, ABS_DEV_ID ". 
           " from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=" .$proto_id. " AND STEP_NR=".$step_nr ;
    $sql->query("$sqls");
    $sql->ReadRow();

    $absStName  = $sql->RowData[1];
    $value      = $sql->RowData[3];
    //$main_step  = $sql->RowData[5];
    $sub_step	= $sql->RowData[6];
    $notes_tmp	= $sql->RowData[7];
    $is_sample	= $sql->RowData[8];
	
	$parm["mand_val"]      = $sql->RowData[9];
    $parm["A_SUBST_ID"]    = $sql->RowData[2];
    $parm["H_UNIT_ID"]	   = $sql->RowData[4];
	$parm["ABS_DEV_ID"]	   = $sql->RowData[10];
    $parm["A_SUBST_name"]  = '';
	$parm["ABS_DEV_name"]  = '';
	
	if ( strlen($notes_tmp) > 40 ){ 
        $notes = $notes_tmp;        //TBD: substr( $notes_tmp, 0, 40 ) . "&nbsp;..."; 
    } else  $notes = $notes_tmp; 
	
    $unitname      = "";
	$substname_href= "";
	//$row_type='[STD]';
	
    if ( $parm["A_SUBST_ID"] ) {
		$sqls= "select NAME from ABSTRACT_SUBST where ABSTRACT_SUBST_ID=".$parm["A_SUBST_ID"];
		$sql->query($sqls);
		$sql->ReadRow();
		$parm["A_SUBST_name"]=$sql->RowData[0];
		$substname_href="<a href=\"edit.tmpl.php?t=ABSTRACT_SUBST&id=".
				$parm["A_SUBST_ID"]."\">".$parm["A_SUBST_name"]."</a>";
    }
	
	if ( $parm["ABS_DEV_ID"] ) {
		$sqls= "select NAME from A_CHIP_READER where A_CHIP_READER_ID=".$parm["ABS_DEV_ID"];
		$sql->query($sqls);
		$sql->ReadRow();
		$parm["ABS_DEV_name"]=$sql->RowData[0];
		$dev_href="<a href=\"edit.tmpl.php?t=A_CHIP_READER&id=".
				$parm["ABS_DEV_ID"]."\">".$parm["ABS_DEV_name"]."</a>";
    }
	
    if ($parm["H_UNIT_ID"]) { 
        $sqls= "select NAME  from H_UNIT where H_UNIT_ID=".$parm["H_UNIT_ID"];
        $sql->query($sqls);
        $sql->ReadRow();
        $unitname=$sql->RowData[0];
    }

    if ( $sub_step<=0 )  {
        $bgcolor="main";
    } 
	
    $cPars = array();	
    if ( $this->conc_proto_id ) {
		// $cPars will be used later ...
		$cPars = $this->protLib->conc_proto_params ( $sql, $step_nr, $this->conc_proto_id, $conc_color); 
		$conc_color	= $cPars['cc']; 	
    } else {
        if (!empty($opt['cPars'])) {
            $cPars = $opt['cPars'];
            if ($cPars['cc']) $conc_color = $cPars['cc']; 
        }
    }
             	
    $actioner_col = $absStName;    // action-name
    if ($absStName=='[SPACE]')  {
        $this->line_show_extra('&nbsp;');
        return 0;
    }
    if ($absStName=='[NOTES]')  {
        $notes_tmp2 = str_replace("\n", '<br>', $notes_tmp);
        $this->line_show_extra($notes_tmp2);
        return 0;
    }
        
    if ( $cPars['dea'] ) {
        $bgcolor="inactive";
        $actioner_col="<font color=gray>".$absStName."</font> <font color=red>(inactive)</font>";
        $this->deactSteps_cnt++;
    }

    if ($cPars['dea'] && $this->deact_notshow) {    
        // do not show line
		return -1;
    } 

	if ($is_sample)	$actioner_col = $actioner_col ." <img src=\"images/icon.SAMPLE.gif\" border=0>";
	
	$st_opt=array();  
	if ( $this->checkboxes_ext ) $st_opt['check_ext']="[".$proto_id."]";
	$this->line_show_tmp($step_nr_par, $sub_step, $bgcolor, $st_opt); 


	if ( $this->ca_mode==self::CA_MODE_CONC ) {  //START: [concrete proto step exists] ?
		
		if ($opt["xtratxt"]!="") echo $opt["xtratxt"];
		else    echo $actioner_col; 
		if ( $this->linexHead>0 ) {
		    echo "<td></td>";
		}

		echo "</td>";
		
		if ($editAllow ) {
			echo "<td><input type=checkbox name=not_done[$step_nr_par] value=1";
			if ( $cPars['dea'] ) echo " checked";
			echo ">";
			echo "</td>";
		}
		
		if ($this->showGoCol) {
			echo "<td>";
			if ( $cPars['is'] ) { /* concrete proto step exists ??? */
				echo "<a href=\"javascript:c_gostep(".$this->conc_proto_id.",'".$step_nr_par."')\" >";
				echo "<IMG src=\"images/but.gocp13.gif\" border=0></a>";
				// echo "<font color=#808030>NOW</font></a>";

			} else {
				echo "&nbsp;";
			}		
			echo "</td>";
		}
		if ($this->absSubstCol_sh ) {
			if ($parm['A_SUBST_name']!='') echo "<td>".$parm["A_SUBST_name"]."</td>"; // COL:ABSTRACT_SUBST
			else $this->_getAbsDevCol($parm);
		}
		
		if ($editAllow ) { // COL:CONCRETE_SUBST
			$this->_edConcSubst( $parm, $cPars, $conc_color, $step_nr_par);
		} else { // COL:CONCRETE_SUBST:view
			$this->_shConcSubst( $parm, $cPars, $conc_color, $step_nr_par);
		}
		
		if ($this->abstract_show ) echo "<td>&nbsp;".$value."</td>"; // COL:QUANTITY

		// COL: QUANTITY

		if ($editAllow) {
			$this_col = "";
			if ($cPars['qu']!="") $this_col = " bgcolor=".$conc_color;	
			echo "<td".$this_col.">";
			$xst='';
			if ($this->opt_showReq AND strstr($parm["mand_val"],"q")) {
			    $xst='style=" border: 2px solid #DFDF00;"'; //	echo "<img src=\"images/but.ed13.gif\"> ";
			}
			echo '<input name=quanti['.$step_nr_par.'] value="'.$cPars['qu'].'" size=8 '.$xst.'>';
		} else {
			// TBD: mandatory ...
			$this_col = "bgcolor=".$conc_color;
			$tmpqwarn = "";	
			if ( $cPars['qu'] == "" ) {
				if ($this->opt_showReq AND strstr($parm["mand_val"],"q") and !$cPars['dea']) {
					$tmpqwarn =  "<img src=\"images/but.ederr13.gif\"> ";
					$this->ederr++;
				}
				$this_col = "";
				if ( !$cPars['dea'] ) $conc_quant_show = $value;
			} else {
				$conc_quant_show = "<B>".$cPars['qu']."</B>";
			}

			echo "<td " .$this_col. ">".$tmpqwarn.$conc_quant_show;
		}

		echo "</td><td>&nbsp;".$unitname."</td>";
		
		// COL:NOTES
		$tmpNotesWrap   = "";
		$conc_notesHtml = htmlentities($cPars['cn'], ENT_COMPAT, $tempenc );
		
		if ($editAllow) {
			echo '<td style="display: inline-block;">';
			$tmpNotesWrap = " NOWRAP";
			$xst='';
			if ($this->opt_showReq AND strstr($parm["mand_val"],"n")) {
			    $xst='style=" border: 2px solid #DFDF00;"'; //	echo "<img src=\"images/but.ed13.gif\"> ";
			}
			echo '<input name=newnote['.$step_nr_par.'] value="'.$conc_notesHtml.'" size='.$this->fieldSZnote.' '.$xst.'>';
			echo "</td>";
		} else {
			if ($this->abstract_show ) {
				if ($cPars['cn'] == "" ) {
					echo "<td>";
				} else {
					echo "<td bgcolor=".$conc_color.">";
					echo $conc_notesHtml;
				}
				echo "</td>";
			} else {
				if ($cPars['cn'] == "" ) {
					echo "<td>";
					if ($this->opt_showReq AND strstr($parm["mand_val"],"n") and !$cPars['dea']) {
						echo "<img src=\"images/but.ederr13.gif\"> ";
						$this->ederr++;	
					}
					echo $notes;
				} else {
					echo "<td bgcolor=".$conc_color.">";
					echo $conc_notesHtml;
				}
				echo "</td>";
			}
		}
		
		if ( $this->abstract_show ) echo "<td".$tmpNotesWrap.">".$notes."</td>";
		
	} else {    // END: [concrete proto step exists]
		echo $actioner_col;

		echo "</td>";
		$aSubstOut = NULL;
		if ($parm["A_SUBST_ID"]) {
			$aSubstOut = "&nbsp;".$substname_href;
			if ($this->options["absSubstIdShow"]>0)  $aSubstOut .= " [ID:".$parm["A_SUBST_ID"]."]";
			if (strstr($parm["mand_val"],"s")) {
				$aSubstOut .= "<img src=\"images/but.ed13.gif\"> ";
			}
		}
		
		if ($parm["ABS_DEV_ID"]) {
			if ($aSubstOut!=NULL) $aSubstOut .= '<br>';
			$aSubstOut .= "&nbsp;Dev: ".$dev_href;
			if ($this->options["absSubstIdShow"]>0)  $aSubstOut .= " [ID:".$parm["ABS_DEV_ID"]."]";
			if (strstr($parm["mand_val"],"d")) {
				$aSubstOut .= "<img src=\"images/but.ed13.gif\"> ";
			}
		}
		echo "<td>".$aSubstOut."</td>"; // "absSubstIdShow"
		echo "<td>&nbsp;".$value;
		if (strstr($parm["mand_val"],"q")) {
			echo "<img src=\"images/but.ed13.gif\"> ";
		}
		echo "</td>";
		echo "<td>&nbsp;".$unitname."</td>";
		echo "<td".$tmpNotesWrap.">".$notes;
		if (strstr($parm["mand_val"],"n")) {
			echo "<img src=\"images/but.ed13.gif\" hspace=2 title=\"edit notes\"> ";
		}
		if (strstr($parm["mand_val"],"a")) {
			echo "<img src=\"images/but.ed13.gif\" hspace=2 title=\"edit attachment\"> ";
		}
		echo "</td>";
		if ($editAllow ) {
			$this->_editButtons( $sql, $proto_id, $step_nr, $sub_step );
		}
	}
	
}  

function outStepEnd($ok) {
	if ($ok>=0) echo "</tr>\n";
	$this->step_out_cnt++;
}


/**
 * show a single stop of ONE protocol
 * @param array $opt -- see outStepStart()
 */
function outstep( $step_nr, &$sql, $opt = NULL ) {
	$ok = $this->outStepStart( $step_nr, $sql, $opt);
	$this->outStepEnd($ok);
}
// show full protocol table
function tableOut(&$sql) {
	$this->table_init();
	$step_count = sizeof( $this->step_array ) ;	
	$i=0;
	
	while ( $i< $step_count ) {
		$step_nr = $this->step_array[$i];
		$this->outstep( $step_nr, $sql );
		$i++;
	}
	$this->table_end();
}                                          

/**
 * get $this->ederr
 */
function get_ederr() {
	return $this->ederr;
}




}
