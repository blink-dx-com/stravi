<?php


require_once ("gui/o.PROTO.stepout1.inc");
require_once ('o.ABSTRACT_PROTO.stepx.inc');

/**
 * class for protocol steps
 * REMARKS:  BUTTON-modal-def:
            <checkbox>    <dummy>
            <dummy-subst> <button-subst>
            <field-quanti> <filed-notes>
 * @package class.protocol_cmp.inc

 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class protostep_cmp extends protostep {			
	
var $conc_proto_arr ; /* array of concrete protocols */
var $conc_proto_name_arr ; /* array of concrete protocol names */
var $conc_proto_arr_len;
var $cmp_options;    /* options for compare level
					     show_subst
						 show_quant
                         show_st_notes = [-1] |1 show step notes
					 */
var $cmp_refproto;
var $colarr;
  

function __construct( &$sql, &$sql2, $abstr_proto_id, $editAllow,
	 $checkboxes,	// use checkboxes for single step actions like copy
	 $conc_proto_id,// can be 0 
	 $options=NULL  // "abstrStepsShow"
                    // "out_type"  = ["display"] | "printer" => optimzed output
                    // "noStepScan" = [0] | 1 do not scan the protocol_steps !!!
                    //                      step_array[] is empty than
	) {

    parent::__construct( $sql, $sql2, $abstr_proto_id, $editAllow, $checkboxes, $conc_proto_id, $options); 
	    
	$this->cProtoCols = $this->protLib->getCSumRawCols();
	$this->cProtoColsSql = implode(",",$this->cProtoCols);
	
	$this->aProtoLib = new  oABSTRACT_PROTO_stepx();
     
}


function table_arr_init( &$sql, $opt=NULL ) {
	
	$txfont         ="<font color=#FFFFFF>";
	$error_color    ="<font color=#FF8080>";
	$tx_con_color   ="<font color=".$this->tmp_c_color.">";
	$bgcolor        ="#D0D0E0";
    $show_st_notes  = $this->cmp_options["show_st_notes"];
    
    $gapcol_str   = "<td bgcolor=#000030><img width=5 height=1</td>";
	$gapcol2_str  = "<td bgcolor=#FFFFFF><img width=5 height=1</td>";
	$infox = NULL;
	$infox["planColspan"] = 5;
	$info_col_len  = 2; // "substance" and "quantity"
	if ($this->cmp_options["show_quant"]<0) $info_col_len = $info_col_len - 1;
	if ($this->cmp_options["show_subst"]<0) $info_col_len = $info_col_len - 1;
    if ($show_st_notes!=-1) $info_col_len++; // and "notes" 
	
    
	echo "<table  cellpadding=1 cellspacing=1 border=0>";
	
	echo "<tr>";
	echo "<td colspan=".$infox["planColspan"]."></td>".$gapcol2_str;
	for ( $i=0; $i< $this->conc_proto_arr_len; $i++) {
		$this_proto = $this->conc_proto_arr[$i];
		$bgcolorx   = "";
		$xtext		= "";
        
        if ($this_proto==$this->cmp_refproto) {
            $xtext    = "<font color=#E0E0E0><B>Reference</B></font>";
            $bgcolorx = " bgcolor=#FF0000" ;
		}
       
		echo "<td".$bgcolorx." colspan=".$info_col_len.">".$xtext."</td>";
			
	}
	echo "</tr>\n";
	
	
	echo "<tr bgcolor=$bgcolor>";
	echo "<td colspan=".$infox["planColspan"].">".$txfont."Planned parameters</td>".$gapcol_str;
	
    for ( $i=0; $i< $this->conc_proto_arr_len; $i++) {
		$th       = $this->conc_proto_arr[$i];
		$bgcolorx = $bgcolor;
        $xtext    = "";
        $imgbg    = "dark";
        
        echo "<td valign=bottom bgcolor=".$bgcolorx." colspan=$info_col_len>".$tx_con_color;
		if ( $th ) {
            echo '<a href="edit.tmpl.php?t=CONCRETE_PROTO&id='.$th.'">';
		}
		$name_ext_tmp=$this->conc_proto_name_arr[$i];
		$name_tmp = $th;
		if ( $name_ext_tmp ) $name_tmp = $name_ext_tmp;
        if ( $opt["nameAsImg"]) {
             $name_tmp="<img src=\"f.imgtxt.php?t=".rawurlencode($name_tmp)."&bg=".$imgbg."\" border=0>";
        }
		echo $name_tmp."</a>".$xtext."</td>";
			
	}
	reset ($this->conc_proto_arr);
	echo "</tr>\n";
	
	/* check for equal protocols */
    
	$sqls= "select name from ABSTRACT_PROTO where ABSTRACT_PROTO_ID=".$this->abstr_proto_id;
	$sql->query("$sqls");
	$sql->ReadRow();
	$tmp_a_proto_name= $sql->RowData[0];
	echo "<tr bgcolor=".$bgcolor."><td colspan=5>".$txfont."protocol (abstract):<B> $tmp_a_proto_name</B></td>".$gapcol_str;
	$cnt=0;
	for ( $i=0; $i< $this->conc_proto_arr_len; $i++) {
		$th = $this->conc_proto_arr[$i ];
	
		$out_text = $tx_con_color."&nbsp;";
		if ($th) { 
            
			 $sqls= "select ABSTRACT_PROTO_ID from CONCRETE_PROTO where CONCRETE_PROTO_ID=".$th;
			 $sql->query("$sqls");
			 $sql->ReadRow();
			 $tmp_a_proto= $sql->RowData[0];
			 if ( $tmp_a_proto != $this->abstr_proto_id ) {
       				 $sqls= "select name from ABSTRACT_PROTO where ABSTRACT_PROTO_ID=".$tmp_a_proto;
				 $sql->query("$sqls");
				 $sql->ReadRow();
				 $tmp_a_proto_name= $sql->RowData[0];
				 $this->conc_proto_arr[$cnt]=0;
				 $out_text = $error_color."<b>$tmp_a_proto_name</b></font>";
			 }
		} else {
			$out_text = "<font color=#FF9090>NO PROTOCOL</font>";
		}
        echo "<td colspan=$info_col_len>".$out_text."</td>";
		$cnt++;
			
	}
	reset ($this->conc_proto_arr);
	echo "</tr>\n\n";
	
	
	echo "<tr bgcolor=#808080>";
	echo "<td>".$txfont."Do what?</td>\n";
	echo "<td>".$txfont."Do with</td>\n";
	echo "<td>".$txfont."Quantity</td>\n";
	echo "<td>".$txfont."Unit</td>\n";
	echo "<td>".$txfont."Notes</td>";
    echo $gapcol_str;
    
	for ( $i=0; $i< $this->conc_proto_arr_len; $i++) {
		$th = $this->conc_proto_arr[$i ];
		if ($th) {
			if ($this->cmp_options["show_subst"]!=-1) echo "<td>".$tx_con_color."With</td>";
			if ($this->cmp_options["show_quant"]!=-1) echo "<td>".$tx_con_color."Qnt</td>";
            if ($show_st_notes!=-1) echo "<td>".$tx_con_color."Notes</td>";
		} else {
			echo "<td colspan=$info_col_len>".$error_color."not analysed</td>";
		}	
	}
	reset ($this->conc_proto_arr);
	echo "</tr>\n\n";
		
	
	$this->step_out_cnt=0;
}

function line_finish( $step_nr, $sub_step ) {
    // Function: call this to count up
    if ($sub_step>0) { /* is it a SUB step ? */
		$this->sub_step_cnt++;
	} else {			
		$this->main_step_cnt ++;
		$this->sub_step_cnt=1;	
	}
    // echo "DEBXX: ".$this->main_step_cnt.".".$this->sub_step_cnt ."<br>";     
}

 
// ------------------------------------------------------------------
// PUBLIC:
function conc_proto_paramids ( &$sql, $step_nr, $c_proto_id  ) {
    // get parameters and Ids, can be called on any time!
	
    $retarr = array();   // means no step
			
    $ColsSql = $this->cProtoColsSql;
    $sqls= "select ".$ColsSql." from CONCRETE_PROTO_STEP ".
           " where CONCRETE_PROTO_ID=".$c_proto_id. " AND STEP_NR=" . $step_nr;
	$sql->query($sqls,'conc_proto_paramids');
    
	if ( $sql->ReadArray() ) {
		$retarr["cstp_is"]    = 1;    /* concrete proto step exists ??? */
		$retarr["c_quant"]    = $sql->RowData['QUANTITY'];
		$retarr["c_subst"]    = $sql->RowData['CONCRETE_SUBST_ID'];
		$retarr["c_note"]     = $sql->RowData['NOTES'];
		$retarr["c_NOT"]      = $sql->RowData['NOT_DONE'];
	}

	return ($retarr);
} 


function tmp_rowBegin_arr($step_nr, $sub_step) {
    
    $bgcolor_show ="";
    $nowrap = "nowrap";
                 
	if ($sub_step<=0) $bgcolor_show=" bgcolor=#E0E0E0";
	
	if ($sub_step>0) { /* is it a SUB step ? */
		
	    $colid = $this->sub_step_col_cnt;
        $bgcolorout   = $this->colarr[$colid];
	    $bgcolor_show = " bgcolor=".$bgcolorout;
	    $this->sub_step_col_cnt++;
	    if ($this->sub_step_col_cnt>2)  $this->sub_step_col_cnt=0;
		
		echo "<tr".$bgcolor_show.">";
		echo "<td ".$nowrap.">&nbsp;";
		echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
		$tmp_step = ($this->main_step_cnt-1).".".$this->sub_step_cnt;
	} else {
		echo "<tr".$bgcolor_show.">";
		echo "<td ".$nowrap.">&nbsp;";
		$tmp_step = $this->main_step_cnt;
		
		$this->sub_step_cnt=1;	
		$this->sub_step_col_cnt=0;
	}
	echo "<a href=\"javascript:a_gostep(".$this->abstr_proto_id.",".$step_nr.")\" >".$tmp_step.".</a> ";
    return ($bgcolorout);
}
         
// ------------------------------------------------------------
// PUBLIC   
// ------------------------------------------------------------

function outstep_arr(     
    $step_nr, 
    &$sql, 
    $opt = NULL    // "xtratxt" put text to first column
                   // "step_nr_par
) {       
    // FUNCTION: give row of protocol step for  $this->conc_proto_arr
    $bgcolor_diff= "#E8E8FF";
    $conc_color  = $this->tmp_c_color;
    $editAllow   = $this->editAllow;
    $proto_id    = $this->abstr_proto_id;
    $step_nr_par = $step_nr;
    if ($opt["step_nr_par"]) $step_nr_par = $opt["step_nr_par"];

	$this->aProtoLib->setObj( $proto_id );
	$extraCols = array('SUB_STEP_NR');
	$apar = $this->aProtoLib->step_info( $sql, $step_nr, 1, $extraCols);
			
    /* PROTOCOL ARRAY COMPARISION */
    $show_st_notes = $this->cmp_options["show_st_notes"];
	$info_col_len  = 2; // "substance" and "quantity"
	if ($this->cmp_options["show_quant"]<0) $info_col_len = $info_col_len - 1;
	if ($this->cmp_options["show_subst"]<0) $info_col_len = $info_col_len - 1;
	$info_col_inact_len = $info_col_len;
    if ($show_st_notes!=-1) $info_col_len++; // and "notes" 
	
    
    $rowcolor = $this->tmp_rowBegin_arr( $step_nr, $apar['SUB_STEP_NR'] );
    $this->line_finish    ( $step_nr, $apar['SUB_STEP_NR'] );
    
    $conc_color  =  $rowcolor;
    $abstrQuantOut = "&nbsp;";
	
	if ( strlen($apar['NOTES']) > 40 ){ 
        $notes = substr( $apar['NOTES'], 0, 40 ) . "&nbsp;..."; 
    } else { $notes = $apar['NOTES']; }

	
    if ( $apar['QUANTITY']!="") $abstrQuantOut = $apar['QUANTITY'];
    echo $apar['NAME'];
    echo "</td><td NOWRAP>&nbsp;".$apar['ABSTRACT_SUBST_name']."</td>";
    echo "<td>".$abstrQuantOut."</td>";
    echo "<td>&nbsp;".$apar['H_UNIT_name']."</td>";
    echo "<td NOWRAP>&nbsp;".$notes."</td>";
    echo "<td bgcolor=#000030><img width=5 height=1</td>";
	
    
    if ($this->cmp_refproto) {
         $reparams =  $this->conc_proto_params ( $sql, $step_nr, $this->cmp_refproto, $conc_color );
    }

    for ( $i=0; $i< $this->conc_proto_arr_len; $i++) { 
    
        $th = $this->conc_proto_arr[$i];
        if ($th) {
            
            $cPars =  $this->conc_proto_params ( $sql, $step_nr, $th, $conc_color);
			$conc_step_is  = $cPars['is'];
   			$conc_quantity = $cPars['qu'];
	  		$conc_subst	   = $cPars['cs'];
			$conc_notes    = $cPars['cn'];
	  		$conc_step_deact = $cPars['dea'];
	 		$ca_subst_name   = $cPars['canam'];

            if ( $conc_step_deact ) {
                $conc_colorx="#FFD0D0";
                echo "<td colspan=".$info_col_inact_len." bgcolor=$conc_colorx>(inactive)</td>";
                if ($show_st_notes!=-1) echo "<td bgcolor=$conc_colorx>&nbsp;</td>"; 
            } else {

                if ($substname!="") $conc_subst_show = $substname;
                else $conc_subst_show = "&nbsp;";
                $this_col="";

                // substance
                if ( $reparams['cs'] != $conc_subst) $this_col = "bgcolor=".$bgcolor_diff;
                if ( $conc_subst != "" ) $conc_subst_show = "<B>".$ca_subst_name."</B>";

                if ($this->cmp_options["show_subst"]!=-1) echo "<td " .$this_col. ">".$conc_subst_show."</td>"; 
                
                $conc_color1 = "";
                if ( $reparams['qu'] != $conc_quantity) $conc_color1="bgcolor=".$bgcolor_diff;  
                // else $conc_color1="bgcolor=".$conc_color;
                	
                if ( $conc_quantity == "" ) {
                    $conc_quant_show = $abstrQuantOut;
                } else {
                    $conc_quant_show = "<B>".$conc_quantity."</B>";
                }
                if ($this->cmp_options["show_quant"]!=-1) echo "<td ".$conc_color1.">" .$conc_quant_show."</td>";
                
                if ($show_st_notes!=-1) {
                    if ( $reparams['cn'] != $conc_notes) $conc_color1=$bgcolor_diff; 
                    else $conc_color1 =  $conc_color;
                    echo "<td bgcolor=".$conc_color1.">&nbsp;<B>".htmlspecialchars($conc_notes)."</B></td>";
                }
            }
        } else {
            echo "<td colspan=$info_col_len>&nbsp;</td>";
        }

    }
    reset($this->conc_proto_arr);
    echo "</tr>\n";
        

     $this->step_out_cnt++;
}

                                               

// =============================================== 


function cproto_arr_ini( 
    $conc_proto_arr, 
    $conc_proto_name_arr, 
    $ref_cproto_id,         // reference protocol
    $options                // option array
		/*
		"show_quant" : show quantity 
		"show_subst" : show substance
		"show_st_notes  : show notes
		*/ 
    ) {
    
	$this->conc_proto_arr     = $conc_proto_arr;
	$this->conc_proto_name_arr= $conc_proto_name_arr;
	$this->conc_proto_arr_len = sizeof($this->conc_proto_arr);
    $this->cmp_refproto       = $ref_cproto_id;
    $this->cmp_options        = $options; 
    
	return (0);
}

}
