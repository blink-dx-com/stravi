<?php
require_once ("f.objview.inc");	

/**
 * 
 * - specials for SAP
 * @package f.mothChildTree.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   UREQ:0000019 GLOBAL
 */
class gMotherChildGui {
	
	// show one array
	function _showOneArray(&$sqlo, $objArray, $tablename, $separator) {
		
		$cnt=0;
		$objLinkLib = new fObjViewC();
		foreach( $objArray as $oneGroupId=>$tmpName) {
			if (!is_numeric($oneGroupId) ) {
				echo $separator . " ..... <br />"."\n";
				break;
			}
			if ($tablename!=NULL) {
				$htmlTemp = $objLinkLib->bo_display( $sqlo, $tablename, $oneGroupId );
				$htmlTemp .= ' '.$tmpName;
			}
			else $htmlTemp = $tmpName;
			echo $separator .  $htmlTemp."<br />"."\n";
			$cnt++;
		}
		
	}
	/**
	 * show tree$options['table'] given, use classic object view via table, ID
	 * text: shown  behind the object bo_display2()
	 * @param array $motherArr [$id] = text
	 * @param array$childArr   [$id] = text
	 * @param array $refarr    array('id'=>, 'txt'=>$text[OPTIONAL] )
	 * @param $options 
		'table' => name of object-tables
	 */
	function showMotherChild(&$sqlo, $motherArr, $childArr, $refarr, $options=NULL) {
		$treeIcon  = '<img src="images/ic.tree3_13.gif">';
		$spaceIcon = '<img src="0.gif" width=16 height=1>';
		
		$separator = NULL;
		
		$tablename  = $options['table'];
		$objLinkLib = new fObjViewC();
		
		if (!empty($motherArr)) {
			$hasMaster=1;
			$this->_showOneArray($sqlo, $motherArr, $tablename, $separator);
			$separator = $treeIcon; 
		}
		
		$refobj_id  = $refarr['id'];
		$refobj_txt = $refarr['txt'];
		if ($tablename!=NULL) {
			$htmlTemp = $objLinkLib->bo_display2( $sqlo, $tablename, $refobj_id);
			$htmlTemp .= ' '.$refobj_txt;
		} else {
			$htmlTemp = $refobj_txt;
		}
		echo $separator . '<span style="font-size:1.4em; font-weight:bold;">'.$htmlTemp."</span><br />"."\n";
		if ($hasMaster) $separator = $spaceIcon . $separator;
		else $separator = $treeIcon;
		
		if (!empty($childArr)) {
			
			$this->_showOneArray($sqlo, $childArr, $tablename, $separator);
			
		}
	}
}
