<?

//
//  class:      PaXMLFileOuput
//  descr:      class for writing partisan xml to a file
//  version:    class version 0.1
//
//  2002-09-09 rogo start
//


require_once("XMLFileOutput.inc");
require_once("PaXMLTags.inc");




// class for writing partisan xml output. this class is derived from
// XMLFileOutput, which declares some variables and simple functions.
class PaXMLFileOutput extends XMLFileOutput
{

    // constructor. set filename
    //
    // input:   string      $filename   output file name
    // output:  -
    //
    function __construct($filename)
    {
        parent::__construct($filename);
    }



    // open file and write tag <partisanxml version='' db=''>
    /**
     * old:  $encoding ="ISO-8859-1"
     * @param string $version
     * @param mixed $databaseID
     * @param string $dbversion
     */
    function startXML($version, $databaseID, $dbversion, $encoding = "UTF-8")
    {
        $this->open();
        $this->write("<?xml version='1.0' encoding='" . $encoding . "' ?" . ">");
        $this->openElement(PaXMLTag_PARTISANXML, array(
				PaXMLTag_PARTISANXML_VERSION    => $version, 
				PaXMLTag_PARTISANXML_DB 		=> $databaseID,
				PaXMLTag_PARTISANXML_DBVERSION  => $dbversion));
    }

    // write tag </partisanxml> and close file
    function endXML()
    {
        $this->closeElement(PaXMLTag_PARTISANXML);
        $this->close();
    }



    // <object type='' wiid='' roid='' [creator=''][modifier=''][creationDate=''][modificationDate=''][fieldID='']>
    function startObject($type, &$infoarr )
	/*		infoarr(
		     	"wiid",  !
				"roid",  !
                "crea_date", !
				"mod_date",
				"creator",
				"modifier"
				"special",
				"attach"
				) 
	*/
    {
        $attribs=array();
        $attribs[PaXMLTag_OBJECT_TYPE]              = $type;
        $attribs[PaXMLTag_OBJECT_WIID]              = $infoarr["wiid"];
        $attribs[PaXMLTag_OBJECT_ROID]              = $infoarr["roid"];
        $attribs[PaXMLTag_OBJECT_CREATION_DATE]     = $infoarr["crea_date"];
        $attribs[PaXMLTag_OBJECT_MODIFICATION_DATE] = $infoarr["mod_date"];
		$attribs[PaXMLTag_OBJECT_CREATOR]     		= $infoarr["creator"];
		$attribs[PaXMLTag_OBJECT_MODIFIER]     		= $infoarr["modifier"];
        $attribs[PaXMLTag_OBJECT_FIELDID]           = $infoarr["special"];
        $attribs[PaXMLTag_OBJECT_ATTACH]            = $infoarr["attach"];
        $this->openElement(PaXMLTag_OBJECT, $attribs);
    }

    // </object>
    function endObject()
    {
        $this->closeElement(PaXMLTag_OBJECT);
    }

    // <object type='' wiid='' roid='' [fieldID=''] />
    function refObject($type, $wiid, $roid, $special = null)
    {
        $attribs=array();
        $attribs[PaXMLTag_OBJECT_TYPE]    = $type;
        $attribs[PaXMLTag_OBJECT_WIID]    = $wiid;
        $attribs[PaXMLTag_OBJECT_ROID]    = $roid;
        $attribs[PaXMLTag_OBJECT_FIELDID] = $special;
        $this->emptyElement(PaXMLTag_OBJECT, $attribs);
    }



    // <parameters [class='']>
    function startParameters($className = null)
    {
        $this->openElement(PaXMLTag_PARAMETERS, 
            array(PaXMLTag_PARAMETERS_CLASS => $className)
            );
    }

    // <parameters />
    function endParameters()
    {
        $this->closeElement(PaXMLTag_PARAMETERS);
    }
    
    //
    function writeParameters($parameters)
    {
        if ($parameters != null and is_array($parameters))
            foreach($parameters as $name => $value)
            {
                if (is_array($value) and $value == null)
                    $this->emptyParameter($value["name"], $value["type"]);
                elseif (is_array($value) and $value != null)
                {
                    $this->startParameter($value["name"], $value["type"]);
                    $this->writeText($value["value"]);
                    $this->endParameter();
                }
                elseif (!is_array($value) and $value == null)
                    $this->emptyParameter($name);
                elseif (!is_array($value) and $value != null)
                {
                    $this->startParameter($name);
                    $this->writeText($value);
                    $this->endParameter();
                }
            }
    }



    // <parameter name='' [type='']>
    function startParameter($name, $type = null)
    {
        $this->openElement(PaXMLTag_PARAMETER, array(PaXMLTag_PARAMETER_NAME => $name, PaXMLTag_PARAMETER_TYPE => $type));
    }

    // </parameter>
    function endParameter()
    {
        $this->closeElement(PaXMLTag_PARAMETER);
    }

    // <parameter name='' [type=''] />
    function emptyParameter($name, $type = null)
    {
        $this->emptyElement(PaXMLTag_PARAMETER, array(PaXMLTag_PARAMETER_NAME => $name, PaXMLTag_PARAMETER_TYPE => $type));
    }

    // <parameter name='' [type='']>content</parameter
    // or
    // <parameter name='' [type=''] />
    function writeParameter($name, $value, $type = null)
    {
        if ($value != null)
        {
            $this->startParameter($name, $type);
            $this->content($value);
            $this->endParameter();
        }
        else
            $this->emptyParameter($name, $type);
    }
    
    // --- END PARAMETERS
    
  
    // <parameter name='' [type='']>
    function startClass($name, $type = null)
    {
        $this->openElement(PaXMLTag_CLASS, array(PaXMLTag_CLASS_NAME => $name, PaXMLTag_CLASS_TYPE => $type));
    }

    // </parameter>
    function endClass()
    {
        $this->closeElement(PaXMLTag_CLASS);
    }

    // <parameter name='' [type=''] />
    function emptyClass($name, $type = null)
    {
        $this->emptyElement(PaXMLTag_CLASS, array(PaXMLTag_CLASS_NAME => $name, PaXMLTag_CLASS_TYPE => $type));
    }



    // <attribute name=''>
    function startAttribute($name)
    {
        $this->openElement(PaXMLTag_CLASSATTRIBUTE, array(PaXMLTag_CLASSATTRIBUTE_NAME => $name));
    }

    // </attribute>
    function endAttribute()
    {
        $this->closeElement(PaXMLTag_CLASSATTRIBUTE);
    }
    
    

    // <parameter name='' [type='']>
    function startOption($type, $name)
    {
        $this->openElement(PaXMLTag_OPTION, array(PaXMLTag_OPTION_NAME => $name, PaXMLTag_OPTION_TYPE => $type));
    }

    // </parameter>
    function endOption()
    {
        $this->closeElement(PaXMLTag_OPTION);
    }

    // <parameter name='' [type=''] />
    function refOption($type, $name)
    {
        $this->emptyElement(PaXMLTag_OPTION, array(PaXMLTag_OPTION_NAME => $name, PaXMLTag_OPTION_TYPE => $type));
    }



    // <parameter name='' [type='']>
    function startRelationships($type)
    {
        $this->openElement(PaXMLTag_ASSOCIATIVES, array(PaXMLTag_ASSOCIATIVES_TYPE => $type));
    }

    // </parameter>
    function endRelationships()
    {
        $this->closeElement(PaXMLTag_ASSOCIATIVES);
    }
    
    
    
    // <parameter name='' [type='']>
    function startRelationship($class)
    {
        $this->openElement(PaXMLTag_ASSOCIATIVE, array(PaXMLTag_ASSOCIATIVE_CLASS => $class));
    }
	
	 // <element file=''>
    function startRelationAttach($file)
    {
        $this->openElement(PaXMLTag_ASSOCIATIVE, array(PaXMLTag_ASSOCIATIVE_FILE => $file));
    }

    // </parameter>
    function endRelationship()
    {
        $this->closeElement(PaXMLTag_ASSOCIATIVE);
    }

}

?>
