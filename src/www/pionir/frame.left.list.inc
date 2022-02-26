<?php
/**
 * show list-view in "left" frame
 * called by frame.left.nav.php
 * @namespace core::gui
 * @package frame.left.list.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 */
require_once("sql_query_dyn.inc");
require_once("frame.left.func.inc");

function frameList_showlist( &$sql, $tablename, $opt ) {
    

	$heaopt = array("css" =>".xbr  { color: #EFEFEF; }" );
	$title  = "Left frame: list";
	
	$pagelib = new gHtmlHead();
	$pagelib->_PageHead ( $title,  $heaopt );
?>

<script language="JavaScript" type="text/JavaScript">
	var lastTag="";
    function goobj( id ) {
        parent.unten.location.href="edit.tmpl.php?t=<?echo $tablename?>&id=" + id;
    }
	function th_snc( tag, id ) {

		var newColor     = "#D0E0FF";
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
        // parent.unten.location.href="edit.tmpl.php?t=<?echo $tablename?>&id=" + id;
	}

</script>
<?php
    $maxShowCount = 20;
    $viewPage     = 0;
    if ( $opt["viewp"]!="") $viewPage = $opt["viewp"];
    
	$optDefTMp = $_SESSION['userGlob']['o.'.$tablename.'.showsets'];
    if ( $optDefTMp!=""  ) {
        if ( is_numeric($optDefTMp) AND $optDefTMp<=40) $maxShowCount = $optDefTMp;
    }
    $hopt=NULL;
	$hopt["back"]    = 1;
    $hopt["addlink"] = '<a href="'.$_SERVER['PHP_SELF'].'?listtab='.$tablename.'&mode=list"><img src="images/but11.refsh.gif" border=0 TITLE="refresh"></a>';
    frameLc::header3out("list mode", $hopt);

    if ( $tablename=="" ) {
        echo "<br><br>\n<font color=#FF8080>Error:</font>";
        echo "Need a table<br>\n";
        return;
    }
    $tablenice = tablename_nice2($tablename);
	$icon      = file_exists( "images/icon.".$tablename.".gif") ?
	               "images/icon.".$tablename.".gif":
	               "images/icon.UNKNOWN.gif";

	frameLc::header2open();

	echo '<tr><td NOWRAP>';
    echo "&nbsp;<a href=\"view.tmpl.php?t=".$tablename."\" target=unten><img src=\"".$icon."\" border=0><font color=#003366> ";
    echo "<B>".$tablenice."</B></font></a>";
    echo " list view<br>";
    echo '</td></tr>';

    
    $primary_key = PrimNameGet2($tablename);
    $importName  = importantNameGet2($tablename);
    
    $sopt["order"] = 1;
    $sqlAfter      = get_selection_as_sql( $tablename, $sopt );
    $sqlAfterNoSort      = get_selection_as_sql( $tablename );
    $sql->query("SELECT count(x.".$primary_key.") FROM ".$sqlAfterNoSort);
    
    $sql->ReadRow(); 
    $objcnt = $sql->RowData[0];
    
    if ($objcnt <= 0) {
        echo "<br><font color=#FF8080>Attention:</font> Please go back and<br>select elements from the list!\n";
        return;
    } 
    echo '<tr><td NOWRAP><font color=#EFEFEF>';
    echo "&nbsp;<B>$objcnt</B> objects selected</font>\n";
    echo '</td></tr>';
    echo '<tr bgcolor=#4488BB><td><img src=0.gif height=5 width=1></td></tr>';
    echo '</table>'."\n"; 
    
    echo '<table id="tagID" border=0 cellspacing=1 cellpadding=0 bgcolor=#FFFFFF width=100%>'."\n";            
    $realObjCnt = 0;
    $showcnt    = 0;
    $startShow  = $viewPage * $maxShowCount;
    $sql->query("SELECT x.".$primary_key.", x.".$importName." FROM $sqlAfter");     
    
    while ($sql->ReadRow() AND $realObjCnt<$startShow ) { 
         $realObjCnt++;
    }
    $flagstop   = 0;
    $endShowCnt = $startShow + $maxShowCount; // define the end 
    $showcnt    = $realObjCnt;  // starts at 1 !
    $sqlFinish  = 0;
    $rowcnt     = 0;
    while ( $realObjCnt < $endShowCnt ) { 
        
        $tmpid   = $sql->RowData[0];
        $tmpname = $sql->RowData[1];  
        
        $tmpnameHtml = htmlspecialchars($tmpname); 
        // <a href=\"edit.tmpl.php?t=".$tablename."&id=".$tmpid."\" target=unten>
        // onclick=\"th_snc( '".$tmpid."' )\"
        echo "<tr id=t".$rowcnt."><td><a href=\"edit.tmpl.php?t=".$tablename."&id=".$tmpid."\" target=unten onclick=\"th_snc( 't".$rowcnt."','".$tmpid."' )\">".$tmpid."</a></td><td nowrap>&nbsp;".$tmpnameHtml."</td></tr>\n";
        $showcnt++;
        $rowcnt++;
        if (!$sql->ReadRow()) {
            $sqlFinish=1;
            break;
        }
        $realObjCnt++;
    }     
    echo "</table>\n";
    echo "<hr noshade size=1>\n";
    if ($showcnt) echo "Results: <B>".($startShow+1)."</B> - <B>".$showcnt."</B><br>\n";
    else echo "Results: <B>0</B><br>\n";
    echo "Page: ";
    if ( $viewPage > 0 ) { 
         echo '<a href="'.$_SERVER['PHP_SELF'].'?listtab='.$tablename.'&viewp=0&mode=list">[1]</a> &nbsp;';
         echo '<a href="'.$_SERVER['PHP_SELF'].'?listtab='.$tablename.'&viewp='.($viewPage-1).'&mode=list">[&lt;&lt;]</a>';
    }

    echo " <B>". ($viewPage+1). "</B>\n";

    if ( $realObjCnt < $objcnt AND (!$sqlFinish)) {
        echo '<a href="'.$_SERVER['PHP_SELF'].'?listtab='.$tablename.'&viewp='.($viewPage+1).'&mode=list">[&gt;&gt;]</a>';
    }
    
    echo "\n<hr noshade size=1>\n";
}
