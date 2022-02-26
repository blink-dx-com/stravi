<?php
/**
 * - shows a graph of the table type structure
 * - relation to other tables
 * @todo use module gGraphRelWork.inc later
 * $Header: trunk/src/www/pionir/glob.objtab.graph1.php 59 2018-11-21 09:04:09Z $
 * @package glob.obj.graph1.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param string $t (tablename)
 * @param $parx[level] level of graph
 * @param $parx[showOpt] 0,1
 * @param string $parx[conTables]  : optional connecting table
 * @param string $parx["ignoreTabs"] : KOMMA separated tables of ignored tables
 */
session_start(); 

require_once ('reqnormal.inc');
require_once ('glob.obj.col.inc');

class gObjtabGraph {
	
	var $tablearr; /*
		store touched tables
		array[$tablename] = array( 
			'ok'   => 0,1
			'type' => 'assoc' 
			'follow' => -1, 0, [1] - follow the table
			)
		*/

function __construct($parx, $tablename) {
	$this->globColGuiObj = new globTabColGuiC();
	$this->parx = $parx;
	$this->ignoreTables = NULL;
	$this->conTableArr = NULL;
	$this->debug=0;
	
	if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
	    $this->debug=1;
	}
	
	if ($this->parx['level']<=0) {
		$this->parx['level'] = 3;
	}
	if ($this->parx['ignoreTabs']!=NULL) {
		$this->ignoreTables = explode(',',$this->parx['ignoreTabs']);
	}
	if ($this->parx['conTables']!=NULL) {
		$this->conTableArr = explode(',',$this->parx['conTables']);
	}
	
	$this->startTable = $tablename;
	$this->importantTables=NULL;
	
	
	$this->tablearr=NULL;
	$this->recText=NULL;
	$this->graphText=NULL;
	$this->graphArr=NULL; /* array[] = array('dest'=>table, 'text'=> text) */
	$this->graphid = intval(0);
	$this->startSeq();
}

function _debug($text) {
	 if (!$this->debug) return;
	 echo 'DEBUG: '.$text."<br />";
}

function startSeq() {
	// rankdir = graph ["LR" ]
	$test_str='
	digraph g {
	graph [
	rankdir ="LR"
	];
	node [
	fontsize = "10"
	shape = "ellipse"
	color=blue
	];
	edge [
	];'."\n"; 
	
	$this->outstr= $test_str;
	$this->endStr='}';
}

function setTable($tablename) {

	$this->tablename= $tablename;
	$oldInfo = $this->tablearr[$tablename];
	
	$this->tablearr[$tablename] = array( 'ok'=>1 );
	$nicename=tablename_nice2($tablename);
	
	if ( $this->importantTables[$tablename]>0 ) {
		$fillcolor = '#FFD0D0';
	} else $fillcolor = '#D0D0FF';
	
	$fillIt='style=filled fillcolor="'.$fillcolor.'" ';
	if ( $oldInfo['type']=='ASSOC' ) {
		$fillIt='';
		$newNode = '"no_'.$tablename.'" [
		label = "<f0> '.$nicename.'| <f1>"
		shape = "record" '.$fillIt.
		'];'."\n";
	} else {
		$newNode = '"no_'.$tablename.'" [
		label = "<f0> '.$nicename.'"
		shape = "record" '.$fillIt.
		'];'."\n";
	}
	$this->recText .= $newNode;
}

/**
 * set one graph from $tablename to $child
 * @param $type = 'BO', 'ASSOC'
 */
function setGraph($tablename, $child, $type=NULL) {
	$start='f0';
	if ($type=='ASSOC')  $start='f1';
	
	$oneArrow = '"no_'.$tablename.'":'.$start.' -> "no_'. $child.'":f0 [id = '.$this->graphid.'];'."\n";
	$this->graphArr[] = array('dest'=>$child, 'text'=>$oneArrow);
	$this->graphid++;
	
	if ($type=='ASSOC') {
		$oneArrow = '"no_'.$tablename.'":'.$start.' -> "no_'. $child.'":f1 [id = '.$this->graphid.'];'."\n";
		$this->graphArr[] = array('dest'=>$child, 'text'=>$oneArrow);
		$this->graphid++;
	}
	
	
	// initialize table
	$doStore=1;
	if (is_array($this->ignoreTables)) {
		if ( in_array($child, $this->ignoreTables) ) {
			$doStore=0;
		}
	}
	
	// will be evaluated later ...
	if (is_array($this->conTableArr)) {
		if ( in_array($child, $this->conTableArr) ) {
			$doStore=0;
		}
	}
	if ($this->tablearr[$child]==NULL and $doStore) {
		$this->_debug('init child: mother: '.$tablename.' child:'.$child);
		$this->tablearr[$child] = array( 'ok'=>0, 'type'=>$type ); // to be done
	}
}

function _assocTables() {
	$tablename = $this->tablename;
	$assoc_tabs = &get_assoc_tables($tablename);
  	$tabIsBo    = cct_access_has2($tablename);

  	$tmptext="";
	$tmpsep="";

	if (sizeof($assoc_tabs)) {
		
		foreach( $assoc_tabs as $tmp_table=>$featarr) {
		
			
			$is_view         = $featarr['is_view'];
			$is_bo           = $featarr['is_bo'];
			$is_intern           = $featarr['is_intern'];
			$showit = 1;
			if ( $is_view ) continue;
			if ( $is_intern ) continue;
			
			$this->setGraph( $tablename, $tmp_table, 'ASSOC' );
				
			
		}
	}
}

/**
 * analyse columns of table
 */
function oneTabColumns( &$sqlo ) {

	$tablename = $this->tablename;
	
	$this->globColGuiObj->initTab($tablename);
	$colnames = columns_get_pos($tablename);
	if (!$colnames) return;
	$denyCols = array('CCT_ACCESS_ID', 'EXRA_OBJ_ID');
	
	
	foreach( $colnames as $dummy=>$colName) {
	
		$colInfos = $this->globColGuiObj->anaColAll( $sqlo, $colName ); 
		$colInfosRaw = colFeaturesGet( $sqlo, $tablename, $colName, 0 ); 
		if ( $colInfos["showcol"]<=0 ) continue; 
	
		$nice_name  = $colInfos["nice"];
	
		if (in_array($colName, $denyCols)) {
			continue;
		}
		
		if ( $colInfos["mother"]==NULL ) {
			continue;
		} 
		
		if ($colInfosRaw['PRIMARY_KEY']==1) continue;
		
		$child = $colInfos["mother"];
		if ( substr($child,0,2)=='H_') {
			continue;
		}
		
		$this->setGraph( $tablename, $colInfos["mother"] );
			 
		
	}

}

/**
 * scan $this->tablearr for followed tables
 */
function oneTableScan(&$sqlo) {

	reset ($this->tablearr); 
	$keys = array_keys($this->tablearr);
	 
	foreach( $keys as $dummy=>$tablename) {
		$valarr= $this->tablearr[$tablename];
		if ($valarr['ok']>0)     continue;
		if ($valarr['follow']<0) continue; // do not follow
		if (is_array($this->ignoreTables)) {
			if ( in_array($tablename, $this->ignoreTables) ) {
				continue;
			}
		}
		$this->setTable($tablename);
		$this->oneTabColumns($sqlo );
		$this->_assocTables();
	}
	reset ($keys); 
}

/**
 * flag all tables, that in further scan, they are not followed 
 */
function _flagNoFollow() {
	
	if ($this->parx['followBlast']>0) return;
	
	reset ($this->tablearr); 
	$keys = array_keys($this->tablearr);
	 
	foreach( $keys as $dummy=>$tablename) {
		$valarr = $this->tablearr[$tablename];
		
		// there are important tables in this structure !
		if ( in_array($tablename, $this->conTableArr) ) {
			continue;
		}	
		$valarr['follow']=-1; 
		$this->tablearr[$tablename] = $valarr;
	}
	reset ($keys);
}

/**
 * build $this->graphText from $this->graphArr
 * 
 */
function _buildGraphText() {
	$this->graphText = '';
	if ($this->graphArr==NULL) return;
	
	reset ($this->graphArr);
	foreach( $this->graphArr as $key=>$valarr) {
		$table = $valarr['dest'];
		
		// table exist as finished object ?
		$tableFeat = $this->tablearr[$table];
		if ($tableFeat['ok']!=1) continue; // no!
		
		$this->graphText .= $valarr['text'];
	}
	reset ($this->graphArr); 
}

/**
 * finally create the graph
 */
function _outGraph(&$sqlo, $tablename) {
	$this->_buildGraphText();
	$allout = $this->outstr ."\n". $this->recText . "\n". $this->graphText."\n".$this->endStr."\n";
	
	if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
		echo "<B>DEBUG:</B> <pre> ".htmlSpecialchars($allout)."</pre><br>\n";
	}
	// echo $allout."<br>\n";
	
	$sqls   = "select VALUE from GLOBALS where NAME='exe.graphviz'";
	$sqlo->query("$sqls");
	$sqlo->ReadRow();   
	$GRAPHVIZ_EXE = $sqlo->RowData[0];
	
	if ( ! is_executable($GRAPHVIZ_EXE) ) { 
		echo 'ERROR:  executable "graph" not found.'; 
		return -1;
	}
	
	$tmp_graph_img= $_SESSION['globals']['http_cache_path'] ."/graph.".$tablename. ".png";
	$cmd = $GRAPHVIZ_EXE . " -Tpng -o" .$tmp_graph_img. " ";
	if ( $_SESSION["userGlob"]["g.debugLevel"]>2 ) {
		echo "<B>DEBUG:</B> CMD_GRAPHVIZ_EXE:<pre> ".htmlSpecialchars($cmd)."</pre><br>\n";
	}
	
	// $cmd2 = $GRAPHVIZ_EXE . " -Tismap -o" .$tmp_graph_map. " ";
	// echo $cmd."<br>\n";
  
   $fp = popen ($cmd, "w");
   $retval = fputs ( $fp, $allout );
   $retval = pclose ($fp); 
   
   $num=rand();
   
    echo "<img src=\"".$tmp_graph_img."?rando=".$num."\">";
}

function _showOpt() {
	require_once ('func_form.inc');
	
	$parx= $this->parx;
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Options";
	$initarr["submittitle"] = "Submit";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["t"]     = $this->startTable;

	$formobj = new formc($initarr, $hiddenarr, 0);

	$fieldx = array ( 
		"title" => "Level", 
		"name"  => "level",
		"object"=> "text",
		"val"   => $parx["level"], 
		"notes" => "1,2,3, .."
		 );
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ( 
		"title" => "Connect tables", 
		"name"  => "conTables",
		"object"=> "text",  'fsize' => "80",
		"val"   => $parx["conTables"], 
		"notes" => "e.g. CONTACT,CONCRETE_PROTO"
		 );
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ( 
		"title" => "Ignore tables", 
		"name"  => "ignoreTabs",
		"object"=> "text",  'fsize' => "80",
		"val"   => $parx["ignoreTabs"], 
		"notes" => "e.g. CONTACT,CONCRETE_SUBST"
		 );
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ( 
		"title" => "Follow all branches", 
		"name"  => "followBlast",
		"object"=> "checkbox",
		"val"   => $parx["followBlast"], 
		"notes" => "Follow all branches"
		 );
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );
}

/**
 * do a scan for one table
 */
function _manageOneTable(&$sqlo, $tablename) {
	if ( !isset($this->tablearr[$tablename]) ) $this->tablearr[$tablename]=NULL;
	$maxloop = $this->parx['level'];
	$cnt = 0;
	while ($cnt<$maxloop) {
		$this->oneTableScan($sqlo);
		$cnt++;
	}
}



function start(&$sqlo, $tablename) {
	
	if ( !$this->parx['showOpt']) {
		echo '[<a href="'.$_SERVER['PHP_SELF'].'?t='.$this->startTable.'&parx[showOpt]=1">Show options</a>] ';
	} else {
		$this->_showOpt();
	}
	echo 'Level: '.$this->parx['level'];
	if ( $this->parx['conTables']!=NULL ) {
		echo '; Connection table: '.$this->parx['conTables'];
	}
	if ( $this->parx['ignoreTabs']!=NULL ) {
		echo '; Ignored tables: '.$this->parx['ignoreTabs'];
	}
	echo "<br>";
	
	$this->importantTables[$tablename]=1;
	if ( $this->parx['conTables']!=NULL ) {
		reset ($this->conTableArr);
		foreach( $this->conTableArr as $dummy=>$tabLoop) {
			$this->importantTables[$tabLoop]=1;
		}
		reset ($this->conTableArr); 
		
	}
	
	$this->_manageOneTable($sqlo, $tablename);
	
	if ( $this->parx['conTables']!=NULL ) {
		
		reset ($this->conTableArr);
		foreach( $this->conTableArr as $dummy=>$tabLoop) {
			$this->_flagNoFollow();
			$this->_manageOneTable($sqlo, $tabLoop);
		}
		reset ($this->conTableArr); 
		
	}
	
	$this->_outGraph($sqlo, $tablename);
}

}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();

$tablename 		= $_REQUEST['t'];
$parx=$_REQUEST['parx'];

$title		= 'Shows a graph of the table type structure';

$infoarr			 = NULL;
$infoarr['scriptID'] = '';
$infoarr['title']    = $title;
$infoarr['title_sh']    = 'Table graph';
$infoarr['form_type']= 'list'; // 'tool', 'list'
$infoarr['design']   = 'norm';
$infoarr['obj_name'] = $tablename;
$infoarr['locrow']   = array( array('glob.objtab.info.php?tablename='.$tablename, ' interactive help') );
/*
$infoarr['obj_id']   = $_REQUEST['id'];
$infoarr['checkid']  = 1;

$infoarr['help_url'] = 'o.EXAMPLE.htm';
$infoarr['locrow']   = array( array('url', 'text') );
*/
$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo, $infoarr);

if ($tablename==NULL) $pagelib->htmlFoot('USERERROR', 'No table given');

$mainlib = new gObjtabGraph($parx, $tablename) ;
$mainlib->start($sqlo, $tablename);

$pagelib->htmlFoot();
