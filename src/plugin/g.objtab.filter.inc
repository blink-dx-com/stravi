<?php
require_once 'f.sql_sel2.inc';

/**
 * activate a filter for a table
 *    
 * @package g.objtab.filter.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $_REQUEST['t'] tablename
 * @param $_REQUEST['filt']  f.sql_sel2.inc:sql_filter_STRUCT
 * @param $forward_url (encoded)
 *   
 */
class g_objtab_filter_XPL extends gPlugin {
	
	function register() {
	    $tablename	= $_REQUEST['t'];
		$title		= 'activate filter';	
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['obj_name'] = $tablename;
		
		$this->infoarr2 = array( 
			'accCheck' => array('tab'=>array('read') ),
		);
	}
	
	function startMain() {
        global $error;

        $sqlo   = &$this->_sqlo;
		$filt   = $_REQUEST['filt'];
		$tablename	= $_REQUEST['t'];
		
		if (empty($filt)) {
		    $this->htmlFoot('ERROR', 'input filt is missing.');
		}
		
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
		    debugOut('TABLE: '.$tablename.' FILTER_in: <pre>'.print_r($filt,1).'</pre>', $FUNCNAME);
		}

		$query_lib = new fSql_sel2($tablename);
		$query_lib->cleanCond();
		$query_lib->set_filter($sqlo, $filt);
	    if ($error->Got(READONLY))  {
	        return;
	    }
	    $query_lib->queryRelase();
	    
	    // reload page
	    $forward_url = $_REQUEST['forward_url'];
	    $this->_html->pageForward($forward_url, 'forward');

	}
	
}
