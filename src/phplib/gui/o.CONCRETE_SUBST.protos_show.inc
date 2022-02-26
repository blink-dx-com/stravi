<?php
require_once ("gui/o.PROTO.stepout2.inc");
require_once 'o.CS_HAS_PR.subs.inc';

/**
 * WIDGET:   show ALL protocols of SUC
 * @package o.CONCRETE_SUBST.protos_show.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @example
 * 
 $proto_out_lib = new oCONCRETE_SUBST_protos_show($sqlo, $id);
 if ( $proto_out_lib->has_protos() ) {
     $proto_out_lib->writeJavascript();
     $proto_out_lib->show($sqlo, $sqlo2);
 }
 
 */
class oCONCRETE_SUBST_protos_show {
    
    function __construct(&$sqlo, $id) {
        
        $cs_PR_lib = new oCS_HAS_PR_subs();
        $cs_PR_lib->setObj($id);
        $this->c_protos  = $cs_PR_lib->getProtoLog($sqlo);
        
        $this->protoShowObj = new oProtocolShowC();
        
    }
    
    function writeJavascript() {
        $this->protoShowObj->writeJavascript();
    }
    
    function has_protos() {
        return sizeof($this->c_protos);
    }
 
    function show($sqlo, $sqlo2) {
        

        if (!sizeof($this->c_protos)) {
            return;
        }
            
    
        foreach( $this->c_protos as $proto_row) {
            
            $c_proto_id = $proto_row['cp'];
            if ( $c_proto_id ) {
                
                $sqlsel="a.NAME from CONCRETE_PROTO c, ABSTRACT_PROTO a where c.CONCRETE_PROTO_ID=".$c_proto_id." AND  a.ABSTRACT_PROTO_ID=c.ABSTRACT_PROTO_ID";
                $sqlo->Quesel($sqlsel);
                $sqlo->ReadRow();
                $a_proto_name = $sqlo->RowData[0];
                
                echo '&nbsp;&nbsp;&nbsp;<font color=gray>Abstract-Name: "'.$a_proto_name.'"</font> ';
                
                $a_proto_id=0;
                $this->protoShowObj->showAll( $sqlo, $sqlo2, $c_proto_id, $a_proto_id );
            }
        }
    }
    

}
	