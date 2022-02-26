<?php
/**
 * o.CONCRETE_PROTO > MOD data by formula
 *
 * @package o.CONCRETE_PROTO.formula_p.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
   @param $id : id of protostep_cmp
    @param string $backurl [OPTIONAL]
      
   @param array $back [OPTIONAL]
      t: table name
      id: ID 
 *
 */
require_once 'o.PROTO.steps.inc';
require_once 'o.ABSTRACT_PROTO.stepx.inc';
require_once 'o.CONCRETE_PROTO.formula.inc';
require_once ('f.msgboxes.inc'); 


class o_CONCRETE_PROTO_formula_p_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'CONCRETE_PROTO';
		$title		= 'Modify data by Formula';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		
		$this->infoarr2 = array( 
		    'accCheck' => array('tab'=>array('read', 'write'), 'obj'=>array('read', 'write') )
				);
		
	}

	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$this->objid  = $_REQUEST['id'];
		
		$sqlo   = &$this->_sqlo;
		
		$this->prc_formula_lib     = new oCONCRETE_PROTO_formula_mod($sqlo, $this->objid );
		if ($error->Got(READONLY))  {
		    return;
		}
		
		// get old data
		$data_in=array();
		$data_in_raw = gProtoOrg::get_all_data($sqlo, $this->objid, array('QUANTITY'));
		foreach($data_in_raw as $step_nr=>$row) {
		    $data_in[$step_nr] = $row['QUANTITY'];
		}
		
		
		if ( $_SESSION["userGlob"]["g.debugLevel"]>1) {
		    
		    $pra_id  = glob_elementDataGet( $sqlo, 'CONCRETE_PROTO', 'CONCRETE_PROTO_ID', $this->objid, 'ABSTRACT_PROTO_ID');
		    $pra_step_lib = new oABSTRACT_PROTO_stepx( $pra_id );
		    $pra_steps = $pra_step_lib->getStepArray_i2($sqlo, array('NAME', 'QUANTITY'));
		    
		    require_once ("visufuncs.inc");
		    $tabobj = new visufuncs();
		    $headOpt = array( "title" => "Old Data" );
		    $headx  = array ("Step", "Name", "Old-Val");
		    $tabobj->table_head($headx,   $headOpt);
		    foreach($pra_steps as $step_nr=>$row) {
		        $dataArr=array($step_nr, $row['NAME'], $data_in[$step_nr]);
		        $tabobj->table_row ($dataArr);
		    }
		    $tabobj->table_close();
		    
		}
		
		
		$this->prc_formula_lib->calculate_all($data_in);
		
		if ( $_SESSION["userGlob"]["g.debugLevel"]>1) {
		    
		    $data_out = $this->prc_formula_lib->get_data_out();
		    
		    $pra_id  = glob_elementDataGet( $sqlo, 'CONCRETE_PROTO', 'CONCRETE_PROTO_ID', $this->objid, 'ABSTRACT_PROTO_ID');
		    $pra_step_lib = new oABSTRACT_PROTO_stepx( $pra_id );
		    $pra_steps = $pra_step_lib->getStepArray_i2($sqlo, array('NAME', 'QUANTITY'));
		    
		    $tabobj = new visufuncs();
		    $headOpt = array( "title" => "New Data" );
		    $headx  = array ("Step", "Name", "Old-Val", "New-Val");
		    $tabobj->table_head($headx,   $headOpt);
		    foreach($pra_steps as $step_nr=>$row) {
		        $dataArr=array($step_nr, $row['NAME'], $data_in[$step_nr], $data_out[$step_nr]);
		        $tabobj->table_row ($dataArr);
		    }
		    $tabobj->table_close();
		    
		}

		$this->prc_formula_lib->save($sqlo);
		if ($error->Got(READONLY))  {
			return;
		}
		
		cMsgbox::showBox("ok", "Computed data by formula.");  //  "ok", "error", "warning", ...

		
		//$back=$_REQUEST['back'];  if ($back['t'] and $back['id']) {
		$backurl=urldecode($_REQUEST['backurl']); 
		if ($backurl!=NULL) {  
		    require_once ( "javascript.inc" );
		    js__location_replace($backurl, 'Forward to object', NULL, 0);
		   
		}
		
	}
	
}