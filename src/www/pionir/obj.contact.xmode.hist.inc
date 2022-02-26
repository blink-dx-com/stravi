<?php
/**
 * - extras for contact history
 * - it contains code, which will be included in edit.tmpl.php, when the rider is set to "MODE"
 * $Header: trunk/src/www/pionir/obj.contact.xmode.hist.inc 59 2018-11-21 09:04:09Z $
 * @package    edit.tmpl.php
 * @subpackage obj.contact.xmode.hist.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */


require_once ('gui/f.pageEntriesOrg.inc');
require_once ('subs/obj.contact_hist.edform.inc');
require_once ('subs/obj.contact_hist.report.inc');

/**
 * show REA_LOG
 */
class oCONTACT_xmode_hist {
	
function __construct( &$editLib ) {
	$this->editLib = &$editLib;
}


/**
 * 
 * @param object $sql
 * @param object $sql2
 * @param int $id
 * @param array $arguByKey
 * @param array $x_rights
 * @param array $extraobj_o
 * @param array $tabArgu
 * 	'page' show page number
 *  'timeZone' : ['past'], 'future'
 * @param variant $editAllow
 * @return -
 */
function xmode_start( &$sql, &$sql2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {
	global    $error;
	
	$table='CONTACT';
	$tmp_editallowed=0;
	if ( $_SESSION['s_sessVars']["o.CONTACT.editmode"] == "edit" ) {
		if ( $x_rights["insert"] ) {
			$tmp_editallowed=1;
		}
	}
	

	
	$EditLib   = new oCONTACT_HIST_edformC ();
	$ReportLib = new oCONTACT_HIST_logSh();
	
	echo $ReportLib->getCss();
	
	echo "<ul>";
	
	if ($tabArgu['go']) {
		
		require_once ('o.CONTACT_HIST.mod.inc');
		$modlib = new oCONTACT_HIST_mod();
		$modlib->setContact($id);
		$modlib->checkAccess($sql);
		$answer = $modlib->paramCheck($sql, $tabArgu);
		
		$formopt= array();
		if ($tabArgu['POS']>0) $formopt['action'] ='update';
		
		if ($answer[0]>0) {
			htmlErrorBox( "Error", $answer[1] );
			
			$dummy  = NULL;
			$EditLib->edform( $sql, 'edit.tmpl.php?t=CONTACT&id='.$id, $tabArgu, $dummy, $formopt );
			return; //TBD: show other logs ???
		}
		
		if ($tabArgu['POS']==NULL)		$modlib->addEntry( $sql, $tabArgu);
		else $modlib->updateEntry($sql, $tabArgu['POS'], $tabArgu);
		
		if ($error->Got(READONLY))  {
			$error->printAll();
			echo "<br />\n";
			
			$dummy  = NULL;
			$EditLib->edform( $sql, 'edit.tmpl.php?t=CONTACT&id='.$id, $tabArgu, $dummy, $formopt );
			return;
		}
		
		$newurl = 'edit.tmpl.php?t=CONTACT&id='.$id;
		js__location_replace($newurl, "back to contact" );
		
		return;
	}
	
	if ( $tmp_editallowed and ($tabArgu['POS']>0) ) {
		// show UPDATE form
		$formopt= array('action'=>'update');
		$dummy  = NULL;
		
		
		$sqlsel="* from CONTACT_HIST where CONTACT_ID=".$id. " and POS=".$tabArgu['POS'];
		$sql->Quesel($sqlsel);
		$sql->ReadArray();
		$parx = $sql->RowData;
		
		
		$EditLib->edform( $sql, 'edit.tmpl.php?t=CONTACT&id='.$id, $parx, $dummy, $formopt );
		return; // show only this form ...
	}
		
	if ($tmp_editallowed) { 
		
		$parx=array(); 
		if ($parx['EVENT_DATE']==NULL) $parx['EVENT_DATE'] = date_unix2datestr( time(),2 );
		
		$formopt= array();
		$dummy  = NULL;
		
		$EditLib->edform( $sql, 'edit.tmpl.php?t=CONTACT&id='.$id, $parx, $dummy, $formopt );
		
	} 

	
	$sqlsel="count(1) from CONTACT_HIST where CONTACT_ID=".$id;
	$sql->Quesel($sqlsel);
	$sql->ReadRow();
	$foundEntries = $sql->RowData[0];
	
	
	$fullcolspan = 4;
	$cnt=0;
	
	$pageNavLib = new f_pageEntriesGui();
	$showinf = array(
		"startShow"   => $tabArgu['page'],
	    "foundEntries"=> $foundEntries
		);
	$parx = array('entryPerPage'=>20);
    $info = $pageNavLib->setParams( $showinf, $parx );
	
    
    // only events in history !
    $sqlsel="* from CONTACT_HIST where CONTACT_ID=".$id. " order by EVENT_DATE desc";
	$sql->Quesel($sqlsel);
	echo "</ul>";
	
	$entryStart = $info['start'];
	$endShow    = $info['end'];
	$pageNow    = $pageNavLib->showinf["startShow"];
	
	$headtext = '<span style="color:gray;"><span'.$textPast.' in past</span> ';
	if ($entriesFuture) {
		$headtext .= ' |<span style="color:gray;'.$futureStyle.'"> '.$textFuture.
			' in future </span>'; 
	}
	
	
	$ReportLib->setPageNow($pageNow);
	$ReportLib->tableStart($headtext);
	
	while ($sql->ReadArray()) {
		
		$showit = 0;
		$thisData = $sql->RowData;
		
		if ( $cnt >= $entryStart ) $showit=1;
		
		if ( $cnt > $endShow )   {
			$moreExists = 1;
			break;
		}
		
		if ($showit) {
			$rowopt=array('editallow'=>$tmp_editallowed);
			$ReportLib->oneRow( $sql2, $sql->RowData, $rowopt );
		}
		

		$cnt++;
	}
	
	if ( !$cnt ) {
		echo "<tr><td colspan=".$fullcolspan."><br><center><b>No entries!</b></center><br><br></td></tr>\n";
	}
	
	$ReportLib->tableEnd();
	
	if ($cnt) {
		$pageNavLib->setLastShownEntry($cnt-1);
		$desturl= $_SERVER['PHP_SELF'].'?t='.$table.'&id='.$id.'&tabArgu[page]=';
		echo "<br />\n";
		$pageNavLib->showEntryNav ( $desturl );
	}
}

}
