<?php
/**
 * Project Selection Form: Select objects from the project structure
 * $Header: trunk/src/www/pionir/obj.proj.selector.php 59 2018-11-21 09:04:09Z $
 * @package obj.proj.selector.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param [$id] $project ID can be NULL
		  $tablename (show objects of type)
		  $cctgoba : string of HTML-id for caller object
		  $fullpath : [0],1 give back full path name (Not Yet Implemented!)
		  $action ['select'] , 'forward' - forward project link
 */

//extract($_REQUEST); 
session_start(); 


require_once("reqnormal.inc");
require_once("object.subs.inc");
require_once("class.history.inc");
require_once("table_access.inc");
require_once("db_x_obj.inc");
require_once("view.tmpl.inc");
require_once("subs/view.tmpl.extra2.inc");
require_once("o.PROJ.subs.inc");
require_once("o.DB_USER.subs2.inc");
require_once("o.proj.profile.inc");

/**
 * main class for the project selector form
 * @author steffen
 *
 */
class O_PROJ_selector {
	var $query_condition;
	var $proj_id; // set in getProject()
	var $tablename;
	var $cctgoba;
	var $t_rights;
	var $proj_fea;
	private $viSubObj;
	
	function __construct($sqlo, $tablename, $cctgoba) {
		$this->tablename=$tablename;
		$this->proj_id = 0;
		$this->cctgoba = $cctgoba;
		
		$this->infox = array();
		
		$hist_obj = new historyc();
		$this->projFromHist = $hist_obj->last_bo_get( "PROJ" );
		
		$this->t_rights = tableAccessCheck( $sqlo, "PROJ" );
	}
	
	function init($sql, $id_ini) {
	    global $error;
	    
	   
	    $existFlag = $this->getProject($sql, $id_ini);
	    
	    $this->infox["homeProj"] = oDB_USER_sub2::userHomeProjGet($sql);
	    
	    $projProfObj = new profile_funcs ();
	    $actionxc   = "CREATE";
	    $projBookId = $projProfObj->getProjBySyn( $sql, "bookmarks", $actionxc );
	    $error->reset();
	    $this->infox["projBookId"] = $projBookId;
	    
	    $this->viSubObj = new viewSubC($this->tablename);
	    $formback = array( 'fid'=>$this->cctgoba );
	    $this->viSubObj->prSetPar1( $sql, $formback );
	    $this->viSubGUI_extra2 = new view_tmpl_extra2($this->viSubObj);
	    
	    return $existFlag;
	}
	
	function forward(&$sql, $id) {
		
	}
	
	function _show_default_proj() {
		
		echo "<br> Select an other folder: [<b><a href=\"".$_SERVER['PHP_SELF'].
			"?tablename=".$this->tablename."&cctgoba=".$this->cctgoba."&id=NULL\">root folder</a></b>] ";
		// get home project
		
	}
	
	private function _projCheckExist(&$sql, $id) {
		if ($id=="NULL") return (1); 
		if (!$id) return 0;
		$sql->query("select name from proj where proj_id=". $id);
	  	if ( !$sql->ReadRow() ) {
			return 0;
		} else {
			return 1;
		}
	     
	}
	
	/**
	 * get current project
	 * @param $sql
	 * @param $id
	 * @return array ($existFlag, $id)
	 */
	function getProject($sql, $id) {
		
		$this->proj_id = NULL;
		
		do {
			if (!$id) $id = $_SESSION['userGlob']["o.".$this->tablename.".projBrowse"];
			$existFlag = $this->_projCheckExist($sql, $id);
			if ($existFlag) break;
			
			$id = $_SESSION['s_sessVars']["boProjBrowse"];
			$existFlag = $this->_projCheckExist($sql, $id);
			if ($existFlag) break;
			
			$id = $this->projFromHist;
			$existFlag = $this->_projCheckExist($sql, $id);
			if ($existFlag) break;
			
			// use ROOT-project !
			$id='NULL';
			$existFlag = 1;
			
		} while (0);
		
		$this->proj_id = $id;
		
		return $existFlag;
	}
	
	function show_history($sqlo) {
	    $this->viSubGUI_extra2->history_select( $sqlo, $this->tablename);
	}
	
	/**
	 * show project link
	 * @param $sql
	 * @param $projid
	 * @param $linkSepIc
	 * @param $type  "", "home", "bookmarks" 
	 */
	function showproj(&$sql, $projid, $linkSepIc,$type=NULL ) {
		
		
		$cctgoba  =$this->cctgoba;
		$tablename=$this->tablename;
		$url = $_SERVER['PHP_SELF']."?tablename=$tablename&cctgoba=$cctgoba&id=$projid";
		
		$sql->query("select name from proj where proj_id=". $projid);
		if ( $sql->ReadRow() ) {
			$projidName = $sql->RowData[0];
			switch ($type) {
				case "home":
					$projidName=""; // do not show name
					$icon ="res/img/folder.home.svg";
					$icon_t = '<img src="'.$icon.'" height=30 title="my home folder">';
					break;
				case "bookmarks":
					$projidName=""; // do not show name
					$icon ="res/img/heart.yel.svg";
					$icon_t = '<img src="'.$icon.'"  height=30 title="bookmarks">';
					break;	
				case "create":
				    $url = 'obj.proj.edname.php?action=create&mother_proj='.$projid;
					$projidName=""; // do not show name
					$icon ="res/img/folder-plus.svg";
					$icon_t = '<img src="'.$icon.'" height=30 title="new folder">';
					break;
					
				default:
					$icon="res/img/folder.yel.svg";
					$icon_t = '<img src="'.$icon.'" height=30 title="">';
				
			}
			echo ' <a href="'.$url.'"> '.$icon_t.' '.$projidName.'</a> ' ; // .$linkSepIc
		}
	}
	
	function proj_error( $errortext, $showprojs=0 ) {
		echo "<br>";
		htmlInfoBox( "Problem", $errortext, "open", "WARN" );
		if ($showprojs) $this->_show_default_proj();
		htmlInfoBox( "", "", "close" );
	}
	
	// show project links
	function _showProjLinks(&$sql, &$sql2, $id) {
		$maxLines=1000;
		echo "<tr><td colspan=2  NOWRAP>\n";
		$sqls_where = " x.PROJ_ID IN (select PRIM_KEY from PROJ_HAS_ELEM where PROJ_ID=".$id." AND TABLE_NAME='PROJ' )";
		$sqls  = "select x.PROJ_ID, x.NAME from PROJ x where ". $sqls_where . " order by x.NAME";
	
		$sql->query($sqls);
		$projcnt=0;
		$icon = "images/icon.PROJ_link.gif";
		while ( $sql->ReadRow() ) {
		  if ($projcnt>$maxLines) {
			 echo " ... <br />";
			 break;
		  }	
		  $sub_projid = $sql->RowData[0];
		  $name       = $sql->RowData[1];
		  $values = access_data_get( $sql2, "PROJ", "PROJ_ID", $sub_projid );
	
		  echo "<tr><td NOWRAP> ";
	      echo "<img src=\"images/0.gif\" width=16 height=2> ";  // placeholder
		  echo "<a href=\"javascript:goproj(".$sub_projid.")\"><img src=\"$icon\" border=0> $name</a></td>";
		  echo "<td NOWRAP>&nbsp;&nbsp;".$values["crea_date"]."</td>";
	
		  echo "</tr>\n";
		  $projcnt++;
		}
	}	
	
	/**
	 * 
	 * @param object $sqlo
	 * @return int
	 *     1 ok
	 *    -1 NOT ok
	 */
	function check_proj($sqlo) {
	    
	    $id = $this->proj_id;
	    
	    $this->proj_fea=array();
	    
	    $this->o_rights=array();
	    $this->o_rights["write"] ="0";
	    $this->o_rights["read"]  ="1";
	    $this->o_rights["insert"]="0";
	    $this->o_rights["delete"]="0";
	    
	    $query_condition = "PRO_PROJ_ID is NULL";
	    
    	if ( $id!="NULL" ) {
    	    
    	    $query_condition = "PRO_PROJ_ID = $id";
    	    $sqls= "select name, pro_proj_id, notes from proj  where proj_id=". $id ;
    	    $sqlo->query($sqls);
    	    if ( $sqlo->ReadRow() ) {
    	        $this->proj_fea['NAME'] = $sqlo->RowData[0]; // $proj_name 	  
    	        $this->proj_fea['PRO_PROJ_ID'] =  $sqlo->RowData[1]; // $master_proj_id =
    	        $this->proj_fea['NOTES'] = $sqlo->RowData[2]; // $proj_notes
    	       
    	    } else {
    	        $this->proj_error("Folder with ID=$id not found.", 1);
    	        return -1;
    	    }
    	    
    	    
    	    $this->o_rights = access_check( $sqlo, "PROJ", $id );
    	    if ( $this->t_rights["write"] != 1 ) {
    	        $this->o_rights["write"] = 0; // has higher priority
    	    }
    	    
    	    if ( !$this->o_rights["read"] ) {
    	        $this->proj_error("Folder [ID=$id] : NO READ PERMISSION !", 1);
    	        return -2;
    	    }
    	    
    	    //$acc_info = access_data_get( $sqlo, "PROJ", "PROJ_ID", $id );
    	    //$editAllow = $this->o_rights["write"];
    	}
    	$this->query_condition = $query_condition;
    	
    	
    	
    	return 1;
	}
	
	function header_row($sql) {
	    
	    
	    $tablename = $this->tablename;
	    $id = $this->proj_id;
	    $nicename_sel = tablename_nice2($tablename);

	   
	    $linkSepIc = "<img src=\"images/ic.sepWhit.gif\" hspace=3>";
	    
	    echo '<div style="width: 100%;  margin: 0; padding-left: 7px; padding-bottom: 5px; background-color:#E0E0E0">';
	    
	    
        // <form method="post" name="obj_act" action=" $_SERVER['PHP_SELF'] ?proj_id= $id &tablename= $tablename ">
 
	    echo "<B>Select $nicename_sel</B> &nbsp;"; // .$linkSepIc
        
        $this->showproj($sql, $id, $linkSepIc, "create");
        if ($this->infox["homeProj"])   $this->showproj($sql, $this->infox["homeProj"], $linkSepIc, "home");
        if ($this->infox["projBookId"]) $this->showproj($sql, $this->infox["projBookId"], $linkSepIc, "bookmarks");
        if ($this->projFromHist)        $this->showproj($sql, $this->projFromHist, $linkSepIc);

        echo ' &nbsp;&nbsp;<a href="edit.tmpl.php?t=PROJ&id='.$id.'" title="Switch to edit mode"><img src="images/but.edit.gif"></a>';
 
        
        echo '</div>'."\n";
        // echo '</form>'."\n";
	}
	
	function show_proj_path($sql) {
	    
	    echo '<table border=0 bgcolor="#EFEFEF" CELLSPACING=0 CELLPADDING=3 width=100%><tr><td valign=top>'."\n";
	    $tablename = $this->tablename;
	    $id = $this->proj_id;
	    $master_proj_id = $this->proj_fea['PRO_PROJ_ID'];
	    
	    $proj_name = $this->proj_fea['NAME'];
	    $proj_notes = $this->proj_fea['NOTES'];
	    
	    $projSubLib = new cProjSubs();
	    $pather    = $projSubLib->getPathArr( $sql, $master_proj_id );
	    $depth_cnt = sizeof($pather);
	    
	    
	    
	    echo '<tr><td bgcolor=#EFEFEF>';
	    $dummy=0;
	    if ($tablename=='PROJ') {
	        $this->viSubObj->_selShowOne( $sql, $proj_name, $id, $dummy );
	    }
	    //echo '<a href="javascript:goproj(\''. $linkto_proj.'\')" ><img src="images/but.projup.gif" TITLE="up" border=0></a>';
	    
	    echo "<font color=gray><B>Folder:</B></font> ";
	    if ($id!="NULL") {
	        echo "<a href=\"javascript:goproj('NULL')\">db:</a>/";
	    }
	    $cnt=$depth_cnt-1;
	    while ( $cnt >= 0) {
	        $master_proj_id= $pather[$cnt][0];
	        $master_name=    $pather[$cnt][1];
	        echo "<a href=\"javascript:goproj(".$master_proj_id.")\" >". $master_name . "</a>/";
	        $cnt--;
	    }
	    echo "<B>$proj_name</B>/</td></tr>\n";
	    if ( $proj_notes ) echo "<tr><td bgcolor=white><I><pre><font color=gray>".$proj_notes."</font></pre></I></td></tr>";
	    echo '</table>'."\n";
	}
	
	function showProjects( &$sql, &$sql2 ) {
		$proj_id = $this->proj_id;
		
		$sql->query("SELECT proj_id, name, extra_obj_id, notes FROM proj WHERE ".$this->query_condition." ORDER BY name");
		$projcnt=0;
		$icon = "images/icon.PROJ.gif";
		while ( $sql->ReadRow() ) { /*TBD: introduce limit to show */
		  $sub_projid = $sql->RowData[0];
		  $name       = $sql->RowData[1];
		  //$eobj_id    = $sql->RowData[2];
		  //$notes      = $sql->RowData[3];
		  $values = access_data_get( $sql2, "PROJ", "PROJ_ID", $sub_projid );
		  
		  echo "<tr><td NOWRAP>  ";
		  if ($this->tablename=="PROJ") {
		        echo "<a href=\"javascript:sel_obj_proj( '".$sub_projid."', '".$name."' )\">".
		        	'<img src="images/leftsel.but.gif" border=0></a> ';
		  } else {
			    echo "<img src=\"images/0.gif\" width=16 height=2> ";  // placeholder
		  }
		  echo "<a href=\"javascript:goproj(".$sub_projid.")\"><img src=\"$icon\" border=0> $name</a></td>";
		  echo "<td NOWRAP>&nbsp;&nbsp;".$values["crea_date"]."</td>";
		
		  echo "</tr>\n";
		  $projcnt++;
		}
		
		if ( $proj_id!=NULL and $proj_id!='NULL' ) {
			$this->_showProjLinks($sql, $sql2, $proj_id);
		} 
		
	}
	
	/**
	 * show all objects: PROJ + other
	 * @param object $sql
	 * @param object $sql2
	 */
	function show_proj_objects($sql, $sql2) {
	    
	    $id = $this->proj_id;
	    $tablename=$this->tablename;
	    
	    $table_pk_name = PrimNameGet2($tablename);
	    $important_col = importantNameGet2($tablename);
	    if ($important_col=="") $important_col=$table_pk_name;
	    
	  
	    
	    echo '<table CELLPADDING=0 width=100% border=0 style="margin-left:3px;">'."\n";
	    
	    $this->showProjects($sql, $sql2);
	    
	    
	    $maxLines=1000;
	    
	    if ($id !="NULL" and $tablename!="PROJ") {
	        
	        echo "<tr><td colspan=2  NOWRAP>\n";
	        
	        
	        $sqls_where = " x.$table_pk_name IN (select PRIM_KEY from PROJ_HAS_ELEM where PROJ_ID=".$id." AND TABLE_NAME='".$tablename."' )";
	        $sqls_main  = "select x.$table_pk_name, x.".$important_col.", x.CCT_ACCESS_ID from  $tablename x where ". $sqls_where . " order by x.". $important_col;
	        
	        $sql->query($sqls_main);
	        $cnt=1;
	        
	        // $cntobj=0; /* BO counter */
	        
	        $objclass=array();
	        $tmp_tablename="$tablename";
	        $icon="images/icon.".$tmp_tablename.".gif";
	        if ( !file_exists($icon) ) $icon="images/icon.UNKNOWN.gif";
	        
	        while ( $sql->ReadRow()  ) { /* TBD: introduce limit to show */
	            
	            if ($cnt>$maxLines) {
	                echo " ... <br />";
	                break;
	            }
	            
	            $obj_id = $sql->RowData[0];
	            $name	= $sql->RowData[1];
	            $access_id	= $sql->RowData[2];
	            
	            if ( $objclass[$tmp_tablename]== "" ) {
	                $tablename_l=strtolower($tmp_tablename);
	                $filename = "obj.".$tablename_l.".xfunc.inc";
	                $retu = file_exists($filename);
	                if ( $retu ) {
	                    require_once($filename);
	                    $tmp_func= "c".$tmp_tablename;
	                    $objclass[$tmp_tablename] = new $tmp_func();
	                } else {
	                    $objclass[$tmp_tablename] = 0; /* not exists */
	                }
	            }
	            
	            $tmp_Name=$name;
	            if ($name=="") $tmp_Name="[".$obj_id."]";
	            $this->viSubObj->_selShowOne( $sql2, $name, $obj_id, $access_id );
	            
	            
	            echo '<a href="edit.tmpl.php?tablename='.$tablename. '&id=' .$obj_id. '"><img src="'.$icon.'" border=0"> ';
	            
	            if ( method_exists ($objclass[$tmp_tablename], 'inProjShowAtt' ) ) {
	                $tmp_optinfo = $objclass[$tmp_tablename]->inProjShowAtt( $sql2, $obj_id );
	            }
	            echo $tmp_Name.'</a> '. $tmp_optinfo.'<BR />'."\n";
	            
	            
	            $cnt++;
	        }
	        
	        echo "\n\n";
	        
	        
	        /****************************************************************************/
	        
	        echo "</nobr>";
	        echo "</td>";
	        echo "</tr>\n";
	        
	        if (!$_SESSION['s_sessVars']["boProjSel"]) $_SESSION['s_sessVars']["boProjSel"] = $id;
	    }
	    echo "</table>\n";
	}

}

global $error;

$error = & ErrorHandler::get();
$sql  = logon2( $_SERVER['PHP_SELF'] );
$sql2 = logon2( $_SERVER['PHP_SELF'] );

$tablename=$_REQUEST['tablename'];
$id=$_REQUEST['id'];
$fullpath=$_REQUEST['fullpath'];
$cctgoba=$_REQUEST['cctgoba'];
$action=$_REQUEST['action'];

$nicename_p   = tablename_nice2('PROJ');
$nicename_sel = tablename_nice2($tablename);

$pagelib = new gHtmlHead();
$headopt=array("noBody" =>1);
$pagelib->_PageHead('Folder: select '.$nicename_sel, $headopt); 

echo '<body marginwidth="0" marginheight="0" bgcolor=white >'."\n";

?>
<script language='JavaScript'>
<!--


function open_info( url )   {				
  InfoWin = window.open( url, "help","scrollbars=yes,width=650,height=500,resizable=yes"); 
  InfoWin.focus();				
}


function goproj( id ) {
  url_name="<?echo $_SERVER['PHP_SELF']?>?tablename=<?echo $tablename?>&cctgoba=<?echo $cctgoba?>";
  location.href=url_name+ "&id=" + id;
  
}

function writeit(dest_id, value, name) {
		window.opener.inputRemote( dest_id, value, name ); 
		window.close(); 
}

function sel_obj( value, name ) {
	window.opener.inputRemote( '<?echo $cctgoba?>', value, name ); 
	window.close();
}

function sel_obj_proj( id, name ) {
	<?
	if ($fullpath) {
		echo 'location.href = "obj.proj.selector.php?id="+ id + "&action=forward&tablename=PROJ&cctgoba='.$cctgoba;
	} else {
		echo "sel_obj( id, name );\n"; // normal selection
	}
	?>
}
function viewTree() { // view project structure as tree
  location.href = "obj.proj.viewTree.php?id=<?echo $id?>";
}

//-->
</script>
<?

$mainLib = new O_PROJ_selector($sql, $tablename, $cctgoba);

if ( $action=='forward') {
	$mainLib->forward($sql, $id);
}

$existFlag = $mainLib->init($sql, $id);

if ($tablename=="") {
	$mainLib->proj_error("Need an object type to select.");
	htmlFoot();
}

if (!$existFlag) {
	$mainLib->proj_error("No folder found from last selection or from history",1);
	htmlFoot();
}

$_SESSION['s_sessVars']["boProjBrowse"] = $id;
$_SESSION['userGlob']["o.".$tablename.".projBrowse"]=$id; /* save for next session */


if ( $mainLib->t_rights["read"] != 1 ) {
  tableAccessMsg( $nicename_p, "read" ); 
  return 0;
} 



$answer = $mainLib->check_proj($sql);
if ($answer<=0) htmlFoot();


$mainLib->header_row($sql);

echo '<div style="margin-top:10px;"></div>'."\n";
$mainLib->show_history($sql);

$mainLib->show_proj_path($sql);

$mainLib->show_proj_objects($sql, $sql2);


$pagelib->htmlFoot(' ');

