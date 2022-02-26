<?php
/**
 * Insert ASSOC elements
 * TBD: not tested since 2017 !
 * @package glob.objtab.assocnew.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $tablename (must be the ASSOC) 
         $argu_xtra : predefined values
 * @version0 2002-11-28
 */

session_start(); 


require_once ('reqnormal.inc');
require_once('table_access.inc');
require_once('sql_query_dyn.inc');
require_once('edit.sub.inc');
require_once('javascript.inc');
require_once('insert.inc');
require_once("edit.edit.inc");
require_once("edit.show.inc"); 

function tmp_head_out($text) {
	echo "<font color=#4040FF size=+1><B>$text</B></font><br>\n";
}

function assoc_new( &$sql,  &$sql2, $sqlAfter, $primary_keys, $argu ) {
	global $tablename, $mothertable ;
	
	

	
	$mother_nice    = tablename_nice2($mothertable);
	$primary_key1   = $primary_keys[0];
	$primary_key2   = $primary_keys[1];
	$primary_key3   = $primary_keys[2];
	
	$primary_key  = $primary_keys[0];
	
	$sqls = "select x.".$primary_key. " from ".$sqlAfter. " order by x.".$primary_key;
	if ( $_SESSION['userGlob']["g.debugLevel"] > 0 ) {
		echo "DEBUG: SQL-command: $sqls	<br>";
	}
        $retVal = $sql->query("$sqls");

	$cnt=0;
	$deny_cnt=0;
	$fine=1;
	$extend_where="";
	
	// get second [third] primary keys
	$primval2 = $argu[$primary_key2];
	if (!$primval2) {
		echo "ERROR: no value given for primary key $primary_key2.<br>";
		return (array($cnt, $deny_cnt));
	} else {
		$extend_where=$primary_key2."=".$primval2;
	}
	
	if ($primary_key3!="") {
		$primval3 = $argu[$primary_key3];
		if (!$primval3) {
			echo "ERROR: no value given for primary key $primary_key2.<br>";
			return (array($cnt, $deny_cnt));
		} else {
			$extend_where = $extend_where. " AND ". $primary_key3."=".$primval3;
		}
	}
	
	while ( $sql->ReadRow() && $fine )  {
		$prim_id  =  $sql->RowData[0];

		$o_rights = access_check( $sql2, $mothertable, $prim_id );

		$argu[$primary_key] = $prim_id;

		if ( $o_rights["insert"] ) {
			$sqls =  "select ".$primary_key." from
				  $tablename where $primary_key=".$prim_id." AND ".$extend_where;
			$retVal = $sql2->query( $sqls );
			
		 	if ( $sql2->ReadRow() ) {
				echo "<font color=#808000>WARNING:</font> Element [$prim_id,$primval2,$primval3]: exists. No insert.<br>";
				$deny_cnt++;
			} else {
				$retval = insert_row( $sql2, $tablename, $argu );
				if ( $retval<1 ) {
					  echo "<font color=red>Error:</font> Element [$prim_id,$primval2,$primval3]: insert failed. Process stopped.";
					  echo "<br>";
					  $fine=0;
					  $deny_cnt++;
				}
			}
		} else {
			echo "<font color=#808000>WARNING:</font> Element  [mother-ID:$prim_id]: INSERT not permitted !<br>";
			$deny_cnt++;	
		}
		$cnt++;
	}	
	
	
	return ( array($cnt, $deny_cnt) );
}

$error = & ErrorHandler::get();
$sql   =  logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();

$tablename=$_REQUEST['tablename'];
$go=$_REQUEST['go'];
$argu_xtra=$_REQUEST['argu_xtra'];

$title = 'Bulk Feature Insert: Insert elements into feature lists';
$infoarr=array();
$infoarr["title"] 		= $title;
$infoarr["title_sh"] 	= "Bulk Feature Insert";
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;
$pagelib = new gHtmlHead();
$pagelib->_PageHead ( $infoarr["title"],  $infoarr );
$pagelib->_startBody($sql, $infoarr);
js_formAll();

if ( $go ) echo " [<a href=\"".$_SERVER['PHP_SELF']."?tablename=$tablename\"> &lt;&lt; Refine parameters</a>]\n";

echo "<blockquote>\n";

$assocnewarr = array(
		"I_SUBST_IN_SOCKET",
		"CONCRETE_PROTO_STEP",
		"EXP_HAS_PROTO"
		);

$goallow=0;
if ( in_array($tablename, $assocnewarr) ) $goallow=1;

if ( !$goallow ) {
	echo "SORRY: this function can not be run for this table.<br>\n";
	htmlfoot();
}

$mothertable    = mothertable_get2($tablename);
$mothernicename = tablename_nice2($mothertable);
$tabnicename    = tablename_nice2($tablename);
$tab_has_cct_access = cct_access_has2($tablename);

if ( !$go ) {
    $tmpmsg = "This script inserts<br>ONE new element of '<B>$tabnicename</B>' <br>".
              " into each object of selected: ".
              " '<B>$mothernicename</B>'.<br><br>".
              " If an element exists, the function keeps it untouched."; 
    htmlInfoBox( "Info", $tmpmsg, "", "HELP"); 
    echo "<br>\n";
}

$t_rights = tableAccessCheck( $sql, $mothertable );
if ( $t_rights['insert'] != 1 ) {
	tableAccessMsg( $mothernicename, 'insert' );
	return;
}

$fromClause  = $_SESSION['s_tabSearchCond'][$mothertable]["f"];
$tableSCondM = $_SESSION['s_tabSearchCond'][$mothertable]["w"];
$whereXtra   = $_SESSION['s_tabSearchCond'][$mothertable]["x"];
$tmp_info    = $_SESSION['s_tabSearchCond'][$mothertable]["info"];
$sqlAfter = full_query_get( $mothertable, $fromClause, $tableSCondM, $whereXtra );

if ( $tmp_info=="" ) {
	echo "Please select elements from list!";
	return;
}

$sqls = "select count(1) from ".$sqlAfter;
$sql->query("$sqls");
if ($sql->ReadRow() ) {
	$num_elem=$sql->RowData[0];
}

if ( $num_elem < 1 ) {
	echo "No objects selected!";
	return;
}

echo "<font color=gray>Selected objects: </font><b>$num_elem</b>"; 
echo " &nbsp;&nbsp;&nbsp;<a href='view.tmpl.php?t=".$mothertable."'>[show list]</a>";
echo "<br><br>\n";

$colNames_tmp 	= columns_get2($tablename);   // get column names
$pk_name 	= PrimNameGet2($mothertable);
$colNames_ori = array();

if ( !$go ) {
    foreach($colNames_tmp as $th1) {
		if ($th1!=$pk_name) $colNames_ori[] = $th1;	// PK can not be set !!! 
	}
	tmp_head_out("1. Edit data for the new element.<br>&nbsp;");

	?>
	<form method="post"  name="editform" action="<?echo $_SERVER['PHP_SELF']?>?go=1&tablename=<?echo $tablename."\" >";
	?>
	<table CELLSPACING=1 CELLPADDING=2 bgcolor=#c0d8e8 valign=top>
	<?
       
    
	
    $editFormLib = new fFormEditC();
	$editFormLib->setObject($tablename, $id);
	$edformOpt = array( "H_EXP_RAW"=>$H_EXP_RAW_DESC_ID );
	$cnt   =0;
	$editAllow=1;
	$action="insert";
	$editFormLib->form_editx( $action, $sql, $colNames_ori, $primary_keys, $argu_xtra,
         	$extraobj_o, $editAllow, $edformOpt );
	
	?>
	</form>
	<?
	htmlFoot();
}

if ( $go == 1 ) {
	
	
	tmp_head_out("2. Prepare data for insert.");
	$colTypeTmp="";
	$showFormLib = new fFormShowC();
	$showFormLib->setObject($tablename, $id);
	$formopt = NULL;
	$formopt["H_EXP_RAW"]=$H_EXP_RAW_DESC_ID;
	
	$colNames_ori=NULL;
	$showFormLib->form_show( $sql, $sql2, $argu, $colNames_ori, $extraobj_o, $formopt);
	
	echo "</TD></TR></TABLE><br>";
	
	?>
	<form method="post"  name="editform"  action="<?echo $_SERVER['PHP_SELF']?>?tablename=<?echo $tablename?>&go=2" >
	<?
       
    	$cnt=0;
     
    	foreach($argu as $th0=>$th1) {
    		$colName= $th0;
    		//$value1 = htmlspecialchars ($th1);
    		echo "<input type=hidden name=argu[". $colName ."] value=\"".$th[1]."\">";
    	}
	
	?>
	<input type="submit" value="Insert objects now!"><P>
	</form>
	<?
} 

if ( $go == 2 ) {

	tmp_head_out("3. Insert NOW !!!<br>&nbsp;");
	
	$primary_keys =	primary_keys_get2($tablename);
	$primary_key  = $primary_keys[0];
	
	if ( !$tab_has_cct_access ) {
		list ($cnt, $deny_cnt) = assoc_new( $sql, $sql2, $sqlAfter, $primary_keys, $argu );
	} else {
        htmlErrorBox("Error","table $tablename has a CCT_ACCESS entry, not allowed for this function");
        htmlFoot();
    }
	
	echo "&nbsp;<br>";
	echo "Touched elements:</font> ".$cnt. "<br>";
	echo "<font color=green>Inserted objects:</font> ". ($cnt-$deny_cnt). "<br>";
	if ( $deny_cnt ) echo "<font color=red>Not inserted:</font> ".$deny_cnt."<br>";
	
	
	echo "<BR>Ready.<br>&nbsp;<br>";
	
	echo "<a href=\"view.tmpl.php?t=".$tablename."\">Back to list view</a><br>";
}


htmlFoot();

