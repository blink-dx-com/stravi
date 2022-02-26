<?php
require_once("subs/toolbar.inc");
// $Header: trunk/src/www/pionir/view.tmplGui.inc 59 2018-11-21 09:04:09Z $

/**
 * special GUI sub-functions for view.tmpl.php
 * @package view.tmplGui.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class viewGuiC {
 
  var $formback; /* 
  	object select mode is active:
  		['fid'] : mode: "return ONE ID", contains the DOM-id of the REMOTE form element
  		['xcol']: [optional] xcol
		['url'] : [optional] back url
		*/
  var $tablename;
  
  /**
   * if $view_opt["fromArch"]=1 is given => this can be the TABLENAME of a DATABASE-LINK!
   * @var string $tablename_SQLuse
   */ 
  var $tablename_SQLuse;
  var $primasnum;
  var $selectBoxes; // show them: 0,1
  var $archdb_mode; // 0,1
  
function __construct(&$viSubObj, $tablename) {
	$this->viSubObj = &$viSubObj;
	$this->tablename= $tablename;
	$this->tablename_SQLuse = $tablename;
	$this->archdb_mode = 0;
	$this->viSubGUI_extra2 = new view_tmpl_extra2($viSubObj);
	
}

function get_tablename_SQLuse() {
	return $this->tablename_SQLuse;
}
function is_archdbMode() {
	return $this->archdb_mode;
}

/**
 * init some variables
 * @param  $sqlo
 * @param array $formback
 */
function initVars( &$sqlo, &$formback ) {
	global  $error, $varcol;
	
	$tablename = $this->tablename;
	$this->access_id_has = cct_access_has2($tablename);
	$this->primas 	     = primary_keys_get2($tablename);
	$this->primasnum 	 = sizeof($this->primas); // number of primary keys
	$this->pk_name 		 = $this->primas[0]; // main primary key
	$this->formback = $formback;
	
	$this->doCount = 1;
	if (table_is_view($tablename)) $this->doCount = 0;  // views should never be counted
	if ( $this->primasnum > 1 )     $this->doCount = 0;  // no ASSOC tables, because can be very time-consuming!
				
	$classes    = $varcol->get_class_names($tablename);
	$this->class_tab_has = sizeof($classes); // extra_classes defined for this table?
			
	$this->menuShow = 1;
	if ($_SESSION['userGlob']["g.listHideMenu"]==1) $this->menuShow = 0;
	
	$this->selectBoxes=1;
	if ($this->formback!=NULL) {
		$this->menuShow = 0;
		if ($this->formback['fid']!=NULL) $this->selectBoxes=0; // no select boxes
	}
	
	
	
	
}

/**
 * show error and open body
 * @param string $errtext
 */
function viewHeadError(&$sqlo, $errorkey, $errtext, $tablename) {
	
	$nicename    = tablename_nice2($tablename);
	if ($nicename==NULL) $nicename    = $tablename;
	$title		= 'list view of table '.$nicename;
	$infoarr			 = NULL;
	$infoarr['title']    = $title;
	$infoarr['title_sh'] = 'list view';
	$infoarr['form_type']= 'tool';
	$infoarr['design']   = 'norm';
	$infoarr['locrow']   = array( array('home.php', 'home') );
	
	$pagelib = new gHtmlHead();
	$headarr = $pagelib->startPage($sqlo, $infoarr);
	
	echo "<br />\n";
	htmlInfoBox( $errorkey, $errtext, "", $errorkey );

	$pagelib->htmlFoot();
}

// is active ?
function selectorActive() {
	if ($this->formback!=NULL) return 1;
	else return 0;
}

/**
 * get SQL string between SELECT ... FROM
 * @param $selectCols
 * @return array($selectstr, $primast)
 */
/*
function _subSelects( &$selectCols ) {
			
	ksort ($selectCols); // sort columns by position flag 
	$komma_str= "";
	$cntcol   = 0;
	$primast  = array();	// positions of Primary keys in colNames
			
	while ( $th = E A C H( $selectCols ) ) {
		$db_key = key($th[1]);
		$selectstr= $selectstr . $komma_str . $db_key;
		$komma_str =",";
		// TBD: change this
		if (isset($this->primas[0]) and ($db_key == "x.".$this->primas[0])) {
			$primast[0]=$cntcol;// repair position of FIRST PRIMARY key,
		}
		if (isset($this->primas[1]) and ($db_key == "x.".$this->primas[1])) {
			$primast[1]=$cntcol; // repair position of 2. PRIMARY key,
		}
		if (isset($this->primas[2]) and ($db_key == "x.".$this->primas[2])) {
			$primast[2]=$cntcol; // repair position of 3. PRIMARY key 
		}
		$cntcol++;
	}
	reset($selectCols);	
	if ( $selectstr == "")  $selectstr="*";
	
	
	
	return array($selectstr, $primast);
}
*/

/**
 * do a count of elements for the condition
 * - exit on SQl-SELECT problem
 * @return number of elements
 */
function doCondCount(&$sql, $sqlAfter, $sel_info) {
	
	$sqls = "select count(*) from ". $sqlAfter; // get number of rows
	$sql->setErrLogging(0); // switch OFF error log (not interesting)
	$retval = $sql->query($sqls);
	$sql->setErrLogging(1);

	if ( $retval <= 0) {
		echo "&nbsp;<br><center>"; 
		
		htmlInfoBox( "The SQL-query is incorrect!", "", "open", "WARNRED");
	
		echo "May be the search condition contains type mismatches.<BR>";
		echo "<a href=\"view.tmpl.php?t=".$this->tablename."&condclean=1\"><B>Clear the search condition</B></A> and try ".
			 "other search parameters.<br><br>";
		echo "<font color=gray>The SQL-search string was:</font><br><B>".$sel_info."</B><br>";
		htmlInfoBox( "", "", "close" );

		$this->publishCnt(0);
		
		echo "</center>";
		echo "</body></html>";
		exit;
	}

	if ( $sql->ReadRow() )  $selectedBaseCnt = $sql->RowData[0]; /* number of selected sets DB */
    
	$showCnt = ($sel_info==NULL) ? 0 : $selectedBaseCnt;
	$this->publishCnt($showCnt);
	
	return $selectedBaseCnt;
}


// start viewform form (checkboxes)
function startViewForm(&$sqlo, $formback) {
	
	$actionstr = 'view.tmpl.php?t='.$this->tablename;
    if ( $formback['url']!=NULL) {
    	$nextAction =  $formback['url'];
    } else {
    	$nextAction='select';
    }

    echo "<form name=\"viewform\" method=\"post\" action=\"".$actionstr."\">\n"; 
    echo '<input type=hidden name=action value="'.$nextAction.'">'."\n";
    /*** WARNING: if you insert further INPUT fields,  MODIFY javascript function selall()    ***/

	if ( $formback['fid']!=NULL ) { /* if this is a selection call, do not show this menus */
	    $this->viSubGUI_extra2->history_select( $sqlo, $this->tablename );
    }
}

function setPageSets(&$view_opt, $viewPage, $selectedBaseCnt) {
	// OUTPUT: $this->viewPgSets, $this->selectedBaseCnt
	//
	
	
	$this->selectedBaseCnt = $selectedBaseCnt;
	
	if ( !isset($view_opt["setsPerView"]) ) $this->viewPgSets = $_SESSION['userGlob']['o.'.$this->tablename.'.showsets'];
	else $this->viewPgSets = $view_opt["setsPerView"]; 
	
	if ( !$this->viewPgSets ) $this->viewPgSets=20; 
	$this->viewPgSetsShow = $this->viewPgSets;
	
	if ( $this->viewPgSets=="all" ) {
		$this->viewPgSetsShow = "all";
		$this->viewPgSets = $selectedBaseCnt; /* show all */
		$viewPgCnt=0;
	} else {
		$viewPgCnt=($viewPage-1) * $this->viewPgSets;
	}
	return ($viewPgCnt);
}

// FUNCTION: print HTML-body + MENU
function htmlBody( &$t_rights ) {

	$this->writeJavaScript();

	if ( $this->menuShow )  {	
		echo '<body alink=#0000ff vlink=#0000ff bgcolor=#ffffff style="margin: 0">'."\n";
		$primasnum = $this->primasnum; // input for menu
		require_once ("view.menu.inc");
		fViewMenu( $menu, $this->tablename, $t_rights, $primasnum );
		$mb = new MenuBuilder();
		$mb->createMenu($menu);
		
		$menuopt=array("menushow"=> 1);
		$objHeadLib = new fObjToolbar($this->tablename, 0, "list");
		$menuRight = $objHeadLib->getMenuRight($menuopt);
		$mb->menuRight($menuRight);
		echo "\n";
	} else {
		// make it slim for SELECT actions !!!
		echo '<body alink=#0000ff vlink=#0000ff bgcolor=#ffffff marginwidth=0 marginheight=0 style="margin: 0">'."\n";
	}
}

/**
 * - show: menu, toolbar, search-form
 *
 * @param $sql
 * @param $selectCols
 * @param $class_params_show
 * @param $fullBaseCnt
 * @param $sel_info
 * @param $t_rights - table access rights
 * @param $option
 *   'info' : extra info string before "Selected:, Show:"
 *
 * @global 	INPUT:
		$this->access_id_has
		$this->doCount
		$this->primasnum
		$this->class_tab_has
		$this->menuShow
		$this->viewPgSetsShow
		$this->viewPgSets
		$this->selectedBaseCnt
 */
function toolbar( &$sql, &$selectCols, &$class_params_show, $fullBaseCnt, &$sel_info, &$t_rights, $option=NULL ) {
	
	require_once("subs/f.tsearch.inc");
	require_once ('f.help.inc');
	
	
	
	$info = NULL; // info string: contains the "Selected, Show" infos
	if ($option['info']!=NULL) $info = $option['info'];
	
	$tablename = $this->tablename;
	if ($sel_info != "") $selIsActive=1;
	else $selIsActive=0;
	
	if ( $this->formback['fid']!=NULL or $this->formback['url']!=NULL) {
		// TABLE: select-mode: close-form-row
		echo "<table cellpadding=1 cellspacing=0 border=0 width=100% bgcolor=#3366cc><tr><td>";
		if ($this->formback['info']!=NULL)
			echo '<span class="yGwhite">&nbsp;<b>'.$this->formback['info'].'</b></span><br />';
		
		if ( $this->formback['url']!=NULL ) {	
			echo "&nbsp;<input type=button value=\"SUBMIT selection\" onclick='document.viewform.submit();'>&nbsp;\n";
		}
		echo "&nbsp;<input type=button value=\"Cancel\" onclick='javascript:location.href=\"".
			$_SERVER['PHP_SELF']."?t=".$tablename."&cctgoCleClo=1\"'>&nbsp;\n";
		echo " <input type=button value=\"Switch to normal view\" onclick='javascript:location.href=\"".
			$_SERVER['PHP_SELF']."?t=".$tablename."&cctgoclear=1\"'>&nbsp;\n";
		if (  $t_rights["insert"]>0 )  {
	        echo '&nbsp;[<a href="glob.obj.crea_wiz.php?tablename='.$tablename.'" class="yGwhite">NEW object</A>]&nbsp;';
	    }
    	$this->showPolicyInfo($this->viSubObj, $sql);
    	$helpLib = new fHelpC();
		$hlpopt=array("object" => 'icon');
		$helpText = $helpLib->link_show("select_an_element.html", "help", $hlpopt);
		echo '&nbsp;'.$helpText;
		echo "</td></tr></table>\n";
	}
	 
	$opter ="&nbsp;<span class=xLite>Show: ".
		$this->viewPgSetsShow . " elements per page [<a href=\"javascript:showset_pref('')\" >default</a>]";
	if ($this->viewPgSetsShow != "all") {
		$opter .= " [<a href=\"javascript:showset_pref('".(int)($this->viewPgSets*0.5)."')\" >--</a>]  
		[<a href=\"javascript:showset_pref('".($this->viewPgSets*2)."')\" >++</a>] 
		[<a href=\"javascript:showset_pref('all')\" >ALL</a>]</span>";
	}
	
	if ( $selIsActive ) {
		$info .= "&nbsp;<span class=xLite>Selected: </span><span style=\"font-weight:bold; font-size:1.2em; color:black;\">".
			$this->selectedBaseCnt."</span><span class=xLite> ";
			if ($this->doCount) $info = $info. " of ".$fullBaseCnt;
			$info .= '</span>&nbsp; <a class="yButSmall" '."href=\"javascript:clearSearchCond('".$tablename."')\">clear Filter</a>";
	} else {
		$info .= "&nbsp;<span class=xLite>No selection active.<br />";
		if ($this->doCount) $info .= "&nbsp;Total elements: ".$fullBaseCnt;
		$info .=  '</span>';
	}
	/*
	if ( $sel_info != "" ) {
		$info .= " <img src=\"images/ic.listSelAct.gif\" border=0 TITLE=\"selection is active\">";
	}
	*/
	if ( $this->primasnum<2 and $selIsActive) $info .= " <a href=\"frame.left.nav.php?listtab=".$tablename.
			"&mode=list\" target=left><img src=\"images/b14.lfram.gif\" border=0 TITLE=\"show list in left frame\"></a>";
	
	$info .= "<br>" .$opter;
	$tolopt = array("menushow"=>$this->menuShow);
	$objHeadLib = new fObjToolbar($tablename, 0, "list");
	$objHeadLib->toolbar_show($sql, $info, $tolopt );
	
	echo "<td width=10>&nbsp;&nbsp;&nbsp;&nbsp;</td>"; // space between left frame and search-form
	echo "<td>";
	// search-form-table:START
	echo '<!-- KEY:SEARCH //-->'."\n";
	
	echo "<table bgcolor=#E0E0FF cellpadding=3 cellspacing=0 style='border:1px solid #D0D0EF;'>".
			"<tr><td bgcolor=#D0D0EF valign=top><img src=\"images/ic.frmmode.gif\"></td><td valign=top nowrap>";
	

	$seaForOpt = NULL;
	if ( $sel_info == "" ) { // conditon exists ?
		$seaForOpt = array( "operShow"=>-1 );
	}
	if ($_REQUEST['searchVario']>0) {
		$seaForOpt['searchVario']=1;
	}
	$classInfo["parashow"] = $class_params_show ;
	$classInfo["name"] 	   = $class_params_show[0]; // TBD: test this !
	$classInfo["classhas"] = $this->class_tab_has;
	// glob_printr( $classInfo, "classInfo info" );
	
	$searchForm = new tSearch_form($tablename);
	$searchForm->tform_show( $sql, $selectCols, $classInfo, $this->access_id_has, $seaForOpt);
	echo "</td></tr>\n";
	echo '<!-- KEY:SEARCH //-->'."\n";
	echo "</table>\n"; // search-form-table:END
	
	echo '</td><td width=50%></td></tr>'."\n";
	
	$tmpPref = intval($_SESSION['userGlob']["o.".$tablename.".condShow"]);
	if ( $tmpPref>-1 and $sel_info!=NULL) {
		echo '<tr>';
		$condMaxLen = 200;
		if ( strlen($sel_info)>$condMaxLen) {
			$sel_info_sh = substr($sel_info,0,$condMaxLen) . '   ......';
		} else $sel_info_sh = $sel_info;
		
		echo "<td valign=top colspan=4>&nbsp;<span class=xLite>Condition:&nbsp;<B>". 
		htmlspecialchars($sel_info_sh). "</B></span>";
		echo '</td>'."\n";
	}
	
	
	$objHeadLib->toolbar_end();  
}



function pageClose() {
	echo "</form>\n"; // the "viewform"
	echo "</body></html>\n";
}

/**
 * TBD: bring to external Javascript-file
 * @return -
 */
function writeJavaScript() {
	
	$tablename=$this->tablename;
	?>
 	<script language="JavaScript">
  	<!--
  	var xCheckall=0;
  
  	function writeit(dest_id, value, name) {
		if (window.opener != null)  {
            window.opener.inputRemote( dest_id, value, name ); 
		    <? /* window.close(); */ ?>
            location.href="f.winclose.php?tablename=<?php echo $tablename?>";
        } else alert("Warning: no parent window"); 
  	}
		 
	function clearSearchCond(tablename) {
		location.href="view.tmpl.php?t="+tablename+"&condclean=1" ;
	}
	
	function selall( num ) {
		inputOffset=1;
		if (!self.xCheckall) {
		  val = true;
		} else {
		   val = false;
		}
    	self.xCheckall = 1-self.xCheckall;
		  
		for( i=0; i<num; i++ )
		{
			/*WARNING: offset inputOffset comes from the INPUT fields in the form*/
			document.viewform.elements[i+inputOffset].checked = val; 
		}
	}
	
	function formAction( actionx ) {
		document.viewform.action.value=actionx;
		document.viewform.submit();
	}
	
	function open_info( url )   {				
    		
		InfoWin = window.open( url, "help","scrollbars=yes,width=650,height=500,resizable=yes"); 
		InfoWin.focus();				
  	}
	function sort_pref(tmp_col_ext, sortcritDirNew) {
		location.href="view.tmpl.php?t=<?echo $tablename?>&userprefset[sortcrit]=" +tmp_col_ext+ "%20" + sortcritDirNew;
	}
	function showset_pref(setnum) {
		location.href="view.tmpl.php?t=<?echo $tablename?>&userprefset[showsets]=" +setnum ;
	}  
    function open_lfr( url ) {
        parent.left.location.href = url;
    }
		
  	//-->
  	</script>

<?
}

/**
 * test to forward an $action-command
 * @return 
 * @param object $sel
 * @param object $action
 */
function forwardSelect( &$sel, $action ) {
	
	if ( ($action!="") AND ($action != "select") ) { //  any action selected -> jump to the script
		
		// reset $s_formback
		$_SESSION['s_formback']=NULL;
		
		if ( strstr($action,"?")==NULL ) $action .= "?"; // add a parameter flag 
					
		$html_param = '';		
		$new_url = $action."&tablename=".$this->tablename . $html_param;
		js__location_replace($new_url, 'new URL' );
		exit(); // return;	
	}
}

function _msgBoxOpen($title) {
	echo "<br><br><center>\n"; 
	$options = array("width"=>'500px;');
	htmlInfoBox( "$title", "", "open", "INFO", $options );
	
}

function _msgBoxClose() {
	htmlInfoBox( "", "", "close" );
	echo "</center>\n";
}

// message: no search
function msgNoSearch($fullBaseCnt) {
	
	$this->_msgBoxOpen('Information');
	do {
		if (table_is_view($this->tablename)) {
			echo "This list is of type VIEW.<br>You have to give a search ";
			echo "condition containing at least a condition with column  <B>".
				$this->pk_name."</B>!";
			break;
		}
		if (!$this->doCount) {
			echo "The number of elements in feature lists will not be counted (to save performance).<br>\n";
			// continue text !!!
		}
		if ($fullBaseCnt=="0") { // counter was set by the SQL-query
			echo "This table is completely empty."; 
			break;
		}
		// normal table with more the 1000 elements
		if ($fullBaseCnt>=1000) echo "The list contains more than 1000 elements.<br>";
		
		echo "Enter a search condition or press the link below to show the list without condition.<br><p>\n";
		echo "<a href='view.tmpl.php?t=".$this->tablename."&listshow=1'>".
			'<img src="res/img/chevrons-right.svg" style="vertical-align:middle; padding-right:10px; padding-left:15px;" >';
		echo '<B><font size="+1">Show list anyway.</font></a></B></p>';
			
	} while (0);
	$this->_msgBoxClose();
	
}

// show message box: too many rows
function msgManyDataShow() {
	// only ask, if the user has "sets per page" to "all"; if he has the value set in the preferences, he knows what he wants
    $nicename    = tablename_nice2($this->tablename);
	$this->_msgBoxOpen('Too many data selected for showing on one page');
    echo "Warning: You want to show more than <B>500</B> data sets on one page!</I><br><br>\n";
	echo "<li> <B><a href=\"javascript:showset_pref('')\" ><font size=\"+1\">Show 20 data sets per page</font></a></B> (recommended)</li><br>\n";
	echo "<li> <B><a href='view.tmpl.php?t=".$this->tablename."&listall=1'>";
	echo "<font size=\"+1\">Show full list (".$this->viewPgSets." data sets).</font></a></B></li>\n";
    echo "<br><font color=gray><I>You can customize the number of <br>\"shown data sets per page\" for <br>";
    echo "the list view of \"".$nicename."\" in the <a href=\"glob.objtab.pref.php?tablename=".$this->tablename."\">preferences</a>.</I></font>\n";
   $this->_msgBoxClose();
   
}

/**
 * - analyse special policies
 * - show a message box, if one found
 * @param $sqlo
 */
function showPolicyInfo(&$sqlo) {
	$roleCache = $this->viSubObj->getRoleCache();
	$info = '';
	if ($roleCache['useRelease']>0) {
		$info .= "You can only select 'released' substances!";
	}
	if ($info!=NULL) {
		$info1 = "Policy info: ".$info;
		echo '&nbsp;<img src="images/i13_info.gif" title="'.str_replace('"','', $info1 ).'">';
	}

}

function showNews($text) {
	echo '&nbsp;<font color=gray><b>Info:</b></font> '.$text.'<br />'."\n";
}

/**
 * - show list-page navigation (next page links)
 * @param array $options 
 * 		'showSUBMIT' : 0,1
 * 		'showSELECT' : 0,1
 * 					
 */
function showPagNav(
	$oristartcnt,
	$cnt,
	$selectedBaseCnt,
	$viewPage,
	$viewedCnt,
	$setsPerView,
	$showcnt,
	$options=NULL
	) {
	
	$tablename = $this->tablename;
	
	if ( $this->selectBoxes ) {
		// show only, if checkboxes are active
		echo "<img src=\"0.gif\" width=24 height=1>";
		echo "<img src=\"images/but.checkall.gif\" align=\"top\" title='check all viewed' ".
			 "vspace=4 onclick=javascript:selall(".$showcnt.")>";
		if ( $options['showSELECT']>0) 
			echo "&nbsp;&nbsp;&nbsp;<input type=submit value=\"Select checked\" style=\"margin-top:3px;\">\n";
		if ( $options['showSUBMIT'] ) {
			echo "&nbsp;<input type=submit value=\"SUBMIT\">\n"; // submit the form with default action=select
		}
	}
	
	echo "<table width=\"100%\" cellpadding=0 cellspacing=0 border=0 background=\"images/bg.listtab.gif\">".
		"<tr><td height=29>";
	
	echo "<font color=gray>";
	echo "&nbsp; Showing <B>".($oristartcnt+1)."</B> ... <B>".$cnt."</B> of <b>".$selectedBaseCnt."</b> Entries";	 
	echo "&nbsp;&nbsp;&nbsp;&nbsp;";
	
	$maxViewPage = ceil($selectedBaseCnt/$setsPerView);
	
	if ($maxViewPage>1) {
	
		if ( $viewPage > 1 ) {
			echo '<a href="view.tmpl.php?t='.$tablename.'&viewPage='.($viewPage-1).'">&laquo; Previous</a>';
		} else {
			echo '&laquo; Previous';
		}
		
		echo "&nbsp;&nbsp;| Page: ";
		if ($viewPage<6) {
			
			$i = 1;
			while ($i <= $maxViewPage) {
				if ( $i>6 ) {
					echo "...&nbsp;";
					break;
				}
				if ($i==$viewPage) echo "<font color=black><b>";
				echo "<a href=\"view.tmpl.php?t=".$tablename."&viewPage=".$i."\">".$i."</a>";
				if ($i==$viewPage) echo "</b></font>";
				echo "&nbsp;&nbsp;\n";
				
				$i++;
			}
		} else {
			echo "<font color=black><b>". $viewPage. "</b></font>&nbsp;&nbsp;\n";
		}
		//echo "<b>";
		echo " |&nbsp;&nbsp;";
		//echo "<img src=\"images/ic.sep.gif\" hspace=4>";
		$viewPage++;
		if ( $viewedCnt < $selectedBaseCnt ) {
			echo '<a href="view.tmpl.php?t='.$tablename.'&viewPage='.$viewPage.'">Next &raquo;</a>';
		}
		//echo "</b>";
	}
	echo "</font>\n";
	echo "</td></tr></table>\n";
}

/**
 * publish the current element count
 * @param int $elemCnt
 * @return -
 */
function publishCnt($elemCnt) {
	if ( $this->access_id_has OR ($_SESSION['s_suflag'] ) ) {
	    $histList_obj = new historyList();
	    $tmpret = $histList_obj->historycheck( $this->tablename, $elemCnt );
	    if ( $tmpret > 0 ) htmlShowHistory(); 
	}
}


}
