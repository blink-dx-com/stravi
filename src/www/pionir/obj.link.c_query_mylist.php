<?php
/**
 * manage/show MY SEARCHES (myquery center) document class "query"
 * @package obj.link.c_query_mylist.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $t : searchtable to be destination  (optional)
			$proj_id ( other project_id to show here )
 */

//extract($_REQUEST); 
session_start(); 


require_once ('db_access.inc');
require_once ('globals.inc');
require_once ('func_head.inc');
require_once ('access_check.inc');
require_once ('table_access.inc'); 
require_once ('o.PROJ.paths.inc');      

require_once("o.LINK.c_query_subs.inc");
require_once("gui/o.LINK.c_query_listGui.inc");

class oLinkMyQueryC {

    function __construct($title, $infoarr, $proj_id, $t) {
    	$this->title  = $title;
    	$this->infoarr  = $infoarr;
    	$this->proj_id  = $proj_id;
    	$this->tablename = $t;
    }
    
//     function error_stop() { 
//         global $error;
        
     
//         echo "<blockquote>";
//         $error->printAll(); 
//         echo "</blockquote>";
//         htmlFoot("Stopped.");
//     }  
    
    private function _show_linked_projs( &$sql, &$sql2, $queryProj) {   
            
    
        $sqls=   "select x.proj_id, x.name, a.db_user_id from proj x, cct_access a where x.PROJ_ID in (".
                 "  SELECT prim_key FROM proj_has_elem WHERE proj_id = ".$queryProj." AND 
                           table_name='PROJ'".
                 "  ) and a.CCT_ACCESS_ID=x.CCT_ACCESS_ID order by x.name";
        
        $sql->query($sqls); 
        $cnt = 0;
        while ( $sql->ReadRow() ) {
            $tmpdir 	= $sql->RowData[0]; 
            $tmpname 	= $sql->RowData[1]; 
            $dbuserid 	= $sql->RowData[2];
            
            $username = "";
            $sql2->query("select nick from db_user where db_user_id='".$dbuserid."'");
            if ( $sql2->ReadRow() ) {
                $username = $sql2->RowData[0];
            } 
            $icon='images/icon.PROJ_link.gif';  
            echo '<a href="'.$_SERVER['PHP_SELF'].'?proj_id='.$tmpdir.'"> ';
            echo '<img src="'.$icon.'" border="0"> '.$tmpname.'</a> ('.$username.')<br>';
            $cnt++;   
        }
        return ($cnt);   
    } 
    
    
    function  pathx( &$sql, &$sql2, $id, $myQueryHomeProj ) {
    	
    	$desturl = $_SERVER['PHP_SELF']."?proj_id=";
    	$optx    = array();
    	$optx["relPioPath"] = "../pionir/";
    	
    	$projPathObj = new oPROJpathC($optx);
    	echo "<table width=100% cellpadding=1 cellspacing=1 border=0 bgcolor=#EFEFEF><tr><td width=5>&nbsp;</td><td>";
    	
    	if ( $myQueryHomeProj!=$id ) {
    		$icon='images/but.projhome.gif';  
            echo '<a href="'.$_SERVER['PHP_SELF'].'?proj_id='.$myQueryHomeProj.'"> ';
            echo '<img src="'.$icon.'" border="0"> <B>home</B></a><br><hr size=1 noshade>'."\n";
            
            $projPathObj->showPath($sql, $id, $desturl);
            echo "<br>";
    	}
    	
    	
    	$projPathObj->showSubProjs($sql, $id, $desturl);
    	
    	$this->_show_linked_projs( $sql, $sql2, $id);
    	
    	echo "</td></tr></table>\n";
    	
    }
    
    function checks( &$sql ) {
    	/*OUT: 
    		$this->myQueryHomeProj
    		$this->nowProj
    		$this->projname
    		$this->sql_extra1
    	*/
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$proj_id = $this->proj_id;
    	
    	
    	
    	$t_rights = tableAccessCheck( $sql, 'LINK' );
    	if ( $t_rights['read'] != 1 ) {
    		$error->set($FUNCNAME, 1, 'no role right "READ" for object type document! Ask your admin.');
    		return;
    	} 
    	  
    	    
    	
    	$queryObj = new myquery();
    	if ( $error->got(READONLY) ) {
    	    $error->set($FUNCNAME, 1, 'Error on Init.');
    	    return;
    	}
    	
    	$this->myQueryHomeProj = $queryObj->getProfQueryProj( $sql );   
    	if ( $error->got(READONLY) ) {
    		$error->set($FUNCNAME, 1, 'Sorry, you can not use this tool.');  
    		return;
    	}
    	
    	if ( !$this->myQueryHomeProj ) {
    		$error->set($FUNCNAME, 1, 'No personal query folder found');
    		return;
    	}  
    	
    	$this->projname = NULL;
    	if ( $proj_id>0 ) {
    		$this->nowProj = $proj_id;
    		$sqls = "select x.name from proj x where x.PROJ_ID=".$this->nowProj;
    		$sql->query($sqls); 
    		if ( $sql->ReadRow() ) {
    			$this->projname = $sql->RowData[0];      
    		} else {
    			$error->set($FUNCNAME, 1, 'folder '.$proj_id.' not found.'); 
    			return;
    		}
    	} else {
    		$this->nowProj = $this->myQueryHomeProj;
    	}
    	
    	$this->queryListGui = new oLINK_c_query_listGui($this->nowProj, $this->tablename);
    	    
    }
    
    function showTable( &$sqlo, &$sqlo2, $headerAfter) {
    	echo $headerAfter."\n";  
    	
    	echo "<br>";
    	$editmode=1;
    	$cnt = $this->queryListGui->showTable( $sqlo, $editmode );
    	
    	if (!$cnt) {
    		htmlFoot();
    	}
    }



}


// ------------------------------------------------------------------------------------------
$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol    = & Varcols::get(); 

$title = 'My Search Center';  

$t = $_REQUEST['t'];
$proj_id = $_REQUEST['proj_id'];

$infoarr = array();  
$infoarr["scriptID"] = "";
$infoarr["title"]    = $title;
$infoarr["help_url"] = 'o.LINK.class.query.html';
$infoarr['icon']	= "images/ic.myqueryLogo.40.png";
$infoarr["form_type"]= "tool";
$infoarr["locrow"] = array( array("home.php", "home") );

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

$searchtable  = $t;
//$thisProjInfo = "";
$headtext 	  = ""; // Link header
?>
<style type="text/css">
    tr.smaller   { font-size:1.0em; }
</style>
<?

$mainlib = new oLinkMyQueryC ($title, $infoarr, $proj_id, $searchtable);
$mainlib->checks( $sql );
if ($error->printAll()) {
	$errorFlag=1;
} else $errorFlag=0;
  
$headtext =  "&nbsp;[<a href=\"edit.tmpl.php?t=PROJ&id=".$mainlib->nowProj."\">manage searches</a>] ";
$headtext .= "&nbsp;[<a href=\"glob.obj.qsearch.php\">Quick search</a>] ";

$headerAfter = ""; 
if ($searchtable!="") {
    $headtext .= "&nbsp;[<a href=\"searchAdvance.php\">Search advanced</a>] ";
    $headtext .= "[<a href=\"".$_SERVER['PHP_SELF']."\">show all queries</a>] ";   // without $t
    $nice_stable = tablename_nice2($searchtable);
    $headtext = $headtext. "[<a href=\"view.tmpl.php?t=".$searchtable."\">list view: $nice_stable</a>] ";   // without $t
    $headerAfter = "<font color=gray>Show queries for table <B>$nice_stable</B></font> &nbsp;&nbsp;[<a href=\"".$_SERVER['PHP_SELF']."\">all tables</a>]<br>\n";
}      

echo  $headtext; 
echo "<br>";

gHtmlMisc::func_hist( "obj.link.c_query_mylist", $title, $_SERVER['PHP_SELF'] );

if ($errorFlag) $pagelib->htmlFoot();

echo "<blockquote>";       
if ( $_SESSION['userGlob']["g.debugLevel"]>0 ) {
    echo "<B>INFO:</B> DEBUG-mode supports 3 levels in this script.<br>\n";
}


$mainlib->pathx( $sql, $sql2, $mainlib->nowProj, $mainlib->myQueryHomeProj );

$mainlib->showTable($sql, $sql2, $headerAfter);
$error->printAll();

htmlFoot();
