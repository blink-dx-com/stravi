<?

/*
 * older versions:
 * 0.5.5 -- older valid version
 * */
define("PaXML_WRITERVERSION", "1.0.0");


require_once("PaXMLLib.inc");               // get library for general functions
require_once("PaXMLHTMLOutputExport.inc");  // html output class for export
require_once("PaXMLStack.inc");             // stack
require_once("PaXMLFileOutput.inc");        // xml file writer
require_once("o.LINK.subs.inc");    // used for linkpath_get()
require_once("glob.image.inc");             // used for imgFullPath()
require_once("role.inc");                   // used for role check
require_once("o.SATTACH.subs.inc"); 

// constants PaXML_XMLVERSION and PaXML_SELFURL have to be defined! usuallay
// this should be done PXMLLib.inc
if (!defined("PaXML_XMLVERSION") or !defined("PaXML_SELFURL"))
    die("Internal error. Constants are not defined. Please contact your administrator.");



/**
// class for writing partisan xml. this is the main export class, using several
// class to his job. this class should contain only the functionality, which
// realizes the export algorithm and its rules. helping function shell be
// external
//              features:
//                  export of object structure (including associative elements)
//                  wiid/roid identification
//                  export class data
//                  export option data
//                  supporting contact info of source database
//                  export of projects with update of project elements
//                  compressing of xml file to gzip format
//                  usage of tar packages
//                  export of images and documents (external files)
//                  handling circular references (temporary objects)
//                  fast sax parser for xml
//                  export of wafer
//                  usage of exim flag
//  devnote:    Tar.inc (TAR) is a self written TAR-tool. It's made like unix
//              tar. Generated files should be readable by linux tar command,
//              but there's no warranty given.

//  devnote:    a partisan session is required to run this script, cause using
//              $_SESSION['globals']['img_path'].
			 - use $table_pk_name = PrimNameGet2($table); instead of $table .'_ID'
//  devnote:    steps of exporting:
//                  init class PaXMLWriter
//                  start xml writing
//                  grab as much you want
//                  end xml write
//                  compress file
//                  tar file
//                  finish
//
//
//Usage:     
                session_start(....);
//
//              $table = "ABSTRACT_SUBST";
//              $field = array("ABSTRACT_SUBST_ID" => 1);
//
//              $pxml = new PAXMLWriter(2, $_SESSION['sec']['dbuser'], $_SESSION['sec']['passwd'], $db, $dbtype);
//              $pxml->run($table, $field);
//
 * @package PaXMLWriter
 * @swreq UREQ:0000161: g.paxml-export > HOME UREQ
 * @author rogo, steffen
 * @version $Header: trunk/src/www/pionir/impexp/partisanxml/PaXMLWriter.inc 59 2018-11-21 09:04:09Z $
 *
 */
class PaXMLWriter
{
    var $lib;               // library class
    var $stack;             // grabbing stack
    var $html;              // html output file
    var $xml;               // xml file output class
    var $db;                // db handle
    
    var $exportPath;        // temppath where xml, and other files are located
    var $basedb;
	var $dbversion;			// version of DB e.g. 1.0.3.2

    var $user;
    var $passwd;
	var $db2;
    var $dbtype;
    var $mode;              // viewmode;
    var $attachedExported;  // ---
	var $secretOption;      // true|false : exists the secret column in cct_access? (only clondiag code)
	var $userInfoFlag;		// true|false : 
							// true:
							//	 - export also the user-info CCT_ACCESS => DB_USER_ID
							//   - ignore "exim"-flags in CCT_TABLE
	private $doNotTakeAttachments; // export attachment files ? default: 0 => export attachments
	
    // constructor. initializing html output class, library class, stack class
    // and xml file output class. this function creates the working directory
    // for export (removes it before existing. also the database login is done.
    // no special transaction control is needed.
    //
    // input:           $mode   html output mode
    //          string  $user   db username
    //          string  $_SESSION['sec']['passwd'] db password
    //          string  $db     db service name
    //          string  $dbtype dbtype (may removeable)
    //
    function __construct($user, $passwd, $db, $dbtype, $mode = 1)
    {
        $this->user             = $user;
        $this->passwd           = $passwd;
        $this->db               = $db;
        $this->dbtype           = $dbtype;
        $this->mode             = $mode;
        $this->attachedExported = false;
		$this->userInfoFlag		= false;
		$this->attachLib	    = new cSattachSubs();
		$this->doNotTakeAttachments = 0;
    }
    
    // set $this->doNotTakeAttachments
    function set_noAttach($flag) {
    	$this->doNotTakeAttachments = $flag;
    	
    }
    
	// function: set archive-flag
    function setArchiveFlag() {
		$this->userInfoFlag		= true;
	}
	
	function get_exportPath() {
	    return $this->exportPath;
	}
	function getFilename() {
	    return $this->xml->getFilename();
	}
    
    // wrapper function to run a complete export process in one function. Use
    // this function to export, compress and tar database stuff.
    //
    // input:   string      $table      tablename
    //          string      $filter     array describing the dataset for grabbing
    // output:  -
    //
    function run($table, $filter, $fieldname = null)
    {
        $this->startXML();
        $this->start($table, $filter, $fieldname);
        $this->endXML();
        $this->tar($this->compress());
    }
    


    // begin to write xml
    //
    // input:   -
    // output:  contact id of database contact | null
    //
    function startXML()
    {
        

        // init objects
        $this->html  = new PaXMLHTMLOutputExport($this->mode);   
        $dummyClass  = NULL;                                                   // display output
        $this->lib   = new PaXMLLib($this->html, $this->user, $this->passwd, $this->db, $this->dbtype, $dummyClass);    // paxml info class
        $this->stack = new PaXMLStack($this->html, $this->user, $this->passwd, $this->db, $this->dbtype);          // paxml stack

		$this->lib->setArchiveFlag($this->userInfoFlag);
		
        if (!$this->lib->checkForCorrectVersion())
                $this->html->warning("Missing updated CCT_TABLE.<br>");
        
        if ($this->doNotTakeAttachments>0) {
        	$this->html->infoline('Do not export data files of images, documents.', PaXML_TINYVIEW);
        }
        
        $this->initWorkPath();
        $this->xml = new PaXMLFileOutput($this->exportPath . "/" . "export.xml");                                // paxml file output

        // connect to database. this handle will be used for all grabbing
        // actions
        if (($this->db = logon_to_db($this->user, $this->passwd, $this->db, $this->dbtype)) == null)
            $this->html->stop("Connecting to database failed.");
		
			
        // role check
        $roleRightIdentifier = "f.PaXml_export";
        $roleRight           = role_check($this->db, $roleRightIdentifier);
        if (!($roleRight["execute"] > 0) and $_SESSION['sec']['appuser'] != "root")
        {
            $this->html->stop("No permission to execute this function. You need "
                            . "role right '$roleRightIdentifier' to use paxml "
                            . "export functionality.");
            return;
        }

        // init stack
        $this->stack->reset();

        // get db serial from current database, null means no serial/demo version
        if (($this->basedb = $this->lib->getDBSerial()) == null)
            $this->html->stop("Missing magasin serial. Please contact your "
                            . "administrator.");
							
		// get db version from current database
        if (($this->dbversion = $this->lib->getDBVersion()) == null)
            $this->html->stop("Missing magasin version. Please contact your administrator.");
        
		// exists the secret column in cct_access? (only clondiag code)
		$stmt = "SELECT column_name FROM cct_col_view WHERE table_name = 'CCT_ACCESS' AND column_name = 'SECRET'";
		$this->runSQL($stmt);
		$this->secretOption = false;
		if ($this->db->ReadRow())
			if ($this->db->RowData[0] == "SECRET")
				$this->secretOption = true;


        // output start
		$infoarr = array();
		if ($this->userInfoFlag) {
			$infoarr[] = array("archive-mode", "activated!");
		}
        $this->html->startXML(PaXML_XMLVERSION, PaXML_SELFURL . $this->basedb, PaXML_WRITERVERSION, $infoarr);
        $this->xml->startXML(PaXML_XMLVERSION, PaXML_SELFURL . $this->basedb, $this->dbversion);

        // service of paxml, to offer the contact of exporting database
        // try to get contact object of exporting database. there's no need for
        // import but export is obligate
		$stmt  = "SELECT c.contact_id FROM contact c, h_wiid w "
		       . "WHERE w.contact_id = c.contact_id "
               . "AND w.name = '" . PaXML_SELFURL . $this->basedb . "'";
        $this->runSQL($stmt);
        
        // write contact or warning; may be skip this -> discus
        if ($this->db->ReadRow()) {
        	
			return $this->grab("CONTACT", array("CONTACT_ID" => $this->db->RowData[0]));
        }
        
        return $this->html->warning("Missing contact information of database. "
                                  . "Please inform your administrator. "
                                  . "<!-- startXML() //-->");
    }
    
    
    
    // init working path
    //
    // input:   -
    // output:  $this->exportPath
    //
    function initWorkPath()
    {
        $workPath = $this->lib->getWorkPath();      // init directory structure

        if ($workPath == null)
            $this->html->stop("Misconfigured application settings. Missing "
                            . "'work path'. Please contact your administrator.");

        // define working path as subdirectory of workPath
        $this->exportPath = $workPath . "pxmlexport." . session_id();
        $this->html->workingPath($this->exportPath);

        // if this export path already exists (double pxml call in a session)
        // then remove recursivly all content and the exportPath itself and
        // create a new one
        if (file_exists($this->exportPath))
            if (!$this->lib->removeDirRecursivly($this->exportPath))
                $this->html->stop("Server file handling error. Removing "
                                . "previously used work path at temporary "
                                . "directory failed.");

        if (!mkdir($this->exportPath, 0700))
            $this->html->stop("Creating work path at temporary directory failed."
                            . " Please contact your administrator.");

        return $this->exportPath;
    }
    
    

    // finish to write xml
    //
    // input:   -
    // output:  -
    //
    function endXML()
    {
        $this->xml->endXML();
        $this->html->endXML();
    }

    // wrapper
    //
    // input:   -
    // output:  true if zipped
    //
    function compress()
    {
        $this->html->compress($this->xml->getFilename());
        return $this->lib->compress($this->xml->getFilename());
    }
    
    // wrapper
    //
    // input:   bool    $withZipped     set true if zipped file should be tared
    //                                  else the xml file is tared
    // output:  -
    //
    function tar($withZipped = false)
    {
        $this->html->tar();
        return $this->lib->tar($withZipped, $this->exportPath);
    }

    // finishes the process of creating export output. output html stuff only
    //
    // input:   -
    // output:  -
    //
    function finish()
    {
        $this->html->finish("paxml", true, true, true, $this->attachedExported, $this->exportPath);
    }




	/**
	 * start paxml export actions
	 * @param object $table
	 * @param array $filter
	 * @param string $fieldname
	 * @return array <NULL, boolean, unknown, multitype:>|boolean
	 */
    function start($table, $filter, $fieldname = null)
    {

        switch ($this->lib->getTableType($table))
		{
			case PaXML_OBJECT  :
			case PaXML_OPTION  :
			case PaXML_CLASS   : return $this->grab($table, $filter, $fieldname); break;
			default            :
	        {
	               $this->html->warning("Select table is ot allowed as entry point for "
	                                   . "export. Please choose either a object table "
	                                   . "or a option table or EXTRA_CLASS.");
	               return false;
	        }
		}
    }




    // basic routine for grabbing, use this function to describe, which object
    // you want to grab. DON'T USE IT ANYMORE TO START AN EXPORT. USE FUNCTION
    // 'START()'!
    //
    // input:   string      $table      tablename
    //          string      $filter     array describing the dataset for grabbing
    // output:  -
    // errors:  -
    //
    function grab($table, $filter, $fieldname = null)
    {
        // add 50 minutes more execution time from now on anytime when grab() is
        // called. this avoids, that etxra long exports will be stopped by
        // webserver cause of max execution time.
        
        
        set_time_limit(3000);
        
        // check input -> return null if check fails
        if ($table === null  or !is_string($table)
            or $filter === null or !is_array($filter))
            return null;
            
        // get value of first element in $filter
        reset($filter);
        $id   = current($filter);
        $keys = array_keys($filter);

        $this->html->grab($table);
        
        $info = $this->lib->getTableInfo($table);
        if (!is_array($info)) {
        	$this->html->stop("Missing meta information about table '$table'. "
        			. "Please contact your administrator. "
        			. "<!-- grabObject() //-->");
        }
        
        if ($info["exim"] == "noexport")
        {   
            $this->html->exportObjectDenied($table);
            //$this->html->warning("Data of table '$table' not allowed for export. "
            //                   . "<!-- grab() //-->");
            return null;
        }
        
        // do an special export action depending on table type 
        $tmptype = $this->lib->getTableType($table); 
        //if ( $_SESSION['userGlob']["g.debugLevel"]>0 ) {
        //    echo "<font color=gray>g.DEBUG: table:$table [$id] table-type:$tmptype </font><br>\n";
        //}
		
		
        switch ($tmptype)
		{
		  case PaXML_OBJECT  : return $this->grabObject($table, $id, $keys[0], $fieldname); break;
		  case PaXML_OPTION  : return $this->grabOption($table, $id); break;
		  case PaXML_CLASS   : return $this->grabExtraClass($id, "CLASS"); break;
		  case PaXML_EXTEND  : return $this->grabDependend($table, $id); break;
		  case PaXML_ASSO    : return $this->grabDependend($table, $id); break;
		  case PaXML_SATACH  : 
		  		$parent = $filter["TABLE_NAME"];
		  		return $this->grabAttach($table, $parent, $id); 
				break;
		  case PaXML_META    : return $this->grabMetaTable($table, $id); break;
          case PaXML_VIEW    : break;        // no warning (steffen)
		  case PaXML_UNKNOWN : break;        // display a warning?
		  default            : break;        // this case will never, should never occure
		}
    }

	
	// copy attachment file 
	function _copyAttachment( 
		$attach, 
		$artificialFilename, 
		$objinfo	// $rowdata["NAME"]
		) {
	

		if (file_exists($attach)) {
			if (!copy($attach, $this->exportPath . "/" . $artificialFilename))
				$this->html->warning("Could not grab file '" . $objinfo 
									. "'! <!-- server filename: $attach //-->");
			else
			{
				$this->attachedExported = true;
				$this->html->fileCopied($objinfo, $artificialFilename);
			}
		} else {	
			
			if ( $objinfo=="" ) $this->html->noFile();
			/*
			if ($rowdata["NAME"] != "")
				$this->html->warning("File not accessable or not existing. Could "
									. "not grab file '" .  $rowdata["NAME"]
									. "'! <!-- server filename: $attach //-->");
			*/
		}
	}

    // this function grabs an object with all its required sub data. if the
    // object has been already exported, only a link will be set in xml. to
    // detect possible double export the stack was made. any object has a table
    // called CCT_ACCESS. this a special table, handled in this function. A
    // object may have links to other object, and option data (controlled
    // vocabular). Almost any object has functionality of extra class and
    // objects may have associative data (mass data).
    //
    // following object have to be handled in a special way:
    //      PROJ using PROJ_HAS_ELEM
    //
    // input:   string      $table      table name
    //          number      $id         database id
    //          string      $field      fieldname for criteria if not default (tablename + _ID)
    //          string      $fieldNamw  fieldName for export, because not generic
    // output:  -
    // errors:  -
    //
    function grabObject($table, $id, $field = null, $specialFieldName = null)
    {
        
    	$table_pk_name = PrimNameGet2($table);
               
        // check input
        if ($table == null or !is_string($table) or $id == null)
            return null;


        // get table description
        $info = $this->lib->getTableInfo($table);
        if (!is_array($info)) {
            $this->html->stop("Missing meta information about table '$table'. "
                            . "Please contact your administrator. "
                            . "<!-- grabObject() //-->");
        }
        
        if ($info["exim"] == "noexport") {   
            $this->html->exportObjectDenied($table);
            return null;
        }

        // generate fieldname if not given
        $field = ($field == null) ? $table_pk_name : $field;
  
        // define filterField
        if ( $table == "PROJ" ) {
			$filterField = $field;
        } else {
			$parent      = $this->lib->getParentTable($table);
			$filterField = ($parent == null) ? $field : PrimNameGet2($parent);
		}
  
		//TBD: analyse EXPORT_DENY
		$export_denied = $this->lib->getObjExportDeny($table, $id);
		if ($export_denied['exim']== "noexport") {
		    $this->html->exportObjectDeniedSingle($table, $id, $export_denied['info']);
		    return null;
		}
		
		$rows = $this->getObjects($table, $field, $id, $filterField);
		
        // run at any object from list.
		for ($row = 0; $row < count($rows); $row++)
		{
			$rowdata = &$rows[$row];
		
			if ($this->secretOption and $rowdata["secret"] > 0)
			{
				if (   $table == "ABSTRACT_PROTO"
					or $table == "CONCRETE_PROTO")
					$this->html->stop("Illegal state. Table $table is not allowed for confidentiality."
									. "<!-- grabObject() //-->");					
			
				$this->html->warning("Confidential object grabbed and replaced by alias object with id " . $rowdata["secret"] .".");
				// $replaceObjects = $this->getObjects($table, $field, $rowdata["secret"], $filterField); // what about filterField here
				$replaceObjects = $this->getObjects($table, $field, $rowdata["secret"], $table_pk_name); // what about filterField here
				if (count($replaceObjects) == 0)
					$this->html->stop("Missing replacement object for confidential data."
									. "<!-- grabObject() //-->");					
				
				$rows[$row]     = $replaceObjects[0];
				$rowdata = &$rows[$row];  // update pointer
			}
			
			$attach = null;       // add now bugfix: $rowdata["wiid"], $rowdata["roid"]
			$tmpObjInfo = array($table_pk_name => $rowdata["id"]);
			if ($table=="PROJ") {
				 $tmpObjInfo = array("name" => "'".htmlspecialchars($rowdata["NAME"])."'");
			}
            $this->html->startObject(
				$table, 
				$tmpObjInfo,
				$rowdata["wiid"], 
				$rowdata["roid"]);

            // was this object already exported/touched ?  yes, write a xml
            // reference. no, describe whole object
            $stacked = $this->stack->find($table, array($table_pk_name => $rowdata["id"]));
            if ($stacked)
            {
                $this->html->alreadyGrabbedObject();
                $this->xml->refObject($table, $rowdata["wiid"],
                                      $rowdata["roid"], $specialFieldName);
                return true;
            }

            // attach a file? check for an object with attached file. file must
            // exists.
            if (!$this->doNotTakeAttachments)
            {
                $attach             = ($table == "LINK") ? linkpath_get($rowdata["id"]) : $attach;
                $attach             = ($table == "IMG") ? imgPathFull($rowdata["id"]) : $attach;
                $artificialFilename=NULL; // or what other Name ???
                $artificialFilename = ($attach != null and file_exists($attach)) ? $table . "." . uniqid(null) : $artificialFilename;
            }
            
            $this->html->exportObject($table);
			
			
			$infoarr = array( 
				"wiid"	   =>$rowdata["wiid"], 
				"roid"	   =>$rowdata["roid"],
				"crea_date"=>$rowdata["crea_date"],
				"mod_date" =>$rowdata["mod_date"],
				"special"  =>$specialFieldName,
				"attach"   =>$artificialFilename
				);
				
			if ($this->userInfoFlag) {
				$infoarr["creator"]  = $rowdata["creator"];
				$infoarr["modifier"] = $rowdata["modifier"];
			}
			
            $this->xml->startObject(
				$table, 
				$infoarr
				);

            // mark it now, that this object has been touched now
            
            // circle reference will be recognized by this because, object is
            // marked as touched, but not described completly in xml. if this
            // object occures again during grabing this object, it will be
            // already in stack and set as found. this is tho stop criteria for
            // to avoid endless loops
			$this->stack->put($table, array($table_pk_name => $rowdata["id"]));

            // programming beautification, makes no sense actually
			/*
			$wiid 		= $rowdata["wiid"];
			$roid		= $rowdata["roid"];
			$crea_date  = $rowdata["crea_date"];
			$mod_date	= $rowdata["mod_date"];
			*/
			$r_id		= $rowdata["id"];
			$result     = array();
			$hresult    = array();
   
			// map values to columns
			foreach ($rowdata as $key => $value)
				if ( $key != "wiid" && 
					 $key != "roid" && 
					 $key != "crea_date" && 
					 $key != "mod_date" &&
					 $key != "creator" && 
					 $key != "modifier" && 
					 $key != "access" && 
					 $key != "id" && 
					 $table_pk_name != $key )
						$result[$key] = $value;

            // object uses a class? export class now. write class definition
            // here.
            $classname = $this->grabExtraClass($result["EXTRA_OBJ_ID"]);

			// is there any option table (h table)? collect fields here now.
            // and grab them.
			foreach($result as $fieldName => $fieldValue) {
            
                // echo "DEBUGI: table:$table field:$fieldName INFO:".$info["columns"][$fieldName]["fk_table"]."<br>";
            
				if ($this->lib->getTableType($info["columns"][$fieldName]["fk_table"]) == PaXML_OPTION)
				{
					
                    $hresult[$fieldName] = $this->grab($info["columns"][$fieldName]["fk_table"],
                                                       array($fieldName => $fieldValue));
                    if ($hresult[$fieldName] == null and $info["columns"][$fieldName]["not_null"] == 1)
                        $this->html->stop("Conflict at meta informations. "
                                        . "Structural problem at database level. "
                                        . "A link has been set to a table. "
                                        . "Either the link is wrong or the linked "
                                        . "table is not allowed for export and the "
                                        . "foreign key is not allowed to be empty. "
                                        . "Please contact your administrator. "
                                        . "Details:  (659) field: $fieldName -> "
                                        . $info["columns"][$fieldName]["fk_table"]
                                        , "grabObject (659)");
                }
            }
            // download images and linked objects. copy them to exportPath,
            // which is a subpath of current working path. give the files
            // artificial names.
            if ($table == "IMG" or $table == "LINK"
                and !$this->DoNotTakeAttachments and $attach != null
                and $artificialFilename != null)
            {
                $this->_copyAttachment($attach, $artificialFilename, $rowdata["NAME"]);
            }

            // start to write parameters in xml
            $this->xml->startParameters($classname);
			foreach($result as $name => $value)
            {
                $special = null;
                // if (foreign key to business object and foreign key object is not this business object)
                // {}
                // elseif (foreign key to option table and value is not null)
                // {}
                // elseif (name not extra_obj_id and has an value and name is not PRO_PROJ_ID)
                
                /*echo "DEBUG:qbi grabObject: line:669 table:$table name:$name val:$value info:".$info["columns"][$name]["fk_table"]." type:";
                echo $this->lib->getTableType($info["columns"][$name]["fk_table"]);
                if ($name=="CONTACT") {
                    echo "INFO: ";
                    print_r($info);
                }
                echo "<br>";
                */
                
                if ($this->lib->getTableType($info["columns"][$name]["fk_table"]) == PaXML_OBJECT AND 
                   ($info["columns"][$name]["fk_table"] != "PROJ")  )
                {   
                    if ($info["columns"][$name]["fk_table"] == strtoupper($table)) {
                        // circular reference possible !!!
                        if ( $this->html->viewMode > PaXML_MINVIEW) echo "<font color=red>WARNING:</font> table:$table circular reference possible<br>\n";
                    }
                    // grab sub object
                    // if (name of field not made by generic concept (table name + _ID)
                    //     mark this field as special
                    if ($info["columns"][$name]["fk_table"] . "_ID" != $name)
                        $special = $name;

                    // grab table behind foreign key
                    
                    $fk_table_pk = PrimNameGet2($info["columns"][$name]["fk_table"]);
                    $grabresult  = $this->grab($info["columns"][$name]["fk_table"],
                                              array( $fk_table_pk => $value),
                                              $special);
                    
                    if ($grabresult == null and $info["columns"][$name]["not_null"] == 1)
                        $this->html->stop("(1) Conflict at meta informations. "
                                        . "Structural problem at database level. "
                                        . "A link has been set to a table. Either "
                                        . "the link is wrong or the linked table "
                                        . "is not allowed for export and the foreign "
                                        . "key is not allowed to be empty. Please "
                                        . "contact your administrator. Details: (715) "
                                        . "field: $name - " . $info["columns"][$name]["fk_table"]
                                        , "grabObject (715)");
                }
                elseif ($this->lib->getTableType($info["columns"][$name]["fk_table"]) == PaXML_OPTION
                    && $hresult[$name] != null)
                {
                    $this->html->exportParameter($info["columns"][$name]["fk_table"],
                                                 $hresult[$name]);
                    $this->xml->writeParameter($info["columns"][$name]["fk_table"],
                                               $hresult[$name], "option");
                }
                elseif ($name != "EXTRA_OBJ_ID" && $value != null && $name != "PRO_PROJ_ID")
                {
                    $this->html->exportParameter($name, $value);
                    $this->xml->writeParameter($name, $value);
                }
                
                // columns called EXTRA_OBJ_ID and PRO_PROJ_ID and with empty
                // values are not written
            }
            $this->grab_Varios($table, $id);
            $this->grabExtraValues($result["EXTRA_OBJ_ID"]);
            $this->xml->endParameters();
            
 	      	// handle sub-PROJ. this is a special solution. ok, I could
            // implement this with proj_has_elem, but we (steffen and me)
            // discussed this and decided to take this solution (binary tree)
            // because of an better performance with root proj. grab all
            // projects, which current project as parent. by this all
            // normal subdirectories should be exported.
            if ($table == "PROJ")
				$this->grab("PROJ", array("PRO_PROJ_ID" => $r_id));

				
			
			// get possible attachments
			$this->grab( "SATTACH", array("OBJ_ID" => $r_id, "TABLE_NAME"=>$table ) );
				
				
            // get all extended data and associative tables which are child
			// of this object
			$tableChilds = $this->lib->getTableChilds($table);
			if ($tableChilds != null) {
				foreach($tableChilds as $child) {
					$this->grab($child, array($table_pk_name => $r_id));
				}
			}
            // puuh, that's it
            $this->html->endObject();
            $this->xml->endObject();
        }
        
        return true;
    }
    
    
	
	/**
	 * get object information
	 * @param string $table
	 * @param string $field
	 * @param mixed $id
	 * @param array $filterField
	 * @return multitype:unknown
	 */
	function getObjects($table, $field, $id, $filterField)
	{
		
		$table_pk_name = PrimNameGet2($table);
		
        // get table description
        if (!($info = $this->lib->getTableInfo($table)))
            $this->html->stop("Missing meta information about table '$table'. "
                            . "Please contact your administrator. "
                            . "<!-- grabObject() //-->");

		// get object from database. create sql statement. take any column
        // except if column name is CCT_ACCESS_ID or column name is the primary
        // key name and column name is ABSTRACT_SUBST_ID at abstract substance
        // coherency and
        $stmt = "SELECT ";
        foreach($info["columns"] as $name => $column)
            if ($name != "CCT_ACCESS_ID"
                and $name != $table_pk_name
                and $column["exim"] != "noexport")
            {
                if (substr($column["datatype"],0,4)=='date')
                    $stmt .= $this->db->Sql2DateString("bo.$name", 1) . ", ";
                else
                    $stmt .= "bo.$name, ";
            }

        $table_pk_name = PrimNameGet2($table);
        $stmt .= $this->db->Sql2DateString("ca.crea_date", 1) . ", "
               . $this->db->Sql2DateString("ca.mod_date", 1)  . ", "
               . "ca.wiid, "
			   . "ca.roid, "
			   . "ca.db_user_id, "
			   . "ca.db__db_user_id, "
               . "bo.cct_access_id, "
			   . "bo.". $table_pk_name . " "
			   . (($this->secretOption) ? ", ca.secret " : "")
               . "FROM $table bo, CCT_ACCESS ca "
		       . "WHERE ca.cct_access_id = bo.cct_access_id "
               . "AND bo." . $filterField . " = $id";
		$this->runSQL($stmt);
        
		
		
        // get results -> usually only one
        $rows = array();
        while ($this->db->ReadRow())
		{
			$index = 0;
			$rowdata=array();
   
            // map db result data to $result array.
            //
            // ATTENTION: the if condition has to be the same like above! if you
            // change something at this condition, you to change also above.
			foreach($info["columns"] as $name => $column)
                if ($name != "CCT_ACCESS_ID"
                    and $name != $table_pk_name
                    and $column["exim"] != "noexport")
                    $rowdata[$name] = $this->db->RowData[$index++];				// set values
       
            // handle special columns
			$rowdata["crea_date"] = $this->db->RowData[$index++];			// set my values ...
			$rowdata["mod_date"]  = $this->db->RowData[$index++];
			$rowdata["wiid"]      = $this->lib->getWiidByID($this->db->RowData[$index++]);
            $rowdata["roid"]      = $this->db->RowData[$index++];
			$rowdata["creator"]   = $this->db->RowData[$index++];
			$rowdata["modifier"]  = $this->db->RowData[$index++];
			$rowdata["access"]    = $this->db->RowData[$index++];
			$rowdata["id"]        = $this->db->RowData[$index++];
			if ($this->secretOption)
				$rowdata["secret"] = $this->db->RowData[$index++];
				
			if ($this->userInfoFlag) {
				$rowdata["creator"]  = ($rowdata["creator"] != null)  ? $this->lib->getUserNick($rowdata["creator"]) : NULL;
				$rowdata["modifier"] = ($rowdata["modifier"] != null) ? $this->lib->getUserNick($rowdata["modifier"]) : NULL;
			}
            $rowdata["wiid"]      = ($rowdata["wiid"] == null) ? PaXML_SELFURL . $this->basedb : $rowdata["wiid"];
            $rowdata["roid"]      = ($rowdata["roid"] == null) ? $rowdata["id"] : $rowdata["roid"];
			
            $rows[]               = $rowdata;
		}

		return $rows;
	}
	
	/**
	 * get VARIO vals
	 * @param string $table
	 * @param int $id
	 */
	function grab_Varios($table, $id) {
	    
	    $stmt = "SELECT KEY,VALUE FROM S_VARIO "
	        . "WHERE TABLE_NAME='".$table."' and OBJ_ID='".$id."' order by KEY";
	    $this->runSQL($stmt);
	                
	    $vario_vals=array();
	    while ($this->db->ReadRow()) {
	          $name  = $this->db->RowData[0];
	          $value = $this->db->RowData[1];
	          
	          if ($value!=='' and $value!==NULL) {
	              $vario_vals[]=array('key'=>$name, 'val'=>$value);
	              
	              $this->html->exportParameter($name, $value);
	              $this->xml->writeParameter($name, $value, PaXMLTag_PARAMETER_VARIO);
	          }
	    }
	    
	    if (empty($vario_vals)) return null;
	    
	    
	    
	}
    
    // grab class. classes and attributes are handle in special way by this
    // function. class including attributes will be grab now. double export will
    // be check in the same way like objects, using stack.
    //
    // input:   number      $id         extra class id / extra obj id
    //          string      $idType     if "EXTRA_OBJ" then $id = extra obj id
    // output:  -
    // errors:  -
    //
    function grabExtraClass($id, $idType = "EXTRA_OBJ")
    {
        if ($id == null or !is_numeric($id))
            return null;

        // special pre step: if id represents extra obj id, so get extra class
        // by extra obj id
        if ($idType == "EXTRA_OBJ")
        {
	        $stmt = "SELECT ec.extra_class_id, ec.name "
		           . "FROM extra_obj eo, extra_class ec "
		           . "WHERE eo.extra_class_id = ec.extra_class_id "
                   . "AND eo.extra_obj_id = '$id'";
		    $this->runSQL($stmt);

		    // return
		    if ($this->db->ReadRow())
            {
                $id = $this->db->RowData[0];
                $classname = $this->db->RowData[1];
            }
            else
                return null;
        }
        else
        {
            $stmt = "SELECT name FROM extra_class where extra_class_id = $id";
 		    $this->runSQL($stmt);

		    // return
		    if ($this->db->ReadRow())
                $classname = $this->db->RowData[0];
            else
                return null;
        }

        // now start to export class
        $this->html->startClass($classname);

		// this dataset touched ??? return. easy case. it's not mandatory to
        // write class definition again if it has been done. already.
        $stacked = $this->stack->find("EXTRA_CLASS", array("EXTRA_CLASS_ID" => $id));
        if ($stacked)
		{
            $this->html->alreadyGrabbedClass();
        	return $classname;
        }

        $this->html->exportClass();
		if (!($info = $this->lib->getTableInfo("EXTRA_CLASS")))
            $this->html->stop("Missing meta information about table 'EXTRA_CLASS'. "
                            . "Please contact your administrator.  <!-- grabExtraClass() //-->");

        if ($info["exim"] == "noexport")
        {
            $this->html->warning("Data of table 'EXTRA_CLASS' not allowed for export. "
                . "Please contact administrator. <!-- grab() //-->", '', PaXML_MINVIEW);
            return null;
        }

        // ask for extra class by id, get data for all possible columns
		$stmt = "SELECT ";
		foreach($info["columns"] as $name => $column)			// foreach columns in $info
			if ($info["columns"][$name]["pk"] == 0 				// exclude columns
			    && $name != "NAME"
			    && $name != "TABLE_NAME"
                and $column["exim"] != "noexport")
            {
                if (substr($column["datatype"],0,4) == "date")
                    $stmt .= $this->db->Sql2DateString($name, 1) . ", ";
                else
				    $stmt .= "$name, ";								// add field
            }
		$stmt .= "name, table_name FROM extra_class WHERE extra_class_id = '$id'";
        $this->runSQL($stmt);
        
        $result=NULL;

        if ($this->db->ReadRow())
        {
            // get all class data, use same if criteria like above
   			$index = 0;

            // ATTENTION: the if condition has to be the same like above! if you
            // change something at this condition, you to change also above.
			foreach($info["columns"] as $name => $column)
				if ($info["columns"][$name]["pk"] == 0
    				and $name != "NAME"
				    and $name != "TABLE_NAME"
                    and $column["exim"] != "noexport")
				    $result[$name] = $this->db->RowData[$index++];
			$classname = $this->db->RowData[$index++];
			$tablename = $this->db->RowData[$index++];
   
            $this->xml->startClass($classname, $tablename);
			foreach($result as $name => $value)
            {
                $this->html->exportParameter($name, $value);
                $this->xml->writeParameter($name, $value);
            }
            
			// grab all attributes of this class now
            if (!($info = $this->lib->getTableInfo("EXTRA_ATTRIB")))
                $this->html->stop("Missing meta information about table 'EXTRA_ATTRIB'. "
                                . "Please contact your administrator.  <!-- grabExtraClass() //-->");

            // ask db for any attribute of extra class, with all parameters per attribute
		    $stmt = "SELECT ";
		    foreach($info["columns"] as $name => $column)													// foreach column
			    if ($name != "EXTRA_ATTRIB_ID" && $name != "EXTRA_CLASS_ID"
                    && $name != "APP_DATA_TYPE_ID" && $column["exim"] != "noexport")	// exclude columns
                {
                    if (substr($column["datatype"],0,4) == "date")
                        $stmt .= $this->db->Sql2DateString("ea.$name", 1) . ", ";
                    else
				        $stmt .= "ea.$name, ";
                }																	// add fields

		    $stmt .= "adt.name FROM extra_attrib ea, app_data_type adt "
		           . "WHERE adt.app_data_type_id = ea.app_data_type_id "
                   . "AND ea.extra_class_id = $id";
            $this->runSQL($stmt);

            // write for every attribute the parameters
            while($this->db->ReadRow())
            {
                // save results
    			$index = 0;

                // ATTENTION: the if condition has to be the same like above! if you
                // change something at this condition, you to change also above.
			    foreach($info["columns"] as $name => $column)
				    if ($name != "EXTRA_ATTRIB_ID" && $name != "EXTRA_CLASS_ID"
                        && $name != "APP_DATA_TYPE_ID" and $column["exim"] != "noexport")
					   $result[$name] = $this->db->RowData[$index++];
			    $result["APP_DATA_TYPE"] = $this->db->RowData[$index++];

                // write attribute
                $this->html->startAttribute($result["NAME"]);
                $this->xml->startAttribute($result["NAME"]);
                foreach($result as $name => $value)
				    if ($name != "NAME" && $value != null)
                    {
                        $this->html->exportParameter($name, $value);
                        $this->xml->writeParameter($name, $value);
                    }
                $this->html->endAttribute();
                $this->xml->endAttribute();
            }

            $this->html->endClass();
            $this->xml->endClass();
            
			// mark this dataset as touched. write to stack
			$this->stack->put("EXTRA_CLASS", array("EXTRA_CLASS_ID" => $id));
   
            return $classname;
        }

		$this->html->stop("Extra class attach to object couldn't be found. "
                        . "<!-- ecid: $id //--><!-- grabExtraClass() //-->");
    }
    
    
    
    
    // this function is used by any other structure using extra class
    // functionality. it's grabbing extra values and writing parameter tags.
    //
    // input:   number      $eoid         extra obj id
    // output:  -
    //
    function grabExtraValues($eoid)
    {

		// check parameter
		if ($eoid == null)
			return;

		// get meta info
		if (!($info = $this->lib->getTableInfo("EXTRA_OBJ")))
            $this->html->stop("Missing meta information about table 'EXTRA_OBJ'. "
                            . "Please contact your administrator.  <!-- grabExtraValues() //-->");

        if ($info["exim"] == "noexport")
        {
            $this->html->warning("Data of table 'EXTRA_OBJ' not allowed for export. "
                . "Please contact administrator. <!-- grabExtraValues() //-->", '', PaXML_MINVIEW);
            return null;
        }

        $attribs=NULL;
        // get a list of all attributes
        $stmt = "SELECT ea.name, ea.map_col FROM extra_attrib ea, extra_obj eo WHERE eo.extra_class_id = ea.extra_class_id and eo.extra_obj_id = $eoid";
        $this->runSQL($stmt);
		while ($this->db->ReadRow())
            $attribs[$this->db->RowData[1]] = $this->db->RowData[0];
        if ($attribs == null)
            return;
		
        // values from extra_obj
        $stmt   = "";
        $prefix = "SELECT";
        foreach($attribs as $mapcol => $name)
        {
            $stmt .= $prefix . " $mapcol";
            $prefix = ", ";
        }
        $stmt .= " FROM extra_obj WHERE extra_obj_id = $eoid";
        $this->runSQL($stmt);
        $this->db->ReadRow();
        // write data to xml
        $index = 0;
        foreach($attribs as $name)
		{
			if ($this->db->RowData[$index] != null)
            {
                $this->html->exportParameter($name, $this->db->RowData[$index]);
                $this->xml->writeParameter($name, $this->db->RowData[$index], "class");
			}
			$index++;
		}
    }
    
    
    
    // option tables are controlled vocabulary in fact. but option tables are
    // very similiar to object, except they do  not have an entry to cct_access.
    // Else they may extra class, and associative table. an option table may not
    // have a link to another option table and to to an object.
    //
    // input:   string      $table      option table name
    //          number      $id         database id
    // output:  null | name field of database entry
    //
    function grabOption($table, $id)
    {
        
        
        // check parameters
        if ($table == null or !is_string($table) or $id == null)
            return null;

		// ask for data of lookup table
		if (!($info = $this->lib->getTableInfo($table)))
            $this->html->stop("Missing meta information about table '$table'. "
                            . "Please contact your administrator. <!-- grabOption() //-->");

        if ($info["exim"] == "noexport")
        {   
            $this->html->exportObjectDenied($table);
            return null;
        }
        
		// create statement to select option by id
		$pkname = $info['pk'];
		$stmt = "SELECT ";
		foreach($info["columns"] as $name => $column)				// foreach column
			if ( $pkname != $name and $column["exim"] != "noexport")		// column excluding condition
            {
                if (substr($column["datatype"],0,4)=='date')
                    $stmt .= $this->db->Sql2DateString("$name", 1) . ", ";
                else
				    $stmt .= "$name, ";									// add field
            }
		$stmt = substr($stmt, 0, strlen($stmt) - 2);				// remove ,
		$stmt .= " FROM $table WHERE " . $pkname . " = $id";		// end of stmt
        $this->runSQL($stmt);
		
		
		$mainNameCol=importantNameGet2($table);
  		if ($mainNameCol=='') $mainNameCol='NAME'; // fallback
  		
  		$result=NULL;
                
        if ($this->db->ReadRow())
        {
		    // map database column and value to an array
		    $index = 0;

            // ATTENTION: the if condition has to be the same like above! if you
            // change something at this condition, you to change also above.
            foreach($info["columns"] as $name => $column)					// foreach column
			    if ($pkname != $name and $column["exim"] != "noexport")								// column excluding condition
				    $result[$name] = $this->db->RowData[$index++];	// set value
            
            $stacked = $this->stack->find($table, array($pkname => $id));
			
            if ($stacked)
                return $result[$mainNameCol];
            
            
		    // option tables may have dynamic parameters, so do this
		    // handle extra class, means write class definition if mandatory
		    $classname = $this->grabExtraClass($result["EXTRA_OBJ_ID"]);
            $childs = $this->lib->getTableChilds($table);
			
            // further description needed or already grabbed. most option tables
            // have only an id and name field. in this case and if option has
            // been already grabbed write short xml tag ...
			
			// bugfix:  if (count($result) == 2 and  ... --> if (count($result) == 1 and 
            if (count($result) == 1 and $classname == null 
				and $childs == null and $table!='DB_USER')
    		{   
                $this->html->startOption($table, $result[$mainNameCol]);
                $this->xml->refOption($table, $result[$mainNameCol]);
                $this->html->endOption();
    		    $this->stack->put($table, array($pkname => $id));
                return $result[$mainNameCol];
            }

            // ... else describe option, beginning here 
            $this->html->startOption($table, $result[$mainNameCol]);
            $this->xml->startOption($table, $result[$mainNameCol]);   
            $this->xml->startParameters($classname);

		    // write parameter
		    foreach($result as $name => $value)
    			if ($name != "EXTRA_OBJ_ID" && $name != "NAME" && $value != null)
                    $this->xml->writeParameter($name, $value);

    		// and now the extra class values
		    $this->grabExtraValues($result["EXTRA_OBJ_ID"]);
            $this->xml->endParameters();
            
            
            // these lines are made for H_EXP_RAW_DESC, but of course in generic
            // way of programming
            if ($childs != null) {
                
                foreach($childs as $child)
                    $this->grab($child, array($pkname => $id));
            }
                
            
            $this->html->endOption();
            $this->xml->endOption();

    		// keep in mind as touched
		    $this->stack->put($table, array($pkname => $id));
            return $result[$mainNameCol];
        }

        // this should never happen
        $this->html->stop("There is a link to table '$table' (id: $id), but this "
                        . "dataset doesn't exists. This is an illegal state. Please "
                        . "inform your administrator! <!-- grabOption() //-->");
    }
    
    
    
    // works for extended data like for associative data, both are depended from
    // a business object or an option table
    //
    // input:   string      $table      associative table name
    //          number      $id         database id
    // output:  -
    //
    function grabDependend($table, $id)
    {
        // check parameters
        if ($table == null or !is_string($table) or $id == null)
            return null;

        // table in stack (touched)? not necessary because object has been
        // catched before, associative table can never be handled alone

		// ask for data of associative table structure
		if (!($info = $this->lib->getTableInfo($table)))
            $this->html->stop("Missing meta information about table '$table'. "
                            . "Please contact your administrator. <!-- grabDependend() //-->");

        if ($info["exim"] == "noexport")
        {
            $this->html->warning("Data of table '$table' not allowed for export. "
                . "Please contact administrator. <!-- grabDependend() //-->", '', PaXML_MINVIEW);
            return null;
        }

		// any associative has a parent table, so get parent table for filter
        // criteria
		if (($parent = $this->lib->getParentTable($table)) == null)
            $this->html->stop("Misconfigured application settings. Missing parent "
                            . "for table '$table'. Please contact your administrator. "
                            . "<!-- grabDependend() //-->");

		// statement: select an associative table filtered by one part of pk
		$stmt = "SELECT ";
		$parinfo = $this->lib->getTableInfo($parent);
		$parPkname = $parinfo['pk'];
		foreach ($info["columns"] as $name => $column)	{			// foreach column
			if ($name != $parPkname and $column["exim"] != "noexport")	// column excluding condition
            {
                if (substr($column["datatype"],0,4)=='date')
                    $stmt .= $this->db->Sql2DateString("$name", 1) . ", ";
                else
				    $stmt .= "$name, ";									// fieldname
            }
		}
        $stmt = substr($stmt, 0, strlen($stmt) - 2) . " FROM $table WHERE " . $parPkname . " = '$id'";
		$this->runSQL($stmt);

        // collect all data into an array -> this may cause memory problems at
        // very very very huge datasets, maybe something like x > 1million
        // finally this may be a critical function for hardware limits
		$rowdata = array();
  		while($this->db->ReadRow())
			array_push($rowdata, $this->db->RowData);
   
        // no data => never write this tag 'elements', because loop will never run
        if (0 < count($rowdata))
        {
            $this->html->startRelationships($table, count($rowdata));
            $this->xml->startRelationships($table);
        }
        
        $result=NULL;
        $hresult=NULL;
        
		for ($row = 0; $row < count($rowdata); $row++)
		{
			// map value to column
			$index = 0;
			foreach($info["columns"] as $name => $column)		                      // foreach column
				if ($name != $parPkname and $column["exim"] != "noexport")	  	  // excluding condition (same like above)
					$result[$name] = $rowdata[$row][$index++];	                      // set value

            // handle extra class, means write class definition if mandatory
			$classname = $this->grabExtraClass($result["EXTRA_OBJ_ID"]);
   
			// there is any system table write here sytsem table definition
			foreach($result as $name => $value)
				if ($this->lib->getTableType($info["columns"][$name]["fk_table"]) == PaXML_OPTION)
				{
				    $fieldName = $name;
					$hresult[$name] = $this->grab($info["columns"][$name]["fk_table"], array($name => $value));
                    if ($hresult[$fieldName] == null and $info["columns"][$fieldName]["not_null"] == 1)
                        $this->html->stop("Conflict at meta informations. Structural "
                            . "problem at database level. A link has "
                            . "been set to a table. Either the link "
                            . "is wrong or the linked table is not "
                            . "allowed for export and the foreign key "
                            . "is not allowed to be empty. Please contact "
                            . "your administrator. Details: (1357) "
                            . "table: ".$table." field: $fieldName -> " . $info["columns"][$fieldName]["fk_table"]
                            , 'grabDependend (1357)');
                }

            // the proj is a link -> no export of linked projects! (steffen)
            if ($table == "PROJ_HAS_ELEM" and $result["TABLE_NAME"] == "PROJ")
                continue;
                
			// write out one relation if parameter is a foreign key
			// so grab data of foreign table
            $this->html->startRelationship();
            $this->xml->startRelationship($classname);
			$index = 0;
   
            // special case PROJ_HAS_ELEM
            if ($table == "PROJ_HAS_ELEM")
            {
                // write parameter normally
    			foreach($result as $name => $value)
				    if ($name != "TABLE_NAME" && $name != "PRIM_KEY" && $name != "PROJ_ID" && $value != null)
                    {
                        $this->html->exportParameter($name, $value);
                        $this->xml->writeParameter($name, $value);
                    }

                // PROJ_HAS_ELEM describes a link to object in a special way,
                // resolve this link
                $this->grab($result["TABLE_NAME"], array( PrimNameGet2($result["TABLE_NAME"]) => $result["PRIM_KEY"]));
            }
            elseif ($table != "PROJ_HAS_ELEM")
    			foreach($result as $name => $value)
    			{
                    $special = null;
                	$fkTableLoop = $info["columns"][$name]["fk_table"];
					$tableType   = $this->lib->getTableType($fkTableLoop);
					
					
                    // for any field with an value, which is ...
                    // ... a foreign key to an business object
                    if ($tableType == PaXML_OBJECT)
    				{
                        // if foreign_key field is not generic (table + '_ID'), then
                        // tell it to the function and set special
                        if ($fkTableLoop . "_ID" != $name)
                            $special = $name;

                        // grab this business object
                        $grabresult = $this->grab($fkTableLoop,
                                                  array(  PrimNameGet2($fkTableLoop) => $value), $special);

                        if ($grabresult == null and $info["columns"][$name]["not_null"] == 1)
                            $this->html->stop("Conflict at meta informations. Structural "
                                            . "problem at database level. A link has "
                                            . "been set to a table. Either the link "
                                            . "is wrong or the linked table is not "
                                            . "allowed for export and the foreign "
                                            . "key is not allowed to be empty. Please "
                                            . "contact your administrator. Details:  (1416) "
                                            . "table: ".$table." field: $name -> " . $fkTableLoop
                                            , '(1416) grabDependend');
                    }
                    // ... a foreign to a option table and there is a value (of course)
    				elseif ($tableType == PaXML_OPTION && $hresult[$name] != null)
                    {
                        $this->html->exportParameter($fkTableLoop, $hresult[$name]);
                        $this->xml->writeParameter($fkTableLoop, $hresult[$name], "option");
                    }
                    else if ($name != "EXTRA_OBJ_ID" && $value != null)
				    {
                        $this->html->exportParameter($name, $value);
                        $this->xml->writeParameter($name, $value);
                    }
                }

			// write extra class parameter
			$this->grabExtraValues($result["EXTRA_OBJ_ID"]);

            $this->html->endRelationship();
            $this->xml->endRelationship();
        }

        if (0 < count($rowdata))
        {
            $this->html->endRelationships();
            $this->xml->endRelationships();
        }
        
        return true;
    }
	
	// write data for ONE dataset
	function _attachWriteData( 
				&$result, 		// name => value
				$attach,  		// file-name on server
				$artificialFilename,
				$parent_info
		) {
	
		//$classname = "";
		$this->html->startRelationship();
		$this->xml->startRelationAttach($artificialFilename);
	
		foreach( $result as $name=>$value) {
		
			if ($value != null) {
				$this->html->exportParameter($name, $value);
				$this->xml->writeParameter  ($name, $value);
			}
		}
		
		
		$this->html->endRelationship();
		$this->xml->endRelationship();
		
		if ( !$this->DoNotTakeAttachments 
				and $attach != null
                and $artificialFilename != null) {
                $this->_copyAttachment($attach, $artificialFilename, $parent_info);
        }
	
	}

	// grep lesser-associative data (like SATTACH)
    //
    // input:   string      $table      associative table name e.g. SATTACH
	//			string		$parent     MOTHER-table
    //          number      $id         database id
    // output:  -
    //
    function grabAttach($table, $parent, $id)
    {
		$FUNCNAME="grabAttach()";
		
        // check parameters
        if ($table == null or $parent==null or $id == null)
            return null;

        // table in stack (touched)? not necessary because object has been
        // catched before, associative table can never be handled alone

		// ask for data of associative table structure
		if (!($info = $this->lib->getTableInfo($table)))
            $this->html->stop("Missing meta information about table '$table'. "
                            . "Please contact your administrator. <!-- ".$FUNCNAME." //-->");

        if ($info["exim"] == "noexport")
        {
            $this->html->warning("Data of table '$table' not allowed for export. "
                . "Please contact administrator. <!-- ".$FUNCNAME." //-->", '', PaXML_MINVIEW);
            return null;
        }
		
		// default columns
		$parent_tab   ="TABLE_NAME";
		$parent_id_col="OBJ_ID";
		
		if ($table=="SATTACH") {
			$parent_tab   ="TABLE_NAME";
			$parent_id_col="OBJ_ID";
		} 

		// statement: select an associative table filtered by one part of pk
		$stmt = "SELECT ";
		$separator  = "";
		//$useColumns = NULL;
		
		foreach ($info["columns"] as $name => $column)				// foreach column
			if ($name != $parent_id_col and $name != $parent_tab and $column["exim"] != "noexport")		// column excluding condition
            {
				//$useColumns[] = $name;
                $stmt .= $separator .$name;		// fieldname
				$separator = ", ";							
            }
		// get parameters
		$stmt = $stmt . " FROM $table ".
			    " WHERE " . $parent_tab ."='".$parent. "' AND " .$parent_id_col . " = '$id'";
		$this->runSQL($stmt);

        
		//$rowdata   = array();
		$cnt       = 0;
		$numvalues = 1; // TBD: count them before!
		
  		while($this->db->ReadArray()) {
		
			if (!$cnt) {
				$this->html->startRelationships($table, $numvalues);
				$this->xml->startRelationships($table);
			}
			
			$onerow = $this->db->RowData;
			$artificialFilename = "";
			
			
			
			if (!$this->doNotTakeAttachments) {
                $attach =  $this->attachLib->getDocumentPath( $parent, $id, $onerow["REL_ID"] );
				
				
                if ($attach != null and file_exists($attach)) {
					$artificialFilename = $table . "." . uniqid(null);
				}
            }
			
			$this->html->exportSattach();
			$this->_attachWriteData($onerow, $attach, $artificialFilename, $parent.":".$id);
			
			$cnt++;
   		}
        
		// no data => never write this tag 'elements', because loop will never run
        if ( !$cnt ) return true;
         
        $this->html->endRelationships();
        $this->xml->endRelationships();   
        
        return true;
    }


    // helper function to fire sql statements quickly
    function runSQL($stmt)
    {
        $this->html->statement($stmt);
        if ($this->db->Query($stmt) != 1)
            $this->html->stop("Database reading error. <!-- stmt: $stmt //-->");
    }
}


// aus. vorbei.
