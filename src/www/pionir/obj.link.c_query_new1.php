<?php
/**
 * create a new document class "query"
 * @package obj.link.c_query_new1.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $name
         $t : searchtable to be destination
         $overwrite : "yes", "no"
*/

session_start(); 


require_once ('reqnormal.inc');      
require_once ("sql_query_dyn.inc");  
require_once ('db_x_obj.inc'); 
require_once ("o.DB_USER.subs2.inc");     
require_once ("insert.inc");  
require_once ("edit.sub.inc");  
require_once ( "javascript.inc" );   

require_once("o.LINK.c_query_subs.inc");
require_once("visufuncs.inc");
require_once('o.PROJ.addelems.inc'); 

class oLINK_queryNewGui {


    function create_document( &$sql, $destproj, $argu, $classargu) {
    // RETURN: ID of document
      global $error;
      global $varcol;
      
      $tablename = "LINK";
      $tmp_pk = insert_row($sql, $tablename, $argu);
      if (!$tmp_pk) {
    	$error->set('create_document', 1, 'Creation of '.$tablename.' failed!');
        return 0;
      }                                                        
      
      $class_id = $varcol->class_name_to_id( "LINK", "query");
      if ( $class_id ) {
        $pk2 = $tmp_pk; 
        $x_obj_lib = new fVarcolMeta($sql, $tablename, $pk2);
        $x_obj_lib->extra_update( $sql, NULL, $class_id, $classargu);
        if ($error->got(READONLY)) return 0;
      } else {
        $error->set('create_document', 1, 'no class_id for class "query"!');
        return 0;
      }
      if ($destproj) {
    	$projLib = new oProjAddElem( $sql, $destproj );
    	if ($error->got(READONLY)) {
    		$error->set($FUNCNAME, 7, "Copy to project ".$destproj." failed.");
        	return;
        }
    	$projLib->addObj( $sql, $tablename, $tmp_pk); 
      }  
      
      return ($tmp_pk);
    }   
    
    function update_document( &$sql, $link_id, $classargu) {
    // RETURN: ID of document
      global $error;
      global $varcol;
                                                            
      $sqls ="select EXTRA_OBJ_ID from LINK where LINK_ID=".$link_id;  
      $sql->query($sqls); 
      if ( $sql->ReadRow() ) {
            $EXTRA_OBJ_ID  =  $sql->RowData[0];
      } 
      if ( !$EXTRA_OBJ_ID ) {
        $error->set('update_document', 1, 'document has no EXTRA_OBJ_ID to identify the "query" class!');
        return 0;
      }
      
      $class_id = $varcol->class_name_to_id( "LINK", "query");
      if ( $class_id ) { 
        $x_obj_lib = new fVarcolMeta($sql, "LINK", $link_id);
        $x_obj_lib->extra_update( $sql, $EXTRA_OBJ_ID, $class_id, $classargu);
        if ($error->got(READONLY)) return 0;
      } else {
        $error->set('update_document', 1, 'no class_id for class "query"!');
        return 0;
      }  
      
      return;
    }      
    
    function cutStr( 
         $searchStr, // NEEDLE
         $foundpos,  // position of the found STRING
         $sqlExtra,  // HAYSTACK, must be TRIMMED for leading AND removals
         $infostr,   // debug info for the help text
         &$actionInfo // debug array
     ) {        
    // RETURN: 
        $actionInfo[] = array ("INPUT string:", $sqlExtra) ; 
        $sqlExtraNew ="";
        $searchStr_len = strlen($searchStr);
        if ( $foundpos>0 ) {  // other conditions in front !!!        
            $sqlExtraNew = substr($sqlExtra, 0,$foundpos ); // before NEEDDLE 
            $sqlExtraNew = trim ( $sqlExtraNew );           // remove SPACES
            
            $searchtmp = " AND";
            $searchtmp_len = strlen($searchtmp);
            $foundpos_and = strlen($sqlExtraNew) - $searchtmp_len;   
            $tmpsub = substr( $sqlExtraNew, $foundpos_and, $searchtmp_len);
            if ( $tmpsub == $searchtmp) {
                // remove also the leading "AND" 
                $sqlExtraNew = substr( $sqlExtraNew, 0, $foundpos_and);
                $searchStr = "AND ".$searchStr; // add it for info reasons
            }
        }
      
        $sqlExtraNew = $sqlExtraNew . substr($sqlExtra, $foundpos+$searchStr_len  );   // after NEEDDLE
            
        $sqlExtraNew = trim ( $sqlExtraNew );  // remove SPACES    
        $actionInfo[] = array ("replace ".$infostr." '".$searchStr."'", $sqlExtraNew) ;
    
        $search_tmp = "AND ";
        $search_tmp_len = strlen($search_tmp);
        if (  ($tmppos = strpos( $sqlExtraNew, $search_tmp)) === 0  ) { // is it at the BEGINNING of the string (pos:0) ?
            // remove the "AND" operator from the previous $search_tmp 
            $sqlExtraNew = substr($sqlExtraNew, $search_tmp_len); // take the REST
            $actionInfo[] = array ("remove the leading AND", $sqlExtraNew);
        } 
        $sqlExtraNew = trim ( $sqlExtraNew );
        return $sqlExtraNew;
    }

}

// ---------------------------------------


$error = & ErrorHandler::get();
$varcol= & Varcols::get(); 
$sql   = logon2(  );
if ($error->printLast()) htmlFoot();  

$name = $_REQUEST["name"];
$t = $_REQUEST["t"];
$overwrite = $_REQUEST["overwrite"];
$searchtable = $t;

$javastr = "
    function overSubmit( flagval ) {
        document.editform.overwrite.value = flagval;
        document.editform.submit();
    }
";

$title = 'Create a new query-document';  
$infoarr=array();
$infoarr['help_url'] = 'o.LINK.class.query.html';

$infoarr["title"] = $title;
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $searchtable;
$infoarr['obj_more'] = "&nbsp;[<a href=\"obj.link.c_query_mylist.php\">my searches</a>]";
$infoarr["javascript"]   = $javastr;

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

echo "<ul>"; 
 
$name = trim($name); // trim it!!!

if ( $_SESSION['userGlob']["g.debugLevel"]>0 ) {
    echo "<B>INFO:</B> DEBUG-mode supports 3 levels in this script.<br>\n";
}

$t_rights = tableAccessCheck( $sql, 'LINK' );
if ( $t_rights['insert'] != 1 || $t_rights['write'] != 1 ) {
	tableAccessMsg( 'document', 'insert, write' );
	return;
} 

$mainLib  = new oLINK_queryNewGui();
$queryObj = new myquery();

if ( $searchtable =="") {  
     htmlFoot("Error", "no name for destination table");
}  
$searchtableNice =  tablename_nice2($searchtable);   

if ( $overwrite == "cancel") { 
	js__location_replace('view.tmpl.php?t='.$searchtable);  
    exit;
}


$class_id = $varcol->class_name_to_id( "LINK", "query");
if ( $error->got(READONLY) ) {
    $error->set('create query document', 1, '"query class" definition missing, please ask the admin to create it!');
    $pagelib->chkErrStop();
}    

$queryProj = $queryObj->getProfQueryProj( $sql );     
if ( $error->got(READONLY) ) {
    $error->set('create query document', 1, 'creation of a project for query-collections failed!');
    $pagelib->chkErrStop();
} 

$formhead1 =  "<form method=\"post\"  name=\"editform\"  action=\"".$_SERVER['PHP_SELF']."?t=$searchtable\" >\n";
$formhead1 .= "<input type=hidden name=overwrite value=\"no\">\n";     

$formhead2 = "<B>Name of query-document</B> <input name=\"name\" value=\"".$name."\">";       

if ($name =="" ) {
     
    echo $formhead1. $formhead2;
    
    echo "<input type=submit value=\"Create query-document\">\n";
    echo "</form>";   
    $extratxt = "";
    if ( $searchtable ) {  
        $extratxt = " for table $searchtableNice";
    }
    
    echo "<font color=gray><B>Existing queries $extratxt:</B></font><br><br>";    
    
    if ( !$queryProj ) {
        echo "No personal query project found"; 
        htmlFoot();
    }     
    $queryInf_arr = $queryObj->getMyQueryDocs( $sql, $queryProj, $searchtable );    
    echo "<font size=-1>";               
    $cnt=0;
    if (sizeof($queryInf_arr)) {
        echo "<table border=0 cellpadding=1 bgcolor=\"#EFEFEF\">";
        foreach( $queryInf_arr as $dummy=>$tmparr) {        

            $name    =  $tmparr[1];  
            $notes   =  $tmparr[2];     

            echo "<tr><td><img src=\"images/icon.LINK.gif\"> ";
            echo htmlspecialchars($name)."</td>";    
            echo "<td><I>".htmlspecialchars($notes)."</I></td>"; 
            echo "</tr>\n";    
            $cnt++;
        } 
        echo "</table>\n";  
    }
    echo "</font>";
    if (!$cnt) {
        echo "&nbsp;&nbsp;&nbsp;<font color=gray>Info: no queries found.</font><br>";
    }
    
    echo "<hr>\n";
    htmlFoot();
} 
 
$sameArr = $queryObj->searchQueryByName( $sql, $queryProj, $name, $searchtable);

if ( ($sameArr != "") && ($overwrite != "yes") ) {
    
    echo "<B><font color=red>WARNING:</font></B> Name '<B>".$name."</B>' exists. Overwrite query ?<br><br>\n";  

    echo $formhead1;
    echo "\n";
    echo "<input type=button value='YES' onclick=\"javascript:overSubmit('yes')\"> \n"; 
    echo "<input type=button value='CANCEL' onclick=\"javascript:overSubmit('cancel')\"> \n";
    echo "<br><br>"; 
    
    echo $formhead2;
    echo "<input type=submit value=\"Submit a new name\">\n";
    echo "</form>";    
    
    htmlFoot();
} 
$querySameNameID = $sameArr[0];
 
// from $_SESSION['s_tabSearchCond']:
//      ."f" :  extra_obj o, extra_class c, cct_access a   => put to $query["dependencies"]
//      ."x" :  a.cct_access_id=x.cct_access_id => remove from query 
//      ."x" :  "x.extra_obj_id = o.extra_obj_id AND c.extra_class_id = o.extra_class_id" => remove
//      ."c" : $classname => $query["options"] = "useclass:$classname"        

$oriQuery_arr = $_SESSION['s_tabSearchCond'][$searchtable]; 
// 	array ( "f"=>$sqlfromXtra, "w"=>$tableSCondM, "x"=>$sqlWhereXtra, "c"=>$classname, "info"=>$sel_info, "mothid"=>$mother_idM ); /* save selection array */
$actionInfo = array();
$sqltext = $oriQuery_arr["w"];  
$actionInfo[] = array("take sql-string from s_tabSearchCond['w']", $sqltext);
$helpflags=array();
$qu_depend = "";
if ( $oriQuery_arr["f"] != "" ) {
     if ( stristr($oriQuery_arr["f"], "cct_access a") != "" ) {  
          $qu_depend =  $qu_depend. "access,"; 
          $helpflags["access"] = 1;
     }
     if ( stristr($oriQuery_arr["f"], "extra_obj o") != "" ) {  
          $qu_depend =  $qu_depend. "class,"; 
          $helpflags["class"] = 1;
     }
}

if ( $oriQuery_arr["x"] != "" )  {  // append xtra_where
     $sqlExtra = trim($oriQuery_arr["x"]);  // remove spaces
     $tmpand = "";
     
     $search_tmp = "a.cct_access_id=x.cct_access_id";
     $search_tmp_len = strlen($search_tmp);
     if ( ($tmppos = strpos($sqlExtra, $search_tmp )) !== FALSE ) {  
        // remove this string 
        $infostr =  "ACCESS-constraint";   
        $sqlExtra = $mainLib->cutStr( $search_tmp, $tmppos, $sqlExtra, $infostr, $actionInfo );
     } 
       
     $search_tmp = "x.extra_obj_id = o.extra_obj_id AND c.extra_class_id = o.extra_class_id";
     if ( ($tmppos = strpos($sqlExtra, $search_tmp )) !== FALSE ) {  
        // remove this string 
        $infostr =  "CLASS-constraint";    
        $sqlExtra = $mainLib->cutStr( $search_tmp, $tmppos, $sqlExtra, $infostr, $actionInfo );
     }
     $sqlExtra = trim ( $sqlExtra ); 
     // both sql-parts can be NULL now!
     if ($sqltext != "" && $sqlExtra !="") $tmpand = " AND ";
     $sqltext = $sqltext. $tmpand. $sqlExtra;  
}

$qu_options = "";
if ( $oriQuery_arr["c"] != "") {  // add class info
    $qu_options =  $qu_options. "useclass:".$oriQuery_arr["c"].",";
}
$argu=array();
$argu["NAME"]   =  $name;
 
$id_table   = $varcol->attrib_name_to_id( "table", $class_id ); 
$id_sqltext = $varcol->attrib_name_to_id( "sqltext", $class_id );
$id_depend  = $varcol->attrib_name_to_id( "dependencies", $class_id ); 
$id_options = $varcol->attrib_name_to_id( "options", $class_id );
$xargu=array();
$xargu[$id_table]   = $searchtable;
$xargu[$id_sqltext] = $sqltext;
$xargu[$id_depend]  = $qu_depend;
$xargu[$id_options] = $qu_options; 

if ( $_SESSION['userGlob']["g.debugLevel"]>1 ) { 
    echo "DEBUG: <br>\n";   
    $tmparr = array("Actions", "resulting SQL text");
    visufuncs::table_out( $tmparr, $actionInfo ); 
    echo "<br>\n";
}

if ( !$querySameNameID ) {
    $document_id = $mainLib->create_document( $sql, $queryProj, $argu, $xargu);
    if ( $error->got(READONLY) ) {
        $error->set('create query document', 1, 'creation of the query-document failed!');
        $error->printLast();
        return -1;
    }
} else {
    // update document
    $mainLib->update_document( $sql, $querySameNameID, $xargu);   
    if ( $error->got(READONLY) ) {
        $error->set('update query document', 1, 'update of the query-document failed!');
        $error->printLast();
        return -1;
    } 
    $document_id =  $querySameNameID;
    
}
   
$newurl = "edit.tmpl.php?t=LINK&id=".$document_id;   
echo "new query-document: <a href=\"".$newurl."\">$name</a><br>\n";
echo "ready<br>\n";

js__location_replace($newurl);  

htmlFoot();

