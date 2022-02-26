<?

// 2002-12-01   rogo    start

// contains a abstract class for xml parsing from a file. taking a xml file as
// input and parses them. how to use, please read class description.



require_once("XMLPath.inc");



// this class contains the basic functionality for xml sax parser. in fact this
// class is an abstract class. if you want to use, create a own class derived
// from XMLFileInput. create functions with names which are a concationation
// of "open" and tagname and "close" and tagname. example, if you have tags like
// "book" and "kitchen" create functions like "openBOOK()" and "closeBOOK" also
// like "openKITCHEN" and "closeKITCHEN". it's not necessary to implement for
// any tag a function.
class XMLFileInput
{
    var $xmlFilename;       // xml file name
    var $xmlPath;           // handle for class XMLPath
    var $tagData;           // data inside a tag
    var $parser;            // parser
	var $html; // initialized by parent ...


    // conctructor.
    //
    // input:   $xmlFilename    xml file
    // output:  -
    //
    function __construct($xmlFilename)
    {
        $this->xmlFilename = $xmlFilename;
        $this->tagData     = null;
        $this->xmlPath     = new XMLPath();
  
    }
    


    // hardcore stop of xml execution. free xml parser and display message
    //
    // input:   sring   $message        your last three words
    // output:  -
    //
    function stopXMLProcessing($message, $kill = true)
    {
        if ($this->parser != null)
            xml_parser_free($this->parser);

        if ($kill)
            die($message);
    }
    
    
    
    // init sax parser. set character encoding (fix). set event handler.
    //
    // input:   -
    // output:  -
    //
    function initParser()
    {
        $success      = true;
        $this->parser = xml_parser_create(); // OLD: "ISO-8859-1"
        $success      = $success and xml_set_object($this->parser, $this);
	    $success      = $success and xml_set_element_handler($this->parser, "tagOpenHandler", "tagCloseHandler");
    	$success      = $success and xml_set_character_data_handler($this->parser, "tagData");
        $success      = $success and xml_set_default_handler($this->parser, "defaultHandler");

        if (!$success)
            $this->stopXMLProcessing("Parser initialisation fault. Execution stopped.");
    }



    // start to parse. run the xml parse process. start this function causes
    // xml events, which are handled by functions of this class.
    //
    // input:   -
    // output:  -
    //
    function parse()
    {
    	
    	$READ_BLOCK_SIZE=65536; //32768;
        // check file
        if (!file_exists($this->xmlFilename))
        	$this->stopXMLProcessing("No xml file found. <!-- session id: "
                                   . session_id() . "; file: "
                                   . $this->xmlFilename . " //-->");
        
        $this->html->XML_file_start($this->xmlFilename, $READ_BLOCK_SIZE);

        // open file
		if (!($file = fopen($this->xmlFilename, "r")))
        	$this->stopXMLProcessing("Could not open xml file $xmlFilename. <!-- session id: "
                                   . session_id() . "; file: " . $this->xmlFilename . " //-->");

		$block_cnt=0;
        // start parsing
		while ($data = fread($file, $READ_BLOCK_SIZE)) {	

			$this->html->XML_file_progress($block_cnt);
			
 			if (!xml_parse($this->parser, $data, feof($file))) {
 				
        		$this->stopXMLProcessing("Parser error, because "
                     . xml_error_string(xml_get_error_code($this->parser))
                     . " at line " . xml_get_current_line_number($this->parser)
                     . " (error number: " . xml_get_error_code($this->parser)
                     . ").");
 			}
 			$block_cnt++;
		}
		
		$this->html->XML_file_end($block_cnt);
        // free parser
        xml_parser_free($this->parser);
    }



    // handle tag opening. set time limit up, to prevent max-execution time
    // event. replace &apos. init tagData. check if function exists for this
    // tag (this works also for dervied classes) and run it. Note, $name and
    // $attribs key are allways written in uppercase letters.
    //
    // input:   resource    $parser         handle of parser
    //          string      $name           tag name
    //          array       $attribs
    // output:  -
    //
    function tagOpenHandler($parser, $name, $attribs)
    {
        set_time_limit(3000);   // add 50 min more execution time. this
                                // avoids a premature interrupt by server
                                // for scripts running to long. this might
                                // happen if you have very big amount of
                                // data, like piet did (4000 exps).

        // &apos; is a special entity in xml attributes, which has to be
        // replaced by '.
        foreach($attribs as $key => $value)
            $attribs[$key] = str_replace("&apos;", "'", $value);

        $this->xmlPath->push($name);
        $this->tagData = "";

        // call function openTAGNAME if exists
        if (method_exists($this, "open" . $name))
			call_user_func(array(&$this, "open" . $name), $parser, $attribs);
    }



    // handle tag closing. check if function exists for this tag (this works
    // also for dervied classes) and run it. Note, $name is allways written in
    // uppercase letters.
    //
    // input:   resource    $parser         handle of parser
    //          string      $name           tag name
    // output:  -
    //
    function tagCloseHandler($parser, $name)
    {
        // are xml tags matching?
        if ($this->xmlPath->pop() != $name)
            $this->stopXMLProcessing("XML Tag mismatch. <!-- $name //-->");

        // call function closeTAGNAME if exists
  		if (method_exists($this, "close" . $name))
		    call_user_func(array(&$this, "close" . $name), $parser);
		    
        $this->tagData = null;
    }



    // handle tag data. data between opening tag and closing tag are saved in
    // tagData here.
    //
    // input:   resource    $parser         handle of parser
    //          string      $data           data
    // output:  -
    //
 	function tagData($parser, $data)
 	{
        // eliminate special entities. changes here have to reflected also in
        // PaXMLFileOutput!
        $translationTable = array(">" => "&gt;", "<" => "&lt;", "&" => "&amp;",
                                  "'" => "&apos;", "\"" => "&quot;");
        $translationTable = array_flip($translationTable);
        $this->tagData   .= strtr($data, $translationTable);
	}



    // do not remove, it's used. it's needed to handle processing instruction.
    // yes, it's a dummy function.
    //
    // input:   resource    $parser         handle of parser
    //                      $data           data of processing instruction
    // output:  -
    //
    function defaultHandler($parser, $data) {}
}

// end.
?>
