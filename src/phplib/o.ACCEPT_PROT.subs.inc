<?php
require_once ("o.CCT_ACCLOG.subs.inc");

/**
 * analysis functions ACCEPT_PROT
 * @package o.ACCEPT_PROT.subs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq 0001724: o.ACCEPT_PROT > automatic connect
 */
class oACCEPT_PROT_subs {
	
    var $acpid;
	
    function __construct (&$sqlo) {
    	$this->H_ALOG_relid = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID($sqlo, 'ACCEPT_PROT', 'released');
    }
    
    /**
     * @param string $CODE_LOC e.g. 'o.ACCEPT_PROT.code_BeadQC'
     * @return string $class_name or  NULL
     */
    static function get_code_class($CODE_LOC) {  
        
        if ($CODE_LOC==NULL) return NULL;
        $full_url = $_SESSION['s_sessVars']['AppLabLibDir'].'/objlib/'.$CODE_LOC.'.inc';
        if ( !file_exists($full_url) ) {
            return NULL; // no code exists
        }
        require_once($full_url);
        $class_name = str_replace('.', '_', $CODE_LOC);
        return $class_name;
    }
    	
    /**
     * get array of ACCEPT_PROT_ID by ABSTRACT_PROTO_ID ($apid)
     * @param $sqlo
     * @param $apid
     * @param $option
     * 	'onlyReleased' : 0,1
     * @return array( of ACPID )
     */
    function getAcceptByAbstract(&$sqlo, $apid, $option=NULL) {
    	$protoArr=NULL;
    	
    	if ($option['onlyReleased']>0) {
    		// get only released protocols
    		if (!$this->H_ALOG_relid) return; // status missing
    		$sqlsel = "ACCEPT_PROT_ID from ACCEPT_PROT x join CCT_ACCESS a on x.CCT_ACCESS_ID=a.CCT_ACCESS_ID ".
    			  " where x.ABSTRACT_PROTO_ID= ".$apid. ' and a.H_ALOG_ACT_ID='.$this->H_ALOG_relid.' order by x.ACCEPT_PROT_ID';
    	} else {
    	
    		$sqlsel = "ACCEPT_PROT_ID from ACCEPT_PROT ".
    			  " where ABSTRACT_PROTO_ID= ".$apid. ' order by ACCEPT_PROT_ID';
    	}
    	$sqlo->Quesel($sqlsel);
    	while($sqlo->ReadRow()) {
    		$acpid = $sqlo->RowData[0];
    		$protoArr[] = $acpid;
    	}
    	return $protoArr;
    }
    
    /**
     * get NUMBER of ACCEPT_PROT by ABSTRACT_PROTO_ID ($apid)
     * - released and non released
     * @param $sqlo
     * @param $apid
     * @return int num of accept protos
     */
    function getNumAcceptByAbs(&$sqlo, $apid) {
    	$sqlsel = "count(ACCEPT_PROT_ID) from ACCEPT_PROT ".
    			  " where ABSTRACT_PROTO_ID= ".$apid;
    	
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadRow();
    	$ac_num = $sqlo->RowData[0];
    	
    	return $ac_num;
    }
    
    
    /**
     * set ACCEPT_PROT_ID
     * @param $acpid
     */
    function setAccProtoID($acpid) {
    	$this->acpid=$acpid;
    }
    
    
    /**
     * get relevant step numbers
     * @param $sqlo
     */
    function getRelevantStepNr(&$sqlo) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	if (!$this->acpid)  sys_error_my( $FUNCNAME.': no init' );
    	
    	$steparr=NULL;
    	
    	$sqlsel= "STEP_NR from ACCEPT_PROT_STEP where ACCEPT_PROT_ID=" .
    		$this->acpid." order by STEP_NR";
        $sqlo->Quesel($sqlsel);
    	while ($sqlo->ReadRow()) {
    		$stepnr = $sqlo->RowData[0];
    		$steparr[]=$stepnr;
    	}
    	
    	return $steparr;
    }
    
    /**
     * get complete data set
     * @return array $datarr[step_nr] = array('min'=>, 'max'=>);
     */
    function getAllStepData(&$sqlo) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	if (!$this->acpid)  sys_error_my( $FUNCNAME.': no init' );
    	
    	$datarr=NULL;
    	
    	$sqlsel= "STEP_NR, MIN_VAL, MAX_VAL from ACCEPT_PROT_STEP where ACCEPT_PROT_ID=" .
    		$this->acpid." order by STEP_NR";
        $sqlo->Quesel($sqlsel);
    	while ($sqlo->ReadRow()) {
    		$stepnr = $sqlo->RowData[0];
    		$datarr[$stepnr] = array('min'=>$sqlo->RowData[1], 'max'=>$sqlo->RowData[2]);
    	}
    	
    	return $datarr;
    }
    
    /**
     * check the concrete step
     * @param  $sqlo
     * @param  $step_nr step-nr in CONCRETE_PROTO_STEP and ACCEPT_PROT_STEP
     * @param array $cpFeats
     * 	  'QUANTITY'
     * @return array
     * 	  'ok' : 1 :ok
     * 			 <1: Error
     *    'errtxt' : error-text
     *    'used' => 0,1
     */
    function getLimitInfoOfStep( &$sqlo, $step_nr, $cpFeats, $use_step_flag) {
    	//global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	if (!$this->acpid)  sys_error_my( $FUNCNAME.': no init' );
    	
    	if (!$use_step_flag) {
    	    return array('ok'=>1, 'errtxt'=>'', 'used'=>0);
    	}
    	
    	$sqlsel= " *  from ACCEPT_PROT_STEP where ACCEPT_PROT_ID=" .
    		$this->acpid." AND STEP_NR=".$step_nr ;
        $sqlo->Quesel($sqlsel);
    	$sqlo->ReadArray();
    	$acceptFeat = $sqlo->RowData;
    	
    	if (is_numeric($acceptFeat['MIN_VAL'])) {
    		if (!is_numeric($cpFeats['QUANTITY'])) {
    		    return array('ok'=>-1, 'errtxt'=>'quantity is not a number', 'used'=>1);
    		}
    		if ($cpFeats['QUANTITY'] < $acceptFeat['MIN_VAL']) {
    		    return array('ok'=>-2, 'errtxt'=>'quantity exceeds minimum', 'used'=>1);
    		}
    	}
    	
    	if (is_numeric($acceptFeat['MAX_VAL'])) {
    		if (!is_numeric($cpFeats['QUANTITY'])) {
    		    return array('ok'=>-1, 'errtxt'=>'quantity is not a number', 'used'=>1);
    		}
    		if ($cpFeats['QUANTITY'] > $acceptFeat['MAX_VAL']) {
    		    return array('ok'=>-3, 'errtxt'=>'quantity exceeds maximum', 'used'=>1);
    		}
    	}
    	
    	return array('ok'=>1, 'errtxt'=>'', 'used'=>1);
    	
    }
    
    /**
     * get Limit-problems, derived by ACCEPT_PROT
     * @swreq SREQ:0001723:003: check limits (defined in ACCEPT_PROT)
     * @param $sqlo
     * @param $option
     *   'ignoreEmptyValues': 0,1 , if 1, ignore data with NULL data (this data will be given later)
     * @return array $limit_problems ('err'=>0,1, 'info'=>text)
     *  or NULL
     */
    public function getAllLimitProblemsOfSteps(&$sqlo, $cpid, $steparr, $option=NULL) {
        
        
        $accProID = $this->acpid;
        if (!$accProID) return; // no check needed

        if (empty($steparr)) return; // no steps to analyse
        
        $errCollect=array();
        $cpFeatArr =array();
        
        $sqlsel= "STEP_NR, QUANTITY from CONCRETE_PROTO_STEP where  CONCRETE_PROTO_ID=" .
            $cpid." AND STEP_NR in (". implode(',',$steparr) .")" ;
            
        $sqlo->Quesel($sqlsel);
        while ($sqlo->ReadArray() ) {
            $cpFeatArr[$sqlo->RowData['STEP_NR']] = array( 'QUANTITY'=>$sqlo->RowData['QUANTITY'] );
        }
        
        foreach( $steparr as $step_nr) {
            
            $cpFeats = $cpFeatArr[$step_nr];
            if ($option['ignoreEmptyValues']>0 and $cpFeats['QUANTITY']===NULL) {
                // ignore the test
            } else {
                $answer  = $this->getLimitInfoOfStep( $sqlo, $step_nr, $cpFeats, 1);
                if ($answer['ok']<1) {
                    $errCollect[]=$step_nr;
                }
            }
        }
        
        if (!empty($errCollect)) {
            return array('err'=>1, 'info'=>'Quantity-Limit-problem on step_nr: '.implode(', ',$errCollect));
        } else {
            return array('err'=>0, 'info'=>'');
        }
    }

}

/**
 * analyse ONE CONCRETE_PROTO
 *
 */
class oACCEPT_PROT_prc {
    
    private $calc_lib;
    private $relevantSteps;
    
    function __construct (&$sqlo, $cpid) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->relevantSteps=array();
        
        $this->cpid = $cpid;
        $this->cp_feats = glob_elemDataGet3( $sqlo, 'CONCRETE_PROTO', array('CONCRETE_PROTO_ID'=>$cpid), array('*') );
        $this->acpid    = $this->cp_feats['ACCEPT_PROT_ID'];
        
        if (!$this->acpid) {
            return;
        }
        $this->acp_feats = glob_elemDataGet3( $sqlo, 'ACCEPT_PROT', array('ACCEPT_PROT_ID'=>$this->acpid), array('*') );
        $this->code_class = oACCEPT_PROT_subs::get_code_class($this->acp_feats['CODE_LOC']);
        
        $this->acc_sub_lib = new oACCEPT_PROT_subs($sqlo);
        $this->acc_sub_lib->setAccProtoID($this->acpid);
        
        if ($this->code_class!=NULL) {
            // alternative calculation ...
            debugOut('Use QC-code-class : '.$this->code_class, $FUNCNAME, 2);
            $this->calc_lib = new $this->code_class();
            $this->calc_lib->set_cpid($sqlo, $cpid, $this->acc_sub_lib);
            $this->calc_lib->init($sqlo);
            $this->relevantSteps = $this->calc_lib->get_relevant_steps();
            
        } else {
            // get needed steps
            $this->relevantSteps = $this->acc_sub_lib->getRelevantStepNr($sqlo); 
        }
        debugOut('relevantSteps: '.print_r($this->relevantSteps, 1), $FUNCNAME, 2);
    }
    
    /**
     * get Limit-problems, derived by ACCEPT_PROT
     * @swreq SREQ:0001723:003: check limits (defined in ACCEPT_PROT)
     * @param $sqlo
     * @param $option
     *   'ignoreEmptyValues': 0,1 , if 1, ignore data with NULL data (this data will be given later)
     * @return array $limit_problems ('err'=>0,1, 'info'=>text)
     *  or NULL
     */
    public function getAllLimitProblems(&$sqlo, $option=NULL) {

        $cpid = $this->cpid;
        $accProID = $this->acpid;
        if (!$accProID) return; // no check needed
        
        if ($this->code_class!=NULL) {
            return $this->calc_lib->calc($sqlo);
        }

        return $this->acc_sub_lib->getAllLimitProblemsOfSteps($sqlo, $cpid, $this->relevantSteps);
    }
    
    /**
     * 
     * @param object $sqlo
     * @param array $step_nr
     * @return array
     * 	  'ok' : 1 :ok
     * 			 <1: Error
     *    'errtxt' : error-text
     *    'used' : 0,1
     */
    public function getLimitInfoOfStep( &$sqlo, $step_nr) {
        
        if (!$this->acpid) {
            return;
        }
        
        $cpid  = $this->cpid;
        $sqlsel= "*  from CONCRETE_PROTO_STEP where  CONCRETE_PROTO_ID=" .$cpid." AND STEP_NR=".$step_nr ;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadArray();
        $cpFeats = $sqlo->RowData;	
        
        $use_step_flag=0;
        if (in_array($step_nr, $this->relevantSteps)) $use_step_flag=1;
        
        return $this->acc_sub_lib->getLimitInfoOfStep( $sqlo, $step_nr, $cpFeats, $use_step_flag);
    }
}
