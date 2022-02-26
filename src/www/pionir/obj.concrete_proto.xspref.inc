<?php

/*MODULE: obj.concrete_proto.xspref.inc
  DESCR: contains function for single object preferences
  AUTHOR: Qbi
*/
class o_CONCRETE_PROTO_XSPREF  extends o_xspref_ABS {
    
    function singlePref( &$sql, $info ) {

    	$tmpShowOpt = unserialize($_SESSION['userGlob']["o.CONCRETE_PROTO.show"]);
        //  "show_st_notes"
        //  "show_diff"
    	//  "shNoteSz"  size of input fields for proto_step NOTES
    	
    	prefNameShow( "Mark mandatory step fields" );
    	$vartmp = "showRequ";
    	$infotmp= $tmpShowOpt[$vartmp];
    	$checked_tmp="";
    	if ($infotmp) $checked_tmp="checked";
    	echo "<td><input type=checkbox name=\"setti[o.CONCRETE_PROTO.show.".$vartmp."]\" value=\"1\" ".$checked_tmp."></td>";
    	prefClose ("In the Protocol-steps: mark quantity, substance and other fields, which have to be filled.");
    	
    	prefNameShow( "Show planned protocol information" );
    	$vartmp = "abstrStepsShow";
    	$infotmp= $tmpShowOpt[$vartmp];
    	$checked_tmp="";
    	if ($infotmp) $checked_tmp="checked";
    	echo "<td><input type=checkbox name=\"setti[o.CONCRETE_PROTO.show.".$vartmp."]\" value=\"1\" ".$checked_tmp."></td>";
    	prefClose ("Unselect will show a more compact protocol information."); 
    	
        prefNameShow( "Hide inactive steps" );
    	$vartmp = "inactStepsNotShow";
    	$infotmp= $tmpShowOpt[$vartmp];
    	$checked_tmp="";
    	if ($infotmp) $checked_tmp="checked";
    	echo "<td><input type=checkbox name=\"setti[o.CONCRETE_PROTO.show.".$vartmp."]\" value=\"1\" ".$checked_tmp."></td>";
    	prefClose ("Check this to show only active steps.");
    	
    	prefNameShow( "Edit &gt; Notes &gt; size" );
    	$vartmp = "shNoteSz";
    	$infotmp= $tmpShowOpt[$vartmp];
    	echo "<td><input type=input name=\"setti[o.CONCRETE_PROTO.show.".$vartmp."]\" value=\"".$infotmp."\" size=3></td>";
    	prefClose ("Size of notes field in proto step editor [default: 22]");
        
        prefNameShow( "Output optimization" );
    	$vartmp = "o.PROTO.outmode";
    	$infotmp= $_SESSION['userGlob']["$vartmp"];
    	
        $sel1="";
    	if ( ($infotmp=="") || ($infotmp=="display") ) $sel1="selected";
    	if ( ($infotmp=="printer") )   $sel2="selected";
        echo "<td><select name=\"setti[$vartmp]\">";
    	echo "<option ".$sel1." value=\"\">display";
    	echo "<option ".$sel2."	value=\"printer\">printer";
    	echo "</select></td>";  
    	prefClose ("Display: no wrapping of lines.<br>Printer: allow wrapping of lines."); 
        
        echo "<tr><td colspan=3 bgcolor=#FFFFFF>&nbsp;</td></tr>";
        
    	 
    }
    
    function prefSave( &$sql, $info, $setti) {  
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $tmpShowOpt = NULL;
    	
    	$flagname="abstrStepsShow";
    	$tmp_val = $setti["o.CONCRETE_PROTO.show.".$flagname] ;
    	if ( $tmp_val != "" ) $tmpShowOpt[$flagname] = $tmp_val;
    
    	
    	$flagname="showRequ";
    	$tmp_val = $setti["o.CONCRETE_PROTO.show.".$flagname] ;
    	if ( $tmp_val != "" ) $tmpShowOpt[$flagname] = $tmp_val;
        
        $flagname="inactStepsNotShow";
    	$tmp_val = $setti["o.CONCRETE_PROTO.show.".$flagname] ;
    	if ( $tmp_val != "" ) $tmpShowOpt[$flagname] = $tmp_val;
    	
    	$flagname="shNoteSz";
    	$tmp_val = $setti["o.CONCRETE_PROTO.show.".$flagname];
    	unset ($tmpShowOpt[$flagname]); 
    	if ( $tmp_val != "" ) $tmpShowOpt[$flagname] = $tmp_val;
    	
        $flagname="o.PROTO.outmode";
    	$tmp_val = $setti[$flagname] ;
    	$_SESSION['userGlob'][$flagname] = $tmp_val;
 
    	$_SESSION['userGlob']["o.CONCRETE_PROTO.show"]    = serialize($tmpShowOpt);
    	
    	if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
    	    debugOut('(91) userGlob][o.CONCRETE_PROTO.show]: '. $_SESSION['userGlob']['o.CONCRETE_PROTO.show']  , $FUNCNAME,1);
    	}
    }
}