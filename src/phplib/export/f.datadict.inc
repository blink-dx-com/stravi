<?php

/**
 * simple DATADICT-export
 * strong support for data streaming !!!
 * @swreq 
 * @package f.csv.inc
 * @namespace core::export
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @unittest exists
 * @example <pre>
 *	$xlsObj = new f_export_datadict($filename);
 *	$xlsObj->openFile() ;
 *	$row=array('hallo', 5.67, 'test', 944392.4234);
 *	$xlsObj->addRow($row);
 *	$xlsObj-> closeFile();
 * </pre>
 */
class f_export_datadict {
	
	/**
	 * structure of data
	 * @var array $_datadict
	 * 'head' : one array of column names
	   'data' : array of array of data columns (one array per row)
	   'meta' : array of KEY=>VAL
	      'title'=> string
	 */
	private $_datadict;

	var $filename;	//Filename which the excel file will be returned as
	var $headerArray;	// Array which contains header information
	var $bodyArray;	// Array with the spreadsheet body
	var $rowNo = 0;	// Keep track of the row numbers


	/**
	 * @param array $options
	 * 	'separator' : e.g. ','
	 */
	function __construct($options=NULL) { 
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
		
		$this->_datadict = array();
		$this->_datadict['head'] = array();
		$this->_datadict['data'] = array();
		$this->_datadict['meta'] = array();
	}

	/**
	 * Accepts an array or var which gets added to the top of the spreadsheet as a header.
	 * if first COL starts with "#" => put to 'meta' !!!
	 * @param array $header_row
	 */
	function addHeader($header_row) {

		if (substr($header_row[0],0,1)=='#') {
			$this->_datadict['meta'][] = $header_row;
		} else {	
			// only one row allowed !
			$this->_datadict['head'] = $header_row;
		}
	}

	/**
		Accepts an array or var which gets added to the spreadsheet body
	 */
	function addRow($row) {
		
		if ( !is_array($row) ) {
			$row=array();
		}
		$row = array_values($row); // create pure NUMBER-based array
		
		$this->_datadict['data'][] = $row;	
		$this->rowNo++;
		
	}
	
	function closeFile() {
		// nothing
	}

	/**
	 * return the data
	 * @param unknown $key
	 * @return multitype:
	 */
	function &getData($key=NULL) {
		return $this->_datadict;
	}
	
	
	
}
