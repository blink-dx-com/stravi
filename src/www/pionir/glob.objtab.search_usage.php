<?php
/**
 * track usage of ( selection of objects ) in other tables
 * @package glob.objtab.search_usage.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $tablename
         $parx["areused"]  = "no", "yes"   option: show all objects, which are used or not
         $parx["withproj"] = ["yes"], "no" take projects into account
		 $parx["maxuse"]   = a number of maximum counts to be found
		 $parx["shProjPath"] = 1 : show full project path, if one project found 
		 ["copyact"] = "" | 1
 */
session_start(); 

require_once ('reqnormal.inc');
require_once ('sql_query_dyn.inc');
require_once ('object.subs.inc');
require_once ("visufuncs.inc");
require_once ('o.PROJ.paths.inc');
require_once ('class.obj.search_usage.inc');


class glob_objtab_track_ONE { 

    function __construct( $tablename, &$parx ) {
    	$this->parx = $parx;
    	$this->tablename=$tablename;
    	$this->projPathObj = new oPROJpathC(); 
    }
    
    
    /**
     * <TR><TD></TD><TD> ...
     * @param string $search_table
     * @param int $obj_id
     * @param string $obj_name
     * @param int $obj_cnt
     * @return string
     */
    function getTabCol(string $search_table, int $obj_id, $obj_name, $obj_cnt) {
        
    	$this->bgcolor = $this->bgcolor=="#efefef" ? "#cccfff" : "#efefef";
    	
        $tabcol1  = "<tr bgcolor=". $this->bgcolor ."><td valign=top NOWRAP>".
    				"<input type=checkbox name=xs[".$obj_id."] value=1> " .
    				($obj_cnt+1). ".</td>".
    				"<td valign=top> <B><a href=\"edit.tmpl.php?t=".$search_table."&id=".$obj_id."\">" .
    				 $obj_name. "</a></B> [".$obj_id."]";
        
    	return ($tabcol1);
    }
    
    function show_now($tabcol1, $tabcol2, $tabcol3, $thisObjUsed, $extraText=NULL) {
    	$areused = $this->parx["areused"];
    	echo  $tabcol1;
    	echo  '</td>'.$tabcol2;
    	if ($areused=="yes" AND $thisObjUsed) echo "<B>".$thisObjUsed."</B> times";
    	echo " ".$extraText;
    	echo  $tabcol3;
    }
    
    function _showProjPath( &$sql, $obj_id ) {
    	// show project path
    	$table = $this->tablename;
    	$sql->query('SELECT PROJ_ID FROM proj_has_elem WHERE'.
    				" table_name = '".$table."' AND prim_key = ".$sql->addQuotes($obj_id));
    	$sql->ReadRow();
    	$projid  = $sql->RowData[0];
    	if (!$projid) return;
    	
    	if ( $this->lastProj[0] == $projid ) {
    		$retval = $this->lastProj[1];
    	} else {
    		$desturl = "edit.tmpl.php?t=PROJ&id=";
    		$stopProjID = 0;
    		$stopLevel  = 8;
    		$retval = $this->projPathObj->showPathSlim($sql, $projid, $desturl,$stopProjID, $stopLevel);
    	}
    	$this->lastProj = array($projid, $retval);
    	
    	return ("<img src=\"images/icon.PROJ.gif\"> ".$retval);
    
    }

}

class glob_objtab_search_GUI {
    
    function __construct( $tablename, &$parx, $sqlAfter ) {
        $this->parx = $parx;
        $this->tablename=$tablename;
        $this->sqlAfter = $sqlAfter;
        
        $this->one_obj = new glob_objtab_track_ONE( $tablename, $parx );
       
    }
    
    function show($sql, $sql2) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $parx=$this->parx;
        $tablename = $this->tablename;
        $sqlAfter  = $this->sqlAfter;
        
        $this->statistics=array();
        
        $tmp_info  = $_SESSION['s_tabSearchCond'][$tablename]['info'];
        
        $showflag     = 1; // show detailed results
        $tmp_usage    = "";
        $tmp_infoplus = " projects INCLUDED;";
        
        if ( $parx["withproj"]=="" )    $parx["withproj"]  = "no";
        if ( $parx["withproj"]=="no" )  $tmp_infoplus = " projects EXCLUDED;";
        
        if (  $parx["maxuse"] > 0) $tmp_infoplus .= " number of usages <= ".$parx["maxuse"].";";
        
        if ( $parx["areused"] != ""   ) $showflag  = 0;
        if ( $parx["areused"] == "no" ) $tmp_usage = "<B>UNUSED objects:</B> Show only objects which are NOT used; ".$tmp_infoplus;
        if ( $parx["areused"] == "yes" )$tmp_usage = "<B>USED objects:</B> Show only objects which are used; ".$tmp_infoplus;
        
        echo "&nbsp;<a href='glob.objtab.trackgui.php?tablename=".$tablename."'>&lt;&lt; Object tracking home</a>&nbsp;&nbsp;";
        echo "[<a href='".$_SERVER['PHP_SELF']."?tablename=$tablename'>Show all scans</a>]
          [<a href='".$_SERVER['PHP_SELF']."?tablename=$tablename&parx[areused]=yes'>only USED objects</a>] &nbsp;&nbsp;&nbsp;
          [<a href='".$_SERVER['PHP_SELF']."?tablename=$tablename&parx[areused]=no'>only UNUSED objects</a>]".
          "<br><br>\n";
        
        if ( $parx["areused"] != "" )  echo '<font color="#999999" >Usage option:</font> ',$tmp_usage,'<br>';
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
            echo '<font color="#999999">Selection condition: ',$tmp_info,'</font><P>';
        }
        
        $search_table = $tablename;
        $pk_arr = primary_keys_get2($tablename);
        if (!empty($pk_arr[1])) {
            $ftab_prim_name=NULL;
            $ftab_imp_name = NULL;
            $search_table = fk_check2($pk_arr[0], $tablename, $ftab_prim_name, $ftab_imp_name);
            $search_table_nice = tablename_nice2($search_table);
            info_out('Info', 'This elements are part of master <B>'.$search_table_nice.'</B>. Only the master object will be searched.');
        }
        
        ?>
        <script>
        <!--
        function gobo_def ( obj_id ) {
          location.href="edit.tmpl.php?tablename=<?=$search_table?>&id="+obj_id;
        }
        //-->
        </script>
        <?php
        $tmpbackurl = urlencode($_SERVER['PHP_SELF']."?tablename=".$tablename);
        echo "<form method=\"post\"  name=\"editform\"  action=\"f.clip.objcopy.php?tablename=".$tablename."&backurl=".$tmpbackurl."\">\n";
        
        echo '<table cellspacing="0" cellpadding="2" border="0" frame="void">';
        echo "<tr bgcolor=#D8D8EF>";
        echo "<td></td><th class=yBrig>Name</th><th class=yBrig>Used by objects</th>";
        echo "</tr>\n";

    
        $obj_cnt    = 0;
        $usedObjs   = 0;
        $obj_id_old = 0;
        $showcnt    = 0;
        $resinf     = NULL;
        $resinf["tooMany"] = 0;
        $tmp_main_col = PrimNameGet2($tablename) ;
        
        $sql2->query('SELECT x.'.$tmp_main_col.' FROM '.$sqlAfter);
        
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on main SQL-command.' );
            return;
        }
        
        while ( $sql2->ReadRow() ) {
        
          $obj_id 		= $sql2->RowData[0];
          $thisObjUsed	= 0;
          $showNew		= 0; // object shown now ?
           
          if ($obj_id == $obj_id_old) { 
              continue;
          }
          
          $obj_name = obj_nice_name($sql, $search_table, $obj_id);	 
          if ($obj_name == '')  $obj_name = '['.$obj_id.']';
        	
        	$tabcol2  = '<td nowrap>';
            $tabcol3  = '</td></tr>'."\n"; 
            
            if ($showflag) {
        		$tabcol1 = $this->one_obj->getTabCol($search_table, $obj_id, $obj_name, $obj_cnt);
        		echo $tabcol1;
        		$showNew=1;
            	$showcnt++;
        	}
        	$objSeaOpt = array("hideHeader"=>1);
        	$objSearch = new object_usage($sql, $search_table, $obj_id, $showflag, "", $objSeaOpt);
            
            if ($showflag) echo $tabcol2;
        	
            $tab_track_info = $objSearch->start($sql);

            foreach ($tab_track_info as $row) {
                $objSearch->getNumOneTab($sql, $row['pa_t'], $row['pa_pk']);
            }
            
        	$objSearch->getProjUsage($sql);
        	
            $thisObjUsedNoProj   = $objSearch->obj_num_all;
            $thisObjUsedwithProj = $objSearch->obj_num_all + $objSearch->proj_num_use;
        	
            $thisObjUsed = $thisObjUsedNoProj;
            if ( $parx["withproj"]=="yes" ) $thisObjUsed = $thisObjUsedwithProj;
            if ($thisObjUsed > 0) { 
              	$usedObjs++;
        	} else {
        	  	// echo '&nbsp;';
        	}
            	
            if ($showflag) {
                
                //FUTURE:  if only ONE type was found ... show parent object ... ???
                
        		echo $tabcol3;
            } else {
        		 $showNowTmp = 0;
                 if ( $parx["areused"] == "no" AND !$thisObjUsed )  $showNowTmp=1;
                 if ( $parx["areused"] == "yes" AND $thisObjUsed ) {
        		 	 $showNowTmp=1;
        			 if ( ($parx["maxuse"]>0)  AND ($thisObjUsed>$parx["maxuse"]) ) {
        			 	$showNowTmp=0;
        				$resinf["tooMany"] ++;
        			 }
        		 }
            	 if ($showNowTmp) {
        		 	$showNew   = 1;
        			$extraText = NULL;
        			$tabcol1   = $this->one_obj->getTabCol($search_table, $obj_id, $obj_name, $obj_cnt);
        			if ($parx["shProjPath"]>0 AND $objSearch->proj_num_use==1 ) {
        	  			$extraText = $this->one_obj->_showProjPath($sql, $obj_id);
        			}
        		 	$this->one_obj->show_now($tabcol1, $tabcol2, $tabcol3, $thisObjUsed, $extraText);
        			$showcnt++;
        		 }
        	}
            	
        	if ($showNew) while (@ob_end_flush()); // send all buffered output
            else echo " "; 	// put dummy space to signalize to the webbrowser, that there is still traffic
        	$obj_cnt++;
        	$obj_id_old = $obj_id;
           
        }
        
        echo '</table>';
        echo "<br>\n";
        echo "<input type=submit value=\"Copy selected objects to clipboard\" class=\"<Button\">\n &nbsp;&nbsp;&nbsp;";
        echo "<input type=button name='checker_button' value='check all' onclick=javascript:selall(".$showcnt.")>\n";
        echo '</form>'."\n";
        echo '<br>'."\n";

        $unusedObjs = $obj_cnt-$usedObjs;
        $this->statistics=array();
        $this->statistics['unusedObjs'] = $unusedObjs;
        $this->statistics['tooMany']    = $resinf["tooMany"];
        $this->statistics['obj_cnt']    = $obj_cnt;
        $this->statistics['showcnt']    = $showcnt;
        $this->statistics['usedObjs']   = $usedObjs;
    }
    

    function get_statistics() {
        return $this->statistics;
    }
}


$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( $_SERVER['PHP_SELF'] );

if ($error->printLast()) htmlFoot();

$tablename = $_REQUEST['tablename'];
$parx      = $_REQUEST['parx'];


$title = 'Object tracking &gt; scan database';

$infoarr=array();
$infoarr["help_url"] = "object_tracking.html";

$infoarr["title"] = $title;
$infoarr["form_type"]= "list";
$infoarr['obj_name'] = $tablename;
$infoarr["obj_cnt"]  = 1;          // show number of objects
		

$pagelib = new gHtmlHead();
$pagelib->_PageHead ( $infoarr["title"],  $infoarr );
?>
<script>
<!--
	function selall( num ) {
		i=0;
		  
		for( i=0; i<num; i++ ) {
			document.editform.elements[i].checked = 1;
		}
	}
//-->
</script>
<?php	
$pagelib->_startBody($sql, $infoarr);


ob_end_flush ();  // send buffered output

$sqlopt=array();
$sqlopt["order"] = 1;
$sqlAfter  = get_selection_as_sql( $tablename, $sqlopt);

$main_lib   = new glob_objtab_search_GUI($tablename, $parx, $sqlAfter);

$main_lib->show($sql, $sql2);
$pagelib->chkErrStop();

$statistics = $main_lib->get_statistics();

echo "<ul>";

$tabobj = new visufuncs();
$dataArr= NULL;
$dataArr[] = array( 'Observed objects:','<B>'.$statistics['obj_cnt'].	'</B>');
$dataArr[] = array( 'Shown objects:','<B>'.$statistics['showcnt'].		'</B>');
$dataArr[] = array( 'Used by other objects:','<B>'.$statistics['usedObjs'].	'</B>');
if ($parx["maxuse"]>0) {
 	$dataArr[] = array( 'More than '.$parx["maxuse"].' usages:','<B>'.$statistics['tooMany'].'</B>');
}
$dataArr[] = array( 'Unused objects:','<B>'.$statistics['unusedObjs'].	'</B>');

$headOpt = array( "title" => "Summary", "headNoShow" =>1);
$headx   = array ("", "");
$tabobj->table_out2($headx, $dataArr,  $headOpt);

$tabobj->table_close();

echo "</ul>";
htmlFoot();
