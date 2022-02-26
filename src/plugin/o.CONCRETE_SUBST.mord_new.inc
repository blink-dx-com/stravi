<?php
/**
 * create SUC by MORDER
 * OPTION on SUA:
 *    VARIO:MORDER.add_main_compo : PRA:{ID},STEP:{ID}
 * @package o.CONCRETE_SUBST.mord_new.inc
 * @swreq
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $_REQUEST['parx']
 *   'morder_id'
 *   'pos' : next MAC pos of MORDER, must be set if old SUC exists
 *    pos_extra: 0,1 -- confirm creation of SUC
 *    prc_copy: 0,1 -- copy PREP-protocols on creation ?
 *   'suc_ori' : OPTIONAL copy from this SUC
 */

require_once ('object.info.inc');
require_once ("f.objview.inc");
require_once ("visufuncs.inc");
require_once 'date_funcs.inc';
require_once 'insertx.inc';
require_once('glob.obj.copyobj2.inc');
require_once 'o.MORDER.subs.inc';
require_once 'o.S_OBJLINK.mod.inc';
require_once ('o.S_VARIO.subs.inc');
require_once 'f.namePolicy.inc';
require_once 'o.MORDER.network.inc';
require_once 'o.MORDER.subs.inc';
require_once 'o.CONCRETE_SUBST.proto.inc';

class o_CONCRETE_SUBST_add_main {
    
    function __construct(int $suc_id) {
        $this->suc_id=$suc_id;
        $this->t_short = globTabGetSHORT('CONCRETE_SUBST');
    }
    
    /**
     * 
     * @param object $sqlo
     * @param string $vario_val
     *    PRA:1234,STEP:23
     */
    function run($sqlo, string $vario_val, $order_id) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        
        $vario_arr=explode(',',$vario_val);
        if (sizeof($vario_arr)<2) {
            throw new Exception('VARIO-value invalid. EXPECT: PRA:{ID},STEP:{ID} (1)');
        }
        $pra_arr =explode(':',$vario_arr[0]);
        $step_arr=explode(':',$vario_arr[1]);
        
        $result = array();
        $result[$pra_arr[0]]=$pra_arr[1];
        $result[$step_arr[0]]=$step_arr[1];
        if ( !is_numeric($result['PRA']) or  !is_numeric($result['STEP']) ) {
            throw new Exception('VARIO-value invalid. EXPECT: PRA:{ID},STEP:{ID} (2)');
        }
        
        // get child-MAC
        // child ORDER
        $order_net_lib = new oMORDER_network($order_id);
        $child_morders = $order_net_lib->get_orders($sqlo);
        
        if (empty($child_morders)) return -2;
        
        $first_child_mord = current($child_morders);
        
        $mord_sub_lib = new oMORDER_subs($first_child_mord);
        $sucs = $mord_sub_lib->get_prod_SUCs($sqlo);
        if (empty($sucs)) return -3;
        $child_suc = current($sucs);
        
        debugOut('Detected a child '.$this->t_short.':'.$child_suc, $FUNCNAME, 1);
        
        $suc_proto_lib = new oCONCRETE_SUBST_proto($this->suc_id);
        $PRC_features=array();
        $prc_id = $suc_proto_lib->create_proto($sqlo, $PRC_features, $result['PRA']);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on PRC creation' );
            return -5;
        }
        $step_params= array(
            $result['STEP']=>array('CONCRETE_SUBST_ID'=>$child_suc)
        );
        
        $suc_proto_lib->mod_steps_by_no($sqlo, $prc_id, $step_params);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 11, 'Error on protocol STEP creation' );
            return -7;
        }
        
    }
}


class o_CONCRETE_SUBST_mord_new_XPL extends gPlugin {
	

	
	function register() {
	    $table = 'MORDER';
	    $suc_table = 'CONCRETE_SUBST';
	    $short = globTabGetSHORT($suc_table);
	    
	    $parx= $_REQUEST['parx'];
	    $mid = 0;
	    if (!empty($parx)) $mid = $parx['morder_id'];
	    
	    $title		= 'create '.$short;
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $table;
		$this->infoarr['obj_id']   = $mid;
		$this->infoarr['checkid']  = 1;
		$this->infoarr['version']     ="2022-02-26";
		$this->infoarr['version.info']='Option: copy Protocols';
		$this->infoarr['ureq.info']   = "SRD-006: Create MAC from Material order";
		

		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read', 'insert'), 'obj'=>array() )
				);
				
		
	}
	
	function attach_MORDER($sqlo, $suc_id) {
	    global $error ;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $order_id = $this->morder_id;
	    
	    $modlib = new oS_OBJLINK_mod();
	    $modlib->setObj($sqlo , 'MORDER', $order_id );
	    
	    $key = 'prod.obj';
	    $retval = $modlib->addLink($sqlo, 'CONCRETE_SUBST', $suc_id, $key);
	    if (!$retval)  {
	        $error->set( $FUNCNAME, 1, 'Could not attach '.$this->t_short.' [ID:'.$suc_id.'] to [MORDER_ID: '.$order_id.'].' );
	        return;
	    }
	}
	
// 	private function get_last_num_suc_type($sqlo, $sua_id) {
// 	    $sqlsel = 'max(CONCRETE_SUBST_ID) from CONCRETE_SUBST where ABSTRACT_SUBST_ID='.$sua_id;
// 	    $sqlo->Quesel($sqlsel);
// 	    $sqlo->ReadRow();
// 	    $suc_id = $sqlo->RowData[0];
	    
// 	    if (!$suc_id) return 0;
	    
// 	    $last_name = glob_elementDataGet( $sqlo, 'CONCRETE_SUBST', 'CONCRETE_SUBST_ID', $suc_id, 'NAME');
// 	    $last_name = trim($last_name);
	   
// 	    // search for number; from back
// 	    $numval = 0;
// 	    $startpos=strlen($last_name)-1;
// 	    for ($pos=$startpos; $pos>=0; $pos--) {
	        
// 	        $charone = substr($last_name,$pos,1);
	       
// 	        if (!is_numeric($charone) ) {
// 	            $valstr = substr($last_name,$pos+1);
// 	            if ($valstr!=NULL) {
// 	                $numval = intval($valstr);
// 	            }
// 	            break;
// 	        }
// 	    }
// 	    return $numval;
// 	}
	
	/**
	 * e.g. create a first protocol ...
	 * @param object $sqlo
	 * @param int $suc_id
	 */
	private function _SUC_post_actions($sqlo, $suc_id) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $VARIO_KEY='MORDER.add_main_compo';
	    $SUA_ID = $this->order_features['vals']['ABSTRACT_SUBST_ID'];
	    
	    // get special VARIO
	    $vario_lib = new oS_VARIO_sub('ABSTRACT_SUBST');
	    $vario_val = $vario_lib->getValByKey( $sqlo, $SUA_ID, $VARIO_KEY );
	    debugOut('(182) vario_val:'.$vario_val, $FUNCNAME, 1);
	    
	    if ($vario_val!=NULL) {
	        $order_id = $this->morder_id;
	        $suc_proto_lib = new o_CONCRETE_SUBST_add_main($suc_id);
	        $result = $suc_proto_lib->run($sqlo, $vario_val, $order_id);
	        debugOut('Result:'.$result, $FUNCNAME, 1);
	    }
	}

	/**
	 * create SUC and connect to BCBATCH
	 * INPUT:
	 * - $this->absSubstID
	 *
	 */
	function create_SUC(&$sqlo, $sqlo2, $sqlo3, $SUA_ID, $suc_id_blueprint) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $tablename  = 'CONCRETE_SUBST';
	    $order_name = $this->order_features['vals']['NAME'];
	    
	    $SUC_params = array();
	    $order_vals = $this->order_features['vals'];
	    $SUC_keys = array('ABSTRACT_SUBST_ID', "CERT_FLAG", "QUANTITY" );
	    
	    
	    $timeStamp      = time();
	    
	    $sua_id = $order_vals['ABSTRACT_SUBST_ID'];
	    $objInfoLib     = new objInfoC();
	    $sua_features = $objInfoLib->getParamsRaw ( $sqlo, 'ABSTRACT_SUBST', $sua_id );
	    $policyStr = $sua_features['vals']['NAMEPOL'];
	    
	    $suc_name = 'NEW NAME'; // temporary
	    $crea_opt = array( 'name_policy'=>array('MORDER.name'=>$order_name) );

	    $this->SUC_params = array( 'vals'=>array() );
	    foreach($SUC_keys as $key) {
	        $SUC_params['vals'][$key] = $order_vals[$key];
	    }
	    $SUC_params['vals']['SOME_DATE'] = date_unix2datestr( $timeStamp,1 ); 
	    $SUC_params['vals']['NAME']      = $suc_name;
	    
	    $new_params= $SUC_params;
	    
	    $proj_id= 0;
	    $objCreaLib  = new objCreaWiz($tablename);
	    $suc_ori = 0;
	    
	    if ($suc_id_blueprint) {
	        $suc_ori = $suc_id_blueprint;
	        // give many new parameters ...
	        $new_params = $SUC_params;
	        
	        $objInfoLib     = new objInfoC();
	        $suc_blue_features = $objInfoLib->getParamsRaw ( $sqlo, 'CONCRETE_SUBST', $suc_ori );
	        $sua_id_now = $suc_blue_features['vals']['ABSTRACT_SUBST_ID'];
	        $new_params['vals']['ABSTRACT_SUBST_ID'] = $sua_id_now;
	        debugOut('Make a copy from a selected blueprint MAC-ID:'.$suc_ori, $FUNCNAME, 1);
	    }
	    
	    if ($this->mord_last_suc) {
	        $suc_ori = $this->mord_last_suc;
	        // just give a new name, nothing else
	        $new_params= array( 'vals'=>array() );
	        $new_params['vals']['NAME'] = $suc_name;
	        debugOut('Make a copy from the last MAC in this order MAC-ID:'.$suc_ori, $FUNCNAME, 1);
	    }
	    
	        // $params_full = array('vals'=>$SUC_params);
	    debugOut('SUC-Parameters: '.print_r($new_params,1), $FUNCNAME, 1);

	    
	    if ($suc_ori) {
	        $deep_copy = array();
	        $suc_id_NEW = $objCreaLib->objCreate($sqlo, $sqlo2, $sqlo3, $suc_ori, $new_params, $deep_copy, $crea_opt);
	    } else {
	        $suc_id_NEW = $objCreaLib->objCreateFrTmpl( $sqlo, $sqlo2, $sua_id, $proj_id, $new_params, $crea_opt ); 
	    }


	    if (!$suc_id_NEW) {
	        $error->set($FUNCNAME, 3, 'Could not SUC.');
	        return;
	    }
	    
	    if ($suc_ori and $this->parx['prc_copy']) {
	        $cp_lib = new oCONCRETE_SUBST_proto_copy($suc_ori, $suc_id_NEW);
	        $cp_lib->copy_PREP_protocols($sqlo, $sqlo2);
	    }
	    
	    $this->attach_MORDER($sqlo, $suc_id_NEW);
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 10, 'Error on MORDER attachment.' );
	        return;
	    }
	    
	    if (!$suc_ori) {
	        $this->_SUC_post_actions($sqlo, $suc_id_NEW);
	        if ($error->Got(READONLY))  {
	            $error->set( $FUNCNAME, 20, 'Error on SUC_post_actions.' );
	            return;
	        }
	    }
	    
	    
	    return $suc_id_NEW;
	    
	}
	
	function form1($suc_cnt) {
	    
	    $initarr   = array();
	    $initarr["title"]       = 'Create MAC'; // title of form
	    $initarr["submittitle"] = "Create"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    $initarr["dblink"]=1;
	    
	    $hiddenarr = array(); // hidden form variables
	    foreach($this->parx as $key=>$val) {
	        $hiddenarr['parx['.$key.']'] = $val;
	    }
	    
	    // init form, uses class formc
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
	    
	   
	    
	    if ($suc_cnt) {
	        $txt = 'Already '.$suc_cnt.' '.tablename_nice2('CONCRETE_SUBST') .' exist(s).';
	        
	        $fieldx = array ( // form-field definition
	            "title" => "Confirm extra creation",
	            "name"  => "pos_extra",
	            "object"=> "checkbox",
	            "val"   => 0,
	            "notes" => $txt
	        );
	        $formobj->fieldOut( $fieldx );
	        
	        if ( $this->old_has_protos ) {
	            $fieldx = array ( // form-field definition
	                "title"   => "Copy Protocols?",
	                "name"  => "prc_copy",
	                "object"=> "checkbox",
	                "val"   => 0,
	                "notes" => 'Copy Prep-Protocols of last '.$this->t_short.'?'
	            );
	            $formobj->fieldOut( $fieldx );    
	        }
	        
	    } else {
	        
	        
	        $fieldx = array ( // form-field definition
	            "title"   => "(OPTIONAL) Copy from ".$this->t_short,
	            "name"  => "suc_ori",
	            "optional" => 1,
	            "object"=> "dblink",
	            "inits" => array( "table"=>"CONCRETE_SUBST", "objname"=>'---', "pos" =>"0", "projlink"=> 1),
	            "val"    => 0,
	           
	        );
	        $formobj->fieldOut( $fieldx );
	        
	    
            $fieldx = array ( // form-field definition
                "title"   => "Copy Protocols?",
                "name"  => "prc_copy",
                "object"=> "checkbox",
                "val"   => 0,
                "notes" => 'Copy Prep-Protocols of Source '.$this->t_short.'?'
            );
            $formobj->fieldOut( $fieldx );
	        
	    }

	    $formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	

	function startMain() {
		global $error;
		
		$FUNCNAME ='startMain';
		$tablename='CONCRETE_SUBST';
		$this->t_short = globTabGetSHORT($tablename);
		
		$this->parx   = $_REQUEST['parx'];
		$go = $_REQUEST['go'];
		$this->morder_id   = $this->parx ['morder_id'];
		$sqlo  = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		$sqlo3 = &$this->_plug_regSql3();
		
		
		if (!$this->morder_id)  {
			$error->set( $FUNCNAME, 1, 'input missing: order_id.' );
			return;
		}
		
		$objInfoLib     = new objInfoC();
		$this->order_features = $objInfoLib->getParamsRaw ( $sqlo, 'MORDER',  $this->morder_id);
		$this->mord_last_suc  = 0;
		
		$order_lib = new oMORDER_subs($this->morder_id);
		$has_produced_suc = $order_lib->has_produced_SUC($sqlo);
		if ($has_produced_suc) {
		    
		    $old_suc_ids         = $order_lib->get_prod_SUCs($sqlo);
		    $this->mord_last_suc = end($old_suc_ids);
		    reset($old_suc_ids);
		    
		    $this->old_has_protos = 0;
		    $prc_lib = new oCONCRETE_SUBST_proto($this->mord_last_suc);
		    $prcs = $prc_lib->get_c_protos($sqlo);
		    if (sizeof($prcs)) $this->old_has_protos = 1;
		    
		    
		    $next_pos_expect = sizeof($old_suc_ids) + 1;
		    if ($this->parx['pos'] != $next_pos_expect) {
		        $error->set( $FUNCNAME, 2, 'order (ID:'.$this->morder_id.') expects parameter POS= '.$next_pos_expect.'.' );
		        return;
		    }
		    
		    if ($go and !$this->parx['pos_extra']) {
		        $error->set( $FUNCNAME, 3, 'Please confirm, that you want to create an extra MAC!' );
		        return;
		    }
		    
		    
		}
		
		if (!$go) {
		    $num_sucs = 0;
		    if (!empty($old_suc_ids)) $num_sucs = sizeof($old_suc_ids);
		    $this->form1($num_sucs);
		    return;
		}

		
		$SUA_ID = $this->order_features['vals']['ABSTRACT_SUBST_ID'];
		if (!$SUA_ID)  {
		    $error->set( $FUNCNAME, 3, 'order (ID:'.$this->morder_id.') has no substance (abstract).' );
		    return;
		}
		
		$suc_id = $this->create_SUC($sqlo, $sqlo2, $sqlo3, $SUA_ID, $this->parx['suc_ori']);
		if ($error->Got(READONLY))  {
		    $error->set( $FUNCNAME, 4, 'Error on '.$this->t_short.' creation.' );
		    return;
		}
		
	
		$objLinkLib = new fObjViewC();
		$html_tmp = $objLinkLib->bo_display( $sqlo, $tablename, $suc_id );
		echo 'Produced '.$this->t_short.': '.$html_tmp.'<br>';
		
		$this->_html->pageForward('edit.tmpl.php?t=MORDER&id='.$this->morder_id, 'forward to material order.');
		
	}
	
}