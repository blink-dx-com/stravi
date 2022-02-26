<?php
require_once ('f.sqleasy.inc');
require_once ('o.PROTO.subs.inc');
require_once 'o.ABSTRACT_PROTO.stepx.inc';


/**
 * update concrete_proto_steps
 * @package o.PROTO.upsteps.inc
 * @author  qbi
 * @example <pre>
 * 
 $step_params = array(
   1 => array('QUANTITY'=>3,    'NOTES=>'hallo'),
   4 => array('QUANTITY'=>3.67, 'NOTES=>'xxx'),
 );
 $proto_mod_lib = new oConcProtoUpSteps();
 $proto_mod_lib->setProto($prc_id);
 $proto_mod_lib->protoUpdate($sqlo, $pra_id, $step_params);
 </pre>
 
  @var array step_params_NO_STRUCT : step parameter array by STEP_NO
      array(
          {STEP_NR} => array( {COLUMN}=> value ),
      )
      where COLUMN : "CONCRETE_SUBST_ID", "QUANTITY",  "NOT_DONE", "NOTES", "DEV_ID"
  @var array step_params_NAME_STRUCT : : step parameter array by STEP_NAME
  array(
   'add susbstance 1' => array('QUANTITY'=>3,    'NOTES=>'hallo'),
   'measure val 2'    => array('QUANTITY'=>3.67, 'NOTES=>'xxx'),
   )
 */
class oConcProtoUpSteps {

    function __construct() {
    	$this->protoManiLib = new oCprotoMani();
    }
    
    function setProto($cpid) {
    	$this->cpid = $cpid;
    }
    
    function _getStepSql($protoid) {
    	$usecol = array( "CONCRETE_SUBST_ID", "QUANTITY",  "NOT_DONE", "NOTES", "DEV_ID");
    	$sqlselX = "STEP_NR, ".implode(",",$usecol) ." from CONCRETE_PROTO_STEP".
    		" where CONCRETE_PROTO_ID=".$protoid.
    	    " order by STEP_NR";
    	return ($sqlselX);
    }
    
    /**
     * get difference to $blueid
     * @return int result:
     * 			 0 : no difference
     *			 1 : diff num of steps
     *			 10 : different abstract_proto
     *			
     */
    function isDiff( &$sqlo,  &$sqlo2, $blueid ) {
    
    	$cpid = $this->cpid;
    
    	$sqlselX = "count(1) from CONCRETE_PROTO_STEP where CONCRETE_PROTO_ID=";
    	$sqlo->Quesel( $sqlselX . $cpid );
    	$sqlo->ReadRow();
    	$destStepNr = $sqlo->RowData[0];
    
    	$sqlo->Quesel( $sqlselX . $blueid );
    	$sqlo->ReadRow();
    	$blueStepNr = $sqlo->RowData[0];
    	if ($destStepNr != $blueStepNr) {
    		return 1;
    	}
    	
    	$sqlX1 = $this->_getStepSql($blueid);
    	$sqlX2 = $this->_getStepSql($cpid);
    
    	$isDiff = 0;
    	$sqlo-> Quesel( $sqlX1);
    	$sqlo2->Quesel( $sqlX2);
    
    	while ( $sqlo->ReadArray() ) {
    		if (!$sqlo2->ReadArray() ) {
    			$isDiff = 1;
    			break;
    		}
    		
    		$result = array_diff_assoc($sqlo->RowData, $sqlo2->RowData);
    		if ( sizeof($result)>0 ) {
    			$isDiff = 2;
    			break;
    		} 
    	}
    
    	if (!$isDiff) return 0;
    	else return 1;
    }
    
    /**
     * update difference to $blueid
     * @return int result: 0 : no difference
     *					   >0 : was different
     */
    function upDiff( &$sqlo,  &$sqlo2, $blueid ) {
    	global $error;
    	$FUNCNAME= 'upDiff';
    	$cpid = $this->cpid;
    
    	$usetab = "CONCRETE_PROTO_STEP";
    	$result = $this->isDiff( $sqlo,  $sqlo2, $blueid );
    	if (!$result) return ($result);
    
    	$apid = fSqlEasyC::get_ColByID( $sqlo, "CONCRETE_PROTO", $cpid, "ABSTRACT_PROTO_ID" );
    
    	// update protocol
    	$assoclib = new  fAssocUpdate();
    	$assoclib->setObj( $sqlo, $usetab, $cpid );
    	if ($error->Got(READONLY)) return;
    	
    	$assoclib->delMore( $sqlo );
    	$assoclib->resetTouched();
    
    	$sqlX1 = $this->_getStepSql($blueid);
    	$sqlo->Quesel( $sqlX1 );
    	while ( $sqlo->ReadArray() ) {
    		$argu = $sqlo->RowData;
    		$argu["ABSTRACT_PROTO_ID"] = $apid;
    		$assoclib->insert($sqlo2, $argu);
    	}
    	return 1;
    }
    
    /**
     * update a concrete_proto with steps in $stepArr
     * - no check of input data, e.g. existance of CONCRETE_SUBST_ID, DEV_ID and so on
     * @param array $stepArr step_params_NO_STRUCT
     */
    function protoUpdate( &$sql,  $aProtoId, $stepArr ) {
    
    	global $error;
    	$FUNCNAME = "protoUpdate";
    
    	$cprotoId = $this->cpid;
    	if ( !sizeof($stepArr) ) return;
    
    	$savopt = array( "defcols" => 1 );
    	
    	$this->protoManiLib->setCprotoID ( $sql, $cprotoId, $aProtoId );
    	$colmap = $this->protoManiLib->getColMap(); // COL => nice
    	             
    	foreach( $stepArr as $aProtoStepnr=>$colValArr) {
        
            $new = NULL;                       
            
    		foreach( $colmap as $key=>$colnice) {
    			// do NOT check, if the value is NULL ...
    			if ( array_key_exists($key, $colValArr) ) $new[$colnice] = $colValArr[$key]; 
    		}
    		
            $this->protoManiLib->oneStepSave( $sql, $new, $aProtoStepnr, $savopt ); 
            if ($error->Got(READONLY))  {
         		$error->set( $FUNCNAME, 1, "Error occurred on proto:$cprotoId step:$aProtoStepnr." );
    			return;
    		}
        }  
    }
    
    /**
     * transform $step_names_Arr (params by STEP_NAME) => $stepArr (params by STEP_NO)
     * $step_names_Arr = array( STEP_NAME => array( step_params) )
     * @param array $step_names_Arr
     */
    function step_arr_name2no(&$sqlo, $step_names_Arr) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if ( !$this->cpid ) {
            throw new Exception('Class not inizialized.');
        }
        
        $pra_id = glob_elementDataGet( $sqlo, 'CONCRETE_PROTO', 'CONCRETE_PROTO_ID', $this->cpid ,  'ABSTRACT_PROTO_ID'); 
        
        $pra_lib = new oABSTRACT_PROTO_stepx();
        $pra_lib->setObj($pra_id);
        $pra_step_info = $pra_lib->getStepArray_i2($sqlo, array('NAME') );
        if ($pra_step_info==NULL) return;
        
        
        $pra_key2no=array();
        foreach($pra_step_info as $step_no => $row) {
            $pra_key2no[$row['NAME']] = $step_no;
        }
        
        $stepArr=array();
        
        foreach($step_names_Arr as $step_name => $step_row) {
            $step_no = $pra_key2no[$step_name];
            if (!$step_no){
                $error->set( $FUNCNAME, 1, 'Step-Name "'.$step_name.'" not found in PRA-ID:'.$pra_id );
                return;
            }
            $stepArr[$step_no] = $step_row;
        }
        
        return $stepArr;
    }

}