<?php
/**
 * show nice list of selection of JOUR_ENTRY (lab journal list) 
 * GLOBALS: $_SESSION["userGlob"]["o.EXP.labjourList"] serialized
 *    "imgInline"
 *    entryPerPage
 *    "mode" :
 *       ["owner"] - only MY entries
 *        "ALL"
 *    
 * @package obj.jour_entry.list.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param  $parx
 * 
 *  ["mode"] : 
 *     ["owner"] - only MY entries
 *      "ALL"
  	["page"] :  id of page, start with 1
		  
	["entryPerPage"] : number of entries per page [saveInUserGlob]
	["order"] 		 : ["date"], "explist"        [saveInUserGlob]
	["imgInline"]    : 0,1 -- show attached images inline
	["viewmode"]     : view mode
	   ['details']   : full details
	    'short'      : just the searched catch words, GOOGLE style
	   
	["proj_id"]      : create a new JOUR_ENTRY selection, get all entries from PROJ_ID
		  
	// not saved in prefs ???
	["searchstr"]    : searchstring
	["search_keys]   : string only keys AND connected
	["sea_my"]       : 0,1 only my data
	["action"] : 
	   ["normal"], 
	   "search", 
	   "setPrefs"
 */

session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("sql_query_dyn.inc");
require_once ("f.sql_query.inc");
require_once ("f.visu_list.inc"); 
require_once ("gui/o.JOUR_ENTRY.gui.inc"); 
require_once ("func_form.inc");

// --------------------------------------------------- 

class oJOUR_ENTRY_search {
    
    function __construct( &$sql, $parx ) {
        
        $tablename= "JOUR_ENTRY";
        $sqlAfter = get_selection_as_sql( $tablename );
        
        if ($parx['proj_id']>0) {
            // get all lab-entries from PROJECT, create new $sqlAfter
            $sqlWhere = 'JOUR_ENTRY_ID in ('. cProjSubs::getTableSQL( $parx['proj_id'], $tablename ).')';
            $dynSqlLib = new fSqlQueryC($tablename);
            $dynSqlLib->cleanCond();
            $dynSqlLib->addCond($sqlWhere);
            $dynSqlLib->queryRelase();
            $sqlAfter = $dynSqlLib->get_sql_after();
        }
        
        $this->info_text='';
        $this->tablename = $tablename;
        // $this->className = "journalEntry";
        $this->sqlAfter  = $sqlAfter;
        $this->parxNow   = $parx;
    }
    
    /**
     * 
     * @param object $sql
     * @param string $taketab  // name of table
     * @param string $pkname
     * @param string $where
     * @param array $option
     *   "BOOL" => "AND", "OR" => if set,  add new CONDITION to old condition
         "myData" => 0,1
     * @return int
     */
    private function _searchx(&$sql,string $taketab, $pkname,  $where,  $option=NULL   ) {

            $found_matches=0;
            
            $join2 = "";
            if ($option["myData"]>0) {
                $join2 = " JOIN CCT_ACCESS a ON a.CCT_ACCESS_ID=x.CCT_ACCESS_ID";
            }
            
            $sqlAfter = $taketab." x ".$join2;
            
            if ($where!="") $sqlAfter .= " where ".$where;
            
            if ($option["bool"]!="") {
                // add new CONDITION to old condition
                $sqlAfter = get_selection_as_sql( $taketab );
                if ($where!="") $sqlAfter .= " AND (".$where.")";
            }
            
            $sqls = "select count(".$pkname.") from ".$sqlAfter;
            // echo "_searchx: SQL: $sqls<br>";
            
            $sql->query($sqls);
            $sql->ReadRow();
            $found_matches = $sql->RowData[0];
            
            
            return $found_matches;
    }
    
    private static function _str2sql($searchstr) {
        $sqlstr = str_replace ("'", "''",$searchstr);
        return ($sqlstr);
    }
    
    function getMyEntries( &$sqlo ) {

        $tablename = $this->tablename;
        $searchtxt = $_SESSION['sec']['db_user_id'];
        $searchCol = "a.DB_USER_ID";
        $conditionCode='=';
        
        $utilLib = new fSqlQueryC($tablename);
        $utilLib->cleanCond();
        $utilLib->addJoin("CCT_ACCESS");
        $utilLib->add_column_cond($sqlo, $searchCol, $conditionCode, $searchtxt);
        
        $this->sqlAfter = $utilLib->get_sql_after( );
        $utilLib->queryRelase();
        
        $sqlo->Quesel("count(1) FROM " . $this->sqlAfter);
        $sqlo->ReadRow();
        $expnum = $sqlo->RowData[0];
        return ($expnum);
    }
    
    private function add_info($text) {
        $and =  $this->info_text!=NULL ? ' and ' : '';
        $this->info_text .= $and . $text;
    }
    
    private static function _search_word_2sql($column, $valx) {
        $val_upper_sql = "UPPER('%".self::_str2sql($valx)."%')";
        $sql_sub_cond  = "UPPER(".$column.") like ".$val_upper_sql;
        return $sql_sub_cond;
    }
    
    private static function build_from_main_input($searchStr, $addnotes) {
        
        $searchStr_arr  	 = explode(" ",$searchStr);
        
        $mainname  = "x.NAME";
        $mainnotes = "x.NOTES";
        $keyx      = "x.KEYS";
            
        $tmpand     = "";
        $where      = "";
        //$whereNotes = "";
        $whereName  = "";
        
        foreach($searchStr_arr as $valx) {
            
            $valx = trim ($valx);
            if ($valx==NULL) {
                continue;
            }
            
            $whereName    .= $tmpand . "( ";
            $whereName .= self::_search_word_2sql($mainname, $valx);
            $whereName .= " OR " . self::_search_word_2sql($keyx, $valx);
            if ($addnotes>0 )  $whereName .=  " OR " . self::_search_word_2sql($mainnotes, $valx);
            
            $whereName .= " ) ";
            $tmpand = " AND ";
                
        }
        if ($whereName!="") $where = "( ".$whereName." ) ";
        
        return $where;
    }
    
    private static function build_from_key_input($searchStr) {
        $searchStr_arr  	 = explode(" ",$searchStr);
        $keyx      = "x.KEYS";
        $tmpand     = "";
        $where      = "";
        $where_full  = "";
        
        foreach($searchStr_arr as $valx) {
            
            $valx = trim ($valx);
            if ($valx==NULL) {
                continue;
            }
            
            $where_full .= $tmpand;
            $where_full .= self::_search_word_2sql($keyx, $valx);
            $tmpand     = " AND ";
 
        }
        if ($where_full!="") $where = "( ".$where_full." ) ";
        
        return $where;
    }
    
    /**
     * translate user input to a search in NAME, KEYS, NOTES
     * @param object $sql
     * @return int
     */
    function search( &$sql ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $taketab   = $this->tablename;
        $tablename = $this->tablename;
        $pkname    = "x.JOUR_ENTRY_ID";
        $this->search_done_txt='';
        // $searchError = "";
        $addnotes 	 = 1;
        $where = '';
        $this->info_text = '';
        
        $searchStr= trim($this->parxNow["searchstr"]);
        $keyStr   = trim($this->parxNow["search_keys"]);
        
        if ($searchStr!=NULL) {
            $where = self::build_from_main_input($searchStr, $addnotes);
            $this->add_info('Name or Notes or Keys:'.$searchStr);
        }
        if ($keyStr!=NULL) {
            $where_x = self::build_from_key_input($keyStr);
            if ($where==NULL) {
                $where  = $where_x;
            } else {
                $where .= ' and '.$where_x;
            }
            $this->add_info('Keys:'.$keyStr);
            debugOut('KEY-SQL: '.$where, $FUNCNAME, 1);
        }

        if ( $this->parxNow["sea_my"] > 0 ) {
            if ($where!="") $where     .= " AND ";
            $where     .= "a.DB_USER_ID=".$_SESSION['sec']['db_user_id'];
            $searchXOpt = array("myData" => 1);
            $this->add_info('(only my entries)');
        }
        $found_matches = $this->_searchx( $sql, $taketab, $pkname, $where, $searchXOpt);
        
        $searchBrTag='';
        $searchdone .= $searchBrTag;
        $searchdone .= $this->info_text;
        $this->search_done_txt = $searchdone;
        //$doBreak     = 1;
        
        debugOut('User-Info: '.$this->info_text, $FUNCNAME, 1);
        
        if (!$found_matches) {
            
            $tmpmsg = '<span style="color:red; font-weight:bold; font-size:1.2em;">No match.</span><br><br>'.
                '<span style="color: gray;">Performed searches:</span><br>'."\n". $this->search_done_txt;
//             if ( $searchError !="" ) {
//                 $tmpmsg .= '<br><span style="color: red;"><b>Search error:</b></span> '.$searchError;
//             }
            htmlInfoBox( "Search summary", $tmpmsg, "", "INFO" );
            echo "<br>\n";
        } else {
            echo "<B>$found_matches</B> matches found. &nbsp; ".$this->search_done_txt."<br><br>\n";

            $dynSqlLib = new fSqlQueryC($tablename);
            $dynSqlLib->cleanCond();
            $dynSqlLib->addJoin("CCT_ACCESS");
            $dynSqlLib->addCond($where, '', $this->info_text);
            $dynSqlLib->queryRelase();
            $sqlAfter = $dynSqlLib->get_sql_after();

            $this->sqlAfter  = $sqlAfter;  // no sorting !!!
        }
        
        return ($found_matches);
    }
    
    function get_search_done_txt(){
        return $this->search_done_txt;
    }
    
    function get_sqlAfter() {
        return $this->sqlAfter;
    }
}

/**
 * LAB JOURNAL report
 * use $_SESSION["userGlob"]["o.EXP.labjourList"]
 *
 */
class oExpLabJourList {
    
    private $parxNow; // INPUT params
    private $parx;    // session params

    function __construct( &$sql, $parx ) {
    
    	$tablename= "JOUR_ENTRY";

    	$this->colorblue = "#CCDDEE";
    	$this->sublib    = new oEXPlabjourC();
    	$this->tablename = $tablename;
    	$this->sqlAfter  = '';
    	$this->parxNow   = $parx;
    	
    	$this->orderNice = array(
    		"date"    => "[by experiment date, newest first]",
    		"dateasc" => "by experiment date, oldest first", 
    		"explist" => "as in 'lab journalentry' list");
    	
    	$this->parx = unserialize($_SESSION["userGlob"]["o.EXP.labjourList"]);
    	if ( $this->parx["entryPerPage"]<=0 ) $this->parx["entryPerPage"] = 5;
    	
    	if ( $parx["action"] == "setPrefs") {
    		$this->parx = NULL;
    		$this->parx["order"] = $parx["order"];
    		if ($parx["entryPerPage"]>0) $this->parx["entryPerPage"] = $parx["entryPerPage"];
    		if ($parx["imgInline"]>0)    $this->parx["imgInline"]    = $parx["imgInline"];
    		if ($parx["wrap"]>0) 		 $this->parx["wrap"] = 1;
    		$this->save_sess_vars();
    	} 
    	
    	if ($this->parxNow['action']=='search') {  
    	    $mode='ALL';
    	    if ($this->parxNow['sea_my']) $mode='owner';
    	    $this->parx["viewmode"] = $this->parxNow['viewmode'];
    	    if (!$this->parx["viewmode"]) $this->parx["viewmode"]='details';  // DEFAULT ..
    	    $this->parx["mode"] = $mode;
    	    $this->save_sess_vars();
    	}
    	
    	if ( $this->parxNow["page"]<=0 ) $this->parxNow["page"] = 1;
    	
    	if ( $this->parx["order"] == "" ) $this->parx["order"] = "date";
    	if ( $this->parx["entryPerPage"]<=0 ) $this->parx["entryPerPage"] = 5;
    	$this->sublib->setParx($this->parx);
    	
    	$ordercrit = "";
    	switch ( $this->parx["order"] ) {
    		case "date":
    			$ordercrit = "x.EXEC_DATE DESC NULLS LAST ";
    			break;
    		case "dateasc":
    			$ordercrit = "x.EXEC_DATE ASC NULLS LAST ";
    			break;
    		case "explist":
    			$ordercrit = query_sort_org( $this->tablename );
    			break;
    		default:
    			 $ordercrit = "x.EXEC_DATE DESC";
    	}
    	$this->ordercrit = $ordercrit;
    }
    
    private function save_sess_vars() {
        $_SESSION["userGlob"]["o.EXP.labjourList"] = serialize( $this->parx );
    }
    
    function get_parx() {
        return  $this->parx;
    }
    
    function set_sqlAfter($sqlAfter) {
        $this->sqlAfter = $sqlAfter;
    }
    
    function getOrderInfo() {
    	return ($this->orderNice[$this->parx["order"]]);
    }
    
    function setSqlOrder() {
    	// FUNCTION: set the SQL-order criteria !
    	
    	if ($this->ordercrit!="") $this->sqlAfter  .=  " order by ".$this->ordercrit;
    }
    
    function setEntryNum($foundEntries) {
    	$this->foundEntries = $foundEntries;
    }
    function show_query_info() {
        echo '<span class="yGgray">No of entries: </span> <b>'.$this->foundEntries.'</b> &nbsp;&nbsp;&nbsp;';
        echo ' <span class="yGgray">Sorted: </span> '.$this->getOrderInfo()."<br>\n";
    }

    
    function HeadLine() {
    	
    	
    	$sepString = "&nbsp;<img src=\"images/ic.sepWhit.gif\">&nbsp;";
    	
    	echo '<table bgcolor="'.$this->sublib->HeadBackColor.'"  border=0 width=100%><tr><td style="padding:6px;" NOWRAP >';
    	
    	
    	echo ' <a href="#" onclick="xShowPrefs()" title="Settings"><img src="res/img/settings.svg" hspace=4 vspace=2 height=25></a> &nbsp; ';
    	echo " <a href=\"obj.jour_entry.ed1.php?action=create\"><img src=\"images/but.40.new.png\" height=25>New entry</a> &nbsp; ";

    	echo ' <a href="view.tmpl.php?t='.$this->tablename.'"><img src="images/but.list2.gif" title="Current selection"> Selection</a> ';
    	echo ' &nbsp; <a href="p.php?mod=DEF/o.JOUR_ENTRY.histogram"><img src="res/img/tag.svg"> Key/Tag cloud</a> ';
    	echo "<br>\n";
    	
        /**
         * parx[action] = search
         * parx[searchstr]
         * parx[sea_my]
         */
    	
    	$myData_checkbox='';
    	if ($this->parx["mode"]=='owner') $myData_checkbox=' checked';

    	// echo $sepString;
    	echo "<form style=\"display:inline;\" method=\"post\"  name=\"searform\"  action=\"".$_SERVER['PHP_SELF']."\" >\n";
    	echo '<input type=submit class="yButSmall" value="Search">'."\n";
    	echo "<input type=hidden name='parx[action]' value='search'>\n";
    	echo '<input type=text   name="parx[searchstr]" size=25 value="'.$this->parxNow['searchstr'].'">'."\n";
    	echo ' <input type=text  placeholder="keys" name="parx[search_keys]" size=25 value="'.$this->parxNow['search_keys'].'">'."\n";
    	// $this->searchFormAdvanced($this->parxNow);
    	echo " <input type=checkbox   name=\"parx[sea_my]\" value=\"1\" ".$myData_checkbox."> myData\n";
    	
    	if (!$this->parx["viewmode"]) {
    	    $this->parx["viewmode"]='details';
    	}
    	$tmp_check=array();
    	$tmp_text=array();
    	$tmp_text[0]='Details';
    	if ($this->parx["viewmode"]=='details') {
    	    $tmp_check[0]='checked';
    	    $tmp_text[0] = '<b>'.$tmp_text[0].'</b>';
    	}
    	$tmp_text[1]='Short';
    	if ($this->parx["viewmode"]=='short') {
    	    $tmp_check[1]='checked';
    	    $tmp_text[1] = '<b>'.$tmp_text[1].'</b>';
    	}
    	echo ' &nbsp; ';
    	echo ' &nbsp; '.$tmp_text[0].':<input type=radio  name="parx[viewmode]" value="details" '.$tmp_check[0].'>'."\n";
    	echo ' &nbsp; '.$tmp_text[1].':<input type=radio  name="parx[viewmode]" value="short" '.$tmp_check[1].'>'."\n";
    	// echo ' <a href="#" onclick="xShowSearchPrefs()" title="Search Settings"><img src="res/img/settings.svg" hspace=4 vspace=2 height=20></a> ';
    	
    	echo "</form>";
    	
    	// echo "&nbsp;<a href=\"help/robo/o.EXP.labjour.html#search\" target=_help><img src=\"images/help.but.gif\" TITLE=\"Help\" border=0></a> ";
    	// echo '<span style="padding-left:40px;">&nbsp;</span>'; // SPACER
    	// echo "<a href=\"".$_SERVER['PHP_SELF']."?parx[mode]=owner\"><b>My</b> entries</a>".$sepString."\n";
    	//$condEncode = urlencode("x.JOUR_ENTRY_ID>0"); // set a dummy condition, to tell the report, that you selected them all
    	// echo "[<a href=\"view.tmpl.php?t=".$this->tablename."&condclean=1&tableSCond=".$condEncode."\">All</a> ";

    	$this->prefForm();
    	
    	echo "</td>";
    	echo "</tr></table>\n";

    }
    
    // not used ..
    private static function searchFormAdvanced($parx) {
        echo '<div id="xSearchAdvBlock" style="display:none;">'."\n";
        echo ' <input type=text  placeholder="keys" name="parx[search_keys]" size=25 value="'.$parx['search_keys'].'">'."\n";
        echo '</div>'."\n";
    }
    
    private function prefForm() {
        
        echo '<div id="xPrefBlock" style="display:none;">'."\n";
        
        echo "<br>\n";
        
    	htmlInfoBox( "Visualization preferences", "", "open", "CALM" );
    	echo "<form style=\"display:inline;\" method=\"post\"  name=\"editform\"  action=\"".$_SERVER['PHP_SELF']."\" >\n";
    	echo '<input type=submit class="yButSmall" value="Set preferences">'."\n";
    	echo "Entries per page: <input type=text name=\"parx[entryPerPage]\" size=3 value=\"".$this->parx["entryPerPage"]."\">\n";
    	echo "Sort: ";
    	$tmptext = formc::selectFget( "parx[order]", $this->orderNice, $this->parx["order"]); 
    	echo $tmptext;
    	
    	$tmpchecked="";
    	if ($this->parx["imgInline"]>0) $tmpchecked=" checked";
    	echo "Show images inline: <input type=checkbox name=\"parx[imgInline]\" value=\"1\" $tmpchecked>\n";
    	
    	$tmpchecked="";
    	if ($this->parx["wrap"]>0) $tmpchecked=" checked";
    	echo "Autowrap: <input type=checkbox name=\"parx[wrap]\" value=\"1\" $tmpchecked>\n";
    	
    	echo "<input type=hidden name='parx[action]' value='setPrefs'>\n";
    	echo "</form>";
    	htmlInfoBox( "", "", "close");
    	
    	$url = "obj.jour_entry.docnew.php";
    	echo '<br><a href="'.$url.'"><img src="images/but.40.new.png" height=25> Create new LabBook</A> <br>'."\n";
    	
    	echo '</div>'."\n";
    }
    
    /**
     *  FUNCTION:  - manage page control of long lists of journal entries
    	See_also:  _default_script.php >> listViewPageControl()
     * @param object $sql
     * @param object $sql2
     */
    function loopy (&$sql, &$sql2) {
        
        $entryPerPage = $this->parx["entryPerPage"];
        
        $search_arr = array(
            'searchstr'  => $this->parxNow['searchstr'],
            'search_keys'=> $this->parxNow['search_keys']
        );
        $this->sublib->set_search_arr($search_arr);
        if ($this->parx['viewmode']=='short') {
            $entryPerPage = 30; // fix ...
        }
        
        $this->sublib->startTable();
    	$sqlAfter = $this->sqlAfter;
    	$sqlsLoop = "SELECT x.* FROM ".$sqlAfter;
    	$sql->query($sqlsLoop);
    	
    	$cnt = 0;
    	$this->moreExists=0;
    	$moreExists = 0;
    	$startShow = ($this->parxNow["page"]-1) * $entryPerPage + 1;
    	$endShow   = $startShow + $entryPerPage - 1;
    	
    	// echo "startShow: $startShow endShow:$endShow entryPerPage: ".$this->parx["entryPerPage"]."<br>";
    	
    	while ( $sql->ReadArray() ) {
    		$showit = 0;
    		$cnt++;
    		$thisData = $sql->RowData;
    		
    		
    		if ( $cnt >= $startShow ) $showit=1;
    		if ( $cnt > $endShow )   {
    			$moreExists = 1;
    			$cnt--;
    			break;
    		}
    		
    		if ($showit) {
    		    if ($this->parx['viewmode']=='short') {
    		        $this->sublib->oneEntrySlim( $sql2, $thisData);
    		    } else {
    		        $this->sublib->oneEntryOut( $sql2, $thisData);
    		    }
    		}
    	}
    	
    	if ($moreExists) {
    		$this->moreExists=1;
    	}
    	
    	$this->sublib->stopTable();
    	
    	$this->showinf["lastshow"]  = $cnt;
    	$this->showinf["startShow"] = $startShow;
    	$this->showinf["endShow"]   = $endShow;
    	$this->showinf["entryPerPage"] = $entryPerPage;
    	
    }
    
    function postLoopy ( &$sql) {
    	// finish  with shower
    	
        $entryPerPage = $this->showinf["entryPerPage"];
    	
    	echo "&nbsp;&nbsp;&nbsp;Entries: <b>".$this->showinf["startShow"]."</B>...<b>".$this->showinf["lastshow"]."</B> of ".$this->foundEntries;
    	echo "&nbsp;&nbsp;&nbsp;&nbsp;Page: ";
    	
    	$pageCnt   = 1;
    	$firstPoints     = 0;
    	$pagesShowAround = 5;
    	$thisPage  = $this->parxNow["page"];
    	if ($thisPage<=5) $pagesShowAround = 10 - $thisPage + 1;
    	
    	$pagemax = ceil ($this->foundEntries / $entryPerPage);
    	
    	if ( $pagemax>1 AND $this->parxNow["page"]>1) echo  "&nbsp;<a href=\"".$_SERVER['PHP_SELF']."?parx[page]=".($this->parxNow["page"]-1)."\">&lt;&lt;prev</a>&nbsp;&nbsp;";
    	
    	while ($pageCnt <= $pagemax) {
    	
    		$pagePrintFlag=0;
    		$pageOut = "<a href=\"".$_SERVER['PHP_SELF']."?parx[page]=".$pageCnt."\">".$pageCnt."</a>";
    		// echo "DEBBB: " . abs($pageCnt-$thisPage).":$pagesShowAround ";
    		if ( $pageCnt==$thisPage ) $pageOut = "<b>" . $pageOut . "</b>";
    		
    		if ( (abs($pageCnt-$thisPage)<$pagesShowAround) OR ($pageCnt==1)  ) {
    				echo $pageOut . " ";
    				$pagePrintFlag=1;
    		}
    		
    		if ( ($pageCnt<$thisPage) AND !$pagePrintFlag AND !$firstPoints) {
    			echo " ... ";
    			$firstPoints=1;
    		}
    		
    		if ( ($pageCnt>$thisPage) AND !$pagePrintFlag ) {
    			echo " ... ";
    			break;
    		}
    		$pageCnt++;
    	}
    	if ( $pagemax>1 AND $this->parxNow["page"]<$pagemax) echo  "&nbsp;&nbsp;<a href=\"".$_SERVER['PHP_SELF']."?parx[page]=".($this->parxNow["page"]+1)."\">next &gt;&gt;</a> ";
    	
    	echo "<br><br>\n";
    	echo "<ul>";
    	echo "<br><br>\n";
    	// $this->prefForm();
    }

}

class oJOUR_ENTRY_mgr_query {
    
    public $sqlAfter;
    
    
    private function count_entries($sqlo) {
        $sqls = "count(JOUR_ENTRY_ID) from ".$this->sqlAfter;
        $sqlo->Quesel($sqls);
        $sqlo->ReadRow();
        $found_matches = $sqlo->RowData[0];
        return $found_matches;
    }
    
    /**
     * 
     * @param object $sqlo
     * @param array $parx
     * @param array $config_arr
     * @return number
     */
    function manage_query(object $sqlo, $parx, &$config_arr) {
        
        $tablename = 'JOUR_ENTRY';
        $this->search_done_txt = '';
        $utilLib  = new fSqlQueryC($tablename);
        
        if ( $parx["action"] == "search" ) {
            
            $search_lib     = new oJOUR_ENTRY_search($sqlo, $parx);
            $foundEntries   = $search_lib->search( $sqlo );
            $this->sqlAfter = $search_lib->get_sqlAfter();
            $this->search_done_txt = $search_lib->get_search_done_txt();

        } else {
        
            if ( $utilLib->get_sql_info()==NULL ) {
                // produce new query ...
                if ($config_arr['mode']=='owner') {
    
                    $searchtxt = $_SESSION['sec']['db_user_id'];
                    $searchCol = "a.DB_USER_ID";
                    $conditionCode='=';
                    $utilLib->cleanCond();
                    $utilLib->addJoin("CCT_ACCESS");
                    $utilLib->add_column_cond($sqlo, $searchCol, $conditionCode, $searchtxt);

                    $utilLib->queryRelase();
                }
            }
            $this->sqlAfter = $utilLib->get_sql_after( );
            $this->search_done_txt = $utilLib->get_sql_info();
            $foundEntries = $this->count_entries($sqlo);
        
        }

        return $foundEntries;
    }
    
    function get_search_done_txt(){
        return $this->search_done_txt;
    }
}

// *********************************************************************************
global $error;
$FUNCNAME ='MAIN';
$parx       = $_REQUEST['parx'];
$tablename	= "JOUR_ENTRY";

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] ); 
$sql2  = logon2( );
if ($error->printLast()) htmlFoot();


$title   = "Lab journal report";

$infoarr = NULL;
$infoarr["title"] = $title;
$infoarr["form_type"]= "list";
$infoarr['help_url'] = 'o.JOUR_ENTRY.html';
$infoarr['icon'] 	 = 'images/icon.JOUR_ENTRY.svg';
$infoarr['icon_y'] 	 = 25;
$infoarr["obj_name"] = $tablename;
$infoarr["obj_row"]  = -1;
$infoarr["design"]   = 'slim';

$infoarr["javascript"] = '
function xShowPrefs() {
    var x = document.getElementById("xPrefBlock");
    if (x.style.display === "none") {
        x.style.display = "block";
    } else {
        x.style.display = "none";
    }
}
function xShowSearchPrefs() {
    var x = document.getElementById("xSearchAdvBlock");
    if (x.style.display === "none") {
        x.style.display = "inline-block";
    } else {
        x.style.display = "none";
    }
}
';

// $infoarr["version"] = '1.0';	// version of script


$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<style type=\"text/css\">\n";
echo ".doc-lx1 { \n";
echo "  margin-left:9px; margin-right:9px;\n";
echo "} \n";
echo ".xgray { color:gray }\n";
echo "</style>\n";

$accCheckArr = array('tab'=>array('read'), 'obj'=>array() );
$pagelib->do_objAccChk($sql, $accCheckArr);
$pagelib->chkErrStop();

$journObj    = new oExpLabJourList($sql, $parx);

debugOut('(508) parx:'.print_r($parx,1), $FUNCNAME, 1);

gHtmlMisc::func_hist("obj.exp.labjourList", $title, $_SERVER['PHP_SELF'] );
$journObj->sublib->setCss();
$journObj->HeadLine();

$config_arr = $journObj->get_parx();

echo '<ul>'."\n";

$query_lib = new oJOUR_ENTRY_mgr_query();
$foundEntries = $query_lib->manage_query($sql, $parx, $config_arr);

if (!$foundEntries) {
    //TBD: ...
	htmlFoot("No Lab journal entries found for current condition.");
}
   
$journObj->set_sqlAfter($query_lib->sqlAfter);
$journObj->setSqlOrder();
$journObj->setEntryNum($foundEntries);

$journObj->show_query_info();

$journObj->loopy ( $sql, $sql2 );
$journObj->postLoopy($sql);

htmlFoot("</ul><hr>");
