<?php
/**
 * Simple excel generating from PHP5
 * COMMENTS by Steffen Kube
 * Format: Microsoft XML Spreadsheet
 * see: http://msdn.microsoft.com/en-us/library/aa140062%28v=office.10%29.aspx
 * @package f.xlsxml.simple.inc
 * @swreq SREQ:0001292: g > provide an export-library 
 * @version $Header: trunk/src/phplib/export/f.xlsxml.simple.inc 59 2018-11-21 09:04:09Z $
 */

/**
 * Simple excel generating from PHP5
 *
 * @package Utilities
 * @license http://www.opensource.org/licenses/mit-license.php
 * @author Oliver Schwarz <oliver.schwarz@gmail.com>
 * @version 1.0
 */



/**
 * Generating excel documents on-the-fly from PHP5
 * 
 * Uses the excel XML-specification to generate a native
 * XML document, readable/processable by excel.
 * 
 * @package Utilities
 * @subpackage Excel
 * @author Oliver Schwarz <oliver.schwarz@vaicon.de>
 * @version 1.1
 * 
 * @todo Issue #4: Internet Explorer 7 does not work well with the given header
 * @todo Add option to give out first line as header (bold text)
 * @todo Add option to give out last line as footer (bold text)
 * @todo Add option to write to file
 */
class Excel_XML
{
		var $rowNo = 0;	// Keep track of the row numbers
	/**
	 * Header (of document)
	 * NEW:Steffen Kube: the tag 'mso-application progid="Excel.Sheet"' forces Windows an automatic Excel regognition of the file type
	 * @var string $header
	 */
        var $header =  "<?xml version=\"1.0\" encoding=\"%s\"?\>
          <?mso-application progid=\"Excel.Sheet\"?\>
           <Workbook xmlns=\"urn:schemas-microsoft-com:office:spreadsheet\" 
           xmlns:x=\"urn:schemas-microsoft-com:office:excel\" xmlns:ss=\"urn:schemas-microsoft-com:office:spreadsheet\" 
           xmlns:html=\"http://www.w3.org/TR/REC-html40\">\n";

        /**
         * Footer (of document)
         * @var string
         */
        var $footer = "</Workbook>";

        /**
         * Lines to output in the excel document
         * @var array
         */
        var $lines = array();

        /**
         * Used encoding
         * @var string
         */
        var $sEncoding;
        
        /**
         * Convert variable types
         * @var boolean
         */
        var $bConvertTypes;
        
        /**
         * Worksheet title
         * @var string
         */
        var $sWorksheetTitle;

        /**
         * Constructor
         * 
         * The constructor allows the setting of some additional
         * parameters so that the library may be configured to
         * one's needs.
         * 
         * On converting types:
         * When set to true, the library tries to identify the type of
         * the variable value and set the field specification for Excel
         * accordingly. Be careful with article numbers or postcodes
         * starting with a '0' (zero)!
         * 
         * @param string $sEncoding Encoding to be used (defaults to UTF-8)
         * @param boolean $bConvertTypes Convert variables to field specification
         * @param string $sWorksheetTitle Title for the worksheet
         */
        function __construct($filename)
        {
        	$this->filename = $filename;
			$this->rowNo = 0;
		
        	$sEncoding = 'UTF-8';
        	$bConvertTypes = true;
        	$sWorksheetTitle = 'Table1';
            $this->bConvertTypes = $bConvertTypes;
        	$this->setEncoding($sEncoding);
        	$this->setWorksheetTitle($sWorksheetTitle);
        }
        
        // PRIVATE
		function _saveRowLine($linedata) {
			$retVal = fputs( $this->fp, $linedata ); 
		}
        
        /**
         * Set encoding
         * @param string Encoding type to set
         */
        function setEncoding($sEncoding)
        {
        	$this->sEncoding = $sEncoding;
        }

        /**
         * Set worksheet title
         * 
         * Strips out not allowed characters and trims the
         * title to a maximum length of 31.
         * 
         * @param string $title Title for worksheet
         */
        function setWorksheetTitle ($title)
        {
                $title = preg_replace ("/[\\\|:|\/|\?|\*|\[|\]]/", "", $title);
                $title = substr ($title, 0, 31);
                $this->sWorksheetTitle = $title;
        }

        /**
         * Add row
         * 
         * Adds a single row to the document. If set to true, self::bConvertTypes
         * checks the type of variable and returns the specific field settings
         * for the cell.
         * 
         * @param array $array One-dimensional array with row content
         */
        function addRow ($array)
        {
        	
        	$cells = "";
        	foreach ($array as $k => $v):
        	        $type = 'String';
        	        if ($this->bConvertTypes === true && is_numeric($v)) {
        	        	$type = 'Number';
        	        	$v = trim($v); // if white spaces or other chars appear in number string
        	        } else {
        	        // $v = htmlentities($v, ENT_COMPAT, $this->sEncoding);
        	        	$v = htmlspecialchars($v, ENT_COMPAT, $this->sEncoding);
        	        }
        	        $cells .= "<Cell><Data ss:Type=\"$type\">" . $v . "</Data></Cell>\n"; 
        	endforeach;
        	$cells = "<Row>\n" . $cells . "</Row>\n";
        	
        	$this->_saveRowLine($cells);
				
			$this->rowNo++;
        }

        
		function openFile() {
			global $error;
			$FUNCNAME= __CLASS__.':'.__FUNCTION__;
			
			$this->fp = fopen($this->filename, 'w'); 
			if ($this->fp==NULL) {
				$error->set( $FUNCNAME, 1, 'Can not open destination-file.' );
				return;
			}
			$retVal = fputs( $this->fp, stripslashes (sprintf($this->header, $this->sEncoding)) );
            $retVal = fputs( $this->fp, "\n<Worksheet ss:Name=\"" . $this->sWorksheetTitle . "\">\n<Table>\n" );
		
		}
		
		function addHeader($header) {
			#Accepts an array or var which gets added to the top of the spreadsheet as a header.
	
			$this->addRow($header);
		}
		
		function closeFile() {
				
			$retVal = fputs( $this->fp, "</Table>\n</Worksheet>\n");
            $retVal = fputs( $this->fp, $this->footer );
					
			fclose( $this->fp );
		}


}

?>