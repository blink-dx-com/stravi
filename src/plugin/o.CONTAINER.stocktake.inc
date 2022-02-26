<?php
require_once '_subs/o.CONTAINER.stocktake_exp.inc';
require_once '_subs/o.CONTAINER.stocktake_imp.inc';
require_once ("f.upload.inc");

/**
 * stocktaking support
 * @package o.CONTAINER.stocktake.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   int $id (container)
 * @param  string $action
 *   'export'
 *   'import'
 */
class o_CONTAINER_stocktake_XPL extends gPlugin {
	
	
	function register() {
		$tablename	= 'CONTAINER';
		$title		= 'inventory stocktaking support';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		$this->infoarr['help_url'] = 'o.CONTAINER.stocktaking.html';
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('write'), 'obj'=>array('read', 'write') ),
			);
		
		if ($_REQUEST['action'] =='import') {
		    // secure against a role right ...
		    $this->infoarr2['role_right_need'] = 'o.CONTAINER.stocktake';
		}
	}
	
	function form1() {
	    
	    $initarr   = array();
	    $initarr["title"]       = "Wähle Import-Inventur-File"; // title of form
	    $initarr["submittitle"] = "Weiter"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    $initarr["ENCTYPE"] = "multipart/form-data";
	    
	    $hiddenarr = NULL; 
	    $hiddenarr["id"]     = $this->objid; 
	    $hiddenarr["action"]     = 'import'; 
	    
	    // init form, uses class formc
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
	    
	    $fieldx = array ( // form-field definition
	        "title"   => "Import file",
	        "name"  => "userfile",
	        "object"=> "file",
	        "namex" => TRUE,
	        "notes" => "Import file"
	    );
	    $formobj->fieldOut( $fieldx ); // output the form-field
	    
	    $formobj->close( TRUE ); // close form, sshow submit-button
	    
	    $this->help_import();
	    
	    echo "<br>";
	}
	
	function form2() {
	    
	    $initarr   = array();
	    $initarr["title"]       = "Import vorbereiten"; // title of form
	    $initarr["submittitle"] = "Update!"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    
	    $hiddenarr = NULL; 
	    $hiddenarr["id"]     = $this->objid; 
	    $hiddenarr["action"]     = 'import'; 

	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 1);

	    $formobj->close( TRUE ); // close form, sshow submit-button
	    
	    echo "<br>";
	}
	
	function help_import() {
	    
	    echo "<br><br>\n";
	    $this->_html->messageBox_open('Short Help', '', 'HELP');
	    ?>
	    <ul>
	    <li>Following columns are required: <b>MAC-ID</b>,	<b>Quantity</b></li>
	    
	    </ul>
	    <?php
	    $this->_html->messageBox_close();
	}
	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		$cont_id   = $_REQUEST['id'];
		$go   = $_REQUEST['go'];
		$action = $_REQUEST['action'];
		$this->objid=$cont_id;
		
		if ($action=='export') {
		    
		    $this->_html->line_out('Exportiere eine Inventur-Liste.');
		    $this->_html->line_out('');
		    
		    $exp_lib = new o_CONTAINER_stocktake_exp($cont_id, $this->_mod);
		    $exp_lib->export($sqlo, $sqlo2);
		}
		
		$myname='inventory.xlsx';
		
		if ($action=='import') {
		    
		    $uploadLib = new uploadC();
		    
		    if (!$go) {
		        $this->form1();
		        return;
		    }
		    
		    $tmpfile='';
		    
		    if ($go==1) {
		        
    		    $userfile_size = $_FILES['userfile']['size'];
    		    $userfile      = $_FILES['userfile']['tmp_name'];
    		    $userfile_name = $_FILES['userfile']['name'];
    		   
    		    
    		    $tmpfile = $uploadLib->mvUpload2Tmp( $this->_mod, $myname, $userfile, $userfile_name, $userfile_size );
    		    if ($error->Got(READONLY))  {
    		        return;
    		    }
    		    
		    
		        $this->form2();
		    }
		    
		    if ($go==2) {
		        $tmpfile = $uploadLib->getFileFromTmp( $this->_mod, $myname);
		        if ($error->Got(READONLY))  {
		            return;
		        }
		        
		        $this->_html->line_out('<b>Do import now!</b>');
		        $this->_html->line_out('');
		    }
		    
		    $exp_lib = new o_CONTAINER_stocktake_imp($cont_id, $tmpfile);
		    $exp_lib->import($sqlo, $sqlo2, $go);
		    
		    if ( !$error->Got(READONLY) )  {
		        echo "<br>\n";
		        $this->_html->messageBox2('ok', 'O.K.');
		    }
		    
		}
	}
}
