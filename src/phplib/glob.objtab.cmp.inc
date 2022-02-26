<? 
require_once('object.subs.inc');

/**
 * compare selected objects 
 * @package glob.objtab.cmp.inc
 * @swreq   UREQ:0000397: f > object compare 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class objCompc {

var $table; 
var $pkname;
var $assocs; // array[] = array( TABNAME, NICE_NAME )
var $SAVE_DET_MAX;

function __construct(
    &$sql, 
    $tablename, 
	$opt  = NULL  
    /*     $opt["clevel"]  compare level
                            [0]: only differences  (yes/no)
                            1: flag of difference types (FEAT:1, feature: probe_on_array:0, ...)
                            2: diff details
           $opt["showobj"] "" : all
                            "1": only different ones
		   $parx["igas"][TABLE][COL]  - ignore fields of special ASSOC-tables, Primary keys are not allowed ...
    */
    ) {
     
    $this->table  = $tablename;
    $this->pkname = PrimNameGet2($tablename);      
    $this->assocs = get_assoc_tables2( $sql, $tablename );
    $this->opt    = $opt;
    $this->SAVE_DET_MAX = 200;
}  

public function getFeat(&$sql, $idx) {

    $retarr = NULL;
    $sqls = "select * from ".$this->table." where ".$this->pkname."=".$idx;
    $sql->query($sqls);
    $sql->ReadArray();
    if (sizeof($sql->RowData)) { 
        foreach( $sql->RowData as $key=>$val) {
            if ( ($key != $this->pkname) AND $key!="EXTRA_OBJ_ID" AND $key!="CCT_ACCESS_ID") {
                $retarr[$key] = $val;
            }
        }
    }
    return ($retarr); 
} 

public function getAssocInfo( &$sql, $idx, $level=0 // 3: means: get values
    ) { 
    // RETURN: counts of elements  and features   
	
    $MAX_ELEMS_STORE = 250;
    
    $featVals = NULL;	// arra of assoc features
    $numElems = NULL;
    if (sizeof($this->assocs)) {
        foreach( $this->assocs as $tmpid=>$tmparr) {
		
            $tmptable = $tmparr[0];
            $tdiffCnt = 0;
            $tdiffSec = NULL;  // save differenct second primary key
            
            // first count elements
            $sqls = "select count(1) from ".$tmptable." where ".$this->pkname."=".$idx;
            $sql->query($sqls);
            $sql->ReadRow();
            $numElems[$tmptable] = $sql->RowData[0]; 
            
            $tmppks = primary_keys_get2($tmptable);
            $hasSecondPK = $tmppks[1]!=NULL? 1 : 0;
            
            if ($level >= 3 and $hasSecondPK) { 
                
				$selstr = $this->getAssocColStr( $tmptable );
				 
                $sqls = "select ".$selstr." from ".$tmptable.
                         " where ".$this->pkname."=".$idx."".
                         " order by ".$tmppks[1]; 
                $sql->query($sqls);
                $elemcnt     = 0;
				$featvalTab  = NULL; // temporary features PER TABLE X
                // TBD: save not more than 100 elements in $tdiffSec ???
                while ($sql->ReadArray() AND $elemcnt < $MAX_ELEMS_STORE) {
				
					$tmpFeatVals = NULL;
                    foreach( $sql->RowData as $key=>$val) {
                        if ($key!=$tmppks[0] AND $key!=$tmppks[1]) {                    
                            if ($val!="") $tmpFeatVals[$key] = $val;
                        }
                    }
					
                    $featvalTab[$sql->RowData[$tmppks[1]]] =  $tmpFeatVals;
                    $elemcnt++;
                }
                $featVals[$tmptable] = $featvalTab;
            }
        } 
        reset ($this->assocs);
    }
    return (array("cnt" => $numElems, "featVals" => $featVals));
}

/**
 * get columns for SQL-command, 
	- if columns are set for "IGNORED" ( $this->opt["igas"] ) => do not set them in the string
 * @param  $assocTabNow
 * @return string like: IMG_ID, POS, EXP_ID
 */
private function getAssocColStr( $assocTabNow ) {

	$selstr = "*";
	if ( is_array($this->opt["igas"][$assocTabNow]) ) {
	
		// ignore some columns
		$assocCols = columns_get2 ( $assocTabNow );
		$assocIgnoreCols = $this->opt["igas"][$assocTabNow];
		$selstr = "";
		$tmpsep = "";
		foreach( $assocCols as $key=>$tmpCol) {
			
			if ( $assocIgnoreCols[$tmpCol]<1 ) {
				$selstr .= $tmpsep . $tmpCol;
				$tmpsep = ", ";
			}
		}
		reset ($assocCols); 
	}
	
	
	return ($selstr);
}

/**
 * compare two objects
 * @param object $sql
 * @param int $id1  Primary KEY 1
 * @param int $id2 Primary KEY 2
 * @param int $level
 * ""  count only
   "1" analyze elements
   "2" store different elements in "diffDetail"
   "3" store differnce values in "diffDetail"
 * @return array $result
 * "diffCnt"    = array[$table] number of different counts
   "diffCont"   = array[$table] number of elements with different content
   "diffDetail" = 
      array[$table] difference detailed
   	  or "1"; the OBJX has one LESS, OBJREF o.k.
   	  or "0"; the OBJX o.k., OBJREF has one LESS
   "diffDetVal" = array[$table] values
 */
private function diffAssocs( &$sql, 
	$id1, 		//
	$id2, 		// 
    $level=NULL  // 
     ) { 
	
	
	
	
	$SAVE_DET_MAX = $this->SAVE_DET_MAX;
    
    $diffElem = NULL;  // $diffElem[$table]
    $diffCont = NULL;
    $diffDetail = NULL;
    $diffDetVal = NULL;
    $debug = 0; 
    if ( $_SESSION['userGlob']["g.debugLevel"]>0 ) $debug=$_SESSION['userGlob']["g.debugLevel"];
    if ($debug>0) echo "<B>DEBUG</B>:diffAssocs: $id1:$id2<br>\n";
	        
    if (sizeof($this->assocs)) {  
     
        if ($debug>0) echo "<B>DEBUG</B>:<pre>\n";
		
        foreach( $this->assocs as $tmparr) {
		
            $assocTabNow = $tmparr[0];
            //$tdiffCnt = 0;
            $tdiffSec = NULL;  // array(SECOND_KEY) = "" or "val", ... save differenct second primary key
            //$tdiffVal = NULL;  // array(SECOND_KEY) = array(diff_values)
            
            // first count elements
            $sqls = "select count(1) from ".$assocTabNow." where ".$this->pkname."=".$id1;
            $sql->query($sqls);
            $sql->ReadRow();
            $numElems=array();
            $numElems[1] = $sql->RowData[0];
            
            $sqls = "select count(1) from ".$assocTabNow." where ".$this->pkname."=".$id2;
            $sql->query($sqls);
            $sql->ReadRow();
            $numElems[2] = $sql->RowData[0];
            
            $tmpdiff = $numElems[2]-$numElems[1];
            if ($tmpdiff) $diffElem[$assocTabNow] = $tmpdiff;
            
            if ($level>=1) {
                 $sqlOrder = "";
                 $tmpsep   = "";
                 $tmppks = primary_keys_get2($assocTabNow); 
                 
                 $orderPK = array(); // [0]=>REFOBJ, [1]=>XOBJ
                 $orderPK = array($id1,$id2);
				 //if ($id1<=$id2) $orderPK = array($id1,$id2);
				 //else  $orderPK = array($id2,$id1);
                 
                 
                 $cnt=0;
                 foreach( $tmppks as $tmppki) {
                     if (!$cnt) {
                         // skip first
                     } else {
                         $sqlOrder .= $tmpsep . $tmppki;
                         $tmpsep    = ", ";
                     }
                     $cnt++;
                 }
                 
                 if ($id1>$id2)  $sqlOrder .= ' DESC'; // sort main PK in other direction (like the user sort option)
                 
                 
                 // produce a PK-ordered query: SECOND_PK, THIRD_PK, FIRST_PK
				 
				 $selstr = $this->getAssocColStr( $assocTabNow );
				 
				
				 $orderID = 0; // alternates 0 or 1
				 
            	 if ($debug>2) {
                        echo "DEBUG: Compare-Start: TAB:".$assocTabNow." OBJ1:".$orderPK[0]." OBJ2:".$orderPK[1]."  PK2:".$tmppks[1]."<br />\n";
                 }
				 
                 $sqls = "select ".$selstr." from ".$assocTabNow.
                         " where (".$this->pkname."=".$id1." OR ".$this->pkname."=".$id2.")".
                         " order by ".$sqlOrder;
                 $sql->query($sqls);
                 $diffcntx = 0;
                 $compNow  = 0;	   // alternate this value 
				 $dataOBX  = NULL;
				 $fav = array();
				 
                 // TBD: save not more than 100 elements in $tdiffSec ???
				 
                 while ($sql->ReadArray()) {	// one LOOP per elements
				 
                    $faNow =  $sql->RowData;	    // current data BLOB
					
					if ($debug>2) {
                        echo "DEBUG: compnow: $compNow, orderID:$orderID ;  current data:";
                        print_r($faNow);
						if (is_array($faNow)) reset ($faNow);
						echo "<br>";
                    }
                    do {
	                    if ($orderID==0) {
	                    	// expect first data
	                    	if ($faNow[$this->pkname]==$orderPK[0]) {
	                    		// o.k.
	                    		$fav[0]  = $faNow;
	                    		$compNow = 0;
	                    	} else  {
	                    		// DIFF! second object occurs; OBJREF missing
		                    	if ($debug>2) {
	                        		echo "DEBUG: first OBJ expected! PK2:".$faNow[$tmppks[1]]."<br>";
	                        	}
	                    		$tdiffSec[$faNow[$tmppks[1]]] = 0;  // the OBREF has one LESS
	                    		$diffcntx++;
	                    		$orderID = 0;
	                    		$compNow = 0;
	                    		break;
	                    	}
	                    } else {
	                    	// expect second data
	                    	if ($faNow[$this->pkname]==$orderPK[1]) {
	                    		// o.k.
	                    		
	                    		// check PK2; e.g. STEP_NR
	                    		if ($faNow[$tmppks[1]] != $fav[0][$tmppks[1]]) { 
	                    			// problem; two errors; 
	                    			//  PK2: 19  ---
	                    			//  PK2: --- 20
	                    			$tdiffSec[$fav[0][$tmppks[1]]] = 1;  // the OBJX has one LESS
	                    			$diffcntx++;
	                    			$tdiffSec[$faNow[$tmppks[1]]]  = 0;  // the OBJREF has one LESS
	                    			$diffcntx++;
	                    			$orderID = 0;
	                    			break;
	                    		}
	                    		
	                    		// o.k.
	                    		$compNow = 1;
	                    		$fav[1] = $faNow;
	                    	} else  {
	                    		// DIFF! FIRST object occurs; OBJREF missing
		                    	if ($debug>2) {
	                        		echo "DEBUG: second OBJ expected! PK2_expect:".$fav[0][$tmppks[1]]."<br>";
	                        	}
	                    		$tdiffSec[$fav[0][$tmppks[1]]] = 1;  // the OBJX has one LESS
	                    		$diffcntx++;
	                    		$orderID = 1;
	                    		$fav[0]  = $faNow;
	                    		$compNow = 0;
	                    		break;
	                    	}
	                    }
	                    
	                    if ($compNow) {
	                    	
	                    	// analyze difference
								
							$dataOBX = &$fav[1]; // reference data
                   
                            $diffcntFeat = 0;
                            $tmpFeatVals = NULL;
							// for each ASSOC-COL do ...
                            foreach( $fav[0] as $key=>$val) {
                            	if ($key!=$tmppks[0] AND $key!=$tmppks[1]) { // no primary keys ?
									// now compare the ASSOC-value
                                    if  ($fav[0][$key] !== $fav[1][$key]) {
                                        $tmpFeatVals[$key] = $dataOBX[$key];
                                        $diffcntFeat++;
                                    }
                                }
                            }
                            reset($fav[0]);
						
                            if ($diffcntFeat) {
							 
                                if ($diffcntx < $SAVE_DET_MAX) {
									if ($debug>1) {
										echo "DIFFERENCE_VALUES: ";
										print_r($tmpFeatVals);
									}
                                    if ($level<=2) $tdiffSec[$fav[0][$tmppks[1]]] = "val";        // only "val"
                                    else           $tdiffSec[$fav[0][$tmppks[1]]] = $tmpFeatVals; // values !!!
                                }
                                $diffcntx++;
                             }    
	                    }
	                    
	                    // toggle OrderID
	                    if (!$orderID) $orderID=1;
	                    else $orderID = 0;
	                    
	                    
	                    
                    } while (0);
                    
                 	if ($debug>2) {
	                       echo "DEBUG: --next: orderID:$orderID fa[0]:".$fav[0][$tmppks[0]].':'.$fav[0][$tmppks[1]];
	                       echo "<br>";
	                }
 
                 
                    
                 }
                 
                 // one compare is open ?
                 if ($orderID==1) {
                 	if ($debug>2) {
                        echo "DEBUG: DIFF at end of compare: OBJ:".$fav[0][$this->pkname]." PK2:".$fav[0][$tmppks[1]]." <br>";
                    }
                    
                 	$tdiffSec[$fav[0][$tmppks[1]]] = "1"; // the OBJX has one LESS
                    $diffcntx++;
                 }
                 
                 
                 if ($diffcntx) $diffCont[$assocTabNow]   = $diffcntx;  // otherwise  $diffCont[$assocTabNow] is not set!
                 if ($level>=2) $diffDetail[$assocTabNow] = $tdiffSec;
                 // if ($level>=3) $diffDetVal[$assocTabNow] = $tdiffVal;  
            }
        }
        reset ($this->assocs);
        if ($debug>0) {
			echo "</pre>\n";    
		}    
    }
    return( array("diffCnt"=>$diffElem, "diffCont"=>$diffCont, "diffDetail"=>$diffDetail, "diffDetVal" => $diffDetVal ) );
}

// PUBLIC

function compare( &$sql, $id1, $id2 ) {
    /* Function compare two objects 
    Return: $retarr[
            "sum"      = 0,n differences
            "features" = array(features)
            "assoc"    = array(feature_lists)
            "access"   = array(access_info)     // will not be compared usually
   */ 
   
   $diffCnt=0;
   
   
   // 1. get features 
   $features=array();
   $features[1] = $this->getFeat( $sql, $id1);
   $features[2] = $this->getFeat( $sql, $id2);
   
   $colNames = columns_get2($this->table);
      
   $featCnt = 0;
   $featArr = NULL;
   foreach( $colNames as $key) {  
        if ( ($key == $this->pkname) OR $key=="EXTRA_OBJ_ID" OR $key=="CCT_ACCESS_ID") {
            continue;
        }
        
        if ($features[1][$key] != $features[2][$key]) {
            $featArr[$key] = $features[2][$key]; // save difference
            $diffCnt++;
        }
        $featCnt++;
   }
   
   
   // 2. get featlists
   
   $assoclevel = 1;
   if (  $this->opt["clevel"]>=2 ) $assoclevel = $this->opt["clevel"];  
   $assocDiff = $this->diffAssocs( $sql, $id1, $id2, $assoclevel);
    
   return ( array("sum"=>$diffCnt, "features"=>$featArr, "assoc" => $assocDiff ) ); 
}

}
