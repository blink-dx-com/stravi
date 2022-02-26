<?php
/**
 * analyse HTTP log
 * $Header: trunk/src/www/pionir/rootsubs/f.userFuncsAnalyze.php 59 2018-11-21 09:04:09Z $
 * @package f.userFuncsAnalyze.php 
 * @author  Steffen Kube
 * @global $_SESSION['globals']["f.userFuncsAnalyze"] : BASE-path for the web-log-files
 * @params
INPUT:  $go  0,1
        $parx[logfile] 
        $parx[action] => "analyze" | 
                         "actlink" | 
                         "llist"   : colorize new hosts
                        
        $parx["hostip"] 
        $parx["extall"]  => [0] | 1 all file extensions?
                            0: only php
                            1: all (also html,...)
        $parx["numShowFuncs"] => number of shown lines
        $parx["searchtext"]   => search text
        $parx["txtrefneg"]    => TEXT (ignore text from referer)
        $parx["txtref"]       => TEXT ( search in referer ) 
        $parx["hostipneg"]    => TEXT ( negative host )
		$parx["shreqtime"]    => 0|1 ( show request time ) 
		$parx["shreqtimemin"]    => NUMBER ( show line with request time >= shreqtimemin ) 
		$parx["lineStart"]	  => e.g. 34555 line number to start
 */
extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc");
require_once ('func_form.inc');
require_once ('visufuncs.inc');

class gHttpLogAnaC {
	function __construct() {
		global $error;
		$FUNCNAME= 'gHttpLogAnaC';
		
		$this->dirbase = $_SESSION['globals']["f.userFuncsAnalyze"];
		if ($this->dirbase=="")  $this->dirbase = "/usr/local/apache/logs/";
		
		if ( !file_exists($this->dirbase)) {
			$error->set( $FUNCNAME, 1, 'Directory "'.$this->dirbase.'" not found.' );
			return;
		}
		if ( !is_readable($this->dirbase)) {
			$error->set( $FUNCNAME, 2, 'Directory "'.$this->dirbase.'" not readable.' );
			return;
		}
	}

}

// ----------------
 
function this_infox( $keyx, $valx ) {
    echo "<tr><td><font color=gray>$keyx:</font></td><td><B>$valx</B></td></tr>\n";
}  

function this_shortTxt( &$text ) { 
      if (strlen($text) > 250) {
            $text_out = substr($text,0,250)."...";
      } else  $text_out = $text; 
      return ($text_out);
}
 
function getNumberofLines ( $filename ) {
    $linecnt = 0;
    $fp = fopen ( $filename, "r" );
    while (!feof ($fp)) {
        $buffer = fgets($fp, 33000);
        $linecnt++;
    }
    fclose ( $fp );
    return ($linecnt);
}   

function this_gethostname( $hostip ) {
    global $hostNameArr;
    
    if ( isset($hostNameArr[$hostip]) ) {
        $hostname = $hostNameArr[$hostip];
    } else {  
        $hostname = gethostbyaddr ( $hostip );
        $hostNameArr[$hostip] = $hostname;
    }  
    return $hostname;
} 
                      

$POS_HOST    = 0;
$POS_DATE    = 3;
$POS_REQTIME = 4;	// requeset time (seconds)
$POS_URL     = 6;
$POS_REFERER = 10;
$POS_AGENT   = 11;
$TEXT_MAX_LEN= 250; // reduce length, because some virus attacks have text-length of 32800 bytes !

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
		
$funcname = "f.userFuncsAnalyze";

$title = "WebLog-Analyzer";

$infoarr["title"] = $title;
$infoarr["form_type"]= "tool";

$infoarr["locrow"]= array( array("rootFuncs.php", "home" ) );

$pagelib = new gHtmlHead();
$pagelib->_PageHead ( $infoarr["title"],  $infoarr );

?>
<script language="JavaScript">
<!-- 
   function thisadvanc( isval ) {
   
      if ( isval==1 ) doval = "0";
      else    doval = "1";
      document.editform.go.value="0";
      document.editform.elements['parx[showpar]'].value = doval;  
      document.editform.submit();
   }
//-->
</script>
<?php

$pagelib->_startBody($sqlo, $infoarr);


$MainLib = new gHttpLogAnaC();
$pagelib->chkErrStop();

$dirbase = $MainLib->dirbase;

echo "<UL>\n";          

$iniarr["logfile"] = "/usr/local/apache/logs/access_log";
$iniarr["numShowFuncs"] = 30;     
$iniarr["linesAnal"]    = 10000;
$iniarr["showRelevant"] = 1;
$iniarr["action"]       = "analyze";  
$iniarr["showpar"]      = "0"; 
$iniarr["shreqtime"]    = "0";
  
if ( !$go AND !is_array($parx) ) {   // parx["logfile"]
    // get from userGlobals 
    if ($_SESSION['userGlob'][$funcname]!="") $parx = unserialize($_SESSION['userGlob'][$funcname]);
}

if ( $parx["linesAnal"]    =="" )  $parx["linesAnal"]    = $iniarr["linesAnal"]; 
if ( $parx["numShowFuncs"] =="" )  $parx["numShowFuncs"] = $iniarr["numShowFuncs"];
if ( $parx["logfile"]      =="" )  $parx["logfile"]      = $iniarr["logfile"];
if ( $parx["action"]       =="" )  $parx["action"]       = $iniarr["action"];
if ( $parx["showpar"]      =="" )  $parx["showpar"]      = $iniarr["showpar"];
if ( $parx["shreqtime"]      =="" )$parx["shreqtime"]    = $iniarr["shreqtime"];

if ( !$go AND !isset($parx["showRelevant"])  )  $parx["showRelevant"] = $iniarr["showRelevant"];

if ( !$_SESSION['s_suflag'] && ($_SESSION['sec']['appuser']!="root") ) {
  echo "Sorry, you must be root or have su_flag.";
  return 0;
}   

gHtmlMisc::func_hist( "f.userFuncsAnalyze", $title,  'rootsubs/f.userFuncsAnalyze.php' );   

if ( !$go ) {
    
    if ( $parx["showpar"] ) { // is advanced
        $tmpclick = "<input type=button name='dummy' value='Normal' OnClick=\"thisadvanc('".$parx["showpar"]."')\">";
        $addurl = $tmpclick." <B>Advanced</B>";
    } else {                  
       $tmpclick = "<input type=button name='dummy' value='Advanced' OnClick=\"thisadvanc('".$parx["showpar"]."')\">";
       $addurl =  "<B>Normal</B> ".$tmpclick;
    }
    $initarr   = NULL;
    $initarr["action"] = $_SERVER['PHP_SELF'];
    $initarr["title"]  = "WebLog &nbsp;&nbsp;&nbsp;&nbsp;\n".$addurl;        
    $initarr["submittitle"] = "Analyze now!"; 
    
    $hiddenarr["parx[showpar]"] =  $parx["showpar"];
         
    $formobj = new formc( $initarr, $hiddenarr, $go );
     
    $fieldx = array ("title" => "Log_file", "name"  => "logfile", fsize => 40, colspan=>"2",
                     "val"   => $parx["logfile"], "notes" => " log-file on dir: ".$dirbase, "object" => "text" ); 
    $formobj->fieldOut( $fieldx );  
    
     
    $fieldx = array ("title" => "Lines_analyze", "name"  => "linesAnal", "fsize" => 6,
					 "addobj" => "&nbsp;&nbsp; or START line <input name=\"parx[lineStart]\" value=\"".$parx["lineStart"]."\" size=6>",
                     "val"   => $parx["linesAnal"], "notes" => "number of last lines to analyze", "object" => "text" );        
    $formobj->fieldOut( $fieldx );
    
    $fieldx = array ("title" => "Show_lines", "name"  => "numShowFuncs",  "fsize" => 6,
                     "val"   => $parx["numShowFuncs"], "notes" => "Show number of lines [40]", "object" => "text" );        
    $formobj->fieldOut( $fieldx );     
         
    $fieldx = array ("title" => "Action", "name"  => "action",
                     "inits" => array("analyze" => "Overview", "actlink" => "Activate Log", "llist" => "Long list"), 
                     "val"   => $parx["action"], "notes" => "overview, activate, long list", "object" => "select" );        
    $formobj->fieldOut( $fieldx );   
              
    if ( $parx["showpar"] ) {
    
        $fieldx = array ("title" => "All_types", "name"  => "extall", "bgcolor" => "OPTIONAL",
                        "val"   => $parx["extall"], "notes" => "Show all file types? (not checked: only php-files)", 
                        "inits" => 1, "object" => "checkbox" );        
        $formobj->fieldOut( $fieldx ); 

        $fieldx = array ("title" => "Show_agent", "name"  => "showagent", "bgcolor" => "OPTIONAL",
                        "val"   => $parx["showagent"], "notes" => "Show agent ?", 
                        "inits" => 1, "object" => "checkbox" );        
        $formobj->fieldOut( $fieldx );  
        
        $fieldx = array ("title" => "Show_only_relevant", "name"  => "showRelevant", "bgcolor" => "OPTIONAL",
                     "val"   => $parx["showRelevant"], "notes" => "Show only relevant scripts, NOT: frame.left.php, main.fr.php, ...", 
                     "inits" => 1, "object" => "checkbox" );        
        $formobj->fieldOut( $fieldx );
    
        
        $fieldx = array ("title" => "Tab_format", "name"  => "showtab", "bgcolor" => "OPTIONAL",
                        "val"   => $parx["showtab"], "notes" => "Tab separated view?", 
                        "inits" => 1, "object" => "checkbox" );        
        $formobj->fieldOut( $fieldx );  
		
		$fieldx = array ("title" => "Show lineno", "name"  => "linenoshow", "bgcolor" => "OPTIONAL",
                        "val"   => $parx["linenoshow"], "notes" => "Show line numbers?", 
                        "inits" => 1, "object" => "checkbox" );        
        $formobj->fieldOut( $fieldx );  
		
		$tmpnotes = "Show needed request time?";
		$fieldx = array ("title" => "Request time", "name"  => "shreqtime", "bgcolor" => "OPTIONAL",
						"addobj" => "&nbsp;&nbsp; Minimum <input name=\"parx[shreqtimemin]\" value=\"".$parx["shreqtimemin"]."\" size=3> [sec]",
                        "val"   => $parx["shreqtime"], "notes" => $tmpnotes, 
                        "inits" => 1, "object" => "checkbox" );        
        $formobj->fieldOut( $fieldx ); 
		  
    }
    
    $fieldx = array ("title" => "Optional search parameters", "object" => "info" );        
    $formobj->fieldOut( $fieldx ); 
    
    $fieldx = array ("title" => "Search_text", "name"  => "searchtext", fsize => 30,
                     "val"   => $parx["searchtext"], "notes" => "search for text", "object" => "text" ); 
    $formobj->fieldOut( $fieldx );
	
    if ( $parx["showpar"] ) {
		$fieldx = array ("title" => "Start_time", "name"  => "timestart", fsize => 30,
						 "val"   => $parx["timestart"], "notes" => "time e.g. '2007-05-01:11:44'", "object" => "text" ); 
		$formobj->fieldOut( $fieldx ); 
		 
		$fieldx = array ("title" => "End_time", "name"  => "timeend", fsize => 30,
						 "val"   => $parx["timeend"], "notes" => "time e.g. '2007-05-01:12:30'", "object" => "text" ); 
		$formobj->fieldOut( $fieldx ); 
    }
	
    $fieldx = array ("title" => "Host_IP", "name"  => "hostip", fsize => 16,
                     "val"   => $parx["hostip"], "notes" => "parts of IP, analyze log from IP only: e.g. 169.192.1.3", "object" => "text" ); 
    $formobj->fieldOut( $fieldx ); 
    
    if ( $parx["showpar"] ) {   
        $fieldx = array ("title" => "Referer", "name"  => "txtref", fsize => 16,
                     "val"   => $parx["txtref"], "notes" => "Search text in referer-url: e.g. 'google'=&gt; than it hilights the google-search-text", "object" => "text" ); 
        $formobj->fieldOut( $fieldx ); 
    }
    if ( $parx["showpar"] ) {     
    
        $fieldx = array ("title" => "Optional NEGATIVE parameters", "object" => "info", "bgcolor" => "OPTIONAL");        
        $formobj->fieldOut( $fieldx );

        $fieldx = array ("title" => "Host_IP_ignore", "name"  => "hostipneg", fsize => 16, "bgcolor" => "OPTIONAL",
                        "val"   => $parx["hostipneg"], "notes" => "parts of IP, ignore IP", "object" => "text" ); 
        $formobj->fieldOut( $fieldx ); 

        $fieldx = array ("title" => "Referer_ignore", "name"  => "txtrefneg", fsize => 16, "bgcolor" => "OPTIONAL",
                        "val"   => $parx["txtrefneg"], "notes" => "Ignore hits from referer-url: e.g. 'http://lims.'", "object" => "text" ); 
        $formobj->fieldOut( $fieldx ); 
    
   } 
                                        
    
      
    $formobj->close( TRUE );   
    return;
}   

     
    // 
    // GO = 1
    //
    
        
    if (sizeof($parx)) {
        // remove empty fields to save memory    
        foreach( $parx as $idx=>$valx) {
            if ($valx=="")  unset ($parx[$idx]); 
        } 
        if (sizeof($parx)) reset ($parx);
        
        $_SESSION['userGlob'][$funcname] = serialize($parx); // save parameters in session-params
    }
    
	
	$filePur  = $parx["logfile"];
	if ( strstr('/',$filePur)!=NULL ) {
		htmlFoot("Error", "Invalid file-name (1)");
	}
	if ( strstr('\\',$filePur)!=NULL ) {
		htmlFoot("Error", "Invalid file-name (2)");
	}
    $filename = $dirbase . $filePur;  
    
    $initarr   = NULL;
    $initarr["action"]      = $_SERVER['PHP_SELF'];
    $initarr["title"]       = "Refine or Reload";        
    $initarr["submittitle"] = "Reload";
    $initarr["goNext"]      = 1; 
    
    $taction  = $parx["action"];
    $listformat = "";
    if ($parx["action"]=="llist") {
       $taction     = "actlink";
       $listformat  = "long";
    }
     
    $hiddenarr = NULL;
    if ( sizeof($parx) ) {
        foreach( $parx as $idx=>$valx) {
              $hiddenarr['parx['.$idx.']']    = $valx;
        }
        reset($parx); 
    }
    $formobj = new formc( $initarr, $hiddenarr, $go );
    
    $formobj->close( TRUE ); 
	
	
     
    
    if ($parx["logfile"] == "" ) {
        htmlFoot("Error", "Give a file-name");
    }
	
	if (!file_exists($dirbase) ) { 
         htmlFoot("Error", "Base-directory '".$dirbase."' does not exist / is not readable.");
    } 
	
    /* $realname = realpath($filename);
	if (  $realname != $filename ) {
		 htmlFoot("Error", "Log-file '".$filename."' not found! (real:$realname)");
	}
	*/
	
    if ( !is_readable($filename) ) { 
         htmlFoot("Error", "File '".$filename."' does not exist / is not readable.");
    }
    if ($parx["numShowFuncs"]> 500) {        
         htmlFoot("Error", "too big number: Num_show_funcs:".$parx["numShowFuncs"]."");
    }
    
    echo "<table bgcolor=#EFEFEF border=0>\n";
    
    $actionhtml = "Analyze log";
    if ( $parx["action"] == "actlink") $actionhtml = "Show detailed lines, activate them." ;
    if ( $parx["action"] == "llist") $actionhtml    = "Show detailed lines (with referer info)." ;
    this_infox( "Action", $actionhtml ); 
    
    this_infox( "File", "'".$filename."'");    
    
    $numlines = getNumberofLines ( $filename );
	$numLinesStart = $numlines -  $parx["linesAnal"];
	if ($parx["lineStart"] > 0)  $numLinesStart = $parx["lineStart"];
	$numLinesAna = $numlines -  $numLinesStart;
	
    if (!$numlines)   htmlFoot("Error", "File contains no lines"); 
    
    this_infox( "Lines to analyze", $numLinesAna. "  &nbsp;&nbsp;&nbsp;</B><font color=gray>Lines in file:</font> ".$numlines);
    if ($parx["timestart"]!="") 
        this_infox( "Time range",$parx["timestart"]." =&gt; ".$parx["timeend"]); 
    
    $tmpinfo = "";
    if ( $parx["hostipneg"] ) $tmpinfo = " </B><font color=gray>ignore:</font> <B>".$parx["hostipneg"]."</B>";              
    if ( $parx["hostip"] OR $parx["hostipneg"] ) this_infox("From host only", $parx["hostip"].$tmpinfo );
     
    if ($parx["searchtext"] !="") this_infox("Search for text: ", $parx["searchtext"]); 
    
    $tmpshow= "all!";
    if ($parx["showRelevant"])  $tmpshow= "only relevant script (NOT frame.left.nav.php, main.fr.php, icono_svr.php )";
    this_infox( "Show all scripts?", $tmpshow ); 
    
    $tmpshow= "only PHP-files";
    if ($parx["extall"])  $tmpshow= "show ALL file-types (php, html, ...)";
    this_infox( "Show_All_types?", $tmpshow );
    
    $tmpinfo = "";
    if ($parx["txtrefneg"]) {
        $tmpinfo ="<I>Ignore:</I>". $parx["txtrefneg"];
    } 
    if ($parx["txtref"]!="" OR $parx["txtrefneg"]!="") {
        if ($parx["txtref"]=="google") $tmpinfo = "(HILITE google question!) ". $tmpinfo;
        this_infox( "Hits_from_referer", $parx["txtref"]." ".$tmpinfo );
    }
    echo "</table>\n";
    
    echo "<hr>";
    

	
    if ( $numLinesStart<0 ) $numLinesStart=0;
    echo "Start-line: $numLinesStart<br>\n";
    
    $fp = fopen ( $filename, "r" );
    if ( $taction == "actlink" ) {
        echo "<pre>";
    }
    $linecnt = 0; 
    $numLinesAnlyzed = 0;
    $numLinesFound   = 0;
    $numLinesShown   = 0;
    $hostArr         = NULL;
    
    while (!feof ($fp)) { 
        
        $linecnt++;
        
        $lognow = 0;
        $buffer = fgets($fp, 33000 ); // have a big number, because of Virus attacks
        
        if ( $buffer=="" ) continue;
         
        if ( $linecnt < $numLinesStart ) continue;  // start analysis ?
                         
        $tmparr = explode(" ", $buffer);
        $url    = $tmparr[$POS_URL];
        $host   = $tmparr[$POS_HOST];
		$time   = $tmparr[$POS_DATE];
		$reqtime= $tmparr[$POS_REQTIME];
        if ( substr($time,0,1)=="[" ) $time   = substr($tmparr[$POS_DATE],1);
        $referer= $tmparr[$POS_REFERER];
        $agent  = $tmparr[$POS_AGENT];

        do {
            if ( $parx["timestart"] !="" ) {
                if ( ($time<$parx["timestart"]) OR ($time>$parx["timeend"])) break;
            }

            if ( $parx["hostip"] AND     strstr($host,$parx["hostip"])   ==NULL )  break;
            if ( $parx["hostipneg"] AND  strstr($host,$parx["hostipneg"])!=NULL )  break;

            if ( $parx["searchtext"]!="" ) { // search for text ???
                if (strstr($url,$parx["searchtext"]) == NULL) break;
            } 

            if ( $parx["txtref"]!="" ) { // search for text ???
                if (strstr($referer, $parx["txtref"]) == NULL) break;
            } 

            if ( $parx["txtrefneg"]!="" ) { // ignore referer-text ???
                if (strstr($referer, $parx["txtrefneg"]) != NULL) break;
            }
			
			if ( $parx["shreqtime"]>0 AND $parx["shreqtimemin"]!="" ) {	// test for a minimum request time
				 if ($reqtime < $parx["shreqtimemin"]) break;
			}
			
            $lognow = 1;
            
        } while (0);
        
        if (!$numLinesAnlyzed) {
            $buffer_out = this_shortTxt($buffer);
            echo "<font color=gray>First analyzed line:".$buffer_out."</font><br>\n";
        } 

        if ($lognow) { 
        
            $tmppos1 = 0; 
            if ( $parx["extall"] OR (($tmppos1 = strpos($url, ".php"))!== FALSE)  ) {  

                $hostArr[$host]++;

                if ( $taction == "analyze" ) {   // overview !!!

                    if (!$tmppos1) {
                        $urlpure = substr($url,0, $TEXT_MAX_LEN ) ; // if not PHP-file
                    } else $urlpure = substr($url,0,$tmppos1+4);

                    // remove the "?"
                    // if ( ($tmppos2 =  strpos($url, "?", $tmppos1)) !== FALSE ) {     
                    // }  

                    if (strstr($urlpure, "header.php")) {
                        if ( ($tmppos2 = strpos($url, "info_user=")) !== FALSE  ) {

                            $tmpposU = $tmppos2+10;   // satrt of user name
                            $lenghtx = strlen($url)-$tmpposU;
                            // another following parameter ?
                            if ( ($tmppos3 = strpos($url, "&", $tmpposU)) !== FALSE  )
                                $lenghtx = $tmppos3 - $tmpposU;
                            $tmpuser   = substr ( $url, $tmpposU, $lenghtx );

                            if ( ($tmppos4 = strpos($url, "info_db=")) !== FALSE  ) {
                                $tmpposD = $tmppos4+8;   // start of user name
                                $tmpuser .= "@" . substr ( $url, $tmpposD );
                            }
                            $loguserarr[$tmpuser]++;
                            $loguser2arr[$tmpuser][$host]++; 

                        } 
                    }
                    $statarr[$urlpure]++;
                } 

                if ( $taction == "actlink" ) { // long list
                    $showthis = 1;
					$lineout  = "";
					
                    $urlhtml = substr($url,0, $TEXT_MAX_LEN ) ; // reduce lenght                        
                    if ( $parx["showRelevant"] ) {  
                        if ( strstr( $urlhtml, "frame.left.nav.php") != NULL )  $showthis = 0; 
                        if ( strstr( $urlhtml, "main.fr.php"   ) != NULL )      $showthis = 0;   
                        if ( strstr( $urlhtml, "icono_svr.php"   ) != NULL )    $showthis = 0;
                    }

                    if ($showthis) {  
						if ($parx["linenoshow"]) $lineout  = $linecnt." ";
                    	$lineEndHtml = "";
                        $host_out = this_gethostname( $host );
                        if ( $parx["showtab"] ) $host_out = str_pad( $host_out, 30, " ", STR_PAD_RIGHT);
                        if ($host_last == $host) {
							$host_out    = "<font color=\"#808080\">".$host_out;
        					$lineEndHtml = "</font>";
						}
						
						echo $lineout . $host_out." ".$time." ";
						if ($parx["shreqtime"]>0) echo "$reqtime ";
                        if   ($listformat=="") echo "<a href=\"$urlhtml\" target=_new>$urlhtml</a>";
                        else {
                            if (strlen($referer_out) > $TEXT_MAX_LEN) {
                                    $referer_out = substr( $referer, 0, $TEXT_MAX_LEN )."...";
                            } else  $referer_out = $referer;
                            if ( $parx["txtref"]!="" ) {
                                if ($parx["txtref"]=="google" AND ($refpos1=strpos($referer_out, "q=")) ) {
                                     // hilight GOOGLE search strings
                                     $refpos2     = strpos($referer_out, "&",$refpos1);
                                     if (!$refpos2) $refpos2=strlen($referer_out);
                                     $len2        = $refpos2-$refpos1-2; 
                                     $referer_out = substr($referer_out,0,$refpos1)."q=<B>".substr($referer_out,$refpos1+2,$len2).
                                                    "</B>&".substr($referer_out,$refpos2+1);
                                     
                                }
                            }
                            echo "$urlhtml $referer_out";
                            if ($parx["showagent"]) echo " ".$agent;
                        }
                        echo $lineEndHtml."\n"; // </font> comes from $host_out
                        if ($numLinesShown > $parx["numShowFuncs"]) {
                            echo "<font color=gray>... stopped, after $numLinesShown lines were shown.</font>\n";
                            break;  
                        }
                        $numLinesShown++; 
                        
                        $host_last = $host; // remember
                        
                    } else {
                        $numNotRelevant++;
                    }    
                }   

                $numLinesFound++;
            }

            // if ( $numLinesAnlyzed<10 ) echo "url: $url, pure:$urlpure stat:".$statarr[$urlpure]."<br>\n"; 
        } 
        $numLinesAnlyzed++;
        
    
    } // end WHILE  
    
    fclose ( $fp );
    if ( $taction == "actlink" ) {
        echo "</pre>";
    } 
    
    
    // statistik
echo "<hr>"; 
echo "<br><B>Statstics</B><BR><br>\n";

echo "Number of analyzed lines: ".$numLinesAnlyzed."<br>"; 
echo "Number of found matches : <font color=green>".$numLinesFound."</font><br>"; 
if ( $taction == "actlink" ) {
    echo "Number of shown lines : <font color=green>".$numLinesShown."</font><br>"; 
    echo "Number of not-relevant matches : ".$numNotRelevant."<br>";
} 
    
if ( $taction == "analyze" ) { 
    
    if ( sizeof ($statarr) ) { 
    
        arsort($statarr);
    
    } else {
         htmlFoot("Bizarre", "No PHP script detected");
    }
       
    echo "Number of found scripts: ".sizeof($statarr)."<br>";

    echo "<table cellpadding=1 cellspacing=1 border=0 bgcolor=#B0B0B0>";
    echo "<tr bgcolor=#D0D0D0><td>Rank</td><td>Hits</td><td>Script</td></tr>\n";               
    $color1 = "#EFEFEF";  // SUN violett 
    $color2 = "#EFEFFF";    
    $cnt = 0;
    $maxcnt = sizeof ($statarr);

    foreach( $statarr as $idx=>$valx)  ) { 
        $showit = 0;
        if ($cnt<$parx["numShowFuncs"]) $showit = 1;
        else { 
        if ($cnt==$parx["numShowFuncs"]) {
                $idx="...";
                $valx= "..."; 
                $showit = 1;
        }    
        } 
        if ($cnt > ($maxcnt-5) ) $showit = 1; 
        if ($showit) {
            if ($color == $color1)  $color = $color2;
            else $color = $color1;  
            echo "<tr bgcolor=\"".$color."\"><td>" .($cnt+1). "</td><td>" .$valx. "</td><td>" .$idx. "</td></tr>\n";
        }
        $cnt++;
    }
    echo "</table>\n";  

    echo "<br><br><B>Users</B><BR>\n";   

    if (sizeof($loguserarr)) {

        arsort($loguserarr);  

        foreach( $loguserarr as $idx=>$valx) {  
            $secondarr[$idx][0] = $valx;
            foreach( $loguser2arr[$idx] as $idx1=>$valx1) {  
                $secondarr[$idx][1] .= $idx1."=&gt;".$valx1." ";
            }
        } 
        reset($loguserarr);

        $tmp_header = array("user", "number of logins", "hosts");
        
        $opt = NULL;
        $opt = array("showid" => 1); 
        visufuncs::table_out( $tmp_header, $secondarr, $opt );
    }
} 

     
if ( sizeof($hostArr) ) {
  
    echo "<br>";
    echo "<B>Found hosts:</B><br>";   
    $outtab = NULL;
    foreach( $hostArr as $tmpip=>$tmpcnt) {
        if (!$tmpip) continue;
        $tmpname = this_gethostname( $tmpip );       // get from $hostNameArr
        $outtab[] = array($tmpip, $tmpname, $tmpcnt);
    }

    $thead= array("IP", "NAME", "HITS" );
    $topt=NULL;
    visufuncs::table_out( $thead, $outtab, $topt); 
 
}
echo "<hr>";
