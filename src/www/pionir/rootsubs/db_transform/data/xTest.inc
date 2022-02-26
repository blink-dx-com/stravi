<?php
/**
 * tests and repair actions
 * @author steffen
 *
 */
class gDB_tranData_xTest {


function getData( &$dataArray, $option=NULL ) {

$dataArray["info"] = "TESTs !!!";
$dataArray["reqs"] = array(
				"DbVersionNeed" => "1.0.0.0",
				"DbVersionNew"  => "1.0.0.0 - 20000000 - 09:29"
				);
$insarr   = NULL;

// repair
	$insarr[] = "/* repair CART_BATCH */";
	$insarr[] = "alter table CART_BATCH
	add constraint FK_ACCESS2CART_BATCH foreign key (cct_access_id)
		references cct_access (cct_access_id)
		on delete cascade";
	
		
$dataArray["sql"]  = &$insarr;
$dataArray["meta"] = &$meta;
}

}
