<?php
require_once ("gui/o.CONTAINER.overview.inc");

// o.CONTAINER.overview_W.inc
class o_CONTAINER_overview_W_XWI extends f_widget_ABS {
    
    function register() {
        
        $tablename='CONTAINER';
        $tablenice=tablename_nice2($tablename) ;
        $title		= $tablenice.' overview';

        $this->infoarr			   = array();
        $this->infoarr['title']    = $title;
        $this->infoarr['t'] = $tablename;
        $this->infoarr['accCheck'] = array( 'tab'=>array('read') );
        $this->infoarr['in_arr_def'] = array(
             array(
                 'var'  => 'proj_id',
                 'title'=> 'folder ID',
                 'type' => 'INT',
                 'req'  => TRUE,
                 'notes'=>'folder containing the '.$tablenice.'s'
             )
        );

    }

    // extend the title ...
    function check_input() {
        $proj_id =  $this->__in_arr['proj_id'];
        if (!$proj_id) { 
            throw new Exception('Input: ID of '.tablename_nice2('PROJ') .' not given.');
        }
        $proj_name = obj_nice_name ( $this->_sqlo, 'PROJ', $proj_id );
        $extra_title = ': '.$proj_name;

        $this->infoarr['title'] .= $extra_title;
    }
    
    function startMain() {
        $sqlo   = &$this->_sqlo; 
        $proj_id =  $this->__in_arr['proj_id'];
        $cont_lib = new oCONTAINER_overview($sqlo);
        $cont_lib->show_Dev_Cont_MAA($sqlo, $proj_id);
    }
    
}
