<?php

require_once ("insertx.inc");
require_once 'o.MODULE.subs.inc';

/**
 * analyse a single MODULE,PLUGIN
 * used in o.MODULE.register_li.php
 * @swreq SREQ:24:001 o.MODULE > manage plugins, modules
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @package o.MODULE.MOD.ana.inc
 */ 
class oMODULE_MOD_ana {

	var $fileShort;
	var $module_code;
	var $baseDir;  // point to Partisan-BASE-Dir; e.g. /opt/partisan
	public  $mod_basename;  // basename of module
	private $mod_type; // 'plugin', 'tool'
	private $_initialized=0;
	
	const MOD_TYPE_PLUGIN='plugin';
	const MOD_TYPE_TOOL='tool';


	/**
	 *
	 * @param string $baseDir 
	 */
	function __construct($baseDir) {
		$this->baseDir = $baseDir;
		$this->lab_sub_dir = $_SESSION['globals']['lab_path'];

	}
	
	// get BASENAME
	static function get_module_BASE_name(string $oneModule) {
	    $base = basename($oneModule);
	    return $base;
	}

	function setFile( $oneModule ) {
	    $this->_initialized=0;
		$this->fileFull  = $this->baseDir  . $oneModule;
		$this->fileShort = $oneModule;
		$this->module_code = NULL;
		$this->mod_type =  '';
		

		/**
		 * plugin/file.inc => PLUGIN:DEF/file
		 * www/pionir/view.tmpl.php =>  MOD:DEF/view.tmpl.php or LAB/o.CART_BATCH.php
		 */
		do {
			$key1='plugin/';
			$len_PLUGIN=strlen($key1);
			if (substr($oneModule,0,$len_PLUGIN)==$key1) {
				$rest = substr($oneModule,$len_PLUGIN);
				if (substr($oneModule,-4)!='.inc') {
					throw new Exception('filename not ends with .inc');
				}
				$rest = substr($rest,0,-4);
				$this->module_code = 'PLUGIN:DEF/'.$rest;
				$this->mod_type = self::MOD_TYPE_PLUGIN;
				break;
			}
				
			$key1='www/'.$this->lab_sub_dir.'/plugin/';
			$len_PLUGIN=strlen($key1);
			if (substr($oneModule,0,$len_PLUGIN)==$key1) {
				$rest = substr($oneModule,$len_PLUGIN);
				if (substr($oneModule,-4)!='.inc') {
					throw new Exception('filename not ends with .inc');
				}
				$rest = substr($rest,0,-4);
				$this->module_code = 'PLUGIN:LAB/'.$rest;
				$this->mod_type = self::MOD_TYPE_PLUGIN;
				break;
			}
				
			$key1='www/pionir/';
			$len_PLUGIN=strlen($key1);
			if (substr($oneModule,0,$len_PLUGIN)==$key1) {
				$rest = substr($oneModule,$len_PLUGIN);
				$this->module_code = 'MOD:DEF/'.$rest;
				$this->mod_type = self::MOD_TYPE_TOOL;
				break;
			}
				
			$key1='www/'.$this->lab_sub_dir.'/';
			$len_PLUGIN=strlen($key1);
			if (substr($oneModule,0,$len_PLUGIN)==$key1) {
				$rest = substr($oneModule,$len_PLUGIN);
				$this->module_code = 'MOD:LAB/'.$rest;
				$this->mod_type = self::MOD_TYPE_TOOL;
				break;
			}
				
		} while (0);

		if ($this->module_code==NULL) {
			throw new Exception('no registered pattern found.');
		}
		
		$this->mod_basename = self::get_module_BASE_name($oneModule);
		$this->_initialized=1;
		
	}

	/**
	 * get module by location
	 * @param string $foo
	 * @param mixed $foo_desc optional description of foo
	 * @return array ($mxid, $exists)
	 */
	function _getModule( &$sqlo ) {
		$module_code = $this->module_code;
		

		$mxid = 0;
		$sqlsel = 'MXID from MODULE where TYPE=3 and  LOCATION='.$sqlo->addQuotes($module_code) ;
		$sqlo->Quesel($sqlsel);
		if ( $sqlo->ReadRow() ) {
			$mxid = $sqlo->RowData[0];
		} else $mxid = 0;

		return $mxid;
	}

	function createEntry(&$sqlo, $infoarr) {

	    $main_table = $infoarr['obj_name'];  // can be empty
		$name = $infoarr['title'];
		if ($infoarr['title_sh']!=NULL) $name = $infoarr['title_sh'];

		$argu = NULL;
		$argu['NAME']     = $name;
		$argu['TITLE']    = $name;
		/**
		 * PLUGIN:DEF/code (example)
		 * MOD:DEF/view.tmpl.php or LAB/o.CART_BATCH.php
		 * @var
		 */
		$argu['LOCATION'] = $this->module_code;
		/*
		 *
		* 3: plugin or file
		*/
		$argu['TYPE'] = 3;
		$args=array('vals'=>$argu);

		$insertLib = new  insertC();
		$mxid =  $insertLib->new_meta($sqlo, 'MODULE', $args);
		
		if ($mxid and $main_table) {
		    $module_lib = new oMODULE_one($mxid);
		    $module_lib->save_meta_val($sqlo, 'TABLE_NAME', $main_table);
		}
	}

	/**
	 * check the plugin-module
	 * @param $mod
	 * @return string $classname
	 */
	function includeMod() {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;

		$fileFull = $this->fileFull;
		$pureModPath = $this->fileShort;
		$relFile     = $this->fileShort;

		if (!file_exists($fileFull)) {
			$error->set( $FUNCNAME, 4, 'Plugin "'.$fileFull.'" not found.' );
			return;
		}
		require_once($fileFull);

		$pureModPathArr = explode('/',$pureModPath);
		$namePos   = sizeof($pureModPathArr)-1;
		$modName   = $pureModPathArr[$namePos];

		$lenWithoutInc = strlen($modName) - strlen('.inc');
		$pureModName = substr($modName,0,$lenWithoutInc); // without .inc
		$classname = $pureModName.'_XPL';
		$classname = str_replace('.','_',$classname);
		if (!class_exists($classname)) {
			$error->set( $FUNCNAME, 5, 'Plugin "'.$relFile.'": class:'.$classname.' not found.' );
			return;
		}

		return ($classname);
	}

	/**
	 * analyse plugin
	 * @return $infoarr
	 * @global $this->fileShort
	 */
	private function ana_Plugin( &$sqlo ) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		$fileShort = $this->fileShort;

		$mxid  = $this->_getModule( $sqlo );
		$classname = $this->includeMod($fileShort);
		if ($error->Got(READONLY))  {
			return;
		}

		$plugLib = new $classname($sqlo);
		$plugLib->register();
		$infoarr = $plugLib->__getInfoarr();
		return array('info'=>$infoarr,'mxid'=>$mxid, 'code'=>$this->module_code);
	}
	
	/**
	 * analyse TOOL
	 * @return array
	
	 */
	private function ana_Tool( $sqlo ) {
	    $mxid    = $this->_getModule( $sqlo );
	    $infoarr = array();
	    return array('info'=>$infoarr,'mxid'=>$mxid, 'code'=>$this->module_code);
	}
	
	/**
	 * analyse MODULE: tool or plugin
	 * @return array

	 */
	public function ana_Module($sqlo) {
	    if (!$this->_initialized)
	        throw new Exception('Class not initialized.');
	    
	    if ($this->mod_type==self::MOD_TYPE_PLUGIN ) {
	        return $this->ana_Plugin( $sqlo );
	    }
	    if ($this->mod_type==self::MOD_TYPE_TOOL ) {
	        return $this->ana_Tool( $sqlo );
	    }
	}
	
	public function getModuleCode() {
		return $this->module_code;
	}



}

