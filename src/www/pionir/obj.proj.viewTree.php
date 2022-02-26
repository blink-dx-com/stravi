<?php
/**
 * view a project as tree
 * @package obj.proj.viewTree.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param    $id          : proj.proj_id for start of tree view
		  $go
  		  $parx["objects"] = 
			   "allbar"  -- colbar $parx["objmode"] == "icon"
		  	   "allicon",     -- all objects or one selected object type $parx["table"]
			  ["projs"], -- only project structure

		  $parx["table"] -- "_ALL_", see $parx["objects"]="select" 
		  $parx["compact"] = 0,1
 */


// extract($_REQUEST); 
session_start(); 


require_once("func_head.inc");
require_once("func_form.inc");
require_once("db_access.inc");
require_once("globals.inc");
require_once("access_check.inc");
require_once("o.PROJ.tree.inc");
require_once("varcols.inc");

class mainTreeC{
    
    /**
     * 
     * @var $showObjects
     *   ["all"] - show projects and objects
		 "projs"- show only projects)
		 "table" = "" : selected table-type
     */
	var $showObjects; 
	var $selTable; 
				
	/**
	 * 
	 * @param object $sql
	 * @param int $projid
	 * @param array $showopt
	 *  "objects" = showObjects_STRUCT
	 *     
	 */
    function init( &$sql, $projid, $showopt=NULL  
    			) {
    								  
    	$this->projid = $projid;	
    	$this->projname =  obj_nice_name    ( $sql, "PROJ", $this->projid);
    	$this->proj_nice  = tablename_nice2("PROJ");
    	
    }
    
    function getShowmode() {
    	return ($this->showObjects);
    }
    
    function getBOs( &$sql ) {
    
    	$selarr=NULL;
    	$sql->query("SELECT table_name, nice_name FROM cct_table where TABLE_TYPE='BO' ORDER BY nice_name");
    	while ( $sql->ReadRow() ) {
    		  $tabname  = $sql->RowData[0];
    		  $nicename = $sql->RowData[1]; 
    		  if ($nicename=="")  $nicename = "[$tabname]";
    		  $selarr[$tabname]=$nicename;
    	}
    	return ($selarr);
    }
    
    function getIcon($tab_name) {
    	$tab_name_icon = ($tab_name == "PROJ") ? "PROJ_link" : $tab_name;
    	if ( $this->tabIcons[$tab_name_icon]!="" ) {
    		$icon = $this->tabIcons[$tab_name_icon];
    	} else {
        	$icon = file_exists("images/icon.$tab_name_icon.gif") ? "images/icon.$tab_name_icon.gif" : "images/icon.UNKNOWN.gif";
    		$this->tabIcons[$tab_name_icon] = $icon;
    	}
    	return ($icon);
    }
    
    function _getProjIcon($proj_extra_obj_id) {
    	global $varcol, $error;
    	
    	if ($proj_extra_obj_id) {
            $class_name = $varcol->obj_id_to_class_name($proj_extra_obj_id);
            if ($error->printLast()) return;
            if (file_exists("../lab/images/icon.PROJ.$class_name.gif"))
                $icon = "../lab/images/icon.PROJ.$class_name.gif";
            elseif (file_exists("images/icon.PROJ.$class_name.gif"))
                $icon = "images/icon.PROJ.$class_name.gif";
            else
                $icon = "images/icon.PROJ.unknown_class.gif";
        } else { // normal project
            $icon = "images/icon.PROJ.gif";
        } 
    	return ($icon);
    }
    
    function _getColor($tab_name) {
    	global $_s_i_table;
    	
    	if ($this->colorCache[$tab_name]=="") { 
    		$colorx = $_s_i_table[$tab_name]['__table_data__']['COLOR'];
    		if ($colorx=="") {
    			$colmd5 = crc32($tab_name);
    			$colnum = fmod($colmd5,65535);
    			$colorx = sprintf("#%06X", $colnum);
    		}
    		$this->colorCache[$tab_name] = $colorx;
    	}
    	$colorx = $this->colorCache[$tab_name];
    	return ($colorx);
    }
    
    function _showAllBos(&$sql, &$sql2, $proj_id_now) {
    	// INPUT: $this->selTable
    	$sqlsadd = "";
    	if ($this->selTable!="") {
    		$sqlsadd = " AND table_name='".$this->selTable."'";
    	}
    	$sqls = "SELECT table_name, prim_key FROM proj_has_elem WHERE proj_id = ".$proj_id_now.$sqlsadd." ORDER BY table_name";
    	
    	$sql2->query($sqls);
    	
    	while($sql2->ReadRow()) {
    		$table_name  = $sql2->RowData[0];
    		$prim_key_id  = $sql2->RowData[1];
    		$icon = $this->getIcon($table_name);
    		$objname = obj_nice_name ( $sql, $table_name, $prim_key_id ); 
    		$out_imp_name = empty($objname) ? $prim_key_id : $objname;
    		echo "<li><a href='edit.tmpl.php?t=$table_name&id=$prim_key_id'><img src='$icon' border='0'> $out_imp_name</a></li>\n";
    	}
    	
    }
    
    function _showBoColBars(&$sql, $proj_id_now) {
    
    	$sqlsadd = "";
    	if ($this->selTable!="") {
    		$sqlsadd = " AND table_name='".$this->selTable."'";
    	}
    	$sqls="SELECT max(table_name), count(1) FROM proj_has_elem WHERE proj_id = ".$proj_id_now.$sqlsadd.
    		" group by table_name ORDER BY table_name";
    	
    	
    	$sql->query($sqls);
    	
    	while($sql->ReadRow()) {
    		$tab_name  = $sql->RowData[0];
    		$numobj      = $sql->RowData[1];
    		
    		$colorx = $this->_getColor($tab_name);
    		$colbarwidth= 2 * $numobj;
    		if ($numobj>400)  $colbarwidth= 2 * 400; // max bar length
    		if ( $this->parx["compact"] ) {
    			$imgheight = 9;
    		} else {
    			$imgheight = 14;
    		}
    		$nicetab = tablename_nice2($tab_name);
    		
    		echo "<img src=0.gif height=".$imgheight." width=".$colbarwidth." style=\"background-color: ".$colorx
    			."\" title=\"".$numobj." of ".$nicetab."\">";
    			
    	}
    	
    }
    
    
    

    /**
     *  prints the pure project tree to the screen
     *  warning! RECURSIVE function
 
     * @param object $sql
     * @param object $sql2
     * @param int $proj_id ... project id of the subtree to print
     * @param string $proj_name ... name of the top-proj of the sub-tree
     * @param array $proj_arr ... array containing whole project tree
     */
    function print_tree_pure( &$sql, &$sql2, $proj_id, $proj_name, &$proj_arr ) {

        //$varcol = & Varcols::get();
        //$error  = & ErrorHandler::get();
        $icon = "images/icon.PROJ.gif";
         
    	if ($this->parx["compact"]) {
    		// no image
    		echo '<li><a href="edit.tmpl.php?t=PROJ&id='.$proj_id.'" class=smallx>'.$proj_name.'</a>'."\n";
    	} else {
        	echo '<li><a href="edit.tmpl.php?t=PROJ&id='.$proj_id.'"><img src="'.$icon.'" border="0"> '.$proj_name.'</a>'."\n";
    	}
    	$this->projcnt++;
        
    	if ( $this->showObjects=="all") {
    		
    		if (  $this->showopt ["objmode"] == "icon" ) {
    			echo '<ul class=xul>'."\n";
    			$this->_showAllBos($sql, $sql2, $proj_id);
    		} else {
    			$this->_showBoColBars($sql, $proj_id);
    			echo '<ul class=xul>'."\n";
    		}
    		
    	} else echo '<ul class=xul>'."\n";
    	
        foreach( $proj_arr[$proj_id] as $proj_id_sub=>$projname) { // show subprojects
            $this->print_tree_pure($sql, $sql2, $proj_id_sub, $projname, $proj_arr);
    		
        }
    	
    	
    	
    	echo '</ul></li>'."\n";
    }
    
    function  setParx( $parx ) {
    	/*
    	$parx["objmode"] = "icon", 
    					   "colbar" -- as colorbar
    	*/
    	if ($parx["table"]== "_ALL_") $parx["table"]="";
    
    	if ( $parx["objects"] == "") {
    		$parx["objects"] = "projs";
    	}
    	$this->showObjects = "projs";
    	
    	if ( $parx["objects"]== "allicon" )  {
    		$this->showObjects = "all";
    		$parx["objmode"] = "icon";
    	}
    	
    	if ( $parx["objects"]== "allbar" ) {
    		 $parx["objmode"] = "colbar";
    		 $this->showObjects = "all";
    	}
    	$this->parx    = $parx;
    	$this->showopt = NULL;
    	$this->showopt["objects"] = $parx["objects"];
    	$this->showopt["objmode"] = $parx["objmode"];
    	
    	
    	$this->selTable = $parx["table"];
    }
    
    function startShow( &$sql, &$sql2 ) {
    	//global $error;
    	
    	$id = $this->projid;
    	
    	//$reading_allowed = 1;
    	//$dummy=NULL;
    	$this->projcnt=0;
    	
    	echo '<ul class=xul>';
    	
    	$proj_arr  = &oPROJ_tree::tree2array($sql, $id);
    	$this->print_tree_pure($sql, $sql2, $id, $this->projname, $proj_arr);
    	
    	//glob_printr($proj_arr, "proj_tree" );
    	echo "<br>";
    	echo "<b>".$this->projcnt."</b> ".$this->proj_nice."s analysed.<br>";
    }

}

//**********************************************************************************
//**********************************************************************************
//**********************************************************************************

$id = $_REQUEST['id'];
$go= $_REQUEST['go'];
$parx= $_REQUEST['parx'];

if (empty($id)) $id=0;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2   = logon2( $_SERVER['PHP_SELF'] );
$varcol = & Varcols::get();


$compactcss = "list-style-type: none;";
if ( $parx["compact"]) $compactcss = "list-style-type: none; line-height:0.8em; ";

$css = "
a:hover  { text-decoration: underline; color: #0000ff; }
a:active { text-decoration: none;      color: #ff0000; }
a        { text-decoration: none;      color: #000000; }
.smallx  { font-size:0.8em; }
.xul     { ".$compactcss." }
"; 


		
$tablename="PROJ";
$proj_nice  = tablename_nice2($tablename);
$title = "Folder Tree View";

$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["help_url"] ="o.PROJ.viewTree.html";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;
$infoarr["show_name"]= 1;
$infoarr["css"]= $css;

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

echo "<ul>";
$projTreeObj = new mainTreeC();

if (empty($id)) htmlFoot("ALERT", "no ".$proj_nice." ID given.");

if ($error->printLast()) htmlFoot();


$projTreeObj->setParx($parx);
$showMode = $projTreeObj->getShowmode();

// $txt1 = "[<a href=\"".$_SERVER['PHP_SELF']."?id=$id&parx[objects]=projs\">Only ".$proj_nice."s</a>]";
// $txt1 .= "&nbsp;&nbsp;<input type=checkbox name =\"parx[objmode]\" value=1> ".
// 	"<font color=#30F050>C</font><font color=#3040F0>o</font>lorbar&nbsp;";
	
$modeinfo = "";

if ( $showMode=="projs" ) {
    $modeinfo="only ".$proj_nice." structure";
}
if ( $showMode=="all" ) {
	$modeinfo="<b>structure with objects</b> ";
	if ($projTreeObj->selTable!="") {
		$selTableNice = tablename_nice2($projTreeObj->selTable);
		$iconx = htmlObjIcon($projTreeObj->selTable);
		$modeinfo.="(only <img src=\"".$iconx."\"> $selTableNice)";
	}
	else $modeinfo.="ALL";
}

$feld = $projTreeObj->getBOs( $sql );

if ($projTreeObj->selTable!="") $selpref = $projTreeObj->selTable;
$seloption = array( "selecttext"=>"--- select objects ---" );
$seltext = formc::selectFget( 
			"parx[table]", // FORM-variable-name
			$feld, 		   // array ( ID => "nice name")
			$selpref,
			$seloption);
			
echo "<table border=0 cellspacing=1 cellpadding=0 bgcolor=#DFDFFF><tr><td>"; // light blue bgcolor
echo "<table border=0 cellspacing=0 cellpadding=2 bgcolor=#EFEFEF><tr><td>"; // white bgcolor
   			
echo "<table border=0 cellspacing=0 cellpadding=4><tr valign=top>";	
echo "<td><form style=\"display:inline;\" method=\"post\"  name=\"editform\"  action=\"".$_SERVER['PHP_SELF']."\" >\n";
echo "<input type=hidden name='id' value='".$id."'>\n";
echo "<input type=hidden name='go' value='1'>\n";

echo '<input type=submit value="Show objects" class="yButton">'."\n";
echo "</td>\n";

// echo "$txt1 &nbsp;&nbsp;\n";
$tmpsuper = "<br><input type=checkbox name =\"parx[compact]\" value=1> super compact";
$radioVals = array(
    "projs"=>  "<img src=\"images/obj.proj.viewTree.01.png\" border=1><br><img src=\"0.gif\" width=25 height=1><b>only ".$proj_nice."s</b> &nbsp;&nbsp;&nbsp;&nbsp;",
  	"allbar"=> "<img src=\"images/obj.proj.viewTree.02.png\" border=1><br><img src=\"0.gif\" width=25 height=1><b>objects as colorbars</b>".$tmpsuper,
	"allicon"=> "<img src=\"images/obj.proj.viewTree.03.png\" border=1><br><img src=\"0.gif\" width=25 height=1><b>full details</b>",
				  );
$radioSeperator = "</td><td bgcolor=#D0D0FF>";

echo "<td bgcolor=#D0D0FF>\n";
formc::radioArrOut( "parx[objects]", $projTreeObj->parx["objects"], $radioVals, $radioSeperator);
echo "</td><td bgcolor=#EFEFEF>\n";
echo "$seltext\n";
echo "<br>this selection is optional!</td></tr></table>\n";
echo "</form>";
echo "  </td></tr></table>\n";
echo "</td></tr></table>\n";

if (!$go) htmlFoot("<hr>"); 

$projTreeObj->init($sql, $id);
echo "<font color=gray>Mode:</font> ".$modeinfo."<br><br>\n";
$projTreeObj->startShow($sql, $sql2);

$error->printLast();
htmlFoot('</ul><hr>');
