<?php
/**
 * show info about a table
 * @package glob.objtab.info.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $tablename
 */
 

session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("f.rider.inc");
require_once ("class.history.inc");
require_once ('glob.obj.col.inc');
require_once ("f.help.inc");
require_once ("o.S_VARIO.subs.inc");
require_once("subs/f.prefsgui.inc");

class fTableInfo {

function __construct( $tablename ) {
	$this->tablename=$tablename;
	
	$this->ColKeyStyle = 'color:#607080; font-weight:bold;';
	
	$this->typeTrans = array(
		"BO"=>"business object",
		"BO_ASSOC"=>"associated element list",
		"SYS"=>"system table",
		"SYS_META"=>"associated element  of a system table",
		);
	$this->globColGuiObj = new globTabColGuiC();
	$this->globColGuiObj->initTab($tablename);
}

function countElems( &$sql ) {
	$tablename = $this->tablename;
	
	if (table_is_view($tablename)) {
		this_info("Is view?", "YES");
	}
	
	if (!table_is_view($tablename)) {
		$sqls = "select count(1) from ".$tablename;
		$sql->query($sqls);
		$sql->ReadRow();
		$retid = $sql->RowData[0];
		$this->info( "Number of elements", $retid);
	}
}

function info ($key, $text, $notes=NULL ) {
    // FUNCTION: print out info text
    if ($notes!="")  $notes = " &nbsp;<I>".$notes."</I>";
    echo "<font color=gray>".$key.":</font> <B>".$text."</B>".$notes."<br>\n";

}

private function subHeader( $text, $notes=NULL ) {
	// echo "<tr bgcolor=#EFEFEF><td colspan=2>&nbsp;&nbsp;&nbsp;<font color=gray><b>".$text.":</b></font>";
	echo "<tr><td colspan=2>&nbsp;&nbsp;&nbsp;<font style='color:#336699; font-weight:bold; font-size:1.2em;'>".$text."</font>";
	echo "&nbsp;&nbsp;<font color=gray>".$notes."</font>";
	echo "<hr size=1 noshade color=\"#6699DD\">";
	echo "</td></tr>\n";
}

function subHeaderClose() {
	echo "<tr><td colspan=2>&nbsp;</td></tr>\n";
}

/**
 * show one SUB-table
 * @param unknown $xmodes
 *   KEY =>array(0=>text, 1:text, 2:url ) 
 */
function showTab( &$xmodes ) {
	
	foreach( $xmodes as $key=>$valarr) {
		$text1 = $valarr[2]=="" ? $valarr[1] : "<a href=\"".$valarr[2]."\">".$valarr[1]."</a>";
		$outhead = $valarr[0];
		if ($outhead!="")  $outhead .= ":";
		echo "<tr><td valign=top align=right><span style=\"".$this->ColKeyStyle."\">".$outhead."</span></td><td>".$text1."</td></tr>\n";
	}
	reset ($xmodes); 
		
}


function table_cct_table_inf( &$sql ) {
	$this->tableFeatures = NULL;
	$sqls = "select * from CCT_TABLE where TABLE_NAME='".$this->tablename."'";
	$sql->query($sqls);
	$sql->ReadArray();
	$this->tableFeatures = $sql->RowData;
}

function listAndSingle( &$sql ) {
	$tablename = $this->tablename;
	$this->subHeader("Main views");
	$xmodes=NULL;
	
	$xmodes[] =	array("List view","<img src=\"images/but.list2.gif\" TITLE=\"list\" border=0 vspace=2>", "view.tmpl.php?t=".$tablename );
	
	
	$hist_obj = new historyc();
	$lastobjects  = $hist_obj->getObjects();
	$tabsel = NULL;
	$tmpsep = "<br>\n";
	$tmptext = "<font color=gray>last objects:</font>";
	$objFound=0;
	
	foreach( $lastobjects as $key=>$valarr) {
		if ($valarr[0]==$tablename) {
			$tabsel[] = $valarr[1];
			$tmpid = $valarr[1];
			$tmptext .= $tmpsep . "<a href=\"edit.tmpl.php?t=".$tablename."&id=".$tmpid."\">".
				"<img src=\"images/arrow.but.gif\" border=0> ".obj_nice_name( $sql, $tablename, $tmpid)."</a>";
			$objFound=1;
		}
	}
	reset($lastobjects);
	
	if ( !$objFound ) $tmptext .= " no objects in history";
	$xmodes[] =	array("Single object view", $tmptext );
	
		 
	$this->showTab($xmodes);
	$this->subHeaderClose();
}

function _tableLink( $table, $withNotes=0 ) {
	$icon = htmlObjIcon($table, 1); // get object-icon: func_head.inc
	$tmptext = $tmpsep . "<a href=\"".$_SERVER['PHP_SELF']."?tablename=".$table."\">".
			"<img src=\"".$icon."\" border=0> ".tablename_nice2($table)."</a>";
	if ( $withNotes ) {
		$tmptext .= "&nbsp;&nbsp;&nbsp;<i>(".table_remark2($table).")</i>";
	}
	return ($tmptext); 
}

function _assocTables() {
	$tablename = $this->tablename;
	$assoc_tabs = &get_assoc_tables($tablename);
  	$tabIsBo    = cct_access_has2($tablename);

  	$tmptext="";
	$tmpsep="";

	if (!empty($assoc_tabs)) {
		
		foreach( $assoc_tabs as $tmp_table=>$featarr) {
		
			$nice_tmp_table   = $featarr['nice_name'];
			$is_view          = $featarr['is_view'];
			$is_bo            = $featarr['is_bo'];
			$showit = 1;
			if ( $is_view ) 	   $showit=0;
			if ( $showit ) {
				$tmptext .= $tmpsep . $this->_tableLink($tmp_table, 1);
				$this->tableAnalysed[$tmp_table] = 1;
				$tmpsep = "<br>\n";
			}
		}
	}

	if ($tabIsBo) {
		$tmp_table='SATTACH';
		$this->tableAnalysed[$tmp_table] = 1;
		$tmptext .= $tmpsep . $this->_tableLink($tmp_table, 1);
	}
		
	if ($tmptext!="") { 
		$xmodes =	array( array("Associated lists", $tmptext ) );
		$this->showTab($xmodes);
	}
	
}

function  _isAsscoTab( &$sql) {
	
	$tablename = $this->tablename;
	$tmptext = "";
	$sqls    = "select CCT_TABLE_NAME, PRIMARY_KEY, COLUMN_NAME from CCT_COLUMN ".
		"where TABLE_NAME='".$tablename."' AND PRIMARY_KEY>0 order by PRIMARY_KEY";
	$sql->query($sqls);
	$i=1;
	while ( $sql->ReadRow() ) {
	
		$foreign = $sql->RowData[0];
		$primkey = $sql->RowData[1];
		$colname = $sql->RowData[2];
		
		$tmptext .= $tmpsep . $i.". ".columnname_nice2($tablename, $colname);
		$tmpsep = ", ";
		$i++;
	}
	if ($tmptext=="") $tmptext="none";
	
	
	return ($tmptext);
}

function _usedByObj( &$sql ) {
	$tablename = $this->tablename;
	$tmptext = "";
	$sqls    = "select TABLE_NAME, COLUMN_NAME, PRIMARY_KEY from CCT_COLUMN where CCT_TABLE_NAME='".$this->tablename."'";
	$sql->query($sqls);
	$tmpsep = "";
	while ( $sql->ReadRow() ) {
	
		$foreign = $sql->RowData[0];
		$primkey = $sql->RowData[1];
		if ( $primkey==1 ) continue;
		if ( table_is_view($foreign) )    continue;
		if ( $this->tableAnalysed[$foreign] ) continue; // already analysed
		if (!table_exists2($foreign)) continue;
		
		$tmptext .= $tmpsep . $this->_tableLink($foreign, 1);
		$tmpsep = "<br>\n";
	}
	if ($tmptext=="") $tmptext="none";
	$xmodes = NULL;
	$xmodes[] =	array("Used by Objects", $tmptext );
	
	$this->showTab($xmodes);
}

function usedBy(&$sql) {

	$tablename = $this->tablename;
	$tableType = $this->tableFeatures["TABLE_TYPE"];
	$this->subHeader("Object relations");
	
	$this->tableAnalysed = NULL;
	
	if ( table_is_view($tablename) ) {
		$xmodes[] = array("Table is view", "no relations analysed");
		$this->showTab($xmodes);
		$this->subHeaderClose();
		return;
	}
	
	$this->_assocTables();
	
	if ( $tableType!="BO_ASSOC" AND $tableType!="SYS_META" ) {
		$this->_usedByObj( $sql );
	}
	
	$this->subHeaderClose();
}

private function _get_bg_image() {
   
    $tablename=$this->tablename;
    
    $bginfo = "<table cellpadding=0 cellspacing=0 border=0><tr valign=top>";
    
    $tablename_l = strtolower($this->tablename);
    $bgimg		= "images/obj.".$tablename_l.".jpg";
    
    if ( !file_exists($bgimg) ) {
        $bginfo .= "<td>none</td>";
    } else {
        $bginfo .='<td width=75 background="'.$bgimg.'"><img src="0.gif" height=75 width=75 title="Object-Passphoto"></td>';
    }
    $bginfo .="</tr></table>\n";
    return ($bginfo);
}

private function _get_bg_color() {
    global $_s_i_table;
    $tablename=$this->tablename;
    
    $tableColor = $_s_i_table[$tablename]['__table_data__']['COLOR'];
    $bginfo = NULL;
    if ($tableColor!=NULL) {
        $bginfo ='<span style="color: white; background-color:'.$tableColor.'"> &nbsp;&nbsp; Object-color &nbsp;&nbsp; </span>&nbsp;';
    }
    
    
   
    return ($bginfo);
}

function _getPassAndColor() {
	global $_s_i_table;
	$tablename=$this->tablename;
	
	$tableColor = $_s_i_table[$tablename]['__table_data__']['COLOR'];
	
	$bginfo = "<table cellpadding=0 cellspacing=0 border=0><tr valign=top>";
	
	$tablename_l = strtolower($this->tablename);
  	$bgimg		= "images/obj.".$tablename_l.".jpg";
  
	if ( !file_exists($bgimg) ) {
		$bginfo .= "<td>none</td>";
	} else {
		$bginfo .="<td width=75 background=\"".$bgimg."\"><img src=\"0.gif\" height=75 width=75></td>";
	}
	if ($tableColor!=NULL) {
		$bginfo .='<td>&nbsp;&nbsp;<span style="'.$this->ColKeyStyle.'">Table-color:</span>&nbsp;</td>';
		$bginfo .='<td style="background-color:'.$tableColor.'"><img src="0.gif" height=75 width=75></td>';
	}
	
	
	$bginfo .="</tr></table>\n";
	return ($bginfo);
}

function _varioInfo( &$sqlo ) {

	if (!glob_table_exists('S_VARIO') ) { // for code down-compatibility
		$outtext = "<font color=gray>table S_VARIO not exists</font>";
		return $outtext;
	}

	$varioLib = new oS_VARIO_sub($this->tablename);
	$keys = $varioLib->getAllKeysNice( $sqlo );

	
	if ($keys==NULL) {
		$outtext = "<font color=gray>none</font>";
		return $outtext;
	}

	$outtext = implode(", ", $keys);
	$outLink = "<a href=\"view.tmpl.php?t=S_VARIO_DESC&condclean=1&searchCol=TABLE_NAME&searchtxt=".$this->tablename."\">Vario column list:</a> ";
	
	$outtext = $outLink . $outtext;
	return ($outtext);
}

function _classInfo( &$sql ) {
	global $varcol;
	
	$classnames = $varcol->get_class_nice_names( $this->tablename );
	
	if ($classnames==NULL) {
		$outtext = "<font color=gray>none</font>";
		return $outtext;
	}
	
	$outtext = implode(", ", $classnames);
	$outLink = "<a href=\"view.tmpl.php?t=EXTRA_CLASS&condclean=1&searchCol=TABLE_NAME&searchtxt=".$this->tablename."\">Class list:</a> ";
	
	$outtext = $outLink . $outtext;
	
	return ($outtext);  
	
}

function columns( &$sql ) {

	$tablename = $this->tablename;
	
	$colnames = columns_get_pos($tablename);
	if (!$colnames) return;
	
	$extra_obj_exists = 0;
	$tmptxt = "<table cellpadding=1 cellspacing=1 border=0>";
	
	foreach( $colnames as $dummy=>$colName) {
	
		$colInfos = $this->globColGuiObj->anaColAll( $sql, $colName ); 
	
		if ($colName=="EXTRA_OBJ_ID") {
			$colInfos["showcol"]=1;
			$extra_obj_exists = 1;
		}
		
		if ( $colInfos["showcol"]>0 ) { 
	
			$nice_name  = $colInfos["nice"];
			$colcomment = "";
			$colcomment = column_remark2($tablename, $colName);
			if ($colName=="EXTRA_OBJ_ID") {
				$nice_name="CLASS";
				$colcomment="extra class";
			}
			
			if ( $colInfos["mother"]!="" ) {
				$niceout = $this->_tableLink( $colInfos["mother"] );
			} else {
				$icon = "<img src=\"images/icon._empty.gif\"> ";
				$niceout =  $icon.$nice_name;
			}
			
			 
	
			$tmptxt .= "<tr  valign=top>";
			$tmptxt .= "<td NOWRAP>".$niceout."</td><td>&nbsp;";
			$tmptxt .= "<I>". $colcomment ."</I>"; /*comment */
			$tmptxt .= "</td>";
			$tmptxt .= '<td style="color:#C0C0C0;">&nbsp;'.$colName."<td>";
			$tmptxt .= "</tr> \n";
		}
	}
	$tmptxt .= "</table>\n";
	
	$this->subHeader("Object columns", "[<a href=\"edit.tmpl.php?t=CCT_TABLE&id=".$tablename."\">column definitions</a>]");
	$xmodes = NULL;
	$xmodes[] =	array("Columns", &$tmptxt );
	
	if ($extra_obj_exists) {
		$classtxt = $this->_classInfo( $sql );
		$xmodes[] =	array("Classes", &$classtxt );
	}

	$tableType = $this->tableFeatures["TABLE_TYPE"];
	if ($tableType=='BO') {
		$variotxt = $this->_varioInfo( $sql );
		$xmodes[] =	array("Vario columns", $variotxt );
	}
	
	$xmodes[] =	array("", "" );
	
	$tableType = $this->tableFeatures["TABLE_TYPE"];
	if ( countPrimaryKeys($tablename)>1  ) {
		$tmptext = $this->_isAsscoTab($sql);
		$xmodes[] =	array("Primary keys", $tmptext );
	}
	
	$this->showTab( $xmodes );
	$this->subHeaderClose();
}

function _helpAnalyze() {

	$tablename = $this->tablename;
	
	$helpLib = new fHelpC();
	
	$helpSpace = array( "pionir"=>"Global help", "lab"=>"Lab specific help" );
	$helpInfo  = "";
	
	foreach( $helpSpace as $baseflag=>$nicespace) {
	
		$helpparam = "o.".$tablename.".html";
		$urlpath   = $helpLib->htmlCheckHelp( $helpparam, $baseflag );
		
		if ($urlpath=="") { 
			if ($baseflag!="lab") {
				$helpInfo .= "<font color=gray>[".$nicespace." not exists]</font> ";
			}
		} else {
			$helpInfo .= "[<a href=\"f.help.php?f=".$helpparam."&base=".$baseflag."\" target=_help>".$nicespace." exists</a>] ";
		}
		
	}
	reset ($helpSpace); 
	
	$helpInfo .= "&nbsp;[<a href=\"glob.objtab.graph1.php?t=".$tablename."\">Show Relation Graph</a>] ";
	
	return ($helpInfo);
}


private function show_main_feats()  {
    
    $tablename = $this->tablename;
    $icon       = htmlObjIcon($tablename, 1);
    $tableShort = globTabMetaByKey( $tablename, 'SHORT' );
    
    echo '<tr><td colspan=2>'."\n";
    echo '<table><tr>'."\n";
    echo '<td>'. $this->_get_bg_image().'</td>';
    echo '<td>'; 
    echo    '<table>'."\n";
    
    $bgcolor_img= $this->_get_bg_color();
    
    $xmodes = array(
        array( "Icon",  "<img src=\"".$icon."\"> &nbsp;&nbsp;&nbsp;". $bgcolor_img  ),
        array( "Name",  "<font size=+1><b>".tablename_nice2($tablename)."</b></font>" ),
        array( "Short-Name", $tableShort ),
        array( "Description",table_remark2($tablename) ),
   

    );
    $this->showTab($xmodes);
    echo    '</table>'."\n";
    echo '</td></tr>';
    echo '</table>'."\n";
    echo '</td></tr>'."\n";
}

function show_all( &$sql ) {
	
	
	$tablename = $this->tablename;
	  
	echo "<table cellpadding=2 cellspacing=1 border=0>";
	
	$this->table_cct_table_inf( $sql );
	$tmpType    = $this->tableFeatures["TABLE_TYPE"];
	
	
	
	$this->subHeader("Main description");
	
	$this->show_main_feats();

	$this->subHeaderClose();

	$this->columns($sql);
	$this->usedBy ($sql);
	
	$this->subHeader("Advanced description");
	$xmodes = array(
	    array( "Code-Name",  $tablename ),
	    array( "Documentation", $this->_helpAnalyze() ),
	    array( "Table type",  $this->typeTrans[$this->tableFeatures["TABLE_TYPE"]] ),
	);
	if ( $this->tableFeatures["CCT_TABLE_NAME"]!="" ) {
	    $xmodes[] = array("Mother-table", $this->_tableLink( $this->tableFeatures["CCT_TABLE_NAME"])  );
	}
	$this->showTab($xmodes);
	$this->subHeaderClose();
	
	$this->listAndSingle($sql);
	
	
	echo "</table>\n";
}

}


global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$tablename = $_REQUEST['tablename'];


// $i_tableNiceName = '???';
// if ($tablename) {
//     $i_tableNiceName = tablename_nice2($tablename);	
// }
$title           = "interactive help for this object type";

$infoarr			 = NULL;

$infoarr["title"]    = $title;
$infoarr["title_sh"] = "interactive help";
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;
$infoarr["design"]   = "slim"; 
// $infoarr['locrow']   = array( array('home.php','home'), array('ohome.php?t='.$tablename, $i_tableNiceName.' (home)') );

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);


if ($tablename==NULL) {
    htmlFoot('ERROR','No table given');
}

$prefsGuiLib = new fPrefsGuiC();
$prefsGuiLib->showHeadTabs( "INFO", $tablename );
echo "<ul>\n";

$mainLib = new fTableInfo( $tablename );
$mainLib->show_all($sql);


// $mainLib->countElems( $sql );
echo "<hr>";
htmlFoot();
