<?php
/**
 * get info of images
 * @package obj.img.list_info.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $go : 0 analyze
   		 $parx["showall"] = 0 : nothing
		 				    1 : only errors
							2 : all
			"series"  : 1 analyse image-series on server
			"savesize": 0,1 only for root
 */
session_start(); 


require_once ('db_access.inc');
require_once ('globals.inc');
require_once ('func_head.inc');
require_once ('access_check.inc');
require_once ('table_access.inc');
require_once ("glob.image.inc");
require_once ('func_form.inc');
require_once ("sql_query_dyn.inc");
require_once ("o.IMG.file.inc");
require_once('f.progressBar.inc');
require_once ('f.update.inc');

class oImgListInfo {

function __construct($parx, $go, $cnt) {
	$this->go = $go;
	$this->parx=  $parx;
	$this->objnum = $cnt;
	$tablename="IMG";
	
	$sqlopt=array();
	$sqlopt["order"] = 1;
	$this->sqlAfter  = get_selection_as_sql( $tablename, $sqlopt);
	
	$this->sizeUpdatePossible = 0;
	if ( glob_column_exists("IMG", "SIZEX" ) ) {
		$this->sizeUpdatePossible = 1;
	}
	$this->saveCnt=0;
}

function tabOut($key, $val, $notes=NULL) {
	 echo "<tr valign=top><td NOWRAP><font color=gray>$key </font></td><td NOWRAP>&nbsp;<B>".$val."</B></td><td NOWRAP>$notes</td></tr>\n";
}

function form0 () {
	

	$parx = $this->parx;
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Select parameters";
	$initarr["submittitle"] = "Analyze now";
	$initarr["tabwidth"]    = "AUTO";
	$initarr["tabnowrap"]=  1;
	$hiddenarr = NULL;

	$formobj = new formc($initarr, $hiddenarr, 0);
	
	$fieldxa=array();
	
	$selarr = array( 0=>"silent", 1=>"Show only errors", 2=>"Show all images" );
	$fieldxa[] = array ( 
		"title"  => "Info level", 
		"name"   => "showall",
		"object" => "select", 
		"inits" => $selarr, 
		"val" => $parx["showall"],
		"notes" => "show images: none, errors, all");
	
	
	$fieldxa[] = array ( 
		"title"  => "Check series?", 
		"name"   => "series",
		"object" => "checkbox", 
		"val" => $parx["series"],
		"notes" => "Analyse image-series-images on NET ?");
		
	if ( glob_isAdmin() AND $this->sizeUpdatePossible) {
		$fieldxa[] = array ( 
			"title"  => "Save size?", 
			"name"   => "savesize",
			"object" => "checkbox", 
			"val" => $parx["savesize"],
			"notes" => "Save image-size in image-object: column size?");
	}
	
	foreach( $fieldxa as $dummy=>$fieldx) {
		//if ( $this->go ) $fieldx["view"] = 1;
		$formobj->fieldOut( $fieldx );
	}
	

	/*
	if ( $this->go ) {
		$optcl = array( "noSubmitButton"=>1 );
	}
	*/
	$optcl=array();
	$formobj->close( TRUE, $optcl );
}

function summary( &$infox ) {
	echo "<br>";
    $iopt = NULL;
    $iopt["width"] = "300";

    htmlInfoBox( "Summary", "", "open", "INFO", $iopt );

    echo "<table cellpadding=0 cellspacing=0 border=0>";
	
	foreach( $infox as $dummy=>$valarr) {
		$key = $valarr[0];
		$val = $valarr[1];
		$notes = $valarr[2];
		
		if ($key=="errors") {
			if ($val>0) $this->tabOut("errors (all)", "<font color=red>".$val."</font>", $notes);
			continue;
		}
		if ($key=="size") {
			$sizemb = $val*0.000001 ;
			$this->tabOut("total size", $sizemb." MBytes");
			continue;
		}
		$this->tabOut($key, $val, $notes );
	}
	reset ($infox); 

	

    echo "</td></tr></table>\n";
    htmlInfoBox( "", "", "close" );
}

function shortNetGet( $name ) {
	//if (strlen($name)>20) {
	//	$name = substr($name,0,20)." ...";
	//}
	return ($name);
}

function funcHead($text) {
	echo "<B>$text</B><br><br>";
}

function help() {
	htmlInfoBox( "Short help", "", "open", "HELP" );
	
	?>
	<b>The tool analyses:</b>
	<ul>
	<li>if an image exists on DB-server</li>
	<li>the of the image (print size summary of all images)</li>
	</ul>
	<br>
	<B>normal 'single' image</B> <ul>
	<li> if the image is a 'single' images, the tool checks first the database-server for the image file, than the PATH-name of the image</li>
	
	</ul>
	<br>
	<b>option 'Check series?'</b><ul>
	 <li>if the image is of class 'series': the tool checks only for the PATH-name of the image.</li>
	<?
	echo "</ul>\n";
	

	htmlInfoBox( "", "", "close" );
}

/**
 * - update size of image in field SIZEX
 * - check, if size already saved on IMG
 */
function _saveSize( &$sql, $img_id, $filesize ) {
	
	if ( !intval($filesize) )  $filesize = NULL;
	// check, if already saved
	$oldsize = glob_elementDataGet( $sql, 'IMG', 'IMG_ID', $img_id, 'SIZEX'); 
	if ($oldsize==$filesize) return; // do not update again
	
	$argu = NULL;
	$argu["SIZEX"] = $filesize;
	$idarr  = array("IMG_ID"=> $img_id);
	gObjUpdate::update_row_s ( $sql, "IMG", $argu, $idarr );  
	$this->saveCnt++;
}

function doAll( &$sql ,&$sql2 ) {
	global $varcol;
	
	$tablename = "IMG";
	#$classname  = $varcol->obj_id_to_class_name( $extra_obj_id );
	#$classname  = $varcol->class_id_to_name($extra_class_id);
	$series_class_id	= $varcol->class_name_to_id( $tablename, "series" );
	$imgSubLib = new oIMG_fileC();
	$flushLib  = new fProgressBar( );
	$prgopt=array();
	$prgopt['objname']='images';
	$prgopt['maxnum']= $this->objnum;
	$flushLib->shoPgroBar($prgopt);
	
	
	$sizeUpdatePossible = 0;
	if ( glob_isAdmin() and $this->parx["savesize"]>0 ) $sizeUpdatePossible = 1;
	if ( !glob_column_exists("IMG", "SIZEX" ) ) {
		$sizeUpdatePossible = 0;
	}
	if ($sizeUpdatePossible) {
		echo "Info: save size of images in image-object: column: size.<br>";
	}
	
	echo "</ul>";
	
	if ( $this->parx["showall"]>0 ) {
	
		echo "<table cellpadding=1 cellspacing=1 border=0 >";
		echo "<tr bgcolor=#D0D0D0>";
		$headx=array();
		$headx[]="";
		$headx[]="exists?";
		$headx[]="bytes";
		$headx[]="type";
		$headx[]="name";
		$headx[]="notes";
	
		foreach( $headx as $dummy=>$text) {
			echo "<th>".$text."</th>";
		}
		echo "</tr>\n";
	}
	
	$allx = NULL;
	$sizex    = 0;
	$img_cnt		 = 0;
	$img_cnt_bad	 = 0;
	
	$imgerr = NULL;
	$colarr=array();
	$colarr["OK"]    = "green";
	$colarr["INFO"]  = "gray";
	$colarr["ERROR"] = "red";
	
	$sql->query("SELECT x.IMG_ID, x.NAME, x.EXTRA_OBJ_ID FROM ".$this->sqlAfter );
	while ( $sql->ReadRow() ) {
	
		$tmpsize   = "";
		$errtxt	   = "";
		$errnotes  = "";
		$img_type  = "single image";
		$errprio   = "ERROR";
		$img_class_id = 0;
		$img_searchOnNet = "SRV"; // "SRV" - search on server
								  // "DIR" - search on NET
		$img_search = "";
		$img_showit = 0;
		$fileSizeSrv = 0;	// file size on server
		
		if ($this->parx["showall"]==2) $img_showit = 1;
	
		$img_id       = $sql->RowData[0];
		$img_name     = $sql->RowData[1];
		$extra_obj_id = $sql->RowData[2];
		
		if ( $extra_obj_id AND $this->parx["series"]>0 ) {
			$sql2->query("SELECT EXTRA_CLASS_ID FROM EXTRA_OBJ where EXTRA_OBJ_ID=".$extra_obj_id);
			$sql2->ReadRow();
			$img_class_id    = $sql2->RowData[0];
			if ($img_class_id == $series_class_id) {
				$img_searchOnNet = "DIR";
				$img_type = "series";
			}
		}
		
		$filename    = $imgSubLib->imgPathFull( $img_id );
		$filenameNet = netfile2serverpath( $img_name );
		$filenameNew = $filename;
		$filesize 	 = 0;
		$nameIsUrl   = 0;
	
		do {
			if ( $img_searchOnNet == "DIR" ) {
				$img_search = "SERIES on NET";
				$filenameNew = $filenameNet;
				if ( !$imgSubLib->imgOnNetExists( $img_name ) ) {
					$errtxt .= "not found on NET";
					$errnotes = "NET: ".$this->shortNetGet($filenameNew);
					$errprio = "ERROR";
					break;
				}
				break;
				
			} 
			
			$flagExists = $imgSubLib->onServerExists( $img_id );
			if ($flagExists) {
				$img_search = ", on Server";
				$filesize = filesize( $filenameNew );
				$fileSizeSrv=$filesize;
				$sizex    = $sizex + $filesize;
				$tmpsize  = $filesize;
			} else {
				$tmpsize = "?";
				$errtxt  = "not on Server";
			}
			$nameIsUrl = $imgSubLib->nameIsUrl($img_name);
			
			
			if ( $img_searchOnNet == "SRV" and $nameIsUrl) {
				$img_search  .= ", on NET";
				$flagExistsNet  = $imgSubLib->imgOnNetExists( $filenameNet );
				$flagExists = $flagExists + $flagExistsNet;
				if ( !$flagExistsNet ) {
					$errtxt .= ", not on NET";
					$errnotes = "NET: ".$this->shortNetGet($filenameNew);
				} else {
					$errtxt = ""; 
				}
				
			}

			if ($errtxt!="") {
				$errprio = "ERROR";
			} else {
				$errprio = "OK";
				$errtxt  .= "ok";
			}
			
		} while (0);
	
		if ( $errprio == "ERROR") {
			if ($this->parx["showall"]>0) $img_showit = 1;	// show on error
			$img_cnt_bad++;
			$imgerr[$img_searchOnNet] = $imgerr[$img_searchOnNet]+1;
		}
		
		if ( $sizeUpdatePossible ) {
			$this->_saveSize($sql2, $img_id, $fileSizeSrv);
		}
	
		if ( $img_showit ) {
		
			$flagExistsOut = "<font color=".$colarr[$errprio].">".$errtxt."</font>";
			echo "<tr bgcolor=#EFEFEF>";
			echo "<td><a href=\"edit.tmpl.php?t=IMG&id=".$img_id."\">".($img_cnt+1).".</a></td>";
			echo "<td NOWRAP>".$flagExistsOut.".</td>";
			echo "<td align=right>".$tmpsize."</td>";
			echo "<td NOWRAP>".$img_type.$img_search."</td>";
			echo "<td NOWRAP>".$img_name."</td>";
			if ($errnotes!="") echo "<td><font color=gray>".$errnotes."</font></td>";
			echo "</tr>\n";
			
		}
		
		$flushLib->alivePoint($img_cnt);
		$img_cnt++;
		
	}
	
	if ( $this->parx["showall"]>0 ) {
		echo "</table>";
	}
	$flushLib->alivePoint($img_cnt,1); // finish
	
	echo "<ul>";
	$allx[]=array("images", $img_cnt );
	$allx[]=array("size", $sizex );
	$allx[]=array("errors",$img_cnt_bad, "DB-images: ".$imgerr["SRV"]."<br>\nseries-images on NET: ".$imgerr["DIR"]);
	if ( $this->parx["savesize"] ) {
		$allx[]=array("saved size count", $this->saveCnt ); 
	}
	
	$this->summary ($allx);
	
	echo "</ul><hr>";
}

}

// ------------------

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$go = $_REQUEST["go"];
$parx = $_REQUEST["parx"];


$tablename = "IMG";

$flushLib  = new fProgressBar( );

$title  = 'image list info';
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1;
$infoarr['css'] = $flushLib->getCss();
$infoarr['javascript'] = $flushLib->getJS();

$copt   = NULL;
if (!$go) $go=0;


$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sql, $infoarr);
echo "<ul>";

$stopReason = "";
$tmp_info   = $_SESSION['s_tabSearchCond'][$tablename]["info"];
if ($tmp_info=="") $stopReason = "No selection info.";
if ($headarr["obj_cnt"] <= 0) $stopReason = "No elements selected.";
if ($stopReason!="") {
    htmlFoot("Attention", $stopReason." Please select images from the list!");
}

$t_rights = tableAccessCheck( $sql, $tablename );
if ( $t_rights['read'] != 1 ) {
	tableAccessMsg( "image", 'read' );
	htmlFoot();
}

$mainObj = new oImgListInfo($parx, $go, $headarr["obj_cnt"]);
$mainObj->form0();


if ( !$go ) {
	$mainObj->help();
	htmlFoot();
}

switch ($go) {
	case 0:
		$mainObj->funcHead("Analyze, if image-files are missing.");
		break;
}


$mainObj->doAll( $sql, $sql2 );

if ( $go>0 ) {
	$mainObj->help();
}


htmlFoot();
