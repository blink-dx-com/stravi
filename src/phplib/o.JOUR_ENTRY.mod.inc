<?php
require_once ('f.update.inc');
require_once ("insertx.inc");
require_once 'f.text_ptxt.inc';
require_once 'o.S_OBJLINK.subs.inc';
require_once 'o.S_OBJLINK.mod.inc';

/**
 * modify JOUR_ENTRY
 * @author skube
 * @package o.JOUR_ENTRY.mod.inc
 *
 */
class oJOUR_ENTRY_mod {
    
    private $obj_id;
    const tablename='JOUR_ENTRY';
    const LINK_KEY ='ptxt';
    const NOTES_MAX_LEN=10000;

    
    function set_obj($sqlo, $jour_id) {
        $this->obj_id = $jour_id;
    }
    
    
    /** update object links
     * 
     * @param object $sqlo
     * @param array $new_objects
     */
    function _update_links($sqlo, &$new_objects) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        // 't'=>, 'id'=>
        $add_objects=array();
        $del_objects=array();

        $old_objects = oS_OBJLINK_subs::getChiObjByKey($sqlo, self::tablename, $this->obj_id , self::LINK_KEY);
        if ( $_SESSION["userGlob"]["g.debugLevel"]>=2 ) {
            debugOut('new_objects:'.print_r($new_objects,1), $FUNCNAME, 1);
            debugOut('old_objects:'.print_r($old_objects,1), $FUNCNAME, 1);
        }
        
        if (empty($old_objects)) {
            
            // just add
            if (empty($new_objects)) {
                debugOut('(49) nothing to update.', $FUNCNAME, 2);
                return; // do nothing ...
            }
            $add_objects = $new_objects;  // add all
            
        } else {
            
            // find to DELETE objects
            foreach($old_objects as $o_row) {
                $found=0;
                foreach($new_objects as $n_row) {
                    if ( ($o_row[0]==$n_row['t']) and  ($o_row[1]==$n_row['id']) ) {
                        $found=1;
                        break;
                    }
                }
                
                if ($found) {
                    // nothing
                } else {
                    $del_objects[] = array('t'=>$o_row[0], 'id'=>$o_row[1]);
                }
            }
            
            // find ADD objects
            foreach($new_objects as $n_row) {
                $found=0;
                foreach($old_objects as $o_row) {
                    if ( $o_row[0]==$n_row['t'] and  $o_row[1]==$n_row['id']) {
                        $found=1;
                        break;
                    }
                }
                
                if ($found) {
                    // nothing 
                } else {
                    $add_objects[] = $n_row;
                }
            }
        }
        
        // nothing to update ...
        
        if ( empty($add_objects) and empty($del_objects) ) {
            debugOut('(94) nothing to update.', $FUNCNAME, 2);
            return;
        }
      
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>=2 ) {
            debugOut('add_objects:'.print_r($add_objects,1), $FUNCNAME, 1);
            debugOut('del_objects:'.print_r($del_objects,1), $FUNCNAME, 1);
        }

        
        $link_mod_lib = new oS_OBJLINK_mod();
        $link_mod_lib->setObj($sqlo, self::tablename, $this->obj_id);
        
        if (!empty($add_objects)) {
            // just add
            foreach($add_objects as $row) {
                $link_mod_lib->addLink($sqlo, $row['t'], $row['id'], self::LINK_KEY);
            }
        }
        
        if (!empty($del_objects)) {
            // delete
            foreach($del_objects as $row) {
                $link_mod_lib->removeLinkByObjID($sqlo, $row['t'], $row['id']);   
            }
        }
 
    }
    
    private function check_args(&$args) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        if( is_string($args['NOTES']) ) {
            if (strlen($args['NOTES'])>self::NOTES_MAX_LEN) {
                $error->set( $FUNCNAME, 1, 'NOTES-field too long: MAX:'.self::NOTES_MAX_LEN.'.' );
                return;
            }
        }
    }
    
    function update($sqlo, $args) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->check_args($args);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on input-check.' );
            return;
        }
         
        $args['JOUR_ENTRY_ID']=$this->obj_id;
        gObjUpdate::update_row( $sqlo, self::tablename, $args);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 2, 'Error on update.' );
            return;
        }
        
        $new_objects = f_text_ptxt::get_objects($sqlo, $args['NOTES']);
        $this->_update_links($sqlo, $new_objects);
    }
    
    function insert($sqlo, $argu) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->check_args($argu['vals']);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on input-check.' );
            return;
        }
        
        $newid = insertC::new_meta( $sqlo, self::tablename, $argu);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on create.' );
            return;
        }
        $this->obj_id = $newid;
        
        $new_objects = f_text_ptxt::get_objects($sqlo, $argu['vals']['NOTES']);
        $this->_update_links($sqlo, $new_objects);
        
        
        
        return $newid;
    }
    
}