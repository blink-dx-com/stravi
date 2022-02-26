<?php
require_once ("gui/o.CHIP_READER.statusView.inc");

// o.CHIP_READER.status.inc
class o_CHIP_READER_status_XWI extends f_widget_ABS {
    
    function register() {
        
        $tablename='CHIP_READER';
        $tablenice=tablename_nice2($tablename) ;
        $title		= $tablenice.' status list';

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
        $main_lib = new oCHIP_READER_statusView();
        $title_opt=NULL;
        $main_lib->showLogList($sqlo, $proj_id, $title_opt);
    }  
}
