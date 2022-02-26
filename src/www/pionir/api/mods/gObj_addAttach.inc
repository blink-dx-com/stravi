<?php

require_once("o.SATTACH.mod.inc");
require_once("o.SATTACH.subs.inc");

/**
 * - add attachment to object of a table
 * - attach file with key via XML-RPC
 * @swreq UREQ: 0001423
 * @package gObj_addAttach.inc
 * @author Marco Albrecht (marco_a@clondiag.com)
**/
class gObj_addAttach_XRPC extends XRPC_BASE {
	


function __checkParams() {
	global $error;
	$FUNCNAME= '__checkParams';
	
	$sqlo = $this->_sqlo;

	$table_name = $this->_params[0];
	$obj_id		= $this->_params[1];
	$dataEnc	= $this->_params[2];
	$key		= $this->_params[3];
	$name		= $this->_params[4];
	$notes		= $this->_params[5];

	$this->__objectCheck($sqlo, $table_name, $obj_id);
	if ( $error->got(READONLY) ) {
		return -1;
	}		
	
	if ($name == null){
        $error->set( $FUNCNAME, 1, 'no filename given' );
		return -1;
	}
	
	//maximum size of decoded data
	$max_data_size_dec = 3145728; //Bytes
	
	//maximum size of encoded data
	$max_data_size_enc = $max_data_size_enc * 1.40; //Bytes
	
	$data_size = strlen($dataEnc);
	if ($data_size_enc > $max_data_size_dec){
		$error->set( $FUNCNAME, 1, 'Encoded data file bigger than ca. 4 MByte: '.$data_size_enc. " Byte.");
		return -1;
	}
	
	$data_size_dec = strlen(base64_decode( $dataEnc ));
	if ($data_size_dec > $max_data_size_dec){
		$error->set( $FUNCNAME, 1, 'Source data file bigger than ca. 3 MByte: '.$data_size_dec. " Byte.");
		return -1;
	}
	
	$attach_rights = access_check( $sqlo, $table_name, $obj_id);
   	if ($attach_rights["insert"]==0) {
    	 $error->set( $FUNCNAME, 11, "No right to insert attachment to object:".$table_name.":".$obj_id ); 
   	}
	
	return 1;
		
}

/**
 * run
 * @param   array $this->f_param: <pre>
 * 			[0] $tablename (name of table),
 * 			[1] $obj_id (id of object),
 * 			[2] $dataEnc (BASE64-encoded data),
 * 			[3] $key (key of the attachment)
 * 			[4] $name (name of attachment)
 * 			[5]	$notes (notes of the attachment)
 * </pre>
 * @return 	$rel_id (position of the attachment on the object)
 */	
function &run() {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;	
	
	$sqlo = $this->_sqlo;
		
	$table_name = $this->_params[0];
	$obj_id		= $this->_params[1];
	$dataEnc	= $this->_params[2];
	$key		= $this->_params[3];
	$name		= $this->_params[4];
	$notes		= $this->_params[5];
	
	$atmodLib = new oSattachMod();
	$atsubLib = new cSattachSubs();
	
	$atmodLib->setObj($table_name, $obj_id);
  	   	
   	$data = base64_decode( $dataEnc );
   	
   	$getRelIDbyName = "REL_ID from SATTACH where NAME = '".$name."' and TABLE_NAME = '".$table_name.
   					"' and OBJ_ID = ".$obj_id;
   	$sqlo->Quesel($getRelIDbyName);
   	$sqlo->ReadRow();
   	$rel_id = $sqlo->RowData[0];

	$argu["KEY"]	= $key;
	$argu["NOTES"]	= $notes;
	$argu["NAME"] 	= $name;
   	if (!$rel_id) {
		$rel_id    = $atmodLib->insertAtt( $sqlo, $argu );
		if ($error->Got(READONLY))  {
			$error->set($FUNCNAME,12,"Insert of attachment to object:".$table_name.":".$obj_id." failed." );
		}
		$attach_text = "Attachment ".$name." added to object:".$table_name.":".$obj_id;
   	}else{
   		$atmodLib->update($sqlo, $rel_id, $argu);
   		if ($error->Got(READONLY))  {
			$error->set($FUNCNAME,13,"Update of attachment on object:".$table_name.":".$obj_id." failed." );
		}
   		$attach_text = "Attachment ".$name." update on object:".$table_name.":".$obj_id;
   	}
  	$dest_path = $atsubLib->getDocumentPath($table_name, $obj_id, $rel_id);
  	$fp = fopen($dest_path, 'w');
  	if($fp==false) {
  		$error->set($FUNCNAME, 14, "Failed to write file: ".$dest_path );
  	}
  	fwrite ( $fp, $data );
  	fclose($fp);

	return $attach_text;
    		
	}
	
}