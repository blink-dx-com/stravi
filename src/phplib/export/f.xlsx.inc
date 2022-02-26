<?php
require dirname(__file__) . '/../../vendor/autoload.php';
use PhpOffice\PhpSpreadsheet;

/**
 * XLSX export
 * @package f.xlsx.inc
 * @namespace core::export
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @unittest exists
 * @example <pre>
 *	$xlsObj = new f_export_xlsx($filename);
 *	$xlsObj->openFile() ;
 *	$row=array('hallo', 5.67, 'test', 944392.4234);
 *	$xlsObj->addRow($row);
 *	$xlsObj-> closeFile();
 * </pre>
 */
class f_export_xlsx {

	var $filename;	//Filename which the excel file will be returned as
	var $headerArray;	// Array which contains header information
	var $bodyArray;	// Array with the spreadsheet body
	var $rowNo = 0;	// Keep track of the row numbers


	/**
	 * @param array $options
	 
	 */
	function __construct($filename, $options=NULL) { 
		$this->filename = $filename;
		$this->rowNo = 0;
		$this->info_str_arr = NULL;

	}

	
	/*
	-------------------------
	START OF PUBLIC FUNCTIONS
	-------------------------
	*/
	
	function openFile() {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$fp = fopen($this->filename, 'w');
		if ($fp==NULL) {
		    $error->set( $FUNCNAME, 1, 'Can not open destination-file.' );
		    return;
		}
		fclose($fp);
		
		/** Create a new Spreadsheet Object **/
		$this->_spreadsheet = new PhpSpreadsheet\Spreadsheet();
		$this->workSheet = $this->_spreadsheet->getActiveSheet();
		$this->workSheet->setTitle('Data');
		
		// Create a new worksheet called "My Data"
		$this->info_workSheet = new PhpSpreadsheet\Worksheet\Worksheet($this->_spreadsheet , 'Meta Info');
		
		// Attach the "My Data" worksheet as the second worksheet in the Spreadsheet object
		$this->_spreadsheet->addSheet($this->info_workSheet, 1);
		
		/*
		$spreadsheet->getProperties()
		->setCreator("Maarten Balliauw")
		->setLastModifiedBy("Maarten Balliauw")
		->setTitle("Office 2007 XLSX Test Document")
		->setSubject("Office 2007 XLSX Test Document")
		->setDescription(
		    "Test document for Office 2007 XLSX, generated using PHP classes."
		    )
		    ->setKeywords("office 2007 openxml php")
		    ->setCategory("Test result file");
		 */
	
	}
	
	function setEncoding( $encoding ) {
	    //TBD:
	}
	
	function set_dataset_infos($info_arr) {
	    $this->info_str_arr = $info_arr;
	}

	function addHeader($header) {
	    #Accepts an array or var which gets added to the top of the spreadsheet as a header.

	    
	    $styleArray = [
	        'font' => [
	            'bold' => true,
	        ]
	    ];
	    
	    $this->addRow($header, $styleArray);
	}

	/**
		Accepts an array or var which gets added to the spreadsheet body
		@param array $major_style
		   major style for the row
	 */
	function addRow($row, $major_style=NULL) {

	    $row_ind=$this->rowNo+1;
	    
	    $col_ind=1;
	    foreach ($row as $val) {
 
	       $this->workSheet->setCellValueByColumnAndRow($col_ind, $row_ind, $val); // accepts UTF-8 chars
	       
	       $col_ind++;
	    }
	    
	    if ($major_style!==NULL) {
	        // $col_str_0 = PhpSpreadsheet\Cell\Coordinate::stringFromColumnIndex(1);
	        
	        $row_ind = 1;
	        $col_0   = 1;
	        $col_1   = $col_ind-1;
	        $this->workSheet->getStyleByColumnAndRow($col_0, $row_ind, $col_1, $row_ind)->applyFromArray($major_style);
	       
	        
	    }
	       
		$this->rowNo++;
		
	}
	
	private function _write_info_worksheet() {
	    
	    $row_ind=1;
	    $col_ind=1;
	    foreach($this->info_str_arr as $one_str) {
	        $this->info_workSheet->setCellValueByColumnAndRow($col_ind, $row_ind, $one_str);
	        $row_ind++;
	    }
	}
	
	function closeFile() {
	    
	    if (is_array($this->info_str_arr)) {
	        $this->_write_info_worksheet();
	    }
	    
	    $writer = new PhpSpreadsheet\Writer\Xlsx($this->_spreadsheet);
	    $writer->save( $this->filename );
	    
	    // Clearing a Workbook from memory
	    $this->_spreadsheet->disconnectWorksheets();
	    unset($this->_spreadsheet);

	}

	
	/*
	--------------------------
	START OF PRIVATE FUNCTIONS
	--------------------------
	*/

	
}
