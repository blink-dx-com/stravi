<?php 
/**
 * - for glob.objtab.import.php
 * - header analysis
 * @package glob.objtab.importHead.inc
 * @swreq UREQ:0001359: g > import object-attributes from CSV-file 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/glob.objtab.importHead.inc 59 2018-11-21 09:04:09Z $
 */
require_once 'import/f.read_file_meta.inc';


/**
 * header analysis
 *
 */
class gObjtabImpHead {
    
    private $col_nice_info; // array( NICE=>array('col'=>RAW name, 't'=>'class') )
    
    function __construct( $tablename, $filename, $file_type) {
        $this->tablename=$tablename;
        $this->filename =$filename;
        $this->file_type=$file_type;
        
        $this->colGoodNames = columns_get2( $tablename );
        
        
    }
    
    

    private function _debug_out ($text) {
        echo "<span style='color:#808000'>INFO: </span>$text<br>\n";
    } 
    
    private function _col_part_info($col_in) {
        
        $col_ext_KEY=':NAME';
        $col_ext_KEY_LEN=strlen($col_ext_KEY);
        
        $name_parts=array( $col_in, 0 );
        
        if ( substr($col_in,strlen($col_in)-$col_ext_KEY_LEN) == $col_ext_KEY ) {
            $colname  = substr($col_in,0, strlen($col_in)-$col_ext_KEY_LEN);  // RENAME !!!!!
            $name_parts[0]= $colname;
            $name_parts[1]= 1; 
        }

        return $name_parts;
    }
    
    /**
     * transform NICE => RAW column
     * @param string $colnameOri
     */
    private function col_nice2Raw($colnameOri) {
        
        $part_info  = $this->_col_part_info($colnameOri);
        $colnameOri = $part_info[0];
        
        if ( $this->col_nice_info[$colnameOri]==NULL ) {
            throw new Exception('Nice-Column "'.$colnameOri.'" is unknown.' );
        }
        
        $col_info = $this->col_nice_info[$colnameOri];
        
        $col_raw = $col_info['col'];
        if ($col_info['t']=='class') {
            $col_raw = 'c:'.$col_info['col'];
        }
        
        if ($part_info[1]==1) {
            $col_raw .= ':NAME';
        }
        
        return $col_raw;
    }
    
    /**
     * input:
     * - $this->colwrong_arr
     * in/out: 
     * - $this->headerInfo
     * @param int $pos
     * @param string $colnameOri - origiginal column name in file
     * @return array
     *   'colname'    => $colname,
            'rememberCol'=> $rememberCol,
            'error'      => $tmperr,
            'colErr'     => array of text
            'colInfo'    => $tmpcolInfo
     */
    private function _one_column($sqlo, $pos, $colnameOri) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $tmpcolErr = array();
        $rememberCol = 1;
        $col_type    ='';
        $tablename   = $this->tablename;
        $colnameOri  = trim($colnameOri);
        $colname     = $colnameOri;
        $colname_nice= '?';
        $primcol     = PrimNameGet2($tablename); 
        
        $class_id       = $this->classInfoArr[0];
        //$class_name     = $this->classInfoArr[1];
        $attribNames_arr= $this->classInfoArr[2]; 
        
        
        
        do {
            if ($colname=="") {
                $tmpcolErr[] = "column POS:$pos has no name!";
                $tmperr = -1;
                break;
            }
            
            if ($this->parx['colname.type']=='HUMAN') {

                try {
                    $colname = $this->col_nice2Raw($colnameOri);
                    $colname_nice = $colnameOri;
                } catch (Exception $e) {
                    $tmpcolErr[] = 'ColumnNice2Raw: '.$e->getMessage();
                    $tmperr = -10;
                    break;
                }
                
                debugOut('col_nice_TRAFO: '.$colnameOri.'=>'.$colname, $FUNCNAME, 1);
                
            }
            
            // check for DENIED NAMES
            foreach( $this->colwrong_arr as $wrongname) {
                if ($colname==$wrongname) {
                    $tmpcolErr[] = "column '$colname' not allowed";
                    $tmperr = -2;
                    break;
                }
            }
            
            if ($tmperr == -2 AND $_SESSION['sec']['appuser']=="root") {
                // reset error
                htmlErrorBox("Warning", "Column warning",  print_r($tmpcolErr,1).". But user 'root' can continue with risk!" );
                echo "<br>";
                $tmperr = 0;
                $tmpcolErr = array();
            }
            
            if ($tmperr<0) {
                break;
            }
            
            $part_info = $this->_col_part_info($colname);
            
            if ( $part_info[1]>0 ) {
                //
                // the identifiying column is a NAME => must be converted to an ID later
                //
                $colname  = $part_info[0];
                $colInfo = colFeaturesGet( $sqlo, $tablename, $colname);
                
                if ( $colInfo["CCT_TABLE_NAME"] == "" ) { // gives the NAME for the object, instead of PRIMARY_KEY !!!
                    $forTable = $tablename;
                } else {
                    $forTable = $colInfo["CCT_TABLE_NAME"];
                }
                if ($forTable=="") {
                    $tmpcolErr[] = "Extension ':NAME' is not allowed. Column is NOT a link to an other object-type!";
                    $tmperr = -8;
                    break;
                }
                
                $tmpcol = importantNameGet2($forTable);
                
                if ($tmpcol=="") {
                    $tmpcolErr[] = "Could not find an IMPORTANT name for table '$forTable'!";
                    $tmperr = -9;
                    break;
                }
                
                $tmparr=array();
                $tmparr[$pos]  = array ("tab"=> $forTable, "column"=> $tmpcol); // remember the transformation !
                $this->columnInfo["objName2ID"] = $tmparr;
                // continue ....
            }
            
            if ( $primcol == $colname ) {
                $this->headerInfo["primcol_pos"] = "$pos";
                break;
            }
            
            if  ( $this->columnInfo["importantName"] == $colname ) {
                $this->columnInfo["importantNamePos"] = "$pos";
                break;
            }
            
            
            if ($colname == "class")  {  // ignore column 'class'
                $rememberCol = 0;
                $tmpcolInfo .= "<span style='color:#808000'>ignored</span>";
                break;
            }
            
            // check for CLASS parameters
            if (substr($colname,0,2) == "c:") {
                
                $tmpcol = substr($colname,2);
                if ( !$class_id ) {
                    $tmpcolErr[] = "extra-class-column detected, but no class given.";
                    $tmperr = -3;
                    break;
                } else {
                    
                    $this->headerInfo["colClassAttribName_arr"][$pos] = $tmpcol;
                    $arguidarr = array_keys( $attribNames_arr ,$tmpcol);
                   
                    
                    if ( sizeof($arguidarr) ) {
                        $attrib_id            = $arguidarr[0];
                        $this->headerInfo["colClass_arr"][$pos]   = $attrib_id;
                        $tmpcolInfo           .= "<span style='color:green'>ok (class-atribute)</span>";
                        $rememberCol          = 0; // because it is a CLASS
                        
                        $class_id     = $this->classInfoArr[0];
                        $colname_nice =  $this->att_names_nice[$attrib_id];
                        $col_type = 'class';
                        
                    } else {
                        $tmpcolErr[] = "extra-class-column '".$tmpcol."' not defined.";
                        $tmperr = -4;
                        break;
                    }
                    
                }
                break;
            } else {
                $colInfo = colFeaturesGet( $sqlo, $tablename, $colname);
                $colname_nice = $colInfo['NICE_NAME'];
            }
            
            if  ( !in_array( $colname, $this->colGoodNames ) ) {
                $tmpcolErr[] = "column '$colname' is not known";
                $tmperr = -5;
                break;
            }
            
            
        } while (0);  
        
        $result=array(
            'colname'    => $colname,
            'col.nice'   => $colname_nice,
            'col.type'   => $col_type,
            'rememberCol'=> $rememberCol,
            'error'      => $tmperr,
            'colErr'     => $tmpcolErr,
            'colInfo'    => $tmpcolInfo
        );
        return $result;
    }
    
    /**
     * produce $this->col_nice_info
     * @param object $sqlo
     */
    private function _init2($sqlo) {
        global $varcol;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $tablename=$this->tablename;
        $cols = columns_get2 ( $tablename );
        
        $this->col_nice_info=array();
        $this->att_names_nice = array();
        
        foreach($cols as $col) {
            $features = colFeaturesGet($sqlo, $tablename, $col);
            $nice = $features['NICE_NAME'];
            $this->col_nice_info[$nice] = array('col'=>$col);
        }
        
        if (!empty($this->classInfoArr)) {
            $class_id    = $this->classInfoArr[0];
            $att_names_raw  = $this->classInfoArr[2];
            $this->att_names_nice = $varcol->get_attrib_nice_names($class_id);
            
            foreach($this->att_names_nice as $attr_id=>$nice) {
                $col_raw = $att_names_raw[$attr_id];
                $this->col_nice_info['c:'.$nice] = array('col'=>$col_raw, 't'=>'class');
            }
        }
        
        debugOut('col_nice_info: '.print_r( $this->col_nice_info,1), $FUNCNAME, 1);
    }
    
    /**
     * 
     * PARSE HEADER
     * - comment lines with "#" are allowed
     * 
     * @param  $sql
     * @param array parx
     *    'action' : "update", "insert"
     *    'colname.type' : ["RAW"], "HUMAN", "AUTO" 
     * @param int $isassoc
     * @param array $attribNames_arr
     * @param array $classInfoArr
           0: $class_id     
           1: $class_name     
           2: $attribNames_arr : array( $attrib_id => $name );
     * @param int $go    // 0: just prepare
                         // 1: all parameters should be fine
     * @param array $option  
     * 			// "motherid" = NUM (defines an ID for a primary KEY)
                //               useful, if you want to update/insert feature-elements only one OBJECT  
                // "infolevel"
       @return array with keys:
       		["columnInfo"]   array of
       		  "primcol"       =  name of PK
              "importantName" = importtant name of table (e.g. NAME)
              "objName2ID"    = the identifier is the NAME instead of ID
                  array( [$pos]  = array ("tab"=> table, "column"=> column) )
       		    
    	    ["primcol_pos"]  = $primcol_pos; 
    	    ["colClassAttribName_arr"]  = $colClassAttribName_arr;
    	    ["colClass_arr"] = $colClass_arr;
    	    ["col_arr"]      = array(POS=array('col'=>, 'nice'=>)
    	    ["headError"]    = $mainerr; 
    	    ["header_arr"]   = $header_arr; 
     */
    public function parseHead( &$sql, $parx,  &$classInfoArr,  $go,  $option = NULL  ) {
        global $error;            
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
        $this->parx = $parx;
        $action = $parx['action'];
        $tablename=$this->tablename;
        $filename=$this->filename ;
        $file_type=$this->file_type;
        $this->classInfoArr = $classInfoArr;
    
        $primcol        = PrimNameGet2($tablename);  
        $infolevel      = $option["infolevel"];       
   
        $colSql_arr     = array();  // array for SQL update 
        //$colHasClass    = 0;
        
        $this->_init2($sql);
        
     
      
      
        $this->headerInfo       = array();
        $this->headerInfo["colClassAttribName_arr"] = array();
        $this->headerInfo["colClass_arr"] = array();
      
        
        $this->columnInfo     = array();   // ["importantNamePos"]
        $this->columnInfo["primcol"]       =  $primcol;
        $this->columnInfo["importantName"] =  importantNameGet2($tablename);
        $this->columnInfo["objName2ID"]    =  array(); // the identifier is the NAME instead of ID
        $this->columnInfo["col_nice_info"]    =  $this->col_nice_info;
            // the tool has to transform the name to an ID
            // array [POS] => 0|1
           
        
       
        
        $this->colwrong_arr=array();
        $this->colwrong_arr[] ='CCT_ACCESS_ID';  
        $this->colwrong_arr[] ='EXTRA_OBJ_ID';
        
        $isassoc = 0;
        $mothertable = mothertable_get2($tablename);
        if ( $mothertable!='') {
            $isassoc = 1;
        }
        

        $importMode = "singleInsert";
        if ($action=="update" OR $isassoc ) $importMode = "objupdate";
        
        if ( $importMode == "singleInsert" ) {
            $this->colwrong_arr[] = $primcol;      // at INSERT of OBJECT, no PRIMARY KEY allowed
        }                                        

        
        //
        // read first line
        // 
        if (!file_exists($filename)) {
            $error->set( $FUNCNAME, 10, 'file "'.$filename.'" not exists.' );
            return;
        }
        $file_lib = new f_read_file_meta($filename, $file_type);
        $file_lib->open_file();
        $headers   = $file_lib->get_headers();
        
        $header_arr= $headers[0];
        $datafieldNum = sizeof($header_arr);   // is not a return value 
        
        if ($datafieldNum<1) {
            $error->set( $FUNCNAME, 12, "header contains no column!<br>Header:<pre>".print_r($header_arr,1)."</pre>" );
            $this->headerInfo["headError"] = -2;
            return ($this->headerInfo);
        }
        
        if ( $infolevel > 2 ) {
            $this->_debug_out( "header_parsing: <UL>");
        }             
    
        // ----------------------------------------------------------------
        $mainerr = 0;
        foreach( $header_arr as $pos=>$colnameOri ) {
 
    		$col_info   = $this->_one_column($sql, $pos, $colnameOri);
    		$colname    = $col_info['colname'];
    		$tmperr     = $col_info['error'];
    		$tmpcolErr  = $col_info['colErr'];
    		$tmpcolInfo = $col_info['colInfo'];
            
            if ($tmperr>=0 AND $col_info['rememberCol']) {
                $colSql_arr[$pos] = array('col'=>$colname, 'nice'=>$col_info['col.nice'] );      
                $tmpcolInfo .= "<span style='color:green'>ok</span>";
            }
                     
            if ( $infolevel > 2 OR ($tmperr<0) ) {
                if ($tmperr<0) $tmpcolInfo .=" <span style='color:red'><B>Error:</B> ". implode("; ",$tmpcolErr) ."</span>";
    			else $tmpcolInfo = "Info: ".$tmpcolInfo;
                $this->_debug_out( "pos:$pos column: '".$colnameOri."' ".$tmpcolInfo);
            } 
            if ($tmperr<0) $mainerr = -1;
            
        }
           
        
        if ( $infolevel > 2 ) {
            echo "</UL>";
        }
        
        //
        // FINAL TESTS 
        //
            
        if ( $importMode == "singleInsert" ) { 
        
            if ( $this->columnInfo["importantNamePos"] == "" ) { 
                    echo "<br>"; 
                    htmlErrorBox("Error", "at parsing the header. Contains no IMPORTANT name [".$this->columnInfo["importantName"]."]");  // for REDUNDANCY TEST     
                    $mainerr = -2;
            } 
            
        } else {
        
            if ( $this->headerInfo["primcol_pos"] == NULL ) {
                if ( $option["motherid"]=="" ) {
                    if ( $go ) {  
                        htmlErrorBox("Error", "Object-ID missing.",
                                 "If you want to update only ONE object, give the Object-ID in the parameter-form");       
                        $mainerr = -3;
                    }
                }
            }
            
        }
        
        //$this->_analyseColTypes( $colSql_arr );
 
        $this->headerInfo["columnInfo"]   = $this->columnInfo;
        $this->headerInfo["col_arr"]   = $colSql_arr;
        $this->headerInfo["headError"]    = $mainerr; 
        $this->headerInfo["header_arr"]   = $header_arr; 
        #$this->headerInfo["notesPos"]		= index($header_arr);
        
        return ( $this->headerInfo );
    } 
    



}


