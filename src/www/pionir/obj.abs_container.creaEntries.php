<?php
/**
 * create entries for ABS_CONTAINER
 * $Header: trunk/src/www/pionir/obj.abs_container.creaEntries.php 59 2018-11-21 09:04:09Z $
 * @package obj.abs_container.creaEntries.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param int $id 
 * @param array $parx[] 
 *   'SHELF', 'RACK', 'BOX', 'BOXPOS'  
 *   'alias_policy' :
 *      ['COORD'] : normal
 *      'boxes'   : Boxes: combi: RACK + SHELF == one box "X01"
 *   'alias_prefix' : e.g. 'B'
 * @param int $go
 * 
 */
session_start(); 

require_once ('reqnormal.inc');
require_once ('func_form.inc');
require_once ("f.assocUpdate.inc");
require_once ('f.msgboxes.inc');


class oABS_CONTAINER_creaEntr {
	
function __construct( $id, $parx, $go ) {
    
	$this->id = $id;
	$this->parx = $parx;
	$this->go = $go;
	$this->vararr = array('SHELF', 'RACK', 'BOX', 'BOXPOS'  );
	
	$this->parx['alias_prefix']=trim($this->parx['alias_prefix']);
	
	$this->padding_arr = array('S'=>1, 'R'=>1, 'B'=>1, 'T'=>1);
	$this->padding_arr['S'] =  $this->_num_digits($this->parx['SHELF']);
	$this->padding_arr['R'] =  $this->_num_digits($this->parx['RACK']);
	$this->padding_arr['B'] =  $this->_num_digits($this->parx['BOX']);
	
	
	/**
	 * FUTURE: make this editable by user
	 * 
	 * @var array $boxposArr
	 *    SUM_CNT => array(
	 *       'dim'=>array(X,Y), dimension in box   
	 *       'padding'=> int -- padding for string build e.g. 1 or 2
	 *    )
	 *    
	 */
	$this->boxposArr = array( 
	    1=>  array('dim'=>array(1,1),    'padding'=>1 ) ,
	    3=>  array('dim'=>array(3,1),    'padding'=>1 ) ,
	    6=>  array('dim'=>array(6,1),    'padding'=>1 ) ,
	    96=> array('dim'=>array(112,8),  'padding'=>2 ) ,
	    100=>array('dim'=>array(10,10),  'padding'=>3 ) ,
	    200=>array('dim'=>array(20,10),  'padding'=>3 ) 
	);
}

private function _num_digits($num) {
    $val = intval(log10 ($num)) + 1;
    
    return $val;
}

function showform(&$sqlo) {
	
    $parx= $this->parx;
    
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Prepare";
	$initarr["submittitle"] = "Next &gt;&gt;";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["id"]     = $this->id;
	// $hiddenarr["parx[alias_policy]"]     = $this->parx['alias_policy'];

	$formobj = new formc($initarr, $hiddenarr, 0);
	
	$vararr = $this->vararr;
	
	$boxPosInit=NULL;
	foreach( $this->boxposArr as $key=>$valarr) { 
		$boxPosInit[$key]=$key;
	}
	
	$alias_arr=array('COORD'=>'COORD',  'boxes'=>'Box Names');
	
	$key='alias_policy';
	$fieldx = array (
	    "title" => 'Alias Policy',
	    "name"  => 'alias_policy',
	    "object"=> "select",
	    "val"   => $parx[$key],
	    "inits"=> $alias_arr,
	    "notes" => '[COORD] or Box-Names',
	);
	$formobj->fieldOut( $fieldx );
	$fieldx = array (
	    "title" => 'Box Name Start Letter',
	    "name"  => 'alias_prefix',
	    "object"=> "text",
	    "val"   => $parx['alias_prefix'],
	    "notes" => 'needed for alias="Box Names" e.g. "B"',
	);
	$formobj->fieldOut( $fieldx );
	 

	$lastkey = NULL;
	foreach( $vararr as $dummy=>$key) {
		
		$keyLow  = strtolower($key);
		$addtitle=NULL;
		if ($lastkey!=NULL) {
			$addtitle=' per '. strtolower($lastkey);
		}
		
		$keyLow_many=$keyLow.'s';
		if ($key=='BOXPOS') {
		    $keyLow_many='box-positions';
		}
		
		$fieldx = array ( 
		    "title" => $keyLow_many.$addtitle, 
		"name"  => $key,
		"object"=> "text",
		"val"   => $parx[$key], 
		"notes" => 'no of '.$keyLow.'s', 
		 );
		 
		if ($key=='BOXPOS' ) {
			$fieldx['object']='select';
			$fieldx['inits']= $boxPosInit;
		}
		 
		$formobj->fieldOut( $fieldx ); 
		$lastkey = $key;
	}
	
	
	
	

	$formobj->close( TRUE );
}

function showform2() {
	$parx = $this->parx;
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Prepare";
	$initarr["submittitle"] = "Create";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["id"]     = $this->id;
	$formobj = new formc($initarr, $hiddenarr, 1);
	$formobj->addHiddenParx( $parx );
	
	$formobj->close( TRUE );
} 

function checkParams(&$sqlo) {
	global $error;
	$FUNCNAME= 'checkParams';
	$MAX_NUM=100000;
	
	
	$sqlsel = 'count(1) from ABS_CONT_ENTRY where ABS_CONTAINER_ID='.$this->id;
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$cnt = $sqlo->RowData[0];
	if ( $cnt>0 ) {
 		$error->set( $FUNCNAME, 1, 'Already '.$cnt.' entries on container. Please remove them first!' );
		return;
	}
	
	$parx = $this->parx;
	$product=1;
	$vararr = $this->vararr;
	foreach( $vararr as $key) {
		if ( $parx[$key]<=0 ) {
 			$error->set( $FUNCNAME, 2, 'missing param '.$key );
			return;
		}
		echo $key.':'.$parx[$key].' ';
		$product = $product * $parx[$key];
	}
	
	echo "<br>\n";
	
	$boxCode=$parx['BOXPOS'];
	$this->boxCode = $boxCode;
	$answer = $this->boxposArr[$boxCode];
	

	
	if ( !is_array($answer) ) {
		$error->set( $FUNCNAME, 3, 'Number for Boxpos unknown!' );
		return;
	} 
	$this->boxpos_one = $answer;
	
	if ($product>$MAX_NUM) {
		$error->set( $FUNCNAME, 3, 'MAX number ('.$MAX_NUM.') of entries reached.' );
		return;
	} 
	$this->product=$product;
	
	if ($parx['alias_policy']=='boxes') {
	    if ($parx['alias_prefix']=='') {
	        $error->set( $FUNCNAME, 3, 'if "alias_policy"="boxes", you must set alias_prefix!' );
	        return;
	    }
	}
	
	echo 'Expected entries: <b>'.$product.'</b>'."<br /><br />\n";
}

/**
 * get BOXPOS alias
 * @param int $cnt
 * @param array $boxposarr
 * @return string
 */
private function _getAliasBOXPOS($cnt, $boxposarr) {
    
    
	$policy='NUM'; // 'LETTER'

	$colRowArr = $boxposarr['dim'];
	$padding   = $boxposarr['padding'];
	

	if ( $policy=='LETTER' ) {
		$num = $cnt-1;
		$rest      = fmod( $num, $colRowArr[1]);
		$rest=$rest+1;
		$letter    = intval($num/$colRowArr[1]);
		$boxposOut = chr($letter+ord('A')) . $rest;
	} else {
	    $boxposOut = str_pad( $cnt, $padding, "0", STR_PAD_LEFT );
	}

	
	return ($boxposOut);
}

/**
 * generate ALIAS name and other entry arguments
 * @return 
 * @param array $cntarr
 * @param object $pos
 */
private function _getArgus($cntarr, $pos, &$boxposarr) {
	
	$argu=NULL;
	$argu['POS'] = $pos;
	$vararr = $this->vararr;
	
	$boxposOut = $this->_getAliasBOXPOS($cntarr[3], $boxposarr);
	
	if ($this->parx['alias_policy']=='boxes') {
	    
	    $super_box_id = $this->parx['RACK']*($cntarr[0]-1) + $cntarr[1];  
	    $argu['ALIAS']= $this->parx['alias_prefix'] . str_pad( $super_box_id, 2, "0", STR_PAD_LEFT ) ; // 'T'.$boxposOut; 
	    
	} else {
	
    	$argu['ALIAS']= 
    	   	'S'.$cntarr[0].
    	   	'R'.str_pad( $cntarr[1], $this->padding_arr['R'], "0", STR_PAD_LEFT ).
    	   	'B'.str_pad( $cntarr[2], $this->padding_arr['B'], "0", STR_PAD_LEFT ).
    	    'T'.$boxposOut;        // TBD: T: A3 ???
	}
	
	$i=0;
	foreach( $vararr as $key) {
		$argu[$key] = $cntarr[$i];
		$i++;
	}
	
	
	if ( $pos<20 ) {
		echo 'Info: '.$pos.' '.$argu['ALIAS'].'<br>'."\n";
		if ($pos==19) {
			echo ' ....<br>';
		}
	}
	
	return ($argu);
}

/**
 * create ONE entry
 * @return 
 * @param object $sqlo
 * @param array $cnt
 */
function _creaOne( &$sqlo, $cnt, $pos, &$boxposarr ) {

	$argu = $this->_getArgus($cnt, $pos, $boxposarr);
	$this->assoclib->insert( $sqlo, $argu );
}

/**
 * create the entries
 * @return 
 * @param object $sqlo
 */
function create( &$sqlo ) {
	global $error;
	$FUNCNAME= 'create';
	
	$parx   = $this->parx;
	$max=array();
	$save=array();
	
	$max[0] = $parx['SHELF'];
	$max[1] = $parx['RACK'];
	$max[2] = $parx['BOX'];
	$max[3] = $parx['BOXPOS'];
	
	$this->entries  = 0;
	$this->assoclib = new  fAssocUpdate();
	$this->assoclib->setObj( $sqlo, 'ABS_CONT_ENTRY', $this->id );
	
	$boxposarr = $this->boxpos_one;
	
	$cnt=array();
	
	$pos=1;
	$i=0;
	$cnt[$i] = 1;
	while ( $cnt[$i] <= $max[$i] ) {  // SHELF
		$save[$i] = $cnt[$i];
		$i=1;
		$cnt[$i] = 1;
		while ( $cnt[$i] <= $max[$i] ) { // RACK
			$save[$i] = $cnt[$i];
			$i=2;
			$cnt[$i] = 1;
			while ( $cnt[$i] <= $max[$i] ) { //  BOX
				$save[$i] = $cnt[$i];
				$i=3;
				$cnt[$i] = 1;
				while ( $cnt[$i] <= $max[$i] ) { //  BOXPOS
					$this->_creaOne( $sqlo, $cnt, $pos, $boxposarr );
					if ($error->Got(READONLY))  {
     					$error->set( $FUNCNAME, 1, 'Error on pos: '.$pos.' posarray: '.print_r($cnt,1) );
						return;
					}
					$pos++;
					if ( ($pos/200.0) ==intval($pos/200.0)) {
						echo '.'; // ECHO !
						if ( ($pos/2000.0) ==intval($pos/2000.0)) {
							echo '<br>'."\n";
						}
					}
					$cnt[$i] = $cnt[$i] + 1;
				}
				$i = 2;
				$cnt[$i] = $save[$i];
				$cnt[$i] = $cnt[$i] + 1;
				
			}
			$i = 1;
			$cnt[$i] = $save[$i];
			$cnt[$i] = $cnt[$i] + 1;
		}
		
		$i = 0;
		$cnt[$i] = $save[$i];
		$cnt[$i] = $cnt[$i] + 1;
	}
	 
	
	$this->entries = $pos-1;
}

function getEntries() {
	return ($this->entries);
}

function info() {
    
    echo 'Alias-Policy:'.$this->parx['alias_policy']."<br>";
    echo 'Alias-Prefix:'.$this->parx['alias_prefix']."<br>";
    echo 'Alias-Names : <br>';
    
    for($i=1; $i<5; $i++) {
    	$pos=$i;
    	$cnt=array(1,1,1,$i);
    	$boxposarr = $this->boxpos_one;
    	$argu = $this->_getArgus($cnt, $pos, $boxposarr);
    	// $sampleAlias = $argu['ALIAS'];
    	// echo '-'.$sampleAlias."<br>\n";
    }
	
	
	
}
	
}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$id 		= $_REQUEST['id'];
$parx 		= $_REQUEST['parx'];
$go 		= $_REQUEST['go'];


$tablename	= 'ABS_CONTAINER';
$title		= 'create abstract container entries';

$infoarr			 = NULL;
$infoarr['scriptID'] = '';
$infoarr['title']    = $title;
$infoarr['form_type']= 'obj'; // 'tool', 'list'
$infoarr['design']   = 'norm';

$infoarr['obj_name'] = $tablename;
$infoarr['obj_id']   = $_REQUEST['id'];
$infoarr['checkid']  = 1;

$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo, $infoarr);
$pagelib->do_objAccChk($sqlo);
$pagelib->chkErrStop();

$MainLib = new oABS_CONTAINER_creaEntr( $id, $parx, $go );

if ( !$go ) {
	$MainLib->showform($sqlo);
	$pagelib->htmlFoot();
}
$MainLib->checkParams($sqlo);
$pagelib->chkErrStop();
$MainLib->info();

if ( $go==1 ) {
	$MainLib->showform2();
	$pagelib->htmlFoot();
}

$MainLib->create($sqlo);
$pagelib->chkErrStop();

$entries=$MainLib->getEntries();

cMsgbox::showBox("ok", "<B>".$entries."</B> entries created."); 

$pagelib->htmlFoot();
