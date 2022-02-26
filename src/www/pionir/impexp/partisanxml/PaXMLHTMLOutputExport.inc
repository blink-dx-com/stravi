<?

// 2003-01-09   rogo    add tinyview mode

require_once("PaXMLHTMLOutput.inc");


// output class for export. I'm too lazy to comment any function now, sorry.
class PaXMLHTMLOutputExport extends PaXMLHTMLOutput
{
    var $indent;
    var $indentString;
    
    var $cond;

    var $lastObject;
    var $loMode;
    var $plusCounter;
    
    var $statistics;        // time, files, objects, classes, htables?


    function __construct($mode)
    {
    	parent::__construct($mode);
        $this->indentString = " &nbsp; &nbsp; ";
        $this->indent       = 0;
    }



    /**
     * 
     * @param string $version
     * @param object $db
     * @param string $rversion
     * @param array $infoarr  // array[] = array(key,$val)
     */
    function startXML($version, $db, $rversion, $infoarr=NULL)
    {
        $this->startTimer("export");

        if ($this->viewMode == PaXML_QUIET)
            return;
            
        echo str_repeat($this->indentString, $this->indent++) . "<font color=" . $this->colors["actionText"] . ">init paxml export ...</font><br>\n";
        
        if ($this->viewMode <= PaXML_TINYVIEW) {
            ob_flush();
            $this->indent--;
            return;
        }
        
        echo 
            str_repeat($this->indentString, $this->indent) . "<font color=" . $this->colors["normalText"] . ">view mode: " . $this->getViewModeAsText() . "</font><br>\n"
            . str_repeat($this->indentString, $this->indent) . "<font color=" . $this->colors["normalText"] . ">exporter version: $rversion</font><br>\n"
            . str_repeat($this->indentString, $this->indent) . "<font color=" . $this->colors["normalText"] . ">xml version: $version</font><br>\n"
            . str_repeat($this->indentString, $this->indent) . "<font color=" . $this->colors["normalText"] . ">current db identifier: $db</font><br>\n";
			
            
        if ( sizeof($infoarr) ) {
			foreach( $infoarr as $dummy=>$tmparr) {
				echo str_repeat($this->indentString, $this->indent) . "<font color=" . $this->colors["normalText"] . ">".$tmparr[0].": ".$tmparr[1]."</font><br>\n";
			
			}
			
		}
		
		$this->indent--;
		echo str_repeat($this->indentString, $this->indent++) . "<font color=" . $this->colors["exportText"] . ">start grabbing ...</font><br>\n";
			
		ob_flush();
    }
    
    function endXML()
    {
        $this->stopTimer("export");

        if ($this->viewMode == PaXML_QUIET)
            return;

        if ($this->indent < 1) $this->indent = 1;       // security instruction
            
        echo "\n<br>" . str_repeat($this->indentString, --$this->indent)
            . "<font color=" . $this->colors["exportText"] . ">grabbing finished ...</font>";
        ob_flush();
    }

    function startObject($table, $filter, $wiid, $roid)
    {
        $this->loMode = false;
        
        $this->lastStructure = "object." . $table;
        $this->lastID = "wiid:$wiid roid:$roid";
        
        if ($this->viewMode == PaXML_TINYVIEW and $table != "PROJ")
        {
            echo "+";
            if (++$this->plusCounter % 20 == 0)
            {
                $this->plusCounter = 0;
                echo " ";
            }

            $this->loMode = true;
            $this->indent++;

            ob_flush();
            return;
        }
        
        if ($this->viewMode == PaXML_MINVIEW and $table == $this->lastObject and $table != "PROJ" and $this->viewMode != PaXML_TINYVIEW)
        {
            echo "+";
            if (++$this->plusCounter % 20 == 0)
            {
                $this->plusCounter = 0;
                echo " ";
            }

            $this->loMode = true;
            $this->indent++;

            ob_flush();
            return;
        }
        
    	if ($this->viewMode >= PaXML_MINVIEW or $this->viewMode == PaXML_TINYVIEW)
        {
            $keys = array_keys($filter);
            $key  = $keys[0];
            $val  = $filter[$keys[0]];

            echo "\n<br>" . str_repeat($this->indentString, $this->indent++) . "<font color=" . $this->colors["dataText"] . ">object:</font><font color=" . $this->colors["normalText"] . "> " . strtolower($table) . "." . strtolower($key) . " = $val</font>";

            if ($this->viewMode >= PaXML_MIDVIEW)
                echo " <font color=" . $this->colors["additionalDataText"] . ">[ wiid: $wiid ][ roid: $roid ]</font>";
            else
                echo "<!-- gid: " . $wiid . "&roid=" . $roid . " //-->\n";

            ob_flush();
        }

        $this->plusCounter = 0;
        $this->lastObject  = $table;
    }

    function alreadyGrabbedObject()
    {
        if ($this->viewMode >= PaXML_TINYVIEW)
            $this->indent--;

        if ($this->indent < 0)
            $this->indent = 0;

        if ($this->viewMode >= PaXML_MINVIEW and !$this->loMode)
            echo "<font color=" . $this->colors["noExportText"] . "> ... already grabbed.</font> ";
    }
    
    function exportObject($table)
    {
        $this->statistics["objects"][$table]++;
        if ($this->viewMode >= PaXML_MINVIEW and !$this->loMode)
            echo "<font color=" . $this->colors["exportText"] . "> ... export.</font> ";
    }
    
    function exportObjectDenied($table)
    {
        $this->statistics["noexpobj"][$table]++;
        if ($this->statistics["noexpobj"][$table]<=1 and $this->viewMode >= PaXML_TINYVIEW) // write it only once !
            $this->warning("Data of table '$table' not allowed for export (due to EXIM-Flag in CCT_COLUMN)."
                . "<!-- grabObject() //-->", '', PaXML_MINVIEW);
    }
    function exportObjectDeniedSingle($table, $objid, $info)
    {
        $this->statistics["noexpobj"][$table]++;
        if (  ( ($this->statistics["noexpobj"][$table]<=1) and $this->viewMode >= PaXML_TINYVIEW) or $this->viewMode >= PaXML_MIDVIEW  )  
            $this->warning("Data of object '$table:$objid' not allowed for export (".$info.")."
                . "<!-- grabObject() //-->", '', PaXML_MINVIEW);
             
    }
    
    function endObject()
    {
        if ($this->viewMode >= PaXML_TINYVIEW /* and !$this->loMode */)
            $this->indent--;

        if ($this->indent < 0)
            $this->indent = 0;
    }
    
    function fileCopied($filename, $aFilename)
    {
        $this->statistics["files"]++;
        if ($this->viewMode >= PaXML_MIDVIEW)
            echo "\n<br>" . str_repeat($this->indentString, $this->indent) . "<font color=" . $this->colors["normalText"] . ">attached document: '". htmlspecialchars($filename) . "'</font> ";
        if ($this->viewMode >= PaXML_MIDVIEW and $this->viewMode < PaXML_DEVVIEW)
            echo "<!-- filename in tar file: ". htmlspecialchars($aFilename) . " //-->";
        if ($this->viewMode >= PaXML_DEVVIEW)
            echo "<font color=" . $this->colors["additionalDataText"] . ">(" . htmlspecialchars($aFilename) . ")</font> ";
        if ($this->viewMode >= PaXML_MIDVIEW)
            echo "<font color=" . $this->colors["exportText"] . "> ... export.</font> ";
    }
    
    function noFile()
    {
        if ($this->viewMode >= PaXML_MINVIEW /* and !$this->loMode */)
            echo "<font color=" . $this->colors["warningText"] . "> ... no file.</font> ";
    }

    function startClass($id)
    {
        $this->lastStructure = "class";
        $this->lastID = "id:$id";

        $this->cond["class"] = $this->viewMode >= PaXML_MIDVIEW;
        if ($this->cond["class"])
            echo "\n<br>" . str_repeat($this->indentString, $this->indent++)
                . "<font color=" . $this->colors["dataText"] . ">class: </font><font color=" . $this->colors["normalText"] . ">name = '$id'</font>";
        ob_flush();
    }

    function alreadyGrabbedClass()
    {
        if ($this->cond["class"])
        {
            $this->indent--;
            echo "<font color=" . $this->colors["noExportText"] . "> ... already grabbed.</font>";
        }

        if ($this->indent < 0)
            $this->indent = 0;
    }
	
	function exportSattach() {
		$this->statistics["attach"]++;
	}
    
    function exportClass()
    {
        $this->statistics["classes"]++;
        if ($this->cond["class"])
            echo "<font color=" . $this->colors["exportText"] . "> ... export.</font>";
    }
    
    function endCLass()
    {
        if ($this->cond["class"])
            $this->indent--;
            
        if ($this->indent < 0)
            $this->indent = 0;
    }
    
    function startAttribute($name)
    {
        if ($this->viewMode >= PaXML_MAXVIEW)
            echo "\n<br>" . str_repeat($this->indentString, $this->indent++) . "<font color=" . $this->colors["additionalDataText"] . ">attribute: " . htmlspecialchars($name) . "</font>";
        ob_flush();
    }
    
    function endAttribute()
    {
        if ($this->viewMode >= PaXML_MAXVIEW)
            $this->indent--;

        if ($this->indent < 0)
            $this->indent = 0;
    }
    
    function exportParameter($name, $value, $extra='')
    {
        if ($this->viewMode >= PaXML_MAXVIEW) {
            echo "\n<br>" . str_repeat($this->indentString, $this->indent). "<font color=" . $this->colors["additionalDataText"] . ">";
            if ($extra!='') {
                echo "Extra: ".$extra.': ';
            }
            echo "parameter: ". htmlspecialchars($name) . " = ". htmlspecialchars($value) . "</font>";
        }
    }

    function startOption($type, $name)
    {
        $this->lastStructure = "option." . $type;
        $this->lastID = "name:$name";
        
        $this->cond["option"] = $this->viewMode >= PaXML_MIDVIEW;
        if ($this->cond["option"])
            echo "\n<br>" . str_repeat($this->indentString, $this->indent++) . "<font color=" . $this->colors["dataText"] . ">option:</font><font color=" . $this->colors["normalText"] . "> "
                . strtolower($type) . ".name = ". htmlspecialchars($name) . "</font><font color=" . $this->colors["exportText"] . "> ... export.</font>";
        ob_flush();
    }

    function endOption()
    {
        if ($this->cond["option"])
            $this->indent--;
            
        if ($this->indent < 0)
            $this->indent = 0;
    }

    function startRelationships($type, $count)
    {
        $this->cond["relationships"] = $this->viewMode >= PaXML_MINVIEW;
        if ($this->cond["relationships"])
            echo "\n<br>" . str_repeat($this->indentString, $this->indent++)
                . "<font color=" . $this->colors["dataText"] . ">list data:</font><font color=" . $this->colors["normalText"] . "> "
                . strtolower($type) . " ($count elements)</font>";
        ob_flush();
    }
    
    function endRelationships()
    {
        if ($this->cond["relationships"])
            $this->indent--;

        if ($this->indent < 0)
            $this->indent = 0;
    }

    function startRelationship()
    {
        $this->cond["relationship"] = $this->viewMode >= PaXML_MAXVIEW;
        if ($this->cond["relationship"])
            echo "\n<br>" . str_repeat($this->indentString, $this->indent++) . "<font color=" . $this->colors["normalText"] . ">element</font>";
        ob_flush();
    }

    function endRelationship()
    {
        if ($this->cond["relationship"])
            $this->indent--;

        if ($this->indent < 0)
            $this->indent = 0;
    }
    

    function workingPath($path)
    {
        if ($this->viewMode >= PaXML_DEVVIEW)
            echo "\n<br><font color=" . $this->colors["debugText"] . ">current working path: " . $path . "</font>";
    }
    
    
    function compress($xmlFilename)
    {
        if ($this->viewMode >= PaXML_MINVIEW)
            echo "\n<br>" . "<font color=" . $this->colors["exportText"] . ">compressing  ... </font>";
    }


    function tar()
    {
        if ($this->viewMode >= PaXML_MINVIEW )
            echo "\n<br>" . "<font color=" . $this->colors["exportText"] . ">making package ... </font>";
    }
    

    function showTarLink($file)
    {
        echo "\n<br>" . "<a href=$file>tar file</a>";
    }
    
    function finish($file, $xml, $zip, $tar, $attached, $exportpath = null)
    {
        function cmp ($a, $b)
        {
            if ($a == $b) return 0;
            return ($a < $b) ? -1 : 1;
        }

        if ($this->viewMode == PaXML_QUIET)
            return;

        // echo "\n<br>";
        echo "<font color=" . $this->colors["exportText"] . ">export finished ...</font><br>";
        
        echo "<hr width=300 noshade size=1 align=left noshade><br>\n";
        
        // EXPORT LINKs
       
        
        if ($tar) {

            echo "<font color=" . $this->colors["normalText"] . " size=+1><b>1. download ";
            echo "<a href=$file.tar.php>paxml.tar</a></b></font>";
            echo " (" . $this->displayFilesize(@filesize($exportpath . ".tar")) . ")";
			echo "  - main paxml package file ";
            if ($attached)
                echo ", including attached documents and images<br><br>";
            else
                echo " ... <br><br>";
        }
        
        if ($this->viewMode >= PaXML_MINVIEW) {
        
            if ($tar and ($xml or $zip)) {
                echo "<ul><font color=" . $this->colors["grayText"] . ">optional file formats:</font><br>";
                echo "<font color=" . $this->colors["normalText"] . ">\n";
            }
   
            if ($xml) {
                echo "- pure xml-file <a href=$file.xml.php>paxml.xml</a>";
                echo " (" . $this->displayFilesize(filesize($exportpath . "/export.xml")) . ") (no attachments like images)";
            }
            if (!($xml and $attached))
                echo " ...<br>\n";
            else
                echo ". No attached documents delivered with this xml file! ...<br>";
            if ($zip)
            {
                echo "- pure compressed xml-file <a href=$file.xml.gz.php>paxml.xml.gz</a>";
                echo " (" . $this->displayFilesize(@filesize($exportpath . "/export.xml.gz")) . ") (no attachments like images)";
            }
            if (!($zip and $attached))
                echo " ...<br>\n";
            else
                echo ". No attached documents delivered with this zip file! ...<br>\n";
            
                
                if ($tar and ($xml or $zip)) {
                    echo "\n</font>";
                    echo "</ul>\n";
                }
                
            
            echo "<br><br>";
        }
        
        
        echo "<font color=" . $this->colors["normalText"] . " size=+1><B>2. <a href='export.clean.php'>clean temporary data at server</a></B></font>".
             " and go back to current project <br><br>\n";
        
        
        // STATS
        
        echo "<hr width=300 noshade size=1 align=left noshade><br><font size=2><b>Statistics:</b></font><br>\n";
        
        echo "<table border=0 cellspacing=0 cellpadding=0>";
        if ($this->statistics["timer"]) {
            foreach($this->statistics["timer"] as $id => $time) {
                echo "\n<tr><td nowrap><font size=2> &nbsp; &nbsp; used time for $id: </font></td><td><font size=2> " .    $this->displayTime($time) . "</font></td></tr>";
            }
            
        } else {
            echo "\n<tr><td nowrap><font size=2> &nbsp; &nbsp; used time </font></td><td><font size=2> not recorded</font></td></tr>";
        }
        
        if ($this->statistics["objects"]) {
            $tmpArray = $this->statistics["objects"];
            uksort($tmpArray, "cmp");
            foreach($tmpArray as $name => $item) {
                $tab_nice = tablename_nice2($name);
                echo "\n".
                    '<tr><td nowrap><font size=2> &nbsp; &nbsp; exported "object.' . $tab_nice . '": &nbsp; </font></td><td><font size=2> ' . $item . '</font></td></tr>';
            }
        }
        
        if ($this->statistics["attach"])
            echo "\n<tr><td nowrap><font size=2> &nbsp; &nbsp; exported attachments: &nbsp; </font></td><td><font size=2> " . $this->statistics["attach"] . "</font></td></tr>";
            
        if ($this->statistics["files"]) 
            echo "\n<tr><td nowrap><font size=2> &nbsp; &nbsp; exported files: &nbsp; </font></td><td><font size=2> " . $this->statistics["files"] . "</font></td></tr>";
        
        if ($this->statistics["classes"])
            echo "\n<tr><td nowrap><font size=2> &nbsp; &nbsp; exported classes: &nbsp; </font></td><td><font size=2> " . $this->statistics["classes"] . "</font></td></tr>";
                
                
        if ($this->statistics["noexpobj"])  // NOT exported objects
        {
            $tmpArray = $this->statistics["noexpobj"];
            uksort($tmpArray, "cmp");
            foreach($tmpArray as $name => $item) {
                $tab_nice = tablename_nice2($name);
                echo "\n". '<tr><td nowrap><font size=2 color=red> &nbsp; &nbsp; objects DENIED for export: "' . $tab_nice .
                  '": &nbsp; </font></td><td><font size=2> ' . $item . '</font></td></tr>';
            }
        }
        echo "</table>\n";
        
        echo "<br>\n";
        //echo "<hr width=300 noshade size=1 align=left noshade><br><font size=2>";
        
        
        
        // echo "<hr width=300 noshade size=1 align=left noshade>";
        
    }
}


