<?php
/**
 * - show protocol log
 * - it contains code, which will be included in edit.tmpl.php, when the rider is set to "MODE"
 * $Header: trunk/src/www/pionir/obj.exp_tmpl.xmode.protocols.inc 59 2018-11-21 09:04:09Z $
 * @package    edit.tmpl.php
 * @subpackage obj.exp_tmpl.xmode.protocol.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

require_once ("gui/o.PROTO.stepout1.inc");
require_once ("gui/o.PROTO.stepout2.inc");

class oExpTmplProtX {
	var $edit_possible;
	
function __construct($id) {
	$this->expTmplID = $id;
}

function newProtoID( &$sql ){
	
	$id = $this->expTmplID;
	$sqls="select max(PROTO_ID), max(step_no) from EXP_TMPL_HAS_PROTO where EXP_TMPL_ID=".$id;
	$sql->query("$sqls");
	$sql->ReadRow();
	$this->proto_id_max = $sql->RowData[0];
	$step_no_max  = $sql->RowData[1]; 
	
	$this->proto_id_new = $this->proto_id_max+1;
	$this->step_no_new=$step_no_max+10;
}

function checkEditMode(&$o_rights) {
	
	
	$edit_possible=0;
	if ( $_SESSION['s_sessVars']["o.EXP_TMPL.editmode"] == "edit" ) {
		if ( $o_rights["insert"] ) {  
			$edit_possible=1;
		} else {
			echo "<B>INFO:</B> You need insert rights on experiment template to insert/update protocols.<br><br>\n";
		}
	}
	$this->edit_possible = $edit_possible;
	return ($this->edit_possible);
}

function showProtos( &$sql, &$sql2) {
	// INPUT: $this->proto_arr
	/*		
	echo "<table cellpadding=2 bgcolor=#a0b8c8 width=100%><tr>";
	echo "<td><font color=#FFFFFF size=+1> ";
	echo "<B>Long list of abstract protocols</B> </font></td></tr>";
	echo "</table>\n";
	*/
    echo "<br>\n";
	echo '<span style="color:gray; font-weight:bold; font-size:1.2em;">List of protocol details</span><br><br>'."\n";
	
	$protoShowObj = new oProtocolShowC();
	$protoShowObj->writeJavascript();
	
	foreach( $this->proto_arr as $proto_st=>$proto_id) {
	
		$sqls="select name, H_PROTO_KIND_ID from ABSTRACT_PROTO ".
			" where ABSTRACT_PROTO_ID=".$proto_id;
	
		$sql->query("$sqls");
		$sql->ReadRow();
		$a_proto_name	=$sql->RowData[0];
		$a_proto_kind	=$sql->RowData[1];
		echo "<a name=\"$proto_st\"></a>\n";
		
		// echo "<table bgcolor=#DFDFDF><tr>\n";
		// echo "<td> <img src=\"images/but.go13.gif\"> <font color=gray>".$proto_st.".</font>";
		echo " <a href=\"edit.tmpl.php?t=ABSTRACT_PROTO&id=".$proto_id."\"><B>".$a_proto_name."</B></a>";
		// echo "</td></tr>";
		// echo "</table>\n";
		
		//echo "<blockquote>\n";
		$protoShowObj->showAll( $sql, $sql2, NULL, $proto_id, NULL );
		//echo "</blockquote>\n";
		echo "<br>";
		
	} 
	
}

function proto_new_form() {
	$id = $this->expTmplID;
	
	htmlInfoBox( "Add a new protocol", "", "open", "CALM" );
	echo "<form name=editform style=\"display:inline;\" ACTION=\"obj.exp_tmpl.step_actio.php\" METHOD=POST>\n";
	
	$xxopt=array( 
	    "showproj" => 1,
	    "noshDel"  => 1
	    
	);
    $jsFormLib = new gJS_edit();
	$answer = $jsFormLib->getAll("ABSTRACT_PROTO", "abstract_proto", "", "          ---          ", 0, $xxopt);
	echo $answer;
  
	echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
	echo "Step-No: <input type=text name=\"step_no_new\" size=2 value=\"".$this->step_no_new."\">";
	echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
	echo '<input class="yButton" type=submit value="Add protocol">'; 
	
	$thide = NULL;
	$thide["id"] 		= $id;
	$thide["xaction"] 	= "add";
	$thide["proto_id_new"] = $this->proto_id_new;
	
	foreach( $thide as $key=>$val) {
		echo "<input type=hidden name=\"".$key."\" value=\"".$val."\">\n";
	}
	
	echo "</form>\n";
	htmlInfoBox( "", "", "close" );
	echo "<br>\n";
	
}

function showShortTable(&$sql, &$sql2) {
	
	$id = $this->expTmplID;
	
	$sqls="select PROTO_ID, ABSTRACT_PROTO_ID, STEP_NO from EXP_TMPL_HAS_PROTO where EXP_TMPL_ID=".$id. 
			" order by STEP_NO";
	$sql->query($sqls);
	
	$cnt=0;
	echo "<table border=0 cellspacing=1 cellpadding=0 bgcolor=#E0E0FF><tr><td>"; // light blue bgcolor
	echo "<table bgcolor=#EFEFEF border=0 cellspacing=0 cellpadding=2>\n";
	
// 	echo "<tr bgcolor=#A0A0A0>";
// 	echo '<td style="color:#FFFFFF" colspan=4>Short list</td>';
// 	echo "</tr>\n";
	
	
	echo "<tr bgcolor=#A0A0A0>";
	$colarr=array("No", "Kind",tablename_nice2('ABSTRACT_PROTO'));
	if ($this->edit_possible) {
		$colarr[] = "Action";
	}
	
	foreach( $colarr as $key=>$val) {
		echo "<td class=special> ".$val." </td>";
	}
	
	echo "</tr>\n";
	
	$this->proto_arr = array();
	
	while ( $sql->ReadRow() ) {
		$a_kind="";
		$a_proto_id="";
		
		$proto_id   = $sql->RowData[0]; /* just loaded from DB */
		$a_proto_id = $sql->RowData[1]; /* just loaded from DB */
		$step_nr    = $sql->RowData[2]; /* just loaded from DB */
		
		if ( $a_proto_id ) {
			$sqls="select NAME, H_PROTO_KIND_ID from ABSTRACT_PROTO where ABSTRACT_PROTO_ID=".$a_proto_id;
			$sql2->query("$sqls");
			$sql2->ReadRow();
			$a_name = $sql2->RowData[0];
			$a_kind = $sql2->RowData[1];
		}
		
		if ($a_kind) {
			$sqls="select NAME from H_PROTO_KIND where H_PROTO_KIND_ID=".$a_kind;
			$sql2->query("$sqls");
			$sql2->ReadRow();
			$a_kind_name = $sql2->RowData[0];
		}
		$inf3 = "";
		
		
		echo "<tr>";
		echo "<td>&nbsp;$step_nr</td>";
		echo "<td>&nbsp;$a_kind_name</td>";
		echo "<td>&nbsp;".
				"<img src=\"images/icon.ABSTRACT_PROTO.gif\" border=0> ".
				"<a href=\"edit.tmpl.php?t=ABSTRACT_PROTO&id=".$a_proto_id."\">".
				$a_name."</a></td>";
		if ($this->edit_possible) {
			$inf3 = "<a href=\"obj.exp_tmpl.step_actio.php?id=".$id."&xaction=del&proto_id=".$proto_id."\">".
					"<img src=\"images/but9.del.gif\" TITLE=\"delete step\" border=0></a>";
			echo "<td>&nbsp;$inf3</td>";
		}
		
		echo "</tr>\n";
		$this->proto_arr[$step_nr]=$a_proto_id;
		$cnt++;
	} 
	echo "</table>\n";
	echo "</td></tr></table>\n";
}

}


/**
 * extension of edit.tmpl.php
 */
class oEXP_TMPL_xmode_protocols {
	
function __construct( &$editLib ) {
	$this->editLib = &$editLib;
}


function xmode_start( &$sqlo, &$sqlo2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {

        
	echo "<UL>\n";
	
	$H_KIND_name='';
	$special_type='Nucleic Acid';
	if ($arguByKey['H_KIND_OF_INTERACT_ID']) {
	    $H_KIND_name=glob_elementDataGet( $sqlo, 'H_KIND_OF_INTERACT', 'H_KIND_OF_INTERACT_ID', $arguByKey['H_KIND_OF_INTERACT_ID'], 'NAME');
	}
	
	if ($H_KIND_name==$special_type) {
	    require_once($_SESSION['s_sessVars']['AppLabLibDir'].'/lablib/gui/o.EXP_TMPL.prot_NA.inc');
	    $expTmplLib = new oExpTmplProt_NA($sqlo, $id);
	    $in_blocks=array();
	    $expTmplLib->form($sqlo, $sqlo2, $in_blocks);
	    
	} else {
	    
    	$expTmplLib = new oExpTmplProtX($id);
    	$expTmplLib->newProtoID($sqlo);
    	
    	$edit_possible = $expTmplLib->checkEditMode($x_rights);
    	
    	// $listlink = '<a class="yGgray" href="view.tmpl.php?t=EXP_TMPL_HAS_PROTO&searchMothId='.$id.'">[Advanced list]</a> ';
    	
    	if ($edit_possible) {	
    	    $expTmplLib->proto_new_form();
    	}
    	
    	if ( !$expTmplLib->proto_id_max ) {
    		echo " <font color=gray>No protocol defined.</font>\n";
    		return;
    	}
    	
    	$expTmplLib->showShortTable($sqlo, $sqlo2);

    	$expTmplLib->showProtos( $sqlo, $sqlo2);
    	
    	echo "</UL>";
	}
}

}