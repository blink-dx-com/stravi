<?php 
/**
 * REST support
 * @package rest_support.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/xmlrpc/rest_support.inc 59 2018-11-21 09:04:09Z $
 */
class rest_support {
	
	function __construct($scriptname=NULL) {
		$this->scriptname = $scriptname;
	}
	/**
	 * output error and exit script
	 * @param unknown $text
	 * @param unknown $errnum
	 */
	public function errorout($text, $errnum)  {
		
		global $error;
		header("icono-err-code: ".$errnum);
		header("icono-err-text: ".$text);
		echo("icono-err-text: ".$errnum.':'.$text);
		
		$error->set( __FUNCTION__, $errnum, $text);
		$error->logError();
		
		exit($errnum);
	}
}