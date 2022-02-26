<?php
require_once ('f.s_historyL.inc'); 
require_once ("o.PROJ.tree.inc");
require_once 'insert.inc';

/**
 * - manage TEMPORAY object CACHE
 * - support recursive search in a PROJECT
 * - uses table T_PROJ (CACHE-project) as temporary CACHE
 * @package o.T_PROJ.manage.inc (old o.PROJ.search.inc)
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oT_PROJ_manage {
    
    var $cacheProjID;
    var $appopt;		/*
    [useold] - 0|1 : if 1 => last-search-cache content; saves time
    [search_other] : 'PROJ_ORI' : search for sub-prpjects
    */
    
    function __construct( $opt ) {   
        $this->cacheProjID = 0;
        $this->appopt = $opt;
        $this->cacheFoundOldProj = 0;
    }
    
    function init(&$sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        if (!table_exists( $sqlo, 'T_PROJ' ))  {
            $error->set( $FUNCNAME, 1, 'cache table T_PROJ does not exist. Please inform the Admin!' );
            return;
        }    
    }

    /**
     * add projects in tree to the $cacheProj
     * FUTURE: put this to other class ...
     * @param object $sql
     * @param int $base_proj_id
     */
    function buildTree( &$sql, $base_proj_id ) {

        if ( $this->appopt["useold"] ) {
            echo "<font color=gray>...use last-search-cache</font><br>";
            return;
        }
        echo "<font color=gray>...build new cache</font><br>";
        
        $projArr = oPROJ_tree::tree2array( $sql, $base_proj_id );
        $this->add_proj( $sql, $this->cacheProjID, $base_proj_id ); // insert the mother project
        
        foreach( $projArr as $projm=>$tmparr) {
            foreach( $tmparr as $proj2=>$name) {
                $this->add_proj( $sql, $this->cacheProjID, $proj2 );
            }
        }
        
    }
    
    /**
     *
     * clean TEMP-proj
     * @param object $sql
     * @param string $newdir
     * @param string $newname
     */
    function cleanRename( &$sql, $newdir, $newname ) {
        // clean
        $this->cacheCleanUp( $sql, $newdir );
        // rename
        $sql->Updatex( "T_PROJ", "NAME='".$newname."' where T_PROJ_ID = ". $newdir);
    }
    
    /**
     *
     * @param object $sqlo
     * @param string $name
     * @return int
     */
    private function _get_cacheDir_byName(&$sqlo, $name) {
        
        $sqls= "select t_proj_id from t_proj where DB_USER_ID=". $_SESSION['sec']['db_user_id']. ' AND NAME='. $sqlo->addQuotes($name);
        $sqlo->query($sqls);
        $sqlo->ReadRow();
        $newdir = $sqlo->RowData[0];
        
        return $newdir;
    }
    
    
    /**
     *
     * get OR create a new CACHE-project
     // - first: find exact name
     // - second: get one from session
     // - third: get one with name "FREE"
     * @param object $sql
     * @param string $sub_search_id -- identifier of the search: e.g. '67236.proj'
     * @return string
     */
    function get_cache_proj( &$sql, $sub_search_id ) {
        global $error;
        
        $sessionx = session_id(); 
        $this->cacheFoundOldProj = 0;
        $newdir   = 0;
        $cacheProjName = $sessionx . "." .$sub_search_id;
        
        // try to get same cache project from session.PROJECT
        $likeName =  $cacheProjName;
        $newdir = $this->_get_cacheDir_byName($sql, $likeName);
        
        if ( $newdir ) {
            $this->cacheFoundOldProj = 1;
            // clear content ?
            if ( !$this->appopt["useold"]) {
                $this->cacheCleanUp( $sql, $newdir );
            }
        } else {
            if ( $this->appopt["useold"] ) echo "<font color=gray>...can not use an old cache</font><br>";
        }
        
        
        if (!$newdir) {
            // try cache project from session
            $likeName =  $sessionx . ".%";
            $newdir = $this->_get_cacheDir_byName($sql, $likeName);
            
            if ( $newdir ) {
                // rename and clear content
                $this->cleanRename( $sql, $newdir, $cacheProjName );
            }
        }
        
        if ( !$newdir ) {
            // search for a FREE project
            $likeName =  "FREE";
            $newdir = $this->_get_cacheDir_byName($sql, $likeName);
            
            if ($newdir ) {
                // rename and clear content
                $this->cleanRename( $sql, $newdir, $cacheProjName );
            }
        }
        
        if ( !$newdir ) {
            
            $timeStamp = time();
            $argu=array();
            $argu["NAME"]        = $cacheProjName;
            $argu["DB_USER_ID"]  =  $_SESSION['sec']['db_user_id'];
            $argu["DATEX"]       = date_unix2datestr($timeStamp, 1);
            $newdir = insert_row(  $sql, "T_PROJ", $argu);
        }
        
        if ($error->got(READONLY)) return (0);
        
        $this->cacheProjID = $newdir;
        
        if (!$this->cacheFoundOldProj) {
            $this->appopt["useold"] = 0; // not possible
        }
        
        return ($newdir);
    }
    
    function get_cacheFoundOldProj () {
        return($this->cacheFoundOldProj);
    }
    
    /**
     *
     * add PROJ to Cache_proj
     * @param object $sql
     * @param int $cacheProj
     * @param int $tmpProj
     */
    private function add_proj( &$sql, $cacheProj, $tmpProj ) {
        $colstr = "T_PROJ_ID, TABLENAME, OBJID";
        $valstr = $cacheProj. ", 'PROJ', ". $tmpProj;
        $sql->Insertx ( "T_PROJ_ELEM", $colstr, $valstr);
    }
    
    /**
     * add ONE object to cache
     * @param object $sqlo
     * @param string $tablename
     * @param int $objid
     * @throws Exception
     */
    public function add_obj( object &$sqlo, string $tablename, int $objid ) {
        if (!$tablename) throw new Exception('Bad Input: TABLENAME is empty. ');
        if (!$objid) throw new Exception('Bad Input: OBJID is empty. ');

        $argu=array(
            'T_PROJ_ID'=>$this->cacheProjID, 
            'TABLENAME'=>$tablename, 
            'OBJID'=>$objid
        );
        insert_row_s($sqlo, "T_PROJ_ELEM", $argu); 
    }
    
    /**
     *
     * @param object $sql
     * @param int $cacheProj
     */
    function cacheCleanUp( &$sql, $cacheProj ) {
        $sql->Deletex( "T_PROJ_ELEM", "T_PROJ_ID = ".$cacheProj );
    }
    
    /**
     *
     * @param object $sql
     * @param int $cacheProj
     * @return int
     */
    function hasElems ( &$sql, $cacheProj ) {
        $sql->query( 'select count(*) from T_PROJ_ELEM  where T_PROJ_ID = '.$cacheProj );
        $sql->ReadRow();
        $elemCnt = $sql->RowData[0];
        return ($elemCnt);
    }
    
    /**
     *
     * get REAL project name
     * @param object $sql
     * @param int $projID
     * @return string
     */
    function getProjName( &$sql, $projID ) {
        $sql->query( 'select name from PROJ  where PROJ_ID = '. $projID);
        $sql->ReadRow();
        $name = $sql->RowData[0];
        return ($name);
    }
    
    /**
     *
     *  remove/rename old cache projecs
     if old project found: rename in: FREE
     * @param object $sql
     */
    function removeOldCacheProjs( &$sql ) {
        
        $exceed_days = 1;
        $exceed_date_unx = time () - $exceed_days * 24 * 3600 ;
        
        $exceed_date_sql = $sql->Timestamp2Sql ( $exceed_date_unx );
        $sqls = "select x.T_PROJ_ID, x.DATEX from T_PROJ x where ".
            " (x.DATEX<" .$exceed_date_sql.")".
            " AND x.NAME!='FREE'";
        $sql->query($sqls);
        $memtmp=array();
        while ( $sql->ReadRow() ) {
            $tmpproj  = $sql->RowData[0];
            $memtmp[] = $tmpproj;
        }
        
        // empty free projects and rename them to "FREE"
        if (sizeof($memtmp)) {
            $newname = "FREE";
            foreach( $memtmp as $tmpdir) {
                echo "$tmpdir<br>\n";
                $this->cleanRename( $sql, $tmpdir, $newname );
            }
        }
    }
    
    /**
     *
     * touch Cache_project
     * @param object $sql
     * @param int $cacheProj
     */
    public function touch_cache( $sqlo, $cacheProj ) {
        $timeStamp = time();
        $datenow   = date_unix2datestr($timeStamp, 1);
        $sqlo->Updatex( "T_PROJ", "DATEX=".$sqlo->addQuotes($datenow)." where T_PROJ_ID = ". $cacheProj);
    }
    
    /**
     * select a table type
     * @param object $sql
     * @param string $tablename
     * @param $cacheProj
     * @param $f
     * @param $sname
     */
    function select_table($sql, $tablename, $cacheProj, $f, $sname) {
        // echo "DEBUG::: tablename:$tablename<br>";
        $sqls_name = "";
        $sqls_cacheProjs = "select OBJID from T_PROJ_ELEM where T_PROJ_ID=".$cacheProj." AND TABLENAME='PROJ'";
        
        if ( $tablename=='PROJ' and $this->appopt['search_other']=='PROJ_ORI') {
            // get all sub projects ...
            
            $pk_name   = "PROJ_ID";
            $mainname_sql = "x." . importantNameGet2($tablename);
            //$tablenice  =  tablename_nice2($tablename);
            $sqls_cacheObjIds = $sqls_cacheProjs;
        } else {
            $pk_name = $tablename."_ID";
            $mainname_sql = "x." . importantNameGet2($tablename);
            //$tablenice =  tablename_nice2($tablename);
            $sqls_cacheObjIds= "select PRIM_KEY from PROJ_HAS_ELEM where PROJ_ID in (".$sqls_cacheProjs.") AND TABLE_NAME='".$tablename."'";
        }
        
        if ( $sname!="" ) {
            $snamex = trim($sname);
            $snamex = str_replace("'", "''", $snamex);
            $sqls_name = " AND (UPPER(".$mainname_sql.") like UPPER('".$snamex."'))";
        }
        $sqls_finish = "x.$pk_name in (".$sqls_cacheObjIds.")". $sqls_name;
        
        
        $sqls = "select count(*) from ".$tablename. " x where " .$sqls_finish;
        // echo "DEBBB: $sqls<br>";
        $sql->query($sqls);
        $sql->ReadRow();
        $elemTabCnt = $sql->RowData[0];
        echo "<B>$elemTabCnt</B> objects found<br>\n";
        
        $forward_param = NULL;
        
        if ( $elemTabCnt OR ($f=="slim")) {
            
            // write table to history to prevent reloading of left frame in view.tmpl.php
            $histList_obj = new historyList();
            $histList_obj->historycheck( $tablename );
            
            $whereurlenc = rawurlencode($sqls_finish);
            
            $forward_param=$whereurlenc;
        }
        if ( $f!="slim" ) {
            echo "<center><br><B>Show object list ... please wait</B><br></center>";
        }
        
        return $forward_param;
    }
    
    /**
     * get SQL sub selection for a TABLE-TYPE
     * @param object $sqlo
     * @param int $cacheProj
     * @param string $tablename
     * @return string
     */
    function sql_get_subsel(object $sqlo, int $cacheProj, string $tablename) {
        $sqls_cmd = "select OBJID from T_PROJ_ELEM where T_PROJ_ID=".$cacheProj." AND TABLENAME=".$sqlo->addQuotes($tablename);
        return $sqls_cmd;
    }
    
    function get_vars() {
        return array($this->cacheProjID, $this->cacheFoundOldProj);
    }
    
}
