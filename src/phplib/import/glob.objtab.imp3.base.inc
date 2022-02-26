<?php
require_once 'glob.obj.copyobj2.inc';
/**
 * - base class for glob.objtab.import3.php
 * @package  glob.objtab.import3.php
 * @subpackage  glob.objtab.imp3.base.inc
 * @author   Steffen Kube (steffen@blink-dx.com)
 * @var importArg_STRUCT the argument structure
 *   array('arguMeta'=>$arguMeta, 'opt'=>$insopt)
 */

class imp3_BASE {
    
    protected $projAddLib;
    protected $fileDataRow;
    protected $row_info_arr;
    var $tablename;
    var $impParamDict; /** 
        - major import parameters
        - single item can be modified with setImpParam()
    	@var array impParamDict_STRUCT $impParamDict
    		'add2projID' => ID of project, add object to this project
    		'gui.proj_select'=> 0,1 show project selector
    		'tablename'  => the tablename
    		'colsExpect'=>array(  : expected columns
    		    COLNAME => FLAG : 
    		    	FLAG: 
    		    	  1 : optional
    		    	  2 : required
    		    #examples
    			'SAMPLE-name'=>2,
    			'c:patcode'=>2,
    			'SampleDate'=>2,
    			'Notes'=>1 ),
    		'importantCol'=> 'SAMPLE-name', : most important column in file-row
    		'convertData' => [OPTIONAL] : 'ISO-8859-1-TO-UTF-8'
    		'delimiter'   => [OPTIONAL] : column delimiter; default: \t
    		'headerPattern' => [OPTIONAL] : pattern for start of header
    	*/
    
    
function __construct() {
	$this->insertLib  = new insertC();
	$this->objCreaLib = NULL; // init later
	$this->_row_info_arr=array();
}

function add_row_info($text) {
    $this->_row_info_arr[]=$text;
}

function initLine(&$dataarr) {
	$this->fileDataRow = $dataarr; // one line from file as array
	$this->_row_info_arr=array();
}

private function init_proj($sqlo) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    $proj_id = $this->impParamDict['add2projID'];
    if ($proj_id>0) {
        if ($this->projAddLib==NULL) {
            $this->projAddLib = new oProjAddElem( $sqlo, $proj_id);
            if ( $error->Got(READONLY) ) {
                $error->set( $FUNCNAME, 3, "no insert access to destination-project (ID:".$proj_id.").");
                return;
            }
        }
        
    }
    
}

function importPrep_super($sqlo) {
    $this->init_proj($sqlo);
}
	
function importPrep(&$sqlo) {}

/**
 * post analysis of file-header-array
 * @param $sqlo
 * @param array $headerArr header array
 * @return NULL
 */
function postHeaderAnalysis(&$sqlo, $headerArr) {}

/**
 * object input parameter preparation
 * @param $sqlo
 * @param $argu
 * @return array importArg_STRUCT
 */
function objPreImport (&$sqlo, &$argu) {}

/**
 * do the import
 * CAN BE OVERWRITTEN !
 * @param $sqlo
 * @param array importArg_STRUCT $argu
 * @return int objid
 */
function objDoImport  (&$sqlo, &$sqlo2, &$sqlo3, &$argu) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
	$arguMeta= $argu['arguMeta'];
	$insopt  = $argu['insopt'];

	
	if ( $insopt['make_copy'] ) {
	    
	    if (!$insopt['obj_blueprint_id']) {
	        $error->set( $FUNCNAME, 1, 'input missing: obj_blueprint_id' );
	        return;
	    }
	    
	    $objFeatNew=array('vals'=>$arguMeta);
	    
	    $newid = $this->objCreaLib->objCreate( $sqlo, $sqlo2, $sqlo3,
	        $insopt['obj_blueprint_id'],
	        $objFeatNew, $insopt['deep_copy'], $insopt['toption'] );
	    
	   
	} else {
	    $newid   = $this->insertLib->new_meta($sqlo, $this->tablename, $arguMeta, $insopt);
	}
	return $newid;
}


function objPostImport(&$sqlo, $objid, &$argu) {}
function showHelp(&$sqlo) {}
	
function getParamsDict() {
	return ($this->impParamDict);
}

function setImpParam($sqlo, $key, $value) {
    $this->impParamDict[$key] = $value;
    if ($key=='add2projID') {
        $this->init_proj($sqlo);
    }
}

/**
 * get :   $this->impParamDict = array(
 'tablename'=>'CONCRETE_SUBST',
 'colsExpect'=>array(
 'SAMPLE-name'=>	array('col.name'=>'SAMPLE-name', 'nice'=>'Sample-Name', 'req'=>2, 'ex'=>'B_Vi13687-2 W1'),
 */
protected function help_get_format() {
    $tmparr = $this->impParamDict['colsExpect'];
    
    $outarr=array ( array(), array(),array());
    foreach($tmparr as $col_info) {
        $outarr[0][] = $col_info['nice'];
        $outarr[1][] = $col_info['col.name'];
        $outarr[2][] = $col_info['ex'];
    }
    
    $outarr[0][0] = '# '.$outarr[0][0]; // add comment char
    
    return $outarr;
}

function row_info_arr() {
    return $this->_row_info_arr;
}


}
