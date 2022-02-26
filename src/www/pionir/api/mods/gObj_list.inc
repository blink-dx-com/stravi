<?php

require_once 'f.sql_query.inc';
require_once 'f.sql_sel2.inc';
require_once 'o.PROJ.subs.inc';

/**
 * get elements of a list
 *
 * @author Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0000938 g.XML-RPC > get list of all objects in project (PROJ)
 * @module gObj_list.inc
 * @package WEBAPI\gObj_list
 */
class gObj_list_XRPC extends XRPC_BASE {

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
	 * search/get elements of a table
	 * 
	 * ```  
	 * IN:  array(
	 *     't'=> TABLENAME -- e.g. 'EXP' 
	 *     'cols'=> array(COLUMNS), -- e.g. 'EXP_TMPL_ID', 'NAME'
	 *     'filter'=>array( 
	 *         'cols'=>array(
	 *            array(
	 *              'col' : column with leading 'x.' or 'a.'
     *              'val' : value
     *              'con' : condition, default: =
     *              'bool': ['AND'], 'OR'),
	 *           ),
	 *         'projid'=>ID of project 
	 *             (DEFAULT: Non-recursive search)
	 *       ) 
	 *     'sort'    => string e.g. 'x.NAME DESC|ASC' [OPTIONAL]
	 *     'pagesize'=> [40] -- max number returned of elements
	 *      'pageno' => FUTURE: ID of page, start with 1,2,3, ...
	 *     )
	 *         
	 * OUT: array( 
	 *    'data'=> array(
	 *        array( COL1=>, COL2=>)
	 *    )
	 * 	)
	 * 
	 * EXAMPLE: Python-Style ... 
	 * args_use = {
     *   't':'CHIP_READER',
     *   'cols': [
     *     'x.NAME',
     *     'x.SERIAL',
     *     'x.STATUS_MX',          
     *     'a.CREA_DATE'
     *     ],
     *    'filter':{
     *         'cols': [
     *             'col':'x.A_CHIP_READER_ID', 
     *             'con':'=',
     *             'val':'38',
     *             'bool':'AND'
     *             ]
     *          }
     *       },
     *   'sort':'x.NAME',
     *   'pagesize':30
     *  }
	 * ```
	 * @global array $this->f_param
	 * @return array( 
	 *   'data'=> array()
	 *   'cnt'=> NUM
	 *   'pageid'=>
	 *   )
     * @throws exception
	 */
	function &run() {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$sqlo = $this->_sqlo;
		$MAX_ROW  = 500;
		$page_size= 40;
		if (is_numeric($this->_params['pagesize'])) $page_size = intval($this->_params['pagesize']);
		if ($page_size>$MAX_ROW) $page_size=$MAX_ROW;
		
		$user_text='';
		
		$tablename = $this->_params['t'];
		$cols      = $this->_params['cols'];
		$pk_name   = PrimNameGet2($tablename);
		
		$utilLib = new fSqlQueryC($tablename);
		$utilLib->cleanCond();
		$utilLib->queryRelase();
		
		$order_str='';
		if ($this->_params['sort']!=NULL) {
		    if (!is_string($this->_params['sort'])) {
		        $error->set( $FUNCNAME, 5, 'Input "sort" must be a string.' );
		        return;
		    }
		    $order_str=' ORDER BY '.$this->_params['sort'];
		}

		$has_input_filter=0;
		$col_build_lib = new fSql_sel2($tablename);
		$col_build_lib->cleanCond();
		
		if (empty($cols) ) {
		    $error->set( $FUNCNAME, 8, 'Input cols missing.' );
		    return;
		}
		foreach($cols as $col_loop) {
		    $col_answer = $col_build_lib->column_is_valid($col_loop);
		    if ($error->Got(READONLY))  {
		        $error->set( $FUNCNAME, 10, 'Input: cols > "'.$col_loop.'" is unknown.' );
		        return;
		    }
		    if ($col_answer['prefix']=='a.') {
		        $col_build_lib->addJoin('CCT_ACCESS');
		    }
		}

		$filter = $this->_params['filter'];
		if ( !empty($filter['cols']) ) {
		    $has_input_filter=1;
		    $col_build_lib->set_filter($sqlo, $filter['cols'] );
		    if ($error->Got(READONLY))  {
		        $error->set( $FUNCNAME, 13, 'Error in Filter-analysis.' );
		        return;
		    }
		}
		$col_build_lib->queryRelase();
		
		
		$utilLib->setDefs($tablename); // reload table condition for session ...
		
		if ( $filter['projid']>0 ) {
		    $has_input_filter=1;
		    $proj_sel_sql = cProjSubs::getTableSQL ( $filter['projid'], $tablename );
		    $sqlselWhere  = 'x.'.$pk_name .' in ('.$proj_sel_sql.')';
		    $utilLib->addCond( $sqlselWhere, '', 'in folder '.$filter['projid'] );
		}
		
		if (!$has_input_filter) {
		    $error->set( $FUNCNAME, 15, 'No Input-filter given.' );
		    return;
		}
		
		$sqlAfter = $utilLib->get_sql_after( );
		
		
		$sqlo->Quesel('count(1) from '.$sqlAfter);
		$sqlo->ReadRow();
		$rowcnt = $sqlo->RowData[0];
		if ($error->Got(READONLY))  {
		    $error->set( $FUNCNAME, 20, 'Error in SQL-Query.' );
		    return;
		}
		
		$colssql = implode(', ',$cols);
		$sqlsel = $colssql." from ".$sqlAfter. $order_str;
		$sqlo->Quesel($sqlsel);
		if ($error->Got(READONLY))  {
		    $error->set( $FUNCNAME, 30, 'Error in Query.' );
		    return;
		}
		$dataarr=array();
		$cnt=0;
		while ( $sqlo->ReadArray() ) {
		    if ($cnt>$page_size) {
		        break;
		    }
		    $dataarr[]=$sqlo->RowData;
		    $cnt++;
		}
    	
		return array('data'=>$dataarr, 'cnt'=>$rowcnt, 'txt'=>$user_text);
	}
}
