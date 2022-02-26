<?php

class FileProtoImpHelp {
	
	var $headerPos; /**
		 - the header-to-Step-array
		 - array ($pos => array(STEP_NR, CODE, ERROR, IS_ID) )
		 */
	
function __construct($headArr1, $abstract_proto_id) {
	$this->headArr1 = $headArr1;
	$this->abstract_proto_id = $abstract_proto_id;
}

/**
 * check, if object exists in DB
 * @param $sql
 * @param $tablename
 * @param $objName NAME or ID
 * @param $opt     "isID" => 1 : $objName is strictly an ID
 * @return array($objok, $objError, $objId, $objName)
 * 		$objok >= 0 : o.k.
 */
function _objCheck( &$sql, 
	$tablename, 
	$objName, 	
	$opt			
	) {
    global $error;
        
    $objok   = 0;
    $objError= "";
    $ropt    = NULL;

	$optIsID = 0;
	if ($opt["isID"]) { // $objName is an ID
		$optIsID = 1;
		$objId   = $objName;
	}
	
    if ($opt["like"]) {
		$ropt["like"] = 1;
		$ropt["wild"] = 1;
	}
    if ( $objName == "" ) {
        $objok = -1;
        $objError = "Name/ID is empty";
        return (array($objok, $objError, $objId)); 
    }
	
	if ($optIsID) {
        if (!is_numeric($objId)) { 
             $objok = -5;
             $objError = '"'.$objId.'" is not a numeric ID';
             return (array($objok, $objError, ""));
        }
        
        if ( !gObject_exists($sql, $tablename, $objId) ) {
             $objok = -6;
             $objError = 'ID "'.$objId.'" not found';
             return (array($objok, $objError, ""));
        } 
		$objName = obj_nice_name($sql, $tablename, $objId);

    } else {
		// search for name
        $objfound = fileC::redundancyTest( $sql, $tablename, "NAME", $objName, $ropt);

        if ($objfound==0) {
        	//DEBUG:
            $objError = "not found";
            $objok = -3;
            return (array($objok, $objError, $objId));
        }
        if ($objfound>1) {
            $objError = $objfound." objects found";
            $objok = -4;
            return (array($objok, $objError, $objId));
        }

        // only one object found ...
        list($tmpcnt, $objId, $objName) = fileC::objGetByName($sql, $tablename, "NAME", $objName, $ropt);
    }
    
    return (array($objok, $objError, $objId, $objName));        

} 

/**
 * get protocol-step-array from file-header between pos0 and pos1 (of header)
 * - check if steps exist in ABSTRACT_PROTO $this->abstract_proto_id
 * @param $sqlo
 * @param $headArr1
 * @param $startpos
 * @param $endpos
 * @return 
 */
function anaHeaderProtoPart( &$sqlo, $startpos, $endpos) {
    global $error;
    
    $abstract_proto_id = $this->abstract_proto_id;
    $this->headerPos = NULL;
    $headArr1 = $this->headArr1;
    if ($headArr1==NULL) {
    	sys_error_my( 'class "FileProtoImpHelp" not initialized.' );
    }
    
    $protoTranslate = NULL;
    $protoTranslate["SUBST"]    = "CONCRETE_SUBST_ID";
	$protoTranslate["SUBST_ID"] = "CONCRETE_SUBST_ID";
	$protoTranslate["DEV"]      = "DEV_ID";
	$protoTranslate["DEV_ID"]   = "DEV_ID";
    $protoTranslate["QUANT"]    = "QUANTITY";
    $protoTranslate["NOTES"]    = "NOTES";
    $protoTranslate["INACT"]    = "NOT_DONE";
    

	$protoColsKeys = array_keys($protoTranslate);
	$protoColsImplode = implode(", ",$protoColsKeys);

    $tmperr   = NULL;
    
     
    if (!sizeof($headArr1)) {
        $error->set("ParseFileHeader()", 2,"no TAB found in header");
        return;
    } 

    $cnt=0;  // valid columns
    $headerPos = NULL;
    
    foreach( $headArr1 as $pos=>$headVal) {
    	
		if ($pos<$startpos) continue;
		if ($pos>$endpos)	break;
		
        $headVal = trim($headVal, "\x00\x09\x0A\x0B\x0D\x20\xA0");
       
        $tmperr  = NULL;
		$tmpIsID    = 0;
		$tmpStepnr  = 0;
		$tmpColCode = NULL;
		$tmpColIsSet = 1;
		
        do {
            
        	if ($headVal==NULL) {
        		$tmpColIsSet = 0;
        		break;
        	}
        	
            $posColon = strpos($headVal,":");
            if (!$posColon) {           
                $tmperr=array(2," no ':' found");
                break;
            }         
            $tmpStepnr = substr( $headVal,0, $posColon);
            if (!is_numeric($tmpStepnr))  {
                $tmperr=array(3," step_nr '$tmpStepnr' is not a number");
                break;
            } 
            // test, if ABSTRACT_PROTO has this STEP_NR
			$sqlsel = 'STEP_NR from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID='.$abstract_proto_id.' and STEP_NR='.$tmpStepnr;
			$sqlo->Quesel($sqlsel);
			if ( !$sqlo->ReadRow() )  {
                $tmperr=array(5,' step_nr '.$tmpStepnr.' not found in protocol (abstract)');
                break;
            } 

            $tmpColCode1 = substr( $headVal, $posColon+1 );
            $tmpColCode  = $protoTranslate[$tmpColCode1];
            if ($tmpColCode=="" or $tmpColCode1=="") {
                $tmperr = array(4, "Input: '".$tmpColCode1."': expect a column-code (".$protoColsImplode.") after '$tmpStepnr:' ");
                break;
            }  
			if ($tmpColCode1=="SUBST_ID") $tmpIsID = 1;
			if ($tmpColCode1=="DEV_ID")   $tmpIsID = 1;
        } while (0);
        
        if (is_array($tmperr)) {  
        	$tmpErrTxt =  $tmperr[1];
            $errGot = TRUE;
        }
        
   		if ( $tmpColIsSet ) {
    		$headerPos[$pos] = array ( $tmpStepnr, $tmpColCode, $tmpErrTxt, $tmpIsID); 
    		$cnt++;
   		}
        
    }
    reset($headArr1); 
    
    $this->headerPos = $headerPos;
            
    if ($errGot) {
    	$error->set("ParseFileHeader()", 10, "Errors got during header parsing");
    }     
    return  $headerPos;
}   

/**
 * analyse one file-line for protocol-steps
 * - get values from $valarr; analyse $this->headerPos and transform the $valarr to $updateStepArr
 * - $updateStepArr can be used by class oConcProtoUpSteps()
 * - return protocol-step-array
 * @param $sql
 * @param $valarr // array ($pos => VALUE ) 
 * @param $headerPos
 * @param $option // "like"
 * @global  $this->headerPos
 * @return array($updateStepArr, $rowInfo)
 * 		$updateStepArr[STEP_NR][COL_CODE] = SQL-value
 *      $rowInfo[$pos] = array ( SQL-value, $error_notes, $nice_notes );
 */
function checkParamsLine( &$sql, $valarr, $option=NULL	) {  
 	global $error;
    
	if ($this->headArr1==NULL) {
    	sys_error_my( 'class "FileProtoImpHelp" not initialized.' );
    }
    
    $pos			= 0;
    $updateStepArr	= NULL;
    $errorOccurred	= 0; 
    $error_notes	= NULL;
    $nice_notes		= NULL;
 
	$headerPos = $this->headerPos;
    $cheOption = NULL;
	if ($option["like"]) $cheOption["like"]=1;
	
	if ( $_SESSION["userGlob"]["g.debugLevel"]>1) {
	    debugOut('updateStepArrProto: '.print_r($this->updateStepArrProto,1), $FUNCNAME, 2);
	}
    
  	foreach( $headerPos as $pos=>$headTmpArr) {    
    
    	$step_nr = $headTmpArr[0];
    	$tmpCode = $headTmpArr[1];
		$tmpisId = $headTmpArr[3];
        $tmpVal  = $valarr[$pos];
        
        //trim whitespaces
        $tmpVal  = trim($tmpVal, "\x00\x09\x0A\x0B\x0D\x20\xA0");

        //DEBUG:
        //var_dump($tmpVal); 
        
        $error_notes = NULL;
        $nice_notes  = NULL;
        $sqlval      = NULL;
		$cheOption 	 = NULL;
		if ($tmpisId) $cheOption["isID"]=1;
        
        if ($tmpVal!="") switch ($tmpCode) {
        	case "CONCRETE_SUBST_ID":
 		
         		list($objok, $obj2Error, $substId,  $substGname ) = $this->_objCheck( $sql, "CONCRETE_SUBST", $tmpVal, $cheOption );
                if ($objok<0) {
                    $error_notes = $obj2Error.' (Input:'.$tmpVal.')';
                    break;
                }         
                $sqlval = $substId;
                $nice_notes = $substGname;
            	break;
			case "DEV_ID":
         		list($objok, $obj2Error, $substId,  $substGname ) = $this->_objCheck( $sql, "CHIP_READER", $tmpVal, $cheOption );
                if ($objok<0) {
                	$error_notes = $obj2Error;
                    break;
                }         
                $sqlval = $substId;
                $nice_notes = $substGname;
            	break;
            case "QUANTITY":
             	if (!is_numeric($tmpVal)) $error_notes = "not numeric";
             	$sqlval = $tmpVal;
                break;
            case "NOTES":         
             	$sqlval = $tmpVal; 
             	break;
            case "NOT_DONE":
            	if ($tmpVal!="0" AND $tmpVal!="1") {
                	$error_notes = "must be 0 or 1";
                    break;
                } 
            	$sqlval = $tmpVal;
            	break;
        }
        if ($error_notes!="") $errorOccurred = 1; 
        $rowInfo[$pos]    			= array ($sqlval, $error_notes, $nice_notes);
        $updateStepArr[$step_nr][$tmpCode] = $sqlval; // order by STEP_NR
    }
    reset($headerPos); 
    
    if ($errorOccurred) $error->set("checkParamsLine()", 1, "error detected"); 
    return array($updateStepArr, $rowInfo); 
}

/**
 * transform $rowInfo array to slim text
 * @param $rowInfo
 * @param $colid: 1 or 2 or ?
 * @return unknown_type
 */
function rowInfo2Str($rowInfo, $colid, $separator=', ') {
	if (!sizeof($rowInfo)) return;
	
	$infostr = NULL;
	reset ($rowInfo);
	$komma = '';
	foreach( $rowInfo as $pos=>$valarr) {
		if ($valarr[$colid]!=NULL) {
			$fileColName = $this->headArr1[$pos];
			$infostr .= $komma . 'col "'.$fileColName.'" '.$valarr[$colid];
			$komma = $separator;
		}
	}
	reset ($rowInfo); 
	return ($infostr);
}

}
