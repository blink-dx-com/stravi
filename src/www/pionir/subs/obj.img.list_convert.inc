<?php
/*MODULE: obj.img.list_convert.inc
  DESCR: - convert images
  	     - mime-type MUST be set
  AUTHOR: qbi

  VERSION: 0.1 - 20040720
*/
require_once ('func_form.inc');
require_once ('f.update.inc');

function this_form0 ($parx, $targetArrIni) {
	

	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Select parameters";
	$initarr["submittitle"] = "Prepare &gt;&gt;";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;

	$formobj = new formc($initarr, $hiddenarr, 0);
	$fieldx = array ( "title" => "mime type", "name"  => "mime_type",
			"object" => "text", "val"   => $parx["mime_type"],
			"notes" => "Convert this Mime type (e.g. image/tiff)" );
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ( "title" => "target type", "name"  => "targettype",
			"object" => "select", "val" => $parx["targettype"],
			"notes" => "Convert to JPG, PNG, BMP ...", "inits" => $targetArrIni );
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );
}

function this_form1 ($parx, $notesarr) {
	

	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Select parameters";
	$initarr["submittitle"] = "Convert now &gt;&gt;";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	//$hiddenarr["mime_type"] = $parx["mime_type"];
	$formobj = new formc($initarr, $hiddenarr, 1);

	$fieldx = array ( "title" => "mime type", "name"  => "mime_type",
			"object" => "info2", "val"   => $parx["mime_type"],
			"notes" => "Convert this Mime type (e.g. image/tiff)" );
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ( "title" => "target type", "name"  => "targettype",
			"object" => "info2", "val"   => $parx["targettype"],
			"notes" => $notesarr["targettype"] );
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );
}

function this_convertImage( &$sql, $img_id, $filename, 
	$copt = NULL // "outtype" : "JPG, "PNG", ...
				 // "params"  : conversion params like "-quality 75"
				 // "destMime": destination mime type e.g.  "image/jpeg"
	) {
	// FUNCTION: JPG compression

	global  $error;

	$params = $copt["params"];
	
	// identify get the MIME_TYPE: identify -format "%m"  FILENAME
	$gostr= '"'.$_SESSION['globals']['img_convert']. '"  '.$params.' "'. $filename .'" '.
			$copt["outtype"].':"'. $filename. '"' ;
	$answer =  system ( $gostr , $retval);
	
	if ($retval>0) {
		if ( $_SESSION['userGlob']["g.debugLevel"]>1 ) echo "DEBUG: SYS_CALL: $gostr<br>\n";
		$error->set("convertImage",1, "Error during conversion [Code:$retval]");
		return;
	}
	
	$argu = NULL;
	$argu["IMG_ID"]    = $img_id;
	$argu["MIME_TYPE"] = $copt["destMime"];
	gObjUpdate::update_row($sql, "IMG", $argu);

	return ($retval);
}

function this_tabOut($key, $val) {
	 echo "<tr><td><font color=gray>$key: </font></td><td>&nbsp;<B>".$val."</B></td></tr>\n";
}

function this_summary( &$infox ) {
	echo "<br>";
    $iopt = NULL;
    $iopt["width"] = "300";

    htmlInfoBox( "Summary", "", "open", "INFO", $iopt );

    echo "<table cellpadding=0 cellspacing=0 border=0>";

	this_tabOut("images", $infox["images"]);
	$sizemb = $infox["size"]*0.000001 ;
	this_tabOut("total size", $sizemb." MBytes");

    echo "</td></tr></table>\n";
    htmlInfoBox( "", "", "close" );
}
