<?php
/**
 * search likely ABSTRACT_PROTO
 * @package o.ABSTRACT_PROTO.likely.inc
 * @param int $id
 * @param $go
 * @param $sel
 * @param $parx
 *   selall : 0,1
 * @param $action
 *   ['show']
 *    'copy' copy objects to clipboard
 * @param
 */

require_once 'f.sql_query.inc';
require_once 'ots/Diff/class.Diff.php';
require_once 'o.ABSTRACT_PROTO.stepx.inc';
require_once ("f.objview.inc");	
require_once 'f.clipboard.inc';
require_once ('func_form.inc');


class o_ABSTRACT_PROTO_likely_ONE  {
    
    /**
     * set reference protocol
     * @param object $sqlo
     * @param int $pra_id
     * @param string $mode
     *    'FULL' search also 'QUANTITY'
     *    'SOFT' search without quantity
     */
    function __construct(&$sqlo, $pra_id, $mode='FULL') {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->infolevel=0;
        $this->ref_string = '';
        
        $this->mode= $mode;
        
        $this->diff_lib = new Diff();
        $this->aProtoLib = new  oABSTRACT_PROTO_stepx();
        
        $this->use_pra_id=$pra_id;
        $this->aProtoLib->setObj($pra_id);
        $ref_steps = $this->aProtoLib->getStepArray( $sqlo );
        
        if (!sizeof($ref_steps)) {
            $error->set( $FUNCNAME, 1, 'Protocol contains no steps' );
            return;
        }
        
        $this->proto_str_keys = array('NAME', 'ABS_DEV_NAME', 'ABSTRACT_SUBST_NAME', 'QUANTITY', 'H_UNIT_NAME', 'NOTES');
        if ($this->mode=='SOFT') {
            $this->proto_str_keys = array('NAME', 'ABS_DEV_NAME', 'ABSTRACT_SUBST_NAME', 'H_UNIT_NAME', 'NOTES');
        }
        
        
        $this->ref_string = $this->_calc_string($sqlo, $ref_steps);   
        $this->ref_step_cnt = sizeof($ref_steps);
        
        
    }
    
    static function get_proto_steps($sqlo, $pra_id) {
        $aProtoLib = new  oABSTRACT_PROTO_stepx();
        $aProtoLib->setObj($pra_id);
        $steps = $aProtoLib->getStepArray( $sqlo );
        return $steps;
    }
    
    function set_info_level($infolevel) {
        $this->infolevel=$infolevel;
    }
    
    function get_ref_str() {
        return $this->ref_string;
    }
    function get_ref_step_cnt()  {
        return $this->ref_step_cnt;
    }
    
    private function _calc_string($sqlo, $ori_steps) {
        
        
        $metainfo=1;
        $step_nice_arr=array();
        
        
        foreach($ori_steps as $row) {
            $step_nr  =$row[0];
            $one_step = $this->aProtoLib->step_info( $sqlo, $step_nr, $metainfo );
            // 'NAME', 'ABSTRACT_SUBST_ID', 'QUANTITY', 'H_UNIT_ID', 'ABS_DEV_ID', 'NOTES', 'STEP_NR' 
            // ABSTRACT_SUBST_NAME, ABS_DEV_NAME, H_UNIT_NAME
            
            $delim='';
            $one_line='';
            foreach($this->proto_str_keys as $key) {
                if ($one_step[$key]!='' and $one_step[$key]!==NULL ) {
                    $one_line .= $delim . $one_step[$key];
                    $delim=' ';
                }
            }
            $step_nice_arr[]=$one_line;
        }
        
        $out_str=implode("\n",$step_nice_arr);
        //echo "DDD:_calc_string: out_str:<pre> $out_str </pre><br><br>";
        
        if ($this->infolevel>0) {
            
            echo "INFO:_calc_string: PRA-ID:".$this->use_pra_id."  refstr:<pre>$out_str</pre><br>\n";
        }
        
        return $out_str;
    }
    
    
    /**
     * return 0, if no steps exist
     * @param object $sqlo
     * @param int $pra_id
     * @return number
     */
    function set_obj(&$sqlo, $pra_id) {
        
        $this->use_pra_id=$pra_id;
        $this->aProtoLib->setObj($pra_id);
        $this->now_steps = $this->aProtoLib->getStepArray( $sqlo );
        
        if (!empty($this->now_steps)) return 1;
        return 0;
    }
    
    function do_likely_score(&$sqlo) {
        
        if ( $this->ref_string=='') {
            throw new Exception('Class error: no reference string exists.');
        }
        
        $now_obj_str = $this->_calc_string($sqlo, $this->now_steps);

        $diff_arr = $this->diff_lib->compare($this->ref_string, $now_obj_str);
        
        if (!sizeof($diff_arr)) $score = 0;
        else {
            $score = 0;
            foreach($diff_arr as $row) {
                if ($row[1]) $score++;
            }
        }
        
        if ($this->infolevel>1) {
            echo "INFO: do_likely_score: SCORE:$score  diff_arr:<pre>".print_r($diff_arr,1)."</pre><br>";
        }
        
        //echo "DDD:do_likely_score: score:$score <br>";
        //echo "DDD:do_likely_score: diff_arr:".print_r($diff_arr,1)."<br>";
        
        return array('score'=>$score, 'step_cnt_this'=>sizeof($this->now_steps) );
    }
    
    function show_diff($sqlo) {
        
        if ( $this->ref_string=='') {
            throw new Exception('Class error: no reference string exists.');
        }
        
        $now_obj_str = $this->_calc_string($sqlo, $this->now_steps);
        
        $diff_arr = $this->diff_lib->compare($this->ref_string, $now_obj_str);
        $html_tmp = $this->diff_lib->toTable($diff_arr);
        
        echo $html_tmp;
    }
}





class o_ABSTRACT_PROTO_likely_XPL extends gPlugin {
    
    private $score_index;
    
    function register() {
        $tablename	= 'ABSTRACT_PROTO';
        $title		= 'search similar protocols';
        
        $this->infoarr			 = NULL;
       
        $this->infoarr['title']    = $title;
        $this->infoarr['form_type']= 'obj';
        $this->infoarr['design']   = 'norm';
        $this->infoarr['obj_name'] = $tablename;
        $this->infoarr['obj_id']   = $_REQUEST['id'];
        $this->infoarr['checkid']  = 1;
        
        $this->infoarr2 = array(
            'accCheck' => array('tab'=>array('read'))
        );
    }
    
    /**
     * save score in arrays
     * FUTURE: use f.scoring.inc : class fScoring()
     * @param int $score
     * @param int $pra_id
     */
    function _save_score($pra_id, $likely_info) {
        
        // $min_old = min($this->score_vals);
        $max_old = max($this->score_vals);
        $do_save = 0;
        $old_len = 0;
        
        $score = $likely_info['score'];
        $step_cnt_this = $likely_info['step_cnt_this'];
        
        
        // difference of STEP-count between REF and THIS is too big
        // THIS protocol has less than HALF number of steps than REF protocol ...
        if ( $score >= $this->ref_step_cnt and  
            $step_cnt_this < ($this->ref_step_cnt*0.5) ) {
                
            return;  
        }
        
        if (sizeof($this->score_vals) < $this->MAX_SAVE_SCORES) {
            $do_save=1;
            $old_len=sizeof($this->score_vals);
        }
        
        if ($score<$max_old or $do_save) {
            
            $cache_vals =$this->score_vals;
            $cache_index=$this->score_index;
            $arr_len= sizeof($cache_vals);
            
            foreach($this->score_vals as $index => $oldval) {
                
               
                if ($score<$oldval) {
                    // insert ...
                    $this->score_vals[$index]=$score;
                    $this->score_index[$index]=$pra_id;
                    
                    //echo "- DDDX: $score : $oldval ; index:$index<br>";
                    
                    
                    // put ONE element in the middle
                    // pop last element
                    //    5
                    // 4,    8,   12
                    // 4, 5, 8,   12
                    $this->score_vals  = array_merge( array_slice($this->score_vals, 0, $index+1), array_slice($cache_vals , $index, $arr_len-$index) );
                    $this->score_index = array_merge( array_slice($this->score_index,0, $index+1), array_slice($cache_index, $index, $arr_len-$index) );
                    break;
                }
            }
            
            if ($do_save) {
                if($old_len==sizeof($this->score_vals)) {
                    // values are not yet saved ...
                    $this->score_vals[] =$score;
                    $this->score_index[]=$pra_id;
                }
            } else {
                array_pop($this->score_vals);
                array_pop($this->score_index);
            }
            
            
        }
        
        //echo "DDD:_save_score: score:$score pra:$pra_id<br>";
        //echo "DDD: VALS:".print_r($this->score_vals,1)."<br>";
        //echo "DDD: INDX:".print_r($this->score_index,1)."<br>";
    }
    
    function form_list() {
        $tablename	= 'ABSTRACT_PROTO';
        $initarr   = array();
        $initarr["title"]       = tablename_nice2($tablename)." selection"; 
        $initarr["submittitle"] = "Next &gt;"; // title of submit button
        $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
        
        $hiddenarr = NULL; // hidden form variables
        $hiddenarr["id"]     = $this->objid; // is a required input parameter
        
        $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
        
        
         $fieldx = array ( // form-field definition
             "title"   => "search in ALL protocols in the database?",
             "name"  => "selall",
             "object"=> "checkbox",
             "notes" => ""
             );
         $formobj->fieldOut( $fieldx ); // output the form-field
         
        
        
        $formobj->close( TRUE ); // close form, sshow submit-button
    }
    
    function form1($parx) {
        
        $initarr   = array();
        $initarr["title"]       = "Search for the most similar protocols"; // title of form
        $initarr["submittitle"] = "Submit"; // title of submit button
        $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
        
        $hiddenarr = NULL; // hidden form variables
        $hiddenarr["id"]     = $this->objid; 
        $hiddenarr["parx[selall]"]  = $parx['selall']; 
        
        $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
        
        /*
        $fieldx = array ( // form-field definition
            "title"   => "Info-Level",
            "name"  => "info",
            "object"=> "text",
            "val"    => 0,
            "notes" => "0...5"
        );
        $formobj->fieldOut( $fieldx ); // output the form-field
        */
       
        
        $formobj->close( TRUE ); // close form, sshow submit-button
    }
    
    function do_loop(&$sqlo, &$sqlo2, $sqlAfter) {
        
        $this->score_vals  = array(); 
        $this->score_index = array(); 
        
        $likely_lib = new o_ABSTRACT_PROTO_likely_ONE($sqlo, $this->objid, 'SOFT' );
        $likely_lib->set_info_level($this->infolevel);
        
        if ($this->infolevel>0) {
            $refstr = $likely_lib->get_ref_str();
            echo "INFO: PRA-ID:$this->objid  refstr:<pre>$refstr</pre><br>";
        }
        
        $this->ref_step_cnt = $likely_lib->get_ref_step_cnt();
        $debug_stop=0;
        
        $cnt_ana=0;
        $sqlsel = "ABSTRACT_PROTO_ID from ".$sqlAfter;
        $sqlo2->Quesel($sqlsel);
        while ( $sqlo2->ReadRow() ) {
            
            $loop_obj_id = $sqlo2->RowData[0];
            
            if ($loop_obj_id==$this->objid) continue;
            
            if ( $likely_lib->set_obj($sqlo, $loop_obj_id) ) {
                
                $cnt_ana++;
                if ($debug_stop and $cnt_ana>$debug_stop) {
                    echo "DEBUG_STOP!<br>";
                    break;
                }
                
                $likely_info = $likely_lib->do_likely_score($sqlo);
                $this->_save_score($loop_obj_id, $likely_info);
            }
            
        }
        
        echo "<br><br>Result:<br>";
        $objLinkLib = new fObjViewC();
        foreach($this->score_vals as $index => $oldval) {
            $pra_id =  $this->score_index[$index];
            
            $html_tmp = $objLinkLib->bo_display( $sqlo, 'ABSTRACT_PROTO', $pra_id );
            
            echo $html_tmp.' score: '.$oldval."<br>";
        }
    }
    

    function show_details($sqlo) {
        
        echo "<br><br><h2>Details of difference:</h2><br><br>";
        $objLinkLib = new fObjViewC();
        
        $likely_lib = new o_ABSTRACT_PROTO_likely_ONE($sqlo, $this->objid, 'FULL' );
        
        $initarr   = NULL;
        $initarr["action"]      = $_SERVER['PHP_SELF'];
        
        $hiddenarr=array();
        $hiddenarr['mod'] = $this->_mod;
        $hiddenarr['action'] = 'copy';
        $hiddenarr['id'] =  $this->objid;
       
        $formobj  = new formPure($initarr, $hiddenarr, 0);
        $formobj->start();
        
        foreach($this->score_vals as $index => $oldval) {
            
            $pra_id =  $this->score_index[$index];
            
            $html_tmp = $objLinkLib->bo_display( $sqlo, 'ABSTRACT_PROTO', $pra_id );
            
            echo '<input type=checkbox name="sel['.$pra_id.']" value=1> ';
            echo $html_tmp.' score: '.$oldval."<br>";
            
            $likely_lib->set_obj($sqlo, $pra_id) ;
            $likely_lib->show_diff($sqlo);
            
            echo "<br><hr><br>";
        }
        
        echo '<input type=submit value="Copy to Clipboard" class="yButton">'."\n"; // SUBMIT
        
        $formobj->close();
    }
    
    function startMain() {
        global $error;
        $FUNCNAME='startMain';
        $tablename	= 'ABSTRACT_PROTO';
        
        $this->MAX_SAVE_SCORES = 10;
        
        ?>
        <style>
        .diff {
        
        }
        
        .diffInserted {
            background-color: #D0D0FF;
        }
        
        .diffDeleted {
            background-color: #FFD0D0;
        }
        .diffUnmodified {
            background-color: #D0FFD0;
        }
        </style>
        <?php
        
        $sqlo  = &$this->_sqlo;
        $sqlo2 = &$this->_plug_regSql2();

        $this->objid = $_REQUEST['id'];
        $go     = $_REQUEST['go'];
        $parx   = $_REQUEST['parx'];
        $action = $_REQUEST['action'];
        $this->infolevel = $parx['info'];
        
        if ($action=='copy') {
             
            if ( empty($_REQUEST['sel']) ) {
                $this->htmlFoot('WARN', 'No protocol selected.!');
            }
               
            $clip_lib = new clipboardC();
            $clip_lib->reset();
            
            $cnt=0;
            foreach($_REQUEST['sel'] as $s_obj_id => $val) {
                $clip_lib->obj_addone($tablename, $s_obj_id);
                $cnt++;
            }
            
            $this->_html->messageBox2('ok', $cnt.' objects copied.');
            
            return;
        }
        
        $ref_steps = o_ABSTRACT_PROTO_likely_ONE::get_proto_steps($sqlo, $this->objid);
        if (empty($ref_steps)) {
            $this->htmlFoot('WARN', 'The reference protocol must have protocol steps!');
        }
        
        $utilLib = new fSqlQueryC($tablename);
        $sqlAfter = $utilLib->get_sql_after( );
        $filter_info = $utilLib->get_sql_info();
        if ($filter_info=='' and !$parx['selall']) {
            $this->_html->messageBox2('warning','Please select some protocols to use this tool.');
            $this->_html->line_out('');
            $this->form_list();
            return;
            
        }
        
        if (!$go) {
            $this->form1($parx);
            return;
        }
        
        $this->do_loop($sqlo, $sqlo2, $sqlAfter);
        
        $this->show_details($sqlo);
        
       
    }
}
