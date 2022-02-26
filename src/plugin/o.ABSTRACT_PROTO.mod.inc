<?php
require_once 'o.ABSTRACT_PROTO.stepMod.inc';
require_once 'o.ABSTRACT_PROTO.stepx.inc';
require_once ('f.debug.inc');

/**
 * modify ABSTRACT_PROTO
 * @package o.ABSTRACT_PROTO.mod.inc
 * @param int $id
 * @param string $steps BASE64 encoded steps of PRA_steps_form_STRUCT
 *  PRA_steps_form_STRUCT:  
 *  [ (list) of { (dict)
 *       "name":"Schritt 1",
 *       "dev":"",
 *       "sua":"",
 *       "q":"60",
 *       "unit":"",
 *       "notes":"a c r",
 *       "mv": mandatory values
 *       "is_sam":0
 *       'stepno' => original step no
 */
class o_ABSTRACT_PROTO_mod_XPL extends gPlugin {
    
    function register() {
        $tablename	= 'ABSTRACT_PROTO';
        $title		= 'modify ABSTRACT_PROTO';
        
        $this->infoarr			 = NULL;
        $this->infoarr['scriptID'] = '';
        $this->infoarr['title']    = $title;
        $this->infoarr['form_type']= 'obj';
        $this->infoarr['design']   = 'norm';
        $this->infoarr['obj_name'] = $tablename;
        $this->infoarr['obj_id']   = $_REQUEST['id'];
        $this->infoarr['checkid']  = 1;
        
        $this->infoarr2 = array(
            'accCheck' => array('tab'=>array('write'), 'obj'=>array('write') )
        );
    }
    
    
    
    function startMain() {
        global $error;
        $FUNCNAME='startMain';
        
        $sqlo = &$this->_sqlo;
        
        
        
        
        $steps_str = base64_decode($_REQUEST['data']);
        $id = $_REQUEST['id'];
        
        $steps_input = json_decode($steps_str, TRUE);
        $mess = json_last_error();

        
        /*
        if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {    
            $tmp =  fDebugC::str2Hexinfo( $steps_input[0]['name'] );
            echo "DDX: $tmp<br>";
        }
        */
        
        debugOut('JSON_encoded input: '.$steps_str, $FUNCNAME, 1);

       
        $error_tmp_arr=array();
        
        if (!is_array($steps_input)) {
            $this->htmlFoot('WARN', 'No Input steps.');
        }
        
        $par_mod_lib = new oAbsProtoStepMod();
        $par_mod_lib->init($sqlo, $id);
        $par_mod_lib->set_log_level(fAssocUpdate::MASS_LOG_POS_RANGE);
        
        // old steps
        $aProtoLib = new  oABSTRACT_PROTO_stepx();
        $aProtoLib->setObj($id);
        $ori_steps = $aProtoLib->getStepArray( $sqlo );
        
        /**
         * 
         * @var array $step_nr_arr : ORIGINAL step array
         *   STEP_NR =Array( 'ori'=>0,1 ; 'form'=>0,1 )
         */
        $step_nr_arr=array();
        
        if (!is_array($ori_steps)) $ori_steps=array();
        foreach($ori_steps as $one_step) {
            $step_nr_arr[$one_step[0]] = array('ori'=>1);
        }
        
        if (sizeof($step_nr_arr)) {
            $last_step_nr = max(array_keys($step_nr_arr));
        } else {
            $last_step_nr = 0;
        }
        $next_free_step_nr = $last_step_nr + 1;
        

        foreach($steps_input as $index => $onestep) {
            $step_nr = $onestep['stepno'];
            if (!$step_nr) {
                $error_tmp_arr[0]= "stepnr missing";
                $error->set( $FUNCNAME, 1, 'stepnr missing :'.print_r($onestep,1) );
                return;
                // debugOut('(100) WARNING: stepnr missing! '.print_r($onestep,1), $FUNCNAME, 0);
            }
            if ($step_nr<=0) {
                $step_nr = $next_free_step_nr;
                $next_free_step_nr = $next_free_step_nr + 1;
                $steps_input[$index]['stepno'] = $step_nr;
            }
            
            $step_nr_arr[$step_nr]['form']=1;
        }
        
        
        debugOut('(110) step_nr_arr: '.print_r($step_nr_arr,1), $FUNCNAME, 1);
        
        // delete old steps
        foreach($step_nr_arr as $step_nr => $one_step) {
            
            if ($one_step['ori'] and !$one_step['form']) {
                echo "... prepare DELETE: STEP:$step_nr: <br>\n";
                $par_mod_lib->f_del($sqlo, $step_nr);
                if ($error->Got(READONLY))  {
                    $errLast   = $error->getLast();
                    $error_txt = $errLast->text;
                    $error_id  = $errLast->id;
                    $error_tmp_arr[$step_nr]='Delete-Error: '.$error_txt;
                    $error->reset();
                }
            }
            
        }
        
        // update, insert
        $main_step=1;
        foreach($steps_input as $one_step) {
            
            $step_nr = $one_step['stepno'];
            $substep = 0;
            // look for leading spaces ...
            
            $name  = $one_step['name'];
            $name  = preg_replace('/\xc2\xa0/', ' ', $name);  // replace Non-Breaking spaces
            $notes = trim(preg_replace('/\xc2\xa0/', ' ', $one_step['notes']));  // replace Non-Breaking spaces
            $mand_values = trim($one_step['mv']);
            
            // echo "DDDX: ".fDebugC::str2Hexinfo2($one_step['notes'])."<br>";

            $init_space_len = strspn($name,' ');
            
            // allow hirarchie levels
            if ($init_space_len>=1 and $init_space_len<=3) {
                $substep = 1;
            }
            if ($init_space_len>=4) {
                $substep = 2;
            }
            $name = trim($name);
            if ($name=='') {
                if ($notes=='') $name='[SPACE]'; // SPACER
                else $name='[NOTES]';     // text only
                
            }
            
            if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
                echo "DDX4: substep:$substep<br>";
            }
            
            $one_step['q'] = trim($one_step['q']);  
            
            if ($one_step['q']==='') {
                $one_step['q'] = NULL;
            }
            if ($one_step['unit']==='') $one_step['unit']=NULL;
            
            $quant = $one_step['q'];
            if ($quant!==NULL and $quant!=='' and !is_numeric($quant)) {
                $error_tmp_arr[$step_nr] = 'Quantity "'.$quant.'" must be a number';
                $one_step['q'] = NULL;
            }
            
            $sua_id = $one_step['sua']>0 ? $one_step['sua'] : NULL;
            $dev_id = $one_step['dev']>0 ? $one_step['dev'] : NULL;
            $is_sample = $one_step['is_sam']>0 ? 1 : 0;
            
            $argu = array(
                'NAME'=>$name,
                'MAIN_STEP_NR'=>$main_step, // defines the ORDER
                'SUB_STEP_NR' =>$substep,
                'ABSTRACT_SUBST_ID'=>$sua_id,
                'QUANTITY'  =>$one_step['q'],
                'H_UNIT_ID' =>$one_step['unit'],
                'ABS_DEV_ID'=>$dev_id,
                'NOTES'     =>$notes,
                'MANDATORY_VALUES' => $mand_values,
                'IS_SAMPLE' => $is_sample
            );
           
            
            if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
                echo "DEBUG: UP_INS: STEP:$step_nr: ";
                var_dump($argu);
                echo "<br>\n";
            }
            
            
            
            if ( $step_nr_arr[$step_nr]['ori'] ) {
                $par_mod_lib->update_step($sqlo, $step_nr, $argu);
            } else {
                $par_mod_lib->insert_step($sqlo, $step_nr, $argu);
            }
            if ($error->Got(READONLY))  {
                // $errLast   = $error->getLast();
                $error_txt = $error->getAllAsText();
               
                $error_tmp_arr[$step_nr]='Update-Error: Step-Nr:'.$step_nr.' '.$error_txt;
                $error->reset();
            }
            
            
            $main_step++;
        }
        
        $par_mod_lib->close($sqlo);
        
        echo $main_step.' steps updated.<br>'; 
        
        if (sizeof($error_tmp_arr)) {
            $this->htmlFoot('ERROR', implode('; ',$error_tmp_arr));
        } 
        
        $this->_html->pageForward('edit.tmpl.php?t=ABSTRACT_PROTO&id='.$id, 'protocol');
    }
}
