<?php

/**
 * 
 * @desc contains function for single object preferences
 * @author Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ: 0001413: p.098 -> (HIV-NAT specific) Result View TimeDia
 */
class o_EXP_XSPREF  extends o_xspref_ABS {
    


function singlePref( &$sql, $info) {
    
    
	require_once ("o.EXP.RESULT.inc");
	require_once ("class.history.inc");
    
    $hist_obj = new historyc();
    $tmpexpid = $hist_obj -> last_bo_get("EXP");
	$spotResLib = new gSpotResC();
	
    prefEmptLine();
	
	echo "<tr><td colspan=3 bgcolor=#EFEFEF><img src=\"images/i20.expres_gen.gif\" style=\"vertical-align:middle; margin:2px\"> <B>Result visualization : main settings</B> </td></tr>";
    
	prefNameShow( "Default result type" );
	$vartmp = "f.spot_result.table";
	$infotmp= $_SESSION['userGlob']["$vartmp"];
	$sel1="";
	
	if ( ($infotmp=="raw") )   $sel1="selected";
	if ( ($infotmp=="") || ($infotmp=="normalized") ) $sel2="selected";
	
	echo "<td><select name=\"setti[".$vartmp."]\">";
	echo "<option ".$sel1." value=\"raw\">RAW";
	echo "<option ".$sel2."	value=\"normalized\">NORMALIZED";
	echo "</select></td>";
	
	prefClose ("Choose result type for actions.");

	prefInfoRow( "Main visualization-column of result-tables:", 1);
		
	prefNameShow( "<img src=\"images/icon.SPOT_RESULT.gif\" hspace=10> <b>spot results</b>" );
	$vartmp = "f.spot_result.norm.colm";
	$infotmp= $_SESSION['userGlob']["$vartmp"];
    echo "<td>";
    $spotResLib->print_select_box( $sql, "setti[".$vartmp."]", $infotmp, $tmpexpid, "normalized");
	echo "</td>";
	prefClose ("Default visualization column for 'normalized results'.");
	
	prefNameShow( "<img src=\"images/icon.EXP_RAW_RESULT.gif\" hspace=10> raw results" );
	$vartmp = "f.spot_result.raw.colm";
	$infotmp= $_SESSION['userGlob'][$vartmp];
    echo "<td>";
    $spotResLib->print_select_box( $sql, "setti[".$vartmp."]", $infotmp, $tmpexpid, "raw");
	echo "</td>";
	prefClose ("Default visualization column for 'raw results'.");
	
	prefNameShow( "<img src=\"images/icon.EXP_CHAN_RES.gif\" hspace=10> channel results" );
	$vartmp = "f.spot_result.chan.colm";
	$infotmp= $_SESSION['userGlob'][$vartmp];
    echo "<td>";
    $spotResLib->print_select_box( $sql, "setti[".$vartmp."]", $infotmp, $tmpexpid, "chan");
	echo "</td>";
	prefClose ("Default visualization column for 'channel results'.");
    
	prefEmptLine();
	echo "<tr><td colspan=3 bgcolor=#EFEFEF><img src=\"images/but.expres2d.gif\" style=\"vertical-align:middle; margin:2px\"> <B>Result visualization : matrix graphics</B> </td></tr>";
	
	//
	// 2D parameters
	//
	$mainOptName = "f.spot_res2Dpref";
	$tmpOpt 	 = unserialize($_SESSION['userGlob'][$mainOptName]);
	prefNameShow( "Array background" );
	$vartmp  = "mxbgk";
	$infotmp = $tmpOpt[$vartmp];
	$sel="";
	if ( $infotmp=="black" ) $sel="selected";
	echo "<td><select name=\"setti[$mainOptName.$vartmp]\">";
	echo "<option value=\"\">white";
	echo "<option ".$sel."	value=\"black\">black";
	echo "</select></td>";
	prefClose ("Select the background color of the array result data images.");
	
	prefNameShow( "Array color mode" );
	$vartmp = "mx_color_mode";
	$infotmp= $tmpOpt[$vartmp];
	$sel1="";
	$sel2="";
	if ( $infotmp=="bw" ) 	   $sel1="selected";
	if ( $infotmp=="grayinv" ) $sel2="selected";
	echo "<td><select name=\"setti[$mainOptName.$vartmp]\">";
	echo "<option value=\"\">color";
	echo "<option ".$sel1."	value=\"bw\">gray";
	echo "<option ".$sel2."	value=\"grayinv\">inverse gray"; 
	echo "</select></td>";
	prefClose ("Select the color mode of the array result data images."); 
	
	prefNameShow( "Gamma factor for colors" );
	$vartmp = "mx_gamma";
	$infotmp= $tmpOpt[$vartmp];
	echo "<td><input name=\"setti[$mainOptName.$vartmp]\" value=\"".$infotmp."\" size=5>";
	echo "</td>";
	prefClose ("Gamma factor for value to color transformation [0.1 ... 10]"); 
    
	prefNameShow( "Fixed Scale values" );
	$vartmp = "mx_minval";
	$infotmp= $tmpOpt[$vartmp];
	echo "<td>Min: <input name=\"setti[$mainOptName.$vartmp]\" value=\"".$infotmp."\" size=5>";
	$vartmp = "mx_maxval";
	$infotmp= $tmpOpt[$vartmp];
	echo "<br>Max: <input name=\"setti[$mainOptName.$vartmp]\" value=\"".$infotmp."\" size=5>";
	echo "</td>";
	prefClose ("Scale min value; typical: Min:0, Max:1.0; EMPTY: automatic scaling");
	
	//
	// Timedia preferences
	//
	prefEmptLine();
	prefInfoRow("<img src=\"images/i20.exptime.gif\" style=\"vertical-align:middle; margin:2px\"> Result visualization : timedia");
	
	prefNameShow( "Show CT values" );
	$tmpOpt = unserialize($_SESSION['userGlob']["o.EXP.time"]);
	$vartmp = "tmpOpt_showCT";
	$checked_tmp="";
	if ( $tmpOpt["showCT"]) $checked_tmp="checked";
	echo "<td><input type=checkbox name=\"setti[$vartmp]\" value=\"1\" $checked_tmp> show CT";
	echo "</td>\n";
	prefClose ("default: show CT values [ON]");
	
	//
	// MISC
	//
	prefEmptLine();
	prefInfoRow("<img src=\"images/i20.expmisc.gif\" style=\"vertical-align:middle; margin:2px\"> Miscalleanous");
	
	prefNameShow( "Show images of series" );
	$tmpOpt = unserialize($_SESSION['userGlob']["o.EXP.singFormOpt"]);
	$vartmp = "tmpOpt_shAlImg";
	$checked_tmp="";
	if ( $tmpOpt["shAlImg"]) $checked_tmp="checked";
	echo "<td><input type=checkbox name=\"setti[$vartmp]\" value=\"1\" $checked_tmp> show all";
	echo "</td>\n";
	prefClose ("default: show only first image");
	
    prefNameShow( "Iconoclust parameters" );
	$vartmp = "f.iconoclust.params";
	$infotmp= $_SESSION['userGlob']["$vartmp"];
	echo "<td><input name=\"setti[$vartmp]\" value=\"".$infotmp."\">";
	echo "</td>";
	prefClose ("[-o] use default script"); 
	
	
	prefNameShow( "Select bioarray (concrete) from" );
	
	$vartmp = "o.EXP.CONCRETE_ARRAY_ID.sel";
	$infotmp= $_SESSION['userGlob']["$vartmp"];
	$sel1="";
	
	if ( ($infotmp=="") || ($infotmp=="CONCRETE_ARRAY") ) $sel1="selected";
	if ( ($infotmp=="W_CHIP_FROM_WAFER") )   $sel2="selected";
	
	$nicetmp  = tablename_nice2('CONCRETE_ARRAY');
	$nicetmp2 = tablename_nice2('W_CHIP_FROM_WAFER');
	
	echo "<td><select name=\"setti[o.EXP.CONCRETE_ARRAY_ID.sel]\">";
	echo "<option ".$sel1." value=\"\">".$nicetmp;
	echo "<option ".$sel2."	value=\"W_CHIP_FROM_WAFER\">".$nicetmp2;
	echo "</select></td>";
	prefClose ("Choose the list, where to select the object from."); 
	
}

function prefSave( &$sql, $info, $setti) {
    
    
	$flagname="o.EXP.CONCRETE_ARRAY_ID.sel";
	$_SESSION['userGlob'][$flagname] = "";
	$tmp_val = $setti[$flagname] ;
	if ( $tmp_val != "" ) $_SESSION['userGlob'][$flagname] = $tmp_val;
	
	$flagname="f.spot_result.table";
	$_SESSION['userGlob'][$flagname] = "";
	$tmp_val = $setti[$flagname] ;
	$_SESSION['userGlob'][$flagname] = $tmp_val;
    
    $flagname = "f.spot_result.norm.colm";
    $_SESSION['userGlob'][$flagname] = "";
	$tmp_val = $setti[$flagname] ;
    $_SESSION['userGlob'][$flagname] = $tmp_val;
	
	$flagname = "f.spot_result.raw.colm";
    $_SESSION['userGlob'][$flagname] = "";
	$tmp_val = $setti[$flagname] ;
    $_SESSION['userGlob'][$flagname] = $tmp_val;
	
	$flagname = "f.spot_result.chan.colm";
    $_SESSION['userGlob'][$flagname] = "";
	$tmp_val = $setti[$flagname] ;
    $_SESSION['userGlob'][$flagname] = $tmp_val;
	
    //
	// 2D parameters
	//
	
	$mainOptName = "f.spot_res2Dpref";
	$tmpShowOpt  = NULL;
	$flagname="mxbgk";
	$_SESSION['userGlob'][$flagname] = "";
	$tmp_val = $setti[$mainOptName.".".$flagname] ;
	if ( $tmp_val != "" ) $tmpShowOpt[$flagname] = $tmp_val;
	
	$flagname="mx_color_mode";
	$tmp_val = $setti[$mainOptName.".".$flagname] ;
	if ( $tmp_val != "" ) $tmpShowOpt[$flagname] = $tmp_val;
	
	$flagname="mx_minval";
	$tmp_val = $setti[$mainOptName.".".$flagname] ;
	if ( $tmp_val != "" ) $tmpShowOpt[$flagname] = $tmp_val;
	$flagname="mx_maxval";
	$tmp_val = $setti[$mainOptName.".".$flagname] ;
	if ( $tmp_val != "" ) $tmpShowOpt[$flagname] = $tmp_val;
	
	$flagname="mx_gamma";
	$tmp_val = $setti[$mainOptName.".".$flagname] ;
	if ( ($tmp_val >=0.1) && ($tmp_val <=10) ) $tmpShowOpt[$flagname] = $tmp_val;
	
	$_SESSION['userGlob'][$mainOptName] = serialize($tmpShowOpt);	// save it !
	
	//
	// other
	//
	$tmpShowOpt = NULL;
	$flagname="tmpOpt_shAlImg";
	$tmp_val = $setti[$flagname] ;
	if ( $tmp_val != "" ) $tmpShowOpt["shAlImg"] = 1;
	$_SESSION['userGlob']["o.EXP.singFormOpt"] = serialize($tmpShowOpt);
	
	$timeShowOpt = NULL;
	$flagname="tmpOpt_showCT";
	$tmp_val = $setti[$flagname] ;
	if ( $tmp_val != "" ) $timeShowOpt["showCT"] = 1;
	$_SESSION['userGlob']["o.EXP.time"] = serialize($timeShowOpt);
    
    $flagname="f.iconoclust.params";
	$_SESSION['userGlob'][$flagname] = "";
	$tmp_val = $setti[$flagname] ;
	if ( $tmp_val != "" ) $_SESSION['userGlob'][$flagname] = $tmp_val;
	
}

}
