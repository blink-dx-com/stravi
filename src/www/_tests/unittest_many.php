<?php 
/**
 * MANY unit tests: collect all UT_*.inc files and test them !
 * @package unittest_many.php.inc
 * @swreq UREQ:9855; FS-REG02-t1 Support validation: allow automated testing of the software; OQ-tests
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   int $go: 0,1
 * @param $parx 
 *  'infolevel': 0-3
 *  'stop' : 0,1
 *  'startscript' : OPTIONAL string
 * @version $Header: trunk/src/www/_tests/www/test/unittest_many.php 59 2018-11-21 09:04:09Z $
 */

extract($_REQUEST); 
session_start(); 

$usePath = "../../phplib/";
require_once ("f.directorySub.inc");
require_once ($usePath."reqnormal.inc"); // includes all normal *.inc files
require_once ($usePath.'func_form.inc');
require_once (dirname(__FILE__).'/../../_test/misc/unittest_onetest.inc');
require_once 'f.progressBar.inc';

class gUnit_helpDir {
	
	/**
	 * contains names of Modules to be tested
	 * @var unknown
	 */
	var $moduleArr;
	
	public function __construct() {
		
	    $UT_dir  = dirname(__FILE__).'/../../_test/';
	    $this->baseDir= $UT_dir.'/src';
		
		$this->ignoreDirs = array(
				'/phplibSimu',
				'/www/test',
				'/www/pionirSimu',
				'/validation'
		
		);
		
		$this->dirAnaLib = new fDirextoryC();
	}
	
	public function get_base_dir() {
		return $this->baseDir;
	}
	

	/**
	 * analyse one dir
	 * @return $this->moduleArr
	 */
	private function _anaOneDir( $srcdir ) {
		global $error;
		$FUNCNAME= '_anaOneDir';
	
		$baseDir = $this->baseDir;
	
		$full_dir=$baseDir.$srcdir;
		
		$incarr1 = $this->dirAnaLib->scanDir( $full_dir, '.inc', 0 );
		$incarr2 = $this->dirAnaLib->scanDir( $full_dir, '.php', 0 );
		$error->reset();  // ????
		
		$incarr = array();
		if (is_array($incarr1)) $incarr = $incarr1;
		if (is_array($incarr2)) $incarr = array_merge($incarr, $incarr2);
		
		if (!sizeof($incarr)) return;
	
		foreach( $incarr as $dummy=>$filex) {
			
			//filex conatins "UT_" ; remove this!
			$original_mod_name = $filex;
			if (substr($filex,0,3)=='UT_') {
				$original_mod_name = substr($filex,3);
			} else {
				continue; // ignore this file ...
			}
			
			$this->moduleArr[]=$srcdir.'/'. $original_mod_name;
		}
		reset ($incarr);
	
	}
	
	/**
	 * get files
	 * @param $sqlo
	 * @param $srcdir
	 * @params array $options
	 */
	public function scanStart(  ) {
		// scan directory for files
		global $error;
		$FUNCNAME= 'scanStart';
	
		$srcdir = '';
	
		$scanLib = new fDirexScanC($this->baseDir);
		$ignorebases=array('CVS');
		$scanLib->setIgnoreDirs( $this->ignoreDirs, $ignorebases );
		$scanLib->scanDirx($srcdir, 0);
		$dirarr = $scanLib->getDirArr();

	
		// analyse files
		$cnt=0;
		foreach( $dirarr as $dummy=>$dirx) {
			$this->_anaOneDir( $dirx );
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 1, 'dir: '.$dirx. ' problem' );
				return;
			}
			$cnt++;
			// if ( $cnt>4) return;
		}
		reset ($dirarr);
		
		return $this->moduleArr;
	}
	
	public function get_moduleArr() {
		return $this->moduleArr;
	}
	
	
	
}



class gUnitTest_manyC {
	
	var $parx;
	
	function __construct($parx, $go, &$flushLib) {
		$this->parx = $parx;
		$this->flushLib = $flushLib;
	}
	
	function formshow() {
	
	    $parx = $this->parx;
	
		$initarr   = NULL;
		$initarr["action"]      = $_SERVER['PHP_SELF'];
		$initarr["title"]       = "Start Unit tests";
		$initarr["submittitle"] = "Submit";
		$initarr["tabwidth"]    = "AUTO";
	
		$hiddenarr = NULL;
	
	
		$formobj = new formc($initarr, $hiddenarr, 0);
		
		$fieldx = array (
				"title" => "Infolevel",
				"name"  => "infolevel",
				"fsize" => 3,
		
				"object"=> "text",
				"val"   => $parx["infolevel"],
				"notes" => "1-low, 2-normal, 3-high"
		);
		$formobj->fieldOut( $fieldx );
		
		$fieldx = array (
		    "title" => "Stop on error",
		    "name"  => "stop",
		    "object"=> "checkbox",
		    "val"   => $parx["stop"],
		    "notes" => "Stop on first error?"
		);
		$formobj->fieldOut( $fieldx );
		
		$fieldx = array (
		    "title" => "Start at ...",
		    "name"  => "startscript",
		    "object"=> "text",
		    "val"   => $parx["startscript"],
		    "notes" => "Optional give PHP-script to start with"
		);
		$formobj->fieldOut( $fieldx );
	
	
		$formobj->close( TRUE );
	}
	
	
	
	
	
	
	/**
	 * scan basedir; start test
	 * @param unknown $sqlo
	 * @param unknown $sqlo2
	 */
	function go(&$sqlo, &$sqlo2) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$helplib   = new gUnit_helpDir();
		$all_files = $helplib->scanStart();
		$base_dir  = $helplib->get_base_dir();
		
		
		$errcnt=0;
		$okcnt =0;
		$cnt   =0;
		
		$numfiles=sizeof($all_files);
		
		$prgopt['objname']='tests';
		$prgopt['maxnum']= $numfiles;
		$this->flushLib->shoPgroBar($prgopt);
		
		$do_analysis = 1;
		if ($this->parx['startscript']!=NULL) $do_analysis = 0;
		
		foreach($all_files as $one_file) {
			
			
			if (substr($one_file,0,1)=='/') $one_file=substr($one_file,1); // remove leading slashes
			
			if (!$do_analysis and $this->parx['startscript']!=NULL) {
			    if ($this->parx['startscript'] == $one_file ) {
			        $do_analysis = 1;
			    }
			}
			
			if (!$do_analysis) continue;
			
			$parx=array("modpath"=>$one_file, "infolevel"=>$this->parx["infolevel"] );
			
			echo 'TEST: '.$one_file.': ';
			
			$do_perform = 1;
			$mainlib = new gUnitTestC($sqlo, $sqlo2, $parx, $go);
			$mainlib->test_init($sqlo);
			
			if ( $mainlib->is_gui_module()>0 ) {
				echo ' NOT tested (is GUI module)';
				$do_perform = 0;
			}
			
			if ( !$error->Got(READONLY) and $do_perform )  {
			
				
				if ($parx['subTest']!=NULL) {
					echo "Subtest: ". htmlspecialchars($parx['subTest']) . "<br>\n";
				}
				
				try {
				    
				    $mainlib->test_execute($sqlo);
				    
				} catch (Exception $e) {
				    $trace_string = $e->getTraceAsString();
				    $trace_string = str_replace("\n", "<br>", $trace_string);
				    $error->set( $FUNCNAME, 1, $e->getMessage().'<br>TRACE: '.$trace_string );
				}
			}
			
			
			if ( $error->Got(READONLY)  ) {

			    $error->printAll();
			    
				$errcnt++;
				if ($this->parx['stop']>0) {
				    echo "Stop after first ERROR!<br>\n";
				    break;
				} else {
				    echo "INFO: Continue despite ERROR.<br>\n";
				}
			} else {
				$okcnt++;
			}
			$cnt++;
			$this->flushLib->alivePoint($cnt);
			
			echo '<br>'."\n";
			
		}
		
		$this->flushLib->alivePoint($cnt,1); // finish
		echo "<br>";
		
		$sumdata = array(
			'ok'=>$okcnt,
			'cnt'=>$cnt, 
			'error'=>$errcnt
		);
		
		// result
		// show summary
		require_once ("f.visuTables.inc");
		
		$visuLib = new fVisuTables();
		$visuLib->showSummary( $sumdata );
		
	}
}

// ---------------------------------------------------
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
$sqlo2 = logon2( );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$go   = $_REQUEST["go"];
$parx = $_REQUEST["parx"];

$javascript = '
var InfoWin = null;

function xopenwin( url ) {

    url_name= url;
    InfoWin = window.open( url_name, "TESTWIN", "scrollbars=yes,status=yes,width=650,height=500");
    InfoWin.focus();

}
';

$flushLib = new fProgressBar(  );
$js_progress = $flushLib->getJS();

$title       		 = "UniTest - Many tests";
$infoarr			 = NULL;
$infoarr["title"]    = $title;
$infoarr["scriptID"] = "unitest";
$infoarr["form_type"]= "tool";
$infoarr["locrow"]   =  array( 
    array("./index.php", " UNITTEST Home") 
    
);
$infoarr['javascript'] = $javascript . $js_progress;
$infoarr['help_url'] = 'ad.UnitTest.html';
$infoarr['css'] = $flushLib->getCss(1);


$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);

echo "<ul>";

$role_right_name = 'sys.unittest';
$role_right = role_check_f($sqlo , $role_right_name );
if ( $role_right !='execute' ) {
	htmlErrorBox( "Error",
	"Sorry , you must have role right ". $role_right_name .
	" to use this tool." );
	htmlFoot();
}

$mainlib = new gUnitTest_manyC($parx, $go, $flushLib);

if ( !$go ) {
	$mainlib->formshow();
	htmlFoot("<hr>");
}

echo 'Input: '.print_r($mainlib->parx,1)."<br>";
echo "Infolevel: ".$mainlib->parx["infolevel"]."<br>\n";

$mainlib->go($sqlo, $sqlo2);


htmlFoot("<hr>");