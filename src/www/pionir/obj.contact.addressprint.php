<?php
/**
 * print address
 * @package obj.contact.addressprint.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   id (contact_id)
 * @version0 2001-05-21
 */
session_start(); 
require_once ('reqnormal.inc');

function adressprint ( $texter ) {
	echo $texter,'<br>';
}

$sql = logon2( $_SERVER['PHP_SELF'] );

$id = $_REQUEST["id"];
$pagelib = new gHtmlHead();
$pagelib->PageHeadLight('print address'); 

if (!$id) {
	$pagelib->htmlFoot('ERROR', 'Need id');
}

$sql->query('SELECT contact_person, address_head, street, city, zip, country, name FROM contact WHERE contact_id = '.$id);
if ($sql->ReadRow()){
  echo '<blockquote>'; 
  echo '<pre>';
  adressprint ($sql->RowData[0]);
  if (empty($sql->RowData[1]))
	adressprint ($sql->RowData[6]);
  else
	adressprint ($sql->RowData[1]);
  adressprint ($sql->RowData[2]);
  adressprint ($sql->RowData[4].' '.$sql->RowData[3]);
  adressprint ("\n");
  adressprint ($sql->RowData[5]);
  echo '</pre>';
  
  
  echo '</blockquote>';
} else {
  echo 'ERROR: no contact!<P>';
}

$pagelib->htmlFoot();