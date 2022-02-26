<?php
require_once ("gui/o.PROTO.stepout1.inc");

/**
 * show ONE protocol: including steps and advanced info (abstract proto info)
 * @package o.PROTO.stepout2.inc (old: obj.protocol.show.inc)
 * @swreq   SREQ:0001608: o.CONCRETE_PROTO > HTML-addin to show a protocol 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @see usedby: e.g. gui/o.CONCRETE_SUBST.protos_show.inc (CONCRETE_SUBST))
 */
class oProtocolShowC {

    function writeJavascript() {
    	?>
    	<script language="JavaScript">
    	function c_gostep( id, step) {
    		url_name="edit.tmpl.php?t=CONCRETE_PROTO_STEP";
    		location.href=url_name+ "&id=" + id + "&primas[1]=STEP_NR&primasid[1]="+step;
    	}
    	function a_gostep( id, step) {
    		url_name="edit.tmpl.php?t=ABSTRACT_PROTO_STEP";
    		location.href=url_name+ "&id=" + id + "&primas[1]=STEP_NR&primasid[1]="+step;
    	}
    	function goSubst( id ) {
    		url_name="edit.tmpl.php?t=CONCRETE_SUBST&id=" + id ;
    		location.href=url_name;
    	}
    	function goDev( id ) {
    		url_name="edit.tmpl.php?t=CHIP_READER&id=" + id ;
    		location.href=url_name;
    	}
    	//-->
    	</script>
    	<? 
    }
    
    /**
     * show one ABSTRACT_PROTO
     * @param object $sql
     * @param object $sql2
     * @param int $a_proto_id 
     * @param array $options
     */
    function proto_abstract_show(  &$sql,  &$sql2,   $a_proto_id, $options=array()  ) {       
    
        $sqls= "select count(*) from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$a_proto_id;
    	$sql->query("$sqls");
    	$sql->ReadRow();
    	$steps=$sql->RowData[0];   
        
        $sqls= "select NAME from ABSTRACT_PROTO where ABSTRACT_PROTO_ID=".$a_proto_id;
    	$sql->query("$sqls");
    	$sql->ReadRow();
    	$abstr_proto_name=$sql->RowData[0];
    	
        echo '<span style="color:gray">Protocol (abstract):</span> <B>'. $abstr_proto_name .'</B> [ID:'.$a_proto_id.']<br>'."\n";
        if ( $steps ) {
            $proto_step_c = new  protostep( $sql, $sql2, $a_proto_id, 0, 0, 0, $options );
            $proto_step_c->tableOut($sql);
    
        } else {
            echo "&nbsp;&nbsp;&nbsp; <font color=gray>No steps defined...</font><br>";
        }
    }
    
    /**
     * show full protocol
     * @param object $sql
     * @param object $sql2
     * @param int $c_proto_id  CONCRETE_PROTO_ID 
     * @param int $a_proto_id  [0], if $c_proto_id given, will be overridden 
     * @param array  $options
     *   "out_type"
    	 "shBlocks" = array( "abs_notes" => -1, 0, [1] ) - show abstract notes ?
     */
    function showAll(object &$sql, object &$sql2, $c_proto_id, $a_proto_id, $options=array() ) {

      $shBlocks = array(
         "abs_notes" => 1
      );
      if ($options["shBlocks"]["abs_notes"]<0) {
    	  $shBlocks["abs_notes"]=0;
      } 
      
      $editAllow= 0;
      $link_id  = 0;
      
      if ( $c_proto_id ) {
            
    	/* overwrite ABSTRACT_PROTO, otherwise conflict between ABSTRACT and CONCRETE !!! */
    	$sqls= "select ABSTRACT_PROTO_ID, NOTES from CONCRETE_PROTO where CONCRETE_PROTO_ID=".$c_proto_id;
    	$sql->query("$sqls");
    	$sql->ReadRow();
    	$a_proto_id   =$sql->RowData[0];
    	$c_proto_notes=$sql->RowData[1];
    	
//     	$sqls= "select count(*) from CONCRETE_PROTO_STEP where CONCRETE_PROTO_ID=".$c_proto_id;
//     	$sql->query("$sqls");
//     	$sql->ReadRow();
//     	$c_steps=$sql->RowData[0];
    	
    	if ( strlen ($c_proto_notes) ) {
    	  $html_txt = htmlspecialchars ($c_proto_notes);
    			echo "<font color=gray>Notes from protocol (concrete):</font><br><pre>$html_txt</pre>\n";
    	}
      }
      
      if ( $a_proto_id  ) {
    	
    	$sqls= "select count(*) from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$a_proto_id;
    	$sql->query("$sqls");
    	$sql->ReadRow();
    	$steps=$sql->RowData[0];
    	
    	$sqls= "select notes from ABSTRACT_PROTO where ABSTRACT_PROTO_ID=".$a_proto_id;
    	$sql->query("$sqls");
    	$sql->ReadRow();
    	$abstr_proto_notes=$sql->RowData[0];
    	
    	if ( $shBlocks["abs_notes"] ) { // show notes ???
    		if ( strlen ($abstr_proto_notes) ) {
    		$html_txt = htmlspecialchars ($abstr_proto_notes);
    		echo "<font color=gray>Notes from protocol (abstract):</font><pre>$html_txt</pre>\n";
    		}
    	}
    	
      } else {
    	echo "<font color=gray>No abstract protocol defined.</font><br>\n";
      }
      
      if ( $c_proto_id ) {
    	$sqls="select i.link_id, i.name from CONCRETE_PROTO c, LINK i where c.CONCRETE_PROTO_ID=" .
    			$c_proto_id . " AND c.LINK_ID=i.LINK_ID";   
    	$sql->query("$sqls");
    	if ($sql->ReadRow()) {
    		  $link_id = $sql->RowData[0];
    		  $link_name = $sql->RowData[1];
    	}	
      } else if ($a_proto_id) {	// CONCRETE proto not set, try ABSTRACT proto
      	$sqls="select i.link_id, i.name from ABSTRACT_PROTO c, LINK i where c.ABSTRACT_PROTO_ID=" .
    			$a_proto_id . " AND c.LINK_ID=i.LINK_ID";   
    	$sql->query("$sqls");
    	if ($sql->ReadRow()) {
    		  $link_id = $sql->RowData[0];
    		  $link_name = $sql->RowData[1];
    	}
      }
      
      if ($link_id) {
    	echo "<img src=\"images/icon.LINK.gif\" border=0> Attached document: <a href=\"edit.tmpl.php?t=LINK&id=$link_id\"> $link_name </a><br>";
      }
    	
      if ( $steps ) { 
      
        $optShow = unserialize($_SESSION['userGlob']["o.CONCRETE_PROTO.show"]);
    	$options["abstrStepsShow"] = $optShow["abstrStepsShow"];
    	
        if ( empty($options["out_type"]) ) $options["out_type"] = $_SESSION['userGlob']["o.PROTO.outmode"];
    	$proto_step_c = new  protostep( $sql, $sql2, $a_proto_id, $editAllow, 0, $c_proto_id, $options );
    	$proto_step_c->tableOut($sql);
        
      }
    }
    
    function showProtoArr( 
    	&$sql, 
    	&$sql2, 
    	&$proto_arr // array(stepnr) = $concprot_id
    	) {
    
    	require_once ("gui/o.PROTO.stepout1.inc");
    	require_once ("obj.protocol.show.inc");
    
    	echo "</UL><br>";
    	echo "<table cellpadding=2 bgcolor=#a0b8c8 width=100%><tr>";
    	echo "<td><font color=#FFFFFF> ";
    	echo "<B>Long list of performed protocols</B></font></td></tr>";
    	echo "</table>\n";
    
    	# echo "<UL>";
    
    	if ( !sizeof ($proto_arr) ) {
    		return;
    	}
    	
    	$this->writeJavascript();
    		
    	foreach( $proto_arr as $step_no=>$concprot_id) {
    
    		if ($concprot_id) {
    
    			$sqls = "select a.name, a.H_PROTO_KIND_ID, a.ABSTRACT_PROTO_ID from CONCRETE_PROTO c, ABSTRACT_PROTO a ".
    				" where c.CONCRETE_PROTO_ID=".$concprot_id. " AND c.ABSTRACT_PROTO_ID=a.ABSTRACT_PROTO_ID";
    
    			$sql->query("$sqls");
    			$sql->ReadRow();
    			$ca_proto_name	=$sql->RowData[0];
    			$ca_proto_kind	=$sql->RowData[1];
    			$ca_proto_id	=$sql->RowData[2];
    			
    			echo "<a name=\"$concprot_id\"></a>\n";
    			echo "<table bgcolor=#DFDFDF><tr>\n";
    			echo "<td> <img src=\"images/but.go13.gif\"> <font color=gray>".$step_no.".</font>";
    			echo " <a href=\"edit.tmpl.php?t=CONCRETE_PROTO&id=".$concprot_id."\"><B>".$ca_proto_name."</B></a> </td></tr>";
    			echo "</table>\n";
    			echo "<blockquote>\n";
    			$this->showAll( $sql, $sql2, $concprot_id, $ca_proto_id );
    			echo "</blockquote>\n";
    			echo "<br>";
    		}
    
    	}
    	
    
    }

}

