<?php
/* MODULE:   test.diagram_img.php
   FUNCTION: test diagrams Image 
   INPUT:  $mode = ["line"], "scatter"
   		   $show = ["dia"], "data"
		   $variant = 1,2,3
        
*/
extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc");
require_once ('func_form.inc');
require_once ('visufuncs.inc');
require_once ("f.grDia.inc");
require_once ("f.grBarDia.inc");

class fTestDiagramImg {

function __construct($mode, $show, $variant=0) {
	$this->mode = $mode;
	$this->show = $show;
	$this->varia = $variant;
}



function linediag_1() {
	global $error;
	
	$dataArr = array (1,3,5,7,2,1,2,1,2,1,);
	$xAx     = range(0, sizeof($dataArr)-1);
	
	$grOpt = NULL;
	$titleArr = array("title"=>"Line diagram 1", "x"=>"timePoints", "y"=>"load");
	$diaObj = new fgrDiaC(
		$titleArr, // array("title", "x", "y")
		$dataArr,  // array[0..n] = value
		$grOpt,
		$xAx       // array[0..n] = x_value
		);
		
	if ($error->Got(READONLY))  {
		$errLast   = $error->getLast();
		$error_txt = $errLast->text;
		$error->reset();
		fgrDiaC::errorOut ("Error from diagram-builder: ".$error_txt);
		return;
	}
	
	$diaObj->dataDrawLine();
	$diaObj->imgOut();
	$diaObj->close();
	
}

function linediag_2() {
	// Multiline
	global $error;
	
	$dataArr = array (1,3,5,7,2,1,2,1,2,1,);
	$xAx     = range(0, sizeof($dataArr)-1);
	
	$grOpt = array("maxY" => 10.0, "multiSet"=>1, "LegendMain"=>200 );
	$titleArr = array("title"=>"Line diagram 1", "x"=>"timePoints", "y"=>"load");
	$dataDummy=NULL;
	$diaObj = new fgrDiaC(
		$titleArr, // array("title", "x", "y")
		$dataDummy,  // array[0..n] = value
		$grOpt,
		$xAx       // array[0..n] = x_value
		);
		
	if ($error->Got(READONLY))  {
		$errLast   = $error->getLast();
		$error_txt = $errLast->text;
		$error->reset();
		fgrDiaC::errorOut ("Error from diagram-builder: ".$error_txt);
		return;
	}
	
	$diaObj->setDataArr($dataArr, 4, "erster");
	$diaObj->dataDrawLine();
	
	$dataArr = array (5,3.3,5.8,7.4,2,1,2,1,6,7,);
	$diaObj->setDataArr($dataArr, 1, "zweiter");
	$diaObj->dataDrawLine();
	
	$dataArr = array (9,3.3,5.8,7.4,2,1,2,1,6.3,7.3,);
	$diaObj->setDataArr($dataArr, 2, "drei");
	$diaObj->dataDrawLine();
	
	$diaObj->imgOut();
	$diaObj->close();
	
}


function scatterplot() {
	global $error;
	
	$number = 25;
	$dataArr = NULL;
	$colorx=0;
	while  ( $colorx < 2 ) {
		$i=0;
		while ( $i < $number ) {
			$dataArr[$colorx][0][$i] = rand(0,100)*0.01;
			$dataArr[$colorx][1][$i] = rand(0,100)*0.01;
			$i++;
		}
		$colorx++;
	}
	// glob_printr($dataArr, "dataArr");
	$gropt = array("type"=>"scatter");	
	$gropt["LegendMain"] = 160;
	
	// Fix scaling
	$gropt["maxX"] = 1.0; // $this->cols[0]["maxval"];
	$gropt["maxY"] = 1.0; // $this->cols[1]["maxval"];
	$gropt["drawAreaPix"] = 300; 
	
	$legendText[] = "- scatter plot";
	$legendText[] = "- two colors";
	$legendText[] = "- right hand legend";
	$legendText[] = "- Fix limits for x and y";
	$legendText[] = "";
	$legendText[] = array( "t"=> "text1 - +", "c"=>"black");
	$legendText[] = array( "t"=> "text2 - +", "c"=>"green");
	
	$titleArr = array("title"=>"Scatter plot 1", "x"=>"x_vals", "y"=>"y_vals");
	$diaObj = new fgrDiaC(
		$titleArr, // array("title", "x", "y")
		$dataArr[0],  // array[0..n] = value
		$gropt
		);
		
	if ($error->Got(READONLY))  {
		$errLast   = $error->getLast();
		$error_txt = $errLast->text;
		$error->reset();
		fgrDiaC::errorOut ("Error from diagram-builder: ".$error_txt);
		return;
	}
	
	$diaObj->dataDrawScatter( array("color"=>"black", "optdata"=>1), $dataArr[0] );
	$diaObj->dataDrawScatter( array("color"=>"green", "optdata"=>1), $dataArr[1] );
	$diaObj->drawLegend($legendText);
	$diaObj->imgOut();
	$diaObj->close();
	
}

}

class fTestBarDiaImg {

function __construct($mode, $show) {
	$this->mode = $mode;
	$this->show = $show;
}

function bar1($width, $errorsim=NULL)	{
	global $error;
	
	$obj_inf = array ( 
				"0"=>array("Exp0"),
				"1"=>array("Exp1"),
				"2"=>array("exp2 long"),
				"3"=>array("last erxp"),
				);
	
	$quant_inf = array(
				"0"=>array("name"=>"quant0"),
				"1"=>array("name"=>"quant1"),
				"2"=>array("name"=>"quant2"),
				"3"=>array("name"=>"quant3 long"),						  						   						   
					  );
	$quantVals = array(
				"0"=> array(123,0,367, 420 ),
				"1"=> array(3,145, 5.5,1),
				"2"=> array("0"=>100,  "2"=>200, "3"=>300),	
				"3"=> array(),			
				);
	if ( $width==2 ) {
		$quantVals["2"] = array("0"=>100,  "2"=>200, "3"=>"Gippie");	
	}
	
	$grOpt  = NULL;
	$titles = array("title"=>"Bar diagram 1 fontalias:".$width, "x"=>"objects", "y"=>"quant-value");
	$infoxz = NULL;
	$optarr = array( "debug"=> $_SESSION["userGlob"]["g.debugLevel"], "scmin"=>"0","scmax"=>450, "fontalias"=>$width  );
	
	if ( $errorsim>0 ) {
		$obj_inf = NULL;
		$titles  = array("title"=>"Bar diagram 4 Error simulation", "x"=>"objects", "y"=>"quant-value");
	}
	
	
	$diaObj = new fgrBarDiaC();
	$diaObj->init ( 
		$quant_inf,
		$obj_inf,
		$quantVals,
		$infoxz, 	// data min/max info
		$titles, 	// array ( "main", "x", "y" )
		$optarr  );
		
	if ($error->Got(READONLY))  {
		$errLast   = $error->getLast();
		$error_txt = $errLast->text;
		$error->reset();
		$diaObj->errorImage ($titles["title"]."\nError from diagram-builder: ".$error_txt);
		return;
	}
	
	$diaObj->drawAll();
}

function histogram() {
	require_once("f.math.hist.inc");
	global $error;
	
	$dataArr0 = array(1,2,3,4,5,6,7,80,90,100, 1.5, 1.6, 17, 18, 50.2, 50.5, 55, 100);
	$dataArr1 = array(10,2,30,4,5,6,7,80,90, 40, 42, 50.5, 51, 100);
	$BINS    = 10;
	$histLib = new fMathHistC();
	$histOpt = array("MAX"=>200);
	$histLib->create($dataArr0, $BINS, $histOpt);
	$histArr = $histLib->getBins();
	$stats   = $histLib->getStats();
	$maxVal0  = max($histArr);
	
	$i = 0;
	$xAx = NULL;
	$visuArr=NULL;
	$step    = $stats["MAX"]/$BINS;
	while ( $i<$BINS ) {
		list($xNow, $yval) = each( $histArr );
		$xAx[] = sprintf("%.2f", $xNow);
		$visuArr[] = $yval;
		$i++;
	}
	
	$histLib->create($dataArr1, $BINS, $histOpt);
	$histArr = $histLib->getBins();
	$stats   = $histLib->getStats();
	$maxVal1  = max($histArr);
	
	$maxVal = max($maxVal0, $maxVal1);
	
	$i = 0;
	
	$visuArr1=NULL;
	$step    = $stats["MAX"]/$BINS;
	while ( $i<$BINS ) {
		list($xNow, $yval) = each( $histArr );
		$visuArr1[] = $yval;
		$i++;
	}
	
	/*
	echo "Show only data:<br>\n";
	glob_printr( $histArr, "histArr" );
	glob_printr( $visuArr, "show" );
	glob_printr( $xAx, "xax" );
	exit;
	*/
	
	
	$grOpt = array("drawAreaPix"=>300, "maxY"=>$maxVal);
	$titleArr = array("title"=>"Histogram", "x"=>"quantities", "y"=>"load");
	$diaObj = new fgrDiaC(
		$titleArr, // array("title", "x", "y")
		$visuArr,  // array[0..n] = value
		$grOpt,
		$xAx       // array[0..n] = x_value
		);
		
	if ($error->Got(READONLY))  {
		$errLast   = $error->getLast();
		$error_txt = $errLast->text;
		$error->reset();
		fgrDiaC::errorOut ("Error from diagram-builder: ".$error_txt);
		return;
	}
	
	$diaObj->dataDrawBar();
	$drawOpt=array( "color" => "green");
	$diaObj->dataDrawBar(1, $visuArr1, $drawOpt);
	$diaObj->imgOut();
	$diaObj->close();
}

}

global $error, $varcol;

$error = & ErrorHandler::get();
if ($error->printLast()) htmlFoot();
         
if ( $mode=="" ) $mode="line";      



switch ( $mode ) {
	case "line":
		$grDiaLib = new fTestDiagramImg($mode, $show, $variant);
		if (!$variant) $grDiaLib->linediag_1();
		else  $grDiaLib->linediag_2();
		break;
	case "scatter":
		$grDiaLib = new fTestDiagramImg($mode, $show);
		$grDiaLib->scatterplot();
		break;
	case "bar1":
		$grBarDiaLib = new fTestBarDiaImg($mode, $show);
		$grBarDiaLib->bar1(2);
		break;
	case "bar2":
		$grBarDiaLib = new fTestBarDiaImg($mode, $show);
		$grBarDiaLib->bar1(1);
		break;
	case "bar3":
		$grBarDiaLib = new fTestBarDiaImg($mode, $show);
		$grBarDiaLib->bar1(3);
		break;
	case "bar4_err":
		$grBarDiaLib = new fTestBarDiaImg($mode, $show);
		$grBarDiaLib->bar1(1, 1);
		break;
	case "histogram":
		$grBarDiaLib = new fTestBarDiaImg($mode, $show);
		$grBarDiaLib->histogram();
		break;
		
}

