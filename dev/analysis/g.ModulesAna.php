 <?php
/**
 * analyse PHP-modules
 * $Header: Exp $
 * @package g.ModulesAna.php
 * @author  Steffen Kube (steffen@clondiag.com)
 * @param string $go 
 */

session_start();
require_once ("reqnormal.inc");
require_once ("f.directorySub.inc");
require_once ("insert.inc");

class gModuleOneC {

function gModuleOneC($baseDir) {
	$this->baseDir = $baseDir;
	$this->cntFiles=0;

	$this->anaGuiIgnore = array(
		'./install.php',
		'./phplib/PHPUnit.php',
		'./phplib/class.ezpdf.php',
		'./phplib/class.pdf.php',
		);
}

function setFile( $srcdir, $filex ) {
	$this->filex = $filex;
	$this->fileFull  = $this->baseDir  . $srcdir .'/'. $filex;
	$this->fileShort = $srcdir .'/'. $filex;
	$this->cntFiles++;
}

/**
 * Example of documenting undetermined function arguments
 * @param string $foo 
 * @param mixed $foo_desc optional description of foo
 * @return array ($mxid, $exists)
 */
function _getModule( &$sqlo ) {
	$fileShort = $this->fileShort;
	// echo $this->cntFiles. '. file: '.$this->fileShort." \n";

	$mxid = 0;
	$sqlsel = 'MXID from MODULE where LOCATION='.$sqlo->addQuotes ($fileShort) ;
	$sqlo->Quesel($sqlsel);
	if ( $sqlo->ReadRow() ) {
		$mxid = $sqlo->RowData[0];
	}
	if ($mxid) {
		return array($mxid, 1);
	}

	$argu = NULL;
	$argu['NAME'] = $this->filex;
	$argu['LOCATION'] = $fileShort;
	$mxid = insert_row($sqlo, 'MODULE', $argu );
	echo " ... inserted.<br>\n";

	return array($mxid, 0);
}

function anaModule( &$sqlo ) {
	global $error;
	$FUNCNAME= 'anaModule';
	$fileShort = $this->fileShort;

	$answer = $this->_getModule( &$sqlo );
	if ( !$answer[0] ) return;
	$mxid = $answer[0];

	/*
	if ( $this->cntFiles > 20 ) {
		echo "DEBUG: break after 20<br>";
		exit;
	}
	*/
	$sqlsel = '* from MODULE where MXID='.$mxid;
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadArray();
	$parOld = $sqlo->RowData;
	
	// update features 
	

	$type = 1;
	$extension = substr( $fileShort, strrpos($fileShort, '.')+1);
	if ($extension=='php') $type = 0;

	if ( in_array( $fileShort, $this->anaGuiIgnore ) ) {
		$type = 1; // ignore for GUI analysis
	}

	$file_len = strlen($fileShort);

	if ( !$type ) {
		$endkey = '.xedit.php';
		$keylen = strlen( $endkey );
		$subk   = substr( $fileShort, $file_len-$keylen );
 		if ( $subk == $endkey ) {
			$type = 1;
		}
	}
	// echo "&nbsp; Extesn: ".$extension." <br>";
	$URL = NULL;
	if ( !$type ) {
		$URL = $parOld['LOCATION'];
		$key = '/www/';
		$keypos = strpos($URL, $key );

		if ( $keypos === FALSE  ) {
			$error->set( $FUNCNAME, 1, 'LOCATION expects "'.$key.'"' );
			return;
		}
		$keyposAfter = $keypos + strlen($key);
		$URL = substr($URL, $keyposAfter );
		// echo "&nbsp;new URL: $URL <br>";
	}

	$argu['MXID'] = $mxid;
	$argu['TYPE'] = $type;
	$argu['URL']  = $URL;
	$ret = update_row($sqlo,'MODULE', $argu);
}

}

class gModulesAna {

function gModulesAna() {
	$this->ignoreDirs = array(
		"./_tests",
		"./_dev",
		"./templates",
		"./www/pionir/images",
		"./www/pionir/help",
		"./phplib/PHPUnit"

	);
	$this->ignoreBases = array( 'CVS' );
	
	$this->baseDir = '../../';
	$this->allarr = array();
	$this->dirarr = array();

	$this->modAnaLib = new gModuleOneC($this->baseDir);
	$this->dirAnaLib = new fDirextoryC();
	
}

function _infoFile($srcdir, $filex) {
	echo $srcdir.'/'. $filex." <br>";
}

function _anaOneDir( &$sqlo, $srcdir ) {
	global $error;
	$FUNCNAME= '_anaOneDir';

	$baseDir = $this->baseDir;

 	$ext='.php';
	$allarr = $this->dirAnaLib->scanDir( $baseDir.$srcdir, $ext, 0 );
	$error->reset();
	$ext='.inc';
	$incarr = $this->dirAnaLib->scanDir( $baseDir.$srcdir, $ext, 0 );
	$error->reset();

	$allarr = array_merge($allarr, $incarr);

	while ( list(,$filex) = each($allarr) ) {
		$this->modAnaLib->setFile( $srcdir, $filex );
		$this->_infoFile($srcdir, $filex);
		$answer = $this->modAnaLib->anaModule( &$sqlo );
		if ($error->Got(READONLY))  {
     		$error->set( $FUNCNAME, 1, $srcdir.'/'. $filex. ' failed.' );
			return;
		}
	}
	reset ($allarr);

}


function scanStart( &$sqlo ) {
	// scan directory for files
	global $error;
	$FUNCNAME= 'scanStart';

	$srcdir = '.';
	$scanLib = new fDirexScanC($this->baseDir);
	$scanLib->setIgnoreDirs($this->ignoreDirs, $this->ignoreBases);
	$scanLib->scanDirx($srcdir, 0);
	$dirarr = $scanLib->getDirArr();
	// glob_printr( $dirarr, "dirarr info" );

	// analyse files
	$cnt=0;
	while ( list(,$dirx) = each($dirarr) ) {
		$this->_anaOneDir( &$sqlo, $dirx );
		if ($error->Got(READONLY))  {
     		$error->set( $FUNCNAME, 1, 'dir: '.$dirx. ' problem' );
			return;
		}
		$cnt++;
		// if ( $cnt>4) return;
	}
	reset ($dirarr); 
}



function analyse( &$sqlo ) {
/*
 - scan recursive
 - for each file do
*/
	$this->scanStart($sqlo);

}

}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $PHP_SELF );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$go 		= $_REQUEST["go"];
$title		= "analyse PHP-modules";

$infoarr			 = NULL;
$infoarr["scriptID"] = "";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "tool"; // "tool", "list"
$infoarr["locrow"]   = array( array("index.php", "index") );

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);

$mainlib = new gModulesAna();

$mainlib->analyse($sqlo);

$error->printAll();
htmlFoot();



