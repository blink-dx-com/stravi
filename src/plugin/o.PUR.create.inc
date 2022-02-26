<?php
require_once ("f.objview.inc");
require_once ("insertx.inc");
require_once 'o.H_UNIT.siSubs.inc';
require_once 'lev1/glob.obj.create_subs.inc';

/**
 * PUR create
 * @package o.PUR.create.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param  $parx
 *     'ABSTRACT_SUBST_ID'
 * @param $go 
 *   0 - select SUA
 *   1 - select params
 *   2 - create 
 *   
 * @param $newparams : alternative to parx: comes from glob.obj.crea_wiz.php
 */
class o_PUR_create_XPL extends gPlugin {
	
    const table='PUR';
    const NEED_VALS = array(
        'H_UNIT_ID'      =>array('col'=>'H_UNIT_ID', 'req'=>1),
        'PROD_CONTACT_ID'=>array('col'=>'CONTACT_ID','req'=>0),
        'PROORI_CON_ID'  =>array('col'=>'SUA.PROORI_CON_ID','req'=>0, 'view'=>1),  # just for info ..
        'PROD_NO'        =>array('col'=>'PROD_NO'   ,'req'=>0),
        'CONT_QUANT'     =>array('col'=>'CONT_QUANT','req'=>0),
    );
	
	function register() {
		$tablename	= 'PUR';
		$title		= 'Create new';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['version']  = '2021-11-15';

		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('write','insert') )
		);
	}
	
	function form1($sqlo, $in_params) {
	    
	    $sua_id=$in_params['ABSTRACT_SUBST_ID'];
	    
	    $initarr   = array();
	    $initarr["title"]       = "Parameters";
	    $initarr["submittitle"] = "Next &gt;"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    $initarr["dblink"]=1 ;
	    
	    
	    $hiddenarr = NULL; // hidden form variables

	    $formobj = $this->_html->formInit($initarr, $hiddenarr);
	    
	    
	    
	    $sua_table='ABSTRACT_SUBST';
	    $fieldx = array ( // form-field definition
	        "title"   => tablename_nice2($sua_table),
	        "name"  => "ABSTRACT_SUBST_ID",
	        "object"=> "dblink",
	        "val"    => $sua_id,
	        "notes" => "Material",
	        "req"  => 1,
	        "inits" => array( 'table'=>$sua_table, 'getObjName'=>1, 'sqlo'=>&$sqlo, 'pos' =>'0', 'projlink'=> 1)
	    );
	    $formobj->fieldOut( $fieldx );
	    
	    
	    
	    $formobj->close( TRUE );
	    
	    
	}
	
	function form2($sqlo, $in_params) {
	    
	    $sua_id = $in_params['ABSTRACT_SUBST_ID'];
	    $sua_table='ABSTRACT_SUBST';
	    $tablename='PUR';
	    
	    $initarr   = array();
	    $initarr["title"]       = "Parameters";
	    $initarr["submittitle"] = "Create"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    $initarr["dblink"]=1 ;
	    
	    /*
	    'H_UNIT_ID'      =>array('col'=>'H_UNIT_ID', 'req'=>1),
	    'PROD_CONTACT_ID'=>array('col'=>'CONTACT_ID','req'=>0),
	    'PROORI_CON_ID'  =>array('col'=>'SUA.PROORI_CON_ID','req'=>0, 'view'=>1),  # just for info ..
	    'PROD_NO'        =>array('col'=>'PROD_NO'   ,'req'=>0),
	    'CONT_QUANT'     =>array('col'=>'CONT_QUANT','req'=>0),
	    */
	    
	    $input_keys=array(
	        'H_UNIT_ID'=>array('req'=>1),
	        'SUA.PROORI_CON_ID'=>array('view'=>1, 'fk_table'=>'CONTACT', 'title'=>'Original producer', 'notes'=>'defined in the MAA'),
	        'CONTACT_ID'=>array('req'=>0),
	        'PROD_NO'=>array('req'=>1),
	        'CONT_QUANT'=>array('req'=>1),
	        'QUANTITY'=>array('req'=>1),
	        'NOTES'=>array('req'=>0)
	    );
	    
	    
	    $hiddenarr = NULL; // hidden form variables
	    $hiddenarr['parx[ABSTRACT_SUBST_ID]'] = $sua_id;
	    
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 1);
	    
	    $sua_table='ABSTRACT_SUBST';
	    $fieldx = array ( // form-field definition
	        "title"   => tablename_nice2($sua_table),
	        "object"=> "info2",
	        "val"=> fObjViewC::	bo_display( $sqlo, $sua_table, $sua_id)  
	    );
	    $formobj->fieldOut( $fieldx ); 
	    
	    $pos=0;
	    foreach($input_keys as $column=>$row) {
	        
	        $val = $in_params[$column];
	        
	        if ($row['fk_table']) {
	            
	            $col_features=array();
	            $fieldx = array ( 
	                "title"   => $row['title'],
	                "object"=> "info2",
	                "val"=>'not given in MAA',
	                'notes'=>$row['notes'],
	            );
	            if ($val) $fieldx['val']=fObjViewC::bo_display( $sqlo, 'CONTACT', $val);
	            
	        } else {
    	        $col_features = colFeaturesGet($sqlo, $tablename, $column);
    	        $fieldx = array ( 
    	            "title" => $col_features['NICE_NAME'],
    	            "name"  => $column,
    	            "object"=> "text",
    	            "val"=> $val,
    	            'req'=> $row['req']
    	        );
	        }
	        
	        if ($col_features["CCT_TABLE_NAME"]) {
	            $fieldx["object"]='dblink';
	            $fieldx["inits"] = array( 'table'=>$col_features["CCT_TABLE_NAME"], 
	                'getObjName'=>1, 'sqlo'=>&$sqlo, 'pos' =>$pos, 'projlink'=> 1);
	        }
	        if ($column=='H_UNIT_ID') {
	            
	            $tmp_si_obj = new oH_UNIT_siSubs();
	            $extraWhere = $tmp_si_obj->get_si_sql_cond($sqlo);
	            
	            $inits=array();
	            $sqlsel = "H_UNIT_ID, NAME from H_UNIT where ".$extraWhere;
	            $sqlo->Quesel($sqlsel);
	            while ( $sqlo->ReadRow() ) {
	                $tmpid = $sqlo->RowData[0];
	                $tmp_name = $sqlo->RowData[1];
	                $inits[$tmpid]=$tmp_name;
	            }
	            $fieldx["object"]= 'select';
	            $fieldx["inits"] = $inits;
	            
	        }
	        
	        $formobj->fieldOut( $fieldx ); 
	        $pos++;
	    }
	    
	    $formobj->close( TRUE );
	    
	    
	}
	
	function check_input1($sqlo, $parx) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $sua_table='ABSTRACT_SUBST';
	    if (!$parx['ABSTRACT_SUBST_ID']) {
	        $error->set( $FUNCNAME, 10, columnname_nice2($sua_table, 'ABSTRACT_SUBST_ID') .' is missing.');
	        return;
	    }
	    
	    $err_collect=array();
	    
	    $sua_id=$parx['ABSTRACT_SUBST_ID'];
	    
	    // check parameters
	    $sua_params = glob_ObjDataGet( $sqlo, $sua_table, $sua_id, array('*'));
	    
	    
	    
	    $new_params=array(
	        'ABSTRACT_SUBST_ID'=>$sua_id
	    );
	    
	    $tab_nice =  tablename_nice2($sua_table);
	    foreach(self::NEED_VALS as $key=>$row) {
	        
	        $val = $sua_params[$key];
	        $required = $row['req'];
	        $new_col  = $row['col'];
	        
	        if ($val===NULL or $val==='') {
	            if ($required) {
	                $err_collect[] =  $tab_nice.', ID:'.$sua_id.' column: '.columnname_nice2($sua_table, $key).' : data missing.';
	            }
	        } else {
	            
	            $new_params[$new_col] = $val;
	            
	        }
	      
	    }
	    
	    
	    if(!empty($err_collect)) {
	        $error->set( $FUNCNAME, 20, implode('<br>',$err_collect));
	        return;
	    }
	    
	    return $new_params;
	}
	
	function check_input2($sqlo, $parx) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $new_params=array();
	    $need_vals = array(
	        'ABSTRACT_SUBST_ID'=>array('req'=>1),
	        'H_UNIT_ID' =>array('req'=>1),
	        'CONTACT_ID'=>array('req'=>0),
	        'PROD_NO' =>array('req'=>1),
	        'CONT_QUANT'=>array('req'=>1),
	        'QUANTITY'=>array('req'=>1),
	        'NOTES'   =>array('req'=>0),
	    );
	    $err_collect=array();
	    
	    $sua_id=$parx['ABSTRACT_SUBST_ID'];
	    $sua_params = glob_ObjDataGet( $sqlo, 'ABSTRACT_SUBST', $sua_id, array('NAME'));
	    $dateHuman = date  ('Y-m-d');
	    
	    
	    foreach($need_vals as $key=>$row) {
	        $val = trim($parx[$key]);
	        if ($row['req']) {
    	        if ($val===NULL or $val==='') {
    	            $err_collect[] =  $key.' > column: '.columnname_nice2('PUR', $key).' : data missing.';
    	        }
	        }
	        $new_params[$key]=$val;
	    }
	    
	    if(!empty($err_collect)) {
	        $error->set( $FUNCNAME, 20, implode('<br>',$err_collect));
	        return;
	    }
	    
	    $new_name=$dateHuman.' '.$sua_params['NAME'];
	    $new_params['NAME']=$new_name;
	    
	    return $new_params;
	}
	
	function create($sqlo, $parx) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
    	$insertlib = new insertC();
    	$args= array( "vals"=>$parx );
    	$new_id = $insertlib->new_meta($sqlo, self::table, $args);
    	if ($error->Got(READONLY))  {
    	    $error->set( $FUNCNAME, 1, 'Creation-Error' );
    	    return;
    	}
    	
    	$wf_manage_lib = new gObjCreaSubs();
    	$wf_manage_lib->addUserWorkflow($sqlo, self::table, $new_id);
    	
    	return $new_id;
    	
    }
	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
		
		if (!empty($_REQUEST['newparams'] )) {
		    $parx = $_REQUEST['newparams'];
		}
		
		if (!$go) {
		    $this->form1($sqlo, $parx);
		}
		
		if ($go==1) {
    		$new_params = $this->check_input1($sqlo, $parx);
    		if ($error->Got(READONLY))  {
    		    $error->set( $FUNCNAME, 1, 'User-Input-Error' );
    		    return;
    		}
    		
    		$this->form2($sqlo, $new_params);
    		return;
		}
		
		if ($go==2) {
		    
		    $new_params = $this->check_input2($sqlo, $parx);
		    if ($error->Got(READONLY))  {
		        $error->set( $FUNCNAME, 1, 'User-Input-Error' );
		        return;
		    }
		    
		    $new_id = $this->create($sqlo, $new_params);
 
		    if ($error->Got(READONLY))  {
		        $error->set( $FUNCNAME, 1, 'Creation Error' );
		        return;
		    }
		    
		    $this->_html->pageForward('edit.tmpl.php?t=PUR&id='.$new_id, $link_text);
		    
		}
		
	}
}
