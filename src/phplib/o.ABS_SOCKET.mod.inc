<?php
require_once ("f.assocUpdate.inc");

/**
 * MODIFICATION methods for ABS_SOCKET:Blink specific
 * @package o.ABS_SOCKET.mod.inc

 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oABS_SOCKET_mod {
    
    function set_obj(&$sqlo, $objid) {
        
        $this->objid = $objid;
        $this->assoclib = new  fAssocUpdate();
        $this->assoclib->setObj( $sqlo, 'ABS_CART_H_SUA', $this->objid );
    }

    
    /**
     * 
     * @param unknown $sqlo
     * @param unknown $pos
     * @param unknown $sua_id
     * @param unknown $key
     * @param unknown $volume TBD:
     * @throws Exception
     */
    function update_one_pos($sqlo, $pos, $sua_id, $key, $volume=NULL) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$this->objid) {
            throw new Exception('Class not initialized.');
        }
        
        
        $objid = $this->objid;
        
        $table='ABS_CART_H_SUA';
        
        if ($key==NULL) {
            $error->set( $FUNCNAME, 1, 'Input: KEY missing.' );
            return;
        }
        
        //old SUA_ID
        $old_data = glob_elemDataGet3( $sqlo, $table, array('ABS_SOCKET_ID'=>$objid, 'POS'=>$pos, 'KEY'=>$key),
            array('ABSTRACT_SUBST_ID', 'STEP') );
        $old_sua_id = $old_data['ABSTRACT_SUBST_ID'];
        $idarr = array('STEP'=>$old_data['STEP']);
        
        $sqlsel = 'max(STEP) from ABS_CART_H_SUA where ABS_SOCKET_ID='.$objid;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $maxstep = $sqlo->RowData[0];
        
        if (!$maxstep) $maxstep=0;
        $next_step = $maxstep+1;
        
        if ($sua_id and !$old_sua_id) {
            // insert
            $argu=array(
                'STEP' => $next_step,
                'ABSTRACT_SUBST_ID' => $sua_id,
                'KEY'   => $key,
                'POS'   => $pos,
                'VOLUME'=> $volume
            );
            $this->assoclib->insert( $sqlo, $argu );
            return;
        }
        if ($sua_id and $old_sua_id) {
            // update: STEP and POS are untouched
            
            $argu=array(
                'ABSTRACT_SUBST_ID'=>$sua_id,
                'KEY'    => $key,
                'VOLUME' => $volume
            );
            $this->assoclib->update( $sqlo, $argu, $idarr);
            return;
        }
        if (!$sua_id and $old_sua_id) {
            // delete
            $this->assoclib->delOneRow( $sqlo, $idarr );
            return;
        }
        
        //glob_elemDataGet3( $sqlo, $tablename, $whereArr, $outColArr )
        
    }
    
    /**
     * update many components
     * @param object $sqlo
     * @param array $sua_ids
     * @param array $volume_arr : array of (POS) => volume (only for REAG
     */
    function update_components($sqlo, $sua_ids, $volume_arr=array() ) {
        
        if (!$this->objid) {
            throw new Exception('Class not initialized.');
        }
        
     
        
        foreach($sua_ids as $pos => $key_arr ) {
            foreach($key_arr as $key => $sua_id ) {
                $volume = NULL;
                if ($key='REAG') {
                    if (array_key_exists( $pos, $volume_arr) ) {
                        $volume = $volume_arr[$pos];
                    }
                }
                $this->update_one_pos($sqlo, $pos, $sua_id, $key, $volume);
            }
        }
    }
    
    
}