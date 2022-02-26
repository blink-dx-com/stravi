<?php
/**
 * simple XLS-export to Excel "BIFF 5" format
 * strong support for data streaming !!!
 * - supports following special cases:
 *   - double numbers and strings 
 *   - leading ZERO on number : is a string !
 *   - large char strings (4000 chars)
 *   - strings must be provided by encoding ISO-8859-1 (!!!)
 * - see also: http://sc.openoffice.org/excelfileformat.pdf
 * @namespace core::export
 * @package f.xls.simple.inc
 * @swreq SREQ:0001292: g > provide an export-library 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @example <pre>
 *	$xlsObj = new f_export_xls_simple($filename);
 *	$xlsObj->openFile() ;
 *	$row=array('hallo', 5.67, 'test', 944392.4234);
 *	$xlsObj->addRow($row);
 *	$row=array('hallo2', 3.67, 'test2s', 2.2);
 *	$xlsObj->addRow($row);
 *	$xlsObj-> closeFile();
 * </pre>
 */
class f_export_xls_simple {

	var $filename;	//Filename which the excel file will be returned as
	var $headerArray;	// Array which contains header information
	var $bodyArray;	// Array with the spreadsheet body
	var $rowNo = 0;	// Keep track of the row numbers
	var $xlsTypeStrict = 0; // 0,1 : strict type e.g. "000302" => STRING !
	var $typeArr = array(); // array( COLID => ('string', 'numeric') )
	
	/**
	 * construct
	 * @param $filename
	 * @param array $options
	 * 	'xlsTypeStrict' : 0,1 : strict type e.g. "000302" => STRING !
	 *  'typeArr' : = array( COLID => ('string', 'numeric') )
	 */
	function __construct($filename, $options=NULL) { 
		$this->filename = $filename;
		$this->rowNo = 0;
		
		$this->toolInfo='WARN: Still no support for Special chars (e.g. micro, percent)';
		if ( $options['xlsTypeStrict']>0) $this->xlsTypeStrict=1;
		if ( is_array($options['typeArr']) ) $this->typeArr=$options['typeArr'];
		
		$this->_codepage = 0x4E2; // 04E2H = 1250 = Windows CP-1250 (Latin II) (Central European)
		
		if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
			require_once ('f.debug.inc');
		    $this->_debug = $_SESSION["userGlob"]["g.debugLevel"];
		    
		}
	}

	
	/*
	-------------------------
	START OF PUBLIC FUNCTIONS
	-------------------------
	*/
	
	function openFile() {
		global $error;
		$FUNCNAME= $this->CLASSNAME.':openFile';
		
		$this->fp = fopen($this->filename, 'w'); 
		if ($this->fp==NULL) {
			$error->set( $FUNCNAME, 1, 'Can not open destination-file.' );
			return;
		}
		
		/**
		 * @var $linedata
		 * signature description:
		 * 0x809 : BOF
		 * 0x8   : 8 chars long
		 * 0x0   : 2x BIFF version: 0000H = BIFF5
		 * 0x10  : 2x Type of the following data: Sheet or dialogue
		 * 0x0   : 2x Additional fields of a BOF record, should be ignored
		 * 0x0   : 2x Additional fields of a BOF record, should be ignored
		 */
		$linedata = pack("ssssss", 0x809, 0x8, 0x0, 0x10, 0x0, 0x0);
		$this->_saveRowLine($linedata);
		
		$this->_storeCodepage();
	
	}

	function addHeader($header) {
	#Accepts an array or var which gets added to the top of the spreadsheet as a header.

		$this->addRow($header, 1);
	}

	/**
		Accepts an array or var which gets added to the spreadsheet body
		@param $ignoreDefType : 0,1
	 */
	function addRow($row, $ignoreDefType=0) {
	
		reset ($row);
		$build = NULL;
		$colNo = 0;
	
		foreach( $row as $colid=>$field) {
			
			$typeUse = 'string'; // default type
			
			if ( $field===NULL ) {
				// nothing
			} else {
				
				if (isset($this->typeArr[$colNo]) and !$ignoreDefType) {
					$typeUse = $this->typeArr[$colNo];
					if ($typeUse=='numeric') {
						if (!is_numeric($field)) {
							// repair type error ???
							$typeUse = 'string'; // yes, repair !!!
						}
					}
				} else {
					if (is_numeric($field)) {
						$typeUse = 'numeric';
					} else {
						$typeUse = 'string';
					}
				}
				
				switch ($typeUse) {
					case 'numeric':
						$build .= $this->numFormat($this->rowNo, $colNo, $field);
						break;
					default: // 'string'
						$build .= $this->textFormat($this->rowNo, $colNo, $field);
						break;
				}
				
			}
			$colNo++;
			
			
		}
		reset ($row); 
		
		$this->_saveRowLine($build);
				
		$this->rowNo++;
		
	}
	
	function closeFile() {
				
		$xls = pack("ss", 0x0A, 0x00);
		$this->_saveRowLine($xls);
				
		fclose( $this->fp );
	}

	function build_string() {
	# build and return the xls 
	
		#Excel BOF, 
		$xls = pack("ssssss", 0x809, 0x8, 0x0, 0x10, 0x0, 0x0);

		#build headers
		if(is_array($this->headerArray)) {
			$xls .= $this->build($this->headerArray);
		}

		#build body
		if(is_array($this->bodyArray)) {
			$xls .= $this->build($this->bodyArray);
		}

		$xls .= pack("ss", 0x0A, 0x00);

		return $xls;
	}

	/*
	--------------------------
	START OF PRIVATE FUNCTIONS
	--------------------------
	*/

	/**
    * Stores the CODEPAGE biff record.
    *
    * @access private
    */
    function _storeCodepage()
    {
        $record          = 0x0042;             // Record identifier
        $cv              = $this->_codepage;   // The code page
 
        $linedata        = pack('sss', $record, 2, $cv);
        $this->_saveRowLine($linedata);
 
    }
	
	function _saveRowLine($linedata) {
		$retVal = fputs( $this->fp, $linedata ); 
	}
	
	# format and return the field as a header
	function textFormat($row, $col, $data) {
		// TBD:1: mb_convert_encoding() needs compiler flag for php
		// $data   = mb_convert_encoding($data, "ISO-8859-1", "auto"); 
		$length = strlen($data);
		$field  = pack("ssssss", 0x204, 8 + $length, $row, $col, 0x0, $length);
		/*if ($this->_debug>1) {
			echo "DEBUG_textFormat length:".$length.' $row:'.$row.' $col:'. $col.
			 ' HEX: ';
			echo fDebugC::str2Hexinfo( $field );
			echo ' text: '.htmlspecialchars($data);
			echo "<br />";
		}
		*/
		$field .= $data;

		return $field; 
	}
		

	function numFormat($row, $col, $data) {
	# format and return the field as a header
    		$field = pack("sssss", 0x203, 14, $row, $col, 0x0);
    		$field .= pack("d", $data); 
		
		return $field; 
	}
}
