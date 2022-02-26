<?php
/**
 * object list search II form
 * @package g.objtab.sea2.inc
 * @swreq 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   
 *  $_REQUEST['t'] - tablename
	$_REQUEST['go'] - 0,1 : submitted vario columns
	$_REQUEST['filt']

 */
require_once 'gui/glob.objtab.filter_GUI2.inc';


class g_objtab_sea2_XPL extends gPlugin {
	
	
	function register() {
		$tablename	= $_REQUEST['t'];
		$title		= 'Search II form';
		
		$this->infoarr			 = NULL;
		
		$this->infoarr['title']    = $title;
		//$this->infoarr['title_sh'] = 'Analysiere';
		//$this->infoarr['help_url'] = 'p129/xxxx'; // link im Wiki: 
		//$this->infoarr['help_base']= 'wiki';  // use wiki
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_cnt']  = 1; // count current selection
		
		
		$this->infoarr2 = array( 
			'accCheck' => array('tab'=>array('read'), 'obj'=>array() ),
			'listChkSelect' => 0
		);
				
		
	}

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		

		$sqlo = &$this->_sqlo;
		//$sqlo2 = &$this->_plug_regSql2();
		
		$tablename = $_REQUEST['t'];
		$go        = $_REQUEST['go'];
		$parx      = $_REQUEST['filt'];
		
		
		$this->tablename = $tablename;
		
		$json_arr = array();
		$url_normal= $this->_plug_get_URL().'&t='.$tablename;
		$sea_form = new g_objtab_filter_GUI2($sqlo, $tablename, $json_arr, $url_normal);
		
		if ($go) {
		    
		    debugOut('(65) FILTER:IN: '.print_r($parx,1), $FUNCNAME, 1);
		    
		    $sea_form->user_filter_save($sqlo, $parx);
		    $sea_form->create_SQL($sqlo);
		    
		    $sql_cmd = $sea_form->get_sqlAfterSort();
		    echo "CMD: $sql_cmd <br>";
		    
		    if (!$error->Got(READONLY) and $sql_cmd)  {
		        echo "Release SQL-Query ...<br>";
		        $sea_form->queryRelase();
		    }
		}
		$sea_form->js();
		
		echo '<form style="display:inline;" method="post" '.
		  		' name="editform"  action="'.$_SERVER['PHP_SELF'].'" >'."\n";
		echo '<input type=hidden name="go" value="1">'."\n";
		echo '<input type=hidden name="mod" value="'.$this->_mod.'">'."\n";
		echo '<input type=hidden name="t" value="'.$tablename.'">'."\n";
		
		$sea_form->form_main($sqlo);
		echo '<input type=submit value="Submit" class="yButton">'."\n"; 
		echo '<form>'."\n";
		
		
		
	}
	
}