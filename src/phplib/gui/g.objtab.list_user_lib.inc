<?php
// 
require_once ("f.objview.inc");
require_once("view.sub.html.inc");
require_once ("glob.obj.update.inc");
require_once 'lev1/f.search_ulib.inc';
require_once 'f.sql_sel2.inc';
require_once 'o.LINK.list_user.inc';

/**
 * main lib to show a LIST VIEW as html
 * @package g.objtab.list_user_lib.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class g_objtab_list_user_lib { 
    
    const MAX_SHOWSET=500;
    const SHOWSET_STD=30;
    const TOOL_NAME='User-List-View';
    const XCOL_PATTERN='_xcol.';
    
    public $tablename;
    
    private $_config;   // o.LINK.list_user.inc:DOC_config_STRUCT
    private $view_opt;  // could be used later ...
    private $classname;
    
    
    var $selectCnt;
    var $infox; /* info array
    
    */
    var $tablename_SQLuse; // if ARCHDB active, the name of an ARCHIVE-table
    
    function __construct(&$sqlo, $doc_id) {
        $this->doc_id = $doc_id;
        $this->_config = oLINK_list_user::doc_load($sqlo, $doc_id);
        
        if (!is_array($this->_config['opt'])) $this->_config['opt']=array();
        
        if ($this->_config['opt']['showsets'] > self::MAX_SHOWSET) {
            $this->_config['opt']['showsets'] = self::MAX_SHOWSET;
        }
        if ($this->_config['opt']['showsets'] <=0) {
            $this->_config['opt']['showsets'] = self::SHOWSET_STD;
        }
    }
    
    // get index or FALSE
    private function col_exists_ind($col_code) {
        $cols = array_column($this->_config['filt'], 'col');
        $index= array_search($col_code, $cols);
        return $index;
    }
    
    function get_congif() {
        return $this->_config;
    }
    function config_set_val($key, $val) {
        $this->_config[$key] = $val;
    }
    
    function config_save($sqlo) {
        $doc_table='LINK';
        $UpdateLib = new globObjUpdate();
        
        $json_str = json_encode($this->_config);
        
        $args= array(
            "vals"=>array(
                'DATAX'=>$json_str
            )
        );
        $UpdateLib->update_meta( $sqlo, $doc_table, $this->doc_id, $args );
    }
    
    function init_table($sqlo) {
        
        $tablename = $this->_config['t'];
        
        $this->tablename = $tablename;
        $this->tablename_SQLuse = $tablename;
        $this->infox   = array();
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
            $this->_debug = $_SESSION["userGlob"]["g.debugLevel"];
        }
        $this->viSubObj  = new viewSubHtml($tablename);
        $this->viSubObj->set_tab_cols_pref($this->_config['cols']);
    }
    
   
    private function _doCnt(&$sqlo) {
        global $error;
        $FUNCNAME= 'doCnt';
        
        $sqls = "select count(*) from ". $this->sqlAfter; // get number of rows
        $retval = $sqlo->query($sqls);
        
        if ( $retval <= 0) {
            $error->set( $FUNCNAME, 1, 'Error on Query' );
            return;
        }
        
        if ( $sqlo->ReadRow() )  $this->selectCnt = $sqlo->RowData[0]; // number of selected sets DB
    }
    
    // remove columns with self::XCOL_PATTERN
    private function user_cols4sql($user_cols) {
        
        $cols4sql=array();
        foreach($user_cols as $col_code) {
            if (str_startswith($col_code, self::XCOL_PATTERN)) {
                continue;
            }
            $cols4sql[]=$col_code;
        }
        return $cols4sql;
    }
    
    private function _build_query($sqlo, fSql_sel2 &$sql_file_lib) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        $tablename = $this->tablename;
        
        debugOut('FILTER_1:'.print_r( $this->_config['filt'],1), $FUNCNAME, 1);
        
        $user_cols = array_keys($this->_config['cols']);
        $user_cols4filter = self::user_cols4sql($user_cols);
        
        $user_filter_pre_lib = new f_search_ulib2($tablename);
        $filter_sql_sel2 = $user_filter_pre_lib->filter($sqlo, $this->_config['filt']);
        
        debugOut('FILTER_2:'.print_r( $filter_sql_sel2,1), $FUNCNAME, 2);
        
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 10, 'Error on User-filter-analysis.' );
            return;
        }

        $sql_file_lib->cleanCond();
        $sql_file_lib->set_filter($sqlo, $filter_sql_sel2);
        $sql_file_lib->set_select_cols($sqlo, $user_cols4filter);
        
        $this->sqlAfter = $sql_file_lib->get_sql_after();
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 10, 'Error on Filter-analysis.' );
            return;
        }
    }
    
    /**
     * prepare QUERY execution
     * @param object $sqlo
     */
    function show_prep($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        $viSubObj  = &$this->viSubObj;
        $tablename = $this->tablename;
        
        
        // $this->view_opt;
        $view_opt  = array();
        $view_opt["ShowFK"]  = $this->_config['opt']["listShowFK"];
        $view_opt["CelSize"] = $this->_config['opt']["CelSize"]; 
        
        $rparams = array(
            "modebase"=>NULL,
            "view_opt"=>&$view_opt
        );
        $viSubObj->prViewModeSet($rparams);
        $viSubObj->sort_link_set_flag(0);
        
        $colNames  = columns_get_pos($tablename);
        
        $this->classname=NULL;
        $class_index = $this->col_exists_ind('c.NAME');
        if ($class_index!==FALSE) {
            $cls_row = $this->_config['filt'][$class_index];
            $this->classname = $cls_row['val'];
        }
        
        // $exp_raw_desc_id = $viSubObj->getDynaColMother($sqlo, $mother_idM);
        $access_id_has   = $viSubObj->access_id_has;
        $class_tab_has   = $viSubObj->class_tab_has;
        list($selectCols, $useJoin) = $viSubObj->colinfoget($sqlo, $tablename, $colNames,
            $access_id_has, $class_tab_has, $this->classname, 0, 1);
        
        if ( $this->classname ) {
            $viSubObj->getClassParams($sqlo, $this->classname);
        }
        
        $selectstr = $viSubObj->sqlColString($selectCols);
        
        $MY_COND=1;
        if ($MY_COND) {
            
            $sql_file_lib = new fSql_sel2($tablename);
            $this->_build_query($sqlo, $sql_file_lib);
            if ($error->Got(READONLY))  {
                return;
            }
            
            $this->infox['sql_info_str']  = $sql_file_lib->get_sql_info();
            $this->infox['sortcritXText'] = $sql_file_lib->set_sort_arr( $this->_config['sort'] );
            $tmpSort  = $sql_file_lib->query_sort_get($this->infox['sortcritXText']);
            debugOut('(303) SORT_txt:'.$this->infox['sortcritXText'], $FUNCNAME, 1);
            debugOut('sqlAfter:'.$this->sqlAfter, $FUNCNAME, 1);
            
            $this->sqlAfterSort = $this->sqlAfter . $tmpSort;
   
        } else {
            
            //OLD: ...
            $sqlDynLib = new fSqlQueryC($tablename);
            $this->sqlAfter = $sqlDynLib->full_query_get( $this->tablename_SQLuse );
            $this->infox['sortcritXText']  = $sqlDynLib->_query_sort_org(); //query_sort_org( $tablename );
            $this->infox['sql_info_str']  = $sqlDynLib->get_sql_info();
            $tmpSort       = $sqlDynLib->_query_sort_get($this->infox['sortcritXText']); // query_sort_get( $tablename,  );
            $this->sqlAfterSort = $this->sqlAfter . $tmpSort;
            $this->sqls_main = "select ".$selectstr." from ". $this->sqlAfterSort;
        }

        $this->sqls_main = "select ".$selectstr." from ". $this->sqlAfterSort;
        $viSubObj->getSortMatrix( $this->infox['sortcritXText'] );
        $this->_doCnt($sqlo);
    }
    
    
    /**
     * build the output
     * @return
     * @param object $sqlo
     * @param object $sqlo2
     */
    function show(&$sqlo, &$sqlo2) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->show_prep($sqlo);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 10, 'on Queriy-Preparation.' );
            return;
        }
        
        $viSubObj = &$this->viSubObj;
        
        $sqlo->query($this->sqls_main);
        $hasResult = $sqlo->ReadRow();
        
        if (!$hasResult) {
            htmlInfoBox( "No data", "This query contains no data. Please check your conditions.", "", "WARN" );
            return 0;
        }
        
        $config_url='p.php?mod=DEF/g.objtab.list_user&id='.$this->doc_id;
        $objLinkLib = new fObjViewC();
        $t_icon = $objLinkLib->_getIcon($this->tablename);
        echo 'Selected: <b><a href="'.
           $config_url.'&action=forward_l" title="forward to the default list view">'.
           '<img src="images/but.list2.gif" height=15> '.
           $this->selectCnt.'</a></b> ';
        echo '&nbsp;&nbsp;&nbsp; list-type: <img src="'.$t_icon.'"> '.tablename_nice2($this->tablename)."\n";
        echo '&nbsp;&nbsp;&nbsp; [<a href="'.$config_url.'">config view</a>]';
        echo '<br>'."\n";
        
        list($colNames_show, $foreign_keys) = $viSubObj->headRowPrep();
        
        $viSubObj->xColsManage( $sqlo2 );
        $viSubObj->prSetPar2( $foreign_keys, $colNames_show);
        $viSubObj->prHeadRow( $sqlo2 );
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 20, 'on Start' );
            return;
        }
        
        $stop_cnt = $this->_config['opt']['showsets'];
        
        $cnt=0;
        while( $hasResult )  {
            
            if ($stop_cnt>0 and $cnt>=$stop_cnt) {
                break;
            }
            $viSubObj->print_row($sqlo2, $sqlo->RowData, $cnt);
            $cnt++;
            if ( !$sqlo->ReadRow() ) break;
            
        }
        
        echo "</table>\n";
        
        if ($cnt>=$stop_cnt) {
            echo 'Show: '.$cnt.' of '.$this->selectCnt." objects. <br>\n";
        }
        
        return 1;
    }
    
    /**
     * release the query acurrent query ...
     * @param object $sqlo
     */
    function release($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        debugOut('(306) release the QUERY.', $FUNCNAME, 2);
        $sql_file_lib = new fSql_sel2($this->tablename);
        $this->_build_query($sqlo, $sql_file_lib);
        if ($error->Got(READONLY))  {
            return;
        }
        $sql_file_lib->queryRelase();
    }

}
