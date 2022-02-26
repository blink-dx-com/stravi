<?php
/**
 * import file and check, if the object exists in database.
 * @package glob.objtab.impsea.php
 * @swreq UREQ:0003206: g > existence check of objects (glob.objtab.impsea.php) 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $tablename
 *    $go
	  $parx:
         [$parx["projx"]]    - optional destination project 
         $parx["objstrings"] - object strings as text field
         $parx["column"]     - NAME or ID, 'EXTRA_OBJ_ID'
		 'classid'
		 'classParam'		 - Name of class-param
         $parx["projDelObj"] - 0|1 delete elements in project ???
         $parx["like"]		 - 0|1 like search
         $parx["ignoreCase"] - 0|1 ignore case
		 $parx["method"]     - "dirty" search parts of name
		 $parx["shNotFound"] - show only non found objects
		 $parx["infolevel"]  - 0,1,2
         $userfile
 */
 
session_start(); 


require_once ("reqnormal.inc");
require_once ('func_form.inc');
require_once ('class.filex.inc');
require_once ('javascript.inc'); 
require_once ('o.PROJ.addelems.inc');
require_once ("visufuncs.inc"); 
require_once ("f.upload.inc");
require_once ('glob.objtab.sub.inc');
require_once ("o.PROJ.addelems.inc");
require_once ('f.progressBar.inc');
// -----------------------------------------


class gImpSeaObjFile {
    
    function __construct($objtext, $filename) {
        $this->objtext=$objtext;
        $this->filename=$filename;
        $this->line='';
        
        $this->streamtype='';
        if ($this->filename!='') $this->streamtype='FILE';
        if ($this->objtext!='')  {
            $this->streamtype='TEXT';
            $this->textarr = explode("\n",$this->objtext);
        }
    }
    
    /**
     * return number of lines
     */
    function countLines() {
        
        if ($this->streamtype=='FILE') {
            $userfile  = $this->filename;
            $FH = fopen( $userfile, 'r');
            if ( !$FH ) {
                return -1;
            }
            
            $cnt=0;
            while( !feof ( $FH ) ) {
                $line = fgets($FH, 8000); // trim !!!
                $cnt++;
            }
            
            fclose($FH);
        } else {
            $cnt = substr_count ( $this->objtext,"\n");
            
        }
        
        $this->max_lines=$cnt;
        return $cnt;
    }
    
    function openstream() {
        if ($this->streamtype=='FILE') {
            $this->FH = fopen( $this->filename, 'r');
        } else {
            $this->line_cnt=0;
        }
    }
    
    function read_row() {
        if ($this->streamtype=='FILE') {
            $answer = !feof ( $this->FH );
            $this->line = trim(fgets($this->FH, 8000));
        } else {
            $answer=0;
            if ($this->line_cnt<$this->max_lines) {
                $answer=1;
                $this->line = trim($this->textarr[$this->line_cnt]);
                $this->line_cnt = $this->line_cnt + 1;
            }
        }
        return $answer;
    }
    
    function close() {
        if ($this->streamtype=='FILE') {
            fclose ( $this->FH); 
        }
    }
}

class gObjtabImpsea {

function __construct( $tablename, $go, $parx, $scriptid ) {
	$this->parx = $parx;
	$this->tablename = $tablename;
	$this->go = $go;
	$this->scriptid = $scriptid;
	

	if ( !$go ) {
		$this->parx["projDelObj"] = 0;
	}

	$this->infolevel = $parx["infolevel"];
	$this->searchLib = new fileC();

	$this->defaultKeys = array( "projx","like","ignoreCase", "column","method","infolevel", "shNotFound");
	$this->splitchars = "#[^[:alnum:]]+#";
}


function info ($key, $text, $notes=NULL, $prio=NULL ) {
    // FUNCTION: print out info text
    if ($notes!="")  $notes = " &nbsp;<I>".$notes."</I>";
	$show=0;
	if ($prio>0) $show=1; 
	if ( $this->infolevel>0 ) $show = $this->infolevel;
    if ($show)
		echo "<font color=gray>".$key.":</font> <B>".$text."</B>".$notes."<br>\n";

}

function t1($key1, $key2) {
 	echo "<tr bgcolor=#D0D0FF ><td>$key1</td><td>$key2</td></tr>\n";
}

function str2sql($searchstr) {
	$sqlstr = str_replace ("'", "''",$searchstr);
	return ($sqlstr);
}

function searchx( 
	&$sql, 
	$taketab,	 // name of table
	$pkname, 
	$where, 
	$option=NULL
	) {
    $found_matches=0;
	
	$sqlAfter = $taketab." x where ".$where;
	
    
    $sqls = "select ".$pkname." from ".$sqlAfter;
	// echo "SQL: $sqls<br>";
    $sql->query($sqls);
    if ( $sql->ReadRow() ) {
        $tmpid = $sql->RowData[0];
        $found_matches = 1;
    } 
    if ( $sql->ReadRow() ) {
        $found_matches = 2;
    }
    return array($found_matches, $tmpid);
}

function dirtySearchOne( &$sql, $taketab, $pkname, $mainname, $idx ) {
    $CONST_BRTAG='<br>';
    $searchdone='';
    $found_matches=0;
	if (!$found_matches ) { 
		$where = "UPPER(".$mainname.") like UPPER('".$this->str2sql($idx)."')";
		list ($found_matches, $found_id) = $this->searchx( $sql, $taketab, $pkname, $where);
		$searchdone .= $searchBrTag."exact NAME: '".$idx."'";
		$searchBrTag = $CONST_BRTAG;
	}

	if (!$found_matches) { 
		$where = "UPPER(".$mainname.") like UPPER('%".$this->str2sql($idx)."%')";
		list ($found_matches, $found_id) = $this->searchx( $sql, $taketab, $pkname, $where, $searchXOpt);
		$searchdone .= $searchBrTag."parts of NAME: like '%".$idx."%'";
		$searchBrTag = $CONST_BRTAG;
	}
	
	return array($found_matches, $found_id);
}

/**
 * dirty search 
 * @param object $sql
 * @param string $tablename
 * @param string $searchColumn
 * @param string $objName
 * @param string $pkname
 * @param string $mainname
 * @param string $splitchars
 * @param int $infolevel
 * @return array
 */
function dirtySearch( &$sql, $tablename, $searchColumn, $objName, $pkname, $mainname, $infolevel) {
		
	$splitchars = $this->splitchars;
	
	$sepNameArr = preg_split($splitchars, $objName);
	if (sizeof($sepNameArr)<=1) return; // no spaced array
	
	$found_matches = 0;
	$found_id      = 0;
	$nicename      = "";
	$searcharr	   = NULL;
	foreach( $sepNameArr as $dummy=>$namePart) {
	
		if ($namePart=="") 		  continue;
		if (strlen($namePart)<=2) continue;
		
		list($found_matches, $found_id) = $this->dirtySearchOne( $sql, $tablename, $pkname, $mainname, $namePart );
		if ($infolevel>0) $searcharr[] = array($namePart, $found_matches); 
		if ($found_matches==1) break; 
	}
	
	if ($found_matches==1) {
		// get name
		$nicename = obj_nice_name($sql, $tablename, $found_id);
	} else {
		$found_id = 0;
	}
	
	return array($found_matches, $found_id, $nicename, $searcharr);
}

function getById( &$sql, $tablename, $pk_name, $id ) {
	$sqls = "select ".$pk_name.", name from $tablename WHERE $pk_name=".$id;
    $sql->query("$sqls");
    $sql->ReadRow();
    $objid  = $sql->RowData[0];
 	$objname  = $sql->RowData[1];
    return array( $objid, $objname );
}

function projGetElemNum( &$sql, $proj_id, $tablename) {  
    $sqls = "select count(*) from proj_has_elem WHERE proj_id = ".$proj_id. " AND TABLE_NAME = '".$tablename."'";
    $sql->query("$sqls");
    $sql->ReadRow();
    $numObj  = $sql->RowData[0];
    return ($numObj);
}  

/**
 * unlink all objects in project $proj_id of type $tablename
 * @todo use class fAssocUpdate or other !!!
 */
function _projObjDel( &$sql, $sql2, $proj_id, $tablename ) { 
	if ( !is_numeric($proj_id) )   sys_error_my( 'project not initialized.' );
	
	$proj_lib = new oProjAddElem($sql, $proj_id);
	
	$sqlsel = "PRIM_KEY from proj_has_elem WHERE proj_id = ".$proj_id. " AND TABLE_NAME = '".$tablename."'";
	$sql2->Quesel($sqlsel);
	while ($sql2->ReadRow() ) {
	   $loop_id  = $sql2->RowData[0];
	   $proj_lib->unlinkObj($sql, $tablename, $loop_id);
	}
    
    return;
}

function endx() {
    $this->help();
    echo "</ul>";
    
}   

function getConcInfo( &$sql, $objId ) {
    $sqls = "select a.NAME from  CONCRETE_SUBST c, ABSTRACT_SUBST a where CONCRETE_SUBST_ID=".$objId.
            " AND c.ABSTRACT_SUBST_ID=a.ABSTRACT_SUBST_ID";
    $sql->query($sqls);
    $sql->ReadRow();
    $tmpName = $sql->RowData[0];
    return ($tmpName); 
}

function help() {

    echo "<br>";
    htmlInfoBox( "Help (version:2020-05-29)", "", "open", "HELP" ); 
    ?>
    <ul>                    
    <li> imports a file or textfield and searches the objects in the database (which give exactly ONE match)</li>
	<li> the result of this tool is a list of IDs of the found objects</li>
    <li> checks, if given NAME or ID of the object exists in database</li>
	<li> remove leading, ending double quotes '"'</li>
    </ul><br>
    <B>Typical File format: NAME search</B><UL>
    <table border=1>
    <tr><td>subst1</td></tr>
    <tr><td>subst2 extx</td></tr>
    <tr><td>subst3 renat5</td></tr>
    <tr><td>...</td></tr>  
    </table>
	</ul>
	<br>
	<B>Typical File format: ID search</B><UL>
	<table border=1>
    <tr><td>345</td></tr>
    <tr><td>87</td></tr>
    <tr><td>235</td></tr>
    <tr><td>...</td></tr>  
    </table>
	</ul>
	<br>

	<b>Special flags:</b><br><br>
	<ul><a name="partsinname"></a>
	<b><font color=green>&lt;Parts in name&gt;</font></b><br>

	<i>This search method searches for ANY word in a line.<br>
	It favoures the entry who gives extactly ONE match</i>
	
	<?
	
	$tabobj = new visufuncs();
	//$headOpt= array( "title" => "example for matches");
	$headx  = array("line in file", "found database objects");
	$rowOpt = array("bgcolor" => "#E0E0FF");
	
	$tabobj->table_head($headx);
	$tabobj->table_row (array("dynamo <b>MOSKVA</b>", "kosmonavt <B>moskva</b> mojo"));
	$tabobj->table_row (array("rosija osch", "Rosija Osch"));

	$tabobj->table_close();
	
	
	
	
	
	?>
	
    </UL>
    <?
    htmlInfoBox( "", "", "close");
	echo "<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;";
	echo "<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;<br>&nbsp;\n";
    
}  

function getObj( &$sql, $table, $objName) {
  
    $primCol = $table."_ID";
    $objNameSql = str_replace ("'", "''", $objName);
    $sqls = "select ".$primCol." from  ".$table." where NAME='".$objNameSql."'";
    $sql->query($sqls);
    $sql->ReadRow();
    $tmpid = $sql->RowData[0];
    return ($tmpid); 
    
}

function formshow1( &$sql, $tablename, &$parx ) {
        global  $varcol;
        
        $initarr   = NULL;
        $initarr["action"]      = $_SERVER['PHP_SELF'];
        $initarr["title"]       = "Parameters";
        $initarr["submittitle"] = "Submit";
        $initarr["tabwidth"]    = "AUTO";
        $initarr["ENCTYPE"]     = "multipart/form-data";
		$initarr["dblink"]      = 1;
        
        $hiddenarr = NULL;
        $hiddenarr["tablename"] = $tablename;
        $formobj = new formc($initarr, $hiddenarr, 0);
        
        $objtabobj = new objtabSubC();
        $colopt    = array("withpk"=>1, 'withEXOB'=>1 );
        $colarr    = $objtabobj->colsGetNorm( $sql, $tablename, $colopt );
        
        if ($parx["column"]=='') {
            $parx["column"] = importantNameGet2($tablename); 
        }
    
        
        $fieldx = array (
            "title" => "Objects", "name"  => "objstrings",
            "colspan"=>2, "val" => $parx["objstrings"],
            "inits"=>array('cols'=>70,'rows'=>10),
            "notes" => "names or IDs of object", "object" => "textarea" );
        $formobj->fieldOut( $fieldx );
        
        $fieldx = array ("title" => "Test Column", "name"  => "column",
            "val" => $parx["column"], "object" => "select", "inits"=> $colarr,
            "notes" => "the column which represents the data");
        $formobj->fieldOut( $fieldx );
        
    
        $fieldx = array (
			"title" => "File", "name"  => "userfile", "namex" => TRUE,
            "colspan"=>2, "optional"=>"1",
            "notes" => "optional: file to import", "object" => "file" ); 
        $formobj->fieldOut( $fieldx ); 
        $tmpinits=array();
		$tmpinits["table"]   = "PROJ";
        $tmpinits["objname"] = " --------- ";
        $tmpinits["pos"]     = "1";
        $tmpinits["projlink"]= "1";
        $fieldx = array ("title" => "destination project", "name"  => "projx", "optional"=>"1",
                         "notes" => "[optional] copy objects to this project", "object" => "dblink",
                         "inits" => $tmpinits );
        $formobj->fieldOut( $fieldx );
		 
		

   

		$classarr = $varcol->get_class_nice_names($tablename);
		if ( $classarr!= NULL ) {
			$fieldx = array ("title" => "Extra class", "name"  => "classid", "optional"=>"1",
							"val" => $parx["classid"], "object" => "select", "inits"=> $classarr,
							"notes" => "an extra class parameter");
			$formobj->fieldOut( $fieldx );
        }

        $fieldx = array ("title" => "Like search", "name"  => "like","optional"=>"1",
                         "val" => $parx["like"],  "inits" => 1,
                         "notes" => "use the LIKE method, add wildcards", "object" => "checkbox" );
        $formobj->fieldOut( $fieldx );
        
        $fieldx = array ("title" => "Ignore case", "name"  => "ignoreCase","optional"=>"1",
                         "val" => $parx["ignoreCase"],  "inits" => 1,
                         "notes" => "ignore case?", "object" => "checkbox" );
        $formobj->fieldOut( $fieldx );
		
		$fieldx = array ("title" => "Parts in name", "name"  => "method","optional"=>"1",
                         "val" => $parx["method"],  "inits" => "dirty",
                         "notes" => "search for parts in names [<a href=\"#partsinname\">help</a>]", "object" => "checkbox" );
        $formobj->fieldOut( $fieldx );
        
		
        $fieldx = array ("title" => "Only unknown", "name"  => "shNotFound","optional"=>"1",
                         "val" => $parx["shNotFound"], "object" => "checkbox",
                         "notes" => "Show only unknown objects");
        $formobj->fieldOut( $fieldx );
		
		
		$tmparr = array ("0" =>"normal", "1" => "high" );
        $fieldx = array ("title" => "Infolevel", "name"  => "infolevel", "optional"=>"1",
                         "val" => $parx["infolevel"], "object" => "select", "inits"=> $tmparr,
                         "notes" => "level of information");
        $formobj->fieldOut( $fieldx );
                
        $formobj->close( TRUE );

} 

function formProjAsk(  ) {
    
    $parx = $this->parx;
         
	$initarr = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Remove all links of '".$this->tablename."' in project?";
	$initarr["submittitle"] = "YES";
	$initarr["tabwidth"]    = "AUTO";
	
	$hiddenarr = NULL;
	$hiddenarr["tablename"]    = $this->tablename;
	$formobj = new formc($initarr, $hiddenarr, 1);    

	$tmpparx = $parx;
	$tmpparx["projDelObj"] = 1;
	$formobj->addHiddenParx( $tmpparx );   
	$formobj->close( TRUE );
}

function formshow2EX( &$sql ) {
	global  $varcol;
    $parx = $this->parx;
         
	$initarr = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Define extra class parameters";
	$initarr["submittitle"] = "Next &gt&gt;";
	$initarr["tabwidth"]    = "AUTO";
	
	$hiddenarr = NULL;
	$hiddenarr["tablename"]    = $this->tablename;

	$formobj = new formc($initarr, $hiddenarr, 1);    
    $formobj->addHiddenParx( $parx ); 

	$classid = $parx['classid'];
	  
	$classParam = $varcol->get_attrib_nice_names($classid);

	$fieldx = array (
		"title" => "class parameter", "name"  => "classParam",
		"val" => $parx["classParam"], "object" => "select", "inits"=> $classParam,
		"notes" => "the extra class parameter");
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );
}

function getFile() {
	$FUNCNAME= 'getFile';

	$go = $this->go;	
	$fileName  = "searchfile.dat";
	$uploadObj = new uploadC();
	
	if ($this->parx['objstrings']!='') {
	    $this->info($FUNCNAME, '', 'take use objects from text field', 1);
	    $this->objTextLib = new gImpSeaObjFile($this->parx['objstrings'],'');
	    return;
	}

	if ( $go==1 ) {
		$this->info($FUNCNAME, 'upload file ...', '', 1);
		$fileFull  = $uploadObj->mvUpload2Tmp( $this->scriptid, $fileName, 
			$_FILES['userfile']['tmp_name'], $_FILES['userfile']['name'], $_FILES['userfile']['size']);
	} 

	if ( $go>1 ) {
		$fileFull  =  $uploadObj->getFileFromTmp(  $this->scriptid, $fileName );
		$this->info($FUNCNAME, 'take file from workdir ...', '', 1);
	}
	$this->fileFull = $fileFull;
	$this->objTextLib = new gImpSeaObjFile('',$this->fileFull);
}

/**
 * prepare project
 */
function _prepProj( &$sqlo, $sqlo2 ) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
	$tablename = $this->tablename;
	$parx      = $this->parx;
	$projid    = $parx["projx"];
	
	$o_rights = access_check($sqlo, 'PROJ', $projid);
	if ( !$o_rights["insert"]) {
		$error->set( $FUNCNAME, 1, 'You do not have write permission on this project.' );
		return;
	}

	$projname = obj_nice_name( $sqlo, "PROJ", $projid );
	$this->info("Destination project", "<a href=\"edit.tmpl.php?t=PROJ&id=".$projid."\">".$projname."</a>", "", 1);
	
	$numobjs = $this->projGetElemNum($sqlo, $projid, $tablename);
	if ( $numobjs ) {
		if (!$parx["projDelObj"]) {  // ask for delete 
			echo "<br>";
			$this->formProjAsk();
			htmlFoot(); 
		} else { 
			$this->info("Delete links", 'NOW', 'delete links of old objects in project');
			$this->_projObjDel( $sqlo, $sqlo2, $projid, $tablename );
		}
	} 
	$this->info("Add objects to project", "NOW");
    
}

function prepSearch( &$sqlo, $sqlo2 ) {
	global $varcol;
	global $error;
	$FUNCNAME= 'prepSearch';

	$tablename = $this->tablename;
	$parx      = $this->parx;

	if ( $parx["projx"] ) {
		$this->_prepProj( $sqlo, $sqlo2 );
	}

	$this->searchColumn = $parx['column'];
	if ( $parx['classid'] ) {
		$paramId = $parx['classParam'];
		$attrib_name = $varcol->attrib_id_to_name( $parx['classid'], $paramId);
		$mapcol      = $varcol->attrib_name_to_mapcol($attrib_name, $parx['classid']);
		
		if ($mapcol==NULL) {
			$error->set( $FUNCNAME, 1, 'No column found for class-ID "'.$parx['classid'].
				'" attribName: "'.$attrib_name.'"' );
			return;
		}
		$this->searchColumn = 'o.'.$mapcol;
	} 
	$this->info('searchColumn:', '', $this->searchColumn, 1);
	
	// ----------------

	if ( $tablename=="CONCRETE_SUBST" ) {
    	$this->info("INFO","show extra column with name of abstract_subst");
	} 
	if ($parx["like"]) {
		$this->info("Search operator", "LIKE");
	} 
	if ($parx["method"]=="dirty") {
		$this->info("Search method", "partially");
	} 
 

	if ( $parx['classid'] ) {
		$classname = $classname  = $varcol->class_id_to_nice_name( $parx['classid'] );
		$this->info("Class", $classname);

		$paramId = $parx['classParam'];
		$paramName = $varcol->attrib_id_to_name( $parx['classid'], $paramId);
		$this->info("Column", $paramName);
	} 
}


	

function mainLoop( &$sql ) {
	// check file 
	global $error;
	$FUNCNAME= 'mainLoop';

	$infox = NULL;
	$userfile  = $this->fileFull;
	$tablename = $this->tablename;
	$parx      = $this->parx;
	
	$lineCnt_ini = $this->objTextLib->countLines(); 
	if ($lineCnt_ini<0) {
	    $error->set( $FUNCNAME, 1, '' );
	    return;
	}
	echo 'Number of lines in file: '.$lineCnt_ini."<br />\n";
	
	
	
	$flushLib = new fProgressBar( $lineCnt_ini );
	$prgopt=array();
	$prgopt['objname']='rows';
  	$prgopt['maxnum'] = $lineCnt_ini;
  	$flushLib->shoPgroBar($prgopt);
  	
  	$this->objTextLib->openstream();
  	
  	
	
	echo "<table cellpadding=1 cellspacing=1 border=0>";
	echo "<tr bgcolor=#D0D0D0><th>Count</th><th>Info</th><th>ID</th><th>column in file</th><th>found NAME</th></tr>";
	$i = 0; 
	$sopt = NULL; 
	$s2opt = NULL;
	if ($parx["like"]) {
		$sopt["like"]  = 1;
		$sopt["wild"]  = 1; // add wild cards
		$s2opt["like"] = 1; // add wild cards
	}

	if ( $parx['classid']>0 ) {
		$sopt['isEXOB']  = 1; 
		$sopt['classid'] = $parx['classid']; 

		$s2opt['isEXOB']  = $sopt['isEXOB'];
		$s2opt['classid'] = $sopt['classid'];
	}
	
	if ($parx['ignoreCase']>0) {
		$sopt ['caseinsense'] = 1;
		$s2opt['caseinsense'] = 1;
	}
	
	$searchColumn = $this->searchColumn;
	$pk_name 	  = PrimNameGet2($tablename);  

	if ( $parx["projx"] ) {
		$projAddLib = new oProjAddElem( $sql, $parx["projx"] );   
	}

	while( $this->objTextLib->read_row() ) {

		$extracol = "";
		$line     = $this->objTextLib->line; 
		$loopShow = 1;
		
		if ($line=="") {
			$i++;
			continue;
		}
		$valarr = explode("\t",$line); 
		$objok      = 1;
		$objId      = "";
		$objError   = "";
		
		$objName = trim($valarr[0],"\"");
		do {
	
			if ( $objName == "" ) {
				$objok = -1;
				$objError = "Name is empty";
				break; 
			}
			
			if ( $parx["column"]=="ID" ) { // search for IDs
				list( $objId, $tmpFoundName ) = $this->getById( $sql, $tablename, $pk_name, $objName);
				if (!$objId) {
					$objError = "not found";
					$objok = -2;
					break;
				}
				
			} else {
				// search for name
				do {
					
					$objfound = $this->searchLib->redundancyTest( $sql, $tablename, $searchColumn, $objName, $sopt);
		
					if ($objfound==0) {
						$objError = "not found";
						$objok = -3;
						break;
					}
					if ($objfound>1) {
						$objError = $objfound." objects found";
						$objok = -4;
						break;
					}
		
					// only one object found ...
					list($tmpcnt, $objId, $tmpFoundName) = $this->searchLib->objGetByName( $sql, $tablename, $searchColumn, $objName, $s2opt );
					
					if (!$objId) {
						$objError = "object not found at objGetByName";
						$objok = -6;
						break;
					}
					
				} while (0);
				
				if ($objok<0 AND $parx["method"]=="dirty") { // try dirty
					list($tmpcnt, $objId, $tmpFoundName, $searchSingArr) = 
						$this->dirtySearch( $sql, $tablename, $searchColumn, $objName, $pk_name, $searchColumn, $infolevel);
					if ($tmpcnt!=1) {
						$objId    = "";
						$objError = "object not found";
						$objok = -5;
						break;
					} else {
						$objok = 0;
						$objError = "";
						
					}
				}
			}
			
			if ($objok<0) break; 
	
			if ( $parx["projx"] ) {
				$popt = NULL;
				$popt["order"] = $i+1;
				$projAddLib->addObj( $sql, $tablename, $objId, $popt ); 
			}
			
			if ( $tablename=="CONCRETE_SUBST" ) {
				$extracol = $this->getConcInfo($sql, $objId);
			}        
		} while (0);

		if ($error->Got(READONLY))  {
			$errLast   = $error->getLast();
			$error_txt = $errLast->text;
			$error_id  = $errLast->id;
			$error->reset();
			$objok=-100;
			$objError .= ' HardError:'.$error_txt;
		}

		if ($objok<0) {
			$objError = "<font color=red>Error:</font> ".$objError;
		}
		
		$objInfo = "";
		if ( $objError !="" ) {
			$objInfo = $objError; // " (".$objok.")";
			$tmpFoundName = "";
			if ($infolevel>0) {
				$tmpbr = "";
				if ( sizeof($searchSingArr) ) {
					foreach( $searchSingArr as $dummy=>$tmparr) {
						$tmpFoundName .= $tmpbr. $tmparr[0].": ".$tmparr[1];
						$tmpbr = "<br>";
					}
				}
			} 
		}
		
		if ( $parx["shNotFound"]>0 ) {
			if ($objok>=0) $loopShow = 0; // do not show good ones 
		}
			
		if ($loopShow) {
			echo "<tr bgcolor=#EFEFEF valign=top>";
			echo "<td>".($i+1)."</td>";
			echo "<td NOWRAP>".$objInfo."</td>";
			echo "<td>".$objId. "</td>";
			echo "<td>".$objName."</td>";
			echo "<td>".$tmpFoundName."</td>";
			if ($extracol!="")  echo "<td>".$extracol."</td>";
			echo "</tr>\n";
		} 
		if ($objok<0) $infox["notfound"] = $infox["notfound"] + 1;
		if ($objok<=-100) {
			break;
		} 
		
		$i++;
		$flushLib->alivePoint($i);
	} 
	
	
	echo "</table>\n";
	$this->objTextLib->close();
	echo "<br>";
	$flushLib->alivePoint($i,1);
	
	
	$infox["lines"] = $i;
	htmlInfoBox( "Statistics", "", "open", "INFO" );
	echo "<table cellpadding=1 cellspacing=1 border=0>";
	echo "<tr><td><font color=gray>Lines:</font></td><td><B>".$infox["lines"]."</B></td></tr>\n";
	echo "<tr><td><font color=gray>Found:</font></td><td><B>".($infox["lines"]-$infox["notfound"])."</B></td></tr>\n";
	echo "<tr><td><font color=gray>Not found:</font></td><td><B>".$infox["notfound"]."</B></td></tr>\n";               
	echo "</table>\n";
	htmlInfoBox( "", "", "close");

}

}

// --------------------------------------------

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2(  );
if ($error->printLast()) htmlFoot(); 
$varcol = & Varcols::get(); 

$tablename=$_REQUEST['tablename'];
$go=$_REQUEST['go'];
$parx=$_REQUEST['parx'];


$flushLib = new fProgressBar( 1000 );
$title = 'ExistenceCheck: check if objects in file exist in database';
$infoarr=array();
$infoarr["title"]    = $title;
$infoarr['scriptID'] = 'glob.objtab.impsea.php';
$infoarr["title_sh"] = "ExistenceCheck";
$infoarr["form_type"]= "list";
$infoarr['help_url'] = 'glob.objtab.impsea.html';
$infoarr["obj_name"] = $tablename;
$infoarr['css']      = $flushLib->getCss();
$infoarr['javascript'] = $flushLib->getJS(); 

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);


$mainlib = new gObjtabImpsea($tablename, $go, $parx, $infoarr['scriptID']);
$parx    = $mainlib->parx;

gHtmlMisc::func_hist( "glob.objtab.impsea", $title,  $_SERVER['PHP_SELF']."?tablename=".$tablename ); 
 
if ( $go>0 ) {
	echo "[<a href=\"".$_SERVER['PHP_SELF']."?tablename=$tablename\">New start</a>]<br>";
}
echo "<ul>";
 
  
if (!$go) {  
    $mainlib->formshow1( $sql, $tablename, $parx );
    $mainlib->endx();
    $pagelib->htmlFoot("<hr>");
}  

$mainlib->getFile();
$pagelib->chkErrStop();

if ( !$parx['classid'] and $parx['column']==NULL ) {
	$pagelib->htmlFoot('ERROR', 'Please choose "column" or "Extra class".');
}

if ($parx['classid'] and $parx['column'] ) {
	$pagelib->htmlFoot('ERROR', 'Please choose "Test column" OR "Extra class".');
}

if ( $go>0 and ($parx['classid']!=NULL) and ($parx['classParam']==NULL) ) {  
    $mainlib->formshow2EX( $sql );
    $mainlib->endx();
    $pagelib->htmlFoot("<hr>");
}  

$infolevel  = $parx["infolevel"];


$mainlib->prepSearch($sql, $sql2);
$pagelib->chkErrStop();

$mainlib->mainLoop( $sql );
$pagelib->chkErrStop();  


$pagelib->htmlFoot("<hr>");

