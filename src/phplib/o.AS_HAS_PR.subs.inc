<?php
require_once 'o.ABSTRACT_SUBST.defs.inc';

/**
 * common analysis methods for AS_HAS_PR for one ABSTRACT_SUBST
 * 
 * @package o.AS_HAS_PR.subs.inc
 * @swreq   SREQ:0002034: manage AS_HAS_PR
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @var proto_arr_STRUCT = array of
	 array( 'st'=>$step_id, 'ap'=>$a_proto_id, 'or'=>$XORDER, 'ty'=> 1(PREP or 2(QC) )
	 OLD: ABSTRACT_PROTO_ID: 'or'=>9998
	 OLD: QC_ABSPROTO_ID' : 'or'=>9999
 */
class oAS_HAS_PR_subs {
	
	function __construct($objid=NULL) {
		if ($objid) $this->setObj($objid);
		
		$this->typeDef=array(oABSTRACT_SUBST_DEFS::PR_TYPE_PREP=>'Prep', oABSTRACT_SUBST_DEFS::PR_TYPE_QC=>'QC'); // TYPE-definition
		$this->tableASPR_exists = glob_table_exists('AS_HAS_PR');
	}
	
	function getTypeDef() {
		return $this->typeDef;
	}
	
	function setObj($objid) {
		$this->objid = $objid;
	}
	
	/**
	 * get full protocol log
	 * @param  $sqlo
	 * @param int $deftype 0,1,2
	 * @return array $proto_arr proto_arr_STRUCT    
	 */
	static function getProtoLogStatic(&$sqlo, int $id, $deftype=0) {
		$proto_arr=array();
		$sqlAdd = NULL;
		if ($deftype>0) $sqlAdd = ' and TYPE='.$deftype;
		$sqls="STEP_ID, ABSTRACT_PROTO_ID, XORDER, TYPE from AS_HAS_PR where ABSTRACT_SUBST_ID=" . $id . $sqlAdd .
		    " order by XORDER";
		$sqlo->Quesel($sqls);
		
		while ( $sqlo->ReadRow() ) {
			$step_id    = $sqlo->RowData[0]; 
			$a_proto_id = $sqlo->RowData[1]; 
			$XORDER     = $sqlo->RowData[2]; 
			$type       = $sqlo->RowData[3]; 
			$proto_arr[]=array('st'=>$step_id, 'ap'=>$a_proto_id, 'or'=>$XORDER, 'ty'=>$type);
		}

		return $proto_arr;
	}
	
	function getProtoLog(&$sqlo, $deftype=0) {
	    if ( !$this->objid ) {
	        throw new Exception('Class not inizialized.');
	    }
	    return self::getProtoLogStatic($sqlo, $this->objid, $deftype);
	}
	
	/**
	 * get STEP_ID by PRA_ID
	 * @param object $sqlo
	 * @param int $pra_id
	 * @return number
	 */
	function pra_STEP_in_log($sqlo, int $pra_id) {
	    $step_id=0;
	    $sqls="STEP_ID from AS_HAS_PR where ABSTRACT_SUBST_ID=" . $this->objid . " and ABSTRACT_PROTO_ID=".$pra_id;
	    $sqlo->Quesel($sqls);
	    if ( $sqlo->ReadRow() ) {
	        $step_id    = $sqlo->RowData[0];
	    }
	    return $step_id;
	        
	}
	
	// add H_PROTO_KIND info ...
	// 'ki_id', 'ki_name', 'ki_key'
	static function add_kind_info(&$sqlo, &$proto_arr) {
	    
	    foreach($proto_arr as $i =>$row) {   
	        $apid = $row['ap'];
	        if ($apid)  {
        	    $kind_id = glob_elementDataGet( $sqlo, 'ABSTRACT_PROTO', 'ABSTRACT_PROTO_ID', $apid, 'H_PROTO_KIND_ID'); ;
        	    if ($kind_id) {
        	        $kind_info = glob_elemDataGet3( $sqlo, 'H_PROTO_KIND', array('H_PROTO_KIND_ID'=>$kind_id), array('NAME', 'KEYX') );
        	        $proto_arr[$i]['ki_id']   = $kind_id;
        	        $proto_arr[$i]['ki_name'] = $kind_info['NAME'];
        	        $proto_arr[$i]['ki_key']  = $kind_info['KEYX'];
        	    }
	        }
	    }
	}
	
	/**
	 * get count by ABSTRACT_PROTO_ID
	 * @param object $sqlo
	 * @param int $cpid
	 * @return int
	 */
	static function suas_have_proto(object $sqlo, int $apid) {
	    $sqlsel= "count(1) from AS_HAS_PR".
	   	    " where ABSTRACT_PROTO_ID=".$apid;
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    $cnt = $sqlo->RowData[0];
	    return $cnt;
	}

}
