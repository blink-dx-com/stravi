<?php
require_once ('object.info.inc');
require_once  'func_form.inc';
require_once ("visufuncs.inc");
require_once  'javascript.inc';
require_once ("f.objview.inc");	

require_once 'o.S_OBJLINK.subs.inc';
require_once 'o.ABSTRACT_PROTO.subs.inc';
require_once 'o.ABSTRACT_SUBST.proto.inc';
require_once 'o.MORDER.proto_def.inc';
require_once 'o.S_VARIO.mod.inc';

require_once ("gui/o.PROTO.stepout2.inc");

/**
 * define EXPECTED protocols STEP DETAILS
 * - store in MORDER:VARIO:SUC.plan.proto.det (SUC => planned protocol details)
 * @package o.MORDER.proto_def2.inc
 * @swreq REQ-0028 o.MORDER > define EXPECTED protocols STEP DETAILS
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   int $id - MORDER_ID
 * @param  int $go
 * @param  $parx
 * @param  string $action
 *   '1_OVER'
 *   '2_PRA'  -- one pra, need parx['STEP']
 *            -- on go=1 : save: ....
 *   'remove' -- remove settings for selected parx['STEP']
 * @param quanti[]
	  ??? $newnote[]
	  $concsubst[]  
 */
class o_MORDER_proto_def2_XPL extends gPlugin {
	
	
	function register() {
		$tablename	= 'MORDER';
		$title		= 'Define PLANNED Protocol details';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		$this->infoarr['ureq.info'] = 'REQ-0028 o.MORDER > define EXPECTED protocols STEP DETAILS';
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('write'), 'obj'=>array('read', 'write') )
			);
	}
	
	
	function form1_selpra($sqlo, $proto_use_def) {

	    $sua_log = $this->sua_log;
	    
	    $initarr   = array();
	    $initarr["title"]       = "Protocols  overview"; 
	    $initarr["submittitle"] = "Delete settings"; 
	    $initarr["tabwidth"]    = "AUTO";  
	    
	    $hiddenarr = NULL; // hidden form variables
	    $hiddenarr["id"]     = $this->objid; 
	    $hiddenarr["mod"]    = $this->_mod; 
	    $hiddenarr["action"] = 'remove';
	    $initarr['colspan']  = 3;
	    
	    // init form, uses class formc
	    $formobj = new formc($initarr, $hiddenarr, 0);
	    
	    $header=array(
	        'Order',
	        'PRA',
	        'Info'
	        
	    );
	    $formobj->row_open();
	    foreach($header as $key) {
	        echo '<th>'.$key.'</th>';
	    }
	    $formobj->row_close();
	    
	    foreach($sua_log as $row) {
	        
	        $info_tmp='';
	        $pra_id = $row['ap'];
	        $order  = $row['or'];
	        $pra_nice = obj_nice_name ( $sqlo, 'ABSTRACT_PROTO', $pra_id ); 
	        
	        $old_params=array();
	        $old_step_info = &$this->old_params_all[$order];
	        if (is_array($old_step_info['steps'])) {
	            $old_params=$old_step_info['steps'];
	            if (!empty($old_params)) $info_tmp = sizeof($old_params).' step details defined.';
	        }


	        $formobj->row_open();
	        echo '<td>'.$order.'</td>';
	        
	        echo '<td><a href="'.$this->_plug_get_URL().'&id='.$this->objid.
	           '&action=2_PRA&parx[STEP]='.$order.'">'.$pra_nice.'</a></td>';
	       
	        
	       
	        echo '<td>'.$info_tmp.'</td>';
	        
	        
	        $formobj->row_close();
	    }

	    $formobj->close( FALSE ); // close form, sshow submit-button
	}
	
	function save($sqlo, $pra_input) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $sua_log = $this->sua_log;
	    $save_pra=array();
	    foreach($sua_log as $row) {
	        $pra_id = $row['ap'];
	        $order  = $row['or'];
	        if ($pra_input[$pra_id]){
	            $save_pra[$order]=intval($pra_id);
	        }
	    }
	    
	    $order_proto_lib = new oMORDER_proto_def($this->objid);
	    $order_proto_lib->save_data($sqlo, $save_pra);
	    if ($error->Got(READONLY))  {
	        return;
	    }
	    
	    $this->_html->messageBox2('ok', 'saved');
	    echo "<br>";
	}
	
	function act2_save($sqlo, $STEP, $pra_id, $params_one_proto) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $order_proto_lib = new oMORDER_proto_def($this->objid);
	    $params_all = $order_proto_lib->proto_details_LOAD($sqlo);
	    $params_all[$STEP]=array('pra'=>$pra_id, 'steps'=>$params_one_proto);
	    
	    debugOut('Data: '.print_r($params_all,1), $FUNCNAME, 1);
	    
	    $order_proto_lib->proto_details_SAVE($sqlo, $params_all);
	    if ($error->Got(READONLY))  {
	        return;
	    }
	    
	    $this->_html->messageBox2('ok', 'saved');
	    echo "<br>";
	}
	
	function act3_remove_save($sqlo, $STEP) {
	    global $error;

	    
	    $order_proto_lib = new oMORDER_proto_def($this->objid);
	    $params_all = $order_proto_lib->proto_details_LOAD($sqlo);
	    unset($params_all[$STEP]);

	    $order_proto_lib->proto_details_SAVE($sqlo, $params_all);
	    if ($error->Got(READONLY))  {
	        return;
	    }
	    
	    $this->_html->messageBox2('ok', 'saved');
	    echo "<br>";
	}
	
	private function form2_pra_det($sqlo, $STEP, $pra_id) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $old_params=array();
	    $old_step_info = &$this->old_params_all[$STEP];
	    if (is_array($old_step_info['steps'])) {
	        $old_params=$old_step_info['steps'];
	    }

	    $dummy2=NULL;
	    $stepOpt=array(
	        "ca_mode"=>protostep::CA_MODE_CONC
	    );
	    
	    $hiddenarr=array(
	        'mod'=>$this->_mod,
	        'id'=>$this->objid,
	        'action'=>'2_PRA',
	        'parx[STEP]'=>$STEP,
	        'go'=>1,
	        
	    );
	    
	    js_formAll();
	    echo '<form name="xtraform" ACTION="p.php" METHOD=POST>'."\n";
	    $proto_step_c = new  protostep( $sqlo, $dummy2, $pra_id, 1, 0, 0,  $stepOpt);
	    $proto_step_c->table_init();
	    $step_count = sizeof( $proto_step_c->step_array ) ;
	    $i=0;
	    while ( $i< $step_count ) {
	        $step_nr = $proto_step_c->step_array[$i];
	        
	        $cPar = $old_params[$step_nr];
	        if ($cPar['cs']) {
	            $cPar['canam'] = obj_nice_name ( $sqlo, 'CONCRETE_SUBST', $cPar['cs'] );
	        }
	        
	        $c_opt=array(
	            'cPars'=>$cPar
	        );
	        $proto_step_c->outstep( $step_nr, $sqlo, $c_opt );
	        $i++;
	    }
	    $proto_step_c->table_end();
	    
	    echo '<input type=submit value="Save" class="yButton">';
	    foreach($hiddenarr as $key=>$val) {
	        echo '<input type=hidden name="'.$key.'" value="'.$val.'">'."\n";
	    }
	    echo '</form>'."\n";
	}
	
	/**
	 * produce o.PROTO.steps.inc::CP_STEP_ONE_gui_STRUCT
	 * @param object $sqlo
	 * @param int $pra_id
	 * @param array $quanti
	 * @param array $concsubst
	 * @param array $newnote
	 */
	private function _transform_step_input($sqlo, $pra_id, $quanti, $concsubst, $newnote) {
	    
	    $params_out=array();
	    
	    $protLib = new gProtoOrg( );
	    $params_out=array();
	    $protLib->setProto( $sqlo, $pra_id, 0);
	    $step_array  = $protLib->step_array;
	    
	    foreach($step_array as $stepno =>$row) {
	        $tmparr=array();
	        if ($quanti[$stepno]!==NULL and $quanti[$stepno]!=='') {
	            $tmparr['qu']=$quanti[$stepno];
	        }
	        if ($concsubst[$stepno]>0) {
	            $tmparr['cs']=$concsubst[$stepno];
	        }
	        if ($newnote[$stepno]!=NULL) {
	            $tmparr['cn']=$newnote[$stepno];
	        }
	            
	        if (!empty($tmparr)) {
	            $params_out[$stepno]=$tmparr;
	        }
	        
	    }
	    return $params_out;
	}
	
	
	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		$tablename	= 'MORDER';
		
		$sqlo = &$this->_sqlo;

		$this->objid = $_REQUEST['id'];
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
		$action = $_REQUEST['action'];
		
		if ($action==NULL) $action='1_OVER';
		
		// get protocol-order of sua
		
		
		$objInfoLib = new objInfoC();
		$raw_opt = array('vario'=>1);
		$obj_features = $objInfoLib->getParamsRaw ( $sqlo, $tablename, $this->objid, $raw_opt);
		
		$order_proto_lib = new oMORDER_proto_def($this->objid);
		$proto_use_def = $order_proto_lib->get_data($sqlo);
	
		$sua_id = $obj_features['vals']['ABSTRACT_SUBST_ID'];
		$sua_proto_lib = new oABSTRACT_SUBST_proto();
		$sua_proto_lib->set_obj($sqlo, $sua_id);
		$sua_log_raw = $sua_proto_lib->get_protos($sqlo);
		
		//repair sua_log => only PREP protocols
		$this->sua_log=array();
		foreach($sua_log_raw as $row) {
		    if ($row['ty']==1) $this->sua_log[]=$row;
		}
		$order_proto_lib = new oMORDER_proto_def($this->objid);
		$this->old_params_all = $order_proto_lib->proto_details_LOAD($sqlo);
		
		switch ($action) {
		    
		    case '1_OVER':

        		if (!$go) {
        		    $this->form1_selpra($sqlo, $proto_use_def);
        		} else {
        		    $this->save($sqlo, $_REQUEST['pra']);
        		    if ($error->Got(READONLY))  {
        		        return;
        		    }
        		    
        		    $this->_html->pageForward('edit.tmpl.php?t=MORDER&id='.$this->objid, 'go back', 1000);
        		}
        		break;
        		
		    case '2_PRA':
		        
		        echo '<a href="'.$this->_plug_get_URL().'&id='.$this->objid .'">OVERVIEW</a> &gt; ';
		        
		        if (!$parx['STEP']) {
		            echo "<br>";
		            $this->htmlFoot("ERROR",'Input error: STEP missing.');
		        }
		        
		        $STEP = $parx['STEP'];
		        // get pra of step
		        $pra_id=0;
		        $found =0;
		        $sua_log = $this->sua_log;
		        foreach($sua_log as $row) {
		            $pra_id = $row['ap'];
		            $order  = $row['or'];
		            if ($order==$STEP) {
		                $found=1;
		                break;
		            }
		        }
		        if (!$found) {
		            echo "<br>";
		            $this->htmlFoot("ERROR",'No protocol found for STEP '.$STEP);
		            return;
		        }
		        echo fObjViewC::bo_display( $sqlo, 'ABSTRACT_PROTO', $pra_id);
		        
		        $old_params=array();
		        $old_step_info = &$this->old_params_all[$STEP];
		        if (!empty($old_step_info)) {
		            echo ' &nbsp;&nbsp;&nbsp; [<a href="'.$this->_plug_get_URL().
		              '&id='.$this->objid.'&action=remove&parx[STEP]='.$STEP.'"><img src="res/img/x.svg"> 
                        Remove all settings for this protocol</a>]';
		        }
		        
		        echo '<br><br>'."\n";
		        
		        
		        if (!$go) {
		            $this->form2_pra_det($sqlo, $parx['STEP'], $pra_id);
		        } else {
		            
		            $params = self::_transform_step_input($sqlo, $pra_id, $_REQUEST['quanti'], $_REQUEST['concsubst'], $_REQUEST['newnote']);
		            $this->act2_save($sqlo, $parx['STEP'], $pra_id, $params);
		            if (!$error->Got(READONLY))  $this->_html->pageForward( $this->_plug_get_URL().'&id='.$this->objid, 'back home');
		        }
		        
		        
		        break;
		        
		    case 'remove':
		        
		        echo '<a href="'.$this->_plug_get_URL().'&id='.$this->objid .'">OVERVIEW</a> &gt; Remove one setting'."<br>";
		        
		        if (!$parx['STEP']) {
		            echo "<br>";
		            $this->htmlFoot("ERROR",'Input error: STEP missing.');
		        }
		        
		        $this->act3_remove_save($sqlo, $parx['STEP']);
		        if (!$error->Got(READONLY))   $this->_html->pageForward( $this->_plug_get_URL().'&id='.$this->objid, 'back home');
		        
		        break;
		}
		
	}
}
