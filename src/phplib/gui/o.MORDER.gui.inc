<?php
/**
 * MORDER standard  table
 * @package o.MORDER.gui.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */

require_once 'o.S_OBJLINK.subs.inc';
require_once ("f.objview.inc");
require_once 'visufuncs.inc';
require_once 'object.info.inc';
require_once 'f.progressBar.inc';
require_once 'o.MORDER.network.inc';
require_once 'o.MORDER.subs.inc';
require_once 'o.MORDER.proto_def.inc';
require_once 'o.ABSTRACT_SUBST.proto.inc';
require_once 'o.CONCRETE_SUBST.proto.inc';
require_once 'o.CONTAINER.subs.inc';


/**
 * helper class for MORDER.SUC
 */
class oMORDER_SUC_subs {
    function __construct($suc_id) {
        $this->suc_id=$suc_id;
    }
    
    /**
     * analyse protocol of SUC
     *  'stepno' => int -- this step number must have a protocol
        'prc.minstatus.arr'
     * @param object  $sqlo
     */
    function analyse_stepno_filter($sqlo, array $x_filt) {
        
        $show_flag = 0;
        $suc_id    = $this->suc_id;
        $prot_lib    = new oCONCRETE_SUBST_proto($suc_id);
        $c_proto_log = $prot_lib->get_c_protos($sqlo);
        
        if (!$x_filt['stepno']) throw new Exception('Input missing: "stepno"');
        if ( empty($x_filt['prc.minstatus.arr']) ) throw new Exception('Input missing: "prc.minstatus.arr"');
        
        $stepno = $x_filt['stepno'];
        foreach($c_proto_log as $row) {
            if ($row['or']==$stepno) {
                // analyse status of prc
                $prc = $row['cp'];
                // status name get
                $acc_info = access_data_get($sqlo, 'CONCRETE_PROTO', 'CONCRETE_PROTO_ID', $prc);
                $status_name = $acc_info['h_log_name'];
                if ($status_name==NULL) break;
                if (in_array($status_name, $x_filt['prc.minstatus.arr'])) $show_flag=1;
                
                break;
            }
        }
        return $show_flag;
    }
}

/**
 * 
 * MORDER GUI class
 *
 */
class oMORDER_gui_table {
    
    /**
     * 
     * @var array sh_options_STRUCT
     *  'PROD_LINE_ID' : ID of production line (CHIP_READER_ID)
     *  'min_status'   : name of minimal status, e.g. "beauftragt"
     *  'status_arr'   : array of status names
     *  'last_days'    : number of days to show
     *  'cols' => array(KEY=>0,1)
     *      'action'   : [1] , -1
     *      'sub_order'  : [-1], 0, 1 -- show suborders ?
     *
     *  'suas' => array of SUA-IDs : show only these SUAs; e.g. array(234, 456, 34)
     *       if contains only ONE SUA: do not show the MAA column ...
     *  'suc.proto.filter' => array( -- show only orders, which fulfill this filter ...
            *        'stepno' => int -- this step number must have a protocol
            *        'prc.minstatus.arr' => array of string e.g. 'tech_ready'
            *   )
     *  'sua.plan.step_min' => array of STEPs; show only orders, which have planned steps
     *      defined in MORDER.VARIO.SUA.protos_use 
     *  
     *  )
     */
    private $sh_options;
    private $_used_cols; // column structure
    private $_container_cache;
   
    
    function __construct() {
        $this->audit_status_arr = array(
            1=>'started',
            2=>'beauftragt',
            3=>'reviewed',
            4=>'prod_start',
            5=>'abgeschlossen',
        );
        
        $this->_container_cache = array();
    }
    
    private function status_arr2ID($sqlo, $status_names) {
        $ids=array();
        foreach($status_names as $name) {
            $sid = glob_elementDataGet2( $sqlo, 'H_ALOG_ACT', array('NAME'=>$status_names), 'H_ALOG_ACT_ID' );
            $ids[$name]=$sid;
        }
        return $ids;
    }
    
    private function status2ID($sqlo, $name) {
        $sid = glob_elementDataGet2( $sqlo, 'H_ALOG_ACT', array('NAME'=>$name), 'H_ALOG_ACT_ID' );
        return $sid;
    }
    
    public function init_progress_bar() {
        $prg_opt=array('imgmaxlen'=>100);
        
        // MORDER progress
        $max_status = sizeof($this->audit_status_arr);
        $this->progress_lib = new fProgressBar($max_status, $prg_opt); 
 
        $max_steps = 6;
        
        // SUC progress
        $this->suc_progress_sua_init = 0;
        $this->suc_progress_lib = new fProgressBar($max_steps, $prg_opt); 
        echo '<style>'."\n";
        echo $this->progress_lib->getCssStatic();
        echo '</style>'."\n";
    }
    
    /**
     * progress bar
     * @param object $sqlo
     * @param int $suc_id
     * @param int $sua_id
     */
    private function  _SUC_prog_graph($sqlo, $suc_id, $sua_id) {
        
        if (!$this->suc_progress_sua_init) {
            // set the progress bar length
            $sua_protos = new oABSTRACT_SUBST_proto();
            $sua_protos->set_obj($sqlo, $sua_id);
            $a_proto_log = $sua_protos->get_protos($sqlo);
            $pra_len= sizeof($a_proto_log);
            $prog_opt=array('maxnum'=>$pra_len);
            $this->suc_progress_lib->setNewLimits($prog_opt);
            $this->suc_progress_sua_init = 1;
        }
        $prot_lib = new oCONCRETE_SUBST_proto($suc_id);
        $c_proto_log = $prot_lib->get_c_protos($sqlo);
        $prc_len     = sizeof($c_proto_log);

        $output = $this->suc_progress_lib->getBarStatic( $prc_len );
        return $output;
    }
    
    /**
     * progress bar : MORE details
     * @param object $sqlo
     * @param int $suc_id
     * @param int $sua_id
     */
    private function  _SUC_prog_graph2($sqlo, $suc_id, $sua_id, $order_id) {
        
       
        $order_def_lib = new oMORDER_proto_def($order_id);
        $order_def=$order_def_lib->get_data($sqlo);
        
        
        if (empty($this->suc_progress_digits)) {
            // set the progress bar length
            $sua_protos = new oABSTRACT_SUBST_proto();
            $sua_protos->set_obj($sqlo, $sua_id);
            $a_proto_log = $sua_protos->get_protos($sqlo);
            $pra_len= sizeof($a_proto_log);

            $this->suc_prog_digits = array(
                'a_proto_log'=>$a_proto_log,
                'maxnum'=>$pra_len
            );
        }
        $prot_lib = new oCONCRETE_SUBST_proto($suc_id);
        $c_proto_log = $prot_lib->get_c_protos($sqlo);
        
        $a_log = &$this->suc_prog_digits['a_proto_log'];
        
        $digits=array();
        foreach($a_log as $a_row) {
            
            $row_out=array();
            $pra_id = $a_row['ap'];
            $p_order= $a_row['or'];
            
            $found=0;
            foreach($c_proto_log as $c_row) {
                if ($c_row['ap']==$pra_id) {
                    $found=1;
                    break;
                }
            }

            $status_name='';
            // analyse prc
            if ($found) {
                
                $color='yellow';
                
                $prc_id = $c_row['cp'];
                
                $CCT_ACCESS_ID = glob_elementDataGet( $sqlo, 'CONCRETE_PROTO', 'CONCRETE_PROTO_ID', $prc_id, 'CCT_ACCESS_ID');
                $H_ALOG_ACT_ID = glob_elementDataGet( $sqlo, 'CCT_ACCESS', 'CCT_ACCESS_ID', $CCT_ACCESS_ID, 'H_ALOG_ACT_ID');
                if ($H_ALOG_ACT_ID) {
                    $status_name = oH_ALOG_ACT_subs::statusID2Name($sqlo, $H_ALOG_ACT_ID);
                    $color='gray';
                    if ($status_name=='tech_ready') $color='#336699';
                }
                $row_out['c']=$color;
            }
            
            if (!empty($order_def)) {
                if ($order_def[$p_order]) {
                    if (!$found) {
                        $row_out['c']='#D0D0FF';
                    }
                }
            }
            
            $digits[] = $row_out;
        }

        $options=array(
            'imgmaxlen' =>300,
            'img_height'=>20
        );
        
        $output = fProgressBar::getBarStaticDigits($digits, $options);
        
        
        return $output;
    }
    
    /**
     * 
     * @param object $sqlo
     * @param int $order_id
     * @return array
     *   'SUCs' array of SUCs
     *   'status_prog_index'
     */
    private function basic_features($sqlo, int $order_id) {
        
        $order_lib = new oMORDER_subs($order_id);
        $prod_SUCs  = $order_lib->get_prod_SUCs($sqlo); 
        
        $objInfoLib = new objInfoC();
        $MORDERer_feats = $objInfoLib->getParamsRaw ( $sqlo, 'MORDER', $order_id);
        
        $cyt_status_id = $MORDERer_feats["access"]['H_ALOG_ACT_ID'];
        if ($cyt_status_id) $cyt_status   = glob_elementDataGet( $sqlo, 'H_ALOG_ACT', 'H_ALOG_ACT_ID', $cyt_status_id, 'NAME' );
        else $cyt_status = NULL;
        
        $status_prog_index = array_search($cyt_status, $this->audit_status_arr);
        $MORDERer_feats['status_prog_index'] = $status_prog_index;
        $MORDERer_feats['SUCs'] = $prod_SUCs;
        
        return $MORDERer_feats;
        
    }
    
    /**
     * at least one of the given $step_arr_in must be in the MORDER
     * @param array $step_arr_in
     */
    private static function _MORDER_ana_planned_step(array $step_arr_in, array $step_arr_now) {
        $found=0;
        
        foreach($step_arr_in as $step_nr) {
           
            if (in_array($step_nr, $step_arr_now)) {
                $found=1;
                break;
            }
        }
        return $found;
    }
    
    private function _get_store_info($sqlo, $suc_id) {
        $storage_str='';
        $containers  = oCONTAINER_many::get_one_SUC_stores($sqlo, $suc_id);
        if (!empty($containers)) {
            
            $komma='';
            foreach($containers as $cont_id) {
                $conts_tmp=array($cont_id);
                $quant = oCONTAINER_many::get_one_SUC_QUANTITY( $sqlo, $suc_id, $conts_tmp );
                
                if ($this->_container_cache[$cont_id]!=NULL) {
                    $cont_name = $this->_container_cache[$cont_id];
                } else {
                    $cont_name = obj_nice_name ( $sqlo, 'CONTAINER', $cont_id );
                    $this->_container_cache[$cont_id] = $cont_name;
                }
                $storage_str .= $komma . $cont_name.': '.$quant;
                $komma='; ';
            }
        }
        return $storage_str;
    }
    
    private function  _SUC_analyse($sqlo, $suc_id) {
        
        $feat_now = glob_elemDataGet3( $sqlo, 'CONCRETE_SUBST', array('CONCRETE_SUBST_ID'=>$suc_id), 
            array('ABSTRACT_SUBST_ID', 'NAME', 'QUANTITY', 'SOME_DATE', 'NOTES') );
        
        if ($this->_used_cols['SUC.storage']) {
            // analyse storage
            $feat_now['storage.info']=$this->_get_store_info($sqlo, $suc_id);
        }
        
        return $feat_now;
    }
    
    /**
     * 
     * @param object $sqlo2
     * @param int $order_id
     * @param int $suc_id
     * @return array
     *      '__SHOW__' : 0,1 -- show this entry ?
     *      '__INFO__' : internal info
     *      'MORDERHtml'
     *      'SUA_ID'    -- planned SUA
            'SUA_html', -- planned SUA
            'order_status.graph',
            'sub.graph' -- graph of sub-orders
            'sub.SUCs.nice'  -- nice list sub-orders.SUCs
            'SUCHtml',
            
            'order_status',
            'QUANTITY'  : planned produced quant
            'orderer',
            'ord.planned_steps'
            'prodDate_sh',
            'notes'  MAC.notes
            'action'
            'SUC.prog.graph'
            'SUC.prog.graph2' : with details
     */
    public function get_features(&$sqlo, $order_id) {
        
        $nrOfChars = 24;
        
        $usagearr=array(
            1=>"Produktion",
            2=>"Entwicklung",
            3=>"Forschung",
        );
        
        $ord_features =  array();
        $ord_features['__SHOW__']=1;
        
        $obj_view_lib = new fObjViewC();
        $objLinkLib2  = new oS_OBJLINK_subs();
        $suc_arr = $objLinkLib2->getChiObjByChiTab($sqlo , 'MORDER', $order_id, 'CONCRETE_SUBST');
        
        
        $suc_raw_features=array();
        $suc_id=0;
        
        if (!empty($suc_arr)) {
            
            $suc_id = current($suc_arr); // first SUC
            $first_suc_lib = new oMORDER_SUC_subs($suc_id);

            foreach($suc_arr as $suc_loop) {       
                $suc_raw_features[] = $this->_SUC_analyse($sqlo, $suc_loop); 
            }
        }
        
        if (!empty($this->sh_options['suc.proto.filter'])) {
            $show_flag = 0; // is now default ...
            if ($suc_id) {
                $x_filt = &$this->sh_options['suc.proto.filter'];
                $show_flag = $first_suc_lib->analyse_stepno_filter($sqlo, $x_filt);
            }
            if (!$show_flag) {
                $ord_features['__SHOW__']=0;
                return $ord_features;
            }
        }
        
        //$orderer	= 'Unknown';
        $ord_features['MORDERHtml'] = $obj_view_lib->bo_display($sqlo, 'MORDER', $order_id);
        
        
        if (!empty($this->sh_options['sua.plan.step_min']) ) {
            
            $order_proto_lib = new oMORDER_proto_def($order_id);
            $proto_use_def = $order_proto_lib->get_data($sqlo);
            $proto_steps_def = array_keys($proto_use_def);
            
            $ord_features['ord.planned_steps'] = implode(', ',$proto_steps_def);
            
            $found_flag = 0; // is now default ...
            $found_flag = self::_MORDER_ana_planned_step($this->sh_options['sua.plan.step_min'], $proto_steps_def);
            if (!$found_flag) {
                $ord_features['__SHOW__']=0;
                $ord_features['__INFO__']='No expected STEP-Nr found.';
                return $ord_features;
            }
        }
        
        $objInfoLib = new objInfoC();
        $MORDERer_feats = $objInfoLib->getParamsRaw ( $sqlo, 'MORDER', $order_id);
        
        
        $DB_USER_ID = $MORDERer_feats["access"]['DB_USER_ID'];
        $usernick   = obj_nice_name ( $sqlo, 'DB_USER', $DB_USER_ID );
        
        $cyt_status_id = $MORDERer_feats["access"]['H_ALOG_ACT_ID'];
        if ($cyt_status_id) $cyt_status   = glob_elementDataGet( $sqlo, 'H_ALOG_ACT', 'H_ALOG_ACT_ID', $cyt_status_id, 'NAME' );
        else $cyt_status = NULL;
        
        $status_prog_index = array_search($cyt_status, $this->audit_status_arr);
        $ord_features['order_status.graph'] = $this->progress_lib->getBarStatic( $status_prog_index );
        
        $ord_features['orderer'] = $usernick;
        $ord_features['SUA_ID']  = $MORDERer_feats['vals']['ABSTRACT_SUBST_ID'];
        $ord_features['CERT_FLAG']=$MORDERer_feats['vals']['CERT_FLAG'];
        $ord_features['usage']	  =$usagearr[$ord_features['CERT_FLAG']];
        $ord_features['QUANTITY']	  =$MORDERer_feats['vals']['QUANTITY'];
        $ord_features['order_status'] = $cyt_status;
        
        if ($ord_features['SUA_ID'])
            
            //  bo_display($sqlo, 'ABSTRACT_SUBST', $ord_features['SUA_ID']);
            $ord_features['SUA_html'] = $obj_view_lib->bo_nice($sqlo, 'ABSTRACT_SUBST', $ord_features['SUA_ID']);
 
            
            if ($suc_id) {
                
                $ord_features['prodDate']	= $suc_raw_features[0]['SOME_DATE'];
                $ord_features['prodDate_sh'] =	substr($ord_features['prodDate'],0,10);
                $notes				= $suc_raw_features[0]['NOTES'];
                
                
                if (strlen($notes)>$nrOfChars)  $notes = substr($notes, 0, $nrOfChars-4).' ...';
                $ord_features['notes']		= $notes;
                
                $komma='';
                $br_tmp='';
                $ord_features['SUCHtml']='';
                $suc_opt=array('showID'=>1);
                foreach($suc_arr as $i=> $suc_loop) {
                    $ord_features['SUCHtml'] .= $komma . $obj_view_lib->bo_display( $sqlo, 'CONCRETE_SUBST', $suc_loop, $suc_opt );
                    
                    if ($this->_used_cols['SUC.storage']) {
                        $ord_features['SUC.storage'] .= $br_tmp . $suc_raw_features[$i]['storage.info'];
                    }
                    $komma=', ';
                    $br_tmp='<br>';
                }
                
              
                
                
                if ($ord_features['order_status']!='abgeschlossen') {
                    $new_status = 'abgeschlossen';
                    $ord_features['action'] = '<a href="glob.obj.acclock.php?t=MORDER&id='.$order_id.'&parx[statName]='.$new_status.'">'.
                        '<img src="images/i13_ok2.gif" title="Bestellung abschliesseng"></a>';
                }
                if ($ord_features['order_status']=='abgeschlossen') {
                    $ord_features['action'] = ''.
                        '<img src="images/icon.MORDER.finish.png" title="Production ist fertig">';
                }
                
            } else {
                
                // STATUS ?
                $audit_change_URL = 'glob.obj.acclock.php?t=MORDER&id='.$order_id.'&parx[statName]=';
                
                if ($ord_features['order_status']=='started') {
                    $ord_features['action'] = '<a href="'.$audit_change_URL.'beauftragt">'.
                        '<img src="images/icon.MORDER.send.png" title="Bestellung senden"></a>';
                }
                
                if ($ord_features['order_status']=='beauftragt') {
                    $ord_features['action'] = '<a href="'.$audit_change_URL.'reviewed">'.
                        '<img src="images/icon.MORDER.take.png" title="Bestellung &uuml;bernehmen"></a>';
                }
                
                if ($ord_features['order_status']=='reviewed') {
                    $ord_features['action'] = '<a href="'.$audit_change_URL.'prod_start">'.
                        '<img src="images/ic13.action.png" title="Start Production"></a>';
                }
                
                if ($ord_features['order_status']=='abgeschlossen') {
                    $ord_features['action'] = ''.
                        '<img src="images/icon.MORDER.finish.png" title="Production ist fertig">';
                }

                
            }
            
            if ($suc_id and is_array($this->_used_cols['SUC.prog.graph'])) {
                // get progress for FIRST  SUC
                $ord_features['SUC.prog.graph'] = $this->_SUC_prog_graph($sqlo, $suc_id, $ord_features['SUA_ID']);  
            }
            if ($suc_id and is_array($this->_used_cols['SUC.prog.graph2'])) {
                // get progress for FIRST  SUC
                $ord_features['SUC.prog.graph2'] = $this->_SUC_prog_graph2($sqlo, $suc_id, $ord_features['SUA_ID'], $order_id);
            }
            
            $do_sub_orders=1;
            if ($do_sub_orders) {
                
                // analyse SUB-ORDERS
                // get 'sub.graph'
                $sub_lib = new oMORDER_network($order_id);
                $sub_ords = $sub_lib->get_orders($sqlo);
                if (!empty($sub_ords)) {
                    
                    $sucs = array();
                    $min_status_ind = sizeof($this->audit_status_arr);
                    // get LOWEST status
                    foreach($sub_ords as $sub_id)  {
                        
                        $sub_feats = $this->basic_features($sqlo, $sub_id);
                        if (is_array($sub_feats['SUCs'])) $sucs = array_merge($sucs,$sub_feats['SUCs']);
                        
                        if ($sub_feats['status_prog_index']<$min_status_ind) {
                            $min_status_ind = $sub_feats['status_prog_index'];
                        }
                    }
                    $ord_features['sub.graph'] = $this->progress_lib->getBarStatic( $min_status_ind );
                    if (!empty($sucs))
                    $ord_features['sub.SUCs.nice']  = implode(", ",$sucs);
                    
                }
            }
            
            return $ord_features;
    }
    
    private function _status_names2id( $sqlo, $status_arr ) {
        $include_sids=array();
        foreach($this->audit_status_arr as $loop_sname) {
            if ( in_array($loop_sname,$status_arr) ) {
                $include_sids[] = $this->status2ID($sqlo, $loop_sname);
            }
        }
        return $include_sids;
    }
    
    /**
     * 
     * @param object $sqlo
     * @param int $nrOfLookupDays
     * @param array $sh_options
     * @return string
     */
    private function _build_sql( $sqlo, $nrOfLookupDays, $sh_options ) {
        
        $sqlsel	=	'x.MORDER_ID'.
            ' from MORDER x, CCT_ACCESS a'.
            ' where x.CCT_ACCESS_ID = a.CCT_ACCESS_ID'.
            ' and a.CREA_DATE >= (CURRENT_DATE-'.$nrOfLookupDays.')';
        
        if ($sh_options['days_start_min']>0) {
            $sqlsel .= ' and a.CREA_DATE < (CURRENT_DATE-'.$sh_options['days_start_min'].')';
        }
        
        if ($sh_options['PROD_LINE_ID']) {
            $sqlsel .= ' and x.PROD_LINE_ID=' . $sh_options['PROD_LINE_ID'];
        }
        if ($sh_options['suas']) {
            $sqlsel .= ' and x.ABSTRACT_SUBST_ID in ('.implode(', ',$sh_options['suas']).')';
        }
        
        if ($sh_options['min_status']) {
            
            // show only minimum statsu ....
            $min_name = $sh_options['min_status'];
            // $min_sid = $this->status2ID($min_name);
            
            $takeit=0;
            $include_sids=array();
            foreach($this->audit_status_arr as $loop_sname) {
                if ($loop_sname==$min_name) {
                    $takeit=1;
                }
                if ($takeit) {
                    $include_sids[] = $this->status2ID($sqlo, $loop_sname);
                }
            }
            
            if (!empty($include_sids))
                $sqlsel .= ' and a.H_ALOG_ACT_ID in ('.implode(',',$include_sids).')'; 
        }
        if (is_array($sh_options['status_arr'])) {
            // take only from these statuses ..
            $include_sids = $this->_status_names2id( $sqlo, $sh_options['status_arr'] );
            $sqlsel .= ' and a.H_ALOG_ACT_ID in ('.implode(',',$include_sids).')';
        }
        
        return $sqlsel;
    }
    
    /**
     * show list of selected MORDERs
     * @param object $sqlo
     * @param object $sqlo2
     * @param array $sh_options sh_options_STRUCT
     */
    function show($sqlo, $sqlo2, $sh_options=array() ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $nrOfLookupDays = 30;
        $table = 'MORDER';
        $title_mess_add='';
        $this->sh_options = $sh_options;
        $sua_show_num=0;
        
        
        if ( $sh_options['last_days']>0 ) {
            $nrOfLookupDays = $sh_options['last_days'];
        }
        if (!empty($this->sh_options['sua.plan.step_min'])) {
            if (!is_array($this->sh_options['sua.plan.step_min'])) {
                echo "CONFIG-ERROR: sh_options[sua.plan.step_min] must be of type ARRAY.<br>\n";
                return;
            }
            $title_mess_add .=' AND only orders with one of planned-protocol-steps:' . implode(', ',$sh_options['sua.plan.step_min']);
        }

        $this->init_progress_bar();

        $cnt = 0;
        $list_url='view.tmpl.php?t='.$table.'&condclean=1';
        if ($sh_options['PROD_LINE_ID']) {
            $list_url .= '&searchCol=PROD_LINE_ID&searchtxt=' . $sh_options['PROD_LINE_ID'];
            $title_mess_add .=' AND Produktion-line: EQU-ID:' . $sh_options['PROD_LINE_ID'];
            
        }
        if ($sh_options['suas']) {
            $list_url .= ''; //TBD
            $title_mess_add .=' AND MAAs:'. implode(', ',$sh_options['suas']);
            $sua_show_num = sizeof($sh_options['suas']);
            if ($sua_show_num==1) {
                $sua_id = current($sh_options['suas']); 
                $title_mess_add .= ' '. fObjViewC::bo_display($sqlo, 'ABSTRACT_SUBST', $sua_id);
            }
        }
        if (!empty($sh_options['suc.proto.filter'])) {
            $xfilt = &$sh_options['suc.proto.filter'];
            $title_mess_add .= ' AND only orders, if MAC.protocol.ORDER-No:'.$xfilt['stepno'];
            $title_mess_add .= ' AND Audit-State:'.implode(", ",$xfilt['prc.minstatus.arr']);
        }
        
        $message = ' Orders of the last '.$nrOfLookupDays.
            ' days. '.$title_mess_add;
        
        $tabobj = new visufuncs();
        
        $headOpt = array( "title" => $message, "title_notes"=>'[<a href="'.$list_url.'">ALLE Bestellungen</a>]');
        
        
        /**
         * 
         * @var array $column_def
         * 'show'
         *   0=> no standard column
         *   1=> standard, but can be unchecked
         *   2=> standard and MUST be shown
         */
        $column_def = array(
            'MORDERHtml'=> array('name'=>'Order-Name', 'sh.ws.nowrap'=>1, 'show'=>2),
            'QUANTITY'  => array('name'=>'PQuant', 'tip'=>'Geplante Menge', 'show'=>1),
            'SUA_html'  => array('name'=>'material template', 'show'=>0),
            'order_status.graph' =>array('name'=>'Fortschritt', 'show'=>1),
            'action'    => array('name'=>'Aktion','show'=>0),
            'SUCHtml'        => array('name'=>'Batch', 'show'=>2),
            'SUC.prog.graph' => array('name'=>'MAC progress', 'show'=>0),
            'SUC.prog.graph2' => array('name'=>'MAC progress Details', 'show'=>0),
            'SUC.storage'    => array('name'=>'in storage'  , 'show'=>0),
            'sub.graph'      => array('name'=>'Sub-Orders'  , 'show'=>0),
            'sub.SUCs.nice'  => array('name'=>'Sub-Order-MACs', 'show'=>0),
            'ord.planned_steps'=>array('name'=>'Planned Protocols',  'show'=>0),
            'orderer'        => array('name'=>'Besteller', 'show'=>0),
            'prodDate_sh'    => array('name'=>'Proddatum', 'show'=>0),
            'notes'          => array('name'=>'MAC notes', 'show'=>0),
        );
        
        if (!is_array($sh_options['cols'])) $sh_options['cols']=array();
            
        // build $this->_used_cols
        $this->_used_cols = array();
        $coldef_keys=array_keys($column_def);
        foreach($coldef_keys as $key) {
            
            $show_flag_ori = $column_def[$key]['show'];
            $show_flag     = $show_flag_ori;
            
            if (array_key_exists($key, $sh_options['cols'] ) ) {
                $show_flag = $sh_options['cols'][$key];  // get show flag from user option
            }
            if ($show_flag_ori>=2) $show_flag=1;
            
            //echo "DDD: $key: $show_flag<br>";
            
            if ($show_flag>0) {
                $this->_used_cols[$key] = $column_def[$key];
            }
        }

        
        $headx = array();
        foreach($this->_used_cols as $row) {
            
            $out=$row['name'];
            if ($row['tip']) {
                $out='<span  title="'.$row['tip'].'">'.$row['name'].'</span>';
                
            }
            $headx[]=$out;
        }
        $keys  = array_keys($this->_used_cols);

        
        $tabobj->table_head($headx,   $headOpt);
            
            
        $sqlsel = $this->_build_sql( $sqlo, $nrOfLookupDays, $sh_options );
        $sqlo->Quesel($sqlsel.' order by x.MORDER_ID desc');
        
        $tr_opt=NULL;
        //    'td_style_arr'=> array('style="{white-space: nowrap;}"')  // MORDERHtml
        
        
        while($sqlo->ReadRow()) {
            
            $order_id	= $sqlo->RowData[0];

            $features = $this->get_features($sqlo2, $order_id);

            if (!$features['__SHOW__']) {
                continue;
            }
            $dataArr=array();
            foreach($keys as $dkey) {
                $dataArr[] = $features[$dkey];
            }
            $tabobj->table_row ($dataArr, $tr_opt);
            $cnt++;
            
        }
        
        //
        // and UNCLOSED older POs
        //
        $lookback_extra = 60; // 6ß days back
        $lookback = $nrOfLookupDays + $lookback_extra;
        $sh_options_new=$sh_options;
        unset($sh_options_new['min_status']);
        $sh_options_new['status_arr'] = array(  'beauftragt','reviewed','prod_start' );
        $sh_options_new['days_start_min'] = $nrOfLookupDays;
        $sqlsel = $this->_build_sql( $sqlo, $lookback, $sh_options_new );
        $sqlo->Quesel($sqlsel.' order by x.MORDER_ID desc');
        
        while($sqlo->ReadRow()) {
            
            $order_id	= $sqlo->RowData[0];
            $features = $this->get_features($sqlo2, $order_id);
            if (!$features['__SHOW__']) {
                if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
                    debugOut('(633 SHOW:0) features:'.print_r($features,1), $FUNCNAME);
                }
                continue;
            }
            
            $dataArr=array();
            foreach($keys as $dkey) {
                $dataArr[] = $features[$dkey];
            }
            $tabobj->table_row ($dataArr, $tr_opt);
            $cnt++;
            
        }
        
        $tabobj->table_close();
    }
}