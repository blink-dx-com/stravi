<?php

/**
 * register modules, support a tool which collects all PLUGINS
 * 
 * ONE tool with many MXID (each has other URL-parameters)
 * LOC: a) view.tmpl.php?t=EXP
 * LOC: a) view.tmpl.php?t=CONCRETE_SUBST
 * 
 * or MAIN_MOD_FLAG : 
 *    0: no
 *    1: the main module to analyse the access rights (nneded, for 
 * 
 * @swreq SREQ:24:001 o.MODULE > manage plugins, modules
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param int go : 0,1 
 * @param array $mxlo [] of module-names
 * @version $Header: trunk/src/www/pionir/rootsubs/o.MODULE.register_li.php 59 2018-11-21 09:04:09Z $
 */ 
 
extract($_REQUEST); 
session_start(); 

require_once ('reqnormal.inc');

require_once ("insertx.inc");
require_once ('root/g.tools.get.inc');
require_once ('o.MODULE.MOD.ana.inc');



class gModulesReg {
	
	var $cntFiles;
	
	public function __construct() {
		$this->helplib =  new gModulesAna('../../');
		$this->baseDir = $this->helplib->getBaseDir();
		
		$this->modAnaLib = new oMODULE_MOD_ana($this->baseDir);
		$this->cntFiles = 0;
	}

	
	public function analyse(&$sqlo) {
		
		$this->helplib->analyse('ALL'); // 'plugin', 'ALL'
		$this->moduleArr = $this->helplib->getModArr();
	}
	
	/**
	 * show one module
	 * form-var: $mxlo[], $go
	 */
	function _showOneModule(&$sqlo, $i, $oneModule) {
		global $error;
		
		$mxid = 0;
		
		try {
			$this->modAnaLib->setFile( $oneModule );
			$this->cntFiles++;
		} catch (Exception $e) {
		    $dataArr = array( $i+1, $mxid, $oneModule, '<b>ERROR:</b> '.$e->getMessage(),);
	    	return $dataArr;
		}
		
		$this_mod_basename = $this->modAnaLib->mod_basename ;
		
		if ( in_array($this_mod_basename,$this->_cache_basenames) ) {
		    $dataArr = array( $i+1, $mxid, $oneModule, '<b>ERROR:</b> cannot call CLASS, beacause there already exists one with same name.',);
		    return $dataArr;
		}
		
		$this->_cache_basenames[] = $this->modAnaLib->mod_basename;
		
		$answer  = $this->modAnaLib->ana_Module( $sqlo ); //ana_Plugin  anaModule
		$infoarr = $answer['info'];
		$mxid    = $answer['mxid']; 
		
		if ($error->Got(READONLY))  {
			$errLast   = $error->getLast();
			$error_txt = $errLast->text;
			$dataArr = array( $i+1, $mxid, $oneModule, '<b>ERROR:</b> '.$error_txt);
	     	$error->reset();
	     	return $dataArr;
		}
		
		if ($mxid) {
			$firstColOut=$mxid;
		} else {
			$firstColOut='<input type=checkbox name="mxlo['.rawurlencode($oneModule).']" value=1>';
		}
		
		$dataArr = array( $i+1, $firstColOut, $oneModule, $answer['code'], $infoarr['title'], $infoarr['obj_name'], $infoarr['form_type'] );	
		return $dataArr;
	}


/** 
 * show register form
 */
function form1(&$sqlo) {

	$this->tabobj = new visufuncs();
	$headOpt = array( "title" => "Overview"
	 		           );
	$headx  = array ( "#", "ID", "Path", "Module-Code", "Title", "Table", "form_type" );
	$this->tabobj->table_head($headx,   $headOpt);
	
	
	
	echo "<form style=\"display:inline;\" method=\"post\" ".
		 " name=\"editform\"  action=\"".$_SERVER['PHP_SELF']."\" >\n";
	
	
	$i=0;
	foreach( $this->moduleArr as $oneModule) {
	    $dataArr = $this->_showOneModule($sqlo, $i, $oneModule);
		$this->tabobj->table_row ($dataArr);
		$i++;
	}
	
	
	$this->tabobj->table_close();
	
	echo "<input type=hidden name='go' value='1'>\n";
	echo '<br><input type=submit value="Register" class="yButton">'."\n"; // SUBMIT
	echo "</form>";
}

function _registerOne(&$sqlo, $oneModule) {
	global $error;
	
	$this->modAnaLib->setFile( $oneModule );
	$answer = $this->modAnaLib->ana_Module( $sqlo );
	$infoarr=$answer['info'];
	$mxid =$answer['mxid']; 
	
	if ($error->Got(READONLY))  {	
		return;
	}
	
	$this->modAnaLib->createEntry($sqlo, $infoarr);
	
}

function registerDo(&$sqlo, &$mxlo) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	echo 'Register modules ...<br>';
	
	foreach( $mxlo as $oneModuleEnc=>$dummy) {
	
		$oneModule = rawurldecode($oneModuleEnc);
		if (!in_array($oneModule, $this->moduleArr)) {
			echo '<b>Error:</b> mod:'.$oneModule.' not known.<br>';
			continue;
		}
		$this->_registerOne($sqlo, $oneModule);
		if ($error->Got(READONLY))  {	
			$error->set( $FUNCNAME, 1, 'module: '.$oneModule );
			return;
		}
		echo '- Module: '.$oneModule.' ... registered.<br>'."\n";
	}
	 
}

}


// --------------------------------------------------- 
global $error;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();

$go 		= $_REQUEST['go'];
$mxlo		= $_REQUEST['mxlo'];
$title		= 'Register modules';

$infoarr			 = NULL;
$infoarr['scriptID'] = '';
$infoarr['title']    = $title;
$infoarr['form_type']= 'tool'; // 'tool', 'list'
$infoarr['design']   = 'norm';
$infoarr['locrow']   = array( array('rootFuncs.php', 'Administration') );

$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo, $infoarr);

if ( !glob_isAdmin() ) {
     htmlErrorBox( "Error",   
     "Only root can execute this!",
     "For security reason it is not allowed for common users" );
     htmlFoot();
}

chdir('..'); // one dir down 'www/pionir'

$mainlib = new gModulesReg();

$mainlib->analyse($sqlo);
$pagelib->chkErrStop();

if ($mainlib->moduleArr==NULL) {
	$pagelib->htmlFoot('INFO', 'No modules found.');
}

if ($go>0 and sizeof($mxlo)) {
	$mainlib->registerDo($sqlo, $mxlo);
	$error->printAll();
}

$mainlib->form1($sqlo);

$pagelib->chkErrStop();
$pagelib->htmlFoot();

