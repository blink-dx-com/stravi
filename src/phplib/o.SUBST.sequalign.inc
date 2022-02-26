<? 
/*MODULE: o.SUBST.sequalign.inc
  DESCR:  alignment tools
  USed by: ABSTRACT_ARRAY.align_target.php, www/pionir/objtools/EXP.virthyb.php
  AUTHOR: qbi
  @version $Header: trunk/src/phplib/o.SUBST.sequalign.inc 59 2018-11-21 09:04:09Z $
*/
class sequActC {

function getSequence( &$sql, 
	$substid 	/* ABSTRACT_SUBST_ID */,
	$opt = NULL /* "giveclass" */
	) 
	/* RETURN: "AGCT..." ok 
	*/
{
	global $varcol;
	
    $varcol   = & Varcols::get();
	$retval   = "";
	$class_id = "";
	
	$sqls= "select EXTRA_OBJ_ID from ABSTRACT_SUBST 
		WHERE ABSTRACT_SUBST_ID=" . $substid;
 	$sql->query("$sqls");
	$sql->ReadRow();
 	$EXTRA_OBJ_ID=$sql->RowData[0];
 	
	if ( $EXTRA_OBJ_ID <= 0 ) return array("", "");
	
	$answer   = $varcol->select_by_name($EXTRA_OBJ_ID);
	$class_id = $answer["extra_class_id"];
    $retval   = $answer["values"]["sequence"];
 
 	return ( array( $retval, $class_id ) );
}


function getArrayInfo (&$sql, &$sql2, $a_array_id, 
	$option=NULL // "breakNotFound" break after "N" scanned spots
	) {
	// get info about sequence EXTREMA
	
	$sqls=" select ABSTRACT_SUBST_ID, SPOT_ID 
			from A_PROBE_ON_ARRAY
			where ABSTRACT_ARRAY_ID=".$a_array_id. " ORDER by SPOT_ID";
	$sql->query("$sqls");
	$sequ_cnt=0;
	$probecnt=0;
	$probe_max_len = 0;
	$retarr = "";
	$sequ_no_cnt = 0;
	while ( $sql->ReadRow() ) {
	
		$subst_id = $sql->RowData[0];
		$spot_id  = $sql->RowData[1];
	
		if ($subst_id > 0){
			list ( $seq_probe, $class_id ) = $this->getSequence( $sql2, $subst_id );
			if ( $seq_probe != "" ) {
				$sequ_cnt++;
				$seq_len = strlen($seq_probe);
				if ( $seq_len && $probe_max_len<$seq_len ) {
					$probe_max_len=$seq_len;
				}
			} else {
				$sequ_no_cnt++;
				// break, after 10 first NON sequence spots
				if ( ($option["breakNotFound"]>0) AND
						!$sequ_cnt AND ($sequ_no_cnt>$option["breakNotFound"]) ) {
					break;
				}
			}
			// $class_tab["$class_id"] = $class_tab["$class_id"]+1;
		}
		$probecnt++; 
	}
	
	$retarr["probecnt"]   = $probecnt;
	$retarr["sequs"]      = $sequ_cnt;
	$retarr["sequMaxLen"] = $probe_max_len;
	
	return ($retarr);
}

function getNTDPALpath(&$sql) {
	// update globals
	global $error;
	
	$NTDPAL_EXE = $_SESSION['globals']['NTDPAL_EXE'];
	
	if ( $NTDPAL_EXE=="" ) {
		$sql->query("select VALUE from GLOBALS where NAME='NTDPAL_EXE'");
		$sql->ReadRow();
		$valtmp = $sql->RowData[0];
		$_SESSION['globals']['NTDPAL_EXE'] = $valtmp;
		$NTDPAL_EXE = $_SESSION['globals']['NTDPAL_EXE'];
	}
	 
	if ( ! is_executable($NTDPAL_EXE) ) { 
		$error->set("getNTDPALpath", 1, "executable 'ntdpal' not executable or found."); 
	}
	return ($NTDPAL_EXE);
}

function alignNow ($target_seq, $probe_sequ, &$hybstring, $opt=NULL) {
	
	
	$gap_extension_penalty = 1000000;
	$gap_opening_penalty = 2;
	$NTDPAL_EXE = $_SESSION['globals']['NTDPAL_EXE'];
	$ntdpal_cmd = $NTDPAL_EXE." -g $gap_opening_penalty -l $gap_extension_penalty -p $target_seq $probe_sequ l";
	if ($opt["print"]) $ntdpal_cmd .= " 2>&1"; // print to screen
	if ($_SESSION['userGlob']["g.debugLevel"]>6) echo "DEBUG:alignNow() $ntdpal_cmd<br>\n";
	$answer = `$ntdpal_cmd`;
 	$match_num=0;
	if ($opt["print"]) {
		$hybstring = $answer;
	}
	if ( $answer != "" ) {
		$matcharr     = explode(" ", $answer);	
		$match_num_str= explode("=",$matcharr[4]);// $matcharr[4]
		$match_num = $match_num_str[1];
	} 
	return ($match_num);
}

} // end CLASS