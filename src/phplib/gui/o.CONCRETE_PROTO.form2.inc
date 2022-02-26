<?php
require_once ("gui/o.PROTO.stepout1.inc");
require_once ("f.objview.inc");	
require_once 'o.SATTACH.subs.inc';
/**
 * protocol editor FORM2 mashup-component: very slim form ...
 * - can be used by other GUI-components 
 * - on submit, calls the DB_save_plugin: obj.concrete_proto.paras.php
 * - option 'backurl' defines the URL, which will be forwarded after the DB_save_plugin
 * @package o.CONCRETE_PROTO.form2.inc
 * @swreq SREQ:0001016: g > protocol editor modul 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */


/**
 * pure protocol step table
 * @author steffen
 *
 */
class oCONCRETE_PROTO_form2Table {
	
	private $step_out_cnt;
	private $protLib; // object
	private $step_array; // array of steps
	private $opt_out_type;
	private $colspan;
	private $mo_suc_id;
	
	/**
	 * init class
	 * @param unknown $sqlo
	 * @param unknown $sqlo2
	 * @param int $PRA_id : ABSTRACT_PROTO_ID
	 * @param int $editAllow
	 * @param int $PRC_id : CONCRETE_PROTO_ID
	 * @param array $stepOpt
	 */
	function __construct($sqlo, $sqlo2, $PRA_id, $editAllow, $PRC_id,  $mo_suc_id, $stepOpt=array() ) {
		
		$this->conc_proto_id  = $PRC_id;
		$this->abstr_proto_id = $PRA_id;
		$this->mo_suc_id      = $mo_suc_id;
		
		$this->editAllow      = $editAllow;
		$this->editConc       = 0;
		
		$this->main_step_cnt  = 1;
		$this->checkboxes     = $checkboxes;
		$this->fieldSZnote    = 20; // field size of CPR:STEP:NOTES field
		
		$this->options = $stepOpt;
		
		$this->protLib = new gProtoOrg( );
		$org_options=array();
		$this->protLib->setProto( $sqlo, $PRA_id, $PRC_id, $org_options);
		$this->step_array = &$this->protLib->step_array;
	}
	
	function get_colspan() {
		return $this->colspan;
	}
	
	function getStepArray() {
		return ($this->protLib->step_array);
	}
	
	private function get_javascript() {
		?>
		<script type="text/javascript">
		<?php 
		/**
		 * @param obj
		 * @param string type:
		 *    q
		 *    n
		 */
		?>
		function myCh(obj, typex) {
		    var value = obj.value;
		    var errorflag=0;
		    var title="";
		    
		    value = value.trim();
		    
			if (typex=='q') {
			    if (isNaN(parseFloat(value))) {
			    	errorflag=1;
			    	title="keine Zahl!";
			    }
			}
		    
		    if (errorflag) {
		    	obj.style.border = "2px solid #FF0000";
		    	obj.title = title;
		    } else {
		    	obj.style.border = '';
		    	obj.title = "";
		    }
		}
		</script>
		<?php
	}
	
	function table_init() {
		
		$bgcolor= "#999933";
		$txfont = "<font color=#FFFFFF>";
		$tx_con_color = "<font color=".$this->tmp_c_color.">";
		
		?>
	    <style type="text/css">
	    td.tprot  { background-color:#999933; font-size:80%; } 
	    </style>
		<?php 
		
		$this->get_javascript();
				
		$border="0";
		if ($this->opt_out_type=="printer") $border="1";
		
	    // WARNING: do not change the order!
	    // mind the submit buttons for concrete_protocol: edit !!!
	    
		echo "<table cellpadding=1 cellspacing=1 border=".$border." style=\"empty-cells:show\"><tr align=center>\n";
		
		$tmpcols = NULL;
		$tmpcols[] 	=   $txfont."Description</td>\n";
		$tmpcols[] =   "<img src=\"images/i13_nouse.gif\" TITLE=\"make step inactive\">"; // TBD: inactive
		$tmpcols[] =   $txfont."Data\n";
		$tmpcols[] =   $txfont."Unit\n";
		$tmpcols[] =   $txfont."Notes";
		
		$this->colid_submit = 2;
		$this->colspan = 0;
		foreach( $tmpcols as $dummy=>$val) {
			echo "<td class=tprot>".$val."</td>";
			$this->colspan++;
		}
		
		echo "</tr>\n";
		$this->step_out_cnt=0;
	}
	
	/**
	 * handle the start of a protocol-step-ROW
	 * @param $bgcolor : '', 'main', 'inactive'
	 */
	private  function line_show_tmp($step_nr, $sub_step, $bgcolor) {
		
		$bgcolor_show ="";

	
		if ($bgcolor== "main")     $bgcolor_show=" bgcolor=#E0E0E0";
		if ($bgcolor== "inactive") $bgcolor_show=" bgcolor=#FFE0E0";
		
		if ($sub_step>0) { /* is it a SUB step ? */
			if ($bgcolor!="inactive") {
				$colid = $this->sub_step_col_cnt;
				$bgcolor_show = " bgcolor=".$this->colarr[$colid];
				$this->sub_step_col_cnt++;
				if ($this->sub_step_col_cnt>2)  $this->sub_step_col_cnt=0;
			}
			echo "<tr".$bgcolor_show.">";
			echo "<td ".$this->opt_out_nowrap.">&nbsp;";
			echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
			$tmp_step = $this->sub_step_cnt;
			$this->sub_step_cnt++;
		} else {
			echo "<tr".$bgcolor_show.">";
			echo "<td ".$this->opt_out_nowrap.">&nbsp;";
			$tmp_step = $this->main_step_cnt;
			
			$this->main_step_cnt ++;
			$this->sub_step_cnt=1;	
			$this->sub_step_col_cnt=0;
			
		}
		
		echo $tmp_step.".&nbsp;"; // step number (main or sub step)
		
		
		$this->_bgcolor_show = $bgcolor_show;
	}  
	
	/**
	 * 
	 * @param unknown $PRA_ST_notes
	 * @return array $answer
	 *   'keyval' : attachment key-value
	 *   'error' : NULL or error text
	 */
	private function _get_attachment_key($PRA_ST_notes) {
		
		$attachment_keyval = '';
		$ATTKEY='Attachment-Key:';
		
		if ( strstr($PRA_ST_notes, $ATTKEY)==NULL ) return array('error'=>'need key "'.$ATTKEY.'" in PRA-step notes.');
		
		$attkeyPos0 = strpos($PRA_ST_notes, $ATTKEY);
		
		$rest = substr($PRA_ST_notes,$attkeyPos0+strlen($ATTKEY));
		$splitarr = explode("\n",$rest);       // split NEWLINE
		$splitarr = explode(" ",$splitarr[0]); // split WHITESPACE
		
		$keyval=trim($splitarr[0]);
		
		if ($keyval==NULL)  return array('error'=>'need value for "'.$ATTKEY.'" in PRA-step notes.');
		return array('keyval'=>$keyval);
	}
	
	/**
	 * get attachment file for KEY=$keyval from $this->mo_suc_id
	 * @param unknown $sqlo
	 * @param unknown $keyval
	 * @return array $attach_info
	 *   'rel_id' => rel_id of attachment
	 */
	private function _get_attachment_file($sqlo, $keyval) {
		$attach_info=array();
		
		$mo_suc_id = $this->mo_suc_id;
		if (!$mo_suc_id)   return $attach_info;
		if ($keyval==NULL) return $attach_info;
		
		$satObj = new cSattachSubs();
		$rel_id = $satObj->getRelIDbyKey($sqlo, 'CONCRETE_SUBST', $mo_suc_id, $keyval);
		
		$attach_info['rel_id'] = $rel_id;
		
		return $attach_info;
	}
	
	/**
	 * start output of a single step of ONE protocol (without row end </tr> )
	 * @param $opt "xtratxt" put text to first column
	 "step_nr_par" : alternative STRING for <input> fields (used by obj.concrete_proto.m_comp2e.php)
	 * @return  0 : o.k.
	 * 			-1: not shown
	 */
	function outstep( &$sqlo, $step_nr, $opt = NULL ) {
		$tempenc='UTF-8';
		
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
		$sqlo->query("$sqls");
		$sqlo->ReadRow();
		
		$absStName  = $sqlo->RowData[1];
		$value      = $sqlo->RowData[3];
		$main_step  = $sqlo->RowData[5];
		$sub_step	= $sqlo->RowData[6];
		$PRA_ST_notes	= trim($sqlo->RowData[7]);
		$is_sample	= $sqlo->RowData[8];
		
		$parm["mand_val"]      = $sqlo->RowData[9];
		$parm["A_SUBST_ID"]    = $sqlo->RowData[2];
		$parm["H_UNIT_ID"]	   = $sqlo->RowData[4];
		$parm["ABS_DEV_ID"]	   = $sqlo->RowData[10];
		$parm["A_SUBST_name"]  = '';
		$parm["ABS_DEV_name"]  = '';
		
		
		$unitname      = "";
		$substname_href= "";
		
		if ( $parm["A_SUBST_ID"] ) {
			$sqls= "select NAME from ABSTRACT_SUBST where ABSTRACT_SUBST_ID=".$parm["A_SUBST_ID"];
			$sqlo->query($sqls);
			$sqlo->ReadRow();
			$parm["A_SUBST_name"]=$sqlo->RowData[0];
			$substname_href="<a href=\"edit.tmpl.php?t=ABSTRACT_SUBST&id=".
					$parm["A_SUBST_ID"]."\">".$parm["A_SUBST_name"]."</a>";
		}
		
		if ( $parm["ABS_DEV_ID"] ) {
			$sqls= "select NAME from A_CHIP_READER where A_CHIP_READER_ID=".$parm["ABS_DEV_ID"];
			$sqlo->query($sqls);
			$sqlo->ReadRow();
			$parm["ABS_DEV_name"]=$sqlo->RowData[0];
			$dev_href="<a href=\"edit.tmpl.php?t=A_CHIP_READER&id=".
					$parm["ABS_DEV_ID"]."\">".$parm["ABS_DEV_name"]."</a>";
		}
		
		if ($parm["H_UNIT_ID"]) {
			$sqls= "select NAME  from H_UNIT where H_UNIT_ID=".$parm["H_UNIT_ID"];
			$sqlo->query($sqls);
			$sqlo->ReadRow();
			$unitname=$sqlo->RowData[0];
		}
		
		if ( $sub_step<=0 )  {
			$bgcolor="main";
		}
		
		$cPars = NULL;
		if ( $this->conc_proto_id ) {
			// $cPars will be used later ...
		
			$cPars = $this->protLib->conc_proto_params ( $sqlo, $step_nr, $this->conc_proto_id, $conc_color);
			$conc_notes	     = $cPars['cn'];
			$conc_color		 = $cPars['cc'];
			$conc_step_deact = $cPars['dea'];
		
		}
		
		$actioner_col = $absStName;    // action-name
		if ( $cPars['dea'] ) {
			$bgcolor="inactive";
			$actioner_col="<font color=gray>".$absStName."</font> <font color=red>(inactive)</font>";
			$this->deactSteps_cnt++;
		}
		
		$this->line_show_tmp($step_nr_par, $sub_step, $bgcolor);
		
		echo $actioner_col;
		
		echo "</td>";
		
		echo "<td>"; // field: INACTIVE 
		if ($editAllow ) {
			echo "<input type=checkbox name=not_done[$step_nr_par] value=1";
			if ( $conc_step_deact ) echo " checked";
			echo ">";
			
		} else {
			echo ' ';
		}
		echo "</td>";

		
		// COL: QUANTITY or NOTES
		$data_type=''; // default
		if ( strstr($parm["mand_val"],"q")!=NULL ) $data_type='q';
		if ( strstr($parm["mand_val"],"n")!=NULL ) $data_type='n';
		if ( strstr($parm["mand_val"],"a")!=NULL ) $data_type='a';
		
		$this_col = "";
		echo "<td".$this_col.">";
		
		switch ($data_type) {
			
			case 'a': // attachment
				
				$attachment_html = '<span style="color:gray;">no attachment</span>';  // default text
				
				$editAllow_here = $editAllow;
				$att_info = $this->_get_attachment_key($PRA_ST_notes);
				if ($att_info['error']!=NULL) {
					echo "Problem: ".$att_info['error'].";";
					$editAllow_here = 0;
				}
				
				$att_file = $this->_get_attachment_file($sqlo, $att_info['keyval']);
				if ($att_file['rel_id']>0) {
					if ($editAllow) {
						$attachment_html = '<span style="">attachment exists</span>';
					} else {
						$tmp_url = cSattachSubs::get_attach_down_url('CONCRETE_SUBST', $this->mo_suc_id, $att_file['rel_id']);
						$attachment_html = '<a href="'.$tmp_url.'">attachment</a>';
					}
					$editAllow_here = 0; // once uploaded, do not allow upload again
				}
				
				if ($editAllow_here) {
					echo "<input name=att[".$step_nr_par."]  type=file >";
					echo '<input name=fileinfo['.$step_nr_par.'][attkey] value="'.$att_info['keyval'].'" type=hidden >';
				} else {
					echo $attachment_html;
				}
				break;
				
			case 'q':
				if ($editAllow) {
					echo "<input name=quanti[".$step_nr_par."] value=\"".$cPars['qu']."\"  onchange=\"myCh(this, 'q')\" size=10>";
				} else {
					$conc_quant_show = $cPars['qu'];
					echo $conc_quant_show;
				}
				break;
			case 'n':
				$tmpNotesWrap   = "";
				$conc_notesHtml = htmlentities($conc_notes, ENT_COMPAT, $tempenc );
				
				if ($editAllow) {
					$tmpNotesWrap = " NOWRAP";
					echo "<input name=newnote[".$step_nr_par."] value=\"".$conc_notesHtml."\" onchange=\"myCh(this, 'n')\" size=".$this->fieldSZnote.">";
				} else {
					echo $conc_notesHtml;
						
				}
				break;
		}
		echo "</td>";
		echo "<td>&nbsp;".$unitname."</td>";
		

		// ABSTRACT NOTES
		echo "<td".$tmpNotesWrap.">".$PRA_ST_notes."</td>";		
		
		echo "</tr>\n";
		$this->step_out_cnt++;
		
	}
	

	
	function table_end() {
		
		echo "</table>\n";
		
	}
}


/**
 * show protocol features + the protocol step table
 * @author steffen
 *
 */
class oCONCRETE_PROTO_form2 {
	
	var $withFeatFields; // 0,1 : show protocol feature fields ?
	private $id;         // CONCRETE_PROTO_ID
	private $mo_suc_id;  // mother CONCRETE_SUBST_ID of protocol
	private $_initialized = 0;
	
	function __construct($id=NULL) {
		
		$this->withFeatFields = 0;
		if ($id) {
			$this->setProtoID($id);
		}
	}
	
	/**
	 * init protocol
	 * @param unknown $c_proto_id
	 * @param number $sucid
	 */
	function setProtoID($c_proto_id, $sucid=0) {
		$this->mo_suc_id = $sucid;
		$this->id        = $c_proto_id;
		$this->features  = NULL;
		
		$this->_initialized = 1;
	}
	
	
	function _featRow($key, $valout) {
		$keynice = columnname_nice2('CONCRETE_PROTO', $key);
		echo '   <tr><td valign=top align=right><span style="color:gray;">'.$keynice.'</span>:&nbsp;</td><td>'.$valout.'</td></tr>'."\n";
	}
	
	/**
	 * show feature fields
	 * @param object $sqlo
	 * @param $editAllow
	 */
	function _showFeatFields( &$sqlo, $editAllow) {	
	}
	
	
	/**
	 * show the editor form
	 * @param  $sql
	 * @param  $sql2
	 * @param  $editAllow 0,1
	 * @param  $option
	 *   'backurl'  : string of backurl
	 * 	 'cs.inp'   : input type: [dblink], text
	 * 	 "showReq"  :    0,1
	 *   "qm_check" :    0,1 do QM-check?
	 *   "nonqm_check" : 0,1 do Non-QM-checks?
	 *   "hiddenFields" : array(key=>val) extra array of hidden form fields
	 *   'mo.CERT_FLAG': optional contains CERT_FLAG of mother; e.g. of SUC
	 */
	function showform( &$sql, &$sql2, $editAllow, $option=NULL ) {
		
		if (!$this->_initialized) sys_error_my( 'class not initialized.' );
		
		$id = $this->id;
	    $this->features  = NULL;
	    
		$backurl="";
	    if ( $option["backurl"]!="" ) $backurl = $option["backurl"];
		
		
		
	    // for buttons you can define input.tbut
	    echo '<style type="text/css">'."\n";
	    echo "input.tbut { border:solid 1px;  background-color:#DFDFDF; border-color:#808080}\n";
	    echo '</style>'."\n";
	    
	    js__openproj();
		
	
		$optShow = unserialize($_SESSION['userGlob']["o.CONCRETE_PROTO.show"]);
		
		$sql->query('SELECT ABSTRACT_PROTO_ID, EXEC_DATE, NOTES FROM CONCRETE_PROTO WHERE CONCRETE_PROTO_ID='.$id);
	    $sql->ReadArray();
	    $this->features = $sql->RowData;
	    $a_proto_id = $sql->RowData['ABSTRACT_PROTO_ID'];
		
		$sqls= "select count(*) from CONCRETE_PROTO_STEP where CONCRETE_PROTO_ID=".$id;
	 	$sql->query("$sqls");
	 	$sql->ReadRow();
	 	$c_steps=$sql->RowData[0];
		
		$sqls= "select count(*) from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$a_proto_id;
	 	$sql->query("$sqls");
	 	$sql->ReadRow();
	 	$steps=$sql->RowData[0];
		if ( !$steps ) {
			echo "&nbsp;<font color=gray>No steps in the abstract protocol defined...</font>";
			return 0;
		}
		
		
		$stepOpt = NULL;
		$protoTableLib = new oCONCRETE_PROTO_form2Table( $sql, $sql2, $a_proto_id, $editAllow, $id,  $this->mo_suc_id, $stepOpt);
		
		if ( $editAllow ) {
			echo '<form name="xtraform" ACTION="obj.concrete_proto.paras.php" METHOD=POST ENCTYPE="multipart/form-data">'."\n"; 
		}
	
		
		$step_arrayX  = $protoTableLib->getStepArray();
		$protoTableLib->table_init();
		$colspan    = $protoTableLib->get_colspan();
		$step_count = sizeof( $step_arrayX );	
		$i=0;
		
	 	while ( $i< $step_count ) {
			$step_nr = $step_arrayX[$i];
			$protoTableLib->outstep( $sql, $step_nr );
			$i++;
		}
		
		if ( $editAllow ) {
	            echo "<tr bgcolor=#6699FF align=center height=30><td valign=middle colspan=". $colspan.
				"><input type=submit value=\"   Submit   \"></td></tr>";
		}
	        
		$protoTableLib->table_end();
		
		
		
		if ( $editAllow ) {
	           echo '<input type=hidden name=conc_proto_id value="'.$id.'">'."\n";
	           echo '<input type=hidden name=mo_suc_id value="'.$this->mo_suc_id.'">'."\n";
			if ($backurl!="") echo '<input type=hidden name=backurl value="'.urlencode($backurl).'">'."\n";
			if (is_array($option["hiddenFields"])) {
				reset ($option["hiddenFields"]);
				foreach( $option["hiddenFields"] as $key=>$val) {
					echo '<input type=hidden name="'.$key.'" value="'.$val.'">'."\n";
				}
			}
			echo "</form>";  
	        echo "<ul>";
				
	
	        echo "</ul>\n";
	    }	
		
	
		return 0;
	}
	
	/**
	 * - manage the head lines of the protocol editor
	 * - checks for access rights of the protocol
	 * @return 
	 * @param object $sqlo
	 * @param int $c_proto_id
	 * @param int $editAllowIn [0,1] : 1 can be downgraded to 0, so if no right to edit the mother,
	 * 			you have also no right to edit the protocol
	 * @param string $nicename
	 * @param array  $optRows optional info-rows array()= array($key, $val)
	 * @param array  $optarr options: 
	 * 	'headAdd' : addtional string behind TITLE
	 * 	'sh_info_head' : [1] : show it, -1: do not show it
	 * 	'sh:AP:NOTES' : 0,1 : show ABSTRACT_PROTO:NOTES
	 */
	function showIntro( &$sqlo, $editAllowIn, $nicename, $optRows=NULL, $optarr=NULL ) {
		$editAllow  = $editAllowIn;
		$tableProto = "CONCRETE_PROTO";
		
		if (!$this->_initialized) sys_error_my( 'class not initialized.' );
		
		$c_proto_id = $this->id;
		$this->withFeatFields = 1; // with feature fields !!!
		
		$show_proto_info_head = 1; // show protocol info header or Not ?
		if ($optarr['sh_info_head']<0) {
			$show_proto_info_head = 0;
			$this->withFeatFields = 0;
		}
		
		$optHeadTxt = $optarr['headAdd'];
		
		
		$objLinkLib = new fObjViewC();
		
		$sqlo->Quesel('* FROM CONCRETE_PROTO WHERE CONCRETE_PROTO_ID='.$c_proto_id);
	    $sqlo->ReadArray();
	    $cp_feat = $sqlo->RowData;
	    
	    $a_proto_id    = $cp_feat['ABSTRACT_PROTO_ID'];
	    
	    if ($a_proto_id) {
	    	// abstract_proto features
		    $sqlo->Quesel('* FROM ABSTRACT_PROTO WHERE ABSTRACT_PROTO_ID='.$a_proto_id);
		    $sqlo->ReadArray();
		    $ap_feat = $sqlo->RowData;
	    }
	    
	    $o_rightsProto = access_check($sqlo, $tableProto, $c_proto_id, $cp_feat['CCT_ACCESS_ID']);
		$acc_write = $o_rightsProto["write"];
		$editAllow = min($editAllow,$acc_write);
	    
		
		echo '<ul style="margin-top:12px;">';
		echo '<img src="images/icon.CONCRETE_PROTO.gif"> ';
		echo '<font style="color:gray; font-weight:bold; font-size:1.2em;">'.
			$nicename.' editor</font> '.$optHeadTxt;
		echo ' <a href="edit.tmpl.php?t=CONCRETE_PROTO&id='.$c_proto_id.'" title="show details"><img src="images/but.view.gif"></a>';
		echo '<br />';
		
		echo '</ul>'."\n";
		
		return ($editAllow);
}

}
