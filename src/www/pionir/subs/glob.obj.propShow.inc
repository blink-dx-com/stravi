<?php

require_once ('f.help.inc');
require_once ('o.DB_USER.subs.inc');
require_once 'edit.sub.assoc.inc';
require_once ("f.objview.inc");
require_once('glob.obj.conabs.inc');


/**
 * sub function for object access > properties show
 * @package glob.obj.propShow.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0000980: g > glob.obj.feat.php : show META features of object
 */
class gObjPropShowC {

// $tablename must be a BO !!!
function initObj( &$sqlo, $tablename, $objid) {
	$this->tablename = $tablename;
	$this->objid = $objid;
	$this->acc_data = array();
	$prim_name = PrimNameGet2($this->tablename);
	$this->cct_access_id = glob_elementDataGet( $sqlo, $tablename, $prim_name, $objid, 'CCT_ACCESS_ID' );
	
	if (!$this->cct_access_id) {
	    htmlInfoBox( "Warning", "Object has no CCT_ACCESS_ID", "", "WARN" );
	    echo "<br>";
	} else {
	    $this->acc_data  = access_data_getai( $sqlo, $this->cct_access_id );
	}
}

function _scanCCTaccessUsed( &$sql, $cct_access_id) {
	// FUNCTION: scan, if CCT_ACCESS_ID is used by other objects
	
	$tablename = $this->tablename;
	$sql->query("SELECT count(1) FROM ".$tablename." WHERE cct_access_id=".$cct_access_id);
	$sql->ReadRow();
	$answer = $sql->RowData[0]; 
	return ($answer);
}



function _tableOut( &$info_tab2 ) {

	// print 5 columns
	echo '<table cellspacing=0 cellpadding=1 border=0>';
	$cnt=0;
	foreach( $info_tab2 as $rowid=>$content) { // $rowid= 0,1,2,3,...
	    
		$tmpEmptCol = "  <td>&nbsp;</td>";
		$komma      = "";
		echo "<tr valign=top>";
		
		for ($ii=0; $ii<2; $ii++) {
		    
			$infoq=NULL;
			#$ikey2=NULL;
			#$ival2=NULL;
			$nowcolspan="";
			if ( !empty($content[$ii]) ) {
				$contentarr  = $content[$ii];
				$ikey = $contentarr[0];
				$ival = $contentarr[1];
				$xopt = $contentarr[2];
				
				if ($xopt["LEVEL"]=="WARNING") {
					$infoq="<font color=#808000><b>WARNING:</b></font>";
				}
				if ($xopt["colspan"]>1) {
					$nowcolspan=" colspan=4";
				}
				$ikeyOut = $ikey.':';
				if ($ikey==NULL) $ikeyOut = '&nbsp;';
				echo $komma."<td align=right NOWRAP><font color='#808080'>$ikeyOut</font>&nbsp;</td>";
				echo "<td".$nowcolspan.">".$infoq." ".$ival."</td>";
				if ($xopt["colspan"]>1) {
					break; // only one column ...
				}
			} else
				echo $tmpEmptCol;
			$komma = "<td>&nbsp</td>";
		}
		echo "</tr>\n";
		$cnt++;
	}
	
	echo "</table>\n";
}

/**
 * DEPRECATED
 * @param object $sqlo
 * @return string[]
 */
/*
 * require_once ('f.archdb.log.inc');
private function _archiveAna(&$sqlo) {
	$tablename = $this->tablename;
	$prim_id   = $this->objid;
	
	
	$helpLib = new fHelpC();
	$hopt = array("object" => "icon");
	$helpText = fHelpC::link_show("g.PDA.html", "help", $hopt);
	
	$answer    = array( 'Data archived', '- &nbsp;'.$helpText.'');
	
	$archLobLib = new fArchDB_log();
	$archLobLib->setObj($sqlo, $tablename, $prim_id);
	$archinfo = $archLobLib->objArchiveInfo($sqlo);
	if ( sizeof($archinfo) ) {
		
		$tableInfo = $archLobLib->archInfo2TableLong($archinfo);
		$infotext =NULL;
		foreach($tableInfo as $onerow) {
			$infotext .= $komma . tablename_nice2($onerow['t']).' '.$onerow['d'];
			$komma = "<br>";
		}
		
		$href = 'p.php?mod=DEF/g.obj.arch&t='.$tablename.'&id='.$prim_id;
		$answer = array( '<span style="color:black; background-color: #EEEE00;
				padding-left:4px;">Data archived</span>', '[<a href="'.$href.'">Archive Detail/Retrieval</a>] '.
				' '.$helpText.'<br>' .
				$infotext);
	}
	return	$answer;
}
*/
	
function _objAccTabGet(&$sql) {
    
    $accdata  = &$this->acc_data;
	$cct_access_id = $this->cct_access_id;
	if (!$cct_access_id) {
		
	} else {	
		$sql->query("SELECT * FROM cct_access a WHERE a.cct_access_id=". $cct_access_id);
		if ($sql->ReadArray()) {
			$cctAccFeat = $sql->RowData;
		} else {
			htmlErrorBox("Error", "CCT_ACCESS_ID not found in CCT_ACCESS");
			return array(NULL, NULL, 0);
		}
		
	}
	
	$info_tab2 = NULL;
	/*  array[ROW] = array[COL_ID] => array( "key", "text", $options )
						$options = "LEVEL"=>  ..
									"colspan" => [0], 2
	*/
	
	if ( $_SESSION['sec']['appuser'] == "root" ) {
		$info_tab2[][0] = array("access ID" ,"<a href='edit.tmpl.php?t=CCT_ACCESS&id=$cct_access_id'>$cct_access_id</a>".
		" <font color=gray>(change owner, dates)</font>");
	}
	
	$ownerFull = DB_userC::getFullname($sql, $accdata["owner_id"]);
	
	$info_tab2[][0] = array( "owner", "<a href=\"edit.tmpl.php?t=DB_USER&id=".$accdata["owner_id"]."\">".
		$accdata["owner"]."</a> - ".$ownerFull);
	if ( $accdata["modifier_id"] ) {
		$modFull = DB_userC::getFullname($sql, $accdata["modifier_id"]);
		$info_tab2[][0] = array("last modifier","<a href='edit.tmpl.php?t=DB_USER&id=".
			$accdata["modifier_id"]."'>".$accdata["modifier"]."</a> - ".$modFull);
	}
	
	$modRow=0;
	$info_tab2[][0] = array( "creation",$accdata["crea_date"]);
	if ($accdata["mod_date"]!="") {
		$info_tab2[][0] = array( "last modification", $accdata["mod_date"] );
		$modRow=1;
	} else {
		$info_tab2[][0] = array( "last modification", '-' );
	}
	
	$info_tab_cnt = 4;
	$info_tab_cnt++;
	
	$keyTmp = 'last action';
	if ($modRow AND $cctAccFeat["UPINFO"]!=NULL) {
		$info_tab2[][0] = array( $keyTmp, htmlspecialchars($cctAccFeat["UPINFO"]) );
		$info_tab_cnt++;
	} else {
		$info_tab2[][0] = array( $keyTmp, '-' );
		$info_tab_cnt++;
	}
	
	$info_tab_cnt++;

	return array($info_tab2, $cctAccFeat, $info_tab_cnt);
}

// add associated elements
function _assoc_elemnts($sqlo, &$info_tab2, $info_tab_cnt) {
    
    $tablename = $this->tablename;
    $objid = $this->objid;
    
    $ObjFormAssocLib = new fObjFormAssoc();
    $ObjFormAssocLib->setObj($tablename, $objid);
    $rowopt=array('sh.link'=>1);
    $assoc_cache = $ObjFormAssocLib->get_assoc_info($sqlo, 0, $rowopt);

    if ( !empty($assoc_cache) ) {

        $info_tab2[$info_tab_cnt][0] = array('', '');
        $info_tab_cnt++;
        $info_tab2[$info_tab_cnt][0] = array('Associated Elements', '');
        $info_tab_cnt++;
        // array($icon, $nice_tmp_table, "the nice text", table notes, edit extend text );
        foreach($assoc_cache as $row) {
            $info_tab2[$info_tab_cnt][0] = array('<img src="'.$row[0].'"> '.$row[1], $row[2]);
            $info_tab_cnt++;
        }
    }
}

private function _obj_get_export_deny($sqlo, $tablename, $id) {
    $denied=0;
    $pk_name = PrimNameGet2($tablename);
    $stmt = "a.EXPORT_DENY from ".$tablename." x join CCT_ACCESS a on x.CCT_ACCESS_ID=a.CCT_ACCESS_ID WHERE ".$pk_name."=".$id;
    $sqlo->Quesel($stmt);
    $sqlo->ReadRow();
    if ($sqlo->RowData[0]>0)  {
        $denied=1;
    }
    return $denied;
}

// show features
function objFeaturesShow( &$sql ) {
	global $error;
	
	$tablename = $this->tablename;
	$prim_id   = $this->objid;
	$cct_access_id = $this->cct_access_id;
	
	list($info_tab2, $cctAccFeat, $info_tab_cnt) = $this->_objAccTabGet($sql);
	
	$wflow_id = $this->acc_data["wflow_id"];
	$wflowModText = "&nbsp;[<a href='p.php?mod=DEF/o.MODULE.trigger_gui&tablename=" . $this->tablename . "&id=".$this->objid."'>select/unselect</a>]";
	if ($wflow_id) {
	    $sqlsel= "name FROM module WHERE MXID = ". $wflow_id;
	    $sql->Quesel($sqlsel);
	    $sql->ReadRow();
	    $wflowname = $sql->RowData[0];
	    $info_tab2[][0] = array( "Workflow: ", "<a href='edit.tmpl.php?t=MODULE&id=".$wflow_id."'>".$wflowname."</a>".$wflowModText );
	} else {
	    $info_tab2[][0] = array( "Workflow: ", "none".$wflowModText );
	}
	$info_tab_cnt++;
	
	
	$d_wiid	 = $cctAccFeat["WIID"];
	$d_roiid = $cctAccFeat["ROID"];
	
	$tmpopt = array("colspan"=>2);
	$tmpstr = "http://".$_SERVER['HTTP_HOST'].$_SESSION["s_sessVars"]["loginURL"]."/edit.tmpl.php?t=".$tablename."&id=".$prim_id;
	if ($_SESSION["s_sessVars"]["g.db_index"]!="") $tmpstr = $tmpstr . "&dbid=".$_SESSION["s_sessVars"]["g.db_index"];
	$info_tab2[$info_tab_cnt][0] = array( "External URL", "<a href=\"".$tmpstr.
			"\" target=_new><img src=\"images/earth.gif\" border=0 TITLE=\"copy the link\"></a> <font color=gray>(copy the hidden link location)</font>",
			$tmpopt);
	$info_tab_cnt++;
	
	$tab_code = globTabMetaByKey($tablename, 'SHORT');
	if ($tab_code!=NULL) {
	    $info_tab2[$info_tab_cnt][0] = array( "Object-Short-Code:", 'db:'.$tab_code.':'.$prim_id, $tmpopt);
    	$info_tab_cnt++;
	}
	
	if (!empty($d_wiid)) {
		$sql->query("SELECT name, contact_id FROM h_wiid WHERE wiid = $d_wiid");
		$error->printAll();
		$has_wiid=1;
		if ($sql->ReadRow()) {
			$d_wiid_name  =$sql->RowData[0];
			$d_contact_id =$sql->RowData[1];
		} else {
		    $has_wiid=0;
		    $info_tab2[$info_tab_cnt]=array();
		    $info_tab2[$info_tab_cnt][0] = array( "Origin-WIID", "ERROR: Wiid-information missing.");
		}
		
		if ($has_wiid) {
    		$tmp_contact = "missing";
    		if (!empty($d_contact_id)) {
    			$sql->query("SELECT name FROM contact WHERE contact_id = $d_contact_id");
    			$error->printAll(); //
    			if ($sql->ReadRow()) {
    				$d_contact = $sql->RowData[0];
    				$tmp_contact = "<a  href='edit.tmpl.php?t=CONTACT&id=$d_contact_id'>$d_contact</a>";
    			} 
    		}
    		$info_tab2[$info_tab_cnt]=array();
    		$info_tab2[$info_tab_cnt][0] = array( "Origin-WIID", $d_wiid_name. "<br>Contact: $tmp_contact");
    		$info_tab2[$info_tab_cnt][1] = array( "Origin-ROID", $d_roiid);
		}
		$info_tab_cnt++;
	}
	
	// export deny level 1
	if ($cctAccFeat['EXPORT_DENY']>0) {
	    $tmpopt = array("LEVEL"=>"WARNING");
	    $info_tab2[$info_tab_cnt][0] = array( "Export denied", 'This object will not be exported by Paxml, caused by "Export-deny-flag"', $tmpopt);
	    $info_tab_cnt++;
	}
	// export deny level 2
	$conc_abs_lib = new gConcAbsC();
	$abs_table = $conc_abs_lib->getTemplate($tablename);
	if ($abs_table) {
	    $abs_obj = $conc_abs_lib->getAbsFrConc($sql, $tablename, $prim_id);
	    if ($abs_obj['id']) {
	        $denied = $this->_obj_get_export_deny($sql, $abs_table, $abs_obj['id']);
	        if ($denied) {
	            $obj_link = fObjViewC::bo_ID_URL($abs_table, $abs_obj['id']);
	            $tmpopt = array("LEVEL"=>"WARNING");
	            $info_tab2[$info_tab_cnt][0] = array( "Export denied", 
	                'This object will not be exported by Paxml, 
                     caused by "Export-deny-flag" of the <a href="'.$obj_link.'">abstract object</a>', $tmpopt);
	            $info_tab_cnt++;
	        }
	    }
	}
	
	// scan, if CCT_ACCESS_ID is used by more than one object
	if ($cct_access_id) $cctAccUsedNum = $this->_scanCCTaccessUsed($sql, $cct_access_id);
	
	if ($cctAccUsedNum>1) {
		$tmpopt = array("LEVEL"=>"WARNING");
		$info_tab2[$info_tab_cnt][0] = array( "ACCESS entry", ($cctAccUsedNum-1). " times used by other objects", $tmpopt);
		$info_tab_cnt++;
	}
	
	// "Copy from"
	if ($cctAccFeat['ORI_ID']) {
		$objLinkLib = new fObjViewC();
		$opts = array('showID'=>1);
		$objNice = $objLinkLib->bo_display( $sql, $tablename, $cctAccFeat['ORI_ID'], $opts );
		
		$info_tab2[$info_tab_cnt][0] = array( "was copied from", $objNice);
		$info_tab2[$info_tab_cnt][1] = array("Action", '[<a href="glob.objtab.cmp.php?tablename='.$tablename.'&parx[obj1]='.
			$prim_id . '&parx[obj2]='. $cctAccFeat['ORI_ID'].
			'">Show diferences to original object</a>]');
		$info_tab_cnt++;
	} else {
		//$info_tab2[$info_tab_cnt][0] = array( "was copied from", "-");
		//$info_tab_cnt++;
	}
	
	$this->_assoc_elemnts($sql, $info_tab2, $info_tab_cnt);
	
	// archive analysis
	//$info_tab2[$info_tab_cnt][0] = $this->_archiveAna($sql);
	//$info_tab_cnt++;
	
	
	$this->_tableOut( $info_tab2 );

	echo "</blockquote>\n";

}

function objAccTabGet( &$sql ) {
	
	list($info_tab2, $cctAccFeat, $info_tab_cnt) = $this->_objAccTabGet($sql);
	if (!$info_tab2)  return;
	$this->_tableOut( $info_tab2 );
}

}