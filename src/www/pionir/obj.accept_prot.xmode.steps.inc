<?php
/**
 * - show acceptance-step editor 
 * - calls obj.accept_prot.paras.php to save the parameters
 * - included by edit.tmpl.php
 * @package obj.accept_prot.xmode.steps.inc
 * @author     Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq SREQ:0001754: o.ACCEPT_PROT > manage edit/show of acceptance steps 
 */
require_once ( "o.PROTO.steps.inc" );
require_once ("f.objview.inc");	

class oACCEPT_PROT_edGui {
	
	function __construct(&$sqlo, $id, $editAllow) {
		$this->id=$id;
		$this->table='ACCEPT_PROT';
		$this->editAllow = $editAllow;
		
		$this->colarr[0]="#F0F0B0";
		$this->colarr[1]="#F8F8C0";
		$this->colarr[2]="#FFFFD0";
		$this->sub_step_col_cnt=0; // color ID counter for $this->colarr
		$this->sub_step_cnt  = 0;
		$this->main_step_cnt = 1;
		
		$sqlsel = 'ABSTRACT_PROTO_ID from '.$this->table.' where ACCEPT_PROT_ID='.$id;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$this->ABS_PROTO_ID = $sqlo->RowData[0];
		
		// get number of abs_proto_steps
		$protoOrgLib = new gProtoOrg();
		$protoOrgLib->setProto($sqlo, $this->ABS_PROTO_ID, 0);
		
		$this->stepArr = $protoOrgLib->getStepArray();
		
	}
	
	function getNumAbsSteps() {
		return sizeof($this->stepArr);
	}
	
	function _table_init() {
		
		?>
	    <style type="text/css">
	    td.thAbsProt  { background-color:#999933; font-size:80%; color: #FFFFFF;} 
		td.thAccProt  { background-color:#333399; font-size:80%; color: #FFFFFF;} 
	    </style>
		<? 
		
		echo "<table cellpadding=1 cellspacing=1 border=0 style=\"empty-cells:show\">
			<tr align=center>\n";
	
		$tmpcols = NULL;
		$tmpcols['a:NAME']   =  "Description";
		$tmpcols['a:SUBST']   =  "Planned<br>substances";	
		$tmpcols['a:QUANTITY'] =  "Quantity<br>planned";
		$tmpcols['a:UNIT']   =  "Unit";
		$tmpcols['a:NOTES']  =  "Notes&nbsp;planned";
		
		$cColumnId = 5;
		$tmpcols['c:MIN']    =  "Min";
		$tmpcols['c:MAX']    =  "Max";
		$tmpcols['c:NOTES']  =  "NOTES";
		
		$this->colspan=0;
		$cnt=0;
		foreach( $tmpcols as $dummy=>$val) {
			if ($cnt<$cColumnId) $className='thAbsProt';
			else $className='thAccProt';
			echo "<td class=".$className.">".$val."</td>";
			$this->colspan++;
			$cnt++;
		}
		echo "</tr>\n";
		$this->step_out_cnt=0;
		
	}
	
	// handle the start of a protocol-step-ROW
	function _line_start( $step_nr, $sub_step, $bgcolor ) {
	
		$bgcolor_show=" bgcolor=#E0E0E0";
		
		if ( $this->linexHead>0 ) {
			$bgcolor_show=" bgcolor=#E0E0E0";
			echo "<tr".$bgcolor_show.">";
			echo "<td>&nbsp;";
		} else {
			
			if ($bgcolor== "main")     $bgcolor_show=" bgcolor=#E0E0E0";

			if ($sub_step>0) { /* is it a SUB step ? */
				
				$colid = $this->sub_step_col_cnt;
				$bgcolor_show = " bgcolor=".$this->colarr[$colid];
				$this->sub_step_col_cnt++;
				if ($this->sub_step_col_cnt>2)  $this->sub_step_col_cnt=0;
				
				echo "<tr".$bgcolor_show.">";
				echo "<td>&nbsp;";
				echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
				$tmp_step = $this->sub_step_cnt;
				$this->sub_step_cnt++;
			} else {
				echo "<tr".$bgcolor_show.">";
				echo "<td>&nbsp;";
				$tmp_step = $this->main_step_cnt;
				
				$this->main_step_cnt ++;
				$this->sub_step_cnt=1;	
				$this->sub_step_col_cnt=0;
				
			}
			 
			echo $tmp_step.".&nbsp;";
			
		}
		$this->_bgcolor_show = $bgcolor_show;
	}  

	
	/**
	 * print one step
	 * @return 
	 * @param object $sqlo
	 * @param object $step_nr
	 */
	function _outstep( &$sqlo, $step_nr ) {
		$sqlsel= " *  from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=" .
			$this->ABS_PROTO_ID." AND STEP_NR=".$step_nr ;
	    $sqlo->Quesel($sqlsel);
		$sqlo->ReadArray();
		$absFeatures = $sqlo->RowData;
		
		$sqlsel= " *  from ACCEPT_PROT_STEP where ACCEPT_PROT_ID=" .
			$this->id." AND STEP_NR=".$step_nr ;
	    $sqlo->Quesel($sqlsel);
		$sqlo->ReadArray();
		$acceptFeat = $sqlo->RowData;
		
		
		// prevent output, where the leading ZERO is missing e.g.: ".4" => transform to "0.4"
		if (is_numeric($acceptFeat['MIN_VAL'])) $acceptFeat['MIN_VAL'] = floatval($acceptFeat['MIN_VAL']);
		if (is_numeric($acceptFeat['MAX_VAL'])) $acceptFeat['MAX_VAL'] = floatval($acceptFeat['MAX_VAL']);
		
		if ( $absFeatures["ABSTRACT_SUBST_ID"] ) {
			$sqlsel= "NAME from ABSTRACT_SUBST where ABSTRACT_SUBST_ID=".
				$absFeatures["ABSTRACT_SUBST_ID"];
			$sqlo->Quesel($sqlsel);
			$sqlo->ReadRow();
			$absFeatures["ABSTRACT_SUBST_name"]=$sqlo->RowData[0];
		}
		if ( $absFeatures["H_UNIT_ID"] ) {
			$sqlsel ="NAME from H_UNIT where H_UNIT_ID=".
				$absFeatures["H_UNIT_ID"];
			$sqlo->Quesel($sqlsel);
			$sqlo->ReadRow();
			$absFeatures["H_UNIT_name"]=$sqlo->RowData[0];
		}
		if (strlen($absFeatures["NOTES"]>100)) {
			$absFeatures["NOTES_short"]=substr($absFeatures["NOTES"],0,100).' ...';
		} else $absFeatures["NOTES_short"]= $absFeatures["NOTES"];
		
		$bgcolor='';
		$this->_line_start( $step_nr, $absFeatures['SUB_STEP_NR'], $bgcolor );
		
		// start column output
		
		echo htmlspecialchars($absFeatures["NAME"]).'</td>';
		echo '<td>'.htmlspecialchars($absFeatures["ABSTRACT_SUBST_name"]).'</td>';
		echo '<td>'.$absFeatures["QUANTITY"].'</td>';
		echo '<td>'.$absFeatures["H_UNIT_name"].'</td>';
		echo '<td>'.htmlspecialchars($absFeatures["NOTES_short"]).'</td>';
		
		if ($this->editAllow) {
			echo '<td>';
			echo "<input name=qmin[".$step_nr."] value=\"".$acceptFeat['MIN_VAL']."\" size=8>";
			echo '</td>';
			echo '<td>';
			echo "<input name=qmax[".$step_nr."] value=\"".$acceptFeat['MAX_VAL']."\" size=8>";
			echo '</td>';
			echo '<td>';
			echo "<input name=qnotes[".$step_nr."] value=\"".$acceptFeat['NOTES']."\" size=30>";
			echo '</td>';
		} else {
			echo '<td>';
			echo $acceptFeat['MIN_VAL'];
			echo '</td>';
			echo '<td>';
			echo $acceptFeat['MAX_VAL'];
			echo '</td>';
			echo '<td>';
			echo htmlspecialchars($acceptFeat['NOTES']);
			echo '</td>';
		}
		
		echo '</tr>'."\n";
		
	}
	
	function _table_end() {
		echo "</table>\n";
	}
	
	function showForm(&$sqlo) {
		
		$editAllow = $this->editAllow;
		if ( $editAllow ) {
			echo '<form name="xtraform" ACTION="obj.accept_prot.paras.php" METHOD=POST>'."\n";
			echo '<input type=hidden name="id" value="'.$this->id.'">'."\n"; 
		}
		
		
		$step_arrayX  = &$this->stepArr;
		$this->_table_init();
		$step_count = sizeof( $step_arrayX ) ;	
		$i=0;
 		while ( $i < $step_count ) {
			$step_nr = $step_arrayX[$i];
			$this->_outstep( $sqlo, $step_nr );
			$i++;
	  	}
		
		if ( $editAllow ) {
            echo '<tr bgcolor=#EFEFEF align=center height=30><td valign=middle colspan='.$this->colspan.
				'><input type=submit value="Submit" class="yButton"></td></tr>';
		}
        
		$this->_table_end();
		
	}
	
}


//   which describes the possible MODEs for table TABLENAME 
class oACCEPT_PROT_xmode_steps {
	
function __construct( &$editLib ) {
}

// PLEASE use this function; still optional 
function xmode_start( &$sql, &$sql2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {
	global $error, $varcol;
	
	echo '<ul>'."\n";
	
	// get edit-link of BO + NAME + icon (object)
	$apid = $arguByKey['ABSTRACT_PROTO_ID'];
	$objLinkLib = new fObjViewC();
	$htmltmp = $objLinkLib->bo_display( $sql, 'ABSTRACT_PROTO', $apid );
	echo tablename_nice2('ABSTRACT_PROTO') .':'.$htmltmp;
	
	echo '</ul>'."\n";
	$editGui = new oACCEPT_PROT_edGui($sql, $id, $editAllow);
	
	if ( !$editGui->getNumAbsSteps() ) {
		echo "&nbsp;<font color=gray>No steps in the abstract protocol defined...</font><br>";
		return;
	}
	
	$editGui->showForm($sql);
}

}
