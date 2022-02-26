<?php 

/**
 * spreadsheet analysis
 * - use with 
 *   - class uploadC : UPLOAD support for html-FORM
     - class f_read_file_meta : load csv, excel files
     
 * @package f.spreadsheet_ana.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @example <pre> 
 * 
 * 
    $file_lib = new f_read_file_meta($tempfile);  // other helper lib!
    $file_lib->open_file();
        
    $spread_lib = new fSpreadheet_ana();
    $spread_lib->set_file_header($header);
    $spread_lib->analyseHeader($colsExpect);
    if ( $error->Got(READONLY) ) {
       return;
    }
    $headerColPos = $spread_lib->getHeaderCol2Pos();
    while( $file_lib->read_line(0) )  {
         $datafields_raw  = $file_lib->get_data_row();
         $datafields      = $spread_lib->getDataByDefColName( $datafields_raw );
    }
    
    $file_lib->close_file();
 * 
 * </pre>
 */
class fSpreadheet_ana {
    
    /**
     * expected columns
     * @var array $header_def header_def_STRUCT 
     *   COL_KEY => array(
     *      'col.name'=> File_Column_Name, 
     *      'req'=> Column required in HEADER
     *        0: not required
     *        1: 
     *        2: required
     *        
     *      'val.req'=> VALUE required in cell [OPTIONAL]
     *        0: not required
     *        1: required
     *        
     *      'info' => info text
     *      ),
     * 
     example: array(
         "id1"   => ('col.name'=>"ID", 'req'=>2),  # col.name: the column name
         "name1" => ('col.name'=>"NAME", 'req'=>1),
      );
     */
    var $header_def; 
    
    /**
     * 
     * @param array $header
     */
    function set_file_header( $header ) {
        $this->header = $header;
    }
    
    
    /**
     * 
     * OUT:
     * - $this->headerPos
     * - $this->colsExpect_N2K
     * @param array $header_def header_def_STRUCT
     */
    function analyseHeader($header_def) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!is_array($this->header)) {
            $error->set($FUNCNAME, 1, 'Class input error: header missing.');
            return;
        }
        
        $this->headerPos=NULL;
        
        $this->header_def = $header_def;
        $this->colsExpect_N2K = array();
        $header = $this->header;
        
        if (sizeof($header_def)) {
            foreach($this->header_def  as $key => $row) {
                $this->colsExpect_N2K[$row['col.name']] = $key;
            }
        }
        
        
        $pos       = 0; 	// column position
        $headerPos = array();  // ["key"] = pos
        $loopError = "";
      
        
        // get position of COLUMN
       
        foreach( $header as $colName) {
            $colName    = trim($colName);
            $expect_key = $this->colsExpect_N2K[$colName];
            $isNeeded   = isset($this->header_def[$expect_key]);
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
        debugOut('(123) headerPos: '.print_r($this->headerPos,1), $FUNCNAME, 1);
        
//         glob_printr( $header , $FUNCNAME.":$header info" );
//         glob_printr( $this->colsExpect_N2K , $FUNCNAME.":colsExpect_N2K info" );
//         glob_printr( $this->headerPos , $FUNCNAME.":headerPos info" );
        
        // check expected columns
        foreach($this->header_def as $expect_key => $ex_row ) {
            
            $expect  = $ex_row['req'];
            $colName = $ex_row['col.name'];
            
            if ($expect==2) { // need this column ?
                if ( !isset($headerPos[$expect_key]) ) {
                    
                    $error->set($FUNCNAME, 5, "Column '".$colName."' missing");
                    return;
                }
            }
        }
    }
    
    function getHeaderCol2Pos () {
        return $this->headerPos;
    }
    
    /**
     * check one data row
     * @param array $data_arr
     */
    function check_data_row(&$data_arr) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $i=0;
        $loop_err=array();
        foreach($this->headerPos as $code_name=>$pos ) {
            
            $ex_row = $this->header_def[$code_name];

            $expect  = $ex_row['val.req'];
            $colName = $ex_row['col.name'];
            
            if ($expect>0) { // need this CELL ?
                $pos = $this->headerPos[$code_name];
                $val = $data_arr[$pos];
                if ( !isset($val) ) {
                    $loop_err[]='Col:'.$colName.': data missing.';
                }
            }
            $i++;
        }
        
        if (!empty($loop_err)) {
            $error->set($FUNCNAME, 5, implode(';',$loop_err) );
        }
        
    }
    
    /**
     * get line as array, index is the KEY-name of $this->header_def
     * column-names of header line must be unique
     * @param array $data_arr - one data array of file
     * @return array(column-name)= value
     */
    function getDataByDefColName( $data_arr ) {
        $keyValArr = array();
       
        foreach( $this->headerPos as $name=>$pos) {
            $keyValArr[$name] = $data_arr[$pos];
        }

        return $keyValArr;
    }
}