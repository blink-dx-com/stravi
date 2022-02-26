<?php
/**
 * Quick search (form + executer)
   - trim $idx (no whitespaces)
   - TBD: check for $sql->addQuotes($idx)
   - New: optional add new condition to old condition
   GLOBAL Vars:
   - $_SESSION['userGlob']["f.glob.obj.qsearch"] : serialized data
 * @package glob.obj.qsearch.php
 * @swreq UREQ:0001029: g > glob.obj.qsearch.php : Quick search; FS-OBF03-a
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/glob.obj.qsearch.php 59 2018-11-21 09:04:09Z $
 * @param   
 *  $idx    ID or "part of name" or list of IDs or NAME
    [$tablename] : tablename 
    $parx -- qsearch_parx_STRUCT
    
    $parx["tableauto"]=> if tablename is empty, get tablename from clipboard 
    $parx["algo"] key word for search algorithm: 
        ["def"]    - default: if $idx contains LINEBREAKS, the tool tries automatic detect of the algo "mixid" or "namelist"
		"mixid"    - mix IDs separated by commas
        "namelist" - list of names 
        "idlist"   - list of IDs
		"moogle"   - google like
        "wild"     - soundex search
    $parx["addnotes"]  => - search also in notes
         				   - but if an ID is given, search only for ID-column
	$parx["stopforward"] = [0] | 1
	$parx["bool"] = [""], "AND", "OR" optional add NEW condition to OLD condition
	$parx["showNoTabSel"] = do not show table selector
         
    $go    0 - show form
           1 - show from, if no tablename given
           2 - immediate search with options from $_SESSION['userGlob']["f.glob.obj.qsearch"]
		   3 - force immediate search
	
 */


session_start(); 


require_once ('reqnormal.inc');
require_once ('f.s_historyL.inc');
require_once ('func_form.inc');
require_once ("f.objview.inc");
require_once ( "javascript.inc" );
require_once ('f.msgboxes.inc');
require_once ('lev1/glob.obj.qsearch1.inc');

require_once ('f.wiid_roid.inc');




/**
 * GUI HELPER class, used by class fSearchGui
 *
 */
class fSearchGuiSub {
    
    var $searchInfo=array();
    
    // set main table
    function set_table($tablename, $parx) {
        $this->tablename = $tablename;
        $this->parx = $parx;
    }
    
    function set_parx_val($key, $val) {
        $this->parx[$key] = $val;
    }
    
    function formRow($key, $value, $notes=NULL)  {
        $this->row_start($key, "opt");
        echo $value;
        if ($notes!="") echo "&nbsp;<font color=gray>".$notes."</font>";
        echo "</td></tr>";
    }
    
    private function _sel($sqlo, $tablename) {
        
        $tab1=array();
        if (sizeof($_SESSION['s_historyL'])) {
            foreach( $_SESSION['s_historyL'] as $tab_key=>$dummy) {
                $tab1[$tab_key] =  tablename_nice2($tab_key);
            }
            reset ( $_SESSION['s_historyL'] );
        }
        
        echo '<select name="tablename" >';
        echo '<option value="">--- select  ---';
        echo "\n";
        $selected_flag=0;
        
        if (sizeof($tab1)) {
            foreach( $tab1 as $key=>$val) {
                $selected="";
                if ( !$selected_flag AND $tablename == $key ) {
                    $selected=" selected";
                    $selected_flag=1;
                }
                echo "<option value=\"".$key."\" $selected>".$val."\n";
            }
            reset($tab1);
            echo "<option value=\"\">&nbsp;\n";
            echo "<option value=\"\">----------------\n";
        }
        
        $TabLib   = new oCCT_TABLE_info();
        $tables = $TabLib->getTablesNiceByType($sqlo, 'BO');
        
        
        foreach( $tables as $key=>$val) {
            $selected="";
            if ( !$selected_flag AND $tablename == $key ) {
                $selected=" selected";
                $selected_flag=1;
            }
            echo "<option value=\"".$key."\" $selected>".$val."\n";
        }
        reset($tables);
        echo "</select>\n";
    }
    
    
    function short_help() {
        
        htmlInfoBox( "Short Help", "", "open", "CALM" );
        ?>
    	      
    	     <B>"ID or NAME" </B> are searched case insensitive, can be a list of IDs or Names (e.g. copied form Excel)<br>
    		  
    		  <br>
    		 Method "<b>normal</b>" supports the following simple search-methods:
    	     <table bgcolor=#EFEFFF>
    	     <tr bgcolor=#CFCFCF><td>What?</td><td>Example</td><td>Description</td></tr>
    	     <tr valign=top> 
    	     <td><B>ID</B></td><td>  2345 </td><td><I>an ID of an object (if first number is a "0" it searches only in NAME) </I></td></tr>
    	     
    	     <tr valign=top> 
    	     <td><B>Exact name</B></td><td>  ACT_oligo_345 </td><td><I>a Exact name of an object (case insensitive)</I></td></tr> 
    	     
    		 <tr valign=top> 
    	     <td><B>Wildcards</B></td><td>  ACT_oligo_% </td><td><I>a PART of a name with WILDCARDS "%"</I></td></tr>    
    	     
    	     <tr valign=top> 
    	     <td  NOWRAP><B>Parts of name</B></td><td>  oligo </td><td><I>just a PART  of a name</I>
    	     </td></tr> 
    	     
    	     <?php 
    	     if($this->tablename=='ABSTRACT_SUBST') {
    	     ?>
    	     <tr valign=top> 
    	     <td><B>Product-Number</B></td><td>PRO:3723-233</td><td><I>search for the external product number of a material</I>
    	     </td></tr> 
    	     <?php
    	     }
    	     ?>
    		 
    		 <tr valign=top> 
    	     <td><B>Remote ID</B></td><td>rid:2:2345</td><td><I>the keyword <b>rid:</b> searches an object, which comes from a remote database with WIID=2 and ROID=2345</I>
    	     </td></tr> 
    		 </table> 
    		 <br>
    		 
    		 <hr />
    		 Following <b>special</b> search methods are supported:<br><br>
    		 
    		
    		 
    		<B><font color=#0000FF>G</font><font color=#FF0000>o</font><font color=#808000>o</font><font color=#0000FF>g</font><font color=#00DD00>l</font><font color=#FF0000>e</font> like</B>
    		 &nbsp;&nbsp;&nbsp; method: <b>google like</b><br>
    		example: Synthese glut<br>
    		Description: WHITE_SPACE separated keywords which must appear in NAME (or NOTES)
    		<br><br>
    	  
    		 
    		 <B>Mixed IDs</B>  &nbsp;&nbsp;&nbsp;method: <b>mix IDs</b><br>
    		 example: 1256,1300-1502,1603 <br> 
    		 Description: IDs or between IDs,  separated by COMMAS or WHITE_SPACE or NEWLINE
    	     <br><br>
    		 
    		<B>List of names</B>  &nbsp;&nbsp;&nbsp;method: <b>List of names</b><br> 
    		example: cs234 cs567 ci345<br>
    		Description: give WHITE_SPACE or NEWLINE separated list of names (can also contain wildcards like '%')
    	    <br><br>
    	  
    	   <hr />
    	   Following objects force special searches : <br><br>
    	  <B>Contact</B> <br> 
    		searching in this tables forces a search in following fields: NAME, CONTACT PERSON, EMAIL
    		<br><br>
    	   
    	   <?
    	    htmlInfoBox( "Short Help", "", "close" );
    }
    
    function htmlFoot($error=NULL, $text=NULL) {  
        
       if ($error!="") {
            htmlErrorBox($error, $text);
            echo "<br>\n";
       }
       
       echo "<br><br><br><br><br><br>\n";
       $this->short_help();
       htmlFoot();
    }
    
    /**
     * 
     * @param string $key
     * @param string $format  // "opt", "warn"
     */
    private function row_start($key, $format=NULL, $css_cls_flag=NULL) {
    	
    	if ($format=="warn") $trbg = " bgcolor=#FFBBBB";
    	if ($format=="opt")  $trbg = " bgcolor=#FFFFFF";
    	if ($format=="space") {
    		echo "<tr bgcolor=#FFFFFF height=3><td colspan=2>\n";
     		return;
    	}
    	
    	$use_class='';
    	if ($css_cls_flag!='') $use_class='  class="'.$css_cls_flag.'"  style="DISPLAY:none;" ';
    	
    	echo "<tr $trbg ".$use_class." valign=top><td align=right>";
    	echo $key;
    	echo " </td><td>";
    }
    
    /**
     *
     */
    private function row_end() {
    
        echo " </td></tr>\n";
    }
    
    /**
     * show search parameter form
     * @param object $sql
     * @param int $idx
     * @param string $tablename
     * @param array $parx - see qsearch_parx_STRUCT
     * @param array $fields DEPRECATED
     * @param array $opt DEPRECATED
     */
    function formshow( &$sql, $idx, $tablename, $parx, $fields=NULL, $opt=NULL ) {
        
        
        ?>
        <script type="text/JavaScript">  
        function x_showField(cls)  
        {  
            
             /*         
             if (document.getElementById(id).style.display == 'none')  
             {  
                  document.getElementById(id).style.display = '';  
             } 
             */ 
    
             var objects = document.getElementsByClassName(cls);
    
             var style_x='none';
             if (objects[0].style.display == 'none')  {
            	 style_x='';
             }
                  
             for (x of objects) {
                x.style.display = style_x;  
                
             }
             
        }    
        </script>  
        <?php
        
        
    
        
       
        $tabRow   = array();
    	if ( $tablename =="" ) {
    		$infolevel = "warn";
    		$tabRow[0] = " bgcolor=#FFBBBB";
    		$tabRow[2] = "<font color=800000>Please select !!!</font>";
    	}
        echo "<form method=\"post\"  name=\"editform\"  action=\"".$_SERVER['PHP_SELF']."\" >\n";
        echo "<input type=hidden name=mode value=\"classic\">\n";                   
        echo "<input type=hidden name=go value=\"3\">\n";
        echo "<input type=hidden name=tablename value=\"".$tablename."\">\n";
        
        // echo "<table cellpadding=1 cellspacing=0 border=0 bgcolor=#D0D0F0><tr><td>";
        echo "<table cellpadding=3 cellspacing=0 border=0 >"; // bgcolor=#E8E8F8
    	 
    	
        $this->row_start( '<input class="yButton" type="submit" value="Search">' );
    	echo "<textarea rows=2 cols=50 name=idx >".$idx."</textarea>";
    	echo '&nbsp; '. "<a href=\"javascript:x_showField('x_advanced')\">". '<img src="images/but.40.config.png" title="Configuration"></a>';
    	//   	"&nbsp;&nbsp;&nbsp; <a href=\"javascript:x_showField('x_tools')\">". '<img src="images/but.40.tools.png" height=30 title="other Tools"></a>';

    	$this->row_end();
    	
    	$varname     = "parx[algo]";
    	//  "idlist" => "list of IDs" ::: use mixedIDs
    	$feld        = array ("def" => "normal", "mixid" =>"mixed IDs", "namelist" => "list of names",  "moogle" =>"google like" );
    	$preselected = $parx["algo"];
    	$soption = NULL;
    	$soption["selecttext"] = "--- optional ---";
    	$htmltmp = formc::selectFget( $varname, $feld, $preselected, $soption);
    	
    	$this->row_start( "Method", "opt" );
    	echo $htmltmp;
    	$this->row_end();
    	
    	// echo '<tr class="x_advanced"  style="DISPLAY:none;" valign=top><td colspan=2>';
    	// echo '<table><!-- ADVANCED_TABLE -->';
    	
    	$this->row_start( "Table", $infolevel, 'x_advanced' );
    	if ( $parx["showNoTabSel"]!=1 OR  $tablename=="") {
    	    $this->_sel($sql, $tablename);
    	}
        
        // echo $tabRow[2];
        $this->row_end();
    	
    	$this->row_start("", "space" );
    	$this->row_end();
    	
    	if ($tablename=="ABSTRACT_SUBST") {
    	    $this->row_start( "Wild search", "opt" );
    	    $tmp_sel="";
    	    if ($parx["wild"]==1) $tmp_sel=" checked";
    	    echo "<input type=checkbox name=\"parx[wild]\" value=1 ".$tmp_sel.">";
    	    echo ' &nbsp;&nbsp; <span class="yGgray">intelligent name rank algorithm - the Levenstein distance.</span>';
    	    $this->row_end();
    	}
    	
    	$this->row_start( "Notes", "opt", 'x_advanced' );
    	
    	$tmpNotesSel="";
    	if ($parx["addnotes"]==1) $tmpNotesSel=" checked";
    	echo "<input type=checkbox name=\"parx[addnotes]\" value=1 ".$tmpNotesSel.">";
        echo ' <span class="yGgray">search in NAME + NOTES</span>';
        $this->row_end();
        
        
    	
        
        
        $this->row_start( "Add condition", "opt", 'x_advanced' );
        if ($parx["bool"]) $tmpval=" checked";
        echo ' <input type=checkbox name="parx[bool]" value="1" '.$tmpval.'> Add this condition to existing condition';
        $this->row_end();
        
        $this->row_start( "Stop forward?", "opt", 'x_advanced' );
        echo "<font color=gray><input type=checkbox name=\"parx[stopforward]\" value=\"1\"> ";
        $this->row_end();
        
        
        $this->row_start( "Other Tools", "opt", 'x_advanced' ); // old: x_tools
        $sepIcon = '&nbsp;&nbsp;|&nbsp;&nbsp; ';
        $headtext = "<a href=\"obj.link.c_query_mylist.php\">My Search Center</a>".$sepIcon.
        "<a href=\"glob.objtab.search_1.php\">Pro-Feature-Search</a>";
        if ($tablename!=NULL) {
            $headtext .= $sepIcon. '<a href="view.tmpl.php?t='.$tablename.'">'.tablename_nice2($tablename).'</a> (list)';
        }
        echo $headtext;
        $this->row_end();
        
        //echo '</table><!-- ADVANCED_TABLE:END -->'."\n";
        //echo '</td></tr>';
    
    	
        echo "</td></tr></table>\n";
        // echo "</td></tr></table>\n";
        echo "</form>\n";   

    	  
    }
    
    /**
     * show all stored $this->sea_info_arr
     * @param string $tmpmsg
     * @param string $error_mess
     */
    function matches_message_box($sea_info_arr, $tmpmsg, $error_mess) {
        
        
        $performed = "<font color=gray>Performed searches:</font><br>\n". implode('<br>',$sea_info_arr);
        
        $out_mess = $tmpmsg . $performed . $error_mess;
        
        $taketab = $this->tablename;
        $objLinkLib = new fObjViewC();
        $icon     = $objLinkLib->_getIcon($taketab);
        $nicename = tablename_nice2($taketab);
        htmlInfoBox( "Search summary for table &nbsp;&nbsp;".
            "<a href=\"view.tmpl.php?t=$taketab\"><font color=#FFFFFF><img src=\"".$icon."\" border=0> $nicename</a></font></B>"
            , $out_mess, "", "INFO" );
        echo "<br>\n";
        
    }
    
    function results_analyse($sql, $methods_info, $sea_info_arr, $parx) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $userError     = $methods_info['userError'];
        $userErrorTxt  = $methods_info['userErrorTxt'];
        $found_matches = $methods_info['found_matches'];
        $found_id      = $methods_info['found_id'];
        $where         = $methods_info['where'];
        $idx           = $methods_info['idx'];
        
        // ---- ANALYSE results ----
        $taketab = $this->tablename;
        
        
        if ( $userError ) {
            echo "<br>\n";
            htmlErrorBox( "Usability error", $userErrorTxt );
            echo "<br>\n";
            
            $result = array('forward_url'=>NULL, 'matches'=>0 );
            return $result;
            
        }
        
        if (!$found_matches) {
            
            $tmpmsg = "<font color=red size=+1><B>No match.</B></font><br><br>";
            
            $error_mess='';
            if ( $userErrorTxt !="" ) {
                $error_mess = "<br><br><font color=red><b>Search error:</b></font> ".$userErrorTxt;
            }
            $this->matches_message_box($sea_info_arr, $tmpmsg, $error_mess);
            
            $result = array('forward_url'=>NULL, 'matches'=>$found_matches);
            return $result;
            
        }
        
        
        $forward_url = "";
        if ($found_matches==1) {
            $forward_url = "edit.tmpl.php?t=".$taketab."&id=".$found_id;
            echo "<b>Single object found!</b><br>\n";
            
            $result = array( 'forward_url'=>$forward_url, 'matches'=>$found_matches );
            return $result;
        }
        
        if ($found_matches>1) {

            $utilLib = new fSqlQueryC($taketab);
            if ( $parx["bool"]==NULL )  $utilLib->cleanCond();
            $utilLib->addCond( $where, '', implode('; ',$sea_info_arr) );
            $utilLib->queryRelase(); // save in session vars : make it user-global
            
            if ( $_SESSION['userGlob']["g.debugLevel"]>0 ) {
                $sql_tmp = $utilLib->full_query_get($taketab);
                debugOut('SQL-CMD:'.$sql_tmp, $FUNCNAME, 1);
            }
            $forward_url = "view.tmpl.php?t=".$taketab;
            echo "<br><B>Found multiple matches!</B><br><br>";
            
            $result = array( 'forward_url'=>$forward_url, 'matches'=>$found_matches );
            return $result;
            
        }
    }
    
    function forward( $sea_info_arr, $forward_url, $delay=0 ) {
        // forward page
        if ( $parx["bool"] != "") {
            $this->searchInfo["bool"] = "added to last condition";
        }
        
        $stopforwardText="";
        if ( $_SESSION['userGlob']["g.debugLevel"]>1 ) {
            $stopforwardText= " <B>DEBUG:</B> Stopped due to debug mode.\n";
            $stopforward = 1;
        }
        if ( $this->parx["stopforward"] ) {
            $stopforward=1;
            $stopforwardText="Due to 'Stop forward' option.";
        }
        
        if ($stopforward) {
            echo "<br>";
            $searchdone = implode('<br>',$sea_info_arr);
            
            htmlInfoBox("SEARCH description:","" ,"open","INFO");
            echo "- ".$searchdone."<br>";
            if ($this->searchInfo["bool"]!="") echo "- ".$this->searchInfo["bool"];
            htmlInfoBox("","" ,"close", "");
            echo "<br><font color=gray><b>Stop forwarding</b>: ".$stopforwardText."</font><br>";
            
        }
        
        // echo "<br>[<B><a href=\"".$forward_url."\">Forward query ==&gt;</a></B>]\n";
        
        if ($stopforward) {
            echo "&nbsp;&nbsp;[<a href=\"".$_SERVER['PHP_SELF']."\">Quick search form</a>]<br><hr>\n";
            htmlFoot();
        }
        
        echo "<br>";
        js__location_replace($forward_url, '... to the results',NULL, $delay );
        
        htmlFoot();
    }
    
}

// ----------------------------------------------------------------------------------------------------------


$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot(); 

$parx     =$_REQUEST['parx'];
$idx      =$_REQUEST['idx'];
$tablename=$_REQUEST['tablename'];
$go       =$_REQUEST['go'];


if ($tablename!=NULL) {
	//$nicename_take   = tablename_nice2($tablename);
	//$objLinkLib = new fObjViewC();
	//$icon = $objLinkLib->_getIcon($tablename);
}

$title        = 'Quick search';


$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "tool";
$infoarr["obj_name"]= $tablename;

$infoarr["locrow"]= array( array('home.php', 'home' ) );
// $infoarr['icon']     = "images/ic.quicks.gif";
$infoarr['help_url'] = "glob.obj.qsearch.html";

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

$mainLib = new fSearchLib();
list($idx, $tablename) = $mainLib->init($parx, $idx, $tablename, $go); // the input could change ...

$guiLib = new fSearchGuiSub();
$guiLib->set_table($tablename, $parx);

gHtmlMisc::func_hist("glob.obj.qsearch", $title,  'glob.obj.qsearch.php' );
 
echo "<blockquote>\n";

if ( $mainLib->go<=2 ) {  // take from STORE   
    $mainLib->takeFromStore();
}    
  
if ( $mainLib->go>=2 ) {  // save some parameters in $_SESSION['userGlob']
	$mainLib->saveParams($sql);
}                             

$fields         = array();    
$fields["algo"] = $mainLib->parx["algo"];
   
$mainLib->doTableAuto();

$inputerror    = "";
$showForm_flag = 0;   
 
if ( $mainLib->go == 2 AND $mainLib->idx=="" AND $mainLib->taketab!="" ) {
	// got to the table
	$forward_url="view.tmpl.php?t=".$mainLib->taketab;
	echo "No Search-text given, lets go to the table.<br>";
	?>
	<script language="JavaScript">
	location.href="<?echo $forward_url?>";
	</script>
	<?php
	htmlFoot(); 
} 

if ( $mainLib->go > 0 ) {   
    if ( $mainLib->idx=="" ) {
        $showForm_flag = 1;
        // $inputerror = 'Please give an ID or a name of an object!';
    } 

    if ( $mainLib->taketab=="") {   // no force to search ?
        $showForm_flag = 1;
        $inputerror = 'Please select a table!';
    }    
}

if ( $mainLib->go<2  ) {
    $showForm_flag = 1;
}
 
if ( $showForm_flag ) {
    echo "<br>\n";
    $guiLib->formshow( $sql, $mainLib->idx, $mainLib->taketab, $mainLib->parx );
    if ($inputerror!="") $guiLib->htmlFoot('ERROR', $inputerror);
    else  $guiLib->htmlFoot();
} 


$methods_info  = $mainLib->mainSearch($sql);
$sea_info_arr  = $mainLib->get_sea_info_arr();
$search_result = $guiLib->results_analyse($sql, $methods_info, $sea_info_arr, $parx); 

if ($mainLib->parx['wild']>0) {
    // show original ids
    echo '<a href="'.$search_result['forward_url'] .'"><img src="images/but.list2.gif"> Show results in the standard list view</a><br><br>';
    
    $mainLib->show_obj_ranks($sql, $tablename);
    $guiLib->set_parx_val("stopforward", 1);   // stop auto forward ...
    
}

if ($search_result['forward_url'] !="") {
    $guiLib->forward( $sea_info_arr, $search_result['forward_url'] );
}

if (!$search_result['matches'] ) {
    
    // if nothing found ...
    
    if ($tablename=='CONCRETE_SUBST') {
        
        if ($parx["algo"]=='') {
            //
            // try a standard search in ABSTRACT_SUBST
            //
            $new_table = 'ABSTRACT_SUBST';
            echo '... try in table "'.tablename_nice2($new_table).'"<br>';
            
           
            $sua_search_lib = new fSearchLib();
            $sua_search_lib->init($parx, $idx, $new_table, $go);
            $guiLib->set_table($new_table, $parx);
            $search_result = $sua_search_lib->mainSearch($sql); 
            $sea_info_arr  = $sua_search_lib->get_sea_info_arr();
            
            if ($search_result['forward_url'] !="") {
                $guiLib->matches_message_box('', '');
                $guiLib->forward( $sea_info_arr, $search_result['forward_url'], 5000 );
            }
        }
    }
    
    $guiLib->formshow( $sql, $idx, $tablename, $parx);
}
    
$guiLib->htmlFoot();



