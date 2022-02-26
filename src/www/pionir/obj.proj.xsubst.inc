<?php
/**
 * show Object navigator
 * - called by edit.tmpl.php and frame.left.nav.php
 * - session_start() must be done in the calling script !!!
 * - dynamic include of OBJECT dependend scripts:  'obj.'.$tablename_l.'.xfunc.inc'
 * 
 * $Header: trunk/src/www/pionir/obj.proj.xsubst.inc 59 2018-11-21 09:04:09Z $
 * @package obj.proj.navigator
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $id	 (=='NULL' go to ROOT)
 * @param $pmode = "" --- comes from frame.left.nav.php
                   "leftframe" => <html><body> must come from caller!          
 * @param $popt array options --- comes from frame.left.nav.php
 *        "noCheck", "markObj"
 * @global $_SESSION['userGlob']
 * 				["o.PROJ.sop.TABLE] ...
   		    	["o.PROJ.pref"] = array ("maxob"=> num, "slim"=>0|1, "addinfown" => 0|1, 
					 				"notes"=> number of shown chars: 0 means OFF
									"addinfdate" => 0|1 )    
 * @todo 1: please do refactoring of the code, put everything to a CLASS !
 * @requires must be included by calling module: reqnormal.inc								             
 */
require_once('menuBuilder2.inc');
require_once('class.history.inc');
require_once 'gui/o.PROJ.explorer.inc';
require_once ("f.text_html.inc");

// -------------------------------------------------------------------------
/*
 * INPUTS from caller:
 * @param $pmode 
 *   'leftframe'
 */

$id = $_REQUEST['id'];

$projLib = new oPROJ_navC($id, $pmode);

$showMode   = ""; // 0: normal; 1: slim; 2: slim2
$singOpt    = unserialize($_SESSION['userGlob']["o.PROJ.pref"]);
if (!is_numeric($singOpt["notes"]))  $singOpt["notes"]=0;
$singOptNew = 1;
$singOptImg = 'chevron-down.svg'; // "ic.arup.gif";
$target  = "";
//$targetj = "";
    
if ( $singOpt["slim"] ) {
	$singOptNew = 0;
	$singOptImg = 'chevron-right.svg'; // "ic.arig.gif";
	$showMode   = 1; // slim
	$popt["noCheck"] = 1;
}
if ($pmode=="leftframe") {
	$showMode = 2;   // slim2
	$popt["projplus"] = 1;
	$target  = " target=unten";
    //$targetj = "parent.unten.";
    $projLib->setHeadFlag(1);
}

$maxobjcnt = $singOpt["maxob"];
if ($maxobjcnt<10) $maxobjcnt=200; // default

$tablename="PROJ";	
global $error, $varcol;
$error  = & ErrorHandler::get();
$sql    = logon2( $_SERVER['PHP_SELF'] );
$sql2   = logon2( $_SERVER['PHP_SELF'] ); 
$varcol = & Varcols::get();
if ($error->printLast()) {
	$projLib->errorExit('error got');
}

$nicename = tablename_nice2('PROJ');
$hist_obj = new historyc();

$t_rights = tableAccessCheck($sql, 'PROJ');

if ( $t_rights['read'] != 1 ) {
  $projLib->errorExit( getTableAccessMsg($nicename, 'read') );
  exit;
} 

$editAllow = 0;
$proj_name = $projLib->ROOT_IMG;
$master_proj_id = 0;
$master_proj_master = 0;
$proj_notes = 'root '.$nicename;
$o_rights=array();
$o_rights['write'] ='0';
$o_rights['read']  ='1';
$o_rights['insert']='0';
$o_rights['delete']='0';

$acc_info=array();
$acc_info['owner'] ='root';
$acc_info['crea_date']='---';



if (empty($id)) {
    $projLib->errorExit("Object navigator: <font color=red><B>Error:</B></font> no folder-id given"); 
    return 0;
}

if ( $id!='NULL' ) {
	
	if (!is_numeric($id)) {
		$projLib->errorExit("Input: ID is not numeric!"); 
    	return 0;
	}

	
	$sqls= 'select name, pro_proj_id, notes, extra_obj_id, cct_access_id from proj  where proj_id='. $id ;
	$sql->query("$sqls");
	if ( $sql->ReadRow() ) {
	  $proj_name 	  = $sql->RowData[0];
	  $master_proj_id = $sql->RowData[1];
	  $master_proj_master = $master_proj_id;
	  $proj_notes 	  = $sql->RowData[2];
      $proj_eobjid    = $sql->RowData[3];
      $cct_access_id  = $sql->RowData[4];
	} else {
	  $projLib->errorExit("Object navigator: <font color=red><B>Error:</B></font> Folder with ID=$id not found.");
	  return;
	}


	$o_rights = access_check($sql, $tablename, $id);
	if ( $t_rights['write'] != 1 ) {
	  $o_rights['write'] = 0; // has higher priority
	}
     

	if ( !$o_rights['read'] ) {
	  $projLib->errorExit(
	  	"<br>&nbsp; Project (ID=$id) : ".
	  	"NO READ PERMISSION ! &nbsp;&nbsp;<a href=\"javascript:history.back()\">Back</a><br>");
	  return 0;
	}
	$accOpt = 3;
	$acc_info   = access_data_getai( $sql, $cct_access_id, $accOpt );
	$editAllow  = $o_rights['write'];
    $class_name = "";
    if ($proj_eobjid) {
        $class_nice_name = $varcol->obj_id_to_class_nice_name((int)$proj_eobjid);
        $class_vals_full = $varcol->select_by_nice_name($proj_eobjid);

    }
	$hist_obj->historycheck( 'PROJ', $id );
	
	$proj_name_title = $proj_name;
	if (strlen($proj_name)>40) $proj_name_title = substr($proj_name,0,40)." ...";
	$proj_name_title = htmlspecialchars($proj_name_title);
	
	$projLib->pageHead('Project ['.$id.'] '.$proj_name_title);
	 
	
} else {
	$projLib->pageHead('Project: /');
     
}


$projTables = $projLib->get_proj_tables($sql);

if ($id=="NULL" && $_SESSION['sec']['appuser']=="root") $o_rights['insert']=1; // allow new projects in ROOT



if ( $showMode == "" ) { // normal
    
    require_once('obj.proj.menu.inc');
    
    $projMenuLib = new oPROJ_menu();
    $menu = $projMenuLib->getMenu($sql, $id, $o_rights, $projTables);
    
   
    
    echo "\n";
    echo '<body marginwidth="0" marginheight="0" bgcolor=white style="margin: 0" >'."\n";

    ?>
    <script language='JavaScript'>
    

    function open_lfr( url ) {
        parent.left.location.href = url;
    }
	function open_win_new( url ) {
		NewWin = window.open( url, "new"); 
		NewWin.focus();
    }

    function createElem(tab_name) {
      url="glob.obj.crea_wiz.php?proj_id=<?echo $id?>&tablename="+tab_name+"&selecter=obj";
      location.href=url;
    }

    function Goselect(x) {
        if(x != ""){		
            url=x + "&";
            location.href = url;
        }
        return;
    }

    function selall( ) {
        i=0;
        num = document.objectsel.elements.length -1 ; <? /* -1: due to hidden field */ ?>
        for( i=0; i<num; i++ ) {
        	document.objectsel.elements[i].checked = true;
        }
    }

    function viewlist( tab_name ) {
      if (tab_name == "") return; 
      <?php 
      // do NOT change the term "pp.PROJ_ID=", because it is tested in glob.objtab.pref.php 
      $tmp_selectExt= urlencode( "_ID IN (select pp.PRIM_KEY from PROJ_HAS_ELEM pp where pp.PROJ_ID=".$id." AND pp.TABLE_NAME='" );
      ?>
        url = "view.tmpl.php?t=" + tab_name + "&condclean=1&tableSCond=" +tab_name + "<?echo $tmp_selectExt?>" + tab_name +"')";
        location.href=url;
    } 

    function viewSubProjs( ) { 
        <?                                           
        if ( $id!="NULL" ) $tmp_selectExt= urlencode( "PRO_PROJ_ID=" . $id );
        else  $tmp_selectExt = urlencode( "PRO_PROJ_ID is NULL" );
        ?>
        url = "view.tmpl.php?t=PROJ&condclean=1&tableSCond=<?echo $tmp_selectExt?>";
        location.href=url;
    }

    function viewTree() { /* view project structure as tree */
       location.href = "obj.proj.viewTree.php?id=<?echo $id?>";
    }
    
    function open_info( url )   {				
       InfoWin = window.open( url, "help","scrollbars=yes,width=650,height=500,resizable=yes"); 
       InfoWin.focus();				
    }

    function goproj( id ) {
       location.href="edit.tmpl.php?t=PROJ&id=" + id;
    }

    </script>
    <?
    $mb   = new MenuBuilder();
	$mb->createMenu($menu);
	$mb->menuRight("<img src=\"images/ich.proj.gif\">");
}

if ( $showMode == "1" ) {
	echo '<body marginwidth="0" marginheight="0" bgcolor=white style="margin: 0">'."\n";
	echo "<script language='JavaScript'>\n";
	echo " function goproj( id ) {\n";
	echo '   location.href="edit.tmpl.php?t=PROJ&id=" + id;'."\n";
	echo " }\n";
	echo "</script>\n";
}

$projLib->calc_path($sql, $master_proj_id);     


if (  $showMode == "" ) {

    echo '<form style="display:inline;" method="post" name="obj_act" action="obj.proj.elementact.php?proj_id='.$id.'">'."\n";
    echo '<table border=0 bgcolor="#E0E0E0" CELLSPACING=0 CELLPADDING=1 width=100%>'."\n";
    
	echo "<tr><td nowrap colspan=2>";
    //echo "<img src=0.gif width=1 height=23><br>\n";
	// inner TABLE
    echo "<table border=0 CELLSPACING=0 CELLPADDING=0 width=100%>\n<tr><td nowrap width=10%>";
    
    if ( $o_rights['insert'] ) {    
        $id_out = $id;
        if ( !$id )  $id_out = "NULL";  
        // obj.proj.elementact.php?proj_id='.$id_out.'&tablename=PROJ&actio_elem=new
        echo ' <a href="obj.proj.edname.php?action=create&mother_proj='.$id_out.'"><img src="images/but.projnew.gif" TITLE="new '.$nicename.'" border=0></a>';
    } else echo '<img src="images/but.projnewD.gif" TITLE="new '.$nicename.'" border=0>';

    if ( $o_rights['write'] ) 
         echo "<a href=\"obj.proj.edname.php?proj_id=".$id."\"><img src=\"images/but.edit.norm.gif\" TITLE=\"edit name and notes\" border=0></a>";
    else echo "<img src=\"images/but.edit.normNo.gif\" border=0>";     

    if ( $o_rights['write'] ) {
        echo "<a href=\"javascript:document.objectsel.actio_elem.value='cut'; document.objectsel.submit();\">";
        echo '<img src="images/but.cut.gif" TITLE="cut (works after paste)" border="0"></a>';
    } else echo '<img src="images/but.cut.low.gif" TITLE="cut">';

    echo "<a href=\"javascript:document.objectsel.actio_elem.value='copy'; document.objectsel.submit();\">";
    echo "<img src=\"images/but.copy.gif\" TITLE=\"copy\" border=0></a>";

    if ( $o_rights['insert'] ) {
        echo '<a href="obj.proj.elementact.php?proj_id='.$id.'&actio_elem=link_paste"><img src="images/but.paste.gif"';
        echo ' TITLE="paste as link" border=0></a>';     
        echo "<a href=\"javascript:document.objectsel.actio_elem.value='del'; document.objectsel.submit();\">";
        echo "<img src=\"images/but.delete.gif\" TITLE=\"delete\" border=0></a>";
    } else {
		echo '<img src="images/but.paste.low.gif" TITLE="paste as link">';     
        echo "<img src=\"images/but.delete.low.gif\" TITLE=\"delete\">";
	}
    echo "<a href=\"javascript:open_lfr('frame.left.nav.php?id=".$id."&mode=proj')\"><img src=\"images/but.lfram.gif\"";
    echo ' TITLE="open in left frame" border=0 hspace=4></a>';
    
    $is_fav = $projLib->proj_is_favorite($sql);
    $fav_ico = 'images/but.heart.png';
    $tmp_text='add to favorites';
    $tmp_a = 'p.php?mod=DEF/g.obj.favact&t='.$tablename.'&id='.$id.'&act=add';
    if ($is_fav) {
        $fav_ico = 'images/but.heart_red.png';
        $tmp_text='remove from favorite list';
        $tmp_a = 'p.php?mod=DEF/g.obj.favact&t='.$tablename.'&id='.$id.'&act=del';
    }
    if ($id!='NULL') echo '&nbsp<a href="'.$tmp_a.'"><img src="'.$fav_ico.'" height=20 TITLE="'.$tmp_text.'" border=0></a>'; 
    
    echo "<img src=0.gif width=13 height=1></td><td align=left valign=top>";
    echo "<font color=gray size=-1><img src=\"images/ic.usergr2.gif\" TITLE=\"owner\">&nbsp;".$acc_info['owner'].
		" &nbsp;&nbsp;<img src=\"images/ic.idgray.gif\" TITLE=\"Folder ID\"> ".$id."\n";
    echo "&nbsp;&nbsp;<img src=\"images/ic.cread.gif\" TITLE=\"creation date\"> ".$acc_info['crea_date'];
    if ($acc_info['mod_date']!="") echo "&nbsp;&nbsp;<img src=\"images/ic.modd.gif\" TITLE=\"modification date\"> ".$acc_info['mod_date']."</font>";
    echo "</td></tr></table>\n";
    echo "</td></tr>\n"; 
    
} else { // slim design
    echo '<table style="width:100%; border:0px; padding:0px; " ><!-- (709) -->'."\n"; // border=0 bgcolor="#E0E0E0" CELLSPACING=0 CELLPADDING=3 width=100%
}

echo '<tr style="background-color:#EFEFEF"><!-- (712) -->';
echo '<td style="background-color:#EFEFEF; vertical-align: center; padding-top:3px; padding-bottom:4px;" >';

if ($pmode!="leftframe")  {
	
	echo '<a href="preferences.php?m=o.PROJ.pref&var=slim&val='.$singOptNew.'"><img src="res/img/'.$singOptImg.
	   '" style="vertical-align:baseline; height:17px; padding: 0px 4px 0px 4px;" TITLE="slim/fat"></a>';
}


$projLib->show_path($master_proj_master, $proj_name);
    
echo "</td></tr>\n";
    
if ( $showMode<2 ) {
    if ( $proj_notes ) {
		echo '<tr bgcolor=#FFFFFF><td style="color:#808080;"><pre>';
		$proj_notes_sp = htmlspecialchars($proj_notes);
		f_text_html::notes_out($sql, $proj_notes_sp);
		echo '</pre></td></tr>';
	}
    if ( $proj_eobjid ) { 
        echo "<tr><td bgcolor=#EFEFEF><font color=gray><img src=\"0.gif\" width=16 height=1> class:</font> $class_nice_name";
        $tmp_vals = $class_vals_full["values"] ;
        $komma = "";
        if (sizeof($tmp_vals)) { 
            foreach( $tmp_vals as $var => $val ) {
                echo " <font color=gray>". $var. ":</font> $val";    
            }
        }
        echo "</td></tr>\n";
    }
} /*else {
    echo "<a href=\"".$this->_get_p_link($id)."\" ><img src=\"images/but.projup.gif\" border=0></a>";
    echo " <font color=gray><B>Project:</B></font> ";
    echo "<B>$proj_name</B>";
    echo "</td></tr>\n";
}
*/

echo "</table>\n";
echo "</form>\n";
$labAddOpt = NULL;

// FUTURE: LAB dependend
// if ($singOpt["addlab"] != "" AND $proj_eobjid) {
// 	// add a lab-specific view
// 	$tmpfile = "../lab/obj.proj.addview.inc";
// 	if ( file_exists($tmpfile) ) {
// 		require_once($tmpfile);
// 		$labaddobj = new projLabView();
// 		$labAddOpt = $labaddobj->labaddview( $sql, $id, $proj_eobjid );
// 	}
// } 
// START of contained objects

if ( $showMode == "" ) {
	 echo '<form style="display:inline;" method="post" name="objectsel" action="obj.proj.elementact.php?proj_id='.$id.'">'."\n";
	 //echo "<img src=\"0.gif\" height=1 width=1>\n";
} 
echo '<div style="margin-top:5px; margin-bottom:0px;"></div>'."\n";


if ($showMode==1) echo '<table style="padding-top:0px; padding-left:6px; width:100%"><!-- (773) -->'."\n"; // extra spacing left
else echo '<table style="width:100%"><!-- (774) -->'."\n";

$projLib->show_sub_projs($sql, $sql2, $singOpt, $popt, $target);
if ($error->Got(READONLY))  {
	return;
}

if ($id !='NULL') { /* the ROOT project has no elements */
  
  $iopt=NULL;
  $iopt["target"]  = $target;
  $iopt["noCheck"] = $popt["noCheck"];
  $iopt["markObj"] = $popt["markObj"];
  $iopt["maxob"]   = $maxobjcnt;
  $iopt["showOnwer"]= $singOpt["addinfown"];
  $iopt["showDate"] = $singOpt["addinfdate"];
  $iopt["notes"]    = $singOpt["notes"];
  if ($labAddOpt!="") $iopt["objOpt"] = $labAddOpt;
  $projLib->bos_infos( $sql, $sql2, $projTables, $iopt );
  
}    

?>
<input type="hidden" name="actio_elem" value="">
</form>
<?
// echo "</td></tr>";
echo "</table>\n";
echo "</nobr>\n";

if ($projLib->get_num_proj_obj()<=0) {
    echo '<div style="padding:15px; color: #A0A0A0; font-size:1.2em;">This folder is empty.</div>'."\n";
}

htmlShowHistory("PROJ", $id);
if (empty($_SESSION['s_sessVars']['boProjSel'])) $_SESSION['s_sessVars']['boProjSel'] = $id;  


?>
</body>
</html>