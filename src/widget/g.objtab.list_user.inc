<?php
require_once ("gui/g.objtab.list_user_lib.inc");

// g.objtab.list_user.inc
class g_objtab_list_user_XWI extends f_widget_ABS {
    
    function register() {
        
        $title		= g_objtab_list_user_lib::TOOL_NAME . ' of objects';

        $this->main_lib=NULL;
        
        $this->infoarr			   = array();
        $this->infoarr['title']    = $title;
        $this->infoarr['in_arr_def'] = array(
             array(
                 'var'  => 'docid',
                 'title'=> 'document ID',
                 'type' => 'INT',
                 'req'  => TRUE
             )
        );
        
        //FUTURE: check this in check_input ...
        //         if ($tablename!=NULL) {
        //             $this->infoarr2 = array(
        //                 'accCheck' => array('tab'=>array('read') ),
        //             );
        //         }
    }

    
    // check doc_id
    // extend the title ...
    function check_input() {
        $docid =  $this->__in_arr['docid'];
        if ($docid) {
            $tablename=NULL;
            $this->main_lib = new g_objtab_list_user_lib($this->_sqlo, $docid);
            $json_arr = $this->main_lib->get_congif();
            if (!empty($json_arr)) {
                $tablename = $json_arr['t'];
            }
            $doc_name = obj_nice_name ( $this->_sqlo, 'LINK', $docid );
            $extra_title = ': '.$doc_name;
            
            $this->infoarr['t'] = $tablename;
            $this->infoarr['title'] .= $extra_title;
        }
    }
    
    function startMain() {
        global $error;

        $sqlo   = &$this->_sqlo;
        $sqlo2 = &$this->_plug_regSql2();

        if ($this->main_lib===NULL) throw new Exception('main_lib not registered.');
        $this->main_lib->init_table($sqlo);
        $this->main_lib->show($sqlo, $sqlo2);
    }
    
}