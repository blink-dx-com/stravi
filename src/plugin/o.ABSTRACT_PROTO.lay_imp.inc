<?php
/**
 * import LAYOUT for protocol steps from spreadsheet file
 * @namespace core::obj:ABSTRACT_SUBST
 * @package  o.ABSTRACT_PROTO.lay_imp.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param 
 *  $id
	[$go]    ==   0,1,2
	[$infolevel]
	[$subst_create] 0|1  create substance, if not found
	$short_file_name : short filename
	$action: optional: 'download' (template for import)
	
    @param $_FILES['userfile'] - uploaded file
 */

require_once ('func_form.inc');
require_once ("glob.obj.update.inc");
require_once ("f.upload.inc");
require_once ('f.msgboxes.inc');
require_once ("visufuncs.inc");
require_once ('lev1/f.exportDataFile.inc');

require_once ("import/f.read_file_meta.inc");
require_once 'f.debug.inc';
require_once 'gui/o.ABSTRACT_PROTO.layout.inc';





class oPRA_lay_impC {
    
    function __construct( $sqlo, $proto_id, $scriptid ) {
        
        $this->proto_id = $proto_id;
        $this->newName   = '';
        $this->scriptid  = $scriptid;
        $this->uploadLib = new uploadC();
        
        $this->pra_layout_lib = new o_ABSTRACT_PROTO_layout($sqlo, $proto_id);
        $this->pra_steps = $this->pra_layout_lib->get_steps_raw();
        
        $this->tmp_sub_dir = str_replace('/', '_',  $this->scriptid );
    }
    
    private function _info_out($key, $val) {
        echo $key.": ".$val."<br>\n";
    }
    
    function form1() {
        
        $proto_id = $this->proto_id;
        
        $initarr   = array();
        $initarr["title"]       = "Import Layout";
        $initarr["submittitle"] = "Upload file"; // title of submit button
        $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
        $initarr["ENCTYPE"]     = "multipart/form-data";
        $initarr["action"]    = "p.php";
        
        $hiddenarr = NULL; // hidden form variables
        $hiddenarr["id"]      = $proto_id; // is a required input parameter
        $hiddenarr["mod"] = $this->scriptid;
        
        
        $formobj = new formc($initarr, $hiddenarr, 0);
        
        $fieldx = array ( // form-field definition
            "title"   => "Layout file",
            "name"  => "userfile",
            "namex" => TRUE,
            "object"=> "file",
            
            
        );
        $formobj->fieldOut( $fieldx ); // output the form-field

        $formobj->close( TRUE ); // close form, sshow submit-button
        
        
    }
    
    function form2($tmpfile) {
        
        $proto_id = $this->proto_id;
        
        $initarr   = array();
        $initarr["title"]       = "Import Layout";
        $initarr["submittitle"] = "Import now!"; // title of submit button
        $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
        $initarr["action"]    = "p.php";
        
        
        $hiddenarr = NULL; // hidden form variables
        $hiddenarr["id"] = $proto_id; // is a required input parameter
        $hiddenarr["userfile"] = $tmpfile;
        $hiddenarr["short_file_name"] = $this->newName;
        $hiddenarr["mod"] = $this->scriptid;
        
        $formobj = new formc($initarr, $hiddenarr, 1);
        
        
        $formobj->close( TRUE );
        
        
    }
    
    function help() {
        ?>
     	<br><br>
     	<?
    	htmlInfoBox( "Short help", "", "open", "HELP" );
    	?>
    	<ul>
    	<LI>Version: 23.6.2020</LI>
     	<LI>import new protocol steps from <b>Excel</b> or <b>CSV-File</b> (TAB-separated-text file)</LI>
     	<li>CSV: can be exported from from Excel, OpenOffice or comparable tools</LI>
     	<li>detects the encoding types UTF-8 or ISO-8859-1</li>
     	
    	</ul>
    	<br>
    	<?php
    	htmlInfoBox( "", "", "close" );
   }
   
   function moveFile($userfile, $userfile_name, $userfile_size) {
       
       $this->newName = $userfile_name;
       $tmpfile = $this->uploadLib->mvUpload2Tmp( $this->tmp_sub_dir, $this->newName, $userfile, $userfile_name, $userfile_size );
       return ($tmpfile);
   }
   
   function getTempFile() {
       $tmpfile = $this->uploadLib->getFileFromTmp($this->tmp_sub_dir, $this->newName);
       return ($tmpfile);
   }
   
   function set_short_file_name($short_file_name) {
       $this->newName = $short_file_name;
   }
   
   function _ana_header($row) {
       
       $this->layout['head']=array();
       
       $tmpa=array();
       foreach($row as $cell) {
           $cell=trim($cell);
           if ($cell==='') {
               throw new Exception('Header-column '.($i+1).' is empty');
           }
           $tmpa[]=$cell;
       }
       $this->layout['head'] = $tmpa;
       
       
       $this->tabobj = new visufuncs();
       $headOpt = array( "title" => "title-text" );
       $headx   = array_merge(array('#'), $tmpa);
       $this->tabobj->table_head($headx,   $headOpt);
   }
   
   function _ana_row($row, $rowid) {

       $has_data=0;
       $tmpa=array();
       $help_row=array();
       $colid=0;
       $help_row[]=$rowid+1;
       
       foreach($row as $cell) {
           $cell=trim($cell);
           if ($cell!='') {
               $has_data=1;
           }
           
           try {
               $c_info = $this->pra_layout_lib->check_cell($cell);
           } catch (Exception $e) {
               $mess = $e->getMessage();
               throw new Exception('Row:'.($i+1).':'.($colid+1).': '.$mess);
           }
           
           $tmpa[]=$cell;
           
           
           // get name
           $stepnr = $c_info['st'];
           $pra_st_row = $this->pra_steps[$stepnr];
           $st_name=$pra_st_row['NAME'];
           if (strlen($st_name)>20) $st_name=substr($st_name,0,20).'..';
           $help_cell = $cell.' '.$st_name;
           
           $help_row[]=$help_cell;
           
           $colid++;
       }
       
       if (!$has_data) {
           throw new Exception('Data-row '.$rowid.' is empty');
       }
       
       $this->layout['rows'][] = $tmpa;
       
       $this->tabobj->table_row ($help_row);
   }
   
   /**
    * import file and create steps
    *
    * @param $sql
    * @param $sql2
    * @param $tempfile
    * @param $go
    * @param $infolevel
    * @param $subst_create
    * @return NULL
    */
   function import_file( &$sqlo, $tempfile, $go, $infolevel) {
       global $error;
       $FUNCNAME= 'import_file';

       $this->layout=array();
       $this->layout['rows']=array();
 
       
       
       $file_lib = new f_read_file_meta($tempfile);
       $file_lib->open_file();
       //$headers  = $file_lib->get_headers();
       //$header   = $headers[0];
       
       $this->encoding_key = $file_lib->get_encoding_info();
       $this->_info_out('Encoding', $this->encoding_key);
       
       $headers = $file_lib->get_headers();
       $this->_ana_header($headers[0]);
       
       $i=0;
       while( $file_lib->read_line(0) )  {
           $datafields_raw  = $file_lib->get_data_row();
         
           
           $this->_ana_row($datafields_raw, $i);
           
           $i++;
       }
       $file_lib->close_file();
       
       $this->tabobj->table_close();
       
       if ($go==2) {
          
           $json_str = json_encode($this->layout);
           $args = array(
               'vals'=>array(
                  'LAYOUT'=>$json_str
              )
               
           );
           
           debugOut('json_str: '.$json_str, $FUNCNAME, 1);
           
           $UpdateLib = new globObjUpdate();
           $UpdateLib->update_meta( $sqlo, 'ABSTRACT_PROTO', $this->proto_id, $args );
       }
   }
}

class o_ABSTRACT_PROTO_lay_imp_XPL extends gPlugin {
	
	
	function register() {
		$tablename	= 'ABSTRACT_PROTO';
		$title		= 'Import LAYOUT for protocol (abstract) step list';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		//$this->infoarr["help_url"] = "o.ABSTRACT_PROTO.import.html";
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('write'), 'obj'=>array('read', 'write') )
			);
	}
	
	function export($sqlo, $proto_id) {
	    // abstract_proto features
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $pra_layout_lib = new o_ABSTRACT_PROTO_layout($sqlo, $proto_id);
	    $layout = $pra_layout_lib->load_layout($sqlo);
	    
	  
	    
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 1, 'Layout problem.' );
	        return;
	    }
	    
	    $tmp_filename = "protocol.layout.xlsx";
	    
	    $tmp_sub_dir = str_replace('/', '_',  $this->_mod );
	    $exportObj = new f_exportDataFile_C('xlsx', $tmp_sub_dir, $tmp_filename);
	    
	    $head1 = $layout['head'];
	    
	    $headerx = array( $head1 );
	    $exportObj->outputStart( $headerx );
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 2, 'File open problem.' );
	        return;
	    }
	    
	    foreach($layout['rows'] as $row) {
	       $row_out=$row;
	       $exportObj->oneRow($row_out);
	    }
	    $exportObj->close();
	    echo $exportObj->getDownloadText();
	    
	}

	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$tablename = "ABSTRACT_PROTO";
		
		$sqlo = &$this->_sqlo;
		//$sqlo2= &$this->_plug_regSql2();
		
		$proto_id = $_REQUEST['id'];
		$go       = $_REQUEST['go'];
		$infolevel= $_REQUEST['infolevel'];

		$short_file_name = $_REQUEST['short_file_name'];
		
		echo "<ul>";
		echo '[<a href="p.php?mod=DEF/o.ABSTRACT_PROTO.lay_imp&id='.$proto_id.'&action=exp">Export Layout</a>]<br><br>';
		
		if (empty($go)) $go = 0;
		
		$mainLib = new oPRA_lay_impC($sqlo, $proto_id, $this->_mod);

		
		$sqls= "count(1) from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$proto_id;
		$sqlo->Quesel($sqls);
		$sqlo->ReadRow();
		$numsteps=$sqlo->RowData[0];
		if ( !$numsteps) {
		    echo "<br>";
		    $this->_html->messageBox2( "warning", "No steps defined." );
		    return;
		}
		
		if ( $_REQUEST['action']=='exp') {
		    $this->export($sqlo, $proto_id);
		        
		    return;
		}
		
		if ( !$go  ) {
		    $mainLib->form1();
		    $mainLib->help();
		    return;
		}
		
		if ( $go < 2 ) {
		    
		    $userfile_size = $_FILES['userfile']['size'];
		    $userfile  = $_FILES['userfile']['tmp_name'];
		    $userfile_name = $_FILES['userfile']['name'];
		    //$userfile_type = $_FILES['userfile']['type'];
		    
		    $tmpfile = $mainLib->moveFile( $userfile, $userfile_name, $userfile_size );
		    if ($error->Got(READONLY))  {
		        return;
		    }
		    
		    $mainLib->form2($tmpfile);
		    
		}
		
		if ( $go == 2 ) {
		    
		    $mainLib->set_short_file_name($short_file_name);
		    
		    $tmpfile = $mainLib->getTempFile();
		    if ($error->Got(READONLY))  {
		        return;
		    }
		    
		}
		
		$mainLib->import_file( $sqlo, $tmpfile, $go, $infolevel );
		if ($error->Got(READONLY))  {
		    return;
		}
		
		if (  $go == 2 ) {
		    
		    echo "<br><br>";
		    
		    cMsgbox::showBox("ok", "Done.");
		    
		    unlink ($tmpfile);
		}
		
		
	}
}
