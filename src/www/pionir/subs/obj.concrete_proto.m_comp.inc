<?php
require_once (dirname(__FILE__)."/class.protocol_cmp.inc");

/*MODULE:  obj.concrete_proto.m_comp.inc
  DESCR: - sub function for compare of concrete_proto_steps
  		 - TBD: problems, when one C_PROT has a different A_PROTO => it confuses the protocol-compare (all steps are different)
  AUTHOR: qbi
  VERSION: 0.1 - 20050310
*/
class oCONCR_PROTOcomp {

var $action;
var	$proj_id;
var	$a_proto_id;     // default abstract_protocol_ID
var	$ref_cproto_id;
var $debugflag;
var $proto_step_c;
var $cproto_abs;     // array of abstract_protocols

function __construct($action, $proj_id) {
	$this->action  = $action;
	$this->proj_id = $proj_id;
	$this->a_proto_id = NULL;   // default abstract_protocol_ID
	$this->ref_cproto_id = NULL;
	
	$this->debugflag = $_SESSION["userGlob"]["g.debugLevel"];
	$this->proto_step_c = NULL;
}


function setAprotoID($a_proto_id) {
	$this->a_proto_id = $a_proto_id;
}

function analyseConcProtoRef(&$sql, $ref_cproto_id ) {
	// FUNCTION:- analyse ref proto
	//			- set $this->ref_cproto_id
	// OUTPUT: set $this->a_proto_id
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$this->ref_cproto_id = $ref_cproto_id;
	$this->a_proto_id    = 0;
	
	if (!$ref_cproto_id) {
		$error->set($FUNCNAME, 1, "The first object must contain a protocol (concrete)!");
		return;
	}  
	
	$sqls= "select ABSTRACT_PROTO_ID from CONCRETE_PROTO where CONCRETE_PROTO_ID=".$ref_cproto_id;
	$sql->query("$sqls");
	if ($sql->ReadRow()) {
		$a_proto_id = $sql->RowData[0];
		$this->a_proto_id = $a_proto_id;	 
	} else {
		$error->set($FUNCNAME, 2, "Reference protocol ".$ref_cproto_id." not found!");
		return;
	}
	
	$sqls= "select count(*) from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$a_proto_id;
	$sql->query("$sqls");
	$sql->ReadRow();
	$steps = $sql->RowData[0];
	if ( !$steps ) {
		$error->set($FUNCNAME, 2, "No abstract protocol steps defined on protocol (abstract) [ID:$a_proto_id].");
		return;
	}
	
	$this->proto_step_c = new  protostep_cmp( $sql, $sql2, $this->a_proto_id, 0, 0, 0, "" );
	
	return ($steps);
} 

function &getStepArray() {
	return ( $this->proto_step_c->getStepArray() );
}

function analyseAbstractProtos( &$sql, &$conc_proto_arr ) {
	// FUNCTION: analyse abstract protros
	// INPUT:  $this->a_proto_id
	// OUTPUT: $this->cproto_abs[cnt] = a_proto_id
	// CALL_SEQ: call after analyseConcProtoRef()
	
	$ref_proto_id = $this->ref_cproto_id;
    $conc_proto_arr_len = sizeof($conc_proto_arr);
	$this->cproto_abs = NULL;
	
	for ( $i=0; $i< $conc_proto_arr_len; $i++) {
	
    	$tmp_cpid  = $conc_proto_arr[$i];
		if ($tmp_cpid) {
			$sqls= "select ABSTRACT_PROTO_ID from CONCRETE_PROTO where CONCRETE_PROTO_ID=".$tmp_cpid;
			$sql->query("$sqls");
			$sql->ReadRow();
			$this_a_proto = $sql->RowData[0];
			$this->cproto_abs[$i] = $this_a_proto;
		}
	}
	
}

function exitError() {
	global $error;
	$error->printAll();
	htmlFoot();
}

function htmlFootF1($text1, $text2) {
	htmlFoot($text1, $text2);
}


function infoout($key, $val) {
    echo "<font color=gray>".$key.":</font> ".$val."<br>\n";
}   


function get_proto_from_nr(&$sql, $table, $objid, $stepno) {
	// FUNCTION: get A_PROTO_ID from PROTO_NO
	$sqls= "select c.ABSTRACT_PROTO_ID from W_WAFER_STEP e, CONCRETE_PROTO c ".
			" where e.W_WAFER_ID=".$objid." AND e.STEP_NR=".$stepno. " AND e.CONCRETE_PROTO_id=c.CONCRETE_PROTO_id";
	$sql->query("$sqls");
	$sql->ReadRow();
	$a_proto_id = $sql->RowData[0];
	return ($a_proto_id);
}                         



function CompareProtStep( 
	&$sql,  
	&$conc_proto_arr, 
	$step_nr, 
    $dprotarr = NULL   // array of flags: shows, which protocol is different
     ) {  
    // FUNCTION: - for ONE protocols step, for ALL protocols
	//			 - calculate, if there are differences between the protocols steps for STEP_NR: $step_nr
	//			 - ignores protocols with other protocol (abstract) than the reference protocol
    // RETURN  : 0 - no difference
    //           1 - yes: differences             
    $isdifferent  = 0; 
	$ref_proto_id = $this->ref_cproto_id;
	$a_proto_id   = $this->a_proto_id;
    $conc_proto_arr_len = sizeof($conc_proto_arr);
	$debugflag = $this->debugflag;
    
    $sqls= "select SUB_STEP_NR from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=" .$a_proto_id. " AND STEP_NR=".$step_nr;
    $sql->query($sqls);
    $sql->ReadRow();
    $sub_step  = $sql->RowData[0]; 
    $refparams = $this->proto_step_c->conc_proto_paramids ( $sql, $step_nr, $ref_proto_id );
			
    for ( $i=0; $i< $conc_proto_arr_len; $i++) {
	
        $thisIsDiff= 0;  // value of difference
						 // 1 : array content
						 // 2 : different size of array
		$otherProtocol = 0;
        $tmp_cpid  = $conc_proto_arr[$i];
		$tmp_apid  = $this->cproto_abs[$i];
        $thisparam = NULL;
        
        if ( $tmp_cpid AND ($tmp_apid==$a_proto_id) ) { // c_proto exists and is same TYPE like reference!
            $thisparam = $this->proto_step_c->conc_proto_paramids ( $sql, $step_nr, $tmp_cpid );	
            if (array_diff_assoc($refparams, $thisparam) != NULL ) { 
                $thisIsDiff = 1;
            } 
            if ( sizeof($refparams)!=sizeof($thisparam) ) {
                $thisIsDiff = 2;
            } 
			// TBD: protocol Y is EMPTY, but has the same default settings like X
        } else {
            // TBD: no protocol ???
			
        } 
        
        if ( $thisIsDiff ) {
            // echo "DEBUG_xcd:: step_nr:$step_nr proto_id:$i a_protoid:$tmp_apid diffReason:$thisIsDiff<br>\n";
            $isdifferent = 1;
            $dprotarr[$i]++;
        }
    }
    return array($isdifferent, $dprotarr, $sub_step);
}  

function getDiffProtos(
        &$sql,  
        &$conc_proto_name_arr, 
        &$sel 
        ) {                
        // FUNCTION: - get only the different protocols   
        //           - if differences: put $ref_cproto_id to the return arrays
        // RETURN:   reduced $sel and $conc_proto_name_arr    
        
		$retarr        = NULL;
		$a_proto_id    = $this->a_proto_id;
		$ref_cproto_id = $this->ref_cproto_id;
		
		$step_arrayX  = $this->proto_step_c->getStepArray();
		$step_count   = sizeof( $step_arrayX ) ;	
          
        $dprotarr  = NULL;
        $diffCount = 0;
		$i         = 0;
		
		// do for all steps ...
 		while ( $i < $step_count ) {
			
			$step_nr = $step_arrayX[$i];
            list ($isdiff, $dprotarr, $sub_step) = $this->CompareProtStep( $sql, $sel, $step_nr, $dprotarr);
            if ($isdiff) { 
                $diffCount++;
            }
            $i++;
	  	}
         
        $conc_proto_arr_len = sizeof($sel); 
        
        if ($ref_cproto_id) { 
            for ( $i=0; $i< $conc_proto_arr_len; $i++) {  
                if ( !$sel[$i] OR ($this->cproto_abs[$i] != $a_proto_id) ) {
                     $dprotarr[$i] = 1;  // different, because no protocol or not same type !!!
                     $diffCount++;
                }
            }
        }  
        
        if ($diffCount) {
           
            for ( $i=0; $i< $conc_proto_arr_len; $i++) {  
                if ($dprotarr[$i] OR $sel[$i]==$ref_cproto_id) {
                    //echo "$i:".$dprotarr[$i]." &nbsp;";
                    $retarr[]  = $sel[$i];
                    $retarrN[] = $conc_proto_name_arr[$i];
                }
            }
        } 
        return array($retarr, $retarrN, $step_count);
}

function ftab_subhead(
	$colarr, 
	$cmd=NULL
	) {
	$colspanMax = 4;
	
	if ($cmd=="head") {
		echo "<tr bgcolor=#D0D0FF><td  colspan=".$colspanMax."><B><font color=gray>".$colarr[0]."</B></td>";
		return;
	}
	if ($cmd=="hr") {
		echo "<tr><td  colspan=".$colspanMax."><hr></td>";
		return;
	}

	echo "<tr bgcolor=#E0E0FF>\n";
	foreach( $colarr as $key=>$val) {
		echo "<td>".$val."</td>";
	}
	reset ($colarr); 
	echo "</tr>\n";
}

function _ftabGetRadios(
		$varname, 
		$valuex
		) {
		
		$valueSet[0] = "-1";
		$valueSet[1] = "1";
		 
		
		// echo "DEBUG: $varname : readyes: $radioYes <br>";
		$selected_1 = $valuex>=0 ? "" : "checked" ;
		$selected_2 = $valuex>=0 ? "checked" : "" ;
		
		$valx[0] = "<input type=radio name=\"setti[".$varname."]\" value=\"".$valueSet[0]."\" ".$selected_1.">";
		$valx[1] = "<input type=radio name=\"setti[".$varname."]\" value=\"".$valueSet[1]."\" ".$selected_2.">";
		
		return ($valx);
}

function ftab(
	$colarr,
	$colspan=0 // "", 2
	) {
	$valx    = NULL;
	
	$key = $colarr[0];
	
	echo "<tr>";
	echo "<td><b>$key</b></td>";

	if ($colspan>0) {
		echo "<td colspan=2>".$colarr[1]."</td>";
	} else {
		echo "<td>".$colarr[1]."</td>";
		echo "<td>".$colarr[2]."</td>";
		
	}
	echo "<td NOWRAP>".$colarr[3]."</td>";
	
	
	echo "</tr>\n";
}

function prefs_formshow($tablename, $go, $step_no, $tmpCompOpt) {
	// FUNCTION: show preferences
	
	$tablenice  = tablename_nice2($tablename);
	
	// glob_printr( $tmpCompOpt, "tmpCompOpt info" ); 
	
    echo "<form method=\"post\"  name=\"editform\"  action=\"".$_SERVER['PHP_SELF']."?tablename=".$tablename."\" >\n";
	echo "<table cellpadding=0 cellspacing=1 border=0 bgcolor=#D0D0FF><tr><td>";
    echo "<table cellpadding=2 cellspacing=0 border=0 bgcolor=#EFEFEF>";
    echo "<input type=hidden name=\"go\" value=\"".$go."\"> \n"; 
    echo "<input type=hidden name=\"action\" value=\"all_steps\"> \n"; 
    echo "<input type=hidden name=\"step_no\" value=\"".$step_no."\"> \n";
    echo "<input type=hidden name=\"prefgo\" value=\"1\"> \n";
	if ($this->proj_id) echo "<input type=hidden name=\"proj_id\" value=\"".$this->proj_id."\"> \n";
    
	$colarr=array("Compare options") ;
    $this->ftab_subhead($colarr, "head");
	
    $colarr=array("","hide", "show","");
    $this->ftab_subhead($colarr);
	
	$vartmp  = "show_quant";
	$infotmp = intval($tmpCompOpt[$vartmp]);
	$radio = $this->_ftabGetRadios( $vartmp, $infotmp, 0 );
	$this->ftab( array("Show quantity", $radio[0], $radio[1] ) );
	
	$vartmp = "show_subst";
	$infotmp= intval( $tmpCompOpt[$vartmp] );
	$radio = $this->_ftabGetRadios( $vartmp, $infotmp, 0 );
	// $this->ftab("Show substance", $vartmp, 1, "", $tabopt  );
	$this->ftab( array("Show substance", $radio[0], $radio[1] ) );
	
	$vartmp = "show_st_notes";
	$infotmp= $tmpCompOpt["show_st_notes"];
	$radio = $this->_ftabGetRadios( $vartmp, $infotmp, 0 );
	$this->ftab( array("Show notes", $radio[0], $radio[1] ) );
	
	$colarr=array();
    $this->ftab_subhead($colarr, "hr");
	
	$vartmp = "show_navert";
	$infotmp= $tmpCompOpt["show_navert"];
	$checked_tmp="";
	if ($infotmp) $checked_tmp="checked";
	$colarr=array("Show vertical", 
				  "<input type=checkbox name=\"setti[".$vartmp."]\" value=\"1\" ".$checked_tmp.">", 
				  "",
				  "Show ".$tablenice." names vertical" );
    $this->ftab($colarr);
	
	
    $vartmp = "show_dprot";
	$infotmp= $tmpCompOpt["show_dprot"];
	$checked_tmp="";
	if ($infotmp) $checked_tmp="checked";
	$colarr= array(
		"different protocols", 
		"<input type=checkbox name=\"setti[".$vartmp."]\" value=\"1\" ".$checked_tmp.">", 
		"",
		"Show only different protocols");
    $this->ftab($colarr);
    
    $vartmp = "show_diff";
	$infotmp= $tmpCompOpt["show_diff"];
	$checked_tmp="";
	if ($infotmp) $checked_tmp="checked";
	$colarr= array(
		"different steps",
		"<input type=checkbox name=\"setti[".$vartmp."]\" value=\"1\" ".$checked_tmp.">", 
		"", 
		"Show only different steps");
	$this->ftab($colarr);
	
    $colarr= array(
		"Ref_protocol_ID", 
		"<input type=text name=\"ref_proto_in\" value=\"\" size=6>",
		"",
		"");
	$this->ftab($colarr, 2);
	
	$colarr= array("", "<input type=submit value=\"Submit\">");
	$this->ftab($colarr, 2);
	
    echo "</table>\n";  
	echo "</td></tr></table>\n";        
    echo "</form>\n";

}
 


function wafer_proto_is_shared( &$sql, $cproto_id ) {
	// RETURN: number of shares: 0 not shared; >0 is shared
	$sqls = "select count(1) from W_WAFER_STEP where CONCRETE_PROTO_id=".$cproto_id;
	$sql->query("$sqls");
	$sql->ReadRow();
	$cnt = $sql->RowData[0] - 1;
	return ($cnt);
}

}