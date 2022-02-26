<?php
/**
 * - manage modification of AS_HAS_PR and ABSTRACT_SUBST: 
 *    
 * @package o.ABSTRACT_SUBST.hasProtMod.inc
 * @swreq SREQ:0002034: o.ABSTRACT_SUBST > manage AS_HAS_PR for one substance 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $_REQUEST['id'] id of ABSTRACT_SUBST 
 * @param $action
 * 	'add'    need $parx: ABSTRACT_PROTO_ID, XORDER
 *    need oparam $opt[crea_pra]
 *  'delete' need $parx['STEP_ID']
 *  'move'   move one step
 *  'copy'   need $parx['XORDER'] create a copy abstract_proto and connect to SUA
 *  'ord.move.many' move all steps in the protocol log need $pras
 *     pras = array( STEP_ID => int (XORDER) )
 * @param parx
 * 	[XORDER]   max val: 9997 !!!
 *  [STEP_ID]
 *  [ABSTRACT_PROTO_ID]
 *  TYPE: 
 *  1: PREP
 *  2: QC
 * @param $opt
 *   'crea_pra' : 
 *      0 : undefined
 *      1 : create a complete NEW protocol, no need of parx[ABSTRACT_PROTO_ID]
 *      2 : select EXSITING
 *   'add_do'   : 0,1 : need this option, if an empty protocol already exists before ...
 */

require_once ('f.msgboxes.inc'); 
require_once ("f.assocUpdate.inc");
require_once ("insertx.inc");
require_once ("glob.obj.update.inc");
require_once 'glob.obj.copyobj1.inc';
require_once 'o.H_PROTO_KIND.subs.inc';
require_once 'o.ABSTRACT_SUBST.qcprot.inc';
require_once 'o.ABSTRACT_SUBST.defs.inc';
require_once ("o.AS_HAS_PR.subs.inc");
require_once ("o.CS_HAS_PR.subs.inc");

require_once 'o.ABSTRACT_PROTO.subs.inc';
require_once 'o.ABSTRACT_PROTO.stepMod.inc';
require_once 'o.ABSTRACT_PROTO.stepx.inc';
require_once ("o.ACCEPT_PROT.subs.inc");

class oABSTRACT_PROTO_copy {
	
	/**
	 * produce new name
	 * 
	 * @param  $sqlo
	 * @param  $protoFeats
	 * @param  $praid - old PROTO
	 * @return string $name
	 */
	private function _getAprotoName(&$sqlo, $protoFeats, $praid) {
		$name = $protoFeats['NAME'];
		$lenx = strlen($name);
		$base = $name;
		$newvers = 2;
		
		$posVers = strrpos($name, ' v');
		
		if ( $posVers>0 and ($lenx-$posVers)<=4 ) {
			// already has version-string
			$base = substr($name,0, $posVers);
			
			// get HIGHEST version-number
			$sqlsel = 'count(1) from ABSTRACT_PROTO where NAME like '.$sqlo->addquotes($base.'%');
			$sqlo->Quesel($sqlsel);
			$sqlo->ReadRow();
			$cnt = $sqlo->RowData[0];
			
			$newvers = $cnt + 1;
		}
		
		return $base.' v'.$newvers;
	}
	
	/**
	 * copy acceptance proto
	 * @param  $sqlo
	 * @param  $sqlo2
	 * @param  $praid
	 */
	private function manageAcc( &$sqlo, &$sqlo2, $src_praid, $dst_praid ) {
		
		$acceptLib = new oACCEPT_PROT_subs($sqlo);
		$accOpt    = array( 'onlyReleased'=>1 );
		$accProtoArr = $acceptLib->getAcceptByAbstract($sqlo, $src_praid, $accOpt);
		
		if (!sizeof($accProtoArr)) {
			return; // no problem ...
		}
		
		$accid   = current($accProtoArr);
		$copyLib = new gObjCopyS();
		$cpopt   = array('attach'=>1);
		$newAccID = $copyLib->copyOne( $sqlo, $sqlo2, 'ACCEPT_PROT', $accid, $cpopt );
		
		if (!$newAccID) {
			return;
		}
		echo '- new acceptance proto created.'."<br />\n";
		
		$args      = array( 'vals'=>array('ABSTRACT_PROTO_ID'=>$dst_praid) );
	 	$UpdateLib = new globObjUpdate();
		$UpdateLib->update_meta( $sqlo, 'ACCEPT_PROT', $newAccID, $args );
		
		// update steps with right ABSTRACT_PROTO_ID
		$assoclib = new  fAssocUpdate();
		$assoclib->setObj( $sqlo, 'ACCEPT_PROT_STEP', $newAccID );
		$argu     = array('ABSTRACT_PROTO_ID'=>$dst_praid);
		$whereAdd = 'ACCEPT_PROT_ID='.$newAccID; // not need, but for security ....
		$assoclib->updateMore( $sqlo, $argu, $whereAdd);
	}
	
	/**
	 * 
	 * Enter description here ...
	 * @param  $sqlo
	 * @param  $sqlo2
	 * @param  $praid
	 * @param  $xtype : 1,2 (PREP or QC)
	 */
	public function copyPra(&$sqlo, &$sqlo2, $praid, $xtype) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$tablename = 'ABSTRACT_PROTO';
		
		$protoFeats = glob_elemDataGet3($sqlo, $tablename, array('ABSTRACT_PROTO_ID'=>$praid), array('NAME'));
		$newName = $this->_getAprotoName($sqlo, $protoFeats, $praid);

		
		// copy proto; name = name 'v2'
		$copyLib = new gObjCopyS();
		$cpopt   = array('attach'=>1);
		$newPraid = $copyLib->copyOne( $sqlo, $sqlo2, $tablename, $praid, $cpopt );
		
		if (!$newPraid) {
			return;
		}

		// update name
		if ($newName!=NULL) {
			$args      = array( 'vals'=>array('NAME'=>$newName) );
	 		$UpdateLib = new globObjUpdate();
			$UpdateLib->update_meta( $sqlo, $tablename, $newPraid, $args );
		}
		
		// get acceptance-proto
		$this->manageAcc( $sqlo, $sqlo2, $praid, $newPraid );
		
		return $newPraid;
	}
}

/**
 * manage AS_HAS_PR modification actions
 * @author steffen
 *
 */
class oAS_HAS_PR_modX {
	
	var $XORDER_LIMIT=9998;      // limit, reserved for final protocols
	var $XORDER_LIMIT_ALL=10000; // limit not allowed
	
	
	
	function __construct() {
		$this->objid=NULL;
		$this->assoclib = new  fAssocUpdate();
		$this->AS_HAS_PR_lib = new oAS_HAS_PR_subs();
	}
	
	/**
	 * set mother object
	 */
	function setObj(&$sqlo, $objid) {
		$this->objid   = $objid;
		$this->assoclib->setObj( $sqlo, 'AS_HAS_PR', $objid );
		$this->AS_HAS_PR_lib->setObj($objid);
		$this->abs_proto_arr = $this->AS_HAS_PR_lib->getProtoLog($sqlo);
	}
	
	function get_old_proto_arr() {
	    return $this->abs_proto_arr;
	}
	
	function _getMaxStepID(&$sqlo) {
		$id = $this->objid;
		$sqls="max(STEP_ID), max(XORDER) from AS_HAS_PR where ABSTRACT_SUBST_ID=".$id;
		$sqlo->Quesel($sqls);
		$sqlo->ReadRow();
		$proto_id_max = $sqlo->RowData[0];
		$order_max = $sqlo->RowData[1];
		
		$proto_id_new = $proto_id_max+1;
		$guessOrder   = intval($order_max)+10;
		return array('step'=>$proto_id_new, 'maxorder'=>$order_max, 'guessOrder'=>$guessOrder);
	}
	
	/**
	 * check protocol
	 * @param $sqlo
	 * @param $apid
	 * @param $xtype
	 * @return  array('ok'=>1, 'errtxt'=>NULL) : ok
	 */
	function _abstrProtoTest(&$sqlo, $apid, $xtype, $xorder ) {
		$answer   = array('ok'=>1, 'errtxt'=>NULL);
		
		//$aprotoFeats = glob_elemDataGet3( $sqlo, 'ABSTRACT_PROTO', array('ABSTRACT_PROTO_ID'=>$apid), 
		//	array('H_PROTO_KIND_ID', 'NAME') );
			
		// check order
		if (!empty($this->abs_proto_arr)) {
			foreach($this->abs_proto_arr as $valarr) {
				if ($valarr['or']==$xorder) {
					$answer = array('ok'=>-2, 'errtxt'=>'Order-No '.$xorder.' already exists.');
					return $answer;
				}
			}
		}
		
		$a_feats = glob_elemDataGet3( $sqlo, 'ABSTRACT_PROTO', array('ABSTRACT_PROTO_ID'=>$apid), array('H_PROTO_KIND_ID') );
		
		if ( $xtype== oABSTRACT_SUBST_DEFS::PR_TYPE_PREP ) {
		    // check, if protocol is a valid QC-protocol ...
		    $qc_type_name = oH_PROTO_KIND_subs::TYPE_QC;
		    $qc_type_id = oH_PROTO_KIND_subs::name2ID($sqlo, $qc_type_name);
		    
		    if ($a_feats['H_PROTO_KIND_ID']==$qc_type_id) {
		        $answer = array('ok'=>-4, 'errtxt'=>'You selected PREP type, but your protocol is type QC!');
		        return $answer;
		    }
		}
		
		
		if ( $xtype== oABSTRACT_SUBST_DEFS::PR_TYPE_QC ) {
			// check, if protocol is a valid QC-protocol ...
			$abQcProtLib = new oAbsSubstQCprot($sqlo);
			$answerLib   = $abQcProtLib->isValid($sqlo, $apid);
			if ($answerLib[0]<=0) {
				$answer = array('ok'=>-3, 'errtxt'=>'QC-Protocol-Test: '.$answerLib[1]);
				return $answer;
			}
			
		}
		
		return $answer;
	}
	
	function _check_params(&$sqlo, &$parx) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $newStepInfo = $this->_getMaxStepID($sqlo);
	    $parx['STEP_ID'] = $newStepInfo['step'];
	    
	    $new_apid = $parx['ABSTRACT_PROTO_ID'];
	    
	    if (!$newStepInfo['step']) {
	        $error->set( $FUNCNAME, 1, 'InternalError: No step_id calculated.' );
	        return;
	    }
	    
	    if ($parx['XORDER']==NULL) {
	        $parx['XORDER'] = $newStepInfo['guessOrder'];
	    }
	    
	    // XORDER>=9998: reserved for final protocols !
	    if ($parx['XORDER']>=$this->XORDER_LIMIT_ALL) {
	        $error->set( $FUNCNAME, 2, 'Order_num too big; Max Order_num: '.($this->XORDER_LIMIT_ALL-1) );
	        return;
	    }
	    
	    $answer = $this->_abstrProtoTest($sqlo, $new_apid, $parx['TYPE'], $parx['XORDER']);
	    if ($answer['ok']<1) {
	        $error->set( $FUNCNAME, 4, $answer['errtxt'] );
	        return;
	    }
	}
	
	/**
	 * add object link
	 * - test, if object already linked => then do nothing
	 * - check for QC-protocol KIND: 'QC test'
	 * @return boolean added ? 
	 */
	function addProto(&$sqlo, $parx) {
		
		$params = array(
		    'STEP_ID' => $parx['STEP_ID'],
			'ABSTRACT_PROTO_ID'=> $parx['ABSTRACT_PROTO_ID'],
			'XORDER'  => $parx['XORDER'],
			'TYPE'    => $parx['TYPE']
		 );
		
		$this->assoclib->insert( $sqlo, $params );
		
		return 1;
	}
	
	/**
	 * remove protocol from substance
	 * @param  $sqlo
	 * @param  int  $pos : pos of protocol
	 */
	function removeProto(&$sqlo, $step_id) {
		
		
		$idarr=array('STEP_ID'=>$step_id);
		$this->assoclib->delOneRow( $sqlo, $idarr );
		
	}
	
	function update(&$sqlo, $params) {
		$idarr=array('STEP_ID'=>$params['STEP_ID']);
		
		unset($params['STEP_ID']);
		
		$this->assoclib->update( $sqlo, $params, $idarr );
	}
	
	/**
	 * update step
	 * @param  $sqlo
	 * @param  array $params
	 * 	'STEP_ID'=>
		'ABSTRACT_PROTO_ID'
		'XORDER' 
		'TYPE' 
	 */
	private function _updateStep(&$sqlo, $params) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		if (!$params['ABSTRACT_PROTO_ID']) {
				$error->set( $FUNCNAME, 1, 'ABSTRACT_PROTO_ID missing' );
				return;
		}
	
		$new_apid = $params['ABSTRACT_PROTO_ID'];
		
		
		if (!$params['STEP_ID']) {
			$error->set( $FUNCNAME, 1, 'STEP_ID missing' );
			return;
		}
		
		$idarr = array('STEP_ID'=>$params['STEP_ID']);
		
		unset($params['STEP_ID']);
		$this->assoclib->update( $sqlo, $params, $idarr );
		
	}
	
	
	
	/**
	 * copy proto
	 * @param $sqlo
	 * @param $parx
	 */
	function copyProto( &$sqlo, &$sqlo2, $parx ) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$tablename = 'ABSTRACT_PROTO';
		
		$xorder = $parx['XORDER'];
		$praid = 0;
		
		// $this->abs_proto_arr
		reset ($this->abs_proto_arr);
		foreach( $this->abs_proto_arr as $key=>$valarr) {
			if ($xorder== $valarr['or']) {
				$praid = $valarr['ap'];
				break;
			}
		}
		
		if (!$praid) {
			echo 'no protocol found on XORDER.';
			return;
		}
		
		$praSubLib  = new oABSTRACT_PROTO_subs();
		$numUsedArr = $praSubLib->isUsed($sqlo, $praid, $this->objid);
		if ( $numUsedArr['PRC']+$numUsedArr['SUA'] == 0 ) {
			$error->set( $FUNCNAME, 1, 'Protocol is not used by other objects. Copy denied.' );
			return;
		}
		
		$praCopyLib = new oABSTRACT_PROTO_copy();
		$newPraid = $praCopyLib->copyPra($sqlo, $sqlo2, $praid, $valarr['ty']);
		
		if (!$newPraid) {
			return;
		}
		
		// attach proto to SUA
		$params = array(
			'STEP_ID'=>$valarr['st'],
			'ABSTRACT_PROTO_ID'=>$newPraid,
			'XORDER' =>$valarr['or'],
			'TYPE'   =>$valarr['ty']
			);
		$this->_updateStep($sqlo, $params);

		
	}

	function close(&$sqlo) {
		$this->assoclib->close($sqlo);
	}
 
}

class oAS_HAS_PR_move_many {
    
    function __construct(&$sqlo, $objid) {
        
        $this->objid=NULL;
        $this->SUA_assoc = new  fAssocUpdate();
        $this->SUA_assoc->setObj( $sqlo, 'AS_HAS_PR', $objid );
        
        $this->SUC_assoc = new  fAssocUpdate();
        
        
        $this->AS_HAS_PR_lib = new oAS_HAS_PR_subs();
        
        $this->objid   = $objid;
        
        $this->AS_HAS_PR_lib->setObj($objid);
        // array('st'=>$step_id, 'ap'=>$a_proto_id, 'or'=>$XORDER, 'ty'=>$type)
        $this->abs_proto_arr = $this->AS_HAS_PR_lib->getProtoLog($sqlo);
        
        // create array PRA_id => STEP_ID
        $this->pra_id_2_step_id=array();
        foreach($this->abs_proto_arr as $row) {  
            $step_id = $row['st'];
            $pra_id  = $row['ap'];
            $this->pra_id_2_step_id[$pra_id] = $step_id;
        }
        
        $this->CS_HAS_PR_lib = new oCS_HAS_PR_subs();
    }
    
    /**
     * for one SUC:  transform STEP_NO : old => new
     * @param object $sqlo
     * @param int $suc_id
     */
    private function _one_suc($sqlo, $suc_id) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $up_opt = array('noAccCheck'=>1 );
        
        $this->SUC_assoc->setObj( $sqlo, 'CS_HAS_PR', $suc_id, $up_opt );
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on MAC init' );
            return;
        }
        $this->SUC_assoc->setNoTouch(1);
        // $this->SUC_assoc->set_log_level(fAssocUpdate::MASS_LOG_POS_RANGE);
        
        //  array('cp'=>ID of protocol, 'ap' => ID of ABSTRACT_PROTO, 'or'=>order number, 'ty'=>1,2)
        $this->CS_HAS_PR_lib->setObj($suc_id);
        $c_proto_log = $this->CS_HAS_PR_lib->getProtoLog($sqlo);
        
        foreach($c_proto_log as $row) {
            
            $cp_id = $row['cp'];
            $ap_id = $row['ap'];

            $xorder_old = $row['or'];
            $xorder_new = $this->STEP_NO_old2new[$xorder_old];
            
            // FALLBACK:  new number not found from OLD number ...
            if ( !$xorder_new ) {
                
                $xorder_new = $this->PRA_ID_2_new_ord[$ap_id];
                
                if(!$xorder_new) {
                    // take the old one ...
                    $xorder_new = $xorder_old; 
                }
                
            } else {
                // check versus PRA
                // but check, if TWO PRAs exist in the defined SUA-log ....
                $xorder_by_pra = $this->PRA_ID_2_new_ord[$ap_id];
                if (!empty($xorder_by_pra)) {
                    if (is_array($xorder_by_pra)) {
                        // @todo: do something ...
                    } else {
                        if ($xorder_new!=$xorder_by_pra) {
                            $xorder_new = $xorder_by_pra;
                        }
                    }
                }
            }

            $idarr=array('CONCRETE_PROTO_ID' =>$cp_id);
            $argu =array('STEP_NO' =>$xorder_new );
            
            debugOut('suc_id:'.$suc_id.' cp_id: '.$cp_id.' ap_id:'.$ap_id.' xorder_old:'.$xorder_old.' xorder_new:'.$xorder_new, $FUNCNAME, 2);
            $this->SUC_assoc->update( $sqlo, $argu, $idarr );
        }
        $this->SUC_assoc->close($sqlo);
    }
    
    /**
     * error-numbers >= 100 : show the input-form again
     * @param object $sqlo
     * @param object $sqlo2
     * @param array $pra_params
     */
    function update(object $sqlo, object $sqlo2, array $pra_params) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $sua_id = $this->objid;
        
        $abs_proto_arr = &$this->abs_proto_arr;
        $this->STEP_NO_old2new =array();
        $this->PRA_ID_2_new_ord=array();
        
        // temp array ...
        $new_order_nums=array();
        
        
        // check user input
        
        $has_double_num=0;
        foreach($abs_proto_arr as $row) {
            
            $pra_id     = $row['ap'];
            $step_id    = $row['st'];
            $xorder_old = $row['or'];
            $xorder_new = trim($pra_params[$step_id]);
            
            if (!is_numeric($xorder_new)) {
                $error->set( $FUNCNAME, 100, 'Input "'.$xorder_new.'" is not a number.' );
                return;
            }
            if ($xorder_new<=0) {
                $error->set( $FUNCNAME, 102, 'Input "'.$xorder_new.'" must be a positive number.' );
                return;
            }
            
            if (in_array($xorder_new, $new_order_nums)) {
                $has_double_num=1;
                break;
            }
            $new_order_nums[] = $xorder_new;
  
            $this->STEP_NO_old2new[$xorder_old]= $xorder_new;
            if (!empty($this->PRA_ID_2_new_ord[$pra_id])) {
                $tmp_ord = $this->PRA_ID_2_new_ord[$pra_id] ;
                $this->PRA_ID_2_new_ord[$pra_id]   = array($tmp_ord);
                $this->PRA_ID_2_new_ord[$pra_id][] = $xorder_new;
            } else {
                $this->PRA_ID_2_new_ord[$pra_id]   = $xorder_new;
            }
        }
        
        if ($has_double_num) {
            // do NOT change the error number !
            $error->set( $FUNCNAME, 104, 'Please input UNIQUE numbers!' );
            return;
        }
        
        foreach($abs_proto_arr as $row) {
            
            $pra_id     = $row['ap'];
            $step_id    = $row['st'];
            $xorder_old = $row['or'];
            $xorder_new = $pra_params[$step_id];

            $idarr=array('STEP_ID'=>$step_id);
            $argu =array('XORDER'=>$xorder_new);
            debugOut('STEP_ID: '.$step_id.' pra:'.$pra_id.' xorder_old:'.$xorder_old.' xorder_new:'.$xorder_new, $FUNCNAME, 2);
            $this->SUA_assoc->update( $sqlo, $argu, $idarr );
        }
        
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on SUA protocol order update.' );
            return;
        }
        
        echo ' ... abstract steps updated<br>';
        
        // change the CONCRETE_SUBST
        $this->pra_params = $pra_params;
        
        $error_cache = array();
        
        $sqlsel = "CONCRETE_SUBST_ID from CONCRETE_SUBST where ABSTRACT_SUBST_ID=".$sua_id . ' order by CONCRETE_SUBST_ID';
        $sqlo2->Quesel($sqlsel);
        while ( $sqlo2->ReadRow() ) {
            
            $suc_id = $sqlo2->RowData[0];
            $this->_one_suc($sqlo, $suc_id);
            if ($error->Got(READONLY))  {
                $errLast   = $error->getLast();
                $error_txt = $errLast->text;
                // $error_id  = $errLast->id;
                $error->reset();
                
                $error_cache[] = $suc_id;
            }
        }
        
        echo ' ... concrete steps updated<br>';
        
        if (!empty($error_cache)) {
            $error->set( $FUNCNAME, 5, 'Last error on MAC:'.$suc_id. ' ERR:'.$error_txt );
            $error->set( $FUNCNAME, 6, 'Error on update of MACs: '.implode(", ",$error_cache) );
            return;
        }
        
    }
}


class o_ABSTRACT_SUBST_hasProtMod_XPL extends gPlugin {

	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'ABSTRACT_SUBST';
		$title		= 'Manage protocol log';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read','write'), 'obj'=>array('write') )
				);	
	}
	
	private function del_form(&$sqlo, $STEP_ID) {
		
		$initarr   = NULL;
		$initarr["title"]       = "Remove protocol from substance?"; // title of form
		$initarr["submittitle"] = "Remove!"; // title of submit button
		$initarr["tabwidth"]    = "AUTO";   // table-width: AUTO

		$hiddenarr = NULL; // hidden form variables
		$hiddenarr["id"]     = $this->objid; // is a required input parameter
		$hiddenarr["action"] = 'delete';
		$hiddenarr["parx[STEP_ID]"]    = $STEP_ID;

		// init form, uses class formc
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);

		$formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	private function copy_form(&$sqlo, $parx) {
		
		$initarr   = NULL;
		$initarr["title"]       = "Create copy of protocol?"; // title of form
		$initarr["submittitle"] = "Copy!"; // title of submit button
		$initarr["tabwidth"]    = "AUTO";   // table-width: AUTO

		$hiddenarr = NULL; // hidden form variables
		$hiddenarr["id"]     = $this->objid; // is a required input parameter
		$hiddenarr["action"] = 'copy';
		$hiddenarr["parx[XORDER]"]    = $parx['XORDER'];

		// init form, uses class formc
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);

		$formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	private function ord_move_many_form($sqlo, $pra_params=array() ) {
	    
	    $AS_HAS_PR_lib = new oAS_HAS_PR_subs();
	    $AS_HAS_PR_lib->setObj($this->objid);
	    
	    // array('st'=>$step_id, 'ap'=>$a_proto_id, 'or'=>$XORDER, 'ty'=>$type)
	    $abs_proto_arr = $AS_HAS_PR_lib->getProtoLog($sqlo);
	    
	    
	    $initarr   = NULL;
	    $initarr["title"]       = "Change order"; // title of form
	    $initarr["submittitle"] = "Save"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    
	    $hiddenarr = NULL; // hidden form variables
	    $hiddenarr["id"]     = $this->objid; // is a required input parameter
	    $hiddenarr["action"] = 'ord.move.many';
	   
	    
	    // init form, uses class formc
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
	    
	    foreach($abs_proto_arr as $row) {
	        
	        $pra_id  = $row['ap'];
	        $step_id = $row['st'];
	        $xorder  = $row['or'];
	        $pra_nice = obj_nice_name ( $sqlo, 'ABSTRACT_PROTO', $pra_id );
	        
	        if (!empty($pra_params)) {
	            $xorder  = $pra_params[$step_id];
	        }
	        
	        
	        $fieldx = array ( 
	            "title" => $pra_nice,
	            'namex' => TRUE ,  
	            "name"  => 'pras['.$step_id.']',
	            "object" => "text",
	            "val"    => $xorder,
	            "notes" => "Order value"
	        );
	        $formobj->fieldOut( $fieldx ); // output the form-field
	        
	    }
	    
	    
	    $formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	private function create_form(&$sqlo, $parx) {
	    
	    cMsgbox::showBox("warning", "The material already has an empty protocol."); 
	    echo '<br>';
	    
	    $initarr   = NULL;
	    $initarr["title"]       = "Create new protocol anyway?"; // title of form
	    $initarr["submittitle"] = "Create"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   
	    
	    $hiddenarr = NULL; // hidden form variables
	    $hiddenarr["id"]     = $this->objid; // is a required input parameter
	    $hiddenarr["action"] = 'add';
	    $hiddenarr["opt[crea_pra]"] = 1;
	    $hiddenarr["opt[add_do]"]   = 1;
	    foreach($parx as $key=>$val) {
	        $hiddenarr['parx['.$key.']']=$val;
	    }
	    
	    // init form, uses class formc
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
	    
	    $formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	private static function _qc_proto_steps($sqlo, $pra_id) {
	    $pra_lib = new oAbsProtoStepMod();
	    $pra_lib->init($sqlo, $pra_id);
	    
	    // $sample_id = glob_elementDataGet( $sqlo, 'ABSTRACT_SUBST', 'NAME', 'DUMMY', 'ABSTRACT_SUBST_ID');
	    
	    $steparr = array(
	       //  1 => array('NAME'=>'QC sample', 'ABSTRACT_SUBST_ID'=>$sample_id, 'MAIN_STEP_NR'=>1, 'IS_SAMPLE'=>1),
	        1 => array('NAME'=>'STATUS', 'MAIN_STEP_NR'=>2,'NOTES'=>'final QC decision: 0 or 1')
	        
	    );
	    $pra_lib->insert_steps($sqlo, $steparr);
	}
	
	/**
	 * create EMPTY protocol
	 * @param object $sqlo
	 * @param int $type
	 * 1
	 * 2
	 * @return number
	 */
	private function _create_empty_proto($sqlo, $type) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $kind_name_arr= array(
	        1=> array('kind'=>'Substance preparation', 'name'=>'Preparation'),
	        2=> array('kind'=>'QC test',               'name'=>'QC test')
	    );
	    
	    $kind_name = $kind_name_arr[$type]['kind'];
	    $kind_id   = glob_elementDataGet( $sqlo, 'H_PROTO_KIND', 'NAME', $kind_name, 'H_PROTO_KIND_ID');
	    if (!$kind_id) {
	        $error->set( $FUNCNAME, 1, 'No protocol kind "'.$kind_name.'" found.' );
	        return;
	    }
	    $sua_name = obj_nice_name ( $sqlo, 'ABSTRACT_SUBST', $this->objid );
	    
	    $insertlib = new insertC();
	    
	    $prot_name = $sua_name.' : '. $kind_name_arr[$type]['name'];
	    $args= array( "vals"=>array(
	        'NAME'  => $prot_name,
	        'H_PROTO_KIND_ID'=>$kind_id
	    ) );
	    
	    $pra_id = $insertlib->new_meta($sqlo, 'ABSTRACT_PROTO', $args);
	    if ($pra_id and $type==2) {
	        $this->_qc_proto_steps($sqlo, $pra_id);
	    }
	    
	    
	    return $pra_id;
	}

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$MAXORD=9999;
		$this->objid   = $_REQUEST['id'];
		$action = $_REQUEST['action'];
		$parx   = $_REQUEST['parx'];
		$go     = $_REQUEST['go'];
		$ext_options  = $_REQUEST['opt'];
		
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		$modLib = new oAS_HAS_PR_modX();
		$modLib->setObj($sqlo, $this->objid);
		
		// $this->_html->line_out('Action: '.$action);
		
		switch ($action) {
			case 'add':
			    $addtxt='';
			    $crea_pra_option = intval($ext_options['crea_pra']);
			    if ($crea_pra_option==2) {
			        $addtxt = 'Protocol-ID:'.$parx['ABSTRACT_PROTO_ID'];
			    }
			    $this->_html->line_out('Add: Protocol '.$addtxt.' Order:'.$parx['XORDER'].' creation-modus:'.$crea_pra_option);
			    $this->_html->line_out('');
				if (!$crea_pra_option) {
				    $this->htmlFoot('ERROR','Input missing "Creation modus".');
				}
				
				if ( $crea_pra_option==1 ) {
				    // create new
				} else {
				    // == 2
    				if (!$parx['ABSTRACT_PROTO_ID']) {
    					$this->htmlFoot('ERROR','Missing ABSTRACT_PROTO_ID.');
    				}
				}
				
				if ($parx['TYPE']==NULL) {
				    $this->htmlFoot('ERROR','Missing TYPE.');
				}
				if ($parx['XORDER']>$MAXORD) {
					$this->htmlFoot('ERROR','XORDER must be smaller than '.$MAXORD);
				}
				
				if ( $crea_pra_option==1 ) {
				    // create new PROTO
				    
				    // check 
				    if (!$ext_options['add_do']) {
    				    $abs_proto_arr = $modLib->get_old_proto_arr();
    				    if (!empty($abs_proto_arr)) {
    				        // get last: array('st'=>$step_id, 'ap'=>$a_proto_id, 'or'=>$XORDER, 'ty'=>$type)
    				        $last_elem = end($abs_proto_arr);
    				        $last_pra  = $last_elem['ap'];
    				        $pra_lib = new oABSTRACT_PROTO_stepx($last_pra);
    				        if (!$pra_lib->step_cnt($sqlo)) {
    				            // show warning
    				            $this->create_form($sqlo, $parx);
    				            return;
    				        }
    				    }
				    }
				    
				    $parx['ABSTRACT_PROTO_ID'] = $this->_create_empty_proto($sqlo, $parx['TYPE'] ); // 'Substance preparation'
				    if ($error->Got(READONLY))  {
				        $error->set( $FUNCNAME, 1, 'Error on protocol creation.' );
				        return;
				    }
				} 
				
				$modLib->_check_params($sqlo, $parx);
				if ($error->Got(READONLY))  {
				    $error->set( $FUNCNAME, 1, 'Error on parameter check.' );
				    return;
				}
				$modLib->addProto($sqlo, $parx);
				break;
				
			case 'copy':
				
				$this->_html->line_out('Create a copy of protocol (abstract) xorder:'.$parx['XORDER']. 
					'. If a released "acceptance protocol" exists, than will also be copied.' );
				$this->_html->line_out('');
				
				if (!$parx['XORDER']) {
					$this->htmlFoot('ERROR','Missing XORDER.');
				}
				
				if (!$go) {
					$this->copy_form($sqlo, $parx);
					return;
				}
				
				$modLib->copyProto($sqlo, $sqlo2, $parx);
				
				
				break;
			case 'delete':
				$this->_html->line_out('Delete: one step.');
				if (!$parx['STEP_ID']) {
					$this->htmlFoot('ERROR','Missing STEP_ID.');
				}
				if (!$go) {
					$this->del_form($sqlo, $parx['STEP_ID']);
					return;
				}
				$modLib->removeProto($sqlo, $parx['STEP_ID']);
				break;
			case 'move':
				$this->_html->line_out('Move step.');
				if (!$parx['STEP_ID']) {
					$this->htmlFoot('ERROR','Missing STEP_ID.');
				}
				if (!$parx['XORDER']) {
					$this->htmlFoot('ERROR','Missing XORDER.');
				}
				if ($parx['XORDER']>$MAXORD) {
					$this->htmlFoot('ERROR','XORDER must be smaller than '.$MAXORD);
				}
				
				$modLib->update($sqlo, $parx);
				break;
				
			case 'ord.move.many':
			    
			    $this->_html->line_out('Change the order of the protocols.');
			    
			    if (!$go) {
			        $this->ord_move_many_form($sqlo);
			        return;
			    } else {
			    
    			    $move_many_lib = new oAS_HAS_PR_move_many($sqlo, $this->objid);
    			    
    			    $pra_params = $_REQUEST['pras'];
    			    if (empty($pra_params) ) {
    			        $this->htmlFoot('ERROR','Missing Prorocol parameters.');
    			    }
    
    			    $move_many_lib->update($sqlo, $sqlo2, $pra_params);
    			    
    			    if ($error->Got(READONLY))  {
    			        
    			        $errLast   = $error->getLast('',0, READONLY);
    			        $error_txt = $errLast->text;
    			        $error_id  = $errLast->id;
    			        echo "<br><br>";
    			        $error->printAll();
    			        $error->reset();
    			        echo "<br><br>";
    			        if ($error_id>=100) {
    			            $this->ord_move_many_form($sqlo, $pra_params);
    			        }
    			        return;
    			        
    			    }
			    }
			    
			    break;
		
			default:
				$this->htmlFoot('ERROR','Action missing.');
		}
		
		if ($error->Got(READONLY))  {
			return;
		}
		
		$modLib->close($sqlo);
		
		$this->_html->pageForward('edit.tmpl.php?t=ABSTRACT_SUBST&id='.$this->objid,'back to substance');
	}
	
}
