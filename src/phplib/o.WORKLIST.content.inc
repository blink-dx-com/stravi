<?php 
require_once ('o.WORKLIST.subs.inc');
require_once ("o.ABS_WORKLIST.subs.inc");
require_once ("o.ABS_WORKLIST.subs2.inc");

/**
 * @package o.WORKLIST.content.inc
 * @swreq UREQ:0000932 db.WORKLIST: Messauftrag organisieren
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version  Exp $
 *
 */
class oWORKLIST_oneRow {

	var $rowArr; // array(xpos => array( $table, $objid, $val ))
	var $_collectErrors; // array of errors
	var $viewTypeColArr; // special columns

	function __construct( &$sqlo, &$absEntries, $WORKLIST_ID, $ABS_WORKLIST_ID ) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;

		$this->absEntries     = $absEntries;
		$this->viewTypeColArr = array();
		$this->objLinkLib     = new fObjViewC();
		$this->absWorkLib     = new oABS_WORKLIST_subs2();
		$this->absWorkId      = $ABS_WORKLIST_ID;
		$this->wl_pluginLib   = NULL;
		$this->_collectErrors = array();
		$this->_butFieldCnt   = 0;

		if ($this->absWorkId) {
			$this->absWorkLib1 = new oABS_WORKLIST_subs($this->absWorkId);
			$this->colOrder = $this->absWorkLib1->getColOrder($sqlo);
				
			// check for Plugin-Class
			$wl_plugin_name = $this->absWorkLib1->get_wl_plugin($sqlo);
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 1, 'Error on WL-Plugin-Analysis for WLA:'.$this->absWorkId );
				return;
			}
			if ($wl_plugin_name!=NULL) {
				$this->wl_pluginLib = new $wl_plugin_name($WORKLIST_ID);
			}
				
			$this->_fill_TypeArr();
		}

	}

	/**
	 * fill $this->viewTypeColArr
	 */
	private function _fill_TypeArr() {
		// analyse $this->absEntries
		if (is_array($this->absEntries)) {
			foreach( $this->absEntries as $xpos=>$valarr) {
				if ($valarr['typ']=='VIEW') {
					if ($valarr['collect']!=NULL) {
						$sqlPrepArr = $this->absWorkLib->anaViewSqlStr($valarr['collect']);
						$this->viewTypeColArr[$xpos]=$sqlPrepArr;
					}
				}
				if ($valarr['opt']!=NULL) {
					$this->absEntries[$xpos]['optarr']= $this->absWorkLib->option2arr($valarr['opt']);
				}
			}
			reset ($this->absEntries);
		}
	}

	/**
	 * get collected errors
	 */
	function getErrors() {
		return $this->_collectErrors;
	}

	/**
	 * init edit actions
	 */
	function initEdit( &$clipArr, $editColID ) {
		$this->clipArr   = $clipArr;
		$this->editColID = $editColID;
	}

	function setRowData(&$rowArr) {
		$this->rowArr = &$rowArr;
	}

	/**
	 * get view data; prepared statement in $this->viewTypeColArr
	 * @param  $sqlo
	 * @param int $xpos
	 * @param array $rowArr
	 * @return string
	 */
	function _getViewData(&$sqlo, $xpos, $colValArr) {
		global $error;
		if ($this->viewTypeColArr[$xpos]==NULL) return;

		if (!sizeof($colValArr)) return;

		$objIdArr = NULL;
		foreach( $colValArr as $pos=>$valarr) {
			$objIdArr[$pos]=$valarr[1];
		}
		reset ($colValArr);

		$sqlPrepArray = $this->viewTypeColArr[$xpos]; // array('sqlCond'=>$condition, 'cols'=>$colPosArr)
		$answer  = $this->absWorkLib->createSqlAfter($sqlPrepArray['sqlCond'], $sqlPrepArray['cols'], $objIdArr);
		$sqlsel  = $answer['sql'];
		$tmperror= $answer['error'];
		if ($sqlsel==NULL) return;

		$answer = $sqlo->Quesel($sqlsel);
		if ($error->Got(READONLY))  {
			$this->viewTypeColArr[$xpos] = NULL; // !!! remove the query
			$this->_collectErrors[] = $error->getAllAsText();
			$error->reset();    // clear the error !
			return '<span style="color:red">Error on query! Stop this column-analysis</span>';
		}
		$sqlo->ReadRow();
		$answer = $sqlo->RowData[0];
		return $answer;
	}

	// get plugin data
	function _getPluginData(&$sqlo, $colname, &$rowArr) {
		if ($this->wl_pluginLib===NULL) return;

		$result = $this->wl_pluginLib->col_view($sqlo, $colname, $rowArr);
		return $result;
	}

	// get object name
	function _getObjName(&$sqlo, $oneentry) {
		$objViewOpts=array("noIcon"=>1);
		if ($oneentry[0]!=NULL and $oneentry[1]>0) {
			$tablename = $oneentry[0];
			$objHtmlText = $this->objLinkLib->bo_display( $sqlo, $tablename, $oneentry[1], $objViewOpts );
			if ($tablename=='CONCRETE_SUBST') $objHtmlText.=' ID:'.$oneentry[1];
			if ($tablename=='EXP') $objHtmlText.=' ID:'.$oneentry[1];
		} else {
			$objHtmlText = NULL;
		}
		return $objHtmlText;

	}

	function _getObjNamePure(&$sqlo, $oneentry) {
		$objViewOpts=array("noIcon"=>1);
		if ($oneentry[0]!=NULL and $oneentry[1]>0) {
			$tablename = $oneentry[0];
			$objHtmlText = obj_nice_name ( $sqlo, $tablename, $oneentry[1]);
		} else {
			$objHtmlText = NULL;
		}
		return $objHtmlText;

	}

	/**
	 * show edit-field for object
	 */
	function _showObjField($useTable,$xpos, $ypos, $optarr, $oneentry, $objname) {

		if ($optarr['input']=='NAME') {
			// prevent Linefeeds from DMX-Scanner
			//  return glob_replaceLF(this,event)
			$htmlText = '<input onKeyDown="return glob_replaceLF(this,event)"  name="cell['.$ypos.']['.$xpos.']" value="'.$objname.'">';
		} else {
			$loopObjArr  = $this->clipArr[$useTable]; // final select box, fill with clipboard
			// show edit field
			if ($oneentry[1]) {
				$preselected = $oneentry[1];
				$loopObjArr[$oneentry[1]] = $objname;
			} else $preselected=NULL;
				
			$htmlText = formc::selectFget( 'cell['.$ypos.']['.$xpos.']', $loopObjArr, $preselected);
		}
		return $htmlText;
	}



	/**
	 * generate Select string for a cell
	 * @param unknown $xpos
	 * @param unknown $ypos
	 * @param unknown $oldval
	 * @param unknown $options_string = "opt1|opt2|opt3"
	 */
	private function selectField($xpos, $ypos, $oldval, $options_string) {
		$optionarr = explode("|",$options_string);
		$newarr = array();
		foreach ($optionarr as $item) {
			$newarr[$item] = $item;
		}

		$selectFiledCssID= 'inp'.$this->_butFieldCnt;
		$preselected = $oldval;
		$varname = 'cell['.$ypos.']['.$xpos.']';
		$tmptxt  = "<select name=\"".$varname."\" id=".$selectFiledCssID." >\n";
		$tmptxt .= "<option value=\"\">---</option>\n";

		if (sizeof($newarr)) {
			foreach( $newarr as $id=>$text) {
				$tmpsel = "";
				if ( $preselected == $id ) {
					$tmpsel = "selected";
					$found=1;
				}
				if ( $tmpissarr AND $preselected[$id] )   $tmpsel = "selected";
				$tmptxt .= "<option value=\"". $id ."\" ".$tmpsel."> ".$text."</option>\n";
			}
		}
		if (!$found AND $preselected!="") {
			$id = $preselected;
			$tmptxt .= "<option value=\"". $id ."\" selected> ".$preselected."</option>\n";
		}
		$tmptxt .= "</select>\n";

		$tmptxt .= " <a href=\"javascript:ed2_valueInSel('".$selectFiledCssID."')\" >";
		$tmptxt .= "<IMG src=\"images/input.png\" border=0 TITLE=\"new value\"></a>";
		$this->_butFieldCnt ++;

		$htmlOut = $tmptxt;
		return $htmlOut;
	}

	/**
	 *
	 * Enter description here ...
	 * @param $sqlo
	 * @param $xpos
	 * @global $this->rowArr, $this->absEntries
	 */
	function getColView(&$sqlo, $xpos) {

		$oneentry    = $this->rowArr[$xpos];

		# echo "DEBUG: xpos:$xpos: ".$oneRowLib->absEntries[$xpos]['typ'];
		# echo " DDD_XPOS: $xpos:".print_r($this->rowArr[$xpos],1)."<br>";
		$objHtmlText = NULL;
		switch ($this->absEntries[$xpos]['typ']) {
			case 'OBJ':
				$objHtmlText = $this->_getObjName($sqlo, $oneentry);
				break;
			case 'NOTES':
				$objHtmlText = htmlspecialchars($oneentry[2]);
				break;
			case 'VIEW':
				$objHtmlText = $this->_getViewData($sqlo, $xpos, $this->rowArr);
				break;
			case 'PLUGIN':
				$title = $this->absEntries[$xpos]['optarr']['title'];
				$objHtmlText = $this->_getPluginData($sqlo, $title, $this->rowArr);
				break;
		}
		return $objHtmlText;
	}

	/**
	* show one edit line
	* @param $sqlo
	* @param $xpos
	* @param $ypos
	* @global $this->rowArr
	*/
	function getColEdit(&$sqlo, $xpos, $ypos) {
		$editColID   = $this->editColID;
		$htmlText    = NULL;
		$preselected = NULL;
		$useTable    = $this->absEntries[$xpos]['t'];
		
		$oneentry = $this->rowArr[$xpos];
		
		switch ($this->absEntries[$xpos]['typ']) {
			case 'OBJ':
				// is there an initial object ?
				
				if ($editColID==$xpos) {
				$objname  = $this->_getObjNamePure($sqlo, $oneentry);
					$htmlText = $this->_showObjField($useTable,$xpos, $ypos, $this->absEntries[$xpos]['optarr'], $oneentry, $objname);
				} else {
					$htmlText =  $this->_getObjName($sqlo, $oneentry);
				}
				break;
			case 'NOTES':
				if ($editColID==$xpos) {
					$htmlText = '<input type=text name="cell['.$ypos.']['.$xpos.']" value="'.
							htmlspecialchars($oneentry[2]).'" size=40>';
					if ($this->absEntries[$xpos]['optarr']['selectdata']!=NULL) {
						$htmlText = $this->selectField($xpos, $ypos, $oneentry[2], $this->absEntries[$xpos]['optarr']['selectdata']);
					}
				} else
					$htmlText = htmlspecialchars($oneentry[2]);
				break;
			case 'VIEW':
				$htmlText = $this->_getViewData($sqlo, $xpos, $this->rowArr);
				break;
			case 'PLUGIN':
				$title = $this->absEntries[$xpos]['optarr']['title'];
				$htmlText = $this->_getPluginData($sqlo, $title, $this->rowArr);
				break;
		}
		return $htmlText;
	}
}
