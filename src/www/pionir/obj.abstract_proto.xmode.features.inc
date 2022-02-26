<?php
/**
 * extension for abstract protocol   
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   SREQ:0001801: o.ABSTRACT_PROTO > abstract protocol anzeigen/editieren 
 */
require_once ('o.S_VARIO.subs.inc');
require_once ('f.msgboxes.inc'); 
require_once ("f.objview.inc");
require_once('subs/obj.abstract_proto.guisub.inc');

class obj_edit_ext extends gObj_edit_feat_ABS { 
    
    function step_table1($sql, $sql2) {
        
        $id = $this->objid;
        $editAllow = 0; // changed on 2019-01-28: activated the new Editor: obj.abstract_proto.xmode.edi.php
        $showCheckBoxes=0;
        if ($this->viewmode == "edit" ) {
            $showCheckBoxes=1;
        }
        
        $options = array("showStepLnk"=>1);
        $proto_step_c = new  protostep( $sql, $sql2, $id, $editAllow, $showCheckBoxes, 0, $options  );
        
        ?>
    		<form name="xtraform" ACTION="obj.abstract_proto.stepact.php?id=<?echo $id?>" METHOD=POST> 
    	<?
    				
		$proto_step_c->tableOut($sql);
		
		if ($showCheckBoxes) {
    		$feld = NULL;
    		$feld["copy"]="copy";
    		if ( $editAllow  ){
    			$feld["del"]  ="delete";
    			$feld["paste"]="paste";
    		}
    		$preselected = "";
    		$seloption = array("selecttext"=>"-- action --");
    		$seltext   = formc::selectFget( "act", $feld, $preselected, $seloption);
    	
    		echo "&nbsp;&nbsp;".$seltext." ";
    		echo "<input type=submit value=\"Submit\">";
		}
		
		
		echo "</form>\n";
    }
    
    function rightWindow( &$sqlo, &$sqlo2) {
        global  $error; 
        
        $id = $this->id;
	    $argu     = &$this->argu;
	    $viewmode = $this->viewmode;
	    $o_rights = $this->o_rights;
	   
    	require_once ( "func_form.inc");
    	require_once ( "gui/o.PROTO.stepout1.inc");
    	
    	$this->objid=$id;
    	$editAllow = $o_rights["write"];
    	if ($viewmode!= "edit" ) $editAllow = 0;
    	
    	$this->viewmode = $viewmode;
    	
    	?>
    	<script>
      	<!--
      	    function a_gostep( id, step) {
    		url_name="edit.tmpl.php?tablename=ABSTRACT_PROTO_STEP";
    		location.href=url_name+ "&idname=ABSTRACT_PROTO_ID&id=" + id + "&primas[1]=STEP_NR&primasid[1]="+step;	
    	    }
    
    	    
    	//-->
    	</script>
    	<?php
    	
    	echo '<div style="margin-left:10px;">'."\n";
    	
    	//$helplib = new oABSTRACT_PROTO_guisub($id);
    	//$helplib->object_tracking($sql);
    	
    	// OLD,FUTURE ? check, if an alternative Editor is active ....
        //     	$variolib      = new oS_VARIO_sub('ABSTRACT_PROTO');
        //     	$pluginAddress = $variolib->getValByKey($sql, $id, 'AlterEditor');
        //     	if ($pluginAddress=='html_attachment') {
        //     		// alternative Editor
        //     		echo '<br><span class="yGgray">INFO: This protocol is marked as "Alternative Editor" (based on attached HTML-Form).</span><br>'.
        //     			'It shows the form as it appears in the "protocol (concrete)". You can NOT submit data for the form here. '."\n";
            	
        //     		require_once('gui/o.PRC.altereditor1.inc');
        //     		$edFormLib = new oPRC_altereditor1($sql);
        //     		$edFormLib->set_PRA($sql, $id);
        //     		if ($error->printAll())  {
        //     			return;
        //     		}
        //     		echo '<hr>'."\n";
        //     		$edFormLib->showform( $sql, $sql2, 2 ); // just show the empty form
        //     		$error->printAll();
        //     		return; // do not show any other options ...
        //     	}
    	
    	$filex = $_SESSION['s_sessVars']['AppLabLibDir'].'/lablib/bASSAY/analyseC.inc';
    	if (file_exists($filex)) {
    	   require_once  ( $filex );
    	   
    	   if ( bASSAY\analyseC::has_assay_file($sqlo, $id) ) {
    	       require_once  ( $_SESSION['s_sessVars']['AppLabLibDir'].'/lablib/bASSAY/gui_analyse.inc' );
    	       
    	       try {
        	       $assay_lib = new bASSAY\Gui_analyse($sqlo, $id);
        	       $assay_lib->show_main_blocks();
    	       } catch (Exception $e) {
    	           cMsgbox::showBox("error", "Error ocurred".$e->getMessage());
    	       }
    	   }
    	       
    	}
    	
        
    	if ($editAllow) { 
            echo "Protocol steps: ";
            
    		// leave action-link here, because: is shown even if NO steps are defined
            echo "[<a href=\"obj.abstract_proto.stepact.php?id=".$id."&act=paste\">".
    				"Paste arguments & steps from clipboard</a>]";
    		echo "<br>\n";
    	} else {
    	    echo "<br>\n";
    	}
    	
    
    	$sqls= "select count(*) from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$id;
        $sqlo->query("$sqls");
     	$sqlo->ReadRow();
     	$steps=$sqlo->RowData[0];
    	if ( !$steps ) {
    		echo "<font color=gray>No steps defined</font><br>\n";
    	} else {
    	    
    	    if ($argu['LAYOUT']!='') {
    	        require_once('gui/o.ABSTRACT_PROTO.layout_sh.inc');
    	        $helplib = new o_ABSTRACT_PROTO_layout_sh($sqlo, $id, $argu['LAYOUT']);
    	        $helplib->show($sqlo);
    	        
    	    } else {
    	       $this->step_table1($sqlo, $sqlo2);
    	    }
    	}
    	
    	echo '</div>'."\n";
    	
    	
    	return 0;
    }

}
