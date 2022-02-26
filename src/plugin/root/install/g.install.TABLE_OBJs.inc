<?php
require_once 'insert.inc';
require_once ("insertx.inc");
require_once 'object.info.inc';
require_once 'f.sql_query.inc';

/**
 * create basic OBJECTS, if not exist
 * - read defs from file o.xxxx.defs.json
 * @package g.install.TABLE_OBJs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de)
 * @param $action
 *   'update'
 *   'export'
 *   
 *  $parx
 *    JSON
 *    
 
 */
class g_install_TABLE_OBJs {
    
    function __construct($tablename) {
        $this->tablename=$tablename;
        $this->pkname=PrimNameGet2($tablename);
        $this->ident_col = importantNameGet2($tablename); 
        
        if (!$this->ident_col) {
            throw new Exception('Missing importantName in table '.$tablename);
        }
        
        $this->go=0;
        echo "IDENTITY-Column: ".$this->ident_col."<br>";
    }
    
    function set_go($go) {
        $this->go=$go;
    }
    /**
     *
     * @param object $sqlo
     * @param array $row
     * @return array  objFeatStruct
     */
    private function export_one(object $sqlo, $obj_id) {
        global $error;
        
        $tablename=$this->tablename;
        $obj_info_lib = new objInfoC();
        $pkname=PrimNameGet2($tablename);
        
        $class_data = $obj_info_lib->getParamsRaw($sqlo, $tablename, $obj_id);
        unset($class_data['vals'][$pkname]);
        unset($class_data['vals']['EXTRA_OBJ_ID']);

        // clean output
        $vals=array();
        foreach($class_data['vals'] as $key=>$val) {
            if ($val!==NULL) {
                $vals[$key]=$val;
            }
        }
        $output = array('vals'=>$vals);
        
        // $output['ass'] = array('EXTRA_ATTRIB'=>$class_cols);
        return $output;
    }
    
    function export($sqlo, $sqlo2, $sqlAfter) {
        
        $tablename=$this->tablename;
        echo "Export definitions.";
        
        $struct_all=array();
        
        $pkname=PrimNameGet2($tablename);
        $sqlsel = 'x.'.$pkname." from ".$sqlAfter;
        $sqlo2->Quesel($sqlsel);
        while ( $sqlo2->ReadRow() ) {
            $tmpid = $sqlo2->RowData[0];
            $loop_out = $this->export_one($sqlo, $tmpid);
            $struct_all[] = $loop_out;
        }
        
        
        echo "<pre>\n";
        echo json_encode($struct_all,  JSON_PRETTY_PRINT);
        echo "</pre>\n";
        echo "\n";
    }
    
    /**
     * analyse $inp_struct
     * @param object $sqlo
     * @param array $row
     */
    private function create_one($sqlo, $args) {
        global $error;
        $tablename=$this->tablename;
        
        
        // insert now
        if ($this->go) {
     
            $insertlib = new insertC();
            $new_id = $insertlib->new_meta($sqlo, $tablename, $args);

            if (!$error->Got(READONLY))  {
                echo "ID: ".$new_id." Created<br>\n";
            }
        }
    }
    
    function update($sqlo, $text) {
        global $error;
        $tablename=$this->tablename;
        
//         $filename= dirname(__FILE__).'/'.self::CONFIG_JSON_FILE;
//         if (!file_exists($filename)) {
//             throw new Exception('File "'.$filename.'" not found.');
//         }
//         $text = file_get_contents($filename);
//        $input_source= 'File "'.$filename.'"';
        $input_source='JSON-string';
        
        $inp_struct = json_decode($text, TRUE);
        if (!is_array($inp_struct)) {
            throw new Exception( $input_source.' has no valid JSON struct.');
        }
        if (empty($inp_struct)) {
            throw new Exception($input_source. ' is empty.');
        }
        
        $ident_col = $this->ident_col;

        foreach($inp_struct as $row) {
            
            $important_val= $row['vals'][$ident_col];
            $pk_array=array($ident_col=>$important_val);
            
            if ( gElement_exists2($sqlo, $tablename, $pk_array) ) {
                echo 'Object :'.$important_val.": EXISTS<br>\n";
            } else {
                echo "Handle Object: ".$important_val." ... <br>\n";
                $this->create_one($sqlo, $row);
                if ($error->Got(READONLY))  {
                    $error->printAll();
                    echo "<br>";
                }
            }
        }
    }
    
}



class  g_install_TABLE_OBJs_XPL extends gPlugin {
    
    // const CONFIG_JSON_FILE='o.EXTRA_CLASS.defs.json';

	function register() {
		
		$title		  = 'Export/Create basic Objects with JSON';
		$tablename    = $_REQUEST['t'];

		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'tool';
		$this->infoarr['design']   = 'norm';
		$this->infoarr['locrow']   = array( array('rootsubs/init/index.php', 'home') );
		
		if ($tablename!=NULL) {
		    $this->infoarr['form_type']= 'list';
		    $this->infoarr['obj_name'] = $tablename;
		}

		$this->infoarr2 = array(
				'onlyAdmin'=>1
		);

	}

	
	function form1() {

		$initarr   = NULL;
		$initarr["title"]       = "Update!"; // title of form
		$initarr["submittitle"] = "Update/Create"; // title of submit button
		$initarr["tabwidth"]    = "AUTO";   // table-width: AUTO

		$hiddenarr = array();
		$hiddenarr['action']='update';
		$hiddenarr['t'] = $this->tablename;
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
		
		
		$fieldx = array (
		    "title" => "JSON",
		    "name"  => "JSON",
		    "object"=> "textarea",
		    "val"   => '',
		    "notes" => "JSON string"
		);
		$formobj->fieldOut( $fieldx );
	
		$formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	function form2($parx) {
	    
	    $initarr   = NULL;
	    $initarr["title"]       = "Update!"; // title of form
	    $initarr["submittitle"] = "Update/Create"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    
	    $hiddenarr = array();
	    $hiddenarr['action']='update';
	    $hiddenarr['t'] = $this->tablename;
	    $hiddenarr['parx[JSON]'] = $parx['JSON'];
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 1);

	    $formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	
	function form_tablesel() {
	    $initarr   = NULL;
	    $initarr["title"]       = "Select table"; // title of form
	    $initarr["submittitle"] = "Select"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    
	    $hiddenarr = array();
	   
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
	    $fieldx = array (
	        "title" => "Table",
	        "name"  => "t",
	        'namex' => 1,
	        "object"=> "text",
	        "val"   => '',
	        "notes" => "e.g. EXP"
	    );
	    $formobj->fieldOut( $fieldx );
	    $formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	function form_export1($parx) {
	    $initarr   = NULL;
	    $initarr["title"]       = "Export"; // title of form
	    $initarr["submittitle"] = "Export now"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    
	    $hiddenarr = array();
	    $hiddenarr['action']='export';
	    $hiddenarr['t'] = $this->tablename;
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
	    
	    $formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	

	function startMain() {
	    global $error, $varcol;
		// $FUNCNAME='startMain';
		
		
		
		$this->standards=array(
		    array('t'=>'LINK', 'name'=>'query')
		);
		
		$go    = $_REQUEST['go'];
		$tablename    = $_REQUEST['t'];
		$action   = $_REQUEST['action'];
		$parx   = $_REQUEST['parx'];
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		if ($tablename==NULL) {
		  $this->form_tablesel();
		  return;
		} 
		$this->tablename=$tablename;
		
		if ($action==NULL) $action='export';
		
		$this->go = $go;
		
		echo '<a href="'.$this->_plug_get_URL().'&action=export&t='.$tablename.'">Export defaults</a> &nbsp;&nbsp;&nbsp;';
		echo '<a href="'.$this->_plug_get_URL().'&action=update&t='.$tablename.'">Create defaults</a>';
		// echo '  &nbsp;&nbsp;&nbsp; CONFIG-file: '.self::CONFIG_JSON_FILE;
		echo "<br><br>";
		
		switch ($action) {
		    
		    case 'export':
		        
		        $utilLib = new fSqlQueryC($tablename);
		        $sqlAfter = $utilLib->get_sql_after( );
		        if ($utilLib->get_sql_info()==NULL) {
		            $this->htmlFoot('WARNING','Please select elements.');
		        }
		        
		       
		        $sqlsel = "count(1) from ".$sqlAfter;
		        $sqlo->Quesel($sqlsel);
		        $sqlo->ReadRow();
		        $obj_cnt = $sqlo->RowData[0];
		        
		        echo $obj_cnt.' objects selected.<br>';
		        $MAX_OBJ=500;
		        if($obj_cnt>$MAX_OBJ) {
		            $this->htmlFoot('WARNING','Too many objects. MAX:'.$MAX_OBJ);
		        }
		        
		        if (!$go) {
		            $this->form_export1();
		            return;
		        }
 

		        
		        $table_lib = new g_install_TABLE_OBJs($tablename);
		        $table_lib->export($sqlo, $sqlo2, $sqlAfter);
		        
		        break;
		        
		    case 'update':
		        
		        $table_lib = new g_install_TABLE_OBJs($tablename);
		        
        		if (!$go) {
        			$this->form1();
        			return;
        		}
        		$json_str = $parx['JSON'];
        		if ($json_str==NULL) {
        		    $this->htmlFoot('WARNING','Input-JSON string missing!');
        		}
        		if ($go==1) {
        		    $this->form2($parx);
        		}
        		

        		
        		if ($go==2) $table_lib->set_go(1);
        		
        		$table_lib->update($sqlo,  $json_str);
		}
		
	}

}
