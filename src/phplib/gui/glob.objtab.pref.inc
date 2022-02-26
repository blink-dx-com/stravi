<?php
require_once ("javascript.inc" );
require_once ("sql_query_dyn.inc");
require_once ('func_form.inc');
require_once ('export/view.tmpl.extra.inc');
require_once ('o.USER_PREF.manage.inc');

/**
 * manage TABLE list settings
 * @package glob.objtab.pref.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class glob_objtab_pref {
    
    private $xcols;  // user selected cols
    /** 
     * current column prefs
     * @var array $_tab_cols_pref : tab_cols_pref_STRUCT
         (array(col_code => 0 or 1)) -- ordered list
             colcode:
              x.COL : standard column 
              a.COL : access column
              _xcol.COL : extra column
     */
    private $_tab_cols_pref; 
    private $tablename;
    
    /**
     *
     * @param string $tablename
     * @param array $colopt
     */
    function __construct($tablename, $colopt=NULL) {
        
        $this->tablename = $tablename;
        $this->COL_PREF_KEY='o.'.$tablename.'.viewcols';
        $this->access_id_has = cct_access_has2($tablename);
        $this->colNames  = columns_get_pos($tablename);
        $this->view_optg = unserialize($_SESSION['userGlob']["g.viewf.opt"]);
        
        $this->_cols_pref_exist=0;
        $this->_tab_cols_pref=array();
    }
    
    private function _init_col_settings() {
        $tablename = $this->tablename;
        $extra_lib       = new view_tmpl_extras($tablename);
        $this->_tab_cols_pref_sort = array();
        $this->_cols_pref_exist = 0;
        
        if (!empty($this->_tab_cols_pref)) {
            $i=0;
            $this->_cols_pref_exist = 1;
            foreach($this->_tab_cols_pref as $col_l=>$val_l) {
                $this->_tab_cols_pref_sort[$col_l]=$i;
                $i++;
            }
        }
        
        // get extented column information
        $this->xcols = array();
        $tablename_l =	strtolower($tablename);
        $xtra_file   = 'obj.'.$tablename_l.'.xfunc.inc';
        if ( file_exists($xtra_file) ) {
            require_once($xtra_file); 		/* object oriented extra functions */
            
            $tmp_func= 'c'.$tablename;
            $objclass = new $tmp_func();
            if (sizeof($objclass->xcolarr)) {
                foreach( $objclass->xcolarr as $infarrx) {
                    $col_text = current($infarrx);
                    $col_code = key($infarrx);
                    $col_rem  = $infarrx["rem"];
                    $this->xcols[$col_code]  = array($col_text, $col_rem);
                }
            }
        }
        
        if ($this->access_id_has) {
            $xcols_new   = $extra_lib->get_xcol_def();
            $this->xcols = array_merge($this->xcols, $xcols_new);
        }
    }
    
    public function set_column_config($tab_cols_pref) {
        $this->_tab_cols_pref = $tab_cols_pref;
        $this->_init_col_settings();
    }
    public function column_config_get() {
        return $this->_tab_cols_pref;
    }
    
    public function default_settings() {
        if ($_SESSION['userGlob'][$this->COL_PREF_KEY]!=NULL) {
            $this->_tab_cols_pref = unserialize($_SESSION['userGlob'][$this->COL_PREF_KEY]);
            $this->_cols_pref_exist=1;
        }
        $this->_init_col_settings();
    }
    
    
    
    // $defval: default value: 0,1
    private function _isChecked(  $colName,  $defval ) {
        
        $checked = 0;
        if (!$this->_cols_pref_exist) return $defval;
        if ( $this->_tab_cols_pref[$colName]>0 ) $checked = 1; // if set & value not = '0' then show
        
        return ($checked);
    }
    
    /**
     *
     * @param string $colName
     * @return
     *   -1 column unknown
     *   >=0 order num
     */
    private function _get_pref_order($colName) {
        if (!$this->_cols_pref_exist) return -1;
        if ( !array_key_exists($colName,$this->_tab_cols_pref_sort)) return -1;
        $ord = $this->_tab_cols_pref_sort[$colName];
        return $ord;
    }

    private function prefNameShow( $text, $align="" ) {
        // GLOBAL function!, do not change
        echo "<tr $align><td NOWRAP align=right><font color=gray>".$text.":</font></td>\n";
    }
    private function prefClose( $info="" ) {
        // GLOBAL function!, do not change
        echo "<td colspan=2 align=left>&nbsp;<I>".$info."</I></td></tr>\n";
    }
    
    /**
     * show one row of DB-column
     * @param array internal_row_STRUCT $input
     *
     */
    private function rowshow_colsel( $input ) {
        
        $col_name=$input['c'];
        $checked=$input['ch'];
        
        $class  ='class="sortx"';  // sortable ...
        $bgcolor=" bgcolor=#EDFEFF";
        $sortable=1;
        if ( $input['ty'] == "acc" ) {
            $bgcolor=" bgcolor=#C0EEC0";
        }
        $colNiceName=$input['na'];
        if ( $input['ty'] == "opt" ) {
            $bgcolor=" bgcolor=#DDEEFF";
            $colNiceName = "<font color=#909090>".$input['na']."</font>";
            $class='';
            $sortable=0;
        }
        echo '<tr' .$bgcolor. ' '.$class.'>';
        
        echo '<td align=right nowrap>';
        if ($sortable) echo '<img src="images/ic.modpx9.gif" style="float:left; padding-left:2px;"> ';
        echo $colNiceName. "</td>";
        echo "<td colspan=2 >";
        
        echo '<input type="hidden" name="cols_h['.$col_name.']" value="1">';
        echo '<input type="checkbox" name="viewcol['.$col_name.']" value="1"';
        if ($checked)  echo ' checked';
        echo ">";
        // echo "</td><td valign=top>";
        echo '&nbsp;&nbsp;<I>',$input['no'],'</I>'; // comment
        echo '</td>';
        //RAW COLNAME: echo '<td><span style="color:#C0C0C0">'.$col_name.'</span></td>';
        echo '</tr>',"\n";
    }
    
    
    
    /**
     * show table columns
     *
     * internal_row_STRUCT $input
     *     array(
     *     'c'=>'a.CREA_DATE',
     *     'na'=>'creation date',
     *     'no'=> notes ...
     *     'ty'=> "acc" => access info
     'opt'
     'ord'=> order INT
     *     );
     *
     * @param object $sql
     * @param string $colNames
     * @param array $xcols
     * @param array $colopt
     */
    public function sh_DBColumns( &$sql ) {
        
        $colNames  = &$this->colNames;
        $xcols=&$this->xcols;
        $tablename = $this->tablename;
        $colNamesX = NULL;
        
        if ($tablename=="SPOT_RESULT" OR $tablename=="EXP_RAW_RESULT" OR $tablename=="EXP_CHAN_RES" ) {
            
            require_once ("o.EXP.RESULT.inc");
            require_once ("class.history.inc");
            
            $hist_obj   = new historyc();
            $spotResLib = new gSpotResC();
            $tmpexpid = $hist_obj -> last_bo_get("EXP");
            $colNamesX= $spotResLib->get_column_names($sql, $tablename, $tmpexpid);
            $sql->query("SELECT h_exp_raw_desc_id FROM exp WHERE exp_id=".$tmpexpid);
            $sql->readRow();
            $tmprawdesc = $sql->RowData[0];
            if ($tmprawdesc)  {
                $sql->query("SELECT name FROM h_exp_raw_desc WHERE h_exp_raw_desc_id=".$tmprawdesc);
                $sql->readRow();
                $tmpname = $sql->RowData[0];
                echo "<font color=gray><B>INFO:</B> 'Nice name' shows the special result description type '<B>$tmpname</B>' from the last experiment in the history.</font><br>";
            }
        }
        
        //<table CELLSPACING=1 CELLPADDING=1 border=0 bgcolor=#b0c8d8 > #CFCFCF
        ?>
    	<tr>
    	<th class=x1>Nice name</th>
    	<th class=x1 colspan=2 align=left>Show &nbsp; Notes</th>

    	</tr>
    	<?php
    	// <th class=x1>Notes</th>
    	// <th class=x1 align=left>Code name</th>
    	
    	$col_rows=array();
    	
           
    	foreach( $colNames as $colName) { 
        
            
            $tmp_nice_found  = 0;
             
    		$colcomment = '';
            $colInfos   = colFeaturesGet($sql, $tablename, $colName, 0); 
            
            $showcol = $this->x_col_allowed($sql, $colName);
    
            if ( !$showcol ) {
                continue; 
            }
            
            $colNiceName="";
            if ( $colName == "CCT_ACCESS_ID" ) {
                $col_rows[] = array( 'c'=>'a.CREA_DATE',   'na'=>'creation date', 'no'=>'creation date',  'ty'=>'acc', 'defc'=>0);
                $col_rows[] = array( 'c'=>'a.MOD_DATE' ,   'na'=>'modification date', 'no'=>'creation date', 'ty'=>'acc', 'defc'=>0);
                $col_rows[] = array( 'c'=>'a.DB_USER_ID',  'na'=>'creator', 'no'=>'user of creation', 'ty'=>'acc', 'defc'=>1);
                $col_rows[] = array( 'c'=>"a.H_ALOG_ACT_ID",'na'=>"audit status", 'no'=>"audit status of object", 'ty'=>'acc', 'defc'=>0);
    			continue;
            } 
            
            if ( $colName == "EXTRA_OBJ_ID"  )  { 
                $colNiceName="extra class";
                $tmp_nice_found=1;
            }
            
            if ( !$tmp_nice_found )  {
                $colNiceName = $colInfos['NICE_NAME'];
            }
            
            if ( is_array($colNamesX) ) {
                $colNiceName = $colNamesX[$colName]; // empty nice names stay EMPTY !!!
            }
              
            if ( $colcomment=="" ) $colcomment = column_remark2($tablename, $colName);
            
            $code_col_name = 'x.'.$colName;

            $ret_primary_name=NULL;
            $ret_imp_name    =NULL;
            $linked_tab = fk_check2( $colName, $tablename, $ret_primary_name, $ret_imp_name );
            if ($linked_tab!="" AND $colName!="EXTRA_OBJ_ID" AND $colName!="CCT_ACCESS_ID") {
                $colcomment .= " <a href=\"view.tmpl.php?t=".$linked_tab."\">=&gt; table</a>";
            }
    		if ($colName=="EXTRA_OBJ_ID") {
    			$colcomment .= " <a href=\"view.tmpl.php?t=EXTRA_CLASS&condclean=1&searchCol=TABLE_NAME&searchtxt=".$tablename."\">=&gt; classes</a>";
    		}
    		
    		$col_rows[] = array( 'c'=>$code_col_name, 'na'=>$colNiceName, 'no'=>$colcomment, 'ty'=>'', 'defc'=>1);
            
         }
         
        
         
         $order_internal = 100;
         $tmp_ord_arr=array();
         $i=0;
         foreach($col_rows as &$row ) {
             
             $code_col_name=$row['c'];
             $checked  = $this->_isChecked($code_col_name, $row['defc'] );
             $ord_pref = $this->_get_pref_order($code_col_name);
             if($ord_pref>=0) {
                 $sort_num=$ord_pref;
             } else {
                 // unknow columns to the end ...
                 $sort_num = $order_internal;
                 $order_internal++;
             }
             $tmp_ord_arr[$i]=$sort_num;
             $row['ch']=$checked;
             
             $i++;
         }
         
         // now resort ...
         
         asort($tmp_ord_arr);
         
         foreach($tmp_ord_arr as $i=>$dummy ) {
             $this->rowshow_colsel( $col_rows[$i] );
         }
    
         if (sizeof ($xcols) ) {
    	   foreach( $xcols as $col_nameq=>$tmpinf) {
    	     
    		 $colname     = "_xcol.".$col_nameq;
    		 $colNiceName = $tmpinf[0];
    		 $colcomment  = $tmpinf[1];
    		 if ($colcomment=="") $colcomment = "this shows a special function";
             $checked = $this->_isChecked( $colname, "0");

             $this->rowshow_colsel( array('c'=>$colname, 'na'=>$colNiceName, 'no'=>$colcomment, 'ty'=>'opt', 'ch'=>$checked) );
    	   }
    	   	
    	 }	
    } 
    
    /**
     * show man INFO row
     * @param string $text
     * @param array $xcopt 
     *     "bgcolor"
    		"submit" => 1
    		'text => string
     */
    public function showInfoRow($text, $xcopt=array()	) {
    	$bgcolor = "#EFEFEF";
    	if ( $xcopt["bgcolor"]!="") $bgcolor = $xcopt["bgcolor"];
    	if ( $xcopt["submit"]!=1 ) {
    		echo "<tr><td colspan=\"4\" bgcolor=\"$bgcolor\">&nbsp;&nbsp;&nbsp;<B>".$text."</B> ".$xcopt['text']."</td></tr>\n";
    	} else {
    		echo "<tr bgcolor=\"$bgcolor\"><td>&nbsp;</td><td colspan=3 align=left><B>".$text."</B></td></tr>\n";
    	}
    }

    
    function savePrefProj(&$sql, $tmpok ) {
        // set .sortcrit in $_SESSION['userGlob']

        $tablename = $this->tablename;
        
        $tmperror="";
        echo "... try get order from folder definition ...<br>";
        $tmpstr = "pp.PROJ_ID=";
        $pos1 = strpos( $_SESSION['s_tabSearchCond'][$tablename]["w"], $tmpstr );
        $tmpNewPos = $pos1+strlen($tmpstr);
        $pos2 = strpos( $_SESSION['s_tabSearchCond'][$tablename]["w"], " ", $tmpNewPos );
        
        do {
        
            if (!$pos1 OR !$pos2) {
                $tmperror = "Could not find folder_id in the condition (search_pos1:$pos1, search_pos1:$pos2)";
                $tmpok = -1;
                break;
            }
            //$primas = NULL;
            //$dummy2 = NULL;
            
            $tmpProjId = substr( $_SESSION['s_tabSearchCond'][$tablename]["w"], $tmpNewPos, $pos2-$tmpNewPos);
            if ( !$tmpProjId) {
                $tmperror = "Could not retrieve a valid proj_id (got '".$tmpProjId."')";
                $tmpok = -1;
                break;
            }
            $tmpCond = "pc.PRIM_KEY=x.".$tablename."_ID and pc.PROJ_ID=".$tmpProjId;
            selectGetEasy( $sql, $tablename, $tmpCond);
            
            $_SESSION['s_tabSearchCond'][$tablename]["f"] = $_SESSION['s_tabSearchCond'][$tablename]["f"] . ", PROJ_HAS_ELEM pc";
            $_SESSION['userGlob']['o.'.$tablename.'.sortcrit'] = "pc.ELEM_ORDER ASC";
            
        } while (0);
        
        if ( $tmperror!="" ) echo "<font color=red>Error:</font> setting order failed: ".$tmperror;
        return ($tmpok);
    }
    
    private function x_col_allowed($sqlo, $col_name) {
        
        if (!in_array($col_name, $this->colNames)) return 0;
        
        $colInfos  = colFeaturesGet($sqlo, $this->tablename, $col_name, 0);
        $showcol   = 1;
        if ( $colInfos['VISIBLE']=="0"  ) {
            $showcol = 0;
        }
        if ( $_SESSION['sec']['appuser'] == 'root' ) $showcol=1;
        return $showcol;
        
    }
    
    /**
     * generate COLUMN config ( $this->_tab_cols_pref) from user input
     * INPUT: $_REQUEST['cols_h']
     * @param array $colNames
     * @param array $viewcol  -- user input
     * @param array $xcols
     */
    public function column_inp2config($sqlo, &$viewcol) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
       
        $xcols = &$this->xcols;
        $has_accid = $this->access_id_has;
        $this->_tab_cols_pref = array(); // reset all
        $this->hidden_cols = $_REQUEST['cols_h'];
        
        $cct_acc_cols=array(
            'a.CREA_DATE' ,
            'a.MOD_DATE' ,
            'a.DB_USER_ID',
            'a.H_ALOG_ACT_ID' 
        ); 
        
        debugOut('(402) viewcol:'.print_r($viewcol,1), $FUNCNAME,1);
        
        $all_keys = array_keys($this->hidden_cols);
        foreach( $all_keys as $col_code ) {
            
            $col_val = $viewcol[$col_code];
            $tmpa    = explode(".",$col_code);
            $prefix   = $tmpa[0];
            $col_pure = $tmpa[1];
            
            // force type to a real integer 
            if ($col_val) $col_val=1;
            else $col_val=0;
            
            switch($prefix) {
                case 'x':
                    $col_allow = $this->x_col_allowed($sqlo, $col_pure);
                    if ($col_allow) {
                        $this->_tab_cols_pref[$col_code] = $col_val;
                    }
                    break;
                case 'a':
                    if ($has_accid) {
                        if (in_array($col_code,$cct_acc_cols) ) { 
                            $this->_tab_cols_pref[$col_code] = $col_val;
                        }
                    }
                    break;
            }
 
        }
        
         
        if (!empty($xcols) ) {
             
            foreach( $xcols as $th0=>$th1) { 
           
                $col_name='_xcol.'.$th0;
                if ( $viewcol[$col_name] != '' ) {
                    $this->_tab_cols_pref[$col_name] = 1;
                } else {
                    unset($this->_tab_cols_pref[$col_name]);
                }
            }
            
        }
    }
    
    function column_saveAsSTD($sqlo) {
        oUSER_PREF_manage::entry_update_on_change($sqlo, $this->COL_PREF_KEY, serialize($this->_tab_cols_pref));
    }
    function table_start() {
        echo '<table id="xtable" cellspacing=0 width=100% style="border: 1px solid #E8E8E8;">';
        echo '<tbody>'."\n";
    }
    
    function table_close() {
        $xcopt = array("submit" => 1 ); 
        $this->showInfoRow( "<input type=\"submit\" value=\" Submit preferences \" class='yButton'>", $xcopt);
        echo '</tbody>'."\n";
        echo "</table>\n";
    }
    
    function show($sql) {
        
        $tablename = $this->tablename;
        
        echo '<form method="post" action="',$_SERVER['PHP_SELF'],'?go=1">'."\n";
        echo '<input type="hidden" name="tablename" value="',$tablename,'">';
        
        $this->table_start();
        //$xcopt = array("bgcolor"=>"#DFDFFF", "submit" => 1 );
        //$mainlib->showInfoRow( "<input type=\"submit\" value=\"   Submit preferences   \">", $xcopt);
        $this->showInfoRow( "Search / Page settings:");
        
        // hide this information for a better design ...
        //echo "<tr><td colspan=3  bgcolor=#EFEFEF><B>Settings for this object table</B> ";
        //echo "</td></tr>\n";
        
        $showsetname='o.'.$tablename.'.showsets';
        $setsPerView = isset($_SESSION['userGlob'][$showsetname]) ? $_SESSION['userGlob'][$showsetname] : NULL;
        
        $this->prefNameShow( "Data rows per page", "valign=top" );
        
        $flagname = $showsetname;
        $checked_0="";
        $checked_1="";
        $checked_2="";
        $showVal=20;
        if ( $setsPerView==""  )   $checked_0="checked";
        if ( $setsPerView=="all" ) $checked_2="checked";
        if ( $setsPerView >0  )  {
            $checked_1="checked";
            $showVal = $setsPerView;
        }
        
        echo "<td NOWRAP><input type=radio name=setti[show_set_all] value=def $checked_0 > default (20) <br>".
            "<input type=radio name=setti[show_set_all] value=0 $checked_1 >
	 <input name=setti[".$flagname."] value=".$showVal." size=4>
	 <br><input type=radio name=setti[show_set_all] value=all $checked_2>ALL </td>";
        $this->prefClose ("Number of maximum shown data rows per page");
        
        
        
        
        $this->prefNameShow( "Show search condition" );
        $flagname='o.'.$tablename.'.condShow';
        // $tmp_val = $_SESSION['userGlob'][$flagname];
        $feld = array('-1'=>'do not show', '0'=>'[default]', '1'=>'show');
        $preselected="0";
        if ($_SESSION['userGlob'][$flagname]!=NULL) $preselected=$_SESSION['userGlob'][$flagname];
        $seltext = formc::selectFget( 'setti['.$flagname.']', $feld, $preselected);
        echo "<td>".$seltext."</td>";
        $this->prefClose ("Shows SQL query code of the selection");
        
        $this->prefNameShow( "Sort conditions" );
        
        echo "<td>[<a href=\"glob.objtab.defsort.php?tablename=".$tablename."\">MultiSort</a>]</td>";
        $this->prefClose ("Advanced: sort by more than one column");
        
        $this->prefNameShow( "Sort condition: clear" );
        $flagname = 'o.'.$tablename.'.sortcrit';
        $sortcrit = $_SESSION['userGlob'][$flagname];
        $checked="";
        echo "<td><input type=checkbox name=setti[".$flagname."] value=1 $checked > clear </td>";
        $this->prefClose ("Clear the sort criteria '$sortcrit' (e.g. for faster queries)");
        
        
        if ( strstr($_SESSION['s_tabSearchCond'][$tablename]["info"], "pp.PROJ_ID") !=NULL ) {
            $this->prefNameShow( "Sort by folder order" );
            $flagname = 'tmp.sortproj';
            $checked="";
            echo "<td><input type=checkbox name=setti[".$flagname."] value=1 > set </td>";
            $this->prefClose ("show order like defined in 'folder has elements'");
        }
        
        $xcopt = array("bgcolor"=>"#FFFFFF" );
        $this->showInfoRow( "&nbsp;", $xcopt);
        
        $xcopt = array("text"=>'[<a href="'. $_SERVER['PHP_SELF'].'?tablename='.$tablename.'&reset_col=1">Reset to standard</a>]' );
        $this->showInfoRow( "Shown columns in list view:", $xcopt);
        $this->sh_DBColumns($sql);
        $xcopt = array("bgcolor"=>"#FFFFFF" );
        $this->showInfoRow( "&nbsp;", $xcopt);
        
        
        $this->showInfoRow("Global settings (for all object tables)");
        
        $this->prefNameShow( 'Reference IDs' );
        $checked = '';
        if  ($this->view_optg['listShowFK']=='1') $checked='checked';
        echo "<td><input type=checkbox name=setti[g.listShowFK] value=1 $checked > show </td>";
        $this->prefClose ("Shows IDs of referenced elements in list view.");
        
        $this->prefNameShow( 'Shown characters per cell' );
        $checked = '';
        $tmpNumber = $this->view_optg["CelSize"];
        if ($tmpNumber<0)  $tmpNumber="";
        echo "<td><input type=text name=setti[g.CelSize] value=\"".$tmpNumber."\" size=5>&nbsp;</td>";
        $this->prefClose ("Number of characters of cells (default: max 30 characters per cell).");
        
        
        $this->prefNameShow( 'Hide menu' );
        $checked = '';
        if  ($_SESSION['userGlob']['g.listHideMenu']==1) $checked='checked';
        echo "<td><input type=checkbox name=setti[g.listHideMenu] value=1 $checked > hide </td>";
        $this->prefClose ("If checked: hide list menu.");
        
        $this->table_close();
        
        
        //echo "</td></tr></table>\n";
        echo "</form>\n";
        echo "<br><br>\n";
        
        
    }
    
    public function JS_out()  {
        js_table_sortable();
    }
    
    /**
     * reset all column settings
     * @param object $sqlo
     */
    function col_prefs_reset() {
        // unset($_SESSION['userGlob'][$this->COL_PREF_KEY]);
        $empty_conf=array();
        $this->set_column_config($empty_conf);
    }
    
    function save_as_default($sql) {
        global $error;
        
        $tablename= $this->tablename;
        
        $viewcol= $_REQUEST['viewcol'];
        
        $setti  = $_REQUEST['setti'];
        
        $view_optg = unserialize($_SESSION['userGlob']["g.viewf.opt"]);
        $tmpok = 0;
        echo '<font color=green>update preferences...</font><br>';
        
        // ------ GLOBAL settings
        
        $view_optg["listShowFK"] = "";
        if ( $setti['g.listShowFK']=='1' ) $view_optg["listShowFK"]='1';
        
        
        $view_optg["CelSize"] = "";
        $view_optg["CelSize"]= $setti['g.CelSize'];
        
        $_SESSION['userGlob']['g.listHideMenu'] = "";
        if ( $setti['g.listHideMenu']=='1' ) $_SESSION['userGlob']['g.listHideMenu']='1';
        
        $_SESSION['userGlob']["g.viewf.opt"] = serialize($view_optg);
        
        // ------ THIS object type
        
        
        $showsetname='o.'.$tablename.'.showsets';
        $_SESSION['userGlob'][$showsetname]='';
        if ( $setti['show_set_all']=='def' ) $_SESSION['userGlob'][$showsetname]='';
        else {
            if ( $setti['show_set_all']=='all' ) $_SESSION['userGlob'][$showsetname]='all';
            else {
                if ( $setti[$showsetname]=='' )    $_SESSION['userGlob'][$showsetname]='20';
                if ( $setti[$showsetname]>0 )      $_SESSION['userGlob'][$showsetname]=$setti[$showsetname];
            }
        }
        
        $flagname= 'o.'.$tablename.'.condShow';
        //$tmp_val = $_SESSION['userGlob'][$flagname];
        $_SESSION['userGlob'][$flagname]='';
        if (!$setti[$flagname]) unset($_SESSION['userGlob'][$flagname]);
        else $_SESSION['userGlob'][$flagname]=$setti[$flagname];
        
        $flagname = 'o.'.$tablename.'.sortcrit';
        // $sortcrit = $_SESSION['userGlob'][$flagname];
        if ( $setti[$flagname]>0 ) $_SESSION['userGlob'][$flagname]='';
        
        $flagname = 'tmp.sortproj';
        if ( $setti[$flagname]>0 ) {
            $tmpok = $this->savePrefProj($sql, $tmpok );
        }
        
        $lener=sizeof($viewcol);
        if ( $lener ) {
            $this->column_inp2config( $sql, $viewcol);
            $this->column_saveAsSTD($sql);
        }
        
        
        //$tmpname = "o.".$tablename.".l.docQuery";
        //if ($colopt["xSQLobj"] OR $_SESSION['userGlob'][$tmpname]) $_SESSION['userGlob'][$tmpname] = $colopt["xSQLobj"];
        echo '<br>';
        
        $newurl = "view.tmpl.php?t=".$tablename;
        echo "[<a href=\"".$newurl."\">next page &gt;&gt;</a>]<br>";
        
        if ($error->Got(READONLY))  {
            $error->printAll();
            return;
        }
        
        if ($tmpok>=0) {
            js__location_replace($newurl, "list view" );  // $stop=array(0=>flag, 1=>reason));
    		return;
    	}
	
	
    }

}
