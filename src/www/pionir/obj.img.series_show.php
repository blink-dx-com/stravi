<?php
/**
 * Show list of images of series
 * GLOBALS: 	$_SESSION['userGlob']["o.IMG.series_show"]= serialize(dimx, dimy, normalize=>0,1)
 * @package obj.img.series_show.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $id (image_id )
         $showimages
		 $opt[zoom] 0 : 150x100
		 			1 : 300x200
					...
		 $opt[normalize]
		 $go= 1 : take options from form
 
 */

session_start(); 


require_once("o.LINK.subs.inc");

require_once("db_access.inc");
require_once("globals.inc");
require_once("access_check.inc");
require_once("func_head.inc");
require_once("varcols.inc");
require_once("glob.image.inc");
require_once("o.IMG.c_series.inc");

function tmp_info_out($col, $info) {
	echo "<tr><td><font color=gray>".$col."</font></td><td>".$info."</td></tr>\n";
}

$error = & ErrorHandler::get();
$varcol   = & Varcols::get();
$sql      = logon2( $_SERVER['PHP_SELF'] );

$id = $_REQUEST["id"];
$showimages = $_REQUEST["showimages"];
$opt = $_REQUEST["opt"];
$go = $_REQUEST["go"];


$retval=0;
$title = "Show image series elements";

$tablename = "IMG";
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;
$infoarr["show_name"]= 1;
$infoarr["checkid"]  = 1;
$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

$dimOpt=array();

if ( $showimages ) {
	$serialOpt = unserialize($_SESSION['userGlob']["o.IMG.series_show"]);

	if ( $go ) {
		$serialOpt["zoom"] = $opt["zoom"];
	}
	
	if ($serialOpt["zoom"]<=0) $serialOpt["zoom"]=0;
	if ($serialOpt["zoom"]>3)  $serialOpt["zoom"]=3;
	
	$dimOpt["dimx"] = 150*($serialOpt["zoom"]+1);
	$dimOpt["dimy"] = 100*($serialOpt["zoom"]+1);
	
	$serialOpt["normalize"]=$opt["normalize"];
	$_SESSION['userGlob']["o.IMG.series_show"] = serialize($serialOpt);
}

echo "<ul>";


$o_rights = access_check( $sql, "IMG", $id );

if (!$o_rights["read"]) {
	echo "ERROR: no right to read.<br>";
	return (-1);
}        

if ( !$showimages ) {
    echo "[<a href=\"".$_SERVER['PHP_SELF']."?id=$id&showimages=1\">Try to show image thumbnails</a>]<br>\n";
}

$sqls= "select NAME, EXTRA_OBJ_ID from IMG where IMG_ID=".$id;
$sql->query("$sqls");
if ( $sql->ReadRow() ) {

	$img_body=$sql->RowData[0];
	$extra_obj_id=$sql->RowData[1];
}

echo "<table cellspacing=1 cellpadding=1 border=0>\n";

if (!$img_body) {
	echo "ERROR: no image name body. Stopped.<br>";
	return (-1);
} 


if ( $extra_obj_id ) {    
	$values_tmp = $varcol->select_by_name ($extra_obj_id);
	$values     = &$values_tmp["values"];
	$info_fileNotExists=0;
	
	// analyse first image
	$net_filename = oIMGseriesC::getFilenameOfImage($img_body, $values, 1);
	$server_filename = netfile2serverpath( $net_filename );
	if ( !file_exists($server_filename) ) {
		$info_fileNotExists=1;
	}
	  	  
	$checkit=array(); 
	$checkit[]="start-number";
	$checkit[]="number";
	$checkit[]="name-extension";

	tmp_info_out("Image name body",$img_body);
	
	while ($th=each ($checkit) ) {
		$indexer = $th[1];
		$this_val = $values[$indexer];

		if ($this_val =="") {
	  	      tmp_info_out($indexer, "<B>ERROR:</B> need a value !");
		      $retval=-1;
		} else {
	  	      tmp_info_out($indexer, $this_val);
		}
	}
	tmp_info_out("offset-time", $values["offset-time"]. " sec");
    tmp_info_out("time-step", $values["time-step"]. " sec"); 
    
	$serverurl = netfile2serverpath($img_body);
    tmp_info_out("server-url", "'".$serverurl."'");
	
	if ($info_fileNotExists) {
		 tmp_info_out("first image file", "<font color=red><b>First image not found by server!</b></font>");
	}
	
	
	 
}
echo "</table>";  

list ($tmpret, $errtxt, $image_arr) = image_series_get( $img_body, $values_tmp);
if ($tmpret<0) {
    echo "<B>ERROR:</B> ".$errtxt. "<br>\n";
    return (-1);
}

$url_pre="";
if ( !strstr($img_body, "://" ) ) {
	$url_pre="file://";
}

echo "<br>";

if ( !$retval ) {
	
    if ( !$showimages ) {
        $img_cnt = 1;
        foreach( $image_arr as $dummy=>$tmparr) {
                $tmpimg   =  $tmparr[0];
                $time_offs=  $tmparr[1];
                echo "<LI> $img_cnt: ";
                echo "<a href=\"".$url_pre.$tmpimg."\" target=_new>".$tmpimg."</a>";
                echo " &nbsp;&nbsp;<font color=gray>time-offset:</font> ".$time_offs." [sec]\n" ;
                $img_cnt++;
        }
    } else {

		echo "<form style=\"display:inline;\" method=\"post\"  name=\"form1\"  action=\"".$_SERVER['PHP_SELF']."\" >\n";
		echo "<input type=hidden name='showimages' value='1'>\n";
		echo "<input type=hidden name='id' value='".$id."'>\n";
		echo "<input type=hidden name='go' value='1'>\n";
		
		echo "<font color=gray><b>Show thumbnails</b>&nbsp;&nbsp;&nbsp;Zoom:</font> \n";
		
		for ( $i=0; $i<4; $i++ ) {
			$tmpsel="";
			if ($i==$serialOpt["zoom"]) $tmpsel=" checked";
			echo "<input type=radio name=opt[zoom] $tmpsel value=\"".$i."\">".($i+1)." \n";
		} 
	
		echo " <input type=submit value=\"Show\">\n";
		echo "</form><br><br>\n";
	
        $img_cnt = 1;
        $dimen_para = "&dim[0]=".$dimOpt["dimx"]."&dim[1]=".$dimOpt["dimy"];
		$optpar  = "&useDefParams=1";
		
        foreach( $image_arr as $dummy=>$tmparr) {
            $tmpimg   =  $tmparr[0];
            $time_offs=  $tmparr[1];

            $tmp_url = str_replace("\\", "/", $tmpimg);
            $shortname=  basename ($tmp_url);

            $tmpimg_enc = urlencode($tmpimg);
            echo "<img src=\"f.image_show.php?filename=".$tmpimg_enc. $dimen_para.$optpar."\" border=0>";
            echo " ".$img_cnt. ": ";
            echo "<a href=\"".$url_pre.$tmpimg."\" target=_new>".$shortname."</a>";
            if ($time_offs!="") echo " &nbsp;&nbsp;<font color=gray>time-offset:</font> ".$time_offs." [sec]" ;
			echo "<br>\n";
            $img_cnt++;
        }
    }
}

echo "<br><br>";
if ( $time_offs>0 ) echo "<br><br><font color=gray>last image time-offset:</font>  ".($time_offs/60.0)." [min]";
echo "<hr>";
htmlFoot();
