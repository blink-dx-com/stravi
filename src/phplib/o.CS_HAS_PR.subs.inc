<?php
require_once ("f.assocUpdate.inc");

/**
 * common analysis methods for CS_HAS_PR for one CONCRETE_SUBST
 * 
 * @package o.CS_HAS_PR.subs.inc
 * @swreq   SREQ:0002035: manage CS_HAS_PR
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * 
 * @var STRUCT cprotoLog_STRUCT = array of
 *    array('cp'=>ID of protocol, 'ap' => ID of ABSTRACT_PROTO, 'or'=>order number, 'ty'=>1,2)
 */
class oCS_HAS_PR_subs {
    
    const TYPE_PREP=1;
    const TYPE_QC=2;
	
	function __construct($objid=NULL) {
		
		$this->_table_exists = glob_table_exists('CS_HAS_PR');
		
		if ($objid) $this->setObj($objid);
		
		$this->typeDef=array(1=>'Prep',2=>'QC'); // TYPE-definition
	}
	
	function getTypeDef() {
		return $this->typeDef;
	}
	
	function setObj($objid) {
		$this->objid = $objid;
	}
	
	static function get_SUC_by_PRC($sqlo, $cpid) {
	    $sqls="CONCRETE_SUBST_ID  from CS_HAS_PR where CONCRETE_PROTO_ID=". $cpid;
	    $sqlo->Quesel($sqls);
	    $sqlo->ReadRow();
	    $suc_id = $sqlo->RowData[0];
	    return $suc_id;
	}
	
	/**
	 * get full protocol log (order by STEP_NO)
	 * @param  $sqlo
	 * @param  $deftype : igf given, return only one type of protocol
	 * @param  $activeOnly : 0, 1: give only active protocols: xorder<10000
	 * @return array cprotoLog_STRUCT $proto_arr
	 */
	function getProtoLog(&$sqlo, $deftype=0, $activeOnly=0) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    if ( !$this->objid ) {
	        throw new Exception('Class not inizialized.');
	    }
	    
		$id = $this->objid;
		$proto_arr=array();
		
		if ($this->_table_exists) {
			$sqlAdd = NULL;
			if ($deftype>0)    $sqlAdd .= ' and TYPE='.$deftype;
			if ($activeOnly>0) $sqlAdd .= ' and STEP_NO<10000'; // @swreq:0001723:005
			
			$sqls="CONCRETE_PROTO_ID, STEP_NO, TYPE from CS_HAS_PR where CONCRETE_SUBST_ID=" .$id. $sqlAdd .
			  " order by STEP_NO";
			$sqlo->Quesel($sqls);
			
			while ( $sqlo->ReadRow() ) {
				$c_proto_id = $sqlo->RowData[0]; 
				$XORDER     = $sqlo->RowData[1];
				$type       = $sqlo->RowData[2]; 
				$proto_arr[]=array('cp'=>$c_proto_id, 'or'=>$XORDER, 'ty'=>$type);
			}
			
			if (sizeof($proto_arr) ) {
			    // update 'ap'
			    foreach($proto_arr as $index => $row) {
			        $apid = glob_elementDataGet( $sqlo, 'CONCRETE_PROTO', 'CONCRETE_PROTO_ID', $row['cp'] ,'ABSTRACT_PROTO_ID' );
			        $proto_arr[$index]['ap'] = $apid;
			    }
			    
			}
		}
		return $proto_arr;
	}
	
	/**
	 * get first protocol by PRA
	 * @param  $sqlo
	 * @param  int $pra_id
	 * @return int $prc
	 */
	function getProtoByPRA(&$sqlo, $pra_id) {
	    $c_proto_id = 0;
	    
	    $sqls="x.CONCRETE_PROTO_ID from CS_HAS_PR x join CONCRETE_PROTO p on x.CONCRETE_PROTO_ID=p.CONCRETE_PROTO_ID".
	   	    " where x.CONCRETE_SUBST_ID=" .$this->objid." and p.ABSTRACT_PROTO_ID=".$pra_id;
	        " order by x.STEP_NO";
	    $sqlo->Quesel($sqls);
	    if ($sqlo->ReadRow() ) {
	        $c_proto_id = $sqlo->RowData[0];  
	    }
	    return $c_proto_id;
	}
	
	
	
	/**
	 * merge array from CONCRETE and ABSTRACT proto log
	 * PRIVATE!
	 * @param array $outarray
	 * @param array $inarray
	 */
	static function _mergeOneArr(&$outarray, $inarray) {
	    
		if (empty($inarray)) return;
		
		
		foreach( $inarray as $key=>$valarr) {
			$ordx=$valarr['or'];
			$outarray[$ordx]['or'] = $ordx;
			$outarray[$ordx]['st'] = $valarr['st'];
			$outarray[$ordx]['ty'] = $valarr['ty'];
			
			if ($valarr['cp']>0) {
				$outarray[$ordx]['cp'] = $valarr['cp'];
				if ($valarr['ap']>0) $outarray[$ordx]['ap'] = $valarr['ap'];
			} else {
				if ($valarr['ap']>0) {
					$outarray[$ordx]['ap'] = $valarr['ap'];
					if ($ordx>=9998)  $outarray[$ordx]['st'] = $ordx; // identify, this is a final protocol
				}
			}
		}
		
		ksort($outarray); // key sort at the end
	}
	
	/**
	 * merge array from CONCRETE and ABSTRACT proto log
	 * @param array cprotoLog_STRUCT $cp_array from CP_HAS_PR
	 * @param array aprotoLog_STRUCT $ap_array from AP_HAS_PR
	 * @return array $outarr of ORDER=>array('st', 'cp', 'ap', 'or', 'ty')
	 */
	static function mergeProtoArr(&$cp_array, &$ap_array) {
		$mergearr = array();
		self::_mergeOneArr($mergearr, $ap_array);
		self::_mergeOneArr($mergearr, $cp_array);
		return $mergearr;
	}
	
	/**
	 * get next free deactivation xorder-value after xorder=10000
	 * needed for protocol deactivation
	 * @param $sqlo
	 */
	function getFreeDeactOrder(&$sqlo) {
		$id = $this->objid;
		$START_XORDER=10000;
		$sqls="max(STEP_NO) from CS_HAS_PR where CONCRETE_SUBST_ID=" .$id. " and STEP_NO>".$START_XORDER;
		$sqlo->Quesel($sqls);
		$sqlo->ReadRow();
		$last_step_no = $sqlo->RowData[0]; 
		if (!$last_step_no) $last_step_no = $START_XORDER;
		$new_stepno = $last_step_no + 1;
		return $new_stepno;
		
	}
	
	/**
	 * add protocol to SUBSTANCE
	 * - check if STEP_NO is occupied
	 * @param object $sqlo
	 * @param int  $STEP_NO
	 * @param int $cpid - CONCRETE_PROTO_ID
	 * @param int $type : 1,2
	 * @throws Exception
	 */
	function add_protocol(&$sqlo, $STEP_NO, $cpid, $type=1) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $suc_id = $this->objid;
	    
	    if ( !$this->objid ) {
	        throw new Exception('Class not inizialized.');
	    }
	    
	    // check STEP_NO
	    $sqls="CONCRETE_PROTO_ID from CS_HAS_PR where CONCRETE_SUBST_ID=" .$suc_id. " and STEP_NO=".$STEP_NO;
	    $sqlo->Quesel($sqls);
	    if ($sqlo->ReadRow()) {
	        $cpid_on_step = $sqlo->RowData[0]; 
	        $error->set( $FUNCNAME, 10, 'SUC-ID:'.$suc_id.': there is already a protocol on Step '.$STEP_NO.'.' );
	        return;
	    }
	    
	    
	    $params = array(
	        'STEP_NO'=> $STEP_NO,
	        'CONCRETE_PROTO_ID'=>$cpid,
	        'TYPE'    => $type
	    );
   
	    $assoclib = new  fAssocUpdate();
	    $assoclib->setObj( $sqlo, 'CS_HAS_PR', $suc_id);
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 20, 'Error on proto-connect for SUC-ID:'.$suc_id );
	        return;
	    }
	    $assoclib->insert( $sqlo, $params );
	    $assoclib->close($sqlo);
	}
	
	function unlink_protocol($sqlo, $cpid) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    if ( !$this->objid ) {
	        throw new Exception('Class not inizialized.');
	    }
	    $suc_id = $this->objid;
	    
	    $assoclib = new  fAssocUpdate();
	    $assoclib->setObj( $sqlo, 'CS_HAS_PR', $suc_id);
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 1, 'Error on mod-init for SUC-ID:'.$suc_id );
	        return;
	    }
	    
	    $idarr=array('CONCRETE_PROTO_ID'=>$cpid);
	    $assoclib->delOneRow($sqlo, $idarr);
	    $assoclib->close($sqlo);
	}
	
	/**
	 * get PRC-ID by PRA-ID
	 * @param array  $proto_arr
	 * @param int $pra_id
	 * @return int
	 */
	static function get_prc_by_pra(array &$proto_arr, $pra_id) {
	    $found=0;
	    foreach($proto_arr as $one_proto) {
	        if ($one_proto['ap']==$pra_id) {
	            $found=1;
	            break;
	        }
	    }
	    if ($found) return $one_proto['cp'];
	}
	
}
