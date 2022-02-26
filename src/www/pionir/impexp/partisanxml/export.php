<?php
/**
 * do the PAXML export
 * @package export.php
 * 
 * @author  Rogo, Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param  
 *   cct_table=PROJ&cct_id=123
          or
    cct_table[0] = PROJ&cct_id[0]=123&cct_table[1] = PROJ&cct_id[1]=124
              elements with same index belong together
	$cct_id
	[$asarchive]  0|1 save paxml with archive-option
	$cct_out = output level
 * @param $opt
    "notar" : 0|1 if=1 : do not produce a tar-file (to save time and disk space ... )
	"doNotTakeAttachments" :[0], 1	
 */
session_start(); 


require_once("func_head.inc");
require_once("PaXMLWriter.inc"); 
require_once("PaXML_guifunc.inc");


$error = & ErrorHandler::get();
$sqlo   = logon2( $_SERVER['PHP_SELF'] );
$opt 	= $_REQUEST['opt'];
$cct_table = $_REQUEST['cct_table'];
$cct_id    = $_REQUEST['cct_id'];
$cct_out = $_REQUEST['cct_out'];
$asarchive= $_REQUEST['asarchive'];

$flushLib = new fProgressBar( 100 ); // default 100


$title = "Paxml Export";
$infoarr = NULL;
$infoarr["help_url"] = "export_of_objects.html";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "tool";

if ($cct_table!=NULL AND $cct_id) {
	$infoarr["form_type"]= "obj";
	$infoarr["obj_name"] = $cct_table;
	$infoarr["obj_id"]   = $cct_id;
}
$infoarr["css"] = $flushLib->getCss(1);
$infoarr["javascript"] = $flushLib->getJS();

$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo, $infoarr);


if ($cct_out === null)
    $cct_out = 0.5;
    
    echo "<ul>";


	echo "<font face=Arial,Helvetica size=1 color=black>";

    $pxml = new PaXMLWriter($_SESSION['sec']['dbuser'], $_SESSION['sec']['passwd'], $_SESSION['sec']['db'], $_SESSION['sec']['_dbtype'], $cct_out);
   
    
	if ($asarchive) $pxml->setArchiveFlag();
	if ( $opt["doNotTakeAttachments"]>0 ) $pxml->set_noAttach(1);
	
	$pxml->startXML();
	$pxml->html->setProgressLib($flushLib);
	
    if (is_array($cct_table)) {
        foreach($cct_table as $i -> $table) {
        	$table_pk_name = PrimNameGet2($table);
            $pxml->start(strtoupper($table), array($table_pk_name => $cct_id[$i]));
    	}	
    } else {
    	$table_pk_name = PrimNameGet2($cct_table);
    	$pxml->start(strtoupper($cct_table), array($table_pk_name => $cct_id));
    }
    $pxml->endXML();
    if ( !$opt["notar"] ) $pxml->tar($pxml->compress());
    $pxml->finish();
    
	echo "</font></ul>\n";
    
$pagelib->htmlFoot('<hr>');
