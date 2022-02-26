<?php
require_once("insertx.inc");

/**
 * modify X_ERP_ARTMAP
 * @package o.X_ERP_ARTMAP.mod.inc
 * @swreq SREQ:0002506 g.SAP > MNR kann auf versch. Substanzen zeigen 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oX_ERP_ARTMAP_mod {
	
	/**
	 * add MNR-entry with version
	 * @param object $sqlo
	 * @param string $tablename
	 * @param int $art_no ARTICLE_NO
	 * @return X_ERP_ARTMAP_ID
	 */
	public function addArtnoVersion(&$sqlo, $tablename, $objid, $art_no) {
		
		// already connected ?
		$sqlsel = "X_ERP_ARTMAP_ID from X_ERP_ARTMAP where ABSTRACT_TABLE=".$sqlo->addQuotes($tablename).
			" and ABSTRACT_ID=".$objid.
			" and ARTICLE_NO=" . $sqlo->addQuotes($art_no); 	
		$sqlo->Quesel($sqlsel);
		if ($sqlo->ReadRow()) {
			return $sqlo->RowData[0];
		}
		
		// get last VERSION of ARTICLE_NO
		$sqlsel = "max(VERSION) from X_ERP_ARTMAP where ABSTRACT_TABLE=".$sqlo->addQuotes($tablename).
			" and ARTICLE_NO=" . $sqlo->addQuotes($art_no); 	
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$lastversion = $sqlo->RowData[0];
		if (!$lastversion) $lastversion=0; // fallback
		$newversion = $lastversion + 1;
		
		$args = array();
		$args["vals"]["ARTICLE_NO"] 	= $art_no;
		$args["vals"]["ABSTRACT_TABLE"] = $tablename;
		$args["vals"]["ABSTRACT_ID"] 	= $objid;
		$args["vals"]["VERSION"] 		= $newversion;
		
		$insertLib = new insertC();
		$new_artmap_id = $insertLib->new_meta($sqlo, "X_ERP_ARTMAP", $args);
		
		return $new_artmap_id;
	}
}
