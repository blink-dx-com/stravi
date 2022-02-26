<?php
require_once ("glob.obj.update.inc");
require_once 'f.objview.inc';
/**
 * 
 * @package o.CCT_COLUMN.adv
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $id
 * @param   $id2
 */
class o_CCT_COLUMN_adv_XPL extends gPlugin {
	
	
	function register() {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
		$tablename	= 'CCT_COLUMN';
		$title		= 'advanced CCT_COLUMN editor';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		
		if (!$_REQUEST['id'] or !$_REQUEST['id2']) {
	        $error->set( $FUNCNAME, 1, 'Input params are missing: id, id2' );
	        return;
		}
		
		$this->infoarr2 = array( 
			'accCheck' => array('tab'=>array('write'), 'obj'=>array('read') ),
		    'onlyAdmin'=>1
		);
	}

	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		$tablename='CCT_COLUMN';
		
		$sqlo = &$this->_sqlo;
		
		
		$idarr=array();
		$idarr['TABLE_NAME'] = $_REQUEST['id'];
		$idarr['COLUMN_NAME'] = $_REQUEST['id2'];
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
		
		if ($go) {
		    
		    echo "Update DATAX ...<br>";
		    if (empty($parx)) {
		        $this->_html->line_out('No input data found.');
                return;
		    }
		    
		    $datax_arr=array();
		    foreach($parx as $key=>$val) {
		        $datax_arr[$key]=trim($val);
		    }
		    
		    $datax_str=json_encode($datax_arr);
		    $args=array(
		        'vals'=>array(
		            'DATAX'=>$datax_str
		        )
		    );
		    globObjUpdate::update_raw($sqlo, $tablename, $args, $idarr);
		    if ($error->Got(READONLY))  {
		        $error->set( $FUNCNAME, 10, 'Error on Update' );
		        return;
		    }
		    $url = fObjViewC::bo_ID_URL($tablename, $idarr['TABLE_NAME'], $idarr['COLUMN_NAME']);
		    $this->_html->pageForward($url, 'Back');
		}
	}
}
