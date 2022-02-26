<?php
require_once ("visufuncs.inc");

/**
 * sub functions to show a query list of queries in a project
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @package o.LINK.c_query_listGui.inc
 */
class oLINK_c_query_listGui {
	var $tablename;
	var $nowProj;
	
function __construct($queriesProj=NULL, $tablename=NULL) {
	global $error, $varcol;
	$FUNCNAME= $this->__CLASS__.':init';
	
	$this->tablename=$tablename;
	$this->nowProj = $queriesProj;
	
	$this->class_id = $varcol->class_name_to_id( "LINK", "query");
	if ( $error->got(READONLY) ) {
		$error->set($FUNCNAME, 1, '"query class" definition missing, please ask the admin!');
		return;
	}
	
	$this->sql_extra1 = "";
	if ($this->tablename!="" ) {
		$nice_stable = tablename_nice2($this->tablename);
		$this->sql_extra1 = " AND S01='".$this->tablename."'"; 
		$this->condition_descr = " for table '$nice_stable'";
	}  
}


function showTable( &$sqlo, $editmode, $title=NULL, $titleNotes=NULL ) {

	$searchtable = $this->tablename;
	$colspan = 2;
	if ( $searchtable=="" ) $colspan++;
	if ( $editmode>0 )      $colspan++;
	
	$base_sql = " from LINK x, (select extra_obj_id, S01 from extra_obj where extra_class_id=".
	   	$this->class_id . $this->sql_extra1.") o".
	   	" where (x.LINK_ID in ".
	   	" (select prim_key FROM proj_has_elem WHERE proj_id = ".$this->nowProj." AND table_name='LINK')) ".
	   	" AND (x.extra_obj_id=o.EXTRA_OBJ_ID)";
	
	   	
	$sqlsel = 'count(1) '.$base_sql;
   	$sqlo->Quesel($sqlsel);
   	$sqlo->ReadRow();
   	$cnt = $sqlo->RowData[0];
   	
   	if (!$cnt) {
   	    htmlInfoBox( 'No query-document found', "No query-document ".$this->condition_descr.
   	        " found in folder ".$this->nowProj.". You can save new queries in the object list views.", 
   	        "", "INFO");
   	    return;
   	}
	
	$headOpt = array( );
	if ($title!=NULL) {
		$headOpt["title"] = '<img src="images/i13.qsearch.gif"> ' . $title . ' '.$titleNotes;
	}
	$headx  = array ("Query name");
	if ( $editmode>0 ) $headx[] = "edit";
	if ( $searchtable=="" ) {
		$headx[] = "searched table";
	}
	$headx[] = "Notes";
	
	$tabobj = new visufuncs();
	$tabobj->table_head($headx,   $headOpt);

	
	$sqls = "select x.link_id, x.name, x.notes, x.extra_obj_id, o.S01" . $base_sql.
			" ORDER by o.S01, x.NAME";  
	           
	$sqlo->query($sqls); 
	$cnt=0;
	$ENTRIES_MAX_SHOW=50;
	
	
	while ( $sqlo->ReadRow() ) {
		
		if ( $cnt>$ENTRIES_MAX_SHOW ) break;
		
		$link_id =  $sqlo->RowData[0]; 
		$name    =  $sqlo->RowData[1];  
		$notes   =  $sqlo->RowData[2];    
		$tabdest =  $sqlo->RowData[4]; 
		
		
		
		$dataArr = array();
		
		$dataArr[] = "<a href=\"obj.link.c_query_exec.php?id=".$link_id.
			"&myqu_go=1\"><img src=\"images/icon.LINK.gif\" border=0> ".htmlspecialchars($name)."</a>";
		
		if ($editmode>0) {
			
			$dataArr[] = "<a href=\"edit.tmpl.php?t=LINK&id=".$link_id."\">".
				" <img src=\"images/i15.edit.gif\" border=0 title=\"edit\"></a>"; 
		}
		
		if ( $searchtable=="" ) {
			$html_dest = tablename_nice2($tabdest); 
			$icon = htmlObjIcon($tabdest, 1); 
			$dataArr[] = '<span style="color:gray;"><img src="'.$icon.'"> '.$html_dest.'</span>';
		} 
		$notes_len = strlen($notes);
		$html_notes="&nbsp;";
		if ($notes_len) {
			$html_notes = htmlspecialchars( substr($notes, 0, 50) ); 
			if ($notes_len>50) $html_notes=$html_notes. "...";
		}   
		$dataArr[] = "<I>".$html_notes."</I>"; 
		
		
		
		$tabobj->table_row ($dataArr);
		   
		$cnt++;
	} 
	$tabobj->table_close();
	
	
		   
	if ( $cnt>$ENTRIES_MAX_SHOW ) { 
		echo "<font color=gray>... more queries</font><br>\n";
	}
	return $cnt;
}
	
	
}
