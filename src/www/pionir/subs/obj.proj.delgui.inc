<?php
require_once('g_delete.inc');
require_once("o.PROJ.tree.inc");
require_once('o.PROJ.addelems.inc');

/**
 * delete objects/links from project
 * @package obj.proj.delgui.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/obj.proj.delgui.inc 59 2018-11-21 09:04:09Z $
 */
class projDelGuiC {

var $infoLevel;
var $CLASSNAME;
var $t_rights;   // role rights for "PROJ"

function __construct() {
	$this->CLASSNAME = "projDelGuiC";
	$this->tabstart  = 0;
	
	$this->dellib = new fObjDelC();
	$this->proj_tab_nice = tablename_nice2('PROJ');
	$this->proj_tab_nice_up = strtoupper(tablename_nice2('PROJ'));
	// $this->t_rights  = $t_rights;
}

function _info_out ( $key, $text ) {
 	$color="gray";
    if ( $key == "ERROR")     $color="red";
	if ( $key == "INFODENY" ) $color="red";
    $tmpstr = "<font color=".$color."><B>".$key.": </B></font> <font color=#808080>".$text."</font>";
	return ($tmpstr);
}

function infoRowOut($text) {
	if (!$this->tabstart) {
		$this->infoTabOpen();
	}
	echo "<tr><td bgcolor=#EFEFEF colspan=3>".$text."</td></tr>\n";
}

// data info
function infoRowInfo( &$sqlo, $cnt, $tmp_table, $loopObjID, $tbl_icon_html, $infoRes  ) {
	if (!$this->tabstart) {
		$this->infoTabOpen();
	}
	
	$tmp_nicename = htmlspecialchars( obj_nice_name($sqlo, $tmp_table, $loopObjID) );
	echo "<tr><td><font color=gray>".$cnt.". ".$tbl_icon_html."</font></td>".
		"<td>&nbsp;".$tmp_nicename."&nbsp;[".$loopObjID."]&nbsp;</td>".
		"<td>";
	echo $infoRes;
	echo "</TD></TR>\n";
	
}

function infoTabOpen() {
	if (!$this->tabstart) {
		echo "<table cellpadding=1 cellspacing=1 border=0 bgcolor=#D0D0D0><tr><td>\n";
		echo "<B><font color=gray>List of selected objects</font></B></td></tr>\n";
		echo "<tr><td>\n";
		echo "<table width=100% cellpadding=1 cellspacing=1 border=0 bgcolor=#FFFFFF>\n";
		$this->tabstart = 1;
	}
}

function infoTabClose() {
	if ($this->tabstart) {
		echo "</table>\n";
 		echo "</td></tr></table>\n"; // border table
		$this->tabstart = 0;
	}
}

function getProjSubObjects(&$sql, $proj_id) {
	$sel = array();
	$sql->query('SELECT TABLE_NAME, PRIM_KEY FROM proj_has_elem WHERE proj_id = '.$proj_id. " order by TABLE_NAME"); // check elements (bo's) in proj
    while ($sql->ReadRow() ) {
		$tabname = $sql->RowData[0];
        $sel[$sql->RowData[0]][$sql->RowData[1]] = 1; 
	}
	
	// real SUB-PROJECTS
	$sql->query('SELECT PROJ_ID FROM proj WHERE pro_proj_id = '.$proj_id); 
	while ($sql->ReadRow() ) {
        $sel["PROJ_ORI"][$sql->RowData[0]] = 1; 
	}
	return ($sel);
}

function treeOrgOne(&$sql, $proj_id, $doflag, $pdopt, $objOpt) {
	// FUNCTION: analyse/delete SUB-PROJECTS of project $proj_id
	
	global $error;
	
	$DUMMY_arr=array();
	$infoAll = array();
	$loopOpt  = array();
	$loopOpt["infoLevel"] = $this->infoLevel - 1;
	
	$tmp_ProjName = obj_nice_name( $sql, "PROJ", $proj_id );
	
	$this->infoRowOut("<B>Analysis of selected SUB-".$this->proj_tab_nice." '".$tmp_ProjName."' [ID:$proj_id] ...</B>");
	
	$projArr = oPROJ_tree::tree2array($sql, $proj_id);
	
	if (empty($projArr)) return; // no sub projects
	
	end($projArr);
	
	while ($xProjId = key($projArr)) { // for all sub-projects in $proj_id
	
		$valx = current	($projArr);
		$ans2 = prev	($projArr);
	
		if ($this->infoLevel > 0) {
			$tmp_ProjName = obj_nice_name( $sql, "PROJ", $xProjId );
			$this->infoRowOut("&nbsp;Sub-".$this->proj_tab_nice.": ".htmlspecialchars($tmp_ProjName)." [$xProjId]");
		}	
		$sel = $this->getProjSubObjects($sql, $xProjId);
		
		$infoAllTmp = $this->theLoop($sql, $sel, $doflag, $pdopt, $objOpt, $xProjId, $loopOpt); // no output of HIDDEN fields !!!
		if (!empty($infoAllTmp)) foreach( $infoAllTmp as $key=>$val) {
  			$infoAll[$key] = $infoAll[$key] + $infoAllTmp[$key];
 		}
		if ($error->Got(READONLY))  {
		    $error->set($this->CLASSNAME.":treeOrgOne", 1, "Error occurred for one single ".$this->proj_tab_nice." [ID:$xProjId].");
			// TBD: continue ???
		}
	}
	return ($infoAll);
}

function projsSelDelrecursive(&$sql, &$sel, $doflag, $proj_id, $pdopt, $objOpt) {
	// FUNCTION: - RECURSIVE Analysis of selected SUB-Projects
	//			 - delete all objects in project recursivly
	global $error;
	
	$bo_ids = $sel["PROJ_ORI"]; // look for PROJ
	$infoAll = array();
	
 	foreach( $bo_ids as $xProjId=>$dummy) { // all IDs
	
		$infoAllTmp = $this->treeOrgOne($sql, $xProjId, $doflag, $pdopt, $objOpt);
		if (!empty($infoAllTmp)) foreach( $infoAllTmp as $key=>$val) {
  			$infoAll[$key] = $infoAll[$key] + $infoAllTmp[$key];
 		}
		
		if ($error->Got(READONLY))  {
		    $error->set($this->CLASSNAME.":projOneDelrecursive", 1, "Error occurred for recursive delete of ".$this->proj_tab_nice." [ID:$xProjId].");
			// TBD: continue ???
		}
	}
	reset($bo_ids);
	return ($infoAll);
}


/**
 * - delete all objects/links in ONE PROJECT level
   - the project MUST NOT have SUB-PROJECTS; this is handled by projsSelDelrecursive()
 * @param object $sql
 * @param array $sel
 * @param string $doflag // "deldb", "delproj"
 * @param array $pdopt  // "projdelsubobj"
 * @param array  $objOpt // "projDeepDel"
 * @param int $proj_id
 * @param string $loopopt
 * "selVarOut" : variable for preparation mode, to save the CHECKED objects ("", "1")
	 		 		1 => set to ONE, if you have to SAVE the selection
	"infoLevel"  0,1,2, ...
 * @return void|multitype:number
 */
function theLoop(&$sql, &$sel, $doflag, $pdopt, $objOpt, $proj_id, $loopopt = NULL	) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$lineOut        = 0;
	$hiddenLineCnt  = 0;
	$cntelem    	= 0;
	$delelem    	= 0;
	$errcnt     	= 0;
	$projs_has  	= 0;
	$projs_only 	= 0;
	$obj_also_exp	= 0;
	$cutelem		= 0;
	$retinf			= array();
 	$lineEverOut	= 0;
	
	if ( empty($sel) ) return;
	
	if ($_SESSION['userGlob']["g.debugLevel"]>1 )  {
		echo "DEBUG: theLoop(): doflag:$doflag pdopt:<pre>";
		print_r($pdopt);
		echo "</pre>\n";
	}
	
	/*
	$o_rights = access_check($sql, 'PROJ', $proj_id);
	if (!$o_rights["insert"] ) {
		$retinf["cntelem"] 		= 	0;
		$retinf["errcnt"] 		=  	1;
		$tmp_nicename = htmlspecialchars(obj_nice_name($sql, 'PROJ', $proj_id) );
		$this->infoRowOut("<B><font color=red>Denied:</font></B> Removing elements in project '$tmp_nicename' [$proj_id] not allowed due to access rights ...");
		return ($retinf);
	}
	*/
	
	$projAddLib  = new oProjAddElem($sql, $proj_id);
	if ($error->Got(READONLY))  {
		$retinf["cntelem"] 		= 	0;
		$retinf["errcnt"] 		=  	1;
		$tmp_nicename = htmlspecialchars(obj_nice_name($sql, 'PROJ', $proj_id) );
		$errLast   = $error->getLast();
		$error_txt = $errLast->text;
		$error_id  = $errLast->id;
		$error->reset();
		$this->infoRowOut("<B><font color=red>Denied:</font></B> Removing elements in ".$this->proj_tab_nice." '$tmp_nicename' [$proj_id] not allowed:".$error_txt);
		return ($retinf);
	}
 
	foreach( $sel as $code_name=>$bo_ids) {
 
		if ( $code_name === 'PROJ_ORI' ) {
			$tmp_table  = 'PROJ';
			$projs_only = 1; 			// only PROJECTS in selection
			$projs_has  = 1;
		} else {
			$projs_only = 0; 			// also other objects selected
			$tmp_table  = $code_name; 	// $code_name needed below
		}
		
		$tmp_tbl_nice = tablename_nice2($tmp_table);
		$icon_tabname = $tmp_table;
		if ($code_name === 'PROJ')  $icon_tabname = "PROJ_link";
		$tbl_icon_html= "<img src=\"images/icon.".$icon_tabname.".gif\" TITLE=\"".$tmp_tbl_nice."\">";
		// $role_rights  = tableAccessCheck($sql, $tmp_table);
		if ($code_name=="PROJ") $tmp_tbl_nice="Link to ".$this->proj_tab_nice; 
		if ($code_name=="EXP" ) $obj_also_exp=1;
		
		foreach( $bo_ids as $loopObjID=>$dummy) { // inner object loop for all IDs of $code_name
		
			$retval 	= 0;
			$infoRes   	= NULL; // result info
			$showLine	= 0;
			
			if (!$doflag) {
			
				if ($loopopt["selVarOut"]) echo '<input type="hidden" name=sel['.$code_name.']['.$loopObjID.'] value="1">'."\n";
	
			} else {   // second round   
	
				do {   // ON_ERROR block
					$retval_tmp = 0;
	
					if ($doflag === 'deldb') { // delete objects from database ?
					
						if ( $pdopt["projdelsubobj"] OR $code_name == "PROJ_ORI" ) {
							// only remove an object, if flag "projdelsubobj" is set !!!
							
							$infoRes = $this->remove_obj($sql, $tmp_table, $code_name, $loopObjID, $proj_id, $doflag, $objOpt);
							if ($error->got(READONLY)) { 
								$retval_tmp = -1;
								break;
							}
							$delelem++;
						}
					}
					
					if ($code_name == "PROJ_ORI") { // PROJ_ORIc is not part of "proj_has_elem"
						// no real action, because it was removed / denied before
					} else {
						// TBD: use project class-method for delete !!!
						$projAddLib->unlinkObj($sql, $tmp_table, $loopObjID);
						if ($error->Got(READONLY))  {
							$retval_tmp = -18; 
							break;
						}
						
						$cutelem++;
					}
					
				} while (0);  // END:  ON_ERROR block
				
				if ($retval_tmp<0) $retval = -20;
							
			}
			
			if ($error->Got(READONLY))  {
				$errLast   = $error->getLast();
				$error_txt = $errLast->text;
				$error_id  = $errLast->id;
				if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
					debugOut('Error: '.$error_txt.':'.$error_id, $FUNCNAME);
				}
				$error->reset();
			}
			
			
			
			if ( $loopopt["infoLevel"]>2 ) $showLine = 1;
			if ( $loopopt["infoLevel"]>1 AND $cntelem<10 )  $showLine = 1;
			if ( $loopopt["infoLevel"]>0 AND $cntelem<5  )  $showLine = 1;
			if ( $retval<0 )      	  $showLine = 1;
			
			if ( $showLine ) {
				$this->infoRowInfo( $sql, ($cntelem+1), $tmp_table, $loopObjID, $tbl_icon_html, $infoRes );
				$lineOut     = 0; // 0 not yet, 1: just printed, 2: last
				$lineEverOut = 1;
				
			} else {
				$hiddenLineCnt ++;
				$lineOut++;
			}
	
			if ($lineEverOut AND $lineOut==1) {
				echo "<tr><td align=center>...</td><td bgcolor=#EFEFEF>...</td></td></tr>\n";
			}
			
			$cntelem++;
			if ( $retval<0 ) $errcnt++;
	
		}
  }
 
	$retinf["hiddenLineCnt"]= 	$hiddenLineCnt;
	$retinf["cntelem"] 		= 	$cntelem;
	$retinf["errcnt"] 		=  	$errcnt;
	$retinf["lineOut"] 		=  	$lineOut;
	$retinf["delelem"]	 	=  	$delelem;
	$retinf["cutelem"]	 	=  	$cutelem;
	$retinf["projs_only"]	=	$projs_only;
	$retinf["projs_has"]	=	$projs_has;
	$retinf["obj_also_exp"]	=	$obj_also_exp;
	
	
 	return ($retinf);
}

/**
 * - delete an object
 * - can also handle PROJECT !!
 */
function remove_obj(&$sql, $tmp_table, $code_name, $objID, $proj_id, $doflag, $objOpt) {
	global $error;
	$FUNCNAME= "remove_obj";
	
	$infox = NULL;
	
	if ($code_name=="PROJ") { // a "link to project"
		// a "link to project" should NOT delete the real project !!!
		$infox = "<font color=green>link </font>\n";
		return ($infox);
	} 
		
	if ( gObject_exists ($sql, $tmp_table, $objID)<1  ) {
		$infox .= "does not exist! (broken link)";
		return ($infox);
	}
	
	$retval_tmp = $this->dellib->obj_delete($sql, $tmp_table, $objID, NULL, $proj_id, $objOpt);
	
	if ( $error->got(READONLY) ) {
		$errLast   = $error->getLast();
     	$error_txt = $errLast->text;
		$infox .= $this->_info_out ( "ERROR", $error_txt." [$retval_tmp]");
		$error->set( $FUNCNAME, 1, "error during obj_delete()");
    } else {
		$infox .= "<font color=green>deleted</font>\n";
	}
	return ($infox);
}


function getLinkObjsCnt( &$sql, $proj_id ) {
	$sqls = "select count(1) from PROJ_HAS_ELEM where PROJ_ID=".$proj_id;
	$sql->query($sqls);
	$sql->ReadRow();
    $cnt = $sql->RowData[0];
	return ($cnt);
}

function getLinkedObjects( 
	&$sql,
	$proj_id, 
	&$sel     // return as reference
	) {
	
	$sel  = NULL;
	$sqls = "select TABLE_NAME, PRIM_KEY from PROJ_HAS_ELEM where PROJ_ID=".$proj_id. " order by TABLE_NAME";
	$sql->query($sqls);
	while ($sql->ReadRow()) {
    	$sel[$sql->RowData[0]][$sql->RowData[1]] = 1;
	}

}

function projdelgui(
    &$sql,
    $proj_id,
    &$sel,    // selection:  array[TABLE][OBJ_ID] = 1
	$state,   // [0] , 1       - 0: preparation, 1: go
	$doflag,  // ""         - preparation phase
              // "delproj"  - Delete object-links from PROJECT
              // "deldb"    - Delete elements from DATABASE
    $opt      // "projdelsubobj" 0|1 delete sub objects from projects
    		  // "objdeepdel"    0|1 deep delete of objects (non-projects)
			  // "projDeepDel"   0|1 deep delete of projects
			  // "selall"        [0] | 1   - select all linked objects !!!
			  // "infoLevel"     [0], 1, 2
    ) {
// FUNCTION: - GUI for delete objects from project
 
 global $error;
 
 $infoAll = array();
 $retvalMain = 0;
 $pdopt  = NULL;
 $objOpt = NULL;
 
 $this->infoLevel = 0;
 if ($opt["infoLevel"]!="") $this->infoLevel = $opt["infoLevel"];
 
 if ( $opt["projdelsubobj"]==1 )  $pdopt["projdelsubobj"]   = 1;
 if ( $opt["objdeepdel"]> 0 )  	  $objOpt["deep"] 		    = 1;
 if ( $opt["projDeepDel"]> 0 )    $objOpt["projDeepDel"]    = 1;
 $objOpt["info"] = 0;
 
 if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) { 
 	$objOpt["info"] = 1;
 }
 
 
 // DEPRICATED: removed Project deactivation on 20050126 (Steffen)
 // $tmproleright = role_check_f( $sql, 'Project_delete');
 // sql: ... proj_has_elem SET deactivated = 1 ...
 // $Project_deactivate_only = 0;
 
 if ( $_SESSION['userGlob']["g.debugLevel"]>1 ) {
    echo "<B>DEBUG</B> projdelgui(): opt:";
	print_r($opt);
	echo "<br>";
 }

 
 if (!$state) {
 	// echo "<B>Delete elements from [PROJECT]</B> or DATABASE</B>";
	echo "<form name=\"delform\" action=\"".$_SERVER['PHP_SELF']."\" method=post>\n";
	echo '<input type="hidden" name=proj_id value="'.$proj_id.'">'."\n";
	echo '<input type="hidden" name=doflag value="">'."\n";
	echo '<input type="hidden" name=actio_elem value="del">'."\n";
	echo '<input type="hidden" name=state value="1">'."\n";
	echo '<input type="hidden" name=opt[infoLevel] value="'. $this->infoLevel.'"  >'."\n";
 } else {

	switch ($doflag) {
		case "delproj":
		    echo "<B>Delete object-links from ".$this->proj_tab_nice."</B>"; break;
		case "deldb":
			echo "<B>Delete elements from DATABASE</B>"; break;
		default:
			htmlFoot("Error", "Action: '".$doflag."' not allowed");
	}

 }
 echo "<br>\n"; // TBD: BR-tag ?
 
 if (empty($sel)) {
 	
	$tmpobjcnt = $this->getLinkObjsCnt($sql, $proj_id);
	if (!$tmpobjcnt) {
	    htmlErrorBox("Error", $this->proj_tab_nice." contains no linked objects" );
		return (-15);
	}

	if (!$opt["selall"]) {
		htmlInfoBox( "NO objects selected!", "", "open", "INFO" );
		echo "<center>\n";
		echo "</form>";
		echo "<form name=\"delform\" action=\"".$_SERVER['PHP_SELF']."\" method=post>\n";
		echo '<input type="hidden" name=proj_id     value="'.$proj_id.'">'."\n";
		echo '<input type="hidden" name=doflag      value=""   >'."\n";
		echo '<input type="hidden" name=actio_elem  value="del">'."\n";
		echo '<input type="hidden" name=state       value="0"  >'."\n";
		echo '<input type="hidden" name=opt[selall] value="1"  >'."\n";
		echo '<input type="hidden" name=opt[infoLevel] value="'. $this->infoLevel.'"  >'."\n";
		echo "Select all <B>".$tmpobjcnt. "</B> linked objects ?<br><br>\n";
		echo '<input type="submit"  value="YES!"><br>'."\n";
		echo '</form>'."\n";
		htmlInfoBox( "", "", "close", "" );
		return (1);
	} else {
		$this->getLinkedObjects( $sql, $proj_id, $sel );
	}
 }
 
 if (empty($sel)) {
 	htmlErrorBox("Error", "No objects selected" );
	return (-15);
 }
 
 
 // TABLE will be opened automatically
 if ($this->infoLevel>0) $this->infoTabOpen();
 
 if (!empty($sel["PROJ_ORI"])) {
 	$savInfoLevel = $this->infoLevel;
	// $this->infoLevel++;  // increase infolevel, due to RECURSIVE deleting
 	$this->infoRowOut("<B>RECURSIVE Analysis of selected SUB-".$this->proj_tab_nice."s ...</B>");
 	$infoAll1 = $this->projsSelDelrecursive($sql, $sel, $doflag, $proj_id,  $pdopt, $objOpt);
	$this->infoRowOut("<B>Analysis of selected objects ...</B>");
	$this->infoLevel = $savInfoLevel;
 }
 
 $loopOpt = NULL;
 $loopOpt["selVarOut"] = 1;
 $loopOpt["infoLevel"] = $this->infoLevel;
 $infoAll2 = $this->theLoop($sql, $sel, $doflag, $pdopt, $objOpt, $proj_id, $loopOpt);
 
 foreach( $infoAll2 as $key=>$val) {
  	$infoAll[$key] = $infoAll1[$key] + $infoAll2[$key];
 }
 
 if ( $infoAll["hiddenLineCnt"] AND ($this->infoLevel>0) ) {
		$this->infoRowOut("<font color=gray>".$infoAll["hiddenLineCnt"]." lines were hidden</font>");
 }
 
 $this->infoTabClose(); // TABLE will be closed
 
 $tmpErrOut1 = "";
 $tmpErrOut2 = "";
 if ($infoAll["errcnt"]) {
 	 $retvalMain = -1; // set the main error
 	 $tmpErrOut  = '<font color=red><B>'.$infoAll["errcnt"].'</B> error(s)';
	 $tmpErrOut1 = "; ".$tmpErrOut;
	 $tmpErrOut2 = $tmpErrOut."<br>\n";
 }
 switch ($doflag) {
	case 'delproj':
	    echo '<B>'.$infoAll["cutelem"].'</B> of <B>'.$infoAll["cutelem"].'</B> object(s) unlinked from '.$this->proj_tab_nice.'(s).<BR>';
		echo $tmpErrOut2;
		break;
	case 'deldb':
		echo '<B>'.$infoAll["delelem"].'</B> of <B>'.$infoAll["cntelem"].'</B> object(s) deleted.<BR>';
		if ($infoAll["cutelem"]) 
		    echo '<B>'.$infoAll["cutelem"].'</B> of <B>'.$infoAll["cntelem"].'</B> object(s) unlinked from '.$this->proj_tab_nice.'(s).<BR>';
		echo $tmpErrOut2;
		break;
	default:
	
		// if selected objects are $projs_only => ask, if sub elements should be cuted from project
		
	    echo '&nbsp;<font size=+1><B>'.$infoAll["cntelem"].'</B> object(s)</font> selected (including sub-'.$this->proj_tab_nice.'s)';
		echo $tmpErrOut1;

		echo "\n<br><br>\n";
		echo "<table border=0 cellpadding=4 cellspacing=2><tr valign=top>\n";
		
		if (!$infoAll["projs_only"]) {
		    $del_from_proj_text = 'Unlink from '.$this->proj_tab_nice.'!';
			echo "<td>";
			$infopt = array("color"=>"#80FF80", "fcolor"=>"#309F30", "icon"=>"ic.projunlink.gif");
			htmlInfoBox( $del_from_proj_text, "", "open", "INFO", $infopt );
			echo "<center><B><input type=button ";
			echo "onClick=\"document.delform.doflag.value='delproj'; document.delform.submit();\" value=\"Unlink\">\n";
			echo "<br><br></B><font size=-1><font color=green>INFO:</font> Objects stay in database.</font></center>";
			htmlInfoBox( "", "", "close" );
			
			echo "</td>";
			echo "<td width=40>&nbsp;</td>\n";
			
		} 

		echo "<td>";
		if ($infoAll["projs_only"]) {
			$butText = "Delete objects!";
		} else {
		    $butText = "Delete objects from ".strtoupper($this->proj_tab_nice)." and DATABASE!";
		}
		$infopt = array("icon"=>"ic.del2.gif");
		htmlInfoBox( $butText, "", "open", "ERROR", $infopt );
		echo "<center>\n";
		echo "<input type=button onClick=\"document.delform.doflag.value='deldb'; document.delform.submit();\" value=\"Delete!\">\n";
		if ($infoAll["projs_only"]) {
		    echo "<br><br><font color=red>WARNING:</font> ".$this->proj_tab_nice."s are removed from database!\n";
		} else {
			echo "\n<br><br><font size=-1><font color=red>WARNING:</font> Objects are removed from database!</font>\n";
		}
		echo "</center>\n";
		if ($infoAll["projs_has"]) {
		    echo "<br><input type=checkbox name=opt[projdelsubobj] value=\"1\"> also delete objects in SUB-".$this->proj_tab_nice_up."S ".
				 "(otherwise stay in database)<br>\n"; 
		} else {
			echo "<input type=hidden name=opt[projdelsubobj] value=\"1\">\n"; // delete objects ...
		} 
		if ($infoAll["obj_also_exp"]) echo "<br><input type=checkbox name=opt[objdeepdel] value=\"1\"> ".
			"allow remove of experiment-sub-objects (images, protocols)<br>\n";  
		htmlInfoBox( "", "", "close" );
		
		echo "</td></tr>";
		
		// vertical space
		echo "<tr><td><img src=0.gif heigth=20 width=1></td></tr>"; 
		
		echo "<tr><td>";
		$newlevel = $this->infoLevel+1;
		echo "<a href=\"javascript:document.delform.state.value='0'; ";
		echo "document.delform.elements['opt[infoLevel]'].value='".$newlevel."'; ";
		echo "document.delform.submit();\">Show more information</a> [level: ".$newlevel."]";
			
		echo "</td></tr>\n";
		 
		echo "</table>\n";
		echo "</form>\n";
		$retvalMain = 1;
		echo "<hr size=1 noshade>\n";
   }

    return ($retvalMain); // stop calling script (forward!)
}

}
