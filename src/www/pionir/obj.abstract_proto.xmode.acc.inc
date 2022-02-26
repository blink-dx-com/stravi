<?php
require_once ("visufuncs.inc");

/**
 * - manage acceptance protocols
 * extension for abstract protocol   
 * @package obj.abstract_proto.xmode.acc.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   SREQ:0001801: o.ABSTRACT_PROTO > abstract protocol anzeigen/editieren 
 */
class oABSTRACT_PROTO_xmode_acc {
	
function __construct( &$editLib ) {
}

// PLEASE use this function; still optional 
function xmode_start( &$sqlo, &$sqlo2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {
	global $error, $varcol;
	
	echo "<ul>";
	
	// check for ACCEPT_PROT
	if ( !glob_table_exists('ACCEPT_PROT') ) {
		echo 'Please ask the admin to activate acceptance protocols.<br>';
		return;
	}
	
	$sqlsel = 'count(1) from ACCEPT_PROT where ABSTRACT_PROTO_ID='.$id;
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$accCnt = $sqlo->RowData[0];
	
	echo '<b>'.tablename_nice2('ACCEPT_PROT') .'</b>: ';
	if ($accCnt) {
		echo '<a href="view.tmpl.php?t=ACCEPT_PROT&condclean=1&searchCol=ABSTRACT_PROTO_ID&searchtxt='.$id.
			'&searchBool='.urlencode('=').'">'.$accCnt. ' protocol(s)</a> exist for this protocol.'."<br />\n";
		
	} else {
		echo 'none! &nbsp; [<a href="view.tmpl.php?t=ACCEPT_PROT">view other</a>] &nbsp;';
	}
	
	echo 'Create new: ';
	echo '[<a href="p.php?mod=DEF/o.ABSTRACT_PROTO.accept&id='.$id.'">from existing</a>] ';
	echo '[<a href="edit.insert.php?tablename=ACCEPT_PROT&argu_xtra[ABSTRACT_PROTO_ID]='.$id.'">complete new</a>]';
	echo "<br>\n";
	
	if (!$accCnt) return; 
	
	$allow_finish=0;
	if ($x_rights['write']>0) {
		$allow_finish=1;
	} else $allow_finish=0;
	
	// -------------------------------------
	
	$tabobj = new visufuncs();
	$headOpt = array( "title" => "Acceptance protocols" );
	$headx  = array ("#", "ID", 'Name', 'Status', 'Creation date', 'Action');
	$tabobj->table_head($headx,   $headOpt);
	
	// show them
	$objLinkLib = new fObjViewC();
	$cnt=1;
	$sqlsel = '* from ACCEPT_PROT where ABSTRACT_PROTO_ID='.$id.' order by ACCEPT_PROT_ID DESC';
	$sqlo2->Quesel($sqlsel);
	while ($sqlo2->ReadArray() ) {
		
		$action_html = NULL;
		
		if ($cnt>50) {
			$dataArr = array($cnt, '...', '...more...', '', '');
			$tabobj->table_row ($dataArr);
			break;
		}
		
		$accID = $sqlo2->RowData['ACCEPT_PROT_ID'];
		$name  = $sqlo2->RowData['NAME'];
		$cct_access_id  = $sqlo2->RowData['CCT_ACCESS_ID'];
		$accInfo = access_data_getai($sqlo, $cct_access_id);
		$statusName = $accInfo['h_log_name'];
		$statusNameHtml = $statusName;
		if ($statusName=='released') {
			$statusNameHtml='<span style="color:green">'.$statusName.'</span>';
		}
		// get date of release
		
		$accIDHtml = $objLinkLib->bo_IDLink( 'ACCEPT_PROT', $accID, $accID);
		
		// @swreq SREQ:0001801:003: erlaube setzen des Status "finished", um accept. protocol zu deaktivieren. 
		if ($statusName!='finished' and $allow_finish) {
			$action_html = '[<a href="glob.obj.acclock.php?t=ACCEPT_PROT&id='.$accID.'&parx[statName]=finished">finish!</a>]';
		}
		
		$dataArr = array($cnt, $accIDHtml, $name, $statusNameHtml, $accInfo['crea_date'], $action_html);
		$tabobj->table_row ($dataArr);
		$cnt++;
	}
	$tabobj->table_close();
	
	echo "<br />\n". ' <span class="yGgray">';
	echo 'Info: Active protocols must have status "released".<br />';
	echo 'Info: Deactivate protocols: set status "finished". To deactivate accept. '.
		' protocols, you must have "edit"-right on this protocol (abstract).<br />';
	echo '</span>';
}

}