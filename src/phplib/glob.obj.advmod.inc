<?php 
require_once ('f.advmod_log.inc');
require_once ('object.info.inc');
require_once ('o.S_VARIO.subs.inc');
require_once ('o.CCT_ACCLOG.subs.inc');
require_once ('date_funcs.inc');

/**
 * g.ADVMOD > provide library to save difference of one object 
 * @package glob.obj.advmod.inc 
 * @swreq  SREQ:0003194: g.ADVMOD > provide library to save difference of one object 
 * @author Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * 
 * @var objdata_STRUCT array(
 * 	"vals" => data, 
   	"xobj" => extra_obj_data, = ('extra_class_id' => xxx, 'values' => array(attribName => value, ...) )
   	"access" => access_data
   	"vario"  => VARIO data OPTIONAL
 * )
 * 
 */
class obj_advmod {
	
    private $cacheOld; // objdata_STRUCT
    private $cacheNew; // objdata_STRUCT
	private $CCT_ACCESS_ID;
	private $_doAssLog; // do assoc table logging for this object ?
	
	const OBJ_FEA_KEYS = array('vals', 'xobj', 'vario');
	
	function __construct() {
		$this->acclogLib = new oAccLogC();
	}
	
	/**
	 * check, if logging is needed
	 * LOG if:
	 *   (1) object has Audit-Status and STATUS != 'started'
	 *   (5) all changes
	 * @param $sqlo
	 * @param $table
	 * @param $objid
	 * @return array(
	 * 	'dolog'=> 
	 * 		[0]=>no,
	 * 		 1 =>yes
	 * )
	 */
	function setObj(&$sqlo, $table, $objid) {
		
		if ( !$_SESSION['globals']['app.advmod'] ) {
			return array('dolog'=>0);
		}
		$advmod_flag = $_SESSION['globals']['app.advmod'];
		
		$this->_debug = 0;
		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
		    $this->_debug = $_SESSION["userGlob"]["g.debugLevel"];
		}
	
		$this->table = $table;
		$this->objid = $objid;
		$this->CCT_ACCESS_ID = 0;
		
		if (!cct_access_has2($table)) return array('dolog'=>0); // object type has no CCT_ACCESS_ID
		
		$pkname = PrimNameGet2($table);
		$params = glob_elemDataGet3($sqlo, $table, array($pkname=>$this->objid), array('CCT_ACCESS_ID'));
		
		$STATUS_ID_started = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $table, 'started' );
		
		if (!$params['CCT_ACCESS_ID']) return array('dolog'=>0);
		
		$this->CCT_ACCESS_ID = $params['CCT_ACCESS_ID'];
		
		$this->_interr_arr = array();
		
		if ($advmod_flag>1) {
		    return array('dolog'=>1); 
		}
		
		// has an audit status ?
		$sqls= "H_ALOG_ACT_ID from CCT_ACCESS where CCT_ACCESS_ID=".$this->CCT_ACCESS_ID;
		$sqlo->Quesel($sqls);
		$sqlo->ReadRow();
		$lastALOG_ACT_ID = $sqlo->RowData[0];
		if (!$lastALOG_ACT_ID) return array('dolog'=>0); // has no status
		if ($lastALOG_ACT_ID==$STATUS_ID_started) return array('dolog'=>0); // not a status to track ...

		return array('dolog'=>1); 
	}
	
	// store internal errors
	private function _interr($text) {
		$this->_interr_arr[] = $text;
	}
	
	/**
	 * get values, store in $cacharr
	 * @param object $sqlo
	 * @param array objdata_STRUCT $cacharr
	 */
	private function _cacheDataDyn(&$sqlo, &$cacharr) {
	    global $varcol;
	    
		$objid = $this->objid;
		$table = $this->table;
		$cacharr = array();

		$objInfoLib = new objInfoC();
		$cacharr = $objInfoLib->getParamsRaw ( $sqlo, $table, $objid);
		
		if (isset($cacharr['vals']['EXTRA_OBJ_ID'])) {
		    $varcol     = & Varcols::get();
		    $classname  = $varcol->obj_id_to_class_name( $cacharr['vals']['EXTRA_OBJ_ID'] );
		    $cacharr['vals']['EXTRA_CLASS.name'] = $classname;
		}
		
	}
	
	/**
	 * cache original data of object
	 * @param $sqlo
	 */
	function getOriData(&$sqlo) {
		$this->cacheOld = array();
		$this->_cacheDataDyn($sqlo, $this->cacheOld);
	}
	
	/**
	 * cache original data of object
	 * @param $sqlo
	 */
	function getOriData_Vario(&$sqlo, $keys) {
	    $this->cacheOld['vario'] = array();
	    $this->_cacheDataDyn_vario($sqlo, $keys, $this->cacheOld['vario']);
	}
	
	/**
	 * get ORI data for VARIO keys
	 * @param object $sqlo
	 * @param array $keys
	 */
	private function _cacheDataDyn_vario(object $sqlo, array $keys, &$cacheVario) {
	    foreach($keys as $key) {
	        $cacheVario[$key] = oS_VARIO_sub::getValByTabKey($sqlo, $this->table,$this->objid, $key);
	    }
	}
	
	/**
	 * - get data after update, save difference
	 * - for VARIO-data: call getOriData_Vario() before ...
	 * difference dict:
	 *   $entryDict = array(
			'old'=>array(
				'vals' => array('NAME'=>'checky'),
				'ass' => array( table => array(diffs))
			     ),
			'new'=> array(
				'vals' => array('NAME'=>'checky 234'),
				'ass' => array( table => array(diffs))
			)		
		  );

	 * @param object $sqlo
	 * @param $touchInfo ('act'=>, 'timest'=>)
	 * @return array('hasDiff'=>0,1);
	 */
	function getPostData(&$sqlo, $touchInfo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		//$objid = $this->objid;
		$table = $this->table;
		$this->cacheNew = array();
		
		$this->_cacheDataDyn($sqlo, $this->cacheNew);
		if (!empty($this->cacheOld['vario'])) {
		    $keys_vario = array_keys($this->cacheOld['vario']);
		    $this->_cacheDataDyn_vario($sqlo, $keys_vario,  $this->cacheNew['vario'] );
		}
		
		// calc difference
		$cacheDiff_old = array();
		$cacheDiff_new = array();
		$hasDiff = 0;
		
		//glob_printr( $this->cacheOld, "cacheOld info" );
		//glob_printr( $this->cacheNew, "cacheNew info" );
		
		$check_col_arr_VALS = columns_get_pos ($table);
		$check_col_arr_VALS[] = 'EXTRA_CLASS.name'; // not a standard column ...

		foreach (self::OBJ_FEA_KEYS as $okey) {

		    $okex_has_diff = 0;
		    $cacheX_Old = &$this->cacheOld[$okey];
		    $cacheX_New = &$this->cacheNew[$okey];
		    $check_col_arr=array();
    		
		    if ($okey=='vals') $check_col_arr=&$check_col_arr_VALS;
		    if ($okey=='xobj') { 
		        
		        if (empty($cacheX_Old) and empty($cacheX_New)) {
		            continue; // has no "xobj"
		        }
		        
		        $val_keys1=array();
		        $val_keys2=array();
		        if (!empty($cacheX_Old['values'])) $val_keys1=array_keys($cacheX_Old['values']);
		        if (!empty($cacheX_New['values'])) $val_keys2=array_keys($cacheX_New['values']);
		        $val_keys = array_merge($val_keys1, $val_keys2);
		        $check_col_arr = &$val_keys;
		        
		        $cacheX_Old = &$this->cacheOld[$okey]['values'];
		        $cacheX_New = &$this->cacheNew[$okey]['values'];
		        
		        if ($this->_debug>1) {
		            glob_printr( $cacheX_Old['values'], $FUNCNAME.":valarr_old XOBJ info" );
		            glob_printr( $cacheX_New['values'], $FUNCNAME.":valarr_new XOBJ info" );
		        }
		    }
		    
		    if ($okey=='vario') {
		        // vario values must be initialized with getOriData_Vario()
		        if (empty($cacheX_Old) and empty($cacheX_New)) {
		            continue; // has no "vario"
		        }
		        if ($this->_debug>1) {
		            debugOut( 'Has VARIO values', $FUNCNAME );
		            glob_printr( $cacheX_Old, $FUNCNAME.":valarr_old VARIO info" );
		            glob_printr( $cacheX_New, $FUNCNAME.":valarr_new VARIO info" );
		        }
		        $check_col_arr = array_keys($cacheX_Old);
		    }
		    
		    foreach( $check_col_arr as $key) {
    			
    			$valarr_old = $cacheX_Old[$key];
    			$valarr_new = $cacheX_New[$key];
    			
    			if ($valarr_old===$valarr_new) {
    				// o.k.
    			} else {
    			    if (!$okex_has_diff) {
    			        $cacheDiff_old[$okey]=array();
    			        $cacheDiff_new[$okey]=array();
    			    }
    			    
    			    $cacheDiff_old[$okey][$key] =$valarr_old;
    			    $cacheDiff_new[$okey][$key] =$valarr_new;
    				$hasDiff = 1;
    				$okex_has_diff=1;
    			}
    		}	
		}
		
		if (!$hasDiff) {
			return array( 'hasDiff'=>0 );
		}
		
		//TBD: reduce data: max 255 chars per field ...
		 
		$fullDiffArr = array(
		    'old'=>$cacheDiff_old,
		    'new'=>$cacheDiff_new
		);
		
		if ($this->_debug>0) glob_printr( $fullDiffArr, $FUNCNAME.":fullDiffArr info" );
		
		
		
		$timeStamp= $touchInfo['timest'];
		$date_str = date_unix2datestr( $timeStamp, 4);
		$testLib  = new f_advmod_log();
		$testLib->writeLine($this->CCT_ACCESS_ID, $date_str, $fullDiffArr);
	
		return array( 'hasDiff'=>$hasDiff );
	}
	
	/**
	 * get ASSOC data
	 */
	private function _cacheDataDyn_ass(&$sqlo, &$cacheAss) {
		$primas     = $this->primas;
		$orderCol   = $this->assinf['orderCol'];
		$is2D_array = 0;
		
		if (!empty($this->assinf['doublekey'])) {
			$is2D_array = 1;
			$orderCol = $this->assinf['doublekey'][0].', '.$this->assinf['doublekey'][1];
			$id1_col = $this->assinf['doublekey'][0];
			$id2_col = $this->assinf['doublekey'][1];
		}
		
		
		//$this->primas[2]
		
		$sqlsel = "* from ".$this->assoctable." where ".$primas[0]."=".$sqlo->addQuotes($this->objid).
			" order by ".$orderCol;
		$sqlo->Quesel($sqlsel);

		if (!$is2D_array) {
			while ( $sqlo->ReadArray() ) {
				unset($sqlo->RowData[$primas[0]]); // no need of primary key ...
				$stepid = $sqlo->RowData[$orderCol];
			    $cacheAss[$stepid] = $sqlo->RowData;
			}
		} else {
			// e.g. WORKLIST_ENTRY
			while ( $sqlo->ReadArray() ) {
				unset($sqlo->RowData[$primas[0]]); // no need of primary key ...
				$stepid = $sqlo->RowData[$id1_col].':'.$sqlo->RowData[$id2_col];
			    $cacheAss[$stepid] = $sqlo->RowData;
			}
		}
	}
	
	/**
	 * get ORIGINAL feature list data (associated)
	 * - also tested for table 'WORKLIST_ENTRY'
	 * @param  $sqlo
	 * @param  string $assoctable
	 * @param array $option
	 *    primas_switch : 0,1 : switch primary key names; 
	 *      needed if an alternative Mother table like DB_USER is used
	 * @return array('dolog'=>0,1)
	 */
	public function getOriData_ass(&$sqlo, $assoctable, $option=array() ) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		$this->_doAssLog = 1;
		
		// analyse only these tables ...
		$goodtables = array(
			'ABSTRACT_PROTO_STEP',
			'ACCEPT_PROT_STEP',
			'ADEV_HAS_SIC',
			'AS_HAS_PR',
		    'CART_H_SUC',
			'CARTB_HAS_SUBS',
			'CONCRETE_PROTO_STEP',
			'CS_HAS_PR',
			'EXP_HAS_PROTO',
			'EXP_TMPL_HAS_PROTO',
			'DB_USER_IN_GROUP',
		    'USER_ROLES'
			// 'WORKLIST_ENTRY'
		   );
		if (!in_array($assoctable, $goodtables)) {
			$this->_doAssLog = 0;
			return array('dolog'=>0);
		}   
		
		$this->assinf     = array();
		$this->assoctable = $assoctable;
		$this->primas     = primary_keys_get2($assoctable);
		if (is_array($option)) {
		    if ($option['primas_switch']>0) {
		        $tmp  = $this->primas;
		        $this->primas[0] = $tmp[1];
		        $this->primas[1] = $tmp[0];
		    }
		    
		}
		
		$orderCol = $this->primas[1];
		if ($this->primas[2]!=NULL) {
			$orderCol = $this->primas[2];
		}
		
		if ($this->assoctable=='WORKLIST_ENTRY') {
			$this->assinf['doublekey'] = array($this->primas[1], $this->primas[2]); 
		}
		
		if ($orderCol==NULL) {
			$this->_interr('Bizarre: assoctable '.$this->assoctable. ' has no second PK');
			$this->_doAssLog = 0;
			return array('dolog'=>0);
		}
		
		$this->assinf['orderCol'] = $orderCol;
		
		$this->cacheAssOld = array();
		
		$this->_cacheDataDyn_ass($sqlo, $this->cacheAssOld);
		
		if ($this->_debug>1) debugOut('ADVMOD active ...', $FUNCNAME);
		
		return array('dolog'=>1);
	}
	
	/**
	 * save modification log
	 * @param  $sqlo
	 * @param array $touchInfo
	 * 	 ('act'=>, 'timest'=>)
	 */
	public function getPostData_ass(&$sqlo, $touchInfo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		if (!$this->_doAssLog) return;
		
		$objid = $this->objid;
		$assoctable = $this->assoctable;
		$this->cacheAssNew = array();
		
		$this->_cacheDataDyn_ass($sqlo, $this->cacheAssNew);
		
		// calc difference
		$cacheDiff_old = array();
		$cacheDiff_new = array();
		$hasDiff = 0;
		
		//glob_printr( $this->cacheOld, "cacheOld info" );
		//glob_printr( $this->cacheNew, "cacheNew info" );
		
		$checkKeyArr = columns_get_pos ($assoctable);
		if (empty($checkKeyArr)) return array('hasDiff'=>0); // no keys to check
		
		$keys_old = array_keys($this->cacheAssOld);
		$keys_new = array_keys($this->cacheAssNew);
		$keys_all = array_merge($keys_old, $keys_new);
		$keys_all = array_unique($keys_all);
		
		if ($this->_debug>2) {
			glob_printr( $keys_all, $FUNCNAME.":keys_all info" );
		}
		
		if (empty($keys_all)) return array('hasDiff'=>0);  // no keys exist
		
		if ($this->_debug>2) {
		    glob_printr( $this->cacheAssOld, $FUNCNAME.":valarr_old info" );
		    glob_printr( $this->cacheAssNew, $FUNCNAME.":valarr_new info" );
		}
		
		foreach( $keys_all as $stepnr) {
		
			$valarr_old = $this->cacheAssOld[$stepnr];
			$valarr_new = $this->cacheAssNew[$stepnr];
			if (!is_array($valarr_old)) $valarr_old=array();
			if (!is_array($valarr_new)) $valarr_new=array();

			$hasDiff=0;
			do {
				
				// array_diff_assoc() checks only in ONE direction; thats why we need 
				// this method twice!
				if ( empty( array_diff_assoc($valarr_old, $valarr_new)) and 
					 empty( array_diff_assoc($valarr_new, $valarr_old)) ) {
					break;
				}
				
				$diffOneStep=array();
				reset ($checkKeyArr);
				foreach( $checkKeyArr as $dummy=>$valkey) {
					if ($valarr_new[$valkey]===$valarr_old[$valkey]) {
						// o.k.
					} else {
						$diffOneStep[]=$valkey;
						$hasDiff = 1;
					}
				}
				
				
			} while (0);
			
			if ($hasDiff) {
				reset ($diffOneStep);
				foreach( $diffOneStep as $valkey) {
					$cacheDiff_old[$stepnr][$valkey] =$valarr_old[$valkey];
					$cacheDiff_new[$stepnr][$valkey] =$valarr_new[$valkey];
				}
				 
			}
		} 
		
		$fullDiffArr = array(
			'old'=>array(
				'ass' => array( 
		    		$assoctable=> $cacheDiff_old
				   )
			     ),
			'new'=> array(
				'ass' => array( 
		    		$assoctable=>$cacheDiff_new
				 )
			)		
		);
		
		if ($this->_debug>1) glob_printr( $fullDiffArr, $FUNCNAME.":fullDiffArr info" );
		

		$timeStamp= $touchInfo['timest'];
		if (!$timeStamp) $timeStamp=time(); // fall back
		$date_str = date_unix2datestr( $timeStamp, 4);
		$testLib  = new f_advmod_log();
		$testLib->writeLine($this->CCT_ACCESS_ID, $date_str, $fullDiffArr);
	
		return array( 'hasDiff'=>$hasDiff );
	}
}
