<?php
/*MODULE:  o.EXP.imgRawMan.inc
  DESCR:   experiment : rawData Manipulator
  AUTHOR:  qbi
  INPUT:   $id   (e.g. ARRAY_LAYOUT_ID)
  VERSION: 0.1 - 20020904
*/

require_once("glob.image.inc");
require_once("o.IMG.c_series.inc");

class oExpImgRawDataMan {

function __construct() {
	$this->dimOpt["dimx"] = 150;
	$this->dimOpt["dimy"] = 150;
}

function expinit(&$sql, $expid) {
	global $error, $varcol;
	$FUNCNAME = "oExpImgRawDataMan:expinit";
	
	$this->expid = $expid;
	
	$o_rights = access_check( $sql, "EXP", $expid );

	if (!$o_rights["insert"]) {
		$error->set( $FUNCNAME, 1, "no right to insert data into experiment.");
		return;
	} 
	
	$sqls = "select IMG_ID from IMG_OF_EXP where EXP_ID=".$expid;
	$sql->query($sqls);
	$sql->ReadRow();
	$imgid = $sql->RowData[0];
	if ( !$imgid ) {
		$error->set( $FUNCNAME, 3, "experiment has no image");
		return;
	}
	$sqls = "select NAME, extra_obj_id from IMG where IMG_ID=".$imgid;
	$sql->query($sqls);
	$sql->ReadRow();
	$img_body = $sql->RowData[0];
	$extra_obj_id = $sql->RowData[1];
	
	if ( !$extra_obj_id ) {
		$error->set( $FUNCNAME, 4, "image has no class");
		return;
	}
	
	$values_tmp = $varcol->select_by_name ($extra_obj_id);
	$values     = &$values_tmp["values"];
	
	list ($tmpret, $errtxt, $this->image_arr) = image_series_get( $img_body, $values_tmp);
	if ($tmpret<0) {
		$error->set( $FUNCNAME, 2, "problems on getting series:".$errtxt);
		return;
	}
}

function _getRawCount(&$sql, $channel_id) {
	$sqls = "select count(spot_id) from EXP_RAW_RESULT  where EXP_ID=".$this->expid." AND CHANNEL_ID=".$channel_id;
	$sql->query($sqls);
	$sql->ReadRow();
	$spotcnt = $sql->RowData[0];
	return ($spotcnt);
}

function doDelete( &$sql, $sel ) {
	
	echo "<b><img src=\"".$_SESSION['s_sessVars']["loginURL"]."/images/ic.del.gif\"> Delete RawResults of selected Channels now!</b><br><br>";
	touch_row($sql, "EXP", $this->expid);
	
	if ( !sizeof($sel)) {
		echo "Info: No channels selected!<br>\n";
		return;
	} 
	
	$cnt=0;
	while ( list($channel_id, $val) = each($sel) ) {
		if (!$val) continue;
		$retval = $sql->Deletex ( "EXP_RAW_RESULT", "EXP_ID=".$this->expid." AND CHANNEL_ID=".$channel_id );
		echo "Channel_ID: <b>".$channel_id."</B> : deleted.<br>\n";
		$cnt++;
	}
	reset ($sel);
	echo "<br>";
	echo "Ready: <b>$cnt</b> RawResults-channels deleted.<br>";
}


function showForm( &$sql, $url ) {
	/* OUTPUT: form-params:
					$go
					$sel[channel_id] = 1
	*/
 	$img_cnt = 1;
	$dimen_para = "&dim[0]=".$this->dimOpt["dimx"]."&dim[1]=".$this->dimOpt["dimy"];
	$optpar  = "&useDefParams=1";
	
	// /images/ic.del.gif
	
	echo "<form style=\"display:inline;\" method=\"post\"  name=\"editform\"  action=\"".$url."\" >\n";
	
	// SUBMIT
	htmlInfoBox( "<img src=\"images/ic.del.gif\"> Delete RawResults of selected images", "", "open", "INFO" );
	echo "<center><br><input type=submit value=\"Delete now!\"></center><br>\n";
	htmlInfoBox( "", "", "close" );
	echo "<br>";
	
	
	$img_cnt = 0;
	echo "<table><tr bgcolor=#EFEFEF>";
	$tabkeys = array( "image","channel_id", "RawResult entries" );
	while ( list($key, $val) = each($tabkeys) ) {
		echo "<td><b>&nbsp;".$val."&nbsp;</b></td>";
	}
	reset ($tabkeys);
	 echo "</tr>";
	
	
	while ( list(, $tmparr) = each($this->image_arr) ) {
	
		$channel_id = $img_cnt+1;
		$tmpimg   =  $tmparr[0];
		$time_offs=  $tmparr[1];
		
		$spotcnt = $this->_getRawCount($sql, $channel_id);

		$tmp_url = str_replace("\\", "/", $tmpimg);
		$shortname=  basename ($tmp_url);

		$tmpimg_enc = urlencode($tmpimg);
		echo "<tr><td>";
		echo "<img src=\"f.image_show.php?filename=".$tmpimg_enc. $dimen_para.$optpar."\" border=0></td>";
		echo "<td>";
		echo "&nbsp;<input type=checkbox name='sel[".$channel_id."]' value='1'>\n";
		echo " ".$channel_id. "</td>";
		echo "<td>$spotcnt</td>";
		// echo "<a href=\"".$url_pre.$tmpimg."\" target=_new>".$shortname."</a>";
		echo "</tr>\n";
		$img_cnt++;
	}
	reset ($this->image_arr);
	echo "</table>\n";
	echo "<input type=hidden name='go' value='1'>\n";
	echo "</form>\n";
}

}