<?php
/**
 * function STRING-REPLACE for a $column of a number of elements
 * @namespace core::misc
 * @package glob.objtab.change_field.php
 * @link  file://CCT_QM_doc/89_1002_SDS_code.pdf   (reflected in this design document)
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $tablename   
         $column   :: put 'c:' before a column, if it is a class-colum
         $go [0], 0.5,1,2 
         $placeMode: 
         	["normal"] : normal str_replace()
             "sreg" : use simple expressions
             "ereg" : use preg_replace()
			 'multiRep' : multi string replace ","
	     $oldtext
	     $newtext
 */
session_start(); 


require_once ('reqnormal.inc');
require_once ('glob.objtab.page.inc');    
require_once ('sql_query_dyn.inc');         
require_once ('func_form.inc');
require_once ('glob.obj.col.inc');
require_once ("visufuncs.inc");
require_once 'glob.obj.touch.inc';
require_once ("glob.obj.update.inc");

class gBulkRepOne {

function __construct($table, $placeMode, $parx) {
	$this->table = $table;
	$this->placeMode = $placeMode;
	$this->parx = $parx;
	$this->prim_col   = PrimNameGet2( $table );
	
	$this->sum = NULL;
}

function initLib($colIsFromClass, $column_sql) {
	$this->colIsFromClass = $colIsFromClass;
	$this->column_sql = $column_sql;
}

function setClassParam($cls) {
	$this->cls = $cls;
}

function setTrans($trans) {
	global $error;
	$FUNCNAME='setTrans';
	$this->trans = $trans;
	
	if ($this->parx['multiRep']) {
		
		$needleArr = explode(",", $this->trans['needle'] );
		$replArr   = explode(",", $this->trans['replacestr'] );
		
		if ( sizeof($needleArr) != sizeof($replArr)) {
			$error->set( $FUNCNAME, 1, 'number of needles must equal to number of replaces.' );
			return;
		}
		$this->trans['needle']  = $needleArr ;
		$this->trans['replacestr'] = $replArr;
		
		echo "Needles: ". print_r($needleArr,1)."<br>";
		echo "Replaces: ". print_r($replArr,1)."<br>";
	}
}

function setObj($id, $name, $extra_obj_id) {
	$this->id = $id;
	$this->name = $name;
	$this->extra_obj_id = $extra_obj_id;
	$this->edit_allow = 0;
	$this->tmpinfo = '';
	$this->moreout = NULL;
	
}


// INPUT: $this->cls
function _classGetColVal( &$sql, $extra_obj_id) { 
    global $error;
    $FUNCNAME= '_classGetColVal';
    $colVal = NULL;
    $sqls = "select ".$this->cls['classColID']." from EXTRA_OBJ where EXTRA_OBJ_ID=".$extra_obj_id.
			" AND extra_class_id=".$this->cls['extra_class_id'];
    $sql->query($sqls);
    if ($sql->ReadRow() ) {
        $colVal  = $sql->RowData[0];
    } else {
        $error->set( $FUNCNAME, 1, 'object has not the defined class!');
        return;
    }
    return ( $colVal );
}

/**
 * replace in $name:  $oldtext by $newtext (all founds)
 * @param  $name
 * @param  $needle
 * @param  $replacestr
 * @return -4: not found
 */
function  col_regReplace($name, $needle, $replacestr ) { 
	global $error;
	$FUNCNAME = 'col_regReplace';
    
    
	
	$placeMode = $this->placeMode;
    
    if ( !strlen($needle) ) {
        return ( $name . $replacestr ); // not an error //return (array(-2, ""));
    }
    if ( !strlen($name) )  {
		return;
	}
	
	$reg_needle='/'.$needle.'/';
	if ( $_SESSION['userGlob']["g.debugLevel"]>1 ) {
         echo "DEBUG: needle:'".htmlspecialchars($needle)."' replacer:'".htmlspecialchars($replacestr)."'<br>\n";
    }
    
	$regexp_error=NULL;
    switch ( $placeMode ) {
         
        case "ereg":
            $text_full_new = preg_replace( $reg_needle, $replacestr, $name);
            // $regexp_error = preg_last_error();
            break;
            /*  
            echo "DEBB: needle:$needle, $replacestr, $name  ";        
            echo "SEARCH_HEX: ";  
            for ($i=0; $i<strlen($needle); $i++) echo ord(substr($needle,$i,1))." ";
            echo "<br>\n";
            for ($i=0; $i<strlen($name); $i++) echo ord(substr($name,$i,1))." ";
            echo "<br>\n";
            */
        case "sreg": 
            $needlenew     = '/'. str_replace ( "\\", "\\\\", $needle ) .'/';
            $replace2      = str_replace ( "\\", '#QBI#', $replacestr);
            $text_full_new = preg_replace( $needlenew, $replace2, $name);
            
            //TODO: Future from PHP5.2: $regexp_error = preg_last_error();
            if (function_exists('error_get_last')) {
	            $lasterror = error_get_last ( );
	            $strlen_tmp = strlen('preg_replace()');
	            if ( substr($lasterror['message'],0,$strlen_tmp)=='preg_replace()' ) {
	            	$error->set( $FUNCNAME, 2, 'Error occurred on Replacement function: '.$lasterror['message'] );
	            	return;
	            }
            }
            
            $text_full_new = str_replace ( '#QBI#', "\\", $text_full_new );
            if ( $_SESSION['userGlob']["g.debugLevel"]>1 ) {
                echo "DEBUG: needle:'".htmlspecialchars($needlenew)."' replacer:'".htmlspecialchars($replace2)."'<br>\n";
            }
            break;
		default:
			$text_full_new = str_replace ($needle, $replacestr, $name);

    }
    
    if ( !strlen($text_full_new) ) {
    	$error->set( $FUNCNAME, 1, 'result string is empty. May an error' );
		return;
    }
    
    return ($text_full_new);
}    

/** 
 * replace one param of one object
 * @except eror>=10 : severe 
 */
function _updateOne( &$sql, $go, $name ) {
	global $error;
	$FUNCNAME = '_updateOne';
	
	$edit_allow = $this->edit_allow;
	$id = $this->id;
	$needle    = $this->trans['needle']; 
	$replacestr= $this->trans['replacestr']; 
	$parx = $this->parx;
	$placeMode = $this->placeMode;
	$tablename = $this->table;
	//$extra_obj_id = $this->extra_obj_id;
	
	$colIsFromClass= $this->colIsFromClass;
	$doUpdate = 0;
        
	$newname = $this->col_regReplace( $name, $needle, $replacestr );
	
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, 'problems at name replace.' );
		return;
	}
	
	// echo "DEBXXXX: $newname,$name<br>";
	if ($newname !== $name) {
		$doUpdate = 1; // only update if needed
		$this->tmpinfo .= "<font color=#808000>replace</font> ";
		$this->sum['repFound'] ++;
	}
	    
	$this->moreout = htmlspecialchars($newname);

	if ( ($go==2) AND ($edit_allow) AND ($doUpdate==1)) {  // only if allowed
	    
		// $newname_sql = str_replace("'", "''", $newname);
		
		if  ( !$colIsFromClass ) {
		    $args = array( 'vals'=>array( $this->column_sql=>$newname  ) );
		    $UpdateLib = new globObjUpdate();
		    $UpdateLib->update_meta( $sql, $tablename, $id, $args );
		} else {
		    
		    $attrib_name = $this->column_sql;
		    $args = array( 'xobj'=> 
		        array( 
		            'extra_class_id' => $this->cls['extra_class_id'],
		            'values' => array( $attrib_name=>$newname )
		        ) 
		    );
		    $UpdateLib = new globObjUpdate();
		    $UpdateLib->update_meta( $sql, $tablename, $id, $args );
		}
			
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 10, "Error at update of obj [$id];" );
			return;
		} else {
			$this->tmpinfo .=   "<font color=green>updated</font>";
			$this->sum['updated'] ++;
		} 
	}
	
}

// @return $this->name
function anaObject( &$sql, $go ) {
	global $error;
	$FUNCNAME= 'anaObject';
	
	$tablename = $this->table;
	$id = $this->id;
	$name = $this->name;
	
	if ( $this->colIsFromClass ) {
		
        $extra_obj_id = $this->extra_obj_id;
        if (!$extra_obj_id) {
			$error->set( $FUNCNAME, 1, 'object has not the class.' );
			return;
        }
        $name = $this->_classGetColVal( $sql, $extra_obj_id); 
		$this->name =  $name;
		
        if ( $error->got() ) {
            $error->set( $FUNCNAME, 2, 'object has not the column.' );
			return;
		}
    }             
    
    $tmp_o_rights = access_check( $sql, $tablename, $id);
    if ( $tmp_o_rights["write"] ) {
        $this->edit_allow = 1;
    } else {
        $this->tmpinfo .= "<font color=red><b>no write access</b></font>";
    }
	
	if ( $go>=1 ) {
        $this->_updateOne( $sql, $go, $name );
    }

}

}

// -------

class gTABstrrep{ // TBD: implement class !!!!

function gTABstrrep($tablename, $placeMode) {
	$this->tablename = $tablename;
	$this->parx = NULL;
	if ($placeMode=="multiRep") {
		$this->parx['multiRep'] = 1;
	}
	$this->placeMode = $placeMode;
	$this->sqls_after = get_selection_as_sql( $tablename );

	$this->globColGuiObj = new globTabColGuiC();
	$this->globColGuiObj->initTab($tablename);
	
	$this->oneObjLib = new gBulkRepOne( $tablename, $placeMode, $this->parx );
}

function setColFromClass($column) {
	$this->column_sql = $column;
	$this->column = $column;
	
	$colIsFromClass = 0; 
	if ( substr($column,0,2) == "c:" ) {  
		$this->column_sql = substr($column,2); 
		$colIsFromClass = 1; 
	} else {
		$this->sql_col_name = "x.".$this->column_sql;
	}    
	$this->colIsFromClass = $colIsFromClass;
}

function out2($first, $second, $third=NULL) {
    echo "<font color=gray>$first:</font> <b style=\"color:black;background-color:#ffff66\">".htmlspecialchars($second). "</b> &nbsp;<I>$third</I><br>\n";
}  

function replaceHex( $haystack ) {
    $replacestr = ""; 
    $starti=0;
    $poslast=0;
    $infoShowAsHex = 0;
    $htmlHexNew = "";
    while  ( ($pos1=strpos($haystack,"\\x", $starti )) !== FALSE ) {
            $asctmp = chr(hexdec(substr($haystack,$pos1+2,2)));
            $replacestr = $replacestr . substr($haystack, $poslast, $pos1-$poslast) . $asctmp; 
            $starti = $pos1+4;
            $poslast= $starti;
            $infoShowAsHex=1;
    } 
    $replacestr = $replacestr . substr( $haystack, $poslast, strlen($haystack)-$poslast );   
    
    if ( $infoShowAsHex ) {
         $htmlHexNew = "HEX: length:".strlen($replacestr). " ";
         $fontx = "#00FF00";
         for ($i=0; $i<strlen($replacestr); $i++) {
             $htmlHexNew .= sprintf("<font color=\"$fontx\">%02x</font>", ord(substr($replacestr,$i,1)) );
             if ($fontx!="#000000") $fontx="#000000";
             else  $fontx = "#00FF00"; 
         }
          
    }
    
    return array($replacestr, $htmlHexNew);
}

function getclassInfo( &$sql, $sqls_after ) {
    global $error, $varcol;
    
	$tablename = $this->tablename;
    $colInfos = colFeaturesGet( $sql, $tablename, "EXTRA_OBJ_ID", 0 ) ;
    if (!is_array($colInfos)) {
        $error->set( "getclassInfo", 1, "The table has no classes!" );
        return;
    }
    
    // get one object ...  
    $sqls = "select x.extra_obj_id from ".$sqls_after;
    $sql->query($sqls);
    
    $sql->ReadRow();
    $extra_obj_id  = $sql->RowData[0];  
    
    if ( $extra_obj_id ) {
         $extra_class = $varcol->obj_id_to_class_name ( $extra_obj_id );
         if ( $error->got(READONLY) ) return;
    } else {
         // $error->set( "getclassInfo", 1, "The first object of the list has no class!" );
         return;
    }  
    $extra_class_id = $varcol->class_name_to_id( $tablename, $extra_class);
    if (  $error->got(READONLY) ) return;
    
    return (array($extra_class_id, $extra_class));
}

function column_check(&$sql, $colName, $extra_class_id) {
    global $varcol;
	
    $tablename = $this->tablename;
    $showcol=1;
    $nice_name = $colName; 
    $colcomment= "";
    
    if ( substr($colName,0,2)=="c:" ) {  // class parameter
        $showcol = 0;
        $ccol    =  substr($colName,2); 

        $attrib_id = $varcol->attrib_name_to_id($ccol, $extra_class_id);
        
        if ( $attrib_id ) {
            $showcol  = 1;  
            $nice_name = $varcol->attrib_id_to_nice_name ($extra_class_id, $attrib_id);
            $colcomment= $varcol->get_attrib_comment     ($extra_class_id, $attrib_id);
        } else {
            echo "<B>Error:</B> Class-Column '$ccol' not found for class-id:$extra_class_id.<br>\n";
        }
        
    } else {   

        $colInfos = $this->globColGuiObj->analyzeColumn( $sql, $colName ); 
		$showcol = $colInfos["showcol"];  

        if ( $showcol>0 ) { 
            $nice_name = $colInfos["nice"];
            $colcomment= column_remark2($tablename, $colName );
        }
    }
	
    return array( $showcol, $nice_name, $colcomment );
}

function rowOut($colname, $nice_name, $colcomment, $selflag=0) {

	$fieldx = array ( 
		"title"  => $nice_name, "name" => "column", "namex"=>1,
		"object" => "radio", "notes" =>  $colcomment, "inits" => $colname
			);
	if ($selflag) $fieldx["val"] = $colname;
	$this->formobj->fieldOut( $fieldx );
		
}
 
function form_out( &$sql, $sqls_after) {
    global  $error, $varcol;
    
	$tablename = $this->tablename;
    $colNames = columns_get2($tablename);   
	
    $retarr = $this->getclassInfo( $sql, $sqls_after );
    if ( $error->got() ) {
        // no class info 
        // $error->printall();
        $class_params="";
        echo "<font color=gray>Info: the first element of the list has no class.</font><br>\n";
    } else { 
		
        $extra_class_id=$retarr[0];
		$extra_class   =$retarr[1];
        
        $class_nice = $varcol->class_id_to_nice_name($extra_class_id);
        $ccolumns   = $varcol->get_attrib_names($extra_class_id);
        if (sizeof($ccolumns) ) {
			foreach( $ccolumns as $dummy=>$tmpname) {
				$colNames[] = "c:".$tmpname;
			}
		}
    }
    
	
	
	$tablename = $this->tablename;
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Select columns";
	$initarr["submittitle"] = "Next &gt;&gt;";
	$initarr["tabwidth"]    = "AUTO";
	$initarr["goNext"]      = "0.5";
	$initarr["tabnowrap"] = 1;
	
	$hiddenarr = NULL;
	$hiddenarr["tablename"] = $tablename;
	// $hiddenarr["col"]     	= $col;
	

	$this->formobj = new formc($initarr, $hiddenarr, "0"); // go=0.5
       
    $firstClassParam=1;
	$cnt=0;
	$mainColName = 'NAME';
	$nice_name   = "name";
	$arguidarr = array_keys( $colNames, $mainColName );
	reset($colNames);
	$name_pos = "";
	if ( sizeof($arguidarr) ) {
		$name_pos = $arguidarr[0];
		$this->rowOut($mainColName, $nice_name, $colcomment, 1);
		unset ($colNames[$name_pos]); // delete column from list
		
		$fieldx = array ("object" => "space");
		$this->formobj->fieldOut( $fieldx );
	}
 
	
    foreach( $colNames as $dummy=>$colName) {
      
        $showcol   = 1;
           
        list ($showcol, $nice_name, $colcomment)  = $this->column_check( $sql, $colName, $extra_class_id);   
        
        if ( $showcol ) { 
            if ( (substr($colName,0,2)=="c:") && $firstClassParam) {
                $fieldx = array ( "title" => "<B>Class parameters for class '$class_nice'</B>", "object" => "info" );
				$this->formobj->fieldOut( $fieldx );
				$firstClassParam=0;
            }
            $this->rowOut($colName, $nice_name, $colcomment);
        }

      }
       
      $this->formobj->close( TRUE );


}
           



function classInit( &$sql, $sqls_after ) {
	global $error;
	
	$retarr = $this->getclassInfo( $sql, $sqls_after );
    if ($error->printLast()) {
        htmlFoot("Error", " You want to modify a CLASS column, but the first object has no class!");
    }  
    list ( $this->cls['extra_class_id'], $this->cls['extra_class'] ) = $retarr;
     
    $sqls = "select MAP_COL from EXTRA_ATTRIB where EXTRA_CLASS_ID=".$this->cls['extra_class_id']." AND NAME='".
			$this->column_sql."'";
    $sql->query($sqls);
    if ( $sql->ReadRow() ) {
         $this->cls['classColID'] = $sql->RowData[0];    // get column-Name e.g. "S02"
    }  else {
        htmlFoot("Error", "Class not known or column not known");
    }   
	
    $this->sql_col_name = "x.EXTRA_OBJ_ID";  // get extra_obj_id 
    $this->ExtraInfo    = " Class: <B>". $this->cls['extra_class'] ."</B>";
	
	$this->oneObjLib->setClassParam($this->cls);
}

function translateReplacer( $oldtext, $newtext ) {

    $needle     = $oldtext;
    $replacestr = $newtext;
    
    if ($this->placeMode=="ereg") {
        list($needle    , $needleInfo)  = $this->replaceHex( $oldtext );
        list($replacestr, $replaceInfo) = $this->replaceHex( $newtext );
    }
	
	$this->trans['needle']     = $needle;
	$this->trans['replacestr'] = $replacestr;
	
	$oldtextHtml  = htmlspecialchars($oldtext); 
	$newtextHtml = htmlspecialchars($newtext);
	
	echo "<table cellpadding=1 cellspacing=1 border=0>";
    echo "<tr><td><font color=gray>Old_text:</font>&nbsp;&nbsp; </td><td bgcolor=#EFEFEF><B>".
			$oldtextHtml."</B></td><td>".$needleInfo."</td></tr>\n";
    echo "<tr><td><font color=gray>New_text:</font>&nbsp;&nbsp; </td><td bgcolor=#EFEFEF><B>".
			$newtextHtml."</B></td><td>".$replaceInfo."</td></tr>\n";
    echo "</table>\n";
	
	$this->oneObjLib->setTrans($this->trans);
}

function initSingleLib( &$sql ) {
	$colIsFromClass = $this->colIsFromClass;
	$column = $this->column;
	$tablename = $this->tablename;
	
	$this->oneObjLib->initLib( $colIsFromClass, $this->column_sql);
	
	if ($colIsFromClass) {   
	  $this->classInit( $sql, $this->sqls_after );
	  $extra_class_id = $this->cls['extra_class_id'];
	} else {
		list( $tmpallow, $dummy1, $dummy2) = $this->column_check( $sql, $column, 0 );
		if ( !$tmpallow ) {
			htmlFoot("Error", "The column '$column' is not allowed to manipulate for table '$tablename'");
		}
	}

}

function help() {
	echo "<br>\n";
    htmlInfoBox("General Help","", "open");
	 echo "<ul>";
    echo "<li>The script exchanges Old_text by New_text <br>";
    echo "<li>Example:<br>String: <B>Test_07_check</B> Old_text:<B>7</B> New_text:<B>Y8</B>  Result: <B>Test_0Y8_check</B> <br>";
	echo "</ul>";
   htmlInfoBox("General Help","", "close");
     
           
    echo "<br><a name=\"helpex\">\n";   
    
    htmlInfoBox("Simple expressions","", "open");
    $this->out2("Wildcard character ", ".", "e.g. '.ab.' is 'sabo' or 'Cabi' "); 
    $this->out2("Beginn of line", "^");
    $this->out2("End of line","$");
    htmlInfoBox( "","", "close" );
    echo "<br>\n";
    
    htmlInfoBox("Hardcore: regular expressions &nbsp;&nbsp;[<a href=\"http://www.php.net/regex\" target=help>more help</a>]","", "open");
    
    $this->out2("One wildcard character ", ".", "e.g. '.ab.' is 'sabo' or 'Cabi' ");
	$this->out2("Many wildcard characters ", ".*", "e.g. '.*' for string 'Cabi' finds 'Cabi' "); 
    $this->out2("Beginn of line", "^");
    $this->out2("End of line","$");   
	$this->out2("Full line ", "(^.*".chr(36).")" );
    $this->out2("Character in hexadecimal notation", "\xhh", "e.g. \\x09 is TAB; use for needle and replacer");
    $this->out2("Special character","\\", "Used for special commands!<br>\n\n");  
    
    echo "Ex1: This replaces URLs with links:<ul>\n";
    echo "Old_text: ". htmlspecialchars("[[:alpha:]]+://[^<>[:space:]]+[[:alnum:]/]"). "<br>";
    echo "New_text: ". htmlspecialchars("<a href=\"\\0\">\\0</a>"). "<br>\n"; 
    echo "</ul>\n"; 
	
	echo "Ex2: Add a newstring BEFORE the old string:<ul>\n";
	$this->out2("Old_text ", htmlspecialchars("(^.*$)"));
    $this->out2("New_text ", htmlspecialchars("PRE_\\1")); 
	echo "<br>\n"; 
	$this->out2("Old data ","ratte_ole");
    $this->out2("New data ", "PRE_ratte_ole"); 
	echo "</ul>\n"; 
	 
    echo "Ex3: Exchanges positions of parts 'XZ' =&gt; 'ZX':<ul>\n";
    
    ?>
    Old_text: '<b>(^...*)(9566....)</b>'  &nbsp;&nbsp;&nbsp; <br>New_text: '<b>\2_\1</b>' 
    <table cellpadding=1 cellspacing=1 border=0 bgcolor=#B0B0B0>
	<tr bgcolor=#000000><td><font color=white>ID</td><td><font color=white>info</td><td><font color=white>before</td><td><font color=white>after</td></tr>
	<tr bgcolor=#EFEFEF class=min><td>96980</td><td><font color=#808000>replace</font> </td><td>P7_9566046_SIR2</td><td>9566046_P7_SIR2</td></tr>
	<tr bgcolor=#EFEFEF class=min><td>96984</td><td><font color=#808000>replace</font> </td><td>P7_9566049_SIR2</td><td>9566049_P7_SIR2</td></tr>
	</table>
    <font color=gray>Description:</font><br>
    (^...*)    <font color=gray>Search from beginning of line 3 characters</font><br>
    (9566....) <font color=gray>Search '9566' and the 4 characters after</font><br>
    <font color=gray>Replacement:</font> \2_\1  <font color=gray>Write the SECOND and than the FIRST part</font>
    <?
    htmlInfoBox( "","", "close" );
	echo "<br>\n";
	htmlInfoBox("String pairs","", "open");
	echo "Ex1: Replace multiple string, separated by KOMMAS<br>\n";
    $this->out2("Old_text ", "MMM,AAA,BBB");
    $this->out2("New_text ", "Em, As, Bs"); 
	echo "<br>\n"; 
	$this->out2("Old_data ", "YYY MMM AAA UUU"); 
	$this->out2("New_data ", "YYY Em As UUU"); 
    htmlInfoBox( "","", "close" );
    echo "<br>\n";
     
    for ($i=0;$i<30;$i++) echo "<br>\n";
}

function showAll( &$sql, &$sql2, $go ) {
	global $error;
	
	$sqls_after = $this->sqls_after;
	$tablename = $this->tablename;
	$sql_col_name = $this->sql_col_name;
	$prim_col   = PrimNameGet2( $tablename );
	
	$sqlImpName = importantNameGet2($tablename);
	// test if equal to analyzing column !!! 
	$objNameShow = 1;
	if ($this->column_sql==$sqlImpName) $objNameShow=0;  // show MAIN column only, if itz is different to the ANALYZING column
	
	echo "<font color=gray><B>The data ...</B></font><br>\n";        
	echo "<table cellpadding=1 cellspacing=1 border=0 bgcolor=#B0B0B0>\n"; 
	echo "<tr bgcolor=#000000><td><font color=white>ID</td><td><font color=white>info</td>";
	if ($objNameShow) echo "<td><font color=white>object name</td>";
	echo "<td><font color=white>before</td>";
	if ($go>=1) echo "<td><font color=white>after</td></tr>\n"; 
	
	$sqls = "select x.$prim_col, x.".$sqlImpName.", ".$sql_col_name.
			  " from ".$sqls_after. " order by x.".$prim_col;
	$sql->query($sqls); 
	$colIsFromClass = $this->colIsFromClass;
	
	while ( $sql->ReadRow()  ) {
	  
		$extra_obj_id = 0;  
		$name     = "";
		$id       = $sql->RowData[0];
		$objname  = $sql->RowData[1];
		$error_id = 0;
		
		if ( !$colIsFromClass ) {
			$name   = $sql->RowData[2];  // if it is extra_class column -> contains extra_obj_id
		} else {   
			$extra_obj_id = $sql->RowData[2];
		}
		
		$this->oneObjLib->setObj($id, $name, $extra_obj_id);
		$this->oneObjLib->anaObject( $sql2, $go );
		
		$name= $this->oneObjLib->name;
		$tmpinfo= $this->oneObjLib->tmpinfo;
		$moreout= $this->oneObjLib->moreout;
		
		if ($error->Got(READONLY))  {
	
			$errLast   = $error->getLast(NULL, 0, READONLY);
			$error_txt = $errLast->text;
			$error_id  = $errLast->id;

			if ( $error->Got(READONLY,'col_regReplace') ) {
				// severe REG_EXP-error
				$error_id = 11;
				$error_txt = $error->getAllAsText();
		    }
		    
			
			$error->reset();
			$tmpinfo = "<font color=red>Error:</font> ".$error_txt." errid:".$error_id;
			$this->oneObjLib->sum['err'] = $this->oneObjLib->sum['err'] + 1;
		}
		echo "<tr bgcolor=#EFEFEF class=min><td>$id</td><td>$tmpinfo</td>";
		if ($objNameShow) echo "<td>".htmlspecialchars($objname)."</td>";
		echo "<td>". htmlspecialchars($name)."</td>";
		if ($go>=1) echo "<td>" .$moreout. "</td></tr>\n";
		
		if ( $go<1 && $cnt>=4) {    // only show first elements
			echo "<tr><td>... more ...</td></tr>\n";
			break;
		}
		$this->oneObjLib->sum['cnt']++;
		
		if ($error_id >=10) {
			echo "<tr bgcolor=#EFEFEF ><td>...</td><td><font color=red><b>Stopped due to severe problem</b></font></td></tr>\n";
			break;
		}		
		
	}  
	echo "</table>"; 
	
	$sum = $this->oneObjLib->sum;
	
	echo "<br>\n";
	 
	$tabobj = new visufuncs();
	$dataArr= NULL;
	$dataArr[] = array( 'Elements analysed:', '<B>'.$sum['cnt'].'</B>' ); // if ( $go >= 1 ) 
	
	if ( $go>=1 ) {
	    $dataArr[] = array( 'Found replacements', '<B>'.$sum['repFound'].'</B>' ); 
	}		
	if ( $go==2 ) {
	    $dataArr[] = array( '<font color=green>Updated elements:</font>', '<B>'.$sum['updated'].'</B>' ); 
	}
	if ($sum['err']>0 ) {
		$dataArr[] = array( '<font color=red>Errors:</font>', '<B>'.$sum['err'].'</B>' ); 
	}
	
	$headOpt = array( "title" => "Summary", "headNoShow" =>1);
	$headx   = array ("Key", "Val");
	$tabobj->table_out2($headx, $dataArr,  $headOpt);

}

}

// -------------------------------------------------------------------
// -------------------------------------------------------------------
// -------------------------------------------------------------------


$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( $_SERVER['PHP_SELF'] ); 
if ($error->printLast()) htmlFoot(); 
$varcol    = & Varcols::get(); 

$tablename=$_REQUEST['tablename'];
$go=$_REQUEST['go'];
$column =$_REQUEST['column'];
$placeMode=$_REQUEST['placeMode'];
$oldtext=$_REQUEST['oldtext'];
$newtext=$_REQUEST['newtext'];

$mainerr=0; 



$placeModeArr=array();
$placeModeArr["normal"] = "normal";
$placeModeArr["sreg"]   = "simple expressions";
$placeModeArr["ereg"]   = "regular expressions";   
$placeModeArr["multiRep"] = "string pairs";    
	
$table_nice = tablename_nice2($tablename);

$title = "Bulk String Replace: string replace of selected objects"; 
$titleShort = "Bulk String Replace: ". $table_nice; 
$title_sh = "Bulk String Replace"; 
$infoarr=array();
$infoarr["title"] 	 = $title;
$infoarr["title_sh"] = $title_sh;
$infoarr["form_type"]= "list";
$infoarr['help_url'] = 'g.bulk_string_replace.html';  
$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1; 
$infoarr["css"]  = 'tr.min   { font-size:0.8em; }'; 

$pagelib = new gObjTabPage($sql, $tablename );
$pagelib->showHead($sql, $infoarr);
$pagelib->initCheck($sql);

   
echo "<ul>\n";
if ($tablename== "") {
    htmlFoot("Error: Give a tablename");
} 

if ($placeMode=="")  $placeMode="normal";

$mainScrLib = new gTABstrrep($tablename, $placeMode);

gHtmlMisc::func_hist( "glob.objtab.change_field", $titleShort, 'glob.objtab.change_field.php?tablename='.$tablename );

$tmppk = primary_keys_get2($tablename) ; 
if (sizeof($tmppk)>1) {
     htmlFoot("Error", "Only tables with ONE primary key allowed (like business objects)! Please ask the developers.");
}

$sqls_after  = $mainScrLib->sqls_after;       
      
if (!$go) $go=0;
echo "<B><font size=+1>";
switch ($go) { 
    case 0:   
        echo "1. Column selection";
        break;
    case 0.5:   
        echo "2. Parameter preparation";
        break;   
    case 1:
        echo "3. Analyse"; 
        break;
    case 2:
        echo "4. Update";       
        break;
}
echo "</font></B>";
if ( $go ) {
    echo " &nbsp; [<a href=\"".$_SERVER['PHP_SELF']."?tablename=".$tablename."\">Start again</a>]";
} 
echo "<br><br>\n";  
 

if ( !$go ) { 
	$mainScrLib->form_out( $sql, $sqls_after);
	htmlFoot("</ul><hr>"); 
 
} 
 
// column preparation 

if ($column== "") {
    htmlFoot("Error: Give a column (parameter-name: column");
}

$mainScrLib->setColFromClass($column);
$mainScrLib->initSingleLib($sql);

echo "Column: <B>".$mainScrLib->column_sql."</B>".$mainScrLib->ExtraInfo."<br><br>\n"; 


if ( $go==0.5 ) {
    
    echo "<form method=\"post\"  name=\"editform\"  action=\"".$_SERVER['PHP_SELF']."?tablename=$tablename&column=$column&go=1\" >"; 
    echo "<table border=0 bgcolor=#EFEFEF>";
    echo "<tr><td> <B>Old_text</B> </td><td> <input type=text name=oldtext value=\"$oldtext\"></td><td><I>To APPEND text, just let it empty</I> </td></tr>";
    echo "<tr><td> <B>New_text</B> </td><td> <input type=text name=newtext value=\"$newtext\"></td><td></td></tr>";
    echo "<tr><td> <font color=gray><B>Replace mode</B></font> </td><td> "; 
 
    
    $tmphtml = formc::selectFget( "placeMode", $placeModeArr, $placeMode );
    echo $tmphtml;
    echo "</td>";
    echo "<td><I>Think hard core ... <a href=\"#helpex\">more help</a></td></tr>";  
    
    echo "<tr bgcolor=#DFDF00><td>&nbsp;</td><td> <input type=submit value=\"Analyse &gt;&gt;\"></td><td>&nbsp;</td></tr>";
    echo "</table>\n";  
    echo "</form>"; 

    
} else {

    $old_len        = strlen($oldtext);
    if (!$old_len)  {  
          echo "<font color=red>Warning:</font> 'Old_text' is EMPTY. The New_text will be appended at the end of NAME.<br>";
    }   
       
	$mainScrLib->translateReplacer($oldtext, $newtext);
    
	
    $placeModeNice = $placeModeArr[$placeMode];   
    if ($placeModeNice=="") {
        htmlFoot("Error", "Unknown replace mode: $placeMode");
    } 
    echo "<font color=gray>Replacemode:</font> <B>$placeModeNice</B> ($placeMode)<br>\n";
     
    echo "<br>\n"; 
       
    if ($go==1) {
        $hopt=NULL;
        $hopt["width"] = "AUTO";
        htmlInfoBox( "Update now", "", "open", "INFO", $hopt ); 
        echo "<form method=\"post\"  name=\"editform\"  action=\"".$_SERVER['PHP_SELF']."?tablename=$tablename&column=$column\" >"; 
        echo "<input type=hidden name=oldtext value=\"".htmlspecialchars($oldtext)."\">\n";
        echo "<input type=hidden name=newtext value=\"".htmlspecialchars($newtext)."\">\n"; 
        echo "<input type=hidden name=placeMode value=\"".$placeMode."\">\n";
        echo "<input type=hidden name=go value=\"2\">\n";  
        echo "<input type=button name='dummy' value='&lt;&lt; Back' onclick=\"document.editform.go.value=0.5; document.editform.submit();\"> \n";
        echo "<input type=submit value=\"Update NOW &gt;&gt;\">"; 
        echo "</form>";
        htmlInfoBox( "", "", "close" );
		echo "<br>"; 
    }
}  


 $mainScrLib->showAll( $sql, $sql2, $go );


if ( $go == 0.5 ) {                  
    $mainScrLib->help();
}


if ($mainerr<0) {
    echo "<font color=red>Warning:</font> Program finished with error(s).<br>";
} 
echo "</ul>\n";
echo "<hr noshade>";

htmlFoot();

?>
