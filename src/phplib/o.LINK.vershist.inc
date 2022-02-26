<?php
/*MODULE:  obj.link.vershist.inc
  DESCR:   - show version history
  AUTHOR:  qbi
  VERSION: 0.1 - 20060418
*/
require_once ("o.LINK.subs.inc");
require_once ("o.LINK.versctrl.inc");
require_once ("visufuncs.inc");

class oLINKvershist {

function __construct(&$versionObj) {
	$this->versionObj=$versionObj;
}

function accinfo(&$sql, $id) {
	// get 'write'-access
	
	$sqls = "select cct_access_id from LINK where link_id=".$id;
	$sql->query($sqls);
	$sql->ReadRow();
	$cct_access_id  = $sql->RowData[0];
	$g_rights = access_getAllRights($sql, $cct_access_id);
	
	foreach( $g_rights as $grpid=>$rigarr) {
		$grpname = obj_nice_name    ( $sql, "USER_GROUP", $grpid);
		if ($rigarr["write"]) {
			 $writestr.=$wrkom . $grpname;
			 $wrkom=", ";
		}
		if ($rigarr["insert"]) {
			$insstr.= $inkom . $grpname;
			$inkom=", ";
		}
	}
	reset ($g_rights); 
	
	echo "<b><font color=gray>full-write-access:</font></b> ".$writestr."<br>";
	echo "<b><font color=gray>insert-access:</font></b> ".$insstr."<br>";
}

function show(&$sql, &$sql2, $id) {

	$tabobj = new visufuncs();
	$headOpt = array( "title" => "Version history table");
	$headx  = array ("","Main", "Sub", "Creator", "Date", "Size [bytes]" ,"Notes");
	$tabobj->table_head($headx,   $headOpt);
		
	
	$sqls = "select POS from LINKV where link_id=".$id. " order by POS DESC";
	$sql2->query($sqls);
	while ( $sql2->ReadRow() ) {
		$tmppos  = $sql2->RowData[0];
		$tmpdata = $this->versionObj->getInfo($sql, $id, $tmppos);
		$tmpdata["creator"] = obj_nice_name( $sql, "DB_USER", $tmpdata["DB_USER_ID"] );
		
		$tmpfilepath = $this->versionObj->getVersPathEasy($id, $tmppos);
		if ( file_exists($tmpfilepath)) {
			$tmpsize = filesize($tmpfilepath);
		} else {
			$tmpsize="<font color=red>not exist</font>";
		}
		$dataArr=NULL;
		$dataArr[] = "<a href=\"obj.link.versedit.php?id=".$id."&pos=".$tmppos."\"><img src=\"images/arrow.but.gif\" border=0></a>";
		// $dataArr[] = $tmpdata["POS"];
		$dataArr[] = $tmpdata["MAIN_VER"];
		$dataArr[] = $tmpdata["SUB_VER"];
		$dataArr[] = $tmpdata["creator"];
		$dataArr[] = $tmpdata["CDATE"];
		$dataArr[] = $tmpsize;
		$dataArr[] = htmlspecialchars(substr($tmpdata["NOTES"],0,50));
		$tabobj->table_row ($dataArr);
		
	}
	
	$tabobj->table_close();
	
	// access info
	echo "<br><br>";
	htmlInfoBox( "<img src=\"images/icon.USER_GROUP.gif\"> <b>Group Access info</b>", "", "open", "INFO" );
	$this->accinfo($sql, $id);
	htmlInfoBox( "","","close");
}

}