<?php
/**
 * import CSV file and create new protocols 
 *    
 * @package o.CONCRETE_PROTO.impcrea.inc
 * @swreq UREQ:13015 Import and Create full protocols
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
   @param $_FILES['userfile'] file
 * @param $go 
  	0
  	1 : get file
  	2 : prepare
  	3 : update
 * @param 
    $parx["apid"]   : ABSTRACT_PROTO_ID
    $parx['projid'] : destination project-ID
   
  FORMAT of userfile: TAB-separated columns

   # comment
   # comment
   [NAME] [EXEC_DATE] [NOTES]	STEP_NR:[SUBST, QUANT, NOTES, INACT] - can be repeated 
   ... data ...
  EXAMPLE:
# protocol (abstract): Abbott Measurement 01 ID:16337					
# concrete protocol id	Proto-Date	Operator	blood sample	HIV_NEG Log(cp/ml)	HIV_LOPOS Log(cp/ml)
NAME	EXEC_DATE	3:NOTES	4:SUBST_ID	8:QUANT	9:QUANT
XXX1	2017-09-23	Bert	231297	2	3
XXX2	2017-09-21	Bert2	231297	3	3
XXX3	2017-09-26	Bert4	231297	4	3
 * @version $Header: trunk/src/plugin/o.CONCRETE_PROTO.impcrea.inc 59 2018-11-21 09:04:09Z $
 */
require_once('f.upload.inc');
require_once ("insertx.inc");
require_once ('class.filex.inc');
require_once ('o.PROTO.subs.inc');
require_once ('o.PROJ.addelems.inc');
require_once ("f.objview.inc");	
require_once ('o.PROTO.upsteps.inc');



class this_import_GUI {
	
	function __construct(&$_html) {
		$this->_html = &$_html;
	}

	function formshow(&$sqlo, $parx) {

		global  $tablename;
		$initarr   = NULL;
		$initarr["title"]       = "1. Import parameter file";
		$initarr["submittitle"] = "Next &gt;&gt;";
		$initarr["tabwidth"]    = "AUTO";
		$initarr["ENCTYPE"]     = "multipart/form-data";
		$initarr["dblink"]      = 1;

		$hiddenarr = NULL;
		// $hiddenarr["tablename"]     = $tablename;

		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);

		$fieldx = array ("title" => "Parameter file", "name"  => "userfile", "namex" => TRUE,
				"colspan"=>2, "val"   => "", "notes" => "CSV file", "object" => "file" );
		$formobj->fieldOut( $fieldx );

		
		$fieldx = array (
				"title" => "Protocol (abstract)",
				"name"  => "apid",
				"val"   => "",
				"notes" => "",
				"req"   => 1,
				"object" => "dblink",
				"inits" => array( 'table'=>'ABSTRACT_PROTO', 'getObjName'=>1, 'sqlo'=>&$sqlo, 'pos' =>'0', 'projlink'=> 1),
		);
		$formobj->fieldOut( $fieldx );
		
		$fieldx = array (
				"title" => "Destination project",
				"name"  => "projid",
				"val"   => "",
				"req"   => 1,
				"notes" => "protocols are stored in this project",
				"object" => "dblink",
				"inits" => array( 'table'=>'PROJ', 'getObjName'=>1, 'sqlo'=>&$sqlo, 'pos' =>'1', 'projlink'=> 1),
		);
		$formobj->fieldOut( $fieldx );


		$formobj->close( TRUE );

	}
	
	function form2($parx) {
		global  $tablename;
	
	
		$initarr   = NULL;
		$initarr["action"]      = $_SERVER['PHP_SELF'];
		$initarr["title"]       = "Prepare database update";
		$initarr["goNext"]	    = "2";
		$initarr["submittitle"] = "Update now";
		$initarr["tabwidth"]    = "AUTO";
	
		$hiddenarr["parx[projid]"]   = $parx['projid'];
		$hiddenarr["parx[apid]"]     = $parx['apid'];
	
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
		$formobj->close( TRUE );
		echo "<br>";
	}
	
	function show_header($headerArr, $headerRaw) {
		echo "<table cellpadding=1 cellspacing=1 border=0>";
		echo "<tr bgcolor=#D0D0D0><th></th><th></th>";
		$cnt = 0;
		foreach( $headerArr as $headPos=>$tmparr) {
			
			echo "<th>".$headerRaw[$cnt];
			echo "</th>";
			$cnt++;
		}
		reset($headerArr);
		echo "</tr>\n";
		
		/*
		echo "<tr bgcolor=#D0D0D0><th></th><th></th>";
		$cnt = 0;
		foreach( $headerArr as $headPos=>$tmparr) {
				
			echo "<td>".print_r($tmparr,1);
			echo "</td>";
			$cnt++;
		}
		reset($headerArr);
		echo "</tr>\n";
		*/
		
	}
	
	function help() {
	
		echo "<br>";
		htmlInfoBox( "Help", "", "open", "HELP" );
		?>
	    <ul>                    
	    <li> imports CSV-file
		<li> create protocols and protocol-steps parameters</li>
		<li> currently no check, if protocol already exists in DB</li>
		<li> Format:</li>
	    <pre>
	- # Some comment at beginning ...    
	- Header: ... contains protocol features and step parameters
	   [NAME] [EXEC_DATE] STEP_NR:[SUBST, QUANT, NOTES, INACT] - can be repeated
	   ... step parameters ... 
	      STEP_NR: step_nr of a protocol step 
	      SUBST:   name or ID of a substance (concrete); give brackets around the ID; e.g. <font color=blue>[73748]</font></li>
	      SUBST_ID:explicitly an ID of a substance (concrete)
	      QUANT:   number
	      NOTES:   string (no TABs!) 
	      INACT:   0|1  
	- Special values:
		  [NULL]: is an empty ID-value for EXP_ID of SUBST_ID (TBD: not yet implemented!)
		  </pre>
	- example 1: mixed names and IDs for experiments and substances<br>
	  	<TABLE CELLSPACING=0 COLS=4 RULES=GROUPS BORDER=1>
		<COLGROUP><COL WIDTH=47><COL WIDTH=84><COL WIDTH=66><COL WIDTH=74><COL WIDTH=66></COLGROUP>
		<TBODY>
			<TR>
				<TD WIDTH=47 HEIGHT=17 ALIGN=LEFT>EXEC_DATE</TD>
				<TD WIDTH=84 HEIGHT=17 ALIGN=LEFT>    1:SUBST</TD>
				<TD WIDTH=66 HEIGHT=17 ALIGN=LEFT>1:QUANT</TD>
				<TD WIDTH=74 HEIGHT=17 ALIGN=LEFT> 2:NOTES</TD>
				<TD WIDTH=74 HEIGHT=17 ALIGN=LEFT> 4:QUANT</TD>
			</TR>
			<TR>
				<TD WIDTH=47 HEIGHT=17 ALIGN=LEFT>2017-05-12</TD>
				<TD WIDTH=84 HEIGHT=17 ALIGN=LEFT>DNA_samp6</TD>
				<TD WIDTH=66 HEIGHT=17 >2.3</TD>
				<TD WIDTH=74 HEIGHT=17 ALIGN=LEFT>jamei isset</TD>
				<TD WIDTH=66 HEIGHT=17 >2.99</TD>
			</TR>
			<TR>
				<TD WIDTH=47 HEIGHT=17 ALIGN=LEFT>2017-05-13</TD>
				<TD WIDTH=84 HEIGHT=17 ALIGN=LEFT>DNA_samp7</TD>
				<TD WIDTH=66 HEIGHT=17  >5.6</TD>
				<TD WIDTH=74 HEIGHT=17 ALIGN=LEFT>hossa</TD>
				<TD WIDTH=66 HEIGHT=17 >2.9</TD>
			</TR>
			<TR>
				<TD WIDTH=47 HEIGHT=17 ALIGN=LEFT>2017-05-12</TD>
				<TD WIDTH=84 HEIGHT=17 ALIGN=LEFT>DNA_samp7</TD>
				<TD WIDTH=66 HEIGHT=17  >5.6</TD>
				<TD WIDTH=74 HEIGHT=17 ALIGN=LEFT>hossa</TD>
				<TD WIDTH=66 HEIGHT=17 >7.9</TD>
			</TR>

		</TBODY>
	</TABLE>
	<br>
	
	- example 2: just with IDs<br>
	  	<TABLE CELLSPACING=0 COLS=4 RULES=GROUPS BORDER=1>
		<COLGROUP><COL WIDTH=47><COL WIDTH=84><COL WIDTH=66><COL WIDTH=74></COLGROUP>
		<TBODY>
			<TR>
				<TD WIDTH=47 HEIGHT=17 ALIGN=LEFT>EXEC_DATE</TD>
				<TD WIDTH=84 HEIGHT=17 ALIGN=LEFT>3:SUBST_ID</TD>
				<TD WIDTH=66 HEIGHT=17 ALIGN=LEFT>4:QUANT</TD>
				<TD WIDTH=74 HEIGHT=17 ALIGN=LEFT>4:INACT</TD>
			</TR>
			<TR>
				<TD WIDTH=47 HEIGHT=17 ALIGN=LEFT>2017-05-12</TD>
				<TD WIDTH=84 HEIGHT=17 ALIGN=LEFT>93921</TD>
				<TD WIDTH=66 HEIGHT=17 >2.3</TD>
				<TD WIDTH=74 HEIGHT=17 ALIGN=LEFT>0</TD>
			</TR>
			<TR>
				<TD WIDTH=47 HEIGHT=17 ALIGN=LEFT>2017-05-12</TD>
				<TD WIDTH=84 HEIGHT=17 ALIGN=LEFT></TD>
				<TD WIDTH=66 HEIGHT=17  ></TD>
				<TD WIDTH=74 HEIGHT=17 ALIGN=LEFT>1</TD>
			</TR>
			<TR>
				<TD WIDTH=47 HEIGHT=17 ALIGN=LEFT>2017-05-12</TD>
				<TD WIDTH=84 HEIGHT=17 ALIGN=LEFT>93945</TD>
				<TD WIDTH=66 HEIGHT=17  >5.6</TD>
				<TD WIDTH=74 HEIGHT=17 ALIGN=LEFT>0</TD>
			</TR>
		</TBODY>
	</TABLE>
	
	
	
	    <?
	    htmlInfoBox( "", "", "close");
	    
	}   
}


/**
 * manage file data, just get a line the the header info
 * @author steffen
 *
 */
class this_FileManage {
	
	/**
	 * 'apid' : abstract protocol ID
	 * 
	 * @var unknown
	 */
	private $config_array;
	
	/**
	 * 
	 * @param unknown $tablename
	 * @param unknown $config_array
	 */
	function __construct($tablename, $config_array) {
		$this->dataStartLine=0; // first line=1
		$this->headStartLine=0;
		$this->tablename = $tablename;
		$this->config_array = $config_array;
	}

	function setFilename($filename) {
		$this->filename=$filename;
	}
	
	function _getHeaderLine(  ) {
		$lineCnt=0;
		$line   = trim(fgets($this->FH, 8000)); // trim !!!
		$lineCnt=1;
		while ( (substr($line,0,1)=="#") AND !feof ( $this->FH ) ) { // ignore commments
			$line  = trim(fgets($this->FH, 8000)); // trim !!!
			$lineCnt++;
		}
		$this->headStartLine=$lineCnt;
		return ($line);
	}
	
	/**
	 * while ( $this->get_line() )
	 */
	function get_line() {
		if ( !feof( $this->FH ) ) {
			$line   = rtrim(fgets($this->FH, 8000));
			return $line;
		} else {
			return FALSE;
		}
		 
	}
	
	
	/**
	 * parse header  
	 * - ignore '#' comments in the first lines
	 * @return $headerPos  : array[POS] = array( $type, $step_nr, $ColCode, $tmpErrTxt, $tmpIsID)
	 *     $type: 'feat' or 'step' 
	 *     $ColCode from ABSTARCT_PROTO_STEP or ABSTRACT_PROTO
	 *    
	 */
	function parseFileHeader() {
		global $error;
		$FUNCNAME= $this->CLASSNAME.':parseFileHeader';
		 
		$tablename = $this->tablename;
		$filename  = $this->filename;
	
		$protoTranslate = NULL;
		$protoTranslate["SUBST"]    = "CONCRETE_SUBST_ID";
		$protoTranslate["SUBST_ID"] = "CONCRETE_SUBST_ID";
		$protoTranslate["DEV"]      = "DEV_ID";
		$protoTranslate["DEV_ID"]   = "DEV_ID";
		$protoTranslate["QUANT"]    = "QUANTITY";
		$protoTranslate["NOTES"]    = "NOTES";
		$protoTranslate["INACT"]    = "NOT_DONE";
		 
		$allowed_feat_cols = array(
			'NAME',
			'EXEC_DATE',
			'NOTES'
		);
	
		$protoColsKeys = array_keys($protoTranslate);
		$protoColsImplode = implode(", ",$protoColsKeys);
	
	
		$this->FH = fopen($filename, 'r');
		if ( !$this->FH ) {
			$error->set($FUNCNAME, 1, "Could not open file '$filename'");
			return;
		}
		 
		$line  = $this->_getHeaderLine( );
	
		if (feof ( $this->FH )) {
			$error->set($FUNCNAME, 12, "no header line found");
			return;
		}
		$headArr1 = explode("\t",$line);
		$tmperr   = NULL;
		 
	
		if (!sizeof($headArr1)) {
			$error->set($FUNCNAME, 2,"no TAB-character found in header");
			return;
		}
	
		$cnt=0;
		$headerPos = NULL;
	
		foreach( $headArr1 as $dummy=>$headVal) {
	
			$coltype = NULL;
			$headVal = trim($headVal);
			$tmperr  = NULL;
			$tmpIsID = 0;
			$tmpStepnr = 0;
			$tmpColCode = NULL;
				
			do {
				
				if (in_array($headVal, $allowed_feat_cols)) {
					$coltype='feat';
					$tmpColCode = $headVal; // set the column
					break;
				}
				
				$posColon = strpos($headVal,":");
				if (!$posColon) {
					$tmperr = array(2," column not known");
					break;
				}
				$tmpStepnr = substr( $headVal,0, $posColon);
				if (!is_numeric($tmpStepnr))  {
					$tmperr=array(3," step_nr '$tmpStepnr' is not a number");
					break;
				}
	
				$coltype='step';
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
			$headerPos[$cnt] = array ( $coltype, $tmpStepnr, $tmpColCode, $tmpErrTxt, $tmpIsID);
	
			$cnt++;
		}
		reset($headArr1);
		 
		if ($errGot) {
			$error->set($FUNCNAME, 10, "Errors got during header parsing");
		}
		return  array($headerPos, $headArr1);
	}
}

/**
 * manage a single file line
 * @author steffen
 *
 */
class this_single_line {
	
	function __construct(&$sqlo, $go, $headerArr, $config_array) {
		$this->headerArr = $headerArr;
		$this->go = $go;
		$this->config_array = $config_array;
		
		if (!$this->config_array['apid']) {
			sys_error_my( 'input: apid missing.' );
		}
		if ( !$this->config_array['projid'] ) {
			sys_error_my( 'input: projid missing.' );
		}
		
		$this->proj_lib = new oProjAddElem( $sqlo, $this->config_array['projid'] );
		
		$this->protoUpdLib = new oConcProtoUpSteps();
	}
	
	static function objCheck( &$sql,
			$tablename,
			$objName, 	// NAME or if $objName is sandwiched by "[" "]" is an ID
			$opt = NULL // "like",
			// "isID" => 1 : $objName is strictly an ID
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
	 * return nice SQL-parameters
	 RETURN:
	 * @param unknown $sql
	 * @param array $valarr  ($pos => VALUE )
	 * @param unknown $headerArr
	 * @param string $option
	 * @return  array of ($step_arr, $feature_array, $rowInfo)
	    $step_arr[$pos] = array ( SQL-value, $error_notes, $nice_notes );
		$feature_array
		
	 */
	function checkParamsLine( &$sql, $valarr,$option=NULL) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;

		$headerArr = $this->headerArr;
		$pos           = 0;
		$step_arr 	   = array();
		$feature_array = array();
		$errorOccurred = 0;
		$error_notes = NULL;
		$nice_notes  = NULL;
		$errorLastNotes = NULL;
			
		

		foreach($headerArr  as $pos => $headTmpArr ) {

			$type    = $headTmpArr[0];
			$step_nr = $headTmpArr[1];
			$tmpCode = $headTmpArr[2];
			$tmpisId = $headTmpArr[4];
			$tmpVal  = trim($valarr[$pos]) ;

			$error_notes = NULL;
			$nice_notes  = NULL;
			$store_val      = NULL;
			$cheOption 	 = NULL;
			if ($tmpisId) $cheOption["isID"]=1;

			if ($tmpVal!="") {
				
				if ($type=='feat') {
					$feature_array[$tmpCode] = $tmpVal;
					$store_val = $tmpVal;
					$nice_notes = $store_val;
				}
				if ($type=='step') {
						
					switch ($tmpCode) {
				
						case "CONCRETE_SUBST_ID":
							list($objok, $obj2Error, $substId,  $substGname ) = self::objCheck( $sql, "CONCRETE_SUBST", $tmpVal, $cheOption );
							if ($objok<0) {
								$error_notes = $obj2Error;
								break;
							}
							$store_val = $substId;
							$nice_notes = $substGname;
							break;
						case "DEV_ID":
							list($objok, $obj2Error, $substId,  $substGname ) = self::objCheck( $sql, "CHIP_READER", $tmpVal, $cheOption );
							if ($objok<0) {
								$error_notes = $obj2Error;
								break;
							}
							$store_val = $substId;
							$nice_notes = $substGname;
							break;
						case "QUANTITY":
							if (!is_numeric($tmpVal)) $error_notes = "not numeric";
							$store_val = $tmpVal;
							break;
						case "NOTES":
							$store_val = $tmpVal;
							break;
						case "NOT_DONE":
							if ($tmpVal!="0" AND $tmpVal!="1") {
								$error_notes = "must be 0 or 1 or 2";
								break;
							}
							$store_val = $tmpVal;
							break;
					}
					$step_arr[$step_nr][$tmpCode] = $store_val; // order by STEP_NR
				}
				if ($error_notes!="") {
					$errorOccurred  = 1;
					$errorLastNotes = $error_notes;
				}
				
				
			}
			$rowInfo[$pos]   = array ($store_val, $error_notes, $nice_notes);
		}

		if ($errorOccurred) $error->set(FUNCNAME, 1, $errorLastNotes);
		return array($step_arr, $feature_array, $rowInfo, $expId);
	}
	
	/**
	 * update a concrete_proto with steps in $stepArr
	 */
	private function protoUpdate( 
		&$sql, 
		$cprotoId, 
		$aProtoId, 
		$stepArr 	// array ( step_nr => $colValArr)
		) {
	
		$this->protoUpdLib->setProto($cprotoId);
		$this->protoUpdLib->protoUpdate( $sql, $aProtoId, $stepArr );
	
	}
	
	function handle_line($sqlo, $valarr) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$go = $this->go;
		$objok = 0;
		
		do {
		
			list( $proto_steps, $feature_array, $rowInfo ) = $this->checkParamsLine( $sqlo, $valarr );
			if ($error->Got(READONLY)) { // remove error from stack ...
				$errLast   = $error->getLast();
				$error_txt = $errLast->text;
				$error->reset();
				$objok    = -2;
				$objError = "checkParams:".$error_txt;
				break;
			}
			
			
		
			if ( $go == 2 ) {

				$insertlib = new insertC();
				
				$feature_array['ABSTRACT_PROTO_ID'] = $this->config_array['apid'];
			
				$args= array( "vals"=>$feature_array );
				$cpid = $insertlib->new_meta($sqlo, 'CONCRETE_PROTO', $args);
				if (!$cpid) {
					$error_txt = $error->getAllAsText();
					$error->reset();
					$objok = -3;
					$objError = "Protocol-creation failed: ".$error_txt;
					break;
				}
				
				$this->proj_lib->addObj( $sqlo, 'CONCRETE_PROTO', $cpid  );
			
				$this->protoUpdate( $sqlo,  $cpid, $this->config_array['apid'], $proto_steps );
				if ($error->Got(READONLY))  {
					$error_txt = $error->getAllAsText();
					$error->reset();
					$objok = -4;
					$objError = "ProtocolSteps:".$error_txt;
					break;
				}
			}
		
		} while (0);

		
		if ($objok<0) {
			$error->set( $FUNCNAME, $objok, $objError );
		}
		
		return  $rowInfo;
	}
}
	

class o_CONCRETE_PROTO_impcrea_XPL extends gPlugin {
	
	
	function register() {
		$tablename	= 'CONCRETE_PROTO';
		$title_sh	= 'Import and Create full protocols';
		$title		= 'Import CSV file; create protocols and steps';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title_sh']    = $title_sh;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['help_url'] = 'o.exp.imp_sample.html';
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read', 'insert'), 'obj'=>array() )
				);
		
	}
	
	/**
	 * main file loop
	 */
	function doMain(&$sqlo, &$file_lib, $headerArr, $parx, $config_array) {
		//
		// START import
		//
		global $error;
		$FUNCNAME= 'doMain';
	
		$filename   = $this->filename;
		$tablename	= 'CONCRETE_PROTO';
		$go = $this->go;
		$this->config_array = $config_array;
	
	
		$i         = 0;
		$allErrSum = 0;
		$allOkSum  = 0;
	
		$copt = NULL;
		$copt["table"] = $tablename;
		if ($parx["expunexact"]) $copt["like"] = 1;
		$line_lib = new this_single_line($sqlo, $go, $headerArr, $this->config_array);
	
		while( ($line=$file_lib->get_line()) !== false ) {
	
			if ($line=="") continue;
		
			$valarr = explode("\t",$line);
			$objok      = 1;
			$objId      = "";
			$objError   = "";
			$objErrorSum= "";
			

			$rowInfo = $line_lib->handle_line($sqlo, $valarr);
			if ($error->Got(READONLY))  {
				$objok = -1;
				$errLast   = $error->getLast();
				$objError = $errLast->text;
				$error_id  = $errLast->id;
			}
	
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
		// if ($allErrSum)  this_help($tablename, $infox["nicobject"]);
	}
	
	private function GoInfo($go, $coltxt=NULL) {
		require_once ("func_form.inc");
		$goArray   = array( "0"=>"Upload file", 1=>"Prepare Import", 2=>"Create protocols" );
		
		$extratext = '[<a href="p.php?mod='.$this->_mod.'">Start again</a>]';
		
		$formPageLib = new FormPageC();
		$formPageLib->init( $goArray, $extratext );
		$formPageLib->goInfo( $go ); 
	
	}

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		$tablename	= 'CONCRETE_PROTO';
		$csv_file_short = 'file.csv';
		$scriptname = str_replace("/","_",$this->_mod);
		
		$go   = $_REQUEST['go'];
		$this->go=$go;
		$parx   = $_REQUEST['parx'];
		$sqlo = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		$guilib = new this_import_GUI($this->_html);
		
		if ( !$go ) {
			echo "Get file header format for the current protocol: [<a href=\"obj.concrete_proto.infimp.php?tablename=".$tablename."\">Used steps</a>] \n";
			echo "[<a href=\"obj.concrete_proto.infimp.php?tablename=".$tablename."&parx[usesteps]=all\">ALL steps</a>]\n";
			echo '<br />'."\n";
		}
		
		echo "<ul>";
		$this->GoInfo($go);
		
		if ( !$go ) {
			$guilib->formshow($sqlo, $parx);
			$guilib->help($tablename, $infox["nicobject"]);
			$this->htmlFoot();
		}
		
		if ($parx['apid']==NULL) {
			$this->htmlFoot('USERERROR', 'no protocol given');
		}
		
		
		
		if ($parx['projid']==NULL) {
			$this->htmlFoot('USERERROR', 'no Project given');
		}
		$objLinkLib = new fObjViewC();
		
		$html_tmp = $objLinkLib->bo_display( $sqlo, 'ABSTRACT_PROTO', $parx['apid'] );
		echo 'Protocol (abstract):'.$html_tmp."<br>";
		$html_tmp = $objLinkLib->bo_display( $sqlo, 'PROJ', $parx['projid'] );
		echo 'Destination project:'.$html_tmp."<br>";
		
		$proj_lib = new oProjAddElem( $sqlo, $parx['projid'] );
		if ($error->Got(READONLY)) {
			return;
		}
		
		$config_array=array(
			'apid'=> $parx['apid'],
			'projid'=> $parx['projid']	
		);
		
		
		$file_lib = new this_FileManage($tablename, $config_array);
		
		if ( $go == 1 ) {
			$upload_lib = new uploadC();
			$filename = $upload_lib->mvUpload2Tmp($scriptname, $csv_file_short, 
					$_FILES['userfile']['tmp_name'] , $_FILES['userfile']['name']);
			if ($error->Got(READONLY)) {
				return;
			}
			if (!file_exists($filename))  {
				$error->set( $FUNCNAME, 1, 'Uploaded file not found.' );
				return;
			}
			
			$topt = NULL;
			$file_lib->setFilename($filename);
			list($headerArr, $headerRaw) = $file_lib->parseFileHeader();
			if ($error->Got(READONLY))  {
				return;
			}
			
			$guilib->form2($parx);
			
		}
		if ($go==2) {
			$upload_lib = new uploadC();
			$filename = $upload_lib->getFileFromTmp($scriptname, $csv_file_short);
			if (!file_exists($filename))  {
				$error->set( $FUNCNAME, 1, 'Uploaded file not found.' );
				return;
			}
			$file_lib->setFilename($filename);
			list($headerArr, $headerRaw) = $file_lib->parseFileHeader();
			if ($error->Got(READONLY))  {
				return;
			}
		}
		$this->filename = $filename;
		$guilib->show_header($headerArr, $headerRaw);
		
		//echo "OOOOK<br>";
		$this->doMain($sqlo, $file_lib, $headerArr, $parx, $config_array);
	}
	
}