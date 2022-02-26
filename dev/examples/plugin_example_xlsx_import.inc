<?php
/**
 * import QC results
 * @namespace core::obj:ABSTRACT_SUBST
 * @package  o.CONCRETE_SUBST.beadqc_imp.inc
 * @author  Steffen Kube (steffen@clondiag.com)
 * @param 
	[$go]    ==   0,1,2
	[$parx]
	
   @param $_FILES['userfile'] - uploaded file
   
 */

require_once ('func_form.inc');
require_once ("f.upload.inc");
require_once ('f.msgboxes.inc');
require_once ("visufuncs.inc");
require_once ("f.objview.inc");	

require_once ("import/f.read_file_meta.inc");
require_once ("import/f.spreadsheet_ana.inc");
require_once 'f.debug.inc';

require_once 'o.ABSTRACT_PROTO.stepx.inc';
require_once 'o.CONCRETE_SUBST.proto.inc';




class o_CONCRETE_SUBST_beadqc_imp_XPL extends gPlugin {
	
	
	function register() {
		$tablename	= 'CONCRETE_SUBST';
		$title		= 'Bead-QC: import Excel-QC-file';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['obj_name'] = $tablename;

		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('write') )
			);
	}
	
	function help() {
	    
     	echo '<br><br>';
     	
    	htmlInfoBox( "Short help", "", "open", "HELP" );
    	?>
    	<ul>
    	<LI>Version: 22.01.2021</LI>
     	<LI>Bead-QC: import Excel-QC-file</LI>
     	<LI>[<a href="../lab_blk/images/Bead-QC-example.xlsx">Download Excel-Example-File</a>]</LI>
		</ul>
		<?php
    	htmlInfoBox( "", "", "close" );
    }
    
	
	function form1() {

	    $initarr   = array();
	    $initarr["title"]       = "Give Excel-file";
	    $initarr["submittitle"] = "Upload file"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    $initarr["ENCTYPE"]     = "multipart/form-data";
	   
	    
	    $hiddenarr = NULL; // hidden form variables
	    

	    $formobj = $this->_html->formInit($initarr, $hiddenarr);
	    
	    $fieldx = array ( // form-field definition
	        "title" => "QC-File",
	        "name"  => "userfile",
	        "namex" => TRUE,
	        "object"=> "file",
	        "val"   => '',
	        
	    );
	    $formobj->fieldOut( $fieldx ); // output the form-field
	    
	    $formobj->close( TRUE );
	    
	    
	}
	
	function form2($tmpfile) {
	    $initarr   = array();
	    $initarr["title"]       = "Import Protocol data";
	    $initarr["submittitle"] = "Import now!"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	  
	    
	    
	    $hiddenarr = NULL; // hidden form variables
	    $hiddenarr["userfile"]     = $tmpfile;
	    $hiddenarr["short_file_name"] = $this->newName;
	    
	    
	    foreach($this->options as $key => $val) {
	        $hiddenarr["parx[".$key."]"]          = $val;
	    }
	    
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 1);
	    
	    
	    $formobj->close( TRUE );
	    
	    
	}
	
	/**
	 * import file and create steps
	 *
	 * @param $sql
	 * @param $sql2
	 * @param $tempfile
	 * @param $go
	 * @return NULL
	 */
	function import_file( &$sqlo, &$sqlo2, $tempfile, $go) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;

	   
	    $file_lib = new f_read_file_meta($tempfile);
	    $file_lib->open_file();
	    $headers  = $file_lib->get_headers();
	    $header   = $headers[0];
	    
	    $this->encoding_key = $file_lib->get_encoding_info();
	 
	    $this->head_alias_arr=array();
	    
	    $header_other_cols=array(
	        'MAC',
	    );
	    
	    // special proto keys
	    $proto_keys=array(
	        'Mikroskop'=>array('type'=>'DEV_ID'),
	        'Path'    =>array('type'=>'NOTES'),
	    );
	    
	    $i=0;
	    foreach($header as $col) {
	        
	        foreach($header_other_cols as $defcol) {
	            if ($col==$defcol) {
	                $this->head_alias_arr[$col]=$i;
	                break;
	            }
	        }
	        $i++;
	    }
	    
	    if (empty($this->head_alias_arr)) {
	        if ($error->Got(READONLY))  {
	            $error->set( $FUNCNAME, 1, 'Defined Header-keys ('.implode(',',$defkeys).') not found.' );
	            return;
	        }
	    }
	    
	    $pra_lib = new oABSTRACT_PROTO_stepx($this->pra_id);
	    $pra_name_arr = $pra_lib->getStepArray_i2($sqlo, array('NAME') );
	    
	    $pra_step_arr=array();
	    
	    foreach($header as $i=>$col) {
	        
	        if (in_array($col, $header_other_cols)) {
	            // other column
	        } else {
	            
	            $found=0;
    	        foreach($pra_name_arr as $pra_row) {
    	            
    	            $pra_name = $pra_row['NAME'];
    	            
    	            if ($col == $pra_name) {
    	                
    	                $data_type='QUANTITY';
    	                if (!empty($proto_keys[$col]) ) {
    	                    $data_type=$proto_keys[$col]['type'];
    	                }
    	                
    	                $step_nr = $pra_row['STEP_NR'];
    	                $pra_step_arr[]= array('hpos'=>$i, 'st'=>$step_nr, 'ty'=>$data_type, 'name'=>$pra_name);
    	                $found=1;
    	                break;
    	            }
    	        }
    	        
    	        if (!$found) {
    	            $error->set( $FUNCNAME, 2, 'File-Column "'.$col.'" not found in Protocol.');
    	            return;
    	        }
	        }
	        $i++;
	        
	    }
	    
	    
	    if ( $error->Got(READONLY) ) {
	        $type= $file_lib->get_type();
	        echo 'File-Type: '.$type.' Header: '.print_r($header,1)."<br>";
	        $error->set( $FUNCNAME, 1, "File-Header analysis failed.");
	        return;
	    }
	    
	    $bead_qc_lib = new oSUC_beadqc_ONE( $this->head_alias_arr, $pra_step_arr, $this->sua_id , $this->pra_id);
	    

	    $tabobj = new visufuncs();
	    $headOpt = array( "title" => "Import Overview" );
	    
	    $header_tab = array_merge( array('MAC', 'Status'),  $header,array('Errors') );
	    
	    $tabobj->table_head($header_tab,   $headOpt);

	    
	    while( $file_lib->read_line(0) )  {
	        
	        $datafields_raw  = $file_lib->get_data_row();
	        $suc             = $datafields_raw[0];
	        $datafields_raw  = $file_lib->get_data_row();
	        $bead_qc_lib->import($sqlo, $datafields_raw, $go-1);
	        $allErrTxt='';
	        $status='OK';
	        
	        if ($error->Got(READONLY))  {
	            $errLast   = $error->getLast(0,0,READONLY);
	            $error_txt = $errLast->text;
	            $error_id  = $errLast->id;
	            
	            $allErrTxt = $error->getAllAsText();
	            
	            if ($error_id>=100) {
	                
	                $status='<span style="color:#AE70FF;">Warning</span>';
	                $allErrTxt = '<span style="color:#AE70FF">Warning:</span> '.$allErrTxt;
	                
	            } else {
 
	               $status='<span style="color:red;">Error</span>';
	               $allErrTxt = '<span style="color:red">ERROR:</span> '.$allErrTxt;
	            }
	            $error->reset();
	           
	        }
	        
	        
	        
	        $html_tmp = fObjViewC::bo_display( $sqlo, 'CONCRETE_SUBST', $suc );
	        $start_arr=array($html_tmp, $status);
	        $data_tmp = array_merge($start_arr, $datafields_raw, array($allErrTxt) );
	        $tabobj->table_row ($data_tmp);
	        
	    }
	    $tabobj->table_close();
	    
	    $file_lib->close_file();

	}
	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		
		$this->sua_id = 1247;
		$this->pra_id = 455;
		
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		$go      = $_REQUEST['go'];
		$parx    = $_REQUEST['parx'];
		
		$short_file_name = $_REQUEST['short_file_name'];
		
		$this->uploadLib = new uploadC();
		$this->tmp_sub_dir = str_replace('/', '_',  $this->_mod );
		

		if (empty($go)) $go = 0;

		
		if ( !$go  ) {
		    $this->form1();
		    $this->help();
		    return;
		}
		
		if ( $go==1 ) {
		    
		    
		    $userfile_size = $_FILES['userfile']['size'];
		    $userfile      = $_FILES['userfile']['tmp_name'];
		    $userfile_name = $_FILES['userfile']['name'];

		    $this->newName = $userfile_name;
		    $tmpfile = $this->uploadLib->mvUpload2Tmp( $this->tmp_sub_dir, $this->newName, $userfile, $userfile_name, $userfile_size );
		    if ($error->Got(READONLY))  {
		        return;
		    }
		    
		    $this->form2($tmpfile);
		}
		
		if ( $go == 2 ) {
		    
		    $this->newName = $short_file_name;
		    
		    $tmpfile = $this->uploadLib->getFileFromTmp($this->tmp_sub_dir, $this->newName);
		    if ($error->Got(READONLY))  {
		        return;
		    }
		}
		
		$this->import_file( $sqlo, $sqlo2, $tmpfile, $go );
		if ($error->Got(READONLY))  {
		   return;
		}

	}
}
