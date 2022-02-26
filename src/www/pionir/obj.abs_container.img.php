<?php
/**
 * image of ABS container object
 * @package obj.abs_container.img.php
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
class oABS_CONTAINER_ana1 {
    
    private $coord_type;
	
	function __construct(&$sqlo, $id, $opt, $testflag) {
		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
		    $this->_debug = $_SESSION["userGlob"]["g.debugLevel"];
		}
		

		$this->cid=$id;
		$this->testflag = $testflag;
		$this->opt = $opt;
		$this->coord_type = '';
		
		$this->absContainerLib = new oABS_CONTAINER_subs();
		
	}
	
	function init(&$sqlo) {
		$id = $this->cid;
		$this->filename = $_SESSION['globals']['http_cache_path'] . '/o.ABS_CONTAINER.'.$id.'.png';
		$fileTime=0;
		if ( file_exists($this->filename) ) {
			$fileTime =  filemtime($this->filename);
		}
	
		$sqlsel = '* from ABS_CONTAINER where ABS_CONTAINER_ID='.$this->cid;
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
	
	function initImage(&$sqlo) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
	    $this->absContainerLib->setContainer($sqlo, $this->cid);
		$limits = $this->absContainerLib->getDimensions($sqlo);
		$this->coord_type = $this->absContainerLib->get_coord_type();
		
		$iniOpt=NULL;
		if ($this->testflag) {
			$iniOpt = array( 'limits'=> array('S'=>5, 'R'=>4, 'B'=>16 ) );
		} else {
		    $iniOpt = array( 'limits'=> $limits, 'coord_type'=>$this->coord_type  );
		}
		$this->contImgLib->initImage_Abs( $sqlo, $this->cid, $iniOpt);
		$this->contImgLib->drawContainer();
		
		debugOut('coord_type:'.$this->coord_type, $FUNCNAME, 1);
		if ($this->coord_type=='BOX') {
		    $this->contImgLib->draw_box_names($sqlo);
		}
		
		$this->infox=NULL;	
		$this->limits = $this->contImgLib->limits;
		if ($this->_debug) glob_printr( $this->limits,'limits' );
		
		// max POS
		$sqlsel = 'max(POS) from ABS_CONT_ENTRY where ABS_CONTAINER_ID='.$this->cid;
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
	
	function drawAll(&$sqlo) {
		
		$limits = $this->limits;
		
		for ($S=1; $S<=$limits['S']; $S++) {
			for ($R=1; $R<=$limits['R']; $R++) {
				for ($B=1; $B<=$limits['B']; $B++) {
					
					$infarr = $this->_getBoxStatus($sqlo, $S, $R, $B);
					
					$fillkey =  'none';

					
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

// $table= 'ABS_CONTAINER';
$id   = $_REQUEST['id'];
$opt  = $_REQUEST['opt'];
$testflag = $_REQUEST['test'];

if (!$id) {
	 htmlFoot('ERROR','No ID given');
}

#$filename = 'C:\home\steffen\Coding\Partisan_testdata\1_Container\1.png';

$MainLib = new oABS_CONTAINER_ana1($sqlo, $id, $opt, $testflag);

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


