<?php
/**
 * - for object creation wizard
 * - object dependent creation functions
 * - optional include $_SESSION['s_sessVars']['AppLibDir']. "/objlib/o.".$tablename.".xadd.inc";
 * - to start the workflow see class gObj_StatusMod
 * 
 * @namespace core::db
 * @package glob.obj.copyobj2.inc
 * @swreq   UREQ:0000972 glob.obj.crea_wiz.php > Object creation wizard (MOTHER-REQ) 
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @unittest exists
 */

require_once ("o.DB_USER.subs.inc");
//require_once ("o.EXP.crea.inc");
require_once ("object.subs.inc");
require_once ("insertx.inc");
require_once ('glob.obj.create.inc');
require_once ('o.PROJ.addelems.inc');
require_once ('glob.image.inc');
require_once ('date_funcs.inc');
require_once ('glob.obj.conabs.inc');
require_once ('db_x_obj.inc');
require_once ('object.info.inc');
require_once ('o.H_ALOG_ACT.subs.inc');

/**
 * object copy
 * @package glob.obj.copyobj2.inc
 * @author steffen
 *
 */
class objCreaWiz {
	
var $creaInfo; // info text array for one object
			   // array[] = text

function __construct( $tablename ) {
	$this->tablename=$tablename;
	
	$this->copyAssocLib = new fCopyAssoc(); 
	$this->objCreaLib   = new gObjCreateC($tablename);
	$this->objConAbsLib = new gConcAbsC();
	
	$this->sqlDateNow = date_unix2datestr( time(), 1 ); // in seconds for perfect time
	
}

       
function getInfo() {
	// FUNCTION: get $this->creaInfo as text with <br> tags
	if ( empty($this->creaInfo) ) return;
	
	$retstr = "";
	foreach( $this->creaInfo as $txt) {
		$retstr .= "- ".$txt . "<br>\n";
	}
	
	return ($retstr);
}

function getInfoArr() {
	return ($this->creaInfo);
}

/**
 * object type specific actions for create BEFORE new values are set
 * @param $sql
 * @param $tablename
 * @param $obj_blueparam -- input/output parameters
 */
private function _objSpec_pre_new(&$sqlo, $tablename, &$obj_blueparam) {
	//global $error;
	//$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	switch ($tablename) {
		
		case "ABSTRACT_SUBST":
			// @swreq UREQ:0003291:001: on CreationWizard: copy object: do NOT copy attribute: "VERSFLAG"
			unset($obj_blueparam["vals"]["VERSFLAG"]); // managed via PLM !
			
			break;
		
		case "CONCRETE_SUBST":
			unset($obj_blueparam["vals"]["CHARGE"]);            // never copy the SAP-number !!!
			break;
	
		case "CHIP_READER":
		   unset($obj_blueparam["vals"]["CONCRETE_PROTO_ID"]); // should be unique !!!
		   unset($obj_blueparam["vals"]["QC_CPROTO_ID"]);      // never copy !!!
		   break;
	
		
			 
		case "MORDER":
		    unset($obj_blueparam["vals"]["REQU_DATE"]);   // never copy !!!
		    unset($obj_blueparam["vals"]["START_DATE"]);  // never copy !!!
		    unset($obj_blueparam["vals"]["END_DATE"]);    // never copy !!!
		    break;
	}
}

/**
 * create real objects in assoc table
 * @param string $tablename
 * @param array $objAssCrea
 *   TABLE => 1
 * @return NULL|string[][]|number[][]
 */
private function assCreaRenewParams($tablename, $objAssCrea) {
    $newarr=NULL;
    
    switch ($tablename) {
        case "EXP":
            foreach( $objAssCrea as $key=>$val) {
                $deep    =0;
                $keyname ="";
                $objtab  ="";
                
                if ($key=="EXP_HAS_PROTO") {
                    $keyname ="CONCRETE_PROTO_ID";
                    $objtab ="CONCRETE_PROTO";
                    $deep=1;
                    $t1arr = array($keyname => array("tab"=>$objtab, "deep"=>$deep) );
                }
                if ($key=="IMG_OF_EXP" ){
                    if (!table_exists2("IMG_OF_EXP") ) continue;
                    $keyname = "IMG_ID";
                    $objtab  = "IMG";
                    $deep	 = 0;
                    $t1arr = array($keyname => array("tab"=>$objtab, "deep"=>$deep) );
                }
                $newarr[$key] = $t1arr;
            }
            reset($objAssCrea);
            break;
            
        case "CONCRETE_SUBST":
            
            foreach( $objAssCrea as $key=>$val) {
                $deep    =0;
                $keyname ="";
                $objtab  ="";
                
                if ($key=="CS_HAS_PR") {
                    $keyname ="CONCRETE_PROTO_ID";
                    $objtab ="CONCRETE_PROTO";
                    $deep=1;
                    $t1arr = array($keyname => array("tab"=>$objtab, "deep"=>$deep) );
                }
                
                $newarr[$key] = $t1arr;
            }
            
            break;
    }
    return ($newarr);
}



 
/**
 * create object from EXISTING obj
 * - use object dependent class oTABLE_xmodC
 * - check audit-status of TMPL-object (deny, if rejected)
 * 
 * @param &$sql, 
 * @param &$sql2, 
 * @param &$sql3,		   -- needed for $option["objAssCrea"];
 * @param $tablename, 
 * @param $obj_blueprint_id, 
 * @param array $new_args  -- new object parameters:  TYPEOF insertx.inc:objFeatStruct  (OLD: $new_params  array[COL]=VAL)
 * @param array $deep_copy -- copy associated lists
 *    array[TABNAME] = 1 : TABNAME = name of assoc tables
 * @param $option = NULL  : options 
 *  "proj_id"     = string : destination project
 *  "objAssCrea"  = array[TABNAME] = 1 : array of ASSOC tables for REAL object copy
 *  "objAssFrgn"  = array of foreign-assoc-tables, 
 *     where the ID is not the primary key in the ASSOC_TABLE (e.g.DB_USER_IN_GROUP for DB_USER) 
 *  "optspecial" : e.g. "showpasswd" : DEPRECATED
 *  "info" 0,1,2,3 : info-level
 * @return $id
 * @global  $this->creaInfo
 * @exceptions
 *   1, "please select an $table_nicename"
 *   2, "Parameter preparation failed."
 *   3, "object not created"
 *   4, "insert of associated elements failed"
 *   5, "insert of associated FOREIGN elements failed"
 *   6, "copy of image file failed."
 *   7, "Prep audit status failed"
 *   8, "Copy to project ".$proj_id." failed."
 *   9, "Parameter preparation failed."
 *   10, "additional object action failed"
 *   15, "insert of associated FOREIGN elements failed"
 */
function objCreate( &$sql,  &$sql2, &$sql3,	$obj_blueprint_id, $new_args, $deep_copy, $option = NULL ) {	     
 	global $error;
 	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$tablename=$this->tablename;
	$this->creaInfo = NULL;
	
	
	$objModLib = NULL;
	      
	if ( !$obj_blueprint_id ) {
		$error->set($FUNCNAME, 1, "Input missing: obj_blueprint_id.");
		return;
	}
	$pkname = PrimNameGet2($tablename);
	
	// FUTURE: put this to directory "objlib" !
	$xtra_file = $_SESSION['s_sessVars']['AppLibDir']. "/objlib/o.".$tablename.".xadd.inc";
	if ( file_exists($xtra_file) ) {
	    require_once($xtra_file);  //  TABLE specific extra methods
	    $dynClassName = 'o'.$tablename.'_xmodC';
	    $objModLib    = new $dynClassName();
	    
	}
	
	
	if ( $option["objAssCrea"]!=NULL ) {
		$objAssCrea = $option["objAssCrea"];
	}
	
	if ( $tablename=="DB_USER" ) {  // always do specials
		$option["objAssFrgn"] = NULL;
		$option["objAssFrgn"]["DB_USER_IN_GROUP"] = "DB_USER_IN_GROUP";
		$option["objAssFrgn"]["USER_ROLES"]       = "USER_ROLES";
		$deep_copy = array ("USER_PREF"=>1);
		
	}

	if ( $tablename=="CONCRETE_SUBST" ) { 
	    if ($deep_copy['CS_HAS_PR']>0) {
	       $objAssCrea['CS_HAS_PR']=1;  // do always deep,deep copy
	    }
	}
	
	// parameter check: if $objAssCrea is set, set also $deep_copy
	if (!empty($objAssCrea)) {
		
		foreach( $objAssCrea as $key=>$val) {
			if ($deep_copy[$key]=="") $deep_copy[$key] = 1; //$error->set($FUNCNAME, 10, "You can NOT select the same REAL COPY element and a COPY element for table '".tablename_nice2($key)."'!");
		}
		
	}
	if ( $error->got(READONLY) ) {
		return;
	}
	
	if ($option["proj_id"]) $proj_id = $option["proj_id"];
		
	$goption=array();
	$goption["xObjParById"]=1;
	$goption["vario"]      =1;  // also VARIO values
	$obj_blueparam = objInfoC::getParamsRaw($sql, $tablename, $obj_blueprint_id, $goption);
	
	// $ori_params = $obj_blueparam["vals"];   // cache them 
	$ori_access = $obj_blueparam["access"]; // cache them
	
	//
	// TBD: move ALL table-specific actions to class oTABLE_xmodC; function xadd_prep() !!!
	//
	$this->_objSpec_pre_new($sql, $tablename, $obj_blueparam);
	if ($error->Got(READONLY)) {
	    $error->set($FUNCNAME, 9, "Parameter preparation failed.");
	    return;
	}
	
	// manipulate values
	if ( is_array($new_args) ) {
		// set new "vals"
	    if (!empty($new_args['vals']))
	        foreach( $new_args['vals'] as $key=>$val) {
    			$obj_blueparam["vals"][$key] = $val;
    		}
	    
	    if (!empty($new_args['xobj'])) {
	        // set new "xobj"
	        if (array_key_exists('extra_class_id', $new_args['xobj']) )  $obj_blueparam['xobj']['extra_class_id'] = $new_args['xobj']['extra_class_id'];
	        if (array_key_exists('values', $new_args['values']) ) {
	            foreach( $new_args['xobj']['values'] as $key=>$val) {
    	            $obj_blueparam['xobj']["values"][$key] = $val;
    	        }
	        }
	    }
	    // TBD: vario ...
	}
	
	unset($obj_blueparam["vals"][$pkname]); // unset primary key
	unset($obj_blueparam["vals"]["EXTRA_OBJ_ID"]);
	unset($obj_blueparam["vals"]["CCT_ACCESS_ID"]);
	unset($obj_blueparam["access"]);

	
	if ($objModLib!=NULL) {
    	if ( method_exists($objModLib, 'copy_prep') ) {
    	    $objModLib->copy_prep( $sql, $obj_blueparam ); // call specific add preparation
    	    if ($error->Got(READONLY)) {
    	        $error->set($FUNCNAME, 2, "Parameter preparation failed.");
    	        return;
    	    }
    	}
	}
	if ( cct_access_has2($tablename) ) {
		// only possible for business objects ...
		$obj_blueparam['acc'] = array('ORI_ID' => $obj_blueprint_id); // save the original object id of the copy
	}
	
	// check tmpl for audit status ...
	list($tmpl_table, $tmpl_colname) = $this->objConAbsLib->getAbsColName($tablename);
	if ($obj_blueparam['vals'][$tmpl_colname]) {
	    $this->ana_audit_of_tmpl($sql, $tmpl_table, $obj_blueparam['vals'][$tmpl_colname]);
	    if ($error->Got(READONLY)) {
	        $error->set($FUNCNAME, 7, "Error on Obj-Creation.");
	        return;
	    }
	}
	
	// use this method, because "post_crea" will be called later
	$obj_new_id =  $this->objCreaLib->create_objX_RAW ($sql, $tablename, $obj_blueparam );
	if ($obj_new_id AND $proj_id) {
		$this->creaInfo[] = "... add object to project ".$proj_id;
		$projLib = new oProjAddElem( $sql, $proj_id );
		if ($error->got(READONLY)) {
			$error->set($FUNCNAME, 8, "Copy to project ".$proj_id." failed.");
	    	return;
	    }
		$projLib->addObj( $sql,  $tablename, $obj_new_id); 
		
    }
	
	
	if (!$obj_new_id OR $error->got(READONLY)) {
		$error->set($FUNCNAME, 3, "object not created");
		return ($obj_new_id);
	}
	
	// copy the WORKFLOW, if exists
	if ( cct_access_has2($tablename) ) {
		$x_params=array();
		$x_params['CCT_ACCESS_ID'] = glob_elementDataGet( $sql, $tablename, $pkname, $obj_new_id, 'CCT_ACCESS_ID');
		if ($ori_access['WFLOW_ID'] and $x_params['CCT_ACCESS_ID']) {
		    $sql->Updatex('CCT_ACCESS', 'WFLOW_ID='.$ori_access['WFLOW_ID'].' where CCT_ACCESS_ID='.$x_params['CCT_ACCESS_ID']);
			if ($error->Got(READONLY)) {
				$error->set($FUNCNAME, 15, "Update of Workflow failed");
				return ($obj_new_id);
			}
		}
	}
	
	// warning: only allowed for user root and table DB_USER
	// TBD: may be other objects use objAssFrgn: is_array($option["objAssFrgn"])
	if ( is_array($option["objAssFrgn"]) AND $tablename=="DB_USER") {
		$retarr = $this->copyAssocLib->new_assocFrgn(
						$sql, $sql2, $tablename, $obj_blueprint_id, $obj_new_id, $option["objAssFrgn"]);
		if ($error->Got(READONLY)) {
			$error->set($FUNCNAME, 5, "insert of associated FOREIGN elements failed");
			return ($obj_new_id);
		}
	}
	
	if ( is_array($deep_copy) ) {
		$objAssCreaNew=NULL;
		$asOpt        =NULL;
	
		
		if ( $objAssCrea ) {
			$objAssCreaNew = $this->assCreaRenewParams($tablename, $objAssCrea);
			$asOpt["sqlhandle"] = &$sql3;
		}

		
		$retarr = $this->copyAssocLib->new_assoc($sql, $sql2, $tablename, $obj_blueprint_id, $obj_new_id, 
					$deep_copy,  $objAssCreaNew, $asOpt);
		if (!empty($retarr)) {
			$this->creaInfo[] = "... copied associated elements:";
			foreach( $retarr as $key=>$val) {
				$this->creaInfo[] =  "&nbsp;&nbsp;&nbsp;- ".tablename_nice2($key).": <B>$val</B> elements";
			}
		}
		if ($error->Got(READONLY)) {
			$error->set($FUNCNAME, 4, "insert of associated elements failed");
			return ($obj_new_id);
		}
	}
	
	
	// POST CREATION ..
	if ($objModLib!=NULL) {
	    if ( method_exists($objModLib, 'post_crea') ) {
	        $objModLib->post_crea(  $sql, $sql2, $obj_new_id, $obj_blueprint_id, $option ); 
	        if ($error->Got(READONLY)) {
	            $error->set($FUNCNAME, 10, "post_crea  action failed");
	            return ($obj_new_id);
	        }
	    }
	}
	
	
	if ( $option["attachments"]>0 ) { // copy
		if ($tablename=="IMG") {
			if (imgOnServerExists( $obj_blueprint_id  ) ) {
				$path_src = imgPathFull( $obj_blueprint_id );
				$path_new = imgPathFull( $obj_new_id );
				$this->creaInfo[] = "copy image file";
				if (!copy ($path_src, $path_new) ) {
					$error->set($FUNCNAME, 6, "copy of image file failed.");
				}
			}
		} 
	}
	
	return ($obj_new_id);
}




/**
 * if SUA is a VersionMaster return the active version of SUA
 * @param  $sqlo
 * @param  $suaid
 * @param  array $newparams
 * @return int $newSUA
 */
private function _SUA_VersTrans(&$sqlo,$suaid) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$versLib  = new oSUA_versman_sub();
	$versType = $versLib->getSUA_type($sqlo, $suaid);
	
	if ($versType!=1) return $suaid;
	
	$newSUA = $versLib->getValidSuaFrMaster($sqlo, $suaid);
	if (!$newSUA) {
		$error->set( $FUNCNAME, 2, 'fuer den VersionMaster [SUA-ID:'.$suaid.'] gibt es keine aktivierte Version!' );
		return;
	}
	return $newSUA;
}

private function ana_audit_of_tmpl($sqlo, $tmpl_tablename, $obj_tmpl_id) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    $rawopt = array('cols'=>array('CCT_ACCESS_ID'));
    $acc_params = objInfoC::getParamsRaw ( $sqlo, $tmpl_tablename, $obj_tmpl_id, $rawopt);
    if ($acc_params['access']['H_ALOG_ACT_ID']) {
        $status_name = oH_ALOG_ACT_subs::statusID2Name($sqlo, $acc_params['access']['H_ALOG_ACT_ID']);
        if ($status_name=='rejected') {
            $short = globTabMetaByKey($tmpl_tablename, 'SHORT');
            $error->set( $FUNCNAME, 1, 'The Template-object ['.$short.'-ID:'.$obj_tmpl_id.'] (Audit-Status=rejected) does NOT allow you to create a new MAC.' );
            return;
        }
    }
}


/**
 * - create object from TEMPLATE
 * - optional add object to project
 * - CHECKS:
 *   - check audit-status of $obj_tmpl_id
 * @param string $tablename, 
 * @param int $obj_tmpl_id,
 * @param array  $proj_id    = NULL,
 * @param array  $argu [OPTIONAL]   TYPEOF insertx.inc:objFeatStruct 
 *   - NAME must be part of this array ...
 *   - e.g. CONCRETE_SUBST:CERT_FLAG
 * @param array $crea_opt
 * @return $obj_new_id
 */
function objCreateFrTmpl( &$sqlo, $sqlo2, $obj_tmpl_id,  $proj_id= NULL, $argu = array(), $crea_opt=array() 	) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	
	$tablename = $this->tablename;
	
	// analyse audit-status
	$tmpl_tabname = $this->objConAbsLib->getTemplate($tablename);
	if ( cct_access_has2($tmpl_tabname) ) {
	    $this->ana_audit_of_tmpl($sqlo, $tmpl_tabname, $obj_tmpl_id);
	}
	
	//FUTURE bring to *xadd.inc"
	switch ( $tablename ) {
	
		case "EXP":
			
			//$tmpTmplLib = new oEXPcreaC();
			//$argu = $tmpTmplLib->getArguByTmpl( $sqlo, $obj_tmpl_id );
			// $argu["NAME"] 	     = $obj_name;
		    if (empty($argu['vals'])) $argu['vals']=array();
			$argu['vals']["EXP_TMPL_ID"] = $obj_tmpl_id;

			break;
			
		case "CONCRETE_SUBST":

			require_once 'lev1/o.SUA.versman.sub.inc';
			// @swreq SREQ:0003291:003: creation of a SUC from a VersionMaster-SUA (VERSFLAG=1) 
			//        on CreationWizard: get valid child!
			$new_tmpl_id = $this->_SUA_VersTrans($sqlo, $obj_tmpl_id);
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 2, 'Parameter-Preparation failed.' );
				return;
			}
			if ($new_tmpl_id!=$obj_tmpl_id) {
				$this->creaInfo[]='die aktive Version der VersionMaster-SUA [ID:'.$obj_tmpl_id.'] wurde ermittelt.';
			}
			
			//get extra_class_id from abstract sua plan
			$varcol_lib = new fVarcolConabs();
			$class_id = $varcol_lib->get_class_for_conc($sqlo, 'ABSTRACT_SUBST', $obj_tmpl_id);
			
			
			//$argu["NAME"]              = $obj_name;
			if (empty($argu['vals'])) $argu['vals']=array();
			$argu['vals']['ABSTRACT_SUBST_ID'] = $new_tmpl_id;
			
			if ($class_id) {
			    $this->creaInfo[] = "add planned Extra Class.";
			    $argu["xobj"] = array ('extra_class_id'=>$class_id); // TBD: overwrite planned $argu["xobj"]  values ???
			}
			
			
			break;
			
		default:
			// get column_name of abstract table
			
			$tmpl_tabname = $this->objConAbsLib->getTemplate($tablename);
			if ($tmpl_tabname==NULL) break;
			
			$abstract_col_name = $tmpl_tabname.'_ID';
			// $argu["NAME"]             = $obj_name;
			if (empty($argu['vals'])) $argu['vals']=array();
			$argu['vals'][$abstract_col_name] = $obj_tmpl_id;

	} 
	
	$obj_new_id = $this->objCreaLib->create_objX ( $sqlo, $sqlo2, $tablename, $argu, 0, $proj_id, $crea_opt);
	
	$_SESSION['s_sessVars']["o.".$tablename.".create.obj_tmpl_id"] = $obj_tmpl_id;
	
	return ($obj_new_id);
}



}

