<?php
/*MODULE: glob.objtab.sub.inc
  DESCR:  sub functions of table lists
  AUTHOR: qbi
  VERSION: 0.1 - 20050828
*/

require_once ("object.subs.inc");

class objtabSubC {

/**
 * get normal columns ( no CCT_ACCESS_ID, no PRIMARY_KEY, no EXTRA_OBJ_ID )
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param array $option  "withpk" => [0],1 give also primary keys
 *				  		 "withEXOB" => [0],1 give also EXTRA_OBJ_ID		 
 */
function colsGetNorm( &$sql, $tablename, $option=NULL ) {
	
	$cnt=0;
	$colNames  = columns_get2($tablename); 
	$colarr    = NULL;
	$saveEXOBJ = 0; 
	
	foreach($colNames as $th1) {
	
	
			$colName   = $th1;
			$showcol   = 1;
			$colInfos  = "";

			$colInfos = colFeaturesGet( $sql, $tablename, $colName, 0 ) ; 
			
			if ( $colInfos["VISIBLE"]=="0"  ) {
				$showcol = 0; 
			}

			if ( $_SESSION['sec']['appuser'] == 'root' ) $showcol=1;  //  all allowed

			if ( $colInfos["PRIMARY_KEY"]>"0" AND !$option["withpk"] ) { 
				$showcol = 0;  // no primary keys
			}

			if ($colName == "CCT_ACCESS_ID") {
				$showcol=0;
			}
			if ( $colName=="EXTRA_OBJ_ID" ) {
				$showcol=0;  
			} 
			if ( $colName=="EXTRA_OBJ_ID" and $option["withEXOB"] ) {
				$saveEXOBJ = 1; 
			}

			if ($showcol) {
				$nicename = $colInfos['NICE_NAME']; 
				$colarr[$colName] = $nicename;
			}
	} 
	

	if ($saveEXOBJ) {
		$colarr['EXTRA_OBJ_ID'] = 'extra class';
	}

	return ($colarr);
}

}