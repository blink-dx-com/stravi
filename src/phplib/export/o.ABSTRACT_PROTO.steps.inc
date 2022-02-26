<?php
require_once ('f.debug.inc');
require_once ('o.ABSTRACT_PROTO.stepx.inc');

/**
 * class for export of ABSTRACT protocol steps
 * @namespace core::
 * @package o.ABSTRACT_PROTO.steps.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oAbsProtoStepsExp {
	var $tabhead; // table header 
	
	/**
	 * init
	 * @param array $options 'ids' => 0,1 : with IDs ?
	 */
function __construct($options=NULL) {
	$this->aProtoLib = new  oABSTRACT_PROTO_stepx();
	$this->options=$options;
	$this->withIDs=0;
	if ($this->options['ids']>0) $this->withIDs=1;
	
}

function setProto( $proto_id ) {
	$this->abstr_proto_id = $proto_id;
	$this->aProtoLib->setObj($proto_id);
}


/**
 * 
 * @return array of head
 */
function tableStart() {
	
	$tabhead = NULL;
	if ($this->options["step_nr"] ) {
	    $tabhead[] = array("STEP_NR","step_nr");
	}
	$tabhead[] = array("NAME","name");
	$tabhead[] = array("ABSTRACT_SUBST_NAME","substance");
	if ($this->withIDs) $tabhead[]=array('ABSTRACT_SUBST_ID','substance ID');
	
	$tabhead[] =  array('QUANTITY',"quantity");
	$tabhead[] =  array('H_UNIT_NAME',"measurement unit");
	$tabhead[] =  array('ABS_DEV_NAME',"device");
	if ($this->withIDs) $tabhead[]=array('ABS_DEV_ID','device ID');
	$tabhead[] = array('NOTES', "notes");
	$tabhead[] = array('MANDATORY_VALUES', "flags");
	
	$outarr=array();
	foreach( $tabhead as $valarr) {
	    $outarr[] = $valarr[1];
		
	}

	$this->tabhead = $tabhead;
	return $outarr;
}

/**
 * get a RAW ABSTRACT single step of ONE protocol

 * @param object $sql
 * @param int $step_nr
 * @param array $opt
 * @return array of values
 */
function outStepAbsRaw(&$sql, $step_nr, $opt = array() ) {
   
    if (!$this->abstr_proto_id) {
        throw new Exception('Class not initialized.');
    }
    
    $tabhead = &$this->tabhead;
    
	//$sepx = "\t";
    //$retx = NULL;
    //$proto_id    = $this->abstr_proto_id;
    //$step_nr_par = $step_nr;
    //if ($opt["step_nr_par"]) $step_nr_par = $opt["step_nr_par"];

	
	$valx = $this->aProtoLib->step_info( $sql, $step_nr, 1);
	
	// show step
	if ($valx["SUB_STEP_NR"]>0) { // is it a SUB step ? 
	    $valx['NAME'] =  "   ".$valx['NAME']; // add SPACER
	    if ($valx["SUB_STEP_NR"]>1) { 
	        $valx['NAME'] =  "   ".$valx['NAME']; // add more SPACER
	    }
		//$tmp_step = $this->sub_step_cnt;
		$this->sub_step_cnt++;
	} else {
		//$tmp_step = $this->main_step_cnt;
		$this->main_step_cnt ++;
		$this->sub_step_cnt=1;	
		
	}
	$tmpnotes = $valx["NOTES"];
	$tmpnotes = str_replace("\r\n","|",$tmpnotes);
	$tmpnotes = str_replace("\n","|",$tmpnotes);
	// fDebugC::str2Hexinfo( $tmpnotes );
	
	$valx["NOTES"] = str_replace("\t"," ",$tmpnotes);
	
	
	//$cnt=0;
	$outarr=array();
	foreach( $tabhead as $valarr) {
		//if ($cnt>0) echo $sepx;
	    $outarr[] = $valx[$valarr[0]];
		
	}
	return  $outarr;
	
}

}
