<?php
/***
 * main code for reading paxml file and importing data
 * 
 * @package     PaXMLReader.inc
 * version:     0.5.5
 * xml version: 0.5
 * @swreq   UREQ:0000171: g.paxml-import > HOME UREQ 
 * @swreq   UREQ:0000663: g.paxml.import > update objects I > ABSTRACT_PROTO_STEP
 * @author  Rene Ogorsolka (Original Master)
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @history
   2001-07-09  rogo	started to write that
   2004-04-06  qbi  new version 0.5.5 due to forcing import_check for all BOs
 					     see openOBJECT() NEW:...
 
 */
//  $Header: trunk/src/www/pionir/impexp/partisanxml/PaXMLReader.inc 59 2018-11-21 09:04:09Z $


//  description:
//
//  this file contains the main class for import of paxml. this is the official
//  public interface. typical usage with an uploaded file:
//
//      require_once("PaXMLReader.inc");
//	    $pxml = new PaXMLReader($_SESSION['sec']['dbuser'], $passwd, $db, $_dbtype, $cct_output);
//      $pxml->start($cct_file, $FILES["cct_file"]["name"], $cct_proj);
//
//  this file contains all necessary functions for import of data to database.
//  this class is used by PaXMLFileInput. PaXMLFileInput contains the logic of
//  a parser.
//  in fact both files could be together, but have a look at the size of code
//  and you will why it's separated
//

//  features:   import object structure (including associative elements)
//              wiid/roid identification
//              import class data
//              import option data
//              supporting contact info of source database
//              import of projects with update of project elements
//              uncompressing of xml file to gzip format
//              import of images and documents (external files) NEW!
//              usage of tar packages NEW!
//              handling circular references (temporary objects) NEW!
//              fast sax parser for xml
//              supporting varcol cache update NEW!
//              supporting wafer import NEW!
//              usage of exim flag NEW! (only export)

//  devnotes:   versioning system.
//
//              there are three version number for the whole paxml system. one
//              is the paxml version, which reflects changes in xml structure.
//              the constant for this is called PaXML_XMLVERSION and located
//              in file PaXMLTags.inc. the other two versions are software
//              version reflecting changes due the php code, maybe bugfix or
//              system changes like extra_val removement. each component, the
//              reader and the writer has its own version number defined as
//              PaXML_READERVERSION and PaXML_WRITERVERSION. the software
//              version are half independent from xml version. it's not required
//              to increase xml version if php code changes only without changes
//              to xml structure. but usually if you change the xml structure
//              the script code will change.

//  devnotes:   handling of attachements.
//
//              at that moment when a file is uploaded, the file will be renamed
//              at the server and the filename is stored in the object. the
//              filename has a scheme to identify object to file.
//              at export the file is copied into exportpath. the file is renamed
//              again to be sure, to have different filenames. this server
//              unique name is saved in xml, so that allways the object and the
//              file are linked together.
//              during import the files will be copied into server directories
//              and the name will change again to the server internal scheme for
//              filenames.
//
//              good reaseans for switching the filenames are:
//              - conflict between filenames of two servers
//              - external paxml writer may not use the server file name scheme
//              - independency from any file naming system

//  devnotes:   transactions.
//
//              PaXMLReader uses three separated transactions. one transaction
//              is used only for import of extra class and extra attributes (not
//              extra values), one is used only for import of option data and
//              one is used for all object stuff. the object transaction may
//              get very big and contains any import action except those for
//              classes and options. Why three handles? class, option and object
//              can be handled independly. if the import of a class successfull,
//              there is no reason to wait until object has been imported
//              completly. Note, a commit (per transaction) will only be done,
//              if data was completly inserted.
//              transaction have drawbacks. depending on transaction level, they
//              may block execution of other users. second, in very bad cases a
//              dead lock may be programmed. be careful!
//

//  devnotes:   self reference resolution
//
//              now in pseudo code how handling for self references works.
//              generally a temporary object is generated and used until the
//              self reference can be solved.
//
//              grabbing object X
//              objectfield A is link to an object
//                  the linked object is object X (self reference)
//                  detecting self reference
//                      object T = create temporary object
//                  return object T
//              field A gets a link to object T
//              note, that object T has to be replaced for field A
//              import object X
//              replace link of field A with link to object X
//              delete object T




// set here the software version of xml reader
define("PaXML_READERVERSION", "0.5.6");	// 0.5.6 : NEW: allow extension of EXTRA_CLASS


// inlude section
require_once("PaXMLHTMLOutputImport.inc");      // contains class for handling screen output
require_once("PaXMLFileInput.inc");             // sax parser class
require_once("PaXMLLib.inc");                   // general functions
require_once('glob.image.inc');                 // used for imgFullPath()
require_once('get_cache.inc');                  // used for varcol cache
require_once('date_funcs.inc');  				// date functions
require_once('access_check.inc');               // used for object access checks
require_once('role.inc');                       // used for role right checks

require_once("o.SATTACH.subs.inc"); 
require_once('o.LINK.subs.inc');        		// used for linkpath_get()
require_once 'o.S_VARIO.mod.inc';


// constants PaXML_XMLVERSION and PaXML_SELFURL have to be defined! usually this
// should be done PXMLLib.inc
if (!defined("PaXML_XMLVERSION"))
    die("Internal error. Constant 'PaXML_XMLVERSION' is not defined. Please contact your administrator.");
if (!defined("PaXML_SELFURL"))
    die("Internal error. Constant 'PaXML_SELFURL' is not defined. Please contact your administrator.");



// class starts here
class PaXMLReader
{
    var $paxml;                 // handle for class PaXMLFileInput()
    var $html;                  // handle for class PaXMLHTMLOutputImport()
    var $lib;                   // handle for class PaXMLLib()
    var $dbObject;              // db handle for object action
    var $dbOption;              // db handle for option action
    var $dbClass;               // db handle for class action
    var $importPath;            // importpath is a sub path in 'workpath' for
                                // handling multiples files (extracted)
    var $rootProject;           // project to link the root objects
    var $user;                  // db username
    var $passwd;                // db password
    var $db;                    // db
    var $dbtype;                // db type
    var $mode;                  // viewmode for PaXMLHTMLOutputImport(). viewmodes
                                // are defined in PaXMLHTMLOutput.inc
    var $updateList;            // list with a complex structure. this list
                                // contains info, which data has to be updated
                                // and in which way. this list is used to solve
                                // the circular reference problem
    var $tempObjectList;        // list of temporary objects, created during
                                // import. these objects are needed to solve the
                                // circular reference problem.
	var $updatingCache;         // flag for update cache force
	
	/* 
	 * @var array $options
	 * "update" = // array of update-options
			 array (
			 		"files" : update file attachments
			 )
		"asarchive" = 0|1 import as archive
			- import user-IDs
			- activate "archOriID"
		"archOriID" = 0|1 - insert original W_WAFER_IDs
		"tempdir"   : use the "tempdir" (after WORK_PATH) instead of default path with SESSION_ID
			- e.g. "pxmlexport.76b7a67c38582b823e732fa6e141917e"
			- do NOT remove the files on this "tempdir"
		"isp" - [OPTIONAL] start import at project (in file)
		  array('ROID'=>, 'WIID'=> )
	*/
	var $options;				
	
    // constructor.
    //
    //
    function __construct($user, $passwd, $db, $dbtype, $mode = PaXML_MINVIEW, 
		$options = NULL
		) {
		
        $this->user   = $user;
        $this->passwd = $passwd;
        $this->db     = $db;
        $this->dbtype = $dbtype;
		$this->updatingCache  = 0;
		$this->options  = $options;
	
		if ($this->options["asarchive"]>0) {
			$this->options["archOriID"]=1;
		} 
        
        // init html output
        $this->html   = new PaXMLHTMLOutputImport($mode);
		$this->attachLib	    = new cSattachSubs();
    }



    // return html output handle
    //
    // input:   -
    // output:  handle of $this->html
    //
    function &getHTML()
    {
        return $this->html;
    }



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
        $this->dbObject = logon_to_db($user, $passwd, $db, $dbtype, "new"); //NEW: no ampersand
        $this->dbOption = logon_to_db($user, $passwd, $db, $dbtype, "new");
        $this->dbClass  = logon_to_db($user, $passwd, $db, $dbtype, "new");
        
        if ($this->dbObject == null || $this->dbOption == null || $this->dbClass == null)
            $this->html->stop("Connecting to database failed.", "user: $user; db: $db; dbtype: $dbtype");

        $this->html->connectedToDatabase();
    }
	
	function _LastErrDBWriteErr () {
		global $error;
		$error = &ErrorHandler::get();
		
		$errLast   = $error->getLast();
		$errorTxt  = "Database writing error: ".$errLast->text . $errLast->id;
		return ($errorTxt);
	}
		
	// descr:	
    //   - change $data: try to insert the ORIGINAL ROID as ID instead of AUTO-ID
	//   - check if ID exists!
	// output: &$data
	function boIDspecials( $table, &$data,  $wiid, $roid ) {
	
		if ($table!="W_WAFER" AND $table!="RESX" AND $table!="W_WAFER_GROUP") return 0;
		
		$pkname = PrimNameGet2($tablename); // // $pkname = $table."_ID";
		
		$newid  = $roid;
		$data[$pkname] = $newid;
		
		$db = &$this->dbObject;
		// check if object-ID exists
		$stmt = "select 1 from ".$table." where $pkname=".$newid;
		$db->Query($stmt);
		if ( $db->ReadRow() ) {
        	$existsid = $db->RowData[0];
			$this->stop("Archive import error. Original-ID ".$table.":".$newid." already exists.", "boIDspecials()");
		}
		return 0;
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
        if ( !file_exists( $tmpFilename ) ) {      
            $this->stop("File '$tmpFilename' not found.");
        }
        
        $this->html->detectFiletype($orgFilename);
        switch($filetype = $this->lib->getFileType($tmpFilename, $orgFilename))
        {
			case "tar" : $tarFilename = $tmpFilename; break;
			case "zip" : $zipFilename = $tmpFilename; break;
			case "xml" : $xmlFilename = $tmpFilename; break;
			default    : $this->stop("Unknown file type of uploaded file (".
					$orgFilename."). It's neither "
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
		
		if ($this->options["tempdir"]!="") { // if tempdir is given: use it!
			$this->importPath = $importPath;
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
    
    function getWorkPath() {
    	return $this->lib->getWorkPath();
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
        

        // if subdirectory not exists, delete subdirectory including all subs of
        // subsdirectory ...
		
		if ( $this->options["tempdir"]=="" ) {
			$importPath = $workPath . "pxmlimport." . session_id();
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
			
		} else {
			// use an existing importSession
			
			$importPath = $workPath . $this->options["tempdir"];
			$this->html->info("use existing work-path", $importPath, PaXML_TINYVIEW);
			if (!file_exists($importPath)) {
				$this->stop("Working path does not exists. The execution is stopped.",
								"initWorkPath(): " .$importPath);
			}
		}
		

        return $importPath;
    }

	// try to extend an EXTRA_CLASS
	function extendClass ( 
		$classDescription, 
		$attribute, 	// attribute structure 
		$clsAttribFeat	// attribute features: array ("NICE_NAME" => ..., "MAP_COL" => ...)
		) {
		 $FUNCNAME = "extendClass";
		 
		 // NAME already exists ?
         $stmt = "SELECT ea.extra_attrib_id FROM extra_attrib ea "
                  . "WHERE "
                  . "ea.name = '" . $attribute["name"] . "' "
                  . "AND ea.extra_class_id = '" . $classDescription["foundClass"] . "' ";
				  
		 $this->html->statement($stmt);
         if ($this->dbClass->Query($stmt) != 1)
                $this->stop("Database query failed. Import abort. SQL-statement: $stmt", $FUNCNAME."()");

         // query or result failed, means class verification failed
         if ($this->dbClass->ReadRow()) {
		 	$this->stop("Attribute (Name: ".$attribute["name"].") already exists for class: ".$classDescription["name"], $FUNCNAME."()");
		 }
		 
		 // MAP_COL already exists ?
         $stmt = "SELECT ea.extra_attrib_id FROM extra_attrib ea "
                  . "WHERE "
                  . "ea.MAP_COL = '" . $clsAttribFeat["MAP_COL"] . "' "
                  . "AND ea.extra_class_id = '" . $classDescription["foundClass"] . "' ";
		 $this->html->statement($stmt);
 		
		 if ($this->dbClass->Query($stmt) != 1)
                $this->stop("Database query failed. Import abort. SQL-statement: $stmt", $FUNCNAME."()");
				
         if ($this->dbClass->ReadRow()) {
		 	$this->stop("Attribute, where MAP_COL='".$clsAttribFeat["MAP_COL"]."' already exists for class: ".$classDescription["name"], $FUNCNAME."()");
		 }
		 
		 // no error: extend class now
		 $this->insertClsAttrib($classDescription["foundClass"], $attribute);
		 
		 $this->updatingCache  = 1;
		 $this->dbClass->commit(); 		// don't know, if autocommit was ON
		 
		 $this->html->warning("Class '".$classDescription["name"]."' was extended by attribute '".$attribute["name"]."'");
		 
		 
		 
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
    
    
    
    // a new class has been added? update varcol cache
    //
    // input:   bool    $updateCache        update required?
    // output:  string (view updateVarcolCache)
    //
    function updateVarcolCache($updateCache)
    {
		global $error;
        // update varcolcache
        if ($updateCache)
        {
			$error = &ErrorHandler::get();
			
            $this->html->updateCache();
            get_cache(true, false);
            
            if ($error->got())
                return "<br>Updating varcol-cache failed.";

            $this->html->updatedCache();
        }
    }
    
    
    /**
     * star xml processing
     * @param string $tmpFilename - upload-file on PHP-Temp-Dir
     * @param string $orgFilename - pure filename
     * @param int $rootProj
     */
    function start($tmpFilename, $orgFilename, $rootProj)
    {   
        
        
        $this->html->start(PaXML_XMLVERSION, PaXML_READERVERSION);
        
        $tempDebugLev = $_SESSION['userGlob']["g.debugLevel"];
		
		if ($orgFilename=="") {
			// get original file name from tempfile-name (when $tmpFilename is not an upload, but a temp-file on server)
			$orgFilename = basename($tmpFilename);  
		} 
		
        if ( $tempDebugLev>0 ) {
            echo "<br>DEBUG: PaXMLReader::start() uploaded file: temporary: '".$tmpFilename."' orgininal-name: '".$orgFilename."'<br>\n";
        }
        
        // init paxml library
        $this->lib = new PAXMLLib($this->html, $this->user, $this->passwd, $this->db, $this->dbtype, $this);

        // with this xml version some new features had been added, which require
        // a updated cct_table entry. checking here for it.
        if (!$this->lib->checkForCorrectVersion())
                $this->stop("Missing updated CCT_TABLE. Inform your admin.", "start()");

        // init database connections ...
        $this->initDatabase($this->user, $this->passwd, $this->db, $this->dbtype);

		// detecting file type
        if ( !file_exists( $tmpFilename ) ) {      
            $this->stop("Paxml-File '$tmpFilename' not found.");
        }
        
        // set root project. check if given project exist. else abort here
        $this->rootProject = $rootProj;
        if ($this->rootProject != -1)
            if ($this->lib->existsProject($this->rootProject) ===  null)
                $this->stop("Given project doesn't exist.", "start(); project: $rootProj");

		// ... and check permissions.
        $this->checkPermissions();
		
        // init files, means to transform any type of input file to a xml file
        // and a import path. possible infput files may be xml, gz or tar
        if (!($this->xmlFilename = $this->initFile($tmpFilename, $orgFilename)))
 			$this->stop("No paxml file found. <!-- session id: " . session_id() . " //-->");

		if ($this->options["update"]) {
			 $tmptext = "";
			 foreach( $this->options["update"] as $key=>$val) {
			 	$tmptext .= "\"$key\" => \"".$val."\" ";
			 }
			 reset($this->options["update"]);
			 $this->html->info("UpdateMode:", $tmptext, PaXML_TINYVIEW );							
		}
		
		if ($_SESSION['globals']["DbArchiveMode"]>0) {
			$this->html->info("DbArchiveMode:", "DB is in ARCHIVE-Mode!", PaXML_TINYVIEW );				
		}
		
		if ($this->options["asarchive"]) {
			$this->html->info("Import as archive:", "activated!", PaXML_TINYVIEW );
			$this->html->info("Import as archive:", "import ORIGINAL W_WAFER_IDs !", PaXML_TINYVIEW );							
		}
        // now start the parser: 5, 4, 3, 2, 1, go ... and he starts, yeah ...
        $this->paxml = new PaXMLFileInput($this, $this->xmlFilename);
        $this->html->initXMLParser();
        $this->paxml->init();
        if (is_array($this->options['isp'])) {
        	$this->paxml->set_option('isp', $this->options['isp']);
        }
        $this->html->parse();
        $this->paxml->parse();
        $this->html->finish($this->importPath);
		
    }
    
    
    
    // rollback all transactions. something went wrong. rollback, to keep data
    // consistent
    //
    // input:   -
    // output:  -
    //
    function rollbackAll()
    {
        $this->html->rollback();
        if ($this->dbObject != null)
        {
            $this->dbObject->rollback();
            $this->dbObject->close();
        }
        if ($this->dbOption != null)
        {
            $this->dbOption->rollback();
            $this->dbOption->close();
        }
        if ($this->dbClass != null)
        {
            $this->dbClass->rollback();
            $this->dbClass->close();
        }
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


    // - validating existing class.
	// - existing class must have all attributes, which
    //   may be imported, if class would not exist.
    // - an imported class must have fewer or same number of attributes than the
    //   existing one and the attributes have to match against each other.
    // - the matching algorithm compares name, table_name, datatype and app_data_type.
    // - name and table_name are important for identification. datatype
    //   and app_data_type are important to be identical for functionality. this
	// - NEW: 20050711: a class can be extended if (MAP_COL not exists) AND (NAME not exists)
    //
    // - the execution stops directly, if a class is not valid.
    //
    // - name and table_name are already checked in a way, that this are the
    //   criterias to find a class
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
            $data = array();	// temporary data
            
            // get values for DATATYPE and APP_DATA_TYPE
            if ($attribute["params"] != null and is_array($classDescription["params"]))  // are there some parameters ?
                foreach($attribute["params"] as $param)	                           // add them
                    //if($param["name"] == "DATATYPE" or $param["name"] == "APP_DATA_TYPE")
                    $data[$param["name"]] = $param["value"];	// save all parameter values

            // it's a required field
            if ($data["APP_DATA_TYPE"] == null)
                $this->stop("Semantic parameter 'app_data_type' is missing. Import aborted.", "validateClass()");

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
                  . "AND adt.name = '" . $data["APP_DATA_TYPE"] . "'";

            $this->html->statement($stmt);
            if ($this->dbClass->Query($stmt) != 1)
                $this->stop("Database query failed. Import abort. SQL-statement: $stmt", "validateClass()");

            // query or result failed, means class verification failed
            if (!$this->dbClass->ReadRow()) {
			
				// try to look for a class extension
				$this->extendClass($classDescription, $attribute, $data);
			
                /*$this->stop("Conflict (1). The class " . strtolower($classDescription["table"])
                          . "." . $classDescription["name"] . " found in xml doesn't "
                          . "have the a compatible structure like the class "
                          . "in database with same name. Problems with "
                          . "attribute '" . $attribute["name"] . "'. An "
                          . "attribute, describe in paxml doesn't exist "
                          . "in database class structure. Import abort.", "validateClass()");
				*/
			}
            
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

	// insert ONE class atribute
	function insertClsAttrib($classID, $attribute) {
	
		$data				 	= array();			    // $data is simple field (key) - value pair
		$data["name"] 			= $attribute["name"];   // name of attribute
		$data["extra_class_id"] = $classID;             // class id
		if ($attribute["params"] != null)			    // other parameters ?
			foreach($attribute["params"] as $param)
				if ($param["name"] == "APP_DATA_TYPE")  { // catch special case of app_data_type
					$data["APP_DATA_TYPE_ID"] = $this->lib->getAppDataType($param["value"]);
					$temp_app_data_type = $param["value"];
				} else
					$data[$param["name"]] = $param["value"];

		if ($data["APP_DATA_TYPE_ID"] == null)
			$this->stop("Semantic PaXML error at attribute tag. Given value for "
						. "parameter 'app_data_type'='".$temp_app_data_type."' is not valid. Import aborted.", "importClass()");
		
		// import into extra_attrib. if import failed, abort.
		$attribID = $this->import("EXTRA_ATTRIB", $data, "EXTRA_ATTRIB_ID", "extra");
		if ($attribID == null)
			$this->stop("Database writing error at insert of a new class attribute. Import aborted.", "importClass()");
	
	}

    // import class.
    //
    // input:   array       $classDescription       have a look at PaXMLFileInput.inc
    // output:  -
    //
    function importClass($classDescription)
    {
        // transform stack data structure into a field-value-pair structure for
        // import function
        $data = array();
        $data["name"]       = $classDescription["name"];	        // $data is simple field (key) - value pair
        $data["table_name"] = $classDescription["table"];           // is validated in startClass()
        if ($classDescription["params"] != null)			        // are there some parameters ?
            foreach($classDescription["params"] as $param)	        // add them to the list
                $data[$param["name"]] = $param["value"];

		// nice_name constraint! (table_name + nicename)
		// if nice_name constraint occurs, change name of nicename
		$stmt = "SELECT extra_class_id FROM extra_class "
			  . "WHERE nice_name = '" . str_replace("'", "''", $data["NICE_NAME"]) . $ext  . "' AND table_name = '" . $data["table_name"] .  "'";

		$this->html->statement($stmt);
		if ($this->dbClass->Query($stmt) != 1)
			$this->stop("Database read failed. Import aborted. <!-- importClass() //-->");
	
		// nice name twice? change the nicename
		if ($this->dbClass->ReadRow())
		{
				$this->html->warning("Nice name conflict at extra classes. The nice name will be changed.");
				$data["NICE_NAME"] = $data["NICE_NAME"] . " (" . $data["name"] . ")";
		}

        // insert class. if import failed stop
        $this->dbClass->setAutoCommit(false);
		$classID = $this->import("EXTRA_CLASS", $data, "EXTRA_CLASS_ID", "extra");
        if ($classID == null)
            $this->stop("Database writing error at insert of a new class. Import aborted.", "importClass()");

        // insert attributes now. resolve the name of app_data_type to the id
        foreach($classDescription["attribs"] as $attribute)
        {
            $this->insertClsAttrib($classID, $attribute);	// insert ONE attribute
		}

        // commit db action
        $this->html->statement("COMMIT");
        $this->dbClass->commit();
        $this->dbClass->setAutoCommit(true);
    }



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
    


    // insert cct_access entry
    //
    // input:   string      $data     key-value pairs
    // output:  cct_access_id
    //
    function importCCTAccess($data)
    {
         return $this->import("CCT_ACCESS", $data, "CCT_ACCESS_ID");
    }
    
    // import ONE VARIO value
    private function importVario_one($table, $bo_id, $vario_row) {
        
        $db = &$this->dbObject;
        $vario_mod_lib = new oS_VARIO_mod();
        $vario_mod_lib->setObject( $db, $table, $bo_id);
        $vario_mod_lib->set_touched(1);
        
        $key = $vario_row['name'];
        $val = $vario_row['value'];
        $vario_mod_lib->updateKeyVal($db, $key, $val);        
    }
    
    // import all VARIOs
    function importVario($table, $bo_id, $vario ) {
        global $error;

        $this->html->infoline("IMPORT VARIOS T:".$table." ID:".$bo_id."...", PaXML_MAXVIEW);
        foreach($vario as $vario_row) {
            $this->html->infoline("IMPORT VARIO: ".print_r($vario_row,1), PaXML_MAXVIEW);
            $this->importVario_one($table, $bo_id, $vario_row);
            
            if ($error->Got(READONLY))  {
                $tmpTxt = $this->_LastErrDBWriteErr();
                $this->stop($tmpTxt, __FUNCTION__."()");
            }

        }
    }
    
    // import BO-object including extra vlues
    //
    //
    function importObject($table, $objectDescription, $cctAccessID, $wiid, $roid, $base, $filename)
    {    
        
        
        $data                  = array();	                  		                 // prepare normal params
        $extra_data	           = array();			                                 // prepeare dynamical params
        $data["cct_access_id"] = $cctAccessID;                                       // get cct_access_id
        $data["extra_obj_id"]  = $this->alignToClass($objectDescription["class"]);   // align a extra class if existing

        // no alignment to extra_obj, but class name was defined ? -> error!
        if ($data["extra_obj_id"] == null && $objectDescription["class"] != null)
            $this->stop("No class was aligned for object. Import aborted.", "importObject()");

        // map parameters to importable data structures. means to split object
        // parameters from class parameters and resolve option values to id
  	    //	- 	dynamic parameters to $extra_data
		//	-	foreign key data with foreign key value to $data
		//	-	normal data to $data
        if ($objectDescription["params"] != null)
			 foreach($objectDescription["params"] as $param)
             {
                if (strtoupper($param["type"]) == "CLASS")
				    array_push($extra_data, $param);
                elseif (strtoupper($param["type"]) == "OPTION") {
                    $data[strtoupper($param["name"]) . "_ID"] = $this->findOption($param["name"], $param["value"]);
				}
				elseif ($param["type"] == null || strtoupper($param["type"]) == "OBJECT")
				    $data[$param["name"]] = $param["value"];
            }
            
        // specials for PRIMARY_KEY ?
		if ( $this->options["archOriID"] ) {
			$this->boIDspecials($table, $data, $wiid, $roid);
		}
		
		// import it now, normal data and extra data
		$table_pk_name = PrimNameGet2($table);
        $bo_id = $this->import($table, $data, $table_pk_name);
        if ($bo_id == null)							
            $this->stop("Failed to write business object. Import aborted.", "importObject()");

        // import extra values
        $this->importExtraValues($table, $objectDescription["class"], $data["extra_obj_id"], $extra_data, "object");
        
        if(!empty($objectDescription['vario'])) {
            $this->importVario($table, $bo_id, $objectDescription['vario']); 
        }

        // circle reference stuff. this object was a circular reference till now.
        // Now the real id is well know. set for any temp object the new id to
        // link for. all object link to temp object instead of this one have to
        // be acknowledged about a new id. where (2. parameter) tells this object
        // how to find current object, which is existing now.
        
        if ($objectDescription["identifyYourself"]) { 
                $this->updateUpdateList($objectDescription["identifyYourself"], $table_pk_name . " = '" . $bo_id . "'");
        }
        // check here for updates of circle references. in fact this function is
        // actually resolving the circular reference, if current object was one.
        $this->updateCircleReferences($table, $wiid, $roid, $bo_id);

        if ($table == "CONTACT")						// the current bo is contact so
            $this->alignWiidToContact($bo_id, $base);	// try to align an wiid with contact
                                                        // diese zeilen gibt's ja zweimal !?

        $this->importAttachment($table, $bo_id, $filename);

        return $bo_id;
    }

    function updateObject($objectDesc)
    {    
        
		
		$table    = $objectDesc["table"];
		$bo_id 	  = $objectDesc["id"];
		$filename = $objectDesc["file"];
		
		$optarr = NULL;
		if ( $this->options["update"]["files"] ) $this->importAttachment($table, $bo_id, $filename, $optarr, PaXML_MINVIEW);
    }   
	
	function importAttachment(
			$table, // "LINK", "IMG", "SATTACH"
			$bo_id, 
			$filename, 
			$optarr = NULL, // for "SATTACH" : array("parent", "rel_id")
			$minInfoLevel=PaXML_MIDVIEW
			) {
		// is there a attached file ? this file is located in import
        // directory. if such a file found copy that file into server
        // location to save it finally. for more read devnotes.
		
        if (($table == "LINK" or $table == "IMG" or $table == "SATTACH") and $filename != null)
        {
			if ($table == "SATTACH") {
				$infoTabobj = $table.":".$optarr["parent"].":".$optarr["rel_id"];
				if ($optarr["parent"]=="" OR !$optarr["rel_id"]) return -1; // IDs missing
				
			} else $infoTabobj = $table;
			
            $this->html->saveAttachedFile($infoTabobj, $bo_id, $minInfoLevel);
            if ($table == "LINK") $destPath = linkpath_get($bo_id);
            if ($table == "IMG")  $destPath = imgPathFull ($bo_id);
			if ($table == "SATTACH") 
					 $destPath = $this->attachLib->getDocumentPath( $optarr["parent"], $bo_id, $optarr["rel_id"] );

            if (!file_exists($this->importPath . "/" . $filename))
                $this->html->warning("Missing file '".$filename."' attached to object (path: ".$this->importPath.") .");
            elseif (!copy($this->importPath . "/" . $filename, $destPath))
                $this->html->warning("Could not copy attached document to internal document directory.", "importObject()");
        }
	} 
    
    // check if linked object type is a valid value
    //
    // input:   string      $tablename      table
    //          string      $wiidname  wiid name (string)
    //          string      $roid      roid
    // output:  object id
    //
    function findObject($tablename, $wiidname, $roid)
    {
    	$table_pk_name = PrimNameGet2($tablename);
    	
        // check input parameters
        if ($tablename == null or $wiidname == null or $roid == null  or !is_string($tablename) or !is_string($wiidname) or !is_string($roid))
            $this->stop("Illegal parameters to findObject(). Import aborted.", "findObject()");

        // try to get wiid id by wiidname. if wiidname not existing in db, a new
        // dataset is created
        if (($wiid = $this->getWiid($wiidname)) == null)
            $this->stop("Couldn't get wiid for $wiidname. Import aborted.", "findObject()");

		// if you want to find your own data, search also for wiid == null
		if ($wiidname == "http://www.clondiag.com/magasin/?db=" .  $this->lib->getDBSerial())
			$x = "((ca.wiid = $wiid AND ca.roid = $roid) OR (ca.wiid IS NULL AND bo." . $table_pk_name . " = $roid))";
		else
			$x = "ca.wiid = $wiid AND ca.roid = $roid";

		$stmt = "SELECT " . $table_pk_name . " FROM $tablename bo, cct_access ca ";
		$stmt .= "WHERE ca.cct_access_id = bo.cct_access_id AND " . $x;

	    $this->html->statement($stmt);
		if ($this->dbObject->Query($stmt) != 1)
			$this->stop("Fatal database reading error. Could not select objects. Import aborted.", "findObject()");

        // get object id
		if ($this->dbObject->ReadRow())
			return $this->dbObject->RowData[0];

		return null;
    }
    

    
    // start transaction
    //
    // input:   -
    // output:  -
    //
    function startObjectImport()
    {
        $this->dbObject->setAutoCommit(false);
    }



    // end transaction
    //
    // input:   -
    // output:  -
    //
    function endObjectImport()
    {
        $this->dbObject->commit();
        $this->dbObject->setAutoCommit(true);
    }


    
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
    
    // search for option entry in database by NAME
    //
    // input:   $type, $name
    // output:  id of table-entry
    //
    function findOption($type, $name)
    {
        if ($type == null or $name == null or !is_string($type) or !is_string($name))
            $this->stop("Development error. Illegal parameters to findOption(). Import aborted.", "findOption()");
		
		if ($type=='DB_USER') {
			return $this->lib->getUserID($name);
		}
		
		$pkname = $this->lib->getPrimKey($type);
		$stmt = "SELECT " . $pkname . " FROM " . $type . " ".
					"WHERE name = '" . $name . "'";

	    $this->html->statement($stmt);
		if ($this->dbOption->Query($stmt) != 1)
			$this->stop("Trying to find option entry. Can't read from database. Import aborted.", "findOption()");

		if ($this->dbOption->ReadRow())
			return $this->dbOption->RowData[0];

		return null;
    }



    // import option.
    //
    //
    function importOption($optionData)
    {    

        $data 		  		  = array();				// prepare data for import function
        $extra_data	  		  = array();				// prepare extra data for import function
        $data["name"] 		  = $optionData["name"];
        $data["extra_obj_id"] = $this->alignToClass($optionData["class"], "option");

        if ($optionData["params"] != null)			// are there some parameters ?
            foreach($optionData["params"] as $param)
                if (strtoupper($param["type"]) == "CLASS")
                    array_push($extra_data, $param);
                elseif (strtoupper($param["type"]) == "OPTION" || $param["type"] == null)
                    $data[$param["name"]] = $param["value"];

        // ... import now
		$table = $optionData["table"];
		$pkname = $this->lib->getPrimKey($table);
        if (($retval = $this->import( $table, $data, $pkname, "option")) == null)
            $this->stop("Fatal database writing error. Import aborted.", "importOption()");

        // ... and now the extra values
        $this->importExtraValues( $table, $optionData["class"], $data["extra_obj_id"], $extra_data, "option");
        return $retval;

    }



    // start transaction
    //
    //
    function startOptionImport()
    {
        $this->dbOption->setAutoCommit(false);
    }



    // end transaction
    //
    //
    function endOptionImport()
    {
        $this->dbOption->commit();
        $this->dbOption->setAutoCommit(true);
    }
    


    // check for right elements type
    //
    // input:   string      $table     table
    // output:  -
    //
    function verifyElements($table)
    {
		$tabletype = $this->lib->getTableType($table);
        if (!(PaXML_ASSO === $tabletype or PaXML_EXTEND === $tabletype or PaXML_SATACH === $tabletype ))
            $this->stop("Semantic PaXML error at elements tag. Table: '".$table."' : given value for attribute 'type' is not valid for this table type '".$tabletype."'. Import aborted.");
    }
    
	function _table_fromPKname($name) {
		 return ( substr($name, 0, strlen($name) - 3) );
	}
	
	// check if protocol step exists: 0:no 1:yes
	function _checkStepExists($absProtoID, $stepnr) {
		$stmt = "SELECT 1 from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$absProtoID." and STEP_NR=".$stepnr;
        $this->dbObject->Query($stmt);
		if ($this->dbObject->ReadRow()) return 1;
		else return 0;
	}
    
    // 
    //
    //
    /**
     * end transaction
     * map parameters to importable structures
     * - dynamic parameters to $extra_data
     * - system parameter with evaluated foreign key to $data
	 * - id of parent object (like normal data). it's important to
			       hava separation by type (parent), because had problems with
			       linked projects. it happened, that there were field PROJ_ID
			       twice in array structure, one for parent one for linked
                   project. do not change this
     * - PROJ_HAS_ELEM element to $data
     * - normal data to $data
     * @param array $elemDescr <pre>
     * [type]   => listingElement
       [table]  => SATTACH
					TBD: parent needed !!!!
        [import] => 1
		[file]	 => name (attached file)
		[class]  = > class ???
            [params] => Array (
			[0]  => Array
                        (
					[name] => ABSTRACT_PROTO_ID
					[value] => 12234
					[type] => PARENT
                        )
			)
			</pre>
     * @return -
     */
    function importElement( $elemDescr )  {
		    $data		 = array();							// prepare normal parameter var
            $extra_data	 = array();							// prepare extra parameter var
			
			
			$table = $elemDescr["table"];
			$func_objid  = 0;
			$func_parent = NULL;
			
			if ($elemDescr["params"] != null) {
			
				
				foreach($elemDescr["params"] as $param)
                {
                	
					if (strtoupper($param["type"]) == "CLASS")
						array_push($extra_data, $param);
					elseif (strtoupper($param["type"]) == "OPTION")
						$data[strtoupper($param["name"]) . "_ID"] = $this->findOption($param["name"], $param["value"]);
					elseif ($param["type"] == "PARENT")
					{
						if ($table=="SATTACH") {
							$func_parent = $this->_table_fromPKname($param["name"]);
							$data["TABLE_NAME"] = $func_parent;
							$data["OBJ_ID"]     = $param["value"];
							$func_objid		    = $param["value"];
							
						} else {
							$data[$param["name"]] = $param["value"];
						}
						
					}
					elseif (strtoupper($elemDescr["table"]) == "PROJ_HAS_ELEM"
						&& substr($param["name"], -3) == "_ID"
						&& $param["type"] != "PARENT"
                        && ($param["type"] == null || strtoupper($param["type"]) == "OBJECT"))
					{
                      	$data["TABLE_NAME"] = $this->_table_fromPKname($param["name"]);
    					$data["PRIM_KEY"]   = $param["value"];
					}
					elseif ($param["type"] == null || strtoupper($param["type"]) == "OBJECT")
                    {
                    	// normal element-column
						$data[$param["name"]] = $param["value"];

                        // extract primkeys
                        if ($this->lib->isPrimKey($elemDescr["table"], $param["name"]))
                            $pkeyFields[$param["name"]] = $param["value"];
                    }
                    
                   
                }
			}
            // get extra obj id entry
            if (($eo_id = $this->alignToClass($elemDescr["class"])) != null)
                $data["extra_obj_id"] = $eo_id;

            // no alignment, but class name was defined ?
            if ($data["extra_obj_id"] == null && $elemDescr["class"] != null)
                $this->stop("Database writing error. Could not add class structure. Import aborted.", "importElement()");

            // also option may have mass data!
            $dbHandle = ($elemDescr["dbHandle"] == "option") ? "option" : "object";
            
            
            // check for element in project if type is PROJ_HAS_ELEM
            $proj_has_elem = false;
            if ($elemDescr["table"] == "PROJ_HAS_ELEM" ) {
                $proj_has_elem = $this->checkForElementInProject($data);
            }
            
            // due to DB-structure change, Bugtrack:0001740: db.NewVersion 1.0.5.7
            if ($table == "SATTACH" and glob_column_exists("SATTACH", "DB_USER_ID")) {
            	$data["DB_USER_ID"] = $_SESSION['sec']['db_user_id']; // correct DB_USER_ID to local user
            }

            // import data
            if (!$proj_has_elem /* && $param["name"] != "PROJ_ID"*/)
            {
            	if ($elemDescr['update']) {
            		// if $elemDescr['update'] only allow few actions !!!
            		if ( $table=='ABSTRACT_PROTO_STEP' ) { // update of protocol steps
            	
	            		if ( !$this->_checkStepExists($data['ABSTRACT_PROTO_ID'], $data['STEP_NR']) ) {
	            			$this->html->info('ABSTRACT_PROTO_STEP:update', 'Proto-ID: '.$data['ABSTRACT_PROTO_ID'].' Step: '.$data['STEP_NR']);
	            			$this->import($table, $data, null, $dbHandle);
	            		}
            		}
            	} else {
    		    	$this->import($elemDescr["table"], $data, null, $dbHandle);
               		$this->importExtraValues($elemDescr["table"], $elemDescr["class"], $data["extra_obj_id"], $extra_data, $dbHandle);
            	}
            }
			
            if ( $table=="SATTACH" AND $elemDescr["file"]!="" and !$elemDescr['update']) { 
				// not allowed on "update", beacause existing attachments are inserted with same Primaray keys ==> would cause an error
				$attachArr = array("parent"=>$func_parent, "rel_id"=>$data["REL_ID"] );
				$this->importAttachment($table, $func_objid, $elemDescr["file"], $attachArr);
			}
            // which case has been catched here? shit alzheimer.
            if ($elemDescr["identifyYourself"])
            {
                $where = "";
                foreach($pkeyFields as $field => $value)
                {
                    $where .= $and . "$field = '$value'";
                    $and = " AND ";
                }
                $this->updateUpdateList($elemDescr["identifyYourself"], $where);
            }


    }



	// general import method with return value. it handles import for all three
    // database handles. why not insert row? because insert row is doing some
    // special checks (maybe).
    //
	// input:   string      $table	     table name
	//	        array       $data	     parameters for input. fieldname => value pairs
	//	        string      $pkey	     primary key fieldname
    //          string      $what        db handle
    // output:  id of inserted object
    //
	function import($table, $data, $pkey = null, $what = "object")
	{
		global $error;
		$FUNCNAME= 'import';
		// check parameters
        if ($data == null or $table == null or !is_string($table))
            return;

        // set db handle
        if ($what == "extra")
            $db = &$this->dbClass;
		if ($what == "option")
            $db = &$this->dbOption;
		if ($what == "object" || $what == null)
            $db = &$this->dbObject;

		if ($db == null)
			$this->stop("Wrong input option to import(). Please inform your administrator. Import aborted.", "import()");

        // To be informed is more important than in former times! (has somebody said)
        $info = $this->lib->getTableInfo($table);

		// if there is one column at least ...
		$doit = false;
		$komma= "";
		foreach($data as $key => $value)
		{
			// check column for existing and value. if value date change dateformat
            // and if value contains ' replace to ''
			if ($this->lib->existsField($table, $key))
			{
			    $doit = true;
			    $datatype = $info["columns"][strtoupper($key)]["datatype"];
				$cols .= $komma . $key;
				if ($value == null)
					$vals .= $komma . "NULL";
                elseif ( $datatype == "date")
                    $vals .= $komma . $db->dateString2Sql($value, 1);
                else
				    $vals .= $komma . "'" . str_replace("'", "''", $value). "'";
				$komma= ", ";
			}
		}

		// ... do an import
		if ($doit)
		{
		    $stmt = "($cols) VALUES ($vals)";
            $this->html->statement('INSERT: '.$stmt);
			if ($pkey == null)
			{
				if ($db->Insertx( $table, $cols, $vals ) <= 0) {
					$tmpTxt = $this->_LastErrDBWriteErr();
					$this->stop($tmpTxt, "import()");
				}
            }
            else
            {
				return $db->queryInsert($table, $stmt, $pkey);
			}
		}
		else
		    $this->stop("Couldn't insert, because all fields in xml file are not found in database.", "import()");
	}



	// import of extra values
    //
	// input:   string      $table	     table name
	//	        array       $data	     parameters for input. fieldname => value pairs
	//	        string      $pkey	     primary key fieldname
    //          string      $what        db handle
    // output:
    //
    function importExtraValues($table, $class, $eo_id, $data, $what = "object")
    {

        
        if ($class == null or $eo_id == null or $table == null)			// check parameters
            return;
        
        // use different DB handles to manage the COMMIT policy
		if ($what == "option")
           $db = &$this->dbOption;
		if ($what == "object" || $what == null)
           $db = &$this->dbObject;

		if ($db == null)
			$this->stop("Wrong input option to importExtraValues(). Please inform your admin. Import aborted.", "importExtraValues()");

        // get all attributes with mapcols
        //
        // get a list of all attributes
        $stmt = "SELECT ea.name, ea.map_col FROM extra_attrib ea, extra_obj eo WHERE eo.extra_class_id = ea.extra_class_id and eo.extra_obj_id = ".$eo_id;
        $this->html->statement($stmt);
        if ($db->Query($stmt) != 1)								// get all attributes to that class
       	    $this->stop("Database reading error. Import aborted.", "importExtraValues()");
		
        while ($db->ReadRow())
            $attribs[$db->RowData[0]] = $db->RowData[1];
        // create string for insert to insert into extra_obj into fields the columns
        
         
        if ($attribs == null)
            return;
        
        $stmt   = "UPDATE extra_obj SET "; // TBD: special update: is allowed to not use Updatex() here
        $prefix = "";
        foreach ($data as $item)
        {
            $mapcol = $attribs[$item["name"]];
            $value  = $item["value"];
            
            if ($value != null)
            {
                $stmtA .= "$prefix $mapcol = '" . str_replace("'", "''", $value) . "'";
                $prefix = ", ";
            }
            $mapcol = "";
            $value  = "";
        }

        if ($stmtA == null)
            return;

        $stmt .= $stmtA . " WHERE extra_obj_id = $eo_id";
        $this->html->statement($stmt);
        if ($db->Query($stmt) != 1)								// get all attributes to that class
       	   $this->stop("Fatal database writing error: ".$stmt.". Import aborted.", "importExtraValues()");


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



    // try to get the contact info for a spezial wiid.
    //
    // input:   integer     $bo_id      object id
    //          string      $base       wiid name
    // output:
    //
    function alignWiidToContact($bo_id, $base)
    {
        if ($base == null and !is_string($base))
            $this->stop("Application error. Import aborted.", "alignWiid2Contact()");

        // get contact from database
        $stmt = "SELECT wiid, contact_id FROM h_wiid WHERE name = '" . $base . "'";
		$this->html->statement($stmt);
		if ($this->dbObject->Query($stmt) != 1)
            $this->stop("Database read error. Import aborted.", "alignWiid2Contact()");
        if ($this->dbObject->ReadRow())
        {
            $pkWIID  = $this->dbObject->RowData[0];
            $contact = $this->dbObject->RowData[1];
        }

        $stmt = null;

        // entry in h_wiid found? no. insert h_wiid entry including contact
        if ($pkWIID == null) {
			// CODE_INFO: insert allowed here
			$stmt = "INSERT INTO h_wiid (contact_id, name) VALUES (" . (($bo_id == null) ? "null" : $bo_id) . ", '" . $base . "')";
		}
        // or entry found, but no contact for it. update wiid entry with contact
        if ($pkWIID != null)
            if ($contact == null and $bo_id != null) {
				// CODE_INFO: update allowed here
			    $stmt = "UPDATE h_wiid SET contact_id = '" . $bo_id . "' WHERE name = '" . $base . "'";
			}
        // run, if a query is to execute
        if ($stmt != null)
		{
            $this->html->statement($stmt);
			if ($this->dbObject->Query($stmt) != 1)
                $this->stop("Database write error. Import aborted.", "alignWiid2Contact()");
        }
    }



	// create extra_obj and return extra_obj_id
    //
    // input:   integer     $class      class id
    //          string      $base       handle identifier
    // output:
    //
	function alignToClass($class, $what = "object")
	{
		if ($class == null or !is_string($class))
			return null;

		if ($what == "option")
           $db = &$this->dbOption;
		if ($what == "object")
           $db = &$this->dbObject;

		// select a class by name
		$stmt = "SELECT extra_class_id FROM extra_class WHERE name = '$class'";

		$this->html->statement($stmt);
		if ($this->dbClass->Query($stmt) != 1)
            $this->stop("Database read error. Import aborted.", "alignToClass()");

		if ($this->dbClass->ReadRow())
			$classid = $this->dbClass->RowData[0];

		if ($classid != null)
		{
			$stmt = "(extra_class_id) VALUES ($classid)";
			$this->html->statement($stmt);
			return $db->queryInsert("EXTRA_OBJ", $stmt, "extra_obj_id");
		}
		else
			return null;	// error
	}



 	// link an object into a project. simple action.
    //
    // input:   string      $table          table name of object to insert
    //          number      $id             id of object to insert
    // output:  -
    // error:   -
    //
    function alignToProject($table, $id)
 	{
		$FUNCNAME="alignToProject";
        if ($table == null or $id == null or !is_string($table))
            $this->stop("Application error. Wrong parameter input. Import aborted.", "alignToProject()");

 		// no project, exit function here
 		if ($this->rootProject < 1)
 			return;

 		$stmt   = "INSERTX: PROJ_HAS_ELEM (".$this->rootProject . ", '" . $table . "', " . $id . ", 0)";
		$colstr = "proj_id, table_name, prim_key, is_link";
		$valstr = $this->rootProject . ", '" . $table . "', " . $id . ", 0";

        $this->html->alignToProject();
		$this->html->statement($stmt);
 		if ($this->dbObject->Insertx ( "PROJ_HAS_ELEM", $colstr, $valstr, $FUNCNAME) != 1)
            $this->stop("Database write error. Import aborted.", "alignToProject()");
 	}
 	
 	
 	
    // returns a temporary created object. if the object to be created needs
    // other child objects, subobjects will be created by a recursive call of
    // getTempObject.
    //
    // problem: at that moment, when temp object exists, every body may use it,
    // and avoid any deletion!
    //
    // input:   string      $name       type of object
    // output:  -
    //
    function getTempObject($name, $wiid, $roid)
    {
        
        
        // check if already an temporary object of type $name exists. if yes
        // use it. 
        $tempDebugLev = $_SESSION['userGlob']["g.debugLevel"];
        if ( $tempDebugLev>1 ) {
            echo "DEBUG:Function:getTempObject(".$name.",$wiid, $roid) debug-level:$tempDebugLev <br>\n";
        }
        
        $id = $this->inTempObjectList($name);
        
        if ( $tempDebugLev>1 ) {
            echo "DEBUG: id from inTempObjectList:$id <br>\n";
        }
        
        if ($id != null) {
            return $id;
        }
        // no temp object. than create one !
        // 1. step get all not nullable fields
        if (($nnFields = $this->lib->getNotNullableFields($name)) === false)
            $this->stop("Error by getNotNullableFields(). Import abort. Contact your administrator.", "getTempObject()");

        // create a CCT_ACCESS entry, needed also for temporary objects
        $data               = array();
        $data["crea_date"]  = date_unix2datestr(time(), 1);    // date format
        $data["db_user_id"] = $_SESSION['sec']['db_user_id'];
        $data["table_name"] = $name;
        
        if ( $tempDebugLev>1 ) {
            echo "DEBUG: before import() parameters:".print_r($data)." <br>\n";
        }
        $cctAccessID = $this->import("CCT_ACCESS", $data, "CCT_ACCESS_ID");
        if ($cctAccessID == null)
            $this->stop("Database write error. Could not create access entry for temporary object. Import abort.", "getTempObject()");

        // create field values for not nullable fields
        //      field get date - current date
        //      field get foreign key - create temp object recursivly
        //      else value = 1 may mean "1", 1, true, 1.0
        // the notes getting a special text, to search for
        $data = array();
        $data["CCT_ACCESS_ID"] = $cctAccessID;
        $data["NOTES"] = "<cct:object type='temporary' wiid='$wiid' roid='$roid'/>";   // objects with this notes should never exist in db
        if ($nnFields)
            foreach($nnFields as $key => $value)
            {
                if ($value["parent"] != null) {
                    if ( $tempDebugLev>1 ) {
                        echo "DEBUG: getTempObject() before getTempObject(".$value["parent"].") <br>\n";
                    }
                    $roidDummy  = -1;
                    $data[$key] = $this->getTempObject($value["parent"], $wiid, $roidDummy); // TBD: values for wiid, roid => is this right ?
                } elseif ($value["datatype"] == "DATE")
                    $data[$key] = date_unix2datestr(time(), 1);
                else
                    $data[$key] = "1";                      // value "1" works for string, long, float, bool
            }
        
        if ( $tempDebugLev>1 ) {
             echo "DEBUG: do import(".$name.", ".print_r($data).", null, true) <br>\n";
        }
        
        $tableTmp_pk_name = PrimNameGet2($name);
        $id = $this->import($name, $data, $tableTmp_pk_name, null, true);     // $data == null ?
        if ($id == null)
            $this->stop("Database write error. Could not create temporary object. Import abort.", "getTempObject()");

        // keep it in mind, there is a temp object which has to be handled in
        // anyway later (delete)
        if ( $tempDebugLev>1 ) {
             echo "DEBUG: before pushTempObjectList($name, $id, $cct_access_id) <br>\n";
        }
        $this->pushTempObjectList($name, $id, $cct_access_id);
        if ( $tempDebugLev>1 ) {
             echo "DEBUG: Function:getTempObject() READY <br>\n";
        }
        return $id;
    }



    // all temp objects created during import are listed tempObjectList. this
    // function cleans database of temp objects by this list
    //
    // input:   -
    // output:  -
    //
    function cleanTempObjectList()
    {
        if ($this->tempObjList and is_array($this->tempObjList))
        {
            // reverse list because of dependend temp objects. t1 is parent of
            // t2. t2 was created before t1. t2 ist before t1 in list
            // tempObjList. but t1 has to be deleted first. so reverse list
            // to delete allways parent objects first.
            $this->tempObjList = array_reverse($this->tempObjList);
            foreach($this->tempObjList as $item)
            {
                $stmt = "DELETE FROM cct_access WHERE cct_access_id = '" . $item["cct_access_id"] . "'";

                $this->html-statement($stmt);
                if ($this->dbObject->Query($stmt) != 1)
                    $this->html->warning("Could not remove temporary object from database.", "cleanTempObjectList()");
            }
        }
        $this->tempObjList = array();
    }



    // all temp objects created during import are listed tempObjectList. this
    // function pushes temp objects into list
    //
    // input:   string      $name           table name of object (object type)
    //          number      $id             object id
    //          number      $cct_access_id  cct_access id
    // output:  -
    // errors:  -
    //
    function pushTempObjectList($name, $id, $cctAccessID)
    {
        $this->tempObjList[] = array("table"         => $name,
                                     "id"            => $id,
                                     "cct_access_id" => $cctAccessID);
    }



    // exist an temporary object already. try to find a temp object in
    // tempObjectList
    //
    // input:   string      $name           table name of object (object type)
    // output:  object id if found else null
    //
    function inTempObjectList($name)
    {
        if ($this->tempObjList and is_array($this->tempObjList))
            foreach($this->tempObjList as $item)
                if ($item["table"] == $name)
                    return $item["id"];

        return null;
    }



    // resolving circular references. an object had used a temporary object to
    // handle a circular reference. this temporary object is representing the
    // object itself. so the link to temp object has to be replaced by the
    // link to itself. (if you have knot in your brain now, don't worry!)
    // this function updates the link to temp object, by redirecting to object
    // which is given. the correct entry will be found with $table, $wiid, $roid.
    //
    // input:   string      $table      tablename of current object
    //          string      $wiid       wiid of current object
    //          number      $roid       roid of ocurrent bject
    //          number      $value      new id to link, means object id (link
    //                                  to itself)
    // output:  -
    //
    function updateCircleReferences($table, $wiid, $roid, $value)
    {    
        
        
        if ($_SESSION['userGlob']["g.debugLevel"]>1) {
            echo "<br>\nDEBUG: updateCircleReferences($table, $wiid, $roid, $value) <br>\n"; 
            echo "DEBUG: updateList:: sizeof:".sizeof($this->updateList)." content:";
            print_r($this->updateList);
            echo "<br>";
        }
        if ($this->updateList and is_array($this->updateList))
            foreach($this->updateList as $item)
            {
                if ($item["table"] == $table
                    and $item["wiid"] == $wiid
                    and $item["roid"] == $roid)
                {
                    $this->html->resolvingCircularReference();
                    $stmt = "UPDATE " . $item["update"]["table"]
                          . " SET " . $item["update"]["field"] . "= '$value'"
                          . " WHERE " . $item["update"]["where"];

                    // update any dataset now, which links to object (wiid,
                    // roid) with valid object id now!
                    $this->html->statement($stmt);
                    if ($this->dbObject->Query($stmt) != 1)
			            $this->stop("Update of objects with temporary object failed "
                                  . " (circular reference). Import abort.", "updateCircleReferences()");
                }
            }
    }

    // if a circular reference is found, means an object needs itself to
    // describe completely, the problem will be solved with temporary objects.
    // a temporary object is used instead of not describable self referencing
    // object.
    // any link to a temporary object has to be resolved later. at that moment,
    // when a circular reference is found, the expected object is known and the
    // field linking to this object is known. updateList will be filled with
    // values for (table, wiid, roid and field). The dataset for updating the
    // link to temp object will be found later, and determined by 'where'
    // attribute. This 'where' has to be written at a differenct part of code
    // than updateCircleReferences is called.
    // this function updates entry in updateList with 'where' attribute.
    //
    // input:   array       $update         array("table" => value,
    //                                            "wiid" => value,
    //                                            "roid" => value)
    // output:  -
    //
    function updateUpdateList($update, $where)
    {   
        
        
        if ($_SESSION['userGlob']["g.debugLevel"]>1) {
                echo "DEBUG: updateUpdateList():: TABLE:".$update["table"]." cond:'$where' wiid:".$update["wiid"]." roid:".$update["roid"]." <br>";
        }
        if ($this->updateList and is_array($this->updateList)) {
                                    
            foreach($this->updateList as $key => $item)
            {
                // get a temp object to this
                if (    $item["table"]== $update["table"]
                    and $item["wiid"] == $update["wiid"]
                    and $item["roid"] == $update["roid"]  )
                {
                    $item["update"]["where"] = $where;
                    $this->updateList[$key]  = $item;
                }
            }
        }
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

        // ... else create a new entry ...
		$stmt = "INSERTX H_WIID (contact_id, name) VALUES (null, '" . $base . "')";
		$colstr="contact_id, name";
		$valstr="null, '" . $base . "'";
		$this->html->statement($stmt);
	    if ($this->dbObject->Insertx ( "H_WIID", $colstr, $valstr)  != 1)
            $this->stop("Database error. Could not set an wiid entry.", "getWiid()");

        // ... and try to get the id this entry
        $stmt = "SELECT wiid FROM h_wiid WHERE name = '" . $base . "'";
		$this->html->statement($stmt);
	    if ($this->dbObject->Query($stmt) != 1)
            $this->stop("Database error. Could not get wiid entry, which was inserted before.", "getWiid()");
        if ($this->dbObject->ReadRow())
            return $this->dbObject->RowData[0];

        // ??? what's that for a shit?
	    if ($this->dbObject->Query($stmt) != 1)
            $this->stop("Database error. Unknown import problem of wiid entry.", "getWiid()");
    }
	
	
}



// end.
?>
