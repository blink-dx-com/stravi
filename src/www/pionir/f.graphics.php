<?php
/**
 * produce PNG-graphics from data-tupel
 * - supported types, described in input-dict['gropt']['type']:
 *    'scatter' - scatter plot
 * @package f.graphics.php 
 * @swreq UREQ:0001672: g > graphic > produce Scatterplot by data-tupels 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   string $inp WORKDIR/file.dat (serialized dict)
 * STRUCT of input-dict
	 * 'legendText' = array of text-lines
	 * 'titleArr' = array("title"=>"Scatter plot 1", "x"=>"x_vals", "y"=>"y_vals");
	 * 'gropt' = array of 
	 *   "maxX" = 1.0; 
	     "maxY" = 1.0; 
		 "drawAreaPix" max 600 !
		 'colorind'=> array( colorindex=>array('leg'=>text, 'co'=>) )  corresponds to index colorindex
		 	'leg' => legend text
		 	'co'  => color-name
	   
	   for "type"=="scatter":
	     'dataarr' = array(
	     	colorindex => array([0]=>data_x, [1]=>data_y )
	     	)
	     'dataHelp' => array( array of (x,y) ) to show help lines around the scatter plot
	   for "type"=="line":
	   	  'dataarr' = array(
	     	colorindex => array[0..n] = y_value 
	      )
	      'xAx' = array[0..n] = x_value
 * @example: 
 *  -see test page: www/pionir/rootsubs/test/test.diagram.php
 * 		$gropt["maxX"] = 1.0; // $this->cols[0]["maxval"];
		$gropt["maxY"] = 1.0; // $this->cols[1]["maxval"];
		$gropt["drawAreaPix"] = 300; 
		$gropt["LegendMain"] = 160;
 */
session_start(); 

require_once ('reqnormal.inc');
require_once ("f.grDia.inc");
require_once ('f.workdir.inc');

class fGraphics_imgC {
	var $debug; // 1: more info on grafics; 2: ONYL text, no graphics
	
	function _errorShow($text) {
		fgrDiaC::errorOut($text);
		exit;
	}

	function init( $datainp ) {
		global $error;
		
		$this->graphType=NULL;
		
		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
		    $this->debug=$_SESSION["userGlob"]["g.debugLevel"];
		}
		if ($this->debug>=2) {
			 echo "DEBUG-Mode active!\n";
		}
		
		if ($datainp==NULL) {
			$this->_errorShow('4: No input parameter');
		}
		
		
		$workdirObj = new workDir();
		$tmpdir   = $workdirObj->getSessionMainDir();
		if ($error->Got(READONLY))  {
			$this->_errorShow('1: Workdir error');
		}
		
		$fullPath = $tmpdir .'/'.$datainp;
		
		if (!file_exists($fullPath)) {
			$this->_errorShow('2: DataFile not found.');
		}
		
		// unserialize( base64_decode($tmparr[0]) );
		$this->inpDict = unserialize(base64_decode(file_get_contents($fullPath)));
		
		if (!is_array($this->inpDict)) {
			$this->_errorShow('3: is not a valid Graphics-DataFile.');
		}
		
		$this->graphType = $this->inpDict['gropt']["type"];
		
		if ( $this->inpDict['gropt']["drawAreaPix"]>600 ) {
			$this->inpDict['gropt']["drawAreaPix"]=600;
		}
		
		if ($this->debug>=2) {
			 glob_printr( $this->inpDict, "DEBUG: inpDict info" );
		}
	}

	function  lineplot() {
		global $error;
		
		$gropt    = $this->inpDict['gropt'];
		$colorind = &$this->inpDict['colorind'];
		// glob_printr($dataArr, "dataArr");
		$gropt["type"] = "line";
		$multiSet      = $gropt["multiSet"];

		$legendText =  $this->inpDict['legendText'];
		$titleArr   =  $this->inpDict['titleArr'];
		$helpData   =  NULL;
		$xAx		=  $this->inpDict['xAx'];
		
		if (!$multiSet) {
			$helpData   =  &$this->inpDict['dataarr'];
		} else $helpData   = NULL;
		
		$diaObj = new fgrDiaC($titleArr, $helpData, $gropt, $xAx);
			
		if ($error->Got(READONLY))  {
			$errLast   = $error->getLast();
			$error_txt = $errLast->text;
			$error->reset();
			$this->_errorShow("Error from diagram-builder: ".$error_txt);
			return;
		}
		
		if (!$multiSet) {
			$diaObj->dataDrawLine();
		} else {
		
			$dataarrPoint = &$this->inpDict['dataarr'];
			reset ($dataarrPoint);
			foreach( $dataarrPoint as $index=>$valarr) {
				$legx=$colorind[$index]['leg'];
				$diaObj->setDataArr($valarr, $index, $legx);
				$diaObj->dataDrawLine();
			}
			
		}
		$diaObj->drawLegend($legendText);
		$diaObj->imgOut();
		$diaObj->close();
		
	}
	
	/**
	 * line plot
	 */
	function scatterplot() {
		global $error;
		
		$gropt    = $this->inpDict['gropt'];
		$colorind = &$this->inpDict['colorind'];
		// glob_printr($dataArr, "dataArr");
		$gropt["type"] = "scatter";	

		$legendText =  $this->inpDict['legendText'];
		$titleArr   =  $this->inpDict['titleArr'];
		$dummy		=  NULL;
		
		$diaObj = new fgrDiaC($titleArr, $dummy,$gropt);
			
		if ($error->Got(READONLY))  {
			$errLast   = $error->getLast();
			$error_txt = $errLast->text;
			$error->reset();
			$this->_errorShow("Error from diagram-builder: ".$error_txt);
			return;
		}
		
		if ( sizeof($this->inpDict['dataHelp']) ) {
			$dataarrPoint = &$this->inpDict['dataHelp'];
			reset ($dataarrPoint);
			foreach( $dataarrPoint as $index=>$valarr) {
				$colorx='gray';
				$diaObj->dataDrawScatter( array("color"=>$colorx, "optdata"=>1, "scatLine" =>1), $valarr );
			}
		}
		
		if (sizeof($this->inpDict['dataarr'])) {
			$dataarrPoint = &$this->inpDict['dataarr'];
			reset ($dataarrPoint);
			foreach( $dataarrPoint as $index=>$valarr) {
				$colorx=$colorind[$index]['co'];
				$diaObj->dataDrawScatter( array("color"=>$colorx, "optdata"=>1), $valarr );
			}
			
		}
		$diaObj->drawLegend($legendText);
		$diaObj->imgOut();
		$diaObj->close();
		
	}

}

// --------------------------------------------------- 
global $error, $varcol;
$error = & ErrorHandler::get();

if ( !glob_loggedin() ) {
	htmlFoot("ERROR","not logged in");
}

$sublib = new fGraphics_imgC();
$sublib->init($_REQUEST['inp']);

if ($sublib->graphType=='scatter') {
	$sublib->scatterplot();
	exit;
}
if ($sublib->graphType=='line') {
	$sublib->lineplot();
	exit;
}
$sublib->_errorShow('graphics-type unknown');
