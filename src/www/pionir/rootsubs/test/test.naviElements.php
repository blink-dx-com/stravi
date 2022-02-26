<?
/** 
 * test navigation elements
 * @package test.naviElements.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
   INPUT:  $go  0,1
   		$actionx : 
   			'progbar1'
   			'progbar2'
   			'progbar3'
 * $Header: trunk/src/www/pionir/rootsubs/test/test.naviElements.php 59 2018-11-21 09:04:09Z $     
 */
extract($_REQUEST); 
session_start(); 


require_once ("func_head.inc");
require_once ('func_form.inc');
require_once ('globals.inc'); 
require_once ('visufuncs.inc');
require_once('f.progressBar.inc');
require_once 'gui/f.colors.inc';

class tNaviElem{
	
    function __construct($actionx, $go) {
    	$this->actionx=$actionx;
    	$this->go=$go;
    	$this->imagedir = '../../images';
    }
    
    /**
     * search in STANDARD dir www/pionir/images
     * @param string $pattern
     * @param string $dir
     * @param string $bad_pattern  REGEXP
     * @throws Exception
     * @return string[]
     */
    private function getFilesFromDir($pattern, $dir=NULL, $bad_pattern=NULL) {
    	
    	if ($dir==NULL) $dir = $this->imagedir;
    	if ( ( $handle = opendir($dir) )==FALSE ) {
    		throw new Exception("Dir ".$dir." not found");
    	}
    	closedir($handle);
    	
    	$result = array();
    	
    	foreach (glob($dir.'/{'.$pattern.'}', GLOB_BRACE) as $filename_full) {
    		$file = basename($filename_full);
    		if ($bad_pattern) {
    		    if( preg_match ( '/'.$bad_pattern.'/' , $file) ) {
    		        continue;
    		    }
    		}
    		$result[] = $file;
    	}
    	
    	sort($result);
    	
    	return $result;
    
    }
     
    function head($name) {
    	echo "<table cellpadding=5 cellspacing=1 border=0 width=100% bgcolor=#D0D000>";
    	echo "<tr><td>";
    	echo "<b>".$name."</b>";
    	echo "</td><tr></table>\n";
    	
    	echo "<ul><br>\n";
    }
    function headend() {
    	echo "</ul><br>\n"; 
    }
    
    /**
     * 
     * @param string $title
     * @param array $iconarr
     *  ("Icon-Name", "Notes")
     */
    function _iconTable($title, $iconarr, $basedir='../../images/' ) {
        
    	$tabobj = new visufuncs();
    	$headOpt = array( 
    	    "title" => $title,
    		"colopt" => array("0"=>"#EFBBEF"),
    	);
    						
    	$headx  = array ("#", "Icon", "Icon-Name", "Notes");
    	$tabobj->table_head($headx,   $headOpt);
    	foreach( $iconarr as $i=> $dataArr) {
    		$iconx = $dataArr[0];
    		$dataArrTab = array( ($i+1), '<img src="'.$basedir . $iconx. '" style="max-height: 40px;">', $iconx,   $dataArr[1] );
    		$tabobj->table_row ($dataArrTab);
    	}
    	
    	$tabobj->table_close();
    }
    
    
    /**
     * 
     * @param string $title
     * @param array $icon_filenames of icon_file_names
     * @param array $def_arr of
     *   icon_file_name => notes
     */
    private function _iconTable2($title, $icon_filenames, $def_arr) {
    	
    	$iconarr = NULL;
    	foreach($icon_filenames as $onefile) {
    		
    		$notes=NULL;
    		if ($def_arr[$onefile]!=NULL) $notes=$def_arr[$onefile];
    		
    		$iconarr[] = array($onefile, $notes);
    	}
    	
    	$this->_iconTable($title, $iconarr);
    	
    	
    }
    /////////////////////////////////
    		
    function n_progressbar(&$flushLib) 
    {
      $this->head("Progress bar GUI");
      
      echo '<a href="'.$_SERVER['PHP_SELF'].'?actionx=progbar1">Activate progress1</a>';
      echo ' | <a href="'.$_SERVER['PHP_SELF'].'?actionx=progbar3">Activate progress3</a> (long)';
      echo ' | <a href="'.$_SERVER['PHP_SELF'].'?actionx=progbar4">Activate progress4</a> (with autoinc)<br>';
      
      $MAXNUM = 135;
      $prgopt = array();
      $prgopt['objname']='rows';
      $prgopt['maxnum']= $MAXNUM;
      $flushLib->shoPgroBar($prgopt);
      $flushLib->alivePoint(40,1); // 
    		  
      if ( $this->actionx=='progbar1' ) {
    	$cnt=3;
    	$flushLib->alivePoint(0,1); // start
    	
    	while ( $cnt < $MAXNUM )
    	{ 
    		$flushLib->alivePoint($cnt);
    		usleep(100000);
    		$cnt++;
    
    	}
    	$flushLib->alivePoint($cnt,1); // finish
      }
      
    	if ( $this->actionx=='progbar3' ) {
    		$cnt=1;
    		$MAXNUM = 100;
    		$prooption=array('maxnum'=>$MAXNUM);
    		$flushLib->setNewLimits($prooption);
    		$flushLib->alivePoint(0,1); // start
    		$useSleep = 500000;
    		
    		while ( $cnt < $MAXNUM )
    		{ 
    			$flushLib->alivePoint($cnt);
    			usleep($useSleep);
    			echo "debug_info: cnt:".$cnt." tpo:" . $flushLib->vpart_tpo." ind:".$flushLib->vpart_index. 
    				" lastCnt:". $flushLib->vpart_lastCnt. "<br>\n";
    			$cnt++;
    			
    			$useSleep += 10000;
    	
    		}
    		$flushLib->alivePoint($cnt,1); // finish
        }
        
        if ( $this->actionx=='progbar4' ) {
        	// auto increment
        	$cnt=1;
        	$MAXNUM = 100;
        	$prooption=array('maxnum'=>$MAXNUM);
        	$flushLib->setNewLimits($prooption);
        	$flushLib->alivePoint(0,1); // start
        	$sleep_sec = 3;
        	$flushLib->alivePoint(20,1);
        	
        	$flushLib->alivePointAuto(1, 2.0); // auto increment every 2 seconds
        
        	while ( $cnt < 5 )
        	{
        		sleep($sleep_sec);
        		echo "debug_info: cnt:".$cnt."<br>";
        		$cnt++;
        
        	}
        	
        	$flushLib->alivePointAuto(2);
        	$flushLib->alivePoint($MAXNUM,1); // finish
        }
      
      $this->headend();
    }
    
    /**
     * CHAR progress
     */
    function n_progressbar2($flushLib) {
      $this->head("Progress bar CHAR");
      
      echo '<a href="'.$_SERVER['PHP_SELF'].'?actionx=progbar2">Activate progress</a><br>';
      
      $MAXNUM = 300;
      $prgopt=array();
      $prgopt['objname']='rows';
      $prgopt['maxnum']= $MAXNUM;
      $prgopt['mode']  = 'char';
      
      
      $flushLib->shoPgroBar($prgopt);
      echo '... shows one char per time-slice.<br />';
    		  
      if ( $this->actionx=='progbar2' ) {
    	$cnt=2;
    	
    	$flushLib->alivePoint(0,1); // start
    	
    	while ( $cnt < $MAXNUM )
    	{ 
    		$flushLib->alivePoint($cnt);
    		usleep(50000);
    		$cnt++;
    
    	}
    	$flushLib->alivePoint($cnt,1); // finish
      } else {
      	$flushLib->alivePoint($cnt,1); // finish
      }
      $this->headend();
    }
    
    function n_bargraphDigit() {
        $this->head("Bar Graph DIGITS");
        
        $digits=array(
            array('c'=>'green'),
            array('c'=>'#D0FFD0'),
            array(),
            array('c'=>'blue'),
            array('c'=>'blue'),
            array('c'=>'gray'),
        );
        $options=array(
            'imgmaxlen' =>300,
            'img_height'=>20 
        );
        
        $out = fProgressBar::getBarStaticDigits($digits, $options);
        echo $out."<br>";
    }
    
    function n_bargraph() {
    	require_once('gui/f.htmlGraph2.inc');
    	
    	$this->head("Bar Graph/Chart");
    	echo 'Possible colors: blue, gray,blue_li,  green, violett,  orange,  yellow<br>';
    	
    	$tabobj = new visufuncs();
    	$headOpt = array( "title" => 'Bar Graph Examples');
    	$headx  = array ("Bar",  "Notes");
    	$tabobj->table_head($headx,   $headOpt);
    	
    	
    	
    	$opt=array('imgUrlPre'=>'../../');
    	$tmplib = new fHtmlGraph2C(250, 400,$opt);
    	
    	$url = $tmplib->getImgHtml(250, NULL, 0);
    	$dataArrTab=array($url, 'MAX 250');
    	$tabobj->table_row ($dataArrTab);
    	
    	$url = $tmplib->getImgHtml(92, 'orange', 0);
    	$dataArrTab=array($url, 'normal 92');
    	$tabobj->table_row ($dataArrTab);
    	
    	$url = $tmplib->getImgHtml(92, 'blue_li', 0);
    	$dataArrTab=array($url, 'normal 92');
    	$tabobj->table_row ($dataArrTab);
    	
    	
    	$url = $tmplib->getImgHtml(92, 'green', 0);
    	$dataArrTab=array($url, 'green 92');
    	$tabobj->table_row ($dataArrTab);
    	
    	$url = $tmplib->getImgHtml(92, 'violett', 1);
    	$dataArrTab=array($url, 'violett, withFillImg 92');
    	$tabobj->table_row ($dataArrTab);
    	
    	$opt=array('imgUrlPre'=>'../../', 'stripHeight'=>20);
    	$tmplib = new fHtmlGraph2C(250, 300,$opt);
    	$url = $tmplib->getImgHtml(100, 'violett', 1);
    	$dataArrTab=array($url, 'FAT strip, violett, withFillImg, 100');
    	$tabobj->table_row ($dataArrTab);
    	
    	$url = $tmplib->getImgHtml(150, 'yellow', 1);
    	$dataArrTab=array($url, 'FAT strip, yellow, withFillImg, 150');
    	$tabobj->table_row ($dataArrTab);
    	
    	
    	$tabobj->table_close();
    	
    	$this->headend();
    }
    
    private function _icons_prep($button_arr, $notes_arr) {
        
        $iconarr = array();
        foreach($button_arr as $onefile) {
            $notes=NULL;
            if ($notes_arr[$onefile]) $notes=$notes_arr[$onefile];
            $iconarr[] = array($onefile, $notes);
        }
        return $iconarr;
    }
    
    function n_buttons() {
    		
    	$this->head("Buttons2");
        
    
    	// get all buttons
    	$buttons  = $this->getFilesFromDir('but.*.*');
    	$buttons2 = $this->getFilesFromDir('i*',NULL, '^icon|^i13');
    	$buttons4 = $this->getFilesFromDir('f*');
    	$buttons3 = $this->getFilesFromDir('*.svg', '../../res/img');
    	
    	// rest
    	$buttons5 = $this->getFilesFromDir('*',NULL, '^i|^obj\.|^f|^but');
    	
    	$buttons = array_merge($buttons, $buttons2, $buttons4);
    	
    	$title = "Button icons";
    	$iconarr = NULL;
    	foreach($buttons as $onefile) {
    		$iconarr[] = array($onefile, '');
    	}
    	$this->_iconTable($title, $iconarr);
    	echo "<br>";
    	
    	$notes_arr=array();
    	$notes_arr['chevrons-right.svg']='forward';
    	$iconarr = $this->_icons_prep($buttons3, $notes_arr);
    	$this->_iconTable('SVG onres/img/', $iconarr, '../../res/img/');
    	
    	$title = "The rest";
    	$iconarr = NULL;
    	foreach($buttons5 as $onefile) {
    	    $iconarr[] = array($onefile, '');
    	}
    	$this->_iconTable($title, $iconarr);
    	echo "<br>";
    	
    	$this->headend();
    }
    
    function n_icons() {
    	$this->head("Icons");
    
    	$title = "icons 13x13";
    	$def_arr = array(
    		"i13_ok.gif" =>	"ok",
    		"i13_err.gif" =>	"error",
    		"i13_ok2.gif" =>	"ok gray",
    		"i13_warning.gif" =>"warning",
    		"i13_gray.gif" =>	"not active",
    		"i13_inwork.gif" =>	"in work"	,
    		"i13_nouse.gif" =>	"invalid, not usable",
    		"i13_info.gif" =>	"info"	,
    		"i13_infog.gif" =>	"info green",
    		"i13_ask.gif" =>	"ask"			 
    		);
    	
    	$buttons = $this->getFilesFromDir('i13*');
    	
    	$this->_iconTable2($title, $buttons, $def_arr);
    	echo "<br>";
    	
    	$this->headend();
    }
    
    function n_colors() {
        
        $this->head("DiffColors");
        
        $colors_raw = fColors::DIFF_COLORS;
        
        $tabobj = new visufuncs();
        $headOpt = array( "title" => 'DiffColors'  );
        
        $headx  = array ("#", "Color-Image", "Color-Code");
        $tabobj->table_head($headx,   $headOpt);
        
        $i=0;
        foreach( $colors_raw as $color) {
            
            $dataArrTab = array( $i, '<div style="width:30px; height:30px; background-color:'.$color.'">&nbsp;</div>', $color );
            $tabobj->table_row ($dataArrTab);
            $i++;
        }
        
        $tabobj->table_close();
        
        $this->headend();
    }
    
    
    
    function n_infotabs() {
    	require_once ('f.msgboxes.inc');
    	
    	$this->head("Info-Tables/Message boxes");
    	echo "Call:  [cMsgbox::showBox()]<br>";
    	
    	cMsgbox::showBox("ok", "Message Box ok."); 
    	echo "<br>";
    	cMsgbox::showBox("error", "Message Box error. "); 
    	echo "<br>";
    	cMsgbox::showBox("warning", "Message Box warning.");
    	echo "<br>";
    	cMsgbox::showBox("info", "Message Box info.");
    	echo "<br>";
    	
    	
        htmlErrorBox("Error", "Error head text", "Detailed descriptiondescription [htmlErrorBox()]"  );
    	echo "<br>";
        htmlInfoBox( "Message head: INFO", "Message detail [htmlInfoBox()]", "", "INFO" );
    	htmlInfoBox( "Message head: HELP", "Message detail [htmlInfoBox()]", "", "HELP" );
    	htmlInfoBox( "Message head: CALM", "Message detail [htmlInfoBox()]", "", "CALM" );
    	htmlInfoBox( "Message head: ERROR", "Message detail [htmlInfoBox()]", "", "ERROR" );
    	htmlInfoBox( "Message head: WARN", "Message detail [htmlInfoBox()]", "", "WARN" );
    	htmlInfoBox( "Message head: WARNRED", "Message detail [htmlInfoBox()]", "", "WARNRED" );
    	$this->headend();
    }
    
    function n_navigation() {
    	require_once ('f.pageSubs.inc');
    	
    	$this->head("Navigation");
    	$goarray = array(
    		array("url"=>"url1", "txt"=>"Link 1"),
    		array("url"=>"url2", "txt"=>"Link 2"),
    		);
    	
    	$pagesubObj = new fPageSubs();
    	$pagesubObj->navigateNext($goarray);
    	
    	
    	
    }

}

$sqlo  = logon2( $_SERVER['PHP_SELF'] ); 
$funcname = "test.naviElements";                
$flushLib = new fProgressBar( );

$title = "Navigation-Element-Test";
$infoarr			 = NULL;
$infoarr["scriptID"] = "";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "tool"; 
$infoarr["locrow"] 	= array( array("../rootFuncs.php", "Administration"),  array("index.php", "test home") );
$infoarr['css'] 	= $flushLib->getCss(1);
$infoarr['javascript'] = $flushLib->getJS(); 

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr); 

echo "<ul>";

$mainLib = new tNaviElem($actionx, $go);

$mainLib->n_progressbar($flushLib,  $go);
$mainLib->n_progressbar2($flushLib);
$mainLib->n_bargraph();
$mainLib->n_bargraphDigit();

$mainLib->n_buttons();
$mainLib->n_icons();
$mainLib->n_colors();
$mainLib->n_infotabs();
$mainLib->n_navigation();


htmlFoot();
