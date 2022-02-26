<?php
/**
 * - create objects by import of CSV data
 * - import policy is defined by $parx["methodKey"]
 * - import code from:  "$_SESSION['globals']["lab_path"]/import/imp3.[methodKey].inc"
 * $Header: trunk/src/www/pionir/glob.objtab.import3.php 59 2018-11-21 09:04:09Z $
 * @package  glob.objtab.import3.php
 * @author   Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param 	$parx
 * @param 	$parx["methodKey"] : lab defined method
			$parx['infolevel'] : DEPRECATED
			$parx['fileshort'] : short name
			$parx["projid"]   - selected project ID
 * @param   _FILES["datafile"]
 * @param 	$go : 0,1,2,
 * @global  $_SESSION["userGlob"]["g.debugKey"] : 'noInsert'
 
 */

session_start(); 

require_once ("reqnormal.inc");
require_once ("func_form.inc");
require_once ('insertx.inc');
require_once ("f.directorySub.inc");
require_once ("class.filex.inc");
require_once ("f.upload.inc");
require_once ('f.workdir.inc');
require_once ("f.visuTables.inc");
require_once ("visufuncs.inc");
require_once ('f.msgboxes.inc');
require_once ('f.progressBar.inc');

require_once ('f.fileana.inc');
require_once 'import/f.read_file_meta.inc';
require_once 'import/f.spreadsheet_ana.inc';


require_once ("o.PROJ.addelems.inc");
require_once ("f.objview.inc");	

require_once ('import/glob.objtab.imp3.base.inc');



/**
 * the IMPORT library
 * 
 */
class gObjImp3C {

    var $CLASSNAME='gObjImp3C';
    var $maxDataLines;
    var $methodKey; // point to a imp3-include-file import/imp3.KEY.inc
    var $doInsert;  // tool is in the preparation state (0)  or in the final execution state (1)
    var $headerPos; // header array [POS] = VAL
    private $header;
    protected $projAddLib;
    
    /**
     * 
     * @param $parx
     * @param $go : 0 : prep1
     * 				1 : prep2
     * 				2 : do insert
     */
    function __construct($parx, $go) {
    	global $error;
    	$FUNCNAME= 'gObjImp3C';
    	$this->go   = $go;
    	$this->parx = $parx;
    	$this->methodKey = $parx['methodKey'];
    	$this->infolevel = $parx['infolevel'];
    	// $this->impOneLib = new gObjImpOneC();
    	
    	//$this->uploadLib   = new fFileanaC();
    	
    	$this->debug = NULL;
    	$this->doInsert = 0;
    	if ($go==2) $this->doInsert = 1;
    	
    	if ( $_SESSION["userGlob"]["g.debugKey"]=='noInsert' ) {
    		echo "<B>INFO:</B> DEBUG-mode: g.debugKey: noInsert<br>\n";
    		$this->doInsert = 0;
    	}
    	if ( $_SESSION["userGlob"]["g.debugLevel"]>2 ) {
    		echo "<B>INFO:</B> DEBUG-mode: show insert data<br>\n";
    		$this->debug = $_SESSION["userGlob"]["g.debugLevel"];
    	}
    	
    	$dynIncFile = "../".  $_SESSION['globals']["lab_path"] .'/import/imp3.'.$this->methodKey.'.inc';
    	if (!file_exists($dynIncFile)) {
    		$error->set( $FUNCNAME, 1, 'no method "'.$this->methodKey.'" known.' );
    		return;
    	}
    	
    	require_once ($dynIncFile);
    	$className = 'imp3_'.$this->methodKey.'_C';
    	$this->MethodLib = new $className();
    	
    }
    
    /**
     * init the import lib
     * 
     * @param object $sqlo
     * @param string $datafileFull : if empty, do not try to analyse the file
     * @return NULL
     */
    function init( &$sqlo, $datafileFull) {
    	global $error;
    	$FUNCNAME= $this->CLASSNAME.':init';
    	
    	$this->tablename= $this->MethodLib->tablename;
    	$this->MethodLib->objCreaLib = new objCreaWiz($this->tablename);
    	
    	
    	
    	$this->MethodLib->importPrep($sqlo);
    	if ( $error->Got(READONLY) ) {
    		$error->set( $FUNCNAME, 1, "");
    		return;
    	}
    	
    	$this->projAddLib  = NULL;
    	if ($this->impParamDict['add2projID']>0) {
    	    $this->projAddLib = new oProjAddElem( $sqlo, $this->impParamDict['add2projID']);
    	    if ( $error->Got(READONLY) ) {
    	        $error->set( $FUNCNAME, 1, "no insert access to destination-project.");
    	        return;
    	    }
    	}
    	
    	$this->MethodLib->importPrep_super($sqlo);
    	if ( $error->Got(READONLY) ) {
    	    $error->set( $FUNCNAME, 2, "importPrep_super failed.");
    	    return;
    	}
    	
    	$this->impParamDict = $this->MethodLib->getParamsDict();
    	$impParamDict = &$this->impParamDict;
    	// glob_printr( $impParamDict, "impParamDict info" );
    	
    	// $this->tablename   = $impParamDict['tablename'];
    	$this->colsExpect  = $impParamDict['colsExpect'];
    	$this->datafileFull= $datafileFull;
    	
    	
    	$this->MethodLib->tablename = $this->tablename;
    	
    	
    	
    	if ($datafileFull==NULL) return;
    	
    	$impopt = array();
    	if ($this->impParamDict['delimiter']!=NULL) {
    	    $delim = $this->impParamDict['delimiter'];
    	} else {
    	    $delim = "\t";
    	}
    	$impopt['delimiter'] = $delim;
    	
    	$this->file_read_lib = new f_read_file_meta($datafileFull,'', $impopt);
    	$this->file_read_lib->open_file();
    	$headers = $this->file_read_lib->get_headers();
    	$header  = $headers[0];

    	$this->maxDataLines = $this->file_read_lib->countDataLines( );
        
//     	if ( $this->maxDataLines<=0 ) {
//     		$error->set( $FUNCNAME, 2, 'No data lines found.');
//     		return;
//     	}

        //  $this->uploadLib->setHeaderParms($this->colsExpect);
        
        // FUTURE: for special HEADER pattern ...
    	// $open_options = array();
    	//if ($this->impParamDict['headerPattern']!=NULL) {
    	//   $open_options['headerPattern'] = $this->impParamDict['headerPattern'];
    	//}

    	$this->spread_lib = new fSpreadheet_ana();
    	$this->spread_lib->set_file_header($header);
    	
    	debugOut('File-Header: '.print_r($header,1), $FUNCNAME, 1);
    	
    	$this->spread_lib->analyseHeader($this->colsExpect);
    	if ( $error->Got(READONLY) ) {
    	    return;
    	}
    	

    	$this->MethodLib->postHeaderAnalysis($sqlo, $header);
    	if ( $error->Got(READONLY) ) {
    		$error->set( $FUNCNAME, 4, "postHeaderAnalysis failed.");
    		return;
    	}
    	$this->header = $header;

    }
    
    function getNumLines() {
    	return ($this->maxDataLines);
    }
    
    function getHeader() {
        return ($this->header);
    }
    
    
    /**
     * analyse header
     * 
     */
    function anaHeader( &$sqlo ) {
    }
    
    //function _valByDB_col( &$dataarr, $dbcol ) {
    //    return ($dataarr[ $this->headerPos[$dbcol] ]);
    //}
        
    /**
     * do UTF8 encoding
     * @param array $dataarr
     */
    private function convert_encoding_2UTF8( &$dataarr ) {
        
        // echo "DEBUG: UTF8-encode ...<br>";
        foreach($dataarr as $index=>$val) {
            
                if ($val!==NULL) {
                    if (utf8_encode($val)!=$val) {
                        $dataarr[$index]=utf8_encode($val);
                    }
                }
        }
       
    }
    
    /**
     * handle ONE file line, create one object
     * output: $this->row_info_arr
     * @param $sqlo
     * @param $dataarr
     * @param $cnt
     */
    function _insertOne( &$sqlo, &$sqlo2, &$sqlo3, $datafields_raw, $dataarr, $cnt ) {
    	global $error;
    	$FUNCNAME= '_insertOne';
    	
    	$argu = array();
    	// will be done before in file_lib ...
//     	if ($this->impParamDict['convertData']!='') {
//     	    if ($this->impParamDict['convertData']=='ISO-8859-1-TO-UTF-8') {
//     	        $this->convert_encoding_2UTF8($dataarr);
//     	    }
//     	}
    	
    	// trim
    	foreach( $dataarr as $index=>$val) {
    	    $argu[$index]  = trim ( $val );
    	}
    	
    	
    	$mostImpCol = $this->impParamDict['importantCol'];
    	$nameData = $argu[$mostImpCol];
    	
    	
    	$this->MethodLib->initLine($datafields_raw);
    	
    	$answer = $this->MethodLib->objPreImport ($sqlo, $argu);
    	if ($error->Got(READONLY))  {
        	$error->set( $FUNCNAME, 1, 'preparation error.' );
    		return;
    	}
    	
    	if ( $this->debug>1 ) {
    	    echo ($cnt+1).". DEBUG:ARGUS: ".glob_array2String( $answer ). "<br>\n";
    	}
    	 
    	if ( $this->doInsert ) {
    		
    		$newid = $this->MethodLib->objDoImport($sqlo, $sqlo2, $sqlo3, $answer);	
    		if ($newid>0 and $this->impParamDict['add2projID']>0 ) {
    			$this->projAddLib->addObj( $sqlo, $this->tablename, $newid );
    		}
    	} else {
    		$newid=1;
    	}
    	if ( $this->debug>1 ) {
    		echo ($cnt+1).". DEBUG: ".glob_array2String( $argu). "<br>\n";
    	}
    	
    	if ($newid<=0 or $error->Got(READONLY) ) {
    		$error->set( $FUNCNAME, 2, 'insert failed of object: "'.$nameData.'"');
    		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
    			echo 'RawData: '. htmlspecialchars( glob_array2String( $argu));
    		}
    		return;
    	}
    	
    	if ( $this->doInsert ) {
    		$this->MethodLib->objPostImport($sqlo, $newid, $argu);
    		if ($error->Got(READONLY) ) {
    			$error->set( $FUNCNAME, 3, 'postImport for object-ID: '.$newid.' failed.');
    			return;
    		}
    	}
    }
    
    private function get_row_info_arr() {
        return $this->MethodLib->row_info_arr();
    }
    
    
    function doImport( &$sqlo, &$sqlo2, &$sqlo3 ) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;

    	echo "<br>";
    	$flushLib = new fProgressBar( ); 
    	$prgopt=array();
    	$prgopt['objname']='rows';
    	$prgopt['maxnum']= $this->maxDataLines;
    	$flushLib->shoPgroBar($prgopt);
    	
    	$this->linecount = 0;
        //$posPK1 = $this->headerPos[$this->primKeys[0]];
        $cnt = 0;
    	$this->sum=array();
    	$this->sum['warn']=0;
    	$this->sum['error']=0;
    	$this->sum['cnt']=0;
    	
    	
    	$tabobj  = new visufuncs();
    	$headOpt = array( "title" => "File content" );
    	$headx   = array_merge( array('#', 'Status', 'Info'), $this->header);
    	$tabobj->table_head($headx,   $headOpt);
    	
    	while( $this->file_read_lib->read_line(0) )  {
    	    
    	    $show_row=1;
    	    $cell_INFO=NULL;
    	    $table_row= array( ($cnt+1)  );
    	    $datafields_raw  = $this->file_read_lib->get_data_row();
    	    $format_row      = $this->file_read_lib->get_format_row();
    	    if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
    	        echo "DDD: datafields_raw: ".print_r($datafields_raw,1)."<br>";
    	        echo "DDD: format_row: ".print_r($format_row,1)."<br>";
    	    } 
    	    
    	    $this->spread_lib->check_data_row($datafields_raw);
    	    if ( $error->Got(READONLY) ) {
    	        // handle later ...
    	    } else {
    	    
        	    $datafields      = $this->spread_lib->getDataByDefColName( $datafields_raw );
        	    $dataarr = $datafields;
        	    $this->_insertOne($sqlo, $sqlo2, $sqlo3, $datafields_raw, $dataarr, $cnt);
    	    }
    	    
    	    $warning_loop='';
    	    if ( $error->Got(CCT_WARNING_READONLY) ) {
    	        // WARNINGS ...
    	        $sub_stack = $error->getAllWarnings();
    	        $sub_array = $error->getAllArray($sub_stack);
	            $tmpSep  = NULL;
	            $errtxt  = NULL;
	            foreach( $sub_array as $oneerror) {
	                $errtxt .= $tmpSep . $oneerror['text'];
	                $tmpSep  = '<br />';
	            }
	            $warning_loop=$errtxt;
    	        $error->resetWarnings();
    	        $this->sum['warn']++;
    	        
    	    } 
    	    
    		if ( $error->Got(READONLY) ) {
    			
    		   
    			if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
    				$errtxt = $error->getAllAsText(NULL, "<br>" );
    			} else {
    				$errarr = $error->getTextAsArray();
    				$tmpSep  = NULL;
    				$errtxt  = NULL;
    				foreach( $errarr as $oneerror) {
    					$errtxt .= $tmpSep . $oneerror;
    					$tmpSep  = '<br />';
    				}

    			}
    			$cell_STATUS = '<span style="color:red;">ERROR</span>';
    			$cell_INFO = 'ERR: '. $errtxt;
    			
    			$error->reset();
    			$this->sum['error']++;
    			
    		} else {
    		    $cell_info_a = $this->get_row_info_arr();
    		    if (!empty($cell_info_a)) $cell_INFO=implode(';',$cell_info_a);
    		    $cell_STATUS='ok';
    		    
    		    if ($warning_loop!='') {
    		        $cell_STATUS = '<span style="color:orange;">WARNING</span>';
    		        $cell_INFO = 'WARN: '. $errtxt.'; '.$cell_INFO; // put warning to front of message ...
    		    }
    		    
    		}
    		
    		$table_row[] = $cell_INFO; // set STATUS
    		$table_row[] = $cell_STATUS; // set Info
    		$table_row = array_merge($table_row, $datafields_raw);
    		
    	    if ( $this->infolevel>2 ) {
    			$show_row=1;
    		}
    		
    		$tabobj->table_row ($table_row);
    	
    		$flushLib->alivePoint($cnt);
    		$cnt++;
    	}
    	
    	$flushLib->alivePoint($cnt,1); // finish
    	$this->sum['cnt'] = $cnt;
    	
    	$this->file_read_lib->close_file();
    	
    	$tabobj->table_close();
    	echo "<br>\n";
    	
    	$visuLib = new fVisuTables();
    	$visuLib->showSummary( $this->sum );
    
    }
    
    function methodHelp(&$sqlo) {
    	if ( !method_exists( $this->MethodLib, 'showHelp') ) return;
    	
    	$this->MethodLib->showHelp($sqlo);
    }
    
    function getParamsDict() {
        return $this->MethodLib->getParamsDict();
    }

    
    function setImpParam($sqlo, $key, $projid) {
        $this->MethodLib->setImpParam($sqlo, $key, $projid);
    }
   
}

/**
 * manage the GUI components of importIII
 */
class gObjtabImp3gui {
	
var $parx;

function __construct( &$sqlo, $parx, $go, $scriptid ) {
	global $error;
	$FUNCNAME= 'gObjtabImp3gui';
	
	$this->scriptid  = $scriptid;
	$this->go   = $go;

	if ( $parx["infolevel"]==NULL ) {
		$parx["infolevel"]=1; // default
	}
	$this->parx = $parx;
	$this->workFileName = '';
	$this->datafileFull = NULL;
	
	$this->dataImpLib = new gObjImp3C($parx, $go);
	
	
	
	if ( $go==1) {
		$uploadObj = new uploadC();
		$this->workFileName = $_FILES['datafile']['name'];
		$this->datafileFull =  $uploadObj->mvUpload2Tmp( 
			$this->scriptid, $this->workFileName, $_FILES['datafile']['tmp_name'], $_FILES['datafile']['name'],
			 $_FILES['datafile']['size'] );
		$this->workDirFull = $uploadObj->getWorkDir();
		$this->parx['fileshort'] = $this->workFileName;
	}
	
	if ( $go==2 ) {
	    
	    $this->workFileName  = $this->parx['fileshort'];
	    
	    if ($this->workFileName==NULL) {
	        $error->set( $FUNCNAME, 2, 'Input missing: fileshort.' );
	        return;
	    }
	    
	    
		$workdirObj = new workDir();
		$this->workDirFull   = $workdirObj->getWorkDir ( $this->scriptid, 1 );
		if ($error->Got(READONLY))  {
			return;
		}
		$this->datafileFull =  $this->workDirFull . DIRECTORY_SEPARATOR . $this->workFileName;
	}
	
	$this->FH = $this->dataImpLib->init($sqlo, $this->datafileFull);
	$this->_infolevels = array( 0=>'silent', 1=>'[only errors]', 2=>'warnings+errors', 3=>'all data' );
	
	if ($go>0) {
	    $paramsDict = $this->dataImpLib->getParamsDict();
	   
	    if ($paramsDict['gui.proj_select']) {
	        if (!$this->parx['projid']) {
	            $error->set( $FUNCNAME, 1, 'Input missing: destination folder.' );
	            return;
	        }
	        $this->dataImpLib->setImpParam($sqlo, 'add2projID', $this->parx['projid']);
	    }
	}
}

function GoInfo($go, $coltxt=NULL) {
	
	$goArray   = array( "0"=>"Upload file", 1=>"Prepare Import", 2=>"Update objects" );
	$extratext = '[<a href="'.$_SERVER['PHP_SELF'].'?parx[methodKey]='.$this->parx['methodKey'].'">Start again</a>]';
	
	$formPageLib = new FormPageC();
	$formPageLib->init( $goArray, $extratext );
	$formPageLib->goInfo( $go ); 

}

function showParams($sqlo) {
	$parx = $this->parx;

	$tabobj = new visufuncs();
	$dataArr= array();
	
	echo "<br>";
	$paramsDict = $this->dataImpLib->getParamsDict();
	$objLinkLib = new fObjViewC();
	
	if ( $paramsDict['add2projID'] ) {
	    $html_tmp = $objLinkLib->bo_display( $sqlo, 'PROJ', $paramsDict['add2projID'] );
	} else $html_tmp='none';
	
	
	$dataArr[] = array( 'File name', $this->workFileName );
	$dataArr[] = array( 'Destination folder', $html_tmp );

	if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
		$lev = $parx['infolevel'];
		$tmpHeader = $this->dataImpLib->getHeader() ;
		$dataArr[] = array( 'InfoLevel:', $this->_infolevels[$lev].' ['.$lev.']' );
		$dataArr[] = array( 'DataLines:',  $this->dataImpLib->getNumLines() );
		$dataArr[] = array( 'Header: (debug)',  glob_array2String( $tmpHeader )  );

	}
	
	$headOpt = array( "title" => "Parameters", "headNoShow" =>1);
	$headx   = array ("Key", "Val");
	$tabobj->table_out2($headx, $dataArr,  $headOpt);
	echo "<br>";
}


function form1(&$sqlo) {
    
    if ($this->parx["infolevel"]===NULL) {
        $this->parx["infolevel"]=3;
    }
    
    $paramsDict = $this->dataImpLib->getParamsDict();
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Upload data file";
	$initarr["submittitle"] = "Next &gt;&gt;";
	$initarr["tabwidth"]    = "AUTO";
	$initarr["ENCTYPE"]		= "multipart/form-data";
	$initarr["dblink"]      = 1;

	$hiddenarr = NULL;
	$hiddenarr["tablename"]     = $this->tablename;
	$hiddenarr["parx[methodKey]"] = $this->parx["methodKey"];
	$formobj = new formc($initarr, $hiddenarr, 0);

	$fieldx = array ( 
		"namex" => TRUE,
		"title" => "Data file", 
		"name"  => "datafile",
		"object"=> "file",
		"val"   => $parx["datafile"], 
		"notes" => "the EXCEL or CSV data file"
		 );
	$formobj->fieldOut( $fieldx );
	
	if ($paramsDict['gui.proj_select']) {

	    $fieldx = array (
	        "title" => "Destination folder",
	        "name"  => "projid",
	        "object"=> "dblink",
	        "inits" => array( 'table'=>'PROJ', 'getObjName'=>1, 'sqlo'=>&$sqlo, 'pos' =>'0', 'projlink'=> 1),
	        "val"   => $this->parx["projid"],
	    );
    	$formobj->fieldOut( $fieldx );
	}
	
	/*
	$fieldx = array ( 
		"title" => "InfoLevel", 
		"name"  => "infolevel",
		"object"=> "select",
		"inits" => $this->_infolevels,
		"val"   => $this->parx["infolevel"], 
		"notes" => "infolevel"
		 );
	$formobj->fieldOut( $fieldx );
	*/

	$formobj->close( TRUE );

	echo "<br>";
	
	$this->dataImpLib->methodHelp($sqlo);
	
	$this->help();
}

function form2(&$sqlo, &$sqlo2, &$sqlo3) {
	
	global $error;
	$FUNCNAME='form2';

	echo "<br>\n";
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Prepare creation";
	$initarr["submittitle"] = "Submit";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["tablename"]     = $this->tablename;
	$hiddenarr["parx[infolevel]"] = $this->parx["infolevel"];
	$hiddenarr["parx[methodKey]"] = $this->parx["methodKey"];
	$hiddenarr["parx[fileshort]"] = $this->parx["fileshort"];
	$hiddenarr["parx[projid]"] = $this->parx["projid"];
	
	$formobj = new formc($initarr, $hiddenarr, 1);
	$formobj->close( TRUE );
	
	
	$this->showParams($sqlo);
	if ($error->Got(READONLY))  {
		return;
	}
	
	$this->dataImpLib->anaHeader( $sqlo );
	$this->dataImpLib->doImport( $sqlo, $sqlo2, $sqlo3 );
	
}

function form3(&$sqlo, &$sqlo2, &$sqlo3) {
	global $error;
	

	$this->showParams($sqlo);
	if ($error->Got(READONLY))  {
		return;
	}
	$this->dataImpLib->doImport( $sqlo, $sqlo2, $sqlo3, $this->FH );
}

function help () {
	
} 

}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
$sqlo2 = logon2(  );
$sqlo3 = logon2(  );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$go		= $_REQUEST["go"];
$parx 	= $_REQUEST['parx'];

$title		= "Data Importer III: method: ". $parx["methodKey"];
$title_sh	= "Importer: ". $parx["methodKey"];

$flushLib = new fProgressBar( ); 

$infoarr			 = NULL;
$infoarr["scriptID"] = "glob.objtab.import3";
$infoarr["title"]    = $title;
$infoarr["title_sh"] = $title_sh;
$infoarr["form_type"]= "tool"; 
$infoarr['design']   = 'norm';
$infoarr['css'] 	 = $flushLib->getCss();
$infoarr['javascript'] = $flushLib->getJS(); 
$infoarr['locrow']   = array( array('home.php', 'home') );

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);
$pagelib->chkErrStop();

if ($parx['methodKey']==NULL) {
	$pagelib->htmlFoot('ERROR', 'No method given');
}

if ($go>0) {
    gHtmlMisc::func_hist( 'glob.objtab.import3?'.$parx['methodKey'], $title_sh,  $_SERVER['PHP_SELF'].'?parx[method]='.$parx['methodKey'] );
}

$mainlib = new gObjtabImp3gui( $sqlo, $parx, $go, $infoarr["scriptID"]  );
$pagelib->chkErrStop();


$mainlib->GoInfo($go);

if ( !$go ) {
	$mainlib->form1($sqlo);
	$pagelib->chkErrStop();
	$pagelib->htmlFoot();
}

if ( $go==1 ) {
    $mainlib->form2($sqlo, $sqlo2, $sqlo3);
	$pagelib->chkErrStop();
	$pagelib->htmlFoot();
}

if ( $go==2 ) {
    $mainlib->form3($sqlo, $sqlo2, $sqlo3);
	$pagelib->chkErrStop();

	echo "<br>";
	cMsgbox::showBox("ok", "O.K."); 

	$pagelib->htmlFoot();
}


$pagelib->htmlFoot();
