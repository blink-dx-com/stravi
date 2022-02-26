<?php
/**
 * compare selected objects
 * INCLUDES:  optional: phplib/o.TABLE.xcmp.inc (e.g. o.ABSTRACT_ARRAY.xcmp.inc)
 *  
 * @package glob.objtab.cmp.php
 * @swreq UREQ:0000397: f > object compare 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   
 *	$tablename 
	$parx["clevel"]  compare level
          0: only differences  (yes/no)
          1: flag of difference types (FEAT:1, feature: probe_on_array:0, ...)
          2: diff details
     $parx["showobj"] "" : all
           "1": only different ones 
     $parx["refid"]   id of reference object
	 $parx["showopt"] : 0|1
	 $parx["igas"][TABLE][COL] - ignore fields of special ASSOC-tables
	 
	 $parx["obj1"] : if this is given: ignore default SQL-selection; need also $parx["obj2"]
	 $parx["obj2"] : if this is given: ignore default SQL-selection; 
 */
session_start(); 


require_once ('reqnormal.inc');
require_once ("sql_query_dyn.inc");
require_once ("glob.objtab.cmp.inc");
require_once ("visufuncs.inc");

/**
 * table specific KEY-VALUE visualization
 * @author steffen
 *
 */
class fCmp_guiHelpTab {
    
    /**
     * 
     * @var array
     *   'vis'
     *   'fktab'
     *   'nice'
     */
    private $colNameCache;
    
	public function initTable(&$sqlo, $table) {
		
		$this->table = $table;
		$this->colNameCache = array();
		
		// analyse KEY-VALUEs, get columns
		$colarr = columns_get_pos($this->table);
		
		if (!sizeof($colarr)) {
			echo "Bizarre! No cols defined.<br />";
			return;
		}

		$this->colNameCache = array();
		reset ($colarr);
		foreach( $colarr as $dummy=>$colname) {
			$looparr = array();
			$oneColArr = colFeaturesGet   ( $sqlo,  $table, $colname);
			if ($oneColArr['CCT_TABLE_NAME']!=NULL) $looparr['fktab']=$oneColArr['CCT_TABLE_NAME'];
			// if ($oneColArr['VISIBLE']<1)  $looparr['vis'] = -1;
			if ($colname=='PASS_WORD') $looparr['vis'] = -1;
			$looparr['nice'] = $oneColArr['NICE_NAME'];
			
			$this->colNameCache[$colname] = $looparr;
		}
		
	}
	
	/**
	 * show feature values of one table
	 * @param $sqlo
	 * @param $tmparr KEY=>VAL of data column
	 */
	function featvalshow( &$sqlo, &$tmparr ) {
		$MAX_VAL_LEN = 30;
		
		if ($this->table==NULL) {
			sys_error_my('Class not initialized.');
		}
		
	    $infout   = "";
	    $tmpkomma = "";
	    reset  ($tmparr);
	    foreach( $tmparr as $key=>$val) { 
	    	
	        $col_feat = &$this->colNameCache[$key];
	        
	    	if ($col_feat['fktab']!=NULL and $val!==NULL) {
	    		$valout = obj_nice_name ( $sqlo, $col_feat['fktab'], $val ). ' [ID:'.$val.']';
	    	} else {
	    		if (strlen($val)> $MAX_VAL_LEN) $val=substr($val,0,$MAX_VAL_LEN)."...";
	    		$valout = htmlentities($val);
	    	}
	    	if ($col_feat['vis']<0 ) {
	    		$valout = '???';
	    	}
	    	
	    	$colname = $col_feat['nice'];
	    	
	    	$infout .= $tmpkomma . '<span style="color:gray;">'.$colname.'=</span>'.$valout;
	        $tmpkomma = "<br> ";
	    } 
	    reset  ($tmparr);
	    return ($infout);
	} 
	
}

class fCmp_guiHelp {
	
	public function __construct() {
		$this->tabHelpLib = new fCmp_guiHelpTab();
	}

	function getObjName(&$sql, $tablename, $important_name, $primary_key, $tmpid) {
		
		$sql->query("SELECT x.".$important_name."  FROM ".$tablename." x where x.".$primary_key."=".$tmpid); 
		$sql->ReadRow();
	    $tmpName = $sql->RowData[0];
	    // if ( $tmpName=="" )  
		$tmpName .=" [".$tmpid."]";
		return ($tmpName); 
	}
	
	function  showOpt( &$sql, &$compObj, $tablename, &$parx) {
		$assocTabs = $compObj->assocs;
		$assocIgnored  = &$parx["igas"];
		echo "<br>";
		if ( !is_array($assocTabs) ) {
			htmlInfoBox( "Info: advanced options", "This object type contains no feature lists", "", "CALM" );
			return;
		}
		echo "<font color=gray><b>Advanced: Ignore following columns of feature lists:</b></font><br>\n";
		foreach( $assocTabs as $key=>$valarr) {
			$assocTabNow = $valarr[0];
			$assocNice   = $valarr[1];
			
			$tabobj = new visufuncs();
			$headOpt = array( "title" => "feature list: $assocNice",);
			$headx  = array ("Ignore", "Column");
			$tabobj->table_head($headx,   $headOpt);
			
			$nowColArr = columns_get2($assocTabNow);
			if (!is_array($nowColArr)) continue;
			foreach( $nowColArr as $key=>$nowCol) {
				$colfeat = colFeaturesGet( $sql, $assocTabNow, $nowCol);
				$colNice = $colfeat["NICE_NAME"];
				$isPK    = $colfeat["PRIMARY_KEY"];
				if ($isPK>0) continue;
				$tmpsel = "";
				if ($assocIgnored[$assocTabNow][$nowCol]>0) $tmpsel="checked";
				$dataArr = array("<input type=checkbox name=\"parx[igas][".$assocTabNow."][".$nowCol."]\" value=1 $tmpsel>", $colNice);
				$tabobj->table_row ($dataArr);
			}
			
			$tabobj->table_close();
			echo "<br>\n";
			
		}
		reset ($assocTabs); 
		echo "<input type=hidden name=\"parx[showopt]\" value=1>\n";
	
	}
	
	/**
	 * show difference of features of two object
	 * @param  $tablename
	 * @param  $tmpName
	 * @param  $tmpid
	 * @param  $infoout1
	 * @param  $infoFeat
	 * @param  $diffAssoc
	 * @param  $tabRowColor
	 * @param  $isRef
	 */
	function thisRowOut(&$sqlo, $tablename, $tmpName, $tmpid, $infoout1, &$infoFeat, &$diffAssoc,
		$tabRowColor, 
		$isRef // 0|1
		) {
			
		$this->tabHelpLib->initTable($sqlo, $tablename);
	
		if (sizeof($infoFeat)) {
			$infoout2 = $this->tabHelpLib->featvalshow( $sqlo, $infoFeat );
			/*
	        $infoout2 = "<font size=-1>";
	        $tmpspace = "";
	        foreach( $infoFeat as $key=>$val) {
	            if (strlen($val)> $MAX_VAL_LEN) $val=substr($val,0,$MAX_VAL_LEN)."...";
	            $infoout2 .= $tmpspace ."<font color=gray>". $key."</font>:".$val;
	            $tmpspace = ", ";
	        }
	        $infoout2 .= "</font>";
	        */
	    } 
	    
	    if (sizeof($diffAssoc)) {
	        $infoout3 = "<font size=-1>";
	        $tmpspace = "";
	        foreach( $diffAssoc as $tmptab=>$tmpcnt) {
	            $infoout3 .= $tmpspace ."<font color=gray>". $tmptab."</font>:".$tmpcnt;
	            $tmpspace = ", ";
	        }
	        reset($diffAssoc);
	        $infoout3 .= "</font>";
	    }
		$tmpSel = "";
		if ($isRef) $tmpSel = "checked";
	    
	    echo "<tr bgcolor=".$tabRowColor."><td>";
		echo "<input type=radio name=parx[refid] value=\"".$tmpid."\" ".$tmpSel.">";
		echo "</td><td><a href=\"edit.tmpl.php?t=".$tablename."&id=".$tmpid."\">".htmlspecialchars($tmpName)."</a></td>";
	    echo "<td>".$infoout1."</td>";
	    echo "<td>".$infoout2."</td>"; 
	    echo "<td>".$infoout3."</td>";
	    echo "</tr>\n";
	}
	
	function info ($key, $text, $notes=NULL ) {
	    // FUNCTION: print out info text
	    if ($notes!="")  $notes = " &nbsp;<I>".$notes."</I>";
	    echo "<font color=gray>".$key.":</font> <B>".$text."</B>".$notes."<br>\n";
	
	}

	public function initAssocTable( &$sqlo, $table ) {
	    $this->tabHelpLib->initTable($sqlo, $table);
	}
	public function featvalshow( &$sqlo, &$tmparr ) {
	    return $this->tabHelpLib->featvalshow($sqlo, $tmparr);
	} 
	
	function colLegend($tmptable) { 
	    echo "<font color=gray><B>Legend for column names</B></font><br>\n";
	    $colarr = columns_get2($tmptable); 
	    $pks    = primary_keys_get2($tmptable);
	    
	    if (!sizeof($colarr)) {
	    	echo ' no column definitions found.<br>';
	    	return;
	    }
	    
	    $infarr = array();
	    
	    foreach( $colarr as $dummy=>$tmpcol) {
	        if ($pks[0]!=$tmpcol AND $pks[1]!=$tmpcol) {  // no primary keys
	            $nicename = columnname_nice2($tmptable, $tmpcol ); 
	            $infarr[] = array( substr($tmpcol,0,3), $nicename );
	        }
	    }
	    $header = array("alias", "nice name");
	    visufuncs::table_out( $header, $infarr ); 
	}

}

// ---------
/**
 * compare two objects
 * @author steffen
 *
 */
class fCmp_gui2 {
	
	public function __construct(&$sqlo, &$compObj, $tablename) {
		$this->tablename = $tablename;
		$this->helpLib = new fCmp_guiHelp();
		$this->compObj = &$compObj;
		
		$this->extraCmpObj = NULL;
		$specialReqFile = $_SESSION['s_sessVars']['AppLibDir'] . DIRECTORY_SEPARATOR ."o.".$tablename.".xcmp.inc";
		if (file_exists($specialReqFile)) {
			require_once($specialReqFile);
			$this->extraCmpObj = new gCompareObjC();
		}
	}
	
	/**
	 * show DETAILED difference of associated elements
	 * @param  $sql
	 * @param  $sql2
	 * @param int $obj1 reference object
	 * @param int $obj2 second object
	 * @param int $assocDiffCnt number of ASSOC differences
	 */
	public function doit(&$sql, &$sql2, $obj1, $obj2, &$retarr) {

		$tablename = $this->tablename; 
		$helpLib   = &$this->helpLib;
		$compObj   = &$this->compObj;
		
		$refobjid  = $obj1;
		$obnames=array();
		$obnames[1] = obj_nice_name($sql, $tablename, $obj1);
	    $obnames[2] = obj_nice_name($sql, $tablename, $obj2);
		
		
	    if ( $this->extraCmpObj!=NULL ) {
			$helpLib->info("Special object compare for this object type","");  
			$result = $this->extraCmpObj->compare( $sql, $obj1, $obj2 );
			if ($result["diffcnt"]>0) {
				
				$tabobj  = new visufuncs();
				$headOpt = array( "title" => "Special object compare: ". $result["infotxt"]);
				$tmpIntersetCol = $this->extraCmpObj->importantCol;
				$headx   = array ( $tmpIntersetCol, $obnames[1], $obnames[2]);
				$tabobj->table_head($headx,   $headOpt);
				$cnt=0;
				while ( ($outarr = $this->extraCmpObj->visuDataOne($sql)) != NULL ) {
					$tabobj->table_row ($outarr);
					if ($cnt>30) break;
					$cnt++;
				
				}
				$tabobj->table_close();
				echo "<br>\n";
			} else {
				echo "Special object compare: ". $result["infotxt"].": <b>No differences detected</b><br>";
			}
			echo "<br>\n";
		}
	
		/**
		 * array of TABLEs => array( STEP=>array(diffvalues) )
		 *   - diffvalues = array(KEY=>VAL)
		 * @var $assocDetails
		 */
		$assocDetails= &$retarr["assoc"]["diffDetail"];
		
		/**
		 * array of TABLEs => LONG (diff counts)
		 * @var $diffAssoc
		 */
		$diffAssoc   = &$retarr["assoc"]["diffCont"]; 
		

		// glob_printr( $assocDetails, "assocDetails info" );
			
		
	    if ( !sizeof($assocDetails) ) {  
	    
	         $helpLib->info("No info about difference details","");  
	         
	    } else {
	        
	        $refAssoc = $compObj->getAssocInfo( $sql, $refobjid, 3 ); // values of REF 
	        $obxAssoc = $compObj->getAssocInfo( $sql, $obj2,     3 ); // values of OBJX
	        
	        foreach( $assocDetails as $tmptable=>$tmparr) { 
	        
	        	/**
				 * array of ( STEP => array(key=>val) )
	        	 */
	            $refAssocVals = &$refAssoc["featVals"][$tmptable];
	            $obxAssocVals = &$obxAssoc["featVals"][$tmptable];
	            
	            if ( !$diffAssoc[$tmptable] ) continue; //  no difference for this ASSOC_TABLE ...
	            
	            $helpLib->initAssocTable( $sql, $tmptable );
	            
		        if ( $_SESSION["userGlob"]["g.debugLevel"]>2 ) {
					// glob_printr( $assocDetails, "assocDetails info" );
					glob_printr( $tmparr, "DEBUG&gt;2: retarr[assoc][diffDetail][$tmptable] info" ); 
					glob_printr( $refAssocVals, "DEBUG&gt;2: refAssocVals[$tmptable] info" ); 
				}
	            
				
	            $helpLib->info("Details of Associated elements:",$tmptable); // OLD: "(only first table will be analyzed)"
	            echo "<table cellpadding=1 cellspacing=1 border=0 bgcolor=#D0D0D0>\n";
	            echo "<tr bgcolor=#DFDFFF>";
	            echo "<th>pos</th>";
	            echo "<th>". $obnames[1]."</th>";
	            echo "<th>". $obnames[2]."</th>";
	            echo "</tr>\n";
	            $cntshow = 0;  
	            if (sizeof($tmparr)) {
	             
	            	// $loopStepID : ID of second PK; e.g. for ABSTRACT_PROTO_STEP: STEP_NR
	                foreach( $tmparr as $loopStepID=>$tmpi) {
	                	
	                	/**
	                	 * $tmpi :
	                	 * "1"; the OBJX=not exists OBJREF=exists
   	  					 * "0"; the OBJX=exists,    OBJREF=not exists
	                	 *  array() the diff vals
	                	 */
	                    $inf1="?";
	                    $inf2="?";
						
	                    do {
	                    	if (is_array($tmpi)) {
	                    		$inf1     = "val";
		                        if (is_array($refAssocVals[$loopStepID])) {
		                            $inf1 = $helpLib->featvalshow( $sql, $refAssocVals[$loopStepID] );
		                        }
		                        $inf2 = $helpLib->featvalshow( $sql, $tmpi ); 
		                        break;
	                    	}
	                    	
	                   		if ($tmpi==="val") {
	                    		$inf1="val";
	                        	$inf2="val";
	                    		break;	
	                    	}
	                    	
	                    	if ($tmpi==0) {
	                    		// OBJX=exists,    OBJREF=not exists
	                    		$inf1="NOT exists";
	                    		$inf2="exists";
	                    		if (is_array($obxAssocVals[$loopStepID])) {
		                            $inf2 = $helpLib->featvalshow( $sql, $obxAssocVals[$loopStepID] );
		                        } 
		                        
	                    		break;
	                    	}
	                    	if ($tmpi==1) {
	                    		// the OBJX=not exists OBJREF=exists
	                    		$inf1="exists";
	                    		$inf2="NOT exists";
	                    		if (is_array($refAssocVals[$loopStepID])) {
		                            $inf1 = $helpLib->featvalshow( $sql, $refAssocVals[$loopStepID] );
		                        } 
	                    		break;
	                    	}
	                    	
	                    } while (0);
	
	                   
	                    echo "<tr bgcolor=".$tabRowColor.">";
	                    echo "<td>$loopStepID</td>";
	                    echo "<td>$inf1</td>";
	                    echo "<td>$inf2</td>";
	                    echo "</tr>";
	                    $cntshow++;
	                }
	                if ( $diffAssoc[$tmptable] > $cntshow ) {
	                    echo "<tr bgcolor=".$tabRowColor.">";
	                    echo "<td colspan=3>... ".($diffAssoc[$tmptable]- $cntshow)." more</td></tr>\n";
	                }
	            }
	
	            echo "</table>\n\n<ul>";
				echo "<br>";
	            $helpLib->colLegend($tmptable);
	            echo "</UL><br>\n";
	            
	        }
	    } 
	}
}

$MAX_ALLOW_ELEMS = 100;


$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2();
if ($error->printLast()) htmlFoot(); 

$tablename = $_REQUEST['tablename'];
$parx	   = $_REQUEST['parx'];

$tablenice = tablename_nice2($tablename); 
$title     = "Basic object compare: ".$tablenice;
$infoarr=array();
$infoarr['help_url'] = "glob.objtab.cmp.html";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;  
$infoarr["obj_cnt"]  = 1;          // show number of objects

// gives back the number of objects
$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sql, $infoarr);
echo "<UL>\n";

// ----------------------------------------------------------------
// SPECIALS for LIST SELECTION
// ----------------------------------------------------------------

$helpLib = new fCmp_guiHelp();

$primary_key    = PrimNameGet2($tablename);
$important_name = importantNameGet2($tablename); 

if (countPrimaryKeys($tablename)>1) {
    htmlFoot("Error", "Works only for tables with ONE primary key!");
}

if ( $parx["obj1"]!=NULL and $parx["obj2"]!=NULL ) {
	$sqlAfter   = $tablename.' x where x.'.$primary_key. ' in ('.$parx["obj1"].', '.$parx["obj2"].') order by x.'.$primary_key;
	$numObjects = 2;
	echo '... compare two objects ...<br>';
} else {
	$sqlAfter   = get_selection_as_sql( $tablename, array("order"=>1) );
	$numObjects = $headarr["obj_cnt"];
}

if ($numObjects <= 0) {
    htmlFoot("Attention", "Please go back and select elements of ".$tablenice." from the list!");
}  

if ($numObjects > $MAX_ALLOW_ELEMS) {
     htmlFoot("Error","Only less than ".$MAX_ALLOW_ELEMS." objects allowed");
} 

$refobjid = $parx["refid"];


gHtmlMisc::func_hist("glob.objtab.cmp", $title,  $_SERVER['PHP_SELF']."?tablename=".$tablename ); 

$tmpnote = "";
if ($numObjects==2) $parx["clevel"] = 3; // increase level
else {
	$tmpnote = " (select TWO objects to show more details)";
}
$compObj = new objCompc($sql, $tablename, $parx);       

$helpLib->info("Info-level",$parx["clevel"], $tmpnote);
if ($parx["showopt"] ) $helpLib->info("Advanced Options","yes", "");
if (is_array($parx["igas"])) {
	echo "<font color=gray>Ignore feature list columns:</font> ";
	print_r($parx["igas"]);
	echo "<br>";
} 

echo "<br>\n";

echo "<form method=\"post\"  name=\"editform\"  action=\"".$_SERVER['PHP_SELF']."?tablename=".$tablename."\" >\n";

echo "<table cellpadding=1 cellspacing=1 border=0 bgcolor=#D0D0D0>\n";
echo "<tr bgcolor=#DFDFFF>";
echo "<th>Reference</th>";
echo "<th>object</th>";
echo "<th></th>";
echo "<th>features</th>";
echo "<th>associated elements</th>";
echo "</tr>\n";

$sql2->query("SELECT x.".$primary_key."  FROM $sqlAfter"); 
$sql2->ReadRow();
$objFirstID   = $sql2->RowData[0];
if (!$refobjid) {
	$refobjid = $objFirstID;
}

if ($refobjid) {
	$tmpid = $refobjid;
	$infoout1 = "";
	$infoFeat    = $compObj->getFeat     ( $sql, $tmpid );
	$diffInfoTmp = $compObj->getAssocInfo( $sql, $tmpid );
	$diffAssoc   = &$diffInfoTmp["cnt"];
	$tabRowColor="EFEFD0";
	$tmpName = $helpLib->getObjName($sql, $tablename, $important_name, $primary_key, $tmpid);
	$helpLib->thisRowOut($sql, $tablename, $tmpName, $tmpid, $infoout1, $infoFeat, $diffAssoc, $tabRowColor, 1);
    
}



echo "<tr bgcolor=#DFDFFF>";
echo "<th>&nbsp;</th>";
echo "<th>object</th>";
echo "<th>diffcount</th>";
echo "<th>feature differences</th>";
echo "<th>differences in associated elements</th>";
echo "</tr>\n";


$sql2->query("SELECT x.".$primary_key."  FROM $sqlAfter"); 
$rowcnt    = 0; 
$diffAssoc = NULL;
while ( $sql2->ReadRow() ) { 
    $infoout1 = "";
    
    $infoFeat = NULL;
    
    
    $tmpid   = $sql2->RowData[0];
	
	if ( $refobjid == $tmpid )  continue; // was evaluated before ... 
	
	$currentID = $tmpid;
	$tmpName = $helpLib->getObjName($sql, $tablename, $important_name, $primary_key, $currentID);
     
    
	// echo object 
	$tabRowColor = "#EFEFEF";
	$retarr   = $compObj->compare( $sql, $refobjid, $currentID );  
	$infoout1 = $retarr["sum"];   
	$infoFeat = &$retarr["features"];
	$diffAssoc= &$retarr["assoc"]["diffCont"]; 
         		
    $helpLib->thisRowOut($sql, $tablename, $tmpName, $currentID, $infoout1, $infoFeat, $diffAssoc, $tabRowColor, 0);
	
	$rowcnt++;
    
} 
echo "</table>\n\n"; 


if ($parx["showopt"]) $helpLib->showOpt($sql, $compObj, $tablename, $parx);



echo "<input type=submit value=\"Start new comparison\" class=\"yButton\">\n";
if (!$parx["showopt"]) echo "&nbsp;<b>[<a href=\"".$_SERVER['PHP_SELF']."?tablename=".$tablename."&parx[showopt]=1\">Advanced Options</a>]</b>";
if ( $parx["obj1"]!=NULL and $parx["obj2"]!=NULL ) {
	echo '<input type=hidden name="parx[obj1]" value="'.$parx["obj1"].'">'."\n";
	echo '<input type=hidden name="parx[obj2]" value="'.$parx["obj2"].'">'."\n";
}

echo "</form><br>\n";

if ($numObjects==2) {

	$mainLib = new fCmp_gui2($sql, $compObj, $tablename);
	$mainLib->doit($sql, $sql2, $refobjid, $currentID, $retarr);
}



htmlFoot('<hr>');

