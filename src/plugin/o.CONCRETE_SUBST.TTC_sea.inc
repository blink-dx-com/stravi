<?php
require_once ("f.objview.inc");
require_once ("visufuncs.inc");
require_once dirname(__FILE__).'/../www/pionir/impexp/suc_ref/o.CONCRETE_SUBST.TTC.inc';
require_once dirname(__FILE__).'/../www/pionir/impexp/suc_ref/o.CONCRETE_SUBST.TTC_gui.inc';

/**
 * SEARCH/ Autodetect Ref_beads for selected list
 * @package o.CONCRETE_SUBST.TTC_sea.inc
 * @swreq REQ:41 BEADS > REF-Beads > Target-Type CHECKER
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @created 2021-06-14
 * @param int $go
 *
 */
class o_CONCRETE_SUBST_TTC_sea_XPL extends gPlugin {
	
	function register() {
		$tablename	= 'CONCRETE_SUBST';
		$title		= 'AUTODETECT '.o_LINK_TTC::CONFIG_NAME_USER.'s';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title_sh']    = $title;
		$this->infoarr['title']    = o_LINK_TTC::TOOL_NAME.' > '.$title;
		$this->infoarr['form_type']= 'list';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_cnt']  = 1;
		$this->infoarr['locrow']   = array(  // defines the tool location-path
		    array('p.php?mod=DEF/o.CONCRETE_SUBST.TTC_home', o_LINK_TTC::TOOL_NAME.' - home'),
		); 
		
		$this->infoarr2 = array(
		    'accCheck' => array('tab'=>array('read', 'write') ),
		    'listChkSelect' =>1
		);

	}
	
	function form1() {  
	    $initarr   = array();
	    $initarr["title"]       = "Search";
	    $initarr["submittitle"] = "Search"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    $hiddenarr = NULL; // hidden form variables
	    //$hiddenarr['id'] = $this->objid;
	    $formobj = $this->_html->formInit($initarr, $hiddenarr);
	    
	    $fieldx = array ( // form-field definition
	        "title"   => "Details",
	        "name"  => "details",
	        "object"=> "checkbox",
	        "val"    => 0,
	        "notes" => "show details?"
	    );
	    $formobj->fieldOut( $fieldx ); // output the form-field
  
	    $formobj->close( TRUE );       
	}
	
	function form2() {
	    $initarr   = array();
	    $initarr["title"]       = "Update detected ".o_LINK_TTC::CONFIG_NAME_USER. 'on MACs';
	    $initarr["submittitle"] = "Update"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    $hiddenarr = NULL; // hidden form variables
	    //$hiddenarr['id'] = $this->objid;
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 1);
	    
	  
	    
	    $formobj->close( TRUE );
	}
	
	/**
	 * 
	 * @param object $sqlo
	 * @param int $suc_id
	 * @return array
	 *   'status' 
	 *   'text'
	 */
	function _sea_target_type($sqlo, $suc_id) {
	    global $error;

	    
	    $doc_arr = $this->bead_gui_lib->check0_get_configs($sqlo);
	    if (empty($doc_arr)) {
	        echo 'No Config found for this MAA.';
	        return;
	    }
	    foreach($doc_arr as $doc_id_loop) {
	        $error->reset(); // reset error for new run ...
	        $answer = $this->bead_gui_lib->check0_this_doc($sqlo, $doc_id_loop);
	        if ($answer==1 and !$error->Got(READONLY)) {
                if ($answer==1) {
                    return array('status'=> 1);
                }
	        }
	    }

	    $this->bead_gui_lib->unset_ref_bead();
	    $errLast   = $error->getLast();
	    $error_txt = $errLast->text;
	    $error->reset();
	    return array('status'=> 0, 'text'=>$error_txt);

	    
	}
	
	/**
	 * 
	 * @param object $sqlo
	 * @param int $suc_id
	 * @return array 
	 *   'text'
	 *   'is_std'
	 *   'doc_id'
	 *   'updated'
	 *   'doc_conn'
	 */
	function _one_SUC($sqlo, $suc_id) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $out=array();
	    $out['text']='';

	    
	    $this->suc_did_update=0;
	    $bead_gui_lib = &$this->bead_gui_lib;
	    // $tablename	= 'CONCRETE_SUBST';
	   
	    
	    $bead_gui_lib->set_suc($sqlo, $suc_id);
	    $ref_bead = $bead_gui_lib->get_ref_doc();

	    
	    if ($ref_bead) {
	        
	        $out['text'] .= 'Has REF-DOC: ';
	        $out['doc_conn'] = 'yes';
	        //$obj_link = fObjViewC::	bo_display( $sqlo, 'LINK', $ref_bead);
	        //echo $obj_link."; ";
	        
	        $answer = $bead_gui_lib->check1_suc($sqlo);
	        $out['is_std'] = $answer;
	        
	        
	    } else {
	        $answer = $this->_sea_target_type($sqlo, $suc_id);
	       
	        
	        $answer_tmp='';
	        if (empty($answer)) {
	            $out['is_std'] = 0;
	        }
	        else {
	            if ($answer['status']==1) {
	               $out['is_std'] = 1;
	            }
	            //$answer_tmp='found: '.$answer[1];
	        }
	        $out['text'] .= $answer_tmp;
	    }
	    
	   
	    
	    if ($error->Got(READONLY))  {
	        return $out;
	    }
	    
	    $ref_bead = $bead_gui_lib->get_ref_doc();
	    
	    
	    
	    if (!$ref_bead) {
	        return $out;
	    }
	    
	    $out['doc_id'] = $ref_bead;

	    $diff     = $bead_gui_lib->get_diff_arr();
	    if (!empty($diff)) {
	        if ($this->parx['details']) {
	            $out['text']  .=  "; DIFF: ".print_r($diff,1);
	        }   
	    }
	    
	    if ($this->do_update) {
	        if ($out['is_std']==1) {
	            $bead_gui_lib->update_suc($sqlo);
    		    $out['updated']=1;
    		}
	    }

	    return $out;
	}
	
	

	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$tablename	= 'CONCRETE_SUBST';
		
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		$go = $_REQUEST['go']; 
		$this->parx = $_REQUEST['parx']; 
		
		
		if (!$go) {
		    $this->form1();
		    return;
		}
		if ($go==1) {
		    $this->form2();
		    echo "<br>";
		}
		
		$this->do_update=0;
		if ($go==2) {
		    $this->do_update=1;
		}
		
		
		$this->bead_gui_lib = new o_CONCRETE_SUBST_TTC($sqlo);
		if ($error->Got(READONLY))  {
		    $error->set( $FUNCNAME, 1, 'init error.' );
		    return;
		}
	
		
		$sqlAfter = $this->listLib->getSqlAfter();
		$sqlsel = "x.CONCRETE_SUBST_ID from ".$sqlAfter;
		$sqlo2->Quesel($sqlsel);
		
		$tabobj = new visufuncs();
		$headOpt = array( "title" => "Summary" );
		$headx  = array ("#", "MAC", 'Config', 'Already connected?', 'Info', 'is standard?', 'Updated?');
		$tabobj->table_head($headx,   $headOpt);

		
		$cnt=0;
		while ( $sqlo2->ReadRow() ) {
		    
		    $suc_id = $sqlo2->RowData[0];
		    $one_suc_out = $this->_one_SUC($sqlo, $suc_id);
		    $text_tmp = $one_suc_out['text'];
		    
		    if ($error->Got(READONLY))  {
		        $errLast   = $error->getLast();
		        $error_txt = $errLast->text;
		        $error->reset();
		        $text_tmp  = "<b>ERROR:</b> ".$error_txt.'; '.$text_tmp;
		    }  
		    
		    $tmp_doc='';
		    if ($one_suc_out['doc_id']) {
		      $tmp_doc = fObjViewC::bo_display( $sqlo, 'LINK', $one_suc_out['doc_id']);
		    }
		    $is_std_text='';
		    if ($one_suc_out['is_std']==1) $is_std_text='ok';
		    
		    $outarr=array(
		        $cnt+1,
		        fObjViewC::bo_display( $sqlo, $tablename, $suc_id),
		        $tmp_doc,
		        $one_suc_out['doc_conn'],
		        $text_tmp,
		        $is_std_text,
		        $one_suc_out['updated'],
		        
		    );
		    
		    $tabobj->table_row ($outarr);
		    
		    $cnt++;
		}
		$tabobj->table_close();

	}
}
