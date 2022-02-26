<?

//
//  class:      PaXMLHTMLOutputImport
//  descr:      html output class at import
//  version:    class version 0.2
//


require_once("PaXMLHTMLOutput.inc");


// output class for export, inherited from PaXMLHTMLOutput
class PaXMLHTMLOutputImport extends PaXMLHTMLOutput
{


    var $indentString;      // string for indention (must be a repeatable string)
    var $indent;            // deep of indention

    var $lastObject;
    var $lastObjectMode;
    
    var $plusCounter;       // counter for '+', which are displayed. after 20 '+'
                            // a space will be inserted, for automatic word wrap
                            // of browsers.



    // contructor
    function __construct($viewmode)
    {
    	parent::__construct($viewmode);

        $this->viewMode     = $viewmode;
        $this->indentString = " &nbsp; &nbsp;";
        $this->indent       = 0;
    }

    function start($version, $rversion)
    {
        if ($this->viewMode <= PaXML_QUIET)
            return;

        echo "<br>\n" . str_repeat($this->indentString, $this->indent++)
            . "<font color=" . $this->colors["actionText"]
            . ">start partisan xml import ...</font>"
            . "<br>\n" . str_repeat($this->indentString, $this->indent)
            . "<font color=" . $this->colors["normalText"]
            . ">view mode: " . $this->getViewModeAsText() . " (".$this->viewMode.")</font>"
            . "<br>\n" . str_repeat($this->indentString, $this->indent)
            . "<font color=" . $this->colors["normalText"]
            . ">importer version: $rversion</font>"
            . "<br>\n" . str_repeat($this->indentString, $this->indent--)
            . "<font color=" . $this->colors["normalText"]
            . ">xml version: $version</font>";
		$this-> flushText();
    }
	
	function info(
		$key, 
		$val,
		$viewModeMin=PaXML_MINVIEW	// minimum viewMode
		)
    {
		// FUNCTION: show info
        if ($this->viewMode < $viewModeMin)
            return;

        echo "<br>\n" . str_repeat($this->indentString, $this->indent)
            . "<font color=" . $this->colors["actionText"]
            . ">".$key."</font><font color="
            . $this->colors["normalText"] . "> ".$val." </font>";
		ob_flush();
    }
    
    /**
     * start XML file parsing
     * @param string $xmlFilename
     */
    public function XML_file_start($xmlFilename, $READ_BLOCK_SIZE) {
    	
	    if ( $this->viewMode <= PaXML_QUIET ) {
	    	
	    	// get size of file + number of blocks
	    	$file_size_x = filesize($xmlFilename);
	    	if (!$READ_BLOCK_SIZE) return;
	    	
	    	$max_cnt = intval($file_size_x / $READ_BLOCK_SIZE);
	    	
	    	$prgopt = array();
	    	$prgopt['mode']='bar';
	    	$prgopt['periodSec']= 60;
	    	$prgopt['objname'] = 'blocks';
	    
	    	$this->startProgress( $max_cnt, $prgopt );
	    	
	    }
    }
    
    /**
     * progress of file reading
     * @param int  $block_cnt
     */
    public function XML_file_progress($block_cnt) {
    	if ( $this->viewMode <= PaXML_QUIET ) {
    		$this->showProgress($block_cnt);
    	}
    }
    
    /**
     * progress of file reading
     * @param int  $block_cnt
     */
    public function XML_file_end($block_cnt) {
    	if ( $this->viewMode <= PaXML_QUIET ) {
    		$this->stopProgress($block_cnt);
    	}
    }

    function detectFiletype($filename)
    {
        if ($this->viewMode <= PaXML_TINYVIEW)
            return;

        echo "<br>\n" . str_repeat($this->indentString, $this->indent)
            . "<font color=" . $this->colors["actionText"]
            . ">detecting filetype of</font><font color="
            . $this->colors["normalText"] . "> $filename ... </font>";
		ob_flush();
    }
    
    function detectedFiletype($filetype)
    {
        if ($this->viewMode <= PaXML_TINYVIEW)
            return;

        echo "<font color=" . $this->colors["normalText"]
            . ">it's a $filetype file.</font>";
		ob_flush();
    }
    
    function createWorkPath()
    {
        if ($this->viewMode <= PaXML_TINYVIEW)
            return;

        echo "<br>\n" . str_repeat($this->indentString, $this->indent)
            . "<font color=" . $this->colors["actionText"]
            . ">creating workpath ... </font>";
		ob_flush();
	}

    function createdWorkPath($path)
    {
        if ($this->viewMode <= PaXML_TINYVIEW)
            return;

        if ($this->viewMode >= PaXML_DEVVIEW)
            echo $path . " ... ";

        echo "<font color=" . $this->colors["normalText"] . ">done</font>";
		ob_flush();
	}

    function untarFile($filename)
    {
        if ($this->viewMode <= PaXML_QUIET)
            return;

        echo "<br>\n" . str_repeat($this->indentString, $this->indent)
            . "<font color=" . $this->colors["actionText"]
            . ">tar file detected. untar file ... </font>";
        echo (($this->viewMode >= PaXML_DEVVIEW) ? $filename . " ... " : "");
		ob_flush();
	}

    function untaredFile()
    {
        if ($this->viewMode <= PaXML_QUIET)
            return;

        echo "<font color=" . $this->colors["normalText"] . ">done.</font>";
		ob_flush();
	}
    
    function uncompressFile($filename)
    {
        if ($this->viewMode <= PaXML_TINYVIEW)
            return;

        echo "<br>\n" . str_repeat($this->indentString, $this->indent)
            . "<font color=" . $this->colors["actionText"]
            . ">gzip file detected. uncompress file ... </font>";
        echo (($this->viewMode >= PaXML_DEVVIEW) ? $filename . " ... " : "");
		ob_flush();
	}
    
    function uncompressedFile()
    {
        if ($this->viewMode <= PaXML_TINYVIEW)
            return;

        echo "<font color=" . $this->colors["normalText"] . ">done.</font>";
		ob_flush();
	}
    
    function updateCache()
    {
        if ($this->viewMode <= PaXML_TINYVIEW)
            return;

        echo "<br>\n" . str_repeat($this->indentString, $this->indent)
            . "<font color=" . $this->colors["actionText"]
            . ">updating internal cache. please wait ... </font>";
		ob_flush();	
    }

    function updatedCache()
    {
        if ($this->viewMode <= PaXML_TINYVIEW)
            return;

        echo "<font color=" . $this->colors["actionText"] . ">done.</font>";
		ob_flush();
    }

    function connectToDatabase()
    {
        if ($this->viewMode <= PaXML_TINYVIEW)
            return;

        echo "<br>\n" . str_repeat($this->indentString, $this->indent)
            . "<font color=" . $this->colors["actionText"]
            . ">connecting to database ...</font>";
		ob_flush();
    }

    function connectedToDatabase()
    {
        if ($this->viewMode <= PaXML_TINYVIEW)
            return;

        echo "<font color=" . $this->colors["normalText"] . ">done.</font>";
		ob_flush();
    }
    
    
    function initXMLParser()
    {
        if ($this->viewMode <= PaXML_TINYVIEW)
            return;

        echo "<br>\n" . str_repeat($this->indentString, $this->indent)
            . "<font color=" . $this->colors["actionText"]
            . ">init xml parser ... </font>";
		ob_flush();	
    }

    function parse()
    {
        if ($this->viewMode <= PaXML_TINYVIEW)
            return;

        echo "<br>\n" . str_repeat($this->indentString, $this->indent)
            . "<font color=" . $this->colors["actionText"]
            . ">parse xml file ... </font>";
		ob_flush();
    }

    function rollback()
    {
        if ($this->viewMode <= PaXML_QUIET)
            return;

        echo "<br><br><br>\n" . str_repeat($this->indentString, $this->indent)
            . "<font color=" . $this->colors["actionText"]
            . ">rollback ... </font>";
		ob_flush();
    }

    function openPARTISANXML($version, $db, $option)
    {
        if ($this->viewMode <= PaXML_QUIET)
            return;

        echo "<br>\n" . str_repeat($this->indentString, $this->indent++)
            . "<font color=" . $this->colors["dataText"]
            . ">paxml</font>"
            . "<br>\n" . str_repeat($this->indentString, $this->indent)
            . "<font color=" . $this->colors["normalText"]
            . ">paxml version detected: $version</font>"
            . "<br>\n" . str_repeat($this->indentString, $this->indent)
            . "<font color=" . $this->colors["normalText"]
            . ">source database id: ".$db;
		if ($option["dbversion"]!="") echo " db-version: ".$option["dbversion"];
		echo "</font>";
		
		ob_flush();
    }

    function closePartisanXML()
    {
        if ($this->viewMode == PaXML_QUIET)
            return;
		
		--$this->indent;
		if ($this->indent<=0)  $this->indent=0;
        echo "<br>\n<br>\n" . str_repeat($this->indentString, $this->indent)
            . "<font color=" . $this->colors["actionText"]
            . ">import successfully finished ... </font>";
		ob_flush();
    }

    function startObject($table, $wiid, $roid, $file = null)
    {
        $this->lastStructure = "OBJECT." . $table;
        $this->lastID = "wiid:$wiid roid:$roid";
        
        $this->statistics["objects"][$table]["grabbed"]++;
        
        if ($this->viewMode == PaXML_QUIET)
            return;

        
        $this->lastObjectMode = false;
        
        /*
        if ($this->viewMode == PaXML_TINYVIEW and $table != "PROJ")
        {
            echo "+" . ((++$this->plusCounter % 20 == 0) ? " " : "");
            if ($this->plusCounter % 20 == 0)
				ob_flush();
            $this->incIndent();
            return true;
        }
        */
        
        /*
        if ($this->viewMode == PaXML_MINVIEW and $this->lastObject == $table and $table != "PROJ")
        {
            echo "+" . ((++$this->plusCounter % 20 == 0) ? " " : "");
            if ($this->plusCounter % 20 == 0)
				ob_flush();
			$this->lastObjectMode = true;
            $this->incIndent();
            return true;
        }
        */
        if ( ($this->viewMode == PaXML_MINVIEW or $this->viewMode == PaXML_TINYVIEW ) and $table != "PROJ") {
        	$this->lastObjectMode = true;
        	$this->incIndent();
        	return true;
        }

        $this->plusCounter = 0;
        $this->lastObject  = $table;
        $this->incIndent();
		
		ob_flush(); 
		
		$tmpWiidRoid = 0;
		if ($this->viewMode >= PaXML_MIDVIEW) $tmpWiidRoid = 1;
		
        if ($this->viewMode >= PaXML_TINYVIEW) {
            echo "<br>\n" . str_repeat($this->indentString, $this->indent - 1)
                . "<font color=" . $this->colors["dataText"] . ">object."
                . strtolower($table) . "</font> ";
			if ($table == "PROJ") $tmpWiidRoid = 1;
		}
        if ($tmpWiidRoid)
            echo "<font color=" . $this->colors["additionalDataText"]
                . ">[" .  $wiid . "&roid=$roid]</font>";

        if ($this->viewMode >= PaXML_MIDVIEW and $file != null)
            echo "<br>\n" . str_repeat($this->indentString, $this->indent)
                . "<font color=" . $this->colors["additionalDataText"]
                . ">attached file: $file</font>\n";  
        
		return true;
    }

    function endObject($displayed)
    {
        if ($this->viewMode == PaXML_QUIET)
            return;

        if ($displayed)
            $this->decIndent();
    }

    function noSearchForObject($table, $wiid, $roid, $file = null)
    {
        $this->lastStructure = "OBJECT." . $table;
        $this->lastID = "wiid:$wiid roid:$roid";

        if ($this->viewMode == PaXML_QUIET)
            return;

        if ($this->viewMode == PaXML_TINYVIEW and $table != "PROJ")
        {
            // $this->incIndent();
            // echo "+" . ((++$this->plusCounter % 20 == 0) ? " " : "");
            return false;
        }

        if ($this->viewMode >= PaXML_DEVVIEW)
        {
			ob_flush();
            echo "<br>\n" . str_repeat($this->indentString, $this->indent - 1)
                . "<font color=" . $this->colors["dataText"] . ">object."
                . strtolower($table) . "</font> " . "<font color="
                . $this->colors["additionalDataText"] . ">[" .  $wiid
                . "&roid=$roid]</font><br>\n"
                . str_repeat($this->indentString, $this->indent - 1)
                . "<font color=" . $this->colors["normalText"]
                . ">attached file: $file</font>"
                . "<font color=" . $this->colors["noImportText"]
                . "> ... already existing by parent.</font>";
            $this->incIndent();
            return true;
        }
    }

    function foundObject($found, $id = null, $table)
    {
        if (!$found)
            $this->statistics["objects"][$table]["imported"]++;
        else
            $this->statistics["objects"][$table]["found"]++;
        
        $obj_char = $found ? 'e' : '+'; // e: exists, +: import
       	if ( ($this->viewMode == PaXML_MINVIEW or $this->viewMode == PaXML_TINYVIEW) and $table != "PROJ") {
        	echo $obj_char . ((++$this->plusCounter % 50 == 0) ? " " : "");
        	if ($this->plusCounter % 50 == 0)
        		ob_flush();
        	$this->lastObjectMode = true;
        	return ;
        }

        if (($this->viewMode >= PaXML_MIDVIEW)
            or ($this->viewMode == PaXML_MINVIEW and !$this->lastObjectMode))
            if ($found)
                echo  "<font color=" . $this->colors["normalText"]
                    . ">.id = $id </font><font color="
                    . $this->colors["noImportText"] . "> ... found. no import."
                    . "</font>";
            else
                echo "<font color=" . $this->colors["importText"]
                    . "> ...  not found. import." . "</font>";
    }

    function circularReferencingObject()
    {
        if ($this->viewMode >= PaXML_MINVIEW)
            echo "<font color=" . $this->colors["specialText"]
                . "> ... circular reference found!</font><br>\n";
    }
    
    function isp_do($wiid, $roid) {
    	echo "\n<br> <font color=" . $this->colors["normalText"]
    	. "> ... Import filter: start import at PROJ, WIID:" .$wiid . ", ROID:" .$roid. "</font>";
    }
    
    function isp_found($wiid, $roid) {
    	echo "\n<br><font color=" . $this->colors["normalText"]
    	  . "> ... START of import found PROJ,WIID:" . $wiid . ", ROID:" .$roid. " d:" . date('H:i:s')."</font>";
    }

    function alreadyLinked()
    {
        if ($this->viewMode >= PaXML_MIDVIEW)
            echo "<font color=" . $this->colors["noImportText"]
                . "> ... already linked.</font>";
	}

    function resolvingCircularReference() {  
        if ($this->viewMode >= PaXML_MIDVIEW)
            echo "<font color=" . $this->colors["noImportText"]
                . "> ... circular reference solved.</font>";
    }




    function openClass($name, $type)
    {
        $this->lastStructure = "CLASS for $type";
        $this->lastID = "name:$name";

        if ($this->viewMode >= PaXML_MIDVIEW)
        {
			ob_flush();
            echo "<br>\n" . str_repeat($this->indentString, $this->indent)
                . "<font color=" . $this->colors["dataText"]
                . ">class</font> <font color=" . $this->colors["normalText"]
                . ">" . strtolower($type) . ".$name</font>";
			$this->incIndent();
        }
    }

    function classFound($id)
    {

        $this->statistics["classes"]["found and validated"]++;
        if ($this->viewMode >= PaXML_MIDVIEW)
			echo "<font color=" . $this->colors["normalText"]
                . ">.id = $id </font><font color="
                . $this->colors["noImportText"] . "> ... found. no import.</font>";
    }

    function classNotFound()
    {

        $this->statistics["classes"]["imported"]++;
        if ($this->viewMode >= PaXML_MIDVIEW)
 			echo "<font color=" . $this->colors["importText"]
                . "> ... not found. import.</font>";
    }
    
    
    function validateClass()
    {
        if ($this->viewMode >= PaXML_MIDVIEW)
            echo "<br>\n" . str_repeat($this->indentString, $this->indent)
                . "<font color=" . $this->colors["normalText"]
                . ">validating class ... </font>";
    }

    function validatedClass()
    {
        if ($this->viewMode >= PaXML_MIDVIEW)
            echo "<font color=" . $this->colors["normalText"] . "> ok.</font>"; 
	}
    
    
    function endClass()
    {
        if ($this->viewMode >= PaXML_MIDVIEW)
            $this->decIndent();
    }
    
    

    
    function startAttribute($name)
    {
        if ($this->viewMode >= PaXML_MAXVIEW)
        {
            echo "<br>\n" . str_repeat($this->indentString, $this->indent)
                . "<font color=" . $this->colors["additionalDataText"]
                . ">attribute:</font> <font color="
                . $this->colors["additionalDataText"] . ">$name</font>";
            $this->incIndent();
        }
    }

    function endAttribute()
    {
        if ($this->viewMode >= PaXML_MAXVIEW)
            $this->decIndent();
    }
    

    function startOption($type, $name)
    {
       $this->lastStructure = "OPTION." . $type;
       $this->lastID = "name:$name";
       ob_flush();
	   echo "<!-- option.$type.$name //-->";
    }

    function endOption()
    {
    }
    
    function foundOption($type, $name, $found)
    {
        if ($this->viewMode >= PaXML_MAXVIEW and $found)
            echo "<br>\n" . str_repeat($this->indentString, $this->indent)
                . "<font color=" . $this->colors["dataText"]
                . ">option</font> <font color=" . $this->colors["normalText"]
                . ">" . strtolower($type) . ".$name</font><font color="
                . $this->colors["actionText"] . "> ... found. no import.</font>";

        if ($this->viewMode >= PaXML_MIDVIEW and !$found)
            echo "<br>\n" . str_repeat($this->indentString, $this->indent)
                . "<font color=" . $this->colors["dataText"]
                . ">option</font> <font color=" . $this->colors["normalText"]
                . ">" . strtolower($type) . ".$name</font><font color="
                . $this->colors["actionText"] . "> ... not found. import.</font>";

    }
    
    
    function startElement()
    {
        if ($this->viewMode >= PaXML_MAXVIEW)
        {
			ob_flush();
            echo "<br>\n" . str_repeat($this->indentString, $this->indent)
                . "<font color=" . $this->colors["normalText"] . ">element</font>";
            $this->incIndent();
        }
    }

    function endElement()
    {
        if ($this->viewMode >= PaXML_MAXVIEW)
            $this->decIndent();
    }

    function startElements($type)
    {
        if ($this->viewMode >= PaXML_MIDVIEW)
        {
			ob_flush();
            echo "<br>\n" . str_repeat($this->indentString, $this->indent)
                . "<font color=" . $this->colors["dataText"]
                . ">list of elements</font>";
            $this->incIndent();
        }

        if ($this->viewMode >= PaXML_MAXVIEW)
            echo " <font color=" . $this->colors["additionalDataText"] . ">"
                . strtolower($type) . "</font>";

    }

    function endElements()
    {
        if ($this->viewMode >= PaXML_MIDVIEW)
            $this->decIndent();
    }


    function startParameter($name, $value)
    {
        if ($this->viewMode >= PaXML_MAXVIEW)
            echo "<br>\n" . str_repeat($this->indentString, $this->indent)
                . "<font color=" . $this->colors["additionalDataText"]
                . ">parameter: $name = '$value'</font>";
    }

    function endParemter()
    {
    }


    function saveAttachedFile($table, $bo_id, $minInfoLevel=PaXML_MIDVIEW) {
		$this->info("save attached file:", "table:".$table. " ID:".$bo_id, $minInfoLevel);
    }




    function startTag($name)
    {
    //    if ($this->viewMode >= PaXML_DEVVIEW)
    //        echo "<font color=#DDCC88>&lt;$name ...&gt;</font><br>\n";
    }

    function endTag($name)
    {
    //    if ($this->viewMode >= PaXML_DEVVIEW)
    //        echo "<font color=#DDCC88>&lt;/$name ...&gt;</font><br>\n";
    }



    function alignToProject()
    {
        if ($this->viewMode >= PaXML_MINVIEW)
            echo "<font color=" . $this->colors["actionText"]
            . "> ... linked to root project.</font>";
    }

    function finish($importpath = null)
    {
		function cmp ($a, $b)
        {
            if ($a == $b) return 0;
            return ($a < $b) ? -1 : 1;
        }
		
        //if ($this->viewMode == PaXML_QUIET)
        //    return;

		ob_flush();
        echo "<br>\n" . str_repeat($this->indentString, $this->indent)
            . "<font color=" . $this->colors["actionText"]
            . ">import completely finished!</font><br><br><hr width=300 "
            . "noshade size=1 align=left noshade><font size=2><br><b>statistics:</b><br>";
                
        echo "<table border=0 cellspacing=0 cellpadding=0>";
        if ($this->statistics["timer"])
            foreach($this->statistics["timer"] as $id => $time)
                echo "\n<tr><td nowrap><font size=2> &nbsp; &nbsp; used time for $id: </font></td><td><font size=2> " . $this->displayTime($time) . "</font></td></tr>";
        else
            echo "\n<tr><td nowrap><font size=2> &nbsp; &nbsp; used time </font></td><td><font size=2> not recorded</font></td></tr>";
        if ($this->statistics["files"])
            echo "\n<tr><td nowrap><font size=2> &nbsp; &nbsp; new files: </font></td><td><font size=2> " . $this->statistics["files"] . "</font></td></tr>";
        if ($this->statistics["classes"])
        {
            $tmpArray = $this->statistics["classes"];
            foreach($tmpArray as $name => $item)
                echo "\n<tr><td nowrap><font size=2> &nbsp; &nbsp; " . strtolower($name) . " classes: </font></td><td><font size=2> " . $item . "</font></td></tr>";
        }

        if ($this->statistics["objects"])
        {
            $tmpArray = $this->statistics["objects"];
            uksort($tmpArray, "cmp");
            foreach($tmpArray as $name => $item)
                echo "\n<tr><td nowrap><font size=2> &nbsp; &nbsp; exported object." . strtolower($name) . ": </font></td><td><font size=2> " . ((int) $item["imported"]) . " / " .$item["grabbed"] . " &nbsp; <font color=gray>imported / found in file</font></font></td></tr>";
        }
        echo "</table>";


            echo "</font><br><hr width=300 noshade size=1 align=left noshade><font "
                . "face=Arial,Helvetica "
                . "size=2><br>[ <a href='import.clean.php?path=$importpath'>go "
                . "back</a> <font color=gray>and clean temporary directory at "
                . "server</font> ]<br><br></font>";

    }
    
    
    function incIndent()
    {
        return ++$this->indent;
    }

    function decIndent()
    {
        if ($this->indent == 0)
            $this->indent = 1;

        --$this->indent;
        return $this->indent;
    }

}

?>
