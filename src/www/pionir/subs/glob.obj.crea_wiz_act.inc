<?php
/**
 * perform creation of object for glob.obj.crea_wiz.php
 * - optional include objlib/o.'.$tablename.'.crea_wiz.inc';
 * @package glob.obj.crea_wiz_act.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq SREQ:0000972  Object creation wizard (MOTHER-REQ) 
 * @version $Header: trunk/src/www/pionir/subs/glob.obj.crea_wizGui.inc 59 2018-11-21 09:04:09Z $
 */

require_once('glob.obj.copyobj2.inc');
require_once('glob.obj.conabs.inc');
require_once('javascript.inc');
require_once ('glob.obj.create.inc');
require_once ('gui/glob.creaWizTrigger.inc');
require_once 'f.prefvals.inc';
require_once ('object.info.inc');
require_once ('f.update.inc');



/**
 * helper methods
 * @author steffen
 *
 */
class objCreaWiz_execHelp {
	/**
	 * used for glob.obj.crea_wiz.php
	    object creation wizard for:
	  	- BOs
		- Future: ASSOC elements
		- single PK objects (only root allowed)
	 */
	public static function cp_file( &$sql, $tablename, $id, $userfile, $userfile_name, $userfile_type) {
	    global $error;
	    
		$retval = 0;
		if ($tablename=='LINK') {
			require_once ('o.LINK.subs.inc');
			$linkUpObj = new oLinkUpload();
			$linkUpObj->link_cp_file( $sql, $userfile, $id, $userfile_name, $userfile_type );
			if ($error->Got(READONLY))  {
				$error->printAllEasy();
				$retval = -1;
			}
			
		}
		if ($tablename=='IMG') {
			require_once ('o.IMG.upload.inc');
			$retval = img_cp_file( $sql, $userfile, $id, $userfile_name, $userfile_type );
		    $error->printAll();
	    }
		
		if ($userfile = '') {
			return 0;
		}
		
		return $retval;
	}
	
	
}

/**
 * main action functions for glob.obj.crea_wiz.php
 */
class objCreaWiz_act {
	  
	private $msgarr;     // message array
	private $wizardOpts; // wizard options
	private $mostimp_ColName;
	private $tablename;
	private $pk_name;
	private $stop_array;
	

	function __construct($tablename, $gopt=array() ) {
	    $this->gopt = $gopt;
    	$this->wizardOpts= NULL;
    	$this->msgarr    = NULL;
    	$this->tablename = $tablename;
    	$this->objCreaLib    = new objCreaWiz($tablename);
    	$this->objConAbsLib  = new gConcAbsC();
    	$this->objBOCreaLib  = new gObjCreateC($tablename);
    	
    	$this->tmpl_tabname 	 = $this->objConAbsLib->getTemplate($tablename);
    	if ($this->tmpl_tabname) $this->tmpl_tabname_nice = tablename_nice2($this->tmpl_tabname);
    	$this->mostimp_ColName   = importantNameGet2($tablename);
    	$this->pk_name 	         = PrimNameGet2($tablename);
    	
    	$this->sessStore = &$_SESSION['s_formState']['glob.obj.crea_wiz'];
    	
    	if ($_SESSION['userGlob']["o.".$tablename.".fCreaOpt"]!=NULL) {
    		$this->wizardOpts = unserialize($_SESSION['userGlob']["o.".$tablename.".fCreaOpt"]);
    	}
    	
    	$this->stop_array=array();
    }
    
    function get_stop_array() {
        return $this->stop_array;
    }
    
    function getTmplInfo() {
    	return ($this->tmpl_tabname);
    }
    
    /**
     * print message
     * @param  $text
     * @param  $prio
     *   <0 : print only on debug-level
     */
    private function addMessage($text, $prio=0) {
    	//$this->msgarr[] = $text;
    	echo $text."<br />\n";
    }
    
    /**
     * get workflow info and more
     * @param object $sqlo
     * @param int $obj_blueprint_id
     * @return int $wflow_id
     */
    private function _get_data_created(&$sqlo, $obj_id) {
        
        if (!$obj_id) return 0;
        
    	if (cct_access_has2($this->tablename)) {
    		$pkName 	  = PrimNameGet2($this->tablename);
    		$access_data = access_data_get($sqlo, $this->tablename, $pkName, $obj_id);
    		return $access_data['wflow_id'];
    	}
    	return 0;
    }
    
    public function set_wizardOps($key, $val) {
    	$this->wizardOpts[$key] = $val;
    }
    
    /**
     * save $this->wizardOpts ==> $_SESSION['userGlob']["o.".$tablename.".fCreaOpt"]
     */
    public function _saveUserGlobCreaOpt() {
        debugOut('(138) selectFget: wizardOpts:'.print_r($this->wizardOpts,1), '_saveUserGlobCreaOpt', 2);
    	$_SESSION['userGlob']["o.".$this->tablename.".fCreaOpt"] = serialize($this->wizardOpts);
    }
    
    function setUploadVars($userfile, $userfile_name, $userfile_type) {
    	$this->upload = NULL;
    	$this->upload["userfile"] = $userfile;
    	$this->upload["userfile_name"] = $userfile_name;
    	$this->upload["userfile_type"] = $userfile_type;
    }
    
    // $keyarr = array of keys (0=>key1, 1=>key2) key2 is optional
    function setSessStore($keyarr, $val) {
    	if (sizeof($keyarr)==1) $this->sessStore[$keyarr[0]]=$val;
    	if (sizeof($keyarr)==2) {
    		if (!isset($this->sessStore[$keyarr[0]])) $this->sessStore[$keyarr[0]]=NULL;
    		$this->sessStore[$keyarr[0]][$keyarr[1]] = $val;
    	}
    }
    
    /**
     * get value from $this->sessStore
     * @param string $key
     */
    public function getSessVal($key) {
    	return $this->sessStore[$key];
    }
    
    function setConnVars($table_name_conn, $conn_params, $id_conn) {
    	/* $table_name_conn    - table_name of parent object
    	  $conn_params	      - extra params for connect table
    	  $id_conn	          - ID of parent object
    	 */
    	$this->connX = NULL;
    	$this->connX["table"]  = $table_name_conn;
    	$this->connX["params"] = $conn_params;
    	$this->connX["id"]     = $id_conn;
    }
    
    function doConn(&$sql, $obj_new_id) {
    
    	$primas   = primary_keys_get2($this->connX["table"]);
    	$updatetable = $this->connX["table"];
    	$new_obj_tab = $this->tablename;
    	$pk_name  = $primas[0];
    	$pk2_name = $primas[1];
    	$argu 	  = array();
    	
    	
    	if ($pk2_name=="") { /* is it just a FK in an object ??? */
    	
    		$argu[$pk_name]   	     = $this->connX["id"];
    		$argu[$new_obj_tab."_ID"]  = $obj_new_id;
    		
    		$retval_tmp= gObjUpdate::update_row($sql, $this->connX["table"], $argu); 
    		if ($retval_tmp<=0) {
    			echo "ERROR: Update of \"".$updatetable."\" failed. <br>";
    			$retval=-1;
    		}
    	} else { /* it is a new object in an ASSOC table */
    	
    		$argu = $this->connX["params"];	/* extra parameters for ASSOC table */
    		$argu[$pk_name]   = $this->connX["id"];
    		$argu[$pk2_name]  = $obj_new_id;
    		$retval_tmp= insert_row($sql, $this->connX["table"], $argu); /* Image */
    		if ($retval_tmp<=0) {
    			echo "ERROR: Attachment of \"".$this->tablename."\" to \"".$this->connX["table"]."\" failed. <br>";
    			$retval=-1;
    		}
    	}
    	return ($retval);
    }
    
    
    /**
     * get ABSTRACT_OBJECT_ID from object parameters; if an ABSTRACT_OBJECT exists
     * @param  $sqlo
     * @param  $tablename
     * @param  array $params : array( 'vals'=> array(COL=>VAL) )
     */
    private function getAbsObjID(&$sqlo, &$params) {
    	
    	$abs_tablename = $this->tmpl_tabname;
    	if ($abs_tablename==NULL) return 0;
    	$abs_id_name= $abs_tablename.'_ID';
    	$absObjID = $params['vals'][$abs_id_name];
    	
    	// glob_printr( $params, "getAbsObjID::params info; absObjID:$absObjID" );
    	
    	return $absObjID;
    }
    
    /**
     * after object was created: do POST actions
     * @param object $sql
     * @param int $obj_new_id
     * @param array $newpx
     * 
     * @param string $selecter
     */
    private function _post_crea_actions($sql, $obj_new_id, $newpx, $selecter) {
        global   $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $tablename = $this->tablename;
        $abstract_obj = 0; // contains ID of an ABSTRACT_OBJECT; if object is derived from ABSTRACT object
        // echo "INFO: Object created.<br>";
        $hist_obj       = new historyc();
        $hist_obj->historycheck( $tablename, $obj_new_id ); /* add new object to history */

        
        // <analyse new parameters>
        $objInfoLib    = new objInfoC();
        $paramsCreated = $objInfoLib->getParamsRaw ( $sql, $tablename, $obj_new_id);
        $abstract_obj  = $this->getAbsObjID($sql, $paramsCreated);
        // </analyse new parameters>
        
        /* object specials */
        
        if ( ($tablename=="IMG") || ($tablename=="LINK") ) {
            if ( $this->upload["userfile_name"]!="" ) {
                $retval_tmp = objCreaWiz_execHelp::cp_file( $sql, $tablename, $obj_new_id, $this->upload["userfile"],
                    $this->upload["userfile_name"], $this->upload["userfile_type"] );
                if ($retval_tmp<0) {
                    echo "<B>ERROR</B>: Upload failed, but continued.<br>";
                    $this->stop_array['anyway'] = 1; // stop referring
                    $this->stop_array['reason'] = "failure of upload, ";
                }
            }
        }
        
        /**
         *	optional: add trigger
         */
        $useTriggerID1 = 0;
        if ($abstract_obj) {
            $useTriggerID2 = glob_creaWizTrigger::getWfl_ofAbstract( $sql, $this->tmpl_tabname, $abstract_obj);
        }
        
        do {
            
            $useTriggerID3=0;
            if (is_array($newpx)) {
                if ($newpx['worflow_id']) {
                    debugOut('(289) Workflows: from newpx', $FUNCNAME, 2);
                    $useTriggerID3 = $newpx['worflow_id'];
                    // take this one ...
                    break;
                }
            }
            
            //if ($selecter=='obj') {
            $useTriggerID3 = $this->_get_data_created($sql, $obj_new_id);
            debugOut('(298) Workflows: from created-object: '.$useTriggerID3, $FUNCNAME, 2);
            //}
            $useTriggerID1 = glob_creaWizTrigger::getTriggerOfTable($sql, $tablename, $this->wizardOpts['trigger'] );
            
        } while (0);
        
        debugOut('(303) Possible Workflows: a:'.$useTriggerID1.' b:'.$useTriggerID2.' c:'.$useTriggerID3, $FUNCNAME, 2);
        
        if ($useTriggerID1 or $useTriggerID2 or $useTriggerID3) {
            require_once ("lev1/glob.obj.create_subs.inc");
            $workFlowLib = new gObjCreaSubs();
            $workFlowLib->addUserWorkflow($sql, $tablename, $obj_new_id, $useTriggerID3, $abstract_obj);
            if ($error->printAll()) {
                $this->stop_array['anyway'] = 1;
            }
        }
        
        /**
         *	check, if this object object should be connected to a parent object.
         */
        if ( $this->connX!=NULL ) {
            $this->doConn($sql, $obj_new_id);
        }
        
        
        $this->addMessage('<span style="color:green;">OK</span>: object created. ID:'.$obj_new_id);
        
        
        echo "<hr>\n";
        
        $tmpurl  = 'edit.tmpl.php?t='.$tablename.'&id='.$obj_new_id;
        $stopOpt = NULL;
        if ( $this->stop_array['anyway'] )  {
            $stopOpt = array(1, $this->stop_array['reason']);
        }
        js__location_replace( $tmpurl,  '&gt;&gt; Go to edit form', $stopOpt);
        
    }
    
    /**
     * create object now!
     * 
     * @param $sql
     * @param $sql2
     * @param $sql3
     * @param $create_now
     * @param array $deep_copy
     * @param array $newparams : additional predefined parameters; e.g. DB_USER:FULL_NAME
     *    if exists, can overrule $obj_name_in !
     * @param array $newpx : extra parameters
     *     'worflow_id' : ID of workflow: overrules all other workflow settings 
     * @param $has_name_flag
     * @param $obj_name_in  (can be overruled by $newparams['NAME'])
     * @param $obj_tmpl_id
     * @param $obj_blueprint_id
     * @param $objAssCrea
     * @param $objAssFrgn
     * @param $optspecial
     * @param $proj_id
     * @param string $selecter type of creation:
     *    "obj_tmpl" - from template
     *    "obj"      - from object
     *    ["empty"]  - empty form
     * @param $KEY_EXISTOBJ_ID
     * @return $obj_new_id
     * 
     * @global $this->upload
    		   $this->connX
     */
    function go1 ( 
    	&$sql, &$sql2, &$sql3,
    	$create_now,
    	$deep_copy,
    	$newparams,
        $newpx,
    	$has_name_flag,
    	$obj_name_in,
    	$obj_tmpl_id,
    	$obj_blueprint_id,
    	$objAssCrea,
    	$objAssFrgn,
    	$optspecial,
    	$proj_id,
    	$selecter,
    	$KEY_EXISTOBJ_ID
    	) {
    
    	
    	global   $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$tablename = $this->tablename;
    	$table_nicename = tablename_nice2($tablename);
    	
    	$retval 	  = 0;
    	$this->stop_array=array();
    	//$pkName 	  = PrimNameGet2($tablename);
    	
    	
    	// $newparams OVERRULES $obj_name_in !
    	
    	if (!is_array($newparams)) $newparams=array();
    	debugOut('(297) newparams (IN1): '.print_r($newparams,1), $FUNCNAME, 2);
    	if ($newparams[$this->mostimp_ColName]!=NULL) $obj_name_in = $newparams[$this->mostimp_ColName];
    	if ($obj_name_in!=NULL) $newparams[$this->mostimp_ColName] = $obj_name_in; // and set $newparams, if objanem is set ...
    	
    	if ($_SESSION['userGlob']["g.debugLevel"]>1) {
    		echo "DEBUG-level activated ...<br>";
    		
    		
    		debugOut('(288) create_now: '.$create_now, $FUNCNAME, 2);
    		
    		if ($_SESSION['userGlob']["g.debugLevel"]>3) {
    		    debugOut('STOP! No create.', $FUNCNAME, 3);
    		    return 0;
    		}
    	}
    	
    	//$this->obj_name   = $obj_name_in;
    	$this->create_now = $create_now;
    	
    	// 1. try in LAB-directory
    	// 2. try in standard directory
    	
    	$helper_file = $_SESSION['s_sessVars']['AppLabLibDir'].'/objlib/o.'.$tablename.'.crea_wiz.inc';
    	if (!file_exists($helper_file)) {
    	   $helper_file = dirname(__FILE__).'/../objlib/o.'.$tablename.'.crea_wiz.inc';
    	}

    	if (file_exists($helper_file)) {
    	    
    	    require_once $helper_file;
    	    $clsName = 'o'.$tablename.'_objCreaWiz'; // base class: objCreaWiz_OBJ
    	    
    	    
    	    $helper_cls   = new $clsName($selecter, $obj_tmpl_id, $obj_blueprint_id);
    	    $helper_cls->set_gopt($this->gopt);
    	    debugOut('(319) newparams (IN): '.print_r($newparams,1), $FUNCNAME, 2);
    	    
    	    $check_result = $helper_cls->preCheck($sql, $obj_name_in, $newparams, $this->upload);
    	    debugOut('(319) check_result: '.print_r($check_result,1), $FUNCNAME, 2);
    	    if ($error->Got(READONLY))  {
    	        return;
    	    }
    	    
    	    if (is_array($check_result['defvals'])) {
    	        $this->wizardOpts['defvals'] = $check_result['defvals'];
    	        debugOut('(325) wizardOpts: '.print_r($this->wizardOpts,1), $FUNCNAME, 2);
    	        $this->_saveUserGlobCreaOpt(); // save for next time
    	    }
    	    if (key_exists('create_now', $check_result)) {
    	        $this->create_now = $check_result['create_now'];
    	        debugOut('(322) create_now: '.$this->create_now, $FUNCNAME, 2);
            }
            $newparams[$this->mostimp_ColName] = $check_result['obj_name'];
    	}
    	
    	// get back modified params
    	$create_now = $this->create_now;
    	// $obj_name   = $this->obj_name;
    	
    	// check input NAME column
    	$name_col_feats = colFeaturesGet   ( $sql,  $tablename, $this->mostimp_ColName);
    	
    	
    	if ($name_col_feats['NOT_NULL']>0) {
    	    if ($newparams[$this->mostimp_ColName]==NULL) {
    	        $error->set( $FUNCNAME, 5, 'User-Input Error: "'.$name_col_feats['NICE_NAME'].'" missing.' );
    	        return;
    	    }
    	}
    	
    	debugOut('(350) newparams (used): '.print_r($newparams,1), $FUNCNAME, 2);
    	

    	
    	$tmpl_tabname = $this->tmpl_tabname;
    	
    		
    	switch ($selecter) {    
    	
    		case "obj_tmpl":
    			if ($tmpl_tabname == '') {
    				echo "<font color=red>ERROR:</font> no template object type for $table_nicename known! No way to do it!<BR>";
    				return 0;
    			}
    		
    			if ( !$obj_tmpl_id ) {
    				echo "ERROR: please select an $table_nicename (template)<br />\n";
    				$retval = -1;
    			} else {
    			    $objFeatNew=array('vals'=>$newparams);
    			    $obj_new_id = $this->objCreaLib->objCreateFrTmpl( $sql, $sql2, $obj_tmpl_id, $proj_id, $objFeatNew );
    			}
    			
    			$infoTxt = 	$this->objCreaLib->	getInfo();
    			$this->addMessage($infoTxt);
    			
    			break; 
    			
    		case "obj": 
    			// create copy from object $obj_blueprint_id
    			
    		    if ($this->mostimp_ColName == $this->pk_name) {
    		        $error->set( $FUNCNAME, 1, 'This object type can not be copied, because the the NAME of the object must be unique.' );
    		        return;
    		    }
 
    			$toption =NULL;
    			$toption["proj_id"] = $proj_id;
    			if (!empty($objAssCrea)) $toption["objAssCrea"] = $objAssCrea;
    			if (!empty($objAssFrgn)) $toption["objAssFrgn"] = $objAssFrgn;
    			if (is_array($optspecial) )    $toption["optspecial"] = $optspecial;
    			
    			//if ($obj_name!="") $newparams[$this->mostimp_ColName] = $obj_name;
    			
    			$objFeatNew=array('vals'=>$newparams);
    			$obj_new_id = $this->objCreaLib->objCreate( $sql, $sql2, $sql3, 
    					$obj_blueprint_id,  $objFeatNew, $deep_copy, $toption);
    			$infoTxt    = 	$this->objCreaLib->	getInfo();
    			$this->addMessage($infoTxt);
    			
    			
    			$_SESSION['s_sessVars'][$KEY_EXISTOBJ_ID] = $obj_blueprint_id;
    			if ($error->Got(READONLY))  {
    			    $error->set( $FUNCNAME, 2, 'Error on copy of object.' );
    				return;
    			}
    			
    			if ( $optspecial["showpasswd"] ) {
    			    $this->stop_array['anyway'] = 1;
    			    $this->stop_array['reason'] = " Show password for email sending.";
    			}
    			
    			
    			
    			break; 
    			
    		default: // empty : show empty form  
    			$this->addMessage("<B>Create an empty $table_nicename</B><br>");
    			
    			$urlOptions="";
    			//OLD: $has_name_flag
    			if ( !empty($newparams) ) {
    			    foreach($newparams as $key =>$val) {
    			        $urlOptions .='&argu_xtra['.$key.']='.rawurlencode($val);
    			    }

    			}
    			if ($proj_id) $urlOptions .=  '&insertToProjId='.$proj_id;
    			if ($newpx['worflow_id'] ?? NULL) $urlOptions .=  '&newpx[worflow_id]='.$newpx['worflow_id'];
    			
    			debugOut('(405) create_now: '.$create_now, $FUNCNAME, 1);
    			
    			if ( $create_now ) {
    			    
    			    $argu=array();
    			    if (!empty($newparams)) $argu=$newparams;
    				$obj_param = array ("vals"=>$argu);
    				
    				debugOut('(418) create argu2: '.print_r($argu,1), $FUNCNAME, 1);
    				debugOut('(420) Create object now.', $FUNCNAME, 2);
    				
    				$obj_new_id = $this->objBOCreaLib->create_objX ( $sql, $sql2, $tablename, $obj_param, 0, $proj_id);
    				if ($obj_new_id<=0) {
    					$error->set( $FUNCNAME, 15, 'object not created!' );
    					return -1;
    				}
    			} else {
    			    if ($this->stop_array['anyway']>0) $stopOpt = array($this->stop_array['reason'], NULL);
    				else $stopOpt = NULL;
    				js__location_replace( 'edit.insert.php?tablename='.$tablename.$urlOptions, 
    						'&gt;&gt; Go to edit form', $stopOpt);
    				return;
    			}	
    			break;
    			
    	}
    	
    	if ( !$obj_new_id ) {
    		$error->set( $FUNCNAME, 20, 'object not created!' );
    		return -1; 
    	} 
    	
    	$this->_post_crea_actions($sql, $obj_new_id, $newpx, $selecter);
    		
    	
    	
    }

}