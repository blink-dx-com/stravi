<?php
/**
 * - component protocol archive
 * - it contains code, which will be included in edit.tmpl.php, when the rider is set to "MODE"
 * $Header: trunk/src/www/pionir/obj.chip_reader.xmode.arch.inc 59 2018-11-21 09:04:09Z $
 * @package    edit.tmpl.php
 * @swreq UREQ:FS-LIM08 Support device/instrument development
 * @subpackage obj.chip_reader.xmode.arch.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

class oCHIP_READER_xmode_arch {
	
function __construct( &$editLib ) {
}		

function xmode_start( &$sqlo, &$sqlo2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {
	
	
	# $tablename  = 'CHIP_READER';
	
	require_once ("o.REA_PRO_ARCH.subs.inc");
	require_once ("visufuncs.inc");
	require_once ("gui/o.CONCRETE_PROTO.edit.inc");
	
	echo "<ul>\n";
	
	$protArchLib = new oREA_PRO_ARCH_subs();
	
	$lastProtoID = $protArchLib->getLastProto($sqlo, $id);
	if ($lastProtoID) {
		$edFormLib = new oConcProtoEdform($sqlo, $lastProtoID);
		$devopt = array();
		$editProtoAllow=0;
		echo '<b>Last version of Component Protocol</b><br />'."\n";
		$edFormLib->showform( $sqlo, $editProtoAllow, $devopt );
		echo "<br />\n";
	}
	
	$dummy=NULL;
	$sqlFrom = $protArchLib->getHistorySql($id);
	$sqlo->Quesel('* '.$sqlFrom);
	$versList=NULL;
	while ( $sqlo->ReadArray() ) {
	    $versid = $sqlo->RowData['VERSID'];
	    $cpid   = $sqlo->RowData['CONCRETE_PROTO_ID'];
	    $versList[] = array($versid, $dummy, $cpid);
	}
	
	$headText ='Archive of the Component Protocols';
	if (empty($versList)) {
		echo $headText.'<br />';
		echo "No history.";
		return;
	}
	
	$tabobj = new visufuncs();
	$headOpt = array( "title" => $headText );
	$headx   = array ("Version", "Date", "Protocol", "Type");
	$tabobj->table_head($headx,   $headOpt);
	foreach( $versList as $dummy=>$dataArr) {
		
		$proto_type='?';
		$cpid = $dataArr[2];
		$PFeatures = glob_elemDataGet3( $sqlo, 'CONCRETE_PROTO', array('CONCRETE_PROTO_ID'=> $cpid), array('EXEC_DATE', 'ABSTRACT_PROTO_ID') );
		$dateProto = $PFeatures['EXEC_DATE'];
		
		$A_Features = glob_elemDataGet3( $sqlo, 'ABSTRACT_PROTO', array('ABSTRACT_PROTO_ID'=> $PFeatures['ABSTRACT_PROTO_ID']), array('H_PROTO_KIND_ID') );
		if ($A_Features['H_PROTO_KIND_ID']) {
			$proto_type = obj_nice_name ( $sqlo, 'H_PROTO_KIND', $A_Features['H_PROTO_KIND_ID'] );
		}
		
	    $dataArr[1]= $dateProto;
	    $dataArr[2]= '<a href="edit.tmpl.php?t=CONCRETE_PROTO&id='.$cpid.'">'.$cpid.'</a>';
	    $dataArr[3]= $proto_type;
	    $tabobj->table_row ($dataArr);
	}
	reset ($versList); 
	$tabobj->table_close();
		
	
}

}