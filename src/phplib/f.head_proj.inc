<?php
require_once ("o.PROJ.subs.inc");
require_once ("sql_query_dyn.inc");
require_once ("f.visu_list.inc"); 

/**
 * Sub functions to manage a script header with alternative calling from PROJ or with TABLE-selection
 * @package f.head_proj.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class gHtmlHeadProjC {

    function __construct() {
    	$this->projHelpObj = new cProjSubs();
    }
    
    /**
     * get count of objects and SQL-command
     * @param object $sql
     * @param string $table
     * @param number $projid
     * @return array
     */
    function getCount( &$sql, $table, $projid=0 ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	global $error;
    	
    	if ( $projid ) {
    
    		$objcnt   = $this->projHelpObj->getTableCnt ( $sql, $projid, $table );
    		$sqlAfter = $this->projHelpObj->getTableSQLInSort ( $projid, $table );
    		if (!$objcnt) {
    		    $stopReason = "No objects of ".tablename_nice2($table)." in project.";
    			$stopFlag=-1;
    		}
    		
    	} else {
    	    $sqlopt=array();
    		$sqlopt["order"] = 1;
    		$sqlAfter  = get_selection_as_sql( $table, $sqlopt);
    		$copt = array("doCount"=>1);
    		$listVisuObj = new visu_listC();
    		list ($stopFlag, $stopReason)= $listVisuObj->checkSelection( $sql, $table, $copt );
    	}
    	
    	if ( $stopFlag<0 ) {
    		 $error->set( $FUNCNAME, 1, $stopReason );
    		 return;
    	}
    	
    	$AnswerArr=array();
    	$AnswerArr["sqlAfter"] = $sqlAfter;
    	$AnswerArr["objcnt"]   = $objcnt;
    	
    	
    	return ($AnswerArr);
    }
    
    /**
     *  - show a page-header
        - analyse $projid
    	- get counts
    	- show errors
     * @param object $sql
     * @param string $table
     * @param number $projid
     * @param array $obj_arr -- optional object-ID array
     * @param string $title
     * @param array $infoarr
     * @return  array(
    			"sqlAfter",  // ordered !
    			"objcnt"
    			)
     */
    function showHead(&$sql, $table, $projid=0, &$obj_arr, $title, $infoarr) {
    
    	
    	$BASEURL = $_SESSION['s_sessVars'][ "loginURL"]."/";
    	//$main_obj_tab = $table;
    	$tableNice    = tablename_nice2($table) ;
    	
    	// $infoarr			 = NULL;
    	$infoarr["scriptID"] = "";
    	$infoarr["title"]    = $title;
    	$infoarr["title_sh"] = $infoarr["title_sh"];
    	$infoarr["jsFile"]   = $infoarr["jsFile"];
    	
    	$obj_sel_type='tab_sql';
    	$sqlAfter    ='';
    
    	if ( $projid ) {
    	    $obj_sel_type='proj_sql';
    		$infoarr["obj_name"] = "PROJ";
    		$infoarr["obj_id"]   = $projid;
    		$infoarr["form_type"]= "obj"; 
    		
    		$objcnt = $this->projHelpObj->getTableCnt ( $sql, $projid, $table );
    		$infoarr["obj_more"] = " <img src=\"".$BASEURL."images/icon.".$table.".gif\"> number of ".$tableNice.": <b>".$objcnt."</b>";
    		$sqlAfter = $this->projHelpObj->getTableSQLInSort ( $projid, $table );
    		
    	} else {
    	    
    		$infoarr["obj_name"] = $table;
    		$infoarr["form_type"]= "list";
    		
    		if (!empty($obj_arr)) {
    		    $obj_sel_type='obj_sel';
    		    $objcnt = sizeof($obj_arr);
    		    $infoarr["obj_more"] = " number of ".$tableNice.": <b>".$objcnt."</b>";
    		} else {
    		    $infoarr["obj_cnt"]  = 1;
    		    $sqlopt=array();
    		    $sqlopt["order"] = 1;
    		    $sqlAfter  = get_selection_as_sql( $table, $sqlopt);
    		}
    		
    	}
    	
    	$pagelib = new gHtmlHead();
    	$headarr = $pagelib->startPage($sql, $infoarr);
    	
    	$AnswerArr = array();
    	
    	switch ($obj_sel_type) {
    	    
    	    case 'tab_sql':
        		$listVisuObj = new visu_listC();
        		$copt = array ("elemNum" => $headarr["obj_cnt"] ); // prevent double SQL counting
        		list ($stopFlag, $stopReason)= $listVisuObj->checkSelection( $sql, $table, $copt );
        		if ($stopFlag<0) $stopReason .=  " Please select elements of '".$tableNice."'!";
        		$objcnt = $headarr["obj_cnt"];
        		break;
        		
    	    case 'proj_sql':
        		if (!$objcnt) {
        			$stopReason = "No objects of ".$tableNice." in project.";
        			$stopFlag=-1;
        		}
        		break;
        		
    	    case 'obj_sel':
    	        if (!$objcnt) {
    	            $stopReason = "No objects of ".$tableNice." in object-selection.";
    	            $stopFlag=-1;
    	        }
    	        break;
    	}
    	
    	if ( $stopFlag<0 ) {
    		echo "<ul>";
    		htmlFoot("Attention", $stopReason);
    	}
    	
    	$AnswerArr["sqlAfter"] = $sqlAfter;
    	$AnswerArr["objcnt"]   = $objcnt;
    	
    	
    	return ($AnswerArr);
    }
    
    /**
     * get SQLafter without SORT !!!
     * @param string $table
     * @param int $projid
     * @return string
     */
    function getSqlAfter($table, $projid) {
    	if ($projid) {
    		$sqlAfterTmp  = $this->projHelpObj->getTableSQL ( $projid, $table );
    		$sqlAfter = $table." x where ".$table."_ID in (".$sqlAfterTmp.")";
    	} else {
    		$sqlAfter  = get_selection_as_sql( $table );
    	}
    	return ($sqlAfter);
    }

}
