<?php
/**
 * manage a Material-ReferenceConfig
 * @package o.LINK.SUC_REF_config.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   int $id
 * @param $action
 *     doc_create
 *     pra_sel   -- activate a PROTOCOL;  needs pno, sua
 *     pra_edit  -- needs pno, sua
 *     pra_deact
 *     pra_config -- needs pno, sua
 *     
 *     sua_pra_act   -- activate  -- need st_path
 *     sua_pra_deact -- deactivate
 *     
 *     vers_new  -- need parx[vers_new]
 *     set_valid_version  -- need parx[version]
 *     set_act_version    -- need parx[version]
 * @param $parx
 *     sua  SUA-ID
 *     pno  protocol-log:STEP_NO
 *     
 */
require_once ('javascript.inc'); 
require_once ('func_form.inc');
require_once ("f.objview.inc");
require_once ("o.proj.profile.inc");
require_once ("o.PROJ.addelems.inc");
require_once ("insertx.inc");
require_once dirname(__FILE__).'/../www/pionir/impexp/suc_ref/o.LINK.TTC.inc';
require_once dirname(__FILE__).'/../www/pionir/impexp/suc_ref/o.LINK.TTC.GUI.inc';



class o_LINK_SUC_REF_config_XPL extends gPlugin {
	
    
	function register() {
		$tablename	= 'LINK';
		$title		= 'manage a Material-Reference-Config';
		$action = $_REQUEST['action'];
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		$this->infoarr['locrow']   = array(  // defines the tool location-path
		    array('p.php?mod=DEF/o.CONCRETE_SUBST.TTC_home', o_LINK_TTC::TOOL_NAME),
		); 

		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('write'), 'obj'=>array('read','write') )
			);
		
		if ($action=='doc_create') {
		    $this->infoarr['form_type']= 'tool'; 
		    $this->infoarr['checkid']  = 0;
		    $this->infoarr2 = array(
		        'accCheck' => array('tab'=>array('insert') )
		    );
		}
	}
	private function get_current_data() {
	    $version = $this->doc_lib->current_version();
	    $data = $this->doc_lib->get_data_of_version($version);
	    return $data;
	}
	
	function form_SUA($sqlo, $sua_id) {
	    
	   
	    $initarr   = NULL;
	    $initarr["title"]       = "Set ".tablename_nice2('ABSTRACT_SUBST');
	    $initarr["submittitle"] = "Set";
	    $initarr["tabwidth"]    = "AUTO";
	    $initarr["dblink"]=1;
	    
	    $hiddenarr = NULL;
	    $hiddenarr['id'] = $this->obj_id;
	    
	    $formobj = $this->_html->formInit($initarr, $hiddenarr);
	    
	    $fieldx = array (
	        "title" => "Linked ".tablename_nice2('ABSTRACT_SUBST'),
	        "name"  => "sua_id",
	        "object"=> "dblink",
	        "val"   => $sua_id,
	        "inits" => array( 'table'=>'ABSTRACT_SUBST', 'getObjName'=>1, 'sqlo'=>&$sqlo, 'pos' =>'0', 'projlink'=> 1),
	        "notes" => ""
	    );
	    $formobj->fieldOut( $fieldx );
	    
	    $formobj->close( TRUE );
	}
	
	function form_DOC_new() {
	    
	    
	    $initarr   = NULL;
	    $initarr["title"]       = "Create new config ";
	    $initarr["submittitle"] = "Set";
	    $initarr["tabwidth"]    = "AUTO";
	   
	    
	    $hiddenarr = NULL;
	    $hiddenarr['action'] = 'doc_create';
	    
	    $formobj = $this->_html->formInit($initarr, $hiddenarr);
	    
	    $fieldx = array (
	        "title" => "Name",
	        "name"  => "name",
	        "object"=> "text",
	        "req"=>1,
	        "val"   => '',
	        "notes" => ""
	    );
	    $formobj->fieldOut( $fieldx );
	    
	    $formobj->close( TRUE );
	}
	
	
	
	//------------------
	
	function _act_doc_create($sqlo, $parx) {
	    global $error, $varcol;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $projProfObj = new profile_funcs ();
	    $actionxc   = "CREATE";
	    $proj_id = $projProfObj->getProjBySyn( $sqlo, "misc", $actionxc );
	
	    $name=trim($parx['name']);
	    if ($name=='') {
	        $error->set( $FUNCNAME, 1, 'Name missing.' );
	        return;
	    }
	    
	    $varcol = & Varcols::get();
	   
	    $class_id = $varcol->class_name_to_id( 'LINK', o_LINK_TTC::DOC_CLASS_REF );
	   
	    $insertlib = new insertC();
	    $args= array( 
	        "vals"=>array(
	            'NAME'=>$name
	        ), 
	        "xobj"=>array(
	            'extra_class_id'=>$class_id
	        )
	        
	    );
	    $doc_id = $insertlib->new_meta($sqlo, 'LINK', $args);
	    
	   
	    // DESCR:   functions to add object links to projects
	    $projAddLib = new oProjAddElem($sqlo, $proj_id);
	    $projAddLib->addObj( $sqlo, 'LINK', $doc_id );
	    
	    return $doc_id;
	}
	
	function _act_sua_pra_act($sqlo, $parx) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $data = &$this->get_current_data();
	    
	    
	    if (!$parx['sua'] or !$parx['pno'] or !$parx['st']) {
	        $error->set( $FUNCNAME, 1, 'Input missing:'.print_r($parx,1) );
	        return;
	    }
	    
	    $lev0_pradata = &$data[$parx['sua']][$parx['pno']];  // a pointer !!!
	    $steps        = $lev0_pradata['steps'];
	    $lev0_index = o_LINK_TTC_one_vers::IND_of_STEP_STRUCT( $steps, $parx['st'] );
	    
	    $one_step_info = $steps[$lev0_index];
	    $one_step_info['sua_link']=1;
	    
	    $new_sua_id = $one_step_info['sua'];
	    
	    // write back
	    $lev0_pradata['steps'][$lev0_index] = $one_step_info;
	    
	    if ($new_sua_id) {
	       $data[$new_sua_id]=array(); // new SUA entry ...
	    }  
	    
	    $this->doc_lib->set_one_proto_def($data);
	    $this->doc_lib->save_config($sqlo);
	}
	
	function _act_sua_pra_deact($sqlo, $parx) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $data = &$this->get_current_data();
	    
	    
	    if (!$parx['sua'] or !$parx['pno'] or !$parx['st']) {
	        $error->set( $FUNCNAME, 1, 'Input missing:'.print_r($parx,1) );
	        return;
	    }
	    
	    $lev0_pradata = &$data[$parx['sua']][$parx['pno']];  // a pointer !!!
	    $steps        = $lev0_pradata['steps'];
	    $lev0_index = o_LINK_TTC_one_vers::IND_of_STEP_STRUCT( $steps, $parx['st'] );
	    
	    $one_step_info = $steps[$lev0_index];
	    unset($one_step_info['sua_link']);
	    
	    $new_sua_id = $one_step_info['sua'];
	    
	    // write back
	    $lev0_pradata['steps'][$lev0_index] = $one_step_info;
	    
	    if ($new_sua_id) {
	        unset($data[$new_sua_id]); // delete SUA entry ...
	    }
	    
	    $this->doc_lib->set_one_proto_def($data);
	    $this->doc_lib->save_config($sqlo);
	}
	
	function _act_vers_new($sqlo, $parx) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	   
	    $vers_new = $parx['vers_new'];
	    $version = $this->doc_lib->current_version();
	    if($version==$vers_new) {
	        $error->set( $FUNCNAME, 1, 'Version already exists.' );
	        return;
	    }
	    $this->doc_lib->create_new_version();
	    $this->doc_lib->save_config($sqlo);
	}
	
	function _act_set_valid_version($sqlo, $parx) {
	    $vers_new = $parx['version'];
	    $this->doc_lib->set_valid_version($vers_new);
	    $this->doc_lib->save_config($sqlo);
	}
	
	function _act_act_version($sqlo, $parx) {
	    $vers_new = $parx['version'];
	    $_SESSION['s_formState'][$this->mod]=array('version'=>$vers_new, 'doc_id'=>$this->obj_id);
	}
	

	/**
	 * activate ONE protocol $parx['pno'] (or 'pno' ==> 'st' | pno )
	 * create EMPTY VERS_PROTO_PRA_STRUCT
	 * $parx['sua'] or !$parx['pno']
	 * @param object $sqlo
	 * @param array  $parx
	 */
	function _act_pra_sel($sqlo, $parx) {
	    $data = &$this->get_current_data();
	    
	    /**
	     * PROTO_NO => {
                'ap'=>433,
                'steps'=> [ VERS_PROTO_PRA_STEPS_STRUCT
                ]
             }
	     */
	    $log_step_no = $parx['pno'];
	    $sua_id=$parx['sua'];
	    $proto_log = o_SUA_TTC_proto::get_protolog($sqlo, $sua_id);
	    
	    $tmp = $proto_log[$log_step_no];
	    $pra_id = $tmp['ap'];
	    $data[$sua_id][$log_step_no]= array('ap'=>$pra_id, 'steps'=>array() );
	    
	    $this->doc_lib->set_one_proto_def($data);
	    $this->doc_lib->save_config($sqlo);
	    // $this->doc_lib->reload();
	}
	
	function _act_pra_deact($sqlo, $parx) {
	    $data = &$this->get_current_data();
	   
	    
	    /**
	     * PROTO_NO => {
	     'ap'=>433,
	     'steps'=> [ VERS_PROTO_PRA_STEPS_STRUCT
	     ]
	     }
	     */
	    $log_step_no = $parx['pno'];
	    $sua_id=$parx['sua'];
	   
	    unset( $data[$sua_id][$log_step_no] );
	    
	    $this->doc_lib->set_one_proto_def($data);
	    $this->doc_lib->save_config($sqlo);
	    
	}
	
	
	/**
	 * edit ONE ABSTRACT_PROTO
	 * @param object $sqlo
	 * @param array  $parx
	 *    'sua'
	 *    'pno'
	 *    'sua_sel'
	 *    'sua_adv' FUTURE: => 0,1 ==> advanced sua selection activated
	 */
	function _act_pra_edit($sqlo, $parx) {
	    $data = &$this->get_current_data();
	    
	    if (empty($parx['sua_sel'])) {
	        echo 'Nothing selected<br>'."\n";
	        return;
	    }
	    
	    $log_step_no = $parx['pno'];
	    $sel_sua_id  = $parx['sua'];

	    $step_arr=array();
	    foreach($data[$sel_sua_id] as $proto_no => $row) {
	        if ($log_step_no==$proto_no ) {
	            foreach($parx['sua_sel'] as $step_nr =>$sua_id ) {
	                if ($sua_id) {
	                   $step_arr[]=array('st'=>$step_nr, 'sua'=>$sua_id);
	                }
	            }
	        }
	    }
	    $data[$sel_sua_id][$proto_no]['steps']= $step_arr;
	    echo 'Protocol steps updated.<br>';

	    
	    $this->doc_lib->set_one_proto_def($data);
	    $this->doc_lib->save_config($sqlo);
	    
	    // $this->doc_lib->show_data_struct();
	    // $this->doc_lib->reload();
	}
	
	function activate_version() {
	    $this->edit_vers=0;
	    $sess_config = $_SESSION['s_formState'][$this->mod];

	    
	    if(!empty($sess_config)) {
	        if ($sess_config['doc_id']==$this->obj_id) {
	            $this->edit_vers = $sess_config['version'];
	            $this->doc_lib->set_edit_version($this->edit_vers);
	        }
	    }
	    if (!$this->edit_vers) {
	        $this->edit_vers = $this->doc_lib->get_released_version_no();
	    }

	}

	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		$obj_id   = $_REQUEST['id'];
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
		$action = $_REQUEST['action'];
		
		$this->parx   = &$parx;
		$this->action = $action;
		$this->obj_id = $obj_id;
		
		if ($action=='doc_create') {
		    if (!$go) {
		        $this->form_DOC_new();
		        return;
		    }
		    $new_doc_id = $this->_act_doc_create($sqlo, $parx);
		    $this->_html->pageForward($this->_plug_get_URL().'&id='.$new_doc_id, 'go to object');
		}
		
		$edit_flag = 1;
		
		$o_rights = access_check($sqlo, 'LINK', $this->obj_id);
		if ($o_rights['write']<=0) {
		    $edit_flag=0;
		}

		$this->doc_lib = new o_LINK_TTC($sqlo, $obj_id);
		if ($error->Got(READONLY))  {
		    $error->set( $FUNCNAME, 1, 'Error on class init.' );
		    return;
		}
		$this->activate_version();
		$version = $this->doc_lib->current_version();
		
		
		echo '<a href="'.$this->_plug_get_URL().'&id='.$obj_id.'">HOME</a>';
		
		if ($edit_flag) {
		    $last_version = $this->doc_lib->last_version();
		    $version_new=$last_version+1;
		    echo ' | <a href="'.$this->_plug_get_URL().'&id='.$obj_id.'&action=vers_new&parx[vers_new]='.$version_new.'">CREATE new version</a>';
		}
		
		echo '<br><br>';
		
		//var_dump($this->doc_lib);

		
		if (!$this->doc_lib->get_SUA()) {
		    
		    $sua_id=$parx['sua_id'];
		    if (!is_numeric($sua_id)) $sua_id=0;
		    if (!$go or !$sua_id) {
		        $sua_id=0;
		        $this->form_SUA($sqlo, $sua_id);
		        return;
		    }
		    $this->doc_lib->set_sua($sqlo, $sua_id);
		    $this->doc_lib->create_new_version();
		    $this->doc_lib->save_config($sqlo);
		    // reload ???
		}
		
		switch ($action) {
		    case 'vers_new':
		        if (!$parx['vers_new']) {
		            $error->set( $FUNCNAME, 318, 'Input missing' );
		            return;
		        }
		        $this->_act_vers_new($sqlo, $parx);
		        break;
		    case 'set_valid_version':
		        if(!$parx['version']) {
		            $error->set( $FUNCNAME, 325, 'Input missing' );
		            return;
		        }
		        $this->_act_set_valid_version($sqlo, $parx);
		        break;
		        
		    case 'set_act_version':
		        if(!$parx['version']) {
		            $error->set( $FUNCNAME, 335, 'Input missing' );
		            return;
		        }
		        $this->_act_act_version($sqlo, $parx);
		        $this->activate_version();
		        break;
		        
		    case 'pra_sel':
		        if (!$parx['sua'] or !$parx['pno']) {
		            $error->set( $FUNCNAME, 342, 'Input missing' );
		            return;
		        }
		        $this->_act_pra_sel($sqlo, $parx);
		        
		        $this->action='pra_edit';
		        
		        break;
		    case 'pra_edit':
		        if (!$go) {
		            break;
		        }
		        if (!$parx['sua'] or !$parx['pno']) {
		            $error->set( $FUNCNAME, 298, 'Input missing' );
		            return;
		        }
		        $this->_act_pra_edit($sqlo, $parx);
		        break;
		    case 'pra_deact':
		        if (!$parx['sua'] or !$parx['pno']) {
		            $error->set( $FUNCNAME, 352, 'Input missing' );
		            return;
		        }
		        $this->_act_pra_deact($sqlo, $parx);
		        break;
		        
		    case 'sua_pra_act':
		        $this->_act_sua_pra_act($sqlo, $parx);
		        break;
		    case 'sua_pra_deact':
		        $this->_act_sua_pra_deact($sqlo, $parx);
		        break;
		}
		
		
		
		$config_show_lib = new o_LINK_SUC_REF_config_SHOW($sqlo, $this->obj_id, $this->_mod, $this->action, $this->parx);
		$config_show_lib->set_edit_flag($edit_flag);
		$config_show_lib->set_edit_version($this->edit_vers);
		
		// echo "DDDX: ".$this->edit_vers."<br>";
		
		
		$config_show_lib->view_versions($sqlo);
		echo "<br>";
		
		$config_show_lib->view_structure($sqlo);
		
		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
		  $this->doc_lib->show_data_struct();
		}
	}
}
