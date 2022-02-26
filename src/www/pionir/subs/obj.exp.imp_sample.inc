<?php
/**
 * sub functions to import protocol parameters
 * used by obj.exp.imp_sample.php
 * @package obj.exp.imp_sample.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 * 
 * @var array headerPos_STRUCT
	 *    array[POS] = array( 
	 *       'st'  => $step_nr, 
	 *       'col' => $tmpColCode, 
	 *       'err' => $tmpErrTxt, 
	 *       'isID'=> $tmpIsID
	 *       'ap'  => PRA_ID if given in file header
	 *    ) 
 */
require_once ('o.PROTO.upsteps.inc');
require_once 'o.CS_HAS_PR.subs.inc';
require_once 'o.AS_HAS_PR.subs.inc';
require_once 'o.CONCRETE_SUBST.proto.inc';
require_once ("import/f.read_file_meta.inc");

/**
 * protocol param file handling functions
 * @author steffen
 *
 */
class gObjMapSampFile {
    
    private $pra_ids; // PRAs defined in the file
    private $filename;
    const Multi_PRA_IDs='#Multi-PRA-IDs';
    
	var $CLASSNAME='gObjMapSampFile';
	function __construct($tablename) {
		$this->dataStartLine=0; // first line=1
		$this->headStartLine=0;
		$this->tablename = $tablename;
		$this->_has_MultiPras=0;
	}

	function openfile($filename) {
	    $this->filename=$filename;
	    $this->_filelib = new f_read_file_meta($this->filename, 'excel');
	    
	    $this->_filelib->open_file();
	    $headers  = $this->_filelib->get_headers();
	    $this->header   = $headers[0];
	    $this->encoding_key = $this->_filelib->get_encoding_info();
	}
	function has_MultiPras() {
	    return $this->_has_MultiPras;
	}
	function get_pra_ids() {
	    return $this->pra_ids;
	}
	
	function read_line() {
	    return $this->_filelib->read_line(1);
	}
	function get_data_row() {
	    return $this->_filelib->get_data_row();
	}
	
	function showFirstLines() {
	    echo "<br>";
	    htmlInfoBox( "First lines of file", "", "open", "CALM" );
	    echo "<pre>";
	    
	    $this->openfile($this->filename);

        $cnt=0;
        while( $this->_filelib->read_line(0) )  {
            
            $valarr  = $this->_filelib->get_data_row();
            echo implode("\t",$valarr);
            if ($cnt>=5) {
                break;
            }
            $cnt++;
        }
        $this->_filelib->close_file();
	    
	    
	    echo "</pre>";
	    htmlInfoBox( "", "", "close");
	}
	
	private function _analyse_comments() {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    $this->pra_ids=array();
	    $xmax = sizeof($this->header)+1;
	    $y_data = $this->_filelib->get_data_start_line_no();
	    $last_comment_y = $y_data - 2;
	    
	    debugOut("(96) comment: xmax:$xmax, last_comment_y:$last_comment_y", $FUNCNAME, 1); 
	    
	    if ($last_comment_y>0) {
	        
	        $pos=0;

	        $comment_field = $this->_filelib->get_data_cell(1, $last_comment_y);
	        if ($comment_field==self::Multi_PRA_IDs) {
	        
    	        $this->_has_MultiPras=1;
    	        for ($x=2; $x<=$xmax;$x++) {
    	           $this->pra_ids[$pos]=$this->_filelib->get_data_cell($x, $last_comment_y);
    	           $pos++;
    	        }
    	        debugOut("(110) cPRAs detected:".print_r($this->pra_ids,1), $FUNCNAME, 1); 
	        }
	    }
	}
	
	/**
     * parse header  (old: thisParseFileHeader)
	 * - ignore '#' comments
	 * @return $headerPos headerPos_STRUCT
	 */
	function parseFileHeader() {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
		$tablename = $this->tablename;
		$filename  = $this->filename;
		
		$this->_analyse_comments();
		
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

	    $headArr1 = $this->header;
	
	    $cnt=0;
	    $step_col_index=0;
	    $headerPos = NULL; 
	
	    foreach( $headArr1 as $headVal) { 
	
	        $headVal = trim($headVal);
	        $tmperr  = NULL;
			$tmpIsID = 0;
			$tmpStepnr = 0;
			$tmpColCode = NULL;
			
	        do {
	            if ( !$cnt ) {
				
	                if ($headVal!=$tablename AND $headVal!=$tablename."_ID")  {
	                    $tmperr=array(5," column '".$tablename."' not found");
	                    break;
	                }         	
	                $tmpStepnr = -1;
	                $tmpColCode = $tablename;
					if ($headVal==$tablename."_ID") $tmpIsID = 1;
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
	        
	        // JUHHUUUU 
	        $tmparr = array ( 'st'=>$tmpStepnr, 'col'=>$tmpColCode, 'err'=>$tmpErrTxt, 'isID'=>$tmpIsID); 
	        if ($this->_has_MultiPras and $cnt) {
	            
	            $pra_tmp = $this->pra_ids[$step_col_index];
	            if (!$pra_tmp) {
	                $tmparr['err'] =  'PRA expected for this column!';
	                $errGot = TRUE;
	            }
	            $tmparr['ap']=$pra_tmp;
	            $step_col_index++;
	        }
	        $headerPos[$cnt] = $tmparr;
	
	        $cnt++;
	    }
	   
	            
	    if ($errGot) {
	    	$error->set($FUNCNAME, 10, "Errors got during header parsing");
	    }          
	    return  array($headerPos, $headArr1);
	} 

	
	function parseFileHeader2( $opt=NULL) {
		 global $error;
	   
	     
	     list($headerArr, $headerRaw) = $this->parseFileHeader( ) ;
	     if ($error->Got(READONLY))  {
	     	   htmlInfoBox( "Error during header parsing", "", "open", "ERROR" );
	           foreach( $headerArr as $pos=>$headTmpArr) {
	           		if ($headTmpArr['err']!="") { 
	                	echo "column ".($pos+1).": '".$headerRaw[$pos]. "'  Error: ".$headTmpArr['err']."<br>\n";
	                } 
	           }
	           
	           htmlInfoBox( "", "", "close" );
	           
	           $this->showFirstLines();
	     }
	
	     return array($headerArr, $headerRaw);
	}
	
	/**
	 * get one valid object
	 * @param $opt
	 *     "like"
	 * @return array($retexpId, $expGname, $objInFileFirst)
	 */
	function  getOneValidObj( &$sql, &$headerArr, $opt=NULL	) {
		$tablename= $this->tablename;
		$out_obj_id = 0;

		$expTmpArr  = current($headerArr); // first element 
		$tmpisId    = $expTmpArr['isID'];
		
		
	    $cheOption = NULL;
		if ($tmpisId) 	  $cheOption["isID"] = 1;
		if ($opt["like"]) $cheOption["like"]=1;

	    $cnt = 0;
	    $objInFileFirst=NULL;
	    
	    while( $this->_filelib->read_line(0) )  {
	        
	        $valarr  = $this->_filelib->get_data_row();

	        $objInFile = trim($valarr[0]);
	        list($obj1ok, $obj1Error, $expId, $expGname)  
	        	= gObjImportSingle::objCheck( $sql, $tablename, $objInFile, $cheOption );
	        if ($obj1ok>=0) {
	            $out_obj_id = $expId;
	            break;
	        }
	        if (!$cnt) $objInFileFirst=$objInFile;
	        
	        $cnt++;
	    } 
	    
	    $this->_filelib->close_file();
	    
	    return array($out_obj_id, $expGname, $objInFileFirst);
	}

}

class gObjImportSingle {
    
    function __construct($headerArr, $MultiPras_flag, $pra_ids) {
        $this->protoUpdLib = new oConcProtoUpSteps();
        $this->headerArr=&$headerArr;
        $this->MultiPras_flag=$MultiPras_flag;
        $this->pra_ids=&$pra_ids;
    }
	
	/**
	 * return nice SQL-parameters
	 RETURN:
	 * @param object $sql
	 * @param array $valarr // array ($pos => VALUE )
	 * @param array $headerArr : headerPos_STRUCT
	 * @param string $option
	 *      // "like"
			// "table" : table name
	 * @return  array $sqlArr[STEP_NR][COL_CODE] = SQL-value
	 $infoArr[$pos] = array ( SQL-value, $error_notes, $nice_notes );
	 */
	function checkParamsLine( &$sql, $valarr, $option=NULL	) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$headerArr= $this->headerArr;
	
		$pos           = 0;
		$sqlArr 	   = array();
		$errorOccurred = 0;
		$error_notes = NULL;
		$nice_notes  = NULL;
		$errorLastNotes = NULL;
	
		$expTmpArr = current($headerArr); // first element
		$expName   = trim($valarr[0]);
		$tmpisId   = $expTmpArr['isID'];
	
		$cheOption = NULL;
		if ($tmpisId) $cheOption["isID"] = 1;
		if ($option["like"]) $cheOption["like"]=1;
	
		list($obj1ok, $obj1Error, $expId, $expGname ) = gObjImportSingle::objCheck( $sql, $option["table"], $expName, $cheOption );
		if ($obj1ok<0) {
			$errorOccurred = 1;
			$errorLastNotes   = 'main object "'.$expName.'":' . $obj1Error;
		}
		$rowInfo=array();
		$rowInfo[$pos]  = array ($expId, $error_notes, $expGname);
		 
	    $step_cell_ind=0;
		foreach( $headerArr as $pos=>$headTmpArr) {
	
		    if (!$pos) continue; // ignore first column (Main object column)
		      
		    /*
		     *  'st'  => $step_nr, 
	 *       'col' => $tmpColCode, 
	 *       'err' => $tmpErrTxt, 
	 *       'isID'=> $tmpIsID
		     * */
			$step_nr = $headTmpArr['st'];
			$tmpCode = $headTmpArr['col'];
			$tmpisId = $headTmpArr['isID'];
			$tmpVal  = trim($valarr[$pos]) ;
	
			$error_notes = NULL;
			$nice_notes  = NULL;
			$sqlval      = NULL;
			$cheOption 	 = NULL;
			if ($tmpisId) $cheOption["isID"]=1;
	
			if ($tmpVal!="") switch ($tmpCode) {
				case "CONCRETE_SUBST_ID":
					list($objok, $obj2Error, $substId,  $substGname ) = gObjImportSingle::objCheck( $sql, "CONCRETE_SUBST", $tmpVal, $cheOption );
					if ($objok<0) {
						$error_notes = $obj2Error;
						break;
					}
					$sqlval = $substId;
					$nice_notes = $substGname;
					break;
				case "DEV_ID":
					list($objok, $obj2Error, $substId,  $substGname ) = gObjImportSingle::objCheck( $sql, "CHIP_READER", $tmpVal, $cheOption );
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
			if ($error_notes!="") {
				$errorOccurred  = 1;
				$errorLastNotes = $error_notes;
			}
			$rowInfo[$pos]  = array ($sqlval, $error_notes, $nice_notes);
			
			if ($this->MultiPras_flag) {
			    $pra_id = $this->pra_ids[$step_cell_ind];
			    if (empty($sqlArr[$pra_id])) $sqlArr[$pra_id]=array();
			    $sqlArr[$pra_id][$step_nr][$tmpCode] = $sqlval;
			} else {
			    $sqlArr[$step_nr][$tmpCode] = $sqlval; // order by STEP_NR
			}
			
			$step_cell_ind++;
		}
		
	
		if ($errorOccurred) $error->set($FUNCNAME, 1, $errorLastNotes);
		return array($sqlArr, $rowInfo, $expId);
	}
	
	
	/**
	 * get CONCRETE_PROTO_ID of object
	 * - if not exists: create one
	 * @param object $sql
	 * @param int $objid
	 * @param int $REF_pra_id
	 * @param string $tablename
	 * @return void|number CONCRETE_PROTO_ID
	 */
	function objCheckProto( &$sql, $objid, $REF_pra_id, $tablename ) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		$cProtoid = 0;
	
	
		switch ($tablename) {
	
			case "CONCRETE_PROTO":
				$sqls= "select ABSTRACT_PROTO_ID from CONCRETE_PROTO where CONCRETE_PROTO_ID=".$objid;
				$sql->query("$sqls");
				$sql->ReadRow();
				$aCompProto = $sql->RowData[0];
				$cProtoid = $objid;
				if ( $aCompProto != $REF_pra_id) {
					$error->set($FUNCNAME, 1, "abstract protocol does not match with reference protocol!");
					return;
				}
				break;
					
			case "CHIP_READER":
				$sqls= "select c.CONCRETE_PROTO_ID  from CHIP_READER c where CHIP_READER_ID=".$objid;
				$sql->query("$sqls");
				$sql->ReadRow();
				$cProtoid = $sql->RowData[0];
					
				if ($cProtoid) {
					$sqls= "select ABSTRACT_PROTO_ID from CONCRETE_PROTO where CONCRETE_PROTO_ID=".$cProtoid;
					$sql->query("$sqls");
					$sql->ReadRow();
					$aCompProto = $sql->RowData[0];
	
					if ( $aCompProto != $REF_pra_id) {
						$error->set($FUNCNAME, 1, "abstract protocol does not match with reference protocol!");
						return;
					}
				}
				break;
					
			case "CONCRETE_SUBST":
			    
			    $suc_cs_lib = new oCS_HAS_PR_subs();
			    $suc_cs_lib->setObj($objid);
			    $proto_log = $suc_cs_lib->getProtoLog($sql);
			    
			    $found=0;
			    foreach($proto_log as $rowtmp) {
			        $this_pra_id = $rowtmp['ap'];
			        if ( $this_pra_id == $REF_pra_id) {
			            $cProtoid = $rowtmp['cp'];
    				    $found=1;
    				    break;
    				}
			    }
			    
			    if (!$found) {
			        //
			        // create a protocol, if missing
			        //
			        $suc_proto_lib = new oCONCRETE_SUBST_proto($objid);
			        $features=array();
			        $cProtoid = $suc_proto_lib->create_proto($sql, $features, $REF_pra_id);
			        if ($error->Got(READONLY))  {
			            $error->set( $FUNCNAME, 1, 'Error on protocol creation for MAC-ID:'.$objid );
			            return;
			        }
			    }
				
				break;
				 
			case "EXP":
				$sqls= "select e.CONCRETE_PROTO_ID from EXP_HAS_PROTO e, CONCRETE_PROTO c where e.EXP_ID=".$objid. " AND
					e.CONCRETE_PROTO_ID=c.CONCRETE_PROTO_ID AND c.ABSTRACT_PROTO_ID=".$REF_pra_id;
				$sql->query("$sqls");
				$sql->ReadRow();
				$cProtoid = $sql->RowData[0];
				break;
					
			case "W_WAFER":
				$sqls= "select c.CONCRETE_PROTO_ID from W_WAFER_STEP e, CONCRETE_PROTO c where e.W_WAFER_ID=".$objid. " AND
					e.CONCRETE_PROTO_ID=c.CONCRETE_PROTO_ID AND c.ABSTRACT_PROTO_ID=".$REF_pra_id;
				$sql->query("$sqls");
				$sql->ReadRow();
				$cProtoid = $sql->RowData[0];
				break;
		}
	
	
		if ( !$cProtoid ) {
			$error->set($FUNCNAME, 1, "concrete Protocol not found");
			return;
		}
	
		$o_rights = access_check($sql, 'CONCRETE_PROTO', $cProtoid);
		if ( !$o_rights['insert']) {
			$error->set($FUNCNAME, 2, "No INSERT-perimission to the protocol");
			return;
		}
	
		return ($cProtoid);
	}
	
	/**
	 * find object
	 * @param object $sql
	 * @param string $tablename
	 * @param string $objName -- NAME or if $objName is sandwiched by "[" "]" is an ID
	 * @param array $opt  
	 *     "like",
		   "isID" => 1 : $objName is strictly an ID
	 * @return 
	 */
	static function objCheck( &$sql,$tablename,$objName, 	$opt = NULL ) {
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
		 
		// test for pattern: "[number]"
		if ( strpos($objName,"[")=="0" AND strpos($objName,"]")==(strlen($objName)-1) ) {
			// contains ID
			$objId = substr($objName,1,strlen($objName)-2); // get number
			$optIsID = 1;
		}
	
		if ($optIsID) {
			if (!is_numeric($objId)) {
				$objok = -5;
				$objError = "is not a numeric ID";
				return (array($objok, $objError, ""));
			}
	
			if ( !gObject_exists($sql, $tablename, $objId) ) {
				$objok = -6;
				$objError = "ID not found";
				return (array($objok, $objError, ""));
			}
			$objName = obj_nice_name($sql, $tablename, $objId);
	
		} else {
	
			$objfound = fileC::redundancyTest( $sql, $tablename, "NAME", $objName, $ropt);
	
			if ($objfound==0) {
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
	 * update a concrete_proto with steps in $stepArr
	 * @param array $stepArr array ( step_nr => $colValArr)
	 */
	function protoUpdate( &$sql, $cprotoId, $aProtoId, 	$stepArr ) {
	    $this->protoUpdLib->setProto($cprotoId);
	    $this->protoUpdLib->protoUpdate( $sql, $aProtoId, $stepArr );
	}
}

/**
 * main lib for import
 * @author steffen
 *
 */
class gObjMapSample {
    
    var $filename;
    var $tablename;
    var $go;
	
	function __construct($tablename, $go) {
		$this->tablename = $tablename;
		$this->go = $go;
		
		$this->fileLib = new gObjMapSampFile($tablename);
	}
	
	function GoInfo($go, $coltxt=NULL) {
		require_once ("func_form.inc");
		$goArray   = array( "0"=>"Give file", 1=>"Select protocol", 2=>"Prepare Import", 3=>"Update protocols" );
		
		
		if ($coltxt!=NULL) {
			$coltxt = '?'.$coltxt;
		}
		
		$extratext = '[<a href="'.$_SERVER['PHP_SELF'].$coltxt.'">Start again</a>]';
		
		$formPageLib = new FormPageC();
		$formPageLib->init( $goArray, $extratext );
		$formPageLib->goInfo( $go ); 
	
	}
	
// 	function setFilename($filename) {
// 		$this->filename = $filename;
// 		$this->fileLib->setFilename($filename);
// 	}
	function openfile($filename) {
	    $this->filename = $filename;
	    $this->fileLib->openfile($filename);
	}
	
	
	
	function fileParseHead2($opt=NULL) {
		return $this->fileLib->parseFileHeader2($opt);
	}
	function getOneValidObj(&$sql, &$headerArr, $opt=NULL ) {
		return $this->fileLib->getOneValidObj($sql, $headerArr, $opt);
	}
	
	private function _one_proto_update($single_Line_lib, $sqlo, $dest_obj_id, $pra_id, $proto_steps) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    $tablename = $this->tablename;;
	    $go = $this->go;
	    
	    // do also access_check ...
	    $tmpCprotoId = $single_Line_lib->objCheckProto( $sqlo, $dest_obj_id, $pra_id, $tablename );
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 1, 'objCheckProto failed.' );
	        return;
	    }
	    if ( $go == 3 ) {
	        $single_Line_lib->protoUpdate( $sqlo,  $tmpCprotoId, $pra_id, $proto_steps );
	        if ($error->Got(READONLY))  {
	            $error->set( $FUNCNAME, 1, 'protocolUpdate failed.' );
	            return;
	        }
	    }
	}

	private function _multi_protos(&$single_Line_lib, $sqlo, $dest_obj_id, $sqlUpdateObjects) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    foreach($sqlUpdateObjects as $pra_id => $proto_steps) {
	        $this->_one_proto_update($single_Line_lib, $sqlo, $dest_obj_id, $pra_id, $proto_steps);
	        if ($error->Got(READONLY))  {
	            $error->set( $FUNCNAME, 1, 'Error for PRA:'.$pra_id );
	            return;
	        }
	    }
	}

	public function has_MultiPras() {
	    return $this->fileLib->has_MultiPras();
	}
	public function get_pra_ids() {
	    return $this->fileLib->get_pra_ids();
	}

	/**
	 * main file loop
	 * @param array $headerArr headerPos_STRUCT
	 */
	function doMain(&$sqlo, $headerArr, $parx) {
		//
		// START import
		//
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		
		$tablename = $this->tablename;
		$go = $this->go;

		$i         = 0;
		$allErrSum = 0;
		$allOkSum  = 0;
		
		$copt = NULL;
		$copt["table"] = $tablename;
		if ($parx["expunexact"]) $copt["like"] = 1;
		
		$MultiPras_flag = $this->fileLib->has_MultiPras();
		$pra_ids = $this->fileLib->get_pra_ids();
		
		$single_Line_lib = new gObjImportSingle($headerArr, $MultiPras_flag, $pra_ids);
		
		

		while( $this->fileLib->read_line() )  {
		    
		    $valarr  = $this->fileLib->get_data_row();
		
		    $objok      = 1;
		    $objError   = "";
		    $objErrorSum= "";
  
		    do {
		        
		        
		    	list( $sqlUpdateObjects, $rowInfo, $dest_obj_id ) = $single_Line_lib->checkParamsLine( $sqlo, $valarr, $copt );
		    	if ($_SESSION['userGlob']["g.debugLevel"]>0) {
		    	    debugOut('sqlUpdateObjects:'.print_r($sqlUpdateObjects,1), $FUNCNAME, 1); 
		    	}
		    	
		        if ($error->Got(READONLY)) { // remove error from stack ...
		        	$objok = -1;
		        	$errLast   = $error->getLast();
		        	$error_txt = $errLast->text;
		        	$error->reset();
		        	$objok    = -2;
		        	$objError = "checkParamsLine():".$error_txt;
		            break;
		        }
		
		        if ($MultiPras_flag) {
		            $this->_multi_protos($single_Line_lib, $sqlo, $dest_obj_id, $sqlUpdateObjects);
		            if ($error->Got(READONLY))  {
		                $errLast   = $error->getLast();
		                $error->reset();
		                $error_txt = $errLast->text;
		                $objok    = -7;
		                $objError = "multi_protos():".$error_txt;
		                break;
		            }
		        } else {
		            // do also access_check ...
		            $this->_one_proto_update($single_Line_lib, $sqlo, $dest_obj_id, $parx["aprotoid"], $sqlUpdateObjects );
		            if ($error->Got(READONLY))  {
		                $error_txt = $error->getAllAsText();
		                $error->reset();
		                $objok = -3;
		                $objError = "protocolUpdate():".$error_txt;
		                break;
		            }
		        }
		 
		    } while (0);
		        
		    if ($objok<0)  {
		        $objErrorSum = "<font color=red>Error:</font>".$objError;
		        $allErrSum++;
		    } else {
		        $objErrorSum = "<font color=green>Ok</font>";
		        $allOkSum++;
		    }  

		    echo "<tr bgcolor=#EFEFEF class=t1>";
		    echo "<td>".($i+1)."</td>";
		    echo "<td>".$objErrorSum."</td>";
		    foreach( $rowInfo as $pos=>$tmpiarr) {
		    	
		        $outErrTxt  = NULL;
		        $outExtra   = NULL;
		        $tmpSqlval  = $tmpiarr[0];
		        $tmpSqlError= $tmpiarr[1];
				$tmpGName   = $tmpiarr[2];
		        $tmpFileVal = $valarr[$pos];
		        
		        if ($tmpFileVal != $tmpSqlval) {
					if (substr($tmpFileVal,0,1)=="[") $outExtra = " <font color=gray>".$tmpGName."</font>";
					else $outExtra = " <font color=gray>[".$tmpSqlval."]</font>";
		        }
				if ($tmpSqlError!="") $outErrTxt = " <font color=red>Error:</font> <font color=gray>".$tmpSqlError."</font>";
		        
		        echo "<td>".$tmpFileVal. $outExtra .$outErrTxt. "</td>";
		    }
		    reset($rowInfo);
		    echo "</tr>\n";
		     
		    $i++;
		}
		echo "</table>\n\n\n"; 
		echo "<br>";
		$iopt = NULL;
		$iopt["width"] = "300";
		htmlInfoBox( "Summary", "", "open", "INFO", $iopt );
		echo "<table cellpadding=0 cellspacing=0 border=0>";
		echo "<tr><td>Lines:&nbsp;</td><td><B>$i</B></td></tr>\n"; 
		echo "<tr><td>Good lines:&nbsp;</td><td><font color=green><B>".$allOkSum."</B></font></td></tr>\n";
		echo "<tr><td>Errors:&nbsp;</td><td>";
		if ($allErrSum) echo "<font color=red><B>$allErrSum</B></font>";
		else  echo "0"; 
		echo "</td></tr></table>\n";
		htmlInfoBox( "", "", "close" ); 
		
		echo "<br>";
		if ($allErrSum)  this_help($tablename, $infox["nicobject"]);
	}
	
	

}

// --------------------------------------------
// TBD: put all functions to the class !

class gObjImp_static {
    
    static function destTabData() {
        $tabDat = array (
            "EXP"=>1,
            "CONCRETE_PROTO"=>1,
            "W_WAFER"=>1,
            "CONCRETE_SUBST"=>1,
            "CHIP_READER"=>1
        );
        return ($tabDat);
    }
    
    /**
     * $userfile_size, $userfile, $userfile_name, $userfile_type
     * @return string
     */
    static function getFile( $filearr ) {
        global  $error;
        
        $userfile_size = $filearr['size'];
        $userfile = $filearr['tmp_name'];
        $userfile_name = $filearr['name'];
        $userfile_type = $filearr['type'];
        
        $FH = fopen($userfile, 'r');
        if ( !$FH ) {
            $tmptxt  = " userfile_size:$userfile_size userfile:$userfile <br>";
            $tmptxt .= " userfile_name:$userfile_name userfile_type:$userfile_type <br>";
            $error->set("this_getFile()", 1, "HTTP-Upload failed: ".$tmptxt );
            
        } else {
            fclose($FH);
            $tmpfile=$_SESSION['globals']['work_path']."/".session_id().".imptmp";
            if (!move_uploaded_file($userfile, $tmpfile))  {
                $error->set("this_getFile()", 2, "Internal-Copy to temporary file failed!");
            }
            $filename = $tmpfile;
        }
        return ( $filename );
    }
	
	/**
	 *
	 * @param object $sql
	 * @param int $objid
	 * @param string $tabelname
	 * @return array $abstract_proto
	 *   $abstract_proto[STEP_NO] = abs_proto_id ; for $tabelname (CONCRETE_PROTO, CONCRETE_SUBST): STEP_NO=1
	 */
	static function getaProtos(&$sql, $objid, $tabelname) {
	
		$aProtoArr 	 = NULL;
		$a_proto_tmp = NULL;
		$tmpprotoid  = "";
	
		switch ($tabelname) {
	
			case "CONCRETE_PROTO":
				$sql->query("select c.abstract_proto_id from concrete_proto c ".
						"where c.concrete_proto_id=".$objid);
				$sql->ReadRow();
				$retid   = $sql->RowData[0];
				if ($retid)
					$a_proto_tmp[1] = $retid;
					
				break;
					
			case "CONCRETE_SUBST":
			    
			    $suc_cs_lib = new oCS_HAS_PR_subs();
			    $suc_cs_lib->setObj($objid);
			    $proto_log = $suc_cs_lib->getProtoLog($sql);
// 			    if (sizeof($proto_log)) {
// 			        $rowtmp = current($proto_log);
// 			        $a_proto_tmp[1] = $rowtmp['ap'];
// 			    } else {
			        
		        // get from SUA
		        $suc_feats = glob_elemDataGet3( $sql, 'CONCRETE_SUBST', array('CONCRETE_SUBST_ID'=>$objid),
		            array('ABSTRACT_SUBST_ID') );
		        $sua_id = $suc_feats['ABSTRACT_SUBST_ID'];
		        
		        $sua_cs_lib = new oAS_HAS_PR_subs($sua_id);
		        $proto_log  = $sua_cs_lib->getProtoLog($sql);
		        if (sizeof($proto_log)) {
		            foreach($proto_log as $rowtmp) {
		                $a_proto_tmp[$rowtmp['or']] = $rowtmp['ap'];
		            }  
		        }
			    
				break;
					
			case "CHIP_READER":
				$sqls= "select c.CONCRETE_PROTO_ID  from CHIP_READER c where CHIP_READER_ID=".$objid;
				$sql->query("$sqls");
				$sql->ReadRow();
				$cProtoid = $sql->RowData[0];
					
				if ($cProtoid) {
					$sqls= "select ABSTRACT_PROTO_ID from CONCRETE_PROTO where CONCRETE_PROTO_ID=".$cProtoid;
					$sql->query("$sqls");
					$sql->ReadRow();
					$tmpprotoid = $sql->RowData[0];
				}
					
				if ($tmpprotoid)
					$a_proto_tmp[1] = $tmpprotoid;
					
				break;
					
			case "EXP":
				$sqls= "select e.STEP_NO, c.ABSTRACT_PROTO_ID from EXP_HAS_PROTO e, CONCRETE_PROTO c where e.EXP_ID=".$objid. " AND
					e.CONCRETE_PROTO_ID=c.CONCRETE_PROTO_ID ORDER by STEP_NO";
				$sql->query("$sqls");
				while ( $sql->ReadRow() ) {
					$a_proto_tmp[$sql->RowData[0]] = $sql->RowData[1];
				}
				break;
	
			case "W_WAFER":
				$sqls= "select e.STEP_NR, c.ABSTRACT_PROTO_ID from W_WAFER_STEP e, CONCRETE_PROTO c where e.W_WAFER_ID=".$objid. " AND
					e.CONCRETE_PROTO_ID=c.CONCRETE_PROTO_ID ORDER by STEP_NR";
				$sql->query("$sqls");
				while ( $sql->ReadRow() ) {
					$a_proto_tmp[$sql->RowData[0]] = $sql->RowData[1];
				}
				break;
	
		}
			
			
		if ( sizeof($a_proto_tmp) ) {
			// get names
			foreach( $a_proto_tmp as $tmp_step=>$tmp_abs_proto) {
				$sqls= "select NAME from ABSTRACT_PROTO where ABSTRACT_PROTO_ID=".$tmp_abs_proto;
				$sql->query("$sqls");
				$sql->ReadRow();
				$name = $sql->RowData[0];
				$aProtoArr[$tmp_abs_proto] = $name;
			}
		}
			
	
		return ($aProtoArr);
	}
	
	static function getaProtoName( &$sql, $aprotoid) {
	    $sqls= "select NAME from ABSTRACT_PROTO where ABSTRACT_PROTO_ID=".$aprotoid;
	    $sql->query("$sqls");
	    $sql->ReadRow();
	    $name = $sql->RowData[0];
	    return ($name);
	}
}


function this_errorEnd($text, $notes=NULL) {
       htmlErrorBox( "Error", $text, $notes);
       htmlFoot();
}
function this_info ($key, $text, $notes=NULL ) {
    // FUNCTION: print out info text
    if ($notes!="")  $notes = " &nbsp;<I>".$notes."</I>";
    echo "<font color=gray>".$key.":</font> <B>".$text."</B>".$notes."<br>\n";

}  
























