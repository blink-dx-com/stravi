<?php
require_once('glob.obj.create.ABS.inc');

/**
 * object specific modification class
 * @package oCHIP_READER.xmod.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oCHIP_READER_xmodC extends gObjCreateC_ABS {
    
    

    //  copy protocol from SRC to DEST
    static function _protoCpy( &$sql, &$sql2,  $tablename, $objid,  $blueObjId ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        global $error;
        
        $creaInfo=array();
        
        $sqlDateNow = date_unix2datestr( time(), 1 );
        $copyAssocLib = new fCopyAssoc(); 
        
        $pk_name = $tablename . "_ID";
        
        $sql->query("select concrete_proto_id from ".$tablename." where ".$pk_name."=".$blueObjId);
        $sql->ReadRow();
        $conc_proto_id = $sql->RowData[0];
        if ( !$conc_proto_id ) {
            return;
        }
        
        $tablename_tmp = "CONCRETE_PROTO";
        $obj_oriparam = objInfoC::getParamsRaw($sql, $tablename_tmp, $conc_proto_id);
        $obj_oriparam["vals"]["EXEC_DATE"] = $sqlDateNow;  // set current date
        unset($obj_oriparam["vals"]["CONCRETE_PROTO_ID"]); // PK
        
        
        $creaInfo[] =  "... creating new protocol for $tablename";
        $c_proto_new_id = insertC::new_meta( $sql, $tablename_tmp, $obj_oriparam );
        if (!$c_proto_new_id) {
            $error->set($FUNCNAME, 1, "object concrete_proto for $tablename not created!");
            return;
        }
        $sqls = "concrete_proto_id=".$c_proto_new_id." where ".$pk_name."=".$objid;
        $sql->Updatex($tablename, $sqls);
        
        $deep_copy = array("CONCRETE_PROTO_STEP" => 1);
        $asOpt = NULL;
        $objAssCreaNew = NULL;
        $copyAssocLib->new_assoc( $sql, $sql2, $tablename_tmp, $conc_proto_id, $c_proto_new_id,
            $deep_copy,  $objAssCreaNew, $asOpt);
        
        if ($error->Got(READONLY)) {
            $error->set($FUNCNAME, 4, "insert of associated elements failed");
            return;
        }
        return $creaInfo;
        
    }
    

   
    function post_crea( &$sql, &$sql2, $objid,  $blueObjId, $option  = array()	) {
        // copy the protocol
        if (!$blueObjId) return;
        oCHIP_READER_xmodC::_protoCpy( $sql, $sql2, 'CHIP_READER', $objid, $blueObjId );
    }

}
