<?php
/**
 * search for objects in project recursivly
 * - support also search for all SUB-projects
 * use cache tables: T_PROJ, T_PROJ_ELEM
 * SESSION-vars:
 *   $_SESSION['s_sessVars']["o.PROJ.search.cacheclean"]
 * @package obj.proj.search.php
 * 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param  $id        - PROJ_ID , "NULL" is not allowed
         $sname     - search_name
         $tablename - search for special objects
            'PROJ_ORI' : search the SUB-projects
         $f         - format "slim": no frame
		 $opt[useold] - 0|1 : if 1 => use last-search-cache content; saves time
		 $go	    - 0|1
 * 
 */

session_start(); 


require_once ('reqnormal.inc');
require_once ('date_funcs.inc');
require_once ('insert.inc');
require_once ('frame.left.func.inc');
require_once('o.CCT_TABLE.info.inc');
require_once 'o.T_PROJ.manage.inc';



class oPROJsearch_GUI {
    static function errorSoft($errtext) {
        htmlErrorBox("Info", $errtext);
        echo "<br><B>Alternative:</B> \n";
        echo " [<a href=\"view.tmpl.php?t=PROJ\" target=unten>Flat project search</a>]\n";
        echo " [<a href=\"help/robo/o.PROJ.search.html\" target=help>help</a>]<br>\n";
        htmlFoot();
    }
    
    function init_GUI($projid, $cacheProjID, $cacheFoundOldProj) {
        $this->cacheProjID = $cacheProjID;
        $this->projid   = $projid;
        $this->cacheFoundOldProj = $cacheFoundOldProj;
       
    }
    
    function show_form(&$sql, $numElems, $projStatUrl, $sname, $tablename, $f) {
        
        $cacheProj = $this->cacheProjID;
        $id = $this->projid;
        $projName = obj_nice_name ( $sql, 'PROJ', $this->projid );
        $table_nice = tablename_nice2('PROJ');
        
        $TabLib   = new oCCT_TABLE_info();
        $nicename = $TabLib->getTablesNiceByType($sql, 'BO');
       
        
        //echo "<br>\n";
        echo "<form method=\"post\"  name=\"editform\"  action=\"".$_SERVER['PHP_SELF']."?id=$id&go=1\" >\n";
        echo "<table border=0 cellpadding=2><tr><td>";
        echo "<font size=-1>Object name:</font><br>";
        echo "<input name=sname value=\"".$sname."\"><br><br>";
        echo '<select name="tablename" >';
        echo '<option value="">- select type -'."\n";
        
        $loopTable='PROJ_ORI';
        $nice     = $table_nice; 
        $selected = "";
        if ($loopTable==$tablename) $selected = " selected";
        echo "<option value=\"".$loopTable."\" $selected>".$nice."\n";
        
        echo '<option value=""> -----';
        foreach( $nicename as $loopTable=>$nice) {
            $selected ="";
            if ($loopTable=='PROJ') $nice = $table_nice.' LINK';
            if ($loopTable==$tablename) $selected = " selected";
            echo "<option value=\"".$loopTable."\" $selected>".$nice."\n";
        }
        echo "</select><br><br>\n";
        echo "<font size=-1>Look in:</font><br>\n";
        
        echo "<table border=0 cellpadding=0 bgcolor=#EFEFEF><tr><td>";
        echo "<a href=\"edit.tmpl.php?t=PROJ&id=$id\" target=unten><font color=black><img src=\"images/icon.PROJ.gif\" border=0> ".
            $projName."</font></a>&nbsp;";
            
            echo "</td></tr></table>\n";
            
            echo "<br><input type=submit value=\"Search now\">\n";
            
            if ( $this->cacheFoundOldProj ) $useOldCheckbox = " checked";
            else  $useOldCheckbox = "";
            
            echo "<br><input type=checkbox name=\"opt[useold]\" value=\"1\" ".$useOldCheckbox.
            "> <font color=gray> use last-search-cache?</font>\n";
            echo "<input type=hidden name=f value=\"".$f."\">\n";
            echo "</td></tr></table>\n";
            echo "</form>";
            
            echo "<hr noshade size=1>";
            
            echo "<table border=0 cellpadding=4><tr><td NOWRAP>";
            echo "<font color=gray><B>Help</B> [<a href=\"help/robo/o.PROJ.search.html\" target=help>more help</a>]<br>";
            echo "- searches recursively in ".$table_nice."<br>";
            echo "- use <B>%</B> as wild cards in 'Object name'<br>";
            echo "- name search is case insensitive<br>";
            echo "- name can be empty<br>";
            echo "- opens a list of found objects<br>\n";
            echo "<br><B>Other actions</B><br>\n";
            if ($numElems) echo "- <a href=\"".$projStatUrl."\" target=unten>count objects in sub-".$table_nice."s</a><br>\n";
            echo "- <a href=\"edit.tmpl.php?t=T_PROJ&id=$cacheProj\" target=unten>go to the list of sub ".$table_nice."s</a><br>\n";
            echo "- <a href=\"view.tmpl.php?t=PROJ\" target=unten>Flat ".$table_nice." search</a><br>";
            echo "</font>\n";
            echo "</td></tr></table>\n";
    }
}

// -----------------------------------------------------------------------------

 
$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
// $sql2  = logon2(  );
if ($error->printLast()) htmlFoot();  

$id        = $_REQUEST['id'];
$sname     = $_REQUEST['sname'];
$tablename = $_REQUEST['tablename'];
$f         = $_REQUEST['f'];
$opt	   = $_REQUEST['opt'];
$go 	   = $_REQUEST['go'];

$table_nice = tablename_nice2('PROJ');

if (!$go) {
	 $opt["useold"] = 1; // try to use last-search-cache ...
}
if ($_REQUEST['tablename']=='PROJ_ORI') {
	$opt["search_other"] = 'PROJ_ORI'; // search the sub-projects
	$tablename = 'PROJ';
}

$mainObj = new oT_PROJ_manage( $opt );


$title = 'Search in '.$table_nice.' recursivly';
$infoarr			 = NULL;
$infoarr["scriptID"] = "";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "obj"; 
$infoarr['help_url'] = 'o.PROJ.html';
$infoarr["obj_name"] = "PROJ";
$infoarr["obj_id"]   = $id;

$pagelib = new gHtmlHead();
$pagelib->_PageHead ( $infoarr["title"],  $infoarr );


if ( $f !="slim" ) {  
    $pagelib->_startBody( $sql, $infoarr );
    echo "<blockquote>";
    
} else {

    $hoopt = NULL;
	$hoopt["back"] = 1; 
	$hoopt["addlink"] = " <a href=\"edit.tmpl.php?t=PROJ&id=$id\" target=unten TITLE=\"back\"><img src=\"images/icon.PROJ.gif\" border=0></a>";
	frameLc::header3out("Search in ".$table_nice, $hoopt);
	$bgcolor = "#6699DD";
    if ($bgcolor=="") $bgcolor="#808080";
	
	/*
	echo "<table border=0 bgcolor=".$bgTopColor." width=100%><tr><td><img src=\"images/ic.frmmode.gif\" hspace=5>";
	echo "<B><font color=".$HeadTxtColor.">Search in project</font></B>";
	echo " <a href=\"edit.tmpl.php?t=PROJ&id=$id\" target=unten TITLE=\"back\"><img src=\"images/icon.PROJ.gif\" border=0></a>";
	echo " <a href=\"frame.left.nav.php\"><img src=\"images/close.but.gif\" border=0 hspace=5></a>";
	echo "</td></tr></table>\n";
	echo "<img src=\"0.gif\" height=4 width=2><br>\n";
	*/

} 

$mainObj->init($sql);
$pagelib->chkErrStop();

if (!$id) oPROJsearch_GUI::errorSoft("No ".$table_nice." given"); 
if ($id=="NULL") {
    oPROJsearch_GUI::errorSoft("Recursive search is<br>not allowed from ".$table_nice."-ROOT");
}


if (!$_SESSION['s_sessVars']["o.PROJ.search.cacheclean"]) {
    echo "<font color=gray>...clear old cache</font><br>";
    $mainObj->removeOldCacheProjs( $sql );
    $_SESSION['s_sessVars']["o.PROJ.search.cacheclean"]=1;
}


$cacheProj = $mainObj->get_cache_proj( $sql, $id.".proj" );       

if (!$cacheProj)  oPROJsearch_GUI::errorSoft("Cache ".$table_nice." not found");             

// touch cache project as used
$mainObj->touch_cache( $sql, $cacheProj ); 

$rescanTmp = 1;
$numElems  = $mainObj->hasElems ( $sql, $cacheProj );
if ( $mainObj->appopt["useold"]>0 )  {
	$rescanTmp = 0;
}
if (!$numElems) {
	$rescanTmp = 1;
	$mainObj->appopt["useold"] = 0;
}

if ( $rescanTmp ) {
	$mainObj->cacheCleanUp( $sql, $cacheProj );  
	$numElems = 0;
}
$projStatUrl = "obj.proj.substat.php?cacheid=".$cacheProj."&oriprojid=".$id;
   
if ($error->printLast()) htmlFoot();

// ... now current session project created
 

if ( $go ) {  

	$mainObj->buildTree( $sql, $id );
	
	// analyse the $cacheProj
    $numElems = $mainObj->hasElems ( $sql, $cacheProj );
    echo "<font color=gray>...<B>$numElems</B> ".$table_nice."s found.</font><br>\n"; 
	$fram_destination = ""; 
    if ( $f=="slim" ) $fram_destination="parent.unten.";  // if runs in left frame!
    
    if ($tablename!="") { 
    	
    	$forward_param = $mainObj->select_table($sql, $tablename, $cacheProj, $f, $sname );
    	
    	if ($forward_param!=NULL) {
    		// forward
    		$url = 'view.tmpl.php?t='.$tablename.'&condclean=1&tableSCond='.$forward_param;
	    	if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
				echo "&nbsp;(Automatic Forward stopped due to debug-level)<br>\n";
				echo '<a href="'.$url.'">URL</a> (HTML-Frame:'.$fram_destination.')';
				return;
			}
    		?>
    		<script language="JavaScript">
    		<?echo $fram_destination?>location.href="<?php echo $url;?>";
    		</script>
    		<? 
    	}
            
    } else {
		// show statistics of objects
						
		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
			echo "&nbsp;(Automatic Forward stopped due to debug-level)<br>\n";
			echo '<a href="'.$projStatUrl.'">URL</a>';
			return;
		}		
		?>
            <script language="JavaScript">
            <?echo $fram_destination?>location.href="<?echo $projStatUrl?>";
            </script>
        <? 
				
		if ( $f!="slim" ) {
            echo "<center><br><B>Show object statistics</B><br></center>";
        }
	}
}

list($cacheProjID, $cacheFoundOldProj) = $mainObj->get_vars();

$gui_lib = new oPROJsearch_GUI();
$gui_lib->init_GUI($id, $cacheProjID, $cacheFoundOldProj);
$gui_lib->show_form($sql, $numElems, $projStatUrl, $sname, $tablename, $f);
	

echo "</blockquote>";

htmlFoot();

?>
