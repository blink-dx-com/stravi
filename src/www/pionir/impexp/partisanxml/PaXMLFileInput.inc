<?php
require_once("XMLFileInput.inc");
require_once("PaXMLTags.inc");
require_once("Stack.inc");

/***
 * this is the paxml parser based on sax technology. this class uses PaXMLReader
 * 
 * this clas is the implementation of the XMLFileInput specialized for PaXML.
 * Here in this class you will find all tag handlers. Also the xml data will be
 * collected here in $stack. And some causality checks are done.
 * 
 * @package PaXMLFileInput.inc
 * @swreq   UREQ:0000171: g.paxml-import > HOME UREQ 
 * @swreq   UREQ:0000663: g.paxml.import > update objects I > ABSTRACT_PROTO_STEP
 * @author  Rene Ogorsolka (Original Master)
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @history 
   2001-07-09  rogo	   started to write that
   2004-02-23  qbi     added ->reader before variable updateList
   2004-02-23  qbi     added ->reader before variable updateList  
   2010-10-23  qbi     added ->support update of ABSTRACT_PROTO_STEP, if a 
   		protocol exists, but a step is missing 
   		this can happen, if this ABSTRACT_PROTO was extended on the original DB

 * devnotes:    how the $stack works.
 *
 *              the $stack variable is a container for collection all data,
 *              which has been parsed and not processed finally. the stack
 *              represents also the current path in xml structure. $stack is
 *              of type array. An element will be add always at the end of
 *              stack and removed from the end (lifo principle). that the
 *              element may have a complexe array structure doesn't matter.

 * devnotes:    principle of xml handling (sax parser)
 *
 *              an xml element is allways processed by three handlers. handler
 *              for opening tag, handler for closing tag and handler for stuff
 *              between tags (data handler). this rule rules also at short tags.
//
 *              how does this program handle tags generally? at opening tag the
 *              parent $data structure will be saved in stack. attributes are
 *              read and saved in a new empty $data structure. the c-data handler
 *              collects all data in string $tagData. at closing tag, the
 *              collected data will be imported and the parent structure
 *              restored from stack. the id of newly created data can be added
 *              now to the parent structure. this is the basic principle of xml
 *              tag handling.
 */
class PaXMLFileInput extends XMLFileInput
{
    var $reader;                // handle of PaXMLReader
    var $html;                  // handle of PaXMLHTMLOutputImport
    var $stack;                 // xml data stack
    
    var $xmlVersion;            // xml version
    var $remoteDatabase;        // source database
    var $updatingCache;         // flag for update cache force
    
	var $validateMode;
	var $lib; 					// THE LIBRARY
	var $data;					/** @var array $data OBJ_ONE_data_STRUCT
        ["type"]       = classElement,attributeElement,objectElement,parametersElement,listingElement
        ["name"]       = name of class or option
        ["table"]      = name of table
        ["params"]     = parameter array
            "name"   => 
			"value" => 
			"type"  => 
			   "class"
               "option"
               "vario"
               "objlink" [TBD:] must be coded
        ["vario"]      = VARIO array of array()
                            "name"  : string
                            "value"  : string
        ["import"]     = true/false do import of this element ?
        ["update"]     = true/false do update of this element ? e.g. ABSTRACT_PROTO_STEP
        ["attribs"]    = array() attributes
	*/
	
	private $isp;
	private $_search_import_start=0;

    // constructor
    //
    // input:   resource    $reader     PaXMLReader object
    //          string      $filename   xml file
    // output:  -
    //
    function __construct( &$reader, $filename)
    {
        parent::__construct($filename);

        if (!$reader and get_class($reader) == "paxmlreader")
            return;

        $this->reader     = &$reader;                // check class type
        $this->xmlVersion = PaXML_XMLVERSION;       // defined in PaXMLTags.inc
		$this->lib 		  = &$reader->lib;
		
		$this->isp = NULL;
		
		$this->_tableNow = NULL;
		$this->_tableInfo = NULL;
    }
	
	// get primary key of table
	// use the cache $this->_tableInfo
	function _getPrimKey($table) 
	{
		if ($this->_tableNow != $table) 
		{
			$this->_tableInfo = NULL;
			$this->_tableInfo['pk'] = $this->lib->getPrimKey($table);
			$this->_tableNow = $table;
		}
		$pkname = $this->_tableInfo['pk'];
		return ($pkname);
	}


	function setValidateMode()
	{
		$this->validateMode = true;
	}


    // init this class
    //
    // input:   -
    // output:  -
    //
    function init()
    {
        $this->html  = &$this->reader->getHTML();
        $this->stack = new Stack();
        $this->initParser();
        $this->html->parser = &$this->parser;
    }

    /**
     * set options for import
     * @param string $key
     * @param mixed $val
     */
	function set_option($key, $val) {
		switch ($key) {
			case 'isp':
				$this->isp = $val;
				$this->_search_import_start = 1;
				break;
		}
	}

    // stop hardcore
    //
    // input:   string      $message        error message
    // output:  -
    //
    function stopXMLProcessing($message, $secret = null)
    {
        // rollback all transactions
        $this->reader->rollbackAll();
        
        // free sax parser resource
        parent::stopXMLProcessing($message, false);

        // try to update varcol cache
		if ( $this->reader->updatingCache > 0) $this->updatingCache = TRUE;
        $message = $message . $this->reader->updateVarcolCache($this->updatingCache);

        // kill execution
        $this->html->stop($message, $secret);
    }



    // handle tag <partisanxml>
    //
    // input:   resource    $parser         handler of parser
    //          array       $attribs        tag attributes
    // output:  -
    //
    function openPARTISANXML($parser, $attribs)
    {
		$pxopt = NULL;
		$pxopt["dbversion"] = $attribs[strtoupper(PaXMLTag_PARTISANXML_DBVERSION)];
        $this->html->openPARTISANXML($attribs["VERSION"], $attribs["DB"], $pxopt);
        $this->html->startTimer("import");
        
        if ( $this->_search_import_start ) {
        	$this->html->isp_do($this->isp['WIID'], $this->isp['ROID']);
        }
        // version check
        if ($attribs["VERSION"] <  PaXML_XMLVERSION_MIN)
            $this->stopXMLProcessing("The partisan xml version '" . $attribs["VERSION"]
                                   . "' is too old and incompatible with current version. Import aborted.");

        if ($attribs["VERSION"] != $this->xmlVersion)
            $this->stopXMLProcessing("The partisan xml version '" . $attribs["VERSION"] . "' is unknown. Import aborted.");

        // db check. if check is successfull, register wiidname with contact
        if ($attribs["DB"] == null)
            $this->stopXMLProcessing("Missing magasin serial. Import abort.");
        else
        {
            $this->reader->alignWiidToContact(null, $attribs["DB"]);
	        $this->remoteDatabase = $attribs["DB"];
        }
		
		$thisDBVersion = $this->lib->getDBVersion();	// get DB version
		if ($thisDBVersion != $pxopt["dbversion"]) {
			$this->html->warning("the DBVersion of the paxml-file ".$pxopt["dbversion"]." is different to this DBVersion ".$thisDBVersion);
		}

        $this->data  = array("type" => "rootElement");      // dummy structure
		$this->html->flushText();
    }



    // handle tag </partisanxml>. stuff which is not commited till now is lost.
    //
    // input:   resource    $parser         handler of parser
    // output:  -
    //
    function closePARTISANXML($parser)
    {
        $this->html->stopTimer("import");
        $this->html->closePARTISANXML();
		if ( $this->reader->updatingCache > 0) $this->updatingCache = TRUE;
        $errormessage = $this->reader->updateVarcolCache($this->updatingCache);
        if ($errormessage)
            $this->html->warning($errormessage);
    }



    // handle tag <class>
    //
    // input:   resource    $parser         handler of parser
    //          array       $attribs        tag attributes
    // output:  -
    //
    function openCLASS($parser, $attribs)
    {
        $attribs["TYPE"]          = strtoupper($attribs["TYPE"]);

        $this->html->openCLASS($attribs["NAME"], $attribs["TYPE"]);

        $this->stack->push($this->data);

        $this->data               = array();
        $this->data["type"]       = "classElement";				     // type of object structure 'class'
        $this->data["name"]       = $attribs["NAME"];        	  	 // set up class name
        $this->data["table"]      = strtoupper($attribs["TYPE"]);    // set up table
        $this->data["params"]     = array();       				     // prepare parameter of extra class
        $this->data["vario"]      = array();       				     // prepare parameter of VARIO
        $this->data["import"]     = true;	        				 // do import = yes! (by default)
        $this->data["attribs"]    = array();	          			 // prepare attributes to extra class (extra_attrib)

        $this->reader->verifyClass($this->data["table"]);            // check $this->data["table"]

        // try to find the class in database
        $this->data["foundClass"] = $this->reader->findClass($attribs["NAME"], $attribs["TYPE"]);

        // class found ...
        if ($this->data["foundClass"] != null)
        {
            // ... mark as 'not for import'
            $this->html->classFound($this->data["foundClass"]);
            $this->data["import"] = false;
        }
        else
        {
            // ... import required. varcol cache update required.
            $this->html->classNotFound();
            $this->data["import"] = true;	        				
            $this->updatingCache  = true;
        }
    }



    // handle tag </class>
    //
    // input:   resource    $parser         handler of parser
    // output:  -
    //
    function closeCLASS($parser)
    {
        if ($this->data["type"] != "classElement")
            $this->stopXMLProcessing("Stack mismatch error. Expecting stack data for tag 'class'. <!-- received data: "
                                   . $this->data["type"] . " //--> Import aborted. Please contact your administrator.");

        // if class already exists ...
        if (!$this->data["import"])
        {
            // ... validate if input class is the same or compatibel to existing
            // class
            $this->html->validateClass();
            $this->reader->validateClass($this->data);
            $this->html->validatedClass();
        }
        else
        {
            // ... import class
            $this->reader->importClass($this->data);
        }
        
        // class finished, restore previous data of super element
        $this->html->endCLASS();
        $this->data = $this->stack->pop('closeCLASS');
    }



    // handle tag <attribute>
    //
    // input:   resource    $parser         handler of parser
    //          array       $attribs        tag attributes
    // output:  -
    //
    function openATTRIBUTE($parser, $attribs)
    {
        if ($this->data["type"] == "classElement")      // is it called in context of 'class'
        {
            $this->html->startAttribute($attribs["NAME"]);
            $this->stack->push($this->data,'openATTRIBUTE');            // ... save previous object 'class'
            $this->data		      = array();			// and create attribuet object structure
            $this->data["type"]   = "attributeElement"; // attribute type
            $this->data["name"]   = $attribs["NAME"];   // attribute name
            $this->data["params"] = array();            // params prepared
        }
    }
    
    
    
    // handle tag </attribute>
    //
    // input:   resource    $parser         handler of parser
    // output:  -
    //
    function closeATTRIBUTE($parser)
    {
        if ($this->data["type"] == "attributeElement")
        {
            $this->html->endAttribute();
            $attributeData           = $this->data; 	 // save current structure in a var ...
            $this->data              = $this->stack->pop('closeATTRIBUTE');
            $this->data["attribs"][] = $attributeData;
        }
    }
    
    
    
    // handle tag <object>
    //
    // input:   resource    $parser         handler of parser
    //          array       $attribs        tag attributes
    // output:  -
    //
    function openOBJECT($parser, $attribs)
    {
        $attribs["TYPE"] = strtoupper($attribs["TYPE"]);

        // was parent object found so take import flag from parent, except
        // parent was an project, so keep import flag on, to update new
        // objects to project
        //
		// old rule:   if a parent object was not imported,
		//			   because it was found, so the child
		//			   bo's have to exist yet
		// new rule: so the child bo's should also be checked for existens
		
		//$parentalDecision = ($this->data["type"] == "objectElement") ? $this->data["import"] : true;
		
		
		$parentalDecision = true; // NEW:2004-04-06 by Qbi LEVEL:1 
		
		if ( $this->_search_import_start ) {
			
			// search for the start project
			$parentalDecision = false;
			if ($attribs["TYPE"] == "PROJ") {
				if ($attribs["WIID"]==$this->isp['WIID'] and  $attribs["ROID"]==$this->isp['ROID']) {
					// start found 
					$parentalDecision = true;
					$this->_search_import_start = 0; // continue normal ...
					$this->html->isp_found($attribs["WIID"], $attribs["ROID"]);
				}
			}
			
		}
		
		if (  $attribs["TYPE"] == "PROJ" ) $parentalDecision = true; // NEW: Steffen 2018
		
		// $parentalDecision = (!$parentalDecision && $this->data["table"] == "PROJ") ? true : $parentalDecision;
		
		
		//if ($this->validateMode)
		//	$parentalDecision = true;
		
		// i save parent dataset here, before pushing this object structure to stack
        // when current object is a project as subproject
        //

		if ($attribs["TYPE"] == "PROJ" && $this->data["table"] == "PROJ")
		{
            $objID    = $this->data["id"];
		    $objTable = $this->data["table"];
		}
		
		// these lines are made for only one case: a project was exported. this
        // project was a link in the source database. the original parent isn't
        // available. the link has to be transformed to a real subproject of
        // current project.
        //
        // example:     there's a project A having a subproject B. there's
        //              another project C having a as subproject a link to B.
        //              if C is exported B is also exported, but as link.
        //              this is a problem at import, because B is exported
        //              without A its real parent. the new real parent project
        //              for B has to be C.
		if ($attribs["TYPE"] == "PROJ" && $this->data["table"] == "PROJ_HAS_ELEM")
		{
		    $savedata     = $this->data;              // save current data
		    $this->data   = $this->stack->pop('openOBJECT:1');      // getting parent of parent
            $specialID    = $this->data["id"];        //
		    $specialTABLE = "PROJ_HAS_ELEM";
		    $this->stack->push($this->data,'openOBJECT:1');          // restore everything
		    $this->data   = $savedata;
		}

        $this->stack->push($this->data,'openOBJECT:2');              // save parent structure
        
        $this->data		             = array();					   // ... and create b structure
		$this->data["type"]          = "objectElement";		       // type business object structure
		$this->data["table"]         = $attribs["TYPE"];
		$this->data["wiid"]          = $attribs["WIID"];
		$this->data["roid"]          = $attribs["ROID"];
		$this->data["field"]         = $attribs["FIELDID"];        // identfies field if there are different from generic concept
		$this->data["crea"]          = $attribs["CREATION_DATE"];  // check is it a date ?
		$this->data["mod"]           = $attribs["MODIFICATION_DATE"];
		$this->data["creator"]       = $attribs["CREATOR"];		   // optional
		$this->data["modifier"]      = $attribs["MODIFIER"];
		
        $this->data["file"]          = $attribs["ATTACH"];
		$this->data["import"]        = $parentalDecision;		   // take parent's import decision
		$this->data["pid"]           = $objID;				       // parent object id
		$this->data["ptable"]        = $objTable;				   // parent object table
        $this->data["specialPID"]    = $specialID;
		$this->data["specialPTABLE"] = $specialTABLE;
        $this->data["params"]        = array();                    // place holder for parameters
        $this->data["objDone"]       = true;
        $this->data["displayed"]     = false;
        // if base of this contact object exists, when save it. it's a
        // special option for contact tables only
        $this->data["base"]          = ($attribs["TYPE"] == "CONTACT") ? $attribs["WIID"] : $this->data["base"];

        // no search/import required?
        if (!$parentalDecision)
        {
            $this->data["displayed"] = $this->html->noSearchForObject($attribs["TYPE"], $attribs["WIID"], $attribs["ROID"], $attribs["ATTACH"]);
            
			if (!$this->validateMode)
				return;
        }

        $this->data["displayed"] = $this->html->startObject($attribs["TYPE"], $attribs["WIID"], $attribs["ROID"], $attribs["ATTACH"]);

        // verify input data from xml file
        $this->reader->verifyObject($this->data["table"]);

        // try to find xml object in database
        $foundObject = $this->reader->findObject($this->data["table"], $this->data["wiid"],  $this->data["roid"]);

        
        // check if current tag is only a object referenz tag (no creation date)
        // and this object was not found. object referenz tag are only written,
        // if object is described before, except object needs itself
        // to describe. a circle reference. this problem will be solved with a
        // temporary object.
		// A circular object will be indicated during the EXPORT by setting data["crea"] = null 		
		// Note, this is a week indicator for a circular
        // referencing object, because it's only indicated by a reference
        // without existing data (usually an illegal state)
        if (!$foundObject and $this->data["crea"] == null and !$this->validateMode)
        {
            // Ahhhhhhhhhhhh. Shit, a circular referencing object get a temp
            // object (FUCKING Kreisreferenz). When object has been describben
            // completly and it gets its local database id, the link to temp
            // object has to be replaced
            $this->html->circularReferencingObject();
            $foundObject           = $this->reader->getTempObject($this->data["table"], $this->data["wiid"], $this->data["roid"] );
            // Qbi:
            if ($this->html->viewMode > PaXML_MIDVIEW) echo "<font color=gray>DEBUG: circularReference: create temporary object</font><br>\n";
            $this->data["update"]  = array("table" => $this->data["table"],
                                          "wiid" => $this->data["wiid"],
                                          "roid" => $this->data["roid"]);
        }
        
        if (!$foundObject)
        {
            // object not found
            $this->html->foundObject(false, null, $this->data["table"]);
            $this->reader->startObjectImport();
        }
        else
        {
			// bo found => do not import the internal elements except class
            // and system but keep id for possible parents objects as foreign
            // key value
            $this->html->foundObject(true, $foundObject, $this->data["table"]);

            $this->data["import"]  = false;
            $this->data["id"]      = $foundObject;
            $this->data["marker"]  = 1;                             // ???

            // special case: an root PROJ has been found, but will not be
            // linked (only linking possible) to possible root proj,
            // because tag 'parameterlist' will not be executed.
            // tag 'parameterlist' handles this linking for subprojects
            // which were created. to link found projects exec this code
            if ($this->stack->getSize() == 1)
            {
                // prepare for aligning to root proj
                $data["PROJ_ID"]    = $this->reader->rootProject;
                $data["TABLE_NAME"] = $this->data["table"];         // is it verified ?
                $data["PRIM_KEY"]   = $foundObject;

                // is it already linked to it ?
                if ($this->reader->checkForElementInProject($data) != null)
                    $this->html->alreadyLinked();
                else
                    $this->reader->alignToProject($this->data["table"], $foundObject);
            }

            // special doing if this is a contact object
            if ($attribs["TYPE"] == "CONTACT")						                // the current bo is contact so
                $this->reader->alignWiidToContact($foundObject, $attribs["WIID"]);	// try to align an wiid with contact
			
			if ( $this->reader->options["update"] ) { // do update ???				
				$this->reader->updateObject($this->data);	
			}
        }
    }
    
    
    
    // handle tag </attribute>
    //
    // input:   resource    $parser         handler of parser
    // output:  -
    //
    function closeOBJECT($parser)
    {
        $bo_table = $this->data["table"];				// save current tablename
        $bo_id    = $this->data["id"];					// save current id
        $bo_field = $this->data["field"];
        $update   = $this->data["update"];
        $objDone  = $this->data["objDone"];             // check for deprecated
        $displayed = $this->data["displayed"];

        $this->data = $this->stack->pop('closeOBJECT');      		// ... and restore bo data (biz)
		

        // about if($update)
        // $update is an array containing informations about an object, which
        // exists temporarly. at that moment, when the real object exist,
        // the fields referencing to this temp object have to be up to date.
        // therefore updateList is made.    
        
                   
        $this->html->endObject($displayed);
        
        // no support for A_SUBST_COHER, C_SUBST_COHER anymore ...
        
        if (   ($this->data["type"] == "parametersElement")
        	|| ($this->data["type"] == "listingElement") )
		{
			
			$colName = $bo_field!=NULL ? $bo_field : PrimNameGet2($bo_table); // generic column name or other name ?
			
           	array_push($this->data["params"], array("name"  => $colName,
												    "value" => $bo_id,
												    "type"  => null));
            if ($update)
            {
                $update["update"] = array("table" =>  $this->data["table"],
                                          "field" =>  $colName);
                $this->data["identifyYourself"] = $update;
                $this->reader->updateList[]     = $update;
            }
        }
        elseif ($bo_field != null && is_array($this->data["params"]))
        {
            array_push($this->data["params"], array("name"  => $bo_field,
                                                    "value" => $bo_id,
                                                    "type"  => null));
            if ($update)
            {   

                $update["update"] = array("table" =>  $this->data["table"],
                                          "field" =>  $bo_field);
                $this->data["identifyYourself"] = $update;
                $this->reader->updateList[]             = $update;
            }
        }

        // maybe commit transaction
        if ($this->stack->getSize() == 0 || $this->data["table"] == "PROJ")
            $this->reader->endObjectImport();
    }




    // handle tag <option>. special thing with option. the xml structure may
    // have two different variants:
    //
    //  <option ... />
    //
    //  <option ...><parameters>...</parameters><elements>...</elements></option>
    //
    // Both variants describe an option completely. in first variant the import
    // has to/can only be done with closing option tag. in second variant the
    // import has to be done with close parameters tag.
    //
    // input:   resource    $parser         handler of parser
    //          array       $attribs        tag attributes
    // output:  -
    //
    function openOPTION($parser, $attribs)
    {
        $attribs["TYPE"]      = strtoupper($attribs["TYPE"]);
		
        $this->stack->push($this->data, 'openOPTION');	
		
		$mainNameCol=importantNameGet2($attribs["TYPE"]); // e.g. for DB_USER
  		if ($mainNameCol=='') $mainNameCol='NAME'; // fallback
  								      
  		// save previous object and create own extra class

        $this->data		      = array();
        $this->data["type"]   = "optionElement";  	          // type of object structure
        $this->data["table"]  = $attribs["TYPE"];             // system table name
        $this->data["name"]   = $attribs["NAME"];       // name and identifier
		$this->data["params"] = array();		        	  // parameters
  		$this->data["import"] = true;

        // verify input
        $this->reader->verifyOption($this->data["table"]);

        $foundOption = $this->reader->findOption($this->data["table"], $this->data["name"]);
		$this->html->foundOption($this->data["table"], $this->data["name"], $foundOption);
        if ($foundOption)
        {
            $this->data["id"]     = $result;
            $this->data["import"] = false;
        }
        else
            $this->reader->startOptionImport();
    }
    
    
    
    // handle tag </option>
    //
    // input:   resource    $parser         handler of parser
    // output:  -
    //
    function closeOption()
    {
        // previously imported? if the id exist, then it's already done.
        if ($this->data["id"] == null and $this->data["import"])
            $this->importOption();

        $this->reader->endOptionImport();
        $this->data = $this->stack->pop('closeOption');
    }



    // import option data. this has to be made in external function, because it
    // may used twice. once from endOPTION, but also form endPARAMETERS.
    //
    // input:   -
    // output:  -
    // error:   -
    //
    function importOption()
    {
		if ($this->data["type"] == "optionElement" && $this->data["import"])
		    $this->data["id"] = $this->reader->importOption($this->data);
    }



    // this tag can be related between two object tags or two options tags.
    //
    // ...
    //  <object type=... wiid=... roid=... creation_date=... attached=...>
    //   <parameters class=...>
    //    ...
    //   </parameters>
    //   ...
    //  </object>
    // ...
    //
    // or
    //
    // ...
    //  <option type=... name=...>
    //   <parameters class=...>
    //    ...
    //   </parameters>
    //   ...
    //  </option>
    // ...
    //
    // input:   array       $attrib         xml attributes
    // output:  -
    // errors:  -
    //
    function openPARAMETERS($parser, $attribs)
    {
        // this tag must be inside business object structure and import
        // state must be true
		
        if ($this->data["type"] == "objectElement" && $this->data["import"])
		{
			// keep data of previous object
			$bo_table            = $this->data["table"];			// current business object table
			$parent              = ($this->data["type"] == "objectElement") ? $this->data["import"] : true;
			$bo_id               = $this->data["id"];				// current business object id
			$pid	             = $this->data["pid"];				// parent object id
			$ptable              = $this->data["ptable"];			// parent object table
            $specialID           = $this->data["specialPID"];
            $specialTABLE        = $this->data["specialPTABLE"];
            
			$this->stack->push($this->data,'openPARAMETERS');			             // save prevoius structure

            $this->data		      = array();			             // create new business object structure
			$this->data["type"]   = "parametersElement";
			$this->data["table"]  = $bo_table;
			$this->data["class"]  = $attribs["CLASS"];
			$this->data["params"] = array();
			$this->data["import"] = $parent;

            // set special parameters (rules exception) ...
            // > ... parent is PROJ_HAS_ELEM and it's related PROJ id is the
            //       parent of current PROJ. the current is a link. this linked
            //       PROJ has no parent, because the link was resolved at export
            //       without taking parent PROJ. so the parent PROJ of this
            //       linked PROJ has to be made to the real parent of this PROJ.
            //       Note, this is a special case.
            //        <object type=PROJ>...
            //         <elements type=PROJ_HAS_ELEM><element>
            //          <object type=PROJ>...</object>
            //         </element></elements>
            //        </object>
            // > ... the parent is not a PROJ but current object is PROJ, so set
            //       root project as parent project
            //        <partisanxml><object type=PROJ>...</object></partisanxml>
            // > ... parent is PROJ and current object is PROJ
            //        <object type=PROJ><object type=PROJ>...</object></object>
            // > ... parent is abstract substance and current object is coherency
            // > ... parent is wafer and current object is wafer step
            // > ... any other case
            if ($specialID != null && $specialTABLE == "PROJ_HAS_ELEM" && $bo_table == "PROJ")
                array_push($this->data["params"], array("name"  => "PRO_PROJ_ID",
														"value" => $specialID,
														"type"  => null));
			elseif ($ptable != "PROJ" && $bo_table == "PROJ" && $this->reader->rootProject > -1)
                array_push($this->data["params"], array("name"  => "PRO_PROJ_ID",
														"value" => $this->reader->rootProject,
														"type"  => null));
            elseif ($pid != null && $ptable == "PROJ" && $bo_table == "PROJ")
                array_push($this->data["params"], array("name"  => "PRO_PROJ_ID",
														"value" => $pid,
														"type"  => null));
            elseif ($bo_id != null)
                array_push($this->data["params"], array("name"  => PrimNameGet2($bo_table),
														"value" => $bo_id,
														"type"  => null));
           
        }

        // this tag must be inside option structure and import state must be
        // true
        if ($this->data["type"] == "optionElement" && $this->data["import"])
            $this->data["class"]  = $attribs["CLASS"];
    }



    // all parameter values are collected. import now for object or import now
    // for option.
    //
    // input:   -
    // output:  -
    // errors:  -
    //
    function closePARAMETERS($parser)
    {
        

        // context: object
        // <object ...><parameters>...</parameters></object>
		
        if ($this->data["type"] == "parametersElement") {
			if( $this->data["import"] ) {
			
				$savedata   = $this->data;					// save bo params (bizparam) ...
				$this->data = $this->stack->pop('closePARAMETERS:1'); 			// ... and restore bo data (biz)
	
				// get data from parent structure (object)
				$data               = array();					// prepare to save cct_access
				$data["wiid"] 	    = $this->reader->getWiid($this->data["wiid"]);
				$wiid               = $this->data["wiid"];
				$data["roid"]       = $this->data["roid"];
				$roid               = $this->data["roid"];
				$filename           = $this->data["file"];
				$data["crea_date"]  = $this->data["crea"];
				$data["mod_date"]   = $this->data["mod"];
				$data["db_user_id"] = $_SESSION['sec']['db_user_id'];
				$base               = $this->data["base"];
				$table		   		= $this->data["table"];
				$data["table_name"] = $this->data["table"];
				
				if ($this->reader->options["asarchive"]) {
					// get user_ids
					$data["db_user_id"]      = $this->lib->getUserID($this->data["creator"]);
					if ($this->data["modifier"]) $data["db__db_user_id"]  = $this->lib->getUserID($this->data["modifier"]);
				}
	
				$cctAccessID = $this->reader->importCCTAccess($data);
	
						// to be sure, that it was written
				if ($cctAccessID == null || $data["wiid"] == null)        // changed hier && -> ||
					$this->stopXMLProcessing("Database writing error with "
											. (($data["wiid"] == null) ? "wiid" : "access")
											. ". Import aborted.");
	
				$this->stack->push($this->data,'closePARAMETERS');	// save current bo data (biz) back
				$this->data = $savedata;			// and set bo params to current
	
				if (!($bo_id = $this->reader->importObject($table, $this->data, $cctAccessID, $wiid, $roid, $base, $filename)))
					$this->stopXMLProcessing("Fatal database writing error with. import aborted.");
	
				$this->data          = $this->stack->pop('closePARAMETERS:2');	 // back to business object (biz)
				$this->data["id"]    = $bo_id;                  // tell object structure the id
	
				// $this->data["table"] = $table;
				// check, if bo directly behind root in structure tree (root as parent) so do this
				// except for projects, because they have pro_project_id
				if ($this->stack->getSize() == 1 && $table != "PROJ")
					$this->reader->alignToProject($this->data["table"], $this->data["id"]);
			

				
			}
		}
		
		// be aware, this element PARAMETERS may also be exists inside OPTION
		// tags. next function has a internal condition to check if this
		// parameters tag was inside object or inside option
		$this->importOption();
    }



    // <parameter>
    // event handler for parameter, which is used in a lot of contexts. save
    // parameter name here.
    //
    // input:   array       $attribs        attributes
    // output:  -
    // error:   -
    //
    function openPARAMETER($parser, $attribs) {
		if ( $this->data["type"] == "classElement"
			|| $this->data["type"] == "attributeElement"
			|| $this->data["type"] == "optionElement"
			|| $this->data["type"] == "listingElement"
			|| $this->data["type"] == "parametersElement") {
			    
            // maybe it's to hard to crash here. maybe a warning is enough
            if  ($attribs["TYPE"] != "class"
                and $attribs["TYPE"] != "option"
                and $attribs["TYPE"] != "vario"
                and $attribs["TYPE"] != null)
                $this->stopXMLProcessing("Semantic PaXML error at parameter tag. Given attribute value 'type' for tag 'parameter' is not valid. Import aborted.");

            
            if ($attribs["TYPE"] == "vario") {
                //$FUNCNAME= __CLASS__.':'.__FUNCTION__;
                //echo "DEBUG_XX:".$FUNCNAME.':'.print_r($attribs,1)."<br>\n";
            }
                
            // $this->html->startParameter($attribs);
            $this->data["params"][] = array("name"  => $attribs["NAME"],
									        "value" => null,
									        "type"  => $attribs["TYPE"]);
           
        }
    }



    // </parameter>
    // event handler for parameter, which is used in a lot of contexts. save
    // parameter value now
    //
    // input:   -
    // output:  -
    // error:   -
    //
    function closePARAMETER($parser)
    {
		if ($this->data["type"] == "optionElement"
			|| $this->data["type"] == "classElement"
			|| $this->data["type"] == "attributeElement"
			|| $this->data["type"] == "parametersElement"
			|| $this->data["type"] == "listingElement")
		{

            $param = array_pop($this->data["params"]);
            
            if ($param["type"]=='vario') {
                $this->data["vario"][]=array(
                    "name"  => $param["name"],
                    "value" => $this->tagData 
                );
                return;
            }
            
		    $this->data["params"][] = array("name"  => $param["name"],
                                            "value" => $this->tagData,
                                            "type"  => $param["type"]);
        }
    }
    
    
    
    // elements. associative table and extended data tables are handled here.
    //
    // ...
    //  <object ...>
    //   <parameters>...</parameters>
    //   <elements type=...>
    //    <element>...</element>
    //    ...
    //   </elements>
    //   ...
    //  </object>
    // ...
    //
    // or
    //
    // ...
    //  <option ...>
    //   <parameters>...</parameters>
    //   <elements type=...>
    //    <element>...</element>
    //    ...
    //   </elements>
    //   ...
    //  </option>
    // ...
    //
    // input:   array       $attrib         xml attributes
    // output:  -
    // errors:  -
    //
    function openELEMENTS($parser, $attribs)
    {
        // upper structure has to be from tag object or from tag option
		$doDataImport = 0;
		$doUpdate     = 0; // needed for ABSTRACT_PROTO_STEP
		if (!$this->data["import"]) {
			if ($this->data["table"] == "PROJ") $doDataImport = 1;
			if ($this->data["table"] == "ABSTRACT_PROTO") {
				$doDataImport = 1;
				$doUpdate = 1;
				
				if ($this->_search_import_start ) {
					$doDataImport = 0;
				}
			}
		}
		
		
		
        if ((($this->data["type"] == "objectElement" or $this->data["type"] == "optionElement") && $this->data["import"])
            || $doDataImport )
		{
            $bo_id                = $this->data["id"];				 // get bo_id of related bo
            $bo_table             = $this->data["table"];
            $ttype                = $this->data["type"];

            $this->stack->push($this->data,'openELEMENTS');			             // save previous object ...

            // ... and add it as normal parameter
            $this->data		      = array();
            $this->data["type"]   = "listingElement";
            $this->data["table"]  = $attribs["TYPE"];
			$this->data["import"] = true;
			if ($doUpdate) $this->data["update"] = true;
			$this->data["params"] = array();
			

            $this->html->startElements($this->data["table"]);

            // verfiy input
            $this->reader->verifyElements($this->data["table"]);

			// bugfix: ... == "option") --> ..== "optionElement")
            if ($ttype == "optionElement")
                $this->data["dbHandle"] = "option";
				
            // parent is a special internal type for a parameter which means
            // the foreign key to parent table. this was introduced because of
            // PROJ_HAS_ELEM could have PROJ as parent table and as element. so
            // it happend that PROJ_ID was twice in sstructure.
            $colname = $this->_getPrimKey($bo_table);
            array_push($this->data["params"], array("name"  => $colname,
									                "value" => $bo_id,
									                "type"  => "PARENT"));
        }
    }



    // elements.
    //
    // input:   array       $attrib         xml attributes
    // output:  -
    // errors:  -
    //
    function closeElements()
    {
        if ($this->data["type"] == "listingElement")
        {
            $this->html->endElements();
            $this->data = $this->stack->pop('closeElements');
        }
    }
    
    
    
    // handle a single entry of an associatve or extended data table
    //
    // input:   array       $attrib         xml attributes
    // output:  -
    // errors:  -
    //
    function openELEMENT($parser, $attribs)
    {
		
        if ($this->data["type"] == "listingElement" and $this->data["import"])
        {
			$this->html->startElement($this->data["table"]);
            $this->data["class"] = $attribs["CLASS"];
			if ( $attribs["FILE"]!="" ) $this->data["file"] = $attribs["FILE"];
        }
    }



    // handle a single entry of an associatve or extended data table
    //
    // input:   array       $attrib         xml attributes
    // output:  -
    // errors:  -
    //
    function closeELEMENT($parser)
    {
    	$dataimport = $this->data["import"];
        if (($this->data["type"] == "listingElement" && $dataimport)
            || (strtoupper($this->data["table"]) == "PROJ_HAS_ELEM"))
		{
			
			
            $this->reader->importElement($this->data);
		
            // if subobjects is a temp object only, the link must be updated
            // later. to know which link, write yourself-identification into
            // updatelist

            // get parent business objects ...
            $this->html->endElement();
            $savedata   = $this->data;
            $this->data = $this->stack->pop('closeELEMENT');
            $bo_id 	    = $this->data["id"];
            $bo_table   = $this->data["table"];
            $this->stack->push($this->data, 'closeELEMENT');
            $this->data = $savedata;

    		// set up new structure as it would be new
            $botable_pk_name = PrimNameGet2($bo_table);
            $this->data["class"]  = null;
            $this->data["params"] = array();
			$pkname = $botable_pk_name;
            array_push($this->data["params"], array("name"  => $pkname,
													   "value" => $bo_id,
													   "type"  => "PARENT"));
		}
		
    }

}


// end.
?>
