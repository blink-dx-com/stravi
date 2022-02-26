<?php
require_once ("visufuncs.inc");
require_once ("date_funcs.inc");
/**
 * - show time stamps 

 * @package o.CONTAINER.entrylog.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $_REQUEST['id']  id of container
 */
class o_CONTAINER_entrylog_XPL extends gPlugin {

	function register() {
		$tablename	= 'CONTAINER';
		$title		= 'Show storage log';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;

		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array() )
				);
	}
	

	function viewList(&$sqlo, $sqlo2) {
	    
	    $tabShowLib = new visufuncs();
	    
	    $cont_nice = tablename_nice2('CONTAINER');
	    $cont_log_nice = tablename_nice2('CONT_LOG');
	    
	    $headOpt = array( "title" => $cont_nice." &gt; ".$cont_log_nice);
	    $headx   = array ("#", "Date", "Material", "Material-ID", "Quantity-Diff", "User", "Action", "Misc");
	    if ($this->showraw) $headx[]='raw info';
	    $tabShowLib->table_head($headx,  $headOpt);
	    
	    $SHOWMAX= 200;

	    $cnt=0;
	    $sqsel = "* ".
	   	          " from CONT_LOG where CONTAINER_ID=".$this->objid. " order by POS desc";
	    $sqlo2->Quesel($sqsel);
	   
	    while ( $sqlo2->ReadArray() ) {
	        
	        if ($cnt> $SHOWMAX ) {
	            $dataArr=array("...","...", "...more...");
	            $tabShowLib->table_row ($dataArr);
	            break;
	        }
	        
	        $vals = &$sqlo2->RowData;
	        
	        $userx = $vals['DB_USER_ID'];
	        $pos   = $vals['POS'];
	        $suc_id = $vals['CONCRETE_SUBST_ID'];
	        $key   = $vals['KEY'];
	       
	        
	        $sqls = "select nick from db_user where db_user_id=".$userx;
	        $sqlo->query($sqls);
	        $sqlo->ReadRow();
	        $nickx = $sqlo->RowData[0];
	        
	        $suc_name = obj_nice_name ( $sqlo, 'CONCRETE_SUBST', $suc_id ); 
	        
	        $dataArr = array($pos, $vals['MOD_DATE'], $suc_name, $suc_id, $vals['QUANTITY'], $nickx, $key,  $vals['DATAX'] );
	        
	        if ( !$cnt ) {
	            
	        }
	        
	        $tabShowLib->table_row($dataArr);
	        $cnt++;
	    }

	    $tabShowLib->table_close();
	}
	

	function startMain() {
		global $error;
		//$FUNCNAME='startMain';
		$this->objid   = $_REQUEST['id'];
		$sqlo   = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		$this->viewList($sqlo, $sqlo2);
	}
}
