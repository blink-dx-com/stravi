<?php
/**
 * analysis methods for WORKLIST
 * - possible TAGs for AUTO_COLLECT
 *  - SQL-condition:
 *  - SQL-view:
 *  - SQL-presel: preselet objects
 * @package o.ABS_WORKLIST.subs2.inc
 * @swreq UREQ:0000932 db.WORKLIST: Messauftrag organisieren
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @var SqlCond_STRUCT array('sqlCond'=>$condition, 'cols'=>$colPosArr)
 * @version $Header: trunk/src/phplib/o.ABS_WORKLIST.subs2.inc 59 2018-11-21 09:04:09Z $
 */
class oABS_WORKLIST_subs2 {
	
	private $PRESELECT_TAG_TEXT='SQL-presel:';
	
	/**
	 * analyse the SQL-condition
	 * @param string $condition
	 * @return SqlCond_STRUCT
	 */
	function _analyseCondStr($condition) {
		global $error;
		$FUNCNAME= __CLASS__.':_analyseCondStr';
		
		if ($condition==NULL) {
			$error->set( $FUNCNAME, 2, "Condition missing." );
			return;
		}
		
		if (!strpos($condition, "[#")) {
			$error->set( $FUNCNAME, 3, "[#xxx] Parameters missing in condition." );
			return;
		}
		
		$answerArr=NULL;
		preg_match_all ( "/\[\#.\]/" , $condition, $answerArr);
		
		// glob_printr( $answerArr, "answerArr info" );
		
		$colPosArr=NULL;
		foreach( $answerArr[0] as $dummy=>$paramTmpStr) {
			$startpos=2;
			$lenx   = strpos($paramTmpStr, ']')-$startpos;
			
			$number = substr($paramTmpStr,$startpos,$lenx );
			if (!is_numeric($number)) {
				$error->set( $FUNCNAME, 4, "Parameters '".$number."' in condition is not numeric." );
				return;
			}
			$colPosArr[]=$number;
			
		}
		
		$collectInfo = array('sqlCond'=>$condition, 'cols'=>$colPosArr);
		return $collectInfo;
	}
	
	
	
	/**
	 * analyse the collect string for one column
	 * @param $collectStr
	 * @return SqlCond_STRUCT
	 */
	function anaAutoCollectStr($collectStr) {
		global $error;
		$FUNCNAME= __CLASS__.':anaAutoCollectStr';
		$TAG_TEXT='SQL-condition:';
		
		if (substr($collectStr,0,1)=='<') {
			// XML-DOM: need PHP5.x
			$errdom = NULL;
			$dom = new DOMDocument();
			$dom->loadXML($collectStr);
			if (!$dom ) {
				$error->set( $FUNCNAME, 1, "Error while parsing the document." );
				return;
			}
			// $root = $dom->document_element();
		
			$tag = 'SQL';
			$els = $dom->getElementsByTagName($tag);
			if ( !sizeof($els) ) {
				$error->set( $FUNCNAME, 2, "No xml-tag '".$tag."' found." );
				return;
			}
			
			$itemx = $els->item(0);
			$condition = $itemx->getAttribute('condition');
		} else {
			// simple key,value pair: e.g. SQL-condition:SOCKET_ID=[#4]
			if (substr($collectStr,0,strlen($TAG_TEXT))!=$TAG_TEXT) {
				// $error->set( $FUNCNAME, 3, "No tag '".$TAG_TEXT."' found." );
				return;
			}
			$condition=substr($collectStr,strlen($TAG_TEXT));
		}
		
		$collectInfo = $this->_analyseCondStr($condition);
		
		return $collectInfo;
	}

	/**
	 * create SQL-After
	 * @param $codeCondition : SQL-code condition with variables: 
	 * 		example: "CYTCART_ID=[#1] and SAMPLEID=[#2]"
	 * @param $colPosArr selected columns array(colpos)
	 * @param $colValArr values of column
	 * @return  $codeCondition = array('sql'=>$codeCondition, 'error'=>$looperror)
	 */
	function createSqlAfter($codeCondition, $colPosArr, $colValArr) {
		global $error;
		$FUNCNAME= __CLASS__.':createSqlAfter';
		
		$looperror=NULL;
		foreach( $colPosArr as $dummy=>$colpos) {
			$oneval = $colValArr[$colpos];
			if ($oneval==NULL) {
				$looperror = "Input value for column ".$colpos." missing." ;
				$codeCondition = NULL;
				break;
			}
			$codeCondition = str_replace('[#'.$colpos.']', $oneval, $codeCondition);
		}
		reset ($colPosArr); 
		return array('sql'=>$codeCondition, 'error'=>$looperror);
	}
	
	/**
	 * analyse the VIEW string from AUTO_COLLECT column
	 * @param $collectStr
	 * @return SqlCond_STRUCT
	 */
	function anaViewSqlStr($collectStr) {
		global $error;
		$FUNCNAME= __CLASS__.':anaViewSqlStr';
		$TAG_TEXT='SQL-view:';
		
		// simple key,value pair: e.g. SQL-condition:SOCKET_ID=[#4]
		if (substr($collectStr,0,strlen($TAG_TEXT))!=$TAG_TEXT) {
			$error->set( $FUNCNAME, 3, "No tag '".$TAG_TEXT."' found." );
			return;
		}
		$condition=substr($collectStr,strlen($TAG_TEXT));
		$collectInfo = $this->_analyseCondStr($condition);
		
		return $collectInfo;
	}
	
	/**
	 * check, if column has PRESELECT SQL data
	 * @param string $collectStr - string from column AUTO_COLLECT
	 * @return 0,1
	 */
	function has_preselect_cond($collectStr) {
		$TAG_TEXT= $this->PRESELECT_TAG_TEXT;
		
		if (substr($collectStr,0,strlen($TAG_TEXT))!=$TAG_TEXT) {
			return 0;
		}
		return 1;
	}
	
	/**
	 * analyse the PRESELECT SQL string from AUTO_COLLECT column to support a preselect of objects
	 * @param $collectStr
	 * @return SqlCond_STRUCT
	 */
	function anaObjPreselectSql($collectStr) {
		global $error;
		$FUNCNAME= __CLASS__.':anaObjPreselectSql';
		$TAG_TEXT= $this->PRESELECT_TAG_TEXT;
		
		// simple key,value pair: e.g. SQL-condition:SOCKET_ID=[#4]
		if (substr($collectStr,0,strlen($TAG_TEXT))!=$TAG_TEXT) {
			$error->set( $FUNCNAME, 3, "No tag '".$TAG_TEXT."' found." );
			return;
		}
		$condition=substr($collectStr,strlen($TAG_TEXT));
		$colPosArr=NULL;
		$collectInfo = array('sqlCond'=>$condition, 'cols'=>$colPosArr);
		
		return $collectInfo;
	}
	
	/**
	 * transform option string to array (key,val)
	 * @param $optionStr string of options, separated by NEWLINE : key=val
	 * @return $outarray array of (key,val)
	 */
	function option2arr($optionStr) {
		if ($optionStr==NULL) return;
		$optarr = explode("\n",$optionStr);
		
		$outarray=NULL;
		reset ($optarr);
		foreach( $optarr as $dummy=>$keyval) {
			$keyval=trim($keyval);
			if ($keyval==NULL) continue; // ignore empty strings
			$oneKeyVal = explode("=",$keyval,2);
			$outarray[trim($oneKeyVal[0])]=$oneKeyVal[1];
		}
		reset ($optarr); 
		return $outarray;
	}
	
}
