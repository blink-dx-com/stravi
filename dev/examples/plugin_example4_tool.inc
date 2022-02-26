<?php
/**
 * - EXAMPLE
 * - example of a common tool (has no direct relation to an object type) 
 * @author  Steffen Kube (steffen@clondiag.com)
 *   
 */

class a0_tool_XPL extends gPlugin {
	
	
	
	function register() {
		$title		= 'A common tool example';
		
		$this->infoarr			   = NULL;
		$this->infoarr['scriptID'] = 'a0_tool';
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = 'common tool';
		$this->infoarr['help_url'] = 'creation_statistics.html';
		$this->infoarr['form_type']= 'tool';  // must be set for a tool
		$this->infoarr['design']   = 'norm';  // normal design
		$this->infoarr['locrow']   = array(  // defines the tool location-path
			array('home.php', 'home'), 
			array('rootsubs/rootFuncs.php', 'root funcs' ) 
			); 
	}
	

	function startMain() {
		global $error;
		global $s_formState;
		$FUNCNAME='startMain';
		
		$sqlo  = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		$this->_html->line_out('I am a plugin-tool');
		
	}
	
}
