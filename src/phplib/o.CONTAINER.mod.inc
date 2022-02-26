<?php
require_once ('insert.inc');
require_once ('date_funcs.inc');
require_once ("f.assocUpdate.inc");
require_once ("o.S_VARIO.mod.inc");
require_once ("o.S_VARIO.subs.inc");
require_once 'lev1/glob.obj.status.mod.inc';

/**
 * container methods: modify entry list
 * - since 2020-11 : analyse after SUC removal : is this SUC is empty in the system ?
 * @package o.CONTAINER.mod.inc
 * @swreq UREQ:0001114: 0001107: o.CONTAINER > modify entries 
 * @unittest exists
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oContainerModC  {
	
    var $CLASSNAME="oContainerModC";
    var $_modIsActive=0; // modification is active?
    /**
     * 
     * @var array $parx
     *  'POS'
     *  'substid'
     */
    var $parx;
    private $cont_log_active;
    //private $_reduce_modlog_flag;
    
    /**
     * system wide stock fill status
     * @var integer
     *   1 there is stock of the MAC
     *   0 unknown
     *   -1 : is empty
     */
    private $SUC_stock_status= 0; 
    private $SUC_status_finished = 0; 
    
    /**
     * auto FINISH on empty SUC?
     * - should be forbidden on MOVE
     * @var integer
     */
    private $SUC_auto_finish = 1;
    
    function __construct ($parx=NULL ) {
    	$this->parx     = $parx;
    	$this->assoclib = new  fAssocUpdate();
    	$this->assoclib->set_log_level(fAssocUpdate::MASS_LOG_FIRST);
    	$this->_modIsActive    = 0;
    	$this->cont_log_active = 1; // default value
    	
    	$this->_init_subst(); 
    }
    
    /**
     * 
     * @param int $flag
     *    -1 : do not auto finish
     *     1: do auto finish
     */
    function auto_finish_SUC($flag) {
        $this->SUC_auto_finish = $flag;
    }
    
    private function _init_subst() {
        $this->SUC_stock_status    = 0;
        $this->SUC_status_finished = 0;
    }
    
    function initSubst($parx) {
    	$this->parx   = &$parx;
    	$this->newpos = NULL;
    	$this->_init_subst();
    }
    
    function setContainer(&$sqlo, $contID) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
    	$this->contID = $contID;
    	$this->maxPos = 0;
    	
    	$sqlsel = 'ABS_CONTAINER_ID	from CONTAINER where CONTAINER_ID='.$contID;
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadRow();
    	$this->ABS_CONTAINER_ID = $sqlo->RowData[0];
    	
    	if ( $this->ABS_CONTAINER_ID ) {
    		$sqlsel = 'max(POS) from ABS_CONT_ENTRY where ABS_CONTAINER_ID='.$this->ABS_CONTAINER_ID;
    		$sqlo->Quesel($sqlsel);
    		$sqlo->ReadRow();
    		$this->maxPos = $sqlo->RowData[0];
    	}
    	
    	$this->cont_log_active=1; // default value
    	
    	$varioLib = new oS_VARIO_sub('CONTAINER');
    	$log_active_tmp = $varioLib->getValByKey($sqlo, $contID, 'log.active');
    	
    	$log_active_tmp = intval($log_active_tmp);
    	if (!$log_active_tmp) {
    	    // do nothing; default value
    	}
    	if ($log_active_tmp<0) {
    	    $this->cont_log_active=0;
    	}
    	
    	$this->varioModLib = new oS_VARIO_mod();
    	$varioopt=array('noAccCheck'=>1);
    	$this->varioModLib->setObject( $sqlo, 'CONTAINER', $contID, $varioopt );
    	
    	debugOut('(111) CONT-ID:'.$contID.' ABS_CONTAINER_ID:'.$this->ABS_CONTAINER_ID.' cont_log_active:'.
    	    $this->cont_log_active, $FUNCNAME, 1);
    }
    
    function modiCheck(&$sqlo) {
    	// check modification rights
    	global $error;
    	$FUNCNAME= "modiCheck";	
    	
    	if (!$this->contID) {
    	    sys_error_my( 'Class not initialized: missing ContID.' );
    	}
    	
    	$cont_tablename="CONTAINER";
    	$contid   = $this->contID;
    	$t_rights = tableAccessCheck( $sqlo, $cont_tablename );
    	if ( $t_rights["write"] != 1 ) {
    		$answer = getTableAccessMsg( $cont_tablename, "write" );
    		$error->set( $FUNCNAME, 2, $answer);
    		return;
    	}
    	
    	$o_rights = access_check($sqlo, $cont_tablename, $contid );
    	if ( !$o_rights["insert"]) {
    		$error->set( $FUNCNAME, 3, "You do not have insert permission on ".
    				tablename_nice2($cont_tablename)." ".$contid."!");
    		return;
    	}
    	
    	$this->activateModi($sqlo);
    	
    }
    
    // activate without check
    function activateModi(&$sqlo) {
    	$contid   = $this->contID;
    	$this->assoclib->setObj( $sqlo, 'CONT_HAS_CSUBST', $contid );
    	$this->assoclib->set_log_level(fAssocUpdate::MASS_LOG_FIRST);
    	
    	$varioopt=array('noAccCheck'=>1);
    	$this->varioModLib->setObject( $sqlo, 'CONTAINER', $contid, $varioopt );
    	$this->_modIsActive=1;
    }
    
    function get_SUC_status_finished() {
        return $this->SUC_status_finished;
    }
    function get_SUC_ID() {
        return $this->parx["substid"];
    }
    
    function getPosOfSubst(&$sqlo) {
    	
    	$sqls="select CONTAINER_ID, POS from CONT_HAS_CSUBST where CONCRETE_SUBST_ID=". $this->parx["substid"].
    			 " AND CONTAINER_ID=".$this->contID;
    	$sqlo->query($sqls);
    	$pos  = 0;
    	if ( $sqlo->ReadRow() ) {
    		$pos  = $sqlo->RowData[1];
    		return $pos;
    	}
    }
    
    /**
     * get last ALIQUOT-ID of substance in container
     * @param object $sqlo
     */
    function getLastAliSubst(&$sqlo) {
    	$sqls="select max(ALIQUOT_NO) from CONT_HAS_CSUBST where CONCRETE_SUBST_ID=". $this->parx["substid"].
    			 " AND CONTAINER_ID=".$this->contID;
    	$sqlo->query("$sqls");
    	$sqlo->ReadRow();
    	$maxAliNum  = $sqlo->RowData[0];
    	if (!$maxAliNum) $maxAliNum=0;
    	return $maxAliNum;
    }
    
    /**
     * get next free position and analyse abstract_container
     * - behind this position can be other areas
     * 00000XXXX00000XXXX CONCRETE_SUBST_ID
     * RRRRRR000000000000 reserved
     *        A
     *        | current pointer
     * @param $sqlo
     * @param $startpos
     * @return int position
     */
    function getFreePosAndAbs(&$sqlo, $startpos=1) {
    	global $error;
    	$FUNCNAME= "getFreePosAndAbs";
    	
    	$freepos = $this->getFreePos($sqlo, $startpos);
    	
    	if ( $freepos>$this->maxPos ) {
    		$error->set( $FUNCNAME, 2, 'Max pos ('.$this->maxPos.') exceeded.' );
    		return;
    	}
    	
    	return ($freepos);
    }
    
    /**
     * get next free position
     * @param $sqlo
     * @param $startpos
     * @return int position
     */
    function getFreePos(&$sqlo, $startpos=0) {
    	
    	$contID = $this->contID;
    	
    	$takePos      = $startpos;
    	if (!$takePos) $takePos = 1; // fall back, if $startpos not given
    	
    	do  {
    		$sqlsel="CONCRETE_SUBST_ID, RESERVED from CONT_HAS_CSUBST where CONTAINER_ID=". $contID.
    					" and POS=".$takePos;
    		$sqlo->Quesel($sqlsel);
    		$sqlo->ReadRow();
    		$hasSubstance = $sqlo->RowData[0];
    		$RESERVED     = $sqlo->RowData[1];
    		
    		if (!$hasSubstance and !$RESERVED) break;
    		
    		$takePos = $takePos+1;
    	} while (1);
    	
    	$freepos = $takePos;
    	
    	return ($freepos);
    }
    
    /**
     * get next free position after LAST POS
     * example:
     *  POS   123456789
     *  SUBST XXXXX----
     *   => return pos 6
     * @param $sqlo
     * @param $startpos
     * @return int position
     */
    function getFreePosEnd(&$sqlo) {
    	
    	$contID = $this->contID;
    	
    	// get last POS
    	$sqlsel="max(POS) from CONT_HAS_CSUBST where CONTAINER_ID=". $contID;
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadRow();
    	$lastPos = $sqlo->RowData[0];
    	
    	if (!$lastPos) $lastPos=0;
    	
    	$freepos = $lastPos+1;
    	
    	return ($freepos);
    }
    
    /**
     * if a new postion should be used, do this check
     * if SUC on old POS already exists: no error
     * @param object $sqlo
     * @param number $startpos
     * @return int
     */
    function add_check(&$sqlo, $startpos=1) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	if (!$this->parx["substid"])  {
    	    $error->set( $FUNCNAME, 1, "Input missing: SUC");
    	    return;
    	}
    	
    	
    	$contID = $this->contID;
    	$posIN  = $this->parx["POS"];
    	
    	if ( is_numeric($posIN) ) {
    	    
    		$sqls="select CONCRETE_SUBST_ID from CONT_HAS_CSUBST where CONTAINER_ID=". $contID.
    				" AND POS=".$posIN;
    		$sqlo->query($sqls);
    		$sqlo->ReadRow();
    		$substold  = $sqlo->RowData[0];
    		if ( $substold and $substold!=$this->parx["substid"] ) {
    			$error->set( $FUNCNAME, 3, "POS ".$posIN." already used!");
    			return;
    		}
    		$newpos = $posIN;
    	} else {
    		
    		$newpos = $this->getFreePos($sqlo, $startpos);
    	}
    	
    	$this->newpos = $newpos;
    	
    	return $newpos;
    }
    
    // store next free position
    function _setStorePointer(&$sqlo, $pos) {
    	$this->varioModLib->updateKeyVal( $sqlo, 'storePointer', $pos);
    }
    
    /**
     * DEPRECATED
     * add a substance to a container
     * @param object $sqlo
     * @param number $touch
     */
    // function add( &$sqlo, $touch=0 ) {
    //     global $error;
    //     $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    // 	$pos = $this->newpos;
    	
    // 	if (!$pos) {
    // 	    $error->set( $FUNCNAME, 1, 'Input: POS missing.' );
    // 	    return;
    // 	}
    	
    // 	$argu = array(
    //   		"POS" => 	$pos,
    // 		"CONCRETE_SUBST_ID" => 	$this->parx["substid"],
    // 	    "QUANTITY" => 	$this->parx["QUANTITY"],
    // 	    "MOD_DATE"	 => date_unix2datestr( time(),1 ),
    // 		);
    	
    // 	$this->assoclib->insert($sqlo, $argu);
    // 	$this->_setStorePointer($sqlo, $pos+1);
    	
    // 	return $pos;
    // }
    
    // 
    
    /**
     * ad CONTAINER log entry
     * @param object $sqlo
     * @param int $pos
     * @param int $suc_id
     * @param float $QUANT_old
     * @param float $QUANT_new
     * @param string $key - e.g. "del"; 
     *    if NOT given, will be calculated by $QUANT_old, $QUANT_new
     */
    private function add_log($sqlo, $cont_pos, $suc_id, $QUANT_old, $QUANT_new, $key=NULL ) {
        
        if (!$this->cont_log_active) return;
        if (!table_exists2('CONT_LOG')) return;
        
    
        $sqlsel="max(POS) from CONT_LOG where CONTAINER_ID=". $this->contID;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $last_pos = $sqlo->RowData[0];
        if(!$last_pos) $last_pos=0;
        $new_pos=$last_pos+1;
        $quant_diff = $QUANT_new - $QUANT_old;
        
        if ($key==NULL) {
            $key='none';
            if ($quant_diff>0) $key='add';
            if ($quant_diff<0) $key='del';
        }
        
        if (!$suc_id) return; // internal error
        
        $datax = json_encode(array('pos'=>$cont_pos));
        
        $argu=array(
            "CONTAINER_ID"=>$this->contID,
            "POS"=>$new_pos,
            "MOD_DATE"=>date_unix2datestr(time(), 1),
            "CONCRETE_SUBST_ID"=>$suc_id,
            'QUANTITY'=>$quant_diff,
            "DB_USER_ID"=> $_SESSION['sec']['db_user_id'],
            "KEY"=>$key
        );
        
        if (glob_column_exists('CONT_LOG', 'DATAX')) {
            $argu['DATAX'] = $datax;
        }
        insert_row_s( $sqlo ,'CONT_LOG', $argu );
    }
    
    static function get_SUC_quant($sqlo, $suc_id) {
        $sqlsel="sum(QUANTITY) from CONT_HAS_CSUBST where  CONCRETE_SUBST_ID=".$suc_id;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $quant  = $sqlo->RowData[0];
        if (!$quant) $quant=0;
        return $quant;
    }
    
    function has_status($sqlo, $status_name, $suc_id) {
        $answer=0;
        $status_lib = new gObj_StatusMod($sqlo, 'CONCRETE_SUBST', $suc_id);
        if ($status_lib->has_status($sqlo, $status_name)) $answer = 1;
            
        return $answer;
    }
    
    /**
     * analyse stock amount system wide
     * - save result in $this->SUC_stock_status
     * - do AUTO finish, if stock is empty ...
     * @param object $sqlo
     * @param int $suc_id
     * @return int
     *    1 is stock
     *   -1 no stock
     */
    private function _ana_stock_of_SUC($sqlo, $suc_id) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        if (!$suc_id) {
            throw new Exception('SUC_ID missing.');
        }
        
        $answer = 0;
        $this->SUC_status_finished = 0;
        
//         $sqlsel="sum(QUANTITY) from CONT_HAS_CSUBST where  CONCRETE_SUBST_ID=".$suc_id;
//         $sqlo->Quesel($sqlsel);
//         $sqlo->ReadRow();
//         $quant  = $sqlo->RowData[0];
        
//         if ($quant>0) $answer=1;
//         else $answer=-1;
        
//         debugOut('SUC:'.$suc_id.' is still on stock:'.$answer, $FUNCNAME, 1);
        
//         $this->SUC_stock_status = $answer;
        
//         if ($this->SUC_stock_status<0) {
//             // try to finish
//             $new_status='finished';
//             $status_lib = new gObj_StatusMod($sqlo, 'CONCRETE_SUBST', $suc_id);
//             if (!$status_lib->has_status($sqlo, $new_status)) {
//                 debugOut('SUC:'.$suc_id.' set status:finish NOW!', $FUNCNAME, 1);
//                 $status_lib->status_change($sqlo, $new_status, 'auto-finished, reason: empty stock');
//                 $this->SUC_status_finished = 1;
//             }
//             debugOut('SUC:'.$suc_id.' finished-analysis: was finished now? '.$this->SUC_status_finished, $FUNCNAME, 1);
//         }
        
        return $answer;
    }
    
    // set status to finished
    function set_suc_finished($sqlo, $suc_id) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        // try to finish
        $new_status='finished';
        $status_lib = new gObj_StatusMod($sqlo, 'CONCRETE_SUBST', $suc_id);
        if (!$status_lib->has_status($sqlo, $new_status)) {
            debugOut('SUC:'.$suc_id.' set status:finish NOW!', $FUNCNAME, 1);
            $status_lib->status_change($sqlo, $new_status, 'auto-finished, reason: empty stock');
            $this->SUC_status_finished = 1;
        }
    }
    
    /**
     * add a substance-aliquot to a container
     * - no special check
     * - store next free pointer
     * @param object $sqlo
     * @param int $pos current position
     * @param int $substid
     * @param int $ali_id : can be 0
     * @param int $quant : quantity of stored substance
     * @return -
     */
    function addAliquot( &$sqlo, $pos, $substid, $ali_id, $quant=0 ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$substid) {
            $error->set( $FUNCNAME, 1, "Input missing: SUC");
            return;
        }
        
        if (!$this->_modIsActive) sys_error_my( 'class '.__CLASS__.' not initialized.' );
      
    	$updateOpt = array( "types" => array("MOD_DATE"=> "DATE1") );
    	$argu = array(
    			"CONCRETE_SUBST_ID" => 	$substid,
    			"ALIQUOT_NO" => $ali_id,
    			"MOD_DATE"	 => date_unix2datestr( time(),1 ),
    	        'QUANTITY' => $quant
    			);
    
    	$quant_old = 0;
    	$sqlsel="POS, QUANTITY from CONT_HAS_CSUBST where CONTAINER_ID=". $this->contID." and POS=".$pos;
    	$sqlo->Quesel($sqlsel);
    	// entry exists ?
    	if ( $sqlo->ReadRow() ) {
    	    
    	    $quant_old  = $sqlo->RowData[1];
    		$pkArr=array( "POS"=>$pos );
    		$this->assoclib->update($sqlo, $argu, $pkArr, $updateOpt);
    
    	} else {
    		$argu["POS"]=$pos;
    		$this->assoclib->insert($sqlo, $argu, $updateOpt);
    		
    	}
    	
    	
    	$this->add_log($sqlo, $pos, $substid, $quant_old, $quant);
    	$this->_setStorePointer($sqlo, $pos+1);
    }
    
    /**
     * update existing POS
     * - update only KEYs in $argu !
     * @param $sqlo
     * @param int $pos
     * @param array $arguin : key, value pairs of CONT_HAS_CSUBST
     */
    function updateAliquot(&$sqlo, $pos, $arguin) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
    	if (!$this->_modIsActive) sys_error_my( 'class '.__CLASS__.' not initialized.' );
    	
    	$this->_init_subst();
    	
    	$allowArr = array(
    		"CONCRETE_SUBST_ID",
    		"ALIQUOT_NO",
    		"RESERVED",
    		"LAST_SUBST_ID",
    		"NOTES",
    	    "QUANTITY"
    	);
    	
    	$old_vals=array();
    	$argu=NULL;
    	foreach( $arguin as $key=>$val) {
    		if (in_array($key,$allowArr)) $argu[$key]=$val;
    	}
    
    	
    	$old_vals = glob_elemDataGet3( $sqlo, 'CONT_HAS_CSUBST', array('CONTAINER_ID'=>$this->contID, 'POS'=>$pos), array("CONCRETE_SUBST_ID",'QUANTITY') );
    	$this->parx["substid"] = $old_vals["CONCRETE_SUBST_ID"];
    	
    	debugOut( 'POS:'.$pos, $FUNCNAME.":OLD_VALS", 1 );
    	debugOut( print_r($old_vals,1), $FUNCNAME.":OLD_VALS", 1 );
    	
    	
    	// set automatically
    	$argu["MOD_DATE"] = date_unix2datestr( time(),1 );
    	$pkArr=array( "POS"=>$pos );
    	$this->assoclib->update($sqlo, $argu, $pkArr);
    	if ($this->cont_log_active and array_key_exists('QUANTITY', $arguin) ) {
    	    // only if QUANTITY is set ...
    	    
    	    $suc_id = $old_vals["CONCRETE_SUBST_ID"];
    	   
    	    $this->add_log($sqlo, $pos, $suc_id, $old_vals['QUANTITY'], $argu['QUANTITY']);
    	    
    	    $this->_ana_stock_of_SUC($sqlo, $suc_id);
    	}
    }
    
    /**
     * take out a quantity
     * @param $sqlo
     * @param int $pos : container-pos
     * @param float quant : take out a quantity
     */
    function takeOut(&$sqlo, $pos, $quant_takeout) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$this->_modIsActive) sys_error_my( 'class '.__CLASS__.' not initialized.' );
        
        $old_feats = glob_elemDataGet3($sqlo, 'CONT_HAS_CSUBST', array('CONTAINER_ID'=>$this->contID, 'POS'=>$pos), array('QUANTITY', 'CONCRETE_SUBST_ID') );
        
        $old_quant = $old_feats['QUANTITY'];
        $new_quant = $old_quant - $quant_takeout;
        if ($new_quant<0) $new_quant = 0;
        
        $argu=array();
        $argu['QUANTITY'] = $new_quant;
        $argu["MOD_DATE"] = date_unix2datestr( time(),1 );
        
        $pkArr=array( "POS"=>$pos );
        
        $suc_id = $old_feats['CONCRETE_SUBST_ID'];
        $this->parx["substid"] = $suc_id;
     
        $this->assoclib->update($sqlo, $argu, $pkArr);
        $this->add_log($sqlo, $pos, $suc_id, $old_quant, $new_quant);
        
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 2, 'Error on update on POS:'.$pos );
            return;
        }
        $this->_ana_stock_of_SUC($sqlo, $suc_id);
    }
    
    
    /**
     * take out one substance on POS; SUC must defined before
     * @return -
     * @param  object $sqlo
     * @global $this->parx[ "POS", "substid" ]
     */
    function del( &$sqlo ) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    	//$tablename = "CONTAINER";
    	$substid= $this->parx["substid"];
    	$pos    = $this->parx["POS"];
    	if (!$substid) {
    	    $error->set( $FUNCNAME, 1, 'Internal error: SUC not set.' );
    	    return;
    	}
    	if (!$pos) $pos=0;
    	
    	// get pos
    	$sqls="select POS,QUANTITY from CONT_HAS_CSUBST where CONCRETE_SUBST_ID=". $substid.
    			 " AND CONTAINER_ID=".$this->contID. " AND POS=".$pos;
    	$sqlo->query($sqls);
    	$sqlo->ReadRow();
    	$pos  = $sqlo->RowData[0];
    	$old_quant  = $sqlo->RowData[1];
    	
    	if ( !$pos ) {
    		$error->set( $FUNCNAME, 2, "Substance not found on container!");
    		return;
    	}
    
    	
    	$idarr = array( 'POS'=>$pos );
    	$this->assoclib->delOneRow($sqlo, $idarr);
    	
    	if ($error->Got(READONLY))  {
    	    $error->set( $FUNCNAME, 2, 'Error on delete on POS:'.$pos );
    	    return;
    	}
    	
    	
    	$new_quant=0;
    	$this->add_log($sqlo, $pos, $substid, $old_quant, $new_quant, 'del');  
    	
    	$this->_ana_stock_of_SUC($sqlo, $substid);
    	
    	
    	// echo "... substance removed from container.<br>";
    }
    
    /**
     * take out one substance from POS 
     * @return -
     * @param  object $sqlo
     */
    function delpos( &$sqlo, $pos ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$this->contID) {
            $error->set( $FUNCNAME, 1, 'Input CONT_ID missing.' );
            return;
        }
        if (!$pos) {
            $error->set( $FUNCNAME, 2, 'Input POS missing.' );
            return;
        }
        if (!$this->_modIsActive) {
            $error->set( $FUNCNAME, 3, 'Modification not initialized.' );
            return;
        }
    	
    	// get pos
    	$sqls="select CONCRETE_SUBST_ID, QUANTITY from CONT_HAS_CSUBST where ".
    		" CONTAINER_ID=".$this->contID. " AND POS=".$pos;
    	$sqlo->query($sqls);
    	$sqlo->ReadRow();
    	$substid    = $sqlo->RowData[0];
    	$old_quant  = $sqlo->RowData[1];
    	
    	$this->parx["substid"] = $substid; // store this SUC-ID
    	
    	if ( !$substid ) {
    		// no action
    		return;
    	}
    
    	
    	$idarr = array( 'POS'=>$pos );
    	$this->assoclib->delOneRow($sqlo, $idarr);
    	
    	if ($error->Got(READONLY))  {
    	    $error->set( $FUNCNAME, 2, 'Error on delete on POS:'.$pos );
    	    return;
    	}
    	
    	$new_quant=0;
    	$this->add_log($sqlo, $pos, $substid, $old_quant, $new_quant, 'del');
    	
    	$this->_ana_stock_of_SUC($sqlo, $substid);
    	
    }

    /**
     * finish modification actions
     * @param  $sqlo
     */
    public function modClose(&$sqlo) {
    	$this->assoclib->close($sqlo);
    }
    
    /**
     * ADD SUA to Container
     * @param object $sqlo
     * @param int $SUA_ID
     */
    function add_SUA($sqlo, $SUA_ID) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$this->contID) {
            sys_error_my( 'Class not initialized.' );
        }
        
        $STO_objid = $this->contID;
        
        $sqlsel = 'POS from CONT_HAS_ASUBST where CONTAINER_ID='.$STO_objid. ' and ABSTRACT_SUBST_ID='.$SUA_ID;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $has_pos = $sqlo->RowData[0];
        if ($has_pos) {
            $error->set( $FUNCNAME, 1, 'Material already in this storage.');
            return;
        }
        
        $sqlsel = 'max(POS) from CONT_HAS_ASUBST where CONTAINER_ID='.$STO_objid;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $last_pos = $sqlo->RowData[0];
        if (!$last_pos) $last_pos=0; // make to integer
        $new_pos  = $last_pos+1;
        
        $args_STO=array();
        $args_STO['ABSTRACT_SUBST_ID'] = $SUA_ID;
        $args_STO['POS'] = $new_pos;
        $assoclib = new  fAssocUpdate();
        $assoclib->setObj( $sqlo, 'CONT_HAS_ASUBST', $STO_objid );
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 2, 'Error on write init of CON-ID:'.$STO_objid );
            return;
        }
        
        $assoclib->insert( $sqlo, $args_STO );
        
    }

    /**
     * remove SUA from CONTAINER
     * @param object $sqlo
     * @param int $SUA_ID
     */
    function remove_SUA($sqlo, $SUA_ID) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$this->contID) {
            sys_error_my( 'Class not initialized.' );
        }
        $STO_objid = $this->contID;
        
        $pk_arr=array();
        $pk_arr['ABSTRACT_SUBST_ID'] = $SUA_ID;
        
        $assoclib = new  fAssocUpdate();
        $assoclib->setObj( $sqlo, 'CONT_HAS_ASUBST', $STO_objid );
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on write init of CON-ID:'.$STO_objid );
            return;
        }
        $assoclib->delOneRow( $sqlo, $pk_arr );
    }


}
