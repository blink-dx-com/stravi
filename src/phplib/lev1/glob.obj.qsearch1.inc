<?php
/**
 * Quick search: sub functions LEVEL1 for  glob.obj.qsearch.php
 * @package glob.obj.qsearch1.inc
 * @swreq UREQ:0001029: g > glob.obj.qsearch.php : Quick search
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/subs/glob.obj.qsearch.inc 59 2018-11-21 09:04:09Z $
 */
require_once ('f.wiid_roid.inc');
require_once ("f.objview.inc");
require_once ("visufuncs.inc");
require_once ('lev1/glob.obj.qsearch2.inc');
require_once 'o.USER_PREF.manage.inc';

/**
 * MAIN NON-Guiclass
 *
 */
class fSearchLib {
    
    /**
     * @var $searchXOpt <pre>
     * 	"bool"
     *  'doidsearch' : 0,1
     *  </pre>
     */
    var $searchXOpt;
    var $xdata; // extra data of wild search
    
    function __construct() {
        $this->searchInfo = array();
    }
    
    /**
     * manage: $idx = MAC:48383
     * @param string $idx
     */
    private function _look_keycode($idx) {
        
        $pos1 = strpos($idx,':');
        if ($pos1===FALSE) {
            return;
        }
        
        $key = trim(substr($idx,0,$pos1));
        if($key==NULL) return;  
        
        $table_tmp = globTablebyShort($key);
        if (!glob_table_exists($table_tmp)) return;
        
        // key code found.
        // calc idx new ...
        $idx = trim(substr($idx,$pos1+1));
        
        $this->idx = $idx;
        $this->tablename=$table_tmp;
        
    }
    
    function init( &$parx, $idx, $tablename, $go ) {
        
       
        $this->parx = &$parx;
        $this->idx = $idx;
        $this->tablename=$tablename;
        
        if ($tablename==NULL) {
            $this->_look_keycode($this->idx);
        }

        
        $this->go = $go;
        $this->taketab = "";
        $this->found_matches = 0;
        $this->idx 	   = trim($this->idx);
        $this->taketab = $this->tablename;
        
        if ($idx!=NULL and $this->parx['algo']==NULL and $this->go>0) {
            $this->_guess_algo();
        }
        
        return array($idx, $this->tablename);
    }
    
    /**
     * guess the algorithm, if $idx contains linebreaks
     * - set $this->parx['algo'] if linebreaks in the search string are detected
     */
    private function _guess_algo() {
        
        $idx = trim($this->idx); // get rid of empty end linebreaks
        
        $testarr = explode("\n",$idx);
        if (sizeof($testarr)<2) return; // no action
        
        // get first element
        $one_element = trim(current($testarr));
        
        $new_algo='';
        do  {
            if (is_numeric($one_element)) {
                $new_algo='mixid';
                break;
            }
            $new_algo='namelist';
        } while (0);
        
        
        $this->infoout('detected a list of search texts, auto detect algorithm: '.$new_algo);
        $this->parx['algo'] = $new_algo;
    }
    
    private function infoout($text) {
        echo '... '.$text."<br>\n";
    }
    
    function takeFromStore() {
        // OUTPUT: $this->tablename, $this->parx, $this->idx
        
        
        $tmpformState = unserialize($_SESSION['userGlob']["f.glob.obj.qsearch"]);
        if ($this->go<=1) {	// take IDX from session, only in mode <= 1 !!!
            if ($this->idx=="")
                $this->idx = $tmpformState["idx"];
        }
        // $this->parx["algo"]        = $tmpformState["algo"]; // do not take from store
        $this->parx["stopforward"] = $tmpformState["stopforward"];
        $this->parx["addnotes"]    = $tmpformState["addnotes"];
        if ( !isset($this->tablename) )
            $this->tablename = $tmpformState["tablename"];
    }
    
    function saveParams($sqlo) {
        
        
        $tmpformState = NULL;
        // if search text is too long, cut it ...
        if ($this->idx!="") $tmpformState["idx"]  = substr($this->idx,0,1500);
        $tmpformState["tablename"]          = $this->tablename;
        $tmpformState["algo"]       = $this->parx["algo"];
        $tmpformState["stopforward"]= $this->parx["stopforward"];
        $tmpformState["addnotes"]   = $this->parx["addnotes"];
        
        oUSER_PREF_manage::entry_update_on_change($sqlo, "f.glob.obj.qsearch", serialize($tmpformState));
       
    }
    
    function doTableAuto() {
        // RETURN: $this->taketab,
        
        
        $this->taketab = $this->tablename;
        if ( $this->parx["tableauto"] ) {   // is quick: take tablename from 'boThis'
            if ($this->tablename=="") {
                if ($this->taketab=="") {
                    $lastbo = $_SESSION['s_sessVars']['boThis'];
                    if ($lastbo!=NULL) {
                        $lastbo_tab = $lastbo["t"];
                        //$lastbo_id  = $lastbo["id"];
                        if ($lastbo_tab!="") $this->taketab = $lastbo_tab;
                    }
                }
            }
        }
    }

    
    private function add_info_str($text) {
        // $searchdone
        $this->sea_info_arr[]=$text;
    }
    
    private function _get_main_notes_col($taketab) {
        $mainnotes= "x.NOTES";
        // test, if NOTES-column exist
        if ( !glob_column_exists($taketab, 'NOTES') ) {
            $mainnotes= NULL;
        }
        return $mainnotes;
    }
    
    /**
     * handle different METHODS
     * @param object $sql
     * @return array $methods_info_STRUCT
     */
    private function do_methods($sql) {
        
        $parx 	 = &$this->parx;
        $idx  	 = $this->idx;
        $taketab = $this->taketab;
        $pkname   = "x." . PrimNameGet2     ($taketab);
        $mainname = "x." . importantNameGet2($taketab);
        $mainnotes = $this->_get_main_notes_col($taketab);
        
        $doBreak = 0;
        
        switch ($parx["algo"]) {
            
            case "idlist":
                $testarr = explode(" ",$idx);
                if (sizeof($testarr)<2) {
                    $testarr = explode("\n",$idx);
                }
                if (sizeof($testarr)<2) {
                    $userError    = 1;
                    $userErrorTxt = "For the Special method '<I>List of IDs</I>' you must give IDs, separated by WHITE_SPACES or NEWLINES";
                    $doBreak      = 1;
                    break;
                }
                
                $tmpor  = "";
                $tmpand = "";
                $where  = "";
                foreach( $testarr as $valx) {
                    $valx = trim ($valx);
                    
                    if ($valx!="") {
                        $where .= $tmpor . $pkname."='".fSearchActC::str2sql($valx)."'";
                        $tmpor = " OR ";
                    }
                }
                list ($found_matches, $found_id) = fSearchActC::searchx( $sql, $taketab, $pkname, $where, $searchXOpt);
                
                $this->add_info_str("list of IDs: $idx");
                
                $doBreak     = 1;
                break;
                
            case "mixid":
                // TBD: not working! (2020)
                do {
                    if ( strstr($idx,"-")!=NULL ) {
                        $testarr = explode(",",$idx);
                        break;
                    }
                    $testarr = explode(",",$idx);
                    if (sizeof($testarr)>1) break;
                    
                    $testarr = explode("\n",$idx);
                    if (sizeof($testarr)>1) break;
                    $testarr = explode(" ",$idx);
                    if (sizeof($testarr)>1) break;
                    
                } while (0);
                
                if ( sizeof($testarr)<2 AND (strstr($idx,"-")==NULL) ) {
                    if ( !is_numeric($idx) ) { // a single number is allowed
                        $userError    = 1;
                        $userErrorTxt = "For the Special method '<I>Mix IDs</I>' you must give IDs or ID-ranges,<br> separated by COMMAS or WHITE_SPACE or NEWLINE";
                        $doBreak      = 1;
                        break;
                    }
                }
                
                $where = "";
                $tmpor = "";
                foreach( $testarr as $valx) {
                    $thisWhere = "";
                    $valx      = trim ($valx);
                    if ($valx=="") continue;
                    
                    $tmpBetweenArr = explode("-",$valx);
                    
                    if (sizeof($tmpBetweenArr)==2) { // search between numbers ???
                        if ( is_numeric($tmpBetweenArr[0]) AND is_numeric($tmpBetweenArr[1]) ) {
                            $thisWhere = "(".$pkname.">=".$tmpBetweenArr[0]." AND ".$pkname."<=".$tmpBetweenArr[1].")";
                            $this->add_info_str("between IDs: $thisWhere");
                            
                        } else {
                            $userError    = 1;
                            $userErrorTxt = "Number error: ".$tmpBetweenArr[0]." or ".$tmpBetweenArr[1]." . You must give IDs, separated by COMMAS";
                            $doBreak      = 1;
                            break;
                        }
                    } else {
                        if (!is_numeric($valx)) {
                            $userError    = 2;
                            $userErrorTxt = "Number error: ".$valx." . You must give IDs, separated by COMMAS or ID-ranges with MINUS";
                            $doBreak      = 1;
                        }
                        $thisWhere   = $pkname."='".fSearchActC::str2sql($valx)."'";
                        $this->add_info_str("ID: $valx");
                        
                    }
                    
                    $where .= $tmpor . $thisWhere;
                    $tmpor = " OR ";
                    
                }
                if ($userError) break;
                
                list ($found_matches, $found_id) = fSearchActC::searchx( $sql, $taketab, $pkname, $where, $searchXOpt);
                $doBreak     = 1;
                break;
                
            case "namelist":
                // first try to explode by NEWLINE, zhan by SPACE
                $testarr = explode("\n",$idx);
                if (sizeof($testarr)<2) {
                    $testarr = explode(" ",$idx);
                }
                
                if (sizeof($testarr)<2) {
                    $userError    = 1;
                    $userErrorTxt = "For the Special method '<I>List of names</I>' you must give names, separated by WHITE_SPACES or NEWLINE";
                    $doBreak      = 1;
                    break;
                }
                $where  = "";
                $tmpand = "";
                foreach( $testarr as $valx) {
                    $valx = trim ($valx);
                    if ($valx!="") {
                        $where .= $tmpor . $mainname." like '".fSearchActC::str2sql($valx)."'";
                        $tmpor = " OR ";
                    }
                }
                list ($found_matches, $found_id) = fSearchActC::searchx( $sql, $taketab, $pkname, $where, $searchXOpt);
                
                
                $this->add_info_str("list of names: ".implode(', ', $testarr));
                
                $doBreak     = 1;
                break;
                
            case "moogle":
                // first try to explode by NEWLINE, zhan by SPACE
                
                $testarr = explode(" ",$idx);
                
                if (sizeof($testarr)<1) {
                    $userError    = 1;
                    $userErrorTxt = "For the Special method '<I>Moogle</I>' you must give WORDS, separated by WHITE_SPACES";
                    $doBreak      = 1;
                    break;
                }
                $tmpand     = "";
                $where      = "";
                $whereNotes = "";
                $whereName  = "";
                foreach( $testarr as $valx) {
                    $valx = trim ($valx);
                    if ($valx!="") {
                        $whereName .= $tmpand . "UPPER(".$mainname.") like UPPER('%".fSearchActC::str2sql($valx)."%')";
                        if ($parx["addnotes"]>0 and $mainnotes!=NULL)
                            $whereNotes .= $tmpand . "UPPER(".$mainnotes.") like UPPER('%".fSearchActC::str2sql($valx)."%')";
                            $tmpand = " AND ";
                    }
                }
                
                $where = "( ".$whereName." )";
                if ($whereNotes!="") $where = "( ".$where." OR ( ". $whereNotes." ) )"; // outer brackets due to the OR
                list ($found_matches, $found_id) = fSearchActC::searchx( $sql, $taketab, $pkname, $where, $searchXOpt);
                
                $tmp_info = '';
                if ($parx["addnotes"]>0 ) $tmp_info .= " also NOTES, ";
                $tmp_info .= "Moogle: $idx";
                $this->add_info_str( $tmp_info );
                $doBreak     = 1;
                
                break;
                
        }
        
        $outarr=array(
            'doBreak'     =>$doBreak,
            'userError'   =>$userError,
            'userErrorTxt'=>$userErrorTxt,
            'found_matches'=> $found_matches,
            'found_id'     => $found_id,
            'where'       =>  $where
            
        );
        
        return $outarr ;
    }
    
    
    
    /**
     * DEFAULT search block: search for ONE column, one value
     * @global $this->found_matches
     * @param object $sql
     * @return  array $methods_info_STRUCT
        'userError' int
        'userErrorTxt' string
        'found_matches' int 
        'found_id' int
        'where' string 
        'xdata' array extra data fro wild searches
     */
    function mainSearch_default( &$sql ) {
        
        global $error;
        
        $this->sea_info_arr = array();
        $parx 	 = &$this->parx;
        $idx  	 = $this->idx;
        $taketab = $this->taketab;
        $found_matches = 0;
        $where         = "";
        $userError    = 0;
        $userErrorTxt = '';
        $xdata = NULL; // extra data from special searches ...
        
        // NOW start SEARCHING
        
        $histList_obj = new historyList();
        $tmpret = $histList_obj->historycheck( $taketab );
        if ( $tmpret > 0 ) htmlShowHistory();
        
        
        if ( $parx["bool"] != "") {
            $this->searchXOpt["bool"] = $parx["bool"];
            $this->searchInfo["bool"] = "added to last condition";
        }
        
        $doBreak = 0;

        do {
            
            if ($doBreak) {
                break; // break from do_methods()
            }
            
            $pkname    = "x." . PrimNameGet2     ($taketab);
            $mainname  = "x." . importantNameGet2($taketab);
            $mainnotes = $this->_get_main_notes_col($taketab);
            
            $this->searchXOpt['doidsearch']= 1;
            if ( is_numeric($idx) ) {
                // could be a FLOAT number or "34E3" ==> is not an ID
                if ( strval(intval($idx)) !== $idx ) $this->searchXOpt['doidsearch'] = 0;
            } else {
                $this->searchXOpt['doidsearch'] = 0;
            }
            
            if ( substr($idx,0,1)=="0" ) $this->searchXOpt['doidsearch'] = 0;  // if FIRST char is a ZERO, then it should be a string
            // if ( $parx["addnotes"]>0 )   $this->searchXOpt['doidsearch'] = 0;  // search also in notes
            if ( substr($idx,0,4) == "rid:") {
                $this->searchXOpt['doidsearch'] = 0;
                $this->searchXOpt["dirty"] = "rid";
            }
            $searchXOpt = $this->searchXOpt;
  
            
            if ($this->searchXOpt["dirty"] == "rid" ) {
                
                $roidSearchObj = new fWiidRoidC();
                
                $ridarr = explode(":",$idx);
                $wiid=$ridarr[1];
                $roid=$ridarr[2];
                $this->add_info_str("search 'remode ID' : WIID:$wiid ROID:$roid");
                $found_id   = $roidSearchObj->getObjID ($sql, $taketab, $wiid, $roid);
                if ($error->Got(READONLY))  {
                    $errLast   = $error->getLast();
                    $error_txt = $errLast->text;
                    $error->reset();
                    $userErrorTxt = $error_txt;
                    $found_matches = 0;
                } else {
                    $found_matches = 1;
                }
                break;  // ready
            }
            
            if ( $this->searchXOpt['doidsearch'] ) {
                $where = $pkname."=". $sql->addQuotes($idx);
                list ($found_matches, $found_id) = fSearchActC::searchx( $sql, $taketab, $pkname, $where, $searchXOpt );
                $this->add_info_str("ID=".$idx);
                break;  // no searching for names
            }
            
            // OBJECT dependend search ...
            $specialObject_lib = NULL;
            switch ($taketab) {
                
                case 'CYTCART':
                    $specialObject_lib = new _oCYTCART_qsearch($this->searchXOpt);
                    list ($found_matches, $found_id, $where, $infoText ) = $specialObject_lib->specialSearch($sql, $idx);
                    $this->add_info_str($infoText);
                    $doBreak     = 1;
                    break;
                    
                case 'CONCRETE_SUBST':
                    $specialObject_lib = new _oCONCRETE_SUBST_qsearch($this->searchXOpt);
                    list ($found_matches, $found_id, $where, $infoText ) = $specialObject_lib->specialSearch($sql, $idx);
                    $this->add_info_str($infoText);
                    $doBreak     = 1;
                    break;
                    
                case 'CONTACT':
                    $specialObject_lib = new _oCONTACT_qsearch($this->searchXOpt);
                    list ($found_matches, $found_id, $where, $infoText ) = $specialObject_lib->specialSearch($sql, $idx);
                    $this->add_info_str($infoText);
                    $doBreak     = 1;
                    break;
                
                    
                case 'DB_USER':
                    $specialObject_lib = new _oDB_USER_qsearch($this->searchXOpt);
                    list ($found_matches, $found_id, $where, $infoText ) = $specialObject_lib->specialSearch($sql, $idx);
                    $this->add_info_str($infoText);
                    $doBreak     = 1;
                    break;
                    
                case 'ABSTRACT_SUBST':
                    
                    $specialObject_lib = new _oABSTRACT_SUBST_qsearch($this->searchXOpt);
                    
                    if ($parx['wild']) {
                        // this search will do not do ANY other search ....
                        list ($found_matches, $found_id, $where, $infoText, $xdata ) = $specialObject_lib->specialSearch($sql, $idx);
                        $this->add_info_str($infoText);
                        $doBreak     = 1;
                        break;
                    }
                    
                    
                    $sea_result = $specialObject_lib->standard_search($sql, $idx, $pkname);
                    // list ($found_matches, $found_id, $where, $infoText )
                    $this->add_info_str($sea_result['info']);
                    $found_id = $sea_result['id'];
                    $where    = $sea_result['where'];
                    $found_matches = $sea_result['matches'];
                    if ($found_matches) {
                        $doBreak     = 1;
                    }
                    if ($sea_result['break']) {
                        $doBreak     = 1;
                    }
                    
                    
                    
                    
                    break;
                    
                    
            }
            if ( $doBreak ) break;
            
            
            if (  $parx["addnotes"]>0 and $mainnotes!=NULL) {
                $where = "(UPPER(".$mainname.") like UPPER('%".fSearchActC::str2sql($idx)."%') OR UPPER(".$mainnotes.") like UPPER('%".fSearchActC::str2sql($idx)."%') )";
                list ($found_matches, $found_id) = fSearchActC::searchx( $sql, $taketab, $pkname, $where, $searchXOpt);
                
                $this->add_info_str("parts of NAME or NOTES: $idx");
                
                break;  // ready
            }
            
            //             if (!$found_matches ) {
            //                 $where = "UPPER(".$mainname.") like UPPER('".fSearchActC::str2sql($idx)."')";
            //                 list ($found_matches, $found_id) = fSearchActC::searchx( $sql, $taketab, $pkname, $where, $searchXOpt);
            //                 $this->add_info_str("exact NAME: '".$idx."'");
                            
            //                 if ($found_matches) break;
            //             }
            
            if (!$found_matches) {
                $where = "UPPER(".$mainname.") like UPPER('%".fSearchActC::str2sql($idx)."%')";
                list ($found_matches, $found_id) = fSearchActC::searchx( $sql, $taketab, $pkname, $where, $searchXOpt);
                $this->add_info_str("parts of NAME: like '%".$idx."%'");
                
                if ($found_matches) break;
            }
 
        } while (0);
        
        $methods_info=array();
        $methods_info['userError']   =$userError;
        $methods_info['userErrorTxt']=$userErrorTxt;
        $methods_info['found_matches']=$found_matches;
        $methods_info['found_id']=$found_id;
        $methods_info['where']=$where;
        $this->xdata = $xdata;
        
        return $methods_info;
        
    }
    
    /**
    * main search block
    * @global $this->found_matches
    * @param object $sql
    * @return array
    *   'forward_url'
    *   'matches' => int
    */
    function mainSearch( &$sql ) {

        if ($this->parx["algo"]!='') {
            $methods_info = $this->do_methods($sql);
        } else {
            $methods_info = $this->mainSearch_default($sql);
        }
        $methods_info['idx'] = $this->idx;
        
        return $methods_info;

    }
    
    function get_sea_info_arr() {
        return $this->sea_info_arr;
    }
    
    /**
     * show all matched objects in a simple table ...
     * @param object $sqlo
     * @param string $tablename
     * @param array $xdata
     *   [OBJ-ID] = array('n'=>object name, 's'=>SCORE );
     */
    function show_obj_ranks($sqlo, $tablename) {
        
        if (empty($this->xdata)) {
            echo 'No wild search data available.<br>';
        }
       
        $tabobj = new visufuncs();
        $headOpt = array( "title" => "Widl search: ranked results, best fit" );
        $headx  = array ("#", tablename_nice2($tablename), 'Match-Score' );
        $tabobj->table_head($headx,   $headOpt);
        $objLinkLib = new fObjViewC();
        $cnt=0;
        foreach($this->xdata as $objid => $inforow) {

            $html_tmp = $objLinkLib->bo_display( $sqlo, $tablename, $objid );
            $dataArr=array( ($cnt+1), $html_tmp, $inforow['s'] );
            $tabobj->table_row ($dataArr);
            $cnt++;
        }
        $tabobj->table_close();
    }
     
}
