<?php
require_once ('f.update.inc');

 /*
 * temp convert
 *    
 * @package temp_convert.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   UREQ:0001088: o.CCT_ACC_UP > AutoCleaner 
 * @param   $_REQUEST['go'] 0,1
 *   
 */
class temp_convert_XPL extends gPlugin {
	
	
	
	function register() {
		$tablename	= 'EXP';
		$title		= 'Convert';
		
		$this->infoarr			 = NULL;
		
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;

		
		$this->infoarr2 = array( 
			'onlyAdmin'   =>1,
			
		);
				
		
	}
	
	function form0() {
	    $initarr=array();
		$initarr["title"]       = "Convert SHORTRES";
		$initarr["submittitle"] = "Now!";
		$initarr["tabwidth"]    = "AUTO";
		$hiddenarr = NULL;
		$formobj = $this->_html->formInit($initarr, $hiddenarr);
		$formobj->close( TRUE );
	}
	
	// find database object CODE
	// @return string
	static function find_db_obj(&$text, $startpos) {
	    $result='';
	    $matches=NULL;
	    
	    $answer = preg_match ("/^db:[A-Z]+:[0-9]+/", substr($text,$startpos), $matches, 0);
	    echo "XXX:INPUT:$text| START:$startpos|  CHAR:".substr($text,$startpos,1 )."<br>";
	    if ($answer) {
	        $result = $matches[0][0];
	        echo " -Result:$result|<pre>".print_r($matches,1)."</pre><br>";
	    }
	    return $result;
	}

	function startMain() {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$a='db:MAC:123';
		$b = self::find_db_obj($a, 0);
		
		$a='db:MAC:122 db:MAC:xdf db:MAC:123 xc';
		$b = self::find_db_obj($a, 11);
		$b = self::find_db_obj($a, 22);

		
	}
	
}