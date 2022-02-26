<?php
require_once ("insertx.inc");
require_once ("f.assocUpdate.inc");

/**
 * modification methods for BCBATCH
 * @package o.BCBATCH.mod.inc
 * @author  Steffen Kube
 * $Header:  $
 */
class oBCBATCH_mod {
	
	private $_isInit=0;
	private $MAX_CARTS = 50000;
	
	function __construct(&$sqlo, $batchid=NULL) {
	    if ($batchid) $this->set_batch($sqlo, $batchid);
	}
	
	function get_MAX_CARTS() {
	    return $this->MAX_CARTS;
	}
	
	function set_batch(&$sqlo, $batchid) {
		$this->batchid = $batchid;
		$this->_isInit=1;
	}
	
	private function _access_create( &$sqlo ) {
	    $access_id = insert_access( $sqlo, 'SOCKET');
		return ($access_id);
	}
	
	function getStartPos( &$sqlo ) {
		if (!$this->_isInit) {
			sys_error_my( 'class not initialized.' );
		}
		$batchid = $this->batchid;
		$sqlsel = "max(POS) from BCBATCH_H_SOC where BCBATCH_ID=".$batchid;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$maxpos = $sqlo->RowData[0];
		if (!$maxpos) $maxpos = 0;
		$start_pos = $maxpos + 1;
		
		return $start_pos;
	}
	
	private function crea_one_socket(&$sqlo, &$assoclib, $pos, $ini_args) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $batchid = $this->batchid;
	    $tablename = 'SOCKET';
	    
	    $NUM_DIGITS_PER_BATCH   = 3; // changed on 2019-08-16 (from 4 to 2)
	    $NUM_DIGITS_BATCHID_FILL= 4; // changed on 2019-08-16 (from 5 to 4)
	    
	    $pos_str = str_pad($pos, $NUM_DIGITS_PER_BATCH, "0", STR_PAD_LEFT );
// 	    if (strlen($pos_str) > $NUM_DIGITS_PER_BATCH) {
// 	        $error->set($FUNCNAME, 2, "POS too big: requirement: number_of_digits(POS) <=".$NUM_DIGITS_PER_BATCH." digits.\n");
// 	        return;
// 	    }
	    
	    # $sName = str_pad($batchid, $NUM_DIGITS_BATCHID_FILL, "0", STR_PAD_LEFT ).str_pad($pos, $NUM_DIGITS_PER_BATCH, "0", STR_PAD_LEFT );
	    // change name-policy: on 2020-01-16 : "BBBB-PPP"
	    $sName = str_pad($batchid, $NUM_DIGITS_BATCHID_FILL, "0", STR_PAD_LEFT ).'-'.str_pad($pos, $NUM_DIGITS_PER_BATCH, "0", STR_PAD_LEFT );
	    
	    //... create sockets
	    $arguSock = $ini_args;
	    $arguSock['BARCODE']       = $sName;
	
	    $opt=NULL;
	    $socket_id = insert_row_s( $sqlo, $tablename, $arguSock, $opt );
	    if ( !$socket_id ) {
	        $error->set($FUNCNAME, 3, "Creation of cartridge '".$sName."' failed\n");
	        return;
	    }
	    
	    //... connect sockets
	    $argu				= NULL;
	    $initialQuality		= 1;
	    $argu['POS'] 	    = $pos;
	    $argu['SOCKET_ID']  = $socket_id;
	    $argu['QUALITY']	= $initialQuality;
	    
	    $socket_id 			= $assoclib->insert( $sqlo, $argu, $opt );
	    
	    return array('name'=>$sName);
	}
	
	/**
	 * create sockets
	 * @param object $sqlo
	 * @param int $num
	 * @param int $absSocID
	 * @param array $options
	 *   'GUI_output' => [0],1
	 */
	function createSockets(&$sqlo, $num, $absSocID, $options=NULL) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		
		
		if (!$this->_isInit) {
			sys_error_my( 'class not initialized.' );
		}
	
		
		$batchid = $this->batchid;
	
	
		$assoctable = 'BCBATCH_H_SOC';
		$opt = NULL;
		$assoclib   = new fAssocUpdate();
		$assoclib->setObj( $sqlo, $assoctable, $batchid, $opt );
		if ($error->Got(READONLY)) return;
	
		//$sqlo->setAutoCommit(false); //TBD: check if necessary???
			
		$batch_access_id = $this->_access_create( $sqlo );
		if (!$batch_access_id) {
			$error->set($FUNCNAME, 1, "Creation of cartridge-access entry failed\n");
			return;
		}
		
		// add access rights ???
		
		
		if ($options['GUI_output']>0) {
			ob_implicit_flush(TRUE);
			ob_end_flush ( );
		}
		
		$pos = $this->getStartPos($sqlo);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 2, 'Error on Preparation.' );
			return;
		}
		
		$max_pos_future = $pos + $num; // 1 + 1 = 2
		if ( $max_pos_future > $this->MAX_CARTS ) {
		    $error->set($FUNCNAME, 3, "Number of new carts not allowed. Max carts allowed: ".$this->MAX_CARTS."\n");
		    return;
		}
		
		//... create sockets
		$ini_args = NULL;
		$ini_args['CCT_ACCESS_ID'] = $batch_access_id;
		$ini_args['ABS_SOCKET_ID'] = $absSocID;
		
		$cnt = 0;
		while ( $cnt < $num ) {
			
			// $sqlo->setAutoCommit(false);
			
		    $sock_answer = $this->crea_one_socket($sqlo, $assoclib, $pos, $ini_args);
		    $sName = $sock_answer['name'];
		    if ($error->Got(READONLY))  {
		        $error->set( $FUNCNAME, 4, 'Error got on POS:'.$pos.'.' );
		        return;
		    }
		    
			if ($options['GUI_output']>0) {
				if ($cnt<20) echo ($cnt+1)." ".$sName."</br>\n";
				if ( fmod($cnt,50) == 0 ) { 
					echo '.';
					if ( fmod($cnt,50*50) == 0 ) echo "<br>\n";
					while (@ob_end_flush()); // send all buffered output
				}
			}
			
			$cnt++;
			$pos++;
		}
		
		if ($options['GUI_output']>0) {
			echo "<br>Last cartridge: ".($cnt)." ".$sName."</br>\n";
		}
		
	}
	
	/**
	 * update entries of BCBATCH_H_SOC
	 * @param object $sqlo
	 * @param array $entries_args = array of array:
	 *   POS
	 *   NOTES => JSON-string; contains encoded dict:
	 *      user 
            date 
            notes
	 *   QUALITY
	 *   NOT allowed: SOCKET_ID
	 */
	public function update_H_SOC(&$sqlo, &$entries_args) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $allow_cols = array(
	        'QUALITY',
	        'NOTES'
	    );
	    
	    if (!$this->_isInit) {
	        sys_error_my( 'class not initialized.' );
	    }
	    
	    $batchid    = $this->batchid;
	    $assoctable = 'BCBATCH_H_SOC';
	    $opt = NULL;
	    $assoclib   = new fAssocUpdate();
	    $assoclib->setObj( $sqlo, $assoctable, $batchid, $opt );
	    if ($error->Got(READONLY)) return;
	    
	    $err_cache = NULL;
	    
	    foreach($entries_args as $one_entry) {
	        
	        $pos = $one_entry['POS']; 
	        if (!$pos) {
	            $err_cache='Pos missing.';
	            continue;
	        }
	        
	        $argu = array();
	        foreach($allow_cols as $col) {
	            // set only data, if COLUMN is given ...
	            if (array_key_exists($col, $one_entry))
	               $argu[$col] = $one_entry[$col];
	        }
	        
	        $idarr = array('POS'=>$pos);
	      
	        $assoclib->update( $sqlo, $argu, $idarr );
	        if ($error->Got(READONLY))  {
	            $errLast   = $error->getLast();
	            $error_txt = $errLast->text;
	            // $error_id  = $errLast->id;
	            $err_cache='Pos: '.$pos.'; Err:'.$error_txt;
	            $error->reset();
	            continue;
	        }
	        
	    }
	    
	    if ($err_cache) {
	        $error->set( $FUNCNAME, 1, 'Errors occured: '.$err_cache );
	        return;
	    }
	}
	
	/**
	 * get all features of one BCBATCH_H_SOC entry
	 * @param object $sqlo
	 * @param int $pos
	 * @return array
	 */
	public function get_H_SOC_entry(&$sqlo, $pos) {
	    
	    $cols   = columns_get2 ( 'BCBATCH_H_SOC' );
	    $result = glob_elemDataGet3( $sqlo, 'BCBATCH_H_SOC', 
	        array('BCBATCH_ID'=>$this->batchid, 'POS'=>$pos), $cols );
	    return $result;
	}
}

