<?php
/**
 * - substance PROTOCOL things
 * - show list of QC and prep protocols
 * - it contains code, which will be included in edit.tmpl.php, when the rider is set to "MODE"
 * @swreq SREQ:0002756: o.ABSTRACT_SUBST > Reiter: "Protocols" : zeige Protocol-Editor 
 * @package obj.abstract_subst.xmode.prot.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * $Header: trunk/src/www/pionir/obj.abstract_subst.xmode.prot.inc 59 2018-11-21 09:04:09Z $
 */

require_once ("f.objview.inc");
require_once ("func_form.inc");
require_once ("o.ABSTRACT_SUBST.qcprot.inc");
require_once ("o.ABSTRACT_SUBST.qmana.inc");
require_once ("o.AS_HAS_PR.subs.inc");
require_once ("gui/o.PROTO.stepout2.inc");	
require_once 'lev1/o.SUA.versman.sub.inc';
require_once 'o.ABSTRACT_PROTO.subs.inc';

/**
 * helper class
 * @author steffen
 */
class oSUA_prepX_help {
	var $edit_possible;
	var $infox; // info array
	var $XORDER_LIMIT=9998;
	var $proto_arr; // proto_arr_STRUCT
	
    function __construct(&$sqlo, $id) {
    	$this->objid = $id;

    	$this->AS_HAS_PR_lib = new oAS_HAS_PR_subs($this->objid);
    	$this->proto_arr =  $this->AS_HAS_PR_lib->getProtoLog($sqlo); 
    		
    	$this->_newProtoID($sqlo);
    	
    	$this->praSubLib = new oABSTRACT_PROTO_subs();
    }
    
    function _newProtoID( &$sqlo ){
    	
    	
    	$id = $this->objid;
    	$sqls="max(STEP_ID), max(XORDER) from AS_HAS_PR where ABSTRACT_SUBST_ID=".$id;
    	$sqlo->Quesel($sqls);
    	$sqlo->ReadRow();
    	$this->proto_id_max = $sqlo->RowData[0];
    	$XORDER_max  		= $sqlo->RowData[1]; 
    	
    	$this->XORDER_max   = $XORDER_max;
    	$this->proto_id_new = $this->proto_id_max+1;
    	$this->XORDER_new   = intval($XORDER_max)+10;
    }
    
    /**
     * substance has protocols ?
     */
    function hasProtos() {
        $answer = 0;
        if (!empty($this->proto_arr)) $answer = 1;
        return $answer;
    }
    
    function getXORDER_max() {
    	return $this->XORDER_max;
    }
    
    function getProtos() {
    	return $this->proto_arr;
    }
    
    function checkEditMode(&$o_rights) {
    	
    	
    	$edit_possible=0;
    	if ( $_SESSION['s_sessVars']["o.ABSTRACT_SUBST.editmode"] == "edit" ) {
    		if ( $o_rights["insert"] ) {  
    			$edit_possible=1;
    		} else {
    			echo "<B>INFO:</B> You need insert rights on this object to insert/update protocols.<br><br>\n";
    		}
    	}
    	$this->edit_possible = $edit_possible;
    	return ($this->edit_possible);
    }
    
    function showProtos( &$sql, &$sql2) {
    	// INPUT: $this->proto_arr
    			
    	echo "<table cellpadding=2 bgcolor=#a0b8c8 width=100%><tr>";
    	echo "<td><font color=#FFFFFF size=+1> ";
    	echo "<B>Long list of abstract protocols</B> </font></td></tr>";
    	echo "</table>\n";
    	
    	$protoShowObj = new oProtocolShowC();
    	$protoShowObj->writeJavascript();
    	
    	
    	foreach( $this->proto_arr as $proto_st=>$proto_id) {
    	
    		$sqls="name, H_PROTO_KIND_ID from ABSTRACT_PROTO ".
    			" where ABSTRACT_PROTO_ID=".$proto_id;
    	
    		$sql->Quesel($sqls);
    		$sql->ReadRow();
    		$a_proto_name	=$sql->RowData[0];
    		$a_proto_kind	=$sql->RowData[1];
    		echo "<a name=\"$proto_st\"></a>\n";
    		echo "<table bgcolor=#DFDFDF><tr>\n";
    		echo "<td> <img src=\"images/but.go13.gif\"> <font color=gray>".$proto_st.".</font>";
    		echo " <a href=\"edit.tmpl.php?t=ABSTRACT_PROTO&id=".$proto_id."\"><B>".$a_proto_name."</B></a> </td></tr>";
    		echo "</table>\n";
    		echo "<blockquote>\n";
    		$protoShowObj->showAll( $sql, $sql2, NULL, $proto_id, NULL );
    		echo "</blockquote>\n";
    		echo "<br>";
    		
    	} 
    	
    }
    
    /**
     * add protocol
     * @param string $title
     * @param string $showFinal --  DEPRECATED
     * @param number $order
     * @param number $type
     */
    function _addProtoGuiSub( $title, $showFinal, $order=0, $type=0  ) {
    	$id = $this->objid;
    	
    	if (!$this->infox['formPos']) $this->infox['formPos']=0;
    	$formElementPos = $this->infox['formPos'];
    	
    	$this->infox['formPos'] = $this->infox['formPos'] + 1;
    	$jsFormLib = new gJS_edit();
    	$jsFormLib->setTable("ABSTRACT_PROTO", $formElementPos);

//OLD ...
//     	function show_Obj_selector() {
//     	    var node = document.getElementById('addProtoSelProto');
//     	    var visibility = node.style.visibility;
//     	    node.style.visibility = "visible";
//     	    PHP: echo $jsFormLib->getJS_open_list().';';           
//<!--   } -->
    	
    	?>
    	<script>
    	  function show_Obj_selector2() {
    		<?php 
    		// show OBJECT selector, if option=2 is selected
    		?>
    		var sel_node = document.getElementById('form_sel2');
			var sel_val  = sel_node.value;
    		if (sel_val==2) {
            	var pra_obj_node = document.getElementById('addProtoSelProto');
                var visibility   = pra_obj_node.style.visibility;
            	pra_obj_node.style.visibility = "visible";
                <?php echo $jsFormLib->getJS_open_list().';';?>
    	  }
        }
    	</script>
    	<?php
    	
    	htmlInfoBox( "Add a ".$title, "", "open", "INFO" );
    	
    	echo "<form name=editform style=\"display:inline;\" 
    		ACTION=\"p.php?mod=DEF/o.ABSTRACT_SUBST.hasProtMod&action=add\" METHOD=POST>\n";
    	
    	echo '<table><tr>'."\n";
    	echo '<td>';
    	
    	echo '<input type=submit class="yButSmall" value="Add protocol">';
    	
    	$thide = NULL;
    	$thide["id"] 		= $id;
    	$thide["action"] 	= "add";
    
	    $feld = $this->AS_HAS_PR_lib->getTypeDef();
	    $selString = formc::selectFget('parx[TYPE]',$feld,1);

	    echo "&nbsp;&nbsp;";
	    
	    // 
	    $sel_option=array(
            "selecttext"=>'--- select modus ---',
    	    "selbody" =>'onchange="show_Obj_selector2();"',
    	    "id"      =>'form_sel2',
    	    "req"     => 1
    	);
	    $crea_feld=array(
	       1=>'create a NEW one',
	       2=>'select an existing one'
	    );
	    $crea_type_sel = formc::selectFget('opt[crea_pra]',$crea_feld,0, $sel_option );
	    echo $crea_type_sel.' &nbsp;&nbsp;&nbsp;  ';
	  
	    //NEW: echo 'Create new:       <input type=radio name="opt[crea_pra]" value="1" checked>&nbsp;&nbsp;&nbsp;  ';
	    echo "No: <input type=text name=\"parx[XORDER]\" size=2 value=\"".$this->XORDER_new."\">&nbsp;&nbsp;";
	    echo "Type: ".$selString;
    	
    	
    	//NEW:echo '&nbsp;&nbsp;&nbsp;&nbsp; add existing: <input type=radio name="opt[crea_pra]" value="0" onclick="show_Obj_selector();">&nbsp;&nbsp;&nbsp; ';
    	
    	echo '</td>'."\n";
    	echo '<td id="addProtoSelProto" style="visibility: hidden">';
    	
    	$xxopt=array( "showproj" => 1);
    	$answer = $jsFormLib->getAll("ABSTRACT_PROTO", "parx[ABSTRACT_PROTO_ID]", "", " ---  ", $formElementPos, $xxopt);
    	echo $answer;
    	
    	echo '</td></tr></table>'."\n";
    	
    	foreach( $thide as $key=>$val) {
    	    echo "<input type=hidden name=\"".$key."\" value=\"".$val."\">\n";
    	}
    	echo "</form>\n";
    	
    	htmlInfoBox( "", "", "close" );
    	echo "<br>\n";
    }
    
    
    
    function addProtoGui() {
    	$this->_addProtoGuiSub( "new protocol", 0 );
    }
    
    function _rowOut($valarr) {
    	echo "<tr>";
    	foreach($valarr as $oneval) {
    		echo "<td>&nbsp;".$oneval."</td>";
    	}
    	echo "</tr>\n";
    }
    
    /**
     * one row
     * @param  $sql
     * @param  $valarr
     */
    private function _oneShortRow(&$sql, $valarr) {
    	
    	$objid = $this->objid;
    	
    	$a_proto_id = $valarr['ap'];
    	$step_id    = $valarr['st'];
    	$XORDER     = $valarr['or'];
    	$type       = $valarr['ty'];
    	
    	if (!$step_id and !$this->finalsNow) {
    		$this->_rowOut( array('---', '---', '--- final ---') );
    		$this->finalsNow=1;
    	}
    	
    	$a_kind		= 0;
    	$a_name		= "";
    	$a_kind_name= NULL;
    	$infostr    = NULL;
    	
    	if ( $a_proto_id ) {
    		$sqls="NAME, H_PROTO_KIND_ID from ABSTRACT_PROTO where ABSTRACT_PROTO_ID=".$a_proto_id;
    		$sql->Quesel($sqls);
    		$sql->ReadRow();
    		$a_name = $sql->RowData[0];
    		$a_kind = $sql->RowData[1];
    	}
    	
    	if ($a_kind) {
    		$sqls="NAME from H_PROTO_KIND where H_PROTO_KIND_ID=".$a_kind;
    		$sql->Quesel($sqls);
    		$sql->ReadRow();
    		$a_kind_name = $sql->RowData[0];
    	}
    	$actstr = "";
    	
    	$typeDef  = $this->AS_HAS_PR_lib->getTypeDef();
    	$typeNice = $typeDef[$type];
    	$actstr = NULL;
    	
    	if ($this->edit_possible) {
    		
    		$step_id_param = $step_id;
    		if ($XORDER>=$this->XORDER_LIMIT) {
    			$step_id_param = $XORDER;
    		}
    		
    		$numUsedArr = array();
    		if ( $a_proto_id ) {
    			$numUsedArr = $this->praSubLib->isUsed($sql, $a_proto_id, $objid);
    		}
    		
    		$actstr = 
    			"<a href=\"p.php?mod=DEF/o.ABSTRACT_SUBST.hasProtMod&action=delete&id=".$objid.
    			"&parx[STEP_ID]=".$step_id_param."\">".
    			"<img src=\"images/but.del2.gif\" TITLE=\"remove protocol\" border=0></a>";
    		
    		if ( ($numUsedArr['PRC'] + $numUsedArr['SUA']) > 0 ) {
    			// show only, if already used
    			$actstr .= 
    				"&nbsp;&nbsp;<a href=\"p.php?mod=DEF/o.ABSTRACT_SUBST.hasProtMod&action=copy&id=".$objid.
    				"&parx[XORDER]=".$XORDER."\">".
    				"<img src=\"images/but.copyobj.gif\" TITLE=\"create copy\" border=0></a>";
    		}
    		
    		if ( $a_proto_id ) {
    			if ($numUsedArr['PRC']+$numUsedArr['SUA']>0) {
    				$infostr .= ' <img src="images/i13_info.gif"> used by: '; 
    			}
    			if ($numUsedArr['PRC']>0) {
    				$infostr .= 'PRC:'.$numUsedArr['PRC'];
    			}
    			if ($numUsedArr['SUA']>0) {
    				if ($numUsedArr['PRC']>0) $infostr .= ', ';
    				$infostr .= 'SUA:'.$numUsedArr['SUA'];
    			}
    		}
    			
    	}
    	
    	// QC-protocol ?
    	if ( $a_proto_id and $type==2) {
    		$protoErrors = $this->suaQm_anaLib->getQM_prob_qcproto($sql, $a_proto_id);
    		if (!empty($protoErrors)) {
    			if ($infostr!=NULL) $infostr .= "<br />";
    			$infostr .= ' <span style="color:#A4671E;">Warning:</span> '. implode('; ',$protoErrors);
    		}
    	}
    	
    	
    	
    	$this->_rowOut( array(
    		$XORDER,
    		$a_kind_name,
    			"<img src=\"images/icon.ABSTRACT_PROTO.gif\" border=0> ".
    			"<a href=\"edit.tmpl.php?t=ABSTRACT_PROTO&id=".$a_proto_id."\">".
    			$a_name."</a>",
    		$typeNice,
    		$actstr,
    		$infostr
    		)
    	);
    	
    	echo "</tr>\n";
    }
    
    
    
    /**
     * show short protocol table
     * @param  $sql
     */
    function showShortTable(&$sql) {

        $t_short = globTabGetSHORT('ABSTRACT_SUBST');
        $tpra_short = globTabGetSHORT('ABSTRACT_PROTO');
    	$id = $this->objid;
    	$colarr=array("No", "Kind", "Protocol (abstract)", "Type", "Action", "Info");
    	$cnt=0;
    	$colspan=sizeof($colarr);
    	$head_color='#C0C0C0';
    	
    	echo "<table border=0 cellspacing=1 cellpadding=0 bgcolor=".$head_color."><tr><td>"; 
    	echo "<table bgcolor=#EFEFEF border=0 cellspacing=0 cellpadding=2>\n";
    	
    	
    	echo "<tr bgcolor=".$head_color.">\n";
    	
    	$sql_whereEnc = urlencode("x.ABSTRACT_PROTO_ID IN ".
    	   "(select x.ABSTRACT_PROTO_ID from AS_HAS_PR x WHERE (x.ABSTRACT_SUBST_ID =".$this->objid."))" ) ;
    	
    	echo '<td colspan="'.$colspan.'"> &nbsp;<span style="color: #606060; font-weight:bold;">'.$t_short.' Protocol log</span> &nbsp;&nbsp;&nbsp;'.
        	'<a href="view.tmpl.php?t=ABSTRACT_PROTO&condclean=1&tableSCond='.$sql_whereEnc.'">'.
        	'<img src="images/but.list2.gif" style="vertical-align:bottom"> '.
        	$tpra_short.'s as list</a>'.
    	  '</td>';
    	echo "</tr>\n";
    	
    	echo "<tr bgcolor=".$head_color.">\n";
    	foreach( $colarr as $key=>$val) {
    		echo '<th style="color: #FFFFFF; font-weight:bold;"> '.$val." </th>";
    	}
    	echo "</tr>\n";
    	
    	$this->suaQm_anaLib = new oABSTRACT_SUBST_qmana();
    	$this->suaQm_anaLib->setObj($sql, $id);
    	
    	if (is_array($this->proto_arr)) {
    		$this->finalsNow = 0;
    		reset ($this->proto_arr);
    		foreach( $this->proto_arr as $key=>$valarr) {
    	
    			$this->_oneShortRow($sql, $valarr);
    	
    			$cnt++;
    		} 
    		
    	}
    	echo "</table>\n";
    	echo "</td></tr></table>\n";
    }
    
    function show_action_links() {
        echo '[<a href="p.php?mod=DEF/o.ABSTRACT_SUBST.hasProtMod&id='.$this->objid.
            '&action=ord.move.many">Change order of protocols</a>]';
        echo '<br>'."\n";
    }



}

/**
 * helper class for long report
 * @author steffen
 */
class oSUA_protoFull_help {

	
	function __construct(&$sqlo) {
		$this->objLinkLib = new fObjViewC();
		
		$this->protoShowObj = new oProtocolShowC();
		$this->protoShowObj->writeJavascript();
		
		$AS_HAS_PR_lib = new oAS_HAS_PR_subs();
		$this->protoType = $AS_HAS_PR_lib->getTypeDef();
	}
	
	/**
	 * 
	 * Enter description here ...
	 * @param $sqlo
	 * @param $sqlo2
	 * @param array $valarr array('st'=>$step_id, 'ap'=>$a_proto_id, 'or'=>$XORDER, 'ty'=>$type)
	 * @param int $cnt 
	 */
	function showOneProto(&$sqlo, &$sqlo2, $valarr, $cnt) {
		
		$apid  = $valarr['ap'];
		$order = $valarr['or'];
		$typex = $valarr['ty'];
		$typeNice = $this->protoType[$typex];
		
		$htmltxt = $this->objLinkLib->bo_display( $sqlo, 'ABSTRACT_PROTO', $apid ); 
		
		if ($cnt) echo "<br /><hr>\n";
		// echo "<b>No: ".$order." Type: ". $typeNice."</b><br>\n";
		echo '<span class="yGgrayBold">Details of </span>'. $htmltxt .' :';
		echo '<br />'."\n";
		/*
		$answer = $substQCProtoLib->isValid($sql, $qcproto_id);
		if ($answer[0]<1) {
			echo '&nbsp; <b><font color=red>WARNING:</font></b> invalid QC-protocol: '.$answer[1].'<br />'."\n";
		}
		*/
		$this->protoShowObj->showAll($sqlo, $sqlo2, 0, $apid, NULL );
	}
	
	/**
	 * 
	 * Enter description here ...
	 * @param $sqlo
	 * @param $sqlo2
	 * @param array proto_arr_STRUCT $proto_arr
	 */
	function showAll( &$sqlo, &$sqlo2, &$proto_arr ) {
		if (!is_array($proto_arr)) {
			echo "no protocol exists for this substance.<br>\n";
			return;
		}
		
		$cnt=0;
		
		foreach( $proto_arr as $key=>$valarr) {
	
			$this->showOneProto($sqlo, $sqlo2, $valarr, $cnt);
	
			$cnt++;
		} 	
	
	}

}

// ------------
		
class oABSTRACT_SUBST_xmode_prot {
	
	function __construct( &$editLib ) {
	}		
	
	function xmode_start( &$sql, &$sql2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {
		global $varcol;
	    
		echo '<ul>'."\n";
		/**
		 * - show protocol editor
		 * - show list
		 */
		
		$class_id = $extraobj_o['extra_class_id'];
		if ($class_id) {
		    $class_name =  $varcol->class_id_to_name( $class_id );
		}

		$versLib  = new oSUA_versman_sub();
		$versType = $versLib->getSUA_type($sql, $id);
		
		if ($versType==1) {
			$edit_possible = 0;
			htmlInfoBox( "VersionMaster!", 'Diese Substanz ist vom Type "VersionMaster". Er darf keine Protokolle enthalten.', "", "INFO" );
			echo "<br>";
		}
		
		if ($class_name=='NA_type') {
		    // (Nucleid assay)
		    require_once($_SESSION['s_sessVars']['AppLabLibDir'].'/lablib/gui/o.ABSTRACT_SUBST.prot_NA.inc');
		    $sua_gui_lib = new oABSTRACT_PROTO_prot_NA($sql, $id);
		    $in_blocks=array();
		    $sua_gui_lib->form($sql, $sql2, $in_blocks);
		    echo '</ul>'."\n";
		    return;
		}
		
		
		$suaHelpLib = new oSUA_prepX_help($sql, $id);
		$edit_possible = $suaHelpLib->checkEditMode($x_rights, $editAllow);
		// $listlink = "<a href=\"view.tmpl.php?t=AS_HAS_PR&searchMothId=".$id."\">[Advanced list]</a> ";
		
		if ($edit_possible) {	
			$suaHelpLib->addProtoGui();
		}
		
		// show only, if exists ...
		$SUC_hasProtos = $suaHelpLib->hasProtos();
		if ($SUC_hasProtos>0) {
		    
			$suaHelpLib->showShortTable($sql);
			if ($edit_possible) {
			    $suaHelpLib->show_action_links();
			}

			echo "<br /><br />\n";
			$proto_arr = $suaHelpLib->getProtos();
			$longReportLib = new oSUA_protoFull_help($sql);
			$longReportLib->showAll($sql, $sql2, $proto_arr );
			//}
		} else {
		    echo '<span class="yGgray">no protocols.</span>'."\n";
		}
		
		echo '</ul>'."\n";
		
	}
}

/* OLD: 
 * OPTIONAL: switch short/long protocol ...
 * if ( $_SESSION['userGlob'][$userglobalVar]=="long" ) {
 $userglobalVar='o.ABSTRACT_SUBST.tabmode_proto';
 if ( $_SESSION['userGlob'][$userglobalVar]=="less" or $_SESSION['userGlob'][$userglobalVar]==NULL ) {
 echo '[<a class=xe href="preferences.php?var='.$userglobalVar.'&val=long"><img src="images/but.plus.gif" border=0 > Long report</a>]';
 } else {
 echo '<br \><br \><span style="font-size:1.2em; font-weight:bold;">Long report</span> &nbsp;&nbsp;&nbsp;'.
 '[<a class=xe href="preferences.php?var='.$userglobalVar.'&val=less"><img src="images/but.minus.gif" border=0 > Unshow report</a>]';
 echo "<hr />\n";
 }
 echo "<br />\n";
 */

