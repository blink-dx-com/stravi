<?php 
require_once 'o.ABSTRACT_SUBST.bom.inc';
require_once ("f.assocUpdate.inc");
require_once '../pionir/subs/o.ABSTRACT_SUBST_bom_form.inc';

/**
 * edit BOM of ABSTRACT_SUBST
 * @author Steffen
 * @param array $bomcompo
 *   POS => array('sua'=>, 'q'=>)
 */
class o_ABSTRACT_SUBST_bom_XPL extends gPlugin {
	
	
	function register() {
		$tablename	= 'ABSTRACT_SUBST';
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
	 * COMPO_SUA_ID, POS, QUANTITY
	 * @param unknown $sqlo
	 * @param unknown $bomcompo
	 */
	function check($sqlo, $bomcompo) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $this_sua_id = $this->objid;
	    $err_stack=array();
	   
	    $key_copy = array_keys($bomcompo);
	    
	    // array [SUA_ID] = cnt
	    $sua_cache=array();
	    foreach($key_copy as $pos) {
	        
	        $loop_sua = $bomcompo[$pos]['sua'];
	        if ($loop_sua) {
	            
	            if (!array_key_exists($loop_sua, $sua_cache) ) {
	               $sua_cache[$loop_sua]=0;
	            }
	            $sua_cache[$loop_sua] = $sua_cache[$loop_sua] + 1;
	            
	        }
	    }
	    
	    // analyse sua_cache
	    if ( max($sua_cache) > 1 ) {
	        $bad_keys = array_keys($sua_cache, max($sua_cache) );
	        $one_key= current($bad_keys);
	        $err_stack[$pos] = 'At least one component (ID:'.$one_key.') exists two times in the BOM! This is not allowed.';
	    }

        
	    foreach( $bomcompo as $pos=> $row) {
	        if ($row['sua'] and $row['q']<=0) {
	            $err_stack[$pos]='Quantity missing.';
	            continue;
	        }
	           
	        if ($row['sua'] ) {
	            if (!is_numeric($row['q'])) {
	                $err_stack[$pos]='Quantity must be a number.';
	                continue;
	            }
	        }
	        
	        foreach($key_copy as $pos) {
	            $loop_sua = $bomcompo[$pos]['sua'];
	            if ($loop_sua==$this_sua_id) {
	                $err_stack[$pos]='a component can not be the mother SUA in the same time!';
	                break;
	            }
	        }
	    }
	    
	    if (sizeof($err_stack)) {
	        $error->set( $FUNCNAME, 1, 'Errors:'.print_r($err_stack,1) );
	        return;
	    }
	    
	}
	
	/**
	 * COMPO_SUA_ID, POS, QUANTITY
	 * @param unknown $sqlo
	 * @param unknown $bomcompo
	 */
	function update_bom($sqlo, $bomcompo) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    $assoctable='SUA_BOM';
	    
	    
	    $assoclib = new  fAssocUpdate();
	    $assoclib->setObj( $sqlo, $assoctable, $this->objid );
	    
	    $sua_bom_lib = new oABSTRACT_SUBST_bom($this->objid);
	    $old_sua_compos  = $sua_bom_lib->get_compos_by_pos($sqlo);
	    
	    foreach( $bomcompo as $pos=> $row) {
	        $idarr=array('POS'=>$pos);
	        $argu=array();
	        $argu['COMPO_SUA_ID']=$row['sua'];
	        $argu['QUANTITY']    =$row['q'];

	        if ( is_array($old_sua_compos[$pos]) ) {
	            // update
	            if (!$row['sua']) {
	                $assoclib->delOneRow($sqlo, $idarr);
	            } else {
	               $assoclib->update( $sqlo, $argu, $idarr);
	            }
	        } else {
	            if ($row['sua']) {
    	            $argu['POS']    =$pos;
    	            $assoclib->insert( $sqlo, $argu);
	            }
	        }
	        
	        if ($error->Got(READONLY))  {
	            $error->set( $FUNCNAME, 1, 'Error on POS:'.$pos );
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
		
		$sqlo = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		$this->check($sqlo, $bomcompo);
	
		if ($error->Got(READONLY))  {
		    $error->set( $FUNCNAME, 1, 'Errors on data check.');
		    $error->printAll();
		    
		    $this->_html->line_out('');
		    
		    $form_sua_lib = new oABSTRACT_SUBST_bom_form($this->objid);
		    $form_sua_lib->form1($sqlo, $bomcompo, 1);
		    
		    return;
		}
		
		$this->update_bom($sqlo, $bomcompo);
		
		if ($error->Got(READONLY))  {
		    return;
		}
		
		echo 'ok. BOM updated.<br>';
		
		$this->_html->pageForward('edit.tmpl.php?t=ABSTRACT_SUBST&id='.$this->objid, 'forward ...');
		
	}
	
}
