<?php
/**
 * - plugin example for a list of experiments
 *    
 * @package o.EXP.qcFlag.inc
 * @author  Steffen Kube (steffen@clondiag.com)
 * @param $_REQUEST['go'] progress of script: 0: show form, 1: update experiments
 * @param $_REQUEST['parx']['key'] classification key
 *   
 */


class o_EXP_qcFlag_XPL extends gPlugin {

	
	function register() {
		$tablename	= 'EXP';
		$title		= 'markiere als QC-Experimente';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = 'als QC';
		$this->infoarr['form_type']= 'list'; 
        #$this->infoarr['help_url'] = '';
		
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_cnt']  = 1;

		$this->infoarr2 = array( 
			'accCheck' => array('tab'=>array('read', 'write'), 'obj'=>array() ),
			'listChkSelect' =>1
			);
				
		
	}
	
	function form1() {
		
		
		$initarr   = NULL;
		$initarr["title"]       = "Auswahl QC-Test-Ziel";
		$initarr["submittitle"] = "Update";
		$initarr["tabwidth"]    = "AUTO";
	
		$hiddenarr = NULL;
	
		$formobj = $this->_html->formInit($initarr, $hiddenarr);
		
		$fieldx = array ( 
			"title" => "Classification", 
			"name"  => "key",
			"object"=> "select",
			"val"   => $parx["key"], 
			"inits" => $selinit,
			"notes" => ""
			 );
		$formobj->fieldOut( $fieldx );
		
		$formobj->close( TRUE );
	}
	
	
	

	function startMain() {
		global $error;
		global $s_formState;
		$FUNCNAME='startMain';
		
		
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
		$key  = $parx['key'];
		
		$sqlAfter = $this->listLib->getSqlAfter();
		
		if (!$go) {
			$this->form1();
			return;
		}

        $sqlsel = "x.EXP_ID from ".$sqlAfter;
		$sqlo2->Quesel($sqlsel);
		while ( $sqlo2->ReadRow() ) {
		    $expid = $sqlo2->RowData[0];
			$cnt++;
		}
		
		
		
		echo $cnt.' experiments updated'.
		$url='view.tmpl.php?t=EXP';
		js__location_replace($url, "experiment list" ); 
		
	}
	
}
