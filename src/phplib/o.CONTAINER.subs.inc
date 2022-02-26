<?php
require_once ('object.info.inc');
require_once 'o.ABS_CONTAINER.subs.inc';

/**
 * CONTAINER sub methods
 * - features:
 *   ['vario']['storePointer'] : storePointer
 *   ['vario']['pos_method']  :   pos_method -- method to calculate POS
 *     [AUTO] : auto calculate POS
 *     ALIAS.PART : give first part of alias
 *     ALIAS.EXACT : give exact ALIAS
 *   ['vario']['alias.preselect'] : KOMMA separated alias strings, needed for pos_method=ALIAS.PART
 * @package o.CONTAINER.subs.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oCONTAINER_SubsC {
	
	var $contID; // container ID
	var $storePointer; // current storepointer
	private $pos_method; // see ['vario']['pos_method'] 
	var $ABS_CONTAINER_ID; 
	
	function __construct() {
	    $this->absContLib = new oABS_CONTAINER_subs();
	}
	
	
	/**
	 * get also  ABS_CONTAINER_ID
	 * @return 
	 * @param object $sqlo
	 * @param object $contid
	 */
	function setContainer(&$sqlo, $contid) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
		$this->contID = $contid;
		$this->feats=NULL;
		
		$objInfoLib = new objInfoC();
		$rawopt=array("vario"=>1);
		$this->featsAll = $objInfoLib->getParamsRaw ( $sqlo, 'CONTAINER', $contid, $rawopt);

		$this->ABS_CONTAINER_ID = $this->featsAll['vals']['ABS_CONTAINER_ID'];
		$this->storePointer     = intval($this->featsAll['vario']['storePointer']);
		if (!$this->storePointer) $this->storePointer=1;
		
		$abs_coord_type='NULL'; // default
		if ($this->ABS_CONTAINER_ID) {
		    $this->absContLib->setContainer($sqlo, $this->ABS_CONTAINER_ID);
		    $abs_coord_type = $this->absContLib->get_coord_type();
		}
		
		$this->pos_method     = trim($this->featsAll['vario']['pos_method']);
		if ($this->pos_method==NULL) {
		    if ($this->ABS_CONTAINER_ID) {
		       if ($abs_coord_type!='NULL') $this->pos_method='ALIAS.EXACT'; // coordinates are given ...
		    }
		}
		if ($this->pos_method==NULL) {
		    $this->pos_method='AUTO'; // if nothing is given
		}
		
		debugOut('(61) abs_cont:'.$this->ABS_CONTAINER_ID.' coord_type:'.$abs_coord_type.' pos_method:'.$this->pos_method, $FUNCNAME, 1);
	}
	
	/**
	 * optional get preselected values for ALIAS
	 * @param object $sqlo
	 */
	function get_alias_select_vals($sqlo) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $this->alias_preselect=array();
	    if ($this->featsAll['vario']['alias.preselect']!=NULL) {
	        // KOMMA separated alias strings
	        debugOut('(75) alias_preselect: get from CONTAINER', $FUNCNAME, 1);
	        $this->alias_preselect= explode(',',$this->featsAll['vario']['alias.preselect']);
	    }
	    
	    if (empty($this->alias_preselect )) {
    	    if ($this->ABS_CONTAINER_ID) {
    	        $this->alias_preselect = $this->absContLib->get_alias_select_vals($sqlo);
    	        debugOut('(83) alias_preselect: get from ABS_CONTAINER; length:'.sizeof($this->alias_preselect), $FUNCNAME, 1);
    	    }
	    }

	    return $this->alias_preselect;
	}
	
	/**
	 * get all containers
	 * @param object $sqlo
	 * @return array[]
	 */
	static function get_containers($sqlo) {
	    
	    $cont_arr = array();
	    $sqlsel   = 'CONTAINER_ID, NAME from CONTAINER order by NAME';
	    $sqlo->Quesel($sqlsel);
	    while ( $sqlo->ReadRow() ) {
	        $id = $sqlo->RowData[0];
	        $name = $sqlo->RowData[1];
	        $cont_arr[$id] = $name;
	    }
	    return $cont_arr;
	}
	
	/**
	 * get data  from kea array
	 * @param array/string  $key_arr
	 * - array('vals'=>array('ABS_CONTAINER_ID') )
	 * - array(
	 *   'ABS_CONTAINER'=>array( // ABS_CONTAINER features
	 *       'poscnt' : number of POS entries
	 *       )
	 * - 'pos_method' => string
	 * @return mixed
	 */
	function getFeature(&$sqlo, $key_arr) {
	    if (!$this->contID) {
	        sys_error_my( 'Class not initialized.' );
	    }
	    
	    $data = NULL;
	    
	    do {
	        
	        if (is_array($key_arr)) {
	        
        	    if (is_array($key_arr['vals'])) {        
        	        foreach($key_arr['vals'] as $column) {
        	            $data[$column] = $this->featsAll['vals'][$column];
        	        } 
        	        break;
        	    }
        	    
        	    if ( is_array($key_arr['ABS_CONTAINER']) ) {

    	            foreach( $key_arr['ABS_CONTAINER'] as $column) {
    	                if ($column=='poscnt') {
    	                    $data['poscnt'] = $this->getAbsPosNum($sqlo); 
    	                }
    	            }
        	        break;
        	    }
	        } else {
	            switch ($key_arr) {
	                case 'pos_method':
	                    $data = $this->pos_method;
	                    break;
	            }
	        }
	    } while (0);
	    
	    
	    return $data;
	}
	
	function getCurrPointer() {
		return $this->storePointer;
	}
	
	function get_ABS_CONTAINER_ID() {
	    return $this->ABS_CONTAINER_ID;
	}
	
	function getPosAlias(&$sqlo, $pos) {
		if (!$this->ABS_CONTAINER_ID) return;
		
		$sqlsel = 'ALIAS from ABS_CONT_ENTRY '.
			' where ABS_CONTAINER_ID='.$this->ABS_CONTAINER_ID. ' and POS='.$pos;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$alias = $sqlo->RowData[0];
		return ($alias);
	}
	
	// get all features of POS from ABS_CONTAINER
	// @return array features [KEY]=VALUE
	function getPosFeats(&$sqlo, $pos) {
		if (!$this->ABS_CONTAINER_ID) return;
		
		$sqlsel = '* from ABS_CONT_ENTRY '.
			' where ABS_CONTAINER_ID='.$this->ABS_CONTAINER_ID. ' and POS='.$pos;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadArray();
		
		return ($sqlo->RowData);
	}
	
	// get number of  of POS from ABS_CONTAINER
	// @return int
	function getAbsPosNum(&$sqlo) {
	    if (!$this->ABS_CONTAINER_ID) return 0;
	    
	    $sqlsel = 'count(1) from ABS_CONT_ENTRY '.
	   	    ' where ABS_CONTAINER_ID='.$this->ABS_CONTAINER_ID;
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    
	    return ($sqlo->RowData[0]);
	}
	
	/**
	 * count number of SUC-positions
	 * @param object $sqlo
	 */
	function count_SUC($sqlo) {
	    $sqlsel = "count(1) from CONT_HAS_CSUBST where CONTAINER_ID=".$this->contID. ' and CONCRETE_SUBST_ID>0';
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    $cnt = $sqlo->RowData[0];
	    
	    return $cnt;
	}
	
	/**
	 * count number of SUAS in CONT_HAS_ASUBST
	 * @param object $sqlo
	 * @return int
	 */
	function count_CHA_SUA($sqlo) {
	    $sqlsel = "count(1) from CONT_HAS_ASUBST where CONTAINER_ID=".$this->contID. ' and ABSTRACT_SUBST_ID>0';
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    $cnt = $sqlo->RowData[0];
	    
	    return $cnt;
	}
	
	/**
	 * SUA is in container ?
	 * @return $pos : first pos
	 * @param object $sqlo
	 * @param int $sua_id
	 */
	function SUA_in_Cont(&$sqlo, $sua_id) {
	    $exists=0;
	    $sqlsel="ABSTRACT_SUBST_ID from CONT_HAS_ASUBST where ABSTRACT_SUBST_ID=". $sua_id.
	       " AND CONTAINER_ID=".$this->contID;
	    $sqlo->Quesel($sqlsel);
	    if ( $sqlo->ReadRow() ) {
	        $exists=1;
	    }
	    
	    return $exists;
	}
	
	// get all features of POS from CONTAINER
	// @return array features [KEY]=VALUE
	function get_Conc_PosFeats(&$sqlo, $pos) {
	    if (!$this->contID) {
	        sys_error_my( 'Class not initialized.' );
	    }
	    
	    $sqlsel = '* from CONT_HAS_CSUBST '.
	   	    ' where CONTAINER_ID='.$this->contID. ' and POS='.$pos;
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadArray();
	    
	    return ($sqlo->RowData);
	}
	
	// get SUC of POS from CONTAINER
	// @return INT
	function get_SUC_of_Pos(&$sqlo, $pos) {
	    if (!$this->contID) {
	        sys_error_my( 'Class not initialized.' );
	    }
	    
	    $sqlsel = 'CONCRETE_SUBST_ID from CONT_HAS_CSUBST '.
	   	    ' where CONTAINER_ID='.$this->contID. ' and POS='.$pos;
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    
	    return ($sqlo->RowData[0]);
	}
	 
	/**
	 * substance is in container ?
	 * @return $pos : first pos
	 * @param object $sqlo
	 * @param object $substid
	 */
	function substInCont(&$sqlo, $substid) {
	
		$sqlsel="POS from CONT_HAS_CSUBST where CONCRETE_SUBST_ID=". $substid.
				 " AND CONTAINER_ID=".$this->contID. ' order by POS';
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$pos  = $sqlo->RowData[0];
		return ($pos);
	}
	
	/**
	 * get all POS of substance is in container ?
	 * @return array int all POS
	 * @param object $sqlo
	 * @param object $substid
	 */
	function subst_all_pos(&$sqlo, $substid) {
	    $posarr=array();
	    $sqlsel="POS from CONT_HAS_CSUBST where CONCRETE_SUBST_ID=". $substid.
	    " AND CONTAINER_ID=".$this->contID. ' order by POS';
	    $sqlo->Quesel($sqlsel);
	    while ($sqlo->ReadRow()) {
	       $pos  = $sqlo->RowData[0];
	       $posarr[]=$pos;
	    }
	    return ($posarr);
	}
	
	/**
	 * get all FEATURES of SUC is in container ?
	 * @return array of 
          (POS, QUANTITY, ...)  
	 * @param object $sqlo
	 * @param object $substid
	 */
	function get_one_suc_ALL(&$sqlo, $substid) {
	    $posarr=array();
	    $sqlsel="POS, QUANTITY, ALIQUOT_NO from CONT_HAS_CSUBST where CONCRETE_SUBST_ID=". $substid.
	    " AND CONTAINER_ID=".$this->contID. ' order by POS';
	    $sqlo->Quesel($sqlsel);
	    while ($sqlo->ReadArray()) {
	        $posarr[]  = $sqlo->RowData;
	       
	    }
	    return ($posarr);
	}
	
	function aliquotInCont(&$sqlo, $substid, $ali_id) {
		$sqlsel="POS from CONT_HAS_CSUBST where CONCRETE_SUBST_ID=". $substid.
				 " AND CONTAINER_ID=".$this->contID. ' and ALIQUOT_NO='.$ali_id;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$pos  = $sqlo->RowData[0];
		return ($pos);
	}

	
	function device_info($sqlo) {
	    $dev_id = $this->featsAll['vals']['CHIP_READER_ID'];
	    if (!$dev_id) return;
	    $feat_arr = glob_elemDataGet3( $sqlo, 'CHIP_READER', array('CHIP_READER_ID'=>$dev_id), array('LOCATION') );
	    return array('DEV_ID'=>$dev_id, 'LOCATION'=>$feat_arr['LOCATION']);
	}
	
	// get SUM(QUANTITY) for SUC
	function get_one_suc_QUANTITY($sqlo, $suc_id) {
	    $sqlsel = "SUM(QUANTITY) from CONT_HAS_CSUBST where CONTAINER_ID=" . $this->contID . ' and CONCRETE_SUBST_ID='. $suc_id.
	       " group by CONCRETE_SUBST_ID ";
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    $quant_sum = $sqlo->RowData[0];
	    return $quant_sum;
	}
	
	/**
	 * get COUNT of SUCs in container
	 * @param object $sqlo
	 * @param int $suc_id
	 * @return float
	 */
	function get_one_suc_COUNT($sqlo, $suc_id) {
	    $sqlsel = "COUNT(1) from CONT_HAS_CSUBST where CONTAINER_ID=" . $this->contID . ' and CONCRETE_SUBST_ID='. $suc_id;
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    $quant_sum = $sqlo->RowData[0];
	    return $quant_sum;
	}
	
	/**
	 * get SQL string for all SUC-quantities
	 * @return string
	 */
	function sql_all_sua_of_sucs() {
	    $sqlsel = "DISTINCT(ABSTRACT_SUBST_ID) from CONCRETE_SUBST where CONCRETE_SUBST_ID in (".
	       "select DISTINCT(CONCRETE_SUBST_ID) from CONT_HAS_CSUBST where CONTAINER_ID=" . $this->contID . 
	       ' group by CONCRETE_SUBST_ID'.
	       ")"; 
	    return $sqlsel;
	}
	
	/**
	 * get CHILDREN containers
	 * @param object $sqlo
	 * @return array of IDs
	 */   
	function get_cont_children($sqlo) {
	    
	    $cont_arr = array();
	    $sqlsel   = 'CONTAINER_ID, NAME from CONTAINER where PARENT_CONT_ID=' . $this->contID . ' order by NAME';
	    $sqlo->Quesel($sqlsel);
	    while ( $sqlo->ReadRow() ) {
	        $id   = $sqlo->RowData[0];
	        //$name = $sqlo->RowData[1];
	        $cont_arr[] = $id;
	    }
	    return $cont_arr;
	}
	
	function has_coordinates($sqlo) {
	    if (!$this->ABS_CONTAINER_ID) return 0;
	    
	    $has_coords = $this->absContLib->has_coordinates($sqlo);
	    return $has_coords;
	}
	
	function get_SUCs_by_SUA_sql($sua_id) {
	    if (! $this->contID) {
	        sys_error_my( 'Class not initialized, CONT_ID missing.' );
	    }
	    $sqlsel = "from CONT_HAS_CSUBST where CONTAINER_ID=" . $this->contID.' AND '.
	   	    ' CONCRETE_SUBST_ID in (select CONCRETE_SUBST_ID from CONCRETE_SUBST where ABSTRACT_SUBST_ID='.$sua_id.')';	   
	    return $sqlsel;
	}
	
	function get_pos_by_alias($sqlo, $pos_alias) {
	    if (!$this->ABS_CONTAINER_ID) {
	        sys_error_my( 'Class not initialized: ABS_CONTAINER' );
	    }
	    return $this->absContLib->get_pos_by_alias($sqlo, $pos_alias);
	}
	
	/**
	 * get a FREE POS by PART of ALIAS of ABS_CONT_ENTRY
	 * @param object $sqlo
	 * @param string $alias
	 * @return int
	 */
	function get_pos_by_alias_part(&$sqlo, $alias, $start_pos=0) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    if (!$this->contID) {
	        sys_error_my( 'Class not initialized.' );
	    }
	    
	    $pos  = $start_pos;
	    $last_pos =0;
	    do {
	        
	        $pos = $this->absContLib->next_pos_by_alias_part($sqlo, $alias, $pos);
	        if (!$pos)  {
	            $error->set( $FUNCNAME, 1, 'Can not get a free POS for alias-part "'.$alias.'"' );
	            return;
	        }
	        if ($last_pos==$pos) {
	            $error->set( $FUNCNAME, 1, 'Bizarre: new pos is same like last analysed pos (Pos:'.$pos.', Alias:'.$alias.')' );
	            return;
	        }
	        
	        $old_suc = $this->get_SUC_of_Pos($sqlo, $pos);
	        if (!$old_suc) {
	            $found=1;
	            break;
	        }
	        $last_pos = $pos;
	        
	        // debugOut('A:$alias start-pos:'.$pos, $FUNCNAME, 1);
 
	        
	    } while (1);
	    
	    if ($found) return $pos;
	    
	    return;
	}
	
	
	// get location of storage
	static function get_location($sqlo, $cont_id) {
	    $sqlsel = 'c.LOCATION from CHIP_READER c join CONTAINER x on x.CHIP_READER_ID=c.CHIP_READER_ID'.
	   	    ' where x.CONTAINER_ID='.$cont_id;
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    $location = $sqlo->RowData[0];
	    return $location;
	}
}

/**
 * MANY CONTAINERs  - analyse SUCs of many containers
 * @package o.CONTAINER.subs.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oCONTAINER_many {
    
    /**
     * get QUANTITY of ALL Sucs, derived from one SUA
     * @param object $sqlo
     * @param int $sua_id
     * @return float
     */
    static function get_one_SUA_QUANTITY($sqlo, $sua_id) {
        
        $subsel='select CONCRETE_SUBST_ID from CONCRETE_SUBST where ABSTRACT_SUBST_ID='.$sua_id;
        $sqlsel = "SUM(QUANTITY) from CONT_HAS_CSUBST where " . 
            ' CONCRETE_SUBST_ID in ('. $subsel.')';
           
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $quant_sum = $sqlo->RowData[0];
        return $quant_sum;
    }
    
    /**
     * get SUM of quantity of all containers
     * - optional: selection of containers
     * @param object $sqlo
     * @param int $suc_id
     * @param array $cont_ids OPTIONAL: ID of containers
     * @return float
     */
    static function get_one_SUC_QUANTITY($sqlo, $suc_id, $cont_ids=array() ) {

        $sqlsel = "SUM(QUANTITY) from CONT_HAS_CSUBST where " .
            ' CONCRETE_SUBST_ID='.$suc_id;
        if (!empty($cont_ids)) {
            $sqlsel .= ' and CONTAINER_ID in ('.implode(',',$cont_ids).')';
        }
        
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $quant_sum = $sqlo->RowData[0];
        return $quant_sum;
    }
    
    /**
     * get COUNT of SUCs in container
     * @param object $sqlo
     * @param int $suc_id
     * @return int
     */
    static function get_one_SUC_COUNT($sqlo, $suc_id,  $cont_ids=array() ) {
        
        $sqlsel = "COUNT(1) from CONT_HAS_CSUBST where CONCRETE_SUBST_ID=". $suc_id;
        if (!empty($cont_ids)) {
            $sqlsel .= ' and CONTAINER_ID in ('.implode(',',$cont_ids).')';
        }
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $quant_sum = $sqlo->RowData[0];
        return $quant_sum;
    }
    
    /**
     * geta all distinct stores of this SUC
     * @param object $sqlo
     * @param int $suc_id
     * @return array $containers
     */
    static function get_one_SUC_stores($sqlo, $suc_id) {
        $containers=array();
        $sqlsel = "distinct(CONTAINER_ID) from CONT_HAS_CSUBST where " .
            ' CONCRETE_SUBST_ID='.$suc_id. ' order by CONTAINER_ID';
        
        $sqlo->Quesel($sqlsel);
        while ($sqlo->ReadRow()) {
            $cont_id = $sqlo->RowData[0];
            $containers[]=$cont_id;
        }
        return $containers;
    }
    
   
}

