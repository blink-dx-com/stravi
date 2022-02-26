<?php
require_once ('table_access.inc');       
require_once ("sql_query_dyn.inc");
require_once ("o.DB_USER.subs2.inc");     
require_once ("insert.inc");  
require_once ("edit.sub.inc");

/**
 * contains sub functions for LINK : class "query"
 * DB-structure:
 * CLASS-name: "query"
 * CLASS-PARAMS
 *  
    "table"       $searchtable (S01)
    "sqltext" :   $sqltext  
    "dependencies" $searchDepend
    "options"    $searchOptions
 * @package o.LINK.c_query_subs.inc
 * @swreq UREQ:xxxxxxxxxxxx
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/o.LINK.c_query_subs.inc 59 2018-11-21 09:04:09Z $
 * @example <pre>
 * 
 * $doc_id = 0; // query document ID
 * $extra_obj_id =  glob_elementDataGet( $sqlo, 'LINK', 'LINK_ID', $doc_id, 'EXTRA_OBJ_ID');
   $values_all = $varcol->select_by_name ($extra_obj_id);
   $values = &$values_all["values"];
 * 
 * $queryObj = new myquery();
 * $options["action"] = "exec";  
 * $myqu_varval = array(); // user parameters
   $queryObj->ShowFrom( $sqlo, $extra_obj_id, $myqu_varval, $formNextUrl, $options ); 
   if ($error->got(READONLY)) {
       ....
   }
 * </pre>
 */
class myquery {
    var $class_id;
    var $fullQuery; // the full SQL-select string (can still contain '#USERVAR')

function __construct () {
    global $error, $varcol; 
    
    $varcol= & Varcols::get();
    $error = & ErrorHandler::get();
    
    $this->class_id = 0;
    
    $this->class_id = $varcol->class_name_to_id( "LINK", "query");
    if ($error->got(READONLY)) {
        $error->set('getMyQueryNames', 1, 'no class "query"!');
        return;
    }
}
 
/**
 * get/make /HOME/.profile/queries directory 
 * @param unknown $sql
 * @param string $action  ["CREATE"], 
                       at "READONLY": if no project exists returns just 0, no error
 * @return number|Ambigous <number, long>
 */
function getProfQueryProj(&$sql, $action="CREATE"  ) {


    global $error;
    
    $querydir = 0;
    $profilename = ".profile";
    
    $home_proj_id =  oDB_USER_sub2::userHomeProjGet( $sql ); // TBD: put the function in an other PHP-script.inc
    if ( !$home_proj_id ) { 
        $error->set('getProfQueryProj', 1, 'no home project!');
        return (-1);
    }
    
    // get ".profile"
    $sqls= "select proj_id from proj where pro_proj_id=".$home_proj_id. " AND NAME='".$profilename."'";
    $sql->query($sqls);
    if ( $sql->ReadRow() ) {
        $profiledir = $sql->RowData[0];
    } else {
        if ( $action == "READONLY" ) return 0; 
        $o_rights = access_check( $sql, "PROJ", $home_proj_id); 
        if ($o_rights["insert"] != 1) {
            $error->set('getProfQueryProj', 1, 'no "insert" right on home project [ID:'.$home_proj_id.']!');
            return (-1);
        } 
        $argu["NAME"] = $profilename;
        $argu["PRO_PROJ_ID"] = $home_proj_id;
        $profiledir = insert_row_role(  $sql, "PROJ", $argu); // with role check !!!
    }   
    if ($error->got(READONLY)) return (0);
     
    $sqls= "select proj_id from proj where pro_proj_id=".$profiledir. " AND NAME='queries'";
    $sql->query($sqls);
    if ( $sql->ReadRow() ) {
        $querydir = $sql->RowData[0];
    } else {
        if ( $action == "READONLY" ) return 0; 
        $o_rights = access_check( $sql, "PROJ", $profiledir); 
        if ($o_rights["insert"] != 1) {
            $error->set('getProfQueryProj', 1, 'no "insert" right on ".profile" project [ID:'.$profiledir.']!');
            return (-1);
        } 
        $argu2["NAME"] = "queries";
        $argu2["PRO_PROJ_ID"] = $profiledir;
        $querydir = insert_row_role(  $sql, "PROJ", $argu2); // with role check !!!
        
    }
    return ($querydir);
}
  
function check_query( &$sqltext ) { 
// FUNCTION: check query for bad SQL-expressions  
        global $error;
        
        $errflag = 0;
        if (stristr($sqltext, "pass_word")!="") {
            $errflag = -1;
            $error->set('check_query', 1, '"pass_word" is not allowed in the query.');
        }    

        $tmpsql_upper = strtoupper($sqltext);       
        $pos=0;
        //while (  ( $posf = strpos($tmpsql_upper, "DB_USER", $pos) ) == TRUE   ) {
        //    if (substr($sqltext, $posf+7,1) != "_") { 
        //        $errflag = -1;
        //        $error->set('check_query', 1, "'db_user' as standalone word is not allowed in the query (character-pos $posf).");
        //    }
        //    $pos = $posf+6;
        //} 
        return ( $errflag );
}

/**
 * search for #USER_VAR[varname]
               - parsing fehler bricht ab!  
     RETURN: $vararr   [] = array( varname, sqltext-pos, endpos
                                 sqltext-pos:: points to the '#' in #USER_VAR[
                                 endpos:: pos of last bracket "]" 
 * @param unknown $sqltext
 * @return void|multitype:multitype:unknown
 */
function get_uservars( &$sqltext ) {
    
        global $error;
        

        $vararr = array();
        $keyword ="#USER_VAR[";
        $keyword_len = strlen($keyword); 

        $postmp = 0;

        while ( ($postmp = strpos( $sqltext, $keyword, $postmp )) !== FALSE ) {
            $posAfterKey = $postmp+$keyword_len;
            $endpos = strpos( $sqltext,"]", $posAfterKey );
            if ($endpos === FALSE) {    
                $error->set('get_uservars', 1, 'Keyword at sqltext-pos'.$postmp.' ends without "]"!');
                return;
            }
            $varname = substr( $sqltext, $posAfterKey, $endpos-$posAfterKey);
            if ($varname == "") {
                $error->set('get_uservars', 1, 'User variable name at sqltext-pos'.$postmp.' is empty "]"!');
                return;
            } 
            $vararr[] = array($varname, $postmp, $endpos);   
            if ( $_SESSION['userGlob']["g.debugLevel"]>1 ) echo "DEBUG: varname:'$varname', start-pos:$postmp, endpos:$endpos<br>\n"; 
            $postmp = $endpos+1;  // next variable
        }

        return ($vararr);

}    

/**
 * compile query
    NAME like '#USER_VAR[experiment ID]'  AND EXP_ID> 345
    // RETURN: $sqltext_compiled
 * @param unknown $sqltext
 * @param unknown $uservars
 * @param unknown $myquvarval
 * @return string
 */
function compile_sqltext( &$sqltext, &$uservars, &$myquvarval ) {
    // 
    //    
    $sqltext_compiled = "";
    $current_query_pos = "0";

    foreach( $uservars as $dummy=>$tmparr) {
        $varname     = $tmparr[0]; 
        $posKeyStart = $tmparr[1];
        $posKeyEnd   = $tmparr[2];
        $sqltext_compiled =  $sqltext_compiled . substr( $sqltext, $current_query_pos, $posKeyStart-$current_query_pos);
        $sqltext_compiled =  $sqltext_compiled . $myquvarval[$varname];
        $current_query_pos = $posKeyEnd+1;  
    } 

    $sqltext_compiled =  $sqltext_compiled . substr($sqltext, $current_query_pos); // the rest      

    reset($uservars);

    return ($sqltext_compiled);
    
}

function queryObjGetVars( &$sql, $objid ) {
	global $error, $varcol;

    $error  = & ErrorHandler::get();
    $varcol = & Varcols::get();


	$sql->query("select EXTRA_OBJ_ID from LINK where LINK_ID=".$objid);
	$sql->ReadRow();
    $extra_obj_id = $sql->RowData[0];

    if ( !$extra_obj_id ) {
		$error->set('queryObjGetVars()', 1, "no extra_obj_id, need class 'query'");
		return;
	}
    
	$extra_class = $varcol->obj_id_to_class_name ( $extra_obj_id );
    if ( $extra_class!="query" ) {
		 $error->set('queryObjGetVars()', 2, "object not of class 'query'");
		 return;
	}
    $values_tmp = $varcol->select_by_name ($extra_obj_id);

    $values = &$values_tmp["values"];
    if ( $values["sqltext"]=="" ) {
        $error->set('queryObjGetVars()', 3, "no content in query-object-column 'sqltext'");
        return;
    }
    if ( $values["table"]  =="" ) {
      	$error->set('queryObjGetVars()', 4, "no content in query-object-column 'table'");
      	return;
    }
    return ($values);
}

/**
 * 	- replace the USERVAR
			 - execute the SQL
	RETURN:   - array of values for ONE SQL-readRow()
	GLOBAL:   - need $this->fullQuery
 * @param unknown $sql
 * @param unknown $varname
 * @param unknown $value
 */
function exeSqlReplace ( &$sql, $varname, $value ) {

	$key = "#USER_VAR[".$varname."]";
	$newsql = str_replace( $key, $value, $this->fullQuery );
	$sql->query($newsql);
	$sql->ReadRow();
	return ($sql->RowData);
}


/**
 * calculate a full $_SESSION['s_tabSearchCond'] entry in $retarr
  RETURN:  structure for $_SESSION['s_tabSearchCond']
 * @param unknown $qObjVals
 * @param unknown $sqltext_compiled
 * @return multitype:unknown 
 */
function createSqlArr( &$qObjVals, &$sqltext_compiled ) {
	

	$retarr	   = NULL;
    $useJoin   = "";
    $condclean = 1; 
    $newCond   = $sqltext_compiled;
    $savedCond = "";
    $searchClass  = "";
	$searchDepend = $qObjVals["dependencies"];
	$searchOptions= $qObjVals["options"];
	$searchtable  = $qObjVals["table"];

    if ( strlen($searchDepend) ) {
        $depend_arr = explode(",", $searchDepend);
        foreach( $depend_arr as $dummy=>$tmpname) {
            // if ( $_SESSION['userGlob']["g.debugLevel"]>2 ) echo  "DEBUG: dependencies value: '$tmpname'<br>\n";
            if ($tmpname == "access") $useJoin = $useJoin . "CCT_ACCESS"; 
            if ($tmpname == "class")  $useJoin = $useJoin . "EXTRA_CLASS";  
        } 
    }    
    
    
    if ( strlen($searchOptions) ) {
        $tmpopt_arr = explode(",", $searchOptions);
        foreach( $tmpopt_arr as $dummy=>$tmpname) {
			$tmpKeyVal = explode(":", $tmpname);
			$optout[$tmpKeyVal[0]] = $tmpKeyVal[1];
			if ( $tmpKeyVal[0] == "useclass" ) {
                $searchClass = $tmpKeyVal[1];
            }
        }
		$info["option"] = $optout;
    }

    //if ( $_SESSION['userGlob']["g.debugLevel"]>2 ) {
    //    echo "DEBUG: Results from parsing data column 'dependencies': use_class:'".$use_class."' <br>\n";
    //}
    $sopt = NULL;
    $sopt["useJoin"] = $useJoin;
    $sopt["sClass"]  = $searchClass;

    list($sqlfromXtra, $tableSCondM, $sqlWhereXtra, $sel_info, $classname, $mother_idM ) =
	     selectGet( $sql, $searchtable, $condclean, $newCond, $savedCond, $sopt );
	$retarr = array ( "f"=>$sqlfromXtra, "w"=>$tableSCondM, "x"=>$sqlWhereXtra, "c"=>$classname, "info"=>$sel_info, "mothid"=>$mother_idM ); /* save selection array */

	return array($retarr, $info);
}

function _tabRow($key, $text, $opt=NULL) {

	if ($opt=="head") {
		echo "<tr bgcolor=#8080FF><td colspan=2><font color=FFFFFF>&nbsp;<b>".$key."</b></font></td></tr>\n";
		return;
	}
	if ($opt=="info") {
		echo "<tr><td colspan=2><font color=gray>".$key."</font></td></tr>\n";
		return;
	}
	
	$keytxt = "<b>".$key."</b>";
	echo "<tr><td>".$keytxt."</td><td>".$text."</td></tr>\n";
}
 
/**
 * a) show form for SQL-query operation
 * b) execute the query  ($options["action"] = "exec")
 * @param object &$sql,
 * @param $extra_obj_id, 
 * @param $query_uservar_vals, // array of user variable values
 * @param $formNextUrl,  // next URL after the input-form e.g. "edit.tmpl.php?t=LINK&id=$id"
                         // contains the var_array $myquvarval[] and $myqu_go
                         // $myqu_go: 0 => prepare
                         //           1 => try to execute (if params => show form)    
                         //           2 => really execute, don't care about $myquvarval[]    
 * @param $options  
 *  ["action"] = "prepare", "exec"
    'formVarBase' : give a base for the FORM variables 
    				(needed for  obj.link.xmode.specials.inc)
    'exec_after_url' : OPTIONAL URL after sql execution; DEFAULT: view.tmpl.php
 */
function ShowFrom( &$sql, $extra_obj_id, $query_uservar_vals, $formNextUrl, $options ) {
    global $error, $varcol;

    $error  = & ErrorHandler::get();
    $varcol = & Varcols::get();

    $contains_user_vars_bln = FALSE;
    $action = $options["action"];
    if ($action=="") $action="prepare";
    
    if ( $options['formVarBase'] != NULL ) { // alternativ form variable-name-base
    	$formVar=array();
    	$formVar['go']   = $options['formVarBase'].'[myqu_go]';
    	$formVar['vars'] = $options['formVarBase'].'[myquvarval]';
    } else {
    	$formVar = array();
    	$formVar['go']   = 'myqu_go';
    	$formVar['vars'] = 'myquvarval';
    }

    if ( $_SESSION['userGlob']["g.debugLevel"]>0 ) {
        echo "<br>DEBUG: action:$action<br>\n";
    }
    
    if ( !$extra_obj_id ) htmlFoot("Error", "not of  class 'query'");
    if ( $extra_obj_id ) $extra_class = $varcol->obj_id_to_class_name ( $extra_obj_id );    
    if ( $extra_class!="query" )  htmlFoot("Error", "not of  class 'query'"); 

    $values_tmp = $varcol->select_by_name ($extra_obj_id);

    $values = &$values_tmp["values"];
    if ( $values["sqltext"]=="" ) {
        $error->set('ShowFrom', 1, "no content in query-object-column 'sqltext'");
        return;
    }
    if ( $values["table"]  =="" ) {
      $error->set('ShowFrom', 1, "no content in query-object-column 'table'");
      return; 
    }
    $sqltext      = &$values["sqltext"];
    $searchtable  = $values["table"];
    $searchDepend = $values["dependencies"];
    $searchOptions= $values["options"];

    $uservars = $this->get_uservars( $sqltext );
    if ($error->got(READONLY)) return;   // TBD: better error message
    
    if ( sizeof($uservars) ) $contains_user_vars_bln = TRUE;
    
    // echo "DEB: $action<br>";
    
    if ( $action == "prepare" ) {
        // show form, if user_vars are defined and no input is available
         
        //$errflag = check_query( $sqltext ); 
        //if ($error->got(READONLY)) { 
        //    $error->set('ShowFrom', 1, "Stopped due to previous error in check_query."); 
        //    return; 
        //}

        echo "<form method=\"post\"  name=\"queryinform\"  action=\"".$formNextUrl."\" >\n";
		$submitText = "<input type=submit value=\"Execute query!\">\n";
		echo "<table bgcolor=#EFEFEF border=0 cellspacing=1 cellpadding=1>\n";
		
		$this->_tabRow( "Query parameters","", "head");
		$iconx = htmlObjIcon($values["table"], 1);
		$this->_tabRow( "Search in table <img src=\"".$iconx."\"> <b>".tablename_nice2($values["table"])."</b>" , "", "info");
		
        if (sizeof($uservars)) { 
            
            foreach( $uservars as $dummy=>$tmparr) {
                $varname = $tmparr[0];
                
				$this->_tabRow( htmlspecialchars ($varname), 
					'<input type=text name="'.$formVar['vars'].'['.$varname.']" value="">' );
            }
            reset($uservars);
			
        } else {
            // echo "<font color=gray>No user variables in query</font><br>\n";
            $dummy=0;
        }
		echo "<tr bgcolor=#E0E0FF><td></td><td>".$submitText."</td></tr>\n";
        echo "</table>\n";
			
		echo "<br>\n";  
		echo '<input type=hidden name="'.$formVar['go'].'" value="2">';
        echo "</form>\n";

        $sqltext_compiled = $sqltext; // this is an error 
        return;

    } else {
        // echo "<br>Raw SQL-text:\n<pre>".htmlspecialchars($sqltext)."</pre>\n";

        if (sizeof($uservars)) { // do user var definitions exist ???
            $sqltext_compiled = $this->compile_sqltext( $sqltext, $uservars, $query_uservar_vals );
        } else {
            $sqltext_compiled = $sqltext;
        }
    }     

    // checks 
    $errflag = $this->check_query( $sqltext_compiled );
    if ($error->got(READONLY)) { 
        $error->set('ShowFrom', 1, "Stopped due to previous error in check_query."); 
        return; 
    }  

    // $condUrlParam =  urlencode( $sqltext_compiled );                                                 
    if ( $_SESSION['userGlob']["g.debugLevel"]>0 ) {
        echo "<br>DEBUG: SQL-user-text:\n<pre>".htmlspecialchars($sqltext_compiled)."</pre>\n";
    }    

    // create internal SQL-FULL-query 

	list($sqlRetArr,) = $this->createSqlArr( $values, $sqltext_compiled );

    $_SESSION['s_tabSearchCond'][$searchtable] = $sqlRetArr;


    if ( $_SESSION['userGlob']["g.debugLevel"]>1 ) {
        $sqlAfter = get_selection_as_sql( $searchtable );
        echo "<br>DEBUG: FULL internal SQL-query:\n<pre>".htmlspecialchars($sqlAfter)."</pre>\n";
    }

    $newurl = "view.tmpl.php?t=".$values["table"];
    if ($options['exec_after_url']!=NULL) {
    	$newurl = $options['exec_after_url'];
    }
    if ( $action == "exec" ) { 

        echo "<a href=\"$newurl\">Execute query</a><br>\n";
        echo "... automatic forward ...<br>\n";

        if ( $_SESSION['userGlob']["g.debugLevel"]>0 ) {
            echo "... stopped due to DEBUG-level.<br>\n";
        } else {
            // automatic forward
            ?> 
            <script language="JavaScript">
            <!--  
                location.href="<?php echo $newurl?>";
            //-->
            </script>
            <?
        }
    }

}  // end show form          

function getMyQueryDocs( &$sql, $proj_id, $searchtable, 
    $options = NULL   // e.g. "maxdocs" - return not more than $maxdocs elements
    ) {   
//  RETURN: array[] = array( $name, $reduced_notes )
    
    global $varcol, $error;   
    
    if ($searchtable!="") {
        $sql_extra1 = " AND S01='".$searchtable."'"; 
    }      
    
    $sqls ="select x.link_id, x.name, x.notes, o.S01" .
           " from LINK x, (select extra_obj_id, S01 from extra_obj where extra_class_id=".$this->class_id. $sql_extra1.") o".
           " where (x.LINK_ID in ". 
             " (select prim_key FROM proj_has_elem WHERE proj_id = ".$proj_id." AND table_name='LINK')) ".
             " AND (x.extra_obj_id=o.EXTRA_OBJ_ID) ORDER by x.NAME";  
        // echo "DEBU:<br>$sqls";             
    $sql->query($sqls); 
    $cnt=0;
    while ( $sql->ReadRow() && $cnt<50 ) {
        $link_id =  $sql->RowData[0]; 
        $name    =  $sql->RowData[1];  
        $notes   =  $sql->RowData[2];    
        $tabdest =  $sql->RowData[3];
        
        $notes_len = strlen($notes);
        if ($notes_len) {
            $notesMin = substr($notes, 0, 50); 
            if ($notes_len>50) $notesMin=$notesMin. "...";
        } 
        $retarr[] = array($link_id, $name, $notesMin, $tabdest);
    } 
    return ($retarr);
}   

function searchQueryByName( &$sql, $proj_id, $name, $searchtable) { 
// search for document
    
    $retarr = NULL;
    if ( $searchtable!="" ) {
        $sql_extra1 = " AND S01='".$searchtable."'"; 
    }
 
    $sqls ="select x.link_id, o.S01" .
           " from LINK x, (select extra_obj_id, S01 from extra_obj where extra_class_id=".$this->class_id.$sql_extra1.") o".
           " where (x.NAME = '".$name."') AND (x.LINK_ID in ". 
             " (select prim_key FROM proj_has_elem WHERE proj_id = ".$proj_id." AND table_name='LINK')) ".
             " AND (x.extra_obj_id=o.EXTRA_OBJ_ID) ";  
    $sql->query($sqls); 
    if ( $sql->ReadRow() ) {
        $retarr[]  =  $sql->RowData[0];
    }
    
    return ($retarr);
}
   
}
