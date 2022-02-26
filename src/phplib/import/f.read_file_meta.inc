<?php
/**
 * support multiple file formats
 *  - automatic encodes into UTF8
 * @package f.read_file_meta.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @example <pre>
 *  $file_lib = new f_read_file_meta($this->userfile, $this->file_type);
    $file_lib->open_file();
    $headers   = $file_lib->get_headers();
    
    $encoding_key = $file_lib->get_encoding_info();
    
     while( $file_lib->read_line() )  {
            $datafields   = $file_lib->get_data_row();
     }
     $file_lib->close_file();
 * </pre>
 */
require dirname(__file__) . '/../../vendor/autoload.php';

use PhpOffice\PhpSpreadsheet;


class f_read_file_IF {
    
    public $DataRow  =NULL; // data row
    public $FormatRow=NULL; // format row: needed for Excel
    protected $headers; // array of array
    protected $linecount;
    protected $comment_lines;
    protected $options;
    protected $col_max_ind=0; // max number of columns to read per line ...
    protected $max_row;
    protected $data_start_line_no;
    public $encoding_key='ISO-8859-1';
    
    function __construct($filename, $type, $options=array() ) {

        $this->options = $options;
        $this->filename = $filename;
        $this->type=$type;
        $this->headers=NULL;
        $this->comment_lines=array();
    }
    
    protected function is_utf8($string) {
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
    
    // set $this->col_max_ind, if the header is empty (needed for Excel)
    function set_cols_max($cols_max) {
        $this->col_max_ind = $cols_max;
    }
    public function get_linecount() {
        return $this->linecount;
    }
    public function get_max_row() {
        return $this->max_row;
    }
    public function get_data_start_line_no() {
        return $this->data_start_line_no;
    }
    // set the linecount
    public function set_linecount($linecount) {
        throw new Exception('Not implemented.');
    }
    
    /**
     * set file-pointer to first line
     */
    function open_file() {}
    
    /**
     * ignore comment lines
     * set $this->headers
     * @return int 0,1
     */
    function header_analyse() {}
    function start_data_read() {}
    
    // set file read pointer to START
    function reset_file_ptr() {
        throw new Exception('Not implemented.');
    }
    
    /**
     * get data cell
     * @param int $x (start with 1)
     * @param int $y (start with 1)
     * @return mixed
     */
    function get_data_cell(int $x, int $y) {
        throw new Exception('Not implemented.');
    }
    function get_cell_object(int $x, $y) {
        throw new Exception('Not implemented.');
    }
    
    
    function get_headers() {
        return $this->headers;
    }
    
    function getComments() {
        return $this->comment_lines;
    }
    
    function detect_encoding() {
        die('"detect_encoding" not implemented.');
    }
    
    /**
     * @return int data exists in $this->DataRow[]
     */
    function read_line($do_trim=1) {}
    
    // @return int number of data lines
    function countDataLines() {}
    function get_all_sheet_names() {throw new Exception('Not implemented.');}
    function set_sheet($sheet_index) {throw new Exception('Not implemented.');}
    function close_file() {}
}

class f_read_file_CSV extends f_read_file_IF {
    
    private $delimiter = "\t";
    
    /**
     * 
     * @param string $filename
     * @param string $type
     * @param array $options
     *   'delimiter'
     */
    function __construct($filename, $type, $options=array() ) {
        parent::__construct($filename, $type, $options);
        
        if ($options['delimiter']) $this->delimiter=$options['delimiter'];
    }
    
    /**
     * ignore comment lines
     * set $this->headers
     * @return int 0,1
     */
    function header_analyse() {
        
        $this->data_start_line_no = -1;
        $this->headers=array();
        $this->comment_lines=array();
        
        $FH = fopen($this->filename, 'r');
        $line  = fgets($FH, 8000);      // no trim !!!
        $this->linecount++;
        
        while ( (substr($line,0,1)=="#") AND !feof ( $FH ) ) { // ignore commments
            $this->comment_lines[] = $line;
            $line  = fgets($FH, 8000);      // no trim !!!
            $this->linecount++;
        }
        fclose($FH);
        
       
        
        $line     = rtrim($line);
        $head_arr = explode($this->delimiter, $line);
        $this->data_start_line_no = $this->linecount;

        $this->headers[] = $head_arr;

    }
    
    function start_data_read() {
        
        $this->FH = fopen($this->filename, 'r');
        if ( !$this->FH ) {
            throw new Exception("Can't open file '".$this->filename."'");
        }
        $this->linecount=1;
        
        // wind file pointer to start of first data row
        while ( !feof ( $this->FH ) and ($this->linecount<$this->data_start_line_no)  ) {
            $line  = fgets($this->FH, 8000);
            $this->linecount++;
        }
        
    }
    
    // set file read pointer to START
    function reset_file_ptr() {
        $this->close_file();
        $this->FH = fopen($this->filename, 'r');
        if ( !$this->FH ) {
            throw new Exception("Can't open file '".$this->filename."'");
        }
        $this->linecount=1;
    }
    
    function countDataLines() {
        $this->start_data_read();
        $CHAR_READ_NUM=8000;
        $cnt=0;
        while ( !feof ( $this->FH ) ) {
            fgets($this->FH, $CHAR_READ_NUM);
            $cnt++;
        }
        fclose($this->FH);
        return  $cnt;
    }
    
    /**
     * detect, if the data in FILE is UTF-8 or ISO-8859-1
     * set $this->encoding_key
     * @return string UTF-8 or ISO-8859-1
     */
    function detect_encoding() {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->encoding_key='ISO-8859-1';
        
        $FH = fopen($this->filename, 'r');
        if ( !$FH ) {
            $error->set( $FUNCNAME, 1, "Can't open file '".$this->filename."'");
            return;
        }
        
        while ( !feof ( $FH ) ) {
            $line  = fgets($FH);
            if ($this->is_utf8($line)) {
                $this->encoding_key = 'UTF-8';
                break;
            }
        }
        fclose($FH);

        return $this->encoding_key;
    }
    
    /**
     * @return int data exists in $this->DataRow[]
     */
    function read_line($do_trim=1) {
        
        if ( feof ( $this->FH ) ) return 0;
        $line  = fgets($this->FH, 8000);
        if (trim($line)==NULL) return 0;
        $this->DataRow = explode($this->delimiter, $line);
        
        if ($this->encoding_key=='ISO-8859-1') {
            // transform to UTF-8
            $tmp_vals=array();
            foreach($this->DataRow as $val) {
                $tmp_vals[] = utf8_encode ($val);
            }
            $this->DataRow = $tmp_vals;
        }
        
        return 1;
    }
    
    /**
     * set file-pointer to first line
     */
    function open_file() {
        
        //$LINE_LENGTH = 32000;
        
        if ( !file_exists($this->filename) ) {
            throw new Exception("Can't open file '".$this->filename."'");
        }
        
        $this->linecount=1;
        
        $this->header_analyse();
    }
    
    function close_file() {
        if (!$this->FH) return;
        
        fclose($this->FH);
    }
}


class f_read_file_EXCEL extends f_read_file_IF {
    
    
    
    /**
     * read header
     * - header row can be empty
     * - optional you can call set_cols_max()
     * trims each cell
     * @param int $row_index
     * @return array
     */
    private function _read_header($row_index) {
        $has_data=1;
        $max_cols=50;
        $last_filled_col_ind=0;
        
        $col_index=1;
        $row=array();
        while ($has_data) {
            
            if ($col_index>$max_cols) {
                break;
            }
            $cell = $this->sheet->getCellByColumnAndRow($col_index, $row_index);
            
            $cell = trim($cell);
            if ($cell!==NULL and $cell!=='') {
                $last_filled_col_ind = $col_index;
            }
            $row[] = $cell;
            $col_index++;
        }
        
        if (!$last_filled_col_ind) {
            // fallback: no data at all
            $last_filled_col_ind = $max_cols;
        }
        
        $row = array_slice($row, 0, $last_filled_col_ind); // get all columns with data
        
        return $row;
    }
    
    /**
     * ignore comment lines
     * set $this->headers
     * @return int 0,1
     */
    function header_analyse() {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->col_max_ind = 0;
        $this->data_start_line_no = -1;
        $this->headers=array();
        $this->comment_lines=array();

        
        $col_index = 1;
        $head_arr = $this->_read_header($this->linecount);
        $cell=$head_arr[0];
        $this->linecount++;
        
        while ( (substr($cell,0,1)=="#")  ) { // ignore commments
            $this->comment_lines[] = $cell;
            $head_arr = $this->_read_header($this->linecount);
            $cell=$head_arr[0];
            $this->linecount++;
        }

        debugOut("(364) COUNT: ".$this->linecount, $FUNCNAME, 2); 
        $this->data_start_line_no = $this->linecount;
        
        $this->headers[] = $head_arr;
        
        $header = $this->headers[0];
        $this->col_max_ind = sizeof($header);
  
    }
    
    function countDataLines() {
//         $this->start_data_read();
//         $cnt=0;
//         while ( $this->read_line(0) ) {
//             $cnt++;
//         }
        
        $cnt =  $this->max_row;
        return  $cnt;
    }
    
    function detect_encoding() {
        //TBD:
        $this->encoding_key='ISO-8859-1';
    }
    
    /**
     * reset linecount
     * {@inheritDoc}
     * @see f_read_file_IF::start_data_read()
     */
    function start_data_read() {
        $this->linecount = $this->data_start_line_no;
    }
    
    // set file read pointer to START
    function reset_file_ptr() {
        $this->linecount = 1;
    }
    
    public function set_linecount($linecount) {
        if ($linecount<=0) {
            throw new Exception('linecnt must be >0.');
        }
        $this->linecount = $linecount;
    }
    
    /**
     * used to get EXCEL:CLACULATED value !
     * $cell_obj = $this->file_obj->get_cell_object($x, $y);
       $newval   = $cell_obj->getOldCalculatedValue();
     * @see f_read_file_IF::get_cell_object()
     */
    function get_cell_object(int $x, $y) {
        return $this->sheet->getCellByColumnAndRow($x, $y);
    }
    
    /**
     * get a data cell
     * @param int $x
     * @param int $y
     * @return NULL
     */
    private function _get_cell($x, $y) {
        
        $cell = $this->sheet->getCellByColumnAndRow($x, $y)->getValue();
        $this->_cell_num_format = $this->sheet->getStyleByColumnAndRow($x, $y)->getNumberFormat()->getFormatCode();
        if ( strstr($this->_cell_num_format,'yyyy')!=NULL ) {
            $time_obj = PhpSpreadsheet\Shared\Date::excelToDateTimeObject($cell);
            if ($time_obj!=NULL) {
                $cell = $time_obj->format('Y-m-d');
            } else {
                $cell=NULL;
            }
        }
        return $cell;
    }
    
    /**
     * read one line
     * - supports DateTime: returns: YYYY-MM-DD
     * INPUT: $this->col_max_ind
     * increment $this->linecount
     * @param int $do_trim :0,1
     * @return int data exists in $this->DataRow[]
     */
    function read_line($do_trim=1) {

        $last_filled_col_ind=0; // may be later
        $has_data = 0;

        $col_index=1;
        $this->DataRow=array();
        $this->FormatRow=array();
        
        if ($this->linecount > $this->max_row) {
            return 0;
        }
        
        while ( $col_index <= $this->col_max_ind ) {

            $cell = $this->_get_cell($col_index, $this->linecount);
            $this->FormatRow[] = $this->_cell_num_format;
            
            if ($do_trim) $cell = trim($cell);
            
            $cell_len = 0;
            if ($cell!==NULL) {
                $cell_len = strlen(trim($cell));
            }
            if ($cell_len>0) {
                $last_filled_col_ind = $col_index;  // may be later
            }
            $this->DataRow[] = $cell;
            $col_index++;
        }
        
        $has_data=1;
        // if ($last_filled_col_ind>0) $has_data=1;
        $this->linecount++;
        
        return $has_data;
    }
    
    function get_data_cell(int $x, int $y) {
        return  $this->_get_cell($x, $y);
    }
    
    private function _sheet_open() {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        $this->sheet     = $this->objReader->getActiveSheet();
        
        debugOut('(497) getHighestRow()', $FUNCNAME,1);
        $this->max_row   = $this->sheet->getHighestRow();
        $this->header_analyse();
    }
    
    function set_sheet($sheet_index) {
        $this->sheet_name=$sheet_index;
        $this->objReader->setActiveSheetIndex($sheet_index);
        $this->_sheet_open();
    }
    function get_all_sheet_names() {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        debugOut('(507) start allSheets', $FUNCNAME,1);
        #$all_sheets = $this->objReader->getAllSheets();
        $all_sheets = $this->objReader->getSheetNames();
        debugOut('(509) end allSheets', $FUNCNAME,1);
        
        $names=array();
        foreach ($all_sheets as $sheetIndex => $sh_name) {
            $names[$sheetIndex]=$sh_name;
        }
        return $names;
    }
    

    /**
     * set file-pointer to first line
     */
    function open_file() {
        
        if ( !file_exists($this->filename) ) {
            throw new Exception("Can't open file '".$this->filename."'");
        }
        
        $this->linecount=1;
        $this->objReader = PhpSpreadsheet\IOFactory::load($this->filename);
        $this->_sheet_open();
        
    }
    
    function close_file() {
        unset($this->sheet);
        unset($this->objReader);
    }
}


/***
 * PUBLIC class  
 *
 */
class f_read_file_meta {
    
    private $type; // file-type: 'csv', 'excel'
    private $filename;
    private $maxDataLines;
    
    /**
     * constructor
     * @param string $filename
     * @param string $type - file type, if EMPTY: AUTODETECT
     *    'csv', 'excel'
     * @param array $options
     * @throws Exception
     */
    function __construct($filename, $type='', $options=array() ) {
        
        $this->filename = $filename;
        $this->maxDataLines = 0;
        
        if ($type=='') {
            // AUTODETECT
            $point_pos = strrpos($filename,'.');
            $point_pos = $point_pos+1;
            $ext = substr($filename, $point_pos );
            
            $type='csv';
            if ($ext=='xlsx') {
                $type='excel';
            }
            
        }
        $this->type=$type;
        
        switch ($type) {
            case 'csv':
                $this->file_lib = new f_read_file_CSV($filename, $type, $options);
                break;
            case 'excel':
                $this->file_lib = new f_read_file_EXCEL($filename, $type, $options);
                break;
            default:
                throw new Exception('type "'.$type.'" unknown.');
        }
    }
    
    function get_type() {
        return $this->type;
    }
    
    function open_file() {
        $this->file_lib->open_file();
        $this->file_lib->detect_encoding();
        $this->maxDataLines =  $this->file_lib->countDataLines();
        $this->file_lib->start_data_read();
    }
    
    function close_file() {
        $this->file_lib->close_file();
    }
    
    function get_headers() {
        return $this->file_lib->get_headers();
    }
    
    // @return int number of data lines
    function countDataLines() {
        return $this->maxDataLines;
    }
    
    function read_line($do_trim=1) {
        return $this->file_lib->read_line($do_trim);
    }
    
    
    function getComments() {
        return $this->file_lib->getComments();
    }
    function get_linecount() {
        return $this->file_lib->get_linecount();
    }
    function set_linecount($linecount) {
        return $this->file_lib->set_linecount($linecount);
    }
    function get_data_start_line_no() {
        return $this->file_lib->get_data_start_line_no();
    }
    
    /**
     * transform data, if needed from 'ISO-8859-1' => UTF-8
     * @return string[]
     */
    function get_data_row() {
        return $this->file_lib->DataRow;
    }
    
    function get_data_cell(int $x, int $y) {
        return $this->file_lib->get_data_cell($x, $y);
    }
    function get_cell_object(int $x, $y) {
        return $this->file_lib->get_cell_object($x, $y);
    }
    
    function reset_file_ptr() {
        return $this->file_lib->reset_file_ptr();
    }
    
    // get format row (special for EXCEL)
    function get_format_row() {
        $tmp_vals = &$this->file_lib->FormatRow;
        return $tmp_vals;
    }
    
    function get_encoding_info() {
        return $this->file_lib->encoding_key;
    }
    
    function set_cols_max($cols_max) {
        $this->file_lib->set_cols_max($cols_max);
    }
    
    function get_all_sheet_names() { return $this->file_lib->get_all_sheet_names(); }
    function set_sheet($sheet_index) { $this->file_lib->set_sheet($sheet_index); }
}