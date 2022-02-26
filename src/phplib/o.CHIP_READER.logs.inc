<?php
require_once('o.REA_LOG.subs.inc');
require_once 'o.DEVSIC.subs.inc';

/**
 * main methods to handle CHIP_READER > REA_LOG
 * cols:
 *  CHIP_READER_ID
    POS
    NAME
    XDATE
    XDATE2
    DB_USER_ID
    KEY
    PRIO
    ACCEPT
    NOTES
    DEVSIC_ID
    XDATA
    MOD_DATE

 * @package o.CHIP_READER.logs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/o.CHIP_READER.logs.inc 59 2018-11-21 09:04:09Z $
 */
class oREA_LOG_subs {
    
    const KEY_Qualifizierung='Qualifizierung';
    const KEY_Working='Working';
    const KEY_STOP='STOP';
    const KEY_Quali_erwartet = 'Quali_erwartet';
    const KEY_Service='Service';
    const KEY_Problem='Problem';
    
    const ACCEPT_pref = array(
        1=>'planned',
        2=>'accepted',
        3=>'started',
        4=>'finished',
        5=>'X-notDone',
        6=>'finished_NIO'
    );
    
    const ACC_PLANNED =1;
    const ACC_ACCEPTED=2;
    const ACC_STARTED=3;
    const ACC_FINISHED=4;
    const ACC_NOTDONE=5;
    const ACC_FINISHED_NIO=6;
    
    public $defKeys;

    
    
	function __construct() {
		
	    $this->defKeys = array('qualiok'=> self::KEY_Qualifizierung, 'qualineed'=>self::KEY_Quali_erwartet);
		$this->dateNow = time();
	}
	
	static function getActColors() {
	    return oREA_LOG_subs2::$actionColors;
	}
	static function get_ACCEPT_name2ID($name) {
	    if($name==NULL) return 0;
	    $ACCEPT_pref_rev = array_flip(self::ACCEPT_pref);
	    return intval($ACCEPT_pref_rev[$name]);
	}
	static function get_ACCEPT_ID2name($id) {
	    if($id===NULL) return;
	    return self::ACCEPT_pref[$id];
	}
	
	
	/**
	 * get current status
	 * @param object $sqlo
	 * @param int $devid
	 * @return array
	 */
	function getCurrentKey(&$sqlo,$devid) {
		$sqlsel = 'POS, KEY from REA_LOG where CHIP_READER_ID='.$devid.
			'  and XDATE<= '.$sqlo->Timestamp2Sql($this->dateNow).' order by XDATE desc';
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$pos = $sqlo->RowData[0];
		$key = $sqlo->RowData[1];
		
		return array($pos,$key);
	}
	
	/**
	 * get ALL columns from last entry with $qualikey (OLD: POS, XDATE, ACCEPT )
	 * @param object $sqlo
	 * @param int $devid
	 * @param string $qualikey
	 * @return array COL=>VAL
	 * 
	 */
	static function _getMaxKey(&$sqlo,$devid, $qualikey, $substatus=NULL) {
		$subsql=NULL;
		if ($substatus) $subsql = ' and ACCEPT='.$substatus;
		$sqlsel = '* from REA_LOG where CHIP_READER_ID='.$devid.
			 ' and KEY='.$sqlo->addQuotes($qualikey) . $subsql.' order by XDATE desc';
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadArray();
		
		return $sqlo->RowData;
	}
	
	
	
	/**
	 * get * from last entry with DEVSIC:TYPE=$devSicType and ACCEPT=$substatus
	 * @param object $sqlo
	 * @param int $devid
	 * @param string $devSicType
	 * @param int $substatus
	 * @return array COL=>VAL
	 *
	 */
	static function getMax_DevSic(&$sqlo, int $devid, string $devSicType, int $substatus=0) {
	    $subsql=NULL;
	    if ($substatus!=NULl) $subsql = ' and ACCEPT='.$substatus;
	    
	    $sql_devsic = 'DEVSIC_ID in (select DEVSIC_ID from DEVSIC where TYPE='.$sqlo->addQuotes($devSicType).')';
	    $sqlsel = '* from REA_LOG where CHIP_READER_ID='.$devid.
	       ' and ' . $sql_devsic. $subsql.' order by XDATE desc';
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadArray();
	    return $sqlo->RowData;
	}
	
	
	
	/**
	 * analyse current OK-state
	 * @param  $sqlo
	 * @param  int $devid
	 * @return  ['STOP','Working','']
	 */
	static function ana_OKState(&$sqlo, $devid) {
	    $qualikey=array();
		$qualikey[0]=self::KEY_STOP;
		$qualikey[1]=self::KEY_Working; 
		
		$infoarr1 = self::_getMaxKey($sqlo, $devid, $qualikey[0]);
		$infoarr2 = self::_getMaxKey($sqlo, $devid, $qualikey[1]);
		$answer='';
		if ($infoarr1['POS'] and $infoarr1['XDATE']>$infoarr2['XDATE']) $answer='STOP';
		if ($infoarr2['POS'] and $infoarr1['XDATE']<$infoarr2['XDATE']) $answer='Working';
		
		
		return $answer;
		
	}
	
	/**
	 * analyse QUALIFICATION-state
	 * @param  $sqlo
	 * @param  int $devid
	 * @return string q or n
	 */
	function ana_Quali_State(&$sqlo, $devid) {
	    
	    $q_val='';
	    // $infoarr1 = $this->_getMaxKey($sqlo, $devid, oREA_LOG_subs::KEY_Qualifizierung); // current Qualifizierung
	    $infoarr2 = $this->_getMaxKey($sqlo, $devid, oREA_LOG_subs::KEY_Quali_erwartet); // Quali_erwartet
	    $infoarr3 = $this->getMax_DevSic($sqlo, $devid, oDEVSIC_subs::TYPE_Q, oREA_LOG_subs::ACC_FINISHED ); // finished Qualifizierung
	    $infoarr4 = $this->getMax_DevSic($sqlo, $devid, oDEVSIC_subs::TYPE_Q, oREA_LOG_subs::ACC_FINISHED_NIO ); // finished NIO
	    
	    if (!empty($infoarr3) and !empty($infoarr4)) {
	        if ($infoarr4['XDATE2']>$infoarr3['XDATE2']) {
	            // last NIO  is newer than last IO
	            $infoarr3=array();
	        }
	    }
	    
	    if ($infoarr2['POS']) {
	        $q_val='n';  // Quali_erwartet
	        // analyse FINSIH-date of QUALI !
	        if ($infoarr3['XDATE2']>$infoarr2['XDATE']) {
	            $q_val='q';   // Qualifizierung FINISHED
	        }
	    } else {
	        // no quali erwartet
	        if ($infoarr3['POS']) {
	            $q_val='q';
	        }
	    }
	    
	    return $q_val;
	}
	
	// from features['STATUS_MX'];
	function getQualiState(&$sqlo, $devid) {
	    
	    $features = glob_elemDataGet3( $sqlo, 'CHIP_READER', array('CHIP_READER_ID'=>$devid), array('STATUS_MX') );
	    $info='';
	    $mx = $features['STATUS_MX'];
	    if (strstr($mx,'q')!=NULL) $info='Qualified';
	    if (strstr($mx,'n')!=NULL) $info='Qualification needed';
		return $info;
	}
	
	/**
	 * get last ENTRY with KEY
	 * @param object $sqlo
	 * @param int $objid
	 * @param string $key
	 * @return array(KEY, XDATE, NAME, POS)
	 */
	public function getLastKey_entry(&$sqlo, $objid, $key) {
	    $sqlsel = "* from REA_LOG where CHIP_READER_ID=". $objid. " and KEY=" . $sqlo->addQuotes($key) .
	        " order by XDATE DESC";
	    $sqlo->Quesel($sqlsel);
	    if (!$sqlo->ReadArray() ) return;
	    return $sqlo->RowData;
	}
	
	/**
	 * get last key with  ACCEPT != 1 (planned)
	 * @param object $sqlo
	 * @param int $objid
	 * @param string $key
	 * @param string $mindate
	 * @return array(KEY, XDATE, NAME, POS)
	 */
	public function getLastKey_SUBLAST(&$sqlo, $objid, $key, $mindate) {
	
		$subSql=NULL;
		if ($key!=NULL) $subSql = " and KEY='" . $key . "'";
		$sqlsel = "KEY, ".$sqlo->Sql2DateString('XDATE', 4 ) .", NAME, POS, ACCEPT from REA_LOG where CHIP_READER_ID=". $objid.
				 ' and XDATE<= ' . $sqlo->Timestamp2Sql($this->dateNow) . $subSql . " and ACCEPT!=1".
				 " and XDATE>=".$sqlo->addQuotes($mindate)." order by XDATE DESC";
		$sqlo->Quesel($sqlsel);
		if (!$sqlo->ReadArray() ) return;
		return $sqlo->RowData;
	}
	
	
	
	/**
	 * main function to recalculate STATUS_MX (flag meanings: see DB-LATEX-doc)
	 * @param object $sqlo
	 */
	function calc_STATUS_MX($sqlo, $devid) {
	    
	    $mx_val='';
	    
	    $q_val = $this->ana_Quali_State($sqlo, $devid);
	    $mx_val.=$q_val;
	    
	    $OKState = $this->ana_OKState($sqlo, $devid);
	    if ($OKState=='STOP') {
	        $mx_val.='s';
	    }
	    if ($OKState=='Working') {
	        $mx_val.='o';
	    }
	    
	    # analyse service
	    $infoarr_s = $this->_getMaxKey($sqlo, $devid, oREA_LOG_subs::KEY_Service);
        if ($infoarr_s['ACCEPT'] == oREA_LOG_subs2::ACCEPT_key2id('START') ) {
            $mx_val.='x';
        }

	    return $mx_val;
	}
}
