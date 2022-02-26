<?php
/**
 * edit one container entry 
 *    
 * @package o.CONTAINER.modone.inc 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq SREQ:0001117: o.CONTAINER > ONE container: edit one entry, delete a selection 
 * @param   $_REQUEST['id'] id of CONTAINER
 * @param   $_REQUEST['go'] 0,1
 * @param   $_REQUEST['act'] action
 *   'mod'   : "take out" quantity
 *   'edit'  : modify one entry, all attributes
 *   'delli' : delete many entries; need param $posar
 *   'REOPEN_SUC' reopen a suc
 *    FINISH_SUC finish SUC
 * @param   $_REQUEST['posar'] array of positions, needed for act=delli
 *   for act=mod ....
 * @param   $_REQUEST['pos']  pos; needed for act='mod'
 * @param   $_REQUEST['parx'] array of params; needed for act='mod' 
 *    'takeoutquant'
 *    'substid' : needed for act=REOPEN_SUC

 */

require_once ('f.msgboxes.inc'); 
require_once ("f.objview.inc");	
require_once ("o.CONTAINER.mod.inc");
require_once 'o.H_UNIT.siSubs.inc';
require_once ('func_form.inc');

require_once ("lev1/o.CCT_ACCESS.reopen.inc");


/**
 * edit one entry
 * @author steffen
 * @swreq   UREQ:0001117: o.CONTAINER > edit one container entry 
 */
class o_CONTAINER_modoneX {
    
    private $oldFeats;
    
    function __construct($sqlo, $contid) {
        $this->objid = $contid;
        
        $params=array();
        $this->cont_raw_lib = new  oContainerModC ($params);
        $this->cont_raw_lib->setContainer($sqlo, $this->objid);
        $this->cont_raw_lib->activateModi($sqlo);
        
        $this->_stop_forward=0;
        $this->suc_table_nice =  tablename_nice2('CONCRETE_SUBST') ;
    }
	
	function init(&$sqlo, $pos, &$_html) {
		$this->_html = &$_html;
		
		$this->pos   = $pos;
		
		$this->formFields = array(
			array("name"=>'CONCRETE_SUBST_ID', 'view'=>1),
			array("name"=>'QUANTITY', 'view'=>0 ),
			array("name"=>'NOTES',    'view'=>0 ),
			array("name"=>'ALIQUOT_NO', 'view'=>0 ),
			array("name"=>'MOD_DATE', 'view'=>1 )
		);
		
		$this->getOldParams($sqlo);
		
		return $this->oldFeats;
	}
	
	function stop_forward() {
	    return $this->_stop_forward;
	}
	
	
	
	private function _check_finished($sqlo) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $suc_id = $this->cont_raw_lib->get_SUC_ID();
	    if (!$suc_id) {
	        $error->set( $FUNCNAME, 1, 'SUC-ID not got from lib.' );
	        return;
	    }
	    $quant_new =$this->cont_raw_lib->get_SUC_quant($sqlo, $suc_id);
	    debugOut("quant_new:$quant_new", $FUNCNAME, 1);
	    if ($quant_new > 0 ) {
	        return;
	    }
	    
	    $was_finished = $this->cont_raw_lib->has_status($sqlo, 'finished', $suc_id);
	    debugOut("was_finished:$was_finished", $FUNCNAME, 1);
	    
	    if ( !$was_finished ) {
	        
	        
	        echo '<br>';
	        cMsgbox::showBox("ok", 'The stock of this '.$this->suc_table_nice.' [ID:'.$suc_id.'] is empty now.');
	        echo '<br>';
	        
	        $initarr   = array();
	        $initarr["action"]      = 'p.php?mod=DEF/o.CONTAINER.modone'; // !!! other module ..
	        $initarr["title"]       = "Do you want to flag the ".$this->suc_table_nice .' as finished?';
	        $initarr["submittitle"] = "Set finished";
	        $initarr["tabwidth"]    = "AUTO";
	        
	        $hiddenarr = array();;
	        $hiddenarr["id"]=$this->objid;
	        $hiddenarr["parx[substid]"]=$suc_id;
	        $hiddenarr["act"]='FINISH_SUC';
	        
	        $formobj = new formc( $initarr , $hiddenarr , 0);
	        
// 	        $fieldx = array (
// 	            "title" => "confirm reopen",
// 	            "name"  => "accept",
// 	            "object"=> "checkbox",
// 	            "notes" => 'Please confirm this action',
	            
// 	        );
// 	        $formobj->fieldOut( $fieldx );
	        
	        $formobj->close( TRUE );
	        
	        $this->_stop_forward=1;
	    }
	}
	
	private function getOldParams(&$sqlo) {
		$sqlsel = '* from CONT_HAS_CSUBST where CONTAINER_ID='.$this->objid.' and POS='.$this->pos;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadArray();
		$this->oldFeats   = $sqlo->RowData;
	}
	
	function get_old_suc() {
	    return $this->oldFeats['CONCRETE_SUBST_ID'];
	}
	
	
	/**
	 * 
	 * @param $parx
	 */
	function setParams($parx) {
		$this->entryFeats = $parx;
	}
	
	private function manageOneRow(&$sqlo, $valarr) {
		$notes_loop = NULL;
		$objType    = 'text';
		$attrName = $valarr['name'];
		$valTmp   = $this->entryFeats[$attrName];
		
		
			
		$attrNice =  columnname_nice2('CONT_HAS_CSUBST', $attrName);
		if ($attrName=='CONCRETE_SUBST_ID') {
			$objType='info2';
			$objlnkLib = new fObjViewC();
			$valTmp = $objlnkLib->bo_display( $sqlo, "CONCRETE_SUBST", $this->oldFeats[$attrName] );
		}
			
		if ($attrName=='QUANTITY') {
		
			$suc_id = $this->oldFeats['CONCRETE_SUBST_ID'];
			if ($suc_id) {
    			$sua_id = glob_elementDataGet( $sqlo, 'CONCRETE_SUBST', 'CONCRETE_SUBST_ID', $suc_id, 'ABSTRACT_SUBST_ID');
    			if ($sua_id) {
    				$measUnitLib = new oH_UNIT_siSubs();
    				$unitName = $measUnitLib->getUnitNameFromSUA($sqlo, $sua_id);
    					
    				if ($unitName!=NULL) {
    					$notes_loop = '['.$unitName.']';
    				}
    			}
			}
		}
			
		$fieldx = array (
				"title" => $attrNice,
				"name"  => $attrName,
				"object"=> $objType,
				"val"   => $valTmp,
				"view"	=> 0,
				"notes"	=> $notes_loop
		);
		
		$fieldx = $valarr + $fieldx;

		return $fieldx;
	}
	
	function formEdit(&$sqlo) {
		$initarr = NULL ;
		$initarr ["title"] = "Set parameters"; // title of form
		$initarr ["submittitle"] = "Update"; // title of submit button
		$initarr ["tabwidth"] = "AUTO";
		$initarr ["tabnowrap"] = 1; 
		
		$hiddenarr = NULL;
		$hiddenarr['id']  = $this->objid;
		$hiddenarr['pos'] = $this->pos;
		$hiddenarr['act'] = 'edit';
		$formobj = $this->_html->formInit ( $initarr , $hiddenarr , 0);
		
		
		
		foreach( $this->formFields as $valarr) {
			
			$fieldx = $this->manageOneRow($sqlo, $valarr);
			$formobj -> fieldOut ( $fieldx ); 
		}
		
		
		$formobj->close ( TRUE ); // close form , sshow submit - button
	}
	
	function formTakeout(&$sqlo) {
		$initarr = NULL ;
		$initarr ["title"] = "Take out quantity"; // title of form
		$initarr ["submittitle"] = "Take out!"; // title of submit button
		$initarr ["tabwidth"] = "AUTO";
		$initarr ["tabnowrap"] = 1;
	
		$hiddenarr = NULL;
		$hiddenarr['id']  = $this->objid;
		$hiddenarr['pos'] = $this->pos;
		$hiddenarr['act'] = 'mod';
		$formobj = $this->_html->formInit ( $initarr , $hiddenarr , 0);
	
		//$objlnkLib = new fObjViewC();
		
		$formFields = array(
				array("name"=>'CONCRETE_SUBST_ID', 'view'=>1 ),
				array("name"=>'QUANTITY',          'view'=>1, 'title'=> 'Current quantity')
				);
		

		foreach( $formFields as $valarr) {
			
			$fieldx = $this->manageOneRow($sqlo, $valarr);
			$formobj -> fieldOut ( $fieldx ); 
		}
		
		
		$fieldx = array (
				"title" => 'Take out quantity',
				"name"  => 'takeoutquant',
				"object"=> 'text',
				"val"   => NULL,
				"notes"	=> 'Entnahmemenge'
		);
		$formobj -> fieldOut ( $fieldx );
		
	
		$formobj->close ( TRUE ); // close form , sshow submit - button
		
		echo '<br>';
		echo '[<a href="p.php?mod=DEF/o.CONTAINER.modone&id='.$this->objid.'&act=edit&pos='.$this->pos.'">Edit all parameters of entry</a>]';
	}
	
	function update(&$sqlo) {
		
		$pos = $this->pos;
		
		$this->cont_raw_lib->updateAliquot($sqlo, $pos, $this->entryFeats);
		$this->_check_finished($sqlo);
	}
	
	/**
	 * take out a quantity
	 * @param object $sqlo
	 * @param float $takeoutquant
	 */
	function takeout($sqlo, $takeoutquant) {
	    
	    $newparams=array();
	    $new_quant = $this->oldFeats['QUANTITY'] - $takeoutquant;
	    if ($new_quant<0) {
	        $new_quant = 0;
	    }
	    
	    if ($new_quant>0) {
	        
    	    $newparams['QUANTITY'] = $new_quant;
    	    $this->setParams($newparams);
    	    $this->update($sqlo);
    	    
	    } else {
	        // delete POS
	        $pos = $this->pos;
	        
	        $this->cont_raw_lib->delpos($sqlo, $pos);
	    }
	    
	    $this->_check_finished($sqlo);
	}
	
	function finish_set($sqlo, $parx) {
	    
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $tablename='CONCRETE_SUBST';
	    $suc_id = $parx['substid'];
	    if (!$suc_id) {
	        $error->set( $FUNCNAME, 1, 'Input SUC-ID missing.' );
	        return;
	    }
	    
	    
	    $key_status='finished';
	    
	    echo '... do finish<br>'."\n";
	    
	    $this->accLogLib    = new oAccLogC();
	    $this->accLogLib->setObject( $sqlo, $tablename, $suc_id );
	    $last_status_name = $this->accLogLib->getLastLogName($sqlo);
	    
	    if ($key_status==$last_status_name) {
	        $error->set( $FUNCNAME, 1, 'Current Status is already "'.$key_status.'"');
	        return;
	    }
	    
	   
	    
	    $this->cont_raw_lib->set_suc_finished($sqlo, $suc_id);

	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 1, 'Error on finish' );
	        return;
	    }
	    
	    echo '<br>';
	    cMsgbox::showBox("ok", 'Did set status of '.$this->suc_table_nice.' [ID:'.$suc_id.'] to '.$key_status.'.');
	    echo '<br>';
	}
	

	
	/**
	 * reopen SUC
	 * @param object $sqlo
	 */
	function reopen($sqlo, $parx) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $tablename='CONCRETE_SUBST';
	    $suc_id = $parx['substid'];
	    if (!$suc_id) {
	        $error->set( $FUNCNAME, 1, 'Input SUC-ID missing.' );
	        return;
	    }
	    
	    if ( !$parx['accept'] ) {
	        $error->set( $FUNCNAME, 1, 'You must accept the reopen actiong.' );
	        return;
	    }
	    
	    $key_status='finished';
	    
	    echo '... do reopen<br>'."\n";
	    
	    $this->accLogLib    = new oAccLogC();
	    $this->accLogLib->setObject( $sqlo, $tablename, $suc_id );
	    $last_status_name = $this->accLogLib->getLastLogName($sqlo);
	    
	    if ($key_status!=$last_status_name) {
	        $error->set( $FUNCNAME, 1, 'Current Status must be "'.$key_status.'"');
	        return;
	    }
	    
	    $cct_access_id = glob_elementDataGet( $sqlo, $tablename, 'CONCRETE_SUBST_ID', $suc_id, 'CCT_ACCESS_ID');
	    
	    $reopenLib = new gObjAccReopen($sqlo);
	    $args=array('notes'=>'stock finished mistake');
	    $re_opt=array('ignoreRights'=>1);
	    $reopenLib->doReopen($sqlo, $cct_access_id, $tablename, $suc_id, $args, $re_opt);
	    
	    $accLogInfo    = $reopenLib->getInfo();
	    
	    if (!empty($accLogInfo)) {
	        echo "<br>";
	        htmlInfoBox( "Info", "", "open", "INFO" );
	        echo implode("<br>",$accLogInfo);
	        htmlInfoBox( "", "", "close" );
	    }
	    
	    
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 1, 'Error on reopen' );
	        return;
	    }
	    
	    echo '<br>';
	    cMsgbox::showBox("ok", 'Did reopen of '.$this->suc_table_nice.' [ID:'.$suc_id.'].');
	    echo '<br>';
	    
	}
	
}


class o_CONTAINER_modone_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'CONTAINER';
		$title		= 'edit one entry';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		$this->infoarr['obj_more'] = 'pos: '.$_REQUEST['pos'];
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read','write'), 'obj'=>array('insert') )
				);
				
		
	}
	
	private function delInit(&$sqlo) {
		
	}
	
	private function delask(&$sqlo) {
		// special forms ....
		require_once ("func_formSp.inc");
		
		$numsel = sizeof($this->posarr);
		
		$formLib = new formSpecialc();
		$title = "Remove from container";
		$asktext = "Do you want remove ".$numsel." substances from the container?";
		$delurl  = 'p.php?mod='.$this->_mod;
		
		$params=NULL;
		$params["go"] = 1;
		$params["act"]= 'delli';
		$params["id"] = $this->objid;
		
		foreach( $this->posarr as $key=>$val) {
			$params["posar[".$key."]"]=$val;
		}
		reset ($this->posarr); 
		
		$formLib->deleteForm( $title, $asktext, $delurl, $params );
	}
	
	private function del_do(&$sqlo) {
		global $error;
		//$FUNCNAME= __CLASS__.':'.__FUNCTION__;


		$cnt=0;
		foreach( $this->posarr as $pos=>$val) {
			if ($val) {
				$this->cont_raw_lib->delpos($sqlo, $pos);
				if ($error->Got(READONLY))  {
					return;
				}
				$cnt++;
			}
		}
		
		$this->_html->line_out( $cnt.' substances removed.');
		
    }
    
    

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$go  = $_REQUEST['go'];
		$act = $_REQUEST['act'];
		$this->objid = $_REQUEST['id'];
		$parx = $_REQUEST['parx'];
		
		$sqlo = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		
		$contModLib = new o_CONTAINER_modoneX($sqlo, $this->objid);
		
		switch ($act) {
			
			case 'mod': // takout only
				$pos   = $_REQUEST['pos'];
				if (!$pos) {
					$this->htmlFoot('ERROR','pos missing');
				}

				$oldFeats = $contModLib->init($sqlo, $pos, $this->_html);
				if (!$oldFeats['CONCRETE_SUBST_ID']) {
				    $this->htmlFoot('ERROR','No MAC exists on this Pos.');
				}
				
				if (!$go) {
				    
					$contModLib->setParams($oldFeats);
					$contModLib->formTakeout($sqlo);
					
				} else {
					
				    if ($parx['takeoutquant']<=0) {
						$this->htmlFoot('ERROR','No valid "take out quantity" given.');
					}
					
					$contModLib->takeout($sqlo, $parx['takeoutquant']);
					
					
					if ($error->Got(READONLY))  {
						$error->printAll();
						$contModLib->formTakeout($sqlo);
						return;
					} else {
					    
					    $suc_id = $contModLib->get_old_suc();
						$this->_html->line_out('o.k.');
						
						if (!$contModLib->stop_forward()) {
    						//$this->_html->pageForward('edit.tmpl.php?t=CONTAINER&id='.$this->objid, 'container');
    						$this->_html->pageForward('edit.tmpl.php?t=CONCRETE_SUBST&id='.$suc_id, tablename_nice2('CONCRETE_SUBST') );
    						return;
						}
					}
				}
				
				
				break;
				
			case 'edit':
					$pos   = $_REQUEST['pos'];
					if (!$pos) {
						$this->htmlFoot('ERROR','pos missing');
					}
				
					
					$oldFeats = $contModLib->init($sqlo,  $pos, $this->_html);
				
					if (!$go) {
						$contModLib->setParams($oldFeats);
					} else {
						$contModLib->setParams($_REQUEST['parx']);
						$contModLib->update($sqlo);
						if ($error->Got(READONLY))  {
							$error->printAll();
							$contModLib->formEdit($sqlo);
							return;
						} else {
							$this->_html->line_out('o.k.');
							$this->_html->pageForward('edit.tmpl.php?t=CONTAINER&id='.$this->objid, 'container');
							return;
						}
					}
				
					$contModLib->formEdit($sqlo);
					break;
			case 'delli':
				$this->posarr   = $_REQUEST['posar'];
				if (!sizeof($this->posarr)) {
					$this->htmlFoot('ERROR','No entries selected.');
				}
				$this->delInit($sqlo);
				
				if (!$go) {
					$this->delask($sqlo);
				} else {
					$this->del_do($sqlo);
					if ($error->Got(READONLY))  {
						$error->printAll();
						
						return;
					} else {
						$this->_html->line_out('o.k.');
						$this->_html->pageForward('edit.tmpl.php?t=CONTAINER&id='.$this->objid, 'container');
						return;
					}
				}
				break;
				
			case 'REOPEN_SUC':
			    $contModLib->reopen($sqlo, $parx);
			    break;
			    
			case 'FINISH_SUC':
			    $contModLib->finish_set($sqlo, $parx);
			    $this->_html->pageForward('edit.tmpl.php?t=CONCRETE_SUBST&id='.$parx['substid'], 'MAC');
			    break;
			default:
			    
			    $this->htmlFoot('ERROR','Action "'.$act.'" unknown.');
			    
		}
	}
	
}
