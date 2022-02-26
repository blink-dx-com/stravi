<?php
/**
 * - plugin table ABS_WORKLIST
 *    
 * @package o.ABS_WORKLIST.guimod.inc
 * @swreq UREQ:0000932 db.WORKLIST: Messauftrag organisieren
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $_REQUEST['id'] id of CYTCART
 * @param $_REQUEST['act'] 
 *   'new' : create one empty row
 *   'update' - update all planned cells: cell[y][x] = ID of object or other values
 * @param $_REQUEST['cell'] used by 'update.plan'
 */
require_once ("o.ABS_WORKLIST.subs2.inc");
require_once ("f.assocUpdate.inc");
/**
 * @author steffen
 *
 */
class o_ABS_WORKLIST_guimod_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'ABS_WORKLIST';
		$title		= 'Modify';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = 'Modify';
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read', 'write'), 'obj'=>array('write') )
				);
					
	}
	
	function _act_NEW(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':_act_NEW';
		
		// get last pos
		$sqlsel = 'max(XPOS) from ABS_WORKLIST_ENTRY where ABS_WORKLIST_ID='.$this->objid;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$lastX = $sqlo->RowData[0];
		if (!$lastX) $lastX=0;
		$nextX=$lastX+1;
		
		$argu = NULL ;
		$argu ["XPOS"]=$nextX;
		$argu ["TYPE"]='OBJ';
		$this->assoclib->insert ( $sqlo , $argu );
	}
	
	function _act_UPDATE_PLAN( &$sqlo, $cells ) {
		global $error;
		$FUNCNAME= __CLASS__.':_act_UPDATE_PLAN';
		
		foreach( $cells as $xpos=>$argu) {
			
				$idarr = NULL ; // the second primary keys of the element row
				$idarr ["XPOS"] = $xpos;
				$this->assoclib->update ( $sqlo , $argu , $idarr );
				
				if ($error->Got(READONLY))  {
					$error->set( $FUNCNAME, 4, 'error on update of column: '.$xpos );
					return;
				}
			
		}
		reset ($cells); 
		$this->_html-> line_out('o.k.');
	}
	
	
	

	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$this->objid   = $_REQUEST['id'];
		$sqlo = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		$parx   = $_REQUEST['parx'];
		$action = $_REQUEST['act'];
		
		if ($action==NULL) {
			$this->htmlFoot('ERROR','Parameter "action" missing');
		}
		
		$this->assoclib = new fAssocUpdate ();
		$this->assoclib->setObj ( $sqlo , 'ABS_WORKLIST_ENTRY', $this->objid );
		if ( $error->Got( READONLY )) { // check access rights
			$error->set ( 'assocTest ', 1, 'error on init' );
			return ;
		}
		
		$this->_html-> line_out('Action: '.$action);
		
		switch ($action) {
			case 'new':
				$this->_act_NEW($sqlo);
				break;
				
			case 'update':
				if (!sizeof($_REQUEST['cell'])) {
					$this->htmlFoot('ERROR','No cells given.');
				}
				$this->_act_UPDATE_PLAN($sqlo, $_REQUEST['cell'] );
				break;
			
		}
		
		if ($error->Got(READONLY))  {
			return;
		}
		
		$url='edit.tmpl.php?t=ABS_WORKLIST&id='.$this->objid;
		js__location_replace($url, "object" );
	}
	
}
