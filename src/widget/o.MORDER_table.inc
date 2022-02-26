<?php
require_once 'gui/o.MORDER.gui.inc';

// o.MORDER_table.inc
class o_MORDER_table_XWI extends f_widget_ABS {
    
    function register() {
        
        $tablename='MORDER';
        $title		= tablename_nice2($tablename).' special view';
        $this->infoarr			   = array();
        $this->infoarr['title']    = $title;
        $this->infoarr['t'] = $tablename;
        $this->infoarr['accCheck'] = array( 'tab'=>array('read') );
        
        /**
         *  'min_status'=>'beauftragt',
    	    'suas'=> array($bead_sua_main), 
    	    'last_days'=> 10,
    	    'cols' => array(
    	        'action'         => 1,
    	        'SUC.prog.graph' => 1,
    	        'SUA_html'       => -1
    	    )
         */
        $this->infoarr['in_arr_def'] = array(
             array(
                 'var'  => 'min_status',
                 'title'=> 'minimum audit status',
                 'type' => 'STRING',
                 'notes'=> 'e.g. reviewed',
                 'req'  => FALSE
             ),
             array(
                'var'  => 'suas',
                'title'=> 'MAA-IDs',
                'type' => 'LIST_OF_INT',
                'notes'=> 'KOMMA separated MAA-IDs',
                'req'  => FALSE
            ),
            array(
                'var'  => 'last_days',
                'title'=> 'number of shown DAYs',
                'type' => 'INT',
                'notes'=> 'e.g. 10',
                'req'  => TRUE
            ),
            array(
                'var'  => 'sua.plan.step_min',
                'title'=> 'Filter: One of the Planned Protocols',
                'type' => 'LIST_OF_INT',
                'notes'=> 'Show only only with one of these planned MAA-Proto-Log-Steps. KOMMA separated STEP-numbers of planned protocols',
                'req'  => FALSE
            ),
            
            array(
                'var'  => 'cols',
                'title'=> 'Columns',
                '_dict_'=> array(
                    array(
                        'var'  => 'action',
                        'title'=> 'Action',
                        'type' => 'INT',
                        'notes'  => '-1 or 1'
                    ), 
                    array(
                        'var'  => 'sub.graph' ,
                        'title'=> 'Sub-Orders Graph',
                        'type' => 'INT',
                        'notes'  => '-1 or 1'
                    ),
                    array(
                        'var'  => 'SUC.prog.graph',
                        'title'=> 'MAC progress graph',
                        'type' => 'INT',
                        'notes'  => '-1 or 1'
                    ),
                    array(
                        'var'  => 'SUC.storage',
                        'title'=> 'MAC storage info',
                        'type' => 'INT',
                        'notes'  => '-1 or 1'
                    ),
                    array(
                        'var'  => 'SUA_html',
                        'title'=> 'MAA Name',
                        'type' => 'INT',
                        'notes'  => '-1 or 1'
                    ),
                    array( 
                        'var'  => 'prodDate_sh',
                        'title'=> 'Production date',
                        'type' => 'INT',
                        'notes'  => '-1 or 1'
                    ),
                    array(
                        'var'  => 'ord.planned_steps',
                        'title'=> 'Planned Protocols',
                        'type' => 'INT',
                        'notes'  => '-1 or 1'
                    ),
                    array(
                        'var'  => 'orderer',
                        'title'=> 'Order person',
                        'type' => 'INT',
                        'notes'  => '-1 or 1'
                    ),
                     
                ),

            ),
          
            
            array(
                'var'  => 'suc.proto.filter',
                'title'=> 'MAC.protocol.filter',
                '_dict_'=> array(
                    array(
                        'var'  => 'stepno',
                        'title'=> 'Stepno',
                        'type' => 'INT',
                        'notes'  => 'No of MAC.protocol stepno'
                    ),
                    array(
                        'var'  => 'prc.minstatus.arr' ,
                        'title'=> 'Audit status names',
                        'type' => 'LIST_OF_STRING',
                        'notes'  => 'tech_ready, ... (komma separated)'
                    ),
                )
             )
         );

    }
    
    function startMain() {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        $sqlo   = &$this->_sqlo;
        $sqlo2 = &$this->_plug_regSql2();

//         $mord_opt= array(
//             'min_status'=>'beauftragt',
//             'suas'=> array(1247),
//             'last_days' => 10,
//             'cols' => array(
//                 'action'       =>1,
//                 'sub.graph'    =>1,
//                 'sub.SUCs.nice'=>1,
//                 'SUA_html'     => -1
//             )
//         );

        $mord_opt =  $this->__in_arr;
        if ($mord_opt['suas']) { // repair SUAs
            $tmp=$mord_opt['suas'];
            $mord_opt['suas']=explode(',',$tmp);
        }
        if (empty($mord_opt['suc.proto.filter']['prc.minstatus.arr']) ) { 
            unset($mord_opt['suc.proto.filter']);
        } else {
            // repair SUAs
            $tmp=$mord_opt['suc.proto.filter']['prc.minstatus.arr'];
            $mord_opt['suc.proto.filter']['prc.minstatus.arr']=explode(',',$tmp); 
        }
        
        $tmp=trim($mord_opt['sua.plan.step_min']);
       
        if ($tmp) {
            $mord_opt['sua.plan.step_min'] = explode(',',$tmp);
        }
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
            debugOut('mord_opt: <pre>'.print_r($mord_opt,1).'</pre>', $FUNCNAME);
        }
        
        $helplib = new oMORDER_gui_table();
        $helplib->show($sqlo, $sqlo2, $mord_opt );
    }
    
}
