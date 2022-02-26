<?php

/**
 * experiment protocol analysis functions
 * @namespace core::obj::EXP
 * @package o.EXP.proto.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * 
 * @var STRUCT exp_protoLog_STRUCT = array of
 *    array(
 *      'cp'=>ID of protocol, 'st'=>step number, 'proto_id'=>int, 'ap'=>ID of abstract proto, 'ki' => kind name
 *      )
 */
class oEXPprotoC {
    
    const PRA_KIND_EXP_PROTO='Experiment protocol';

    /**
     * get abstract protocols from experiment $expid
     * @param object $sql
     * @param int   $expid
     * @return  array $a_proto [step] = array(ABSTRACT_PROTO_ID, NAME);
     */
    function getAbsProtocols (&$sql, $expid) { 
     	$a_proto     = NULL;
    	$a_proto_tmp = NULL;
    	
    	$sqls= "select e.STEP_NO, c.ABSTRACT_PROTO_ID from EXP_HAS_PROTO e, CONCRETE_PROTO c where e.EXP_ID=".$expid. 
    				" AND e.CONCRETE_PROTO_ID=c.CONCRETE_PROTO_ID ORDER by STEP_NO";
    	
    	$sql->query($sqls);
    	while ( $sql->ReadRow() ) {
    		$a_proto_tmp[$sql->RowData[0]] = $sql->RowData[1];
    	}
    
    	if ( sizeof($a_proto_tmp) ) {
    		
    		foreach( $a_proto_tmp as $tmp_step=>$tmp_abs_proto) {
    			$sqls= "select NAME from ABSTRACT_PROTO where ABSTRACT_PROTO_ID=".$tmp_abs_proto;
    			$sql->query("$sqls");
    			$sql->ReadRow();
    			$name=$sql->RowData[0];
    			$a_proto[$tmp_step] = array($tmp_abs_proto, $name);
    		}
    
    	}
    	return ($a_proto);
    }
    
    /**
     * get concrete_proto_id by ABSTRACT_PROTO_ID of EXP x
     * @return int CONCRETE_PROTO_ID
     */
    static function getConcProtoByAbstract (&$sql, $expid, $aProtoID) {
    	
    	$sqls= "select e.CONCRETE_PROTO_ID from EXP_HAS_PROTO e, CONCRETE_PROTO c ".
    		   "where e.EXP_ID=".$expid." AND c.ABSTRACT_PROTO_ID=".$aProtoID. 
    				" AND e.CONCRETE_PROTO_ID=c.CONCRETE_PROTO_ID";
    	
    	$sql->query($sqls);
    	$sql->ReadRow();
    	$cprotoid = $sql->RowData[0];
    	return ($cprotoid);
    }
    
    /**
     * get concrete_proto_id by ABSTRACT_PROTO:KIND:NAME of EXP x
     * - doeas not analyse, if kind exisdts more than once in experiment
     * @param $kind e.g. 'Measurement'
     * @return int CONCRETE_PROTO_ID
     */
    static function getConcByAbstractKind (&$sqlo, $expid, $kind) {
    	
    	$sqlsel = 'H_PROTO_KIND_ID from H_PROTO_KIND where name='.$sqlo->addQuotes($kind);
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadRow();
    	$H_KIND_ID = $sqlo->RowData[0];
    	if (!$H_KIND_ID) return;
    	
    	$sqlsel= "e.CONCRETE_PROTO_ID from EXP_HAS_PROTO e, CONCRETE_PROTO c ".
    		" where e.EXP_ID=".$expid. 
    		" AND e.CONCRETE_PROTO_ID=c.CONCRETE_PROTO_ID".
    		" AND c.ABSTRACT_PROTO_ID in ".
    		"  (select ABSTRACT_PROTO_ID from ABSTRACT_PROTO where H_PROTO_KIND_ID=".$H_KIND_ID.
    		"  )";
    	
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadRow();
    	$cprotoid = $sqlo->RowData[0];
    	return ($cprotoid);
    }
    
    /**
     * get all concrete protocol IDs of experiment
     * @param $expid
     * @return array $cprotos = array of IDs
     */
    function getCProtos (&$sqlo, $expid) {
    	$sqlsel= "e.STEP_NO, e.CONCRETE_PROTO_ID from EXP_HAS_PROTO e".
    	   " where e.EXP_ID=".$expid. 
           " order by e.STEP_NO";
    	
    	$cprotos = array();
    	$sqlo->Quesel($sqlsel);
    	while ( $sqlo->ReadRow() ) {
    		$cprotos[$sqlo->RowData[0]] = $sqlo->RowData[1];
    	}
    	return $cprotos;
    
    }
    
    /**
     * get details
     * @param object $sqlo
     * @param int $expid
     * @return array exp_protoLog_STRUCT
     */
    function get_c_protos_details (&$sqlo, $expid) {

        $cprotos = array();
        
        $sqls = "x.STEP_NO, x.CONCRETE_PROTO_ID, p.ABSTRACT_PROTO_ID from EXP_HAS_PROTO x join CONCRETE_PROTO p ".
            " on x.CONCRETE_PROTO_ID=p.CONCRETE_PROTO_ID" .
            " where EXP_ID=" . $expid.
            " ORDER by STEP_NO";
        $sqlo->Quesel($sqls);
        while ( $sqlo->ReadRow() ) {
            $cprotos[] = array('st'=>$sqlo->RowData[0], 'cp'=> $sqlo->RowData[1], 'ap'=>$sqlo->RowData[2] );
        }
        
        if (!sizeof($cprotos)) {
            return ($cprotos);
        }
        
        foreach( $cprotos as $index=>$row ) {
            $apid = $row['ap'];
           
            $kind_id = glob_elementDataGet( $sqlo, 'ABSTRACT_PROTO', 'ABSTRACT_PROTO_ID', $apid, 'H_PROTO_KIND_ID'); ;
            if ($kind_id) {
                $kind_name = glob_elementDataGet( $sqlo, 'H_PROTO_KIND', 'H_PROTO_KIND_ID', $kind_id, 'NAME'); ;
                $cprotos[$index]['ki'] = $kind_name;
            }
        }
        
       
        
        return ($cprotos);
    }
    
    
    /**
     * get concrete protocol of experiment, defined by EXP_HAS_PROTO.STEP_NO
     * @param $sqlo
     * @param int $expid experiment id
     * @param int $step STEP_NO in table EXP_HAS_PROTO
     * @return int $cproto (ID of protocol)
     */
    function getCProtoByStep (&$sqlo, $expid, $step) {
    	$sqlsel= "e.CONCRETE_PROTO_ID from EXP_HAS_PROTO e".
    	   " where e.EXP_ID=".$expid. 
           " and e.STEP_NO=".$step;
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadRow();
    	$cproto = $sqlo->RowData[0];
    	return ($cproto);
    
    }
    
    /**
     * merge array from CONCRETE and ABSTRACT proto log
     * PRIVATE!
     * @param array exp_protoLog_STRUCT $outarray
     * @param array exp_protoLog_STRUCT $inarray
     */
    private function _mergeOneArr(&$outarray, $inarray) {
        if (!sizeof($inarray)) return;
        
        
        foreach( $inarray as $key=>$valarr) {
            
            $step = $valarr['st'];
            $outarray[$step]['st'] = $valarr['st'];
                
            if ($valarr['cp']>0) {
                $outarray[$step]['cp'] = $valarr['cp'];
                $outarray[$step]['ki'] = $valarr['ki'];
                
                if ($valarr['ap']>0) {
                    $outarray[$step]['ap'] = $valarr['ap'];
                }
            } else {
                if ($valarr['ap']>0) {
                    $outarray[$step]['ap'] = $valarr['ap'];
                    $outarray[$step]['ki'] = $valarr['ki'];
                }
            }
        }
        
        ksort($outarray); // key sort at the end
    }
    
    /**
     * merge array from CONCRETE and ABSTRACT proto log
     * @param array  $cp_array exp_protoLog_STRUCT
     * @param array  $ap_array exp_protoLog_STRUCT
     * @return array $outarr of exp_protoLog_STRUCT
     */
    function mergeProtoArr(&$cp_array, &$ap_array) {
        $mergearr = array();
        $this->_mergeOneArr($mergearr, $ap_array);
        $this->_mergeOneArr($mergearr, $cp_array);
        
        return $mergearr;
    }
    
    /**
     * get EXP_ID by CONCRETE_PROTO_ID
     * @param object $sqlo
     * @param int $cpid
     * @return int
     */
    static function get_one_exp_by_proto(object $sqlo, int $cpid) {
        $sqlsel= "e.EXP_ID from EXP_HAS_PROTO e".
            " where e.CONCRETE_PROTO_ID=".$cpid;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $exp_id = $sqlo->RowData[0];
        return $exp_id;
    }
    
    // 
    /**
     * get SQL command after FROM: EXP by (ABSTRACT_PROTO_ID, CONCRETE_SUBST_ID)
     * @param object $sqlo
     * @param int $exa_id
     * @param array $params
     *   pra_id
     *   suc_id : can be INT or array of INT
     *   sua_id
     *   exp_date : equals to EXP_DATE (between)
     * @return string
     */
    static function sql_exp_by_tmpl(object $sqlo, int $exa_id, array $params) {
        $pra_id=$params['pra_id'];
        
        $sub1 = "select CONCRETE_PROTO_ID from CONCRETE_PROTO_STEP where ".
            "ABSTRACT_PROTO_ID=".$pra_id;
        if (!empty($params['suc_id'])) {
            if (is_array($params['suc_id'])) {
                $sub1 .= " and CONCRETE_SUBST_ID in ( ".implode(',',$params['suc_id']).')';
            } else {
                $sub1 .= " and CONCRETE_SUBST_ID=".$params['suc_id'];
            }
        }
        if ($params['sua_id']) {
            $sub1 .= " and CONCRETE_SUBST_ID in (select CONCRETE_SUBST_ID from CONCRETE_SUBST where ".
                "ABSTRACT_SUBST_ID=".$params['sua_id'].")";
        }
        
        
        $sqlsel= "EXP x where x.EXP_TMPL_ID=".$exa_id." and x.EXP_ID in ".
            "(select e.EXP_ID from EXP_HAS_PROTO e".
              " where e.CONCRETE_PROTO_ID in ".
              "(".$sub1.")\n".
            " )";
        if ($params['exp_date']) {
            $day_start = $params['exp_date'].' 00:00';
            $day_end   = $params['exp_date'].' 23:59';
            $sqlsel .= " and (x.EXP_DATE  between ".$sqlo->addQuotes($day_start)." and ".$sqlo->addQuotes($day_end).')';
        }
        return $sqlsel;
    }

}