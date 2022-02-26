<?php
/**
 * download a document
 * @package obj.sattach.down.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $t
  		  $id
		  $rel_id
          [$mime] show/download with special mime-type (overwrite mime of the document)
 */
session_start(); 


require_once ("reqnormal.inc");
require_once ("down_up_load.inc");
require_once ("o.SATTACH.subs.inc");

$t =$_REQUEST['t'];
$id=$_REQUEST['id'];
$rel_id=$_REQUEST['rel_id'];
$mime=$_REQUEST['mime'];

$tablename = $t;
$sql = logon2( $_SERVER['PHP_SELF'] );

if ($tablename=="" OR $id<=0 OR $rel_id<=0) {
	echo "Error: missing attachment ID<br>";
  	return;
}


$tmp_rights = access_check($sql, $tablename, $id);
$satObj     = new cSattachSubs();

if ($tmp_rights["read"]==0) {
  echo "Error: no read rights for object: $tablename, $id.<br>";
  return;
}

$attachFile = $satObj->getDocumentPath($tablename, $id, $rel_id);

if( !file_exists($attachFile) ) {
  echo($attachFile." --- No File Found");
  return;
}

$ret = $sql->query("SELECT name, mime_type FROM sattach WHERE table_name='".$tablename."' AND OBJ_ID=".$id." AND REL_ID=".$rel_id);
$sql->ReadRow();

$tmp_filename = $sql->RowData[0];
$link_mime    = $sql->RowData[1];
if ($mime!="") $link_mime=$mime; // overwrite mime-type from document

$file_size     = filesize($attachFile);

if (!empty($link_mime)) {
  set_mime_type($link_mime, $tmp_filename);
} else {
  set_mime_type("application/octet-stream", $tmp_filename);
}

header('Content-Length: '.$file_size);
readfile( $attachFile ,"r");