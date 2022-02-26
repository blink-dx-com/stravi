<?php
require_once 'insert.inc';
require_once 'object.info.inc';
require_once 'o.EXTRA_CLASS.mod.inc';

/**
 * create basic CLASSES, if not exis
 * - read defs from file o.EXTRA_CLASS.defs.json
 * @package g.install.CLASSES.inc
 * @author  Steffen Kube (steffen.kube@gmx.de)
 * @param $action
 *   'update'
 *   'export'
 */


class  g_install_CLASSES_XPL extends gPlugin {
    
    const CONFIG_JSON_FILE='o.EXTRA_CLASS.defs.json';

	function register() {
		
		$title		  = 'Create basic CLASSES';

		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'tool';
		$this->infoarr['design']   = 'norm';
		$this->infoarr['locrow']   = array( array('rootsubs/init/index.php', 'home') );

		$this->infoarr2 = array(
				'onlyAdmin'=>1
		);

	}

	
	function form1() {

		$initarr   = NULL;
		$initarr["title"]       = "Update!"; // title of form
		$initarr["submittitle"] = "Submit"; // title of submit button
		$initarr["tabwidth"]    = "AUTO";   // table-width: AUTO

		$hiddenarr = array();
		$hiddenarr['action']='update';
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);

	
		$formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	function form_export() {
	    $initarr   = NULL;
	    $initarr["title"]       = "Export"; // title of form
	    $initarr["submittitle"] = "Submit"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    
	    $hiddenarr = array();
	    $hiddenarr['action']='export';
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);

	    $formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	/**
	 * analyse $this->classes_DEF_struct
	 * @param object $sqlo
	 * @param array $row
	 */
	private function create_one($sqlo, $row) {
	    global $error;
 
	    $found=0;
	    foreach( $this->classes_DEF_struct as $index=>$def_row ) {

	        $DEF_class_table = $def_row['vals']['TABLE_NAME'];
	        $DEF_cls_name    = $def_row['vals']['NAME'];
	        
	        if ($row['t']==$DEF_class_table and $row['name']==$DEF_cls_name)  {
	            $found=1;
	            break;
	        }
	    }
	    
	    if(!$found) {
	        throw new Exception('Class DEF for TABLE:"'.$row['t'].'", NAME:"'.$row['name'].'" not found.');
	    }
	    
	    // insert now
	    if ($this->go) {
    	    $class_ofs = &$def_row;
    	    $class_id  = oEXTRA_CLASS_new::create_by_OFS($sqlo, $class_ofs);
    	    
    	    if (!$error->Got(READONLY))  {
    	       echo "Created<br>\n";
    	    }
	    }
	}
	
	function update($sqlo) {
	    global $error;
	    
	    $filename= dirname(__FILE__).'/'.self::CONFIG_JSON_FILE;
	    if (!file_exists($filename)) {
	        throw new Exception('File "'.$filename.'" not found.');
	    }
	    
	    $text = file_get_contents($filename);
	    $this->classes_DEF_struct = json_decode($text, TRUE);
	    if (!is_array($this->classes_DEF_struct)) {
	        throw new Exception('File "'.$filename.'" has no valid JSON struct.');
	    }
	    if (empty($this->classes_DEF_struct)) {
	        throw new Exception('File "'.$filename.'" is empty.');
	    }
	    
	    $standards = &$this->standards;
	    
	    $sqlsel = "* from EXTRA_CLASS order by TABLE_NAME, NAME";
	    $sqlo->Quesel($sqlsel);
	    while ( $sqlo->ReadArray() ) {
	        
	        $loop_id = $sqlo->RowData['EXTRA_CLASS_ID'];
	        $loop_name = $sqlo->RowData['NAME'];
	        $table = $sqlo->RowData['TABLE_NAME'];
	        $nice  = $sqlo->RowData['NICE_NAME'];
	        
	        foreach($standards as $index=>$row) {
	            if ($row['t']==$table and $row['name']==$loop_name)  {
	                $standards[$index]['found']=1;
	            }
	        }
	    }
	    
	    foreach($standards as $index=>$row) {
	        
	        if ($row['found']) {
	            echo "Class ".$row['t'].':'.$row['name'].": EXISTS<br>\n";
	        } else {
	            echo "Handle Class ".$row['t'].':'.$row['name']." ... <br>\n";
	            $this->create_one($sqlo, $row);
	            if ($error->Got(READONLY))  {
	                $error->printAll();
	                echo "<br>";
	            }
	        }
	    }
	}
	
	/**
	 * 
	 * @param object $sqlo
	 * @param array $row
	 * @return array  objFeatStruct
	 */
	private function export_one(object $sqlo, array $row) {
	    global $error, $varcol;
	    
	    $class_table='EXTRA_CLASS';
	    $assocTable='EXTRA_ATTRIB';
	    
	    $class_id	= $varcol->class_name_to_id( $row['t'],  $row['name'] );
	    if ($error->Got(READONLY))  {
	        // class not found
	        $error->reset();
	        return;
	    }
	    
	    $obj_info_lib = new objInfoC();
	    
	    $class_data = $obj_info_lib->getParamsRaw($sqlo, $class_table, $class_id);
	    unset($class_data['vals']['EXTRA_CLASS_ID']);
	    
	    $class_cols = array();
	    $sqlsel= "* from EXTRA_ATTRIB where EXTRA_CLASS_ID=".$class_id. " order by POS";
	    $sqlo->Quesel($sqlsel);
	    while ( $sqlo->Readarray() ) {
	        $tmp = $sqlo->RowData;
	        unset($tmp['EXTRA_CLASS_ID']);
	        unset($tmp['EXTRA_ATTRIB_ID']);
	        
	        $class_cols[] = $tmp;
	    }
	    
	    $output = $class_data;
	    $output['ass'] = array('EXTRA_ATTRIB'=>$class_cols);
	    return $output;
	}
	
	function export($sqlo) {
	    echo "Export definitions.";
	    $standards = &$this->standards;
	    $struct_all=array();
	    
	    foreach($standards as $index=>$row) {
	        $loop_out = $this->export_one($sqlo, $row);
	        $struct_all[] = $loop_out;
	    }
	    
	    echo "\n";
	    echo json_encode($struct_all,  JSON_PRETTY_PRINT);
	    echo "\n";
	    echo "\n";
	}

	function startMain() {
	    global $error, $varcol;
		// $FUNCNAME='startMain';
		
		$varcol = & Varcols::get();
		
		$this->standards=array(
		    array('t'=>'LINK', 'name'=>'query')
		);
		
		$go   = $_REQUEST['go'];
		$action   = $_REQUEST['action'];
		$sqlo = &$this->_sqlo;
		
		$this->go = $go;
		
		echo '<a href="'.$this->_plug_get_URL().'&action=export">Export defaults</a> &nbsp;&nbsp;&nbsp;';
		echo '<a href="'.$this->_plug_get_URL().'&action=update">Create defaults</a>';
		echo '  &nbsp;&nbsp;&nbsp; CONFIG-file: '.self::CONFIG_JSON_FILE;
		echo "<br><br>";
		
		switch ($action) {
		    case 'export':
		        if (!$go) {
		            $this->form_export();
		            return;
		        }
		        $this->export($sqlo);
		        break;
		    default:   
		        
        		if (!$go) {
        			$this->form1();
        		}
        		$this->update($sqlo);
		}
		
	}

}
