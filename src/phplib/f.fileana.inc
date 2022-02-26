<?php
require_once ('f.upload.inc');

/**
 * analyse an uploaded file (CSV-format)
 * FUTURE: please use instead:
 *    - f.read_file_meta.inc  - read CSV, EXCEL files into raw arrays
 *    - f.spreadsheet_ana.inc - interprete SPREADSHEETs : header analysis and so on
 *    
 * @namespace core::misc::import
 * @package f.fileana.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 * @example
    $colsExpect = array (
      "id1"   => ('col.name'=>"ID", 'req'=>2),
      "name1" => ('col.name'=>"NAME", 'req'=>1),
     );
    $uplib = new fFileanaC();
    $uplib->setHeaderParms($colsExpect);
	$FH = $uplib->openFile( $sql, $uploadFile );
	if ( $error->Got(READONLY) ) {
		$error->set( $FUNCNAME, 1, "File-Header analysis failed.");
		return;
	}
	while( !feof ( $FH ) ) { 
	
		$line = fgets($FH, $LINE_LENGTH); 
		$dataarr = $uplib->getLineArr( $line );
		 OR
		$dataarr = $uplib->getLineArrByCol( $line );
	}	
	
 * see also UPLOAD support for FORM: 
        
     $uploadObj = new uploadC();
     $scriptid = __CLASS__;
     $workFileName = 'datafile.dat';
        
     // form: GO=1
    
     $datafileFull  =  $uploadObj->mvUpload2Tmp(
     $this->scriptid, $workFileName, $_FILES['datafile']['tmp_name'], $_FILES['datafile']['name']);
     $workDirFull = $uploadObj->getWorkDir();
     
     // form: GO=2
     $workDirFull   = $uploadObj->set_existing_workdir( $scriptid);
     $datafileFull =  $workDirFull . DIRECTORY_SEPARATOR . $workFileName;
     
     
     
 * @version $Header: trunk/src/phplib/f.fileana.inc 59 2018-11-21 09:04:09Z $
 */
class fFileanaC {

    private $linecount; // dataline count
    var $headerLines;   // line-no of start of header
    var $headerPos;     // array[column-KEY] = pos
    var $headerArr;     // array[POS] = NAME
    var $comment_lines; // array of comment lines
    private $header_raw_arr; // RAW header array
    var $delimiter;
    
    /**
     * expected columns
     * @var array $colsExpect colsExpect_STRUCT array of column-KEY => array()
     *   
     *   'col.name' => name of the column in FILE
	     "req":  1: optional
	             2: need
     * 
      "id1"   => ('col.name'=>"ID", 'req'=>2),  # col.name: the column name
      "name1" => ('col.name'=>"NAME", 'req'=>1),
     */
    var $colsExpect; 
    var $colsExpect_N2K; // column file column-KEY => KEY 
    var $options;    /**
    	'headerPattern'=> string : OPTIONAL: search the header STARTING with this pattern
    */
    private $uploadFile; // the uploaded file

/**
 * 
 * @param string $uploadFile
 */
function __construct($uploadFile=NULL) {
	$this->delimiter = "\t";
	$this->comment_lines = array();
    $this->uploadLib = new uploadC();
    
    $this->uploadFile = $uploadFile;
}

/**
 * set header
 * @param  array $colsExpect colsExpect_STRUCT
 *    
 * @global $this->colsExpect		        
 * @return -
 */
function setHeaderParms($colsExpect) {
		
	$this->colsExpect = $colsExpect;
	$this->colsExpect_N2K = array();
	if (sizeof($colsExpect)) {
	    foreach($this->colsExpect as $key => $row) {
	        $this->colsExpect_N2K[$row['col.name']] = $key;
	    }
	}
}

function setDelimiter($delimiter) {
	$this->delimiter = $delimiter;
}

/**
 * get first Header line 
 * get  $this->comment_lines
 * @return string $line header-line
 */
function _getHeaderLine( &$FH ) {
	
    $this->comment_lines = array();
	$this->linecount = 1;

	$line   = fgets($FH, 8000); // no trimming of header line !
	
	while ( (substr($line,0,1)=="#") AND !feof ( $FH ) ) { // ignore commments    
	    $this->comment_lines[] = $line;
		$line  = fgets($FH, 8000);      // no trim !!!
		$this->linecount++;
	}
	
	$this->headerLines = $this->linecount;
	
	$this->linecount++;
	
	if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
	    echo "DEBUG: Comment_lines: ".print_r($this->comment_lines,1)."<br>\n";
    }
	
	return ($line);
}

/**
 * get first Header line by searching first occurence of $pattern in the line
 * - do NOT trim the output line !
 * @return string $line header-line
 */
function _getHeaderLinePatt( &$FH, $pattern ) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
	$this->linecount = 1;

	// do the search
	$found=0;
	$patlen=strlen($pattern);
	while ( !feof ( $FH ) ) {
	    
		 $line  = fgets($FH, 8000);
		 
		 if (substr($line, 0, $patlen)==$pattern) {
		     $found=1;
		     break;
		 }
		 
		 $this->linecount++;
	}
	
	if (!$found) {
	    $error->set($FUNCNAME, 1, 'no header found, starting with pattern "'.$pattern.'"');
	    return; 
	}
	
	$this->headerLines = $this->linecount;
	
	$this->linecount++;
	
	return ($line);
}

function getHeaderArr() {
	return ($this->headerArr);
}

function get_header_raw_arr() {
    return $this->header_raw_arr;
}

function getHeaderCol2Pos () {
	return $this->headerPos;
}

/**
 * - parse header 
 * - ignore '#' comments
 * - get positions of columns in $this->colsExpect
 * - $this->options['headerPattern']
 * - creates $this->headerPos 
 * @param object filePointer $FH
 * @global $this->colsExpect [IN]
 *       $this->headerPos  : array[NAME] = pos
	     $this->headerArr
 * @return -
 */
function _analyseHeader( &$FH ) { 
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$this->headerArr = NULL;
    $this->headerPos=NULL;
	
    if ($this->options['headerPattern']==NULL) {
    	$line  = $this->_getHeaderLine( $FH );
    } else {
    	$line  = $this->_getHeaderLinePatt( $FH, $this->options['headerPattern'] );
    }
    if ( feof ( $FH ) or $error->Got(READONLY) ) {
		$error->set($FUNCNAME, 1, "no header line found");
        return; 
	}
    $headArr1 = explode( $this->delimiter, $line); 
  
	if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
		echo "DEBUG: header_line: ".$line."<br>\n";
					
	}
    
	$this->header_raw_arr = $headArr1;
     
    if (!sizeof($headArr1)) {
        $error->set($FUNCNAME, 2,"data-delimiter-char not found in header");
        return;
    } 

    $pos       = 0; 	// column position
    $headerPos = NULL;  // ["key"] = pos
	$loopError = "";
	$this->headerArr = $headArr1;
	
	// get position of COLUMN
	
    foreach( $headArr1 as $dummy=>$colName) { 
		$colName    = trim($colName);
		$expect_key = $this->colsExpect_N2K[$colName];
		$isNeeded   = isset($this->colsExpect[$expect_key]);
		if ( $isNeeded ) {
		    $headerPos[$expect_key] = $pos;
		} else {
			// column not defined => ignore it
			//$loopError = "column '$colName' is unknown.";
			//break;
		}
		$pos++;
	}
	
	
	if ($loopError!="")  {
		$error->set($FUNCNAME, 4, $loopError);
        return;
	}
    
    $this->headerPos = $headerPos;
	
	// check expected columns
    foreach($this->colsExpect as $expect_key => $ex_row ) {
        $expect  = $ex_row['req'];
        $colName = $ex_row['col.name'];
        
		if ($expect==2) { // need this column ?
		    if ( !isset($headerPos[$expect_key]) ) {
				
				$error->set($FUNCNAME, 5, "Column '".$colName."' missing");
				if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
					$error->set($FUNCNAME, 5, "DEB: Expected_cols: ". glob_array2String( $this->colsExpect ).
						' Found_cols: '. glob_array2String( $headerPos) );
					
				}
        		return;
			} 
		}
	}
	
	
}

/**
 *  - read file header
	- read each line
	  - get patient info
	  - create contact
	  - create samples
 * @param  $sql
 * @param string $uploadFile
 * @param array $options
 * 	 'headerPattern'=> string : OPTIONAL: search the header for this pattern
 * @return object filePointer $FH
 */
function openFile( &$sql, $uploadFile, $options=NULL ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$this->uploadFile = $uploadFile;
	$this->options    = $options;

    //$LINE_LENGTH = 32000;
    $FH = fopen($this->uploadFile, 'r');
	if ( !$FH ) {
		$error->set( $FUNCNAME, 1, "Can't open file '".$this->uploadFile."'");
		return;
	}
	
	$this->linecount=1;
	
	$this->_analyseHeader($FH);
	if ( $error->Got(READONLY) ) {
		$error->set( $FUNCNAME, 1, "File-Header analysis failed.");
		return;
	}
	
	
	return ($FH);
}

/**
 * count data lines
 * @param $uploadFile
 * @param array $opt 'stopOnEmptyLine' : 0,1
 */
function countDataLines( $uploadFile, $opt=NULL ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	if ($uploadFile==NULL) {
		$error->set( $FUNCNAME, 1, 'Filename not initialized.');
		return;
	}

    $LINE_LENGTH = 32000;
    $FH = fopen($uploadFile, 'r');
	
	$this->_getHeaderLine( $FH );
	if ( $error->Got(READONLY) ) {
		$error->set( $FUNCNAME, 1, "File-Header analysis failed.");
		return;
	}
	
	$linecount = 0;
	while ( !feof ( $FH ) ) { // ignore commments
		 $line  = trim(fgets($FH, 32000));
		 
		 if ($line==NULL) break;
		 $linecount++;
	}
	
	fclose($FH);
	
	return ($linecount);
}

/**
 * get one line as array
 * @param string $line
 * @return array $data_array [POS] = value (trimmed)
			OR NULL
 */
function getLineArr( $line, $no_trim=0 ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
	if (trim($line)=="") { // last line: no data
		return; 
	}
	
	$dataArr1 = explode( $this->delimiter, $line );

	if (!sizeof($dataArr1)) return;
	
	$dataArr=NULL;
	// trim
	foreach( $dataArr1 as $pos=>$val) {
	    if (!$no_trim) {
		  $dataArr[$pos]=trim($val);
	    } else {
	      $dataArr[$pos]=$val;
	    }
	}
	reset($dataArr1);
	
	return ($dataArr);
}

/**
 * get line as array, index is the KEY-name of $this->colsExpect
 * column-names of header line must be unique
 * @param string $line
 * @param array $no_trim_arr array of KEYs, which have not to be TRIMMED
 * @return array(column-name)= value
 */
function getLineArrByCol( $line, $no_trim_arr=array() ) {
	$linearr = $this->getLineArr( $line );
	if (!is_array($linearr)) return;
	
	$keyValArr = NULL;
	
	foreach( $this->headerPos as $name=>$pos) {
		$keyValArr[$name] = $linearr[$pos];
	}
	
	
	if (sizeof($no_trim_arr)) {
	    // overwrite trimmed VALUEs
	    $linearr = $this->getLineArr( $line, 1 );
	    foreach( $no_trim_arr as $key) {
	        $pos = $this->headerPos[$key];
	        $keyValArr[$key] = $linearr[$pos];
	    }
	}
	
	
	return $keyValArr;
}

/**
 * check, if no data is in line, ignore white spaces
 * @param string  $line
 */
function line_is_empty($line) {
    $data = trim($line);
    $data = trim($data, $this->delimiter);
    $data = trim($line);
    
    if ($data==NULL) return 1;
    else return 0;
}

/// ---------------------

function uploadInit($scriptid, $shortFile) {
    $this->scriptid  = $scriptid;
    $this->shortFile = $shortFile;
}

function uploadFile($userfile, $userfile_name) {
	$this->fileNameFull = $this->uploadLib->mvUpload2Tmp( $this->scriptid, $this->shortFile, $userfile, $userfile_name );
}

function getFileNameFull() {
	return $this->fileNameFull;
}

function getFileFromTemp() {
	global $error;
	$FUNCNAME="getFileFromTemp";
	
	$this->fileNameFull = $this->uploadLib->getFileFromTmp( $this->scriptid, $this->shortFile);
	if (!file_exists($this->fileNameFull)) {
		$error->set( $FUNCNAME, 1, "File '".$this->shortFile."' not found on Temp-dir." );
		return;
	}
}

/**
 * string is UTF-8 ?
 * @param string $string
 * @return number 0,1
 */
private function is_utf8($string) {
    return preg_match('%(?:'
        . '[\xC2-\xDF][\x80-\xBF]'                // non-overlong 2-byte
        . '|\xE0[\xA0-\xBF][\x80-\xBF]'           // excluding overlongs
        . '|[\xE1-\xEC\xEE\xEF][\x80-\xBF]{2}'    // straight 3-byte
        . '|\xED[\x80-\x9F][\x80-\xBF]'           // excluding surrogates
        . '|\xF0[\x90-\xBF][\x80-\xBF]{2}'        // planes 1-3
        . '|[\xF1-\xF3][\x80-\xBF]{3}'            // planes 4-15
        . '|\xF4[\x80-\x8F][\x80-\xBF]{2}'        // plane 16
        . ')+%xs', $string);
}

/**
 * detect, if the data in FILE is UTF-8 or ISO-8859-1
 * @return string UTF-8 or ISO-8859-1
 */
function detect_encoding() {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    $encoding_key='ISO-8859-1';
    
    $FH = fopen($this->uploadFile, 'r');
    if ( !$FH ) {
        $error->set( $FUNCNAME, 1, "Can't open file '".$this->uploadFile."'");
        return;
    }
    
    while ( !feof ( $FH ) ) {
        $line  = fgets($FH);
        if ($this->is_utf8($line)) {
            $encoding_key = 'UTF-8';
            break;
        }
    }
    fclose($FH);
    
    return $encoding_key;
}

// convert to UTF-8
function line_to_utf8($line) {

    $line = utf8_encode ($line);
    return $line;
}

function get_current_line_no() {
    return $this->linecount;
}

function get_comment_lines() {
    return $this->comment_lines;
}

}
