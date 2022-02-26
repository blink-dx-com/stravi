<?php
/**
 * simple HTML-export
 * strong support for data streaming !!!
 *
 * @namespace core::export
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @example <pre>
 *	$xlsObj = new f_export_xls_simple($filename);
 *	$xlsObj->openFile() ;
 *	$row=array('hallo', 5.67, 'test', 944392.4234);
 *	$xlsObj->addRow($row);
 *	$xlsObj-> closeFile();
 * </pre>
 */
require_once ("visufuncs.inc");

class f_export_html {

	var $filename;	//Filename which the excel file will be returned as
	var $headerArray;	// Array which contains header information
	var $bodyArray;	// Array with the spreadsheet body
	var $rowNo = 0;	// Keep track of the row numbers


	#Class constructor
	function __construct($filename) { 
		$this->filename = $filename;
		$this->rowNo = 0;
		
	}

	
	/*
	-------------------------
	START OF PUBLIC FUNCTIONS
	-------------------------
	*/
	
	function openFile() {
		global $error;
		$FUNCNAME= $this->CLASSNAME.':openFile';
		
		$this->tabobj = new visufuncs();
	}

	function addHeader($header) {
	#Accepts an array or var which gets added to the top of the spreadsheet as a header.
		$this->addRow($header);

	}

	/**
		Accepts an array or var which gets added to the spreadsheet body
	 */
	function addRow($row) {
		
		if (!$this->rowNo) {
			$headOpt = array( "title" => "");
			$this->tabobj->table_head( $row, $headOpt);
		} else {
			$this->tabobj->table_row ($row);		
		}
		$this->rowNo++;
		
	}
	
	function closeFile() {	
		$this->tabobj->table_close();
	}

	
}
