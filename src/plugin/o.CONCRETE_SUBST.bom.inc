<?php 
require_once 'o.ABSTRACT_SUBST.bom.inc';
require_once 'o.CONCRETE_SUBST.bom.inc';
require_once 'o.CONCRETE_SUBST.bom_mod.inc';
require_once ("f.assocUpdate.inc");


/**
 * edit BOM of ABSTRACT_SUBST
 * @author Steffen
 * @param array $bomcompo
 *   array('suc'=>, 'q'=>, 'pos'=>, 'apos'=>)
 */
class o_CONCRETE_SUBST_bom_XPL extends gPlugin {
	
	
	function register() {
		$tablename	= 'CONCRETE_SUBST';
		$title		= 'show/edit BOM';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read','write'), 'obj'=>array('insert', 'write') )
				);

		
	}
	
	/**
	 * input:
	 * - $this->sua_compos
	 * @param object $sqlo
	 * @param array $bomcompo of POS => array()
	 *   'suc' : 
	 */
	function check($sqlo, $bomcompo) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    
	    
	}
	
	/**
	 * COMPO_SUA_ID, POS, QUANTITY
	 * @param object $sqlo
	 * @param array $bomcompo
	 */
	function update_bom($sqlo, $bomcompo_in) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;

	    $mod_lib = new oCONCRETE_SUBST_bom_mod();
	    $mod_lib->set_mother($sqlo, $this->objid );
	    
	    debugOut('bomcompo_in: '.print_r($bomcompo_in,1), $FUNCNAME, 2);

	    // $old_suc_compos = $this->suc_compos_old;
	    //$sua_compos = &$this->sua_compos;
	    
	    foreach( $bomcompo_in as $i=>$in_row) {
	        
	        
	        $suc_id = $in_row['suc'];
	        $args = array();
	        $args['COMPO_SUC_ID']=$suc_id;
	        $args['APOS']=$in_row['apos'];
	        $args['POS'] =$in_row['pos'];
	        $args['QUANTITY'] =$in_row['q'];
	        
	        if (!$args['APOS']) {
	            $error->set( $FUNCNAME, 1, 'APOS missing on row '.($i+1) );
	            return;
	        }
	       
	        if ($in_row['pos']<=0) {
	            
	            if (!$args['COMPO_SUC_ID']) continue;
	            
	            // get POS
	            $upos = $mod_lib->get_last_pos($sqlo) + 1;
	        } else {
	            $upos = $in_row['pos'];
	        }
	        
	        debugOut('UPDATE: UPOS: '.$upos.' args: '.print_r($args,1), $FUNCNAME, 2);
	        
	        $mod_lib->update_pos($sqlo, $upos, $args);
	        
	        if ($error->Got(READONLY))  {
	            $error->set( $FUNCNAME, 1, 'Error on POS:'.$upos );
	            return;
	        }
	    }
	    
	}
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$go  = $_REQUEST['go'];
		$act = $_REQUEST['act'];
		
		$this->objid = $_REQUEST['id'];
		$bomcompo = $_REQUEST['bomcompo'];
		
		$sqlo  = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		    
		$suc_features = glob_elemDataGet3( $sqlo, 'CONCRETE_SUBST', array('CONCRETE_SUBST_ID'=>$this->objid), array('ABSTRACT_SUBST_ID') );
	    $sua_id       = $suc_features['ABSTRACT_SUBST_ID'];
	    $sua_bom_lib = new oABSTRACT_SUBST_bom($sua_id);
	    $this->sua_compos  = $sua_bom_lib->get_compos_by_pos($sqlo);
	    
	    $suc_bom_lib = new oCONCRETE_SUBST_bom($this->objid );
	    $this->suc_compos_old = $suc_bom_lib->get_compos_by_pos($sqlo);
		
		
		$this->check($sqlo, $bomcompo);
	
		if ($error->Got(READONLY))  {
		    $error->set( $FUNCNAME, 1, 'Errors on data check.');
		    $error->printAll();
		    
		    $this->_html->line_out('');

		    return;
		}
		
		$this->update_bom($sqlo, $bomcompo);
		
		if ($error->Got(READONLY))  {
		    return;
		}
		
		echo 'ok. BOM updated.<br>';
		
		$this->_html->pageForward('edit.tmpl.php?t=CONCRETE_SUBST&id='.$this->objid, 'forward ...');
		
	}
	
}
