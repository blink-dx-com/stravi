<?php
require_once ('o.MODULE.MOD.ana.inc');

/**
 * analyze application log > MODUL_LOG
 * - related to Redmine:#3368
 * @package f.appLogAna.inc
 * @swreq SREQ:0001004 g > f.appLogAna.php application log analysis 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * $Header: trunk/src/www/pionir/rootsubs/f.appLogAna_MODUL.inc 59 2018-11-21 09:04:09Z $
 * 
 * FILE-FORMAT: TAB-separated fields\\

0	 1	  2	     3   4
TIME USER MODULE NUM IP
2017-11-03T10:33:05\tgrit\tXMLRPC:connect2\t\t192.168.100.107
2013-09-19T10:19:43\tanke_s,juliane_k,annett_r,alexander,ines_w,kornelia,\tXMLRPC:META:DEF/oEXP_get_absArray\t17
--------
where NUM is optional!
 */
class fModulLog extends fappLogAnaC {
	
	/**
	 * 
	 * Enter description here ...
	 * @var array $histoCache
		[$modname] = array('c'=>count, 'd'=>date );
	 */
	private $histoCache;
	
	function __construct (&$sqlo, $parx) {
		
		parent::__construct($parx);
		
		$this->sqlo = &$sqlo;
		
		$this->tableTitle='MODUL_LOG';
		$this->fileName = $_SESSION["globals"]["app.log.dir"].'/app.modulLog.dat';
		$this->headx  = array ( 'Line', 'Date', 'User', 'Info', 'Occurence', 'IP' );
		
		if ($this->parx['shHisto']>0) {
			$this->headx  = array ( '#', 'No of occur.', 'Last Date', 'Info' );
		}
		
		$this->histoCache=array();
		
		$this->toolAdStr='www/';
		
		$this->modAnaLib = new oMODULE_MOD_ana('../../../');
		
		$this->appRoot     = $_SESSION['s_sessVars']['DocRootURL']; // e.g.	/~steffen/partisan/www;
		
		
	}
	
	public function getMoreFields($parx) {
	
		$fieldxArr=array();
		$fieldxArr[] = array ( 
			"title" => "Show histogram", 
			"name"  => "shHisto",
			"object"=> "checkbox",
			"val"   => $parx["shHisto"], 
			"notes" => "show histogram ?"
			 );
			 
		$fieldxArr[] = array ( 
			"title" => "Search Module", 
			"name"  => "seaText",
			"object"=> "text",
			"val"   => $parx["seaText"], 
			"notes" => "search for a special module in log"
			 );
		
		$fieldxArr[] = array (
				"title" => "Search User",
				"name"  => "seaUser",
				"object"=> "text",
				"val"   => $parx["seaUser"],
				"notes" => "search for a User in log"
		);
		
		$fieldxArr[] = array (
				"title" => "Exact phrase",
				"name"  => "seaExact",
				"object"=> "checkbox",
				"val"   => $parx["seaExact"],
				'inits' => 1,
				"notes" => "search exact pharse, no wildcards"
		);
			 
		$fieldxArr[] = array ( 
			"title" => "Sort by", 
			"name"  => "seaSort",
			"object"=> "select",
			"val"   => $parx["seaSort"], 
			"inits" => array('mod'=>'module', 'date'=>'date', 'occ'=>'occurence'),
			"notes" => ""
			 );
			 
		$fieldxArr[] = array ( 
			"title" => "and real tools", 
			"name"  => "histoReal",
			"object"=> "checkbox",
			"val"   => $parx["histoReal"], 
			"notes" => "show existing tools and how they are used (need historgram option"
			 );
		
		
		return $fieldxArr;
		
	}
	
	
	/**
	 * - show one line
	 * @param $line
	 * @return int
	 */
	function _rowout($lineno, $line) {

		/**
		 * 0	1	 2	    3   4
		 * TIME USER MODULE NUM IP
		 */
		$lineArr = explode("\t",$line);
		$modname = trim($lineArr[2]);
		$user    = trim($lineArr[1]);
		$usecnt  = trim($lineArr[3]);
		
		if ($this->parx['shHisto']>0) {
		
			$num=1;
			if ($usecnt>0) $num = $usecnt;
			// in year 2014, the log-format contains here an IP-address ...
			// correct this error ...
			if (strstr($num,'.')!=NULL) $num = 1; 
			
			if (!isset($this->histoCache[$modname])) {
				$this->histoCache[$modname] = array('c'=>$num, 'd'=>$lineArr[0]);
			} else {
				// update
				 
				$this->histoCache[$modname]['c']=$this->histoCache[$modname]['c']+$num;
				$this->histoCache[$modname]['d']=$lineArr[0];
			}
			
			if ( $lineno/1000.0 == intval($lineno/1000.0)) echo '.';
			
		} else {
			
			if ($this->parx['seaText']!=NULL) {
				// search only for this text
				if ($this->parx['seaExact']) {
					// exact search
					if ( $this->parx['seaText']!=$modname ) return 1;
				} else {
					// wildcard search
					if ( !stristr($modname, $this->parx['seaText']) ) return 1;
				}
			}
			
			if ($this->parx['seaUser']!=NULL) {
				// search for user name
				if ( $this->parx['seaUser']!=$user ) return 1;
				
			}
			
			$dataArr = array (
				$lineno,
				$lineArr[0],
				$lineArr[1],
				htmlentities($modname),
				$lineArr[3],
				$lineArr[4] // IP
				);
			$this->tabobj->table_row ($dataArr);
		}
		
	}
	
	function finishLoop() {
		
		if ($this->parx['shHisto']>0) {
			// show histogram
			
			$this->histInfoCnt = sizeof($this->histoCache);
			
			if (!sizeof($this->histoCache)) return;
			
			$sortedkeys = array();
			
			echo "<br>\n SORT::".$this->parx['seaSort'];
			
			switch ($this->parx['seaSort']) {
				case 'date':
					$keys = array_keys($this->histoCache);
					reset ($keys);
					$sortarr = array();
					foreach( $keys as $dummy=>$modname) {
						$sortarr[$modname]=$this->histoCache[$modname]['d'];
					}
					asort($sortarr);
					$sortedkeys = array_keys($sortarr);
					
					break;
					
				case 'occ':
					$keys = array_keys($this->histoCache);
					reset ($keys);
					$sortarr = array();
					foreach( $keys as $dummy=>$modname) {
						$sortarr[$modname]=$this->histoCache[$modname]['c'];
					}
					asort($sortarr);
					$sortedkeys = array_keys($sortarr);
					
					break;
				default:
					// sort by key (modul)
					ksort($this->histoCache);
					$sortedkeys = array_keys($this->histoCache);
			}
			
		
			foreach( $sortedkeys as $i=>$key) {
				$valarr = $this->histoCache[$key];
				$dataArr= array( ($i+1), $valarr['c'], $valarr['d'], $key);
				$this->tabobj->table_row ($dataArr);
			}
			
		}
	}
	
	/**
	 * analyse one module
	 * - check if entry in table MODULE exists
	 * @param string $modname
	 * @return  array('file'=>$realFile, 'locked'=>-1, 0, 1)
	 */
	private function _anaOneHistoFile(&$sqlo, $modname) {
		
		$lockedFlag = 0;
		$moduleCode = NULL;
		
		$modname = trim($modname);
		// analyse MOD-name
		if (substr($modname,0,7)=='PLUGIN:') {
			$submod = substr($modname,7);
			$subdir = substr($submod,0,3);
		
			$plugdir = '';
			if ($subdir=='DEF') {
				$plugdir = 'plugin';
			}
			if ($subdir=='LAB') {
				$plugdir = 'www/lab/plugin';
			}
			$realFile = $plugdir.'/'.substr($submod,4).'.inc';
		} else {
			$realFile = str_replace($this->appRoot, '', $modname);
			if (substr($realFile,0,1)=='/') $realFile = substr($realFile,1); // remove first /
			$realFile = $this->toolAdStr . $realFile ;
		
		}
		
		// check MODULE
		try {
			
			
			
			$info = $this->modAnaLib->setFile($realFile);
			$mxid = $this->modAnaLib->_getModule( $sqlo );
			$moduleCode = $this->modAnaLib->getModuleCode();
			
			
			
			if ($mxid) {
				// is locked ? 
				$lockedFlag = glob_elementDataGet( $sqlo, 'MODULE', 'MXID', $mxid, 'LOCKED');
				if (!$lockedFlag) $lockedFlag = -1; // exists, but not locked
			}
			
			
		} catch (Exception $e) {
			// no module found ...
			$temp_error = $e->getMessage();
			// no action needed
		}
		
		
		
		return array('file'=>$realFile, 'locked'=>$lockedFlag);
	}
	
	/**
	 * analyse used modules versus existing
	 * - input: $this->histoCache
	 */
	private function anaUsedVersEx(&$moduleArr) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$sqlo = &$this->sqlo;
		
		if (!is_array($moduleArr)) {
			$error->set( $FUNCNAME, 1, 'No input given.' );
			return;
		}
		
		
		$shortUrlPos = strlen($this->appRoot);
		
		
		// $realFile : the real file name in system on /opt/partisan
		reset ($moduleArr);
		
		/**
		 * array KEY => array(
		 * 	'p' => path
		 *  'c' => count
		 *  'l' => locked flag 0,1 (locked), 2: exists but not locked
		 * )
		 */
		$newModArr = array();
		foreach( $moduleArr as $key=>$pathx) {
			$newModArr[$key]=array('p'=>$pathx);
		}
		
		
		// $realFile : the real file name in system on /opt/partisan
		reset ($this->histoCache);
		foreach( $this->histoCache as $modname=>$valarr) {

			$anaDict = $this->_anaOneHistoFile($sqlo, $modname);
			$realFile = $anaDict['file'];
			
			$keyInModarr = array_search($realFile, $moduleArr);
			
			// echo "DEBUG: $keyInModarr : '$realFile' <br>\n";
			
			if ($keyInModarr!==FALSE) {
				$newModArr[$keyInModarr]['c'] = $valarr['c'];
				$newModArr[$keyInModarr]['l'] = $anaDict['locked'];
				//$newModArr[$keyInModarr]['moco'] = $anaDict['moco']; // for debugging
			} else {
				echo 'Problem: module not found: '.$realFile."<br>\n";
			}
		}
		
		// glob_printr( $newModArr, "newModArr info" );
		// show
		$this->tabobj = new visufuncs();
		$headOpt = array( "title" => 'usage in existing modules');
		$headx   = array('module', 'usage', 'locked?');
		$tabobj = new visufuncs();
		$tabobj->table_head($headx,   $headOpt);
		
		reset ($newModArr);
		foreach( $newModArr as $dummy=>$valarr) {
			$dataArr=array($valarr['p'],$valarr['c'], $valarr['l']);
			$tabobj->table_row ($dataArr);
		}
		$tabobj->table_close();
		
		echo "Num of existing modules: ".sizeof($newModArr)."<BR>";
		
	}
	
	public function postInfo() {
		if ($this->parx['shHisto']>0) {
			echo 'unique Modules:'.$this->histInfoCnt."<br />\n";
		}
		
		if ( $this->parx['histoReal'] ) {
			require_once 'root/g.tools.get.inc';
			
			$basedir = '../../../';
			$helplib = new gModulesAna($basedir);
			$helplib->analyse('ALL');
			$moduleArr = $helplib->getModArr();
			// glob_printr( $moduleArr, "moduleArr info" );
			
			$this->anaUsedVersEx($moduleArr);
			// analyse in existing tool
		}
	}
	
}


