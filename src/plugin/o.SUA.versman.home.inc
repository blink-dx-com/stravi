<?php
/**
 * o.ABSTRACT_SUBST > [VersMan] > home
 *    
 * @package o.SUA.versman.home.inc
 * @swreq   SREQ:0003324: o.ABSTRACT_SUBST > [VersMan] > Home einer Substanz 
 * @swreq   SREQ:0003314: o.ABSTRACT_SUBST > [VersMan] > VM aktivieren, editieren
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $_REQUEST['id'] SUA_ID
 * @param   $_REQUEST['act'] 
	['show']   : show table
	'activate' : activate VersMan
	'editrow'  : edit one version; 
		$id is CHILD_ID;
		need $parx['versnum'];
	    bei update: $_REQUEST['go']=1
 * 	0: prepare
 * 	1: do create
 *  @param $parx
 *    'this_master' : 0,1
 *  @version $Header: trunk/src/plugin/o.SUA.versman.home.inc 59 2018-11-21 09:04:09Z $  
 */

require_once 'lev1/o.SUA.versman.mod.inc';
require_once 'lev1/o.SUA.versman.sub.inc';
require_once 'gui/o.SUA.versman.gui.inc';

require_once ("visufuncs.inc");
require_once ("func_form.inc");
require_once ("insertx.inc");
require_once ("f.objview.inc");	
require_once 'glob.obj.copyobj1.inc';
require_once ("glob.obj.update.inc");


/**
 * edit Version params
 * @author steffen
 *
 */
class o_SUA_vers_editrow {
	
	private $childid;
	private $objid;
	private $_html;
	private $parx;
	private $vers_feats;
	
	public function __construct(&$sqlo, &$_html, $childid, $parx) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->childid = $childid;
		$this->objid = 0;
		$this->_html = &$_html;
		$this->parx = $parx;
		$this->sua_short = globTabGetSHORT('ABSTRACT_SUBST');
		
		$this->fields = array(
			'DATE_START'=>array( 'req'=>1,
				"notes"=>'format: YYYY-MM-DD, max:9999-12-01; Version gilt ab diesem Datum'), 
			'DATE_END'  =>array('req'=>1,
				"notes"=>'format: YYYY-MM-DD, max:9999-12-01; Version gilt bis einschliesslich diesem Datum'), 
			'NOTES'     =>array('req'=>1,
				"object"=>'textarea', 'colspan'=>2, 'inits'=>array('rows'=>10, 'cols'=>80)), 
		);
		
		$this->_html->messageBox('Info: Version Parameters','','INFO', 'If you want to edit the Valid-Date of the version: , 
			first you have to release the MAA (released or D_released)');
		$this->_html->line_out('');
		
		// get SUA-Info
		$vers_feats = glob_elemDataGet3( $sqlo, 'SUA_VERS', array( 'CHILD_ID'=>$this->childid ), 
			array('CHILD_ID','DATE_START', 'DATE_END', 'NOTES', 'ABSTRACT_SUBST_ID', 'XVERS') );
		$versmaster_id = $vers_feats['ABSTRACT_SUBST_ID'];
		
		if (!$versmaster_id) {
			$error->set( $FUNCNAME, 1, 'no VersionMaster found for Version '.$parx['versnum'] );
			return;
		}
		
		// @swreq SREQ:0003314:val03: User braucht Rollen-Rechte SUA: insert,write, 
		//        Objekt-Recht: insert-right auf MASTER-SUA
		$o_rights = access_check($sqlo, 'ABSTRACT_SUBST', $versmaster_id);
		if ( !$o_rights["insert"]) {
			$error->set( $FUNCNAME, 2, 'you need "insert"-rights for the VersionMaster [ID:'.$versmaster_id.']' );
			return;
		}
		
		
		$this->vers_feats = $vers_feats;
		$this->objid = $vers_feats['ABSTRACT_SUBST_ID'];
		
		$sua_vers_id = $this->childid;
		$objLinkLib  = new fObjViewC();
		$guiopt      = array("showID"=>1);
		$suaHtml = $objLinkLib->bo_display( $sqlo, 'ABSTRACT_SUBST', $sua_vers_id, $guiopt );
		
		
		$auditLib = new oAccLogC();
		$auditLib->setObject( $sqlo, 'ABSTRACT_SUBST', $sua_vers_id);
		$statusName = $auditLib->getLastLogName($sqlo);
		
		// ' <span class="yGgray">SUA:</span>'.$suaHtml.
		$this->_html->line_out('<span class="yGgray">Version:</span> '.$vers_feats['XVERS'].
		    ' <span class="yGgray">'.$this->sua_short.'-Status: </span>'.$statusName);
			
		if ($statusName!='released' and $statusName!='D_released' and $statusName!='R_released') {
			$error->set( $FUNCNAME, 3, 'The VersionChild must be "released", "D_released" oder "R_released" , to edit the  Version-parameters!' );
			return;
		}
	}
	
	public function getOriData() {
		return $this->vers_feats;
	}
	
	/**
	 * show edit form
	 * @param  $sqlo
	 */
	public function do_editForm(&$sqlo, $parx) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		
		// check status of SUA
		
		$initarr   = NULL;
		$initarr["title"]       = "Edit Version parameters"; // title of form
		$initarr["submittitle"] = "Update"; // title of submit button
		$initarr["tabwidth"]    = "AUTO";   // table-width: AUTO

		$hiddenarr = NULL; // hidden form variables
		$hiddenarr["id"]     = $this->childid; // is a required input parameter
		$hiddenarr["act"]    = 'editrow';
		$hiddenarr["parx[versnum]"] = $parx['versnum'];

		// init form, uses class formc
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
		
		
		// trim HH:MM:SS
		$parx['DATE_START'] = substr($parx['DATE_START'],0,10);
		$parx['DATE_END']   = substr($parx['DATE_END'],0,10);
		
		if ($parx['DATE_START']==NULL) $parx['DATE_START'] = date_unix2datestr( time(),3 );
		if ($parx['DATE_END']==NULL)   $parx['DATE_END']   = '9999-12-01';
		
		
		
		foreach( $this->fields as $column=>$iniField) {
			
			$colFeats = colFeaturesGet( $sqlo,  'SUA_VERS', $column);
			
			$fieldx = array ( // form-field definition
				"title"   => $colFeats['NICE_NAME'], 
				"name"    => $column,
				"object"  => "text",
				"val"     => $parx[$column], 
				"notes"   => $colFeats['COMMENTS']
				 );
				 
			if ( is_array($iniField)) {
				$fieldx = array_merge( $fieldx, $iniField);
			}
			$formobj->fieldOut( $fieldx ); // output the form-field
		}


		$formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	/**
	 * check overlap of dates with other versions
	 * @param  $sqlo
	 * @param  $unix1
	 * @param  $unix2
	 * @return $number of conflicts
	 */
	private function _checkDates(&$sqlo, $unix1, $unix2) {
		
		$formD1 = $sqlo->Timestamp2Sql($unix1);
		$formD2 = $sqlo->Timestamp2Sql($unix2);
		
		
		// check other Children of Master for date-range, but ignore the child itself
		$sqlsel = "count(1) from SUA_VERS where ABSTRACT_SUBST_ID=".$this->vers_feats['ABSTRACT_SUBST_ID'].
			' and (DATE_START<='.$formD2.' and DATE_END>='.$formD1.') and CHILD_ID!='.$this->childid;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$cnt = $sqlo->RowData[0];	
		return $cnt;
	}
	
	/**
	 * do update
	 * @param $sqlo
	 * @return int $success : <0: error, >0 o.k.
	 */
	public function do_editUpdate(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$parx = $this->parx;
		
		$errorarr = array();
		
		reset ($this->fields);
		foreach( $this->fields as $column=>$iniField) {
			// check fields
			
			
			
			switch ($column) {
				case 'DATE_START':
				    $iniField['dtype']='DATE1';
					break;
				case 'DATE_END':
				    $iniField['dtype']='DATE1';
					break;
				case 'NOTES':
				    $iniField['dtype']='STRING';
				    $iniField['maxlen']=4000;
					break;
			}
			
			
			$answer = formc::check_field( $iniField, $parx[$column] );
			if ($answer[0]<=0) {
				$errorarr[]='Attribut: '.$column.': '.$answer[1];
			}
		} 
		
		if (sizeof($errorarr)) {
			$this->_html->messageBox('Parameter-Error', '', 'WARN', implode('; ',$errorarr) ) ;
			$this->_html->line_out('');
			$this->do_editForm($sqlo, $parx);
			return -1;
		}
		
		$unix1 = date_str2unix($parx['DATE_START'], 5);
		$unix2 = date_str2unix($parx['DATE_END'], 5);
		
		
		if ($unix1>$unix2) {
			$errorarr[]='DATE_START muss gr&ouml;sser sein als DATE_END.';
		}
		
		// check for DATE-overlap
		// @swreq SREQ:0003314:val02: Check, das START_DATE,END_DATE nicht so schon existieren)
		$conflictCnt = $this->_checkDates($sqlo, $unix1, $unix2);
		if ($conflictCnt) {
			$errorarr[]='Konflikt mit anderen Versionen im G&uuml;ltigkeitszeitraum.';
		}
		
		if (sizeof($errorarr)) {
			$this->_html->messageBox('Parameter-Fehler', '', 'WARN', implode('; ',$errorarr) ) ;
			$this->_html->line_out('');
			$this->do_editForm($sqlo, $parx);
			return -1;
		}
		
		
		
		// update
		$assoclib = new  fAssocUpdate();
		$assoclib->setObj( $sqlo, 'SUA_VERS', $this->objid );
		
		$argu = array( 
			'DATE_START'=>$parx['DATE_START'],
			'DATE_END'  =>$parx['DATE_END'],
			'NOTES'     =>$parx['NOTES'] 
		);
		$idarr = array('CHILD_ID'=>$this->vers_feats['CHILD_ID']);
		$assoclib->update($sqlo, $argu, $idarr);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'error on updating version-params.' );
			return -2;
		}
		
		return 1;
	}
}





class o_SUA_versman_home_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'ABSTRACT_SUBST';
		$title		= '[VersMan] HOME';
		$sua_short = globTabGetSHORT('ABSTRACT_SUBST');
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title_sh'] = '[VersMan] HOME';
		$this->infoarr['title']    = 'VersionManagement - '.$sua_short.'-Home';
		
		//$this->infoarr['help_url'] = 'p129/xxxx'; // link im Wiki: 
		//$this->infoarr['help_base']= 'wiki';  // use wiki
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read','write','insert'), 'obj'=>array() )
				);	
		
	}
	
	private function active_form(&$sqlo) {
		// check status of SUA
		
		$parx = $this->parx;
		
		$initarr   = NULL;
		$initarr["title"]       = "Activate VersionManagement"; // title of form
		$initarr["submittitle"] = "Activate"; // title of submit button
		$initarr["tabwidth"]    = "AUTO";   // table-width: AUTO

		$hiddenarr = NULL; // hidden form variables
		$hiddenarr["id"]     = $this->objid; // is a required input parameter
		$hiddenarr["act"]    = 'activate';

		// init form, uses class formc
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);

		if ($parx['NOTES']==NULL) {
			$parx['NOTES'] = 'initial';
		}
		
		$fieldx = array ( // form-field definition
			"title"   => 'Notes', 
			"name"    => 'NOTES',
			"object"  => "textarea",
			"val"     => $parx['NOTES'], 
			"notes"   => 'Version-Notes',
			'req'	  => 1
		);
		$formobj->fieldOut( $fieldx ); // output the form-field
		
		$fieldx = array ( // form-field definition
		    "title"   => 'This MAA is the new Master',
		    "name"    => 'this_master',
		    "object"  => "checkbox",
		    "val"     => $parx['this_master'],
		    "inits"=>1,
		    "optional"=>1,
		    "notes"   => '... for EXPERTS only!',
		    
		);
		$formobj->fieldOut( $fieldx ); // output the form-field

		$formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	function initx(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->versSubLib = new oSUA_versman_sub();
		$this->versType = $this->versSubLib->getSUA_type($sqlo, $this->objid);
		
		$this->master_id = NULL;
		if ($this->versType==1) $this->master_id = $this->objid;
		if ($this->versType==2) {
			$this->versParams  = $this->versSubLib->getMainParams($sqlo, $this->objid);
			$this->master_id = $this->versParams['ABSTRACT_SUBST_ID'];	
		}
		
		
		$this->versGuiLib = new oSUA_gui();
	}
	
	/**
	 * create sua (V:1) from this SUA
	 * THIS sua == Master
	 * sua (V1) attached to master ...
	 * @param object $sqlo
	 */
	private function _do_this_to_master($sqlo, $sqlo2) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $this->_html->line_out('... Make this MAA to the master and create a new copy from this (this will be new V1)');
	    
	    $suaid = $this->objid;
	    $parx  = $this->parx;
	    $sua_feats = glob_elemDataGet3( $sqlo, 'ABSTRACT_SUBST', array( 'ABSTRACT_SUBST_ID'=>$suaid), array('NAME') );

	    
	    $copy_lib = new gObjCopyS();
	    $copy_opt=array();
	    $sua2 = $copy_lib->copyOne( $sqlo, $sqlo2, 'ABSTRACT_SUBST', $suaid, $copy_opt ); 
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 1, 'error on creation of new material Tempate (v1)' );
	        return;
	    }

	    
	    $new_name = $sua_feats['NAME'].' -PLM';
	    $args_up=array(
	        'vals'=> array(
	            'NAME'=> $new_name,
	            'VERSFLAG'=>1 
	        ),
	        
	    );
	    
	   
	    $UpdateLib = new globObjUpdate();
	    $UpdateLib->update_meta( $sqlo, 'ABSTRACT_SUBST', $suaid, $args_up );
	    
	    
	    $versModLib = new oSUA_versman_mod();
	    $versModLib->setSubst($sqlo, $sua2);
	    $params     = array( 'NOTES'=>$parx['NOTES'] );
	    $add_options= array("noRightCheck"=>1);
	    $versModLib->addToMaster($sqlo, $suaid, $params, $add_options);
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 1, 'error on creation of VersionMaster' );
	        return;
	    }
	    $this->_html->line_out('VersionManagent activated.');
	}
	
	private function _do_create_NEW_master($sqlo) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    // create master
	    $suaid = $this->objid;
	    $parx  = $this->parx;
	    $sua_feats = glob_elemDataGet3( $sqlo, 'ABSTRACT_SUBST', array( 'ABSTRACT_SUBST_ID'=>$suaid), array('NAME') );
	    
	    $inslib = new insertC();
	    
	    $args = array('vals'=>array(
	        'NAME' => $sua_feats['NAME'].' -PLM',
	        'VERSFLAG'=>1
	    ));
	    $master_id = $inslib->new_meta($sqlo, 'ABSTRACT_SUBST', $args);
	    
	    if (!$master_id) {
	        $error->set( $FUNCNAME, 1, 'error on creation of VersionMaster' );
	        return;
	    }
	    
	    $versModLib = new oSUA_versman_mod();
	    $versModLib->setSubst($sqlo, $suaid);
	    $params     = array( 'NOTES'=>$parx['NOTES'] );
	    $add_options= array("noRightCheck"=>1);
	    $versModLib->addToMaster($sqlo, $master_id, $params, $add_options);
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 1, 'error on creation of VersionMaster' );
	        return;
	    }
	    
	    $this->_html->line_out('VersionManagent activated.');
	}

	
	/**
	 * activate master
	 * @param object $sqlo
	 * @param number $make_this_to_master
	 */
	public function do_activate(&$sqlo, $sqlo2, $make_this_to_master=0) {
		
		
		if ($this->versType) return; // is already activated
		
		
		if ($make_this_to_master) {
		    $this->_do_this_to_master($sqlo, $sqlo2);
		} else {
		    $this->_do_create_NEW_master($sqlo);
		}
		
		
		
	}
	
	
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$this->objid   = $_REQUEST['id'];
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
		$sqlo = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		$this->initx($sqlo, $parx);
		
		if ($_REQUEST['act']=='activate') {
			if ( !$go ) {
				$this->active_form($sqlo);
				return;
			} else {
				
				if (trim($parx['NOTES'])==NULL) {
					$this->htmlFoot('ERROR', 'Bitte Notes eingeben.');
				}
				
				echo "DDD: PP:".print_r($parx,1)."<br>";
				$this->do_activate($sqlo, $sqlo2, $parx['this_master'] );
				if ($error->Got(READONLY))  {
					$error->printAll();
					$this->active_form($sqlo);
					return;
				}
				$this->_html->pageForward('edit.tmpl.php?t=ABSTRACT_SUBST&id='.$this->objid, 'object');
			}
			
		}
		if ($_REQUEST['act']=='editrow') {
		    $editRowLib = new o_SUA_vers_editrow($sqlo, $this->_html, $this->objid,  $parx );
			if ($error->Got(READONLY))  {
				$error->printAllEasy();
				return;
			}
				
			if (!$go) {
				$oridata = $editRowLib->getOriData();
				$editRowLib->do_editForm($sqlo, $oridata);
				if ($error->Got(READONLY))  {
					$error->printAll();
					return;
				}
				return;
			}
			
			$action_ok = $editRowLib->do_editUpdate($sqlo);
			if ($error->Got(READONLY))  {
				$error->printAll();
				return;
			}
			if ($action_ok>0) {
				$this->_html->pageForward('edit.tmpl.php?t=ABSTRACT_SUBST&id='.$this->objid, 'object');
			}
			return;
		}
		
		$this->_html->line_out( 'Action not defined.');
		
	}
	
}