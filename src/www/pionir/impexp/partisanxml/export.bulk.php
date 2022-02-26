<?php
	// Module: export.bulk.php
	// FUNCTION: export objects, select them by condition
	// header("Content-Type: text/xml");
    // input:   cct_table=PROJ&cct_id=123
    //
    //          or
    //
    //          not up to date
    //            cct_table[0] = PROJ&cct_where[0]=123&cct_table[1] = PROJ&cct_id[1]=124
    //          elements with same index belong together
    //            cct_table[0] = EXTRA_CLASS&cct_WHERE=NAME NOT LIKE '_%'
    //          cct_back
    //          wheremem 0|1 if 1: take where condition from session-vars
	//			$cct_out  (otptional)
	
extract($_REQUEST); 
session_start(); 

	
	require_once("PaXMLWriter.inc");
	require_once("func_head.inc");
    require_once("sql_query_dyn.inc");
    require_once("PaXML_guifunc.inc");
    
    echo "<html><head><title>Paxml Export</title></head>";
    echo "<body bgcolor=white>";
    echo "<ul>";
    echo "<br><font face=Arial,Helvetica size=4 color=black><b>Paxml Export (alternative object selection)</b></font>";
    echo "<hr width=300 noshade size=1 align=left noshade><br>";
	echo "<font face=Arial,Helvetica size=1 color=black>";

	$error = & ErrorHandler::get();
	$sql   = logon2( $_SERVER['PHP_SELF'] );
	
	$retval = paxmlHelpC::exportCheckRole($sql, 1);
	if (!$retval) htmlFoot();
	
	
    if (is_null($cct_out))
        $cct_out = PaXML_TINYVIEW;

	$pxml = new PaXMLWriter($_SESSION['sec']['dbuser'], $_SESSION['sec']['passwd'], $_SESSION['sec']['db'], $_SESSION['sec']['_dbtype'], $cct_out);

	$pxml->startXML();

    $db_handler = logon_to_db($_SESSION['sec']['dbuser'], $_SESSION['sec']['passwd'], $_SESSION['sec']['db'], $_SESSION['sec']['_dbtype'], "new");

    if (is_array($cct_table))
    {
        foreach($cct_table as $idx => $table)
        {   
            // adding alias "x" to the table name to allow where condition from dynamic SQL query builder
            $sql = "SELECT " . $table . "_ID FROM $table x" . (($cct_where[$idx] != null) ? " WHERE " . $cct_where[$idx] : "");
			echo "<font color=gray>exported objects (SQL): </font>". htmlspecialchars($sql). "<br>\n";
            $db_handler->Query($sql);
            while ($db_handler->ReadRow())
       	        $pxml->start($table, array($table . "_ID" => $db_handler->RowData[0]));
        }
    }
    else
    {   
        // adding alias "x" to the table name to allow where condition from dynamic SQL query builder
        $wheretmp    = "";
        $tablenice   = tablename_nice2($cct_table);
        $primary_key = PrimNameGet2($cct_table);
		
        if ($cct_where != null) $wheretmp = " WHERE $cct_where";
		
        $sqlsSelect = "SELECT x." . $primary_key . "    FROM $cct_table x" . $wheretmp;
        $sqlsCnt    = "SELECT count(x.".$primary_key.") FROM $cct_table x" . $wheretmp;
		
        if ($wheremem) {
            // take condition from session
            $sqlAfter     = get_selection_as_sql( $cct_table );
            if ( $_SESSION['s_tabSearchCond'][$cct_table]["info"]=="" ) { 
                  htmlFoot("Error", "no elements selected", "If you want to export <B>$tablenice</B>, you have to select elements first! (Error:1)"  );
            }
            $sqlsSelect = "SELECT x." . $primary_key . " FROM ".$sqlAfter;
			$sqlsCnt    = "SELECT count(x.".$primary_key.") FROM $sqlAfter";	 
        } 
		
		$db_handler->query($sqlsCnt); 
		$db_handler->ReadRow(); 
		$e_cnt = $db_handler->RowData[0];
		if (!$e_cnt) {  
			htmlFoot("Error", "no elements selected", "If you want to export elements from $tablenice, you have to select elements first! (Error:2)"  );    
		}
		
		echo "<font color=gray>Selected elements from $tablenice:</font> <B>$e_cnt</B><br>\n";
        
		$db_handler->Query($sqlsSelect);
		while ($db_handler->ReadRow()) {
			$pxml->start($cct_table, array( $primary_key => $db_handler->RowData[0]) );
		}
        
    }

    $db_handler->Close();

    $pxml->endXML();
    $pxml->tar($pxml->compress());
    $pxml->finish();
	echo "</font></ul></body></html>";

?>
