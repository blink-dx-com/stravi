<?php 
require_once 'o.S_OBJLINK.subs.inc';
require_once 'o.ABS_SOCKET.subs.inc';
require_once 'o.ABSTRACT_SUBST.bom.inc';
require_once 'date_funcs.inc';
require_once 'date_work.inc';
require_once 'o.S_VARIO.subs.inc';


/**
 * MORDER sub functions
 * @author Steffen
 * @package o.MORDER.subs.inc
 *
 */
class oMORDER_subs {
    
    const VARIO_KEY='prod.obj';
    const SOBJ_SUA_WFL_KEY='o.MORDER.Workflow';
    
    const STATUS_ARR = array(
        1=>'started',
        2=>'beauftragt',
        3=>'reviewed',
        4=>'prod_start',
        5=>'abgeschlossen',
    );
    
    const STATUS_STARTED='started';
    const STATUS_ORDERED='beauftragt';
    const STATUS_REVIEWED='reviewed';
    const STATUS_PROD_START='prod_start';
    const STATUS_PROD_CLOSED='abgeschlossen';
    
    
    function __construct($order_id=NULL) {
        if ($order_id) {
            $this->order_id = $order_id;
        }
    }
    
    function set_obj($order_id) {
        $this->order_id = $order_id;
    }
    
    function get_all_features(&$sqlo) {
        if(!$this->order_id) return;
        
        $sqlsel = '* from MORDER where MORDER_ID='.$this->order_id;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadArray();
        $data = $sqlo->RowData;
        
        return $data;
    }
    
    /**
     * get produced children
     * @param object $sqlo
     * @return array of SUC-ID
     */
    function get_prod_SUCs(&$sqlo) {
        if (!$this->order_id) {
            sys_error_my( 'Class not initialized.' );
        }
        
        $objlink_lib = new oS_OBJLINK_subs();
        $children = $objlink_lib->getChiObjByChiTab($sqlo, 'MORDER', $this->order_id, 'CONCRETE_SUBST');
        
        return $children;
    }
    
    /**
     * get MORDER of SUC
     * @param object $sqlo
     * @return int $parent_id
     */
    static function SUC_has_order(&$sqlo, $suc_id) {

        $objlink_lib = new oS_OBJLINK_subs();
        $parent_id = 0;
        $parents   = $objlink_lib->getMoObjsbyTabKey($sqlo, 'CONCRETE_SUBST', $suc_id, 'MORDER', self::VARIO_KEY);
        if (!empty($parents)) {
            $parent_id = $parents[0];
        }
        
        return $parent_id;
    }
    
    /**
     * SUA has BOM ???
     * @param object $sqlo
     * @return number
     */
    function sua_has_bom($sqlo) {
        
        $answer = 0;
        $order_features = glob_elemDataGet3( $sqlo, 'MORDER', array('MORDER_ID'=>$this->order_id), array('ABSTRACT_SUBST_ID') );
        $SUA_ID = $order_features['ABSTRACT_SUBST_ID'];
        
        if (!$SUA_ID) return 0;
        
        $bom_lib = new oABSTRACT_SUBST_bom($SUA_ID);
        $components = $bom_lib->get_components($sqlo);
        if (sizeof($components)) $answer = 1;
        return $answer;
    }
        
    
    /**
     * return 0 or 1
     * @param object $sqlo
     */
    function has_produced_SUC(&$sqlo) {
        if (!$this->order_id) {
            sys_error_my( 'Class not initialized.' );
        }
        
        $children = $this->get_prod_SUCs($sqlo);
       
        $result=0;
        if (!empty($children)) $result = 1;
        return $result;
    }
    
    /**
     * 
     * @param object $sqlo
     * @return string $type
     *   'normal'  : will be a normal SUC
     *   'BCBATCH' : will be a BCBATCH
     */
    function get_type_of_SUA($sqlo) {
        if (!$this->order_id) {
            sys_error_my( 'Class not initialized.' );
        }
        
        $order_features = glob_elemDataGet3( $sqlo, 'MORDER', array('MORDER_ID'=>$this->order_id), array('ABSTRACT_SUBST_ID') );
        $SUA_ID = $order_features['ABSTRACT_SUBST_ID'];
        
        $type='normal';
        
        $ABS_SOCKET_ID = oABS_SOCKET_subs::get_ASOC_from_SUA($sqlo, $SUA_ID);
        if ($ABS_SOCKET_ID) {
            $type='BCBATCH';
        }
        
        return $type;
    }
    /**
     * get planned MORDER-Workflow-ID from SUA:S_OBJLINK
     * @param object $sqlo
     * @return int 
     */
    function get_POR_wfl_fr_SUA($sqlo) {
        $mord_feats = glob_ObjDataGet( $sqlo, 'MORDER', $this->order_id, array('ABSTRACT_SUBST_ID'));
        $sua_id = $mord_feats['ABSTRACT_SUBST_ID'];

        $module_ids = oS_OBJLINK_subs::getChiObjByTabOrKey($sqlo, 'ABSTRACT_SUBST', $sua_id, 'MODULE', self::SOBJ_SUA_WFL_KEY);
        if (empty($module_ids)) return 0;
        return current($module_ids);
    }
    
    /**
     * get IDs of direct sub orders
     * @param object $sqlo
     * @return array of MORDER_ID
     */
    function get_sub_orders($sqlo) {
        if (!$this->order_id) {
            sys_error_my( 'Class not initialized.' );
        }
        $objlink_lib = new oS_OBJLINK_subs();
        $children = $objlink_lib->getChiObjByChiTab($sqlo, 'MORDER', $this->order_id, 'MORDER');
        return $children;
    }
    
    /**
     * get estimated time (hours) of production for one batch
     * - analyse ABSTRACT_SUBST : VARIO: 'production.time_h'
     * @param object $sqlo
     */
    function estimate_prod_time_h($sqlo) {
        
        $features = glob_elemDataGet3( $sqlo, 'MORDER', array('MORDER_ID'=>$this->order_id), array('ABSTRACT_SUBST_ID') );
        
        $sua_id = $features['ABSTRACT_SUBST_ID'];
        if (!$sua_id) return 0;
        
        // get prod hours
        $key='production.time_h';
        $vario_lib = new oS_VARIO_sub('ABSTRACT_SUBST');
        $prod_time_h  = $vario_lib->getValByKey( $sqlo, $sua_id, $key );
        
        return $prod_time_h;
    }
    
    /**
     * get date1 = date0 + work_days($add_work_hours) : consider weekend!
     * @param int $date0
     * @param int $add_work_hours
     * @return int unix time stamp
     */
    function date_add_work_hours($date0, $add_work_hours) {
        
        $HOUR_PER_DAY = Date_work::$HOUR_PER_DAY;
        $days  = ceil($add_work_hours/$HOUR_PER_DAY);
        $date1 = Date_work::addWorkingDays($date0, $days);
        return $date1;
    }
    
    /**
	 * 
	 * get MOREDRs for a time range for MANY MOREDRs
	 *  - take care, if too many answers !!!!
	 *  - use ONLY for small time range ?
	 * @param $sqlo
	 * @param array $dateArr
	 *   [0] time start UNIX
	 *   [1] time end   UNIX
	 * @param string $deviceSubSql subQuery to select the MORDERs
	 * @return array timeStamps MOR_timestamp_STRUCT
	 *    array of array(
	 *        MORDER_ID, START_DATE, END_DATE, REQU_DATE 
	 *    );
	 */
	static function getOfRangeMany(&$sqlo, $dateArr, $deviceSubSql) {
		
		$dateArrSql=NULL;
		$dateArrSql[0]=date_unix2datestr( $dateArr[0], 3);
		$dateArrSql[1]=date_unix2datestr( $dateArr[1], 3);
		
		$sqlsel = 'MORDER_ID, START_DATE, END_DATE, REQU_DATE, START_DATE_PLAN, END_DATE_PLAN from MORDER where MORDER_ID in (' .$deviceSubSql. ') '.
			' and ( '.
			  '    (START_DATE>='.$sqlo->addQuotes($dateArrSql[0]).' and START_DATE<='.$sqlo->addQuotes($dateArrSql[1]).')'.
			  ' OR (END_DATE>='.$sqlo->addQuotes($dateArrSql[0]).  ' and END_DATE<='.$sqlo->addQuotes($dateArrSql[1]).')'.
			  ' OR (REQU_DATE>='.$sqlo->addQuotes($dateArrSql[0]). ' and REQU_DATE<='.$sqlo->addQuotes($dateArrSql[1]). ')'.
			  ' OR (START_DATE_PLAN>='.$sqlo->addQuotes($dateArrSql[0]). ' and START_DATE_PLAN<='.$sqlo->addQuotes($dateArrSql[1]). ')'.
			' ) '.
			' order by START_DATE';
		$sqlo->Quesel($sqlsel);
		
		$timeStamps=array();
		$cnt=0; 
		while ($sqlo->ReadArray()) {
			$timeStamps[]=$sqlo->RowData;
			$cnt++;
		}
		return $timeStamps;
	}
	
	/**
	 * get SQL WHERE condition fro CONCRETE_SUBST_ID to select SUCs from a MORDER filter
	 * TBD: still not tested !!!
	 * @param object $sqlo
	 * @param array $filt f.sql_sel2.inc:sql_filter_STRUCT
	 * -- this is the SQL-filter
	 */
	static function sql_get_morders($sqlo, array $filt) {
	    
	    $sub_arr=array();
	    foreach($filt as $row) {
	        $sub1 = $row['col'] . ' '.$row['con'].' '. $row['val'];
	        $sub_arr[]=$sub1;
	    }
	    $morder_where = implode(' AND ',$sub_arr);
	    
	    $sub_sel   ='select MORDER_ID from MORDER where '.$morder_where;
	    $childTable='CONCRETE_SUBST';
	    $sql_where = "select CH_ID from S_OBJLINK where CH_TABLE=".$sqlo->addquotes($childTable).
	    ' and MO_ID in ('.$sub_sel.') and MO_TABLE=MORDER';
	    return $sql_where;
	    
	}
	
	/**
	 * get info from workflow
	 * @param object $sqlo
	 * @return int 0 or 1
	 */
	function get_pre_reviewed_need(object $sqlo) {
	    $CCT_ACCESS_ID = glob_ObjDataColGet($sqlo, 'MORDER', $this->order_id, 'CCT_ACCESS_ID');
	    $wfl_id = glob_ObjDataColGet($sqlo, 'CCT_ACCESS', $CCT_ACCESS_ID, 'WFLOW_ID');
	    if (!$wfl_id) return 0;
	    $key='state.pre-reviewed.need';
	    $value = glob_elementDataGet2($sqlo, 'MOD_META', array('MXID'=>$wfl_id, 'KEY'=>$key ), 'VALUE' );
	    if (!$value) $value=0;
	    $value=intval($value);
	    return $value;
	}
	
	
}