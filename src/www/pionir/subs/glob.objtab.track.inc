<?php
/**
 * - functions for object tracking
   - functions for "used by objects"
 * @package glob.objtab.track.inc
 * @swreq   UREQ:0001242: g > object tracking > list of objects 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

require_once("sql_query_dyn.inc");
require_once ( "javascript.inc" );

/**
 * - functions for object tracking
   - functions for "used by objects"
 * @swreq   UREQ:0001045: g > object tracking > HOME 
 */
class gObjTrackSub2 {
	
	function __construct() {
		$this->tabarr=NULL;
		$this->tabArrAssoc=NULL;
		$this->refTabsCount=0;
		$this->last_table   = "";
	}
		
	/**
     * analyse one foreign-key
	 */
	function foreignElement(&$RowData, $allForeigns=0) {
		
		$tmpRefCnt	     = 0;
		$tabName_tmp     = $RowData[0];
		$tabNameNice_tmp = $RowData[1];
	    $pk_tmp          = $RowData[2]; 
	    $colname         = $RowData[3]; 
		$tabOtherPks	 = countPrimaryKeys($tabName_tmp);
		
	    if ($this->last_table == $tabName_tmp) {  // a new table
	        // is a table with TWO references to the DESTINATION table
	        // e.g. A_SUBST_COHER ...
	       $tmpRefCnt   =1; // increment REF-Counter
	       
	       // count one back to store only one FK
	       if (!$allForeigns) $this->refTabsCount--;
	    } 
	    
		if ( $tabOtherPks>1 AND $pk_tmp==1 ) {
			// assoc tables
			$this->tabArrAssoc[$this->refTabsCount] = array( 
				"t"=>$tabName_tmp, "nice"=>$tabNameNice_tmp,
				"pk"=> $pk_tmp, "ref"=>$tmpRefCnt, 'col'=>$colname
				 );
	    	
	    } else {
			$this->tabarr[$this->refTabsCount] = array( 
				"t"=>$tabName_tmp, "nice"=>$tabNameNice_tmp,
				"pk"=> $pk_tmp, "ref"=>$tmpRefCnt, 'col'=>$colname
				  );
		}   
		$this->last_table  = $tabName_tmp;
		$this->refTabsCount++;
	}

}


/**
 * - functions for object tracking
   - functions for "used by objects"
 * @swreq   UREQ:0001045: g > object tracking > HOME 
 */
class gObjTrackSubGui {

var $search_table;
var $fk_directCnt; // counter for javascript-array-index

function __construct($tablename, $parx) {
	$this->tablename = $tablename; 
	$this->parx = $parx;
	
	$this->foreignLib = new gObjTrackSub2();
	
	$this->sqlAfter  = get_selection_as_sql( $tablename);
	$this->mostImpCol  = PrimNameGet2($tablename);
	$this->sqlInSel = " IN (select ".$this->mostImpCol." from ". $this->sqlAfter . ")";
 	$this->selcond  = "x.". $this->mostImpCol.$this->sqlInSel;

	$this->fk_directCnt = 0;
}

function getSecondFk( &$sql, 
    // try to get the second foreign key
    $table,  // e.g. CONCRETE_SUBST
    $fktable,// e.g. C_SUBST_COHER
    $pkName  // the normal foreign key
    ) { 
    // get second foreign key in DESTINATION table
    
    $sqls = "SELECT x.COLUMN_NAME FROM cct_column x ".
	        "WHERE x.table_name ='".$fktable."' AND x.cct_table_name ='".$table."' AND x.COLUMN_NAME!='".$pkName."'";
            
    $sql->query("$sqls"); 
    $sql->ReadRow();
	$newFkName  = $sql->RowData[0];
    return ($newFkName);
} 

function getFkName( &$sql, 
    // try to get the second foreign key
    $table,  // e.g. CONCRETE_SUBST (get column-name for this fk-table)
    $reftable // e.g. C_SUBST_COHER
    ) { 
    // get second foreign key in DESTINATION table
    
    $sqls = "SELECT x.COLUMN_NAME FROM cct_column x ".
	        "WHERE x.table_name ='".$reftable."' AND x.cct_table_name ='".$table."'";
       
    $sql->query($sqls); 
    $sql->ReadRow();
	$newFkName  = $sql->RowData[0];
    return ($newFkName);
} 

function rowout( $text, $url, $icon, $sqlcondNew ) {
 	echo "<tr>";
	echo "\n";
	echo '<script language="JavaScript">'."\n";
	echo 'varx['.$this->fk_directCnt.'] = "'.$sqlcondNew.'";'."\n";
	echo '</script>'."\n";
	$this->fk_directCnt++;
	
	echo "<td valign=top>";         
	echo "<a href=\"".$url."\">";
	echo "<img src=\"images/$icon\" border=0> ";
	echo $text."</a></td></tr>\n";
}

/**
 * produce special tracking conditions
 * @param object $sql
 * @param array $inarr
 *  $inarr["fk_table"] 	   = "EXP";
	$inarr["pkname"]       = "EXP_ID";
	$inarr["text"] 		   = "Experiments use samples";
	-- OPT 1 --
	$inarr["bindTable"]    = "SAMPLE_IN_EXP";
	$inarr["bindCol"]      = [default: "pkname" ]  EXP_ID -- the binding-column
	$inarr["bindCol2"]     =  SAMPLE_CONCRETE_SUBST_ID
	-- OPT 2 --
	$inarr["sql_special"] = "(select CONCRETE_SUBST_ID from CONCRETE_PROTO_STEP where CONCRETE_PROTO_ID in (".
			 	"select CONCRETE_PROTO_ID from EXP_HAS_PROTO where x.EXP_ID ". $this->sqlInSel .
			   "))";
 */
function _specialTrack( &$sql, $inarr ) {
	
	// $tablename = $this->tablename;
	// $newFkName  = $this->getFkName( $sql, $tablename, $inarr["bindTable"] );
	if ($inarr["sql_special"]!=NULL) {
	    $sqlcond    = "x.". $inarr["pkname"]." IN " . $inarr["sql_special"];
	} else {
    	$sqlcond    = "x.". $inarr["pkname"]." IN (select x.".$inarr["bindCol"].
    				  " from ". $inarr["bindTable"] ." x  where x.".$inarr["bindCol2"].$this->sqlInSel .")";
	}
	$icon = "icon.".$inarr["fk_table"].".gif";
	$url = "javascript:this_gotab2('".$inarr["fk_table"]."', '".$this->fk_directCnt."')";
	$this->rowout( $inarr["text"], $url, $icon, $sqlcond );
}

function specialUsed( &$sql ) {
	// FUNCTION: special tables
	$tablename = $this->tablename;
	
	switch ( $tablename ) {
		case "EXP":
			
			// show sample of exp
			$inarr = NULL;		
			$inarr["fk_table"] 	  = "CONCRETE_SUBST";
			$inarr["pkname"]      = "CONCRETE_SUBST_ID";
			$inarr["bindTable"]   = "SAMPLE_IN_EXP";
			$inarr["bindCol"]     = "SAMPLE_CONCRETE_SUBST_ID";
			$inarr["bindCol2"]    = "EXP_ID";
			$inarr["text"] 		  = "Samples of experiment";
			
			$this->_specialTrack( $sql, $inarr );
			
			// show material of exp
			$inarr = NULL;
			$inarr["fk_table"] 	  = "CONCRETE_SUBST";
			$inarr["pkname"]      = "CONCRETE_SUBST_ID";
			$inarr["sql_special"] = "(select CONCRETE_SUBST_ID from CONCRETE_PROTO_STEP where CONCRETE_PROTO_ID in (".
			 			"select CONCRETE_PROTO_ID from EXP_HAS_PROTO where EXP_ID ". $this->sqlInSel .
			   "))";
			$inarr["text"] 		  = "Material of experiment";
			
			$this->_specialTrack( $sql, $inarr );
			
			break;
	}
	
	switch ( $tablename ) {
		case "CONCRETE_SUBST":
			
			// show sample of exp
			$inarr = NULL;
			$inarr["fk_table"] 	   = "EXP";
			$inarr["pkname"]       = "EXP_ID";
			$inarr["bindTable"]    = "SAMPLE_IN_EXP";
			$inarr["bindCol"]      = "EXP_ID";
			$inarr["bindCol2"]     = "SAMPLE_CONCRETE_SUBST_ID";
			$inarr["text"] 		   = "Experiments use samples";
			
			$this->_specialTrack( $sql, $inarr );
			
			// old VIEW 
// 			$inarr = NULL;
// 			$inarr["fk_table"] 	   = "CONCRETE_SUBST";
// 			$inarr["pkname"]       = "CONCRETE_SUBST_ID";
// 			$inarr["bindTable"]    = "SAMPLE_IN_CONCRETE_SUBST";
// 			$inarr["bindCol"]      = "PRODUCT_CONCRETE_SUBST_ID";
// 			$inarr["bindCol2"]     = "SAMPLE_CONCRETE_SUBST_ID";
// 			$inarr["text"] 		   = "Substances uses as samples";
			
			$this->_specialTrack( $sql, $inarr );
			
			break;
	}
}

function _trackShowStart() {
	$selcondUrl =  $this->selcond;
	  
	echo '<input type="hidden" name="tableSCond" value="'.$selcondUrl.'">'."\n";
	echo '<input type="hidden" name="t"    value="">'."\n"; 
	echo "</form>\n";
	
}

/**
 * show tracking links of a table-MULTI-column

 */
function _trackShowMultiCol( &$sql, &$tabarr, $destTable) {
	
	$tablename = $this->tablename;
	
	$icon="images/icon.".$destTable.".gif";
	if ( !file_exists($icon) ) $icon="images/icon.UNKNOWN.gif";
	echo "<img src=\"$icon\" border=0> <b>".tablename_nice2 ($destTable) ."</b>";
	echo ' &nbsp;<font color=gray>(search only in this table)</font>';
	echo "<br>";
	
	echo '<ul>';
	echo "<table border=0 cellpadding=0 cellspacing=0>"."\n";
	echo "\n";
	
	foreach( $tabarr as $idx=>$rowarr) {
	
		$forTable     = $rowarr["t"];
		$forTableNice = $rowarr["nice"];
		$pk_tmp 	  = $rowarr["pk"];
		$tmpRefCnt    = $rowarr["ref"];
		$colname	  = $rowarr['col'];
		
		$icon="images/icon.".$tablename.".gif";
		if ( !file_exists($icon) ) $icon="images/icon.UNKNOWN.gif";
		
		$niceCol = columnname_nice2($forTable, $colname);
		
		echo "<tr><td valign=top>";         
		echo "<a href=\"javascript:form2_gotab('".$forTable."', '".$colname."')\">";
		echo "<img src=\"$icon\" border=0> ";
		echo $niceCol."</a>"; 
		echo "</td>";
		echo "</tr>\n";
		
	}
	
	echo " </table>\n";
	echo '</ul><br />'."\n";
}

/**
 * analyse extra tables
 * @param $sqlo
 * @param $go 
 * 	0: show form
 *  1: doi forward
 */
function _extraTables(&$sqlo, $go, $parx) {
	$tablename = $this->tablename;
	switch ($tablename) {
		case 'WORKLIST':
			require_once('objlib/WORKLIST.trackList.inc');
			$helpTrackLib = new oWORKLIST_listTrack($this->selcond, $parx);
			if (!$go) {
				$helpTrackLib->showForm($sqlo);
				
			} else {
				$helpTrackLib->goSearch($sqlo);
			}
	}
}
	

/**
 * show tracking links
 * @param $optkey=NULL  "showExtra" 
 */
function _trackShow( &$sql, $tabarr, $optkey=NULL	) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$tablename = $this->tablename;
	 
	echo "<table border=0 cellpadding=0 cellspacing=0>"."\n";
	$optioncnt    = 1;
	 
	$last_table   = ""; // remember the last table
	echo "\n";
	
	if ( sizeof($tabarr) ) {
		foreach( $tabarr as $idx=>$rowarr) {
		
			$forTable     = $rowarr["t"];
			$forTableNice = $rowarr["nice"];
			$pk_tmp 	  = $rowarr["pk"];
			$tmpRefCnt    = $rowarr["ref"];
			$colname	  = $rowarr['col'];
			
			$icon="images/icon.".$forTable.".gif";
			if ( !file_exists($icon) ) $icon="images/icon.UNKNOWN.gif";
			
			echo "<tr><td valign=top>";         
			// echo "<a href=\"javascript:this_gotab('".$forTable."')\">";
			echo "<a href=\"javascript:form2_gotab('".$forTable."')\">";
			echo "<img src=\"$icon\" border=0> ";
			echo "$forTableNice</a>"; 
			
			if ($tmpRefCnt) {   
				// is a table with TWO references to the DESTINATION table
				// e.g. A_SUBST_COHER ... 
				$newFkName  = $this->getSecondFk( $sql, $tablename, $forTable, $this->mostImpCol);
				if ($newFkName!="") {	
					echo '&nbsp;&nbsp;[<a href="'.$_SERVER['PHP_SELF'].'?tablename='.$tablename.
						'&parx[destTab]='.$forTable.'&parx[foreignOpt]=1'.
						'">MORE columns ...</a>]'."\n";
					// echo " <a href=\"javascript:form2_gotab('".$forTable."', '".$newFkName."')\">";
					// echo "<font color=gray>[second]</font></a>";
				}
			}
			echo "</td>";
			
			if ($pk_tmp>0) {  // if the object is a FOREIGN-KEY 
			
				$tdTagPrinted = FALSE;
				echo "\n";    
				$pkname_arr = primary_keys_get2($forTable);
				
				foreach( $pkname_arr as $dummy=>$pkname) { 
				
					$fk_table = fk_check2( $pkname, $forTable, $ftab_prim_name, $ftab_imp_name );  
		
					if ( $fk_table!="" AND $fk_table!=$this->search_table ) {   // only if root not equal 
						$ft_nice  = tablename_nice2($fk_table);
		
						$selcond2 = "x.". $pkname." IN (select ".$pkname." from ". $forTable ." x  where ".$this->selcond.")";
						echo "\n";
						echo '<script language="JavaScript">'."\n";
						echo 'varx['.$this->fk_directCnt.'] = "'.$selcond2.'";'."\n";
						echo '</script>'."\n";
						
						$icon="images/icon.".$fk_table.".gif"; 
						if ( !file_exists($icon) ) $icon="images/icon.UNKNOWN.gif";
						
						if ( !$tdTagPrinted ) {
							echo "<td valign=top>&nbsp; ==&gt; &nbsp;</td>";  
							echo "<td valign=top>";
							$tdTagPrinted = TRUE;
						}
									
						echo "<a href=\"javascript:this_gotab2('".$fk_table."', '".$this->fk_directCnt."')\">";
						echo "<img src=\"$icon\" border=0> ";
						echo "$ft_nice</a> &nbsp;";
						
						$this->fk_directCnt++;
					} 
				}
				if ( $tdTagPrinted ) echo "</td>";
			} // else echo "<td>&nbsp;</td>";
			
			echo "</tr>\n";
		}  
	}
	
	if ( $optkey == "showExtra") $this->specialUsed($sql);
	
	echo " </table>\n";
	
	// only for assoc tables
	if ( $optkey == NULL) {
		$this->_extraTables($sql,0, NULL);
		if ($error->Got(READONLY))  {
			$errLast   = $error->getLast();
			$error_txt = $errLast->text;
			$error_id  = $errLast->id;
			$error->reset();
			htmlInfoBox( "Warning", $error_txt, "", "WARN" );
		}
	}

}

function trackShowAssocs( &$sqlo ) {
	 $this->_trackShow( $sqlo, $this->tabArrAssoc );
}

function assocsExist() {
	return ( !empty($this->tabArrAssoc) );
}




/**
 * build reference arrays
 * @param  
 * @return $refTabsCount
 */
function _buildInfoArr(&$sql) {
	$search_table = $this->search_table;
	
	$getAllForeigns=0;
	
	$sqls = "SELECT x.table_name, y.nice_name, x.PRIMARY_KEY, x.column_name ".
  		  "FROM cct_column x, cct_table y ".
	      "WHERE x.cct_table_name ='".$search_table."' AND x.table_name = y.table_name ".
          "ORDER by y.nice_name ASC";  // SORT by table, see below check for TWO references
    $sql->query($sqls);
    $optioncnt    = 0;
    $refTabsCount = 0;
  
    while ( $sql->ReadRow() ) {
        $this->foreignLib->foreignElement( $sql->RowData );
	        
    } 
    
    $refTabsCount = $this->foreignLib->refTabsCount;
  
	return $refTabsCount;

}

/**
 * - build reference arrays for dest-table with MULTI-columns for ONE foreign-table
 * - e.g. CARTB_HAS_SOCKET
 * @param  $destTable
 * @return $refTabsCount
 */
function _infoArrMultiCol(&$sql, $destTable) {
	
	$search_table = $this->search_table;
	$sqls = "SELECT x.table_name, y.nice_name, x.PRIMARY_KEY, x.column_name ".
  		  "FROM cct_column x, cct_table y ".
	      "WHERE x.cct_table_name ='".$search_table."' AND x.table_name = y.table_name ".
          " and x.table_name=".$sql->addQuotes($destTable);
    $sql->query($sqls);
   
  
    $allForeigns=1;
    while ( $sql->ReadRow() ) {
        $this->foreignLib->foreignElement( $sql->RowData, $allForeigns );
	        
    } 
    $refTabsCount = $this->foreignLib->refTabsCount;
  
	return $refTabsCount;

}

function track_gui( &$sql) {
// descr: prints a HTML form for object tracking
  
  
  $tablename = $this->tablename;
 
	
  ?>
    <script language="JavaScript">
        
        function this_gotab( table ) {
            document.editform.t.value = table;
            document.editform.submit();
        }
        function this_gotab2(table, idx) { 
            document.editform.tableSCond.value = varx[idx];
            this_gotab( table );
        }

        function form2_gotab( desttable, column2 ) {
            document.form2.elements['parx[destTab]'].value = desttable;
            if (column2!=null) document.form2.elements['parx[col2]'].value = column2;
            document.form2.submit();
        }
    </script>
  <? 
  
  // second form to store tracking2 parameters ...
  echo '<form style=\"display:inline;\" method="post" name="form2" action="'.$_SERVER['PHP_SELF'].'">'."\n";
  echo '<input type="hidden" name="actionx" value="forwNorm">'."\n";
  echo '<input type="hidden" name="tablename" value="'.$tablename.'">'."\n";
  echo '<input type="hidden" name="parx[destTab]" value="">'."\n";
  echo '<input type="hidden" name="parx[col2]" value="">'."\n";
  echo '</form>'."\n";
  
   // main form to store tracking parameters for view.tmpl.php
  echo '<form method="post" name="editform" action="view.tmpl.php">'."\n";
  echo '<input type="hidden" name="condclean" value="1">'."\n";

  $search_table = $tablename;

  $pk_arr = primary_keys_get2($tablename);
  if ( $pk_arr[1] != "" ) {
	$search_table      = fk_check2($pk_arr[0], $tablename, $ftab_prim_name, $ftab_imp_name);
	$search_table_nice = tablename_nice2($search_table);
	echo "<B>Info</B>: This element part of <B>$search_table_nice</B>. Search for for this object...<P>\n";
  } 
  $this->search_table = $search_table;

  // 1. search for object links with PK=1
  // 2. search for objects links with PK>1
  // 3. search for objects links with PK==NULL
  
  // select sub elemments
  $tabarr = NULL;
  $foreignOpt = $this->parx['foreignOpt'];
  
  if ( !$foreignOpt ) {
  	// standard OP
  	$refTabsCount = $this->_buildInfoArr($sql, $tabarr);
  } else {
  	// all foreign keys of one destination table
  	$refTabsCount = $this->_infoArrMultiCol($sql, $this->parx['destTab']);
  }
  $tabarr       = $this->foreignLib->tabarr;
  $this->tabArrAssoc = $this->foreignLib->tabArrAssoc;
  
  	if ( !$refTabsCount ) { 
	  	echo "</form>\n";
		echo "<font color=gray>no relation found</font>";
		return;
	}
 
  
	// define SQL-remember array
	// 3 primary keys per table are possible !!!  
	echo '<script language="JavaScript">';
	echo "\n";
	// TBD: why 4 times ???
	echo '  var varx = new Array('.(sizeof($tabarr)*4).');'."\n";  // assign an amount of variables
	echo '</script>'."\n";
	echo "\n";
	$this->_trackShowStart();
	if ( !$foreignOpt ) {
		$this->_trackShow( $sql, $tabarr, "showExtra");
	} else {
		$this->_trackShowMultiCol( $sql, $tabarr, $this->parx['destTab'] );
	}
	
}

/**
 * - prepare parameters to do FORWARD to view.tmpl.php
 * @param $sqlo
 * @param array $parx 
 * 		'destTab' : destiantion table
 * 		'col2'    : column name in destination table (can be NULL : than use first column name)
 * 		'special' : 0,1 very special forward
 */
function forwardDo( &$sqlo, $parx) {
	global $error;
	$FUNCNAME= 'forwardDo';
	
	$search_table   = $this->tablename;
	$destTable  = $parx['destTab'];
	$fgCol      = $parx['col2'];
	
	if ($parx['special']>0) {
		// special forward
		$this->_extraTables($sqlo, 1, $parx);
		return;
	}
	
	$pkname_arr = primary_keys_get2($destTable);
	$pkname     = $pkname_arr[0];
	
	$i_tableNiceName = tablename_nice2($search_table);
	echo '- track to table "'.$i_tableNiceName.'" (column '.$fgCol.')<br />'."\n";
	
	
	$sqls = "select x.table_name, y.nice_name, x.PRIMARY_KEY, x.column_name ".
  		    " FROM cct_column x, cct_table y ".
	        " WHERE x.cct_table_name ='".$search_table."' AND x.table_name=".$sqlo->addQuotes($destTable);
         
	$sqlo->query($sqls);
	
	if ( !$sqlo->ReadRow() ) {
		$error->set( $FUNCNAME, 1, 'destination table '.$destTable.' can not be tracked for table '.$search_table );
		return;
	}
	$tabName_tmp     = $sqlo->RowData[0];
	$tabNameNice_tmp = $sqlo->RowData[1];
    $pk_tmp          = $sqlo->RowData[2]; 
    if ($fgCol==NULL) $fgCol = $sqlo->RowData[3];  // use default column name
	
	
	if ( $parx['destVia']==NULL ) {
		// normal forward
		
		$selcond2 = 'x.' . $fgCol. $this->sqlInSel;
		$selcondUrl = urlencode($selcond2);
		$urlnew =  'view.tmpl.php?t='.$destTable.'&condclean=1&tableSCond='.$selcondUrl;
		js__location_replace($urlnew, 'tracked table' );
	} else {
		// track via table 'destVia'
	}
}

}