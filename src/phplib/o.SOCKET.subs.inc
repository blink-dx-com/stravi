<?php
require_once('o.S_VARIO.subs.inc');
require_once ("o.S_OBJLINK.subs.inc");
require_once ('o.ABS_SOCKET.subs.inc');

/**
 * helper methods for SOCKET:Blink specific
 * @package o.SOCKET.subs.inc
 * @swreq UREQ:
 * @author  Steffen Kube (steffen@blink-dx.com)
 * 
 * @var array cart_content_simple_arr_STRUCT - SIMPLE content of cartridge
 *   array[$pos] = 
*          array(
*              'step' => step NR
*              'key'  => KEY
*              'vol'  => volume (only for REAG)
*              'suc'  => ID , : ID of substance (concrete)
*              'grp'  =>
*              )
 */
class oSOCKET_subs {
	
	
	private $objid;
	
	function __construct() {
	    $this->absCartLib = new oABS_SOCKET_subs();
	}
	
	
	/**
	 * init object
	 * @param object $sqlo
	 * @param int $objid
	 * @return array
	 *    'ABS_SOCKET_ID'
	 */
	function init(&$sqlo, $objid) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->objid = $objid;
		$cart_data = glob_elemDataGet3( $sqlo, 'SOCKET', array('SOCKET_ID'=>$objid), array('ABS_SOCKET_ID') );
		$this->ABS_SOCKET_ID = $cart_data['ABS_SOCKET_ID'];
		if (!$this->ABS_SOCKET_ID) {
		    $error->set( $FUNCNAME, 1, 'no ABS_SOCKET_ID on cartridge!' );
		    return;
		}
		
		$ABS_SOCKET_data = $this->absCartLib->init($sqlo, $this->ABS_SOCKET_ID);
		
		return array('ABS_SOCKET_ID'=>$this->ABS_SOCKET_ID, 'H_SOCK_TYPE_ID'=>$ABS_SOCKET_data['H_SOCK_TYPE_ID'] );
	}

	function get_socket_type_key() {
	    if (!$this->objid) {
	        sys_error_my( 'Class not initialized.' );
	    }
	    
	    return $this->absCartLib->get_socket_type_key();
	}
	
	// count components of ABS_SOCKET
	function abs_cnt_components($sqlo) {
	    if (!$this->objid) {
	        sys_error_my( 'Class not initialized.' );
	    }
	    return $this->absCartLib->cnt_components($sqlo);
	}
	
	/**
	 * get data from ABS_CART_H_SUA and CART_H_SUC
	 * WARNING: this structure is NOT  because this structure is JSON prove!
	 * @param object $sqlo
	 * @return array  o.ABS_SOCKET.subs.inc:cart_content_arr_STRUCT - structure change on 2021-03-12
	 */
	function get_cart_content_arr(&$sqlo) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    if (!$this->objid) {
	        sys_error_my( 'Class not initialized.' );
	    }

	    /**
	     * get  cart_content_arr_STRUCT
	     */
	    $cart_content_arr = $this->absCartLib->get_cart_content_arr($sqlo);
	    if ( $_SESSION["userGlob"]["g.debugLevel"]>=2 ) {
	        debugOut('(90) cart_content_arr:'.print_r($cart_content_arr,1), $FUNCNAME, 2);
	    }
	    
	    $DEF_KEY='REAG';
	    
	    // now concrete data
	    $sqlsel = "* from CART_H_SUC where SOCKET_ID=".$this->objid . ' and  (KEY='.$sqlo->AddQuotes($DEF_KEY).' or KEY is NULL) order by POS';
	    $sqlo->Quesel($sqlsel);
	    while ( $sqlo->ReadArray() ) {
	        
	        $step   = $sqlo->RowData['STEP'];
	        $pos    = $sqlo->RowData['POS'];
	        $suc_id = $sqlo->RowData['CONCRETE_SUBST_ID'];
	        $key    = $sqlo->RowData['KEY'];
	        $vol    = $sqlo->RowData['VOLUME'];
	        $grp    = $sqlo->RowData['GRP'];
	        
	        // echo "DDDX",print_r($sqlo->RowData,1)."<br>";

	        $cart_content_arr[$pos]['suc'] = $suc_id;
	        if ($grp) {
	            $cart_content_arr[$pos]['grp'] = $grp;
	        }
	        
	        if ($vol!==NULL) $cart_content_arr[$pos]['vol'] = $vol;
	        if ($key!=NULL) {
	            $cart_content_arr[$pos]['key_suc']=$key; // @todo do we need that ???
	        }
	    }
	    
	    foreach($cart_content_arr as $pos=>$one_well) {
            if ($one_well['suc']) {
                $nicename = obj_nice_name ( $sqlo, 'CONCRETE_SUBST', $one_well['suc'] );
                $cart_content_arr[$pos]['suc.nice']=$nicename;
            }
	    }
	    
	    if ( $_SESSION["userGlob"]["g.debugLevel"]>=2 ) {       
	        debugOut('(128) cart_content_arr:'.print_r($cart_content_arr,1), $FUNCNAME, 2);
	    }
	    
	    return $cart_content_arr;
	}
	
	/**
	 * get data (as LIST) from ABS_CART_H_SUA and CART_H_SUC
	 * - use this method for EXTERNAL tools, because this structure is JSON prove
	 * - IMPORTANT: suc.nice becomes suc_nice !!!
	 * @param object $sqlo
	 * @return array  cart_content_list_STRUCT
	 *    [] => array('pos'=>POS, + cart_content_arr_STRUCT:ROW )
	 */
	function get_content_list(&$sqlo) {
	    $xarr = $this->get_cart_content_arr($sqlo);
	    $out=array();
	    foreach($xarr as $pos=>$row) {
	        
	        // transform suc.nice, sua.nice
	        if (array_key_exists('suc.nice', $row)) {
	            $row['suc_nice'] = $row['suc.nice'];
	            unset($row['suc.nice']);
	        }
	        if (array_key_exists('sua.nice', $row)) {
	            $row['sua_nice'] = $row['sua.nice'];
	            unset($row['sua.nice']);
	        }
	        
	        $row['pos'] = $pos;
	        $out[] = $row;
	    }
	    return $out;
	}
	
	
	/**
	 * get SIMPLE data from ABS_CART_H_SUA and CART_H_SUC
	 * @param object $sqlo
	 * @return array  cart_content_simple_arr_STRUCT
	 */
	static function cart_content_arr_simple_STAT(&$sqlo, $objid) {
	    // TBD: alias !!!
	    $cart_content_arr=array();
	    $sqlsel = "* from CART_H_SUC where SOCKET_ID=".$objid . ' order by POS';
	    $sqlo->Quesel($sqlsel);
	    while ( $sqlo->ReadArray() ) {
	        $pos    = $sqlo->RowData['POS'];
	        $cart_content_arr[$pos] = array(
	            'step' => $sqlo->RowData['STEP'],
	            'key'  => $sqlo->RowData['KEY'],
	            'suc'  => $sqlo->RowData['CONCRETE_SUBST_ID'],
	            'grp'  => $sqlo->RowData['GRP']
	        );
	    }
	    return $cart_content_arr;
	}
	// @see cart_content_arr_simple_STAT()
	function &get_cart_content_arr_simple(&$sqlo) {
	    if (!$this->objid) {
	        sys_error_my( 'Class not initialized.' );
	    }
	    return self::cart_content_arr_simple_STAT($sqlo, $this->objid);
	}
	
	/**
	 * get UNIQUE data of GRP in CART_H_SUC
	 * @param object $sqlo
	 * @return array of  array(
            'suc'  => int
            'grp'  => int
            'cnt' => num of entries
        );
	 */
	function get_unique_GRP_list(&$sqlo) {
	    if (!$this->objid) {
	        sys_error_my( 'Class not initialized.' );
	    }

	    $out_tmp=array();
	    $sqlsel = "* from CART_H_SUC where SOCKET_ID=".$this->objid . ' and GRP>0 order by GRP';
	    $sqlo->Quesel($sqlsel);
	    while ( $sqlo->ReadArray() ) {
	        
	        $grp    = $sqlo->RowData['GRP'];
	        if (empty($out_tmp[$grp])) {
    	        $out_tmp[$grp] = array(
    	            'suc'  => $sqlo->RowData['CONCRETE_SUBST_ID'],
    	            'grp'  => $grp,
    	            'cnt'  => 1
    	        );
	        } else {
	            $out_tmp[$grp]['cnt'] =  $out_tmp[$grp]['cnt'] + 1;
	        }
	    }
	    $output = array_values($out_tmp);
	    
	    return $output;
	}
	
	static function SUC_is_part_of_cart($sqlo, $suc_id) {
	    // now concrete data
	    // cid
	    
	    $output=array('cnt'=>0);
	    
	    $sqlsel = "unique(SOCKET_ID) from CART_H_SUC where CONCRETE_SUBST_ID=".$suc_id;
	    $sqlo->Quesel($sqlsel);
	    if ($sqlo->ReadRow()) {
	        $output['cnt']=1;
	        $output['cid']=$sqlo->RowData[0];
	    }
	    if ($sqlo->ReadRow()) {
	        $output['cnt']=2;
	    }

	    return $output;
	}
	
}