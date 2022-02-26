<?php
require_once ('f.update.inc');
require_once 'o.PROTO.subs.inc';

/**
 * modification class
 * @package o.CONCRETE_PROTO.mod.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oCONCRETE_PROTO_mod {
	
    /**
     * copy main FEATURES and all STEPS to $id
     * @param object $sql
     * @param object $sql2
     * @param int $src_id
     * @param int $id
     * @param array $options
     *   'exec_date'
     * @return number
     */
    static function assoc_copy( &$sql, &$sql2, $src_id, $id, $options ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $sqls= "select EXEC_DATE, LINK_ID, NOTES from CONCRETE_PROTO where CONCRETE_PROTO_ID=".$src_id;
        $sql->query("$sqls");
        $sql->ReadRow();
        $tmp_link = $sql->RowData[1];
        $tmp_notes= $sql->RowData[2];
        
        $argu = array(
            "EXEC_DATE" => $options['exec_date'],
            "LINK_ID"   => $tmp_link,
            "NOTES"     => $tmp_notes
        );
        
        $idarr = array("CONCRETE_PROTO_ID"=>$id );
        gObjUpdate::update_row_s ( $sql, "CONCRETE_PROTO", $argu, $idarr );
        
        $sqls= "select * from CONCRETE_PROTO_STEP where CONCRETE_PROTO_ID=".$src_id;
        $sql->query("$sqls");
        // $cnt_elem=0;
        $save_cnt_elem=0;
        $retval = 0;
        
        $cModLib = new oCprotoMani();
        $cModLib->setCprotoID ( $sql, $id, 0 );
        
        while ( $sql->ReadArray() ) {
            
            $args=$sql->RowData;
            unset($args["CONCRETE_PROTO_ID"]);
            
            $cModLib->oneStepInsertRaw( $sql2, $args );
            if ($error->Got(READONLY))  {
                $error->set( $FUNCNAME, 1, '' );
                return -1;
            } else {
                $save_cnt_elem++;
            }
        }
        return $retval;
    }        

}
