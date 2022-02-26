<?php 
require_once ("f.objview.inc");
require_once 'lev1/o.SUA.versman.sub.inc';

/**
 * - create/modify mapping between abstract object and erp_article
 * - check params before input
 *    
 * @package o.X_ERP_ARTMAP.guisub.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0001555: p.107 > X_ERP_ARTMAP > plugin zum Anlegen von SAP Mappings 
 */
class o_X_ERP_ARTMAP_guisub {
	
	public function showVersTable(&$sqlo, $matnr, $suaid=0) {
		
		if ($matnr==0) return;
		
		// sort DESCENDING !!!
		$sqlsel = "* from X_ERP_ARTMAP where ARTICLE_NO=" . $sqlo->addQuotes($matnr) . ' order by VERSION DESC'; 		
		
		$cachearr=NULL;
		$sqlo->Quesel($sqlsel);
		while ($sqlo->ReadArray()){
			$cachearr[] = $sqlo->RowData;
		}
		
		if (!sizeof($cachearr)) return;
		
		$objLinkLib = new fObjViewC();
		$versionLib = new oSUA_versman_child();
		
		$iconGreen = '<img src="images/i13_green.gif">';
		$iconGray  = '<img src="images/i13_gray.gif">';
		
		$tabobj = new visufuncs();
		$headOpt = array( 
		   "title" => "Version overview for MNR:".$matnr
		  );
		$headx  = array ("MNR-Version", "substance (SUA)", "SUA-ID", "", "SUA-G&uuml;ltigkeit");
		$tabobj->table_head($headx,   $headOpt);
		
		reset ($cachearr);
		foreach( $cachearr as $key=>$valarr) {
			
			$validInfo = NULL;
			$icon = NULL;
			
			if ($valarr['ABSTRACT_ID']) {
				
				$abstract_id = $valarr['ABSTRACT_ID'];
				$tmphtml  = $objLinkLib->bo_display( $sqlo, 'ABSTRACT_SUBST', $abstract_id );
				
				do {
					// check Versions
					// analyse SUA	
					$versionLib->setChild($sqlo, $abstract_id);
					$suaType   = $versionLib->getSuaType();
					if ($suaType==0) {
						$found=1;
						$validInfo = 'o.k. (nicht versioniert!)';
						$icon = $iconGreen;
						break;
					}
					if ($suaType==1) {
						$validInfo = 'no: MasterVersion.';
						$icon = $iconGray;
						continue;
					}
					
					$validFlag = $versionLib->getValidFlag();
					if ($validFlag==2) {
						// take first found object
						$found=1;
						$validInfo = 'o.k. (g&uuml;ltige Version)';
						$icon = $iconGreen;
						break;
					} else {
						$validInfo = 'Version nicht aktiv!';
						$icon = $iconGray;
					}
				} while (0);
				
			}
			
			$shopt = array();
			
			if ($suaid==$valarr['ABSTRACT_ID']) {
				$shopt["bgcolor"] = '#D0D0FF'; // mark this sua
			}
			$dataArr = array($valarr['VERSION'], $tmphtml, $valarr['ABSTRACT_ID'], $icon, $validInfo);
		 	$tabobj->table_row ($dataArr, $shopt);
		}
	
		
	
		$tabobj->table_close();
	
	}
}
