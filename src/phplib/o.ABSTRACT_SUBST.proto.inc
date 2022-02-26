<?php
require_once 'o.ABSTRACT_SUBST.defs.inc';
require_once ("o.AS_HAS_PR.subs.inc");

/**
 * sub functions for concrete subst,  protocol things
 * @package o.ABSTRACT_SUBST.proto.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oABSTRACT_SUBST_proto {

    function __construct() {
        $this->AS_HAS_PR_lib   = new oAS_HAS_PR_subs();
      
    }
    
    function set_obj(&$sqlo, $sua_id) {
        $this->sua_id = $sua_id;
        $this->AS_HAS_PR_lib->setObj($sua_id);
    }
    
    static function get_protos_STATIC(&$sqlo, int $sua_id) {
        $proto_log = oAS_HAS_PR_subs::getProtoLogStatic($sqlo, $sua_id);
        return $proto_log;
    }

    /**
     * get protocol log
     * @param object $sqlo
     * @throws Exception
     * @return array proto_arr_STRUCT
     */
    function get_protos(&$sqlo) {
        if ( !$this->sua_id ) {
            throw new Exception('Class not inizialized.');
        }
        $proto_log = $this->AS_HAS_PR_lib->getProtoLog($sqlo);
        return $proto_log;
    }
    
    /**
     * do object tracking for to the next SUA level 
     * @param object $sqlo
     * @return array
     *   'suas'=> array of SUA-IDs
     *   'cnt'=> real number of hits
     */
    function obj_tracking_sua(&$sqlo) {
        if ( !$this->sua_id ) {
            throw new Exception('Class not inizialized.');
        }
        
        $MAX_STORE=50;
        
        $sua_arr = array();
        
        $sqlsel_1 = 'select ABSTRACT_PROTO_ID from ABSTRACT_PROTO_STEP where ABSTRACT_SUBST_ID='.$this->sua_id;
        $sqlsel_2 = "from AS_HAS_PR where ABSTRACT_PROTO_ID in (".$sqlsel_1.")";
        
        $sqlo->Quesel("count(1) ".$sqlsel_2);
        $sqlo->ReadRow();
        $cnt = $sqlo->RowData[0];
        
        $sqlo->Quesel("ABSTRACT_SUBST_ID ".$sqlsel_2);
        $i=0;
        while ($sqlo->ReadRow() ) {
            if ($i>$MAX_STORE) break;
             $sua_loop = $sqlo->RowData[0];
             $sua_arr[]= $sua_loop;
             $i++;
        }
             
        return array('cnt'=>$cnt, 'suas'=>$sua_arr);
    }

}
