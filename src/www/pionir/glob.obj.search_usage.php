<?php
/**
 * object tracking: search for usage of object X in other tables
 * $Header: trunk/src/www/pionir/glob.obj.search_usage.php 59 2018-11-21 09:04:09Z $
 * @package glob.obj.search_usage.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $tablename
 * @param $prim_id
 * @swreq ID:PRO03
 * @swreq UREQ:0001045: g > object tracking > HOME
 */

session_start(); 


require_once ('reqnormal.inc');
require_once ('class.obj.search_usage.inc');
require_once ('db_x_obj.inc');

$error= & ErrorHandler::get();
$sql  = logon2( $_SERVER['PHP_SELF'] );
$sql2 = logon2( $_SERVER['PHP_SELF'] );
$sql3 = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$tablename = $_REQUEST['tablename'];
$prim_id   = $_REQUEST['prim_id'];

$title = 'Object tracking';

$infoarr=array();
$infoarr['help_url'] = 'object_tracking.html';

$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr['obj_name'] = $tablename;
$infoarr["obj_id"]   = $prim_id;
$infoarr["show_name"] = 1;

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

if (empty($prim_id) || empty($tablename)) htmlFoot('ALERT', 'Please give table name and id!');

$pk_arr  = primary_keys_get2($tablename);
// $pk_name = $pk_arr[0];


echo '<blockquote>';
echo '<img src="images/search.usage_single.gif" TITLE="used by other objects"> ';
echo '<font color="#999999">Search usage of object with id='.$prim_id. '</font><br><br>';

$search_table = $tablename;

if (!empty($pk_arr[1])) {
  $ftab_prim_name=NULL;
  $ftab_imp_name =NULL;
  $search_table = fk_check2($pk_arr[0], $tablename, $ftab_prim_name, $ftab_imp_name);
  $search_table_nice = tablename_nice2($search_table);
	
  info_out('Info', 'This element part of <B>'.$search_table_nice.'</B>.');
} 

$useopt = array("showicon"=>1);

$objSearch = new object_usage($sql, $search_table, $prim_id, 1, "", $useopt );
$tab_track_info = $objSearch->start($sql);


if ($search_table!="PROJ") { 
	  
    // standard OBJECT tracking 
    foreach ($tab_track_info as $row) {
        
        $objSearch->getNumOneTab($sql, $row['pa_t'], $row['pa_pk']);
		$tmp_table = $objSearch->current_table;  
		$tmp_num   = $objSearch->usedtabs[$tmp_table] ;  
		$imp_col   = $objSearch->usedtabs_i[$tmp_table] ;
		// when only ONE parent found => search deeper
		if ($tmp_num==1) {
			$objSearch->get_parent_obj( $sql3, $tmp_table, $search_table, $imp_col );
			echo "<br>";
		}
		
		
	
	}
}


$objLinkTabs = $objSearch->objlink_start($sql);
if (!empty($objLinkTabs)) {
	foreach( $objLinkTabs as $moTable) {
		$objSearch->objlink_oneTab($sql, $moTable);
	}
	
}

echo '<br>';   
$objSearch->getProjUsage($sql);
if ( $objSearch->proj_num_use == 1 ) {
    $sql->query("SELECT PROJ_ID FROM proj_has_elem WHERE
                 table_name = '".$search_table."' AND prim_key = '".$prim_id."'");
    $sql->ReadRow();
	$tmp_projid = $sql->RowData[0];
    if ($tmp_projid) {
        $sql->query("SELECT NAME FROM proj WHERE PROJ_ID=$tmp_projid");
        $sql->ReadRow();
	    $proj_name = $sql->RowData[0];
        echo "<br>&nbsp;&nbsp;&nbsp;  <a href=\"edit.tmpl.php?t=PROJ&id=$tmp_projid\"><img src=\"images/icon.PROJ.gif\" border=0> 
            <B>$proj_name</B></a> <br>";
    }            
}

echo '<br>';   
$objSearch->getWorklistUsage($sql);
if ( $objSearch->wklist_num_use == 1 ) {
    $sql->query("SELECT worklist_id FROM worklist_entry WHERE
                 table_name = '".$search_table."' AND objid = '".$prim_id."'");
    $sql->ReadRow();
	$tmp_wklistid = $sql->RowData[0];
    if ($tmp_wklistid) {
        $sql->query("SELECT NAME FROM WORKLIST WHERE WORKLIST_ID=$tmp_wklistid");
        $sql->ReadRow();
	    $wklistname_name = $sql->RowData[0];
        echo "<br>&nbsp;&nbsp;&nbsp;  <a href=\"edit.tmpl.php?t=WORKLIST&id=$tmp_wklistid\"><img src=\"images/icon.WORKLIST.gif\" border=0> 
            <B>$wklistname_name</B></a> <br>";
    }            
}

$allet_num = $objSearch->obj_num_all + $objSearch->proj_num_use;
	
if ($allet_num == 0) echo 'no relation found.<P>';
   

if ( $tablename== "CONCRETE_SUBST" AND $objSearch->obj_num_all>0) {
    echo '<hr size="1" noshade>';  
    echo "<font color=gray><B>Special sample tracking:</B> (where it has been used as sample!)</font><br><br>";
    require_once('lev1/o.CONCRETE_SUBST.tracking.inc');
    $suc_track_lib = new oCONCRETE_SUBST_trackC($prim_id);
    $suc_track_lib->track_back( $sql );
}


echo '<hr size="1" noshade>';

htmlFoot('</blockquote>');

