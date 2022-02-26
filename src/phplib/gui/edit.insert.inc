<?php

require_once('insert.inc');
require_once 'insertx.inc';
require_once('db_x_obj.inc');
require_once('edit.sub.inc');
require_once('edit.edit.inc');
require_once('class.history.inc');
require_once('javascript.inc');
require_once('validate.inc');
require_once("o.PROJ.addelems.inc");	
require_once('o.PROJ.paths.inc');	
require_once 'glob.obj.conabs.inc';
require_once ('glob.obj.create.inc');
require_once ('gui/glob.creaWizTrigger.inc');
require_once ("lev1/glob.obj.create_subs.inc");

/**
 * - sub functions for inserting a new object
 * - mainly used by edit.insert.php, but can be used by other packages
 * @package edit.insert.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param
 */
class fEditInsLibC {
	
var $newID;
var $object_is_bo;
var $pk_arr;
var $pkName;
	
function __construct($tablename, $destProj ) {
	$this->tablename = $tablename;
	$this->destProj  = $destProj;
	$this->infoArr   = NULL;
	$this->newID 	 = NULL;
	$this->pkName	 = PrimNameGet2($tablename);
	$this->extra_obj_col_exists = 0;
}

function _infoSet( $prio, $text ) {
	$this->infoArr[] = array($prio, $text);
}

/**
 * repair boolean data types, set default to "0"
 * @return array $argu
 * @param object $sql
 * @param array $argu
 */
function _booleanRepair( &$sql, &$argu ) {
	
	$varnames = $this->colNames_ori;
	$sql->query("select APP_DATA_TYPE_ID from APP_DATA_TYPE where name='boolean'");
    $APP_DATA_TYPE_boolean_ID = -1;
    if ( $sql->ReadRow() ) $APP_DATA_TYPE_boolean_ID = $sql->RowData[0]; /* just loaded from DB */
    foreach( $varnames as $colname_tmp) {
       
        $colInfos = colFeaturesGet($sql, $this->tablename, $colname_tmp) ;
        if ($colInfos["APP_DATA_TYPE_ID"]==$APP_DATA_TYPE_boolean_ID) {
            if (empty($argu[$colname_tmp])) {
               $argu[$colname_tmp] = "0";
            }
        }
    }
    reset ($varnames);
	
}

/**
 * set main arguments
 * @return 
 * @param object $arguobj
 * @param object $xargu
 * @param object $argu
 * @param array  $colNames[optional] column names
 */
function setArgus($arguobj, $xargu, $argu, $colNames=NULL, $newpx=NULL ) {
	$tablename = $this->tablename;
	
	if (!isset($arguobj)) $arguobj = array();
	if (!isset($xargu))   $xargu = array('CLASS' => NULL);
	
	//$arguobj_old    = NULL;
	
	if (empty($argu))  $argu = NULL;
	if (empty($newpx)) $newpx=array();

	$this->argu    = $argu;
	$this->arguobj = $arguobj;
	$this->xargu   = $xargu;
	$this->newpx   = $newpx;
	
	if ( $colNames==NULL ) $this->colNames_ori = columns_get_pos($tablename); 
	else $this->colNames_ori = $colNames;
	

	$extra_obj_col_exists = (in_array('EXTRA_OBJ_ID', $this->colNames_ori) and ($tablename != 'EXTRA_OBJ'));
	reset ($this->colNames_ori);
	$this->extra_obj_col_exists = $extra_obj_col_exists;
	
}


/**
 * do initial checks
 * @global $this->object_is_bo
 * @global $this->pk_arr
 * @global $this->destProj
 * @return 
 */
function initChecks(&$sql) {
	global $error;
	$FUNCNAME= 'initChecks';
	
	$tablename = $this->tablename;
	$argu 	   = $this->argu;
	$this->is_assoc_table = 0;
	
	if ($tablename=="") {
		$error->set( $FUNCNAME, 1, 'No table given'); 
		return;
	}
	
	$object_is_bo= 0;
	if (cct_access_has2($tablename)) $object_is_bo = 1;
	
	if ( $object_is_bo ) {
		$AddProjLib = new oProjAddElem( $sql );
	    list($this->destProj, $projname) = $AddProjLib->projectTest($sql, $this->destProj);
	    if ( ($this->destProj <= 0) OR $error->Got(READONLY) ) {
			$error->set($FUNCNAME, 5, "Can not add object to project.");
			return;
	    }
		//$destProj  = $this->destProj;
	}
	
	//$nicename = tablename_nice2($tablename);
	
	if (!glob_table_exists($tablename)) {
		$error->set( $FUNCNAME, 2, "&nbsp;<br><center>Table <B>$tablename</B> is not described by the system.<br>\n" .
			"Please ask the administrator!");
		return;
	}
	
	
	
	$this->editFormLib = new fFormEditC();
	$this->editFormLib->setObject($tablename, NULL);
	

	$mothertable = mothertable_get2($tablename);
	
	$this->object_is_bo = $object_is_bo;
	$this->pk_arr   = primary_keys_get2($tablename); /* get primary keys */
	if (sizeof($this->pk_arr) > 1) {
	    $this->is_assoc_table = 1;
	}
	
	if ($mothertable!="") $nicename_mother = tablename_nice2($mothertable);
	
	$t_rights = tableAccessCheck($sql, $tablename);
	$x_rights=array();
	$x_rights["write"] = 1;
	
	if ( table_is_view($tablename)) {
	    $error->set( $FUNCNAME, 3, "This table is a read-only collection ('view'). No manipulation is possible.'");
		return;
	}
	
	if ( $t_rights["insert"] != 1 ) {
		$answer = getTableAccessMsg( $tablename, "insert" );
	    $error->set( $FUNCNAME, 4, $answer);
	    return;
	}
	
	if ( !$this->is_assoc_table ) {
	    
	    // if is a single PK object => unset PRIMARY KEY column 
	    foreach( $this->colNames_ori as $index => $col) {
	        if ($col == $this->pkName) {
	            
	            $app_type_id = appDataTypeGet2($tablename, $col);
	            $app_type    = appDataTypeNameGet2($app_type_id);
	            
	            $do_unset_PK_column = 1;
	            if ($app_type=='name') $do_unset_PK_column = 0; // a primary column of type NAME is allowed to edit ...
	            
	            if ( $do_unset_PK_column )  unset($this->colNames_ori[$index]);
	        }
	    }
	    
	}

	
	$idname	= PrimNameGet2($tablename);
	// it is a ASSOC table, but not a BO (like A_SUBST_COHER)
	if ( $mothertable && !$object_is_bo ) {
	    $tmp_id = $argu[$idname]; // insert preparation 
	    if ($tmp_id) {
	        $o_rights = access_check($sql, $tablename, $tmp_id);
	        $x_rights = $o_rights;
	    }
		
	    if ( glob_isAdmin() ) $x_rights["insert"] = 1; // if no PRIMARY_key exists, set right anyway
	    if ( !$x_rights["insert"] ) {
	    	$error->set( $FUNCNAME, 6, 'You have no right to insert elements associated with "<B>'.$nicename_mother.'</B>" (ID:'.$tmp_id.').');
	        return;
	    } 
	}
	
	if ( $object_is_bo ) {
		$projPathLib = new oPROJpathC();
		$projText = $projPathLib->getPathText($sql, $this->destProj);
		echo " <font color=gray>destination project:</font> <img src=\"images/icon.PROJ.gif\"> ". $projText;
	}
	echo "<br>\n";
}

function hasArgus() {
	if ( sizeof($this->argu) ) return 1;
	else return 0;
}

/**
 * - do the SQL insert
 * - manage EXTRA_OBJ
 * - add BO to project
 * 
 * @return 
 * @param object $sql
 * @param object $sql2
 * @param array $insopt[optional] options of insert_row
 * @global $this->newID
 */
function doInsert( &$sql, &$sql2, $insopt=NULL ) {
	global $error, $varcol;
	$FUNCNAME= 'doInsert';
	
	$tablename= $this->tablename;
	$argu 	  = $this->argu;
	$varnames = $this->colNames_ori; /* copy data for new sql command */
	$pk_arr   = $this->pk_arr;
	$arguobj  = $this->arguobj;
	$xargu	  = $this->xargu;
    // $retVal   = 0;
	$this->newID = NULL;
	
	$this->editFormLib->formParamCheck($sql2, $argu, $arguobj, "insert", $varnames);
    if ($error->Got(READONLY)) {
        $error->set( $FUNCNAME, 1, "Input-Parameter check failed!" );
		return;
    }

    // set boolean values to "0" if not given from HTML form.checkbox element
	$this->_booleanRepair($sql, $argu );
	
	$extra_class_id=0;
	if ($this->extra_obj_col_exists) {
        /* translate indices NAME => ID */
//         $arguobj_id = array();
//         if (sizeof($arguobj)) {
//         	$extra_class_id = $xargu['CLASS'];
//             foreach( $arguobj as $attName=>$attValue) {
//               $newid = $varcol->attrib_name_to_id( $attName, $extra_class_id);
//               if ( $error->Got(READONLY) ) return;
//               $arguobj_id[$newid] = $attValue;
//             }
// 			reset($arguobj);
//         }
    }
        
	$validLib = new fValidate();
	$dtypes   = $validLib->check_columns($sql, $tablename, $argu, false);
	if ($error->Got(READONLY)) {
		return;
	}
	
 	if ($error->Got(READONLY))  {
    	$error->set( $FUNCNAME, 2, "Parameter check failed. Please check your values!" );
		return;
	}
	
	if ( empty($insopt) ) {
		if (sizeof($pk_arr)>1) $insopt=array("touchmom"=>1); // touch mother object ...
	}
	
	if ($this->object_is_bo) {
	    // use now the HIGH level object insert ...
	    $obj_param=array(
	        'vals'=>$argu
	    );
	    if ($extra_class_id) {
	        $obj_param["xobj"]=array(
	            'extra_class_id' => $extra_class_id,
	            "values"  => $arguobj
	        );
	    }
	    $objBOCreaLib  = new gObjCreateC($tablename);
	    $id = $objBOCreaLib->create_objX ( $sql, $sql2, $tablename, $obj_param );
	    $logarr = $objBOCreaLib->get_log();
	    if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
	        if (!empty($logarr)) {
	            debugOut('creation.log:'.print_r($logarr,1), $FUNCNAME, 1);
	        }
	    }
	    
	} else {
	   $id = insert_row($sql, $tablename, $argu, $insopt);  
	}
	
	if (!$id) {
		$error->set( $FUNCNAME, 3, "Problems at insert, please check your values!" );
		return;
	}
	$this->newID = $id;
	
	$idname		  = PrimNameGet2($tablename);
    $argu[$idname]= $id; // on successful insert save id

    
    
    $abstract_obj_id = 0;
    $concabs_lib  = new gConcAbsC();
    $abs_col_info = $concabs_lib->getAbsColName($tablename);
    if (is_array($abs_col_info)) {
        $abs_col_name = $abs_col_info[1];
        if ($abs_col_name!=NULL)  $abstract_obj_id = $argu[$abs_col_name];
    }
    
	/**
	 *	optional: add trigger
	 */
    
    $input_mx_id=0;
    if ($this->newpx['worflow_id']) {
        $input_mx_id = $this->newpx['worflow_id'];
    }
    
    gObjCreaSubs::addUserWorkflow( $sql, $tablename, $id, $input_mx_id, $abstract_obj_id );
	if ($error->Got(READONLY))  {
    	$error->set( $FUNCNAME, 5, "Problem on setting the trigger!" );
		return;
	}
	
	
	
    if ( $this->object_is_bo && ($this->destProj > 0) ) { /* add BO to project ? */
	    $AddProjLib = new oProjAddElem($sql, $this->destProj);
		$AddProjLib->addObj( $sql, $tablename, $id ); 
        if ($error->Got(READONLY))  {
        	info_out("WARNING", "could not add BO to project!");
			$error->reset();
		}
	    $this->_infoSet(2, 'added BO to project '.$this->destProj);
    }
    
  
}

/**
 * forward page
 * @param object $sql
 * @param int $remotepos position of form-object in calling form
 * @param string $backurl : overrules $remotepos
 */
function pageForward( &$sql, $remotepos, $backurl=NULL ) {
	
	$tablename = $this->tablename;
	$pk_arr    = $this->pk_arr;
	$id		   = $this->newID;
	$argu	   = $this->argu;
	
	if ($backurl!=NULL) {
		js__location_replace($backurl, 'next action' ); 
    	return;
	}
	
	/* now call edit.tmpl again with inserted values */
    $tmp_name = obj_nice_name($sql, $tablename, $id);
    $primas=array();
    $primasid=array();
    
	if (!empty($pk_arr[1])) {
    	$primas[1]   = $pk_arr[1];
        $primasid[1] = $argu[$primas[1]];
    }
    if (!empty($pk_arr[2])) {
        $primas[2]   = $pk_arr[2];
        $primasid[2] = $argu[$primas[2]];
    }
    
    if (!is_numeric($remotepos)) $usePos = -1; // just go to the object
    $newUrl = 'edit.tmpl.php?tablename='. $tablename. '&'. fObjFormSub::urlGetPrimkeys($id, $primas, $primasid);
    
    if ($usePos<0) {
    	js__location_replace($newUrl, 'the new object' ); 
    	return;
    }
    
    ?>
    <script language="JavaScript">
    <!--
    // handle the fact, if the parent window is closed
    if ( window.opener==null ) {
	    location.replace ('<?php echo $newUrl?>');
	} else { // do something else   
        remotepos = '<? echo $remotepos?>';
        id        = '<? echo $id?>';
        tmp_name  = '<? echo $tmp_name?>';
        submitCheckModal(remotepos, id, tmp_name);                
    }
    //-->
    </script>        
    
    <?
	
	return; // just arrives here at an error

}

/**
 * show HTML form
 * 
 * @param array $options :
 * 	 'remotepos'   = string
 *   'extraHidden' = array(key) of values
 *   'actUrl'	   = URL of form action
 *   'presel'      = defined preselections: (array of COLNAMES) = preselected ID 
 * @return 
 */
function showForm( &$sql, &$sql2, $options=NULL ) {

	$argu 	   = &$this->argu;
	$tablename = $this->tablename;
	$destProj  = $this->destProj ;
	$xargu	   = $this->xargu;
	$actUrl    = $_SERVER['PHP_SELF'];
	
	if ($options['actUrl']!=NULL) $actUrl = $options['actUrl'];

	
	echo '<table width=100% border=0>'."\n";
	echo '<tr><td valign=top>'. "\n";
	
	$extraobj_o = array();
	if ( $this->extra_obj_col_exists  ) {
		$extra_class_id = $xargu['CLASS'];
		$extraobj_o     = array( "extra_obj_id"=>NULL, "extra_class_id"=>$extra_class_id,
		    "arguobj"=>$this->arguobj );
	}
	
	if ( is_array($argu) ) $id = $argu[$this->pkName]; 
	else $id=NULL;
	$H_EXP_RAW_DESC_ID = NULL;
	if ( ($tablename=="EXP_RAW_RESULT")  || ($tablename=="EXP_CHAN_RES" ) ) { /* need for column names */
	    if ($id) {
	        $sql->query("select H_EXP_RAW_DESC_ID from EXP where EXP_ID=$id");
	        $sql->ReadRow();
	        $H_EXP_RAW_DESC_ID=$sql->RowData[0];
	    }
	}   
	
	echo '<form name="editform" method="post" action="'.$actUrl.'">'."\n";
	
		
	$editFormLib = new fFormEditC();
	$editFormLib->setObject($tablename, NULL);
	$edformOpt= array( "H_EXP_RAW"=>$H_EXP_RAW_DESC_ID );
	$colopt   = $editFormLib->x_cols_extra($sql, $argu);
	
	if ($options['presel']!=NULL) {
	    //
	    // set pre-selection, add them to existing $colopt
		//
	    if (!is_array($colopt['cols'])) $colopt['cols']=array();
	    
		foreach( $options['presel'] as $key=>$objid) {
		    if (!is_array($colopt['cols'][$key])) $colopt['cols'][$key]=array();
		    $colopt['cols'][$key]['selACondId'] = $objid;
		}
	}
	
	$editAllow = 1;
	$action    = "insert";
	$editFormLib->form_editx( $action, $sql,  $this->colNames_ori, $this->pk_arr, $argu,
	               $extraobj_o, $editAllow, $edformOpt, $colopt  ); 
				   
	if ( $options['remotepos']!=NULL )
	     echo '<input type="hidden" name="remotepos" value="'.$options['remotepos'].'">';
	
	echo "<input type=\"hidden\" name=\"tablename\" value=\"" .$tablename. "\" >\n";
	echo "<input type=\"hidden\" name=\"insertToProjId\" value=\"" .$destProj. "\" >\n"; 
	
	if ( is_array($options['extraHidden']) ) {
		$editFormLib->addHidden( $options['extraHidden'] );
	}
	echo "</form>\n";
	echo "</TD></TR></TABLE>";
	
}

function getObjID() {
	return ($this->newID);
}

}
