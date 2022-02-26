<?php
/*
//  class:      PaXMLHTMLOuput
//  descr:      class for writing partisan xml to a file
//  version:    class version 0.1.beta
//
//  ...         rogo start
//  2003-01-13  rogo added statistics
 *  @version $Header: trunk/src/www/pionir/impexp/partisanxml/PaXMLHTMLOutput.inc 59 2018-11-21 09:04:09Z $
*/

require_once("f.progressBar.inc");

// viewmodes:
// 1 - minimum visualization
// 2 - middle visualization
// 3 - maximum visualization
// 4 - developer view
define ("PaXML_QUIET",    0);
define ("PaXML_TINYVIEW", 0.5);
define ("PaXML_MINVIEW",  1);
define ("PaXML_MIDVIEW",  2);
define ("PaXML_MAXVIEW",  3);
define ("PaXML_DEVVIEW",  4);

define ("PaXML_VIEWMODE_IMPORT", 1);
define ("PaXML_VIEWMODE_EXPORT", 2);


class PaXMLHTMLOutput
{
    var $viewMode;      // have a look at constants above
    var $pxmlMode;
    var $text;

    var $colors;

    var $lastStructure;
    var $lastID;
    
    var $parser;        // handle of xml-sax-parser

    var $statistics;    // time, files, objects, classes, htables?

	var $flushLib;

    // constructor.
    //
    // input:   number  viewmode        view mode (view constants)
    //
    // output:  -
    //
    function __construct($viewMode)
    {
    	$this->viewMode                     = $viewMode;
    	$this->pxmlMode                     = null;
        $this->text                         = "";
        
        $this->colors["warningText"]        = "#FFAA00";
        $this->colors["errorText"]          = "red";
        $this->colors["debugText"]          = "#665522";
        $this->colors["normalText"]         = "black";
        $this->colors["actionText"]         = "green";
        $this->colors["importText"]         = "green";
        $this->colors["noImportText"]       = "#990099";
        $this->colors["specialText"]        = "#FF00FF";
        $this->colors["dataText"]           = "darkblue";
        $this->colors["additionalDataText"] = "gray";
        $this->colors["exportText"]         = "green";
        $this->colors["noExportText"]       = "#990099";
        $this->colors["grayText"]           = "gray";
        
        $this->statistics                   = array();
        $this->flushLib						= NULL;
    }
    
    /**
     * show a simple info line
     * @param string $text
     * @param int $minlevele.g. PaXML_DEVVIEW
     */
    function infoline($text, $minlevel=0) {
    	if ($this->viewMode < $minlevel)
    		return;
    
    	echo '<span style="color:' . $this->colors["debugText"] . ';">Info: '.$text.'</span>'."<br>\n";
    	
    }

    // output of statement.
    //
    // input:   string  $text       statement text
    //
    // output:  -
    //
    function statement($text)
    {
        if ($this->viewMode == PaXML_QUIET)
            return;

    	if ($this->viewMode >= PaXML_DEVVIEW)
            echo "<br>\n<font color=" . $this->colors["debugText"] . ">SQL: $text</font>";
	    elseif ($this->viewMode >= PaXML_MAXVIEW)
	        echo "<!-- SQL: $text //-->\n";
    }



    // output a warning.
    //
    // input:   string  $text       public error message text
    //          string  $secret     private error message part (maybe function call)
    //
    // output:  -
    //
    function warning($text, $secret = null, $min_level=NULL)
    {
        if ($this->viewMode == PaXML_QUIET)
            return;
        
        if ($min_level===NULL) $min_level=PaXML_TINYVIEW;
       
        if ($this->viewMode >= $min_level)
	        echo "\n<br><font color=" . $this->colors["warningText"] . ">Warning! $text</font><br>";

	    if ($this->viewMode >= $min_level and $secret != null)
            echo "<!-- $secret //-->";

        if ($this->viewMode >= $min_level)
            echo "\n";
    }

    function backtrace_out(&$array) 
    {     
    
         
          $cnt=0;
          
          foreach( $array as $id=>$content) { 
              
              if (!$cnt) {
                  // remove first function-backtrace; it is the call stop() itself
                  $cnt++;
                  continue;
              }
              
                echo "<B>$id: </B> \n";
                $tmparr=array();
                $tmparr[0]=basename($content["file"]);   
                $tmparr[1]=$content["function"];
                $tmparr[2]=$content["class"];
                $tmparr[3]=$content["type"];
                $tmparr[4]=$content["args"];
                $tmparr[5]=basename($content["line"]);  
                echo "file: ".$tmparr[0]. " func: ".$tmparr[1]." line: ".$tmparr[5]." class: ".$tmparr[2]. " type: ".$tmparr[3]." args: ";
                print_r($tmparr[4]);
                echo "<br>\n"; 
          }
          
    }

    /**
     * get short backtrace-string
     * @param array $tmparr
     * @return string $result
     */
    function backtrace_short(&$array) {
    	
        $cnt=0;
    	$result=NULL;
    	foreach( $array as $id=>$content) {
    	    
    	    if (!$cnt) {
    	        // remove first function-backtrace; it is the call stop() itself
    	        $cnt++;
    	        continue;
    	    }
    	    
    	    
    	    $tmparr=array();
    		$tmparr[0]=basename($content["file"]);
    		$tmparr[1]=$content["function"];
    		$tmparr[2]=$content["class"];
    		$tmparr[5]=basename($content["line"]);
    		$result .="id:".$id." file:".$tmparr[0]. " func:".$tmparr[1]." line:".$tmparr[5];
    		$result .=';';
    	}
    	
    	return $result;
    }

    // output a error. note! this function kills current php script execution.
    //
    // input:   string  $text       public error message text
    //          string  $secret     private error message part (maybe function call)
    //
    // output:  -
    //
    function stop($text, $secret = null)
    {
    	global $error;
    	$error = & ErrorHandler::get();
    	
        //if ($this->viewMode == PaXML_QUIET)
        //    exit;

        echo "\n<br><font color=" . $this->colors["errorText"] . ">Error! $text <br>";
        $error->printAll();
        echo "<br>";

        if ($this->lastStructure or $this->lastID)
            echo "Last data: " . strtolower($this->lastStructure) . " (" . $this->lastID . ")";

        if ($this->parser)
            echo "; XML file line number: " . xml_get_current_line_number($this->parser);
        
        echo "</font>"; 
        
        echo "\n<br><font color=gray><B>Function-backtrace:</B><br>\n";
        $tmparr = debug_backtrace();
        $this->backtrace_out($tmparr);
        echo "</font>";
        
        // put error to error-log
        $trace_str_short = $this->backtrace_short($tmparr);
        $error->set( __CLASS__, 1, 'PaXML-Error: '.$text.'; ErrStack:'.$trace_str_short );
        $error->logError();
        
        if ($secret != null)
            echo "<!-- $secret //-->";

        echo "\n";
        exit;
    }
    
    
    
    // notify grabbing of table
    function grab($table)
    {
        if ($this->viewMode >= PaXML_DEVVIEW)
            echo "<!-- grabbed: $table //-->";
    }
    
    
    
    // return current viewmode. (view define part at top.)
    //
    // input:   -
    //
    // output:  number
    //
    function getViewMode()
    {
        return $this->viewMode;
    }
    
    
    
    // return a human readable, usefull text for view modes.
    //
    // input:   -
    //
    // output:  string
    //
    function getViewModeAsText()
    {
        switch($this->viewMode)
        {
            case PaXML_QUIET    : return "";
            case PaXML_TINYVIEW : return "Tiny View";
            case PaXML_MINVIEW  : return "Minimum View";
            case PaXML_MIDVIEW  : return "Medium View";
            case PaXML_MAXVIEW  : return "Maximum View";
            case PaXML_DEVVIEW  : return "Developer View";
        }
    }
    
    
    
    // begin time measurement. set an identifier for your time, so you may measure
    // several times.
    //
    // input:   string  $id         identifier
    //
    // output:  -
    //
    function startTimer($id)
    {
        $this->statistics["timer"][$id] = time();
    }



    // stop time measurement. use same identifier as you used for startTimer().
    // The time difference will be calculated directly.
    //
    // input:   string  $id         identifier
    //
    // output:  -
    //
    function stopTimer($id)
    {
        $this->statistics["timer"][$id] = time() - $this->statistics["timer"][$id];
    }


    // format a filesize for output. depending on filesize the unit will be bytes
    // kbytes or mbytes. Also the number of digits is customizable.
    //
    // input:   number  $size       filesize
    //          number  $digits     number of digits to display
    // output:  string
    //
    function displayFilesize($size, $digits = null)
    {
        if ($size == null)
            $size = 0;

        if ($size < 2) return "$size Byte";
        if ($size < 1024 * 10) return "$size Bytes";
        if ($size < 1024 * 1024 * 5)
        {
            if ($digits === null)
                $digits = 2;

            $factor = pow(10, $digits);

            if ($digits < 1)
                $result = (round($size / 1024)) . " KByte";
            else
                $result = (round(($size / 1024) * $factor) / $factor) . " KByte";

            return $result;
        }

        if ($digits === null)
            $digits = 1;

        $factor = pow(10, $digits);

        if ($digits < 1)
            $result = (round($size / (1024 * 1024))) . " MByte";
        else
            $result = (round(($size / (1024 * 1024)) * $factor) / $factor) . " MByte";

        return $result;
    }



    // format a time difference for output. depending on time the unit will be
    // seconds or minutes. Also the number of digits is customizable.
    //
    // input:   number  $time       a time difference
    //          number  $digits     number of digits to display
    // output:  string
    //
    function displayTime($time, $digits = null)
    {
        if ($time == null)
            $time = 0;

        if ($time == 0) return $time . " seconds";
        if ($time == 1) return $time . " second";
        if ($time > 60)
        {
            if ($digits === null)
                $digits = 2;

            $factor = pow(10, $digits);
            return (round(($time / 60) * $factor) / $factor) . " minute(s)";
        }
        else
            return $time . " seconds";

    }
	
	function flushText() {
		// flush HTML text
		ob_end_flush ();
		while (@ob_end_flush()); // send all buffered output
	}
	
	function setProgressLib(&$flushLib) {
		$this->flushLib = &$flushLib;
	}
	
	/**
	 * show progress bar
	 * - $this->flushLib must be set in setProgressLib()
	 * @param int $ininum
	 * @param array $options
	 * 	'mode': 'char', ['bar']
	 * 	
	 */
	function startProgress( $ininum, $options=array() )  { 

    	if ($this->flushLib===NULL) return;
    	
    	$prgopt=array();
    	$prgopt['maxnum'] = $ininum;
		$prgopt['mode']  = 'char';
		if ($options['mode']!=NULL) {
			$prgopt['mode']  = $options['mode'];
		} 
		if ($options['objname']!=NULL) $prgopt['objname']  = $options['objname'];
		
		$this->flushLib->shoPgroBar( $prgopt );
    }
    
	function showProgress($cnt) {
		if ($this->flushLib===NULL) return;
		
    	$this->flushLib->alivePoint($cnt);
    	
    }
    function stopProgress($cnt) {	
		if ($this->flushLib===NULL) return;
		
    	$this->flushLib->alivePoint($cnt, 1);
    	echo "<br />\n";
  
    }
    
    

}


// end.
?>
