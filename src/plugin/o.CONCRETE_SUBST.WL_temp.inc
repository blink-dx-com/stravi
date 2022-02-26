<?php
require_once ("visufuncs.inc");

/**
 * edit temporary worklist
 * @package o.CONCRETE_SUBST.WL_temp.inc
 * @author Steffen
 * @param array $sucs
 *   array[SUC_ID] => 1
 * @param $action
 *   'edit'
 *   'show'
 */
class o_CONCRETE_SUBST_WL_temp_XPL extends gPlugin {
	
	
	function register() {
		$tablename	= 'CONCRETE_SUBST';
		$title		= 'temporary worklist';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title_sh'] = $title;
		$this->infoarr['title']    = 'Create a temporary worklist with selected MACs; show a Barcode-List';
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		
		$this->infoarr2 = array( 
			'accCheck' => array('tab'=>array('read') )
		);

		
	}
	
	function form_clear() {
	    
	    $initarr   = array();
	    $initarr["title"]       = "Do you want to clear the list?"; // title of form
	    $initarr["submittitle"] = "CLEAR"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO

	    
	    $hiddenarr = NULL;
	    $hiddenarr["action"]     = 'clear';
	    
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);

	    $formobj->close( TRUE );
	}
	
	function form1($sqlo, $wl_struct) {
	    
	    $initarr   = array();
	    $initarr["title"]       = "Select MACs"; // title of form
	    $initarr["submittitle"] = "Select"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    $initarr["dblink"] = 1;
	    
	    $hiddenarr = NULL;
	    $hiddenarr["action"]     = 'edit'; 
	    
	    $sucs = $wl_struct['sucs'];
	    
	    // init form, uses class formc
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
	    
	    $tablename='CONCRETE_SUBST';
	    
	    $fieldx = array ( // form-field definition
	        'namex' => TRUE    ,
	        "title" => "Worklist Notes",
	        "name"  => "notes",
	        "object"=> "text",
	        "fsize" => 100,
	        "val"   => $wl_struct['notes'],
	        "notes" => ""
	    );
	    $formobj->fieldOut( $fieldx );
	    
	    $MAX_SUCS = 10;
	    for ($i=0;$i<$MAX_SUCS; $i++) {
	    
    	    $fieldx = array ( // form-field definition
    	        'namex' => TRUE    ,
    	        "title" => "MAC ".($i+1),
    	        "name"  => "suc[".($i)."]",
    	        "object"=> "dblink",
    	        "val"   => $sucs[$i],
    	        "inits" => array( 'table'=>$tablename, 'getObjName'=>1, 'sqlo'=>&$sqlo, 'pos' =>$i, 'projlink'=> 1),
    	        "notes" => ""
    	    );
    	    $formobj->fieldOut( $fieldx );
	    }
	    $formobj->close( TRUE );
	}
	
	function printx($sqlo, $wl_struct) {
	    
	    $tablename	= 'CONCRETE_SUBST';
	    
	    echo '<span style="color:gray;">Worklist Notes: </span><b>'.$wl_struct['notes']."</b><br><br>";
	    
	    $tabobj = new visufuncs();
	    $headOpt = array(  );
	    $headx  = array ("#", "Name", "MAC-ID", "Barcode");
	    $tabobj->table_head($headx,   $headOpt);
	    
	    $sucs = $wl_struct['sucs'];
	    
	    $i=0;
	    foreach($sucs as $suc_id) {
	        
	        if ($suc_id) {
    	        $suc_feats = glob_elemDataGet3( $sqlo, $tablename, array('CONCRETE_SUBST_ID'=>$suc_id), array('NAME', 'ABSTRACT_SUBST_ID') );
    	        $sua_feats = glob_elemDataGet3( $sqlo, 'ABSTRACT_SUBST', array('ABSTRACT_SUBST_ID'=>$suc_feats['ABSTRACT_SUBST_ID']), array('NAME') );
    	        
    	        $barcode = '<img src="p.php?mod=DEF/f.barcode&c='.urlencode($suc_id).'&sub=B&y=50"';
    	        
    	        $dataArr = array( ($i+1), $suc_feats['NAME'] . '('.$sua_feats['NAME'].')', $suc_id, $barcode );
	        } else {
	            $dataArr = array( ($i+1),'', '', '' );
	        }
	        $tabobj->table_row ($dataArr);
	        $i++;
	    }
	    $tabobj->table_close();

	}
	

	function startMain() {
		global $error;
		// $FUNCNAME='startMain';
		
		$go     = $_REQUEST['go'];
		$action = $_REQUEST['action'];
		if ($action=='') $action='edit';
		
		$sqlo  = &$this->_sqlo;
		
		
		$base_url = $this->_plug_get_URL() . '&action=';
		echo '[<a href="'.$base_url.'edit">Edit</a>] ';
		echo '[<a href="'.$base_url.'show">Show</a>] &nbsp;';
		echo '[<a href="'.$base_url.'clear">Clear list</a>] ';
		echo '';
		echo "<br><br>\n";
		
		if ($action=='clear') {
		    
		    if (!$go) {
		        $this->form_clear();
		        return;
		    }
		    
		    echo "... clear the list ...<br><br>";
		    
		    $_SESSION['s_formState']['o_CONCRETE_SUBST_WL_temp'] = NULL;
		    $action = 'edit';
		    $go = 0;
		}
		
		if ($action=='edit' and $go) {
		    
		    $sucs = $_REQUEST['suc'];
		    if (empty($sucs)) {
		       echo 'Bizarre. Input is empty.<br>'; 
		    } else {
		        $wl_struct = array('sucs'=>$sucs, 'notes'=>$_REQUEST['notes']);
		        $_SESSION['s_formState']['o_CONCRETE_SUBST_WL_temp'] = $wl_struct;
		        $action='show';
		    }
		    
		}
		
		$wl_struct = $_SESSION['s_formState']['o_CONCRETE_SUBST_WL_temp'];

	    if ($action=='edit') {   
	        
	        $this->form1($sqlo, $wl_struct);
	    }
	    
	    if ($action=='show') {
	       
	        $this->printx($sqlo, $wl_struct);
	    }
	    
	    
		
	}
	
}
