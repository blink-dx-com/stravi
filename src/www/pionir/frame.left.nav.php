<?php
/**
 * Manage the LEFT frame of the application frame set
 * 
 * - check for system message 'DbUserMessage'
 * - used css-classes:
 *   - yLFramHead
 *   - head1
 *   - ylfr2
 *   
 * @package frame.left.nav.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param <pre>
	$clearlist     : clear $_SESSION['s_history']
    $clearListhist : 1: clear list history $_SESSION['s_historyL']
                     2: resort 1
					 3: resort 2
					 9: shorter
	$histlist:    =0,1 show all
		          =2   list reduced
				  =3   hide
	$clearClip : =1: clear $_SESSION['s_clipboard']
	$mode      : [""]  normal history
                 "list" short list view
                 "proj" explorer mode => proj from $id
    $id        : ID of object (for $mode='proj')
	$amode 	   : => save in $_SESSION['s_sessVars']["frameL.amode"]
			     "lsearch"
    $imode     : 0|1 info mode
    $listtab   : tablename for $mode="list"
    $viewp     : view page number
	$startsessflag: 0|1
	$clearFuncClip : 0,1
	--- TREE params ---
    $clearTree : 0,1
	$tree_tab: tablename for tree element
	$tree_oid: object_id for tree
	$action:   "plus","minus"
	
 * </pre>
 * @global <pre>
 *   $_SESSION['s_funcclip']
     $_SESSION['s_sessVars']["frameL.mode"] = $mode
     $_SESSION['s_sessVars']["frameL.tab"] = tablename of shown table
     $_SESSION['s_tree'][$slice_cnt][$tab_key][$id]="ls".$level; 
	    " ", "l"       -leaf
	    "h", "s", "e"  -show
	    LEVEL  	 -level
	$_SESSION['userGlob']["g.histlist"]["shmode"]  : 1 - show, 2 - hide
	$_SESSION['s_historyL'],
	$_SESSION['s_history']
 * </pre> 
 * @version $Header: trunk/src/www/pionir/frame.left.nav.php 59 2018-11-21 09:04:09Z $
 */ 

// extract($_REQUEST); 
session_start(); 


require_once ('reqnormal.inc');
require_once ("o.DB_USER.subs2.inc");
require_once ("class.history.inc");
require_once ("frame.left.func.inc");

$clearlist  = $_REQUEST['clearlist'];
$clearListhist= $_REQUEST['clearListhist'];
$histlist  = $_REQUEST['histlist'];
$clearClip= $_REQUEST['clearClip'];
$mode = $_REQUEST['mode'];
$id   = $_REQUEST['id'];
$amode= $_REQUEST['amode'];
$imode = $_REQUEST['imode'];
$listtab  = $_REQUEST['listtab'];
$startsessflag= $_REQUEST['startsessflag'];

$clearTree= $_REQUEST['clearTree'];
$tree_tab= $_REQUEST['tree_tab'];
$tree_oid= $_REQUEST['tree_oid'];
$action= $_REQUEST['action'];

//   brown:     #9c7552
//   dark-blue: blue: #52759c

$_SESSION['s_sessVars']["frameL.mode"] = $mode;
// save mode in a session var ???
if (isset($listtab)) $_SESSION['s_sessVars']["frameL.tab"]  = $listtab;
$listtab = $_SESSION['s_sessVars']["frameL.tab"];

global $error, $varcol;
$error = & ErrorHandler::get();
 
if ( $mode=="proj" ) {
	// do not put this into a function
	
	if ($id=="") {
		$hist_obj = new historyc();
		$id       = $hist_obj->last_bo_get( "PROJ" );
	}
	$pmode    = "leftframe";
	
	$title = "Left frame: folder";
	$pagelib = new gHtmlHead();
	$infoarr=array();
	$pagelib->_PageHead ( $title,  $infoarr );
	
	?> 
	<script language='JavaScript'>
	
	var lastTag="";
	
	function goproj( id ) {
		location.href="<?php echo $_SERVER['PHP_SELF']?>?mode=proj&id=" + id;
	}
	
	function th_snc( tag ) {
	
		var newColor     = "#C0D0FF";
		var touchColor   = "#EFEFFF";
		
		if (lastTag!="") {
			document.getElementById(lastTag).style.backgroundColor = touchColor;
		}
		 
		if( (document.getElementById) && (document.getElementById( tag )!=null)) {
			var myElement = document.getElementById( tag );        
			if ((myElement.style)&& (myElement.style.backgroundColor!=null)) {            
				document.getElementById(tag).style.backgroundColor = newColor;
			}
		}   
		lastTag = tag;
	}
	
	</script>
	<?php
	
	$hoopt = NULL;
	$hoopt["back"] = 1; 
		
	frameLc::header3out("Explorer mode", $hoopt);
	$popt=array();
	$popt["noCheck"]=1;
	$popt["markObj"]=1;
	include("obj.proj.xsubst.inc");
	exit();
} 
 



$sql  = logon2( $_SERVER['PHP_SELF'] );
// WARNING: $sql2 will be opened only for treeview !!!
 
if ( $mode=="list" AND $listtab!="") {
    require_once("frame.left.list.inc");
    $sopt=NULL;
    $sopt["viewp"]=$_REQUEST['viewp'];
    frameList_showlist( $sql, $listtab, $sopt );
    exit();
} 


// required modules, which are only used here
require_once('f.s_historyL.inc'); 

// ---- start worker class

class gLframeC {

    function __construct() {
    	$this->subFuncs= array(
    		 "funcclip" =>"Function history",
    		 "listhist" =>"List history", 
    		 "objhist"  =>"Object history", 
    		 "treeview" =>"Tree", 
    		 "clip"     =>"Clipboard", 
    		 "morefunc" =>"More functions"
    		 );
    }
    
    private function space_line($height) {
        echo '<div style="line-height: '.$height.'px; min-height:'.$height.'px;"></div>'."\n"; 
    }
    
    /**
     * - check for system message
     * - must be accepted once per session
     * - show a warning frame
     * @global $_SESSION['s_sessVars']['DbUserMessage'] message read flags
     */
    function checkForMessage(&$sqlo) {
    	
    	
    	$mainKey='DbUserMessage';
    	$sqlsel = "VALUE from GLOBALS where NAME='".$mainKey."'";
    	$sqlo->Quesel($sqlsel);
    	if (!$sqlo->ReadRow()) return;
    	$message = $sqlo->RowData[0];
    	if ($message==NULL) return;
    	
    	// show message
    	$cutlen=15;
    	$firstchars = substr($message,0,$cutlen);
    	if ( $_SESSION['s_sessVars'][$mainKey]['txt']==$firstchars ) {
    		if ( $_SESSION['s_sessVars'][$mainKey]['read']>0 ) return;
    	} else {
    		// save first characters ...
    		$_SESSION['s_sessVars'][$mainKey]['txt'] =$firstchars;
    		$_SESSION['s_sessVars'][$mainKey]['read']=0;
    	}
    	
    	// message box
    	echo '<table border=0 cellspacing=1 cellpadding=0 bgcolor=#FF0000 style="margin:3px;"><tr><td>';
        echo "<table border=0 cellspacing=0 cellpadding=2 bgcolor=#FFFFFF><tr><td>";
        echo "<p class=xdark><b>System message!</b><br>";
        echo $message;
        echo '<br />[<a class=xdark href="sessVars_set.php?variable='.$mainKey.
        	'&key2=read&val=1&$backurl='.urlencode($_SERVER['PHP_SELF']).'">O.K.</a>]'."\n";
        echo '</p>';
        echo "</td></tr></table>\n";
        echo "</td></tr></table>\n<br />";
    	
    	
    }
    
    /**
     * show list history
     * @param array $info_arr
     * @return -
     */
    function showHistL( &$info_arr ) {
        
        $br='';
        echo '<div style="white-space: nowrap;">'."\n";
        foreach( $_SESSION['s_historyL'] as $tab_key1 => $tabInfoArr ) {
               
                // save for other tools in the frame
                echo $br;
                if ( empty($info_arr["$tab_key1"]) ) {
                    $info_arr["$tab_key1"]["IDN"]  = PrimNameGet2($tab_key1);
                    $info_arr["$tab_key1"]["NICE"] = tablename_nice2($tab_key1);
                }
                $nicename = $info_arr["$tab_key1"]["NICE"];
                echo "<img src=\"0.gif\" width=21 height=1>";
                $icon="images/icon.".$tab_key1.".gif";
                if ( !file_exists($icon) ) $icon="images/icon.UNKNOWN.gif";
                echo "<a href=\"view.tmpl.php?t=".$tab_key1."\" target=\"unten\">";
                echo "<img src=\"" .$icon. "\" border=0> &nbsp;";
                echo $nicename."</a>";
                if ($tabInfoArr['cnt']>0)  echo ' ('.$tabInfoArr['cnt'].')';
                $br = "<br>\n";
        }
        echo "</div>"."\n";
        
    }   
    
    function tabHeadShow(
    	$key, // funcclip, listhist, objhist, treeview, clip, morefunc, 
    	$rest, // 
    	$showButArr  // "link", "imgtitle", "ison"
    	) {
    	
    	$title    = $this->subFuncs[$key];
    	if ( $showButArr["ison"] ) $butimg  = "res/img/chevron-down.svg";
    	else $butimg   = "res/img/chevron-right.svg";
    	
    	$showButt = "<a href=\"".$showButArr["link"]."\">".
    			  '<img src="'.$butimg.'" TITLE="'.$showButArr["imgtitle"].'" height=20></a>';
    	echo "<table ><tr><td nowrap>"; // class=yLFramHead
    	echo "<span class=head1>&nbsp;".$showButt."&nbsp;".$title."</span> ";
    	echo $rest;
    	echo "</td></tr></table>\n";
    	$this->space_line(3);
    	
    }
    
    function tabEnd($endsize=0) {
    	if ($endsize>=0) {
    	    echo '<div style="line-height:5px;"></div>';
    		//echo '<br style="line-height: 4px;">'."\n"; // <img src=0.gif width=1 height=5><br>\n"; // special <hr>
    	}
    }
    
    function funcclip_show() {
    
    
    	if ( !empty($_SESSION['s_funcclip']) ) {
    		$tmpShowInfo = array("ison"=>1, "imgtitle"=>"clear clipboard", "link"=>"frame.left.nav.php?clearFuncClip=1" );
    		$this->tabHeadShow("funcclip", "", $tmpShowInfo);
            echo "<ul TYPE=\"SQUARE\" style='white-space: nowrap;'>\n";
            
            foreach( $_SESSION['s_funcclip'] as $onefunc ) {
    	 
                $tmp_text = $onefunc["t"];
    			$tmp_url  = $onefunc["u"];
    			$tmp_xtra = $onefunc["x"];
    			$endbr    = NULL;
    			if ( $_SESSION['s_sessVars']["g.funcclip"] == $onefunc["k"] ) {
    				echo "<b>";
    				$endbr = "</b>";
    			}
    			echo "<LI><a href=\"".$tmp_url."\" target=unten>".$tmp_text."</a> ".$tmp_xtra."</LI>".$endbr."\n";
    		}
    		
    		echo "</ul>\n";
    		$this->tabEnd();
    	}
    }
    
    function getAbsObjName( &$sql, $tablename, $id, $cname ) {
    	
    	if ($tablename=="CONCRETE_PROTO") {
    		$sqls="select a.name from CONCRETE_PROTO c, ABSTRACT_PROTO a ".
    			" where c.CONCRETE_PROTO_ID=".$id. " AND c.ABSTRACT_PROTO_ID=a.ABSTRACT_PROTO_ID";
    	} elseif ($tablename=="CONCRETE_SUBST") {
    		$sqls="select a.name from CONCRETE_SUBST c, ABSTRACT_SUBST a ".
    			" where c.CONCRETE_SUBST_ID=".$id. " AND c.ABSTRACT_SUBST_ID=a.ABSTRACT_SUBST_ID";
    	}
    	
    	$sql->query($sqls);
    	$sql->ReadRow();
    	if ($cname=="") $text = "[".$id."]";
    	else $text = $cname;
    	$text .= isset($sql->RowData[0]) ? ": ".$sql->RowData[0] : '';
    	return ($text);
    }
    
    /**
     * get name of object
     * @param object $sql
     * @param string $tablename
     * @param string $idname
     * @param int $id
     * @return array (string NICENAME, INT number of PKs)
     */
    function tmp_getname( &$sql, $tablename, $idname, $id ) {
    
      $key_num = countPrimaryKeys($tablename);
      
      if (!glob_table_exists($tablename)) {
      	return array('', 1);
      }
      
      if ($key_num > 1) {
    	$nice_name = tablename_nice2($tablename);
    	
    	$sql->query('SELECT COUNT(1) FROM '.$tablename.' WHERE '.PrimNameGet2($tablename).' = '.$sql->addQuotes($id));
    	$sql->ReadRow();
    	$text = $nice_name.': '.$sql->RowData[0].' elements';
      } else {
    	$main_name = importantNameGet2($tablename);
    	if ( $main_name == '') $main_name = $idname;
    
    	$sql->query('SELECT '.$main_name.' FROM '.$tablename.' WHERE '.$idname.' = '.$sql->addQuotes($id));
    	$sql->ReadRow();
    	$text = isset($sql->RowData[0]) ? $sql->RowData[0] : '';
    	
    	if ($tablename=="CONCRETE_PROTO" || $tablename=="CONCRETE_SUBST") { // show (name or id) and abstract proto
    		$text = $this->getAbsObjName( $sql, $tablename, $id, $text );
    	}
    
      }
      return array($text, $key_num);
    }
    
    function clipShow(&$sql) {
    	
    	
    	$tmpShowInfo = array("ison"=>1, "imgtitle"=>"clear clipboard", "link"=>"frame.left.nav.php?clearClip=1" );
    	$this->tabHeadShow("clip", "" , $tmpShowInfo );
    	
    	if ( $_SESSION['s_sessVars']["clipActCutProj"] ) {
    		echo " - cutted from folder: <a href=\"edit.tmpl.php?t=PROJ&id=".
    			$_SESSION['s_sessVars']["clipActCutProj"]."\" target=\"unten\">";
    		$projid = $_SESSION['s_sessVars']["clipActCutProj"];
    		if ($projid!='NULL') echo obj_nice_name( $sql, "PROJ", $projid);
    		echo "</a><br>";
    	}
    	$cnt = 0;
    	$clipShowMax = 20;
    	$numelem     = sizeof($_SESSION['s_clipboard']);
    	reset($_SESSION['s_clipboard']);
    	
    	foreach( $_SESSION['s_clipboard'] as $clip_filed ) {
    		$tab_key  = ($clip_filed['tab'] == 'PROJ_ORI') ? 'PROJ' : $clip_filed['tab'];
    		$nicename = tablename_nice2($tab_key);
    		echo '&#8226; <a href="edit.tmpl.php?t='.$tab_key.'&id='.$clip_filed['ida'].
    			 '&primasid[1]='.$clip_filed['idb'].'&primasid[2]='.$clip_filed['idc'].
    			 '" target="unten">'.$nicename.' '.$clip_filed['ida'].' '.$clip_filed['idb'].'</A><br>';
    		$cnt++;
    		if ($cnt > $clipShowMax) {
    			echo '... and '.($numelem-$clipShowMax).' more elements.<br>';
    			break;
    		}
    	}
    	
    	if ($cnt <= $clipShowMax)  echo "<br>";
    	
    	$this->tabEnd();
    }
    
    function ShowHistory( 
    		&$sql,
    		&$hist_obj,
    		$clearListhist,
    		$histActstr,
    		$histlist_now,			
    		&$info_arr		
    		) {
    	// FUNCTION: show object history
    					
    	
    	
    	$gifarr= array(2=>14, 3=>12, 4=>10, 5=>"08", 6=>"06");
    	// $this_obj_text = "";
    	$tmp_hliclear = "<a href=\"frame.left.nav.php?clearlist=1\"><img src=\"res/img/x.svg\" width=20 TITLE=\"clear list\"></a> ";
        
        $tmprest = "";
    	$endsize = 0;
    	$tmpShowInfo = array("ison"=>1, "imgtitle"=>"hide", "link"=>"frame.left.nav.php?histlist=3" );
        switch ( $histlist_now ) {
    		case 2:
            	$tmprest  = $tmp_hliclear.'<img src="images/but.listlessNo.gif" border=0> '."\n";  
            	$tmprest .= '<a href="frame.left.nav.php?histlist=1"><img src="images/but.listmore.gif" border=0 TITLE="show more"></a> '."\n";
    			break;
    		case 3:	// is hidden
    			$tmprest  =  '';
    			$tmpShowInfo = array("ison"=>0, "imgtitle"=>"show", "link"=>"frame.left.nav.php?histlist=1" );
    			$endsize = -1;
    			break;
    		default: // is shown
            	// $tmprest  = $tmp_hliclear.'<a href="frame.left.nav.php?clearListhist=2"><img src="images/but.sort1.gif" border=0 TITLE="resort by visit-history"></a> '."\n"; 
    			$tmprest  = $tmp_hliclear;
    			$tmprest .= '<a href="frame.left.nav.php?clearListhist=3"><img src="res/img/align-justify.svg" height=20 TITLE="sort by type"></a> '."\n"; 
            	$tmprest .= '<a href="frame.left.nav.php?clearListhist=9"><img src="res/img/chevrons-up.svg" height=20 TITLE="show less"></a> '."\n";
    	} 
    	
    	
    	// $this->space_line(); // extra space
    	$this->tabHeadShow("objhist", $tmprest, $tmpShowInfo);
    	
        if ( $clearListhist>0 ) {
    		echo "<img src=\"0.gif\" width=21 height=1>... ".$histActstr."<br>\n";
    	}
    	if ( $histlist_now < 3 and is_array($_SESSION['s_history']) ) {
    		
    		echo "<table cellpadding=0 cellspacing=0 border=0 width=100%>";
    		// echo "<nobr>";
    		$boThisString = $_SESSION['s_sessVars']["boThis"]["t"].":".$_SESSION['s_sessVars']["boThis"]["id"];
    		$imgSpace = "<img src=\"0.gif\" width=21 height=1>";
    		
    		$sortidarr = $hist_obj->showByPush();
    
    		foreach($sortidarr as $hi=>$dummy) {
    		
    			$valarr  = $_SESSION['s_history'][$hi];
    			
    			$isact   = "ylfr2";  // default css-style
    			$tab_key1= key($valarr);
    			$value1  = current($valarr);
    			$lastbo   = $valarr["last"];
    			$tmpord  =$valarr["o"];
    			
    			if ( ($histlist_now<=1) || $lastbo ) {
    				$imgSpaceNow = $imgSpace;
    				if ( empty($info_arr["$tab_key1"]) ) {
    					$info_arr["$tab_key1"]["IDN"]  = PrimNameGet2($tab_key1);
    					$info_arr["$tab_key1"]["NICE"] = tablename_nice2($tab_key1);
    				}
    				$idname   = $info_arr["$tab_key1"]["IDN"];
    				$nicename = $info_arr["$tab_key1"]["NICE"];
    				
    				list ($obj_text, $key_num) = $this->tmp_getname($sql, $tab_key1, $idname, $value1 );
    				if ( $obj_text=="" ) $obj_text ="[$value1]";
    				$icon="images/icon.".$tab_key1.".gif";
    				if ( !file_exists($icon) ) $icon="images/icon.UNKNOWN.gif";
    				
    				if ($tmpord>8)  $isact   = "ylfr1";  // less color
    				if ($tmpord>15)  $isact  = "ylfr0";  // less color
    				
    				if ( $tmpord > 1 AND $tmpord <= 6 ) {
    					$ballgif = $gifarr[$tmpord];
    					$imgSpaceNow = "<img src=\"images/ic.ball".$ballgif.".gif\" hspace=3 TITLE=\"used previously\"><img src=\"0.gif\" width=2 height=1>";
    				}
    				if ( $boThisString ==  $tab_key1.":".$value1 ) { // last object ?
    					$isact = "ylfr3";
    					//$this_obj_text = $obj_text; // save the name
    					$imgSpaceNow = "<img src=\"images/ic.ball14n.gif\" hspace=3 TITLE=\"now\"><img src=\"0.gif\" width=2 height=1>";
    				}
    				echo "<tr><td class=\"$isact\">";
    				echo "<a href=\"edit.tmpl.php?t=".$tab_key1."&id=".$value1."\" class=\"$isact\" target=\"unten\">";
    				echo $imgSpaceNow;
    				echo "<img src=\"" .$icon. "\" title=\"".$nicename."\"> &nbsp;";
    				echo $obj_text."</a>";
    				echo "</td></tr>\n";
    			}
    	
    		}
    		
    		echo "</table>\n"; // </nobr>
        	
    	}
    	$this->tabEnd($endsize);
    }
    
    function histHandleFlags( $histlist ) {
    	// return: "shmode" - flag
    	
    	$globKey = 'g.histlist';
    	
    	if (isset($histlist) && ($histlist !== '')) {
    		$tmparr = unserialize($_SESSION['userGlob'][$globKey]);
    		$tmparr["shmode"] = $histlist;
    		$_SESSION['userGlob'][$globKey] = serialize($tmparr);
    	}
    	$tmparr = unserialize($_SESSION['userGlob'][$globKey]);
    	
    	return ($tmparr["shmode"]);
    }
    
    /**
     * NEW: PHP-converted each ..
     * @param object $sql
     * @param object $hist_obj
     * @param string $imode
     */
    function shMoreFunc(&$sql, &$hist_obj, $imode) {
        
        $this_tab = $_SESSION['s_sessVars']["boThis"]["t"];
        $this_id  = $_SESSION['s_sessVars']["boThis"]["id"];
        
        echo "<ul>";
        $lastproj = $hist_obj->last_bo_get( "PROJ" );
        
        if ( !$imode AND $lastproj ) {
            /*
            echo "<a href=\"javascript:newtree('PROJ', '".$lastproj."')\">";
            echo "<img src=\"images/ic.treevmi1.gif\" title=\"tree view\">";
            echo " Current project as tree</a><br>\n";
            */
        }
        if ($imode) {
            if (!empty($_SESSION['s_historyL'])) {
                $tab_key1 = key( $_SESSION['s_historyL'] );
                reset($_SESSION['s_historyL']);
            }
            
            if ($lastproj) {
                echo "<a href=\"javascript:newtree('PROJ', '".$lastproj."')\">";
                echo "<img src=\"images/ic.treeview.gif\" title=\"tree view\">";
                echo " Current folder as tree</font></a><br>\n";
            }
            if ($this_id) {
                $this_obj_text = obj_nice_name    ( $sql, $this_tab, $this_id);
                echo "<a href=\"javascript:newtree('".$this_tab."', '".$this_id. "')\">".
                    '<img src="images/ic.treeview.gif"> Tree </a> of "'.$this_obj_text.'"' ."<br>\n";
            }
            echo "<a href=\"".$_SERVER['PHP_SELF']."?amode=lsearch&listtab=".$tab_key1."\"><img src=\"images/ic.listsearch.gif\"> Search in list</a><br>\n";
            echo "<a href=\"obj.proj.search.php?id=".$lastproj."&f=slim\"><img src=\"images/ic.projsearch.gif\"> Search in folder</a><br>\n";
            echo "<a href=\"".$_SERVER['PHP_SELF']."?mode=proj&id=".$lastproj."\"><img src=\"images/ic.projexpl.gif\"> Explorer</a><br>\n";
            echo "<a href=\"help/robo/pionir_lframe.html\" target=help><img src=\"images/help.but.gif\" hspace=7> Help for navigation</a><br>\n";
        }
        echo "</ul>\n";
    }

}

$mainScriptObj = new gLframeC();
$extraOptions  = "";
//if (sizeof($_SESSION['s_tree']) ) $extraOptions  = " onload=\"jumpTreeObj()\"";
if (isset($amode)) $_SESSION['s_sessVars']["frameL.amode"] = $amode;
$fhopt = array("noBody" => 1);
$title = "Left frame";
$pagelib = new gHtmlHead();
$pagelib->_PageHead ( $title,  $fhopt );

echo "<body class=\"yBodyDark\"".$extraOptions.">\n";

?>
<script language="JavaScript">

	
	function gobo(tablename, id) {
		parent.frames['unten'].location.href="edit.tmpl.php?t="+tablename+"&id="+id ;
	}
	
	
	function golist(tablename, id) {
		parent.frames['unten'].location.href="view.tmpl.php?t="+tablename+"&searchMothId="+id ;
	}

	function newtree(tablename, id) {
		location.href="<?php echo $_SERVER['PHP_SELF']?>?clearTree=1&tree_tab="+tablename+"&tree_oid="+id ;
	}
	
	function jumpTreeObj() {
		if (document.anchors["theobject"]) {
			document.anchors["theobject"].focus();
			return true;
		}
    }

</script>

<style type="text/css">
   .head1 { font-weight:bold; color:#B0B0B0;}
   p.xdark { color:#000000;}
   a.xdark { color:#0000FF;}
   a        { color: #FFFFFF; }
   ul       { margin-bottom: 2px; margin-top: 2px; }
   hr       { color: #808080; background-color: #808080; height:1px; border:1px; }
   img      { border-width:0px}
   .thmark   { background-color: #22356c;  font-weight:bold; padding-right:8px;}
   
   .svgnavi { fill: white; }
</style>
<?php
// bg: #7295bc
if ( $_SESSION['s_sessVars']["frameL.amode"]=="lsearch" AND $listtab!="") {
	require_once("subs/frame.left.lsearch.inc");
    this_lsearch( $sql, $listtab );
    // exit();
}

// first space line
frameLc::header3out( "history mode" );
$hist_obj = new historyc();
$lastproj = $hist_obj->last_bo_get( "PROJ" );


$mainScriptObj->checkForMessage($sql);

if (!empty($_REQUEST['clearFuncClip']))   $_SESSION['s_funcclip']=array();
	
if (!empty($_SESSION['s_funcclip'])) {
	$mainScriptObj->funcclip_show();
}

$histActstr = "";
if ( $clearListhist=="1" ) {
	
	$listLib = new historyList();
    $listLib->reset();
	$histActstr = "cleared";
}

if ( $clearListhist=="9" ) {
   $hist_obj->shorter( );
   $histActstr = "shorter";
}



if ( $clearListhist=="2" ) {  
	$lastsort = $_SESSION['s_sessVars']["g.histsort"]; // did sort before ?
    $lastsort = $hist_obj->resort( $lastsort );
	$histActstr = "sorted by click history";
} 
if ( $clearListhist=="3" ) {  
	$lastsortY = $_SESSION['s_sessVars']["g.histsortY"]; // did sort before ?
    $lastsortY = $hist_obj->resort2( $lastsortY );
	$histActstr = "sorted by object type";
} 

if ( $clearListhist<=0 ) {
    $lastsort = 0; // no sort 
	$lastsortY= 0;
}



$_SESSION['s_sessVars']["g.histsort"]  = $lastsort;  // remember
$_SESSION['s_sessVars']["g.histsortY"] = $lastsortY; // remember

reset ($_SESSION['s_history']); 
if (!empty($clearlist)) {
	$lastproj  = $hist_obj->last_bo_get( "PROJ" );
 	$_SESSION['s_history'] = array();
	$hist_obj->historycheck("PROJ", $lastproj);
}
 
 if ( empty($_SESSION['s_history']) ) { /* try to get home project */
 	$proj_id = oDB_USER_sub2::userHomeProjGet($sql);
	if ( $proj_id ) {
	  $hist_obj->historycheck( "PROJ", $proj_id );
	}
 }



$histlist_now = $mainScriptObj->histHandleFlags($histlist);

$info_arr = array();

if ( !empty($_SESSION['s_historyL']) ) {   
    //<img src="images/but.list3.gif"> 
	$tmpShowInfo = array("ison"=>1, "imgtitle"=>"clear list", "link"=>"frame.left.nav.php?clearListhist=1" );
    $mainScriptObj->tabHeadShow("listhist", "", $tmpShowInfo);    
    
    $mainScriptObj->showHistL( $info_arr ); 
	//$mainScriptObj->tabEnd();
}
 
if (!empty ($_SESSION['s_history']) ) {
	$mainScriptObj->ShowHistory( $sql, $hist_obj, $clearListhist, $histActstr, $histlist_now, $info_arr);		
}
 
if (!empty($clearTree)) $_SESSION['s_tree']=array();
$treesize = 0;
if (!empty($_SESSION['s_tree'])) $treesize = sizeof($_SESSION['s_tree']);

if ( $treesize OR $tree_tab OR $tree_oid) {  

    $sql2 = logon2( $_SERVER['PHP_SELF'] );
	
	$tmpShowInfo = array("ison"=>1, "imgtitle"=>"clear tree", "link"=>"frame.left.nav.php?clearTree=1" );
	$mainScriptObj->tabHeadShow("treeview", "", $tmpShowInfo);
	echo '<div style="white-space: nowrap;">'."\n";
 
    require_once('gui/go.treeview.inc');
    
	$treeobj = new treeViewC( );
	$treeobj->do_action($sql, $sql2, $tree_oid, $tree_tab, $action);
	$first_elem = $treeobj->get_root_obj();
	
	$level=1;
	$sh_opt=array();
	$tree_show_lib = new treeView_SHOW( $treeobj, $_SERVER['PHP_SELF']  );
	$tree_show_lib->show_start($sh_opt);
	$tree_show_lib->show( $sql, $first_elem['id'], $first_elem['t'], $level);
	
	
	echo '</div>'."\n";
}    
if ( $treesize ) {
    $mainScriptObj->tabEnd();
}

if (isset($clearClip)) {
  $_SESSION['s_clipboard']=array();
}


if ( !empty($_SESSION['s_clipboard']) ) {
	$mainScriptObj->clipShow($sql);
}

$tmprest = "";
	$tmpShowInfo = array("ison"=>1, "imgtitle"=>"hide", "link"=>"frame.left.nav.php" );
if ($imode<=0) 
	$tmpShowInfo = array("ison"=>0, "imgtitle"=>"show more", "link"=>$_SERVER['PHP_SELF']."?imode=1" );
$mainScriptObj->tabHeadShow("morefunc", $tmprest, $tmpShowInfo);

$mainScriptObj->shMoreFunc($sql, $hist_obj, $imode);



?>
<script language="JavaScript">
	jumpTreeObj();

</script>
</body>
</html>
