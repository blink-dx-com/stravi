<?php
/**
 * test diagrams 
 * @package test.diagram.php
 * @swreq UREQ:0001675: g > TEST> testscript for diagramms   
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc");
require_once ('func_form.inc');
require_once ('visufuncs.inc');
require_once ("f.grDia.inc");
require_once 'gui/f.htmlGraph2.inc';

class fTestDiagram {

function __construct() {
	global $error;
	
	$this->tabobj = new visufuncs();
	
	require_once ('f.workdir.inc');
	$workdirObj = new workDir();
	
	$this->subdir = "test.diagram.php";
	$this->tmpdir   = $workdirObj->getWorkDir ( $this->subdir );
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, '1: Workdir error');
	}
}
function _head($name) {
	echo "<br><font color=#8080FF><b>".$name."</b></font><br><hr size=1 noshade color=#8080FF>\n";
	echo "<ul>\n";
}
function _headend() {
	echo "</ul><br>\n"; 
}

function _showInfo( $info ) {
	$dataArr= NULL;
	$headColor="#E0E0FF";
	$headOpt = array( "title" => "bargraph-Diagram programming features", "headNoShow" =>1);
	$headx   = array ("Key", "Val");
	$this->tabobj->table_head($headx, $headOpt);
	
	$mainLineOpt = array( "bgcolor" =>$headColor, "colspan" => 2 );
	
	if ($info["req"]!="") { 
		$reqPoi = &$info["req"];
		$dataArr = array( "require_once" );
		$this->tabobj->table_row(  $dataArr, $mainLineOpt );
		
		foreach( $reqPoi as $dummy=>$oneReq) {
			$requFile =$oneReq["f"];
			$requtitle=$oneReq["tit"];
			$dataArr = array(  "<b>".$requFile."</b>", $requtitle );
			$this->tabobj->table_row($dataArr);
		}
		reset ($reqPoi); 
		 
		
	}
	$this->tabobj->table_close();
	echo "<br>\n";
}

function linediag() {
	global $error;	
	$this->_head("Line-plot");
	echo "<img src=\"test.diagram_img.php\"> ";
	echo "<img src=\"test.diagram_img.php?mode=line&variant=1\"> ";
	
	
	
	// -----------------------
	
	
	$subdir  = $this->subdir;
	$tmpdir = $this->tmpdir;
	$filePur = "graphDataLine.dat";
	$inp = $subdir .'/'.$filePur;

	// produce data
	$number = 10;
	$maxval = 1.0;
	$fact=$maxval/$number;
	
	$dataArr = NULL;
	$signum=1;
	
	
		$i=0;
		while ( $i < $number ) {
			$xLoopOff = $maxval/10 * $signum * $color*0.5;
			$xax[$i] = $i;
			$dataArr[$i] = $i*$fact;
			$i++;
			$signum=$signum * -1;
		}
	
	$colorind= array('black', 'red');
	
	
	$gropt = array("type"=>"line");	
	
	// Fix scaling
	//$gropt["maxX"] = 1.0; // $this->cols[0]["maxval"];
	//$gropt["maxY"] = 1.0; // $this->cols[1]["maxval"];
	$gropt["drawAreaPix"] = 300; 
	$gropt["LegendMain"] = 160;
	$gropt["titleDY"]    = 52;
	
	$legendText[] = "- line plot";
	$legendText[] = array( "t"=> "text1 - +", "c"=>"black");
	
	
	$titleArr = array("title"=>"Line plot: f.graphics.php", "x"=>"x_vals", "y"=>"y_vals",
		'titlearr'=>array('Subtitle1 go yes', 'Subtilte2 WgWWWWWW', 'Subtilte2 WWWWWWWW'));
	
	$inputArr = array(
		'legendText'=>$legendText,
		'gropt'     =>$gropt,
		'colorind'  =>$colorind,
		'titleArr'  =>$titleArr,
		'dataarr'   =>$dataArr,
		'xAx'		=>$xax,
	
		
	);
	$destFullPath = $tmpdir . '/' . $filePur;
	
	if (!$handle = fopen($destFullPath, "w")) {
		print "Kann $destFullPath nicht zum Schreiben oeffnen! <br>";
		return (-1);
	}
	if (   !fwrite( $handle, base64_encode(serialize($inputArr)) )   ) {
		print "Kann in die $filenameNice nicht schreiben<br>";
		return (-1);
	}
	fclose($handle);
	
	echo '<img src="../../f.graphics.php?inp='.$inp.'">'."<br>";
	
	$this->_headend();
}
	
	


function scatter() {
	global $error;	
	$this->_head("Scatter-plot");
	echo "<img src=\"test.diagram_img.php?mode=scatter\"><br>";
	
	
	$subdir  = $this->subdir;
	$tmpdir = $this->tmpdir;
	$filePur = "graphData.dat";
	$inp = $subdir .'/'.$filePur;
	
	
	// produce data
	$number = 10;
	$maxval = 1.0;
	$fact=$maxval/$number;
	
	$dataArr = NULL;
	$signum=1;
	
	for ( $color=0; $color<=2; $color++) {
		$i=0;
		while ( $i < $number ) {
			$xLoopOff = $maxval/10 * $signum * $color*0.5;
			$dataArr[$color][0][$i] = $i*$fact + $xLoopOff;
			$dataArr[$color][1][$i] = $i*$fact;
			$i++;
			$signum=$signum * -1;
		}
	}
	$colorind= array('black', 'red');
	
	// helplines 
	$dataHelp=array(
		array( array(0,0.7,0.9) , array(0.2, 0.75, 0.95) ),
		array( array(0.1,0.6, 0.9), array(0.1, 0.6, 0.8) ),
		);
	
	// glob_printr($dataArr, "dataArr");
	$gropt = array("type"=>"scatter");	
	
	// Fix scaling
	$gropt["maxX"] = 1.0; // $this->cols[0]["maxval"];
	$gropt["maxY"] = 1.0; // $this->cols[1]["maxval"];
	$gropt["drawAreaPix"] = 300; 
	$gropt["LegendMain"] = 160;
	
	$legendText[] = "- scatter plot";
	$legendText[] = array( "t"=> "text1 - +", "c"=>"black");
	
	
	$titleArr = array("title"=>"Scatter plot: f.graphics.php", "x"=>"x_vals", "y"=>"y_vals");
	
	$inputArr = array(
		'legendText'=>$legendText,
		'gropt'     =>$gropt,
		'colorind'  =>$colorind,
		'titleArr'  =>$titleArr,
		'dataarr'   =>$dataArr,
		'dataHelp' => $dataHelp
	);
	$destFullPath = $tmpdir . '/' . $filePur;
	
	if (!$handle = fopen($destFullPath, "w")) {
		print "Kann $destFullPath nicht zum Schreiben oeffnen! <br>";
		return (-1);
	}
	if (   !fwrite( $handle, base64_encode(serialize($inputArr)) )   ) {
		print "Kann in die $filenameNice nicht schreiben<br>";
		return (-1);
	}
	fclose($handle);
	
	echo '<img src="../../f.graphics.php?inp='.$inp.'">'."<br>";
	$this->_headend();
}

function bar1() {
	global $error;	
	$this->_head("Bargraph1");
	$infopt = array(
				"req" => array ( 
							array("f"=>"f.grBarDia.inc", "tit"=>"bargraph-Diagram programming features")
							)
				    );
	$this->_showInfo( $infopt );
	
	echo "<img src=\"test.diagram_img.php?mode=bar1\">";
	echo "<img src=\"test.diagram_img.php?mode=bar2\">";
	echo "<img src=\"test.diagram_img.php?mode=bar3\"><br>";
	echo "<img src=\"test.diagram_img.php?mode=bar4_err\"><br>";
	
	$this->_headend();
}

function barEasy() {
	$this->_head("Bargraph-Easy (IMG-Tag-based");
	$infopt = array(
			"req" => array (
					array("f"=>"gui/f.htmlGraph2.inc", "tit"=>"horizontal bargraph-Diagrams")
			)
	);
	$this->_showInfo( $infopt );
	
	$maxval = '43.5';
	$maxPix = 400;
	$options=NULL;
	$options['imgUrlPre']='../../';
	$graphlib = new fHtmlGraph2C($maxval, $maxPix,$options);
	echo 'Info: Maxval:'.$maxval."<br>";
	echo  $graphlib->getImgHtml(10, '', 0) . "<br>";
	echo $graphlib->getImgHtml(20, '', 1). "<br>";
	echo $graphlib->getImgHtml(30, '', 1). "<br>";
	echo $graphlib->getImgHtml(40, '', 1). "<br>";
	
	$this->_headend();
}

function histogram() {
	global $error;	
	$this->_head("Histogram");
	$infopt = array( "req"=> array(
						array( "f"=>"f.grBarDia.inc", "tit"=> "Bragraphs"),
						array( "f"=>"f.math.hist.inc", "tit"=> "Histogram features")
						) 
					);
	$this->_showInfo( $infopt );
	
	echo "<img src=\"test.diagram_img.php?mode=histogram\"><br>";
	$this->_headend();
}

}


global $error, $varcol;

$sqlo  = logon2( $_SERVER['PHP_SELF'] ); 
$error = & ErrorHandler::get();
if ($error->printLast()) htmlFoot();
 
$funcname = "test.diagram";                
$title = "Diagram-Test";

$infoarr			 = NULL;
$infoarr["scriptID"] = "";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "tool"; 
$infoarr["locrow"] = array( array("../rootFuncs.php", "Administration"),  array("index.php", "test home") );

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr); 

$mainLib = new fTestDiagram();
if ($error->Got(READONLY))  {
	$error->printAll();
	return;
}

echo "<ul>\n"; 

$tabobj = new visufuncs();

$dataArr= NULL;
$dataArr[] = array(  "require_once", "f.grDia.inc" );
$headOpt = array( "title" => "Diagram programming features", "headNoShow" =>1);
$headx   = array ("Key", "Val");
$tabobj->table_out2($headx, $dataArr,  $headOpt);
	
echo "<br>";
 
$mainLib->linediag();
$mainLib->scatter();
$mainLib->bar1();
$mainLib->barEasy();
$mainLib->histogram();


htmlFoot();