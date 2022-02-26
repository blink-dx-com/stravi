<?php
require_once ("gui/o.JOUR_ENTRY.gui.inc"); 
require_once ("subs/obj.jour_entry.ed.inc");
require_once ("o.S_OBJLINK.subs.inc");
require_once ("visufuncs.inc");
require_once ('f.msgboxes.inc');

/**
 * - JOUR_ENTRY lab view
 * - it contains code, which will be included in edit.tmpl.php, when the rider is set to "MODE"
 * @package    edit.tmpl.php
 * @subpackage obj.jour_entry.xmode.labview.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oJOUR_ENTRY_xmode_labview {
	
    
    
    
    /**
     * manage JOUR_ENTRY lab view
     *
     * @param object $sql
     * @param object $sql2
     * @param object $id
     * @param object $arguByKey
     * @param object $x_rights
     * @param object $extraobj_o
     * @param object $tabArgu
     *   'action' : 
     *      'STD' : standard
     *      'paste_objs' : paste objects from clipbord
     * @param object $editAllow
     * 
     * other INPUT from user
     * @global array $_REQUEST['parx']
     * @global array $_REQUEST['params'] 
     * @global int $_REQUEST['go']
     */
    function xmode_start( &$sql, &$sql2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {
    	global  $error;
    	
    	require_once ("f.objview.inc");	
    	
    
    	$parx = $_REQUEST['parx'];
    	$params = $_REQUEST['params'];
    	$go   = $_REQUEST['go'];
    	
    	$tmp_editallowed=0;
    	if ( $_SESSION['s_sessVars']["o.JOUR_ENTRY.editmode"] == "edit" ) {
    		if ( $x_rights["write"] ) {
    			$tmp_editallowed=1;
    		}
    	}
    	
    	$labjourLib = new oEXPlabjourC();
    	$labjourLib->setCss();
    	$labjourLib->setPrefs();
    	$labjourLib->setImgInline( 1 );
    	
//     	$deviceIDs = $labjourLib->getDeviceInfo($sql, $id);
//     	if (!empty($deviceIDs)) {	
//     		$oneDevID = current($deviceIDs);
//     	}
    	
    	$tmpWhiteSplit = "&nbsp;<img src=\"images/ic.sepWhit.gif\">&nbsp; ";
    	echo "<ul>"; // gray: #B8AFB8
    	
    	
    	
    	if (  $editAllow ) {
    		echo '<a href="obj.sattach.edit.php?tx=JOUR_ENTRY&id='.$id.'&action=insert&opt[forward]=obj">'.
    		"<img src=\"images/icon.SATTACH.gif\" border=0> Add attachment</a> ";
    		echo $tmpWhiteSplit;
    	
    		/* moved to menu ...
    		$tmpurlPart = 'glob.obj.assocadd.php?id='.$id.
    			'&mtable=JOUR_ENTRY&asoctab=JOUR_H_EXP&asoccol=POS&tablename=EXP&selorder=x.EXP_ID';
    		echo ' <a href="'.$tmpurlPart.'" ><img src="images/icon.EXP.gif" border=0 hspace=2>'.
    			'Add exp.</a>'. $tmpWhiteSplit;
    		*/
    		
    		// echo "Table: <a href=\"obj.jour_entry.tabed.php?id=".$id."&act=new\">Add</a>";
    		// echo ", ";
    		// echo "<a href=\"obj.jour_entry.tabed.php?id=".$id."&act=edit\">Edit</a>";
    		// echo $tmpWhiteSplit;
    		
            // 		if ($oneDevID) {
            // 			echo '<img src="images/icon.DB_USER.gif"> <a href="p.php?mod=DEF/o.JOUR_ENTRY.defUsers&id='.$id.'">Add/Remove staff</a>';
            // 			echo $tmpWhiteSplit;
            // 		}
    		// echo "<br>\n";
    	}
    	
    	echo '<a href="obj.jour_entry.list.php"><img src="images/o.JOUR_ENTRY.report.svg" height=25> Journal report</a>'.
    		$tmpWhiteSplit;
    	echo "<a href=\"obj.jour_entry.ed1.php?action=create\"><img src=\"images/but.40.new.png\" height=25>New entry</a>";
    	if (  $editAllow ) {
    	    echo ' &nbsp;&nbsp;&nbsp; <a href="'.$_SERVER['PHP_SELF']."?t=JOUR_ENTRY&id=".$id.
    	     '&tabArgu[action]=paste_objs"><img src="images/but.paste.gif" height=25 title="Paste objects from clipboard"> Paste objects</a>';
    	}
    		
//     	if ($oneDevID) {
//     		$objLinkLib = new fObjViewC();
//     		$tmp_dev_html = $objLinkLib->bo_display( $sql, 'CHIP_READER', $oneDevID );
//     		echo 'Workingline: ' . $tmp_dev_html . $tmpWhiteSplit;
//     	}
    		
    	// $condurl = urlencode('x.EXP_ID in (select EXP_ID from JOUR_H_EXP where JOUR_ENTRY_ID='.$id.')');
    	/* echo '<a href="view.tmpl.php?t=EXP&condclean=1&tableSCond='.$condurl.
    		'"><img src="images/icon.EXP.gif" border=0 hspace=2>show experiment list</a>'.
    		$tmpWhiteSplit;	
    	*/
    		
    	//echo "<a href=\"obj.jour_entry.ed1.php?action=create\"><img src=\"images/ic.object_new_set.gif\" border=0>New entry</a>".
    	//	$tmpWhiteSplit; 
    	
    	
    	echo "<br><br>\n";
    	// echo "<ul>";
    	// echo "<br>\n";
    	
    	$sqlsLoop = "SELECT x.* FROM JOUR_ENTRY x where x.JOUR_ENTRY_ID=".$id;
    	$sql->query($sqlsLoop);
    	$sql->ReadArray();
    	$thisData = $sql->RowData;
    	
    	if ($tmp_editallowed) {
    	
    		$addInfo = $labjourLib->getProjInfo( $sql, $id );
    		
    		$action = "update";
    		$labObj = new oEXPlabjour1($id, $action);
    		unset ($thisData["EXTRA_OBJ_ID"]);
    		unset ($thisData["CCT_ACCESS_ID"]);
    		
    		
    		
    		if (!$go) {
    			$tmpname = $thisData["NAME"];
    			$parx["NAME"] = $tmpname;
    			$tmpdate = substr($tmpname,0, $labObj->dateStringLength); 
    			$tmpret = $labObj->isValidDate($tmpdate);
    			if ($tmpret) {
    				$parx["NAME"] = substr($tmpname,$labObj->dateStringLength+1); // cut a leading date-string e.g. "2000-01-01 "
    			}
    			
    			$labObj->setParams($sql, $thisData, $parx, 0);
    			
    			if ($tabArgu['action']=='paste_objs') {
    			    $num_obj = $labObj->paste_objects($sql);
    			    if (!$error->printAllEasy()) {
    			        if ($num_obj) {
        			        echo "<br>";
        			        cMsgbox::showBox("ok", "Object links inserted at the end of the text."); 
        			        echo "<br>";
    			        }
    			    } 
    			}
    			
    		} else {
    			$labObj->setParams($sql, $params, $parx, 1);
    			$labObj->updateEntry($sql);
    			if ($error->printAllEasy()) {
    				$forwardStopped = 1;
    				$stopReason = "update problem.";
    				echo "<br><br>";
    			} else {
    				$newurl = $_SERVER['PHP_SELF']."?t=JOUR_ENTRY&id=".$id;
    				js__location_replace( $newurl ); 
    				echo "<br><hr>";
    				return;
    			}
    		}
    		$labObj->showform($sql, $addInfo);
    		$labObj->Hints();
    		
    	} else {
    		
    		$labjourLib->startTable();
    		$xoutopt = array("info"=> "onePage");
    		$labjourLib->oneEntryOut( $sql, $thisData, $xoutopt);
    		$labjourLib->stopTable ();
    		
    		
    	}
    	
    	
    }

}
