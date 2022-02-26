<?php
/**
 * ABSTRACT_PROTO : FORMULA
 * 
 * Formula
 * (S23+A23)*0.5
 * QUESTIONMARK-Operator: ?(S24:S23)  if S24 is empty: use S23
 * 
 * @package o.ABSTRACT_PROTO.formula.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @var $formula_STRUCT
 * 'test' [ -- Unit-Test data
	{'step':1, 'val':4.6 }
     ]
 *  'prec': '.2f' -- default output precision
   'calc_lines': [ -- list of single formulas
 {'calc':'S1+S3/S4', 'step':2, 'prec':'0.2f'},
 {'calc':'(S1+S2/S4)*100.34', 'step':3},
 {'calc':'S3*100.34', 'step':4}
 ]
 */

require_once ("glob.obj.update.inc");

class X_Field_calculate {
    const PATTERN = '/(?:\-?\d+(?:\.?\d+)?[\+\-\*\/])+\-?\d+(?:\.?\d+)?/';
    
    const PARENTHESIS_DEPTH = 10;
    
    public function calculate($input){
        if(strpos($input, '+') != null || strpos($input, '-') != null || strpos($input, '/') != null || strpos($input, '*') != null){
            //  Remove white spaces and invalid math chars
            $input = str_replace(',', '.', $input);
            $input = preg_replace('[^0-9\.\+\-\*\/\(\)]', '', $input);
            
            //  Calculate each of the parenthesis from the top
            $i = 0;
            while(strpos($input, '(') || strpos($input, ')')){
                $input = preg_replace_callback('/\(([^\(\)]+)\)/', 'self::callback', $input);
                
                $i++;
                if($i > self::PARENTHESIS_DEPTH){
                    break;
                }
            }
            
            //  Calculate the result
            if(preg_match(self::PATTERN, $input, $match)){
                return $this->compute($match[0]);
            }
            // To handle the special case of expressions surrounded by global parenthesis like "(1+1)"
            if(is_numeric($input)){
                return $input;
            }
            
            return 0;
        }
        
        return $input;
    }
    
    private function compute($input){
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        $compute=''; // dummy value ...
        $eval_str='$compute='.$input.';';
        debugOut('eval_str:'.$eval_str, $FUNCNAME, 2);
        eval($eval_str);
        

        if ((string)$compute=='INF') {
            throw new Exception('Invalid result from formula "'.$input.'"');
        }
        
        return $compute;
    }
    
    private function callback($input){
        if(is_numeric($input[1])){
            return $input[1];
        }
        elseif(preg_match(self::PATTERN, $input[1], $match)){
            return $this->compute($match[0]);
        }
        
        return 0;
    }
}

class oABSTRACT_PROTO_formula_line {
    
}

/***
 * formula analysis
 * @author skube
 *
 */
class oABSTRACT_PROTO_formula {
    
    private $formula_struct;
    private $data_out; //  data of CONCRETE_PROTO_STEP: array[STEP_NR] = val
    private $data_pra; //  data of ABSTRACT_PROTO_STEP: array[STEP_NR] = val
    
    function __construct($sqlo, $pra_id, $load_db=1, $formula_json='') {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->objid=$pra_id;
        if ($load_db) {
            $formula_json = $this->load_formula($sqlo);
        } else {
            // nothing special
        }
        
        if ($formula_json==NULL) {
            return;
        }
        
        $this->formula_struct = json_decode($formula_json, TRUE);
        if (!is_array($this->formula_struct)) {
            $error->set( $FUNCNAME, 1, 'Internal formular struct is corrupt.' );
            return;
        }
        
        $this->_load_step_quants($sqlo);
        
        debugOut('formula_struct: <pre>'.print_r($this->formula_struct,1).'</pre>', $FUNCNAME, 3);
    }
    
    private function _load_step_quants($sqlo) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        $this->data_pra=array();
        $sqlsel = "STEP_NR, QUANTITY from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".
            $this->objid.' and QUANTITY is not NULL order by STEP_NR';
        
        $sqlo->Quesel($sqlsel);
        while ( $sqlo->ReadRow() ) {
            $step = $sqlo->RowData[0];
            $q    = $sqlo->RowData[1];
            $this->data_pra[$step]=$q;
        }
        
        debugOut('_load_step_quants: '.print_r($this->data_pra,1), $FUNCNAME, 3);
    }
    
    function load_formula($sqlo) { 
        $formula_json  = glob_elementDataGet( $sqlo, 'ABSTRACT_PROTO', 'ABSTRACT_PROTO_ID', $this->objid, 'FORMULA');
        return $formula_json;
    }
    
    function get_formula_STRUCT() {
        return $this->formula_struct;
    }
    
    function get_calc_lines() {
        return $this->formula_struct['calc_lines'];
    }
    
    /**
     * get Unit-Test data
     * @return array [step_nr]=value
     */
    function get_UT_data() {
        
        $dataout=array();
        if (!is_array($this->formula_struct['test'])) return $dataout;
        
        foreach($this->formula_struct['test'] as $row) {
            $dataout[$row['step']]= $row['val'];
        }
        return $dataout;
    }
    
    /**
     * get Unit-Test data
     * @return array [step_nr]=value
     */
    function get_UT_raw() {
        return $this->formula_struct['test'];
    }
    
    function set_UT_data($ut_rows) {
        $this->formula_struct['test']=$ut_rows;
    }
    
    /**
     * get step_nr and pos, check, if value exists
     * @param string $formula_line
     * @param int $pos
     * @param int $pos_max
     * @param string $t_flag 
     *    S - CONCRETE 
     *    A - ABSTRACT
     * @param int $error_emp_flag throw error on empty value
     * @throws Exception
     * @return array
     *  'step_nr'=>$step_nr, 'pos'=>$pos, 'val'=>$value
     */
    private function _one_variable($formula_line, $pos, $pos_max, $t_flag, $error_emp_flag=0) {
        $step_chars='0123456789';
        
        $var_pos_start=$pos;
        $pos=$pos+1; // start of number
        $step_nr_str='';
        while ($pos<$pos_max) {
            $charx=$formula_line[$pos];
            //echo "-- DDXL: ".$pos.' :'.$charx."<br>";
            if (strstr($step_chars, $charx)!=NULL) {
                $step_nr_str .= $charx;
                $pos++;
            } else {
                break;
            }
        }

        
        if ($step_nr_str==NULL) {
            throw new Exception('Var-Definition: no Step-Nr given (Col-Pos:'.$var_pos_start.').');
        }
        $step_nr=intval($step_nr_str);
        if ($t_flag=='S') {
            
            $value    = $this->data_out[$step_nr];
            if ($error_emp_flag) {
                if ( $value===NULL or  $value==='') {
                    throw new Exception('Var-Definition: No Data-Value for Step-Nr '.$step_nr.'');
                }
            }
        } else {
   
            // step must be defined ...
            if (!array_key_exists($step_nr, $this->data_pra)) {
                throw new Exception('Var-Definition: Step-Nr '.$step_nr.' not defined.');
            }
            
            $value    = $this->data_pra[$step_nr];
            if ($error_emp_flag) {
                if ( $value===NULL or  $value==='') {
                    throw new Exception('Var-Definition: No Template-Data-Value for Step-Nr '.$step_nr.'');
                }   
            }
        }
        return array('step_nr'=>$step_nr, 'pos'=>$pos, 'val'=>$value, 'var'=>$t_flag.$step_nr);
    }
    
    private function _one_variable_auto($formula_line, $pos, $pos_max) {
        
        $charx=$formula_line[$pos];
        if ($charx=='S') {
            $var_info = $this->_one_variable($formula_line, $pos, $pos_max,'S', 0);
           
        }
        if ($charx=='A') {
            $var_info = $this->_one_variable($formula_line, $pos, $pos_max, 'A', 0);
        }
            
        return $var_info;
    }
    
    /**
     * ?(S24:S23)  
     * @param string $formula_line
     * @param int $pos
     * @param int $pos_max
     * @throws Exception
     */
    private function _quest_operator($formula_line, $pos, $pos_max) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        $var_pos_start=$pos;
        $pos=$pos+1; // start of "("
        
        if (substr($formula_line,$pos,1)!='(') {
            throw new Exception('Quationmark-OP: first ( missing');
        }
        $endpos_quest_op = strpos($formula_line,')', $pos);
        if (!$endpos_quest_op) {
            throw new Exception('Quationmark-OP: last ( missing');
        }
        
        debugOut('Part: '.substr($formula_line, $var_pos_start, $endpos_quest_op-$var_pos_start+1), $FUNCNAME, 2);
        
        $pos=$pos+1; // start variable
        
        $var_info = $this->_one_variable_auto($formula_line, $pos, $endpos_quest_op);
        debugOut('Result1: '.print_r($var_info,1), $FUNCNAME, 2);
        if ($var_info['val']===NULL or $var_info['val']==='') {
            $pos = $var_info['pos']+1; // next start of variable
            $var_info = $this->_one_variable_auto($formula_line, $pos, $endpos_quest_op);
            debugOut('Result2: '.print_r($var_info,1), $FUNCNAME, 2);
            if ($var_info['val']==='') {
                throw new Exception('No value given for: '.substr($formula_line, $var_pos_start, $endpos_quest_op-$var_pos_start+1) );
            }
        }
        
        $next_pos = $endpos_quest_op + 1; // go to end of questmark-Operator
        $var_info['pos']=$next_pos;
        debugOut('Result_Fin: '.print_r($var_info,1), $FUNCNAME, 2);
        return $var_info;
        
    }
    
    
    
    /**
     * compute one line
     * INPUT: $this->data_out array[STEP_NR] = val
     * @param string $formula_line
     * @return float value
     */
    private function var_line_comp($formula_line)  {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $pos=0;
        
        debugOut('formula_line: '.$formula_line, $FUNCNAME, 2);
        $formula_line .="\n"; // add EOL
        $pos_max=strlen($formula_line);
       
        $formula_line_out='';
        
        
        
        while ($pos<$pos_max) {
            
            $charx=$formula_line[$pos];
  
            do {
                if ($charx=='?') {
                    $var_info = $this->_quest_operator($formula_line, $pos, $pos_max);
                    $pos      = $var_info['pos'];
                    $formula_line_out .= $var_info['val']; 
                    break;
                } 
                if ($charx=='S') {
                    $var_info = $this->_one_variable($formula_line, $pos, $pos_max,'S', 1);
                    $pos      = $var_info['pos'];
                    $formula_line_out .= $var_info['val']; 
                    break;
                } 
                if ($charx=='A') {
                    $var_info = $this->_one_variable($formula_line, $pos, $pos_max, 'A', 1);
                    $pos      = $var_info['pos'];
                    $formula_line_out .= $var_info['val']; 
                    break;
                } 
                
                $formula_line_out .= $charx;
                $pos++;
                
            } while (0);
        }
        
        $formula_line_out =trim($formula_line_out); // remove LINEFEED
        debugOut('- formula_line_out: '.$formula_line_out, $FUNCNAME, 2);
        
        
        $calc_lib = new X_Field_calculate();
        $valOut = $calc_lib->calculate($formula_line_out);
        debugOut('- valOut:'.$valOut, $FUNCNAME, 2);
       
        return $valOut;
        
    }
    
    function reset_formula_lines() {
        
    }
    
    function set_line($line_id,$row) {
        $this->formula_struct['calc_lines'][$line_id]['calc']=$row['calc'];
        $this->formula_struct['calc_lines'][$line_id]['step']=$row['step'];
    }
    
    function set_calc_lines($calc_lines) {
        $this->formula_struct['calc_lines']=$calc_lines;
    }
    
    /**
     *
     * @param string $input 'S1+S3/S4'
     */
    public function calculate_all(&$data_in){
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        $this->data_out = $data_in;
        
        $calc_line = &$this->formula_struct['calc_lines'];
        
        foreach($calc_line as $line_id=>$row) {
            
            $formula_line=$row['calc'];
            $step_nr = $row['step'];
            if ($formula_line!==NULL and $formula_line!=='') {
                try {
                    debugOut('step: :'.$step_nr.' Formula:'.$formula_line, $FUNCNAME, 1);
                    $value   = $this->var_line_comp($formula_line);
                } catch (Exception $e) {
                    $oldmess =  $e->getMessage();
                    throw new Exception('Destination-Step-Nr:'.$step_nr.' Error:'.$oldmess);
                }
                $this->data_out[$step_nr]=$value;
            }
        }
        
        return $this->data_out;
    }
}

class oABSTRACT_PROTO_formula_mod {
    
    private $_error_lines=array(); // array by line_id
    
    function __construct($sqlo, $pra_id) {
        $this->objid=$pra_id;
        $this->sub_lib = new oABSTRACT_PROTO_formula($sqlo, $pra_id);
    }
    
    /**
     * check user input
     * @param array $formula_struct_IN
     * @param array $data_in
     */
    function check_user_input($formula_struct_IN, $data_in) {

        // initial settings
        $this->sub_lib->reset_formula_lines();
        foreach($formula_struct_IN as $line_id=>$row) {
            $this->sub_lib->set_line($line_id,$row);
        }
        
        $this->sub_lib->calculate_all($data_in);
    }
    
    function ins_line($line_id_ins) {
        
        //$row_new=array('step'=>'', 'calc'=>'');
        $calc_lines_out=array();
        $lines_olds = $this->sub_lib->get_calc_lines();
        
        foreach($lines_olds as $line_id=>$row) {
            $calc_lines_out[]=$row;
            if ($line_id==$line_id_ins) {
                
                $row_new = $row;
                $calc_lines_out[]=$row_new;
                break;
            }
 
        }
        
        if ( $line_id<( sizeof($lines_olds)-1 ) ) {
            $calc_lines_out = array_merge($calc_lines_out, array_slice($lines_olds, $line_id+1));
        }
        
        //echo "DDX:ins_line: <pre>".print_r($calc_lines_out,1)."</pre><br>";
        
        $this->sub_lib->set_calc_lines($calc_lines_out);
        
    }
    
    function set_UT_data($ut_rows) {
        $this->sub_lib->set_UT_data($ut_rows);
    }
    
    function save_formula($sqlo) {
        
        $struct =  $this->sub_lib->get_formula_STRUCT();
        $formula_json = json_encode($struct);
        
        $args=array(
            'vals'=>array(
               'FORMULA'=>$formula_json
            )
            
        );
        $UpdateLib = new globObjUpdate();
        $UpdateLib->update_meta( $sqlo, 'ABSTRACT_PROTO', $this->objid, $args );
        
        
        
        
    }
}
