<?php
require_once 'f.assocUpdate.inc';

 /**
 * ROLE modification functions
 * @package o.ROLE.mod.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
 class oROLE_mod {
     
     private $modified;
     
     function __construct($sqlo, $role_id) {
         $this->role_id = $role_id;
 
         $this->UR_cnt=0;
         $this->UR_assoclib = new fAssocUpdate();
         $this->UR_assoclib->setObj( $sqlo, 'RIGHT_IN_ROLE', $this->role_id );
         
         $this->ROL_cnt=0;
         $this->ROL_assoclib = new fAssocUpdate();
         $this->ROL_assoclib->setObj( $sqlo, 'USER_ROLES', $this->role_id ); 
         
        
         
     }
     
     function add_user_right($sqlo, $user_right_id) {
         $pk_array=array('USER_RIGHT_ID'=>$user_right_id, 'ROLE_ID'=> $this->role_id );
         if ( gElement_exists2($sqlo, 'RIGHT_IN_ROLE', $pk_array) ) {
             return;
         }
         
         
         $argu=array('USER_RIGHT_ID'=>$user_right_id);
         $this->UR_assoclib->insert($sqlo, $argu);
         $this->UR_cnt++;
     }
     
     function remove_user_right($sqlo, $user_right_id) {
         $idarr=array('USER_RIGHT_ID'=>$user_right_id);
         $this->UR_assoclib->delOneRow($sqlo, $idarr);
         $this->UR_cnt++;
     }
     
     function add_user_role($sqlo, $user_id) {
         $pk_array=array('DB_USER_ID'=>$user_id, 'ROLE_ID'=> $this->role_id );
         if ( gElement_exists2($sqlo, 'USER_ROLES', $pk_array) ) {
             return;
         }
         
         $USR_assoclib = new fAssocUpdate();
         $ro_opt=array('mothertab'=>'DB_USER');
         $USR_assoclib->setObj( $sqlo, 'USER_ROLES', $user_id, $ro_opt );
         
         $argu=array('DB_USER_ID'=>$user_id);
         $this->ROL_assoclib->insert($sqlo, $argu);
         $this->ROL_cnt++;
         
         $USR_assoclib->touch_test($sqlo, "new", array());
         $USR_assoclib->close($sqlo);
     }
     
     function remove_user_role($sqlo, $user_id) {
         
         $USR_assoclib = new fAssocUpdate();
         $ro_opt=array('mothertab'=>'DB_USER');
         $USR_assoclib->setObj( $sqlo, 'USER_ROLES', $user_id, $ro_opt );
         
         $idarr=array('DB_USER_ID'=>$user_id);
         $this->ROL_assoclib->delOneRow($sqlo, $idarr);
         $this->ROL_cnt++;
         
         $USR_assoclib->touch_test($sqlo, "new", array());
         $USR_assoclib->close($sqlo);
     }
     
     function close($sqlo) {
         if ($this->UR_cnt)  $this->UR_assoclib->close($sqlo);
         if ($this->ROL_cnt) $this->ROL_assoclib->close($sqlo);
     }
 }
 