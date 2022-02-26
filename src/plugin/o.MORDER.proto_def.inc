<?php
require_once ('object.info.inc');
require_once 'func_form.inc';
require_once ("visufuncs.inc");

require_once 'o.S_OBJLINK.subs.inc';
require_once 'o.ABSTRACT_PROTO.subs.inc';
require_once 'o.ABSTRACT_SUBST.proto.inc';
require_once 'o.MORDER.proto_def.inc';
require_once 'o.S_VARIO.mod.inc';
/**
 * define protocols, which are expected ...
 * @package o.MORDER.proto_def.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   int $id
 */
class o_MORDER_proto_def_XPL extends gPlugin {
	
	
	function register() {
		$tablename	= 'MORDER';
		$title		= 'Define Protocols';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('write'), 'obj'=>array('read', 'write') )
			);
	}
	
	
	function form1($sqlo, $sua_log, $proto_use_def) {
	    
	    
	    $praEmailKey   = oMORDER_proto_def::S_OBJLINK_PRA_GrpEmailKey;
	    
	    $initarr   = array();
	    $initarr["title"]       = "Select protocols, which should be performed"; 
	    $initarr["submittitle"] = "Save"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    
	    $hiddenarr = NULL; // hidden form variables
	    $hiddenarr["id"]     = $this->objid; 
	    $hiddenarr["mod"]    = $this->_mod; 
	    $initarr['colspan']  = 4;
	    
	    // init form, uses class formc
	    $formobj = new formc($initarr, $hiddenarr, 0);
	    
	    $header=array(
	        'Order',
	        '',
	        'PRA',
	        'Type',
	        'Info'
	        
	    );
	    $formobj->row_open();
	    foreach($header as $key) {
	        echo '<th>'.$key.'</th>';
	    }
	    $formobj->row_close();
	    
	    foreach($sua_log as $row) {
	        
	        $pra_id = $row['ap'];
	        $order  = $row['or'];
	        $pra_nice = obj_nice_name ( $sqlo, 'ABSTRACT_PROTO', $pra_id ); 
	        $type_nice='';
	        $type_style='';
	        
	        $pra_feats = oABSTRACT_PROTO_subs::get_PROTO_KIND_features($sqlo, $pra_id);
	        if (!empty($pra_feats)) {
	           $type_nice =$pra_feats['NAME'];
	           if ($pra_feats['KEYX']=='s.QC') {
	               $type_style='style="background-color:#e0e0A0"';
	           }
	        }
	        $val=0;
	        if ($proto_use_def[$order]==$pra_id) {
	            $val=1;
	        }

	        $fieldx = array ( // form-field definition
	            "title" => $order.': '.$pra_nice,
	            "name"  => "pra[".$pra_id."]",
	            'namex' => TRUE,
	            "object"=> "checkbox",
	            "val"    => $val,  
	        );
	        $formobj->row_open();
	        echo '<td>'.$order.'</td>';
	        echo '<td>';
	        $formobj->_valueField( $fieldx ); // output the form-field
	        echo '</td>';
	        echo '<td>'.$pra_nice.'</td>';
	        echo '<td '.$type_style.'>'.$type_nice.'</td>';
	        
	        $info_tmp='';
	        if ($this->suc_code_script) {
	            
	            $email_groups  = oS_OBJLINK_subs::getChiObjByTabOrKey($sqlo, 'ABSTRACT_PROTO', $pra_id, 'USER_GROUP', $praEmailKey);
	            if (!empty($email_groups)) {
	                $grp_names=array();
	                foreach($email_groups as $grp_id_loop) {
	                    $grp_name = obj_nice_name( $sqlo, 'USER_GROUP', $grp_id_loop );
	                    $grp_names[]=$grp_name;
	                }
	                $info_tmp .='send email on Status "tech_ready" to GRPs: '.implode(', ',$grp_names).'. ';
	            }
	            
	            $vario_key=oMORDER_proto_def::VARIO_PRA_AuditPostAction;
	            $pra_action_flag = oS_VARIO_sub::getValByTabKey($sqlo, 'ABSTRACT_PROTO', $pra_id, $vario_key);
	            if ($pra_action_flag) {
	                $info_tmp .= 'Trigger Email-Action after "tech_ready" for next planned Protocol.';
	            }
	        }
	        echo '<td>'.$info_tmp.'</td>';
	        
	        
	        $formobj->row_close();
	    }

	    $formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	function save($sqlo, $pra_input, $sua_log) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
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
	
	private function feature_table($sqlo, $sua_id) {
	    
	    $data_out=array();
	    
	    $vario_key=oMORDER_proto_def::VARIO_SUA_PRC_AuditActions;
    	$this->suc_code_script = oS_VARIO_sub::getValByTabKey($sqlo, 'ABSTRACT_SUBST', $sua_id, $vario_key);
    	$mess='no';
    	if ($this->suc_code_script) $mess='<b>YES</b>';
    	
    	$data_out[]=array('Send emails on protocol Audit-Status?', $mess, 
    	    '... when protocol changes Audit-Status to "tech_ready"? <br>'.
    	    'Configure with: <ul>'.
    	    '<li> MAA:VARIO:'.$vario_key.'=Code-File</li>'.
    	    '<li>the PRA which triggers action after ready: set PRA:VARIO:'.oMORDER_proto_def::VARIO_PRA_AuditPostAction.'=1 </li>'.
    	    '<Li>the next PRA which should inform Email-Groups:  the PRA:Object-Link-Key:'.oMORDER_proto_def::S_OBJLINK_PRA_GrpEmailKey.'</li>'.
    	    '</ul>'
    	    
    	);
    	

    	$tabobj = new visufuncs();
    	$headOpt = array( "title" => "Basic Settings", "headNoShow" =>1);
    	$headx   = array ("Key", "Val", "Notes");
    	$tabobj->table_out2($headx, $data_out,  $headOpt);
    	
    	echo "<br>";
	}
	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		$tablename	= 'MORDER';
		
		$sqlo = &$this->_sqlo;

		$this->objid = $_REQUEST['id'];
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
		
		// get protocol-order of sua
		
		
		$objInfoLib = new objInfoC();
		$raw_opt = array('vario'=>1);
		$obj_features = $objInfoLib->getParamsRaw ( $sqlo, $tablename, $this->objid, $raw_opt);
		
		$order_proto_lib = new oMORDER_proto_def($this->objid);
		$proto_use_def = $order_proto_lib->get_data($sqlo);
	
		$sua_id = $obj_features['vals']['ABSTRACT_SUBST_ID'];
		$sua_proto_lib = new oABSTRACT_SUBST_proto();
		$sua_proto_lib->set_obj($sqlo, $sua_id);
		$sua_log = $sua_proto_lib->get_protos($sqlo);
		
		// other features
		$this->feature_table($sqlo, $sua_id);
		
		
		
		if (!$go) {
		    $this->form1($sqlo, $sua_log, $proto_use_def);
		} else {
		    
		    $this->save($sqlo, $_REQUEST['pra'], $sua_log);
		    if ($error->Got(READONLY))  {
		        return;
		    }
		    
		    $this->_html->pageForward('edit.tmpl.php?t=MORDER&id='.$this->objid, 'go back', 1000);
		}
		
		
	}
}
