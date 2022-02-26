<?php
/**
 * EXP home
 * - included by module ohome.php
 * @package obj.exp._home.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param array parx 
 */


require_once ("visufuncs.inc");
require_once ("f.textOut.inc");

class oEXP_home {

	function start(&$sqlo) {
		$labdir = "../lab/";
		$pioDir = NULL;
		
		echo '<ul>';
		
		$flist = NULL;
		$flist[] = array("ty"=>"head", "txt"=>'Common', "lay"=>"1" );
		$flist[] = array("ty"=>"lnk", "txt"=>"import protocol parameters",
		 "href"=>"obj.exp.imp_sample.php?tablename=EXP");
		$flist[] = array("ty"=>"headclose");
		
		$txtopt = array();
		$textoutObj = new textOutC($txtopt);
		$textoutObj->linksOut($flist);
		
		echo '</ul>';
	}
}
