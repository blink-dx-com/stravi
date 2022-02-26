<?php
/**
 * auto insert all entries for CCT_TABLE
 * $Header: trunk/src/www/pionir/rootsubs/init/cct_table_ins.php 59 2018-11-21 09:04:09Z $
 * @package cct_table_ins.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param string $go 
 * @param string $tablename [OPTIONAL] for one table
 */


session_start(); 

require_once ('reqnormal.inc');

class oCCT_TABLE_autoIns{

function oCCT_TABLE_autoIns($go) {
	$this->go = $go;
}

// params see insert_row_s
function insert_row_GetADM( &$sql, $tablename, $argu, $prim_name ) {
	
    $options = array();
	$sqls      = ' VALUES (';
	$sql_cols  = '(';
	$i         = 0;
	
	reset($argu);
	foreach( $argu as $xcol=>$xval) {
	
		if ($i) {// beim ersten val KEIN komma
		$sqls     .=', ';
		$sql_cols .=', ';
		}
		if (($prim_name === $xcol) &&  !$xval){
			$xval = ''; // make 0 to NULL for triggers
		}	 
		$sql_cols .= $xcol;
		if ( $xval === "") $tmpvalSql = 'NULL';
		else {
			$tmpvalSql = $sql->addQuotes($xval);
		
			if ($options["types"]!="") {
				$xtype=$options["types"][$xcol];
				if ($xtype!="") {
					if ($xtype=="DATE1") {
						$tmpvalSql = $sql->DateString2Sql($xval,1);
					}
				}
			}
		}
		
		$sqls .= $tmpvalSql;
		$i++;
	}
	reset($argu);
	
	$sqls = $sql_cols.') '.$sqls.')'; // assemble whole query
	return ($sqls);
}

function insert_row_ADM(   
	&$sql,       
	$tablename,     // db-name of table for insert (in capital letters)  
	$argu,			// array ("COLUMN_NAME" => value, ...) (column_names in capital letters!)
	$prim_name
	) {
# descr: - insert row without checking for CCT_ACCESS (see also insert_row)  
#        - NEW: for ASSOC elements: use class fAssocUpdate()
# return: >0 or "text": OK -> this is the primary key
#	      =0 error

  
  $sqls   = $this->insert_row_GetADM( $sql, $tablename, $argu, $prim_name );
  $retval = $sql->queryInsert($tablename, $sqls, $prim_name); // do insert
  return ($retval);
}

function initData( &$sqlo, $tablename ) {
	$this->tablename = $tablename;
	if ($tablename) {
		$this->entrycnt = 1; 
	} else {
	
		$sqlsel = "count(1) from CCT_TABLE order by TABLE_NAME";
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$this->entrycnt = $sqlo->RowData[0];
	}
	
	if ($this->entrycnt) 
		echo "Number of existing entries: ".$this->entrycnt." <b><font color=red>will be deleted</font></B><br>"; 
}

function allChecks( &$sqlo, &$sqlo2) {
	
	$cond= 'TABLE_NAME is not NULL';
	if ($this->tablename!=NULL) {
		$cond= 'TABLE_NAME='.$sqlo->addQuotes($this->tablename);
	} 
	$sqlo->deletex('CCT_TABLE', $cond);
	
	if ($this->tablename!=NULL) {
		$table = $this->tablename;
		$argu=array(
				'TABLE_NAME'=> $table,
				'NICE_NAME' => strtolower($table),
				'CCT_TABLE_NAME'=> NULL,
				'TABLE_TYPE'=> 'SYS',
				'IS_VIEW' => '0',
				'INTERNAL'=> '0',
				'EXIM'    => '0'
		);
		$this->insert_row_ADM($sqlo, 'CCT_TABLE', $argu, 'TABLE_NAME');
		$cnt = 1;
	} else {
	
		$sqlsel = "NAME from CCT_TAB_VIEW order by NAME";
		$sqlo2->Quesel($sqlsel);
		while ( $sqlo2->ReadRow() ) {
			
			$table = $sqlo2->RowData[0];
			
			$argu=array(
				'TABLE_NAME'=> $table,
				'NICE_NAME' => strtolower($table),
				'CCT_TABLE_NAME'=> NULL,	
				'TABLE_TYPE'=> 'SYS',	
				'IS_VIEW' => '0',	
				'INTERNAL'=> '0',	
				'EXIM'    => '0'
				);
	  		$this->insert_row_ADM($sqlo, 'CCT_TABLE', $argu, 'TABLE_NAME');
			$cnt++;
			echo ". ";
		}
	}
	
	echo "<br>Ready: $cnt elements created.<br> ";
}

function formshow($tablename) {
	require_once ('func_form.inc');
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Prepare";
	$initarr["submittitle"] = "Submit";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = array();
	

	$formobj = new formc($initarr, $hiddenarr, 0);
	
	$fieldx = array (
	    "title" => "Table",
	    "name"  => "tablename",
	    "object"=> "text",
	    "val"   => $tablename,
	    "namex"  => TRUE,
	    "notes" => "OPTIONAL for one table"
	);
	$formobj->fieldOut( $fieldx );
	
	$formobj->close( TRUE );
}

}

// --------------------------------------------------- 
global $error, $varcol;

$error  = & ErrorHandler::get();
$sqlo   = logon2( $_SERVER['PHP_SELF'] );
$sqlo2  = logon2(  );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();
$go		= $_REQUEST['go'];
$tablename  = $_REQUEST['tablename'];

$title		= 'auto insert all entries for CCT_TABLE';
if ($tablename) $title .= ' ONLY for table:'.$tablename;

$infoarr			 = NULL;
$infoarr['scriptID'] = '';
$infoarr['title']    = $title;
$infoarr['form_type']= 'tool'; // 'tool', 'list'
$infoarr['design']   = 'norm';

$infoarr['locrow']   = array( array('index.php', 'Init database') );

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);

if ( !glob_isAdmin() ) {
  htmlFoot('ERROR', 'You must be admin.');
}
$mainLib = new oCCT_TABLE_autoIns($go);
$mainLib->initData($sqlo, $tablename);

if ( !$go ) {
	$mainLib->formshow($tablename);
	$pagelib->htmlFoot();
}

$mainLib->allChecks( $sqlo, $sqlo2);

htmlFoot('');