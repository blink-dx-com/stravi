<?php
class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
        $o_rights = $this->o_rights;
        $viewmode = $this->viewmode;

    
        require_once ("gui/o.CONCRETE_PROTO.edit.inc");
        
        echo '<div style="margin-left:10px;">'."\n";
        
    	echo "<B><img src=\"images/greenball.gif\"> Protocol steps</B>";
    	
    	$sqlSubst = "x.CONCRETE_SUBST_ID IN ".
    			"(select x.CONCRETE_SUBST_ID from CONCRETE_PROTO_STEP x WHERE (x.CONCRETE_PROTO_ID = ".$id.") )";
    	$sqlUrl = urlencode($sqlSubst);
    	echo  "&nbsp;&nbsp;&nbsp;[<a href=\"view.tmpl.php?t=CONCRETE_SUBST&condclean=1&tableSCond=".$sqlUrl."\">select substances</a>]";
    	echo "<br>\n";
    	
    	$editAllow  = $o_rights["write"];
    	$viewmode   = $_SESSION['s_sessVars']['o.CONCRETE_PROTO.editmode'];
        if ($viewmode!= "edit" ) $editAllow = 0;
    	
    	$edFormLib = new oConcProtoEdform($sqlo, $id);
    	$edFormLib->showform( $sqlo, $editAllow );
    	echo "</div\n";
    	
    	return 0;
    }

}
