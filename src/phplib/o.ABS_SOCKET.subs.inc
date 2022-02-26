<?php
require_once('o.S_VARIO.subs.inc');
require_once ("o.S_OBJLINK.subs.inc");
require_once ('o.H_SOCK_TYPE.subs.inc');
require_once ('f.wiid_roid.inc');

/**
 * helper methods for ABS_SOCKET:Blink specific
 * @package o.ABS_SOCKET.subs.inc
 * @swreq UREQ:
 * @author  Steffen Kube (steffen@blink-dx.com)
 * 
 * @var array cart_content_arr_STRUCT - content of cartridge -- structure change on 2021-03-12
 *   array[$pos] = 
*       array(
*          'step' => step NR
*          'pali' : position alias, e.g. A1
*          'vol'  => volume (only for REAG)
*          'sua'=>ID, : ID of substance (abstract)
*          'sua.nice'=> string, 
*          --- from SOCKET ---
*          'suc'=>ID , : ID of substance (concrete)
*          'suc.nice'  : string,
*          'grp'       : int WellGroup
*          
*          'xelem'     : other optional elements  e.g. ADAPT, MISC
*               array { 
*                 'step' => step NR, 'key':ADAPT, 'sua'=>ID,  'sua.nice'=> string,  
*               }
*        )
*          
 * @var array cart_content_arr_FULL_STRUCT - content of cartridge with REAG, MISC and so on
 *   array[$pos] = 
 *      array( $KEY =>    // e.g. REAG, ADAPT, MISC
 *          array(
 *              'step' => step NR
 *              'vol'  => volume (only for REAG)
 *              'sua'=>ID, : ID of substance (abstract)
 *              'sua.nice'=> string, 
 *              'suc'=>ID , : ID of substance (concrete)
 *              'suc.nice'=> string
 *              )
 *         )
 
 */
class oABS_SOCKET_subs {
	
    private $ABS_SOCKET_feat; // some main features of ABS_SOCKET: NAME, NOTES; ...
    private $H_SOCK_TYPE_ID;
    private $H_SOCK_TYPE_data;
	private $absSoc;
	private $absSubstID;
	
	/**
	 * get ABS_SOCKET_ID from SUA
	 * @param object $sqlo
	 * @param int $sua_id
	 * @return int
	 */
	static function get_ASOC_from_SUA(&$sqlo, $sua_id) {
	    //global $error;
	    //$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $key = 'ERPchild';
	    $objLink = new oS_OBJLINK_subs();
	    $child_objs = $objLink->getChiObjByKey($sqlo, 'ABSTRACT_SUBST', $sua_id, $key);
	    if (!is_array($child_objs)) {
	        return 0;
	    }
	    $child_obj = $child_objs[0];
	    if ($child_obj[0]=="ABS_SOCKET" and $child_obj[1]!=NULL ) {
	        $abs_socket_id = $child_obj[1];
	        return $abs_socket_id;
	    } else {
	        return 0;
	    }
	}
	
	/**
	 * init object
	 * @param object $sqlo
	 * @param object $objid
	 * @return array
	 *  'H_SOCK_TYPE_key'
	 *  'H_SOCK_TYPE_ID'
	 */
	function init(&$sqlo, int $objid) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		if(!$objid) throw new Exception('input objid missing.');
		
		$this->H_SOCK_TYPE_data = array();
		$this->MAX_ENTRIES = 0;
		$this->absSoc = $objid;
		$this->H_SOCK_TYPE_key = NULL;
		//get absSubstID by Object_Link from absSoc
		$key = 'ERPchild';
		$objLink = new oS_OBJLINK_subs();
		$absSubst = $objLink->getMoObjbyKey($sqlo, 'ABS_SOCKET', $objid, $key);
		if ($absSubst[0]=="ABSTRACT_SUBST" and $absSubst[1]!=NULL ) $absSubstID = $absSubst[1];
		else {
			//$error->set( $FUNCNAME, 1, 'Abstraktes Socket [SKA-ID:'.$this->absSoc.']'.
			//		' ist kein Kind einer Abstrakten Substanz welches per Objekt Link mit KEY:"'.$key.'" verbunden ist.');
			//return;
		}
	
		$this->absSubstID = $absSubstID;
		
		// get Socket-Type H_SOCK_TYPE_ID
		$abs_data = glob_elemDataGet3( $sqlo, 'ABS_SOCKET', array('ABS_SOCKET_ID'=>$objid), array('H_SOCK_TYPE_ID', 'NAME', 'NOTES') );
		$this->ABS_SOCKET_feat = $abs_data;
		$this->H_SOCK_TYPE_ID  = $abs_data['H_SOCK_TYPE_ID'];
		if (!$this->H_SOCK_TYPE_ID) {
		    $error->set( $FUNCNAME, 1, 'ABS_SOCKET (ID:'.$this->absSoc.') missing H_SOCK_TYPE_ID.');
		    return;
		}
		    
		$sock_type_lib = new oH_SOCK_TYPE_subs();
		$this->H_SOCK_TYPE_key  = $sock_type_lib->get_type($sqlo, $this->H_SOCK_TYPE_ID);
		$this->H_SOCK_TYPE_data = $sock_type_lib->features_by_id($sqlo, $this->H_SOCK_TYPE_ID );
		$this->MAX_ENTRIES = $this->H_SOCK_TYPE_data['CART_MAX_POS'];

		debugOut('THIS:'.print_r($this,1), $FUNCNAME, 2);
		
		return array( 'H_SOCK_TYPE_key'=>$this->H_SOCK_TYPE_key, 'H_SOCK_TYPE_ID'=>$this->H_SOCK_TYPE_ID );
	}
	
	/**
	 * get socket type KEY froma STATIC method
	 * @param object $sqlo
	 * @param int $abs_sock_id
	 * @return int or NULL
	 */
	static function get_socket_type_key_STAT(object $sqlo, int $abs_sock_id) {
	    $H_SOCK_TYPE_ID = glob_elementDataGet( $sqlo, 'ABS_SOCKET', 'ABS_SOCKET_ID', $abs_sock_id, 'H_SOCK_TYPE_ID' );
	    if(!$H_SOCK_TYPE_ID) return NULL;
	    $socket_type = oH_SOCK_TYPE_subs::get_type($sqlo, $H_SOCK_TYPE_ID);
	    return $socket_type;
	}
	
	function get_socket_type_key() {
	    return $this->H_SOCK_TYPE_key;
	}
	function get_H_SOCK_TYPE_data() {
	    return $this->H_SOCK_TYPE_data;
	}
	function get_MAX_ENTRIES() {
	    return $this->MAX_ENTRIES;
	}
	
	// for MTP: POS => x(start0), y(start0), (x,y start with 0)
	private function _mtp_pos2XY($pos, $xcnt, $ycnt) {
	    $pos=$pos-1;
	    $y = floor($pos/$xcnt);
	    $x = $pos - $y*$xcnt;
	    return array('x'=>$x, 'y'=>$y);
	}
	// for MTP: POS => x(start0), y(start0),
	private static function _mtp_XY2pos($x, $y, $xcnt, $ycnt) {
	    if ($y>=$ycnt) {
	        throw new Exception('Y ('.$y.') too big (max:'.$ycnt.').');
	    }
	    if ($x>=$xcnt) {
	        throw new Exception('X ('.$x.') too big (max:'.$xcnt.').');
	    }
	    $pos = $xcnt * $y + $x + 1;
	    return $pos;
	}
	
	// Layout: WELL-POS => POS-ALIAS; e.g. 13 => B01
	function wellPos2Alias(int $pos) {
	    $out=$pos;
	    switch($this->H_SOCK_TYPE_key) {
	        case '07': // MTP
	            $xcnt = $this->H_SOCK_TYPE_data['xcnt'];
	            $ycnt = $this->H_SOCK_TYPE_data['ycnt'];
	            if (!$xcnt or !$ycnt) {
	                break; // input data missing
	            }
	            $arr  = $this->_mtp_pos2XY($pos, $xcnt, $ycnt);
	            
	            $xtxt = $arr['x']+1;
	            // 0 padding for TWO digits ! forever A01, never A1
	            // need TWO DIGITS for KEYENCE instrument position system ...
	            $xtxt = str_pad( $xtxt, 2, "0", STR_PAD_LEFT ); 
	            $out  = chr(65+$arr['y']) . $xtxt;
	            break;
	    }
	    
	    // debugOut("(146) pos:$pos xcnt:$xcnt ycnt:$ycnt out:$out xtxt:$xtxt arr:".print_r($arr,1), $FUNCNAME, 2);
	    
	    return $out;
	}
	
	// Layout: POS-ALIAS => WELL-POS : e.g. B1 => 13 
	function wellAlias2Pos(string $alias) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    $out=0;
	    
	    switch($this->H_SOCK_TYPE_key) {
	        case '07': // MTP
	            
	            $xcnt = $this->H_SOCK_TYPE_data['xcnt'];
	            $ycnt = $this->H_SOCK_TYPE_data['ycnt'];
	            if (!$xcnt or !$ycnt) {
	                break; // input data missing
	            }
	            
	            $xtxt = substr($alias,0,1);
	            $ytxt = substr($alias,1);
	            
	            $y  = ord($xtxt) - ord('A');
	            $x  = intval($ytxt)-1;
	            
	            $out = self::_mtp_XY2pos($x, $y, $xcnt, $ycnt);

	            break;
	    }
	    
	    debugOut("(146) alias:$alias xcnt:$xcnt ycnt:$ycnt xtxt:$xtxt XY:$x:$y out:".$out, $FUNCNAME, 2);
	    
	    return $out;
	}
	
	/**
	 * check, if ABS_SOCKET has the right component PROTOCOL
	 */
	function check_socket_PROTO(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		if (!$this->absSoc) {
			sys_error_my( 'Class not initialized.' );
		}
		if ( !$this->absSubstID ) { 
			$error->set( $FUNCNAME, 1, 'ABS_SOCKET (ID:'.$this->absSoc.') has no SUA.');
			return;
		}
		
		$pra_id = glob_elementDataGet( $sqlo, 'ABSTRACT_SUBST', 'ABSTRACT_SUBST_ID', $this->absSubstID, 'ABSTRACT_PROTO_ID');
		$step_nr = $this->reag_step_of_parent_pra($sqlo, $pra_id);
		
		if (!$step_nr) {
			$error->set( $FUNCNAME, 2, 'ABS_SOCKET (ID:'.$this->absSoc.') > SUA (ID:'.$this->absSubstID.
					') has no Prep-Prot with Step-Name "REAGENT_SET"');
			return;
		}
	}
	
	function get_SUA() {
		if (!$this->absSoc) {
			sys_error_my( 'Class not initialized.' );
		}
		return $this->absSubstID;
	}
	
	/**
	 * get ExpiryDays of abs_socket
	 * @param object $sqlo
	 * @return float
	 */
	function getExpiryDays(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		if (!$this->absSoc) {
			sys_error_my( 'Class not initialized.' );
		}
		
		if (!$this->absSubstID) {
			$error->set( $FUNCNAME, 1, 'ABS_SOCKET (ID:'.$this->absSoc.') has no related SUA!');
			return;
		}
		
		$ExpiryDays = glob_elementDataGet( $sqlo, 'ABSTRACT_SUBST', 'ABSTRACT_SUBST_ID', $this->absSubstID, 'EXPIRY_DAYS'); 

		return $ExpiryDays;
	}
	
	/**
	 * calc EXPIRY-date by $PROD_DATE and $ExpiryDays (of SUA)
	 * @param object $sqlo
	 * @param string $PROD_DATE : HUMAN format
	 * @return string $expiryDate
	 */
	function calc_expiry_date(&$sqlo, $PROD_DATE) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    if (!$this->absSoc) {
	        sys_error_my( 'Class not initialized.' );
	    }
	    
	    $ExpiryDays = $this->getExpiryDays($sqlo);
	    if ($ExpiryDays<=0) {
	        $error->set( $FUNCNAME, 5, 'SUA (ID:'.$this->absSubstID.'): EXPIRY_DAYS missing' );
	        return;
	    }
	    
	    $stableTime   =  date_unixDay2Sec($ExpiryDays);
	    $prodDate_UNX = strtotime($PROD_DATE);
	    $expiryDate = date_unix2datestr( $prodDate_UNX + $stableTime, 3);
	    
	    return $expiryDate;
	}
	
	/**
	 * get BOM of ABS_SOCKET > ABSTRACT_SUBST
	 * @param object $sqlo
	 * @return $value
	 */
	function get_BOM(&$sqlo) {
		if (!$this->absSoc) {
			sys_error_my( 'Class not initialized.' );
		}
		
		$pra_id = glob_elementDataGet( $sqlo, 'ABSTRACT_SUBST', 'ABSTRACT_SUBST_ID', $this->absSubstID, 'ABSTRACT_PROTO_ID');
		return $pra_id;
	}
	
	
	/**
	 * get SUAs of BOM
	 * @param object $sqlo
	 * @return array of array
	 *  'pos' =>
	 *  'sua' => SUA_ID
	 */
	function get_SUAs(&$sqlo) { 
	    
	    if (!$this->absSoc) {
	        sys_error_my( 'Class not initialized.' );
	    }
	
	    $cart_content_arr=array();
	    
	    $sqlsel = "* from ABS_CART_H_SUA where ABS_SOCKET_ID=".$this->absSoc . ' order by POS';
	    $sqlo->Quesel($sqlsel);
	    while ( $sqlo->ReadArray() ) {
	        
	        $pos    = $sqlo->RowData['POS'];
	        $sua_id = $sqlo->RowData['ABSTRACT_SUBST_ID'];
	        $cart_content_arr[]=array('pos'=>$pos, 'sua'=>$sua_id);
	    }
	    return $cart_content_arr;
	}
	
	/**
	 * get data from ABS_CART_H_SUA
	 * TBD: the array should be filled with all POS-entries, even if empty content (for JSON-exports !!!)
	 * TBD: currently NON-'REAG' keys are not handled
	 * @param object $sqlo
	 * @return array  cart_content_arr_STRUCT
	 */
	function get_cart_content_arr(&$sqlo) {
	    if (!$this->absSoc) {
	        sys_error_my( 'Class not initialized.' );
	    }
	    
	    $cart_content_arr=array();
	    if ($this->MAX_ENTRIES) {
	        // create pre filled array
	        for ($pos=1; $pos<=$this->MAX_ENTRIES; $pos++) {
	            // pali ...
	            $pali   = $this->wellPos2Alias($pos);
	            $cart_content_arr[$pos]=array('pali'=>$pali);
	        }
	    }
	    
	    $DEF_KEY='REAG';
	    
	    $sqlsel = "* from ABS_CART_H_SUA where ABS_SOCKET_ID=".$this->absSoc . ' and  (KEY='.$sqlo->AddQuotes($DEF_KEY).' or KEY is NULL) order by POS';
	    $sqlo->Quesel($sqlsel);
	    while ( $sqlo->ReadArray() ) {
	        
	        $step   = $sqlo->RowData['STEP'];
	        $pos    = $sqlo->RowData['POS'];
	        $sua_id = $sqlo->RowData['ABSTRACT_SUBST_ID'];
	        $vol    = $sqlo->RowData['VOLUME'];
	        // $key    = $sqlo->RowData['KEY'];

	        if ( !is_array($cart_content_arr[$pos]) ) $cart_content_arr[$pos]=array();
	        $cart_content_arr[$pos]= array_merge($cart_content_arr[$pos], array('step'=>$step, 'sua'=>$sua_id, 'vol'=>$vol) );
	    }
	    
	    foreach($cart_content_arr as $pos=>$one_well) {
            if ($one_well['sua']) {
                $nicename = obj_nice_name ( $sqlo, 'ABSTRACT_SUBST', $one_well['sua'] );
                $cart_content_arr[$pos]['sua.nice']=$nicename;
            } 
	    }
	    
	    return $cart_content_arr;
	}
	
	/**
	 * get data from ABS_CART_H_SUA in FULL-Format
	 * @param object $sqlo
	 * @return array  cart_content_arr_FULL_STRUCT
	 */
	function get_content_arr_FULL(&$sqlo) {
	    if (!$this->absSoc) {
	        sys_error_my( 'Class not initialized.' );
	    }
	    
	    $cart_content_arr=array();
	    if ($this->MAX_ENTRIES) {
	        // create pre filled array
	        for ($i=0; $i<=$this->MAX_ENTRIES; $i++) {
	            $cart_content_arr[$i]=array();
	        }
	    }
	    
	    $sqlsel = "* from ABS_CART_H_SUA where ABS_SOCKET_ID=".$this->absSoc . ' order by POS';
	    $sqlo->Quesel($sqlsel);
	    while ( $sqlo->ReadArray() ) {
	        $step   = $sqlo->RowData['STEP'];
	        $pos    = $sqlo->RowData['POS'];
	        $sua_id = $sqlo->RowData['ABSTRACT_SUBST_ID'];
	        $key    = $sqlo->RowData['KEY'];
	        $vol    = $sqlo->RowData['VOLUME'];
	        $cart_content_arr[$pos][$key]=array('step'=>$step, 'sua'=>$sua_id, 'vol'=>$vol);
	    }
	    
	    foreach($cart_content_arr as $pos=>$one_well_key) {
	        foreach($one_well_key as $key=>$one_well) {
	            if ($one_well['sua']) {
	                $nicename = obj_nice_name ( $sqlo, 'ABSTRACT_SUBST', $one_well['sua'] );
	                $cart_content_arr[$pos][$key]['sua.nice']=$nicename;
	            }
	        }
	    }
	    
	    return $cart_content_arr;
	}
	
	/**
	 * get number of components
	 * @param object $sqlo
	 * @return int
	 */
	function cnt_components($sqlo) {
	    if (!$this->absSoc) {
	        sys_error_my( 'Class not initialized.' );
	    }
	    
	    $sqlsel = "count(1) from ABS_CART_H_SUA where ABS_SOCKET_ID=".$this->absSoc;
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    $cnt   = $sqlo->RowData[0];
	    return $cnt;
	}
	
	/**
	 * get full format for external applications
	 * @param object $sqlo
	 * @return array ABS_SOCKET_json_STRUCT
	 *  'object': {'id': ID of object, 'wiid': database identifier, 't': table key name }
	 *  'fea': { features
	 *    'NAME': name of cart_tmpl
	 *    'H_SOCK_TYPE_name': '04' (type of cart)
	 *  }
	    'chambers' : [
    	  0 : { 
    	   'vol_m': max basis volume [ml] 
    	   'vol' : volume of used sua in [ml]
    	   'reag_id'   => ID of material template
    	   'reag_name' => nice name of sua
    	   }
    	  1 : {...} 
	      ...
	    ]
	 */
	function get_cart_JSON(&$sqlo) {
	    
	    if (!$this->absSoc) {
	        sys_error_my( 'Class not initialized.' );
	    }
	    
	    
	    $out_arr = array();

	    $wiidLib = new fWiidRoidC();
	    $wiid_roid = $wiidLib->getWiroiByID( $sqlo, 'ABS_SOCKET', $this->absSoc );
	    if (!is_array($wiid_roid)) {
	        $wiid_roid[0] = $_SESSION['globals']['magasin_serial'];
	        $wiid_roid[1] = $this->absSoc;
	    }
	    
	    $objinfo= array( 'id'=>$wiid_roid[1], 'wiid'=>$wiid_roid[0] );
	    $out_arr['object'] = $objinfo;
	    
	    $feature_arr = array(
	        'NAME'=> $this->ABS_SOCKET_feat['NAME'],
	        'H_SOCK_TYPE_name'=> $this->H_SOCK_TYPE_key 
	    );
	    $out_arr['fea']    = $feature_arr;
	   
	    $chamb_arr = $this->get_content_arr_FULL($sqlo);
	    $H_SOCK_TYPE_data = &$this->H_SOCK_TYPE_data;
	    
	    $use_KEY='REAG';
	    $chambs_of_type = $H_SOCK_TYPE_data['chambers'];
	    $chambers_all=array();
	    foreach($chambs_of_type as $pos=>$def_row) {
	        
	        $t_chamb = &$chamb_arr[$pos][$use_KEY];
	        
	        $one_chamb=array();
	        if ($t_chamb['sua'])              $one_chamb['reag_id']   = $t_chamb['sua'];
	        if ($t_chamb['sua.nice']!='')     $one_chamb['reag_name'] = $t_chamb['sua.nice'];
	        if (is_numeric($t_chamb['vol']) ) $one_chamb['vol']       = floatval($t_chamb['vol']);
	        $one_chamb['vol_m'] = $def_row['vol_m'];
	        
	        $chambers_all[] = $one_chamb;
	    }
	    
	    $out_arr['chambers'] = $chambers_all;
	    
	    return $out_arr;
	    
	}
	
}