<?php
require_once('db_x_obj.inc');
require_once('f.objview.inc');
require_once 'o.proj.profile.inc';

/**
 * show Object navigator
 * - called by edit.tmpl.php and frame.left.nav.php
 * - session_start() must be done in the calling script !!!
 * - dynamic include of OBJECT dependend scripts:  'obj.'.$tablename_l.'.xfunc.inc'
 *
 
 * @package o.PROJ.explorer.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @global $_SESSION['userGlob']
 * 				["o.PROJ.sop.TABLE] ...
        ["o.PROJ.pref"] = array ("maxob"=> num, "slim"=>0|1, "addinfown" => 0|1,
             "notes"=> number of shown chars: 0 means OFF
             "addinfdate" => 0|1 )
 */ 
class oPROJ_navC {
    
    var $proj_id; // int or string 'NULL'
    var $ROOT_IMG;
    private $cnt_proj;
    private $cnt_all_obj;
    private $query_condition;
    
    /**
     * 
     * @param int $id or string 'NULL'
     * @param string $pmode
     *     "" e.g. edit.tmpl.php
           "leftframe" => <html><body> must come from caller!   
     */
    public function __construct($id, $pmode) {
        $this->proj_id = $id;
        $this->headerIsOut=0;
        $this->ROOT_IMG  = "db:";
        $this->pmode = $pmode;
        $this->query_condition='UNDEFINED';
        $this->cnt_proj=0;
        $this->cnt_all_obj=0;
        
        $this->query_condition = 'PRO_PROJ_ID is NULL';
        if ( $this->proj_id!='NULL' ) {
            $this->query_condition = "PRO_PROJ_ID=".$this->proj_id;
        }
    }
    
    // get distinct tables in project
    public function get_proj_tables($sqlo) {
        
        if ($this->proj_id=='NULL') return array();
        
        $projTables = array(); // array[table]=1
        $tmpProjTab = array();
        $sqls="select distinct(TABLE_NAME) from PROJ_HAS_ELEM where PROJ_ID=".$this->proj_id;
        $sqlo->query($sqls);
        while ( $sqlo->ReadRow() ) {
            $tmpProjTab[$sqlo->RowData[0]] = 1;
        }
        
        if (sizeof($tmpProjTab)) {  // resort types: PROJ and EXP to front
            $projTables = array();
            if (isset($tmpProjTab["PROJ"])) $projTables["PROJ"]=1;
            if (isset($tmpProjTab["EXP"]))  $projTables["EXP"] =1;
            $projTables = $projTables + $tmpProjTab; // merge
        }
        return $projTables;
    }
    
    /**
     * set header mode
     * @param int $mode
     * 		1 : header is already out
     */
    public function setHeadFlag($mode) {
        $this->headerIsOut = $mode;
    }
    
    /**
     * show HTML-page head
     * @param  $title
     */
    public function pageHead($title) {
        if ($this->headerIsOut) return; // already out
        
        $headOpt = array('noBody'=>1);
        $htmlHeadObj = new gHtmlHead();
        $htmlHeadObj->_PageHeadStart ($title );
        // css
        echo '<style type="text/css">'."\n";
        echo 'td  { white-space: nowrap; padding: 0px;}'."\n";
        echo '</style>'."\n";
        echo '<link rel="stylesheet" type="text/css" href="res/css/glob.menu.css">'."\n";
        $htmlHeadObj->_PageHeadEnd($headOpt);
        
        $this->headerIsOut = 1;
    }
    
    /**
     * throw error
     * @param $text
     */
    public function errorExit($text) {
        $htmlHeadObj = new gHtmlHead();
        
        if (!$this->headerIsOut) {
            
            $htmlHeadObj->_PageHead ( 'project' );
        }
        $htmlHeadObj->htmlFoot('ERROR',$text);
        exit;
        
    }
    
    public function getClsXtra($class_name) {
        // RETURN: array ("clsobj"=>, "icon"=>)
        $retarr = NULL;
        $tmpfile = "../lab/o.PROJ.c.".$class_name.".inc";
        $tmpClsName = "PROJ_c_".$class_name;
        if (file_exists($tmpfile)) {
            require_once($tmpfile);
            $retarr["clsobj"] = new $tmpClsName();	// create class
            if (method_exists ($retarr["clsobj"],'xinfo')) $retarr["xinfo"] = 1;
        }
        
        if (file_exists("../lab/images/icon.PROJ.$class_name.gif"))
            $icon = "../lab/images/icon.PROJ.$class_name.gif";
            elseif (file_exists("images/icon.PROJ.$class_name.gif"))
            $icon = "images/icon.PROJ.$class_name.gif";
            else
                $icon = "images/icon.PROJ.unknown_class.gif";
                $retarr["icon"] = $icon;
                return ($retarr);
    }
    
    
    /**
     * show all Business objects
     * - output: $this->cnt_all_obj
     * @param object $sqlx
     * @param object $sqly
    
     * @param array $projTables :  distinct tables
     * @param array $opt
     *      "noCheck", "target", "maxob",
     "objOpt" => object view Options; format: array(TABLENAME = array(KEY => VAL))
     "showOnwer" => show owner name => WARNING: is time consuming!
     "showDate" => show date => WARNING: is time consuming!
     "notes"    => [0] or number of chars
     "markObj"
     "noCheck" : 0,1
     */
    function bos_infos( &$sqlx, &$sqly, &$projTables, $opt=NULL ) {

        $FUNCNAME = "bos_infos";
        $id=$this->proj_id;
        $objOpt    = array();
        $target    = $opt["target"];
        $maxobjcnt = $opt["maxob"];
        $showcheck = 1;
        $this->cnt_obj     = 0;
        $this->cnt_all_obj = 0;
        $objclass = array();
        
        // echo '<tr><td colspan=2  NOWRAP>'."\n";
        
        if ($opt["noCheck"]) $showcheck = 0;
        if ($maxobjcnt<2   ) $maxobjcnt = 50; // fallback
        //$nowColspanCnt=1;
        //if ( !$opt["showOnwer"]) $nowColspan= " colspan=2";
        //else $nowColspan= "";
        
        // for all table types ...
        if (empty($projTables))  return;
        
        foreach( $projTables as $tmp_tablename=>$dummy ) {
            
            $colname	    = importantNameGet2 ($tmp_tablename);
            $tmp_idname	    = PrimNameGet2      ($tmp_tablename);
            
            /* OBJECT ORIENTED FUNCTIONS */
            if ( empty($objclass[$tmp_tablename]) ) {   // create the class only ONCE for an object-type
                
                $tablename_l=strtolower($tmp_tablename);
                $filename = 'obj.'.$tablename_l.'.xfunc.inc';
                $retu = file_exists($filename);
                if ( $retu ) {
                    require_once($filename);
                    $tmp_func= 'c'.$tmp_tablename;
                    $objclass[$tmp_tablename] = new $tmp_func();
                    if ( $_SESSION['userGlob']["o.PROJ.sop.".$tmp_tablename]!="" ) { // options ???
                        $objOpt[$tmp_tablename] = unserialize($_SESSION['userGlob']["o.PROJ.sop.".$tmp_tablename]);
                    }
                    if ($opt["objOpt"][$tmp_tablename]!="") $objOpt[$tmp_tablename] = $opt["objOpt"][$tmp_tablename]; // overwrite if given
                } else {
                    $objclass[$tmp_tablename] = 0; /* not exists */
                }
            }
            
            $sql_accessid= ", x.CCT_ACCESS_ID";
            $sql_impname = ", x.$colname";
            if ($colname==$tmp_idname) {
                $sql_impname = ", 1"; // SQL does not like same names after the SELECT
            }
            
            $sql_notes = "";
            if ( $opt["notes"]>0 ) {
                if (columnname_nice2($tmp_tablename, "NOTES") !="") // column exists ?
                    $sql_notes = ", NOTES";
            }
            
            $sqls = "SELECT p.prim_key, x.".$tmp_idname. $sql_accessid . $sql_impname . $sql_notes .
            " FROM $tmp_tablename x RIGHT OUTER JOIN ".
            "  ( SELECT prim_key FROM proj_has_elem WHERE proj_id = $id AND ".
            "    table_name='".$tmp_tablename."') p on p.prim_key=x.$tmp_idname ".
            "  ORDER by x.$colname";  // "left join" to find also BROKEN links!
            $sqlx->query($sqls, $FUNCNAME);
            
            $icon='images/icon.'.$tmp_tablename.'.gif';
            if ($tmp_tablename == 'PROJ') $icon='images/icon.PROJ_link.gif';
            if ( !file_exists($icon) )    $icon='images/icon.UNKNOWN.gif';
            $tmp_tab_meth = method_exists ($objclass[$tmp_tablename], 'inProjShowAtt' );
            
            
            
            // LOOP for ONE table-type $tmp_tablename !
            while ( $sqlx->ReadRow() AND ($this->cnt_obj<$maxobjcnt)) {
                
                $tmp_RealName  = "";
                $tmp_notes 	   = "";
                $tmp_PRIM_KEY  =  $sqlx->RowData[0];
                $obj_exist_KEY =  $sqlx->RowData[1];
                $cct_access_id =  $sqlx->RowData[2];
                if ($sql_impname!="")
                    $tmp_RealName=$sqlx->RowData[3]; // only if given in SQL command
                    if ($sql_notes !="") {
                        $tmp_notes = $sqlx->RowData[4];
                    }
                    
                    if ($opt["showOnwer"] OR $opt["showDate"]) {
                        $acc_data = access_data_getai( $sqly, $cct_access_id, 3 );
                    }
                    
                    $tmp_Name   = "[".$tmp_PRIM_KEY."]";
                    if ( $tmp_RealName!="" )  $tmp_Name = $tmp_RealName;
                    
                    echo "<tr id=t".$this->cnt_obj."><td>";
                    
                    if ($showcheck) echo " <input type='checkbox' name='sel[" .$tmp_tablename."][".$tmp_PRIM_KEY ."]' value='1'>";
                    
                    if ( $obj_exist_KEY ) {
                        if ( $tmp_tab_meth ) {
                            $tmp_optinfo = $objclass[$tmp_tablename]->inProjShowAtt($sqly, $tmp_PRIM_KEY, $objOpt[$tmp_tablename]);
                        } else {
                            $tmp_optinfo = '';
                        }
                        
                        echo ' <a href="edit.tmpl.php?&t='.$tmp_tablename.'&id='.$tmp_PRIM_KEY.'"'.$target;
                        if ( $opt["markObj"] ) echo " onclick=\"th_snc('t".$this->cnt_obj."')\"";
                        echo '>';
                        echo '<img src="'.$icon.'" border="0"> '.$tmp_Name.'</a> '.$tmp_optinfo;
                    } else {
                        $broken_link_txt = '<font color="#800000">broken link</font>';
                        echo ' <img src="'.$icon.'" border="0"> ['.$tmp_PRIM_KEY.'] '.$broken_link_txt;
                    }
                    echo "</td>";
                    if ( $opt["notes"]>0 ) {
                        echo "<td>";
                        if ($tmp_notes!="")
                            $tmp_notes = "<font color=gray>".htmlspecialchars(substr($tmp_notes,0,$opt["notes"]))."</font>";
                            echo $tmp_notes."</td>";
                    }
                    if ( $opt["showDate"] ) {
                        echo "<td>".$acc_data["crea_date"]."</td>";
                    }
                    if ($opt["showOnwer"]) {
                        echo "<td>".$acc_data["owner"]."</td>";
                    }
                    echo "</tr>\n"; // "<br>\n";
                    $this->cnt_obj++;
                    
            }  // END loop for one table-type
            if ( $this->cnt_obj>=$maxobjcnt ) break;
        }
        
        $this->cnt_all_obj = $this->cnt_obj;
        
        if ( $this->cnt_obj>=$maxobjcnt ) {  // TBD:5: please put this code in obj.proj.selector.php
            $sqlx->query("SELECT count(*) FROM proj_has_elem WHERE proj_id = $id");
            $sqlx->ReadRow();
            $maxelem = $sqlx->RowData[0];
            $this->cnt_all_obj = $maxelem;
            $rest    = $maxelem-$this->cnt_obj;
            if ($rest) echo "<tr><td>&nbsp;&nbsp;<B>... " . $rest .
            " more objects.</B> <font color=gray>Please change the ".
            "<a href=\"glob.obj.pref.php?tablename=PROJ&id=$id\">preferences</a> to show more.</font></td></tr>\n";
        }
        echo "\n</td></tr>\n";
            
    }  
    
    /**
     * OUTPUT: $this->pather, $this->depth_cnt
     *
     * @param object $sql
     * @param int $master_proj_id
     */
    function calc_path(&$sql, $master_proj_id) {
        $depth_cnt=0;
        $pather=array();
        while ( $master_proj_id && ($depth_cnt < 10) ) {
            
            $sqls= 'select pro_proj_id, name,proj_id from proj  where proj_id=' . $master_proj_id ;
            $sql->query($sqls);
            if ( $sql->ReadRow() ) {
                $master_proj_id= $sql->RowData[0];
                $master_name=    $sql->RowData[1];
                $pather[$depth_cnt][0]=$sql->RowData[2];
                $pather[$depth_cnt][1]=$master_name;
            }
            $depth_cnt++;
        }
        
        $this->pather = $pather;
        $this->depth_cnt = $depth_cnt;
    }
    
    private function _get_p_link($id) {
        $link = 'edit.tmpl.php?t=PROJ&id=' . $id;
        if ($this->pmode=='leftframe') $link = 'frame.left.nav.php?id='.$id.'&mode=proj';
        return $link;
    }
    
    // object is a favorite ?
    function proj_is_favorite($sqlo) {
        $help_lib = new oPROJ_bookmarks();
        if ($this->proj_id=='NULL') return 0;
        $answer   = $help_lib->obj_is_in_proj($sqlo, 'PROJ', $this->proj_id );
        return $answer;
    }
    
    function show_path($master_proj_master, $proj_name) {
        
        $id       = $this->proj_id;
        $pather    = &$this->pather;
        $depth_cnt = $this->depth_cnt;
        
        if (!$master_proj_master ) $linkto_proj = 'NULL';
        else $linkto_proj = $master_proj_master;
        //
        echo "<a href=\"".$this->_get_p_link($linkto_proj)."\" ><img src=\"images/but.projup.gif\" border=0 ".
            "TITLE=\"up\" hspace=3></a>";
        echo '<font color=gray><B>Folder:</B></font> ';
        
        if ($id!='NULL') {
            echo "<a href=\"".$this->_get_p_link('NULL')."\"><font color=#0000A0>".$this->ROOT_IMG."</font></a>/";
        }
        
        $tmps=1;
        $cnt=$depth_cnt-1;
        while ( $cnt >= 0) {
            $master_proj_id= $pather[$cnt][0];
            $master_name=    $pather[$cnt][1];
            echo "<a href=\"".$this->_get_p_link($master_proj_id)."\" >". $master_name . "</a>/";
            $cnt--;
            $tmps++;
        }
        
        echo "<B><a href=\"".$this->_get_p_link($id)."\" ><font color=#000000>". $proj_name . "</font></a></B>/";
    }
    
    /**
     * 
     * @param object $sql
     * @param object $sql2
     * @param array $singOpt
     *   "addinfown"
     *   "addinfdate"
     *   "notes"
     * @param array $popt
     *   "noCheck"
     *   "projplus"
     * @param string $target
     */
    function show_sub_projs(&$sql, &$sql2, $singOpt, $popt, $target) {
        global $error, $varcol;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $query_condition  = $this->query_condition;
        $projVarGetAcc = 0; // reduced SQL-LOAD ?
        if ($singOpt["addinfdate"] OR $singOpt["addinfown"] ) $projVarGetAcc = 1;
        
        // this query works also with $id='NULL'
        $natural_order = $sql->strOrderNaturalCompact('NAME');
        $sql->query("SELECT proj_id, name, extra_obj_id, cct_access_id, notes FROM proj WHERE $query_condition ORDER BY ".$natural_order);
        $this->cnt_proj = 0;
        $clsXtraArr = NULL; // array for Extra_class specials
        
        while ( $sql->ReadRow() ) { // TBD: introduce limit to show
            
            $sub_projid = $sql->RowData[0];
            $name       = $sql->RowData[1];
            $eobj_id    = $sql->RowData[2];
            $acc_id     = $sql->RowData[3];
            $tmp_notes  = $sql->RowData[4];
            
            if ($projVarGetAcc) $accvals = access_data_getai($sql2, $acc_id, 3);
            $icon = 'images/icon.PROJ.gif';
            
            if ($eobj_id) {
                $class_name = $varcol->obj_id_to_class_name((int)$eobj_id);
                if ($error->printLast()) return;
                
                if (!isset($clsXtraArr[$class_name])) $clsXtraArr[$class_name] = $this->getClsXtra($class_name);
                $icon = $clsXtraArr[$class_name]["icon"];
            }
            
            echo '<tr><td>'; // OLD: NOWRAP
            if ($popt["noCheck"]!=1) {
                echo "<input type=checkbox name=\"sel[PROJ_ORI][".$sub_projid ."]\" value=\"1\"> ";
            }
            
            if ($popt["projplus"]) {
                // edit.tmpl.php?t=PROJ&id=".$sub_projid."&mode=proj
                echo "<a href=\"frame.left.nav.php?id=".$sub_projid."&mode=proj\">".
                    "<img src=\"images/but.plus.gif\" border=0 TITLE=\"browse\"></a> ";
            }
            $htmltmp = htmlspecialchars($name);
            if (substr($name,0,1) == ".") {
                $htmltmp = "<font color=gray>$htmltmp</font>";
                $icon = 'images/icon.PROJ.inact.gif';
            }
            echo "<a href=\"edit.tmpl.php?t=PROJ&id=".$sub_projid."\"".$target."><img src=\"$icon\" border=0> $htmltmp</a>";
            echo "</td>";
            
            if ($singOpt["notes"]>0) {
                echo "<td><font color=gray>".htmlspecialchars(substr($tmp_notes,0,$singOpt["notes"]))."</font></td>";
            }
            
            if ( $singOpt["addinfdate"] ) {
                echo "<td>".$accvals["crea_date"]."</td>";
            }
            if ( $singOpt["addinfown"] ) {
                echo "<td>".$accvals["owner"]."</td>";
            }
            if ($eobj_id AND $clsXtraArr[$class_name]["xinfo"]) {
                $tmpinfo = $clsXtraArr[$class_name]["clsobj"]->xinfo($sql2, $sub_projid, $eobj_id);
                if ($tmpinfo!="") echo "<td>".$tmpinfo."</td>";
            }
            //  echo '<td>&nbsp;<font style="font-style:italic; color:#999999;">'.strtok($notes, "\n").'</td>';
            
            echo "</tr>\n";
            $this->cnt_proj++;
        }
        
    }
    
    // get number of all projects + objects
    function get_num_proj_obj() {
        return ($this->cnt_proj +  $this->cnt_all_obj);
    }
    
} // end of CLASS
