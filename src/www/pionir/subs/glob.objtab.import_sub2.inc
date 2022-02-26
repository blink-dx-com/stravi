<?php 
/**
 * - for glob.objtab.import.php
 * - common functions
 * - GUI functions
 * @package glob.objtab.import_sub2.inc
 * @swreq UREQ:0001359: g > import object-attributes from CSV-file 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */


/**
 * a helper lib
 * @author steffen
 */
class gObjtabImpSubs {

    static function _arrayToString( $arrx ) {
    	if (!sizeof($arrx) ) return;
    	$tmpsep = "";
    	foreach( $arrx as $key=>$val) {
    		$newstr .= $tmpsep . $key."=".$val;
    		$tmpsep = ","; 
    	}
    	
    	return ($newstr);
    }
    
    static function _name2IdExt( &$sql, $objTransInfo, $objname) {
    
    	$tablename  = $objTransInfo["tab"];
    	$NameColumn = $objTransInfo["column"];
    	
    	list($tmpcnt, $tmpobjid, $tmpobname) = fileC::objGetByName( $sql, $tablename, $NameColumn, $objname );
                        
    	if ( $tmpcnt<1 ) {
    		$tmpinfo2 .= "name2IdExt():Object not found";
    		$funcret   = -2;
    	}
    	if ( $tmpcnt>1 ) {	
    		$tmpinfo2 .= 'More than one object found: <B>'.$tmpcnt.'</B> matches';
    		$funcret   = -2;
    	}
    	return array($funcret, $tmpobjid, $tmpobname, $tmpinfo2 );
    }
    					
    
    static function  _name2Id( &$sql, $tablename, $importantName, $objname) {
    
    	list($tmpcnt, $tmpobjid, $tmpobname) = fileC::objGetByName( $sql, $tablename, $importantName, $objname );
                        
    	if ( $tmpcnt<1 ) {
    		$tmpinfo2 .= 'name2Id():Object not found';
    		$funcret   = -2;
    	}
    	if ( $tmpcnt>1 ) {	
    		$tmpinfo2 .= 'More than one object found: <B>'.$tmpcnt.'</B> matches';
    		$funcret   = -2;
    	}
    	return array($funcret, $tmpobjid, $tmpobname, $tmpinfo2 );
    }
    
    static function objTransformWiid( &$sql, &$iniarr, &$obj_id, &$wiidTransArr) {
        
           if ( $wiidTransArr[$obj_id] ) {  // take from cache ???
                return  ($wiidTransArr[$obj_id]);
           }
           
           $prim_name =  $iniarr["pname"];
           $db_wiid   =  $iniarr["dbwiid"];
           
           $sqls = "select x.".$prim_name." from CCT_ACCESS a, ".$iniarr["mother"]." x where (a.table_name='".$iniarr["mother"]."' AND a.WIID=".$db_wiid." AND a.ROID=".$obj_id.")".
                    " AND a.CCT_ACCESS_ID=x.CCT_ACCESS_ID";
           $sql->query($sqls);
           if ( $sql->ReadRow() ) {
                $new_obj_id  = $sql->RowData[0]; 
                if ( sizeof($wiidTransArr)< 5000 ) {   // if not too many main ID's ==> save in cache
                  $wiidTransArr[$obj_id] = $new_obj_id;
                }
           } else {
              $new_obj_id = 0;
           }
           return ($new_obj_id);
    }    
    
    static function obj_name2ID($sqlo, $from_obj_id, $objName2ID_arr ) {
        
        $tmpname 	  = $from_obj_id;
        $tmpNameTable = $objName2ID_arr["tab"];
        $tmpNameCol   = $objName2ID_arr["column"];
        
        list($funcretx, $tmpobjid, $tmpobname, $tmpinfox) = self::_name2Id( $sqlo, $tmpNameTable, $tmpNameCol, $tmpname);
        
        return array(
            'retval'=>$funcretx, 
            'info'=>$tmpinfox, 
            'objid'=>$tmpobjid
            
        );
        
        
    }
    
}




// ----------------------------------------------------------

// gui sub functions
class gObjtabImpGui {

    function __construct($tablename) {
    	$this->tablename = $tablename;
    }
    
    private function _add_css() {
        ?>
        <style type="text/css">
        tr.row_silent td {
            color:gray;
        }
        </style>
        <?php
    }
    
    function get_redund_column_arr($sqlo) {
        $output=array();
        $tablename = $this->tablename;
        $all_cols = columns_get_pos($this->tablename);
        foreach($all_cols as $col) {
            $col_feats = colFeaturesGet( $sqlo, $tablename, $col);
            if ( $col_feats['VISIBLE'] and $col_feats['PRIMARY_KEY']!=1 ) {
                $nice_name = $col_feats['NICE_NAME'];
                if ($col_feats['CCT_TABLE_NAME']!=NULL) $nice_name = 'ID of '.$nice_name;
                $output[$col] = $nice_name;
            }
        }
        return $output;
    }
    
    /**
     * 
     * 
     * @param array $parx
     * @param int $go
     * @param array $param_ini
     * @param array $options
     *   'showProjID' => 0,1
     *   'redund_cols' = array of columns
     */
    function form1( &$sqlo, $parx, $go, $importMode, $param_ini, $options) {
    	global   $varcol;
    	
    	// following CSS is used for hiding the optional fields
    	?>
    	<style type="text/css">
        #opt1, #opt2, #opt3, #opt4, #opt5, #opt6  { visibility: collapse;}
        </style>
       
    
    	<script type="text/JavaScript">  
      
       
        function showFields()  
        {  
            newstate   = 'visible';
            switchText = "Hide Options";
    		if ( document.getElementById("opt1").style.visibility=='visible') {
    			 newstate='collapse';
    			 switchText = "Show Options";
    		}
            
        	for (i=0; i<6; i++) {
            	fieldx = "opt" + (i+1);
            	document.getElementById(fieldx).style.visibility = newstate;  
        	}  
        	document.getElementById("optionFlagText").innerHTML = switchText;
        }  
        
            
        </script>  
        <?php
    	
    	$tablename = $this->tablename;
    	$initarr   = NULL;
        $initarr["action"] = $_SERVER['PHP_SELF'];
        $initarr["title"]  = "Give import information";
        $initarr["ENCTYPE"]= "multipart/form-data";     
        $initarr["submittitle"] = "Prepare &gt;&gt;";
        $initarr["dblink"]      = 1;
         $initarr["tabwidth"]   = "AUTO";
         
        $hiddenarr = NULL;
        $hiddenarr["tablename"]     = $tablename; 
        $hiddenarr["MAX_FILE_SIZE"] = $_SESSION['globals']["F.ASCI_TABLE.IMPORT.UPLOAD_MAX_SIZE"];
        $hiddenarr["parx[doUpload]"]= 1;
        $hiddenarr["parx[motherid]"]= $parx["motherid"];
        $hiddenarr["parx[action]"]  = $parx["action"];
    	if ($parx["showOpts"]) $hiddenarr["parx[showOpts]"]= $parx["showOpts"];
        
        $formobj = new formc( $initarr, $hiddenarr, $go );
                                      
        $fieldx = NULL;
        $tmpnotes = " Max file size: <B>". ($_SESSION['globals']["F.ASCI_TABLE.IMPORT.UPLOAD_MAX_SIZE"]/1e6)."</B> MBytes.";
        $fieldx = array (
        		"title" => "Data file", 
        		"name"  => "userfile", 
        		"namex" => TRUE,
                "val"   => "", 
                "colspan" => 2, 
                "notes" => $tmpnotes, 
                "object" => "file",
         		"req"   => 1  );
                               
        $formobj->fieldOut( $fieldx );
        
            
        
        
        if ( $options['showProjID']) {
        	 $fieldx = array (
        	 	"title" => 'Destination Folder <img src="images/icon.PROJ.gif">', 
        	 	"name"  => "projid",
                "val"   => $parx['projid'],  
                "notes" => "destination folder", 
        	 	// "req"   => 1
                "object" => "dblink",
        		"inits" => array( 
        			'table'=>'PROJ', 'getObjName'=>1, 'sqlo'=>&$sqlo, 
        			'pos' =>'0', 
        			'projlink'=> 1)
        	 );
        	 $formobj->fieldOut( $fieldx ); 
        }
        
        $tmpnotes = "";
        $classarr = $varcol->get_class_nice_names( $tablename );
        if (sizeof($classarr)) { 
        
            $fieldx = array ("title" => "Class", "name"  => "class_id", "inits" => $classarr,
                            "val"   => $parx["class_id"],  
                            "valid" => "", "notes" => $tmpnotes, "object" => "select" );        
            $formobj->fieldOut( $fieldx );
        }
        
        if ($importMode=='singleInsert') {
        
    	    $fieldx = array ("title" => "Redundancy test", "name"  => "testredunt", "object" => "checkbox",
    	                     "val"   => $param_ini["testredunt"], "req"   => "", "inits" => "1", "valid" => "",
    	                     "notes" => 'do NOT insert, if the "Redundancy column"-value of the object is already in the database (e.g. a serial number)' );   
    	    $formobj->fieldOut( $fieldx ); 
    	    
    	    $fieldx = array ("title" => "Redundancy column", "name"=>"redundancy_cols_in", "object" => "select",
    	        "val"   => $param_ini["redundancy_cols_in"], "req"=> "", "inits" => $options['redund_cols'], "valid" => "",
    	        "notes" => "use this column for redundancy check" );
    	    $formobj->fieldOut( $fieldx ); 
        }
    
    
    	$fieldx = array (
    		"title"   => "",
    		"val"     => '<a href="javascript:showFields()" ><span id="optionFlagText">show Options</span></a>', 
    		"object"  => "info2",
    	);        
    	$formobj->fieldOut( $fieldx ); 
    		
    		//
    		// following fields are CSS-hidden by default, DEFAULT: hidden
    		//
    		
    	$key='colname.type';
    	$initsarr = array ("HUMAN"=>'normal names', "RAW"=>'raw database names');
    	$tmpval   = "HUMAN";
    	if ($parx[$key] !="" ) $tmpval = $parx[$key];
    	$fieldx = array (
    	    "title" => "Column Name Type", "name"  => $key, "inits" => $initsarr,
    	    "val"   => $tmpval,  "optional"=>1,
    	    "type"  => "", "valid" => "", "notes" => 'RAW or normal column names', 
    	    "object" => "select",
    	    "rowid"   => "opt6" );
    	$formobj->fieldOut( $fieldx );   
        
    	$initsarr = NULL;
    	$initsarr["continue"] = "continue";    
    	$initsarr["50"]       = "stop after 50 errors";
    	$initsarr["5"]        = "stop after 5 errors";
    	$initsarr["1"]        = "stop after first error";
    	
    	$tmpnotes = "What to do, if insert of one data set fails?";
    	$fieldx = array (
    		"title" => "On_error", "name"  => "errcase", "inits" => $initsarr,
    		"val"   => $parx["errcase"], "optional"=>1,
    		"type"  => "", "valid" => "", "notes" => $tmpnotes, "object" => "select",
    		"rowid"   => "opt1"
    		);        
    	$formobj->fieldOut( $fieldx ); 
    		
    	
    	$initsarr = array ("5" => "5", "10" => "10", "20" => "20", "ALL" => "ALL" );
    	$tmpval   = "5";
    	if ($parx["i_shownum"] !="" ) $tmpval = $parx["i_shownum"]; 
    	$tmpnotes = "Number of shown lines in file";
    	$fieldx = array (
    		"title" => "Show_lines", "name"  => "i_shownum", "inits" => $initsarr,
    		"val"   => $tmpval,  "optional"=>1,
    		"type"  => "", "valid" => "", "notes" => $tmpnotes, "object" => "select",
    		"rowid"   => "opt2" );        
    	$formobj->fieldOut( $fieldx );   
    	
    	$fieldx = array ("title" => "To clipboard", "name"  => "toclip", "object" => "checkbox",
    					"val"   => $param_ini["toclip"], "inits" => "1", "optional"=>1, 
    					"notes" => "Copy objects to clipboard?", 
    					"rowid"   => "opt3" );   
    	$formobj->fieldOut( $fieldx );
    	
    
    	$initsarr = array ("0" => "only error lines", "1" => "show each line","2" => "more more", "3" => "guru" );
    	$tmpval   = "0";
    	if ($parx["infolevel"] !="" ) $tmpval = $parx["infolevel"]; 
    	$tmpnotes = "";
    	$fieldx = array (
    		"title" => "Info-level", "name"  => "infolevel", "inits" => $initsarr,
    		"val"   => $tmpval, "optional"=>1 ,
    		"type"  => "", "valid" => "", "notes" => $tmpnotes, "object" => "select",
    		"rowid"   => "opt4" );        
    	$formobj->fieldOut( $fieldx );
    
    	$fieldx = array ("title" => "trim double quotes", "name"  => "trimDouQout", "object" => "checkbox",
    					"val"   => $param_ini["trimDouQout"], "inits" => "1", "optional"=>1, 
    					"notes" => "trim double-quotes from cells?", 
    					"rowid"   => "opt5" );   
    	$formobj->fieldOut( $fieldx );
    		             
        
        $formobj->close( TRUE );
    }
    
    /**
     * select the action
     * Enter description here ...
     */
    function form1_action( ) {
    	
    	$tablename = $this->tablename;
        $initarr   = NULL;
        $initarr["action"]      = $_SERVER['PHP_SELF'];
        $initarr["title"]       = "Select action"; 
        $initarr["submittitle"] = "Continue &gt;&gt;";
        $initarr["tabwidth"]    = "AUTO";
        
        
        $hiddenarr = NULL;
        $hiddenarr["tablename"]     = $tablename; 
    
        $formobj = new formc( $initarr, $hiddenarr, -1 ); 
    
        $initsarr = NULL;
        $initsarr["update"] = "update";
        $initsarr["insert"] = "insert";
        $tmpval   = "update";
     
        $tmpnotes = "UPDATE data of existing objects or INSERT new objects";
        $fieldx = array ("title" => "Action", "name"  => "action", "inits" => $initsarr,
                         "val"   => $tmpval,  "req"   => 1,
                         "valid" => "", "notes" => $tmpnotes, "object" => "select" );        
        $formobj->fieldOut( $fieldx ); 
        
         $formobj->close( TRUE );
    }
    
    function form2( $parx, $go, $formok, $importMode, $tmpfile) {
        // TBD: $parxerr ???
        $this->_add_css();
        
    	$tablename = $this->tablename;
        $initarr   = NULL;
        $initarr["action"]      = $_SERVER['PHP_SELF'];
        $initarr["title"]       = "Final INSERT choises";
        $initarr["submittitle"] = "Insert now!";
        $initarr["tabwidth"]    = "AUTO";
        
        if ( $importMode == "objupdate" ) {
            $initarr["title"]       = "More UPDATE parameters"; 
            $initarr["submittitle"] = "Prepare update &gt;&gt;";
        } else {
            $initarr["goNext"] = 3;    // jump over step 2 
        }
        $hiddenarr = NULL;
        $hiddenarr["tablename"]     = $tablename; 
        
        $param_ini = $parx;  // save input parameters
        
        // delete interactive parameters
        unset($parx["shFirstObj"] );
        unset($parx["ignoreMissObj"] );
        unset($parx["WIID"]);
        unset($parx["motherid"]);
        
        if ( sizeof($parx) ) {
            foreach( $parx as $idx=>$valx) {
                  $hiddenarr['parx['.$idx.']']    = $valx;
            }
            reset($parx); 
        }    
          
        $formobj = new formc( $initarr, $hiddenarr, $go );   
        
        if ( $importMode == "objupdate" ) {
        
            
            if ( $parx["motherneed"] ) {  // no PRIMARY_KEY column given ?
                $fieldx = array ("title" => "Mother_Object_ID", "name"  => "motherid", 
                                "val"   => $param_ini["motherid"], "req"   => 1,
                                "errtxt"=> $parxerr["motherid"], 
                                "notes" => "the ID of the mother-object", "object" => "text" );        
                $formobj->fieldOut( $fieldx );
                $doImport = 0; // can not show import-prepare, because PRIMARY-KEY still missing  !!!
            }
            
            $tmpnotes = "for data from the origin database ID; e.g. '12'";
            $fieldx = array ("title" => "ORI-WIID", "name"  => "WIID", 
                            "val"   => $param_ini["WIID"], "req"   => "", 
                            "valid" => "", "notes" => $tmpnotes, "object" => "text" );        
            $formobj->fieldOut( $fieldx );    
    
    
            $fieldx = array ("title" => "Show main objects", "name"  => "shFirstObj", "object" => "checkbox",
                            "val"   => $param_ini["shFirstObj"], "req"   => "", "inits" => "1", "valid" => "",
                            "notes" => "show first entry of a main object" );        
            $formobj->fieldOut( $fieldx );      
    
            $fieldx = array ("title" => "Ignore missing objects", "name"  => "ignoreMissObj", "object" => "checkbox",
                            "val"   => $param_ini["ignoreMissObj"], "req"   => "", "inits" => "1", "valid" => "",
                            "notes" => "continue, if object was not found " ); 
            $formobj->fieldOut( $fieldx );   
            
                          
        } else { 
    
                 
    
        }
             
        if ( $formok >=0 ) $showSubmit = TRUE;
        else  $showSubmit = FALSE;        
        
        $formobj->close( $showSubmit );
    }
    
    function _infoRow( 
            $printPoints, 
            &$col_arr,
            $tab_len, 
            $tmpinfo,    // short info in SECOND column
            &$argu, 
            $class_id, 
            &$class_argu, 
            $tmpinfo2,
            $funcret     // if <0 :: error     
            ) {
                  
            global $error;
    
            
    
            if ( $printPoints ) {    // still open ???
                echo "</td></tr>\n";
                $printPoints = 0;
            }     
          
    
            echo "<tr>";
            echo "<td>$tab_len</td>";
            if ($funcret<0) $tmpinfo = "<font color=red><b>ERROR:</B></font> ".$tmpinfo." (Err-code:".$funcret.")";
            if ($error->Got(READONLY))  {
    	        $allErrTxt    = $error->getAllAsText(NULL,'<br />');
    	        $tmpinfo .= '<br />'.$allErrTxt;
    	        
            }
            echo "<td NOWRAP>$tmpinfo</td>";
            echo '<td nowrap><span style="color:red">'.$tmpinfo2.'</span></td>';  // error info
    
            foreach( $col_arr as $pos=>$colinfo){
                $col=$colinfo['col'];
                echo "<td>".$argu[$col]."</td>";
            }
           
    
            if ($class_id) {
                foreach( $class_argu as $tv) echo "<td>".$tv."</td>";
            } 
    
            
    
            echo "</tr>\n";
    
            $error->reset(); // remove error stack
    }
    
    /**
     * print progress
     * @param int $printPoints
     * @param int $tab_len
     * @param int $numColsTotal
     * @param int $validCnt
     */
    function _printProg( $printPoints, $tab_len, $numColsTotal, $validCnt) {
    
        if ( !$printPoints  ) {  
            echo "<tr bgcolor=#D0D0D0><td>".$tab_len."</td><td colspan=".($numColsTotal+2)."><font color=gray>more lines </font>";
        } 
    
        if ($printPoints<50) {             
            echo "*";
        } else {  
            if ($printPoints==100)  echo $validCnt.'.';
            if ($printPoints==500)  echo $validCnt.'.';
            if ($printPoints>=1000 AND ( round($printPoints*0.001) == ($printPoints*0.001) ) )  echo $validCnt.'.';
        }
        while (@ob_end_flush()); // send all buffered output
    } 
    
    /**
     * 
     * @param array $datafields
     * @param array $col_arr
     * @param array $classImpFields class attribute fields to import:  pos => ID 
     * @param array $classPossFields possible class attribute fields :  id  => name
     */
    function _print_header( 
        $datafields, 
        $col_arr,
        $classImpFields,  
        $classPossFields  
        ) {
        	
        ob_implicit_flush(TRUE);
    	ob_end_flush ( );
    	
    	$this->_add_css();
        
        echo "<table bgcolor=#EFEFEF>\n";
        
        echo "<tr bgcolor=#D0D0D0>\n";
        echo "<td>Line</td> \n";
        echo "<td>Info</td>\n";
        echo "<td>Info-Details</td>\n";
        foreach( $col_arr as $colinfo) {
            $col = $colinfo['nice'];
            echo "<td>".$col."</td>\n";
        }
        if ( !empty($classImpFields) ) {
            foreach( $classImpFields as $tv) {
                echo "<td bgcolor=#EFEFD0>".$classPossFields[$tv]."</td>";
            }        
        }
       
        echo "</tr>\n";
        
        echo '<tr class="row_silent">';
        echo "<td colspan=3>Raw columns =></td>";
       
        foreach( $col_arr as $colinfo) {
            $col=$colinfo['col'];
            echo "<td>".$col."</td>";
        }
        if ( !empty($classImpFields) ) {
            foreach( $classImpFields as $tv) {
                echo "<td bgcolor=#EFEFD0>".$classPossFields[$tv]."</td>";
            }
        }
        
        echo "</tr>\n";
        
        
    } 
    
    function showResultStats($retarr, $tmpimpinfo) {
        echo "<br>\n\n";
        htmlInfoBox( "Import result", "", "open", "INFO" ); 
             
        echo "<B>"            .$retarr["objcnt"]."</B> lines analyzed. <br>";
        echo "<font color=green><B>"  .$retarr["valids"]."</B></font> data-sets ".$tmpimpinfo.".<br> ";
        echo "<font color=#808000><B>".$retarr["found"]."</B></font> data-sets with same NAME found in DB.<br>";
        if ( $retarr["bad"] )  echo "<font color=red><B>".$retarr["bad"]."</B> data set(s) failed.</font><br>";
        htmlInfoBox( "", "", "close");
        echo "<br>";
    }
    
    function help_general() {
       
        
        ?> 
        Tool-Version: 07.04.2021<br><br>
    
        <ul>
        <LI>News (07.04.2021) Improved redundancy check: choose any column for the check (e.g. serial number)
     	</ul>
     	<br>
        
        <b>GENERAL file format</b>
    	<ul>
     	<LI>Supported: <b>CSV</B> (Text, separated by TAB-characters) or <B>EXCEL</B> (Excel2007 and newer)</LI>
     	<li>support character encoding: UTF-8 and ISO-8859-1 (Excel)</li>
    	<LI><b>Column names:</B> use the CODE names from the database e.g. 'ABSTRACT_SUBST_ID'</LI>
    	<li><B>Class attributes</B> are identified by leading; '<B>c:</B>' example: '<B>c:sequence</B>'</li> 
    	<li><B>Clean cell data: </b>Following characters in cells will be removed: Double quotes, White Spaces</li> 
        <li>Column '<B>class</B>' will be ignored (comes from the 'object list export')</li>  
        <li><B>Formats</B>: </li> <ul>
        	<li><b>DATE</b>: YYYY-MM-DD, e.g. 2010-05-29</li>
        	<li><b>NUMBER</b>: english number format, e.g. 567.554</li>
        	<li><b>Object-Reference by name</b> add ":NAME" to the hedaer column to accept object-names as reference (see example 2)</li>
        </ul>
    	
     	</ul>
     	<br>
     	
     	<b>History Log</b>
    	<ul>
    	<LI>2020-05-04: supports now <B>EXCEL-Format</B> (Excel2007 and newer)</LI>
    	<LI>2020-04-22: introduced data validation tests</LI>
     	<LI>2020-04-16: support character sets: UTF-8 and ISO-8859-1 (Excel)</LI>
     	</ul>
     	<br><br>
        
        <?
        
    }
    
    function help_update() {
    	htmlInfoBox( "Import help", "", "open", "HELP" );  
    	$this->help_general();
         
        ?>
        <b>UPDATE file format</b>
        <ul>
        <LI><B>Import Business objects</B>:<ul>
        <li>first column should be the PRIMARY_ID of the object (e.g. 'CONCRETE_SUBST_ID': 234, 235, 236) or</li>
        <li>can also be the NAME of the object (add ':NAME' to the column_name e.g. 'CONCRETE_SUBST_ID:NAME': sub1, sub2, sub3)</LI> 
        </ul>
    
    	<LI><B>Import Feature list elements</B> (like 'probe on array')</LI>
        
        <UL> 
        <li>if the features come from DIFFERENT Business objects: the PRIMARY_ID must be given in the file</li>
        <li>if all features come from ONE Business object: the PRIMARY_ID of the object will be given in the user-form</li>
        </UL>
        
        <br>
        <table bgcolor=#EFEFEF cellspacing=1>\n
        <tr bgcolor=#D0D0D0>
        <td><B>material template</B></td>
        <td>batch</td>
        <td>c:sequence</td>
        <td>notes</td>
        </tr>
        
        <tr bgcolor=#EFEFEF>
        <td>234</td>
        <td>Ethylenglykol</td>
        <td>actg</td>
        <td>used only for assay experiment type XZ</td>
        </tr>
        
        <tr bgcolor=#EFEFEF>
        <td>345</td>
        <td>Bidest</td>
        <td>actgaaa</td>
        <td>&nbsp;</td>
        </tr>
        
        </table>
        
        Version2 (RAW columns)
        
        <br>
        <table bgcolor=#EFEFEF cellspacing=1>\n
    	<tr bgcolor=#D0D0D0>
    	<td><B>CONCRETE_SUBST_ID</B></td>
    	<td>BATCH</td>
    	<td>c:orijam</td>
    	<td>NOTES</td>
    	</tr>
    	
    	<tr bgcolor=#EFEFEF>
    	<td>234</td>
    	<td>Ethylenglykol</td>
    	<td>15</td>
    	<td>used only for assay experiment type XZ</td>
    	</tr>
    	
    	<tr bgcolor=#EFEFEF>
    	<td>345</td>
    	<td>Bidest</td>
    	<td>2</td>
    	<td>&nbsp;</td>
    	</tr>
    		
     	</table>
     	
     	<?php
        htmlInfoBox( "", "", "close" ); 
    }    
    
    function help_insert() {
    	
    	htmlInfoBox( "Import format", "", "open", "HELP" );
    	$this->help_general();
        
        ?> 
        <b>INSERT file format</b>
        <ul>
      	Primary-Object-IDs are not allowed, the ID will be generated automaticaly
     	<br><br></ul>	
     	
     		
       <b>Examples</b> <ul>
        
        <br>Example 1 (normal columns)
        <table bgcolor=#EFEFEF cellspacing=1>
        <tr bgcolor=#D0D0D0>
        <td><B>material template</B></td>
        <td>batch</td>
        <td>c:sequence</td>
        <td>notes</td>
        </tr>
        
        <tr bgcolor=#EFEFEF>
        <td>234</td>
        <td>Ethylenglykol</td>
        <td>actg</td>
        <td>used only for assay experiment type XZ</td>
        </tr>
        
        <tr bgcolor=#EFEFEF>
        <td>345</td>
        <td>Bidest</td>
        <td>actgaaa</td>
        <td>&nbsp;</td>
        </tr>
        
        </table>
        
        <br>
        <br>Example 2 (normal columns, Object-Reference by name)
        <table bgcolor=#EFEFEF cellspacing=1>
        <tr bgcolor=#D0D0D0>
        <td><B>material template:NAME</B></td>
        <td>name</td>
        <td>batch</td>
        <td>c:sequence</td>
        <td>notes</td>
        </tr>
        
        <tr bgcolor=#EFEFEF>
        <td>template1</td>
         <td>batch1</td>
        <td>Ethylenglykol</td>
        <td>actg</td>
        <td>used only for assay experiment type XZ</td>
        </tr>
        
        <tr bgcolor=#EFEFEF>
        <td>template1</td>
         <td>batch2</td>
        <td>Bidest</td>
        <td>actgaaa</td>
        <td>&nbsp;</td>
        </tr>
        
        </table>
        
        <br>Example 3 (RAW columns)
        <table bgcolor=#EFEFEF cellspacing=1>
        <tr bgcolor=#D0D0D0>
        <td>ABSTRACT_SUBST_ID</td>
        <td>BATCH</td>
        <td>c:orijam</td>
        <td>NOTES</td>
        </tr>
        
        <tr bgcolor=#EFEFEF>
        <td>234</td>
        <td>Ethylenglykol</td>
        <td>15</td>
        <td>used only for assay experiment type XZ</td>
        </tr>
        
        <tr bgcolor=#EFEFEF>
        <td>345</td>
        <td>Bidest</td>
        <td>2</td>
        <td>&nbsp;</td>
        </tr>
        </table>
        
     	</ul>
    
       <?php
        htmlInfoBox( "", "", "close" );
    } 

}

