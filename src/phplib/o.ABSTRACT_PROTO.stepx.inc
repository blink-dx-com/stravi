<?php
/**
 * abstract protocol step sub funcs
 * @package o.ABSTRACT_PROTO.stepx.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @example
 *  $pra_lib = new oABSTRACT_PROTO_stepx($pra_id);
    $pra_steps= $pra_lib->get_steps($sqlo );
    
    $pra_all_steps=array();
    foreach($pra_steps as $step_nr) {
        $step_feats = $pra_lib->step_info( $sqlo, $step_nr );
        $pra_all_steps[$step_nr] = $step_feats;
    }
 */
class oABSTRACT_PROTO_stepx {
    
    const MANDATORY_KEYS=array('q','s','n','d');
    const MANDA_KEYS2CONC=array(
        'q'=>'QUANTITY',
        's'=>'CONCRETE_SUBST_ID',
        'n'=>'NOTES',
        'd'=>'DEV_ID'
    );
    const MANDA_KEYS2NICE=array(
        'q'=>'quantity',
        's'=>'material',
        'n'=>'notes',
        'd'=>'equipment'
    );
    
    private $AbsConcSumCols;
    
    /**
     * 
     * @var array $_step_cache cache of steps 
     * [$STEP_NR] => array()
     *   'index' => string
     */
    private $_step_cache=array();

    function __construct($proto_id=0) {
    	$this->AbsConcSumCols = array('NAME', 'ABSTRACT_SUBST_ID', 'QUANTITY', 
    	    'H_UNIT_ID', 'ABS_DEV_ID', 'NOTES', 'STEP_NR', 'MANDATORY_VALUES', 'SUB_STEP_NR' );
    	if ($proto_id) $this->setObj($proto_id);
    
    }
	
    function setObj($proto_id) {
    	$this->proto_id = $proto_id;
    	$this->_step_cache=array();
    }
    
    // count steps
    function step_cnt($sqlo) {
        $sqlsse = "count(*)  from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$this->proto_id;
        $sqlo->Quesel($sqlsse);
        $sqlo->ReadRow();
        $cnt = $sqlo->RowData[0];
        return $cnt;
    }
    
    /**
     * get array of STEP_NR in right order
     * @param  $sqlo
     * @param  $abstr_proto_id
     * @return array $steparr of STEP_NR
     */
    function get_steps( &$sqlo ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        if (!$this->proto_id) sys_error_my( $FUNCNAME.': Protocol not set.' );
        
        $sqlsse = "STEP_NR ".
            " from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$this->proto_id.
            " ORDER by MAIN_STEP_NR";
        $sqlo->Quesel($sqlsse);
        
        $step_array=array();;
        while ( $sqlo->ReadRow() ) {
            $step_nr= $sqlo->RowData[0];
            $step_array[]=$step_nr;
        }
        return $step_array;
    }

    /**
     * get array of (STEP_NR,SUB_STEP_NR) in right order
     * @param  $sqlo
     * @param  $abstr_proto_id
     * @return array $steparr of 
     *   [id] = array(STEP_NR, SUB_STEP_NR)
     */
    function getStepArray( &$sqlo ) {
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	if (!$this->proto_id) sys_error_my( $FUNCNAME.': Protocol not set.' );
    	
    	$sqlsse = "STEP_NR, MAIN_STEP_NR, SUB_STEP_NR ".
    		" from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$this->proto_id. 
    		" ORDER by MAIN_STEP_NR";
     	$sqlo->Quesel($sqlsse);
     	
    	$step_array=NULL;
    	
    	$fail_tmp=0;
     	while ( $sqlo->ReadRow() ) {
    			
    		$step_nr= $sqlo->RowData[0];
    		$main_nr= $sqlo->RowData[1];
    		$sub_nr	= $sqlo->RowData[2];		
    		if ($main_nr==0) {
    			$fail_tmp=$step_nr;
    		}
    		$step_array[]=array($step_nr,$sub_nr);
    
    	}
    	
    	if ( $fail_tmp>0 ) {
    		$step_array=NULL;  // error occurred
    	}
    	
    	return $step_array;
    }
    
    /**
     * STATIC: get the steps in right order, with defined step columns
     * @param  $sqlo
     * @param  array $keyarray: array of ABSTRACT_PROTO_STEP columns
     * @return array $steparr of
     *   [step_nr] = array(STEP_NR, KEY1, KEY2, ...)
     */
    static function getStepArray_STAT( &$sqlo, $proto_id, $keyarray=NULL) {
    	//FUTURE: $sql_cols = implode(', ', $keyarray);	
    	$sqlsse = "*".
    			" from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$proto_id.
    			" ORDER by MAIN_STEP_NR";
    	$sqlo->Quesel($sqlsse);
    	$step_array=array();
    	while ( $sqlo->ReadArray() ) {	
    		$step_array[$sqlo->RowData['STEP_NR']] = $sqlo->RowData;
    	}
    	return $step_array;
    }
    
    function getStepArray_i2( &$sqlo, $keyarray=NULL ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        if (!$this->proto_id) sys_error_my( $FUNCNAME.': Protocol not set.' );
        return self::getStepArray_STAT($sqlo, $this->proto_id, $keyarray);
    }
    	
    /**
     *  get all attribute-values of one step + nice-names of linked objects
     *  - adds info from $this->_step_cache ...
     * @param object $sql
     * @param int   $step_nr
     * @param int   $metainfo 0,1 get nice-names ?
     * @return array() keys:
     *    'NAME', 'ABSTRACT_SUBST_ID', 'QUANTITY', 'H_UNIT_ID', 'ABS_DEV_ID', 'NOTES', 'STEP_NR'
     *    nice-name-cols: ABSTRACT_SUBST_NAME, ABS_DEV_NAME, H_UNIT_NAME
     * 
     */
    function step_info( &$sql, $step_nr, $metainfo=0 ) { 
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	if (!$this->proto_id) sys_error_my( $FUNCNAME.': Protocol not set.' );
    	$proto_id = $this->proto_id;
        $ret = NULL;

        $sqls= "select * from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=" .$proto_id. " AND STEP_NR=".$step_nr ;
        $sql->query($sqls);
        $sql->ReadArray();
        $ret = $sql->RowData;
        
    	if ( $metainfo>0 ) {
    		if ($ret["ABSTRACT_SUBST_ID"] ) {
    			$sqls= "select NAME  from ABSTRACT_SUBST where ABSTRACT_SUBST_ID=".$ret["ABSTRACT_SUBST_ID"] ;
    			$sql->query($sqls);
    			$sql->ReadRow();
    			$ret["ABSTRACT_SUBST_NAME"]=$sql->RowData[0];
    		}
    		
    		if ($ret["ABS_DEV_ID"] ) {
    			$sqls= "select NAME  from A_CHIP_READER where A_CHIP_READER_ID=".$ret["ABS_DEV_ID"] ;
    			$sql->query($sqls);
    			$sql->ReadRow();
    			$ret["ABS_DEV_NAME"]=$sql->RowData[0];
    		}
    		
    		if ($ret["H_UNIT_ID"]) { 
    			$sqls= "select NAME  from H_UNIT where H_UNIT_ID=".$ret["H_UNIT_ID"];
    			$sql->query($sqls);
    			$sql->ReadRow();
    			$ret["H_UNIT_NAME"]=$sql->RowData[0];
    		}   
    	}
    	
    	if (!empty($this->_step_cache[$step_nr])) {
    	    $tmpc = &$this->_step_cache[$step_nr];
    	    if ($tmpc['index']!=NULL) $ret["index"]=$tmpc['index'];
    	}
    
        return $ret;
    }     	
    
    /**
     * add the index number-string to $this->_step_cache[ST]['index']
     * @param object $sqlo
     */
    function build_index($sqlo) {
        
        if (empty($this->_step_cache)) {
            $this->_step_cache = $this->getStepArray_i2( $sqlo, $this->AbsConcSumCols );
        }
        if (empty($this->_step_cache)) {
            return;
        }
        
        $main_step_cnt = 0;
        $sub_step_cnt  = 0;
        $sub_step2_cnt = 0; 
        
        $step_arr = array_keys($this->_step_cache);
        foreach($step_arr as $step_nr) {
            
            $tmp      = $this->_step_cache[$step_nr];
            $sub_step = $tmp['SUB_STEP_NR'];
            
            if ($sub_step>0) { // is it a SUB step ?

                if ($sub_step>1) {
                    $sub_step2_cnt++;
                    $tmp_step = $main_step_cnt .'.'. $sub_step_cnt.'.'.$sub_step2_cnt;
                } else {
                    $sub_step2_cnt=0;
                    $sub_step_cnt++;
                    $tmp_step = $main_step_cnt .'.'. $sub_step_cnt;
                }
                
            } else {
                
                $main_step_cnt++;
                $tmp_step = $main_step_cnt;
                $sub_step_cnt =0;
                $sub_step2_cnt=0;  
            }
            $this->_step_cache[$step_nr]['index'] = $tmp_step;
        }
    }
    
    function get_step_cache() {
        return $this->_step_cache;
    }
    
    /**
     * get all SUAs of protocol
     * @return arra( array('step'=>$step_nr, 'sua_id'=>$sua_id) )
     */
    function get_SUAs(&$sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        if (!$this->proto_id) sys_error_my( $FUNCNAME.': Protocol not set.' );
        
        
        $sqlsse = "STEP_NR, ABSTRACT_SUBST_ID".
            " from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$this->proto_id. " and ABSTRACT_SUBST_ID>0" .
            " ORDER by MAIN_STEP_NR";
        $sqlo->Quesel($sqlsse);
        
        $sua_arr = array();
        while ( $sqlo->ReadArray() ) {
            $step_nr = $sqlo->RowData['STEP_NR'];
            $sua_id  = $sqlo->RowData['ABSTRACT_SUBST_ID'];
            $sua_arr[] = array('step'=>$step_nr, 'sua_id'=>$sua_id);
        }
        
        return $sua_arr;
    }
	
}