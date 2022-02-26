<?php
require_once ('o.S_VARIO.subs.inc');

/**
 * container (abstract) sub methods
 * - VARIO:coord.alias.end : e.g. "T" or 'NULL'
 * @namespace core
 * @package o.ABS_CONTAINER.subs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

class oABS_CONTAINER_subs {
    
    private $COORD_KEYS=array('S','R','B','T'); // can be overwritten by setContainer()
    
    /**
     * 
     * @var $coord_type
     *  [NULL], - if no coordiantes given
     *  EXACT,  - is STANDARD, if coordinates given
     *  BOX
     */
    private $coord_type; //  
    private $alias_preselect;
	
    function __construct($id=0) {
        $this->id = $id;
        $this->coord_type='EXACT';
    }
	/**
	 * set ABS_CONTAINER_ID
	 * - sets $this->coord_type
	 * @param object $sqlo
	 * @param object $id
	 */
	function setContainer(&$sqlo, $id) {
		$this->id = $id;
		
		$vario_lib = new oS_VARIO_sub('ABS_CONTAINER');
		
		$this->alias_preselect_str  = $vario_lib->getValByKey( $sqlo, $id, 'alias.preselect' );
		
		$coord_key_alias_start  = $vario_lib->getValByKey( $sqlo, $id, 'coord.alias.end' ); // e.g. 'T'
		if ($coord_key_alias_start!=NULL) {
		    // overwrite $COORD_KEYS
		    if ($coord_key_alias_start=='NULL') $this->COORD_KEYS = array(); // no leading key
		    else $this->COORD_KEYS = array($coord_key_alias_start); 
		}
		$this->coord_type  = $vario_lib->getValByKey( $sqlo, $id, 'coord.type' );
		
		if ($this->coord_type==NULL) {
    		if ($this->has_coordinates($sqlo)) {
    		    $this->coord_type='EXACT'; // fall back
    		}
		}
		if ($this->coord_type==NULL) $this->coord_type='NULL';  // fall back
		
		
	}
	
	function get_coord_type() {
	    return $this->coord_type;
	}
	
	/**
	 * optional get preselected values for ALIAS
	 * @param object $sqlo
	 */
	function get_alias_select_vals($sqlo) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $this->alias_preselect=array();
	    if ($this->alias_preselect_str!=NULL) {
	        // KOMMA separated alias strings
	        $this->alias_preselect= explode(',',$this->alias_preselect_str);
	        debugOut('(73) alias_preselect: length:'.sizeof($this->alias_preselect), $FUNCNAME, 1);
	    }
	    return $this->alias_preselect;
	}
	
	function has_coordinates($sqlo) {
	    $absid  = $this->id;
	    $sqlsel = 'count(1) '.
	   	    'from ABS_CONT_ENTRY where ABS_CONTAINER_ID='.$absid;
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    $cnt=$sqlo->RowData[0];
	    
	    $out=0;
	    if ($cnt) $out=1;
	    
	    return $out;
	}
	
	
	/**
	 * get container dimension
	 * @return 
	 * @param object $sqlo
	 */
	function getDimensions(&$sqlo) {
		$absid  = $this->id;
		$sqlsel = 'max(SHELF), max(RACK), max(BOX) '.
			'from ABS_CONT_ENTRY where ABS_CONTAINER_ID='.$absid;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$feat=array();
		$feat['S'] = intval($sqlo->RowData[0]);
		$feat['R'] = intval($sqlo->RowData[1]);
		$feat['B'] = intval($sqlo->RowData[2]);
		
		// because it is VARCHAR !
		$sqlsel = 'count(BOXPOS) from ABS_CONT_ENTRY where ABS_CONTAINER_ID='.$absid. ' and SHELF=1 and  RACK=1 and BOX=1';
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$feat['BP'] = intval($sqlo->RowData[0]);
		
		// TBD: BP_x, BP_y dimensions
		if ($feat['BP']>0) {
			$feat['BP_x'] = intval(sqrt($feat['BP']));
			$feat['BP_y'] = ceil( $feat['BP'] / $feat['BP_x'] );
		}
		return ($feat);
	}
	
	function getBoxPosMinMax(&$sqlo, $S, $R, $B) {
		$absid  = $this->id;
		$sqlsel = 'min(POS), max(POS) from ABS_CONT_ENTRY where ABS_CONTAINER_ID='.$absid.
			  ' and SHELF='.$S.' and  RACK='.$R.' and BOX='.$B;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$retarr=array();
		$retarr['posFirst'] = $sqlo->RowData[0];
		$retarr['posLast']  = $sqlo->RowData[1];
		return $retarr;
	}
	
	
	
	/**
	 * get POS by exact ALIAS of ABS_CONT_ENTRY
	 * @param object $sqlo
	 * @param string $alias
	 * @return int
	 */
	function get_pos_by_alias(&$sqlo, $alias) {
	    
	    if (!$this->id) {
	        sys_error_my( 'Class not initialized.' );
	    }
	    
	    $absid  = $this->id;
	    $sqlsel = 'POS from ABS_CONT_ENTRY where ABS_CONTAINER_ID='.$absid.
	       ' and ALIAS='.$sqlo->addQuotes($alias);
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    
	    return $sqlo->RowData[0];
	}
	
	
	function get_alias_sql_pattern($alias) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $alias_pattern=NULL;
	    $found_cnt=0;
	    
	    // $this->COORD_KEYS can be an empty array (for pure ALIASES)
	    foreach($this->COORD_KEYS as $key) {
	        $keypos = strpos($alias, $key);
	        if ($keypos===FALSE) {
	            // break now ...
	            $alias_pattern = $alias . $key . '%';
	            break;
	        }
	        $found_cnt++;
	    }
	    debugOut('(138) alias_pattern: '.$alias_pattern.' COORD_KEYS:'.print_r($this->COORD_KEYS,1), $FUNCNAME, 1);
	    
	    // 	    if (!$found_cnt) {
	    // 	        $error->set( $FUNCNAME, 1, 'You must at least give a start alias-pos.' );
	    // 	        return;
	    // 	    }
	    if ($alias_pattern==NULL) {
	        $alias_pattern = $alias; // take alias as EXACT alias....
	        // WHY ? $last_pos = 0; // ignore $last_pos !
	    }
	    return $alias_pattern;
	}
	/**
	 * get POS by part of ALIAS of ABS_CONT_ENTRY
	 * @param object $sqlo
	 * @param string $alias
	 * @param int $last_pos
	 * @return int
	 */
	function next_pos_by_alias_part(&$sqlo, $alias, $last_pos=0) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    if (!$this->id) {
	        sys_error_my( 'Class not initialized.' );
	    }
	    
	    $alias_pattern = $this->get_alias_sql_pattern($alias);
	    
	    $absid  = $this->id;
	    
	    $sqlsel = 'POS from ABS_CONT_ENTRY where ABS_CONTAINER_ID='.$absid.
	       ' and ALIAS like '.$sqlo->addQuotes($alias_pattern);
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    $one_pos =$sqlo->RowData[0];
	    if (!$one_pos) {
	        $error->set( $FUNCNAME, 1, 'alias-part "'.$alias_pattern.'" is unknown.' );
	        return;
	    }
	    
	    
	    $sqlsel = 'POS from ABS_CONT_ENTRY where ABS_CONTAINER_ID='.$absid.
	       ' and ALIAS like '.$sqlo->addQuotes($alias_pattern);
	    $sqlsel .= ' and POS>'.$last_pos;
	    $sqlsel .= ' order by POS';
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    $pos =$sqlo->RowData[0];

	    
	    if (!$pos) {
	        $error->set( $FUNCNAME, 2, 'No Pos found for alias-part "'.$alias_pattern.'" and POS>'.$last_pos.'.' );
	        return;
	    }
	    
	    return $pos;
	}
	
	/**
	 * get POS by ALIAS of ABS_CONT_ENTRY
	 * @param object $sqlo
	 * @param int $pos
	 * @return int
	 */
	function get_alias_by_pos(&$sqlo, $pos) {
	    
	    if (!$this->id) {
	        sys_error_my( 'Class not initialized.' );
	    }
	    
	    $absid  = $this->id;
	    $sqlsel = 'ALIAS from ABS_CONT_ENTRY where ABS_CONTAINER_ID='.$absid.
	       ' and POS='.$sqlo->addQuotes($pos);
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    
	    return $sqlo->RowData[0];
	}
	
	/**
	 * 
	 * @param object $sqlo
	 * @param int $S
	 * @param int $R
	 * @param int $B
	 * @return string
	 */
	function get_alias_by_SRB(&$sqlo, int $S, int $R, int $B) {
	    $absid  = $this->id;
	    $sqlsel = 'ALIAS from ABS_CONT_ENTRY where ABS_CONTAINER_ID='.$absid.
	    ' and SHELF='.$S.' and  RACK='.$R.' and BOX='.$B;
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    $alias = $sqlo->RowData[0];
	    
	    return $alias;
	}
	

}
