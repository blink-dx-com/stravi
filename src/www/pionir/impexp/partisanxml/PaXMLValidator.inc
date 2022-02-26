<?php

// PaXMLValidator

// validate xml content against Database content. there are two levels
// level 1:	
//		- check if object (wiid, roid) exists
//		- check for number of associated elements
//		- check if extra class exists (table_name, name)
//		- check if h-tables exists (name)
// level 2:
//		- all checks from level 1
//		- check if all fields are equal
//
//	2003-02-03	rogo	start
//

// set here the software version of xml reader
define("PaXML_VALIDATORVERSION", "0.1");


// inlude section
require_once("PaXMLValidatorOutput.inc");      // contains class for handling screen output
require_once("PaXMLFileInput.inc");             // sax parser class
require_once("PaXMLLib.inc");                   // general functions
require_once('o.LINK.subs.inc');        // used for linkpath_get()
require_once('glob.image.inc');                 // used for imgFullPath()
require_once('get_cache.inc');                  // used for varcol cache
require_once('access_check.inc');               // used for object access checks
require_once('role.inc');                       // used for role right checks



// constants PaXML_XMLVERSION and PaXML_SELFURL have to be defined! usually this
// should be done PXMLLib.inc
if (!defined("PaXML_XMLVERSION"))
    die("Internal error. Constant 'PaXML_XMLVERSION' is not defined. Please contact your administrator.");
if (!defined("PaXML_SELFURL"))
    die("Internal error. Constant 'PaXML_SELFURL' is not defined. Please contact your administrator.");



class PaXMLValidator
{
	var $html;	
	var $rootProject;
    var $user;                  // db username
    var $passwd;                // db password
    var $db;                    // db
    var $dbtype;                // db type
    var $mode;                  // viewmode for PaXMLHTMLOutputImport(). viewmodes
                                // are defined in PaXMLHTMLOutput.inc
	
	

	function __construct($user, $passwd, $db, $dbtype, $mode = PaXML_MINVIEW)
	{		
        $this->user   = $user;
        $this->passwd = $passwd;
        $this->db     = $db;
        $this->dbtype = $dbtype;
		
		$this->html = new PaXMLValidatorOutput($mode);
	}
	
	
	
	// 
	function &getHTML()
	{
		return $this->html;
	}



	// dummy
	function rollbackAll()
	{}



    // init database connections. create three connections, which are not shared
    // to have completely separated transactions between them.
    //
    // input:   string      $user       username
    //          string      $passwd     password
    //          string      $db         database name
    //          string      $dbtype     db type (maybe deprecated)
    // output:  -
    //
    function initDatabase($user, $passwd, $db, $dbtype = "CDB_OCI8")
    {
        $this->html->connectToDatabase();
        $this->dbObject = &logon_to_db($user, $passwd, $db, $dbtype, "new");
        $this->dbOption = &logon_to_db($user, $passwd, $db, $dbtype, "new");
        $this->dbClass  = &logon_to_db($user, $passwd, $db, $dbtype, "new");
        
        if ($this->dbObject == null || $this->dbOption == null || $this->dbClass == null)
            $this->html->stop("Connecting to database failed.", "user: $user; db: $db; dbtype: $dbtype");

        $this->html->connectedToDatabase();
    }	
	
	
	
    // check for access rights and role rights related to paxml. function stops
    // hardcore if a right is missing.
    //
    // input:   -
    // output:  -
    //
    function checkPermissions()
    {
        

        // check access rights for the root project. insertright needed for adding
        // new elements to project
        if ($this->rootProject > 0)
        {
            $accessRight = access_check($this->dbObject, "PROJ",  $this->rootProject);
            if (!($accessRight["insert"] > 0))
            {
                $this->html->stop("You have no permission to add elements to selected project. "
                                . "Import aborted.", "checkPermissions()");
                exit;
            }
        }

        // role check. root has any right.
        $roleRightName = "f.PaXml_import";
        $roleRight = role_check($this->dbObject, $roleRightName);
        if (!($roleRight["execute"] > 0) and $_SESSION['sec']['appuser'] != "root")
        {
            $this->html->stop("You have no permission to execute this function. You need "
                            . "role right '$roleRightName' to use paxml import functionality.",
                              "checkPermissions()");
            exit;
        }
    }



    // init uploaded file. uploaded file maybe a xml, a gzipped or a tar file.
    // determine filetype and handle this. a tar file has to be extracted first.
    // a gzipped file has to be uncompressed and a xml file is the wanted result.
    // of course, a tar may contain a gzipped file.
    //
    // input:   string  $tmpFilename        name of uploaded file as it is in at server side
    //          string  $orgFilename        filename at client side
    // output:  the name of xml file in temp directory
    //
    function initFile($tmpFilename, $orgFilename)
    {
        // detecting file type
        $this->html->detectFiletype($orgFilename);
        switch($filetype = $this->lib->getFileType($tmpFilename, $orgFilename))
        {
        case "tar" : $tarFilename = $tmpFilename; break;
        case "zip" : $zipFilename = $tmpFilename; break;
        case "xml" : $xmlFilename = $tmpFilename; break;
        default    : $this->stop("Unknown file type of uploaded file. It's neither "
                               . "a tar file nor a gzip compressed file nor xml file. Execution stopped.",
                                 "initFile()");
        }
        $this->html->detectedFiletype($filetype);

        // get import path. import path is a sub directory in the current work
        // path (temp directory). the sub directory contains the extracted tar
        // file potentially
        $importPath = $this->initWorkPath();
        // $this->importPath = $importPath;
        // handle tar file. unpack and get one of two possible results, zip file
        // or xml file. save importpath, because it's possible, that there are
        // some data files
        if ($tarFilename != null)
    	{
            $this->html->untarFile($tarFilename);
            $tarFiles         = $this->lib->untar($tarFilename, $importPath);
            $zipFilename      = $tarFiles["zip"];
            $xmlFilename      = $tarFiles["xml"];
            $this->importPath = $importPath;
            $this->html->untaredFile();
        }

        // handle zip file. single result of a zip should be xml file
        if ($zipFilename != null)
    	{
            $this->html->uncompressFile($zipFilename);
            $zipFiles         = $this->lib->uncompress($zipFilename, $importPath);
            $xmlFilename      = $zipFiles["xml"];
            $this->html->uncompressedFile();
        }

        return $xmlFilename;
    }
    
    
    
    // prepare work path. work path is a sub directory in the current work
    // path defined in globals. the work path is session-wide-unique. if the
    // work path already exists, it will be removed before.
    //
    // input:   -
    // output:  the name of a temp directory
    //
    function initWorkPath()
    {
        // handling workpath
        $this->html->createWorkPath();
        if (($workPath = $this->lib->getWorkPath()) == null)
            $this->stop("Misconfigured application settings. Missing work path. "
                      . "Please contact your administrator.", "initWorkPath()");

        // add '/' to path if necessary
        $workPath   = (substr($workPath, -1) != "/") ? $workPath . "/" : $workPath;

        // set a sub dir in temp files path ...
        $importPath = $workPath . "pxmlimport." . session_id();

        // if subdirectory not exists, delete subdirectory including all subs of
        // subsdirectory ...
        if (file_exists($importPath))
            if (!$this->lib->removeDirRecursivly($importPath))
                $this->stop("Could not prepare working directory for handling "
                          . "uploaded file. Working path already exists and "
                          . "is not removable. The execution is blocked.",
                            "initWorkPath(); session id: " . session_id());
                          
        // create workpath
        if (!mkdir($importPath))
                $this->stop("Could not create working directory for handling "
                          . "uploaded file. Execution stopped.",
                            "initWorkPath(); session id: " . session_id());
        $this->html->createdWorkPath($importPath);

        return $importPath;
    }



    // ermergency stop. call this function from inside this class to stop
    // execution. a secure stop will be done
    //
    // input:   string      message         your last three words before dieing
    // output:  -
    //
    function stop($message, $secret = null)
    {
        if ($this->paxml == null)
            $this->html->stop($message, $secret);
        else
            $this->paxml->stopXMLProcessing($message, $secret);
    }




	function start($tmpFilename, $orgFilename, $rootProj)
	{
		$this->html->start(PaXML_XMLVERSION, PaXML_VALIDATORVERSION);
		
        // init paxml library
        $this->lib = new PAXMLLib($this->html, $this->user, $this->passwd, $this->db, $this->dbtype, $this);

        // with this xml version some new features had been added, which require
        // a updated cct_table entry. checking here for it.
        if (!$this->lib->checkForCorrectVersion())
                $this->stop("Missing updated CCT_TABLE. Inform your admin.", "start()");

        // init database connections ...
        $this->initDatabase($this->user, $this->passwd, $this->db, $this->dbtype);

        // ... and check permissions.
        $this->checkPermissions();

        // sett root project. check if given project exist. else abort here
        $this->rootProject = $rootProj;
        if ($this->rootProject != -1)
            if ($this->lib->existsProject($this->rootProject) ===  null)
                $this->stop("Given project doesn't exist.", "start(); project: $rootProj");
		
        // init files, means to transform any type of input file to a xml file
        // and a import path. possible infput files may be xml, gz or tar
        if (!($this->xmlFilename = $this->initFile($tmpFilename, $orgFilename)))
 			$this->stop("No paxml file found. <!-- session id: " . session_id() . " //-->");
		
		// now start the parser: 5, 4, 3, 2, 1, go ... and he starts, yeah ...
        $this->paxml = new PaXMLFileInput($this, $this->xmlFilename);
        $this->paxml->setValidateMode();
		$this->html->initXMLParser();
        $this->paxml->init();
        $this->html->parse();
        $this->paxml->parse();
        $this->html->finish($this->importPath);
	}


    // finds a class by given class name and table name
    //
    // input:   string      $name       classname
    //          string      $table      tablename
    // output:  -
    //
    function findClass($name, $table)
    {
        if ($name == null or $table == null or !is_string($name) or !is_string($table))
            $this->stop("Illegal parameters to function findClass(). Import aborted.", "findClass()");

  		$stmt = "SELECT extra_class_id FROM extra_class "
              . "WHERE name = '" . $name . "' AND table_name = '" . $table . "'";

        $this->html->statement($stmt);
		if ($this->dbClass->Query($stmt) != 1)
			$this->stop("Database read failed. Import aborted. <!-- findClass() //-->");

		if ($this->dbClass->ReadRow())
			return $this->dbClass->RowData[0];

		return null;
    }



    // validating existing class. existing class must have all attributes, which
    // may be imported, if class would not exist.
    // a imported class must have fewer or same number of attributes than the
    // existing one and the attributes have to match against each other.
    // the matching algorithm compares name, table_name, datatype and app_data_type.
    // name and table_name are important for identification. datatype
    // and app_data_type are important to be identical for functionality. this
    // solution has been found in a joined work with piet.
    //
    // the execution stops directly, if a class is not valid.
    //
    // name and table_name are already checked in a way, that this are the
    // criterias to find a class
    //
    // input:   array       $classDescription       have a look at PaXMLFileInput.inc
    // output:  -
    //
    function validateClass($classDescription)
    {
        // if the class tag was only a reference (without attributes), don't
        // do any checks, because it has already be done at startClass().
        if ($classDescription["attribs"] == null)
            return;

        // foreach attribute create a statement, to query for attribute
        foreach($classDescription["attribs"] as $attribute)
        {
            $data = array();
            
            // get values for DATATYPE and APP_DATA_TYPE
            if ($attribute["params"] != null and is_array($classDescription["params"]))  // are there some parameters ?
                foreach($attribute["params"] as $param)	                           // add them
                    if($param["name"] == "DATATYPE" or $param["name"] == "APP_DATA_TYPE")
                        $data[$param["name"]] = $param["value"];

            // it's a required field
            if ($data["APP_DATA_TYPE"] == null)
                $this->stop("Semantic Given value for "
                          . "parameter 'app_data_type' is not valid. Import aborted.", "validateClass()");

            // check for valid value
            //if (!(strtoupper($data["DATATYPE"]) == strtoupper("string_val")
            //    or strtoupper($data["DATATYPE"]) == strtoupper("num_val")
            //    or strtoupper($data["DATATYPE"]) == strtoupper("bin_val")
            //    or strtoupper($data["DATATYPE"]) == strtoupper("time_val")))
            //    $this->stop("Semantic PaXML error at attribute tag. Given value for "
            //                        . "parameter 'datatype' is not valid. Import aborted.", "validateClass()");

            // now create sql string to ask for attribute
            $stmt = "SELECT ea.extra_attrib_id FROM extra_attrib ea, app_data_type adt "
                  . "WHERE ea.app_data_type_id = adt.app_data_type_id "
                  . "AND ea.name = '" . $attribute["name"] . "' "
                  . "AND ea.extra_class_id = '" . $classDescription["foundClass"] . "' "
            //      . "AND ea.datatype = '" . $data["DATATYPE"] . "' "
                  . "AND adt.name = '" . $data["APP_DATA_TYPE"] . "'";

            $this->html->statement($stmt);
            if ($this->dbClass->Query($stmt) != 1)
                $this->stop("Database query failed. Import abort. ", "validateClass(); $stmt");

            // query or result failed, means class verification failed
            if (!$this->dbClass->ReadRow())
                $this->stop("Conflict (1). The class " . strtolower($classDescription["table"])
                          . "." . $classDescription["name"] . " found in xml doesn't "
                          . "have the a compatible structure like the class "
                          . "in database with same name. Problems with "
                          . "attribute '" . $attribute["name"] . "'. An "
                          . "attribute, describe in paxml doesn't exist "
                          . "in database class structure. Import abort.", "validateClass()");

            // :( not found!
            if ($this->dbClass->RowData[0] == null)
                $this->stop("Conflict (2). The class " . strtolower($classDescription["table"])
                          . "." . $classDescription["name"] . " found in xml doesn't "
                          . "have the a compatible structure like the "
                          . "class in database with same name. Problems "
                          . "with attribute '" . $attribute["name"] . "'. "
                          . "An attribute, describe in paxml doesn't exist "
                          . "in database class structure. Import abort.", "validateClass()");
        }

        return;
    }



    // check if linked table type is a valid value for having extra class.
    //
    // input:   string      $table      table
    // output:  true | false
    //
    function verifyClass($table)
    {
        $tabletype = $this->lib->getTableType($table);
        if (!($tabletype === PaXML_OBJECT or $tabletype === PaXML_OPTION or $tabletype === PaXML_EXTEND or $tabletype === PaXML_ASSO))
            $this->stop("Semantic paxml error at tag 'class'. Given value <!-- "
                      . $table ." //--> for attribute 'type' is not valid. Import aborted.", "verifyClass()");
    }
	
	
	
    // dummy
    function importClass($classDescription)
    {}
	
	
	
    // check if linked object type is a valid value
    //
    // input:   string      $table      table
    // output:  true | false
    //
    function verifyObject($table)
    {
        if (!(PaXML_OBJECT === $this->lib->getTableType($table)))
            $this->stop("Semantic PaXML error at object tag. Given value for attribute "
                      . "'type' is not valid. Import aborted.", "verifyObject()");
    }
    
	
	
    // dummy
    function importCCTAccess($data)
    {
         return 1;
    }
    
    

	// dummy
    function importObject($table, $objectDescription, $cctAccessID, $wiid, $roid, $base, $filename)
    {
		return 1;
	}



    // check if linked object type is a valid value
    //
    // input:   string      $type      table
    //          string      $wiidname  wiid name (string)
    //          string      $roid      roid
    // output:  object id
    //
    function findObject($type, $wiidname, $roid)
    {
        // check input parameters
        if ($type == null or $wiidname == null or $roid == null  or !is_string($type) or !is_string($wiidname) or !is_string($roid))
            $this->stop("Illegal parameters to findObject(). Import aborted.", "findObject()");

        // try to get wiid id by wiidname. if wiidname not existing in db, a new
        // dataset is created
        if (($wiid = $this->getWiid($wiidname)) == null)
            $this->stop("Couldn't get wiid for $wiidname. Import aborted.", "findObject()");

		// if you want to find your own data, search also for wiid == null
		if ($wiidname == "http://www.clondiag.com/magasin/?db=" .  $this->lib->getDBSerial())
			$x = "((ca.wiid = $wiid AND ca.roid = $roid) OR (ca.wiid IS NULL AND bo." . $type . "_id = $roid))";
		else
			$x = "ca.wiid = $wiid AND ca.roid = $roid";

		$stmt = "SELECT " . $type . "_id FROM $type bo, cct_access ca ";
		$stmt .= "WHERE ca.cct_access_id = bo.cct_access_id AND " . $x;

	    $this->html->statement($stmt);
		if ($this->dbObject->Query($stmt) != 1)
			$this->stop("Fatal database reading error. Could not select objects. Import aborted.", "findObject()");

        // get object id
		if ($this->dbObject->ReadRow())
			return $this->dbObject->RowData[0];

		return null;
    }
    

    
    // dummy
    function startObjectImport()
    {}



    // dummy
    function endObjectImport()
    {}




    // check for right option type
    //
    // input:   string      $optionTable     table
    // output:  -
    //
    function verifyOption($optionTable)
    {
        if (!(PaXML_OPTION === $this->lib->getTableType($optionTable)))
            $this->stop("Semantic PaXML error at option tag. Given "
                      . "value for attribute 'type' is not valid. Import aborted.", "verifyOption()");
    }
    
	
	
    // search for option entry in database.
    //
    // input:   -
    // output:  -
    //
    function findOption($type, $name)
    {
        if ($type == null or $name == null or !is_string($type) or !is_string($name))
            $this->stop("Development error. Illegal parameters to findOption(). Import aborted.", "findOption()");

		$stmt = "SELECT " . $type . "_id FROM " . $type . " ";
		$stmt .= "WHERE name = '" . $name . "'";

	    $this->html->statement($stmt);
		if ($this->dbOption->Query($stmt) != 1)
			$this->stop("Trying to find option entry. Can't read from database. Import aborted.", "findOption()");

		if ($this->dbOption->ReadRow())
			return $this->dbOption->RowData[0];

		return null;
    }



    // dummy
    function importOption($optionData)
    {
		return 1;
    }



    // dummy
    function startOptionImport()
    {}



    // dummy
    function endOptionImport()
    {}
	
	
    // check for right elements type
    //
    // input:   string      $table     table
    // output:  -
    //
    function verifyElements($table)
    {
        if (!(PaXML_ASSO === $this->lib->getTableType($table) or PaXML_EXTEND === $this->lib->getTableType($table)))
            $this->stop("Semantic PaXML error at elements tag. Given value for attribute 'type' is not valid. Import aborted.");
    }
    

    
    // dummy
    function importElement($elementDescription)
    {}

	
	// dummy
    function updateVarcolCache($updateCache)
	{
		return "";
	}

	
	
	
    // test if an element is alread in a project. returns only false if it is
    // not in project, else true
    //
    // input:   array       $data           array("PROJ_ID" => value,
    //                                            "TABLE_NAME" => value,
    //                                            "PRIM_KEY" => value)
    // output:  -
    //
    function checkForElementInProject($data)
    {
        if ($data == null or !is_array($data))
            return;

        // seems to be strange to say true if there seems to be an error at
        // parameters. but this function returns only false, when 'data not
        // existing' is a fact. this helps to say broken links in xml file from
        // source database will be handled, because higher level function,
        // believe, that this object has been already linked to proj.
        if ($data["PROJ_ID"] == null || $data["TABLE_NAME"] == null || $data["PRIM_KEY"] == null)
            return true;

        $stmt = "SELECT proj_id FROM PROJ_HAS_ELEM WHERE proj_id = " . $data["PROJ_ID"]
              . " AND table_name = '" . $data["TABLE_NAME"] . "'"
              . " AND prim_key = " . $data["PRIM_KEY"];

		$this->html->statement($stmt);
		if ($this->dbObject->Query($stmt) != 1)
            $this->stop("Database read error. Import aborted.", "checkForElementInProject()");
		if ($this->dbObject->ReadRow())
			return $this->dbObject->RowData[0];
    }



	// dummy
    function alignWiidToContact($bo_id, $base)
    {}


    // dummy
    function alignToProject($table, $id)
 	{}


    // dummy
    function getTempObject($name)
    {
		return 1;
	}



    // return wiid by wiid name, if no wiid exist for give base, then create
    // wiid without contact info, so that an object can allways have a link to
    // an entry in h_wiid
    //
    // NOTE!!! this sql statements have to be inside dbObject transaction, else
    // you may run in danger of deadlock's (aaaahhhhh took me one day to find
    // out this problem)
    //
    // input:   string      $base           wiid name
    // output:  string                      wiid id
    //
 	function getWiid($base)
 	{
        if ($base == null or !is_string($base))
 			$this->stop("Internal error. Wrong parameter input. Please contact your admin.", "getWiid()");

        if ($this->dbObject == null)
            $this->stop("Internal error. No database connection. Please contact your admin.", "getWiid()");

        // try to get a h_wiid entry for $base ...
        $stmt = "SELECT wiid FROM h_wiid WHERE name = '" . $base . "'";
		$this->html->statement($stmt);
	    if ($this->dbObject->Query($stmt) != 1)
            $this->stop("Database error.", "getWiid()");

        // ... and return if found ...
        if ($this->dbObject->ReadRow())
            return $this->dbObject->RowData[0];
			
		return 1;

		
    }

}


?>