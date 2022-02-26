<?php
/**
 * AutoCleaner core class
 * 
 * @package o.CCT_ACC_UP.AutoCleanMod.inc
 * @swreq   UREQ:0001088: o.CCT_ACC_UP > AutoCleaner 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 * @version $Header: trunk/src/phplib/lev1/o.CCT_ACC_UP.AutoCleanMod.inc 59 2018-11-21 09:04:09Z $
 */

require_once 'date_funcs.inc';

/**
 * AutoCleaner core class: clean by NUM of entries of one CCT_ACCESS_ID
 * 
 * - new:
 *  
 * - CONCRETE_OBJETCS:
 *  all entries DATE< (now - 3 years) for CONCRETE objects 
 * - ABSTRACT_OBJECTS:
 *   - clean all entries of CCT_ACC_UP where: COUNT(of one object)> MAX_STORE_NUM
 * 
 * @swreq   UREQ:0001088: o.CCT_ACC_UP > AutoCleaner 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 */
class oCCT_ACC_UP_AutoCleanMod {
	var $_isInit =0;
	var $_doDelete=0;
	
	function init(&$sqlo) {
		$this->MAX_STORE_NUM = 50;
		
		$this->_isInit   = 1;
		$this->_doDelete = 1;
		$this->POS		 = 0;
		$this->cntOld    = 0;
		
	}
	
	
	
	/**
	 * delete all entries of one CCT_ACCESS_ID
	 * - delete all elements, which are bigger than MAX_STORE_NUM
	 * - start with the oldest entries ...
	 * @param $sqlo
	 * @return int number of deleted entries
	 */
	function _delForOneAccess( &$sqlo, $CCT_ACCESS_ID ) {
		$MAX_STORE_NUM = $this->MAX_STORE_NUM;
		
		// get the START-POS, start at newset entry ...
		$sqlsel = 'POS from CCT_ACC_UP where CCT_ACCESS_ID=' . $CCT_ACCESS_ID . 
			' order by POS DESC';
		$sqlo->Quesel($sqlsel);
		$cnt=0;
		while ( $sqlo->ReadRow() ) {		
			$POS = $sqlo->RowData[0];
			if ($cnt >= $MAX_STORE_NUM) {
				break;
			}
			$cnt++;
			
		}
		
		$this->POS=$POS;
		
		// this is the start-pos, to delete old entries
		if (!$POS) return;
		
		// @swreq UREQ:1088:002: do NOT delete entries with UPINFO like '%a:arch%'
		$deleteWhere = 'CCT_ACCESS_ID=' . $CCT_ACCESS_ID . ' and POS<='.$POS. "and UPINFO not like '%a:arch%'";
		
		$sqlsel = 'count(1) from CCT_ACC_UP where ' . $deleteWhere;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$delCnt = $sqlo->RowData[0];
		
		// delete it !!!
		if ($this->_doDelete) {
			$sqlo->Deletex ( 'CCT_ACC_UP', $deleteWhere );
		}
		return $delCnt;
	}
	
	/**
	 * init the cleaning process
	 * @param $sqlo : this handle is reserved for method oneRow !!!
	 */
	function cleanStart( &$sqlo ) {
		$this->delCnt  = 0;
		$MAX_STORE_NUM = $this->MAX_STORE_NUM;
		$sqlsel_PRE = 'CCT_ACCESS_ID  from CCT_ACC_UP group by CCT_ACCESS_ID '.
			'     having count(CCT_ACCESS_ID) > ' . $MAX_STORE_NUM;
		
		$sqlsel = 'count(1) from CCT_ACCESS where CCT_ACCESS_ID in (select '.$sqlsel_PRE .')';
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$objCnt = $sqlo->RowData[0];

		// this SQL-handle is reserved !!!
		$sqlsel = $sqlsel_PRE . ' order by CCT_ACCESS_ID';
		$sqlo->Quesel($sqlsel);
		return ($objCnt);
	}
	
	
	/**
	 * analyse one row
	 * @param $sqlo
	 * @param $sqlo2
	 * @return boolean 0,1
	 */
	function oneRow(&$sqlo, &$sqlo2) {
		
		$this->delCnt  = 0;
		if ( $sqlo->ReadRow() ) {
		 	$CCT_ACCESS_ID = $sqlo->RowData[0];
		 	$this->CCT_ACCESS_ID = $CCT_ACCESS_ID;
		    $this->delCnt = $this->_delForOneAccess($sqlo2, $CCT_ACCESS_ID);
		    return 1;
		}
		return 0;
	}
	
	function getRowInfo() {
		return array( 
			'CCT_ACCESS_ID'=> $this->CCT_ACCESS_ID, 'cnt'=>$this->delCnt, 
			'pos'=>$this->POS
		 );
	}
	
	function getMAX_STORE_NUM() {
		return $this->MAX_STORE_NUM;
	}
	
}

/**
 * AutoCleaner core class by DATE
 * 
 * - new:
 *  
 * - CONCRETE_OBJETCS:
 *  all entries DATE< (now - 3 years) for CONCRETE objects 
 * - ABSTRACT_OBJECTS:
 *   - clean all entries of CCT_ACC_UP where: COUNT(of one object)> MAX_STORE_NUM
 * 
 * @swreq   UREQ:0001088: o.CCT_ACC_UP > AutoCleaner 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 */
class oCCT_ACC_UP_CleanModDate {
	var $_isInit =0;
	var $_doDelete=0;
	
	function init(&$sqlo) {

		$this->infox = array();
		$this->_isInit   = 1;
		$this->_doDelete = 1;
		
		$this->cntOld    = 0;
		
		$now = time();
		$yearsLimit = 2;
		$conc_date_limit_unx = $now - date_unixDay2Sec($yearsLimit*365);
		
		$this->conc_date_limit = date_unix2datestr($conc_date_limit_unx, 2);
		
		// @swreq UREQ:1088:001: folgende Tabellen:
		$this->conc_tablenames = array(
			'EXP', 'CONCRETE_PROTO', 'CONCRETE_SUBST', 'CART_BATCH', 'CHIP_READER', 'IMG', 'WORKLIST'
		);
	}
	
    function get_startDateHum() {
		return $this->conc_date_limit;
	}
	
	function getInfox() {
		return $this->infox;
	}
	
	/**
	 * delete all entries of one CCT_ACCESS_ID
	 * - delete all elements, which are bigger than MAX_STORE_NUM
	 * - start with the oldest entries ...
	 * @param $sqlo
	 * @return int number of deleted entries
	 */
	function _delForOneAccess( &$sqlo, $CCT_ACCESS_ID ) {
		$MAX_STORE_NUM = $this->MAX_STORE_NUM;

		//@swreq UREQ:1088:002: do NOT delete entries with UPINFO like '%a:arch%'
		$deleteWhere = 'CCT_ACCESS_ID=' . $CCT_ACCESS_ID. ' and MODI_DATE < ' . $sqlo->addQuotes($this->conc_date_limit) .
			" and UPINFO not like '%a:arch%'";
		
		$sqlsel = 'count(1) from CCT_ACC_UP where ' . $deleteWhere;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$delCnt = $sqlo->RowData[0];
		
		if (!$delCnt) return 0;
		
		// delete it !!!
		if ($this->_doDelete) {
			$sqlo->Deletex ( 'CCT_ACC_UP', $deleteWhere );
		}
		
		
		
		return $delCnt;
	}

   /**
	 * init the cleaning process for CONCRETE_OBJECTS
	 * @param $sqlo : this handle is reserved for method oneRow !!!
	 */
	function cleanStart( &$sqlo ) {
		$this->delCnt  = 0;
		
		
		reset ($this->conc_tablenames);
		$komma =NULL;
		foreach( $this->conc_tablenames as $dummy=>$table) {
			$tableNames_sql .= $komma . "'".$table."'";
			$komma =',';
		}
		reset ($this->conc_tablenames); 

		
		/* - make sure, you got the right tables !
		 * - do NOT use the VIEW  "CCT_ACC_UP_VIEW" here; the query would be very SLOW!
		 */
		$sqlsel_PRE = 'CCT_ACCESS_ID  from CCT_ACCESS where TABLE_NAME in ('.$tableNames_sql.') and '.
				' MOD_DATE < ' . $sqlo->addQuotes($this->conc_date_limit);
		
		$sqlsel = 'count(1) from CCT_ACCESS where CCT_ACCESS_ID in (select '.$sqlsel_PRE .')';
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$objCnt = $sqlo->RowData[0];
		
		$sqlsel = 'max(CCT_ACCESS_ID) from CCT_ACCESS where CCT_ACCESS_ID in (select '.$sqlsel_PRE .')';
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$this->infox['CCT_ACCESS_max'] = $sqlo->RowData[0];

		// this SQL-handle is reserved !!!
		$sqlsel = $sqlsel_PRE . ' order by CCT_ACCESS_ID';
		$sqlo->Quesel($sqlsel);
		return ($objCnt);
	}
	
	/**
	 * analyse one row
	 * @param $sqlo
	 * @param $sqlo2
	 * @return boolean 0,1
	 */
	function oneRow(&$sqlo, &$sqlo2) {
		
		$this->delCnt  = 0;
		if ( $sqlo->ReadRow() ) {
		 	$CCT_ACCESS_ID = $sqlo->RowData[0];
		 	$this->CCT_ACCESS_ID = $CCT_ACCESS_ID;
		    $this->delCnt = $this->_delForOneAccess($sqlo2, $CCT_ACCESS_ID);
		    return 1;
		}
		return 0;
	}
	
	function getRowInfo() {
		return array( 
			'c'=> $this->CCT_ACCESS_ID, 'cnt'=>$this->delCnt
			
		 );
	}
	
	
}

 