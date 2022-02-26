<?php
/**
 * non-GUI functions for PROTOCOL-object step analysis
 * structure: CP_STEP_gui_STRUCT: -- array of ALL steps
 *    array ( STEP_NO => CP_STEP_ONE_gui_STRUCT )
 *  
 * structure CP_STEP_ONE_gui_STRUCT : array of ONE step
 *      'is'=>0,1, 
		'qu'=> quantity, 
		'cs'=> conc_subst_id, 
		'cn'=> conc_notes, 
		'cc'=> conc_color, 
		'dea'  => step_deact, 
		'canam'=> ca_subst_name, 
		'anam' => abs_subst_name,
		'dev'  => device id
		'devna'=> device_name
		'devAnam' => abstract device name
		
 * @package o.PROTO.steps.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @unittest exists
 */
class gProtoOrg {
	
	/**
	 * array of STEP_NR in right order
	 * @var array $step_array of STEP_NR
	 */
	var $step_array;
	
	const GUI_KEYs_NICE=array(
	    'is'=> 'step exists',
	    'qu'=> 'quantity',
	    'cs'=> 'material',
	    'cn'=> 'material notes',
	    'dea'  => 'step is deactivated', 
	);
	
function __construct() {

	$this->AbsConcSumCols = array('NAME', 'ABSTRACT_SUBST_ID', 'ABS_DEV_ID', 'QUANTITY', 'H_UNIT_ID', 'NOTES', 'IS_SAMPLE');
    $this->ConcSumColNice = array(
			'CONCRETE_SUBST_ID'=>'SUBST', 
	 		'DEV_ID'=>'DEV', 
			'QUANTITY'=>'QUANT', 
   			'NOTES'=>'NOTES', 
	  		'NOT_DONE'=>'INACT' 
			);
	$this->ConcSumCols    = array_keys($this->ConcSumColNice);
	
}	

/**
 * set PRC (can be 0) and PRA 
 * @param object $sql  can be NULL if "noStepScan" = 1
 * @param int $abstr_proto_id
 * @param int $conc_proto_id : can be 0
 * @param array $options  "noStepScan" = [0] | 1 do not scan the protocol_steps !!!
 */
function setProto(	&$sql, $abstr_proto_id, $conc_proto_id, $options = array() 	) {

	$this->abstr_proto_id = $abstr_proto_id;
	if (!is_array($options)) $options=array();
	
	if ( $options["noStepScan"]<=0 ) {
		$this->getStepArraySql( $sql, $abstr_proto_id );
	}	
}

/**
 * set concrete protocol with step scan
 * @param $sqlo
 * @param $conc_proto_id
 * @return NULL
 */
function setConcProto(&$sqlo, $conc_proto_id) {
	$this->concrete_proto_id=$conc_proto_id;
	$abstr_proto_id = glob_elementDataGet( $sqlo, 'CONCRETE_PROTO', 
		'CONCRETE_PROTO_ID', $conc_proto_id, 'ABSTRACT_PROTO_ID');
	$this->abstr_proto_id = $abstr_proto_id;
	$this->getStepArraySql( $sqlo, $abstr_proto_id );
}

function getStepArraySql( &$sql, $abstr_proto_id) {
	
	$sqls = "select STEP_NR, MAIN_STEP_NR, SUB_STEP_NR ".
		" from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$abstr_proto_id. 
		" ORDER by MAIN_STEP_NR";
		
 	$sql->query($sqls, 'protostep');
	$this->step_array=array();
	
	$fail_tmp=0;
 	while ( $sql->ReadRow() ) {
			
		$step_nr= $sql->RowData[0];
		$main_nr= $sql->RowData[1];
		//$sub_nr	= $sql->RowData[2];		
		if ($main_nr==0) {
			$fail_tmp=$step_nr;
		}
		$this->step_array[]=$step_nr;

	}
	
	if ( $fail_tmp>0 ) {
		$this->step_array=NULL;  // error occurred
	}
}

/**
 * add 'stepnr.nice' to $pra_steps
 * @throws Exception
 */
static function stepnr_nice_add(&$pra_steps) {
    
    if (!is_array($pra_steps)) {
        throw new Exception('Input-Error: step_array not an array.');
    }
    
    $sub_step_cnt=0;
    $sub_step_col_cnt=0;
    $main_step_cnt=0;
    $sub_step_col_cnt = 0;
    $sub_step2_cnt=0;
    
    $step_ind = array_keys($pra_steps);
    
    foreach($step_ind as $i) {
        
        $sub_step= $pra_steps[$i]['SUB_STEP_NR'];
        #$tmp_spaces=''; # needed later ..
        
        if ($sub_step>0) { // is it a SUB step ?
            
            $sub_step_col_cnt++;
            if ($sub_step_col_cnt>2)  $sub_step_col_cnt=0;
            #$tmp_spaces = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";

            if ($sub_step>1) {
                #$tmp_spaces .= "&nbsp;&nbsp;&nbsp;&nbsp;";
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
            $sub_step_col_cnt=0;
        }
        $pra_steps[$i]['stepnr.nice'] = $tmp_step;
    }
 
}

// get all features of ALL steps
static function get_pra_all_steps( &$sqlo, $abstr_proto_id) {
    $sqlo->Quesel("*  from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$abstr_proto_id. " ORDER by MAIN_STEP_NR");
    $step_arr2=array();
    while ( $sqlo->ReadArray() ) { 
        $step_nr= $sqlo->RowData['STEP_NR'];
        $step_arr2[$step_nr]=$sqlo->RowData;
    }
    return $step_arr2;
}
// get all features of ONE step
static function get_pra_one_steps( &$sqlo, $abstr_proto_id, $step_nr) {
    $sqlo->Quesel("*  from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$abstr_proto_id. " and STEP_NR=".$step_nr);
    $step_arr=array();
    $sqlo->ReadArray();
    $step_arr=$sqlo->RowData;
    return $step_arr;
    
}

/**
 * protocol has any data
 * @param object $sqlo
 * @param int $prc_id
 * @return int : number of steps
 */
static function has_any_step(object $sqlo, int $prc_id) {
    $sqlsel = "count(1) from CONCRETE_PROTO_STEP where CONCRETE_PROTO_ID=".$prc_id;
    $sqlo->Quesel($sqlsel);
    $sqlo->ReadRow();
    $step_cnt= $sqlo->RowData[0];
    return $step_cnt;
}

/**
 * get data of selected COLUMNS ($columns)  for all steps, 
 * @param object $sqlo
 * @param int $prc_id
 * @param array $columns -- auto included: STEP_NR
 * @return array array[STEP_NR]=>array('QUANTITY'=>)
 */
static function get_all_data(&$sqlo, $prc_id, $columns) {
    $column_sql=implode(', ',$columns);
    $sqlsel = "STEP_NR, ".$column_sql.
        " from CONCRETE_PROTO_STEP where CONCRETE_PROTO_ID=".$prc_id.
        " ORDER by STEP_NR";
    $sqlo->Quesel($sqlsel);
    
    $result=array();
    while ( $sqlo->ReadArray() ) {    
        $step_nr= $sqlo->RowData['STEP_NR'];
        $result[$step_nr]=$sqlo->RowData;
        
    }
    return $result;
}

// can be called without 
function conc_subst_get( &$sql, $c_subst_id ) {
    if (!$c_subst_id) return (""); 
    $cs_name="";
    $sqls= "select NAME, ABSTRACT_SUBST_ID from CONCRETE_SUBST where CONCRETE_SUBST_ID=" . $c_subst_id;
    $sql->query("$sqls");
    $sql->ReadRow();
    $cs_name=$sql->RowData[0];
    $as_id=$sql->RowData[1];
        
    if ($cs_name=="") {    
        $sqls= "select NAME from ABSTRACT_SUBST where ABSTRACT_SUBST_ID=" . $as_id;
        $sql->query("$sqls");
        $sql->ReadRow();
        $cs_name=$sql->RowData[0];
    } 
    return($cs_name);
}

/**
 * get info of ONE step, including SUA-name, DEV-name
 * can be called without initialization of $c_proto_id
 */
function cproto_step_info( &$sql, $c_proto_id, $proto_step_nr ) {
    $c_arr=NULL;
    if ($c_proto_id) {  
        $colSql = implode(',',$this->ConcSumCols );
        $sqls= "select ". $colSql ." from CONCRETE_PROTO_STEP ".
               " where CONCRETE_PROTO_ID=".$c_proto_id. " AND STEP_NR=".$proto_step_nr;
        $sql->query($sqls);
		
        if ( $sql->ReadArray() ) { 
            $c_arr = $sql->RowData;
			
            if ($c_arr["CONCRETE_SUBST_ID"]) {
               $c_arr["SUBST_NAME"] = $this->conc_subst_get($sql, $c_arr["CONCRETE_SUBST_ID"] );      
            }
			
			if ( $c_arr["DEV_ID"]>0 ) {
				$sqls = 'select name from CHIP_READER where CHIP_READER_ID='.$c_arr["DEV_ID"];
				$sql->query($sqls);
				$sql->ReadRow();
				$c_arr["DEV_NAME"]=$sql->RowData[0];
			} 
        }
    }
    return ( $c_arr ); 
}

/**
 * get info of ONE step
 * can be called without initialization of $c_proto_id
 */
static function cStep_pure( &$sql, $c_proto_id, $proto_step_nr ) {
    $c_arr=array();
    if (!$c_proto_id) return array();

    $sqlsel = "* from CONCRETE_PROTO_STEP ".
        " where CONCRETE_PROTO_ID=".$c_proto_id. " AND STEP_NR=".$proto_step_nr;
    $sql->Quesel($sqlsel);
    if ( $sql->ReadArray() ) {
        $c_arr = $sql->RowData;
    }
    
    return $c_arr;
}

function &getStepArray() {
	return ($this->step_array);
}

/**
 * get SHORT format
 *      'is'=>0,1, 
		'qu'=> quantity, 
		'cs'=> conc_subst_id, 
		'cn'=> conc_notes,  
		'dea'  => step_deact, 
		'dev'  => device id
		
 * @param object $sql
 * @param int $step_nr
 * @param int $c_proto_id
 * @return array short CP_STEP_ONE_gui_STRUCT
 */
static function conc_proto_params_RAW ( &$sqlo, $c_proto_id, $step_nr ) {
    $out=array();
    $out['is'] = 0;
    $sqls = '* from CONCRETE_PROTO_STEP where CONCRETE_PROTO_ID='.$c_proto_id.
       ' AND STEP_NR=' . $step_nr;
    
    $sqlo->Quesel($sqls);
    if ( $sqlo->ReadArray() ) {
        $out['is'] = 1; 		//  concrete proto step exists ???
        $out['qu'] = $sqlo->RowData['QUANTITY'];
        $out['cs'] =$sqlo->RowData['CONCRETE_SUBST_ID'];
        $out['dev']=$sqlo->RowData['DEV_ID'];
        $out['cn'] =$sqlo->RowData['NOTES'];
        $NOT_DONE  =$sqlo->RowData['NOT_DONE'];
        // don't change 0 (and NULL) values
        if ($out['qu']!=0) {
            $out['qu']=floatval($out['qu']);
        }
        if ( $NOT_DONE==1) {
            $out['dea'] =1;
        }
    }
    return $out;
}

/**
 * get step parameters
 * @return: array CP_STEP_ONE_gui_STRUCT
 */
static function conc_proto_params ( &$sql, $step_nr, $c_proto_id, $conc_color  ) {
    
    $conc_step_is = 0;
	$sqls = 'select * from CONCRETE_PROTO_STEP where CONCRETE_PROTO_ID='.$c_proto_id.
		' AND STEP_NR=' . $step_nr;
	
	$sql->query($sqls);
	if ( $sql->ReadArray() ) {
		$conc_step_is = 1; 		//  concrete proto step exists ??? 
		$conc_quantity=$sql->RowData['QUANTITY'];
		$conc_subst   =$sql->RowData['CONCRETE_SUBST_ID'];
		$conc_dev     =$sql->RowData['DEV_ID'];
		$conc_notes   =$sql->RowData['NOTES'];
		$NOT_DONE     =$sql->RowData['NOT_DONE'];
		/* 
		 * BEGIN:: UREQ: 0000887: o.CONCRETE_PROTO_STEPS > number format for quantity column
		 * replace simple "QUANTITY" with special "QUANTITY" using a special number format to display leading zero
		 * (e.g.: ".611" will be shown as "0.611")
		 */
		// don't change 0 (and NULL) values 
		if ($conc_quantity!=0) {	
			$conc_quantity=floatval($conc_quantity);
		}
        /* END:: UREQ: 0000887: o.CONCRETE_PROTO_STEPS > number format for quantity column*/
		        
		if ( $NOT_DONE==1) {
			$conc_color="#FFD0D0";
			$conc_step_deact=1;
		}
	}

	if ($conc_subst) {
		$subst_name_ext=" [$conc_subst] ";
        
		$sqls = 'select a.name, c.name from ABSTRACT_SUBST a, CONCRETE_SUBST c where ' .
		  'c.CONCRETE_SUBST_ID=' .$conc_subst. ' AND a.ABSTRACT_SUBST_ID=c.ABSTRACT_SUBST_ID';
		$sql->query($sqls);
		$sql->ReadRow();
		$a_name=$sql->RowData[0];
		$c_name=$sql->RowData[1];
		if ( $c_name ) {
			$ca_subst_name=$c_name;
		} else {
			$ca_subst_name=$a_name;
		}
		
		if ($ca_subst_name == "") $ca_subst_name=$subst_name_ext;
	} else {
		$ca_subst_name=NULL;
	}
	
	if ($conc_dev) {
		$dev_name_ext=" [$conc_dev] ";
        
		$sqls = 'select a.name, c.name ' . 
		   ' from CHIP_READER c LEFT OUTER JOIN A_CHIP_READER a ON c.A_CHIP_READER_ID=a.A_CHIP_READER_ID ' .
		   ' where c.CHIP_READER_ID=' .$conc_dev. ' ';
		
		$sql->query($sqls);
		$sql->ReadRow();
		$deva_name=$sql->RowData[0];
		$devc_name=$sql->RowData[1];
		if ( $devc_name!=NULL ) {
			$dev_name=$devc_name;
		} else {
			$dev_name=$deva_name;
		}
		if ($dev_name == "") $dev_name = $dev_name_ext;
	} else {
		$dev_name = "";
	}
    
	return array( 'is'=>$conc_step_is, 'qu'=>$conc_quantity, 'cs'=>$conc_subst, 'cn'=>$conc_notes, 
				  'cc'=>$conc_color, 'dea'=>$conc_step_deact, 'canam'=>$ca_subst_name, 'anam'=>$a_name,
				  'dev' =>$conc_dev, 'devna'=>$dev_name, 'devAnam'=>$deva_name );
}    

/** 
 * get MERGED values of one abstract_step and concrete_step
 * - e.g. quant contains NOTEs from CONCRETE_PROTO_STEP, if not exists => from ABSTRACT_PROTO_STEP
 * @return array (
 *   'name'  => NAME of step, 
 *   'substn'=> name of SUC
 *   'quant'=> quantity,
 *   'note'=>  notes 
 *   'inact'=> inactive?, 
 *   'devn'=>  device name 
 *   'unitn' => unit name
 *   )
 */
function cStepSumGet( &$sql, $step_nr, $c_proto_id ) {
    
    $proto_id    = $this->abstr_proto_id;
    $aSubstName  = "";
    $h_unit_name = "";
	
    $sqls= 'select ' .implode(",",$this->AbsConcSumCols). 
           ' from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=' .$proto_id. ' AND STEP_NR='.$step_nr ;
    $sql->query($sqls);
    $sql->ReadArray();

    $actName    = $sql->RowData['NAME'];
    $aSubstId   = $sql->RowData['ABSTRACT_SUBST_ID'];
	$aDevId     = $sql->RowData['ABS_DEV_ID'];
    $quant      = $sql->RowData['QUANTITY'];
    $notes	    = $sql->RowData['NOTES'];
    $is_sample	= $sql->RowData['IS_SAMPLE']; 
    $H_UNIT_ID  = $sql->RowData['H_UNIT_ID']; 
    
    if ($aSubstId) {
        $sqls= 'select NAME  from ABSTRACT_SUBST where ABSTRACT_SUBST_ID='.$aSubstId;
        $sql->query($sqls);
        $sql->ReadRow();
        $SubstName=$sql->RowData[0];
    }
	if ($aDevId) {
		$sqls= 'select NAME from A_CHIP_READER where A_CHIP_READER_ID='.$aDevId;
        $sql->query($sqls);
        $sql->ReadRow();
        $aDevName= $sql->RowData[0];
		$devName = $aDevName;
	}
	if ($H_UNIT_ID) {
	    $sqls= "select NAME  from H_UNIT where H_UNIT_ID=".$H_UNIT_ID;
	    $sql->query($sqls);
	    $sql->ReadRow();
	    $h_unit_name = $sql->RowData[0];
	} 
	
    $ret = $this->conc_proto_params ( $sql, $step_nr, $c_proto_id, $conc_color);
    
    if ($ret['qu'] !='') $quant     = $ret['qu'];
    if ($ret['cs'] !='') $SubstName = $ret['canam'];
    if ($ret['cn'] !='') $notes     = $ret['cn']; 
    if ($ret['dev'] !='') $devName  = $ret['devna']; 
	
    return array(
        'name'  =>$actName,
        'substn'=>$SubstName, 
        'quant' =>$quant, 
        'note'  =>$notes, 
        'inact' =>$ret['dea'], 
        'devn'  =>$devName,
        'unitn' =>$h_unit_name
        
    );
}

/** 
 * get RAW values of one concrete_step
 * @return array ('name'=>, 'substn'=>, 'quant'=>, 'note'=>, 'inact'=> )
 */
function cStepGet( &$sqlo, $step_nr, $c_proto_id ) {
    
    $proto_id    = $this->abstr_proto_id;
    $aSubstName  = "";
	
    $sqls= 'select NAME from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=' .$proto_id. ' AND STEP_NR='.$step_nr ;
    $sqlo->query($sqls);
    $sqlo->ReadRow();

    $actName    = $sqlo->RowData[0];
    $dummy = NULL;
    $ret = $this->conc_proto_params ( $sqlo, $step_nr, $c_proto_id, $dummy);  
 
    return (array('name'=>$actName, 'substn'=>$ret['canam'], 'quant'=>$ret['qu'], 'note'=>$ret['cn'], 'inact'=>$ret['dea'] ));
}

function getCSumCols() {
	return ( $this->ConcSumColNice );
}

function getCSumRawCols() {
	return ( $this->ConcSumCols );
}

}
