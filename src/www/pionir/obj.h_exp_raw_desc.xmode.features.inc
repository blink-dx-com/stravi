<?
require_once("f.help.inc");

class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
        $argu  = &$this->argu;

    	$helpLib = new fHelpC();
    	$namex   = $argu["NAME"]; 
    	?>
     	<ul>
    	<img src="images/greenball.gif"> <a href="obj.h_exp_raw_desc.import.php?id=<?echo $id?>">Import columns</a><BR>
    	<?
    	$helpUrl = $helpLib->getTableEntryHelp( "H_EXP_RAW_DESC", $namex );
    	if ($helpUrl!="") {
    		echo "<img src=\"images/greenball.gif\"> <a href=\"".$helpUrl."\" target=_help>Help for this type</a><BR>\n";
    	} else {
    		echo "<img src=\"images/greenball.gif\"> <font color=gray>No help for this type</font><BR>\n";
    	}
    	echo "</ul>\n";
    
    	
    	return 0;
    }

}
