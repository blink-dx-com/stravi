<?php
/**
 * o.ABSTRACT_PROTO > MOD/SHOW formula
 *
 * @package o.ABSTRACT_PROTO.formula_p.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
   @param $fl -- formula lines : array(line_id => array)
 *
 */
require_once 'o.ABSTRACT_PROTO.stepx.inc';
require_once 'o.ABSTRACT_PROTO.formula.inc';
require_once 'func_form.inc';

/**
 * formula GUI lines
 * @author skube
 *
 */
class o_PRA_formula_gli {
    
    private $_lineid=0;
    
    function __construct($objid, $pra_step_info, $edit=0) {
        $this->objid=$objid;
        $this->pra_step_info=$pra_step_info;
        $this->editmode=$edit;
    }
    
    /**
     * 
     * @param array $row
     *    'step'
     *    'ut_val'
     *    'errmsg'
     * @throws Exception
     */
    function oneline($row) {
        
        $step_nr = $row['step'];
        if ($step_nr) {
            $pra_step_info_row = $this->pra_step_info[$step_nr];
            if (!is_array($pra_step_info_row)) {
                throw new Exception('Line:'.($this->_lineid+1).' step_nr-nr:'.$step_nr.' not found in PRA.');
            }
        } else {
            $pra_step_info_row=array();
        }
        $calc_str=$row['calc'];
        
        echo ' <tr>'."\n";
        
        if ($this->editmode) {
            $step_tmp='<input type="text" name="fl['.$this->_lineid.'][step]" value="'.$step_nr.'" size=3>';
        } else {
            // echo '<input type="hidden" name="fl['.$this->_lineid.'][step]" value="'.$step_nr.'">';
            $step_tmp=$step_nr;
        }
        
        echo '  <td>'.($this->_lineid+1).'</td>'."\n";
        echo '  <td>'.$step_tmp.'</td>'."\n";
        echo '  <td>'.$pra_step_info_row['NAME'].'</td>'."\n";
        echo '  <td>'.$row['sua'].'</td>'."\n";
        
        if ($this->editmode) {
            $tmp='<input type="text" name="fl['.$this->_lineid.'][calc]" value="'.$calc_str.'">';
        } else {
            $tmp=$calc_str;
        }
        echo '  <td>'.$tmp.'</td>'."\n";
        
        $error_msg='';
        echo '  <td><a href="p.php?mod=DEF/o.ABSTRACT_PROTO.formula_p&id='.$this->objid.'&action=add&line_id='.$this->_lineid.'">Add</a></td>'."\n";
        echo '  <td>'.$row['ut_val'].'</td>'."\n";
        echo '  <td>'.$error_msg.'</td>'."\n";
        echo '</tr>'."\n";
        
        $this->_lineid = $this->_lineid + 1;
    }
}


class o_ABSTRACT_PROTO_formula_p_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'ABSTRACT_PROTO';
		$title		= 'Modify Formula';
		
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
	
	function _show_UT_data($ut_data) {
	    
	    echo "<table>";
	    $header=array('Step-Nr', 'Step-Name', 'Test-Quantity');
	    
	    echo "<table>\n";
	    echo '<tr>';
	    foreach($header as $cell) {
	        echo '<th>'.$cell.'</th>';
	    }
	    echo '</tr>';
	    foreach($ut_data as $step_nr=>$val) {
	        
	        if ($val===NULL or $val==='') continue;
	        
	        //$step_nr = $row['step'];
	        if ($step_nr) {
	            $pra_step_info_row = $this->pra_steps[$step_nr];
	        } else {
	            $pra_step_info_row=array('NAME'=>'?');
	        }
	        echo ' <tr>'."\n";
 
	        $step_tmp=$step_nr;

	        echo '  <td>'.$step_tmp.'</td>'."\n";
	        echo '  <td>'.$pra_step_info_row['NAME'].'</td>'."\n";
	        echo '  <td>'.$val.'</td>'."\n";
	        echo ' </tr>'."\n";
	    }
	    echo "</table>";
	}
	
	private function proto_steps_show($sqlo) {
	    
	    $pra_steps = &$this->pra_steps;
    	echo '<b>Proto-Steps</b><br>';
    	$header=array('Step-Nr', 'Step-Name', 'MAA', 'Quantity');
    	
    	echo "<table>\n";
    	echo '<tr>';
    	foreach($header as $cell) {
    	    echo '<th>'.$cell.'</th>';
    	}
    	echo '</tr>';
    	
    	foreach($pra_steps as $step_nr=>$row) {
    	    echo '<tr>';
    	    
    	    $sua_id = $row['ABSTRACT_SUBST_ID'];
    	    $sua_name='';
    	    if ($sua_id) {
    	        $sua_name=obj_nice_name ( $sqlo, 'ABSTRACT_SUBST', $sua_id );
    	    }
    	    $row_out=array($step_nr, $row['NAME'], $sua_name, $row['QUANTITY']);
    	    foreach($row_out as $cell) {
    	        echo '<td>'.$cell.'</td>';
    	    }
    	    echo '</tr>'."\n";
    	}
    	echo "</table>";
    }
	
	function form1($sqlo) {
	    
	    $editallow = 1;
	    
	    
	    echo '<table>'."\n";
	    echo '<tr valign=top><td><!-- MAIN_TABLE -->'."\n";
	    
	    $ut_data = $this->formula_lib->get_UT_data();
	    
	    if (empty($ut_data)) {
	        echo "WARN: no UnitTest-Data.<br>";
	    } else {
	        echo 'UnitTest-Data:  <a href="p.php?mod='.$this->_mod.'&id='.$this->objid.'&action=save_UT">Edit</a><br>';
	    
	        $this->_show_UT_data($ut_data);
	        echo "<br>";
	    }
		
		$initarr   = array();
		$initarr["title"]       = "Modify formula"; // title of form
		$initarr["action"]       = "p.php";
	
		$hiddenarr = NULL; 
		$hiddenarr["id"]     = $this->objid; 
		$hiddenarr["mod"]     = $this->_mod; 
		$hiddenarr["action"]     = 'save'; 

		$formobj = new formPure($initarr, $hiddenarr, 0);
		$formobj->start();
		
		$old_lines = $this->formula_lib->get_calc_lines(  );
		if ($old_lines==NULL) {
		    $old_lines=array();
		}
		
		if (sizeof($old_lines)) {
		    
		    try {
		      $ut_test_output = $this->formula_lib->calculate_all($ut_data);
		    } catch (Exception $e) {
		        $this->_html->messageBox('Formula-Calc-ERROR', '', 'ERROR', $e->getMessage() );
		        echo "<br>";
		        
		    }
		    
		} else {
		    foreach($this->pra_steps as $step_nr => $row) {
		        $old_lines[]=array('step'=>$step_nr, 'calc'=>'');
		    }
		}
		
	
		
		$for_lines_lib = new o_PRA_formula_gli($this->objid, $this->pra_steps, $editallow);
		
		$header=array('Line-ID', 'Step-Nr', 'Step-Name', 'MAA', 'Formula', 'Test data output');
		
		echo "<table>\n";
		echo '<tr>';
		foreach($header as $cell) {
		    echo '<th>'.$cell.'</th>';
		}
		echo '</tr>';
		
		
		$ind=0;
		foreach($old_lines as $row) {
		    
		    $step_nr=$row['step'];
		    if ($step_nr) {
		        $row['ut_val'] = $ut_test_output[$step_nr];
		        
		        $sua_id = $this->pra_steps[$step_nr]['ABSTRACT_SUBST_ID'];
		        $sua_name='';
		        if ($sua_id) {
		            $sua_name=obj_nice_name ( $sqlo, 'ABSTRACT_SUBST', $sua_id );
		            $row['sua']=$sua_name;
		        }
		        
		    }
		    $for_lines_lib->oneline($row);
		    $ind++;
		}
		echo "</table>";
		echo '<input class="yButton" type=submit value="Save">'."\n"; 

		$formobj->close( TRUE ); // close form, sshow submit-button
		
		echo '</td><td>'."\n";
		
		$this->proto_steps_show($sqlo);
		
		
		
		echo '</td></tr><!-- MAIN_TABLE -->'."\n";
		echo '</table>'."\n";
	}
	
	function form2_UT($sqlo) {
	    
	    $old_data = $this->formula_lib->get_UT_raw();
	    
	    $data_show=array();
	    $data_by_step=array();
	    
	    if (!empty($old_data)) {
	        $data_by_step=array();
	        foreach($old_data as $row) {
	            $data_by_step[$row['step']] = $row['val'];
	        }
	    }

	    foreach($this->pra_steps as $step_nr => $row) {
	        $data_show[]=array('step'=>$step_nr, 'val'=>$data_by_step[$step_nr]);
	    }
	    
	    
	    $editallow = 1;
	    
	    $initarr   = NULL;
	    $initarr["title"]       = "Modify Test-Data"; 
	    $initarr["action"]       = "p.php";
	    
	    $hiddenarr = NULL;
	    $hiddenarr["id"]     = $this->objid;
	    $hiddenarr["mod"]     = $this->_mod;
	    $hiddenarr["action"]     = 'save_UT';
	    
	    $formobj = new formPure($initarr, $hiddenarr, 0);
	    $formobj->start();
	    
	    echo "<h3>UnitTest data Editor</h3><br>";
	    $header=array('Step-Nr', 'Step-Name', 'MAA', 'Test-Value');
	    echo "<table>\n";
	    echo '<tr>';
	    foreach($header as $cell) {
	        echo '<th>'.$cell.'</th>';
	    }
	    echo '</tr>';
	    
	    
	    foreach($data_show as $ind=>$row) {
	        
	        $step_nr = $row['step'];
	        if ($step_nr) {
	            $pra_step_info_row = $this->pra_steps[$step_nr];
	        } else {
	            $pra_step_info_row=array('NAME'=>'?');
	        }
	        $sua_id = $pra_step_info_row['ABSTRACT_SUBST_ID'];
	        $sua_name='';
	        if ($sua_id) {
	            $sua_name=obj_nice_name ( $sqlo, 'ABSTRACT_SUBST', $sua_id ); 
	        }
	        
	        echo ' <tr>'."\n";
	        
	        if ($editallow) {
	            $step_tmp='<input type="hidden" name="ut['.$ind.'][step]" value="'.$step_nr.'">'. $step_nr;
	        } else {
	            // echo '<input type="hidden" name="fl['.$this->_lineid.'][step]" value="'.$step_nr.'">';
	            $step_tmp=$step_nr;
	        }
	        $val_tmp='<input type="text" name="ut['.$ind.'][val]" value="'.$row['val'].'" size=6>';

	        echo '  <td>'.$step_tmp.'</td>'."\n";
	        echo '  <td>'.$pra_step_info_row['NAME'].'</td>'."\n";
	        echo '  <td>'.$sua_name.'</td>'."\n";
	        echo '  <td>'.$val_tmp.'</td>'."\n";
	        echo ' </tr>'."\n";
	    }
	    echo "</table>";
	    echo '<input class="yButton" type=submit value="Save">'."\n";
	    
	    $formobj->close( TRUE ); // close form, sshow submit-button
	}
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		
		$this->objid  = $_REQUEST['id'];
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
		$action = $_REQUEST['action'];
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		$pra_step_lib = new oABSTRACT_PROTO_stepx($this->objid );
		$this->pra_steps = $pra_step_lib->getStepArray_i2($sqlo, array('NAME', 'QUANTITY', 'ABSTRACT_SUBST_ID'));
		
		$this->formula_lib     = new oABSTRACT_PROTO_formula($sqlo, $this->objid );
		$this->formula_mod_lib = new oABSTRACT_PROTO_formula_mod($sqlo, $this->objid );
		
		echo '[<a href="p.php?mod='.$this->_mod.'&id='.$this->objid.'">Editor</a>] ';
		echo "<br>";
		
		if ($action=='add') {
		   
		    $line_id=$_REQUEST['line_id'];
		    echo 'ADD: '.$line_id."<br>";
		    $this->formula_mod_lib->ins_line($line_id);
		    $this->formula_mod_lib->save_formula($sqlo);
		    
		    // reload ...
		    $this->formula_lib     = new oABSTRACT_PROTO_formula($sqlo, $this->objid );
		    
		}
		
		if ($action=='save_UT' and $go==1) {
		    
		    $ut_rows=$_REQUEST['ut'];
		    $this->formula_mod_lib->set_UT_data($ut_rows);
		    $this->formula_mod_lib->save_formula($sqlo);

		    // reload ...
		    $this->formula_lib     = new oABSTRACT_PROTO_formula($sqlo, $this->objid );
		    
		}
		
		if (!$go) {
		    
		    if ( empty($this->formula_lib->get_UT_data()) or $action=='save_UT' ) {
		        $this->form2_UT($sqlo);
		    } else {
		    
			    $this->form1($sqlo);
		    }
			return;
		}
		
		$formula_struct_IN = $_REQUEST['fl'];
		
		$ut_data = $this->formula_lib->get_UT_data();
		//$ut_data = array( 4=>4.1, 5=>5.1);
		
		$this->formula_mod_lib->check_user_input($formula_struct_IN, $ut_data);
		
		if ($error->Got(READONLY))  {
			return;
		}
		$this->formula_mod_lib->save_formula($sqlo);
		
		
	}
	
}