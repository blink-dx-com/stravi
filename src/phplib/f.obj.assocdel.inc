<?php
/*MODULE:  f.obj.assocdel.inc
  DESCR:   class to delete all elements ASSOCIATED TABLE ($asoctab) belonging to mother ($tablename) with ID=$id
  		   $go = 0  ... show form
		   $go = 1  ... delete
		   $go = 2  ... go back to object
  AUTHOR:  qbi
  VERSION: 0.1 - 20070918
*/

class gObjAssocDelC {

function __construct($tablename, $id, $asoctab) {
	$this->tablename =$tablename;
	$this->id = $id;
	$this->asoctab =$asoctab;
	
	$this->pkName = PrimNameGet2($tablename);
}

function checkRights(&$sql) {
	global $error;
	$FUNCNAME="checkRights";
	
	$tableNiceName = tablename_nice2($this->tablename);
	
	$t_rights = tableAccessCheck( $sql, $this->tablename );
	if ( $t_rights["write"] != 1 ) {
		$error->set($FUNCNAME, 1, "no access-right 'write' for table $tableNiceName");
		return;
	}
	
	$o_rights = access_check( $sql, $this->tablename, $this->id );
	if ( !$o_rights["write"] or !$o_rights["insert"] ) {
		$error->set($FUNCNAME, 1, "You do not have write and insert permission on this ".$tableNiceName."!");
		return;
	}

}

function getElemNum(&$sql) {
	$this->elementNum = 0;
	
	$sqls = "select count(1) from ".$this->asoctab. " where ".$this->pkName."=".$this->id;
	$sql->query($sqls);
	$sql->ReadRow();
	$retid = $sql->RowData[0];
	$this->elementNum = $retid;
	return($retid);
}

function go0( 
	$formextras=NULL // array of extra form parameters (instead of default params
	) {
	$iopt="";
	$iopt["icon"] = "ic.del.gif";
	htmlInfoBox( "Delete elements &nbsp;&nbsp;", "", "open", "INFO", $iopt );
	echo "<center>";
	echo "<form name=\"delform\" action=\"".$_SERVER['PHP_SELF']."\" method=post>\n";
	echo '<input type="hidden" name=go value="0">'."\n";
	echo '<input type="hidden" name=tablename value="'.$this->tablename.'">'."\n";
	echo '<input type="hidden" name=id value="'.$this->id.'">'."\n";
	
	if (is_array($formextras)) {
		foreach( $formextras as $key=>$val) {
			echo "<input type=\"hidden\" name=\"".$key."\" value=\"".$val."\">\n";
		}
		reset ($formextras); 
	} else {
		echo '<input type="hidden" name=asoctab value="'.$this->asoctab.'">'."\n";
	}
	
	echo "<br>";
	echo "<b>Delete ".$this->elementNum." elements now?</b>";
	echo "<br>";
	echo "<br><input type=button value=\"YES\" onClick=\"document.delform.go.value=1; document.delform.submit();\">\n";
	
	echo " &nbsp;&nbsp;&nbsp;<input type=button value=\"no\" onClick=\"document.delform.go.value=2; document.delform.submit();\">\n";
	
	echo "</form>\n";
 	htmlInfoBox( "", "", "close");
}

function go1( &$sql ) {
	// delete elements
	echo "... delete elements ...<br>";
	$wherestr = $this->pkName."=".$this->id;
	$answer   = $sql->Deletex ( $this->asoctab, $wherestr ); 
	  
}

}