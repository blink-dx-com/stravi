<?php
require_once 'o.CCT_NOTES.sub.inc';

/**
 * modify one CCT_NOTES entry
 * @package o.CCT_NOTES.mod.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param string $t
 * @param int    $id
 * @param string $action
 *   'add'
 * @param string $backurl (urlencoded)
 * @param   array parx
 *   NAME
 *   NOTES 
 */
class o_CCT_NOTES_mod_XPL extends gPlugin {
	
	
	function register() {
	    $tablename = $_REQUEST['t'];
		$title		= 'add/modify on object note entry';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		$this->infoarr2 = array( 
		        // @todo: may be in future this access restriction are more loose for the TABLE CCT_NOTES ....
				'accCheck' => array('tab'=>array('write'), 'obj'=>array('read', 'write') )
			);
	}
	

	
	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
		$action = $_REQUEST['action'];
		
		$notes_lib = new oCCT_NOTES_sub( $sqlo, $_REQUEST['t'], $_REQUEST['id'] );
		
		switch ($action) {
		    
		    case 'add':
		        
		        if ( !$parx['NAME'] or !$parx['NOTES'] ) {
		            $error->set( $FUNCNAME, 1, 'Input missing: name or notes.' );
		            return;
		        }
		        
		        $notes_lib->add($sqlo, $parx);
		        
		        $this->_html->line_out('entry added ...');
		        
		        break;
		}
		
		
		if ($error->Got(READONLY))  {
		    $error->set( $FUNCNAME, 2, 'Error on action.' );
		    return;
		}
		
		if ($_REQUEST['backurl']) {
		    
		    $forward_url = urldecode( $_REQUEST['backurl'] );
		    $this->_html->pageForward($forward_url, 'go back');
		    
		}
	}
}
