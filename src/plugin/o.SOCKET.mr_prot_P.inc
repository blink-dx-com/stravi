<?php
/**
 * compare all SUCs on a SOCKET
 * @package o.SOCKET.mr_prot_P.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   int $id
 * @param   $t  OPTIONAL tablename
 */

require_once 'gui/o.SOCKET.mr_prot_show.inc';

class o_SOCKET_mr_prot_P_XPL extends gPlugin {
    
    
    function register() {
        
        $tablename_s	= 'SOCKET';
        
        $tablename	= 'SOCKET';
        if ($_REQUEST['t']) {
            $tablename	= $_REQUEST['t'];
        }
        
        
        $title		= tablename_nice2($tablename_s). ' Component Protocol Compare';
        
        $this->infoarr			 = NULL;
        $this->infoarr['title']    = $title;
        $this->infoarr['form_type']= 'obj';
        $this->infoarr['obj_name'] = $tablename;
        $this->infoarr['obj_id']   = $_REQUEST['id'];
        $this->infoarr['checkid']  = 1;
       
        
        $this->infoarr2 = array(
            'accCheck' => array('tab'=>array('write'), 'obj'=>array('read') )
        );
    }
    
   
    
    
    function startMain() {
        global $error;
        $FUNCNAME='startMain';
        
        $tablename_s = 'SOCKET';
        $tablename	 = 'SOCKET';
        if ($_REQUEST['t']) {
            $tablename	= $_REQUEST['t'];
        }
        $this->tablename = $tablename;
        
        $sqlo = &$this->_sqlo;
       
        $this->objid   = $_REQUEST['id'];
        $go     = $_REQUEST['go'];
        $parx   = $_REQUEST['parx'];
        $step_no= $_REQUEST['step_no'];
        
        
        
        if (!in_array($tablename, array('EXP','SOCKET')) ) {
            $this->htmlFoot('ERROR', 'This table is not supprted.');
        }
        
        // $o_rights = access_check($sqlo, $tablename, $this->objid);
        
//         if(!$step_no) {
//             $this->htmlFoot('ERROR', 'Input step_no missing.');
//         }
        
        if ($_REQUEST['action']=='settings') {
            if (!$go) {
                $hiddenarr=array( 't'=>$this->tablename, 'id'=>$this->objid, 'mod'=>$this->_mod );
                $tmp = o_CONCRETE_PROTO_li_show1::settings_get();
                o_CONCRETE_PROTO_li_show1::form_settings($tmp, $step_no, $hiddenarr);
                return;
            } else {
                o_CONCRETE_PROTO_li_show1::settings_save($parx);
                $go = 0;
            }
        }

        $form_url = $this->_plug_get_URL() . '&id='.$this->objid.'&t='.$this->tablename.'&step_no='.$step_no;
        $tool_opts = o_CONCRETE_PROTO_li_show1::settings_get();
        $tool_opts['form_url'] = $form_url;
        
        $socket_id = $this->objid;
        	
        if ($tablename=='EXP') {
            $socket_id = glob_elementDataGet( $sqlo, $tablename, 'EXP_ID', $this->objid, 'SOCKET_ID');
            if(!$socket_id) {
                $this->htmlFoot('INFO', 'The '.tablename_nice2($tablename). 
                    ' needs a '.tablename_nice2($tablename_s).' to use this tool.');
            }
        }
        
        $help_lib = new o_SOCKET_mr_prot_show($socket_id);
        $help_lib->show($sqlo, $tool_opts);
        
        
    }
}
