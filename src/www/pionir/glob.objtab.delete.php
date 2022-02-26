<?php
/**
 * delete a set of objects (BO or associated elements)
 * @package glob.objtab.delete.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq 0001025: g > delete elements in list 
 * @param $tablename
 * @param  $forceflag NULL (ask for deletion)
    =1   (do deletion) 
	=2   do it for BO 
 * @param $parx["optdeep"] ???  
 * @param $parx["alldeep"] : DELETE also sub-objects ?
 *   [0] 
      1  - yes also sub objects
 * @param $parx["projLinksIgnore"] : ignore, if object is linked to a project ?
 *   [0]
 *    1  - can be linked to max ONE project
 *    2  - can be linked to MANY projects
 */
session_start(); 

	
require_once ('reqnormal.inc');
require_once ('sql_query_dyn.inc');
require_once ('g_delete.inc');
require_once ("f.visu_list.inc"); 
require_once ("visufuncs.inc");
require_once('f.progressBar.inc');
require_once('g_deleteMeta.inc');
require_once 'glob.obj.touch.inc';

class fObjtabDel {

function __construct(&$sql, $tablename, &$flushLib) {
	global  $error;
	
	$this->tablename=$tablename;
	$this->debugLevel  = $_SESSION['userGlob']['g.debugLevel']>0 ? $_SESSION['userGlob']['g.debugLevel'] : 0;
	$this->MAX_SHOW_OBJ = 10;
	$this->infox    = NULL;
	
	$pk_keys   	= primary_keys_get2($tablename);
	$this->objecttype = "object";
	$this->typeKey    = "OBJ";
	if ( sizeof($pk_keys)> 1) {
		$this->typeKey = "ASSOC";
		$this->objecttype = "associated element";
	}
	
	$table_nice  	   = tablename_nice2($tablename);
	$this->mothertable = mothertable_get2($tablename);
	$this->primary_col = PrimNameGet2($tablename);
	$this->primary_col_nice   = columnname_nice2($tablename, $this->primary_col);
	$this->is_BO 	   = cct_access_has2($tablename);
	
	$this->onObjLib = new fObjDelMeta($sql, $tablename);
	$this->infox["tabHasAssocs"] = $this->onObjLib->getAssocInfo();
	
	$sqlopt=array();
	$sqlopt["order"] = 1;
	if ( $this->is_BO ) {
		$joinstr = "CCT_ACCESS"; 
		query_add_join( $sql, $tablename, $joinstr );
	}
	
	$this->sqlAfter  	   = get_selection_as_sql( $tablename, $sqlopt);
	$this->sqlAfterNoSort  = get_selection_as_sql( $tablename);
	$this->sel_info		   = query_get_info($tablename);
	
	$sql->query('SELECT COUNT(1) FROM '.$this->sqlAfterNoSort);
	if ($error->Got(READONLY))  {
    	$error->set( "fObjtabDel", 1, "Error occurred during object-count." );
		return;
	}
	$this->infox["selobjCnt"] = 0;
	if ($sql->ReadRow()) $this->infox["selobjCnt"] = $sql->RowData[0]; // number of selected rows for delete
	
	$this->flushLib = &$flushLib;
}

function setParams($forceflag, $parx) {
	$this->forceflag= $forceflag;
	$this->parx = $parx;
	$this->onObjLib->setParams($forceflag, $parx);
}

function _showProgress($cnt, $force=0) {
	$this->flushLib->alivePoint($cnt, $force);
}

/**
 * 
 * @param string $type 'BO', 'ASSOC'
 * @param int $objCnt
 * @param int $objOKCnt
 * @param int $objErrCnt
 * @param int $resultText
 */
function _showSum($type, $objCnt, $objOKCnt,$objErrCnt, $resultText, $objAssocCnt) {
	$assocText=NULL;
	if ($type=='ASSOC') $assocText='element';
	echo "<br>";
	echo "<B>$objCnt</B> object(s) analysed.<br>";
	echo "<B>$objOKCnt</B>  ".$assocText." object(s) ".$resultText.".<br>";
	echo "<B>$objAssocCnt</B>  ASSOC object(s) deleted<br>";
	if ( $objErrCnt)  echo "<font color=red><B>$objErrCnt</B></font> Errors.<br>";
}

function prepInfo() {

	echo "<br>";
	$infarr = NULL;
	$infarr[] = array("Selection condition:", 
		'<font color="#999999" size=-1>'.htmlspecialchars($this->sel_info).'</font>');
	$infarr[] = array($this->objecttype."s", $this->infox["selobjCnt"].
				" selected for DELETE");
	
	if ( $this->forceflag != null ) { // SHOW, WHEN FORCEFLAG IS SET 	
		if ($this->parx["alldeep"]>0) {
			$infarr[] = array("SubObjects:", "DELETE also sub-objects");
		} else {
			$infarr[] = array("SubObjects:", "do not touch sub-objects");
		}
		
		if ($this->parx["projLinksIgnore"]>0) $tmp_info = 'YES';
		else $tmp_info = 'no';
		$infarr[] = array("Delete project links:",$tmp_info);
	}

	if ($this->debugLevel) $infarr[] = array('DEBUG:', ' delete criteria: '.$this->sqlAfterNoSort);
	
	if ( $this->infox["tabHasAssocs"] ) {
		$tmparr = $this->onObjLib->getAssocsNice();
		$infarr[] = array("Has SubObjects", $tmparr);
	}
	
	$this->infox["hasAssocDoDeep"] = 0;
	if ( ($this->parx["alldeep"]>0) AND ($this->infox["tabHasAssocs"]) ) { 
		$this->infox["hasAssocDoDeep"] = 1;
		$this->onObjLib->setAssocDo(1);
	}
	
	
	$tabobj = new visufuncs();
	$headOpt = array( "title" => "Summary", "headNoShow" =>1);
	$headx   = array ("Key", "Val");
	$tabobj->table_out2($headx, $infarr, $headOpt);
	echo "<br>";
}

function form1() {

	$nextText = "YES";
	if ( $this->is_BO ) $nextText = "Prepare &gt;";

	$iopt=array();
	$iopt["icon"] = "ic.del.gif";
	htmlInfoBox( "Delete ".$this->objecttype."s", "", "open", "INFO", $iopt );
	echo "<center>";
	
	echo '<br><B>DELETE <font size=+2>',$this->infox["selobjCnt"],'</font> '.$this->objecttype.
		 '(s) now?</B><br>&nbsp;';
	echo "<form name=\"delform\" action=\"".$_SERVER['PHP_SELF']."\" method=post>";
	echo '<input type="hidden" name=tablename value="'.$this->tablename.'">';
	echo '<input type="hidden" name=forceflag value="0">';
	
	echo "<table>";
	echo '<tr><td align=center style="paddig:10px;">';
	echo "<input type=button class='yButton' value=\"".$nextText."\" onClick=\"document.delform.forceflag.value=1; ".
		 "document.delform.submit();\">";
	echo "</td></tr>";
	
	
	if ( $this->is_BO ) {
	
    	echo "<tr><td>";
    	if (glob_isAdmin()) {
    		echo "<input type=checkbox name=parx[projLinksIgnore] value=\"1\"> ";
    	} else {
    		echo "Admin option:";
    	}
    			
    	echo " ignore links to projects? (default: no delete, if link to project exists)\n";
    	echo "</td></tr>\n";
	}
	
	
	if ( $this->infox["tabHasAssocs"] ) {
		echo "<tr><td><input type=checkbox name=parx[alldeep] value=\"1\"> ".
			 " allow remove of sub-objects\n";
		echo "</td></tr>\n";  
	}
	
	echo '</table>'."\n";
	echo '</form>';
	htmlInfoBox( "", "", "close");
	
}

function form2() {

	$nextText = "Delete NOW!";
	$iopt=array();
	$iopt["icon"] = "ic.del.gif";
	htmlInfoBox( "Delete ".$this->objecttype."s", "", "open", "INFO", $iopt );
	echo "<center>";
	
	echo '<br><B>DELETE <font size=+2>',$this->infox["selobjCnt"],'</font> '.$this->objecttype.
		 '(s) now?</B><br>&nbsp;';
	echo "<form name=\"delform\" action=\"".$_SERVER['PHP_SELF']."\" method=post>";
	echo '<input type="hidden" name=tablename value="'.$this->tablename.'">';
	echo '<input type="hidden" name=forceflag value="0">';
	if ($this->parx["alldeep"]>0) echo "<input type=hidden name=parx[alldeep] value=\"1\">\n";
	if ($this->parx["projLinksIgnore"]>0) echo "<input type=hidden name=parx[projLinksIgnore] value=\"1\">\n";
	
	echo "<table>";
	echo '<tr><td align=center>';
	echo "<input type=button class='yButton' value=\"".$nextText."\" onClick=\"document.delform.forceflag.value=2; ".
		 "document.delform.submit();\">";
	echo "</td></tr>";
	echo '</table>'."\n";
	echo '</form>';
	htmlInfoBox( "", "", "close");
}


/**
 * - delete associated elements
 * @param  $sql
 * @param  $sql2
 * @global $_SESSION['s_tabSearchCond'] [INPUT]
 * @global $this->flushLib
 * @global $this->infox["objcnt"] [OUTPUT]
 * @return -
 */
function act_noBO(&$sql, &$sql2) {   
	global  $error;
	$FUNCNAME= __CLASS__.':act_noBO';

	$tablename   = $this->tablename;
	$primary_col = $this->primary_col;
	$fromClause  = $_SESSION['s_tabSearchCond'][$tablename]['f'];
	$sqlWhere    = $_SESSION['s_tabSearchCond'][$tablename]['w'];

	$del_allow   = 0;
				
	if ($this->mothertable!=NULL) $del_allow = 1;
	if ($_SESSION['sec']['appuser'] == 'root') $del_allow = 1;
				
	if ( !$del_allow ) {
		$error->set( $FUNCNAME, 1, 'Denied: Elements of this table can not be deleted by normal users. Please ask the admin!' );
		return;
	}
	$mother_nice =  tablename_nice2($this->mothertable);
				
	echo '<font color=gray>Checking permissions of ',$this->primary_col_nice,' from mother-object ',$mother_nice,' ...</font><br><br>'."\n";
	
	// count mother elements
	$sql->Quesel('count(DISTINCT('.$this->primary_col.')) FROM '.$this->sqlAfterNoSort); 
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 2, 'error on init (SQL-statement failed.)' );
		return;
	}
	$sql->ReadRow();
	$motherCount  = $sql->RowData[0];
	$prgopt=array();
	$prgopt['objname']='rows';
  	$prgopt['maxnum'] = $motherCount;
  	$this->flushLib->shoPgroBar($prgopt);
	
	$table_nice  	   = tablename_nice2($tablename);
	echo "<b>Delete selected elements of following ".$motherCount." mother-objects:</b><br>";
	echo "<ul>\n";
	
	// distinct should not be sorted, could be the wrong column !!!
	$sql->Quesel('DISTINCT('.$this->primary_col.') FROM '.$this->sqlAfterNoSort); 
	
	$objcnt    = 0;
	$last_id   = 0;
	$show_info = 0; // show each line ?
	$denyCnt   = 0;
	
	if ($this->debugLevel>0) $show_info = 1;
	
	while ($sql->ReadRow()) {
	
		$prim_id  = $sql->RowData[0];
		$o_rights = access_check($sql2, $this->mothertable, $prim_id);
	
		$infoLine =  ($objcnt+1).". '". $table_nice."' of ".$this->primary_col_nice."=".$prim_id.': ';
		
		if ($o_rights['insert']) { // ask the insert-right for ASSOC-elements of MOTHER
			if ($show_info) $infoLine;
			// TBD: only works, if no JOIN on the SQL-condition ($_SESSION['s_tabSearchCond'][$tablename]['f']) 
			$sql2->Deletex( $tablename, $primary_col.' = '.$sql->addQuotes($prim_id).' AND ('.$sqlWhere.')', "x" );
			if ($error->printLast()) htmlFoot('</blockquote>');
			
			$actarr = array('d'=>array('x'=>array('ass'=>array('t'=>$tablename))), 'a'=>array('key'=>'del') );
			globObjTouch::touch($sql2, $this->mothertable, $prim_id, $actarr);  // if is a new mother object => touch it
			if ($show_info) echo '<B>OK:</B> deleted!<br>'."\n";	
		} else {
			echo $infoLine;
			echo ' &nbsp;&nbsp;&nbsp;<font color=red><b>WARNING:</b></font> Delete not permitted! (missing "insert"-right)<br>'."\n";
			$denyCnt++;
		}
		$this->flushLib->alivePoint($objcnt);
		$objcnt++;
	}
	$this->flushLib->alivePoint($objcnt,1);
	echo "</ul>\n";
	
	if ( $objcnt==1 ) { // only if exactly ONE object
		echo '<br><a href="edit.tmpl.php?t=',$this->mothertable,'&id=',$prim_id,'">';
		echo '<img src="images/but.close.gif" border="0"> Back to mother object <B>',$mother_nice,
			 '</B></a><br>&nbsp;<br>';
	} 
	
	$this->infox["objcnt"] = $objcnt;
	
	$objOKCnt=$objcnt-$denyCnt;
	$this->_showSum('ASSOC', $objcnt, $objOKCnt, $denyCnt, 'deleted', NULL);
}


function act_BO(&$sql, &$sql2) {

	global $error;
	
	if ( $this->debugLevel<2 ) error_reporting  ( E_ERROR ); // to hide SQL-errors
	
	if ($this->forceflag==1) {
		$textOver = "Prepare deletion";
		$oktext = "o.k.";
		$resultText = "prepared for delete";
	} 
	if ($this->forceflag==2) {
		$textOver = "Delete NOW!";
		$oktext    = "deleted";
		$resultText = "deleted";
	}
	echo "<br><font size=+2>".$textOver."</font><br><br>\n";
	$prgopt = NULL;
	$prgopt['objname']='objects';
	$prgopt['maxnum']=$this->infox["selobjCnt"];
	
	$this->flushLib->shoPgroBar($prgopt);
  
	$tablename   = $this->tablename;
	$primary_col = $this->primary_col;
	$primary_col_nice = $this->primary_col_nice;
	
	
	
	$main_name_sel = ', x.'.$primary_col;
	$sqlAfterNew   = $this->sqlAfter;
	
	$sqlStrFull = 'SELECT x.cct_access_id'.$main_name_sel.' FROM '.$sqlAfterNew;
	
	if ( $this->debugLevel >= 4 ) {
			echo "<B>INFO:</B> DEBUG-level [".$this->debugLevel."]: SQLFull:<br> ".
				 htmlspecialchars($sqlStrFull)."<br>\n";
			echo "<B>INFO:</B> DEBUG: no DELETE, just pass the loop.<br>";
	}
	
	$sql->query($sqlStrFull);
	if ($error->printLast()) htmlFoot('</ul>'); //TBD: error-set ?
	$objOKCnt = 0;
	$objCnt=0;
	$objErrCnt = 0;
	$objAssocCnt = 0;
	
	echo "<ul>\n";
	
	while ($sql->ReadRow())  {
	
		$delinfo = NULL;
		$errFlag = 0;
		$acc_id  = $sql->RowData[0]; // not needed ?
		$main_id = $sql->RowData[1];
		
		$delinfo = $this->onObjLib->doOneObj($sql2, $main_id, $acc_id);
		$doShowObj = 0;
		$objOK = $delinfo['ok'];
		$objtext = "<li>ID= <b>".$main_id."</b> ";
		if ( $this->debug>2 ) $doShowObj = 1; // show all objects
		if ($error->Got(READONLY)) {
			$doShowObj=1;
			$errFlag = 1;
		}
		
		if ($doShowObj) {
			echo $objtext;
			if ( $objOK>0 ) echo " <font color=green>o.k.</font> ";
			if ($errFlag)  {
				$error->doLog( 0 );
				$error->printAllEasy();
				$error->doLog( 1);
				$error->reset();
			}
			if ( $this->infox["hasAssocDoDeep"] ) {
				$retstr =  $this->onObjLib->AssGetObjCnt();
				if ($retstr!=NULL) $objtext .= " SubObjects: ".$retstr. " ";
				if ( $delinfo["assocDelCnt"]>0  ) echo " AssocDel: ".$delinfo["assocDelCnt"];
				if ( $delinfo["assocErr"]!=NULL ) echo " <font color=red>AssocErrors:</font> ".$delinfo["assocErr"];
			}
			echo "</li>";
		}
		
		// final error reset ...
		$error->reset(); 
		
		if ($objOK>0) {
			$objOKCnt++;
		}
		if ($delinfo["assocDelCnt"]) {
			$objAssocCnt = $objAssocCnt + $delinfo["assocDelCnt"];
		}
		if ( $errFlag>0 ) {
			$objErrCnt++;
		}
		
		$this->_showProgress($objCnt);
			
		$objCnt++;
	}
	echo "</ul>\n";
	$this->_showProgress($objCnt,1);
	
	$this->_showSum('BO', $objCnt,$objOKCnt,$objErrCnt, $resultText, $objAssocCnt);
}


}

// --------------------------------------------------------------------
$error 		= & ErrorHandler::get();
$sql   		= logon2();
$sql2  		= logon2();
if ($error->PrintLast()) htmlFoot();

$tablename=$_REQUEST['tablename'];
$go=$_REQUEST['go'];
$parx=$_REQUEST['parx'];
$forceflag=$_REQUEST['forceflag'];

$pk_keys   	= primary_keys_get2($tablename);
$title = 'Delete objects';
if ( sizeof($pk_keys)> 1) {
	$title = "Delete associated elements";
}

$flushLib = new fProgressBar( );
$tablenice = tablename_nice2($tablename);
$infoarr=array();
$infoarr["title"]    = $title;
$infoarr["form_type"]= "list";
$infoarr['obj_name'] = $tablename;
$infoarr["obj_cnt"]  = 1;   
$infoarr['css'] 	   = $flushLib->getCss() ;
$infoarr['javascript'] = $flushLib->getJS();

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage( $sql, $infoarr );

echo "<ul>";

$mainLib = new fObjtabDel($sql, $tablename, $flushLib);
if ( $error->printAll() ) {
	htmlFoot();
}
$mainLib->setParams($forceflag, $parx);

if ($tablename === 'PROJ') htmlFoot('INFO', 'Deleting of projects only possible from project navigator!');

$listVisuObj = new visu_listC();
// check TABLE selection
$copt = array ("elemNum" => $headarr["obj_cnt"] ); // prevent double SQL counting
list ($stopFlag, $stopReason)= $listVisuObj->checkSelection( $sql, $tablename, $copt );
if ( $stopFlag<0 ) {
    htmlFoot("Attention", $stopReason." Please select elements of '".$tablenice."'!");
}

$t_rights = tableAccessCheck( $sql, $tablename );
if ( $t_rights['delete'] != 1 ) {
	tableAccessMsg( $tablenice, 'delete' );
	return;
}

// -----------------------------------------------------------




if ($forceflag<=0) { // ask user if continue with deleting
	$mainLib->form1();
	$mainLib->prepInfo();
	htmlFoot();
} 

$mainLib->prepInfo();

if (!$mainLib->is_BO) {    // no cct_access_id in table
	$mainLib->act_noBO($sql, $sql2);
	$error->printAll();
 
} else { // has cct_access

	if ($forceflag==1) {
		$mainLib->form2();
	}
	$mainLib->act_BO($sql, $sql2);

}

htmlFoot('<hr></ul>');
