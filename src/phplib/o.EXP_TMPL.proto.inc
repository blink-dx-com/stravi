<?php
require_once 'f.assocUpdate.inc';

/**
 * exp_tmpl : protocol functions
 * @package o.EXP_TMPL.proto.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @var array exp_t_proto_STRUCT of array()
 *       'proto_id' second PK
     *   'step'
     *   'pra'
     *   'kind_id'
     *   'kind_name'
     *   'kind_key'
 */
class oEXP_TMPL_protoC {

    function __construct($exp_tmpl_id) {
        $this->exp_tmpl_id = $exp_tmpl_id;
    }
    
    /**
     * 
     * @param object $sql
     * @throws Exception
     * @return array exp_t_proto_STRUCT
     */
    function abs_protos_details (&$sqlo) {
        
        if (!$this->exp_tmpl_id) {
            throw new Exception('Class not initialized.');
        }
        
        $a_proto_arr = array();
        
        $sqls = "x.STEP_NO, x.ABSTRACT_PROTO_ID, p.H_PROTO_KIND_ID, x.PROTO_ID from EXP_TMPL_HAS_PROTO x join ABSTRACT_PROTO p ".
            " on x.ABSTRACT_PROTO_ID=p.ABSTRACT_PROTO_ID" .
            " where EXP_TMPL_ID=" . $this->exp_tmpl_id.
            " ORDER by STEP_NO";
        $sqlo->Quesel($sqls);
        while ( $sqlo->ReadRow() ) {
            $a_proto_arr[] = array('step'=>$sqlo->RowData[0], 'pra'=> $sqlo->RowData[1], 'kind_id'=>$sqlo->RowData[2], 'proto_id'=>$sqlo->RowData[3] );
        }
        
        if (!sizeof($a_proto_arr)) {
            return ($a_proto_arr);
        }
        
        foreach( $a_proto_arr as $index=>$row ) {
            $kind_id = $row['kind_id'];
            if ($kind_id) {
                $kind_inf = glob_elemDataGet3( $sqlo, 'H_PROTO_KIND', array( 'H_PROTO_KIND_ID'=> $kind_id), array('NAME','KEYX') );
                $a_proto_arr[$index]['kind_name'] = $kind_inf['NAME'];
                $a_proto_arr[$index]['kind_key']  = $kind_inf['KEYX'];
            }
        }
        
        return ($a_proto_arr);
    }
    
    /**
     * add a protocol to experiment-template
     * @param object $sql
     * @param int $abstract_proto_id
     * @param int $step_no
     * @return NULL
     */
    function add_proto( &$sqlo, $abstract_proto_id, $PROTO_ID, $step_no) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$this->exp_tmpl_id) {
            throw new Exception('Class not initialized.');
        }
 
        $assoclib = new  fAssocUpdate();
        $assoclib->setObj( $sqlo, "EXP_TMPL_HAS_PROTO", $this->exp_tmpl_id );
        if ($error->Got(READONLY))  {
            return 0;
        }
      
        $argu=array();
        $argu['PROTO_ID']	       = $PROTO_ID;
        $argu['STEP_NO']	       = $step_no;
        $argu['ABSTRACT_PROTO_ID'] = $abstract_proto_id;
        $ret_id = $assoclib->insert( $sqlo, $argu );
        if ($ret_id<=0) {
            $error->set( $FUNCNAME, 2, "error on proto-add: exp_tmpl_id: ".$this->exp_tmpl_id." PROTO_ID:$PROTO_ID");
            return 0;
        }
        
        
        return 1;
    } 
    
    function del_proto( &$sqlo, $PROTO_ID) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$this->exp_tmpl_id) {
            throw new Exception('Class not initialized.');
        }
        
        $assoclib = new  fAssocUpdate();
        $assoclib->setObj( $sqlo, "EXP_TMPL_HAS_PROTO", $this->exp_tmpl_id );
        if ($error->Got(READONLY))  {
            return 0;
        }
        $idarr=array('PROTO_ID'=>$PROTO_ID);
        $assoclib->delOneRow( $sqlo, $idarr );

    } 
    
    /**
     * get count by ABSTRACT_PROTO_ID
     * @param object $sqlo
     * @param int $cpid
     * @return int
     */
    static function exp_tmpl_has_proto(object $sqlo, int $apid) {
        $sqlsel= "count(1) from EXP_TMPL_HAS_PROTO".
            " where ABSTRACT_PROTO_ID=".$apid;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $cnt = $sqlo->RowData[0];
        return $cnt;
    }
    

}