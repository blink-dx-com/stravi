<?php
//  * $Header: trunk/src/phplib/o.CONCRETE_SUBST.xadd.inc 59 2018-11-21 09:04:09Z $
require_once 'lev1/o.SUA.versman.sub.inc';
require_once('glob.obj.create.ABS.inc');
require_once 'f.namePolicy.inc';
require_once ("glob.obj.update.inc");
require_once 'class.filex.inc';

/**
 * object specific modification class
 * @package oCONCRETE_SUBST.xadd.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq SREQ:0003292: glob.obj.crea_wiz.php > o.CONCRETE_SUBST : Regeln 
 */
class oCONCRETE_SUBST_xmodC extends gObjCreateC_ABS {
	
	var $infox;

    /**
     * before CREATION of object check/prepare arguments
     * manipulates this array, format see also object.info.inc 
      format: array ( "vals" => data, "xobj" => extra_obj_data, "access" => access_data)
     */
    function crea_prep(&$sqlo, &$obj_param ) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$a_tablename='ABSTRACT_SUBST';
    	
    	$sqlDateNow = date_unix2sql( time() );
    	$obj_param["vals"]["SOME_DATE"] = $sqlDateNow; // set current date	
    
    	$suaid    = $obj_param["vals"]["ABSTRACT_SUBST_ID"];
    	
    	$sqlsel = '* from ABSTRACT_SUBST where ABSTRACT_SUBST_ID='.$suaid;
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadArray();
    	$suaFeats  = $sqlo->RowData;
    	
    	if ($suaFeats['CCT_ACCESS_ID']>0) {
    	    
    		// check Audit-Status, if exists
    		$auditLib = new oAccLogC();
    		$auditLib->setObject( $sqlo, $a_tablename, $suaid, $suaFeats['CCT_ACCESS_ID'] );
    		$auditStatusName = $auditLib->getLastLogName($sqlo);
    		if ($auditStatusName=='deactivated') {
    			$error->set( $FUNCNAME, 4, 'Parent SUA [ID:'.$suaid.'] has status="deactivated". Creation of a SUC from this SUA is not allowed.'.
    					' Ask the owner of this SUA!' );
    			return;
    		}
    	}
    	
    	if ($suaFeats['IS_META']>0) {
    		$error->set( $FUNCNAME, 1, 'creation of a SUC from a META-SUA (is_meta=1) not allowed!' );
    		return;
    	}
    	
    	if ($suaFeats['VERSFLAG']==1) {
    		// @swreq UREQ:0003291:C003 creation of a SUC
    		$error->set( $FUNCNAME, 2, 'creation of a SUC from a VersionMaster-SUA (VERSFLAG=1) not allowed!' );
    		return;
    	}
    	
    	if ($suaFeats['VERSFLAG']==2) {
    		
    	    $cert_flag = $obj_param["vals"]["CERT_FLAG"];
    	    if ( $cert_flag==2 or $cert_flag==3) {
    			// o.k., all time allowed, no version check !
    			// @swreq UREQ:0003291:C005: exception: if LCS=3 (Forschung, Entwicklung) allow creation from NON-released SUA !!!
    		} else {
    			// @swreq UREQ:0003291:C004 on SUC-creation: check, if SUA-version is valid (by DATE) !!!
    			$versLib = new oSUA_versman_child();
    			$versLib->setChild($sqlo, $suaid);
    			if ($versLib->getValidFlag() !=2) {
    				$error->set( $FUNCNAME, 3, 'SUA [ID:'.$suaid.'] ist versioniert, aber wurde nicht aktiviert! '.
    					'Bitte aktivieren. Optional: w&auml;hle LCS=Forschung' );
    				return;
    			}
    		}
    	}
    }
    
    /**
     *
     * @param object $sqlo
     * @param string $saved_sel
     * @param int $obj_blueprint_id
     * @param int $obj_tmpl_id
     * @param $options
     *   'name_policy' => array( 'MORDER.name'=>$order_name )
     *   
     * @return array('name'=>, 'policy_has'=>0,1)
     */
    private function _SUC_name_of_policy(&$sqlo, $obj_id, $options=array()) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $tablename='CONCRETE_SUBST';
        $policyStr= NULL;

        $feats  = glob_elemDataGet3( $sqlo, 'CONCRETE_SUBST', array('CONCRETE_SUBST_ID'=> $obj_id), array('ABSTRACT_SUBST_ID','NAME', 'BATCH') );
        $sua_id = $feats['ABSTRACT_SUBST_ID'];
        $policyStr = glob_elementDataGet( $sqlo, 'ABSTRACT_SUBST', 'ABSTRACT_SUBST_ID', $sua_id, 'NAMEPOL');
        
        
        if ($policyStr==NULL) return;
        
        if ($error->Got(READONLY))  {
            return; // error exists before ...
        }
        
        
        $name_pol_lib   = new fNamePolicy('CONCRETE_SUBST', $sua_id);
        $timeStamp      = time();
        $paramarr       = array('DATE'=>$timeStamp, 'ID'=>$obj_id, 'BATCH'=>$feats['BATCH'] );
        if (is_array($options)) {
            if (is_array($options['name_policy'])) {
                $paramarr['MORDER.name']=$options['name_policy']['MORDER.name'];
            }
        }
        $policyArray    = $name_pol_lib->analysePolicyString($policyStr);
        $new_name       = $name_pol_lib->generate_name_POST($sqlo, $policyArray, $paramarr);
        
        debugOut('has fNamePolicy? YES; new-name:'.$new_name, $FUNCNAME, 2);

        if ($error->Got(READONLY))  {
            // ignore this error ...
            // TBD generate a warning ?
            $error->logError();
            $error->reset();
            $new_name  = $feats['NAME']; // fallback ...
        }
        
       
        if ( fileC::redundancyTest2( $sqlo, $tablename, array('NAME'=>$new_name, 'ABSTRACT_SUBST_ID'=>$sua_id), $obj_id) ) {
            // check, if object with name already exists
            // create alternative name ...
            // try 50 times ...
            $cnt=0;
            $MAX_CNT=50;
            do {
                if ($cnt>$MAX_CNT) {
                    $error->set( $FUNCNAME, 1, 'object with name "'.$new_name.'" already exists. stop after '. $cnt . ' alternative names.');
                    return;
                }
                $loop_name = $new_name.' ('.($cnt+2).')';
                if ( !fileC::redundancyTest2( $sqlo, $tablename, array('NAME'=>$loop_name,  'ABSTRACT_SUBST_ID'=>$sua_id), $obj_id) ) {
                    $new_name = $loop_name;
                    break;
                }
                $cnt++;
            } while (1);
        }
        
        if ($feats['NAME']!=$new_name) {
            //TBD: what if name already exists ...
            $argu =array('NAME'=>$new_name);
            $idarr=array('CONCRETE_SUBST_ID'=> $obj_id);
            gObjUpdate::update_row_s( $sqlo, $tablename, $argu, $idarr);
        }
        
        
    }
    
    function post_crea( &$sqlo,  &$sqlo2, $objid,  $blueObjId, $option  = array()	) {
        $this->_SUC_name_of_policy($sqlo, $objid,  $option);
    }

}
