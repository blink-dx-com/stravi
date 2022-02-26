<?php
/**
 * fast search, ordered by NAME
 * used by REST-API for header.php
 * @author Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @package WEBAPI\gObj_search
 * @module gObj_search.inc
 */
require_once('f.sql_query.inc');

class gObj_search_XRPC extends XRPC_BASE {

    /** check parameters
     * @return : >0 : o.k.
     */
    function __checkParams() {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $sqlo = $this->_sqlo;
        $tablename = $this->_params['t'];
        
        if (!glob_table_exists($tablename))  {
            $error->set( $FUNCNAME, 1, 'table "'.$tablename.'" not exists.' );
            return;
        }
        
        $id=0;
        $acc_matrix=array( 'tab' => array('read'), 'obj'=>array() );
        $this->__objAccChk($sqlo, $tablename, $id, $acc_matrix );
        
        return 1;
    }
    
    
	/**
	 * - get elements of a table
	 * - example: <pre>
	 *    get EXP
	 * 		IN:  array(
	 *             't': tablename; example: 'EXP', 
	 *             'q': search text
	 *             'maxrows' : [OPTIONAL] number of max return rows, e.g. 30
	 *             )
	 * 		OUT: array( 
	 *         'data'= array(
	 * 	           of NAMES
	 *         )
	 * 		)
	 *  </pre>
	 * @global array $this->f_param  <pre>
	 *	
	 * 	</pre>
	 * @return array( 
	 *   'data'=> array( of strings)
	 *   'cnt'=> NUM
	 *   )
     * @throws exception
	 */
	function &run() {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$sqlo = $this->_sqlo;
		$MAX_ROW  = 30;
		//$user_text='';
		
		$tablename   = $this->_params['t'];
		$search_term = trim($this->_params['q']);
		
		if ($search_term==NULL) {
		    return array('data'=>array(), 'cnt'=>0);
		}
		if (is_numeric($search_term)) {
		    // do not search for numbers ...
		    return array('data'=>array(), 'cnt'=>0);
		}
		
		//$pk_name   = PrimNameGet2($tablename);
		$name_col  = importantNameGet2($tablename);

		//$utilLib = new fSqlQueryC($tablename);
		//$utilLib->cleanCond();
		
		$sqlAfter  = $tablename.' where UPPER('.$name_col.') like UPPER('.$sqlo->addQuotes('%'.$search_term.'%').')';
		$sql_order = ' order by '.$name_col;

		switch ($tablename) {
		    case 'ABSTRACT_SUBST':
		        $sqlAfter .= ' or UPPER(SYNONYMX) like UPPER('.$sqlo->addQuotes('%'.$search_term.'%').')';
		        break;
		}
		
		$sqlo->Quesel('count(1) from '.$sqlAfter);
		$sqlo->ReadRow();
		$rowcnt = $sqlo->RowData[0];
		if ($error->Got(READONLY))  {
		    $error->set( $FUNCNAME, 1, 'Error in SQL-Query.' );
		    return;
		}
		
		$colssql = $name_col;
		$sqlsel  = $colssql." from ".$sqlAfter . $sql_order;
		$sqlo->Quesel($sqlsel);
		$dataarr=array();
		$cnt=0;
		while ( $sqlo->ReadRow() ) {
		    if ($cnt>$MAX_ROW) {
		        break;
		    }
		    $dataarr[]=$sqlo->RowData[0];
		    $cnt++;
		}
    	
		return array('data'=>$dataarr, 'cnt'=>$rowcnt);
	}
}
