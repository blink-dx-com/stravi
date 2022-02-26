
<?php
require_once 'o.PROJ.subs.inc';
require_once 'o.PROJ.addelems.inc';
require_once ('f.msgboxes.inc');

/**
 * example of test module
 * @package o.PROJ.elem_order_ch.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $id
 * @param   $dtab string destination table
 * @param   ord[id]=order 
 */
class o_PROJ_elem_order_ch_XPL extends gPlugin {
	
	
	function register() {
		$tablename	= 'PROJ';
		$title		= 'Change order of objects in folder';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('write'), 'obj'=>array('read','insert') )
			);
	}
	
	function form0( $sqlo ) {
	    
	    
	    $initarr   = array();
	    $initarr["title"]       = "Select table"; // title of form
	    $initarr["submittitle"] = "Submit"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    
	    $hiddenarr = NULL; // hidden form variables
	    $hiddenarr["id"]     = $this->objid; // is a required input parameter
	    
	    // init form, uses class formc
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
	    
	    $tables = cProjSubs::tablesInProject($sqlo, $this->objid );
	    $table_arr=array();
	    foreach( $tables as $tablex) {
	        $nice = tablename_nice2( $tablex );
	        $table_arr[$tablex] = $nice;
	    }
	    
	    $fieldx = array ( // form-field definition
	        "title"   => 'Table',
	        "name"    => "dtab",
	        "object"  => "select",
	        "val"     => '',
	        'inits'   => $table_arr,
	        "namex"   => TRUE
	        
	    );
	    
	    $formobj->fieldOut( $fieldx );
	    
	    $formobj->close( TRUE );
	}
	
	function form1( $sqlo, $objects ) {
	    
	    $d_tablename = $this->dtab;
	    
	    $initarr   = array();
	    $initarr["title"]       = "Define order of objects in folder"; // title of form
	    $initarr["submittitle"] = "Submit"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    
	    $hiddenarr = NULL; // hidden form variables
	    $hiddenarr["id"]     = $this->objid; 
	    $hiddenarr["dtab"]   = $d_tablename; 
	    
	    // init form, uses class formc
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 1);
	    
	    foreach($objects as $objid => $order) {
	        
	        $name = obj_nice_name ( $sqlo, $d_tablename, $objid );
	    
    	    $fieldx = array ( // form-field definition
    	        "title"   => $name,
    	        "name"    => "ord[".$objid."]",
    	        "object"=> "text",
    	        "val"   => $order,
    	        "namex" => TRUE      
    	      
    	    );
    	    
    	    $formobj->fieldOut( $fieldx ); 
	    }
	    
	    $formobj->close( TRUE );
	}
	
	
	
	function init_objects($sqlo) {
	    
	    $objects=array();
	    $d_tablename = $this->dtab;
	    
	    $objects=array();
	    
	    $sqlcmd = 'PRIM_KEY, ELEM_ORDER from '. cProjSubs::getTable_ELEMORD_SQL($this->objid, $d_tablename);
	    $sqlo->Quesel($sqlcmd);
	    while ( $sqlo->ReadRow() ) {
	        $tmpid    = $sqlo->RowData[0];
	        $ordernum = $sqlo->RowData[1];
	        $objects[$tmpid]=$ordernum;
	    }
	    
	    return $objects;
	}
	
	function update($sqlo, $objects) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $d_tablename = $this->dtab;
	    $proj_mod_lib = new oProjAddElem($sqlo, $this->objid);
	    
	    echo 'update '.sizeof($objects).' entries.<br>';
	    
	    foreach($objects as $objid => $order) {
	        
	        $order=trim($order);
	        if ($order==='') {
	            //ok
	        } else {
	            if (!is_numeric($order)) {
	                echo "- Error: ".$order." is not a number<br>";
	                continue;
	            }
	        }
	        
	        $argu = array('ELEM_ORDER'=>$order);
	        $proj_mod_lib->mod_entry($sqlo, $d_tablename, $objid, $argu);
	        if ($error->Got(READONLY))  {
	            $error->set( $FUNCNAME, 1, 'Error on setting order for object '. $d_tablename.':'. $objid);
	            return;
	        }
	    }
	}
	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		$go   = $_REQUEST['go'];
		$this->objid   = $_REQUEST['id'];
		$this->dtab    = $_REQUEST['dtab'];
		$ord           = $_REQUEST['ord'];
		
		if ($this->dtab==NULL) { 
		    $this->form0( $sqlo );
		    return;
		} 
		
		
		$objects = $this->init_objects($sqlo);
		$this->form1( $sqlo, $objects );
		
		
		if ($go==2) {
		    $objects = $ord;
		    
		    if (empty($objects)) {
		        $this->htmlFoot('ERROR', 'No data given.');
		    }
		    
		    $this->update($sqlo, $objects);
		    
		    if (!$error->Got(READONLY))  {
		        
		        cMsgbox::showBox("ok", "updated.");  //   "ok", "error", "warning", ...
		        
		        $this->_html->pageForward('edit.tmpl.php?t=PROJ&id='.$this->objid , 'go back');
		    }
		    
		}
		
		
	}
}
