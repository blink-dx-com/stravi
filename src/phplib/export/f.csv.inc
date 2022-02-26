<?php

/**
 * simple CSV-export
 * strong support for data streaming !!!
 * @swreq SUBREQ:01 replace NEWLINE and SEPARATOR by WHITE-SPACE, otherwise CSV-format is corrupt
 * @package f.csv.inc
 * @namespace core::export
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @unittest exists
 * @example <pre>
 *	$xlsObj = new f_export_xls_simple($filename);
 *	$xlsObj->openFile() ;
 *	$row=array('hallo', 5.67, 'test', 944392.4234);
 *	$xlsObj->addRow($row);
 *	$xlsObj-> closeFile();
 * </pre>
 */
class f_export_csv {

	var $filename;	//Filename which the excel file will be returned as
	var $headerArray;	// Array which contains header information
	var $bodyArray;	// Array with the spreadsheet body
	var $rowNo = 0;	// Keep track of the row numbers


	/**
	 * @param array $options
	 * 	'separator' : e.g. ','
	 */
	function __construct($filename, $options=NULL) { 
		$this->filename = $filename;
		$this->rowNo = 0;
		$this->separator = "\t";
		
		if ($options['separator']!=NULL) {
			$this->separator = $options['separator'];
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
	
	}

	function addHeader($header) {
	#Accepts an array or var which gets added to the top of the spreadsheet as a header.

		$this->addRow($header);
	}

	/**
		Accepts an array or var which gets added to the spreadsheet body
	 */
	function addRow($row) {
	
		$build = NULL;
		
		if ( !is_array($row) ) {
			$this->_saveRowLine($build);
			$this->rowNo++;
			return;
		}
		reset ($row);
		
		$colNo = 0;
		$separator = NULL; // start
		$tempInCellSpacer = ' '; // temporary space in the cell, if a bad CHAR is in the cell

	
		foreach( $row as $dummy=>$field) {
			 
			if ( is_string($field) ) {
				//@swreq SUBREQ:01 replace line feeds and the SEPARATOR itself
				$texttmp = preg_replace("/[\r\n".$this->separator."]/",$tempInCellSpacer,$field);  
				
				// search for NON-allowed chars in the cell
				$texttmp = str_replace( $separator, $tempInCellSpacer, $texttmp); 
			} else {
				$texttmp = $field;
			}
			
			$build .= $separator . $texttmp;
			$separator = $this->separator; // update
			$colNo++;
			
		}
		reset ($row); 
		
		$this->_saveRowLine($build);
				
		$this->rowNo++;
		
	}
	
	function closeFile() {
				
		fclose( $this->fp );
	}

	
	/*
	--------------------------
	START OF PRIVATE FUNCTIONS
	--------------------------
	*/

	function _saveRowLine($linedata) {
		$retVal = fputs( $this->fp, $linedata."\n" ); 
	}
	
}
