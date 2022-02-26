<?php
/**
 * show selected  VARIO-columns of selected objetcs of a table
 *   $_SESSION['s_formState']['g.objtab.s_vario.sh'][TABLE]=array(vario=>array of columns)
 * @package g.objtab.s_vario.sh.inc
 * @swreq UREQ:5176
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   
 *  $_REQUEST['t'] - tablename
	$_REQUEST['go'] - 0,1 : submitted vario columns
	$_REQUEST['parx']
	  'onlyvario' - 0,1
	  'keys' - array of varion column-KEYs (array KEY => 0,1)
	$_REQUEST['showForm'] : 0,1 show from ?
 * @version $Header: trunk/src/plugin/g.objtab.s_vario.sh.inc 59 2018-11-21 09:04:09Z $
 */
require_once ('gui/glob.objtab.showList.inc');
require_once 'o.S_VARIO.subs.inc';

/**
 * worker class
 * @author steffen
 *
 */
class o_GLOB_VARIO_Help extends gShowObjList {

    function initExtend(&$sqlo, $options=NULL) {
		$tablename    = $this->tablename;
		$this->pkname = PrimNameGet2($tablename);
		
		$this->variolib = new oS_VARIO_sub($tablename);
	}
	

	public function oneRowExtend(&$sqlo, $rawData) {
		
		$objid = $rawData[$this->pkname];
		
		// get VARIO-VALS
		$dataarr = array();

		
		foreach($this->coldesc as $key=>$dummy) {
			if (substr($key,0,2)=='y.') {
				
				$variokey = substr($key,2);
				$dataarr[$key] = $this->variolib->getValByKey( $sqlo, $objid, $variokey );
			}
		}
		
		return $dataarr;
	}

}



class g_objtab_s_vario_sh_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= $_REQUEST['t'];
		$title		= 'Show VARIO-columns';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
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
			'listChkSelect' => 1
		);
				
		
	}
	
	private function searchForm($key) {
		
		$outtext=NULL;
		
		$tmp_arr=array();
		$tmp_arr['']           = '---';
		$tmp_arr['=']           = '=';
		$tmp_arr['LIKE']        = 'LIKE';
		$tmp_arr['NOT LIKE']    = 'not LIKE';
		$tmp_arr['>']           = '&gt';
		$tmp_arr['<']           = '&lt;';
		$tmp_arr['>=']          = '&gt;=';
		$tmp_arr['<=']          = '&lt;=';
		$tmp_arr['!=']          = '!=';
		$tmp_arr['is NULL']     = 'NULL';
		$tmp_arr['is NOT NULL'] = 'not NULL';
		$tmp_arr['NOT LIKE strict'] = 'not LIKE strict';
		
		
		$outtext = '<select name="bool['.$key.']">'."\n";
		foreach( $tmp_arr as $cond_tmp=>$cont_txt) {
			$selected="";
			if ($cond_tmp == $tmp_cond_last) $selected=" selected";
			$outtext .= "<option value=\"".$cond_tmp."\" ".$selected.">".$cont_txt."\n";
		}
		
		$outtext .= "</select>\n";
		
		$outtext .= '<input name="searchtxt['.$key.']">';
		
		return $outtext;
	}
	
	/**
	 * show vario form
	 * @param unknown $sqlo
	 */
	private function form1(&$sqlo) {
		
		$oldvars = $_SESSION['s_formState']['g.objtab.s_vario.sh'][$this->tablename]['vario'];
		$old_varioKeys = $oldvars['keys'];
		
		$variolib = new oS_VARIO_sub($this->tablename);
		$colarray =  $variolib->getAllKeysNice( $sqlo );
		
		if (!sizeof($colarray)) {
			echo ' No VARIO-columns for this table defined.<br>';
			return;
		}
		
		$initarr = NULL ;
		$initarr ["title"]       = "Select vario columns"; // title of form
		$initarr ["submittitle"] = "Show columns"; // title of submit button
		$initarr ["tabwidth"]    = "AUTO"; // table - width : AUTO
		$hiddenarr = NULL ;
		$hiddenarr ["t"] = $this->tablename ; // is a required input parameter
		
		$formobj = $this->_html->formInit( $initarr , $hiddenarr , 0);
		
		$fieldx = array (
				"title" => 'Select VARIO-entries only',
				"name"  => 'onlyvario',
				"object"=> "checkbox",
				"val"   => $oldvars['onlyvario'],
				"inits" => 1,
				"notes" => 'show only rows with vario content?'
		);
			
		$formobj->fieldOut( $fieldx );
		
		$fieldx = array (
				"object"=> "hr",
		);
		$formobj->fieldOut( $fieldx );
		
		foreach($colarray as $key=>$nice) {
			
			$features = $variolib->getColFetaures( $sqlo, $key );
			
			$oldval = 0;
			if (isset($old_varioKeys[$key]) and $old_varioKeys[$key]>0 ) $oldval = 1;
			
			$moretext = $this->searchForm($key);
			
			$fieldx = array (
					"title" => $nice,
					"name"  => 'parx[keys]['.$key.']',
					"object"=> "checkbox",
					"val"   => $oldval,
					"inits" => 1,
					"namex" => TRUE,
					"colspan"=>"2",
					"notes" => $moretext. ' '. $features['NOTES']. '; Code:'.$key
			);
			
			$formobj->fieldOut( $fieldx ); 
		}
		
		$formobj -> close ( TRUE ); // close form , sshow submit - button
		
	}
	
	private function getMoreConditions(&$sqlo, $parx, $pkName) {
		$tablename    = $this->tablename;
		if (!sizeof($parx['keys'])) return;
		
		
		$cond=NULL;
		
		foreach($parx['keys'] as $key=>$val) {
			
			
			if ($_REQUEST['bool'][$key]!=NULL) {
				$search_val = NULL;
				if ( trim($_REQUEST['searchtxt'][$key] )!=NULL) {
					$search_val = trim($_REQUEST['searchtxt'][$key]);
					$search_val = $sqlo->addQuotes($search_val);
				}
				
				$cond .= ' and '.$pkName.' in ('.
					'select OBJ_ID from S_VARIO where TABLE_NAME='.
					$sqlo->addQuotes($tablename). ' AND ( KEY='.$sqlo->addQuotes($key).' and VALUE '. $_REQUEST['bool'][$key] .' '.$search_val. ')'.
					' )';
			}
			
		}
				
		return $cond;		
		
	}
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		

		$sqlo = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		$tablename = $_REQUEST['t'];
		$go        = $_REQUEST['go'];
		$parx      = $_REQUEST['parx'];
		$showForm  = $_REQUEST['showForm'];
		
		$this->tablename = $tablename;
		
		if (isset($parx) and sizeof($parx)) {
			// store column settings
			$_SESSION['s_formState']['g.objtab.s_vario.sh'][$this->tablename]['vario'] = $parx;
		}
		
		$oldvars = $_SESSION['s_formState']['g.objtab.s_vario.sh'][$this->tablename]['vario'];
		
		$this->sqlAfter = 'from ' . $this->listLib->getSqlAfterNoOrder();
		
		
		$numPKs = countPrimaryKeys($tablename);
		if ($numPKs!=1) {
			$this->htmlFoot('ERROR', 'Only table supported with ONE Primary Key.');
		}
		
		$pkName = PrimNameGet2($tablename);
		$imName = importantNameGet2($tablename);
		
		if ($showForm or !isset($oldvars) ) {
			$this->form1($sqlo);
			return;
		} else {
			echo '[<a href="p.php?mod='.$this->_mod.'&t='.$tablename.'&showForm=1">Show Column Selector</a>]'."<br>\n";
		}
		
		if (!isset($parx) and isset($oldvars)) {
			// get parx from SESSION
			$parx = $oldvars;
		}
		
		if ($parx['onlyvario']) {
			
			$moreCond = $this->getMoreConditions($sqlo, $parx, $pkName);
			$this->sqlAfter .= ' and '.$pkName.' in (select OBJ_ID from S_VARIO where TABLE_NAME='.$sqlo->addQuotes($tablename) .')' . $moreCond ;
			
			// echo "DEBUG: ".$this->sqlAfter ."<br>";
		}
		
		
		$this->showkeys = array(
				'x.'.$pkName => array('sort'=>1),
				'x.'.$imName => array('sort'=>1),
		);
		
		$variolib = new oS_VARIO_sub($this->tablename);
		
		if (sizeof($parx['keys'])) {
			foreach($parx['keys'] as $key=>$val) {
				$features = $variolib->getColFetaures( $sqlo, $key );
				$this->showkeys['y.'.$key] = array('nice'=>$features['NICE']);
			}
			
			
		}
		
		$showLib = new o_GLOB_VARIO_Help();
		
		
		$options = array(
				'entryPerPage' => 30,
				'URL'  => "p.php?mod=".$this->_mod.'&t='.$tablename,
				'title'=> 'VARIO list',
				'sql.select'=> 'x.*, a.* ',
				'sql.order' => 'x.'.$pkName.' DESC',
				'modname'   => $this->_mod
		);
		
		$showLib->init( $sqlo, $tablename, $this->showkeys, $this->sqlAfter, $options);
		
		$showLib->showList($sqlo, $sqlo2);
		
	}
	
}