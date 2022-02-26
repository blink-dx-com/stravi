<?php
/**
 * meta function s for html-tables
 * $Header: trunk/src/phplib/f.visuTables.inc 59 2018-11-21 09:04:09Z $
 * @package f.visuTables.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

require_once ("visufuncs.inc");

class fVisuTables {

/**
 * show summary table
 * @param array $datain  array[key] =  $value [ keys: ok, cnt, error, warn ]
 * @param array $mapNice array[key] = $niceKey
 */
function showSummary( $datain, $mapNice=NULL ) {
	$infostyle = array( 
		'ok'   =>array('sty'=>'color: green; font-weight:bold;', 'nice'=>'' ), 
		'cnt'  =>array('sty'=>'', 'nice'=>''), 
		'error'=>array('sty'=>'color: #FF0000; font-weight:bold;', 'nice'=>''), 
		'warn' =>array('sty'=>'color: #606000; font-weight:bold;', 'nice'=>'warning')
	 );

	$tabobj = new visufuncs();
	$dataArr= NULL;

	foreach( $datain as $key=>$value) {
		
		$onestyle = $infostyle[$key]['sty'];
		if ( $value>0 OR $key=='cnt' OR $key=='cnt' ) {
			$keyOut = $key;
			if ( $infostyle[$key]['nice']!='' ) $keyOut = $infostyle[$key]['nice'];
			if ( $mapNice[$key]!=NULL ) {
				$keyOut =  $mapNice[$key];
			} 
			$tabrow    = array( '<font style="'.$onestyle.'">'.$keyOut.'</font>', '<font style="'.$onestyle.'">'.$value.'</font>');
			$dataArr[] = $tabrow;
		}
	}
	reset ($datain); 
	
	$headOpt = array( "title" => "Summary", "headNoShow" =>1);
	$headx   = array ("Key", "Val");
	$tabobj->table_out2($headx, $dataArr,  $headOpt);
}

/**
 * show key-value-pairs, no HTML tags!
 * @param $datain
 * @return unknown_type
 */
function showKeyValRaw(&$datain, $title) {
	reset ($datain);
	$newarr=NULL;
	foreach( $datain as $key=>$val) {
		$newarr[] = array($key,htmlspecialchars($val));
	}
	reset ($datain); 
	
	
	// glob_printr( $newarr, "array info" );
	
	$tabobj  = new visufuncs();
	$headOpt = array( "title" => $title, "headNoShow" =>1);
	$headx   = array ("Key", "Val");
	$tabobj->table_out2( $headx, $newarr,  $headOpt);
}

}