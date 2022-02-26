<?

//
//  class:      XMLFileOuput
//  descr:      this class implements XMLOutput. It's a xml-to-file-writer.
//  version:    class version 0.1
//
//  2002-09-09 rogo start
//



require_once("XMLOutput.inc");



// class for writing xml output
class XMLFileOutput extends XMLOutput
{


    var $buffer;        // output buffer
    var $buffersize;    // output buffer size

    var $file;          // file handle
    var $filename;      // outfile filename

    var $indentString;  // string for indention
    var $indent;        // xml tag indent
    var $charset;       // name of charset
    
    var $contentFlag;   // internal flag; do not use for own programming
    
    
    
    // constructor. takes file and initialize write buffer.
    //
    // input:   string      $filename   output file name
    //          number      $buffersize buffer size for writing
    // output:  -
    //
    function __construct($filename, $buffersize = 64768)
    {
        $this->buffer       = "";
        $this->buffersize   = $buffersize;
        $this->filename     = $filename;
        $this->file         = null;
        $this->indent       = 0;
        $this->indentString = " ";
        $this->charset      = null;
        $this->contentFlag  = false;
    }



    // opens output file. returns result of fopen (handle or false). the result
    // is also saved to class variable
    //
    // input:   -
    // output:  fopen()
    //
    function open()
    {
        $this->file = fopen($this->filename, "w");
   	    return $this->file;
    }



    // closes output file. before flush buffer. returns false if something went
    // wrong.
    //
    // input:   -
    // output:  fclose() | void
    //
    function close()
    {
        if (!$this->file)
            return;
            
        $this->flush();
        return fclose($this->file);
    }
    
    
    
    // write xml data to file. buffers output before writing. returns true of
    // false
    //
    // input:   string      $data       content to write
    //          bool        $flush      force to write buffer
    // output:  void
    //
    function write($data, $forcedFlush = false)
    {
        if (!$this->file)
            return;

        $this->buffer .= $data;
        if (strlen($this->buffer) > $this->buffersize or $forcedFlush)
        {
            // write to file and reset buffer
            $result = fwrite($this->file, $this->buffer);
            $this->buffer = "";
        }
    }
    

    
    // open element by writing open tag with attributes
    // <element>
    // <element attrib1='value1' ...>
    //
    // input:   string      $name       name of element
    //          array       $attribs    attribs of element (name => value)
    // output:  -
    //
    function openElement($name, $attribs = null)
    {
        // generate output  for attributes. check attributes for ' and replace
        // them by &apos;. (XML
        $tagAttribs = "";
        if ($attribs != null and is_array($attribs))
            foreach($attribs as $key => $value)
                if ($value != null)
                    $tagAttribs .= " $key='" . str_replace("'", "&apos;", $value) . "'";

        $this->write("\n" . str_repeat($this->indentString, $this->indent++) . "<$name" . $tagAttribs . ">");
        $this->contentFlag = false;
    }
    
    
    
    // close element
    // </element>
    //
    // input:   string      $name       name of element
    // output:  -
    // errors:  -
    //
    function closeElement($name)
    {
        if ($this->contentFlag)
        {
            $this->write("</$name>");
            --$this->indent;
        }
        else
            $this->write("\n" . str_repeat($this->indentString, --$this->indent) . "</$name>");

        $this->contentFlag = false;
    }
    
    
    
    // write empty element (no content)
    // <element />
    // <element attrib1='value1' attrib2='value2' />
    //
    // input:   string      $name       name of element
    //          array       $attribs    attribs of element (name => value)
    // output:  -
    // errors:  -
    //
    function emptyElement($name, $attribs = null)
    {
        // generate output  for attributes. check attributes for ' and replace
        // them by &apos;. (XML
        $tagAttribs = "";
        if ($attribs != null and is_array($attribs))
            foreach($attribs as $key => $value)
                if ($value != null)
                    $tagAttribs .= " $key='" . str_replace("'", "&apos;", $value) . "'";

        $this->write("\n" . str_repeat($this->indentString, $this->indent) . "<$name" . $tagAttribs . " />");
    }
    
    
    
    // write content for elements
    // ...>content</...
    //
    // input:   string      $data       content of element
    // output:  -
    //
    function content($data)
    {
        $this->contentFlag = true;
        $transtable = array(">" => "&gt;", "<" => "&lt;", "&" => "&amp;",
                            "'" => "&apos;", "\"" => "&quot;");
        $this->write(strtr($data, $transtable));
    }
    
    
    
    // write comment
    //
    // input:   string      $comment    comment
    // output:  -
    //
    function comment($comment)
    {
        $this->write("<!-- $comment //-->");
    }



    // flushes buffer
    //
    // input:   -
    // output:  -
    //
    function flush()
    {
        $this->write(null, true);
    }

    
    
    // return filename
    //
    // input:   -
    // output:  $this->filename
    //
    function getFilename()
    {
        return $this->filename;
    }
}

?>
