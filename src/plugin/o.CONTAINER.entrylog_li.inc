<?php
require_once ("visufuncs.inc");
require_once ("date_funcs.inc");
/**
 * - for ALL containers: show time stamps 
 

 * @package o.CONTAINER.entrylog_li.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)   
 */
class o_CONTAINER_entrylog_li_XPL extends gPlugin {
	
	function register() {
		$tablename	= 'CONTAINER';
		$title		= 'Show storage logs of all containers';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh']    = 'show all logs';
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;

		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array() )
				);
				
	}
	

	function viewList(&$sqlo, $sqlo2, $parx) {
	    
	    $tabShowLib = new visufuncs();
	    
	    $cont_nice = tablename_nice2('CONTAINER');
	    $cont_log_nice = tablename_nice2('CONT_LOG');
	    
	    $headOpt = array( "title" => $cont_nice." &gt; ".$cont_log_nice);
	    $headx   = array ("#", "Container", "Date", "Material", "MAC-ID", "Quantity-Diff", "User", "Action", "Pos");
	    if ($this->showraw) $headx[]='raw info';
	    $tabShowLib->table_head($headx,  $headOpt);
	    
	    $SHOWMAX = 200;
	    $user_cache_MAX=40;
	    $user_cache=array();
	    $cont_cache_MAX=40;
	    $cont_cache=array();
	    
	    $sql_add='';
	    $suc_name = $parx['name'];
	    if ($suc_name!=NULL) {
	        $sql_add='where CONCRETE_SUBST_ID in (select CONCRETE_SUBST_ID from CONCRETE_SUBST where UPPER(name) like UPPER('. $sqlo->addQuotes('%'.$suc_name.'%') . ') )';
	    }

	    $cnt=0;
	    $sqls = "select POS, MOD_DATE, CONCRETE_SUBST_ID, QUANTITY, DB_USER_ID, KEY, CONTAINER_ID ".
	   	    " from CONT_LOG ".$sql_add." order by MOD_DATE desc";
	    $sqlo2->query($sqls);
	   
	    while ( $sqlo2->ReadRow() ) {
	        
	        if ($cnt> $SHOWMAX ) {
	            $dataArr=array("...","...", "...more...");
	            $tabShowLib->table_row ($dataArr);
	            break;
	        }
	        
	        $cont_id  = $sqlo2->RowData[6];
	        $pos      = $sqlo2->RowData[0];
	        $datex    = $sqlo2->RowData[1];
	        $suc_id   = $sqlo2->RowData[2];
	        $quant_diff = $sqlo2->RowData[3];
	        $userx     = $sqlo2->RowData[4];
	        $key      = $sqlo2->RowData[5];
	       
	        
	        if ($user_cache[$userx]) $nickx=$user_cache[$userx];
	        else {
    	        $sqls = "select nick from db_user where db_user_id=".$userx;
    	        $sqlo->query($sqls);
    	        $sqlo->ReadRow();
    	        $nickx = $sqlo->RowData[0];
    	        if (sizeof($user_cache)<$user_cache_MAX)
    	           $user_cache[$userx]=$nickx;
	        }
	        
	        if ($cont_cache[$cont_id]) $cont_name=$cont_cache[$cont_id];
	        else {
	            $sqls = "select NAME from CONTAINER where CONTAINER_ID=".$cont_id;
	            $sqlo->query($sqls);
	            $sqlo->ReadRow();
	            $cont_name = $sqlo->RowData[0];
	            if (sizeof($cont_cache)<$cont_cache_MAX)
	                $cont_cache[$cont_id] = $cont_name;
	        }
	        
	        $suc_name = obj_nice_name ( $sqlo, 'CONCRETE_SUBST', $suc_id ); 
	        
	        $dataArr = array( ($cnt+1), $cont_name, $datex, $suc_name, $suc_id, $quant_diff, $nickx, $key, $pos );
	        
	       
	        
	        $tabShowLib->table_row($dataArr);
	        $cnt++;
	    }
	    
	    
		
	    $tabShowLib->table_close();
	}
	
	/**
	 * - creates a form with one text field "Min-value"
	 * - creates a HTML-page parameter $_REQUEST['parx']['minval'] after SUBMIT
	 */
	function form1($parx) {
	    
	    $initarr   = array();
	    $initarr["title"]       = "Search in log"; 
	    $initarr["submittitle"] = "Search"; 
	    $initarr["tabwidth"]    = "AUTO"; 
	    $hiddenarr = array();
	   
	    
	    // init form, uses class formc
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
	    
	    $fieldx = array ( // form-field definition
	        "title"   => "MAC name part",
	        "name"  => "name",
	        "object"=> "text",
	        "val"    => $parx['name'],
	        "notes" => "e.g. Chitosan (search material name)"
	    );
	    $formobj->fieldOut( $fieldx ); // output the form-field
	    
	    $formobj->close( TRUE ); // close form, sshow submit-button
	}
	

	function startMain() {
		global $error;
		//$FUNCNAME='startMain';

		$sqlo   = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		$parx = $_REQUEST['parx'];

		$this-> form1($parx);
		echo "<br>";
		
		
		$this->viewList($sqlo, $sqlo2, $parx);
	}
	
}
