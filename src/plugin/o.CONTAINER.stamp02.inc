<?php

require_once ("f.assocUpdate.inc");
require_once ("date_funcs.inc");
/**
 * - show time stamps of a substance with $id
 * - create a new time stamp for substance with $id
 * - work on table: CSUB_HAS_STAMP
 *    
 * @package o.CONTAINER.stamp02.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0001345: o.CONTAINER > checkout/checkin timestamp
 * @param $_REQUEST['id']  id of CONCRETE_SUBST
 * @param $_REQUEST['action'] = 
 * 	['view'] |
 * 	'STAMP'   : create new time stamp (UREQ:0001345) : 
 * 				parx['key'] = 1,2, 
 * 				parx['cont_id'] (container_id)
 * 				- !!! need insert right on connected container
 * 				- need no insert-right on substance
 *  'EDITONE' : edit one pos, need $parx['pos']
 * $parx['key'] = 1,2
 * $parx['notes'] = notes for action==EDITONE'
 * $parx['pos'] = pos of time stamp
 *   
 */
class o_CONTAINER_stamp02_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'CONCRETE_SUBST';
		$title		= 'Analyse substance time stamps';
		
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
	
	/**
	 * check time stamping
	 */
	function stampCheck(&$sqlo, $parx) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->containerPos = 0;
		$tablename    = $this->infoarr['obj_name'];
		$this->parx   = $parx;
		$substance_id = $this->objid;
		$container_id = $parx['cont_id'];
		
		$this->_html->line_out("... add time stamp for key=" . $this->keyDefs[$parx['key']] . 
			"; connected container-id:".$parx['cont_id'] );
		
		if (!$container_id) {
			$error->set( $FUNCNAME, 6, 'container_id missing!' );
			return;
		}
		
		$o_rights = access_check($sqlo, 'CONTAINER', $container_id);
		if ( !$o_rights["insert"]) {
			$error->set( $FUNCNAME, 5, "You do not have insert permission on this CONTAINER ".$container_id."!" );
			return;
			
		}
		if (!$parx['key']) {
			$error->set( $FUNCNAME, 1, 'KEY missing!' );
			return;
		}
		
		if (strlen($parx['notes'])>255) {
			$error->set( $FUNCNAME, 7, 'Notes too long. Max:255 chars' );
			return;
		}
		
		$sqlsel = 'POS from CONT_HAS_CSUBST where CONCRETE_SUBST_ID='.$substance_id.' and CONTAINER_ID='.$container_id;
		$sqlo->Quesel($sqlsel);
		if (!$sqlo->ReadRow()) {
			$error->set( $FUNCNAME, 3, 'substance not exists in container!' );
			return;
		}
		$this->containerPos = $sqlo->RowData[0];
		
		$sqlsel = 'POS, DATEX, KEY from CSUB_HAS_STAMP where CONCRETE_SUBST_ID='.$substance_id. ' order by DATEX DESC';
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$keyNow = $sqlo->RowData[2];
		if ($keyNow==$parx['key']) {
			$error->set( $FUNCNAME, 4, 'Status-key already set to '.$keyNow );
			return;
		}
		
	}
	
	/**
	 * add time stamp
	 */
	function stampDo(&$sqlo) {
		$parx = $this->parx;
		$substance_id = $this->objid;
		
		$sqlsel = 'max(POS) from CSUB_HAS_STAMP where CONCRETE_SUBST_ID='.$substance_id;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$lastpos = $sqlo->RowData[0];
		if (!$lastpos) $lastpos=0;
		$newpos = $lastpos+1;
		
		$thisTime =  time();
		
		$argu=NULL;
		$argu["POS"] = $newpos;
		$argu["KEY"] = $parx['key'];
		$argu["DATEX"]= date_unix2datestr($thisTime,1 );
		$argu["SH_NOTES"]= $parx['notes'];
		
		$assoclib = new  fAssocUpdate();
		$asocOpt = array('noAccCheck'=>1); // no access check needed, everybody is allowed
		$assoclib->setObj( $sqlo, 'CSUB_HAS_STAMP', $substance_id, $asocOpt );
		$assoclib->insert( $sqlo, $argu );
		
		
		
		/**
		 * touch container-position, to show the user, that something changed
		 * 
		 */
		if ($this->containerPos) {
			$CONTAINER_ID = $parx['cont_id'];
			$argu = array(	
				"MOD_DATE"	 => date_unix2datestr( $thisTime,1 )
			);
			$assoclib = new  fAssocUpdate();
			$assoclib->setObj( $sqlo, 'CONT_HAS_CSUBST', $CONTAINER_ID );
			$assoclib->setNoTouch(1); // do not touch container !
			$pkArr=array( "POS"=>$this->containerPos );
			$assoclib->update($sqlo, $argu, $pkArr, $updateOpt);
		}
		
	}
	
	function editCheck(&$sqlo, $parx) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->parx = $parx;
		$pos = $parx['pos'];
		$substance_id = $this->objid;
		
		/*
		$o_rights = access_check($sqlo, 'CONCRETE_SUBST', $substance_id);
		if ( !$o_rights["insert"]) {
			$error->set( $FUNCNAME, 5, "You do not have insert permission on this substance!" );
			return;		
		}
		*/
		if ( !$pos) {
			$error->set( $FUNCNAME, 5, "Pos missing!" );
			return;		
		}
		
		
		$sqlsel = '* from CSUB_HAS_STAMP where CONCRETE_SUBST_ID='.$substance_id.' and POS='.$pos;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadArray();
		$feats = $sqlo->RowData;
		if (!$this->go) {
			$this->parx['notes'] = $feats['SH_NOTES'];
		}
		$this->stampfeats = $feats;
	}
	
	function editForm($sqlo) {
		$parx= $this->parx;
		$pos = $parx['pos'];
		
		// $this->_html->line_out('time: '.$this->stampfeats['DATEX'].' key:'.$this->stampfeats['KEY']);
		
		$initarr   = NULL;
		$initarr["title"]       = "Add time stamp";
		$initarr["submittitle"] = "Save";
		$initarr["tabwidth"]    = "AUTO";
		
		$hiddenarr=array(
			'parx[key]'=>$parx['key'],
			'parx[cont_id]'=>$parx['cont_id'],
			'action'=>'STAMP',
			'id'=>$this->objid
		);
		$formobj = $this->_html->formInit($initarr, $hiddenarr);
		
		$fieldx = array ( 
			"title" => "Notes", 
			"name"  => "notes",
			"object"=> "text",
			"fsize"  => 60,
			"val"   => $parx["notes"], 
			"notes" => "optional notes of time stamp"
			 );
		$formobj->fieldOut( $fieldx );
		$formobj->close( TRUE );
	}
	
	/**
	 * do edit
	 * @param $sqlo
	 */
	function editDo(&$sqlo) {
		
	}
	
	function viewList(&$sqlo) {
	
		$this->_html->line_out('Show checkin/checkout events; used to track critical storage events.');
		$headx = array('#','time stamp', 'key', 'notes');
		$headOpt=array('title'=>'time stamps');
		$this->_html->_table3_init($headx, $headOpt);
		
		$sqlsel = 'POS, DATEX, KEY, SH_NOTES from CSUB_HAS_STAMP where CONCRETE_SUBST_ID='.$this->objid. ' order by DATEX ASC';
		$sqlo->Quesel($sqlsel);
		$cnt=0;
		while ($sqlo->ReadRow() ) {
			$tpos = $sqlo->RowData[0];
			$rowdat=array();
			// $tmpstr   = '<a href="p.php?mod=' .$this->_mod. '&action=EDITONE&id='.$this->objid.'&parx[pos]=' .$tpos. '">#'. ($cnt+1) . '</a>';
			$rowdat[] = ($cnt+1);
			$rowdat[] = $sqlo->RowData[1];
			$key      = $sqlo->RowData[2];
			$keyNice  = $key==2 ? 'out' : 'in';
			$rowdat[] = $keyNice;
			$rowdat[] = $sqlo->RowData[3];
			$this->_html->_table3_row($rowdat);
			$cnt++;
		}
		$this->_html->_table3_close();
	}
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$this->objid   = $_REQUEST['id'];
		$this->go      = $_REQUEST['go'];
		
		$parx  = $_REQUEST['parx'];
		$action= $_REQUEST['action'];
		$this->pos   = $pos;
		$sqlo   = &$this->_sqlo;
		
		if ($action==NULL) $action='view';
		
		$this->keyDefs = array ( 1=>'checkin', 2=>'checkout' );
		
		if ( $action=='STAMP') {
		
			$this->stampCheck($sqlo, $parx);
			if ($error->Got(READONLY))  {
				return;
			}
			if (!$this->go) {
				$this->editForm($sqlo);
				return;
			} else {
				$this->stampDo($sqlo);
				if ($error->Got(READONLY))  {
					return;
				}
			}
			
			if ($parx['cont_id']) {
				$this->_html->pageForward('edit.tmpl.php?t=CONTAINER&id='.$parx['cont_id'],'back to connected container');
			}
		} 
		
		if ( $action=='view') {
			$this->viewList($sqlo);
		}
		
		if ( $action=='EDITONE') {
		
			$this->editCheck($sqlo, $parx);
			if ($error->Got(READONLY))  {
				return;
			}
			if (!$this->go) {
				$this->editForm($sqlo);
				if ($error->Got(READONLY))  {
					return;
				}
			} else {
				// not yet implemented .....	
				$this->_html->pageForward('edit.tmpl.php?t=CONCRETE_SUBST&id='.$this->objid,'back to substance');
			}
		} 
		
	}
	
}
