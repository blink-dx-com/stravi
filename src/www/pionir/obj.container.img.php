<?php
/**
 * image of container object
 * $Header: trunk/src/www/pionir/obj.container.img.php 59 2018-11-21 09:04:09Z $
 * @package obj.container.img.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param int $id INPUT 
 * @param $opt['refresh'] : 1 : must referesh!
 * @param int $test 0,1
 * @param array $redbox : index: number value "$S,$R,$B"; e.g. 3,2,3
*/

session_start(); 

require_once ('reqnormal.inc');
require_once ('object.info.inc');
require_once ('date_funcs.inc');
require_once ('../../phplib/gui/o.CONTAINER.img.inc');
require_once ('o.ABS_CONTAINER.subs.inc');

/**
 * analyse container
 */
class oCONTAINER_ana1 {
	
	function __construct(&$sqlo, $id, $opt, $testflag) {
		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
		    $this->_debug = $_SESSION["userGlob"]["g.debugLevel"];
		}
		$this->cid=$id;
		$this->testflag = $testflag;
		$this->opt = $opt;
		
		$this->absContainerLib = new oABS_CONTAINER_subs();
		
	}
	
	function init(&$sqlo) {
		$id = $this->cid;
		$this->filename = $_SESSION['globals']['http_cache_path'] . '/o.CONTAINER.'.$id.'.png';
		$fileTime=0;
		if ( file_exists($this->filename) ) {
			$fileTime =  filemtime($this->filename);
		}
	
		$sqlsel = '* from CONTAINER where CONTAINER_ID='.$this->cid;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadArray();
		$this->cfeat   = $sqlo->RowData;
		$CCT_ACCESS_ID = $this->cfeat['CCT_ACCESS_ID'];
		$accInfo = access_data_getai($sqlo, $CCT_ACCESS_ID);
		
		$this->contImgLib  = new oCONTAINER_img();
		
		$fileHumanDate = date_unix2datestr($fileTime, 1);
		// if a new thumbnail already exists: show the thumbnail
		// echo "accMod: ".$accInfo['mod_date'] ." : file:". $fileHumanDate."<br>";
		if ($accInfo['mod_date'] < $fileHumanDate) {
			// show thumbnail
			
			return 1;
		}
		return 0;
	}
	
	/**
	 * 
	 * @param object $sqlo
	 * @return int 0,1
	 */
	function initImage(&$sqlo) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		if (!$this->cfeat['ABS_CONTAINER_ID']) {
			$errarr = array('No '.tablename_nice2('ABS_CONTAINER'). ' connected.');
			$this->contImgLib->errorImage( $errarr  );
			return 0;
		}
		$this->ca_id = $this->cfeat['ABS_CONTAINER_ID'];
		$this->absContainerLib->setContainer($sqlo, $this->cfeat['ABS_CONTAINER_ID']);
		
		if (!$this->absContainerLib->has_coordinates($sqlo)) {
		    $errarr = array(tablename_nice2('ABS_CONTAINER'). ' has no coordinate system.');
		    $this->contImgLib->errorImage( $errarr, 'INFO' );
		    return 0; // no coordinates ...
		}
		$this->coord_type = $this->absContainerLib->get_coord_type();
		
		$limits = $this->absContainerLib->getDimensions($sqlo);
		
		$iniOpt=NULL;
		if ($this->testflag) {
			$iniOpt = array( 'limits'=> array('S'=>5, 'R'=>4, 'B'=>16 ) );
		} else {
		    $iniOpt = array( 'limits'=> $limits, 'coord_type'=>$this->coord_type   );
		}
		$this->contImgLib->initImage_Conc( $sqlo, $this->cid, $this->ca_id, $iniOpt);
		$this->contImgLib->drawContainer();
		
		
		$this->infox=NULL;	
		$this->limits = $this->contImgLib->limits;
		if ($this->_debug) glob_printr( $this->limits,'limits' );
		
		// max POS
		$sqlsel = 'max(POS) from CONT_HAS_CSUBST where CONTAINER_ID='.$this->cid;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$this->infox['maxpos'] = intval($sqlo->RowData[0]);
		
		return 1;
	}
	
	
	/**
	 * get allocation status of a box
	 * @param $sqlo
	 * @param $S
	 * @param $R
	 * @param $B
	 * @return array ('posLast','substid')
	 */
	function _getBoxStatus(&$sqlo, $S, $R, $B) {
		
		$retarr=NULL;
		$retarr = $this->absContainerLib->getBoxPosMinMax($sqlo, $S, $R, $B);
		
		$sqlsel = 'count(CONCRETE_SUBST_ID), count(RESERVED) from CONT_HAS_CSUBST where CONTAINER_ID='.$this->cid.
			' and (POS>='.$retarr['posFirst'].' and POS<='.$retarr['posLast'].' )';
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		
		$retarr['substCnt'] = $sqlo->RowData[0];
		// $retarr['used']    = intval($sqlo->RowData[1]);
		$retarr['reserve'] = $sqlo->RowData[1];
		
		return $retarr;
	}
	
	function dotest() {
		$this->contImgLib->titleText('UNDER CONSTRUCTION: example');
	
		for ($i=1; $i<=16; $i++) {
			$this->contImgLib->oneBoxFill(1, 1, $i, 'fill' );
		}
		
		$this->contImgLib->oneBoxFill(2, 1, 2, 'fill' );
		$this->contImgLib->oneBoxFill(1, 3, 3, 'half' );
		$this->contImgLib->oneBoxFill(2, 2, 1, 'reserve' );
		$this->contImgLib->oneBoxFill(2, 2, 12, 'reserve' );
		
	}
	
	function draw_content(&$sqlo) {
	    
	    
		$limits = $this->limits;
		$limit_25 = 0.25*$limits['BP'];
		$limit_50 = 0.50*$limits['BP'];
		$limit_75 = 0.75*$limits['BP'];
		
		
		
		for ($S=1; $S<=$limits['S']; $S++) {
			for ($R=1; $R<=$limits['R']; $R++) {
				for ($B=1; $B<=$limits['B']; $B++) {
					
					$infarr = $this->_getBoxStatus($sqlo, $S, $R, $B);
					
					$fillkey =  'none';
					if ($infarr['reserve']>0) {
						$fillkey = 'reserve';
					}
					
					$suc_cnt=$infarr['substCnt'];
					if ( $suc_cnt>0 ) {
						$fillkey = 'fill';
						do {
						    if ($suc_cnt < $limit_25 ) {
    							$fillkey = 'fill1';
    							break;
    						}
    						if ($suc_cnt < $limit_50 ) {
    						    $fillkey = 'fill2';
    						    break;
    						}
    						if ($suc_cnt < $limit_75 ) {
    						    $fillkey = 'fill3';
    						    break;
    						}
    						
    						$fillkey = 'fill4';
    						
						} while (0);
					}
					
					$this->contImgLib->oneBoxFill($S, $R, $B, $fillkey );
					
					if ($this->_debug>1) glob_printr( $infarr, "oneBoxFill: $S, $R, $B: key:".$fillkey );
					// check for last pos
					if (!$infarr['posLast'] or $infarr['posLast']>=$this->infox['maxpos']) {
						return;
					}
				}
			}
		}
		
		
		
	}
	
	function drawAll(&$sqlo) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $this->draw_content($sqlo);
	    debugOut('coord_type:'.$this->coord_type, $FUNCNAME, 1);
	    if ($this->coord_type=='BOX') {
	        $this->contImgLib->draw_box_names($sqlo);
	    }
	}
	
	/**
	 * draw red boxes 
	 * @param array $redbox : values : string 'S,R,B'
	 */
	function drawRedBoxes(&$redbox) {
		$tmperr=NULL;
		$limits = $this->limits;
		
		foreach( $redbox as $boxstr) {
			$srb = explode(',', $boxstr);
			if (sizeof($srb)!=3) {
				$tmperr='At least one redbox-param is invalid.';
				continue;
			}
			if ($srb[0]>$limits['S']) $tmperr='S too big';
			if ($srb[1]>$limits['R']) $tmperr='R too big';
			if ($srb[2]>$limits['B']) $tmperr='B too big ('.$srb[2].')';
			if ($tmperr!=NULL) continue;
			
			$this->contImgLib->oneBoxFrame($srb[0], $srb[1], $srb[2]);
		}
		
		if ($tmperr!=NULL) {
			$this->contImgLib->titleText('Error: '.$tmperr,1, 'red');
		}
	}
	
	function showThumb() {
		$this->contImgLib->streamOut($this->filename);
	}
	
	function show() {
		if ($this->_debug) {
			debugOut('Debug was active.', 'show');
			return;
		}
		
		$filename=$this->filename;
		$this->contImgLib->save($filename);
		$this->contImgLib->streamOut($filename);
	}
	
}


global $error;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();

//$table= 'CONTAINER';
$id   = $_REQUEST['id'];
$opt  = $_REQUEST['opt'];
$testflag = $_REQUEST['test'];

if (!$id) {
	 htmlFoot('ERROR','No ID given');
}

#$filename = 'C:\home\steffen\Coding\Partisan_testdata\1_Container\1.png';

$MainLib = new oCONTAINER_ana1($sqlo, $id, $opt, $testflag);

$newThumbExists = $MainLib->init($sqlo);
if ( $newThumbExists and !$opt['refresh']) {
	$MainLib->showThumb();
	return;
}

if ( !$MainLib->initImage($sqlo) ) {
	$MainLib->show();
	return;
}

if ($MainLib->testflag) {
	$MainLib->dotest();
} else {
	$MainLib->drawAll($sqlo);
}

if (!empty($_REQUEST['redbox'])) {
	$MainLib->drawRedBoxes($_REQUEST['redbox']);
}

$MainLib->show();


