<?
require_once("db_access.inc");
require_once("Tar.inc");
require_once('glob.obj.conabs.inc');
require_once("o.DB_USER.subs.inc");


// table types. define some constant
define ("PaXML_OBJECT",  "object");
define ("PaXML_OPTION",  "option");
define ("PaXML_ASSO",    "associative");
define ("PaXML_SATACH",  "objattach");
define ("PaXML_EXTEND",  "extended");
define ("PaXML_CLASS",   "extra");
define ("PaXML_ACCESS",  "access");
define ("PaXML_META",    "meta");
define ("PaXML_VIEW",    "view");
define ("PaXML_UNKNOWN", "unknown");


# NOW: the "db" attribute just contains the database-ID
# OLD: till 2020-03-20: PaXML_SELFURL = "http://www.clondiag.com/magasin/?db="
define ("PaXML_SELFURL",  ""  ); 



/**      
//      class for getting info's needed for paxml
//      a) returns info's about table structure and table type
//      b) returns general info's (image path, temp path)
//      c) handle file compressing
//      d) handle tared files
 * @package PAXMLLib
 * @swreq UREQ:0000161: g.paxml-export > HOME UREQ 
 * @swreq UREQ:0000171: g.paxml-import > HOME UREQ 
 * @author rogo, steffen
 * @version $Header: trunk/src/www/pionir/impexp/partisanxml/PaXMLLib.inc 59 2018-11-21 09:04:09Z $
 *
 */
class PaXMLLib
{
    var $db;        // only reading by this handle, don't care about transactions
    var $html;      // html output class, serverd by external code
    
    var $tableinfo; // table info structure cache
    var $dbid;      // database id
    var $remoteClass; // a remote class, contains methods: stop(), ...
	var $userIDs;	// array of USER_IDs array[NICK] = DB_USER_ID
    var $userInfoFlag;		// true|false 
							// true:
							//  - export also the user-info CCT_ACCESS => DB_USER_ID
							//  - export: ignore "exim"-flags in CCT_TABLE
	var $exportOpts; /* array of export options:
						  "SysFull" : 0,1 export full system objects, e.g. DB_USER (all features)
					 */

    // constructor.
    //
    // input:   handle      $htmlOutput output for html
    //          string      $user       database user name
    //          string      $passwd     database user password
    //          string      $db         database host name
    //          string      $dbtype     database type
    // output:  -
    // errors:  -
    //
    function __construct(&$htmlOutput, $user, $passwd, $db, $dbtype, &$remoteClass)
    {
        $this->html        = $htmlOutput;
        $this->db          = logon_to_db($user, $passwd, $db, $dbtype);   // separate transaction
        $this->tableinfo   = array();                                     // table info structure cache
        $this->dbid        = null;                                        // database id
        $this->remoteClass = &$remoteClass;
		$this->userInfoFlag		= false;
		
		$this->conc_abs_lib = new gConcAbsC();
		
    }
	
	// function: set archive-flag
    function setArchiveFlag( $flag // true|false
		) {
		$this->userInfoFlag	= $flag;
	}
	
	// function: set export options
    function setExpModus( $optArr
		) {
		$this->exportOpts = $optArr;
	}


    function exitSecure($message)
    {
        if ($this->remoteClass != null)
            $this->remoteClass->stop($message);
        else
            $this->html->stop($message);
    }
    
   
    // *************************************************************************
    //
    // db structure function
    //
    
    
    /**
     * TableInfo post processing
     * - modify $columns
     * @return ---
     * @param object $table
     * @param object $columns [output]
     */
    function _doTableInfoPost($table, &$columns) 
	{
		switch ($table) {
			case 'DB_USER':
				// allow only NICK !!!
				// @swreq SREQ:0000243:001: table DB_USER: do not export system intern columns 
				foreach( $columns as $colname=>$valarr) {
					if ($colname!='NICK') $valarr["exim"]='noexport';
					$columns[$colname]=$valarr;
				}
				reset ($columns);
			case 'SATTACH':
				// set some columns
				// @swreq SREQ:0000243:001: table SATTACH: do not export system intern columns 
				$denyCols=array('DB_USER_ID','NO_EXPORT','XESIG_ID','ARCHIVE');
				foreach( $columns as $colname=>$valarr) {
					if (in_array($colname,$denyCols)) {
						$valarr["exim"]='noexport';
						$columns[$colname]=$valarr;
					}
				}
				reset ($columns);
			break;
		}
	}

    /**
     *  returns table information. use this function as often as you want,
    // because returned data is chached.
    //
    // This function should use mac-cache for the future.
    //
    // input:   string      $table      table name
    // output:  table info structure: 
    //            array("type"    => value,
    //                  "columns" =>  array( COLNAME => array(
     						     "pk" => value,
                                 "fk_table" => string,
                                 "not_null" => 0|1,
      							 "exim"     => 0|1,
                                 "datatype" => int
                                 )
    				     ) 
    				  )
    // errors:  null means error
    */
    function getTableInfo($table)
    {
		// parameter check
        // can be NULL
		/*if ($table == null or !is_string($table)) {      
			$this->exitSecure("Internal error. Wrong parameter input. Please contact your admin. <!-- getTableInfo() //-->");
        }
        */
        
		// table touched already ?  yes, so use saved tableinfo
		if (isset($this->tableinfo[$table]))
			$result = $this->tableinfo[$table];
		else
		{
            if (!$this->db)
			    $this->exitSecure("Internal error. No database connection. Please contact your admin. <!-- getTableInfo() //-->");

            // check for view
            $stmt = "SELECT is_view, exim FROM cct_table WHERE table_name = '$table'";

            $this->html->statement($stmt);
            if ($this->db->Query($stmt) != 1)           // success ?
			    $this->exitSecure("Database read error. <!-- getTableInfo() //-->");
            else
            {
                if ($this->db->ReadRow())               // get date
                    if ($this->db->RowData[0] == 1)  {   // is_view == 1
                        $this->setTableInfo($table, "type", PaXML_VIEW);
                    	return $this->tableinfo[$table];
                    } else {
						$tmpexim = ($this->db->RowData[1] == 1) ? "noexport" : "export"; //TBD: exim
						if ( $this->userInfoFlag == true ) $tmpexim = "export";
                        $this->setTableInfo($table, "exim", $tmpexim);
					}
            }
            

			// no, query database to get new info, first query
			// cct_col_view, because this view represents database
			// reality. put result in $columns
			$columns = array();
			$stmt    = "SELECT column_name, data_type FROM cct_col_view WHERE table_name = '$table'";

            $this->html->statement($stmt);
			if ($this->db->Query($stmt) != 1)
			    $this->exitSecure("Database read error. <!-- getTableInfo() //-->");
			while($this->db->ReadRow())
				$columns[$this->db->RowData[0]] = array();

			// now try to get additional meta information by cct_column
			$stmt  = "SELECT cc.column_name, cc.primary_key, cc.cct_table_name, cc.not_null, cc.exim, adt.name ";
			$stmt .= "FROM cct_column cc, app_data_type adt WHERE cc.table_name = '$table' AND cc.app_data_type_id = adt.app_data_type_id";

            $this->html->statement($stmt);
			if ($this->db->Query($stmt) != 1)
			    $this->exitSecure("Database read error. <!-- getTableInfo() //-->");

			$thepk = NULL;
			// add new data to $columns
			while($this->db->ReadRow())
            {
				// TBD: exim ...
				$tmpexim = ($this->db->RowData[4] == 1) ? "noexport" : "export";
				if ( $this->userInfoFlag == true ) $tmpexim = "export";
				$colname = $this->db->RowData[0];
				$datatype= $this->db->RowData[5];
				
				// test $datatype: otherwise: import of a real DATE fails!,
				// see PaXMLReader.inc: function import(): "$vals .= $komma . $db->dateString2Sql($value, 1);"
				if ($datatype=='date3') $datatype='date'; 
				
				$columns[$colname] = array(
					"pk" => $this->db->RowData[1],
					"fk_table" => $this->db->RowData[2],
					"not_null" => $this->db->RowData[3],
                    "exim"     => $tmpexim,
                    "datatype" => $datatype
					);
				
					
				if ($this->db->RowData[1]==1) $thepk =  $colname;
                if ($this->db->RowData[3] == 1 and $this->db->RowData[4] == 1)
                    $this->exitSecure("Conflict in CCT_COLUMN. Column '" . $this->db->RowData[0] . 
					 "' is declared for not nullable and not exportable. This is an illegal state. ".
					 "Contact your admin.");
            }
			
			
			// post processing
			if ($table=='DB_USER' or $table=='SATTACH') {
				$this->_doTableInfoPost($table, $columns);
			}

			// save the whole new information in class variable
			// and return result.
            $this->setTableInfo($table, "pk", $thepk);
			$this->setTableInfo($table, "columns", $columns);
            $result =  $this->tableinfo[$table];
			
			
        }
		

		if ($result == null)
		    $this->exitSecure("Table $table doesn't exists. <!-- getTableInfo() //-->");

		return $result;
    }

    /* set a value in table info structure.
    //
    // input:   string      $table      table name
    //          string      $key        property name 
					('type', 'columns', 'exim', 'childs', 'parent'. 'pk')
    //          mixed       $value      value
    // output:  $value
    */
    function setTableInfo($table, $key, $value)
    {
		$this->tableinfo[$table][$key] = $value;
		return $value;
    }

    
    /**
     * export is DENIED ? analyse:
     * - CCT_ACCESS.EXPORT_DENY
     * - check, if it has an abstract object, which is DENIED ...
     * @param string $table
     * @param int $id
     * @return array
     *   'exim' : 'noexport'
     *   'info' : text
     */
    function getObjExportDeny(string $table, int $id) {
        
        $out=array();
         
        $pk_name = PrimNameGet2($table);
        $stmt = "SELECT a.EXPORT_DENY from ".$table." x join CCT_ACCESS a on x.CCT_ACCESS_ID=a.CCT_ACCESS_ID WHERE ".$pk_name."=".$id;
        //$this->html->statement($stmt);
        $this->db->Query($stmt);
        $this->db->ReadRow();
        if ($this->db->RowData[0]>0)  {
            $out['exim']='noexport';
            $out['info']='EXPORT_DENY flag of object is set.';
            return $out;
        }
        
        $tmp_table=$this->conc_abs_lib->getTemplate($table);
        if ($tmp_table) {
            $abs_obj = $this->conc_abs_lib->getAbsFrConc($this->db, $table, $id);
            if ($abs_obj['id']) {
                $out = $this->getObjExportDeny($tmp_table, $abs_obj['id']);
                if ($out['exim']=='noexport') {
                    $out['info']='EXPORT_DENY flag of template-object [ID:'.$abs_obj['id'].'] is set.';
                    return $out;
                }
            }
        }
 
        return $out;
    }

    // returns a list of child tables for given table, child tables may be
    // associative tables, extended tables or object tables (sub objects)
    //
    // input:   string      $table      tablename
    // output:  array(string childtablename, ...)
    // errors:  -
    //
    function getTableChilds($table)
    {
		// parameter check
		if ($table == null and !is_string($table))
			$this->exitSecure("Internal error. Wrong parameter input. Please contact your admin. <!-- getTableChilds() //-->");

        $info = $this->getTableInfo($table);
        
		// is there data in memory
		if (isset($info["childs"]))
			return $info["childs"];

        if (!$this->db)
            $this->exitSecure("Internal error. No database connection. Please contact your admin. <!-- getTableChilds() //-->");

		
		// no, query database
        $childs = array();
		
		if ($table=='DB_USER') {
			// do NOT export children of DB_USER !
        	return $this->setTableInfo($table, "childs", $childs);
        }
		
        $stmt   = "SELECT table_name FROM cct_table "
                . "WHERE cct_table_name = '$table'";

        $this->html->statement($stmt);
        if ($this->db->Query($stmt) != 1)
            $this->exitSecure("Database error. <!-- getTableChilds() //-->");

        while ($this->db->ReadRow())
            $childs[] = $this->db->RowData[0];

        // save the whole new information in class variable
        // and return result.
        return $this->setTableInfo($table, "childs", $childs);
    }

    
    
    // returns a parent table for given table. a table can have only one parent.
    //
    // input:   string      $table      tablename
    // output:  string tablename
    //
    function getParentTable($table)
    {
		// parameter check
		if ($table == null and !is_string($table))
			$this->exitSecure("Internal error. Wrong parameter input. Please contact your admin. <!-- getParentTable() //-->");

        $info = $this->getTableInfo($table);

        // table info in mem ?
		if (isset($info["parent"]))
            return $info["parent"];

        if (!$this->db)
            $this->exitSecure("Internal error. No database connection. Please contact your admin. <!-- getParentTable() //-->");

    	// query database
        $stmt = "SELECT cct_table_name FROM cct_table "
              . "WHERE table_name = '$table'";

        $this->html->statement($stmt);
        if ($this->db->Query($stmt) != 1)
            $this->exitSecure("Database error. <!-- getTableChilds() //-->");

        // there can only be one parent table and if this parent table doesn't
        // exist return nothing
        if ($this->db->ReadRow())
            return $this->setTableInfo($table, "parent", $this->db->RowData[0]);

        return null;
    }
    
    
    
    // returns type of a table. use this function as often as you want, because
    // returned values are cached.
    //
    // input:   string      $table      tablename
    // output:  tabletype (as defined above
    //
    function getTableType($table)
    {
        // get informations about table structure, fields and more
        $info = $this->getTableInfo($table);

        // if this table was already evaluated for table type, use saved value
        // to speed up
        if (isset($info["type"]))
            return $info["type"];

        // catch table cct_access. cct_access is a special case
        if ($table == "CCT_ACCESS")
            return $this->setTableInfo($table, "type", PaXML_ACCESS);
			
		 if ($table == "SATTACH")
            return $this->setTableInfo($table, "type", PaXML_SATACH);

        // check for extra tables
        // rule: table name begins with 'EXTRA_'
        if (substr($table, 0, 6) == "EXTRA_")
            return $this->setTableInfo($table, "type",PaXML_CLASS);

        // check for business object
        // rule: only business objects except cct_access have a field called
        //       cct_access_id
        foreach($info["columns"] as $name => $column)
            if ($name == "CCT_ACCESS_ID")
                return $this->setTableInfo($table, "type", PaXML_OBJECT);

        // check for meta tables
        // rule: if table called app_data_type, cct_column, cct_table, globals,
        //       error_mesg
        if ($table == "APP_DATA_TYPE"
			|| $table == "CCT_COLUMN"
			|| $table == "CCT_TABLE"
			|| $table == "GLOBALS"
			|| $table == "ERROR_MESG")
			return $this->setTableInfo($table, "type", PaXML_META);

        // prepare next checks
        // count pk and fk at pk
        $pk_count = 0;
        $fk_at_pk_count = 0;
        foreach($info["columns"] as $name => $column)
        {
            if ($column["pk"] > 0)
                $pk_count++;
            if ($column["pk"] > 0 && $column["fk_table"] != null )
                $fk_at_pk_count++;
        }

        // check forlookup
        // rule: one primary key and no foreign key as primary key, and no
        //       cct_access_id
        if ($pk_count == 1 && $fk_at_pk_count == 0)
            return $this->setTableInfo($table, "type",  PaXML_OPTION);

        // check for extend table
        // rule: one part of primary key is a foreign key
        if ($fk_at_pk_count == 1)
            return $this->setTableInfo($table, "type", PaXML_EXTEND);

        // check for associatve table
        // rule: more than 1 primary keys
        if ($fk_at_pk_count > 1)
            return $this->setTableInfo($table, "type", PaXML_ASSO);

		// unknown table
        // this case should never happen!
		return PaXML_UNKNOWN;
    }
    
    
    
    // returns all fields which are not nullable. works only for object and
    // excludes primary keys.
    //
    // ATTENTION: this function requires a valid cct_column!
    //
    // input:   string      $table          table name
    // output:  array(columnname => array("parent" => parent, "datatype" => datatype))
    //
    function getNotNullableFields($table)
    {
        // check input
        if ($table == null or !is_string($table))
			$this->exitSecure("Internal error. Wrong parameter input. Please contact your admin. <!-- getNotNullableFields() //-->");

        if (!$this->db)
    	    $this->exitSecure("Internal error. No database connection. Please contact your admin. <!-- getNotNullableFieldso() //-->");
         
        $stmt = "SELECT col.column_name, col.cct_table_name, adt.data_type "
              . "FROM cct_column col, app_data_type adt "
              . "WHERE adt.app_data_type_id = col.app_data_type_id "
              . "AND col.table_name = '$table' AND col.not_null = 1 "
              . "AND col.primary_key = 0 "
              . "AND (col.cct_table_name != 'CCT_ACCESS' OR col.cct_table_name IS NULL) ";

        $this->html->statement($stmt);
        if ($this->db->Query($stmt) != 1)
            $this->exitSecure("Database error. <!-- getNotNullableFields() //-->");
            $result=array();
        while ($this->db->ReadRow())
            $result[$this->db->RowData[0]] = array("parent" => $this->db->RowData[1],
                                                   "datatype" => $this->db->RowData[2]);

        return $result;
    }



    // returns if a field of a table is a primkey field or not. returns true or
    // false
    //
    // ATTENTION: this function requires a valid cct_column!
    //
    // input:   string      $table          tablename
    //          string      $field          fieldname
    //
    function isPrimKey($table, $field)
    {
        // check input
        if ($table == null or !is_string($table) or $field == null or !is_string($field))
			$this->exitSecure("Internal error. Wrong parameter input. Please contact your admin. <!-- isPrimKey() //-->");

        // run for a faster yes-answer (most cases) by using internally cached
        // data, else ...
        $info = $this->getTableInfo($table);
        if ($info["columns"][strtoupper($field)]["pk"] > 0)
            return true;

        return false;
    }
    
	/**
	 * get primary key of table
	 */
    function getPrimKey($table)
    {
        $info = &$this->getTableInfo($table);
        $pkname = $info['pk'];

        return $pkname;
    }
    
	// test for exitence of a column. that's important, because this function
	// is used to avoid to create illegal statements with columns, which doesn't
    // exists
    //
    // input:   string      $table          table name
    // output:  array(columnname => array("parent" => parent, "datatype" => datatype))
    //
	function existsField($table, $field)
	{
		$FUNCNAME='existsField';
        if ($table == null or !is_string($table) or $field == null or !is_string($field))
			$this->exitSecure("Internal error. Wrong parameter input. Please contact your admin. <!-- existsField() //-->");

        // run for a faster yes-answer (most cases) by using internally cached
        // data, else ...
        $info = $this->getTableInfo($table);
        if ($info["columns"][strtoupper($field)] != null)
            return true;
 
        // ... go for database query
        if (!$this->db)
            $this->exitSecure("Internal error. No database connection. Please contact your admin. <!-- existsField() //-->");
            
		$stmt  = "SELECT column_name FROM cct_col_view WHERE table_name = '";
		$stmt .= strtoupper($table) . "' AND column_name = '" . strtoupper($field) . "'";

		$this->html->statement($stmt);
		if ($this->db->Query($stmt, $FUNCNAME) != 1)
            $this->exitSecure("Database error. <!-- existsField() //-->");

		if ($this->db->ReadRow())
            if ($this->db->RowData[0] != null)
    			return true;

		return false;
	}
 
 
 
 
 
    // *************************************************************************
    //
    // db functions
    //

 
 
    // get version from db.
    //
    // input:   -
    // output:  only version number e.g. 1.0.3.2
    //
    function getDBVersion()
    {
        if (!$this->db)
			    $this->exitSecure("Internal error. No database connection. Please contact your admin. <!-- getDBSerial() //-->");

		$stmt = "SELECT value FROM globals WHERE name = 'DBVersion'";

		if ($this->db->Query($stmt) != 1)
			$this->exitSecure("Database error. <!-- getDBVersion() //-->");

		if ($this->db->ReadRow()) {
			$fullversion = $this->db->RowData[0];
			$tmparr = explode(" ", $fullversion);
			$version = $tmparr[0];
			return $version;
		}
		return null;
        
    }
	
	// get serial number from db.
    //
    // input:   -
    // output:  -
    //
    function getDBSerial()
    {
        if ($this->dbid != null)
            return $this->dbid;
        else
        {
            if (!$this->db)
			    $this->exitSecure("Internal error. No database connection. Please contact your admin. <!-- getDBSerial() //-->");

            $stmt = "SELECT value FROM globals WHERE name = 'magasin_serial'";

            $this->html->statement($stmt);
		    if ($this->db->Query($stmt) != 1)
                $this->exitSecure("Database error. <!-- getDBSerial() //-->");

		    if ($this->db->ReadRow())
    		{
			    $this->dbid = $this->db->RowData[0];
			    return $this->dbid;
		    }
            return null;
        }
    }


    
    // returns temp directory, written in database. all returned paths end
    // with '/'
    //
    // input:   -
    // output:  -
    //
    function getWorkPath()
    {
        

        return (substr($_SESSION['globals']['work_path'], - 1) != "/") ? $_SESSION['globals']['work_path'] . "/" : $_SESSION['globals']['work_path'];
    }
    
	// returns temp directory for PAXML
    // without '/'
    function getPaxmlWorkPath() {
		$pathFull = $this->getWorkPath() . "pxmlexport." . session_id();
		return ($pathFull);
	}

    // return wiid by wiid name, if no wiid exist for give base, then create
    // wiid without contact info, so that an object can allways have a link to
    // an entry in h_wiid
    //
    // input:   string      $base           wiid name
    // output:  -
    //
 	function getWiid($base)
 	{
		$FUNCNAME="getWiid";
        if ($base == null or !is_string($base))
 			$this->exitSecure("Internal error. Wrong parameter input. Please contact your admin. <!-- getWiid() //-->");

        if (!$this->db)
            $this->exitSecure("Internal error. No database connection. Please contact your admin. <!-- getWiid() //-->");

        $stmt = "SELECT wiid FROM h_wiid WHERE name = '" . $base . "'";

		$this->html->statement($stmt);
	    if ($this->db->Query($stmt) != 1)
            $this->exitSecure("Database error. <!-- getWiid() //-->");

        // return or insert as new
        if ($this->db->ReadRow())
            return $this->db->RowData[0];

		$stmt = "INSERTX: H_WIID (contact_id, name) VALUES (null, '" . $base . "')";
		$this->html->statement($stmt);
		$colstr = "contact_id, name";
		$valstr = "null, '" . $base . "'";
	    if ($this->db->Insertx ( "H_WIID", $colstr, $valstr, $FUNCNAME) != 1)
            $this->exitSecure("Database error. Could not set an wiid entry. <!-- getWiid() //-->");

        $stmt = "SELECT wiid FROM h_wiid WHERE name = '" . $base . "'";

		$this->html->statement($stmt);
	    if ($this->db->Query($stmt) != 1)
            $this->exitSecure("Database error. Could not get wiid entry, which was inserted before. <!-- getWiid() //-->");

        // return or error
        if ($this->db->ReadRow())
            return $this->db->RowData[0];

	    if ($this->db->Query($stmt) != 1)
            $this->exitSecure("Database error. Unknown import problem of wiid entry. <!-- getWiid() //-->");
    }


 	// return wiid by wiid name only.
    //
    // input:   string      $id           wiid id
    // output:  -
    //
    function getWiidByID($id)
    {
        /*
        if ($id == null)
 			$this->exitSecure("Internal error. Wrong parameter input. Please contact your admin. <!-- getWiidByID() //-->");
        */
        if (!$this->db)
            $this->exitSecure("Internal error. No database connection. Please contact your admin. <!-- getWiidByID() //-->");

        if ($id == null)
            return null;

        $stmt = "SELECT name FROM h_wiid WHERE wiid = '" . $id . "'";
		$this->html->statement($stmt);
        if ($this->db->Query($stmt) != 1)
            $this->exitSecure("Database error. Unknown import problem of wiid entry. <!-- getWiidByID() //-->");

        // return or insert as new
        if ($this->db->ReadRow())
            return $this->db->RowData[0];
    }
    
    




	// get app data type id by name
	function getAppDataType($name)
	{
		$stmt = "SELECT app_data_type_id FROM app_data_type WHERE name = '$name'";

		$this->html->statement($stmt);

		if ($this->db->Query($stmt) != 1)
  			$this->secureStop("Fatal database writing error. Import aborted.");

		if ($this->db->ReadRow())
			return $this->db->RowData[0];

		return null;
	}




    function checkForCorrectVersion()
    {
		// may be future checks ...

        return true;
    }


	// get app data type id by name
	function existsProject($proj_id)
	{
		$stmt = "SELECT name FROM proj WHERE proj_id = '$proj_id'";
		$this->html->statement($stmt);

		if ($this->db->Query($stmt) != 1)
  			$this->secureStop("Fatal database writing error. Import aborted.");

		if ($this->db->ReadRow())
			return $this->db->RowData[0];

		return null;
	}
	
	function getUserNick($user_id) {
		// FUNCTION: get nick form $user_id, cache the nick
		// INPUT:    $this->userIDs
		// RETURN:   $nick
		
		
		if (!$user_id) return;
		
		if ( $this->userIDs[$user_id]!="" ) return ($this->userIDs[$user_id]);
		
		$stmt = "select NICK from db_user where db_user_id=".$user_id;
		if ( $this->db->Query($stmt)!=1 ) {
		 	$this->exitSecure("Database error. <!-- getUserNick() //-->");
		}
		if ($this->db->ReadRow()) {
			$nick = $this->db->RowData[0];
		} else {
			$nick = "root";
		}
		$this->userIDs[$user_id] = $nick;
		return ($nick);
	}


	function getUserID($nick) {
		// FUNCTION:- get ID form NICK ind DB_USER, cache the nick
		//			- if user does not exist => create a new one
		// INPUT:    $this->userNicks
		// RETURN:   $nick
		
		
		if ($nick=="") return $_SESSION['sec']['db_user_id']; 
		
		if ( $this->userNicks[$nick]!="" ) return ($this->userNicks[$nick]);
			
		$stmt = "select db_user_id from db_user where nick='".$nick."'";
		$this->db->Query($stmt);
		
		if ($this->db->ReadRow()) {
			$dbuserid = $this->db->RowData[0];
		} else {
			// create a user		
			
		    $tmppassword = fPasswordC::get_nonlogin_pw();
			
			$stmt = "(NICK, EMAIL, PASS_WORD, LOGIN_DENY)  VALUES ('".$nick."', 'unknown@clondiag.com', '".$tmppassword."',1)";
            $dbuserid = $this->db->queryInsert("DB_USER", $stmt, "DB_USER_ID");
			 
		}
	 	
		$this->userNicks[$nick] = $dbuserid;
		
		return ($dbuserid);
	}
    // *************************************************************************
    //
    // file functions
    //

    
    
    // removes a directory recursivly
    //
    // input:   string      $path           path to remove
    //
    // output:  true | false
    //
    function removeDirRecursivly($path)
    {
        if ($path == null)
            return true;

        $return = true;
        if ($dir = opendir($path))                                              // open dir
        {
            while (($file = readdir($dir)) !== false)                           // read element of dir
            {
                if ($file != "." and $file != "..")                             // ignore '.' and '..'
                    if (is_dir($path . "/" . $file))                                          // is element a dir
                        $return = $return && $this->removeDirRecursivly($path . "/" . $file);        // call this function again for subdir
                    else
                        $return = $return && unlink($path . "/" . $file);       // remove element, which is a file
            }
            closedir($dir);                                                     // close current dir
            $return = $return && rmdir($path);                                  // remove current dir
        }

        return $return;                                                         // return true if succeed, else false if one not succeed
    }
    
    

    
    
    // file type detector. detects a file of type gzipped, gnu tared and xml
    // (by extension). the magix number are token from linux os /etc/magic.
    //
    // input:   string  $filename       real filename which is accessable for server
    //          string  $orgFilename    filename like it was at client machine
    //
    // output:  null | zip | tar | xml
    //
    function getFileType($filename, $orgFilename)
    {
        // mimetypes ?
        $return = null;
        if (!file_exists($filename))
            return;
        
        if (($file = fopen($filename, "rb")) == false)
            return false;

        $char=array();
        $char[0] = fgetc($file);
        $char[1] = fgetc($file);

        // first two bytes of a gzipped file ?
        if (ord($char[0]) == 31 and ord($char[1]) == 139)
            $return = "zip";

        // detect gnu tar by 'ustar\040\040\0'
        fseek($file, 257, SEEK_SET);

        $char[0] = fgetc($file);
        $char[1] = fgetc($file);
        $char[2] = fgetc($file);
        $char[3] = fgetc($file);
        $char[4] = fgetc($file);
        $char[5] = fgetc($file);
        $char[6] = fgetc($file);
        $char[7] = fgetc($file);

        if (ord($char[0]) == 117
            and ord($char[1]) === 115
            and ord($char[2]) === 116
            and ord($char[3]) === 97
            and ord($char[4]) === 114
            and ord($char[5]) === 32
            and ord($char[6]) === 32
            and ord($char[7]) === 0)
            $return = "tar";

        fclose($file);

        if ($return == null and strtolower(substr($orgFilename, -4)) == ".xml")
            $return = "xml";

        return $return;
    }
    
    
    
    
    
    // compress xml file. it's recommended to do this, because xml files have
    // usually a very good rate. it's not recommended for data, especiially for
    // image files. they have a very bad rate.
    //
    // input:   string      $xmlFilename        a filename
    //
    // output:  true | false
    //
    function compress($xmlFilename)
    {
        if ($xmlFilename == null or !is_string($xmlFilename))
            return false;

        // is the gzip package included ?
        if (function_exists('gzopen') && function_exists('gzwrite') && function_exists('gzclose'))
        {
            $zipFilename = $xmlFilename . ".gz";            // add .gz extension; new zip file

            if (!($zipFile = gzopen($zipFilename, "w9")))   // write mode; maximum compression
                return false;

            if (!($xmlFile = fopen($xmlFilename, "r")))     // open file for read
                return false;

            // read data - compress it - write it
            gzwrite($zipFile, fread($xmlFile, filesize($xmlFilename)), filesize($xmlFilename));

            gzclose($zipFile);                              // close
            fclose($xmlFile);                               // close

            return true;
        }

        return false;
    }

    // uncompress a gzipped file
    //
    // input:   bool    $withZipped     exlude xml ?
    //          string  $path           path to tar
    //
    // output:  -
    //
    function uncompress($zipFilename, $destPath)
    {
        // check for gzip libary
        if (!function_exists("gzopen") || !function_exists("gzeof") || !function_exists("gzread")  || !function_exists("gzclose"))
            $this->html->stop("Missing gzip module functionality. The server is misconfigured. contact your administrator!");

        // open and create file
        if (!($zip = gzopen($zipFilename, "r")))
            $this->html->stop("Couldn't open gzip file.");

        if (substr($destPath, -1) != "/")
            $destPath = $destPath . "/";

        // extract filename without xml extension
        $xmlFilename = $destPath . "paxml.xml";
        if (!($xml = fopen($xmlFilename, "w")))
            $this->html->stop("Couldn't create temoprary xml file for uncompressing gzip file.");

        // transform gzip -> xml
        while (!gzeof($zip))
            fwrite($xml, gzread($zip, filesize($zipFilename)));

        gzclose($zip);                                      // close
        fclose($xml);                                       // close

        return array("xml" => $xmlFilename);
    }



    /**
     * tar your files if you want to handle several files like one. tar is doing
     * the same like linux command tar. if $withZipped is true, the xml file
     * will be excluded from tar process, else the zip file will be excluded
     * 
     * - support for showing progress now (simple chars per time-slice)
     *
     * @param   bool    $withZipped     exlude xml ?
     * @param   string  $path           path to tar
     */
    function tar($withZipped = false, $path)
    {
        // creating now an tar archive
	    $tar = new Tar();
	    $tar->create($path . ".tar");       // create tar file
        $tar->cd($path . "/");              // change to path for taring

        // read file by file from dir and add to tar archive. catch readdir
        // returns '.', '..', xml file or zip file (?)
        
        
        // get number of files
        $ininum=0;
        $exportPath = opendir($path);
        while (($dirfile = readdir($exportPath)) != false)
        {
            if (!($dirfile == "." or $dirfile == "..") ) {
            	$ininum++;
            }
                
        }
        closedir($exportPath);
        
        $progopt=array('mode'=>'char');
        $this->html->startProgress( $ininum, $progopt );
        $filecnt=0;
        
        
        // start TAR !
        $exportPath = opendir($path);
        while (($dirfile = readdir($exportPath)) != false)
        {
            if (!($dirfile == "."
                or $dirfile == ".."
                or ($withZipped and strtolower(substr($dirfile, -4)) == ".xml")
                or (!$withZipped  and strtolower(substr($dirfile, -3)) == ".gz"))) {
                	$tar->add($path . "/" . $dirfile);
                	// show progress ...
                	$this->html->showProgress($filecnt);
                	$filecnt++;
                	
                }

            $dirfile = "";
        }
        // finish
        $this->html->stopProgress($filecnt);
        closedir($exportPath);
        
        $tar->close();                      // close tar file

    }



    // untar (paxml) a tar file into a destination path.
    //
    // NOTE: big tar files may cause harddisk problems. suppose a 500 mb tar
    // file is untar also 500 mb. so you'll need 1 gb harddisk!
    //
    // input:   string  tarFilename     name of tar file (must be accessable for server)
    //          string  destPath        path where to put the content of tar file
    //
    // output:  array("zip" => zipfilename, "xml" => xmlfilename)
    //
    function untar($tarFilename, $destPath)
    {
        // add '/' if not done
        if (substr($destPath, -1) != "/")
            $destPath = $destPath . "/";

        // open tar file
        $tar = new Tar();
        if (!$tar->open($tarFilename))
            $this->html->stop("Could not open paxml package file (tar). <!-- file: $tarFilename //-->");

        // extract all files from tar. if a zip file is found, set zip file
        // for return. if a xml file is found, set xml file for return.
        //
        // NOTE! if several zip or xml files are included in tar. the last in
        // tar will be returnred only
        $tarFileCounter = 0;
        $result=array();
        while (($filename = $tar->extractNext($destPath)))
        {
            $filetype = $this->getFileType($destPath . $filename, $destPath . $filename);

            if ($filetype == "zip") $result["zip"] = $destPath . $filename;
            if ($filetype == "xml") $result["xml"] = $destPath . $filename;
            
            $tarFileCounter++;
        }
        
        // finish. close file.
        $tar->close();
        return $result;
    }
}


?>
