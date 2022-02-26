<?php
require_once ("f.assocUpdate.inc");

/**
 * manipulation functions for steps of ACCEPT_PROT
 * - the functions handle exact ONE step 
 * - update or insert one ACCEPT_PROT_STEP
 * - main functions: setCprotoID() oneStepCreate() oneStepSave()
 * @package o.ACCEPT_PROT.mod.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oACCEPT_PROT_mod {
	
function __construct () {
	$this->allow = array('MIN_VAL', 'MAX_VAL', 'NOTES');
}
	
function setCprotoID ( &$sqlo, $cpid, $acheck=0 ) {
	global $error;
	$FUNCNAME="setCprotoID";
	
	$this->info = NULL;
	$this->cpid = $cpid;
	
	$apid = glob_elementDataGet( $sqlo, 'ACCEPT_PROT', 'ACCEPT_PROT_ID', $cpid, 'ABSTRACT_PROTO_ID');
	$this->apid = $apid;
	
	$this->assoclib = new  fAssocUpdate();
	$asopt = NULL;
	if ( !$acheck ) $asopt['noAccCheck']=1;
	$asopt['oneTouch'] = 1;
	$this->assoclib->setObj( $sqlo, 'ACCEPT_PROT_STEP', $cpid, $asopt );
	
	
}


/**
 * - create a single empty protocol step (if not exist)
   - with touch management
 * @param $sql
 * @param $step_nr
 * @param $opt
 * @return  flag, if created 0|1
 */
function oneStepCreate( 
	&$sql, 
    $step_nr,
	$opt  = NULL
	) {
	
	global $error;
    $FUNCNAME= "oneStepCreate";
	
	$cpid = $this->cpid;
	$apid = $this->apid;

	
	$sqls= "select 1 from CONCRETE_PROTO_STEP ".
           " where CONCRETE_PROTO_ID=".$cpid. " AND STEP_NR=" . $step_nr;
    $sql->query($sqls);
    if ( $sql->ReadRow() ) { 
		return (0); // already exists
	}
	
	$argu = NULL;
	$argu["ABSTRACT_PROTO_ID"] = $apid;
	$argu["STEP_NR"] = $step_nr;
	
	$this->assoclib->insert( $sql, $argu, $argu );
	return (1);
}

/**
 * - update OR create one proto_step, identified by $step_nr
 * - if  $opt["acheck"] => check access and touch protocol
 * @param object $sql
 * @param array  $new   array of values: 
 * 		 ["MIN_VAL"], ["MAX_VAL"], ["NOTES"]
 *   - must be an array !
 * @param int    $step_nr
 * @param array  $opt
 * @return $info info-string
 */
function oneStepSave( 
    &$sql, 
    $new,
    $step_nr,
    $opt  = NULL
    ) {
    global $error;
	$FUNCNAME= "oneStepSave";
    
	$cpid = $this->cpid;
	$apid = $this->apid;
	
    $formHasData = 0;
    $dbHasStep   = 0;
	
	
	foreach( $new as $key=>$val) {
		if (!in_array($key, $this->allow)) {
			$error->set( $FUNCNAME, 1, 'key "'.$key.'" not allowed' );
			return;
		}
	}
	reset ($new);
	
	if ( is_numeric($new["MIN_VAL"]) or is_numeric($new["MAX_VAL"]) or $new["NOTES"]!=NULL ) {
		$formHasData = 1;
	}
	
	$sqls = "select 1 from ACCEPT_PROT_STEP ".
            " where ACCEPT_PROT_ID=".$cpid. " AND STEP_NR=" . $step_nr;
    $sql->query($sqls);
    if ( $sql->ReadRow() ) { 
    	$dbHasStep = 1;
    }
	
	
	
	// check if step will be deleted
	if ( $dbHasStep and !$formHasData ) {
		// delete row, if no values given
		// this is needed, if you want to remove an abstract-protocol step
		$idarr = NULL;
		$idarr["STEP_NR"]           = $step_nr;
		$idarr["ABSTRACT_PROTO_ID"]= $apid; 
		$this->assoclib->delOneRow( $sql, $idarr );
		return;
	}
	
	if (!$formHasData) {
		return; // no action
	}

    if ( $dbHasStep ) { 
        
		$argu  = NULL;
		$idarr = NULL;
		$idarr["STEP_NR"]          = $step_nr;
		$idarr["ABSTRACT_PROTO_ID"]= $apid; 

		// update each column, even if the value is not set !
		foreach( $new as $key=>$val) {
			$argu[$key]	= $val;
		}
		reset ($new);
		
		$retval = $this->assoclib->update( $sql, $argu, $idarr ); 
		$this->info .= " updated (retval:$retval)";
            	
        
    } else { 
    
		$argu = NULL;
		$argu["ABSTRACT_PROTO_ID"] = $apid;
		$argu["STEP_NR"]  = $step_nr;
		foreach( $new as $key=>$val) {
			$argu[$key]	= $val;
		}
		reset ($new);
	
		$retval = $this->assoclib->insert( $sql, $argu);
            
            $this->info   .= " inserted ($retval) ";
            
        
    } 
	
    return;
}   

}
